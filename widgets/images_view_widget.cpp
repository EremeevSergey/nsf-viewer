#include <QRegion>
#include <QScrollBar>
#include <QFontMetrics>
#include <QItemSelectionRange>
#include <QPainter>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QImage>
#include "images_view_widget.h"
#include "project_model.h"
#include "core/image_field.h"

CImagesViewWidget::CImagesViewWidget(QWidget *parent):
    QAbstractItemView(parent)
{
    columnCount = 4;
    imageWidth  = minImageSize;
    imageHeight = minImageSize;
    setItemDelegate(new CImageListDelegate());
    setSelectionMode(ContiguousSelection);

}

void CImagesViewWidget::setModel(QAbstractItemModel *model)
{
    QAbstractItemView::setModel(model);
}

void CImagesViewWidget::setColumnCount(int count)
{
    if (count>0){
        columnCount = count;
        updateGeometries();
    }
}


QRect CImagesViewWidget::visualRect(const QModelIndex &index) const
{
    QRect rect;
    if (index.isValid()){
        int row = index.row();
        if (row>=0 && row<model()->rowCount()){
            rect = QRect(row%columnCount*imageWidth    - horizontalScrollBar()->value(),
                         (row/columnCount)*imageHeight - verticalScrollBar()->value(),
                         imageWidth,imageHeight);
        }
    }
    return rect;
}

void CImagesViewWidget::scrollTo(const QModelIndex &index,QAbstractItemView::ScrollHint)
{
    QRect viewRect = viewport()->rect();
    QRect itemRect = visualRect(index);
    if (itemRect.left() < viewRect.left())
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()
                                        + itemRect.left() - viewRect.left());
    else if (itemRect.right() > viewRect.right())
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()
                                        + qMin(itemRect.right() - viewRect.right(),
                                               itemRect.left() - viewRect.left()));
    if (itemRect.top() < viewRect.top())
        verticalScrollBar()->setValue(verticalScrollBar()->value() +
                                      itemRect.top() - viewRect.top());
    else if (itemRect.bottom() > viewRect.bottom())
        verticalScrollBar()->setValue(verticalScrollBar()->value() +
                                      qMin(itemRect.bottom() - viewRect.bottom(),
                                           itemRect.top() - viewRect.top()));
    viewport()->update();
}

QModelIndex CImagesViewWidget::indexAt(const QPoint &in_point) const
{
    QPoint point(in_point);
    point.rx() += horizontalScrollBar()->value();
    point.ry() += verticalScrollBar()->value();
    int row = point.y()/imageHeight;
    int col = point.x()/imageWidth;
    row = row*columnCount + col;
    if (row>=0 && row<model()->rowCount()){
        return model()->index(row, 0, rootIndex());
    }
    return QModelIndex();
}

QModelIndex CImagesViewWidget::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                          Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)
    QModelIndex index = currentIndex();
    if (index.isValid()) {
        int offset = 0;
        if      (cursorAction == MoveLeft)  offset =-1;
        else if (cursorAction == MoveRight) offset =+1;
        else if (cursorAction == MoveUp)    offset =-columnCount;
        else if (cursorAction == MoveDown)  offset =+columnCount;
        else if (cursorAction == MoveHome)  offset =-index.row();
        else if (cursorAction == MoveEnd)   offset =model()->rowCount()-index.row();

        int new_row = index.row()+offset;
        if (new_row>=0 && new_row<model()->rowCount())
            index = model()->index(new_row,index.column(),index.parent());
    }
    return index;
}

int CImagesViewWidget::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int CImagesViewWidget::verticalOffset() const
{
    return verticalScrollBar()->value();
}

void CImagesViewWidget::setSelection(const QRect &rect,
                                     QFlags<QItemSelectionModel::SelectionFlag> flags)
{
    QRect rectangle = rect.translated(horizontalScrollBar()->value(),
                                      verticalScrollBar()->value()).normalized();
    int start=-1;
    int stop=-1;

    for (int i=0;i<model()->rowCount();i++){
        if (QRect(i%columnCount*imageWidth,(i/columnCount)*imageHeight,
                  imageWidth,imageHeight).intersects(rectangle)){
            if (start<0) start = i;
            stop = i;
        }
    }
    if (start>=0 && stop>=0){
        QItemSelection selection(
                    model()->index(start, 0, rootIndex()),
                    model()->index(stop, 0, rootIndex()));
        selectionModel()->select(selection, flags);
    }
    else {
        QModelIndex invalid;
        QItemSelection selection(invalid, invalid);
        selectionModel()->select(selection, flags);
    }
}

QRegion CImagesViewWidget::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    foreach (const QItemSelectionRange &range, selection) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
            for (int column = range.left(); column < range.right();
                 ++column) {
                QModelIndex index = model()->index(row, column, rootIndex());
                region += visualRect(index);
            }
        }
    }
    return region;

}

void CImagesViewWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(viewport());
    painter.setRenderHints(QPainter::Antialiasing|
                           QPainter::TextAntialiasing);
    for (int row = 0; row < model()->rowCount(rootIndex()); ++row){
        QModelIndex index = model()->index(row, 0, rootIndex());
        QRect rect = visualRect(index);
        if (!rect.isValid() || rect.bottom() < 0 ||
                rect.y() > viewport()->height())
            continue;
        QStyleOptionViewItem option = viewOptions();
        option.rect = rect;
        if (selectionModel()->isSelected(index))
            option.state |= QStyle::State_Selected;
        if (currentIndex() == index)
            option.state |= QStyle::State_HasFocus;
        itemDelegate()->paint(&painter, option, index);
        paintOutline(&painter, rect);
    }
}

void CImagesViewWidget::paintOutline(QPainter *painter, const QRect &rect)
{
    painter->save();
    painter->setPen(QPen(palette().dark().color(), 0.5));
    painter->drawRect(rect);
    painter->setPen(QPen(Qt::black, 0.5));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
    painter->drawLine(rect.bottomRight(), rect.topRight());
    painter->restore();
}

void CImagesViewWidget::mousePressEvent(QMouseEvent *event)
{
    QAbstractItemView::mousePressEvent(event);
    setCurrentIndex(indexAt(event->pos()));
}

void CImagesViewWidget::resizeEvent(QResizeEvent*)
{
    updateGeometries();
}

void CImagesViewWidget::updateGeometries()
{
    QRect rect = viewport()->rect();
    int image_size = rect.width()/columnCount;
    if (image_size<minImageSize) image_size=minImageSize;
    if (image_size>maxImageSize) image_size=maxImageSize;
    imageHeight = image_size;
    imageWidth = image_size;
    horizontalScrollBar()->setSingleStep(imageWidth/2);
    horizontalScrollBar()->setPageStep(viewport()->width());
    int col = model()->rowCount()>columnCount ? columnCount:model()->rowCount();
    horizontalScrollBar()->setRange(0,
                                    qMax(0, col*imageWidth - viewport()->width()));
    verticalScrollBar()->setSingleStep(imageHeight/2);
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0,
                                  qMax(0, (imageHeight*(model()->rowCount()+columnCount-1))/columnCount - viewport()->height()));
}



void CImageListDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    if (painter && index.isValid() && index.column()==0){
        int row = index.row();
        const CProjectModel* project = dynamic_cast<const CProjectModel*>(index.model());
        if (project && row>=0 && row<project->getFieldCount()){
            const TField* field = project->getField(row);
            if (field){
                if (option.state & QStyle::State_Selected)
                      painter->fillRect(option.rect,option.palette.brush(QPalette::Highlight));
                else painter->fillRect(option.rect,painter->brush());
                // Уменьшим прямоугольник
                QRect rect(option.rect.x()+imageMargin,
                           option.rect.y()+imageMargin,
                           option.rect.width()-2*imageMargin,
                           option.rect.height()-2*imageMargin);
                QImage img;
                if (field->Type()==TField::EImage){
                    const TImageField* img_field = dynamic_cast<const TImageField*>(field);
                    if (img_field)
                        img = img_field->drawByteArray(project->getData());
                }

                if (img.isNull()) img.load(":/images/raw_data.png");

                float scale=1;
                if (img.width()>img.height())
                    scale = rect.width()/float(img.rect().width());
                else
                    scale = rect.height()/float(img.rect().height());

                if (scale>maxImageScale) scale = maxImageScale;
                img = img.scaled(QSize(img.rect().width()*scale,img.rect().height()*scale));

                painter->drawImage(rect.x() + (rect.width()-img.rect().width())/2,
                                   rect.y() + (rect.height()-img.rect().height())/2,
                                   img);
                return;
            }
        }
    }
    QStyledItemDelegate::paint(painter,option,index);
}


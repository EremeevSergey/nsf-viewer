#include <QPainter>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QAbstractItemView>
#include <QModelIndexList>
#include <QMouseEvent>
#include <QCursor>
#include "hex_view_widget.h"

#include <QDebug>

CHexViewWidget::CHexViewWidget(QWidget *parent):QWidget(parent)
{
    rowData     = nullptr;
    widthInByte = defaultWidthInByte;
    startOffset = 0;
    sizeData    = 0;

    margine     = 4;
    showAscii   = true;
    geometryUpdate();
}

CHexViewWidget::~CHexViewWidget()
{

}

bool CHexViewWidget::setData(const QByteArray* data)
{
    rowData = data;
    clearSelection();
    geometryUpdate();
    update();
    return true;
}

bool CHexViewWidget::setOffset(qint32 offset)
{
    if (rowData && offset>=0 &&
            (offset + sizeData)<=static_cast<quint32>(rowData->size())) {
        startOffset=offset;
        //        clearSelection();
        geometryUpdate();
        repaint();
        return true;
    }
    return false;
}

bool CHexViewWidget::setSizeData(qint32 size)
{
    if (rowData && size>0 &&
            (startOffset + size)<=static_cast<quint32>(rowData->size())) {
        sizeData=size;
        //        clearSelection();
        geometryUpdate();
        repaint();
        return true;
    }
    return false;
}

void CHexViewWidget::setWidthInByte (quint16 i)
{
    if (i>0 && i<sizeData){//rowData.size()) {
        widthInByte=i;
        geometryUpdate();
        repaint();
    }
}



/*
 *   Addr             Bytes
 * XXXXXXXX : XX XX XX XX XX XX ... XX   [[[[[[[...[[
 * XXXXXXXX : XX XX XX XX XX XX ... XX   [[[[[[[...[[
 * XXXXXXXX : XX .. XX
 *    8 + 1+1+3*widthInByte            3
 */

QRect CHexViewWidget::getByteRect(quint32 addr)
{
    QRect ret(0,0,charWidth*2,charHeight); //Два символа на байт
    if (addr>=startOffset && addr<startOffset+sizeData){
        addr-=startOffset;
        ret.moveLeft(byteDrawingOffset+(addr%widthInByte)*3*charWidth);
        ret.moveTop (addr/widthInByte*charHeight);
    }
    return ret;
}

bool CHexViewWidget::getAddrByPoint(const QPoint& pt, quint32& addr)
{
    if (pt.x()>=byteDrawingOffset &&
            pt.x()<(byteDrawingOffset+(3*widthInByte-1)*charWidth)){
        // Попали в зону байтов
        addr = (pt.x()-byteDrawingOffset)/(3*charWidth);
        addr+=pt.y()/charHeight*widthInByte;
        if (addr>=startOffset && addr<startOffset+sizeData) return true;
    }
    else if (showAscii && pt.x()>=charDrawingOffset &&
             pt.x()<(charDrawingOffset+widthInByte*charWidth)){
        // Попали в зону символов
        addr = (pt.x()-charDrawingOffset)/charWidth;
        addr+=pt.y()/charHeight*widthInByte;
        if (addr>=startOffset && addr<startOffset+sizeData) return true;
    }
    return false;
}

QRect CHexViewWidget::getCharByteRect(quint32 addr)
{
    QRect ret(0,0,charWidth,charHeight);
    if (addr>=startOffset && addr<startOffset+sizeData){
        addr-=startOffset;
        ret.moveLeft(charDrawingOffset+(addr%widthInByte)*charWidth);
        ret.moveTop (addr/widthInByte*charHeight);
    }
    return ret;
}


void CHexViewWidget::geometryUpdate()
{
    QFontMetrics fmetr = fontMetrics();
#if (QT_VERSION >= QT_VERSION_CHECK(5,11,0))
    charWidth  = fmetr.horizontalAdvance ("C")+1;
#else
    charWidth  = fmetr.width('C')+1;// horizontalAdvance ("C")+1;
#endif
    charHeight = fmetr.height();

    byteDrawingOffset = margine+(addressNiblLength+1+1+1)*charWidth;
    charDrawingOffset = byteDrawingOffset + (3*widthInByte+2)*charWidth;

    int w = charDrawingOffset;
    if (showAscii) w+=widthInByte*charWidth;
    else w-=3*charWidth;
    int h = (sizeData+widthInByte-1)/widthInByte;
    setFixedSize(w+margine,2*margine+h*charHeight);
}

void CHexViewWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QRect r = event->rect();
    // Пока не обрезаем по горизонтале
    int start = (r.top   ()-margine)/charHeight;
    int stop  = (r.bottom()+charHeight-1)/charHeight;
    QPainter painter;
    painter.begin(this);
    for (;start<=stop;start++)
        drawRow(painter,start*widthInByte);//*widthInByte+startOffset,start*charHeight);
    painter.end();
}

void CHexViewWidget::drawRow(QPainter &painter, quint32 offset)
{
    // Выводим адрес
    quint32 addr = startOffset+offset;
    QRect rect;
    if (addr<startOffset+sizeData){
        rect = getByteRect(addr);
        for (int i=0;i<addressNiblLength;i++){
            drawNibble(painter,addr%16,margine+charWidth*(addressNiblLength-i-1),rect.y());
            addr/=16;
        }
        // Двоеточие
        painter.drawText(QPoint(margine+(addressNiblLength+1)*charWidth,
                                rect.y()+charHeight-painter.fontMetrics().descent()),
                         ":");
        // Данные (байты)
        addr = startOffset+offset;
        QColor color;
        bool selected = false;
        for (int i=0;i<widthInByte && addr<startOffset+sizeData;i++,addr++){
            if ((selected=isSelected(addr,color))){
                painter.fillRect(QRect(rect.x()-charWidth/2,rect.y(),charWidth*3,rect.height()),
                                 QBrush(color));
            }
            quint8 byte = rowData->at(addr);
            drawNibble(painter,byte/16,rect.x(),rect.y());
            drawNibble(painter,byte%16,rect.x()+charWidth,rect.y());
            // Символы
            if (showAscii){
                if (selected)
                    painter.fillRect(QRect(charDrawingOffset+i*charWidth,rect.y(),charWidth,charHeight),
                                     QBrush(color));
                if (byte<0x20) byte='.';
                painter.drawText(QRect(charDrawingOffset+i*charWidth,rect.y(),charWidth,charHeight),
                                 Qt::AlignCenter,QChar(byte));
            }
            rect.moveLeft(rect.x() + 3*charWidth);// Двигаем прямоугольник на место байта
        }
    }
}

void CHexViewWidget::drawNibble(QPainter& painter,quint8 nibble,int x,int y)
{
    nibble&=0xF;
    nibble = nibble<10 ? nibble+'0':nibble-10+'A';
    painter.drawText(QRect(x,y,charWidth,charHeight),Qt::AlignCenter,QChar(nibble));
}

quint32 CHexViewWidget::addressByPoint(int x,int y)
{
    int yy=(y-margine)/charHeight;
    int xx = (x-margine)/charHeight;
    xx-=addressNiblLength+1+1;
    if (xx<0) xx=0;
    return startOffset + (yy*widthInByte) + xx;
}

void CHexViewWidget::addSelection(quint32 start,quint32 size, QColor color)
{
    selectionList.append(TSelection(start,size,color));
    update();
}

void CHexViewWidget::clearSelection()
{
    selectionList.clear();
    update();
}

bool CHexViewWidget::isSelected(quint32 address,QColor& color)
{
    for (QList<TSelection>::iterator i=selectionList.begin() ;i!=selectionList.end();i++){
        if (address >=(*i).Start && address<(*i).Start+(*i).Size){
            color = (*i).Color;
            return true;
        }
    }
    return false;
}

void CHexViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event && event->button() == Qt::LeftButton){
        quint32 addr;
        if (getAddrByPoint(event->pos(),addr)){
            //            qDebug() << QString::number(addr,16) <<Qt::endl;
            emit signalClick(addr);
        }
    }
    QWidget::mousePressEvent(event);
}

void CHexViewWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if ( event->button() == Qt::LeftButton ){
//        qDebug() << "CHexViewer::mouseDoubleClickEvent";
        quint32 addr=0;
        if (getAddrByPoint(event->pos(),addr)){
            emit signalDblClick(addr);
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}

/******************************************************************************\

\******************************************************************************/

CHexViewer::CHexViewer(QWidget* parent):
    QScrollArea(parent)
{
    Project = nullptr;
    hexWidget = new CHexViewWidget();
    setWidget(hexWidget);
    connect(hexWidget,SIGNAL(signalClick(quint32)),
            this,SIGNAL(signalClick(quint32)));
    connect(hexWidget,SIGNAL(signalClick(quint32)),
            this,SIGNAL(signalClick(quint32)));
    connect(hexWidget,SIGNAL(signalDblClick(quint32)),
            this,SIGNAL(signalDblClick(quint32)));
}

void CHexViewer::setProject(const CProjectModel* model)
{
    if (model){
        Project = model;
        connect(model,SIGNAL(destroyed(QObject*)),
                this,SLOT(slotProjectDestroyed(QObject*)),Qt::DirectConnection);

        connect (model,SIGNAL(modelReset()),this,SLOT(slotUpdate()));
    }
}

void CHexViewer::slotProjectDestroyed(QObject *obj)
{
    if (obj)
        Project = nullptr;
    hexWidget->setData(nullptr);
}

void CHexViewer::slotUpdate()
{
    hexWidget->setData(&(Project->getData()));
    hexWidget->setOffset(0);
    hexWidget->setSizeData(Project->getData().size());
    hexWidget->clearSelection();
}

void CHexViewer::slotSelectionChanged(const QItemSelection &selected,
                                      const QItemSelection &deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    hexWidget->clearSelection();
    QItemSelectionModel* view = qobject_cast<QItemSelectionModel*>(sender());
    if (view){
        const QModelIndexList list = view->selectedIndexes();
        for (QModelIndexList::const_iterator i=list.begin();i!=list.end();i++){
            const CProjectModel* project = dynamic_cast<const CProjectModel*>((*i).model());
            if (project){
                const TField* field = project->getField((*i).row());
                if (field) hexWidget->addSelection(field->Start,field->Size,
                                                   palette().brush(QPalette::Highlight).color());
            }
        }
    }
}

void CHexViewer::slotCurrentChanged  (const QModelIndex &current,
                                      const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (current.isValid()){
        const CProjectModel* project = dynamic_cast<const CProjectModel*>(current.model());
        if (project){
            const TField* field = project->getField(current.row());
            if (field) {
                QRect pt1 = hexWidget->getByteRect(field->Start);
                QRect pt2 = hexWidget->getByteRect(field->Start+field->Size);
                QPoint c = hexWidget->mapFromGlobal(QCursor::pos());

                //                qDebug() << "start=" << pt1.top() << "  end=" <<pt2.bottom() << "   cursor=" <<c.y();

                if (c.y()<pt1.top() || c.y()>pt2.bottom()){
                    //                    ensureVisible(pt2.x(),pt2.y());
                    ensureVisible(pt1.x(),pt1.y());
                }
            }
        }
    }
}



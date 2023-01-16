#ifndef IMAGES_VIEW_WIDGET_H
#define IMAGES_VIEW_WIDGET_H
#include <QAbstractItemView>
#include <QStyledItemDelegate>

class CImagesViewWidget:public QAbstractItemView
{
public:
    CImagesViewWidget(QWidget *parent = nullptr);

    void setColumnCount(int count);

    void  setModel(QAbstractItemModel *model) override;
    QRect visualRect(const QModelIndex &index) const override;
    bool  isIndexHidden(const QModelIndex&) const override {return false;}
    void  scrollTo(const QModelIndex &index,
                  QAbstractItemView::ScrollHint) override;
    QModelIndex indexAt(const QPoint &point) const override;
    int  horizontalOffset() const override;
    int  verticalOffset() const override;
    void setSelection(const QRect &rect,
                      QFlags<QItemSelectionModel::SelectionFlag> flags) override;

    QRegion visualRegionForSelection(const QItemSelection &selection) const override;
    void paintEvent     (QPaintEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent    (QResizeEvent*) override;

protected:
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                           Qt::KeyboardModifiers modifiers) override;
    void updateGeometries() override;
private:
    int columnCount; //! Количество колонок
    int imageWidth;  //! Ширина одной картинки
    int imageHeight; //! Высота одной картинки

    void paintOutline(QPainter *painter, const QRect &rect);

    const int minImageSize=64;
    const int maxImageSize=320;
};

class CImageListDelegate:public QStyledItemDelegate
{
    Q_OBJECT
public:
    CImageListDelegate(QObject *parent = nullptr):
        QStyledItemDelegate(parent){
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
private:
    static QImage imageImage;
    const int maxImageScale = 2;
    const int imageMargin   = 8;
};

#endif // IMAGES_VIEW_WIDGET_H

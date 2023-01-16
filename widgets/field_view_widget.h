#ifndef FIELD_VIEW_WIDGET_H
#define FIELD_VIEW_WIDGET_H
#include <QListView>
#include <QStyledItemDelegate>

class CFieldsViewWidget:public QListView
{
    Q_OBJECT
public:
    CFieldsViewWidget(QWidget *parent = nullptr);
protected:
};

class CFieldsListDelegate:public QStyledItemDelegate
{
    Q_OBJECT
public:
    CFieldsListDelegate(QObject *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override;

private:
    static QImage imageLinkTop;
    static QImage imageLinkBottom;
    static QImage imageNoLinkTop;
    static QImage imageNoLinkBottom;
    static QImage imageOverLinkTop;
    static QImage imageOverLinkBottom;

    static QImage imageImage;
    static QImage imageRawData;

    enum ELinkType{
        ENone=0,
        ELink,
        ENoLink,
        EOverlappedLink
    };
};


#endif // FIELD_VIEW_WIDGET_H

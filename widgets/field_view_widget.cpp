#include <QDebug>
#include <QBrush>
#include <QRect>
#include <QPainter>
#include <QPalette>
#include <QImage>
#include <QStyleOptionViewItem>
#include "project_model.h"
#include "field_view_widget.h"
#include "core/image_field.h"


QImage CFieldsListDelegate::imageLinkTop;
QImage CFieldsListDelegate::imageLinkBottom;
QImage CFieldsListDelegate::imageNoLinkTop;
QImage CFieldsListDelegate::imageNoLinkBottom;
QImage CFieldsListDelegate::imageOverLinkTop;
QImage CFieldsListDelegate::imageOverLinkBottom;

QImage CFieldsListDelegate::imageImage;
QImage CFieldsListDelegate::imageRawData;


CFieldsListDelegate::CFieldsListDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{
//    qDebug() << "Loading link images";
    imageLinkTop       .load(":/images/link_top.png");
    imageLinkBottom    .load(":/images/link_bottom.png");
    imageNoLinkTop     .load(":/images/link_no_top.png");
    imageNoLinkBottom  .load(":/images/link_no_bottom.png");
    imageOverLinkTop   .load(":/images/link_over_top.png");
    imageOverLinkBottom.load(":/images/link_over_bottom.png");

    imageImage  .load(":/images/image.png");
    imageRawData.load(":/images/raw_data.png");
}



namespace{
// Параметры основного прямоугольника
const char strBackgraundColor    []= "#7cffa8";
const char strOverlappedLinkColor[]= "#ff9081";
const char strNoLinkColor        []= "#ffcf6e";
const qreal mainRectRadius = 10.0;
const int   margineRect = 3;
const int   margineText = 0;
const int   nameFontIncrease = 2; // На сколько увеличиваем текущий шрифт для имени поля
const int linkImageXOffset = 5;

QSize getStringSize(const QFont& font, const QString& text){
    QFontMetrics fm(font);
    QRect rect = fm.boundingRect(text);
    return QSize(rect.width(),rect.height());
}

QString getPlacementString(const TField* field){
    if (field){
        return QString("%1 - %2 ").
                arg(field->Start,8,16,QChar('0')).
                arg(field->Start+field->Size-1,8,16,QChar('0')).toUpper() +
                QString(QObject::tr("(%3 байт)")).arg(field->Size);
    }
    else return "\?\?\?\?";
}

QString getSizeString(const TField* field){
    if (field->Type()==TField::EImage){
        const TImageField* image = dynamic_cast<const TImageField*>(field);
        if (image) return QString(QObject::tr("Картинка %1x%2")).
                arg(image->Width*2).
                arg(image->Size/image->Width);
    }
    return QString(QObject::tr("Данные %1 байт")).
            arg(field->Size);
}
}

QSize CFieldsListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    if (index.isValid()){
        const CProjectModel* project = dynamic_cast<const CProjectModel*>(index.model());
        if (project && index.row()>=0 && index.row()<project->getFieldCount()){
            const TField* field = project->getField(index.row());
            if (field){
                QFont f(option.font);
                f.setPointSize(option.font.pointSize()+nameFontIncrease);
                f.setBold(true);
                QSize s_name = getStringSize(f,field->Name);
                QSize s_plac = getStringSize(option.font,getPlacementString(field));
                QSize s_size = getStringSize(option.font,getSizeString(field));
                int h = s_name.height() + margineText +
                        s_plac.height() + margineText +
                        s_size.height();
                int w = qMax(s_name.width(),s_plac.width());
                w = margineRect + qMax(w,s_size.width()) + margineRect;
                w += h + margineRect; // Добавляем к ширине место для иконки
                //(ширной такой же, как и высота всех текстов)
                h= margineRect + h + margineRect; //отступы с краёв
                return QSize(w,h);
            }
        }
    }
    return QStyledItemDelegate::sizeHint(option,index);
}



/*


*/

void CFieldsListDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (painter && index.isValid() && index.column()==0){
        int row = index.row();
        const CProjectModel* project = dynamic_cast<const CProjectModel*>(index.model());
        if (project && row>=0 && row<project->getFieldCount()){
            // Проверяем связи с соседними полями
            const TField* field = project->getField(row);
            if (field){
                ELinkType top=ENone;
                if (row>0){
                    const TField* top_field = project->getField(row-1);
                    if (top_field){
                        if      (top_field->Start+top_field->Size > field->Start) top = EOverlappedLink;
                        else if (top_field->Start+top_field->Size < field->Start) top = ENoLink;
                        else top = ELink;
                    }
                }
                ELinkType bottom=ENone;
                if (row<project->getFieldCount()-1){
                    const TField* bottom_field = project->getField(row+1);
                    if (bottom_field){
                        if      (field->Start+field->Size > bottom_field->Start) bottom = EOverlappedLink;
                        else if (field->Start+field->Size < bottom_field->Start) bottom = ENoLink;
                        else bottom = ELink;
                    }
                }

                // Рисуем прямоугольник
                QRect mainRect = option.rect;
                mainRect.setHeight(mainRect.height()-1);
                mainRect.setWidth(mainRect.width()-1);
                if (option.state & QStyle::State_Selected)
                    painter->setBrush(option.palette.brush(QPalette::Highlight));
                else{
                    if (top==EOverlappedLink || bottom==EOverlappedLink)
                        painter->setBrush(QBrush(QColor(strOverlappedLinkColor)));
                    else if (top==ENoLink || bottom==ENoLink)
                        painter->setBrush(QBrush(QColor(strNoLinkColor)));
                    else
                        painter->setBrush(QBrush(QColor(strBackgraundColor)));
                }
                painter->drawRoundedRect(mainRect,mainRectRadius,mainRectRadius);

                // Выводим текст данных поля

                int x = option.rect.x() + option.rect.height(); // Отступаем ещё высоту, чтобы получить квадрат для иконки
                int y = option.rect.y() + margineRect;
                // Поле название (Имя)
                QFont name_font(option.font);
                name_font.setPointSize(option.font.pointSize()+nameFontIncrease);
                name_font.setBold(true);
                QSize str_size = getStringSize(name_font,field->Name);
                painter->setFont(name_font);
                y+=str_size.height();
                painter->drawText(x,y-QFontMetrics(name_font).descent(),field->Name);
                // Размещение в файле
                QString str = getPlacementString(field);
                str_size = getStringSize(option.font,str);
                painter->setFont(option.font);
                y+=margineText + str_size.height();
                painter->drawText(x,y-option.fontMetrics.descent(),str);
                // Размер в байтах или в пикселях
                str = getSizeString(field);
                str_size = getStringSize(option.font,str);
                y+=margineText + str_size.height();
                painter->drawText(x,y-option.fontMetrics.descent(),str);

                // Рисуем иконку
                QImage* img = &imageRawData;
                if (field->Type()==TField::EImage) img = &imageImage;
                QRect r_img = img->rect();
                QRect r_dest(option.rect.x()+margineRect,
                             option.rect.y()+margineRect,
                             option.rect.height()-2*margineRect,
                             option.rect.height()-2*margineRect);
                painter->drawImage(r_dest,*img,r_img);

                // Рисуем связи
                if (top!=ENone){
                    img = &imageLinkTop;
                    if (top==ENoLink) img = &imageNoLinkTop;
                    else if (top==EOverlappedLink) img = &imageOverLinkTop;
                    QRect r_img = img->rect();
                    QRect r_dest(r_img);
                    r_dest.moveTo(option.rect.x()+linkImageXOffset,option.rect.y());
                    painter->drawImage(r_dest,*img,r_img);
                }
                if (bottom!=ENone){
                    img = &imageLinkBottom;
                    if (bottom==ENoLink) img = &imageNoLinkBottom;
                    else if (bottom==EOverlappedLink) img = &imageOverLinkBottom;
                    QRect r_img = img->rect();
                    QRect r_dest(r_img);
                    r_dest.moveTo(option.rect.x()+linkImageXOffset,option.rect.bottom() - r_img.height());
                    painter->drawImage(r_dest,*img,r_img);
                }
                return;
            }
        }
    }
    QStyledItemDelegate::paint(painter,option,index);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
CFieldsViewWidget::CFieldsViewWidget(QWidget *parent):
    QListView(parent)
{
    setItemDelegate(new CFieldsListDelegate());
    setSelectionMode(ContiguousSelection);
}


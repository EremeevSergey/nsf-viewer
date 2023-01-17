#include <QByteArray>
#include <QStringList>
#include  <QPainter>
#include "image_field.h"

const Qt::GlobalColor TImageField::Palitra16[]={
    Qt::GlobalColor::black,       //0
    Qt::GlobalColor::white,       //1
    Qt::GlobalColor::cyan,        //2 green
    Qt::GlobalColor::yellow,      //3
    Qt::GlobalColor::magenta,     //4
    Qt::GlobalColor::green,       //5
    Qt::GlobalColor::lightGray,   //6
    Qt::GlobalColor::red,         //7
    Qt::GlobalColor::black,       //8
    Qt::GlobalColor::darkBlue,    //9
    Qt::GlobalColor::darkGreen,   //A
    Qt::GlobalColor::darkYellow,  //B
    Qt::GlobalColor::darkMagenta, //C
    Qt::GlobalColor::darkCyan,    //D
    Qt::GlobalColor::darkGray,    //E
    Qt::GlobalColor::blue         //F
};

QImage TImageField::tmpPixmap;

bool TImageField::FromList (const QStringList& slist)
{
    if (slist.size()==1)    {
        bool fl;
        Width = slist.at(0).toInt(&fl);
        if (fl && Width>0) return true;
    }
    return false;
}

QImage& TImageField::drawByteArray(const QByteArray &data,
                                   int pixel_size, int TrasparentIndex) const
{
    int height = (Size+Width-1)/Width;
    tmpPixmap = QImage(Width*2*pixel_size,height*pixel_size,QImage::Format_RGB444);
    tmpPixmap.fill(QColor(Qt::white));

//    int offset=Start;
    int offset=0;//Start;
    int data_size = data.size();
    for (int y=0;y<height && offset<Size;y++){
        for (int x=0;x<Width && offset<Size;x++){
            int pos = (offset&1)? Start + offset-1: Start + offset+1;
            if (pos>=0 && pos < data_size){
                quint8 byte = data.at(pos);
                QColor color(0,0,0,0);
                if (TrasparentIndex<0 || TrasparentIndex>=16 || (byte>>4)!=TrasparentIndex)
                    color = Palitra16[byte>>4];
                drawPixel1(tmpPixmap,color,2*x*pixel_size,y*pixel_size,pixel_size);

                if (TrasparentIndex<0 || TrasparentIndex>=16 || (byte & 0xF)!=TrasparentIndex)
                    color = Palitra16[byte & 0xF];
                else color = QColor(0,0,0,0);
                drawPixel1(tmpPixmap,color,(2*x+1)*pixel_size,y*pixel_size,pixel_size);
            }
            offset++;
        }
    }
    return tmpPixmap;
}

void  TImageField::drawPixel1(QImage& image,const QColor& color,int x,int y,int pixel_size)
{
    if (pixel_size<=4){
        for (int j=0;j<pixel_size;j++)
            for (int i=0;i<pixel_size;i++)
                image.setPixelColor(x+i,y+j,color);
    }
    else{
        QPainter painter;
        painter.begin(&image);
        painter.setBrush(QBrush(color));
        painter.drawRect(x,y,pixel_size,pixel_size);
        painter.end();
    }
}



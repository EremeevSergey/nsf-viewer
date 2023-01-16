#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QWheelEvent>
#include <QApplication>
#include <QFontMetrics>
#include <QScrollBar>
#include "preview_widget.h"


CPreviewWidget::CPreviewWidget(QWidget *parent):
    QScrollArea(parent)
{
    currentScale=1;
    connect(horizontalScrollBar(),SIGNAL(actionTriggered(int)),
            this,SLOT(update()));
    connect(verticalScrollBar(),SIGNAL(actionTriggered(int)),
            this,SLOT(update()));
}

void CPreviewWidget::setImage(const QImage& image)
{
    currentScale=1;
    currentImage = image;
    geometryUpdate();
    update();
}

void CPreviewWidget::setScale(int scale)
{
    if (currentScale!= scale && scale>0 && scale<=maxScale){
//        qDebug() << "CPreviewWidget::setScale(" << scale << ")";
        currentScale = scale;
        geometryUpdate();
        update();
    }
}

void CPreviewWidget::paintEvent(QPaintEvent *event)
{
//    qDebug() << "CPreviewWidget::paintEvent()";
    QScrollArea::paintEvent(event);
    QPainter painter(viewport());
    if (!currentImage.isNull()){
        QImage img = currentImage.scaled(QSize(currentImage.rect().width()*currentScale,
                                               currentImage.rect().height()*currentScale));
        QRect r_dst(viewport()->rect());
        if (r_dst.width ()>img.width ()) r_dst.moveLeft((viewport()->rect().width()-img.rect().width())/2);
        if (r_dst.height()>img.height()) r_dst.moveTop ((viewport()->rect().height()-img.rect().height())/2);
        painter.drawImage(r_dst.x()-horizontalScrollBar()->value(),
                          r_dst.y()-verticalScrollBar  ()->value(),
                           img);
        if (currentScale>1){
            QFont f(painter.font());
            f.setPointSize(f.pointSize()*1.1);
            f.setBold(true);
            painter.setFont(f);
            QString str = QString("x%1").arg(currentScale);
            QRect rect = painter.fontMetrics().boundingRect(str);
            rect.moveLeft(viewport()->rect().width()-rect.width());
            rect.moveTop(0);
            painter.fillRect(rect,QBrush(Qt::white));
            painter.drawText(rect.left(),rect.bottom()-painter.fontMetrics().descent(),str);
        }
    }
}

void CPreviewWidget::geometryUpdate()
{
    horizontalScrollBar()->setSingleStep(currentScale);
    horizontalScrollBar()->setPageStep(currentImage.rect().width()/4);
    horizontalScrollBar()->setRange(0,
                                    qMax(0,currentImage.rect().width()*currentScale - viewport()->width()));
    verticalScrollBar()->setSingleStep(currentScale);
    verticalScrollBar()->setPageStep(currentImage.rect().height()/4);
    verticalScrollBar()->setRange(0,
                                    qMax(0,currentImage.rect().height()*currentScale - viewport()->height()));
}

void CPreviewWidget::wheelEvent(QWheelEvent *event)
{
    if (!currentImage.isNull() && (QApplication::keyboardModifiers() & Qt::ControlModifier)){
        if (event->angleDelta().y()>0)  setScale(currentScale*2);
        else                            setScale(currentScale/2);
    }
    QScrollArea::wheelEvent(event);
}

void CPreviewWidget::resizeEvent(QResizeEvent *event)
{
    geometryUpdate();
    QScrollArea::resizeEvent(event);
}

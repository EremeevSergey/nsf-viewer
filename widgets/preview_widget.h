#ifndef PREVIEW_WIDGET_H
#define PREVIEW_WIDGET_H

#include <QScrollArea>
#include <QImage>

class CPreviewWidget:public QScrollArea
{
    Q_OBJECT
public:
    explicit CPreviewWidget(QWidget *parent = nullptr);
    void     setImage(const QImage& image);
    void     setScale(int scale);
private:
    void     geometryUpdate();
private:
    QImage currentImage;
    int    currentScale;
    const int maxScale=16;
protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};

#endif // PREVIEW_WIDGET_H

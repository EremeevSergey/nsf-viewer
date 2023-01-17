#ifndef HEX_VIEW_H
#define HEX_VIEW_H

#include <QList>
#include <QWidget>
#include <QPainter>
#include <QScrollArea>
#include <QItemSelection>
#include "project_model.h"

/**
 * @brief Просмотр QByteArray в hex виде
 *
 * Можно вставлять его в QScrollArea. Прорисовка осуществляется только выдимой
 * области по paint. Адрес, не обрезается до XXXXX0, какой задали, такой и будет
 * в начале строки
 *
 * Выводит в следующем виде:
 *   Addr             Bytes
 * XXXXXXXX : XX XX XX XX XX XX ... XX
 * XXXXXXXX : XX XX XX XX XX XX ... XX
 *
 * Количество выводимых байт в строке можно задавать (\ref setWidthInByte). По
 * умолчанию - 16 байт в строке.
 *
 */
class CHexViewWidget: public QWidget
{
    Q_OBJECT
private:
    const int  defaultWidthInByte = 16;//! Дефолтное количество байт в сроке
    const int  addressNiblLength  = 8 ;//! Длина адреса в ниблах
    const int  dataOffset         = addressNiblLength+3;//! Смещение начала вывода данных
public:
    explicit CHexViewWidget(QWidget* parent=nullptr);
    ~CHexViewWidget();

    /**
     * @brief setData Задать, отображаемые данные.
     *
     * @param data данные
     * @return true - если всё ОК, иначе false.
     */
    bool setData(const QByteArray* data);

    /**
     * @brief setWidthInByte Задаёт количество байт, выводимых в одной строке
     * @param i
     */
    void setWidthInByte (quint16 i);

    /**
     * @brief setOffset Задасть смещение для отображения данных
     * @param offset - Величина смещения
     * @return true - если всё ОК, иначе false.
     */
    bool setOffset(qint32 offset);

    /**
     * @brief setOffset Задасть размер данных (но не более чем размер массива)
     * @param size - Размер данных
     * @return true - если всё ОК, иначе false.
     */
    bool setSizeData(qint32 size);

    /**
     * @brief Добавить выделение
     * @param start Начало
     * @param size  Размер
     * @param color Цвет (по умолчанию QPalette::Highlight)
     */
    void addSelection(quint32 start,quint32 size, QColor color = QPalette::Highlight);

    /**
     * @brief Удалить все выделения
     */
    void clearSelection();

    /**
     * @brief Поиск адреса по точке, заданной координатами
     * @param pt точка
     * @param addr Найденный адрес (если он найден)
     * @return \b true если адрес найден иначе \b false
     */
    bool getAddrByPoint(const QPoint& pt, quint32& addr);

    /**
     * @brief Возвращает прямоугольник, для байта по
     * заданному адресу
     * @param addr
     * @return
     */
    QRect getByteRect(quint32 addr);

    /**
     * @brief Возвращает прямоугольник, для символа
     * заданному адресу
     * @param addr
     * @return
     */
    QRect getCharByteRect(quint32 addr);

protected:
    /**
     * @brief paintEvent Отрисовка актуального содержимого (переопределение)
     * @param event
     */
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
signals:
    void  signalClick   (quint32 addr);
    void  signalDblClick(quint32 addr);
private:
    const QByteArray* rowData ; //! Отображаемый массив.
    quint16     widthInByte ; //! Количество байт в сроке
    quint32     startOffset ; //! Начальный адресс данных
    quint32     sizeData    ; //! Размер отображаемых данных

    int         charWidth   ; //! Ширина символа
    int         charHeight  ; //! Высота символа
    int         margine     ; //! Ширина обрамление
    bool        showAscii   ; //! Показывать также и символы

    int         byteDrawingOffset;//! Отступ от начала виджета, откуда рисуем байты
    int         charDrawingOffset;//! Отступ от начала виджета, откуда рисуем символы

    struct TSelection{
        TSelection(quint32 start,quint32 size,QColor  color){
            Start = start;
            Size  = size;
            Color = color;
        }
        quint32 Start;
        quint32 Size;
        QColor  Color;
    };

    QList<TSelection> selectionList;

    /**
     * @brief Проверить выделен ли адрес или нет
     * @param address Проверяемый адрес
     * @param color   сли адрес выделен, то цвет выделения
     * @return \b true  если адрес попадает в выделенный участок, иначе \b false
     */
    bool              isSelected(quint32 address,QColor& color);

    /**
     * @brief updateGeometry Обновить размеры (геометрию) виджета
     */
    void        geometryUpdate();

    void        drawRow   (QPainter& painter, quint32 offset);
    void        drawNibble(QPainter& painter, quint8 nibble, int x, int y);
    quint32     addressByPoint(int x,int y);
};

class CHexViewer:public QScrollArea
{
    Q_OBJECT
public:
    CHexViewer(QWidget* parent=nullptr);
    void setProject(const CProjectModel* model);

    bool setData(const QByteArray* data){return hexWidget->setData    (data);}
    void setWidthInByte (quint16 i)     {       hexWidget->setWidthInByte(i);}
    bool setOffset(qint32 offset)       {return hexWidget->setOffset(offset);}
    bool setSizeData(qint32 size)       {return hexWidget->setSizeData(size);}
    void addSelection(quint32 start,quint32 size,
                      QColor color = QPalette::Highlight){
        hexWidget->addSelection(start,size,color);
    }
    void clearSelection()               {        hexWidget->clearSelection();}

private:
    CHexViewWidget* hexWidget;
    const CProjectModel* Project;
private slots:
    void slotProjectDestroyed(QObject *obj);
    void slotUpdate();
public slots:
    void slotSelectionChanged(const QItemSelection &selected,
                              const QItemSelection &deselected);
    void slotCurrentChanged  (const QModelIndex &current,
                              const QModelIndex &previous);
signals:
    void  signalClick(quint32 addr);
    void  signalDblClick(quint32 addr);
};

#endif // HEX_VIEW_H

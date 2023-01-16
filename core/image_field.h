#ifndef IMAGE_FIELD_H
#define IMAGE_FIELD_H
#include <QImage>
#include "field.h"
/**
* @brief Картинка в файле
*/
struct TImageField: public TField{
    int        Width; //! Ширина картинки

    /*!
    * \brief Конструктор по умолчанию.
    */
    TImageField():TField(){
        Width = 0;
    }

    /*!
    * \brief Конструктор копирования.
    */
    explicit TImageField(const TImageField& other):TField(other){
        Width = other.Width;
    }

    TImageField(const QString& name,int start,int size,int width):
        TField(name,start,size){
        Width = width;
    }

    virtual EFieldType Type() const override {return EImage;}

    /*!
    * \brief Формирует строковое представление поля (к полям базового
    * класса дописывает ширину картинки).
    * \return Строковое представление поля.
    */
    virtual    QString toString ()  const override{
        return TField::toString() + Delimiter + QString::number(Width);
    }


    /**
    * @brief Возвращает обычную картинку на основе данных из массива
    *
    * Рисует на статической картинке класса \ref tmpPixmap
    * @param data массив, с данными
    * @param pixel_size размер пикселя (по умолчанию 1)
    * @param TrasparentIndex индекс цвета прозрачности (\ref Palitra16)
    * @return картинку
    */
    QImage &drawByteArray(const QByteArray &data, int pixel_size=1,
                          int TrasparentIndex=-1) const;
private:
    static const Qt::GlobalColor Palitra16[]; //! Политра цветов для отрисовки
    static QImage tmpPixmap; //! Для временной отрисовки \ref drawByteArray
    inline static void drawPixel1(QImage &image, const QColor &color, int x, int y, int pixel_size);

private:
    /*!
    * \brief Считывает из списка строк ширину картинки.
    * \param slist - Список дополнительных параметров.
    * У списка должны быть отброшены основные поля, т.е. фактически он
    * должен состоять из одной строки, представляющей собой ширину
    * картинки.
    * \return \b true, если ширину успешно счтитали, иначе \b false.
    */
    virtual    bool FromList   (const QStringList& slist) override;
};

#endif // IMAGE_FIELD_H

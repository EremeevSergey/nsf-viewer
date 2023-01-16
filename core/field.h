#ifndef FIELD_H
#define FIELD_H
#include <QString>
#include <QMetaClassInfo>

/**
* @brief Базовый класс для полей файла.
*/
struct TField{
    /*!
    * \brief Типы полей
    */
    enum EFieldType{
        ERowData = 0,
        EImage
    };

    QString    Name; //! имя поля
    quint32    Start;//! начало поля
    quint32    Size; //! размер поля

    static char Delimiter; //! разделитель полей в текстовом представлении
    static char DelimiterReplacement; //! Заменитель разделителя полей для имени поля
    /*!
         * \brief Создаёт поле из строки
         * \param str строковое представление поля
         * \return Созданное поле, или \b nullptr если не удалось создать поле
         */
    static TField*  createFromString(const QString& str);

    /*!
         * \brief Создает копию поля.
         * \param type Поле, копию которого необходимо создать.
         * \return Созданное поле, или \b nullptr если не удалось создать поле
         */
    static TField*  createCopy    (const TField* orig_field);

    /*!
         * \brief Формирует строковое представление поля
         * \return Строковое представление поля
         */
    virtual    QString toString () const{
        QString tmp(Name);
        return tmp.replace(Delimiter,DelimiterReplacement) + Delimiter +
                QString::number(Start)    + Delimiter +
                QString::number(Size)     + Delimiter +
                QString::number(int(Type()));
        //            return  QString("%1;%2;%3;%4").
        //                    arg(Name).
        //                    arg(QString::number(Start)).
        //                    arg(QString::number(Size)).
        //                    arg(QString::number(int(Type())));
    }

    /*!
    * \brief Конструктор по умолчанию. Содается поле с типом \ref ERowData
    * с нулевым смещением и нулевым размером.
    */
    TField(){
        Start = 0;
        Size  = 0;
    }

    /*!
    * \brief Конструктор копирования.
    */
    explicit TField(const TField& other){
        Name  = other.Name;
        Start = other.Start;
        Size  = other.Size;
    }

    /*!
    * \brief Конструирует поле (\ref ERowData) на основе заданных значений.
    * \param name Имя поля.
    * \param start Начальное смещение поля.
    * \param size Размер поля.
    */
    TField(const QString& name,int start,int size):
        Name(name){
        Start = start;
        Size = size;
    }

    /*!
    * \brief Деструктор - ничего не делает
    */
    virtual ~TField(){;}

    /*!
    * \brief Возвращает тип поля.
    * \return Тип поля
    */
    virtual EFieldType Type() const {return ERowData;}
private:
    /*!
    * \brief Заготовка, для чтения дополнительных параметров полей
    * (не \ref Type, \ref Name, \ref Start и \ref Size).
    * \param slist - Список дополнительных параметров поля. Основные поля
    * должны быть отброшены. В базовом классе ни делает ничего.
    * \return В базовом классе всегда возвращает \b true.
    */
    virtual bool FromList   (const QStringList& /*slist*/){ return true;}
};

typedef TField* TPtrField;
Q_DECLARE_METATYPE(TPtrField);

#endif // FIELD_H

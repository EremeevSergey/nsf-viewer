#ifndef PROJECT_H
#define PROJECT_H
#include <QString>
#include <QStringList>
#include <QList>
#include <QByteArray>
#include <QMetaClassInfo>
#include "field.h"
/*!
 * \brief Класс проекта
 *
 * Содержит данные файла и список полей в этом файле.
 * Позволяет создавать, сохранять, считывать и манипулировать данными проекта.
 */
class CProject
{
public:
    /*!
     * \brief Конструктор по умолчанию, создаёт пустой проект. Для работы с
     * проектом необходимо его считать из файла (\ref open),
     * либо инициализировать его методом \ref create.
     */
    CProject ();

    virtual ~CProject();
    
    /*!
     * \brief Создать "безымянный" проект.
     * \param bin_file_name Имя бинарного файла
     * \return \b true если удалось открыть бинарный файл и создать проект,
     * иначе \b false.
     */
    virtual bool    create  (const QString& bin_file_name);

    /*!
     * \brief Открыть файл проекта. Считывает бинарный файл проекта и все его
     * поля.
     * \param name Имя файла проекта
     * \return \b true если удалось прочитать бинарный файл проекта и его поля,
     * иначе \b false.
     */
    virtual bool    open    (const QString& name);

    /*!
     * \brief Сохранить проект в файле
     * \return \b если удалось сохранить, иначе \b false.
     */
    bool    save    (){return saveAs(fileName);}

    /*!
     * \brief Сохранить проект под заданным именем.
     * \param name Имя сохраняемого файла.
     * \return \b если удалось сохранить, иначе \b false.
     */
    bool    saveAs  (const QString& name);
    

    /*!
     * \brief Возвращает количество полей в проекте.
     * \return Количество полей.
     */
    int     getFieldCount() const {return fieldList.size();}

    /*!
     * \brief Возвращает поле по его позиции.
     * \param pos Позиция поля [0;\ref getFieldCount).
     * \return Указатель на поле или \b nullptr, если поле не найдено.
     */
    const TField* getField(int pos) const;

    TField* getField(int pos);

    TField* operator[] (int index);

    /*!
     * \brief Добавляет поле в проект.
     *
     * Добавление происходит основываясь на
     * смещение \ref TField::Start, т.е. поля с меньшим смещением идут в начале,
     * а с большим в конце.
     * \param field Добавляемое поле.
     * \return позицию в списке полей, куда добавилось поле иначе \b -1
     */
    virtual int addField(TField* field);

    /*!
     * \brief Удаляет поле в заданной позиции
     * \param pos позиция поля в списке полей.
     * \return \b true если поле удалено, иначе \b false.
     */
    virtual bool    removeField(int pos);
    
    /*!
     * \brief Разбить поле на равные части того же типа.
     *
     * Размер результирующих полей не может быть меньше 1. Если размер
     * оригинального поля не делится нацело на \ref count, то последнее поле
     * будет длинее (на остаток от деления).
     * \param pos Позиция разбиваемого поля в списке полей
     * \param count Количество частей, на которое разбивается поле (>1).
     * \return \b true в случае удачи, иначе \b false.
     */
    virtual bool    splitField(int pos,int count);

    /*!
     * \brief Объединить ячейки.
     *
     * Создается ячейка, соответствующая типу первой из объединяемых. Если
     * ибъединяются картинки, ширина картинки объединённой ячеки будет равна
     * первой.
     * \param start_pos Первая из объединяемых ячеек
     * \param end_pos  Последняя из объединяемых ячеек
     * \return \b true в случае удачного объединения иначе \b false.
     */
    virtual bool    mergeFields(int start_pos,int end_pos);

    /*!
     * \brief data
     * \return
     */
    const QByteArray& getData() const {return rowData;}


    /**
     * @brief По заданному адресу возвращает поле и его индекс, если оно найдено
     * или предыдущее поле и его индекс
     * @param addr   интересуемый адрес
     * @param field  указатель на указатель поля (куда возвращать).
     * Если не найдено, и предыдущего нет, то присваивается \b nullptr).
     * @param index  индекс поля (предыдущего поля) или ноль
     * @return \b true если найдено иначе false
     */
    bool     getFieldByAddress(quint32 addr, TField ** const field, int& index);

    QString& getFileName   (){return fileName;}
    QString& getBinFileName(){return binFileName;}

    bool     isOpened(){return flOpened;}

private:
    /*!
     * \brief Очистить все данные проекта
     */
    void           clear();
    QByteArray     rowData;    //!  Данные файла
    QString        fileName;   //!  Имя файла текущего проекта
    QString        binFileName;//!  Имя бинарного файла проекта
    QList<TField*> fieldList;  //!  Список полей проекта
    bool           flOpened;   //!  Проект открыт (создан) или нет
private:
    const static int  sigFileLength          = 16   ;//! Длина сигнатуры файла
    const static int  sigUnknownFileLength   = 16*3 ;//! То что идет после сигнатуры
    const static int  unknown0x140FileLength = 0x140;//! То что идет после сигнатуры
    const static QString strBinFile;
};

#endif // PROJECT_H

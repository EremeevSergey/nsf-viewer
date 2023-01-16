#ifndef PROJECT_MODEL_H
#define PROJECT_MODEL_H

#include <QAbstractTableModel>
#include "core/project.h"

class CProjectModel: public QAbstractTableModel, public CProject
{
    Q_OBJECT
public:
    CProjectModel(QObject *parent = nullptr);

    /*!
     * \brief Открыть файл проекта. Считывает бинарный файл проекта и все его
     * поля.
     * \param name Имя файла проекта
     * \return \b true если удалось прочитать бинарный файл проекта и его поля,
     * иначе \b false.
     */
    virtual bool    open    (const QString& name) override;
    virtual bool    create  (const QString& bin_file_name) override;
    virtual int     addField(TField*)  override;
    virtual bool    removeField(int pos)  override;
    virtual bool    mergeFields(int start_pos,int end_pos) override;
    virtual bool    splitField(int pos,int count) override;

    const TField* getFieldByIndex(const QModelIndex&) const;
    TField*       getFieldByIndex(const QModelIndex&);

    void            sendUpdateField(const QModelIndex &index);

public:
    QVariant data       (const QModelIndex &index,
                         int role = Qt::DisplayRole) const override;
    int      rowCount   (const QModelIndex &parent = QModelIndex()) const override;
    int      columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const override;
private:
    enum EFieldCollums{
        EName = 0,
        EOffset,
        ESize,
        ETotal
    };
    static char strName  [];
    static char strOffset[];
    static char strSize  [];
};

#endif // PROJECT_MODEL_H

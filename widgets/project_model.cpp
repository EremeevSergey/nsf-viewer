#include <QModelIndex>
#include "project_model.h"


char CProjectModel::strName  [] = "Название";
char CProjectModel::strOffset[] = "Адрес"   ;
char CProjectModel::strSize  [] = "Размер"  ;

CProjectModel::CProjectModel(QObject *parent):
    QAbstractTableModel(parent),
    CProject()
{
}

int CProjectModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return getFieldCount();
}

int CProjectModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ETotal;
}

QVariant CProjectModel::data(const QModelIndex &index,int role) const
{
    if (index.isValid() && index.row()>=0 && index.row()<getFieldCount()){
        const TField* field = getField(index.row());
        if (role==Qt::DisplayRole){
            int col = index.column();
            if (col==EName)   return field->Name;
            if (col==EOffset) return field->Start;
            if (col==ESize)   return field->Size;
        }
    }
    return QVariant();
}

QVariant CProjectModel::headerData (int section, Qt::Orientation orientation,
                     int role) const
{
    if (orientation==Qt::Horizontal && role == Qt::DisplayRole){
        if (section==EName)   return strName;
        if (section==EOffset) return strOffset;
        if (section==ESize)   return strSize;
    }
    return QAbstractTableModel::headerData(section,orientation,role);
}

bool CProjectModel::open    (const QString& name)
{
    beginResetModel();
    bool ret= CProject::open(name);
    endResetModel();
    return ret;
}

bool CProjectModel::create  (const QString& bin_file_name)
{
    beginResetModel();
    bool ret= CProject::create(bin_file_name);
    endResetModel();
    return ret;
}

const TField* CProjectModel::getFieldByIndex(const QModelIndex& index) const
{
    const TField* ret(nullptr);
    if (index.isValid())
        ret = CProject::getField(index.row());
    return ret;
}

TField* CProjectModel::getFieldByIndex(const QModelIndex& index)
{
    TField* ret(nullptr);
    if (index.isValid())
        ret = CProject::getField(index.row());
    return ret;
}

int CProjectModel::addField(TField* field)
{
    int ret = CProject::addField(field);
    if (ret>=0){
        beginInsertRows(QModelIndex(),ret,ret);
        endInsertRows();
    }
    return ret;
}

bool CProjectModel::removeField(int pos)
{
    bool ret = CProject::removeField(pos);
    if (ret){
        beginRemoveRows(QModelIndex(),pos,pos);
        endRemoveRows();
    }
    return ret;
}

bool CProjectModel::mergeFields(int start_pos,int end_pos)
{
    bool ret = CProject::mergeFields(start_pos,end_pos);
    if (ret){
        beginRemoveRows(QModelIndex(),start_pos+1,end_pos);
        endRemoveRows();
        sendUpdateField(index(start_pos,0));
    }
    return ret;
}

bool CProjectModel::splitField(int pos,int count)
{
    bool ret = CProject::splitField(pos,count);
    if (ret){
        sendUpdateField(index(pos,0));
        beginInsertRows(QModelIndex(),pos+1,pos+count);
        endInsertRows();
    }
    return ret;
}

void CProjectModel::sendUpdateField(const QModelIndex &index)
{
    if (index.isValid()){
        emit dataChanged(index,index);
    }
}

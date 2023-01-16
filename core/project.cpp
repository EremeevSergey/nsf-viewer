#include <QFile>
#include <QByteArray>
#include <QStringList>
#include <QMessageBox>
#include <QTextStream>
#include <QTextCodec>
#include "project.h"

/******************************************************************************\
|                                                                              |
\******************************************************************************/
const QString CProject::strBinFile="bin_file";

CProject::CProject()
{
    flOpened = false;
}

CProject::~CProject()
{
    clear();
}

void CProject::clear()
{
    for (int i=0,n=fieldList.size();i<n;i++)
        delete fieldList.at(i);
    fieldList  .clear();
    rowData    .clear();
    binFileName.clear();
    fileName   .clear();
    flOpened = false;
}

bool CProject::create  (const QString& bin_file_name)
{
    clear();
    fileName="noname.prj";
    QFile file(bin_file_name);
    if (file.open(QIODevice::ReadOnly)){
        rowData = file.readAll();
        if (rowData.size()>16){
            binFileName = bin_file_name;
            TField* field = new TField("Сигнатура",0,sigFileLength);
            if (addField(field)>=0){
                field = new TField("Что-то",sigFileLength,sigUnknownFileLength+3*unknown0x140FileLength);
                if (addField(field)>=0){
                    flOpened = true;
                    return true;
                }
            }
            delete field;
        }
    }
    return false;
}

bool CProject::open (const QString& name)
{
    clear();
    QFile file(name);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        fileName = name;
        QTextStream ts(&file);
        ts.setCodec("CP1251");

        QString str;
        QStringList slist;
        if (ts.readLineInto(&str)){
            str = str.trimmed();
            slist = str.split('=');
            if (slist.size()==2 && slist.at(0)==strBinFile){
                binFileName = slist.at(1);
                QFile file(binFileName);
                if (file.open(QIODevice::ReadOnly)){
                    rowData = file.readAll();
                    while (ts.readLineInto(&str)){
                        str = str.trimmed();
                        if (str.size()>0 && !str.startsWith("#")){
                            // Не пустая строка и не комментарий
                            if (addField(TField::createFromString(str))<0){
                                QMessageBox mb(QMessageBox::Critical,"Ошибка",QString("Не могу добавить поле \"%1\"").arg(str));
                                mb.exec();
                                return false;
                            }
                        }
                    }
                    flOpened = true;
                    return true;
                }
                else{
                    QMessageBox mb(QMessageBox::Critical,"Ошибка",QString("Не могу открыть бинарный файл \"%1\"").arg(binFileName));
                    mb.exec();
                }
            }
        }
    }
    return false;
}

bool CProject::saveAs  (const QString& name)
{
    QFile file(name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream ts(&file);
        ts.setCodec("CP1251");
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
        ts << QString("%1=%2").arg(strBinFile).arg(binFileName) << Qt::endl;
#else
        ts << QString("%1=%2").arg(strBinFile).arg(binFileName) << endl;
#endif
        // Сохраняем поля
        for (int i=0,n=fieldList.size();i<n;i++){
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
            ts << fieldList.at(i)->toString() << Qt::endl;
#else
            ts << fieldList.at(i)->toString() << endl;
#endif
        }
        fileName = name;
        return true;
    }
    return false;
}

int CProject::addField(TField* field)
{
    if (field){
        if (field->Name.size()>0 && field->Start<static_cast<quint32>(rowData.size()) &&
                field->Size>0){
            field->Size = (field->Start+field->Size <=static_cast<quint32>(rowData.size()))?
                        field->Size : static_cast<quint32>(rowData.size()) - field->Start;
            if (field->Size>0){
                // Теперь ищем куда вставить поле....
                /*int pos=0;
                for (int i=0;i<fieldList.count();i++){
                    TField* tmp = fieldList.at(i);
                    if (tmp && tmp->Start>field->Start) {break;}
                    pos = i+1;
                }
                // Вроде нашли позицию, куда вставлять
                fieldList.insert(pos,field);
                */
                int pos=0;
                QList<TField*>::iterator i = fieldList.begin();
                for (;i!=fieldList.end();i++,pos++){
                    if (*i && (*i)->Start>field->Start) break;
                }
                fieldList.insert(i,field);
                return pos;
            }
        }
    }
    return -1;
}

const TField* CProject::getField(int pos) const
{
    if (pos>=0 && pos<fieldList.size()) return fieldList.at(pos);
    return nullptr;
}

TField* CProject::getField(int pos)
{
    if (pos>=0 && pos<fieldList.size()) return fieldList[pos];
    return nullptr;
}

TField* CProject::operator[](int index)
{
    if (index>=0 && index<fieldList.size()) return fieldList[index];
    return nullptr;
}


bool CProject::removeField(int pos)
{
    if (pos>=0 && pos<fieldList.size()){
        delete fieldList.at(pos);
        fieldList.removeAt(pos);
        return true;
    }
    return false;
}

bool CProject::splitField(int pos,int count)
{
    try {
        if (count>1 && pos>=0 && pos<fieldList.size()) {
            TField* orig = fieldList.at(pos);
            if (orig && orig->Size> static_cast<quint32>(count)){
                int origSize= orig->Size;
                // Первое поле - просто модифицируем оригинальное
                orig->Size/=count; // У оригинального сразу уменьшаем размер
                // поля между первым и последним
                pos++;
                for (int i=1;i<count-1;i++){
                    TField* tmp = TField::createCopy(orig);
                    if (tmp){
                        tmp->Start=orig->Start+orig->Size;
                        fieldList.insert(pos++,tmp);
                        orig = tmp;
                    }
                }
                // Последнее поле
                TField* tmp = TField::createCopy(orig);
                if (tmp){
                    tmp->Start=orig->Start+orig->Size + origSize%count;
                    fieldList.insert(pos,tmp);
                }
                return true;
            }
        }
    }  catch (...) {
        throw;
    }
    return false;
}

bool CProject::mergeFields(int start_pos,int end_pos)
{
    end_pos = (end_pos<fieldList.count())? end_pos: fieldList.count();
    if (start_pos>=0 && start_pos<fieldList.size() && start_pos<end_pos){
        TField* first = fieldList.at(start_pos);
        if (first){
            for (int i=start_pos+1;i<=end_pos;end_pos--){
                TField* curr = fieldList.at(i);
                if (curr) {
                    first->Size=curr->Start + curr->Size - first->Start;
                    delete curr;
                }
                fieldList.removeAt(i);
            }
            return true;
        }
    }
    return false;
}

bool CProject::getFieldByAddress(quint32 addr,TField** const field,int& index)
{
    *field = nullptr;
    index = 0;
    if (fieldList.size()>0){
        int pos = 0;
        for (QList<TField*>::const_iterator i=fieldList.begin();i!=fieldList.end();i++,pos++){
            if (*i){
                if (addr>=(*i)->Start && addr<(*i)->Start +(*i)->Size){
                    *field = *i; index = pos;
                    return true;
                }
                else if ((*i)->Start<addr){
                    *field = *i; index = pos;
                }
            }
        }
    }
    return false;
}

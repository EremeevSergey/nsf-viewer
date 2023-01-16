#include <QFile>
#include <QByteArray>
#include <QStringList>
#include <QMessageBox>
#include <QTextStream>
#include <QTextCodec>
#include "field.h"
#include "image_field.h"

char TField::Delimiter = ';';
char TField::DelimiterReplacement = '_';

TField* TField::createFromString(const QString& str)
{
    TField* ret= nullptr;
    QString tmp_str = str.trimmed();
    QStringList list = tmp_str.split(Delimiter);//';');
    if (list.size()>=4 && list.at(0).trimmed().length()>0){
        bool fl1,fl2,fl3;
        int start = list.at(1).toInt(&fl1);
        int size  = list.at(2).toInt(&fl2);
        int type  = list.at(3).toInt(&fl3);
        if (type==ERowData){
            ret = new TField(list.at(0).trimmed(),start,size);
        }
        else if (type==EImage){
            ret = new TImageField();
            if (ret){
                ret->Name = list.at(0).trimmed();
                ret->Start = start;
                ret->Size  = size;
                if (!ret->FromList(list.mid(4))){
                    delete ret;
                    ret = nullptr;
                }
            }
        }
    }
    return ret;
}

TField* TField::createCopy (const TField* orig_field)
{
    if (orig_field){
        EFieldType type = orig_field->Type();
        if (type == ERowData)
            return new TField(*orig_field);
        else if (type == EImage){
            const TImageField* tmp = dynamic_cast<const TImageField*>(orig_field);
            if (tmp) return new TImageField(*tmp);
        }
    }
    return nullptr;
}


#ifndef ADD_FIELD_DLG_H
#define ADD_FIELD_DLG_H

#include <QDialog>
#include "core/image_field.h"

namespace Ui {
class add_field_dlg;
}

class CAddFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CAddFieldDialog(QWidget *parent = nullptr);
    ~CAddFieldDialog();

    /**
     * @brief Вызывает окно создания (настройки нового поля). Поле создается в
     * куче. Удалять его должны те, кто вызывал данный метод.
     * @param maxDataSize Максимальны размер данных в файле для которого
     * создается поле
     * @param start Смещение (по умолчанию), если задано
     * @param size  Размер (по умолчанию), если задано
     * @param width Для кортинок - ширина (по умолчанию), если задано
     * @return Если поле создано - то указатель но него, иначе \b nullptr
     */
    TField*  Execute(quint32 maxDataSize, int start=-1, int size=-1, int width=-1);

private slots:
    void on_cbType_currentIndexChanged(int index);

private:
    Ui::add_field_dlg *ui;
};

#endif // ADD_FIELD_DLG_H

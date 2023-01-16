#ifndef SPLIT_DIALOG_H
#define SPLIT_DIALOG_H

#include <QDialog>
#include "core/image_field.h"

namespace Ui {
class split_dialog;
}

class CSplitFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CSplitFieldDialog(QWidget *parent = nullptr);
    ~CSplitFieldDialog();

    /**
     * @brief Вывести диалог разбиения ячейки
     * @param field Поле, которое надо разбить
     * @return Если >1, то количество а которое разбиваем ячейку, иначе - \b Cancel
     */
    int Execute(TField* field);

private:
    Ui::split_dialog *ui;
};

#endif // SPLIT_DIALOG_H

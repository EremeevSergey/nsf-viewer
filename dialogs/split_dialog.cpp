#include "split_dialog.h"
#include "ui_split_dialog.h"

CSplitFieldDialog::CSplitFieldDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::split_dialog)
{
    ui->setupUi(this);
}

CSplitFieldDialog::~CSplitFieldDialog()
{
    delete ui;
}

int CSplitFieldDialog::Execute(TField* field)
{
    int ret = -1;
    if (field){
        ui->spinBox->setRange(2,field->Size);
        ui->spinBox->setValue(2);
        if (exec()==QDialog::Accepted)
            ret = ui->spinBox->value();
    }
    return ret;
}

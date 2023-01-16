#include <QPushButton>
#include "add_field_dlg.h"
#include "ui_add_field_dlg.h"

CAddFieldDialog::CAddFieldDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::add_field_dlg)
{
    ui->setupUi(this);
    QPushButton* pb = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if (pb) {
        pb->setAutoDefault(true);
        pb->setDefault(true);
    }

}

CAddFieldDialog::~CAddFieldDialog()
{
    delete ui;
}

TField* CAddFieldDialog::Execute(quint32 maxDataSize, int start, int size, int width)
{
    TField* field = nullptr;
    ui->cbType->clear();
    ui->cbType->addItem("Данные");
    ui->cbType->addItem("Картинка");
    ui->cbType->setDisabled(false);
    ui->cbType->setCurrentIndex(TField::EImage);
    ui->leName->setText("Field ");
    ui->spOffset->setRange(0,maxDataSize);
    if (start>=0) ui->spOffset->setValue(start);
    else          ui->spOffset->setValue(0);

    ui->spSize->setRange(0,maxDataSize-ui->spOffset->value());
    if (size>=0) ui->spSize->setValue(size);
    else         ui->spSize->setValue(16);

    ui->spImageWidth->setRange(0,ui->spSize->value());
    if (width>=0) ui->spImageWidth->setValue(width);
    else          ui->spImageWidth->setValue(16);

    if (exec() == QDialog::Accepted){
        if (ui->cbType->currentIndex()==TField::EImage){
            TImageField* image = new TImageField();
            if (image) image->Width = ui->spImageWidth->value();
            field = image;
        }
        else
            field = new TField();
        if (field){
            field->Name  = ui->leName->text();
            field->Start = ui->spOffset->value();
            field->Size  = ui->spSize->value();
        }
    }
    return field;
}

void CAddFieldDialog::on_cbType_currentIndexChanged(int index)
{
    bool fl_visibl = (index == TField::EImage);
    ui->lbImageWidth->setVisible(fl_visibl);
    ui->spImageWidth->setVisible(fl_visibl);
}

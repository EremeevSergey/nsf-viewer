#include <QDebug>
#include <QPushButton>
#include "edit_field_dlg.h"
#include "ui_edit_field_dlg.h"
#include "utils.h"

CEditFieldDialog::CEditFieldDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::edit_field_dlg)
{
    dataArray = nullptr;
    origField = nullptr;
    result = QDialog::Rejected;
    setWindowFlag(Qt::Window,true);
    setWindowModality(Qt::ApplicationModal);
    ui->setupUi(this);
    hexWidget = new CHexViewer();
    setWidgetTo(hexWidget,ui->hexFrame);
    previewWidget = new CPreviewWidget();
    setWidgetTo(previewWidget,ui->previewFrame);
    imageWidget = new QLabel();
    setWidgetTo(imageWidget,ui->imageScrollArea);
    setWindowTitle(tr("Редактор полей."));
    eventloop = new QEventLoop(this);

    QPushButton* pb = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if (pb) {
        pb->setAutoDefault(true);
        pb->setDefault(true);
    }
}

CEditFieldDialog::~CEditFieldDialog()
{
    delete ui;
}

void  CEditFieldDialog::hideEvent(QHideEvent*)
{
    eventloop->exit();
}

void CEditFieldDialog::on_buttonBox_accepted()
{
    result = QDialog::Accepted;
    close();
}

void CEditFieldDialog::on_buttonBox_rejected()
{
    close();
}

QDialog::DialogCode CEditFieldDialog::Execute(CProject* project,int field_index)
{
    result = QDialog::Rejected;
    if (project){
        TField* field = project->getField(field_index);
        if (field && project->getData().size()>0){
            dataArray = &(project->getData());
            origField = nullptr;
            ui->editName->setText(field->Name);
            ui->spStartOffset->setRange(0,dataArray->size()-field->Size);
            ui->spStartOffset->setValue(field->Start);
            ui->spSize->setRange(1,dataArray->size()-field->Start);
            ui->spSize->setValue(field->Size);
            ui->spSize->setSingleStep(1);

            hexWidget->setData(dataArray);
            hexWidget->setOffset(field->Start);
            hexWidget->setSizeData(field->Size);
            lastSize=field->Size;

            TField* field_other = project->getField(field_index-1);
            if (field_other)
                hexWidget->addSelection(0,field_other->Start +field_other->Size,
                                        "#ffa897");
            field_other = project->getField(field_index+1);
            if (field_other)
                hexWidget->addSelection(field_other->Start,
                                        dataArray->size()-field_other->Start,
                                        "#ffa897");

            TImageField* image_field=nullptr;
            if (field->Type()==TField::EImage &&
                    (image_field=dynamic_cast<TImageField*>(field))){
                // Картинка
                ui->spImageWidth->setRange(1,field->Size);
                ui->spImageWidth->setValue(image_field->Width);
                hexWidget->setWidthInByte(image_field->Width);
            }
            else {
                ui->imageScrollArea->hide();
                ui->spImageWidth->hide();
                ui->labelImageWidth->hide();
                ui->previewFrame->hide();
            }
            origField = field;
            updatePreview();
            updateImage();

            connect(ui->spSize,SIGNAL(valueChanged(int)),
                    this, SLOT(slotSizeChanged(int)));

            show();
            eventloop->exec();
            if (result==QDialog::Accepted){
                // переделываем поле в соответствии с изменениями
                field->Name  = ui->editName->text();
                field->Start = ui->spStartOffset->value();
                field->Size  = ui->spSize->value();
                if (image_field)
                    image_field->Width = ui->spImageWidth->value();
            }
        }
    }
    return result;
}

void CEditFieldDialog::updatePreview()
{
    if (dataArray && origField && origField->Type()==TField::EImage){
        TImageField image_field("",
                                ui->spStartOffset->value(),
                                ui->spSize->value(),
                                ui->spImageWidth->value());
        previewWidget->setImage(image_field.drawByteArray(*dataArray));
    }
}

void CEditFieldDialog::updateImage  ()
{
    if (dataArray && origField && origField->Type()==TField::EImage){
        TImageField image_field("",
                                ui->spStartOffset->value(),
                                ui->spSize->value(),
                                ui->spImageWidth->value());
        imageWidget->clear();
        QImage img(image_field.drawByteArray(*dataArray,8));
        imageWidget->setPixmap(QPixmap::fromImage(img));
        ui->imageScrollArea->setWidget(imageWidget);
    }
}

void CEditFieldDialog::on_spStartOffset_valueChanged(int arg1)
{
    updateImage  ();
    updatePreview();
    hexWidget->setOffset(arg1);
    if (dataArray)
        ui->spSize->setRange(1,dataArray->size()-ui->spStartOffset->value());
}

void CEditFieldDialog::slotSizeChanged(int arg1)
{
    disconnect(ui->spSize,SIGNAL(valueChanged(int)),
               this, SLOT(slotSizeChanged(int)));

    int width = ui->spImageWidth->value();
    if (arg1>lastSize){
        lastSize = arg1-(arg1%width) +width;
    }
    else if (arg1<lastSize){
        lastSize = arg1-(arg1%width);
    }
    hexWidget->setSizeData(lastSize);
    if (dataArray)
        ui->spStartOffset->setMaximum(dataArray->size()-lastSize);
    ui->spImageWidth->setMaximum(lastSize);
    ui->spSize->setValue(lastSize);
    updateImage  ();
    updatePreview();
    connect(ui->spSize,SIGNAL(valueChanged(int)),
               this, SLOT(slotSizeChanged(int)));
}

void CEditFieldDialog::on_spImageWidth_valueChanged(int arg1)
{
    hexWidget->setWidthInByte(arg1);
    updateImage  ();
    updatePreview();
}

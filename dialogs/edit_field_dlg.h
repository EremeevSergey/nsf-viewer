#ifndef EDIT_FIELD_DLG_H
#define EDIT_FIELD_DLG_H

#include <QWidget>
#include <QDialog>
#include <QImage>
#include <QLabel>
#include <QEventLoop>
#include "core/image_field.h"
#include "widgets/hex_view_widget.h"
#include "widgets/preview_widget.h"

namespace Ui {
class edit_field_dlg;
}

class CEditFieldDialog : public QWidget
{
    Q_OBJECT

public:
    explicit CEditFieldDialog(QWidget *parent = nullptr);
    ~CEditFieldDialog();

    QDialog::DialogCode Execute(TField* field,const QByteArray* data);
protected:
    void  hideEvent(QHideEvent *event) override;
    void updatePreview();
    void updateImage  ();
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_spStartOffset_valueChanged(int arg1);

    void on_spSize_valueChanged(int arg1);

    void on_spImageWidth_valueChanged(int arg1);

private:
    Ui::edit_field_dlg *ui;
    CHexViewer*         hexWidget;
    CPreviewWidget*     previewWidget;
    QLabel*             imageWidget;
    QEventLoop*         eventloop;

    const QByteArray*   dataArray;
    TField*             origField;


    QDialog::DialogCode result;
};

#endif // EDIT_FIELD_DLG_H
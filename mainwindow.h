#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QToolBar>
#include "widgets/project_model.h"
#include "core/image_field.h"
#include "widgets/field_view_widget.h"
#include "widgets/hex_view_widget.h"
#include "widgets/images_view_widget.h"
#include "widgets/preview_widget.h"

#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    CProjectModel Project;
    bool          projectModified;
    quint32       lastFieldStart;
    quint32       lastFieldSize;
    quint32       lastFieldWidth;

private:
    QAction* newAction(const QString& name, const char* icon_name,
                       const char* method);

    inline void createActions();
    inline void createToolBar();
    void        updateControls();

    bool         showModifiedDialog();
    TField*      getCurrentField(QModelIndex* current=nullptr);
    TImageField* getCurrentImageField();
    void         updatePreviewImage();
private:
    Ui::MainWindow *ui;
    CFieldsViewWidget* fieldWidget;
    CHexViewer*        hexWidget;
    CImagesViewWidget* imagesWidget;
    CPreviewWidget*    previewWidget;

    QToolBar*       fieldsToolBar;

    QAction*        actNew;
    QAction*        actOpen;
    QAction*        actSave;
    QAction*        actSaveAs;

    QAction*        actAdd;
    QAction*        actMod;
    QAction*        actDel;
    QAction*        actMerge;
    QAction*        actSplit;

private slots:
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveAsProject();

    void onAddField();
    void onModField();
    void onDelField();
    void onMergeFields();
    void onSplitField();

    void onHexWidgetClick   (quint32);
    void onHexWidgetDblClick(quint32);
    void onCurrentChanged   (const QModelIndex&,const QModelIndex&);
    void onSelectionChanged (const QItemSelection&,const QItemSelection&);
    void onDblClick         (const QModelIndex&){onModField();}
};
#endif // MAINWINDOW_H

#include <QAction>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include "core/image_field.h"
#include "dialogs/edit_field_dlg.h"
#include "dialogs/add_field_dlg.h"
#include "dialogs/split_dialog.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    previewWidget = new CPreviewWidget();
    setWidgetTo(previewWidget,ui->previewScrollArea);


    fieldWidget = new CFieldsViewWidget();
    setWidgetTo(fieldWidget,ui->fieldsFrame);
    fieldWidget->setModel(&Project);
    connect(fieldWidget->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this,SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)));
    connect(fieldWidget,SIGNAL(doubleClicked(const QModelIndex&)),
            this,SLOT(onDblClick(const QModelIndex&)));

    hexWidget = new CHexViewer();
    setWidgetTo(hexWidget,ui->hexFrame);
    hexWidget->setProject(&Project);
    connect(fieldWidget->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            hexWidget,SLOT(slotSelectionChanged(const QItemSelection&,const QItemSelection&)));
    connect(fieldWidget->selectionModel(),SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            hexWidget,SLOT(slotCurrentChanged(const QModelIndex&,const QModelIndex&)));
    connect(fieldWidget->selectionModel(),SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this,SLOT(onCurrentChanged(const QModelIndex&,const QModelIndex&)));
    connect(hexWidget,SIGNAL(signalClick(quint32)),
            this,SLOT(onHexWidgetClick(quint32)));
    connect(hexWidget,SIGNAL(signalDblClick(quint32)),
            this,SLOT(onHexWidgetDblClick(quint32)));


    imagesWidget = new CImagesViewWidget();
    setWidgetTo(imagesWidget,ui->imagesFrame);
    imagesWidget->setModel(&Project);
    imagesWidget->setSelectionModel(fieldWidget->selectionModel());
    connect(imagesWidget,SIGNAL(doubleClicked(const QModelIndex&)),
            this,SLOT(onDblClick(const QModelIndex&)));

    projectModified = false;
    lastFieldStart = -1;
    lastFieldSize  = -1;
    lastFieldWidth = -1;

    createActions();
    createToolBar();
    updateControls();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QAction* MainWindow::newAction(const QString& name,const char* icon_name,
                               const char* method)
{
    QAction* act = new QAction(QIcon(QString::fromUtf8(icon_name)),name,this);
    connect(act,SIGNAL(triggered()),this,method);
    return act;
}

void MainWindow::createActions()
{
    actNew    = newAction(tr("?????????? ????????????..."         ),":/images/new.png"   ,SLOT(onNewProject   ()));
    actOpen   = newAction(tr("?????????????? ????????????..."       ),":/images/open.png"  ,SLOT(onOpenProject  ()));
    actSave   = newAction(tr("?????????????????? ????????????"        ),":/images/save.png"  ,SLOT(onSaveProject  ()));
    actSaveAs = newAction(tr("?????????????????? ????????????, ??????..."),":/images/saveas.png",SLOT(onSaveAsProject()));

    actAdd    = newAction(tr("???????????????? ????????..."        ),":/images/add.png"   ,SLOT(onAddField   ()));
    actMod    = newAction(tr("???????????????? ????????..."        ),":/images/edit.png"  ,SLOT(onModField  ()));
    actDel    = newAction(tr("?????????????? ????????"            ),":/images/del.png"   ,SLOT(onDelField  ()));

    actMerge  = newAction(tr("???????????????????? ????????"             ),":/images/merge.png"   ,SLOT(onMergeFields()));
    actSplit  = newAction(tr("?????????????? ???????? ???? ???????????? ??????????..."),":/images/split.png"   ,SLOT(onSplitField  ()));
}

void MainWindow::createToolBar()
{
    QToolBar* tb = new QToolBar(tr("????????"));
    tb->addAction(actNew);
    tb->addAction(actOpen);
    tb->addAction(actSave);
    tb->addAction(actSaveAs);
    addToolBar(Qt::TopToolBarArea,tb);

    fieldsToolBar = new QToolBar(tr("????????"));
    fieldsToolBar->addAction(actAdd);
    fieldsToolBar->addAction(actMod);
    fieldsToolBar->addAction(actDel);
    fieldsToolBar->addAction(actMerge);
    fieldsToolBar->addAction(actSplit);
    addToolBar(Qt::TopToolBarArea,fieldsToolBar);
}

void MainWindow::updateControls()
{

    bool fl = Project.isOpened();
    if (fl){
        QString name(Project.getFileName());
        if (projectModified) name.prepend("*");
        setWindowTitle(name);
    }
    actSave  ->setEnabled(fl && projectModified);
    actSaveAs->setEnabled(fl && projectModified);
    fieldsToolBar->setVisible(fl);

    QModelIndex current = fieldWidget->currentIndex();
    fl = current.isValid();
    actDel->setEnabled(fl);
    QModelIndexList list = fieldWidget->selectionModel()->selectedIndexes();
    actMod->setEnabled(fl && (list.count()==1));

    actSplit->setEnabled(list.count()==1);

    // ???????? ?????????????????? ?????????? ???? ???? ???????????????????? ????????
    // 1. ?????????? ???????????? ???????? ???????????? ????????????
    // 2. ???????????? ???????????????? ???????????? ???????? ????????????????????
    // 3. ????????, ???? ???????????????????? ?????????????????? - ???????? ????????????????
    fl = false;
    int width = 0;
    for (QModelIndexList::iterator i=list.begin();i!=list.end();i++){
        fl = true;
        const TImageField* img_filed = dynamic_cast<const TImageField*>(Project.getFieldByIndex(*i));
        if (img_filed){
            if (width<=0) width = img_filed->Width;
            else if (width != img_filed->Width){
                fl = false;
                break;
            }
        }
    }
    actMerge->setEnabled(fl && list.size()>1);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (showModifiedDialog()) event->accept();
    else                      event->ignore();
}

TField* MainWindow::getCurrentField(QModelIndex* current)
{
    QModelIndex index = fieldWidget->currentIndex();
    TField* ret= Project.getFieldByIndex(index);
    if (current) *current = index;
    return ret;
}

TImageField* MainWindow::getCurrentImageField()
{
    TField* ret = getCurrentField();
    if (ret && ret->Type()==TField::EImage)
        return dynamic_cast<TImageField*>(ret);
    else return nullptr;
}


bool MainWindow::showModifiedDialog()
{
    if (Project.isOpened() && projectModified){
        QMessageBox msgBox;
        msgBox.setText(tr("???????????? ?????? ??????????????."));
        msgBox.setInformativeText(tr("???????????? ?????????????????? ???????????????????"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Save:
            if (Project.save()!=true){
                QMessageBox mb(QMessageBox::Critical,tr("????????????"),QString(tr("???? ???????? ?????????????????? ????????????.")));
                mb.exec();
                return false;
            }
            break;
        case QMessageBox::Cancel:
            return false;
        }
    }
    return true;
}

void MainWindow::onNewProject()
{
    if (showModifiedDialog()){
        QString fname=QFileDialog::getOpenFileName(0,tr("?????????????? ????????????"),".",
                                                   tr("???????????????? ???????? (*.bin);;?????? ?????????? (*.*)"),0);
        if (fname.length()>0 && Project.create(fname)){
            projectModified = true;
            updateControls();
        }
    }
}

void MainWindow::onOpenProject()
{
    if (showModifiedDialog()){
        QString fname=QFileDialog::getOpenFileName(0,tr("?????????????? ????????????"),".",
                                                   tr("?????????? ???????????????? (*.prj)"),0,QFileDialog::ReadOnly);
        if (fname.length()>0 && Project.open(fname)){
            updateControls();
        }
    }
}

void MainWindow::onSaveProject()
{
    if (Project.save()!=true){
        QMessageBox mb(QMessageBox::Critical,tr("????????????"),QString(tr("???? ???????? ?????????????????? ????????????.")));
        mb.exec();
    }
    else{
        projectModified = false;
        updateControls();
    }
}

void MainWindow::onSaveAsProject()
{
    QString fname = QFileDialog::getSaveFileName(0,tr("?????????????????? ????????????"),".",tr("?????????? ???????????????? (*.prj)"),0);
    if (fname.length()>0){
        if (Project.saveAs(fname)!=true){
            QMessageBox mb(QMessageBox::Critical,tr("????????????"),QString(tr("???? ???????? ?????????????????? ????????????.")));
            mb.exec();
        }
        else{
            projectModified = false;
            updateControls();
        }
    }
}

void MainWindow::onAddField()
{
    CAddFieldDialog dlg;
    TField* field = dlg.Execute(Project.getData().size(),
                                lastFieldStart+lastFieldSize,
                                lastFieldSize,lastFieldWidth);
    if (field){
        int index=Project.addField(field);
        if (index>=0){
            projectModified = true;
            fieldWidget->setCurrentIndex(fieldWidget->model()->index(index,0));
        }
        else {
            QMessageBox mb(QMessageBox::Critical,tr("????????????"),QString(tr("???? ???????? ???????????????? ????????...")));
            mb.exec();
            delete field;
        }
        updateControls();
    }
}

void MainWindow::onModField()
{
    QModelIndex current;
    TField* field = getCurrentField(&current);
    if (field){
        CEditFieldDialog dlg;
        if (dlg.Execute(&Project,current.row())){
            projectModified = true;
            Project.sendUpdateField(current);
            updateControls();
        }
//        if (dlg.Execute(field,&(Project.getData()))==QDialog::Accepted){
//            projectModified = true;
//            Project.sendUpdateField(current);
//            updateControls();
//        }
    }
}

void MainWindow::onDelField()
{
    QModelIndexList list = fieldWidget->selectionModel()->selectedIndexes();
    if (list.size()>0){
        QMessageBox msgBox;
        msgBox.setText(tr("???????????????? ????????(????)."));
        msgBox.setInformativeText(tr("?????????? ???????????? ?????????????? %1 ???????????").arg(list.size()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec()==QMessageBox::Yes){
            for (int i=0;i<list.size();i++)
                Project.removeField(list.at(0).row());
            projectModified = true;
            updateControls();
        }
    }
}

void MainWindow::onMergeFields()
{
    //qDebug() << fieldWidget->selectionModel()->selectedIndexes();
    QModelIndexList list=fieldWidget->selectionModel()->selectedIndexes();

    if (list.size()>0){
        QMessageBox msgBox;
        msgBox.setText(tr("?????????????????????? ??????????."));
        msgBox.setInformativeText(tr("?????????? ???????????? ???????????????????? %1 ???????????").arg(list.size()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec()==QMessageBox::Yes){
            if (Project.mergeFields(list.first().row(),list.last().row())){
                projectModified = true;
                fieldWidget->setCurrentIndex(list.first());
                fieldWidget->scrollTo(list.first());
            }
        }
    }
}

void MainWindow::onSplitField()
{
    QModelIndex current;
    TField* field=getCurrentField(&current);
    if (current.isValid() &&  field){
        CSplitFieldDialog dlg;
        int count=0;
        if ((count=dlg.Execute(field))>=2 && Project.splitField(current.row(),count)){
            projectModified = true;
            fieldWidget->setCurrentIndex(current);
            fieldWidget->scrollTo(current);
            updatePreviewImage();
        }
    }
}

void MainWindow::onHexWidgetClick(quint32 addr)
{
//    qDebug() << "onHexWidgetClick(";
    TField* field(nullptr);
    int index(0);
    //    fieldWidget->scrollTo(fieldWidget->model()->index(index,0));
    if (Project.getFieldByAddress(addr,&field,index))
        fieldWidget->setCurrentIndex(fieldWidget->model()->index(index,0));
    else
        fieldWidget->scrollTo(fieldWidget->model()->index(index,0));
}

void MainWindow::onHexWidgetDblClick(quint32 addr)
{
//    qDebug() << "onHexWidgetDblClick(";
    TField* field(nullptr);
    int index(0);
    if (Project.getFieldByAddress(addr,&field,index)){
        fieldWidget->setCurrentIndex(fieldWidget->model()->index(index,0));
        onModField();
    }
    else{
        lastFieldStart = field->Start;
        lastFieldSize  = field->Size;
        const TImageField* img_field;
        if (field->Type()==TField::EImage &&
                (img_field=dynamic_cast<const TImageField*>(field)))
            lastFieldWidth = img_field->Width;
        onAddField();
    }
}

void MainWindow::updatePreviewImage()
{
    const TImageField* field = getCurrentImageField();
    if (field) previewWidget->setImage(field->drawByteArray(Project.getData()));
    else       previewWidget->setImage(QImage());
}

void MainWindow::onCurrentChanged(const QModelIndex& current,
                                  const QModelIndex& previous)
{
//    qDebug() << "onCurrentChanged(";
    //    qDebug() << "    " << current << ",";
    //    qDebug() << "    " << previous;
    //    qDebug() << ");";

    Q_UNUSED(current)
    Q_UNUSED(previous)
    const TField* field = getCurrentField();
    if (field){
        lastFieldStart = field->Start;
        lastFieldSize  = field->Size;
        const TImageField* img_field;
        if (field->Type()==TField::EImage &&
                (img_field=dynamic_cast<const TImageField*>(field)))
            lastFieldWidth = img_field->Width;
    }
    else{
        lastFieldStart = -1;
        lastFieldSize  = -1;
        lastFieldWidth = -1;
    }
    updatePreviewImage();
    updateControls();
}

void MainWindow::onSelectionChanged(const QItemSelection& selected,
                                    const QItemSelection& deselected)
{
//    qDebug() << "onSelectionChanged(";
    //    qDebug() << "    " << selected << ",";
    //    qDebug() << "    " << deselected;
    //    qDebug() << ");";

    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    updateControls();
}

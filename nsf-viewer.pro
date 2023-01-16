QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/field.cpp \
    core/image_field.cpp \
    dialogs/add_field_dlg.cpp \
    dialogs/edit_field_dlg.cpp \
    dialogs/split_dialog.cpp \
    main.cpp \
    mainwindow.cpp \
    utils.cpp \
    widgets/field_view_widget.cpp \
    core/project.cpp \
    widgets/hex_view_widget.cpp \
    widgets/images_view_widget.cpp \
    widgets/preview_widget.cpp \
    widgets/project_model.cpp

HEADERS += \
    core/field.h \
    core/image_field.h \
    dialogs/add_field_dlg.h \
    dialogs/edit_field_dlg.h \
    dialogs/split_dialog.h \
    mainwindow.h \
    utils.h \
    widgets/field_view_widget.h \
    core/project.h \
    widgets/hex_view_widget.h \
    widgets/images_view_widget.h \
    widgets/preview_widget.h \
    widgets/project_model.h

FORMS += \
    dialogs/add_field_dlg.ui \
    dialogs/edit_field_dlg.ui \
    dialogs/split_dialog.ui \
    mainwindow.ui

TRANSLATIONS += \
    nsf-viewer_ru_RU.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qrc.qrc

DISTFILES += \
    README.md


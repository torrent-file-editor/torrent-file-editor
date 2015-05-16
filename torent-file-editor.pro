#-------------------------------------------------
#
# Project created by QtCreator 2014-11-06T13:16:17
#
#-------------------------------------------------

QT       += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = torent-file-editor
TEMPLATE = app

CONFIG += c++11
DEFINES += HAVE_QT5 DEBUG
SOURCES += \
    aboutdlg.cpp \
    abstracttreeitem.cpp \
    abstracttreemodel.cpp \
    application.cpp \
    bencode.cpp \
    bencodedelegate.cpp \
    bencodemodel.cpp \
    datewidget.cpp \
    folderedit.cpp \
    lineeditwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    proxystyle.cpp \
    urledit.cpp

HEADERS  += \
    aboutdlg.h \
    abstracttreeitem.h \
    abstracttreemodel.h \
    application.h \
    bencode.h \
    bencodedelegate.h \
    bencodemodel.h \
    datewidget.h \
    folderedit.h \
    lineeditwidget.h \
    mainwindow.h \
    proxystyle.h \
    urledit.h

FORMS    += \ 
    aboutdlg.ui \
    mainwindow.ui

RESOURCES += \
    resources.qrc

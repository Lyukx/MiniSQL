#-------------------------------------------------
#
# Project created by QtCreator 2014-11-16T02:03:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MiniSQL
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    API.cpp \
    BPlusTree.cpp \
    BufferManager.cpp \
    CatalogManager.cpp \
    IndexManager.cpp \
    Interpret.cpp \
    RecordManager.cpp

HEADERS  += mainwindow.h \
    BPlusTree.h \
    BufferManager.h \
    IndexManager.h \
    Interpret.h \
    RecordManager.h \
    Structures.h

FORMS    += mainwindow.ui

#-------------------------------------------------
#
# Project created by QtCreator 2016-09-28T23:23:36
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ljd
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    page.cpp \
    image.cpp \
    contentmanager.cpp \
    entry.cpp

HEADERS  += mainwindow.h \
    page.h \
    image.h \
    contentmanager.h \
    entry.h

FORMS    += mainwindow.ui

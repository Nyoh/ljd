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
    netpage.cpp \
    page.cpp \
    image.cpp \
    contentmanager.cpp

HEADERS  += mainwindow.h \
    netpage.h \
    page.h \
    image.h \
    contentmanager.h

FORMS    += mainwindow.ui

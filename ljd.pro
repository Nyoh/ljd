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
    entry.cpp \
    downloader.cpp \
    printer.cpp

HEADERS  += mainwindow.h \
    page.h \
    image.h \
    entry.h \
    downloader.h \
    printer.h

FORMS    += mainwindow.ui

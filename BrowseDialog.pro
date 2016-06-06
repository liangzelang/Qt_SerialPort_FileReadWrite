#-------------------------------------------------
#
# Project created by QtCreator 2016-06-03T09:37:39
#
#-------------------------------------------------
QT += core
QT += core gui
QT += serialport
#QT += testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BrowseDialog
TEMPLATE = app


SOURCES += main.cpp\
        browsedialog.cpp

HEADERS  += browsedialog.h

FORMS    += browsedialog.ui

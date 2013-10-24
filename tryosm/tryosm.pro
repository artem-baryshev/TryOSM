#-------------------------------------------------
#
# Project created by QtCreator 2013-07-05T20:13:41
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tryosm
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tosmtosqlite.cpp \
    tosmwidget.cpp \
    tstat.cpp \
    tmouseman.cpp \
    troutedijkstra.cpp \
    routingprofiles.cpp

HEADERS  += mainwindow.h \
    tosmtosqlite.h \
    tosmwidget.h \
    tstat.h \
    tmouseman.h \
    troutedijkstra.h \
    routingprofiles.h \
    pointers.h

FORMS    += mainwindow.ui

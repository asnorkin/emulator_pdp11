#-------------------------------------------------
#
# Project created by QtCreator 2016-10-17T14:04:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PDP11
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    image.cpp \
    pdp_memory.cpp \
    pdp_processor.cpp \
    pdp_tester.cpp \
    pdp.cpp

HEADERS  += mainwidget.h \
    image.h \
    data_types.h \
    pdp_memory.h \
    pdp_processor.h \
    pdp_tester.h \
    pdp.h

FORMS    += mainwidget.ui

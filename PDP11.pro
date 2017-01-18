#-------------------------------------------------
#
# Project created by QtCreator 2016-10-17T14:04:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PDP11
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11


SOURCES += main.cpp\
        mainwidget.cpp \
    image.cpp \
    pdp_memory.cpp \
    pdp_processor.cpp \
    pdp_tester.cpp \
    pdp.cpp \
    icache.cpp \
    pipeline.cpp \
    wb_buffer.cpp

HEADERS  += mainwidget.h \
    image.h \
    data_types.h \
    pdp_memory.h \
    pdp_processor.h \
    pdp_tester.h \
    utils.h \
    pdp.h \
    icache.h \
    pipeline.h \
    clocks.h \
    wb_buffer.h

FORMS    += mainwidget.ui

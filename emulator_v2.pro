TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    pdp.cpp \
    pdp_memory.cpp \
    pdp_processor.cpp \
    pdp_tester.cpp

HEADERS += \
    pdp.h \
    pdp_memory.h \
    data_types.h \
    pdp_processor.h \
    pdp_tester.h


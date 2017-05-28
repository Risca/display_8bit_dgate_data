#-------------------------------------------------
#
# Project created by QtCreator 2012-05-09T21:55:00
#
#-------------------------------------------------

QT       += core gui

TARGET = display_8bit_data
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    hexspinbox.cpp \
    dg_image.cpp

HEADERS  += mainwindow.h \
    hexspinbox.h \
    dg_image.h \
    vga_color_palette.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -fPIC

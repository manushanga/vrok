#-------------------------------------------------
#
# Project created by QtCreator 2012-10-01T20:59:44
#
#-------------------------------------------------

QT       += core gui
LIBS     += -lvlc -ljack -lsupc++

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vrok
TEMPLATE = app

DEFINES += USE_OOURA

SOURCES += main.cpp\
        vrokmain.cpp \
    vplayer.cpp \
    out_jack.cpp \
    out.cpp \
    shibatch/Equ.cpp \
    shibatch/Fftsg_fl.c

HEADERS  += vrokmain.h \
    vplayer.h \
    vputils.h \
    out_jack.h \
    out.h \
    shibatch/paramlist.hpp \
    shibatch/Equ.h

FORMS    += vrokmain.ui

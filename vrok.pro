#-------------------------------------------------
#
# Project created by QtCreator 2012-10-01T20:59:44
#
#-------------------------------------------------

CONFIG   += qt thread
QT       += core gui
LIBS     += -lsupc++ -lm -lasound -lFLAC -lmpg123 -lvorbisfile

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -O4 -std=c++11
TARGET = vrok
TEMPLATE = app

DEFINES += USE_OOURA

SOURCES += main.cpp\
        vrokmain.cpp \
    vplayer.cpp \
    out.cpp \
    effects/shibatch/Equ.cpp \
    effects/shibatch/Fftsg_fl.c \
    effect.cpp \
    effects/eq.cpp \
    effects/vis.cpp \
    outs/alsa.cpp \
    players/flac.cpp \
    players/mpeg.cpp \
    players/ogg.cpp

HEADERS  += vrokmain.h \
    vplayer.h \
    vputils.h \
    out.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/Equ.h \
    effect.h \
    effects/eq.h \
    effects/vis.h \
    outs/alsa.h \
    players/flac.h \
    players/mpeg.h \
    players/ogg.h

FORMS    += vrokmain.ui

OTHER_FILES += \
    README.md \
    LICENSE

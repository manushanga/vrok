#-------------------------------------------------
#
# Project created by QtCreator 2012-10-01T20:59:44
#
#-------------------------------------------------

QT       += core gui
LIBS     += -lsupc++ -lm -lpthread -lasound -lFLAC -lmpg123

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11
TARGET = vrok
TEMPLATE = app

DEFINES += USE_OOURA

SOURCES += main.cpp\
        vrokmain.cpp \
    vplayer.cpp \
    out.cpp \
    shibatch/Equ.cpp \
    shibatch/Fftsg_fl.c \
    outs/out_alsa.cpp \
    players/player_flac.cpp \
    players/player_mpeg.cpp \
    effects/effect_eq.cpp \
    effect.cpp

HEADERS  += vrokmain.h \
    vplayer.h \
    vputils.h \
    out.h \
    shibatch/paramlist.hpp \
    shibatch/Equ.h \
    outs/out_alsa.h \
    players/player_flac.h \
    players/player_mpeg.h \
    effect.h \
    effects/effect_eq.h

FORMS    += vrokmain.ui

OTHER_FILES += \
    README.md

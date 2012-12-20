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
    outs/out_alsa.cpp \
    players/player_flac.cpp \
    players/player_mpeg.cpp \
    effects/effect_eq.cpp \
    effect.cpp \
    players/player_ogg.cpp

HEADERS  += vrokmain.h \
    vplayer.h \
    vputils.h \
    out.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/Equ.h \
    outs/out_alsa.h \
    players/player_flac.h \
    players/player_mpeg.h \
    effect.h \
    effects/effect_eq.h \
    players/player_ogg.h

FORMS    += vrokmain.ui

OTHER_FILES += \
    README.md \
    LICENSE

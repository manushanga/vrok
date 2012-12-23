#-------------------------------------------------
#
# Project created by QtCreator 2012-10-01T20:59:44
#
#-------------------------------------------------

CONFIG   += qt thread
QT       += core gui
LIBS     += -lsupc++ -lm -lFLAC -lmpg123 -lvorbisfile

linux-g++* {
LIBS     += -lasound
}

win32 {
LIBS     += -lboost_system -lboost_thread
}
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
    players/flac.cpp \
    players/mpeg.cpp \
    players/ogg.cpp \
    config.cpp

linux-g++* {
SOURCES += outs/alsa.cpp
}

win32 {
SOURCES += outs/waveout.cpp
}

HEADERS  += vrokmain.h \
    vplayer.h \
    vputils.h \
    out.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/Equ.h \
    effect.h \
    effects/eq.h \
    effects/vis.h \
    players/flac.h \
    players/mpeg.h \
    players/ogg.h \
    thread_compat.h \
    config.h

linux-g++* {
HEADERS  += outs/alsa.h
}

win32 {
HEADERS  += outs/waveout.h
}

FORMS    += vrokmain.ui

OTHER_FILES += \
    README.md \
    LICENSE

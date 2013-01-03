#-------------------------------------------------
#
# Project created by QtCreator 2012-10-01T20:59:44
#
#-------------------------------------------------

CONFIG   += qt thread
QT       += core gui
LIBS     += -lsupc++ -lm -lFLAC -lmpg123 -lvorbisfile -lvorbis -logg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

TARGET = vrok

DEFINES += USE_OOURA

SOURCES += main.cpp\
        vrokmain.cpp \
    vplayer.cpp \
    effects/shibatch/Equ.cpp \
    effects/shibatch/Fftsg_fl.c \
    effects/eq.cpp \
    effects/vis.cpp \
    players/flac.cpp \
    players/mpeg.cpp \
    players/ogg.cpp \
    config.cpp

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
    decoder.h \
    config.h \
    config_out.h

FORMS    += vrokmain.ui

OTHER_FILES += \
    README.md \
    LICENSE

QMAKE_CXXFLAGS += -O3

linux-g++* {
LIBS    += -lasound
QMAKE_CXXFLAGS += -std=c++11
HEADERS  += outs/alsa.h
SOURCES += outs/alsa.cpp
}

win32 {
LIBS    +=  -lws2_32  -lole32 -ldxguid -ldsound
HEADERS  +=  include/dsound.h \
    include/d3dtypes.h \
    outs/dsound.h
SOURCES += outs/dsound.cpp
}



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

INCLUDEPATH += include

SOURCES += main.cpp \
        frontend/vrokmain.cpp \
    frontend/eqwidget.cpp \
    vplayer.cpp \
    effects/shibatch/Equ.cpp \
    effects/shibatch/Fftsg_fl.c \
    effects/eq.cpp \
    players/flac.cpp \
    players/mpeg.cpp \
    players/ogg.cpp \
    config.cpp
HEADERS  += \
    include/vplayer.h \
    include/decoder.h \
    include/effect.h \
    include/config_out.h \
    include/config.h \
    frontend/vrokmain.h \
    frontend/eqwidget.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/Equ.h \
    effects/eq.h \
    players/flac.h \
    players/mpeg.h \
    players/ogg.h \
    include/threads.h
FORMS    += frontend/vrokmain.ui \
            frontend/eqwidget.ui

OTHER_FILES += \
    README.md \
    LICENSE

QMAKE_CXXFLAGS += -Wall -O3

linux-g++* {

linux-g++-64 {
LIBS    += /opt/libsamplerate/lib/libsamplerate.a
}
linux-g++-32 {
LIBS    += /opt/libsamplerate32/lib/libsamplerate.a
}

LIBS += -lasound -pthread
#QMAKE_CXXFLAGS += -std=c++11
HEADERS  += outs/alsa.h
SOURCES += outs/alsa.cpp
}

win32 {
LIBS    +=  -lws2_32  -lole32 -ldxguid -ldsound
HEADERS  +=  outs/dsound.h
SOURCES += outs/dsound.cpp
}



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
    config.cpp \
    utils.cpp
HEADERS  += \
    include/vplayer.h \
    include/decoder.h \
    include/effect.h \
    include/config.h \
    frontend/vrokmain.h \
    frontend/eqwidget.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/Equ.h \
    effects/eq.h \
    players/flac.h \
    players/mpeg.h \
    players/ogg.h \
    include/threads.h \
    include/vrok.h \
    vputils.h

FORMS    += frontend/vrokmain.ui \
            frontend/eqwidget.ui

OTHER_FILES += \
    README.md \
    LICENSE

QMAKE_CXXFLAGS += -Wall -O4 -msse4.2

linux-g++-64 {
LIBS += -lasound -lpthread -lsamplerate
HEADERS  += outs/alsa.h
SOURCES += outs/alsa.cpp
}

linux-g++-32 {
QMAKE_CXXFLAGS += -march=i686
}

win32 {
QMAKE_CXXFLAGS += -march=i686
LIBS    +=  -lws2_32  -lole32 -ldxguid -ldsound
HEADERS  +=  outs/dsound.h
SOURCES += outs/dsound.cpp
}

RESOURCES += \
    vrok.qrc



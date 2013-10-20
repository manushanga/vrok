#-------------------------------------------------
#
# Project created by QtCreator 2012-10-01T20:59:44
#
#-------------------------------------------------

QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

TARGET = vrok

DEFINES += USE_OOURA

INCLUDEPATH += include

console {
SOURCES += \
    cli/main.cpp \
    vplayer.cpp \
    effects/shibatch/Equ.cpp \
    effects/shibatch/Fftsg_fl.c \
    effects/eq.cpp \
    players/flac.cpp \
    players/mpeg.cpp \
    players/ogg.cpp \
    util/utils.cpp \
    util/cpplib.cpp
HEADERS  += \
    include/vplayer.h \
    include/decoder.h \
    include/effect.h \
    include/threads.h \
    include/vrok.h \
    include/out.h \
    include/vputils.h \
    include/cpplib.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/Equ.h \
    effects/eq.h \
    players/flac.h \
    players/mpeg.h \
    players/ogg.h

} else {
SOURCES += \
    frontend/main.cpp \
    frontend/vrokmain.cpp \
    frontend/eqwidget.cpp \
    vplayer.cpp \
    effects/shibatch/Equ.cpp \
    effects/shibatch/Fftsg_fl.c \
    effects/eq.cpp \
    players/flac.cpp \
    players/mpeg.cpp \
    players/ogg.cpp \
    util/utils.cpp \
    util/cpplib.cpp
HEADERS  += \
    include/vplayer.h \
    include/decoder.h \
    include/effect.h \
    include/vputils.h \
    include/threads.h \
    include/vrok.h \
    include/cpplib.h \
    frontend/vrokmain.h \
    frontend/eqwidget.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/Equ.h \
    effects/eq.h \
    players/flac.h \
    players/mpeg.h \
    players/ogg.h


FORMS    += frontend/vrokmain.ui \
            frontend/eqwidget.ui
}

OTHER_FILES += README.md \
    LICENSE

#QMAKE_CXXFLAGS += -Wall -O4 -msse4.2 -ffast-math

linux-clang {
    QMAKE_CXXFLAGS_DEBUG += -fno-omit-frame-pointer \
                            -O2 #-pg
    #QMAKE_LFLAGS_DEBUG += # -fsanitize=thread  #-pg
}

!linux-clang {
    QMAKE_CXXFLAGS_DEBUG += -O0 #-pg
    QMAKE_LFLAGS_DEBUG += -O0 #-pg
}

linux* {
LIBS     += -lm -lFLAC -lmpg123 -lvorbisfile -lvorbis -logg
LIBS += -lpthread

PulseAudio {
    LIBS += -lpulse-simple
    DEFINES += VPOUT_PULSE
    HEADERS += outs/pulse.h
    SOURCES += outs/pulse.cpp
}

Dummy {
    LIBS += -lsamplerate
    DEFINES += VPOUT_DUMMY
    HEADERS += outs/dummy.h
    SOURCES += outs/dummy.cpp
}

Alsa {
    LIBS += -lsamplerate -lasound
    DEFINES += VPOUT_ALSA
    HEADERS += outs/alsa.h
    SOURCES += outs/alsa.cpp
}
Ao {
    LIBS += -lao
    DEFINES += VPOUT_AO
    HEADERS += outs/ao.h
    SOURCES += outs/ao.cpp
}

}

win32* {

Dsound {
    LIBS += -lole32 -ldxguid -ldsound
    DEFINES += VPOUT_DSOUND
    HEADERS  +=  outs/dsound.h
    SOURCES += outs/dsound.cpp
}

Ao {
    LIBS += -lao
    DEFINES += VPOUT_AO
    HEADERS += outs/ao.h
    SOURCES += outs/ao.cpp
}

QMAKE_CXXFLAGS += /TP
debug {
QMAKE_CXXFLAGS += /INCREMENTAL
}
release {
QMAKE_CXXFLAGS += /Ox
}
INCLUDEPATH += ./libs/include \
               ./libs/include/libmpg123 \
               .
LIBS    += -L"C:\Users\madura\Desktop\vrok-master\libs\static\release"
LIBS     += -lutf8_static -lwin_utf8_io -llibFLAC -llibmpg123 -llibvorbisfile -llibvorbis -llibogg
LIBS    +=  -lws2_32 -lkernel32 -luser32 -lshlwapi -ladvapi32 -lshell32 -loleaut32 -luuid

}

TRANSLATIONS += vrok_si.ts

RESOURCES += vrok.qrc

SOURCES += \
    vrok.cpp \
    settings.cpp \
    frontend/playlistfactory.cpp \
    frontend/displayticker.cpp \
    frontend/folderseeker.cpp

HEADERS += \
    frontend/playlistfactory.h \
    frontend/displayticker.h \
    frontend/folderseeker.h




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
    effects/shibach/eq.cpp \
    players/flac.cpp \
    players/mpeg.cpp \
    players/ogg.cpp \
    util/utils.cpp \
    util/cpplib.cpp \
    thirdparty/ooura_fft.c

HEADERS  += \
    include/vplayer.h \
    include/decoder.h \
    include/effect.h \
    include/threads.h \
    include/vrok.h \
    include/out.h \
    include/vputils.h \
    include/cpplib.h \
    include/ooura_fft.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/eq.h \
    players/flac.h \
    players/mpeg.h \
    players/ogg.h

} else {
SOURCES += \
    frontend/main.cpp \
    frontend/vrokmain.cpp \
    frontend/eqwidget.cpp \
    vplayer.cpp \
    effects/shibatch/eq.cpp \
    players/flac.cpp \
    players/mpeg.cpp \
    players/ogg.cpp \
    util/utils.cpp \
    util/cpplib.cpp \
    thirdparty/ooura_fft.c

HEADERS  += \
    include/vplayer.h \
    include/decoder.h \
    include/effect.h \
    include/vputils.h \
    include/threads.h \
    include/vrok.h \
    include/cpplib.h \
    include/ooura_fft.h \
    frontend/vrokmain.h \
    frontend/eqwidget.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/eq.h \
    players/flac.h \
    players/mpeg.h \
    players/ogg.h


FORMS    += frontend/vrokmain.ui \
            frontend/eqwidget.ui
}

OTHER_FILES += README.md \
    LICENSE


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

linux-clang {
    QMAKE_CXXFLAGS_RELEASE += -Wall -O4 -msse4.2 -ffast-math
    QMAKE_CXXFLAGS_DEBUG += -fno-omit-frame-pointer \
                            -O2 #-pg
    #QMAKE_LFLAGS_DEBUG += # -fsanitize=thread  #-pg
}

linux-gcc {
    QMAKE_CXXFLAGS_RELEASE += -Wall -O4 -msse4.2 -ffast-math
}

!linux-clang {
    QMAKE_CXXFLAGS_DEBUG += -O0 #-pg
    QMAKE_LFLAGS_DEBUG += -O0 #-pg
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

INCLUDEPATH += ./libs/include \
               ./libs/include/libmpg123 \
               .

LIBS     += -lutf8_static -lwin_utf8_io -llibFLAC -llibmpg123 -llibvorbisfile -llibvorbis -llibogg
LIBS    +=  -lws2_32 -lkernel32 -luser32 -lshlwapi -ladvapi32 -lshell32 -loleaut32 -luuid

QMAKE_CXXFLAGS += /TP

debug {
LIBS    += -L"C:/src/vrok/libs/static/debug"
LIBS    += msvcrtd.lib
QMAKE_LFLAGS = /INCREMENTAL
}
release {
LIBS    += -L"C:/src/vrok/libs/static/release"
QMAKE_CXXFLAGS += /MD /GS /arch:SSE2 /Ot /O2 /Oi /Oy- /fp:fast
QMAKE_LFLAGS = /INCREMENTAL /NODEFAULTLIB:"MSVCRTD" /NODEFAULTLIB:"LIBCMTD" /ALLOWISOLATION /TLBID:1 /DYNAMICBASE /NXCOMPAT /MACHINE:X86 /ERRORREPORT:QUEUE
}
}

TRANSLATIONS += vrok_si.ts

RESOURCES += vrok.qrc

SOURCES += \
    vrok.cpp \
    settings.cpp \
    frontend/playlistfactory.cpp \
    frontend/folderseeker.cpp \
    frontend/vswidget.cpp \
    frontend/dockmanager.cpp \
    effects/shibatch/equ.cpp \
    effects/visualization/vpeffectvis.cpp \
    frontend/reverbwidget.cpp

HEADERS += \
    frontend/playlistfactory.h \
    frontend/displayticker.h \
    frontend/folderseeker.h \
    frontend/vswidget.h \
    frontend/dockmanager.h \
    frontend/manageddockwidget.h \
    effects/shibatch/equ.h \
    effects/visualization/vpeffectvis.h \
    frontend/reverbwidget.h

FORMS += \
    frontend/vswidget.ui \
    frontend/reverbwidget.ui

RC_FILE += \
    vrok.rc

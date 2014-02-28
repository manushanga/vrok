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
    thirdparty/ooura_fft.c \
    cli/main.cpp \
    vplayer.cpp \
    vrok.cpp \
    settings.cpp \
    effects/shibatch/eq.cpp \
    effects/shibatch/equ.cpp \
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
    include/ooura_fft.h \
    effects/shibatch/paramlist.hpp \
    effects/shibatch/eq.h \
    effects/shibatch/equ.h \
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
    players/ffmpeg.cpp \
    util/utils.cpp \
    util/cpplib.cpp \
    thirdparty/ooura_fft.c \
    vrok.cpp \
    settings.cpp \
    frontend/playlistfactory.cpp \
    frontend/vswidget.cpp \
    frontend/dockmanager.cpp \
    effects/shibatch/equ.cpp \
    effects/visualization/vpeffectvis.cpp \
    effects/reverb/reverb.cpp \
    frontend/reverbwidget.cpp \
    effects/spatial/spatial.cpp \
    frontend/spatialwidget.cpp

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
    players/ogg.h \
    frontend/playlistfactory.h \
    frontend/displayticker.h \
    frontend/vswidget.h \
    frontend/dockmanager.h \
    frontend/manageddockwidget.h \
    effects/shibatch/equ.h \
    effects/visualization/vpeffectvis.h \
    effects/reverb/reverb.h \
    frontend/reverbwidget.h \
    players/ffmpeg.h \
    effects/spatial/spatial.h \
    frontend/spatialwidget.h \
    frontend/orderer.h


FORMS    += frontend/vrokmain.ui \
            frontend/eqwidget.ui \
            frontend/vswidget.ui \
            frontend/reverbwidget.ui \
            frontend/spatialwidget.ui

TRANSLATIONS += vrok_si.ts

RESOURCES += vrok.qrc

}

OTHER_FILES += README.md \
    LICENSE


linux* {
LIBS     += -lm -lFLAC -lmpg123 -lvorbisfile -lvorbis -logg
LIBS    +=  -lavformat -lavcodec -lavutil
LIBS += -lpthread -ldl

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
    QMAKE_CXXFLAGS_RELEASE += -Wall -O3 -msse4.2 -ffast-math
    QMAKE_CXXFLAGS_DEBUG +=  -g -O3 #-pg
    #QMAKE_LFLAGS_DEBUG += # -fsanitize=thread  #-pg
}

linux-gcc {
    QMAKE_CXXFLAGS_RELEASE += -Wall -O3 -msse4.2 -ffast-math
    QMAKE_CXXFLAGS_DEBUG +=  -g -O3 #-pg
}

!linux-clang {
    QMAKE_CXXFLAGS_DEBUG += -O0 #-pg
    QMAKE_LFLAGS_DEBUG += -O0 #-pg
}

}

win32* {

DEFINES += _USE_MATH_DEFINES

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
LIBS    +=  -lavformat -lavcodec -lavutil
LIBS    +=  -lws2_32 -lkernel32 -luser32 -lshlwapi -ladvapi32 -lshell32 -loleaut32 -luuid

debug {
LIBS    += -L"C:/src/vrok/vrok/libs/static/debug"
LIBS    += -L"C:/src/vrok/vrok/libs/shared/release"
}
release {
LIBS    += -L"C:/src/vrok/vrok/libs/static/release"
LIBS    += -L"C:/src/vrok/vrok/libs/shared/release"

win32-msvc* {
QMAKE_CXXFLAGS += /TP
QMAKE_CXXFLAGS +=  /arch:SSE2 /Ot /O2 /Oi /Oy- /fp:fast
QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRTD
}

}

RC_FILE += \
    vrok.rc

}

HEADERS += \
    frontend/playlistwidget.h \
    frontend/controlswidget.h

SOURCES += \
    frontend/playlistwidget.cpp \
    frontend/controlswidget.cpp

FORMS += \
    frontend/playlistwidget.ui \
    frontend/controlswidget.ui




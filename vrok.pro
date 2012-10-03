#-------------------------------------------------
#
# Project created by QtCreator 2012-10-01T20:59:44
#
#-------------------------------------------------

QT       += core gui
LIBS     += -lvlc -ljack

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vrok
TEMPLATE = app


SOURCES += main.cpp\
        vrokmain.cpp \
    vplayer.cpp \
    out_jack.cpp \
    out.cpp

HEADERS  += vrokmain.h \
    vplayer.h \
    vputils.h \
    out_jack.h \
    out.h

FORMS    += vrokmain.ui

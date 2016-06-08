QT += core network
QT -= gui

CONFIG(debug, release|debug):DEFINES += _DEBUG
#DEFINES += _BIG_ENDIAN
DEFINES += _LITTLE_ENDIAN

TARGET = test-rec-qt5
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    parser.cpp \
    recorder.cpp \
    cleanexit.cpp \
    ncursesuse.cpp \
    jitterbuffer.cpp

HEADERS += \
    parser.h \
    recorder.h \
    audio.h \
    cleanexit.h \
    ncursesuse.h \
    jitterbuffer.h

LIBS += -lncurses

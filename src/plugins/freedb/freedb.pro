include(../plugins.pri)
include(../../portable.pri)

PVERSION = 1.0.0
DEFINES += PLUGIN_VERSION=\\\"$$PVERSION\\\"

HEADERS = freedb.h
SOURCES = freedb.cpp
OTHER_FILES += freedb.json

HEADERS += ../../qoobar_app/discidhelper.h \
           ../../qoobar_app/idownloadplugin.h \
           ../../qoobar_app/coverimage.h \
           ../../qoobar_app/qoobarglobals.h \
           ../../qoobar_app/applicationpaths.h

SOURCES += ../../qoobar_app/discidhelper.cpp \
           ../../qoobar_app/coverimage.cpp \
           ../../qoobar_app/qoobarglobals.cpp \
           ../../qoobar_app/applicationpaths.cpp

TARGET = freedb

include(../../libdiscid.pri)

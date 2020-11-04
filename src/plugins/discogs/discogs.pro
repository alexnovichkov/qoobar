include(../plugins.pri)
include(../../portable.pri)

PVERSION = 1.0.0
DEFINES += PLUGIN_VERSION=\\\"$$PVERSION\\\"

HEADERS = discogs.h
SOURCES = discogs.cpp
OTHER_FILES += discogs.json

HEADERS += ../../qoobar_app/idownloadplugin.h \
           ../../qoobar_app/coverimage.h \
           ../../qoobar_app/applicationpaths.h

#HEADERS += ../../qoobar_app/qoobarglobals.h
#SOURCES += ../../qoobar_app/qoobarglobals.cpp

HEADERS += ../../qoobar_app/logging.h
SOURCES += ../../qoobar_app/logging.cpp

SOURCES += ../../qoobar_app/coverimage.cpp \
           ../../qoobar_app/applicationpaths.cpp

#HEADERS += ../../qoobar_app/o2/*.h
#SOURCES += ../../qoobar_app/o2/*.cpp

TARGET = discogs

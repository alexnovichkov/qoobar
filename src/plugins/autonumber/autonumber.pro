include(../plugins.pri)
include(../../portable.pri)

PVERSION = 1.0.0
DEFINES += PLUGIN_VERSION=\\\"$$PVERSION\\\"

HEADERS = autonumberplugin.h
SOURCES = autonumberplugin.cpp
OTHER_FILES += autonumber.json

HEADERS += ../../qoobar_app/tagger.h \
           ../../qoobar_app/iqoobarplugin.h \
           ../../qoobar_app/coverimage.h
SOURCES += ../../qoobar_app/tagger.cpp \
           ../../qoobar_app/coverimage.cpp

os2:TARGET = autonumb
!os2:TARGET = autonumber


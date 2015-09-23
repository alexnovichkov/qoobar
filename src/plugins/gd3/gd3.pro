include(../plugins.pri)
include(../../portable.pri)

PVERSION = 1.0.0
DEFINES += PLUGIN_VERSION=\\\"$$PVERSION\\\"

HEADERS = gd3.h
SOURCES = gd3.cpp
OTHER_FILES += gd3.json

HEADERS += ../../qoobar_app/idownloadplugin.h \
           ../../qoobar_app/coverimage.h \
           ../../qoobar_app/qoobarglobals.h \
           ../../qoobar_app/applicationpaths.h

SOURCES += ../../qoobar_app/coverimage.cpp \
           ../../qoobar_app/qoobarglobals.cpp \
           ../../qoobar_app/applicationpaths.cpp

HEADERS += ../../qoobar_app/logging.h
SOURCES += ../../qoobar_app/logging.cpp

TARGET = gd3

#include(../../libdiscid.pri)
mac {
    LIBS += -F../../../mac_os/ -framework discid
    INCLUDEPATH += ../../../mac_os/discid.framework/Versions/A/Headers
    DEPENDPATH += ../../../mac_os/discid.framework/Versions/A/Headers
}

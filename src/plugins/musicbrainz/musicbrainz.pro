include(../plugins.pri)
include(../../portable.pri)

PVERSION = 1.0.2
DEFINES += PLUGIN_VERSION=\\\"$$PVERSION\\\"

HEADERS = musicbrainz.h
SOURCES = musicbrainz.cpp
OTHER_FILES += musicbrainz.json

HEADERS += ../../qoobar_app/idownloadplugin.h \
           ../../qoobar_app/coverimage.h \
           ../../qoobar_app/qoobarglobals.h \
           ../../qoobar_app/discidhelper.h \
           ../../qoobar_app/applicationpaths.h

SOURCES += ../../qoobar_app/coverimage.cpp \
           ../../qoobar_app/qoobarglobals.cpp \
           ../../qoobar_app/discidhelper.cpp \
           ../../qoobar_app/applicationpaths.cpp

os2:TARGET = muscbrnz
!os2:TARGET = musicbrainz

#include(../../libdiscid.pri)
mac {
    LIBS += -F../../../mac_os/ -framework discid
    INCLUDEPATH += ../../../mac_os/discid.framework/Versions/A/Headers
    DEPENDPATH += ../../../mac_os/discid.framework/Versions/A/Headers
}

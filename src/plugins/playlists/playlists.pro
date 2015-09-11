include(../plugins.pri)
include(../../portable.pri)

PVERSION = 1.0.0
DEFINES += PLUGIN_VERSION=\\\"$$PVERSION\\\"

HEADERS = playlistsplugin.h \
          AbstractPlaylist.h
SOURCES = playlistsplugin.cpp

OTHER_FILES += playlists.json

HEADERS += ../../qoobar_app/tagger.h \
           ../../qoobar_app/iqoobarplugin.h \
           ../../qoobar_app/coverimage.h

SOURCES += ../../qoobar_app/tagger.cpp \
           ../../qoobar_app/coverimage.cpp

os2:TARGET = playlist
!os2:TARGET = playlists

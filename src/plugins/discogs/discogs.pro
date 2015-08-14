include(../plugins.pri)

PVERSION = 1.0.0
DEFINES += PLUGIN_VERSION=\\\"$$PVERSION\\\"

HEADERS = discogs.h
SOURCES = discogs.cpp
OTHER_FILES += discogs.json

HEADERS += ../../qoobar_app/idownloadplugin.h \
           ../../qoobar_app/coverimage.h \
           ../../qoobar_app/qoobarglobals.h \
           ../../qoobar_app/applicationpaths.h

SOURCES += ../../qoobar_app/coverimage.cpp \
           ../../qoobar_app/qoobarglobals.cpp \
           ../../qoobar_app/applicationpaths.cpp

lessThan(QT_MAJOR_VERSION, 5) {
    HEADERS += ../../qoobar_app/ereilin/json.h
    SOURCES += ../../qoobar_app/ereilin/json.cpp
}

TARGET = discogs

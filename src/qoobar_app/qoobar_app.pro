TEMPLATE = app
VERSION = 1.6.6
INSTALL_PREFIX = /usr
SHARED_PATH = $${INSTALL_PREFIX}/share
DOC_PATH = $${SHARED_PATH}/doc

DEFINES *= QOOBAR_VERSION=\\\"$$VERSION\\\"
DEFINES *= QOOBAR_SHARED_PATH=\\\"$${SHARED_PATH}/qoobar\\\"
DEFINES *= QOOBAR_DOC_PATH=\\\"$${DOC_PATH}/qoobar-doc\\\"

# place this define in OS-specific section to enable command-line interface
# DEFINES *= QOOBAR_ENABLE_CLI

greaterThan(QT_MAJOR_VERSION, 4) {
  message(Using Qt 5)
  QT *= widgets
  QT *= concurrent
  DEFINES *= HAVE_QT5
  CONFIG *= c++11
}

TARGET = qoobar
CONFIG *= warn_on
CONFIG *= release
CONFIG *= no_keywords

QT *=  network

# main sources and headers
SOURCES = main.cpp \
    mainwindow.cpp \
    tagseditor.cpp \
    tablewidget.cpp \
    filesrenamer.cpp \
    tagsfiller.cpp \
    tagger.cpp \
    settingsdialog.cpp \
    tab.cpp \
    completerdelegate.cpp \
    lineedit.cpp \
    splitdialog.cpp \
    qoobarglobals.cpp \
    undoactions.cpp \
    imagedialog.cpp \
    tabwidget.cpp \
    imagebox.cpp \
    treewidget.cpp \
    legendbutton.cpp \
    displayedtagsdialog.cpp \
    application.cpp \
    discidhelper.cpp \
    texteditdelegate.cpp \
    stringroutines.cpp \
    id3v1stringhandler.cpp \
    newtagdialog.cpp \
    schemeeditor.cpp \
    configpages.cpp \
    releaseinfowidget.cpp \
    taggingscheme.cpp \
    completionsdialog.cpp \
    macsplitter.cpp \
    replaygaindialog.cpp \
    mp3tagsdialog.cpp \
    processlinemaker.cpp \
    statusbar.cpp \
    searchpanel.cpp \
    highlightdelegate.cpp \
    filedelegatehighlighter.cpp \
    checkableheaderview.cpp \
    sections.cpp \
    sectiondelegatehighlighter.cpp \
    delegatehighlighter.cpp \
    coverimage.cpp \
    tagsreaderwriter.cpp \
    corenetworksearch.cpp \
    tagparser.cpp \
    placeholders.cpp \
    fancylineedit.cpp \
    clearlineedit.cpp \
    model.cpp \
    replaygainer.cpp \
    columnsdialog.cpp \
    cuesplitter.cpp \
    filenamerenderer.cpp \
    tagsrenderer.cpp \
    applicationpaths.cpp \
    imageresizedialog.cpp \
    taskbarprogress.cpp \
    clparser.cpp \
    searchbar.cpp \
    platformprocess.cpp \
    treeview.cpp


HEADERS = mainwindow.h \
    tagseditor.h \
    tablewidget.h \
    filesrenamer.h \
    tagsfiller.h \
    tagger.h \
    settingsdialog.h \
    tab.h \
    completerdelegate.h \
    lineedit.h \
    splitdialog.h \
    qoobarglobals.h \
    undoactions.h \
    imagedialog.h \
    tabwidget.h \
    imagebox.h \
    treewidget.h \
    searchresults.h \
    legendbutton.h \
    displayedtagsdialog.h \
    application.h \
    discidhelper.h \
    texteditdelegate.h \
    stringroutines.h \
    id3v1stringhandler.h \
    newtagdialog.h \
    schemeeditor.h \
    configpages.h \
    releaseinfowidget.h \
    taggingscheme.h \
    completionsdialog.h \
    applicationpaths.h \
    macsplitter.h \
    replaygaindialog.h \
    mp3tagsdialog.h \
    processlinemaker.h \
    statusbar.h \
    searchpanel.h \
    highlightdelegate.h \
    delegatehighlighter.h \
    filedelegatehighlighter.h \
    checkableheaderview.h \
    sections.h \
    sectiondelegatehighlighter.h \
    iqoobarplugin.h \
    coverimage.h \
    tagsreaderwriter.h \
    corenetworksearch.h \
    enums.h \
    tagparser.h \
    placeholders.h \
    idownloadplugin.h \
    fancylineedit.h \
    clearlineedit.h \
    model.h \
    replaygainer.h \
    columnsdialog.h \
    cuesplitter.h \
    filenamerenderer.h \
    tagsrenderer.h \
    imageresizedialog.h \
    taskbarprogress.h \
    searchbar.h \
    platformprocess.h \
    treeview.h

lessThan(QT_MAJOR_VERSION, 5) {
    SOURCES *= ereilin/json.cpp
    HEADERS *= ereilin/json.h
}

OTHER_FILES *= splitandconvert.bat \
               splitandconvert.sh

RESOURCES *= qoobar.qrc
TRANSLATIONS = qoobar_ru.ts \
    qoobar_en.ts \
    qoobar_de.ts \
    qoobar_it.ts \
    qoobar_fr.ts \
    qoobar_pl.ts \
    qoobar_nl.ts


system(lupdate qoobar_app.pro&&lrelease qoobar_app.pro)
system(lupdate-qt4 qoobar_app.pro&&lrelease-qt4 qoobar_app.pro)

INCLUDEPATH += taglib
HEADERS += taglib/*.h   \
           taglib/ape/*.h \
           taglib/asf/*.h \
           taglib/flac/*.h \
           taglib/mp4/*.h \
           taglib/mpc/*.h \
           taglib/mpeg/*.h \
           taglib/mpeg/id3v1/*.h \
           taglib/mpeg/id3v2/*.h \
           taglib/mpeg/id3v2/frames/*.h \
           taglib/ogg/*.h \
           taglib/ogg/flac/*.h \
           taglib/ogg/speex/*.h \
           taglib/ogg/vorbis/*.h \
           taglib/riff/*.h \
           taglib/riff/aiff/*.h \
           taglib/riff/wav/*.h \
           taglib/toolkit/*.h \
           taglib/trueaudio/*.h \
           taglib/wavpack/*.h \
           taglib/ogg/opus/*.h \
           taglib/dsf/*.h

SOURCES += taglib/*.cpp \
           taglib/ape/*.cpp \
           taglib/asf/*.cpp \
           taglib/flac/*.cpp \
           taglib/mp4/*.cpp \
           taglib/mpc/*.cpp \
           taglib/mpeg/*.cpp \
           taglib/mpeg/id3v1/*.cpp \
           taglib/mpeg/id3v2/*.cpp \
           taglib/mpeg/id3v2/frames/*.cpp \
           taglib/ogg/*.cpp \
           taglib/ogg/flac/*.cpp \
           taglib/ogg/speex/*.cpp \
           taglib/ogg/vorbis/*.cpp \
           taglib/riff/*.cpp \
           taglib/riff/aiff/*.cpp \
           taglib/riff/wav/*.cpp \
           taglib/toolkit/*.cpp \
           taglib/trueaudio/*.cpp \
           taglib/wavpack/*.cpp \
           taglib/ogg/opus/*.cpp \
           taglib/dsf/*.cpp

HEADERS += sparkleupdater.h

# QOCOA wrappers for OS X
INCLUDEPATH += qocoa
HEADERS += qocoa/qocoa_mac.h \
           qocoa/qbutton.h \
           qocoa/qprogressindicatorspinning.h

mac {
    OBJECTIVE_SOURCES += qocoa/qbutton_mac.mm \
                         qocoa/qprogressindicatorspinning_mac.mm
} else {
    SOURCES += qocoa/qbutton_nonmac.cpp \
               qocoa/qprogressindicatorspinning_nonmac.cpp
}




DEFINES += QOOBAR_NO_PROPERTY_MAPS
DEFINES += HAVE_ZLIB

CONFIG(release, debug|release):BUILD_DIR = ../../release
CONFIG(debug, debug|release):BUILD_DIR = ../../debug

DESTDIR = $$BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR/build
RCC_DIR = $$BUILD_DIR/build
MOC_DIR = $$BUILD_DIR/build

include(../libz.pri)

unix {
!mac {
    message(Linux build)
    CONFIG *= link_pkgconfig qt
    QT += dbus

    DEFINES *= QOOBAR_ENABLE_CLI

    HEADERS += clparser.h \
               argsparser.h
    SOURCES += argsparser.cpp \
               sparkleupdater_dummy.cpp

    INCLUDEPATH += libebur128
    SOURCES += libebur128/ebur128.c
    SOURCES += libebur128/filetree.c
    SOURCES += libebur128/input.c
    SOURCES += libebur128/input_ffmpeg.c
    SOURCES += libebur128/input_gstreamer.c
    SOURCES += libebur128/scanner-common.c
    SOURCES += libebur128/scanner-tag.c
    HEADERS += libebur128/ebur128.h
    HEADERS += libebur128/filetree.h
    HEADERS += libebur128/input.h
    HEADERS += libebur128/input_ffmpeg.h
    HEADERS += libebur128/input_gstreamer.h
    HEADERS += libebur128/queue.h
    HEADERS += libebur128/scanner-common.h
    HEADERS += libebur128/scanner-tag.h


    PKGCONFIG += glib-2.0
#    PKGCONFIG += libavcodec
#    PKGCONFIG += libavformat
#    PKGCONFIG += libavutil
#    DEFINES += USE_FFMPEG

    PKGCONFIG += gstreamer-app-0.10
    PKGCONFIG += gstreamer-audio-0.10
    DEFINES += WITH_DECODING

    EXEC_PATH = $${INSTALL_PREFIX}/bin
    PIXMAP_PATH = $${SHARED_PATH}/pixmaps
    MAN_PATH = $${SHARED_PATH}/man/man1
    DESKTOP_PATH = $${SHARED_PATH}/applications

    # targets for binary package
    icon.path = $$PIXMAP_PATH
    icon.files = icons/qoobar.png
    resources.path = /usr/share/qoobar
    resources.files = *.qm
    resources.files += args.json splitandconvert.sh
    schemes.path = $${SHARED_PATH}/qoobar/schemes
    schemes.files = schemes/*.xml
    completions.files = completions/*.txt
    completions.path = $${SHARED_PATH}/qoobar/completions
    manfiles.path = $$MAN_PATH
    manfiles.files = ../../qoobar.1
    desktop.path = $$DESKTOP_PATH
    desktop.files = ../../qoobar.desktop
    target.path = $$EXEC_PATH
    docfiles.path = $${DOC_PATH}/qoobar
    docfiles.files = ../../README*
    INSTALLS += target icon docfiles resources
    INSTALLS += schemes desktop manfiles completions

    # documentation
    htmldocfiles.path = $${DOC_PATH}/qoobar-doc/html
    htmldocfiles.files = ../../html/*
    htmldocfiles_html_en.path = $${DOC_PATH}/qoobar-doc/html/en
    htmldocfiles_html_en.files = ../../html/en/*.htm
    htmldocfiles_imgs_en.path = $${DOC_PATH}/qoobar-doc/html/en/images
    htmldocfiles_imgs_en.files = ../../html/en/images/*.png
    INSTALLS += htmldocfiles htmldocfiles_html_en htmldocfiles_imgs_en
}
}

win32|win {
    greaterThan(QT_MAJOR_VERSION, 4) {
        qtHaveModule(winextras) {
          QT *= winextras
        }
    }

# So far no cli support in Win
#    DEFINES *= QOOBAR_ENABLE_CLI

    RC_FILE = qoobar.rc

    exists(K:/My/build/winsparkle/Release/WinSparkle.lib) {
        INCLUDEPATH += K:/My/build/winsparkle/include
        LIBS += K:/My/build/winsparkle/Release/WinSparkle.lib
    }
    exists("D:/soft/Programming/WinSparkle-0.3/WinSparkle.lib") {
        INCLUDEPATH += D:/soft/Programming/WinSparkle-0.3
        LIBS += D:/soft/Programming/WinSparkle-0.3/WinSparkle.lib
    }
    SOURCES += sparkleupdater.cpp
}

os2 {
    message(OS/2 build)

# So far no cli support for OS/2 tested
#    DEFINES *= QOOBAR_ENABLE_CLI

    SOURCES += sparkleupdater_dummy.cpp

    INSTALL_PREFIX = /qoobar # <- edit this macro to change the installation root
    SHARED_PATH = $${INSTALL_PREFIX}
    DOC_PATH = $${SHARED_PATH}/doc
    EXEC_PATH = $${INSTALL_PREFIX}
    #PIXMAP_PATH = $${SHARED_PATH}/pixmaps
    MAN_PATH = $${SHARED_PATH}/man
    #DESKTOP_PATH = $${SHARED_PATH}/applications

    # targets for binary package
    #icon.path = $$PIXMAP_PATH
    #icon.files = icons/qoobar.png
    resources.path = $${SHARED_PATH}
    resources.files = *.qm
    resources.files += args.json
    schemes.path = $${SHARED_PATH}/schemes
    schemes.files = schemes/*.xml
    completions.files = completions/*.txt
    completions.path = $${SHARED_PATH}/completions
    manfiles.path = $$MAN_PATH
    manfiles.files = qoobar.1
    #desktop.path = $$DESKTOP_PATH
    #desktop.files = qoobar.desktop
    target.path = $$EXEC_PATH
    docfiles.path = $${DOC_PATH}
    docfiles.files = README*
    INSTALLS += target
    #INSTALLS += icon desktop
    INSTALLS += docfiles resources
    INSTALLS += schemes manfiles completions

    # documentation
    htmldocfiles.path = $${DOC_PATH}/html
    htmldocfiles.files = html/*
    htmldocfiles_html_en.path = $${DOC_PATH}/html/en
    htmldocfiles_html_en.files = html/en/*.htm
    htmldocfiles_imgs_en.path = $${DOC_PATH}/html/en/images
    htmldocfiles_imgs_en.files = html/en/images/*.png
    INSTALLS += htmldocfiles htmldocfiles_html_en htmldocfiles_imgs_en

    RC_FILE = qoobar_os2.rc
    INCLUDEPATH += . \
       o:/usr/local/include
}

mac|macx {
    message(Mac OS build)
# Don't want to enable cli in Mac OS X because can you ever imagine invoking
# "/Applications/qoobar.app/Contents/MacOS/qoobar smth..." in terminal?
#    DEFINES *= QOOBAR_ENABLE_CLI

    greaterThan(QT_MAJOR_VERSION, 4) {
        qtHaveModule(macextras) {
          QT *= macextras
        }
    }

    QMAKE_LFLAGS += -F/Frameworks
    LIBS += -F../../mac_os/ -framework discid
    INCLUDEPATH += ../../mac_os/discid.framework/Versions/A/Headers
    DEPENDPATH += ../../mac_os/discid.framework/Versions/A/Headers

    LIBS += -framework AppKit -framework Foundation
    greaterThan(QT_MAJOR_VERSION, 4) {
        LIBS += -F$$PWD/../../mac_os/ -framework Sparkle
        INCLUDEPATH += $$PWD/../../mac_os/Sparkle.framework/Versions/A/Headers
    }
    lessThan(QT_MAJOR_VERSION, 5) {
        LIBS += -framework Sparkle
    }


    ICON = qoobar.icns
    INSTALL_PATH = $$DESTDIR/qoobar.app/Contents

    OTHER_FILES += ../../Info.plist
    QMAKE_INFO_PLIST = ../../Info.plist
    HEADERS +=              cocoainit.h
    OBJECTIVE_SOURCES +=    sparkleupdater.mm \
                            cocoainit.mm \
                            mactaskbar.mm

    icon.path = $$INSTALL_PATH/Resources
    icon.files = qoobar.icns
    resources.path = $$INSTALL_PATH/Resources
    resources.files = *.qm
    resources.files += args.json splitandconvert.sh
    schemes.path = $$INSTALL_PATH/Resources/schemes
    schemes.files = schemes/*.xml
    completions.path = $$INSTALL_PATH/Resources/completions
    completions.files = completions/*.txt

    INSTALLS += icon resources schemes completions

    # documentation
    htmldocfiles.path = $$INSTALL_PATH/Resources/html
    htmldocfiles.files = ../../html/*
    htmldocfiles_html_en.path = $$INSTALL_PATH/Resources/html/en
    htmldocfiles_html_en.files = ../../html/en/*.htm
    htmldocfiles_imgs_en.path = $$INSTALL_PATH/Resources/html/en/images
    htmldocfiles_imgs_en.files = ../../html/en/images/*.png
    INSTALLS += htmldocfiles htmldocfiles_html_en htmldocfiles_imgs_en

    discid_framework.path = $$INSTALL_PATH/Frameworks/discid.framework
    discid_framework.files = ../../mac_os/discid.framework/*
    #discid_framework.extra= $$[QT_INSTALL_BINS]/macdeployqt $$DESTDIR/qoobar.app

    sparkle_framework.path = $$INSTALL_PATH/Frameworks/Sparkle.framework
    sparkle_framework.files = ../../mac_os/Sparkle.framework/*
    sparkle_framework.extra= $$[QT_INSTALL_BINS]/macdeployqt $$DESTDIR/qoobar.app

    utilities.path = $$INSTALL_PATH/Resources
    utilities.files = ../../mac_os/shntool ../../mac_os/flac ../../mac_os/mac \
    ../../mac_os/mp3gain ../../mac_os/enca ../../mac_os/vorbisgain ../../mac_os/metaflac \
    ../../mac_os/replaygain ../../mac_os/mppdec ../../mac_os/aacgain ../../mac_os/wvgain \
    ../../mac_os/*.dylib

    INSTALLS += discid_framework
    INSTALLS += utilities
    INSTALLS += sparkle_framework

    public_key.path = $$INSTALL_PATH/Resources
    public_key.files = ../../mac_os/dsa_pub.pem

    INSTALLS += public_key
}

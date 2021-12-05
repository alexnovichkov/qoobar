TEMPLATE = app
VERSION = 1.7.0
INSTALL_PREFIX = /usr
SHARED_PATH = $${INSTALL_PREFIX}/share
DOC_PATH = $${SHARED_PATH}/doc

DEFINES *= QOOBAR_VERSION=\\\"$$VERSION\\\"
DEFINES *= QOOBAR_SHARED_PATH=\\\"$${SHARED_PATH}/qoobar\\\"
DEFINES *= QOOBAR_DOC_PATH=\\\"$${DOC_PATH}/qoobar-doc\\\"

include(../portable.pri)

# this define adds the OS X specific features to the app.
# Right now all OS X support is switched off as I have no possibility to test it properly
# DEFINES *= OSX_SUPPORT_ENABLED

# place this define in OS-specific section to enable command-line interface
# DEFINES *= QOOBAR_ENABLE_CLI

QT *= widgets concurrent network

greaterThan(QT_MAJOR_VERSION, 5) {
  QT *= core5compat
}

# minimal c++ version is c++11
CONFIG *= c++11

TARGET = qoobar
CONFIG *= warn_on
CONFIG *= release
CONFIG *= no_keywords

# main sources and headers
SOURCES = main.cpp \
    checkabletablemodel.cpp \
    columnsmodel.cpp \
    fileiconprovider.cpp \
    headerview.cpp \
    importmodel.cpp \
    mainwindow.cpp \
    releaseinfomodel.cpp \
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
    treeview.cpp \
    styledbar.cpp \
    logging.cpp \
    autonumber.cpp

HEADERS = mainwindow.h \
    checkabletablemodel.h \
    columnsmodel.h \
    fileiconprovider.h \
    headerview.h \
    importmodel.h \
    releaseinfomodel.h \
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
    treeview.h \
    clparser.h \
    mactoolbar.h \
    styledbar.h \
    logging.h \
    qoobarhelp.h \
    autonumber.h

OTHER_FILES *= splitandconvert.bat \
               splitandconvert.sh

TRANSLATIONS = qoobar_ru.ts \
    qoobar_en.ts \
    qoobar_de.ts \
    qoobar_it.ts \
    qoobar_fr.ts \
    qoobar_pl.ts \
    qoobar_nl.ts

RESOURCES *= qoobar.qrc

system(lupdate qoobar_app.pro&&lrelease qoobar_app.pro)

#INCLUDEPATH += taglib
#HEADERS += taglib/tag.h   \
#           taglib/tagunion.h \
#           taglib/fileref.h \
#           taglib/audioproperties.h \
#           taglib/ape/apetag.h \
#           taglib/ape/apefile.h \
#           taglib/ape/apefooter.h \
#           taglib/ape/apeitem.h \
#           taglib/ape/apeproperties.h \
#           taglib/asf/asfattribute.h \
#           taglib/asf/asffile.h \
#           taglib/asf/asfpicture.h \
#           taglib/asf/asfproperties.h \
#           taglib/asf/asftag.h \
#           taglib/asf/asfutils.h \
#           taglib/flac/flacfile.h \
#           taglib/flac/flacmetadatablock.h \
#           taglib/flac/flacpicture.h \
#           taglib/flac/flacproperties.h \
#           taglib/flac/flacunknownmetadatablock.h \
#           taglib/mp4/mp4atom.h \
#           taglib/mp4/mp4coverart.h \
#           taglib/mp4/mp4file.h \
#           taglib/mp4/mp4item.h \
#           taglib/mp4/mp4tag.h \
#           taglib/mp4/mp4properties.h \
#           taglib/mpc/mpcfile.h \
#           taglib/mpc/mpcproperties.h \
#           taglib/mpeg/mpegfile.h \
#           taglib/mpeg/mpegproperties.h \
#           taglib/mpeg/mpegheader.h \
#           taglib/mpeg/xingheader.h \
#           taglib/mpeg/id3v1/id3v1genres.h \
#           taglib/mpeg/id3v1/id3v1tag.h \
#           taglib/mpeg/id3v2/id3v2header.h \
#           taglib/mpeg/id3v2/id3v2extendedheader.h \
#           taglib/mpeg/id3v2/id3v2footer.h \
#           taglib/mpeg/id3v2/id3v2frame.h \
#           taglib/mpeg/id3v2/id3v2framefactory.h \
#           taglib/mpeg/id3v2/id3v2synchdata.h \
#           taglib/mpeg/id3v2/id3v2tag.h \
#           taglib/mpeg/id3v2/frames/attachedpictureframe.h \
#           taglib/mpeg/id3v2/frames/chapterframe.h \
#           taglib/mpeg/id3v2/frames/commentsframe.h \
#           taglib/mpeg/id3v2/frames/eventtimingcodesframe.h \
#           taglib/mpeg/id3v2/frames/generalencapsulatedobjectframe.h \
#           taglib/mpeg/id3v2/frames/ownershipframe.h \
#           taglib/mpeg/id3v2/frames/popularimeterframe.h \
#           taglib/mpeg/id3v2/frames/privateframe.h \
#           taglib/mpeg/id3v2/frames/relativevolumeframe.h \
#           taglib/mpeg/id3v2/frames/synchronizedlyricsframe.h \
#           taglib/mpeg/id3v2/frames/tableofcontentsframe.h \
#           taglib/mpeg/id3v2/frames/textidentificationframe.h \
#           taglib/mpeg/id3v2/frames/uniquefileidentifierframe.h \
#           taglib/mpeg/id3v2/frames/unknownframe.h \
#           taglib/mpeg/id3v2/frames/urllinkframe.h \
#           taglib/mpeg/id3v2/frames/unsynchronizedlyricsframe.h \
#           taglib/ogg/oggfile.h \
#           taglib/ogg/oggpage.h \
#           taglib/ogg/oggpageheader.h \
#           taglib/ogg/xiphcomment.h \
#           taglib/ogg/flac/oggflacfile.h \
#           taglib/ogg/speex/speexfile.h \
#           taglib/ogg/speex/speexproperties.h \
#           taglib/ogg/opus/opusfile.h \
#           taglib/ogg/opus/opusproperties.h \
#           taglib/ogg/vorbis/vorbisfile.h \
#           taglib/ogg/vorbis/vorbisproperties.h \
#           taglib/riff/rifffile.h \
#           taglib/riff/aiff/aifffile.h \
#           taglib/riff/aiff/aiffproperties.h \
#           taglib/riff/wav/wavfile.h \
#           taglib/riff/wav/wavproperties.h \
#           taglib/riff/wav/infotag.h \
#           taglib/toolkit/taglib.h \
#           taglib/toolkit/tbytevector.h \
#           taglib/toolkit/tbytevectorlist.h \
#           taglib/toolkit/tbytevectorstream.h \
#           taglib/toolkit/tdebug.h \
#           taglib/toolkit/tfile.h \
#           taglib/toolkit/tfilestream.h \
#           taglib/toolkit/tiostream.h \
#           taglib/toolkit/tlist.h \
#           taglib/toolkit/tlist.tcc \
#           taglib/toolkit/tmap.h \
#           taglib/toolkit/tmap.tcc \
#           taglib/toolkit/tpropertymap.h \
#           taglib/toolkit/trefcounter.h \
#           taglib/toolkit/tstring.h \
#           taglib/toolkit/tstringlist.h \
#           taglib/toolkit/tutils.h \
#           taglib/toolkit/unicode.h \
#           taglib/trueaudio/trueaudiofile.h \
#           taglib/trueaudio/trueaudioproperties.h \
#           taglib/wavpack/wavpackfile.h \
#           taglib/wavpack/wavpackproperties.h \
#           taglib/dsf/dsffile.h \
#           taglib/dsf/dsfproperties.h

#SOURCES += taglib/tag.cpp   \
#           taglib/tagunion.cpp \
#           taglib/fileref.cpp \
#           taglib/audioproperties.cpp \
#           taglib/ape/apetag.cpp \
#           taglib/ape/apefile.cpp \
#           taglib/ape/apefooter.cpp \
#           taglib/ape/apeitem.cpp \
#           taglib/ape/apeproperties.cpp \
#           taglib/asf/asfattribute.cpp \
#           taglib/asf/asffile.cpp \
#           taglib/asf/asfpicture.cpp \
#           taglib/asf/asfproperties.cpp \
#           taglib/asf/asftag.cpp \
#           taglib/flac/flacfile.cpp \
#           taglib/flac/flacmetadatablock.cpp \
#           taglib/flac/flacpicture.cpp \
#           taglib/flac/flacproperties.cpp \
#           taglib/flac/flacunknownmetadatablock.cpp \
#           taglib/mp4/mp4atom.cpp \
#           taglib/mp4/mp4coverart.cpp \
#           taglib/mp4/mp4file.cpp \
#           taglib/mp4/mp4item.cpp \
#           taglib/mp4/mp4tag.cpp \
#           taglib/mp4/mp4properties.cpp \
#           taglib/mpc/mpcfile.cpp \
#           taglib/mpc/mpcproperties.cpp \
#           taglib/mpeg/mpegfile.cpp \
#           taglib/mpeg/mpegproperties.cpp \
#           taglib/mpeg/mpegheader.cpp \
#           taglib/mpeg/xingheader.cpp \
#           taglib/mpeg/id3v1/id3v1genres.cpp \
#           taglib/mpeg/id3v1/id3v1tag.cpp \
#           taglib/mpeg/id3v2/id3v2header.cpp \
#           taglib/mpeg/id3v2/id3v2extendedheader.cpp \
#           taglib/mpeg/id3v2/id3v2footer.cpp \
#           taglib/mpeg/id3v2/id3v2frame.cpp \
#           taglib/mpeg/id3v2/id3v2framefactory.cpp \
#           taglib/mpeg/id3v2/id3v2synchdata.cpp \
#           taglib/mpeg/id3v2/id3v2tag.cpp \
#           taglib/mpeg/id3v2/frames/attachedpictureframe.cpp \
#           taglib/mpeg/id3v2/frames/chapterframe.cpp \
#           taglib/mpeg/id3v2/frames/commentsframe.cpp \
#           taglib/mpeg/id3v2/frames/eventtimingcodesframe.cpp \
#           taglib/mpeg/id3v2/frames/generalencapsulatedobjectframe.cpp \
#           taglib/mpeg/id3v2/frames/ownershipframe.cpp \
#           taglib/mpeg/id3v2/frames/popularimeterframe.cpp \
#           taglib/mpeg/id3v2/frames/privateframe.cpp \
#           taglib/mpeg/id3v2/frames/relativevolumeframe.cpp \
#           taglib/mpeg/id3v2/frames/synchronizedlyricsframe.cpp \
#           taglib/mpeg/id3v2/frames/tableofcontentsframe.cpp \
#           taglib/mpeg/id3v2/frames/textidentificationframe.cpp \
#           taglib/mpeg/id3v2/frames/uniquefileidentifierframe.cpp \
#           taglib/mpeg/id3v2/frames/unknownframe.cpp \
#           taglib/mpeg/id3v2/frames/urllinkframe.cpp \
#           taglib/mpeg/id3v2/frames/unsynchronizedlyricsframe.cpp \
#           taglib/ogg/oggfile.cpp \
#           taglib/ogg/oggpage.cpp \
#           taglib/ogg/oggpageheader.cpp \
#           taglib/ogg/xiphcomment.cpp \
#           taglib/ogg/flac/oggflacfile.cpp \
#           taglib/ogg/speex/speexfile.cpp \
#           taglib/ogg/speex/speexproperties.cpp \
#           taglib/ogg/opus/opusfile.cpp \
#           taglib/ogg/opus/opusproperties.cpp \
#           taglib/ogg/vorbis/vorbisfile.cpp \
#           taglib/ogg/vorbis/vorbisproperties.cpp \
#           taglib/riff/rifffile.cpp \
#           taglib/riff/aiff/aifffile.cpp \
#           taglib/riff/aiff/aiffproperties.cpp \
#           taglib/riff/wav/wavfile.cpp \
#           taglib/riff/wav/wavproperties.cpp \
#           taglib/riff/wav/infotag.cpp \
#           taglib/toolkit/tbytevector.cpp \
#           taglib/toolkit/tbytevectorlist.cpp \
#           taglib/toolkit/tbytevectorstream.cpp \
#           taglib/toolkit/tdebug.cpp \
#           taglib/toolkit/tfile.cpp \
#           taglib/toolkit/tfilestream.cpp \
#           taglib/toolkit/tiostream.cpp \
#           taglib/toolkit/tpropertymap.cpp \
#           taglib/toolkit/trefcounter.cpp \
#           taglib/toolkit/tstring.cpp \
#           taglib/toolkit/tstringlist.cpp \
#           taglib/toolkit/unicode.cpp \
#           taglib/trueaudio/trueaudiofile.cpp \
#           taglib/trueaudio/trueaudioproperties.cpp \
#           taglib/wavpack/wavpackfile.cpp \
#           taglib/wavpack/wavpackproperties.cpp \
#           taglib/dsf/dsffile.cpp \
#           taglib/dsf/dsfproperties.cpp

HEADERS += sparkleupdater.h
HEADERS += argsparser.h
SOURCES += argsparser.cpp

# QOCOA wrappers for OS X
INCLUDEPATH += qocoa
HEADERS += qocoa/qocoa_mac.h \
           qocoa/qbutton.h \
           qocoa/qprogressindicatorspinning.h

mac {
    OBJECTIVE_SOURCES += qocoa/qbutton_mac.mm
    OBJECTIVE_SOURCES += qocoa/qprogressindicatorspinning_mac.mm
    OBJECTIVE_SOURCES += mactoolbar.mm \
                         qoobarhelp.mm
    OBJECTIVE_SOURCES += machelper.mm
} else {
    SOURCES += qocoa/qbutton_nonmac.cpp
    SOURCES += qocoa/qprogressindicatorspinning_nonmac.cpp
    SOURCES += mactoolbar.cpp  \
               qoobarhelp.cpp
    SOURCES += machelper.cpp
    HEADERS += impl.h
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

    SOURCES += sparkleupdater_dummy.cpp

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

    PKGCONFIG += gstreamer-app-1.0
    PKGCONFIG += gstreamer-audio-1.0
    DEFINES += WITH_DECODING

    EXEC_PATH = $${INSTALL_PREFIX}/bin
    PIXMAP_PATH = $${SHARED_PATH}/pixmaps
    ICON_PATH = $${SHARED_PATH}/qoobar/icons
    MAN_PATH = $${SHARED_PATH}/man/man1
    DESKTOP_PATH = $${SHARED_PATH}/applications

    # targets for binary package
    icon16.path = $$PIXMAP_PATH/16x16/apps
    icon16.files = icons/app/16/qoobar.png
    icon32.path = $$PIXMAP_PATH/32x32/apps
    icon32.files = icons/app/32/qoobar.png
    icon48.path = $$PIXMAP_PATH/48x48/apps
    icon48.files = icons/app/48/qoobar.png
    icon64.path = $$PIXMAP_PATH/64x64/apps
    icon64.files = icons/app/64/qoobar.png
    icon128.path = $$PIXMAP_PATH/128x128/apps
    icon128.files = icons/app/128/qoobar.png
    icon256.path = $$PIXMAP_PATH/256x256/apps
    icon256.files = icons/app/256/qoobar.png
    INSTALLS += icon16 icon32 icon48 icon64 icon128 icon256
    resources.path = $${SHARED_PATH}/qoobar
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
    icons.files = icons/maia
#    icons.files += icons/*.png
#    icons.files += icons/*.gif
    icons.path = $${SHARED_PATH}/qoobar/icons
    coloredicons.files = icons/coloured
#    coloredicons.files += icons/coloured/*.png
#    coloredicons.files += icons/coloured/*.gif
#    coloredicons.files += icons/coloured/*.json
    coloredicons.path = $${SHARED_PATH}/qoobar/icons
    INSTALLS += target docfiles resources icons coloredicons
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
    qtHaveModule(winextras) {
      QT *= winextras
    }

# So far no cli support in Win
#    DEFINES *= QOOBAR_ENABLE_CLI

    RC_FILE = qoobar.rc

    contains(QT_ARCH, "i386") {
        INCLUDEPATH *= G:/soft/Programming/WinSparkle-0.7.0/include
        LIBS *= G:/soft/Programming/WinSparkle-0.7.0/Release/WinSparkle.lib
        INCLUDEPATH *= G:/soft/Programming/taglib-1.12.1/include
        LIBS *= G:/soft/Programming/taglib-1.12.1/lib/libtag.dll.a
    }
    contains(QT_ARCH, "x86_64") {
        INCLUDEPATH *= G:/soft/Programming/WinSparkle-0.7.0/include
        LIBS *= G:/soft/Programming/WinSparkle-0.7.0/x64/Release/WinSparkle.lib
        INCLUDEPATH *= G:/soft/Programming/taglib-1.12.1-x64/include
        LIBS *= G:/soft/Programming/taglib-1.12.1-x64/lib/libtag.dll.a
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
    icons.files = icons/*.ico
    icons.files += icons/*.png
    icons.files += icons/*.gif
    icons.path = $${SHARED_PATH}/icons/default
    coloredicons.files = icons/coloured/*.ico
    coloredicons.files += icons/coloured/*.png
    coloredicons.files += icons/coloured/*.gif
    coloredicons.files += icons/coloured/*.json
    coloredicons.path = $${SHARED_PATH}/icons/coloured
    INSTALLS += target
    #INSTALLS += icon desktop
    INSTALLS += docfiles resources icons coloredicons
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

#mac|macx {
#    message(Mac OS build)

#    OTHER_FILES *= ../../mac-deploy.command

#    DEFINES *= QOOBAR_ENABLE_CLI

#    qtHaveModule(macextras) {
#      QT *= macextras
#    }

#    LIBS += -F../../mac_os/ -framework discid
#    INCLUDEPATH += ../../mac_os/discid.framework/Versions/A/Headers
#    DEPENDPATH += ../../mac_os/discid.framework/Versions/A/Headers

#    LIBS += -framework AppKit -framework Foundation
#    LIBS += -F$$PWD/../../mac_os/ -framework Sparkle
#    INCLUDEPATH += $$PWD/../../mac_os/Sparkle.framework/Versions/A/Headers



#    ICON = icons/app/qoobar.icns
#    INSTALL_PATH = $$DESTDIR/qoobar.app/Contents

#    OTHER_FILES += ../../Info.plist \
#                   ../../Entitlements.plist
#    QMAKE_INFO_PLIST = ../../Info.plist
#    HEADERS +=              cocoainit.h
#    OBJECTIVE_SOURCES +=    sparkleupdater.mm \
#                            cocoainit.mm \
#                            mactaskbar.mm

#    icon.path = $$INSTALL_PATH/Resources
#    icon.files = icons/app/qoobar.icns
#    resources.path = $$INSTALL_PATH/Resources
#    resources.files = *.qm
#    resources.files += args.json splitandconvert.sh
#    schemes.path = $$INSTALL_PATH/Resources/schemes
#    schemes.files = schemes/*.xml
#    completions.path = $$INSTALL_PATH/Resources/completions
#    completions.files = completions/*.txt
#    icons.files = icons/*.ico
#    icons.files += icons/*.png
#    icons.files += icons/*.gif
#    icons.path = $$INSTALL_PATH/Resources/icons/default
#    coloredicons.files = icons/coloured/*.ico
#    coloredicons.files += icons/coloured/*.png
#    coloredicons.files += icons/coloured/*.gif
#    coloredicons.files += icons/coloured/*.json
#    coloredicons.path = $$INSTALL_PATH/Resources/icons/coloured

#    INSTALLS += icon resources schemes completions icons coloredicons

#    public_key.path = $$INSTALL_PATH/Resources
#    public_key.files = ../../mac_os/dsa_pub.pem

#    INSTALLS += public_key
#}

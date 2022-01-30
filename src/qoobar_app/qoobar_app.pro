TEMPLATE = app

# reading file version.h to get the actual app version
VERSIONH = $$cat(version.h, lines)
VERSIONH = $$first(VERSIONH)
VERSIONH = $$section(VERSIONH, \",1,1)
VERSION = $$section(VERSIONH, \\,0,0)
DEFINES *= QOOBAR_VERSION=\\\"$$VERSION\\\"

message(Version is $${VERSION})

QT *= widgets concurrent network

greaterThan(QT_MAJOR_VERSION, 5) {
  QT *= core5compat
}

TARGET = qoobar

# minimal c++ version is c++11
CONFIG *= c++11
CONFIG *= warn_on
CONFIG *= release
CONFIG *= no_keywords

# The install directory. When compiling into deb should be /usr. When make-installed should be /usr/local.
# When compiling into AppImage should be something like ~/deploy/qoobar/AppDir
# INSTALL_ROOT = /

# used for non-shadow building
CONFIG(release, debug|release):BUILD_DIR = ../../release
CONFIG(debug, debug|release):BUILD_DIR = ../../debug

DESTDIR = $$BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR/build
RCC_DIR = $$BUILD_DIR/build
MOC_DIR = $$BUILD_DIR/build


message(The install path is $${INSTALL_ROOT})

SHARED_PATH = $${INSTALL_ROOT}/share
DOC_PATH = $${SHARED_PATH}/doc

DEFINES *= QOOBAR_SHARED_PATH=\\\"$${SHARED_PATH}/$${TARGET}\\\"
DEFINES *= QOOBAR_DOC_PATH=\\\"$${DOC_PATH}/$${TARGET}-doc\\\"

include(../portable.pri)

# 3rdparty libraries Qoobar links with
include(../3rdparty/libz.pri)
include(../3rdparty/libtag.pri)
include(../3rdparty/libffmpeg.pri)
include(../3rdparty/libdiscid.pri)
include(../3rdparty/libebur128.pri)
include(../3rdparty/libsparkle.pri)

# this define adds the OS X specific features to the app.
# Right now all OS X support is switched off as I have no possibility to test it properly
# DEFINES *= OSX_SUPPORT_ENABLED

# place this define in OS-specific section to enable command-line interface
# DEFINES *= QOOBAR_ENABLE_CLI


# main sources and headers
SOURCES = main.cpp \
    checkabletablemodel.cpp \
    columnsmodel.cpp \
    dsffile.cpp \
    dsfproperties.cpp \
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
    autonumber.cpp \
    argsparser.cpp

HEADERS = mainwindow.h \
    checkabletablemodel.h \
    columnsmodel.h \
    dsffile.h \
    dsfproperties.h \
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
    autonumber.h \
    sparkleupdater.h \
    argsparser.h

# loudgain-master
# Qoobar uses parts of the loudgain sources. TODO: rewrite them
  HEADERS += loudgain-master/src/lg-util.h \
  loudgain-master/src/scan.h \
  loudgain-master/src/printf.h

  SOURCES += loudgain-master/src/scan.c \
  loudgain-master/src/printf.c

# Scripts for splitting images by cue
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

# OS-specific settings

unix {
!mac {
    message(Linux build)
    CONFIG *= link_pkgconfig qt
    QT += dbus

    DEFINES *= QOOBAR_ENABLE_CLI

    SOURCES += sparkleupdater_dummy.cpp

    # EXEC_PATH = $${INSTALL_ROOT}/usr/bin
    EXEC_PATH = $${INSTALL_ROOT}/bin
# Ubuntu 20.04 requires the app icons be placed into /usr/local/share/  
# if install from make install, and into /usr/share/  if installed from deb
    PIXMAP_PATH = $${SHARED_PATH}/icons/hicolor
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
  # winextras module, or just QWinTaskbarProgress in case of Qt6
  lessThan(QT_MAJOR_VERSION, 6) {
      QT *= winextras
  }
  else {
      LIBS += -lole32 -lshlwapi -lshell32 -ldwmapi
      win32:!qtHaveModule(opengl)|qtConfig(dynamicgl):LIBS += -lgdi32
      HEADERS +=  winextras/qwintaskbarbutton.h \
                  winextras/qwintaskbarbutton_p.h \
                  winextras/qwintaskbarprogress.h \
                  winextras/qwinfunctions.h \
                  winextras/qwinevent.h \
                  winextras/qwineventfilter_p.h \
                  winextras/qwinfunctions_p.h \
                  winextras/windowsguidsdefs_p.h \
                  winextras/winshobjidl_p.h \
                  winextras/winpropkey_p.h
      SOURCES +=  winextras/qwintaskbarbutton.cpp \
                  winextras/qwintaskbarprogress.cpp \
                  winextras/qwinfunctions.cpp \
                  winextras/qwinevent.cpp \
                  winextras/qwineventfilter.cpp \
                  winextras/windowsguidsdefs.cpp
  }
    SOURCES += sparkleupdater.cpp

  # So far no cli support in Win
#  DEFINES *= QOOBAR_ENABLE_CLI

# Two distinct RC files with different appcast.xml to separately update
# Qt6 version and Qt5 version
greaterThan(QT_MAJOR_VERSION, 5) {
  message(Qt6 appcast.xml is used)
  RC_FILE = qoobar.rc
}
else {
  message(Qt5 appcast-qt5.xml is used)
  RC_FILE = qoobar-qt5.rc
}
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

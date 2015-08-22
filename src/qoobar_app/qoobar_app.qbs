import qbs

CppApplication {
    name: qbs.targetOS.contains("osx") ? "Qoobar" : "qoobar"

    consoleApplication: false

    Depends { name: "Qt.core"}
    Depends { name: "Qt.widgets" }
    Depends { name: "Qt.network" }
    Depends { name: "Qt.gui" }
    Depends { name: "Qt.concurrent" }

    property string qoobarVersion: '1.6.7'

    property string sharedPath
    property string docPath
    property string execPath
    property string pixmapPath
    property string manPath
    property string desktopPath
    property string resourcesPath
    property string htmlPath
    property string readmePath


    cpp.defines: {
        var list = [ "QOOBAR_VERSION=\"1.6.7\"",
                    "QOOBAR_SHARED_PATH=\""+sharedPath+"\"",
                    "QOOBAR_DOC_PATH=\""+docPath+"/qoobar-doc\"",
                    "QOOBAR_NO_PROPERTY_MAPS", "HAVE_ZLIB",
                    "HAVE_QT5",
                ]
        if (qbs.targetOS.contains("unix") && !qbs.targetOS.contains("osx")) list.concat(["WITH_DECODING"])
        print(list)
        return list
    }

    cpp.linkerFlags: {
        if (qbs.buildVariant == "release" && (qbs.toolchain == "gcc" || qbs.toolchain == "mingw"))
            return ["-Wl,-s"]
    }
    cpp.warningLevel: "all"
    cpp.treatWarningsAsErrors: false
    cpp.includePaths: ["../taglib","../..", "taglib" ]


    files: {
        var baseFiles = [
                    "main.cpp",
                    "mainwindow.*",
                    "tagseditor.*",
                    "tablewidget.*",
                    "filesrenamer.*",
                    "tagsfiller.*",
                    "tagger.*",
                    "settingsdialog.*",
                    "tab.*",
                    "completerdelegate.*",
                    "lineedit.*",
                    "splitdialog.*",
                    "qoobarglobals.*",
                    "undoactions.*",
                    "imagedialog.*",
                    "tabwidget.*",
                    "imagebox.*",
                    "treewidget.*",
                    "legendbutton.*",
                    "displayedtagsdialog.*",
                    "application.*",
                    "discidhelper.*",
                    "texteditdelegate.*",
                    "stringroutines.*",
                    "id3v1stringhandler.*",
                    "newtagdialog.*",
                    "schemeeditor.*",
                    "configpages.*",
                    "releaseinfowidget.*",
                    "taggingscheme.*",
                    "completionsdialog.*",
                    "macsplitter.*",
                    "replaygaindialog.*",
                    "mp3tagsdialog.*",
                    "processlinemaker.*",
                    "statusbar.*",
                    "searchpanel.*",
                    "highlightdelegate.*",
                    "filedelegatehighlighter.*",
                    "checkableheaderview.*",
                    "sections.*",
                    "sectiondelegatehighlighter.*",
                    "delegatehighlighter.*",
                    "coverimage.*",
                    "tagsreaderwriter.*",
                    "corenetworksearch.*",
                    "tagparser.*",
                    "placeholders.*",
                    "fancylineedit.*",
                    "clearlineedit.*",
                    "model.*",
                    "replaygainer.*",
                    "columnsdialog.*",
                    "cuesplitter.*",
                    "filenamerenderer.*",
                    "tagsrenderer.*",
                    "argsparser.*",
                    "searchresults.h",
                    "applicationpaths.*",
                    "delegatehighlighter.h",
                    "iqoobarplugin.h",
                    "enums.h",
                    "idownloadplugin.h",
                    "clparser.h",
                    "qoobar.qrc",
                    "qoobar.rc"
                ]
        //if (qbs.targetOS.contains("windows")) baseFiles.concat(["qoobar.rc"])
        return baseFiles
    }

    //properties for Qt 5.0 or newer
    Properties {
        condition: Qt.core.versionMajor >= 5
        cpp.cxxFlags: base.concat(["-std=c++11"])
    }
    //properties for release build
    Properties {
        condition: qbs.buildVariant == "release"
        cpp.debugInformation: false
        cpp.optimization: "false"
        qbs.destinationDirectory: "../../release"
    }
    //properties for debug build
    Properties {
        condition: qbs.buildVariant == "debug"
        cpp.debugInformation: true
        cpp.optimization: "none"
        qbs.destinationDirectory: "../../debug"
    }

    // Linux and like
    Properties {
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("osx")

        //Depends:
        sharedPath: '/usr/share'
        docPath: sharedPath+'/doc'
        execPath: '/usr/bin'
        pixmapPath: sharedPath+'/pixmaps'
        manPath: sharedPath+'/man/man1'
        desktopPath: sharedPath+'/applications'
        resourcesPath: sharedPath+'/qoobar'
        htmlPath: docPath+'/qoobar-doc'
        readmePath: docPath+'/qoobar'

//                    PKGCONFIG += glib-2.0
//        #    PKGCONFIG += libavcodec
//        #    PKGCONFIG += libavformat
//        #    PKGCONFIG += libavutil
//        #    DEFINES += USE_FFMPEG

//            PKGCONFIG += gstreamer-app-0.10
//            PKGCONFIG += gstreamer-audio-0.10
//            CONFIG += link_pkgconfig qt
//            PKGCONFIG += zlib
    }

    // Mac OS X
    Properties {
        condition: qbs.targetOS.contains("osx")
        // TO DO
        sharedPath: '/usr/share'
        docPath: sharedPath+'/doc'
        execPath: '/bin'
        pixmapPath: sharedPath+'/pixmaps'
        manPath: sharedPath+'/man/man1'
        desktopPath: sharedPath+'/applications'
        resourcesPath: sharedPath+'/qoobar'
        htmlPath: docPath+'/qoobar-doc'
        readmePath: docPath+'/qoobar'

        cpp.dynamicLibraries: base.concat(["z"])
        cpp.frameworks: []
    }

    //Windows
    Properties {
        condition: qbs.targetOS.contains("windows")

        sharedPath: {
            print("QBS toolchain="+qbs.toolchain)
            return qbs.buildDirectory
        }
        docPath: sharedPath
        execPath: sharedPath
        pixmapPath: sharedPath
        manPath: sharedPath
        desktopPath: sharedPath
        resourcesPath: sharedPath
        htmlPath: docPath
        readmePath: docPath

        cpp.staticLibraries: base.concat(["K:/My/programming/sources/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a"])
        cpp.includePaths: base.concat(["K:/My/programming/sources/libz-1.2.3-1-mingw32-dev/include"])
        //files: base.concat(["qoobar.rc"])
    }

    Group {
        name: "json"
        condition: Qt.core.versionMajor < 5
        files: [ "ereilin/json.cpp", "ereilin/json.h" ]
    }

    Group {
        name: "TagLib"
        files: [
            "taglib/*.h", "taglib/*.cpp",
            "taglib/ape/*.h", "taglib/ape/*.cpp",
            "taglib/asf/*.h", "taglib/asf/*.cpp",
            "taglib/flac/*.h", "taglib/flac/*.cpp",
            "taglib/mp4/*.h", "taglib/mp4/*.cpp",
            "taglib/mpc/*.h", "taglib/mpc/*.cpp",
            "taglib/mpeg/*.h", "taglib/mpeg/*.cpp",
            "taglib/mpeg/id3v1/*.h", "taglib/mpeg/id3v1/*.cpp",
            "taglib/mpeg/id3v2/*.h", "taglib/mpeg/id3v2/*.cpp",
            "taglib/mpeg/id3v2/frames/*.h", "taglib/mpeg/id3v2/frames/*.cpp",
            "taglib/ogg/*.h", "taglib/ogg/*.cpp",
            "taglib/ogg/flac/*.h", "taglib/ogg/flac/*.cpp",
            "taglib/ogg/speex/*.h", "taglib/ogg/speex/*.cpp",
            "taglib/ogg/vorbis/*.h", "taglib/ogg/vorbis/*.cpp",
            "taglib/riff/*.h", "taglib/riff/*.cpp",
            "taglib/riff/aiff/*.h", "taglib/riff/aiff/*.cpp",
            "taglib/riff/wav/*.h", "taglib/riff/wav/*.cpp",
            "taglib/toolkit/*.h", "taglib/toolkit/*.cpp",
            "taglib/trueaudio/*.h", "taglib/trueaudio/*.cpp",
            "taglib/wavpack/*.h", "taglib/wavpack/*.cpp",
            "taglib/ogg/opus/*.h", "taglib/ogg/opus/*.cpp",
        ]
        cpp.includePaths: ["..","../..", "taglib" ]
    }



    Group {
        name: "libebur128"
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("osx")
        files: [
            "libebur128/*.c",
            "libebur128/*.h",
        ]
        cpp.includePaths: outer.concat([ "libebur128" ])
    }

    Group {
        name: "Translations"
        files: [
            "qoobar_en.ts",
            "qoobar_de.ts",
            "qoobar_it.ts",
            "qoobar_fr.ts",
            "qoobar_pl.ts",
            "qoobar_nl.ts",
            "qoobar_ru.ts",
        ]
        qbs.install: true
        qbs.installDir: resourcesPath
    }

    Group {
        name: "icon"
        files: [ "icons/qoobar.png" ]
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("osx")
        qbs.install: true
        qbs.installDir: pixmapPath
    }

    Group {
        name: "resources"
        files: [ "*.qm", "args.json" ]
        qbs.install: true
        qbs.installDir: resourcesPath
    }

    Group {
        name: "schemes"
        files: [ "schemes/*.xml" ]
        qbs.install: true
        qbs.installDir: resourcesPath+'/schemes'
    }

    Group {
        name: "completions"
        files: [ "completions/*.txt" ]
        qbs.install: true
        qbs.installDir: resourcesPath+'/completions'
    }

    Group {
        name: "man"
        files: [ "../../qoobar.1" ]
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("osx")
        qbs.install: true
        qbs.installDir: manPath
    }

    Group {
        name: "desktop"
        files: [ "../../qoobar.desktop" ]
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("osx")
        qbs.install: true
        qbs.installDir: desktopPath
    }

    Group {
        name: "readme"
        files: [ "../../README*" ]
        qbs.install: true
        qbs.installDir: readmePath //docPath+'/qoobar'
    }

    Group {
        name: "docs"
        prefix: "../../html/"
        files: [ "*", "en/*.htm", "en/images/*.png" ]
        qbs.install: true
        qbs.installDir: htmlPath
    }

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: execPath
    }
}

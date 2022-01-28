win32|win {
    include(../../../win-libs/libsparkle.pri)
}

mac {
    LIBS += -framework AppKit -framework Foundation
    LIBS += -F$$PWD/../../mac_os/ -framework Sparkle
    INCLUDEPATH += $$PWD/../../mac_os/Sparkle.framework/Versions/A/Headers
}

unix {
!mac {
    #no sparkle lib in linux
}
}

os2 {
    #needs testing
}

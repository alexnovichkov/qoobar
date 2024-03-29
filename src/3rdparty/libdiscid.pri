win32|win {
    include(../../../win-libs/libdiscid.pri)
}

mac {
    LIBS += -F../../../mac_os/ -framework discid
    INCLUDEPATH += ../../../mac_os/discid.framework/Versions/A/Headers
    DEPENDPATH += ../../../mac_os/discid.framework/Versions/A/Headers
}

unix {
!mac {
    CONFIG *= link_pkgconfig
    PKGCONFIG *= libdiscid
}
}

os2 {

}

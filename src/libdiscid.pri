win32|win {

}

mac {
    LIBS += -F../../../mac_os/ -framework discid
    INCLUDEPATH += ../../../mac_os/discid.framework/Versions/A/Headers
    DEPENDPATH += ../../../mac_os/discid.framework/Versions/A/Headers
}

unix {
!mac {

}
}

os2 {

}

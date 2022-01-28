win32|win {
    include(../../../win-libs/libz.pri)
}

mac {
    LIBS += -lz
}

unix {
!mac {
    CONFIG *= link_pkgconfig
    PKGCONFIG *= zlib
}
}

os2 {

}

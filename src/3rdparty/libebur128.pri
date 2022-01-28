win32|win {
    include(../../../win-libs/libebur128.pri)
}

mac {
    #needs testing
}

unix {
!mac {
    CONFIG *= link_pkgconfig
    PKGCONFIG *= libebur128
}
}

os2 {
    #needs testing
}

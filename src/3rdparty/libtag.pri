win32|win {
    include(../../../win-libs/libtag.pri)
}

mac {
    # needs rewriting
}

unix {
!mac {
    CONFIG *= link_pkgconfig
    PKGCONFIG *= taglib
}
}

os2 {
    # needs rewriting
}

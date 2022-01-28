win32|win {
    include(../../../win-libs/libffmpeg.pri)
}

mac {
    # needs testing
}

unix {
!mac {
    CONFIG *= link_pkgconfig
    PKGCONFIG *= libavcodec
    PKGCONFIG *= libavformat
    PKGCONFIG *= libavutil
    PKGCONFIG *= libswresample
}
}

os2 {
    # needs testing
}

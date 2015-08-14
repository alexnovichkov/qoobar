win32|win {
    exists(K:/My/programming/sources/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a) {
        INCLUDEPATH += K:/My/programming/sources/libz-1.2.3-1-mingw32-dev/include
        LIBS += K:/My/programming/sources/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a
    }
    exists(D:/soft/Programming/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a) {
        INCLUDEPATH += D:/soft/Programming/libz-1.2.3-1-mingw32-dev/include
        LIBS += D:/soft/Programming/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a
    }
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

win32|win {
    exists(E:/My/programming/sources/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a) {
        INCLUDEPATH += E:/My/programming/sources/libz-1.2.3-1-mingw32-dev/include
        LIBS += E:/My/programming/sources/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a
    }
    exists(G:/soft/Programming/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a) {
        INCLUDEPATH += G:/soft/Programming/libz-1.2.3-1-mingw32-dev/include
        LIBS += G:/soft/Programming/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a
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

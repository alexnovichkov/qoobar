win32|win {
    exists(E:/My/programming/sources/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a) {
        INCLUDEPATH += E:/My/programming/sources/libz-1.2.3-1-mingw32-dev/include
        LIBS += E:/My/programming/sources/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a
    }
    exists(G:/soft/Programming/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a) {
        INCLUDEPATH += G:/soft/Programming/libz-1.2.3-1-mingw32-dev/include
        LIBS += G:/soft/Programming/libz-1.2.3-1-mingw32-dev/lib/libz.dll.a
    }
    # 64-bit zlib, from mingw8.1
    exists(C:\Qt\Tools\mingw810_64\x86_64-w64-mingw32\lib\libz.a) {
        INCLUDEPATH += G:/soft/Programming/libz-1.2.3-1-mingw32-dev/include
        LIBS += C:\Qt\Tools\mingw810_64\x86_64-w64-mingw32\lib\libz.a
        #DEFINES *= ZLIB_WINAPI
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

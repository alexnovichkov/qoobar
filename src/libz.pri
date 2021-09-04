win32|win {
    # 64-bit zlib, from mingw8.1
    contains(QT_ARCH, "x86_64") {
    message("64-bit zlib, from mingw8.1")
        INCLUDEPATH *= G:/soft/Programming/libz-1.2.3-1-mingw32-dev/include
        LIBS += C:\Qt6\Tools\mingw810_64\x86_64-w64-mingw32\lib\libz.a
        #DEFINES *= ZLIB_WINAPI
    }
    # 32-bit zlib, from mingw8.1
    contains(QT_ARCH, "i386") {
        message("32-bit zlib, from mingw8.1")
        INCLUDEPATH *= G:/soft/Programming/libz-1.2.3-1-mingw32-dev/include
        LIBS += C:\Qt6\Tools\mingw810_32\i686-w64-mingw32\lib\libz.a
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

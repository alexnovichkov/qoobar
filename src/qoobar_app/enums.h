#ifndef ENUMS_H
#define ENUMS_H

enum {
    COMPOSER,
    ALBUM,
    TITLE,
    PERFORMER,
    ARTIST,
    CONDUCTOR,
    ORCHESTRA,
    SUBTITLE,
    KEY,
    COMMENT,
    GENRE,
    YEAR,//11
    TRACKNUMBER,//12
    TOTALTRACKS,//13
    ALBUMARTIST,//14
    CATEGORY,
    PUBLISHER,
    COPYRIGHT,
    MOOD,
    TEMPO,
    LYRICIST,
    LYRICS,
    DISCNUMBER,
    TOTALDISCS,
    ENCODEDBY,
    REMIXEDBY,
    RATING,
    ORIGINALALBUM,
    ORIGINALARTIST,
    ORIGINALLYRICIST,
    URL,
    ISRC,

    TAGSCOUNT
};

#include <QtGlobal>

enum MessageType {
    MT_WARNING=0,
    MT_INFORMATION=1,
    MT_ERROR=2,
    MT_SUCCESS=3
};

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#define LOCATION_PATH location
#else
#define LOCATION_PATH path
#endif

#ifdef Q_OS_WIN
#define FILE_NAME(f) reinterpret_cast<const wchar_t *>((f).utf16())
#else
#define FILE_NAME(f) QFile::encodeName((f)).constData()
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    #define SKIP_EMPTY_PARTS QString::SkipEmptyParts
#else
    #define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
    #define HORIZONTAL_ADVANCE width
#else
    #define HORIZONTAL_ADVANCE horizontalAdvance
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    #define BIRTH_TIME created
#else
    #define BIRTH_TIME birthTime
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    #define STYLE_OPTION_VIEW_ITEM QStyleOptionViewItemV4
#else
    #define STYLE_OPTION_VIEW_ITEM QStyleOptionViewItem
#endif


#define QSL(s) QStringLiteral(s)

#define QLS(s) QLatin1String(s)

#define SMALL_ICON_SIZE 16
#define MEDIUM_ICON_SIZE 24
#define LARGE_ICON_SIZE 32

#define MAXIMUM_FILENAME_LENGTH 255

enum {
    ALWAYSVISIBLE=0,
    VISIBLEIFNOTEMPTY=1,
    HIDDEN=2
};

enum ID3V1TagsMode {
    ID3V1_UPDATE_ALWAYS=0,
    ID3V1_UPDATE_ONLY_EXISTING=1,
    ID3V1_DELETE=2
};

enum TagType {
    TAG_NONE  =0x0000,
    TAG_ID3V2 =0x0001,
    TAG_ID3V1 =0x0002,
    TAG_APE   =0x0004,
    TAG_VORBIS=0x0008,
    TAG_ASF   =0x0010,
    TAG_MP4   =0x0020,
    TAG_ALL   =0xffff
};

enum SortType {
    SortString,
    SortInt,
    SortTime,
    SortFloat
};

#endif // ENUMS_H

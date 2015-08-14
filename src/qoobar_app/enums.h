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
    YEAR,
    TRACKNUMBER,
    TOTALTRACKS,
    ALBUMARTIST,
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

enum MessageType {
    MT_WARNING=0,
    MT_INFORMATION=1,
    MT_ERROR=2,
    MT_SUCCESS=3
};

#ifdef Q_OS_WIN32
#define FILE_NAME(f) reinterpret_cast<const wchar_t *>((f).utf16())
#else
#define FILE_NAME(f) QFile::encodeName((f)).constData()
#endif

#ifdef HAVE_QT5
  #define SETSECTIONRESIZEMODE setSectionResizeMode
  #define SETSECTIONSCLICKABLE setSectionsClickable
#else
  #define SETSECTIONRESIZEMODE setResizeMode
  #define SETSECTIONSCLICKABLE setClickable
#endif

#ifdef HAVE_QT5
#define QSL(s) QStringLiteral(s)
#else
#define QSL(s) (s)
#endif

#define QLS(s) QLatin1String(s)

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

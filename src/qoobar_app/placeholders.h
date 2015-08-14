#ifndef PLACEHOLDERS_H
#define PLACEHOLDERS_H

#include <QString>

namespace Placeholders
{
    struct Placeholder {
        const char placeholder;
        const char *text;
        const char *description;
        const int index;
    };

    //-1 means formatted track number
    //-2 means read-only field
    //-3 means void placeholder
    const int placeholdersCount = 44;
    const Placeholder placeholders[placeholdersCount]= {
        {'c', "%c", QT_TRANSLATE_NOOP("Placeholders","Composer"), 0},
        {'a', "%a", QT_TRANSLATE_NOOP("Placeholders","Album"), 1},
        {'t', "%t", QT_TRANSLATE_NOOP("Placeholders","Title"), 2},
        {'p', "%p", QT_TRANSLATE_NOOP("Placeholders","Performer"), 3},
        {'A', "%A", QT_TRANSLATE_NOOP("Placeholders","Artist"), 4},
        {'d', "%d", QT_TRANSLATE_NOOP("Placeholders","Conductor"), 5},
        {'o', "%o", QT_TRANSLATE_NOOP("Placeholders","Orchestra"), 6},
        {'s', "%s", QT_TRANSLATE_NOOP("Placeholders","Subtitle"), 7},
        {'k', "%k", QT_TRANSLATE_NOOP("Placeholders","Key"), 8},
        {'C', "%C", QT_TRANSLATE_NOOP("Placeholders","Comment"), 9},
        {'g', "%g", QT_TRANSLATE_NOOP("Placeholders","Genre"), 10},
        {'y', "%y", QT_TRANSLATE_NOOP("Placeholders","Year"), 11},
        {'n', "%n", QT_TRANSLATE_NOOP("Placeholders","Track number"), 12},
        {'N', "%N", QT_TRANSLATE_NOOP("Placeholders","Track number"), -1},
        {'T', "%T", QT_TRANSLATE_NOOP("Placeholders","Total tracks"), 13},
        {'u', "%u", QT_TRANSLATE_NOOP("Placeholders","Album artist"), 14},
        {'G', "%G", QT_TRANSLATE_NOOP("Placeholders","Musical category"), 15},
        {'P', "%P", QT_TRANSLATE_NOOP("Placeholders","Publisher"), 16},
        {'X', "%X", QT_TRANSLATE_NOOP("Placeholders","Copyright"), 17},
        {'m', "%m", QT_TRANSLATE_NOOP("Placeholders","Mood"), 18},
        {'B', "%B", QT_TRANSLATE_NOOP("Placeholders","Tempo"), 19},
        {'w', "%w", QT_TRANSLATE_NOOP("Placeholders","Lyricist"), 20},
        {'W', "%W", QT_TRANSLATE_NOOP("Placeholders","Lyrics"), 21},
        {'D', "%D", QT_TRANSLATE_NOOP("Placeholders","Disc number"), 22},
        {'S', "%S", QT_TRANSLATE_NOOP("Placeholders","Total discs"), 23},
        {'e', "%e", QT_TRANSLATE_NOOP("Placeholders","Encoded by"), 24},
        {'x', "%x", QT_TRANSLATE_NOOP("Placeholders","Remixed by"), 25},
        {'R', "%R", QT_TRANSLATE_NOOP("Placeholders","Rating"), 26},
        {'O', "%O", QT_TRANSLATE_NOOP("Placeholders","Original album"), 27},
        {'v', "%v", QT_TRANSLATE_NOOP("Placeholders","Original artist"), 28},
        {'i', "%i", QT_TRANSLATE_NOOP("Placeholders","Original lyricist"), 29},
        {'U', "%U", QT_TRANSLATE_NOOP("Placeholders","URL"), 30},
        {'I', "%I", QT_TRANSLATE_NOOP("Placeholders","ISRC"), 31},
        {'b', "%b", QT_TRANSLATE_NOOP("Placeholders","Bitrate"), -2},
        {'l', "%l", QT_TRANSLATE_NOOP("Placeholders","Length"), -2},
        {'L', "%L", QT_TRANSLATE_NOOP("Placeholders","Length in seconds"), -2},
        {'r', "%r", QT_TRANSLATE_NOOP("Placeholders","Sample rate"), -2},
        {'h', "%h", QT_TRANSLATE_NOOP("Placeholders","Number of channels"), -2},
        {'f', "%f", QT_TRANSLATE_NOOP("Placeholders","Filename without extension"), -2},
        {'F', "%F", QT_TRANSLATE_NOOP("Placeholders","Filename with extension"), -2},
        {'E', "%E", QT_TRANSLATE_NOOP("Placeholders","Filename extension"), -2},
        {'z', "%z", QT_TRANSLATE_NOOP("Placeholders","Filename with path and extension"), -2},
        {'Z', "%Z", QT_TRANSLATE_NOOP("Placeholders","File path"), -2},

        {'%', "%%", QT_TRANSLATE_NOOP("Placeholders","Not necessary field"), -3}
    };

    int placeholderToId(const QString &placeholder);
    QString idToPlaceholder(const int id);
    QString getWritablePlaceholders();
    QString getReadOnlyPlaceholders();
    QString getVoidPlaceholder();
    QString placeholderToDescription(const QChar &);
}

#endif // PLACEHOLDERS_H

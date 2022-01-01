#ifndef TAGSREADERWRITER_H
#define TAGSREADERWRITER_H

#include "taglib/taglib.h"
#include <QIcon>
#include "taggingscheme.h"

namespace TagLib {
    class ByteVector;
    class Tag;
    class File;
    namespace MPEG {class File;}
    namespace FLAC {class File;}
    namespace ID3v2 {class Tag; }
    namespace ID3v1 {class Tag; }
    namespace Ogg {class XiphComment; }
    namespace APE {class Tag;}
    namespace MP4 {class Tag;}
    namespace ASF {class Tag;}
}

class FLAC_File;

#include "tagger.h"

class TagsReaderWriter
{
public:
    TagsReaderWriter();
    TagsReaderWriter(Tag *tag);
    void setTag(Tag *tag) {this->tag=tag;}
    void readTags(int tagTypes=TAG_ALL);
    bool writeTags();
private:
//    void readReplayGain(TagLib::ID3v2::Tag *);
//    void readReplayGain(TagLib::APE::Tag *);
    void parseTag(const QString &, const TaggingScheme::TagType , QVariant);
    TagLib::File *readResolver(int tagTypes);
    void readFlacPicture(TagLib::FLAC::File *f);
    void writeFlacPicture(TagLib::FLAC::File *f);

    void readID3v2(TagLib::ID3v2::Tag *);
    void readID3v1(TagLib::ID3v1::Tag *);
    void readXiph(TagLib::Ogg::XiphComment *);
    void readAPE(TagLib::APE::Tag *);
    void writeID3v2(TagLib::ID3v2::Tag *);
    void writeXiph(TagLib::Ogg::XiphComment *, bool opusFile = false);
    void writeAPE(TagLib::APE::Tag *);
#if TAGLIB_MINOR_VERSION >= 6
    void readAsf(TagLib::ASF::Tag *);
    void writeAsf(TagLib::ASF::Tag *);
    void readMP4(TagLib::MP4::Tag *);
    void writeMP4(TagLib::MP4::Tag *);
#endif
    void writeID3v1(TagLib::ID3v1::Tag *);
    void renderTags(const TaggingScheme::TagType, TagLib::Tag *);
    void simplifyTag(int i);

//    void cleanAfterReading(int tagID, QString &value);
    Tag *tag;
    QHash<int, QStringList> rawValues;
    QHash<QString, QStringList> rawUserValues;
};

#endif // TAGSREADERWRITER_H

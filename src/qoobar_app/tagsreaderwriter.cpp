#include "tagsreaderwriter.h"

#include "application.h"
#include "stringroutines.h"

#include <QRegularExpression>

#include "taglib/textidentificationframe.h"
#include "taglib/unsynchronizedlyricsframe.h"
#include "taglib/urllinkframe.h"
#include "taglib/commentsframe.h"
#include "taglib/attachedpictureframe.h"
#include "taglib/popularimeterframe.h"
#include "taglib/ownershipframe.h"
#include "taglib/synchronizedlyricsframe.h"
#include "taglib/mpcfile.h"
#include "taglib/oggflacfile.h"
#include "taglib/trueaudiofile.h"
#include "taglib/speexfile.h"
#include "taglib/wavpackfile.h"
#include "taglib/mpegfile.h"
#include "taglib/vorbisfile.h"

#include "taglib/id3v2tag.h"
#include "taglib/xiphcomment.h"
#include "taglib/id3v1tag.h"
#include "taglib/apetag.h"

#include "taglib/asffile.h"
#include "taglib/wavfile.h"
#include "taglib/aifffile.h"
#include "taglib/mp4file.h"
#include "taglib/privateframe.h"
#include "taglib/mp4coverart.h"
#include "taglib/opusfile.h"
#include "dsffile.h"
#include "dsfproperties.h"

#include "taglib/apefile.h"
#include "taglib/apeproperties.h"
#include "taglib/flacfile.h"
#define APEFILE TagLib::APE::File

#define QS(s) QString::fromStdWString(s.toWString())
#define TS(s) TagLib::String(s.toStdWString())

#define TAGLIB_VERSION (TAGLIB_MAJOR_VERSION * 10000 \
                        + TAGLIB_MINOR_VERSION * 100 \
                        + TAGLIB_PATCH_VERSION)

const char id3v2tags[]=
        "AENC.APIC.ASPI.COMM.COMR.ENCR.EQU2.ETCO.GEOB.GRID."
        "LINK.MCDI.MLLT.OWNE.PRIV.PCNT.POPM.POSS.RBUF.RVA2."
        "RVRB.SEEK.SIGN.SYLT.SYTC.TALB.TBPM.TCOM.TCON.TCOP."
        "TDEN.TDLY.TDOR.TDRC.TDRL.TDTG.TENC.TEXT.TFLT.TIPL."
        "TIT1.TIT2.TIT3.TKEY.TLAN.TLEN.TMCL.TMED.TMOO.TOAL."
        "TOFN.TOLY.TOPE.TOWN.TPE1.TPE2.TPE3.TPE4.TPOS.TPRO."
        "TPUB.TRCK.TRSN.TRSO.TSOA.TSOP.TSOT.TSRC.TSSE.TSST."
        "TXXX.UFID.USER.USLT.WCOM.WCOP.WOAF.WOAR.WOAS.WORS."
        "WPAY.WPUB.WXXX.EQUA.IPLS.RVAD.TDAT.TIME.TORY.TRDA."
        "TSIZ.TYER.TCMP.TCAT.TDES.TGID.WFED";

const char mp4tags[]=
        "trkn.disk.cpil.pgap.pcst.hdvd.tmpo.tvsn.tves.cnID."
        "sfID.atID.geID.stik.rtng.akID.plID.rati.covr.©too."
        "©wrt.©alb.©nam.©ART.cond.subt.©cmt.©gen.©day.aART."
        "©grp.labl.©cpy.cprt.mood.lyrt.©lyr.©enc.rmix.©ope."
        "olyr.©url.ISRC.covr.desc.tven.tves.tvsh.tvsn.rate"
        ;
//TODO: Add support of as many id3v2 tags as possible
//TODO: Add support of all id3v2 tags supported by taglib

static const char *RG_STRING_UPPER[] = {
    "REPLAYGAIN_TRACK_GAIN",
    "REPLAYGAIN_TRACK_PEAK",
    "REPLAYGAIN_TRACK_RANGE",
    "REPLAYGAIN_ALBUM_GAIN",
    "REPLAYGAIN_ALBUM_PEAK",
    "REPLAYGAIN_ALBUM_RANGE",
    "REPLAYGAIN_REFERENCE_LOUDNESS",
    "MP3GAIN_MINMAX",
    "MP3GAIN_ALBUM_MINMAX",
    "MP3GAIN_UNDO"
};
static const char *RG_STRING_LOWER[] = {
    "replaygain_track_gain",
    "replaygain_track_peak",
    "replaygain_track_range",
    "replaygain_album_gain",
    "replaygain_album_peak",
    "replaygain_album_range",
    "replaygain_reference_loudness",
    "mp3gain_minmax",
    "mp3gain_album_minmax",
    "mp3gain_undo"
};

enum RG_ENUM {
    RG_TRACK_GAIN,
    RG_TRACK_PEAK,
    RG_TRACK_RANGE,
    RG_ALBUM_GAIN,
    RG_ALBUM_PEAK,
    RG_ALBUM_RANGE,
    RG_REFERENCE_LOUDNESS
};

#include <QStringList>
#include <QTextCodec>
#include <QLibrary>
#include <QDir>
#include <QtDebug>
#include <qmath.h>

#include "enums.h"

//replaces all \r\n by \n
void simplifyLineEnding(QString &line)
{
    line.replace("\r\n","\n");
    line.replace('\r','\n');
}

void adjustLineEnding(QString &line)
{
    switch (App->id3v2LineEnding) {
    //we assume that line only contains \n line endings
    case 0: {//CRLF
        line.replace("\n","\r\n");
        break;
    }
    case 1: {//LF
        break;
    }
    case 2: {//CR
        line.replace('\n','\r');
        break;
    }
    }
}

TagsReaderWriter::TagsReaderWriter(): tag(0)
{DD;
}

TagsReaderWriter::TagsReaderWriter(Tag *tag): tag(tag)
{DD;
}

QString peak(int v)
{DD;
    return QString::number((float)(qPow(10.f,(float)v / 5120.f)/32768.f),'f',6);
}
QString gain(int v)
{DD;
    return QString("%2 dB").arg(v == 0 ? 0.f : 64.82f - float(v) / 256.f,0,'f',2);
}

void TagsReaderWriter::readTags(int tagTypes)
{DD;
    tag->clear();
    TagLib::File *f=readResolver(tagTypes);
    if (f) {
        TagLib::AudioProperties  *properties = f->audioProperties();
        if (properties) {
            tag->d->length = properties->length();
            tag->d->bitrate = QString::number(properties->bitrate());
            tag->d->sampleRate = properties->sampleRate();
            tag->d->channels = properties->channels();
            TagLib::MPC::Properties *p=dynamic_cast<TagLib::MPC::Properties *>(properties);
            if (p) {
                if (p->trackPeak()!=0)
                    tag->d->replayGainInfo.trackPeak = peak(p->trackPeak());
                if (p->trackGain()!=0)
                    tag->d->replayGainInfo.trackGain = gain(p->trackGain());
                if (p->albumPeak()!=0)
                    tag->d->replayGainInfo.albumPeak = peak(p->albumPeak());
                if (p->albumGain()!=0)
                    tag->d->replayGainInfo.albumGain = gain(p->albumGain());
            }
        }
        delete f;
    }
    QHashIterator<int, QStringList> i(rawValues);
    while (i.hasNext()) {
        i.next();
        if (i.key() >= tag->d->tags.size()) break;
        QStringList l = i.value();
        l.removeDuplicates();
        tag->d->tags[i.key()] = l.join(";");
    }
    QHashIterator<QString, QStringList> ii(rawUserValues);
    while (ii.hasNext()) {
        ii.next();
        QStringList l = ii.value();
        l.removeDuplicates();
        tag->d->otherTags[ii.key()] = l.join(";");
    }

    static int intTags[] = {YEAR, TRACKNUMBER, TOTALTRACKS, RATING, DISCNUMBER, TOTALDISCS, TEMPO};
    for (int j=0; j<7; ++j) {
        if (tag->tag(intTags[j])=="0")
            tag->d->tags[intTags[j]] = QLS("");
    }
}

TagLib::File *TagsReaderWriter::readResolver(int tagTypes)
{DD;
    switch (tag->fileType()) {
        case Tag::MP3_FILE: {
            TagLib::MPEG::File *f=new TagLib::MPEG::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (f->hasID3v1Tag()) tag->d->tagTypes |= TAG_ID3V1;
                if (f->hasID3v2Tag()) tag->d->tagTypes |= TAG_ID3V2;
                if (f->hasAPETag()) tag->d->tagTypes |= TAG_APE;

                if (tagTypes<TAG_ALL) {//some of tags to read, not affected by app settings
                    if (tagTypes & TAG_APE)   readAPE(f->APETag());
                    if (tagTypes & TAG_ID3V2) readID3v2(f->ID3v2Tag());
                    if (tagTypes & TAG_ID3V1) readID3v1(f->ID3v1Tag());
                    return f;
                }
                if (App->mp3readape) readAPE(f->APETag());
                if (App->mp3readid3) readID3v2(f->ID3v2Tag());
                readID3v1(f->ID3v1Tag());
            }
            return f;
        }
        case Tag::DSF_FILE: {
            TagLib::DSF::File *f=new TagLib::DSF::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (!f->tag()->isEmpty()) tag->d->tagTypes |= TAG_ID3V2;
                if (tagTypes & TAG_ID3V2) readID3v2(f->tag());
            }
            return f;
        }
        case Tag::TTA_FILE: {
            TagLib::TrueAudio::File *f=new TagLib::TrueAudio::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (f->hasID3v1Tag()) tag->d->tagTypes |= TAG_ID3V1;
                if (f->hasID3v2Tag()) tag->d->tagTypes |= TAG_ID3V2;
                if (tagTypes & TAG_ID3V2) readID3v2(f->ID3v2Tag(false));
                if (tagTypes & TAG_ID3V1) readID3v1(f->ID3v1Tag());
            }
            return f;
        }
        case Tag::OGG_FILE: {
            TagLib::Ogg::Vorbis::File *f=new TagLib::Ogg::Vorbis::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (!f->tag()->isEmpty()) tag->d->tagTypes |= TAG_VORBIS;
                if (tagTypes & TAG_VORBIS) readXiph(f->tag());
            }
            return f;
        }
        case Tag::FLAC_FILE: {
            TagLib::FLAC::File *f=new TagLib::FLAC::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (f->hasID3v1Tag()) tag->d->tagTypes |= TAG_ID3V1;
                if (f->hasID3v2Tag()) tag->d->tagTypes |= TAG_ID3V2;
                if (f->hasXiphComment()) tag->d->tagTypes |= TAG_VORBIS;
//                if (App->flacreadid3) readID3v2(f->ID3v2Tag(false));
//                if (App->flacreadogg)
                {
                    readXiph(f->xiphComment(false));
                    readFlacPicture(f);
                }
                readID3v1(f->ID3v1Tag());
            }
            return f;
        }
        case Tag::OGA_FILE: {
            TagLib::Ogg::FLAC::File *f=new TagLib::Ogg::FLAC::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (f->hasXiphComment()) tag->d->tagTypes |= TAG_VORBIS;
                if (tagTypes & TAG_VORBIS)
                    readXiph(f->tag());
                return f;
            }
            delete f;

            TagLib::Ogg::Vorbis::File *ff=new TagLib::Ogg::Vorbis::File(FILE_NAME(tag->fullFileName()));
            if (ff->isValid()) {
                if (!ff->tag()->isEmpty()) tag->d->tagTypes |= TAG_VORBIS;
                if (tagTypes & TAG_VORBIS)
                    readXiph(ff->tag());
            }
            return ff;
        }
        case Tag::SPX_FILE: {
            TagLib::Ogg::Speex::File *f=new TagLib::Ogg::Speex::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid() && (tagTypes & TAG_VORBIS)) readXiph(f->tag());
            if (!f->tag()->isEmpty()) tag->d->tagTypes |= TAG_VORBIS;
            return f;
        }
        case Tag::OPUS_FILE: {
            TagLib::Ogg::Opus::File *f=new TagLib::Ogg::Opus::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid() && (tagTypes & TAG_VORBIS)) readXiph(f->tag());
            if (!f->tag()->isEmpty()) tag->d->tagTypes |= TAG_VORBIS;
            return f;
        }
        case Tag::MPC_FILE: {
            TagLib::MPC::File *f=new TagLib::MPC::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (f->hasID3v1Tag()) tag->d->tagTypes |= TAG_ID3V1;
                if (f->hasAPETag()) tag->d->tagTypes |= TAG_APE;
                if (tagTypes & TAG_APE) readAPE(f->APETag(false));
                if (tagTypes & TAG_ID3V1) readID3v1(f->ID3v1Tag());
            }
            return f;
        }
        case Tag::WV_FILE: {
            TagLib::WavPack::File *f=new TagLib::WavPack::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (f->hasID3v1Tag()) tag->d->tagTypes |= TAG_ID3V1;
                if (f->hasAPETag()) tag->d->tagTypes |= TAG_APE;
                if (tagTypes & TAG_APE) readAPE(f->APETag(false));
                if (tagTypes & TAG_ID3V1) readID3v1(f->ID3v1Tag());
            }
            return f;
        }
        case Tag::APE_FILE: {
            APEFILE *f = new APEFILE(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (f->hasID3v1Tag()) tag->d->tagTypes |= TAG_ID3V1;
                if (f->hasAPETag()) tag->d->tagTypes |= TAG_APE;
                if (tagTypes & TAG_APE) readAPE(f->APETag(false));
                if (tagTypes & TAG_ID3V1) readID3v1(f->ID3v1Tag());
            }
            return f;
        }

        case Tag::WAV_FILE: {
            TagLib::RIFF::WAV::File *f = new TagLib::RIFF::WAV::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                if (f->hasID3v2Tag()) tag->d->tagTypes |= TAG_ID3V2;
//                if (f->hasInfoTag())
                if ((tagTypes & TAG_ID3V2) && f->tag())
                    readID3v2(f->tag());
            }
            return f;
        }
        case Tag::AIFF_FILE: {
            TagLib::RIFF::AIFF::File *f = new TagLib::RIFF::AIFF::File(FILE_NAME(tag->fullFileName()));
            if (f->tag()) {
                if (f->isValid() && (tagTypes & TAG_ID3V2) )
                    readID3v2(f->tag());
                if (f->hasID3v2Tag()) tag->d->tagTypes |= TAG_ID3V2;
            }
            return f;
        }

        case Tag::WMA_FILE: {
            TagLib::ASF::File *f=new TagLib::ASF::File(FILE_NAME(tag->fullFileName()));
            if (f->tag()) {
                if (f->isValid() && (tagTypes & TAG_ASF) )
                    readAsf(f->tag());
                if (!f->tag()->isEmpty()) tag->d->tagTypes |= TAG_ASF;
            }
            return f;
        }

        case Tag::M4A_FILE: {
            TagLib::MP4::File *f=new TagLib::MP4::File(FILE_NAME(tag->fullFileName()));
            if (!f) return nullptr;
            if (f->tag()) {
                if (!f->tag()->isEmpty()) tag->d->tagTypes |= TAG_MP4;
                if (f->isValid() && (tagTypes & TAG_MP4) )
                    readMP4(f->tag());
            }
            return f;
        }
        default: return 0;
    }
    return 0;
}

QStringList handleFrameList(const TagLib::ID3v2::FrameList &frames)
{DD;
    QStringList result;
    for (auto it = frames.begin(); it!=frames.end(); ++it) {
        TagLib::String s=(*it)->toString();
        result.append(QS(s));
    }
    return result;
}

void TagsReaderWriter::readID3v2(TagLib::ID3v2::Tag *id3v2tag)
{DD;
    if (!id3v2tag) return;

    const TagLib::ID3v2::FrameListMap &map = id3v2tag->frameListMap();

    for (auto it = map.begin(); it != map.end(); ++it)  {
        QString id = QS(TagLib::String((*it).first, TagLib::String::Latin1));

        if (id=="COMM") {
            for (unsigned int i=0; i<(*it).second.size(); ++i) {
                TagLib::ID3v2::CommentsFrame *frame=
                        dynamic_cast<TagLib::ID3v2::CommentsFrame *>((*it).second[i]);
                if (!frame) continue;
                parseTag(QString("COMM:%1").arg(QS(frame->description())), TaggingScheme::ID3, QS(frame->text()));
            }
        }
        else if (id=="PRIV") {
            for (unsigned int i=0; i<(*it).second.size(); ++i) {
                TagLib::ID3v2::PrivateFrame *frame=
                        dynamic_cast<TagLib::ID3v2::PrivateFrame *>((*it).second[i]);
                if (!frame) continue;

                QString owner = QS(frame->owner());
                //we skip all PRIV frames that we do not support
                //it is a bad decision, but I do not know how to
                //define whether PRIV frame contains text and not binary data
                if (owner == "WM/Mood") {
                    TagLib::String ts=TagLib::String(frame->data(),TagLib::String::UTF16LE);
                    parseTag(QString("PRIV:%1").arg(owner),TaggingScheme::ID3,QS(ts));
                }
            }
        }
        else if (id=="APIC") {
            if ((*it).second.size()>0) {
                TagLib::ID3v2::AttachedPictureFrame *frame=
                        dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>((*it).second[0]);
                if (!frame) continue;
                TagLib::ByteVector data=frame->picture();
                tag->d->image.setType(frame->type());
                tag->d->image.setMimetype(QS(frame->mimeType()));
                tag->d->image.setDescription(QS(frame->description()));
                tag->d->image.setPixmap(QByteArray(data.data(),data.size()));
            }
        }
        else if (id=="POPM") {
            if ((*it).second.size()>0) {
                TagLib::ID3v2::PopularimeterFrame *frame=
                        dynamic_cast<TagLib::ID3v2::PopularimeterFrame *>((*it).second[0]);
                if (!frame) continue;
                int rating_=frame->rating();
                if (rating_>0) {
                    rating_ = (rating_-19)/59+1;
                    parseTag(QSL("POPM"),TaggingScheme::ID3,rating_);
                }
            }
        }
        else if (id=="TCON")
            parseTag(id,TaggingScheme::ID3,QS(id3v2tag->genre()));
        else if (id=="TXXX") {
            QStringList values=handleFrameList((*it).second);
            Q_FOREACH (const QString &item, values) {
                QString idd = item.mid(1,item.indexOf(']')-1);
                parseTag(QString("%1:%2").arg(id).arg(idd),TaggingScheme::ID3,
                         item.right(item.length()-idd.length()*2-4));
            }
        }
        else if (id=="WXXX") {
            QStringList values=handleFrameList((*it).second);
            Q_FOREACH (const QString &item, values) {
                QString idd = item.mid(1,item.indexOf(']')-1);
                parseTag(QString("%1:%2").arg(id).arg(idd),TaggingScheme::ID3,
                         item.right(item.length()-idd.length()*2-3));
            }
        }
        else if (id=="OWNE" || id.startsWith(QLS("T")) || id.startsWith(QLS("W"))) {
            QStringList values = handleFrameList((*it).second);
            parseTag(id, TaggingScheme::ID3,values.join(QSL(";")));
        }
        else if (id=="USLT") {//unsynchronized lyrics frame
            QStringList values = handleFrameList((*it).second);
            parseTag(id,TaggingScheme::ID3,values);
        }
        else if (id=="SYLT") {//synchronized lyrics frame
            QStringList values; //qDebug()<<"frames SYLT count"<<(*it).second.size();
            for (unsigned int i=0; i<(*it).second.size(); ++i) {
                auto frame= dynamic_cast<TagLib::ID3v2::SynchronizedLyricsFrame *>((*it).second[i]);
                if (!frame) continue;

                TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList synchedText = frame->synchedText();
                //qDebug()<<"lines count"<<synchedText.size();
                for (uint j=0; j<synchedText.size(); ++j) {
                    //qDebug()<<"line"<<j+1<<":"<<synchedText[j].time<<QS(synchedText[j].text);
                    QString s=QS(synchedText[j].text);
                    if (!s.isEmpty() && s[0]=='\n')
                        s.remove(0,1);
                    values.append(QString("%1: %2").arg(synchedText[j].time).arg(s));
                }
            }
            //qDebug()<<values;
            parseTag(id,TaggingScheme::ID3,values);
        }
    }
}

void writeID3v2Frame(TagLib::ID3v2::Tag *id3v2tag,const QString &s,const QString &key)
{DD;
    // f.e. id = "COMM" in "COMM:<description>"
    TagLib::ByteVector id = TagLib::ByteVector(key.toLatin1().data()).mid(0,4);
    QString description = key.mid(5);

    if (!QString(id3v2tags).contains(key.mid(0,4))) {
        //user text tags, write into TXXX frame
        id = "TXXX";
        description = key;
    }

    if (id=="COMM") { // comment
        QString comment1=s;

        if (description=="MusicMatch_Preference") {
            if (comment1=="1") comment1=QSL("Poor");
            else if (comment1=="2") comment1=QSL("Fair");
            else if (comment1=="3") comment1=QSL("Good");
            else if (comment1=="4") comment1=QSL("Very Good");
            else if (comment1=="5") comment1=QSL("Excellent");
        }
        else {
            adjustLineEnding(comment1);
        }
        //first we need to find comments
        TagLib::ID3v2::CommentsFrame *frame = TagLib::ID3v2::CommentsFrame::findByDescription(id3v2tag, TS(description));
        if (frame) {
            if (comment1.isEmpty()) id3v2tag->removeFrame(frame,true);
            else frame->setText(TS(comment1));
        }
        else if (!comment1.isEmpty()) {//create new
            frame = new TagLib::ID3v2::CommentsFrame(TagLib::String::UTF8);
            frame->setDescription(TS(description));
            //new frames get default language
            frame->setLanguage("eng");
            frame->setText(TS(comment1));
            id3v2tag->addFrame(frame);
        }
    }
    else if (id=="TXXX") {
        TagLib::ID3v2::UserTextIdentificationFrame *frame =
                TagLib::ID3v2::UserTextIdentificationFrame::find(id3v2tag, TS(description));
        if (frame)
            if (s.isEmpty()) id3v2tag->removeFrame(frame,true);
            else frame->setText(TS(s));
        else if (!s.isEmpty()) {
            frame = new TagLib::ID3v2::UserTextIdentificationFrame(TagLib::String::UTF8);
            frame->setDescription(TS(description));
            frame->setText(TS(s));
            id3v2tag->addFrame(frame);
        }
    }
    else if (id=="WXXX") {
        TagLib::ID3v2::UserUrlLinkFrame *frame =
                TagLib::ID3v2::UserUrlLinkFrame::find(id3v2tag, TS(description));
        if (frame)
            if (s.isEmpty()) id3v2tag->removeFrame(frame,true);
            else frame->setText(TS(s));
        else if (!s.isEmpty()) {
            frame = new TagLib::ID3v2::UserUrlLinkFrame(TagLib::String::Latin1);
            frame->setDescription(TS(description));
            frame->setUrl(TS(s));
            id3v2tag->addFrame(frame);
        }
    }
    else if (id=="PRIV") {
        TagLib::String data=TS(s);

        TagLib::ID3v2::PrivateFrame *frame = 0;

        auto l = id3v2tag->frameList("PRIV");
        for(auto it = l.begin(); it != l.end(); ++it) {
            auto f = dynamic_cast<TagLib::ID3v2::PrivateFrame *>(*it);
            if (f && f->owner() == TS(description))
                frame = f;
        }

        if (frame)
            if (s.isEmpty()) id3v2tag->removeFrame(frame,true);
            else frame->setData(data.data(TagLib::String::UTF16LE));
        else if (!s.isEmpty()) {
            frame = new TagLib::ID3v2::PrivateFrame();
            frame->setOwner(TS(description));
            frame->setData(data.data(TagLib::String::UTF16LE));
            id3v2tag->addFrame(frame);
        }
    }
    else if (id=="USLT") {
        id3v2tag->removeFrames(id);
        if (!s.isEmpty()) {
            auto frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame(TagLib::String::UTF8);
            QString s1=s;
            adjustLineEnding(s1);
            frame->setText(TS(s1));
            frame->setLanguage("eng");
            frame->setDescription("");
            id3v2tag->addFrame(frame);
        }
    }
    else if (id=="SYLT") {
        id3v2tag->removeFrames(id);
        if (!s.isEmpty()) {
            TagLib::ID3v2::SynchronizedLyricsFrame *frame =
                    new TagLib::ID3v2::SynchronizedLyricsFrame(TagLib::String::UTF8);
            frame->setType(TagLib::ID3v2::SynchronizedLyricsFrame::Other);
            frame->setTimestampFormat(TagLib::ID3v2::SynchronizedLyricsFrame::AbsoluteMilliseconds);
            frame->setLanguage("eng");
            frame->setDescription("");
            TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList list;
            QStringList l = s.split("\n");
            Q_FOREACH(const QString &line, l) {
                QString ss=line.section(": ",1); if(!ss.startsWith('\n')) ss.prepend('\n');
                list.append(TagLib::ID3v2::SynchronizedLyricsFrame::SynchedText(line.section(": ",0,0).toUInt(),
                                                                                TS(ss)));
            }
            frame->setSynchedText(list);
            id3v2tag->addFrame(frame);
        }
    }
    else if (id=="POPM") {
        id3v2tag->removeFrames(id);
        int rating_ = s.toInt();
        switch (rating_) {
            case 1: rating_ = 19; break;
            case 2: rating_ = 78; break;
            case 3: rating_ = 137; break;
            case 4: rating_ = 196; break;
            case 5: rating_ = 255; break;
            default: rating_ = 0; break;
        }
        if (rating_>0) {
            TagLib::ID3v2::PopularimeterFrame *popm=new TagLib::ID3v2::PopularimeterFrame();
            popm->setEmail(TagLib::String("Windows Media Player 9 Series",TagLib::String::Latin1));
            popm->setRating(rating_);
            popm->setCounter(0);
            id3v2tag->addFrame(popm);
        }
    }
    else if (id=="OWNE") {
        id3v2tag->removeFrames(id);
        if (!s.isEmpty()) {
            TagLib::ID3v2::OwnershipFrame *owne = new TagLib::ID3v2::OwnershipFrame();
            QRegularExpression price("pricePaid=(\\w+)\\s+");
            QRegularExpressionMatch match;
            int index = s.indexOf(price, 0, &match);
            if (index >= 0)
                owne->setPricePaid(TS(match.captured(1)));

            QRegularExpression date("datePurchased=(\\w+)");
            index = s.indexOf(date, index<0?0:index, &match);
            if (index >= 0)
                owne->setDatePurchased(TS(match.captured(1)));

            QRegularExpression seller("seller=([\\w\\s]*)");
            index = s.indexOf(seller, index<0?0:index, &match);
            if (index >= 0)
                owne->setSeller(TS(match.captured(1)));
            id3v2tag->addFrame(owne);
        }
    }
    else if (id.startsWith("W")) {
        id3v2tag->removeFrames(id);
        if (!s.isEmpty()) {
            TagLib::ID3v2::UrlLinkFrame *frame = new TagLib::ID3v2::UrlLinkFrame(id);
            frame->setUrl(TS(s));
            id3v2tag->addFrame(frame);
        }
    }
    else if (id.startsWith("T")) {
        id3v2tag->removeFrames(id);
        if (!s.isEmpty()) {
            TagLib::ID3v2::TextIdentificationFrame *frame = new TagLib::ID3v2::TextIdentificationFrame(id,TagLib::String::UTF8);
            QString s1=s;
            if (id=="TCOM") adjustLineEnding(s1);
            frame->setText(TS(s1));
            id3v2tag->addFrame(frame);
        }
    }
    //remove empty frames but do not touch non-empty ones
    else if (s.isEmpty()) id3v2tag->removeFrames(id);
}

void TagsReaderWriter::writeID3v2(TagLib::ID3v2::Tag *id3v2tag)
{DD;
    if (!id3v2tag) return;

    renderTags(TaggingScheme::ID3, id3v2tag);

    id3v2tag->setGenre(TS(tag->tag(GENRE)));

    id3v2tag->removeFrames("APIC");
    const CoverImage &image=tag->image();
    if (!image.pixmap().isEmpty()) {
        TagLib::ID3v2::AttachedPictureFrame *frame=
                new TagLib::ID3v2::AttachedPictureFrame();
        frame->setMimeType(TS(image.mimetype()));
        frame->setType((TagLib::ID3v2::AttachedPictureFrame::Type)image.type());
        frame->setDescription(TS(image.description()));
        frame->setPicture(TagLib::ByteVector(image.pixmap().data(),(unsigned int)image.pixmap().size()));
        id3v2tag->addFrame(frame);
    }

    //other tags
    const QMap<QString,QString> &otherTags = tag->userTags();
    QMap<QString,QString>::const_iterator i = otherTags.constBegin();
    for (; i != otherTags.constEnd(); ++i) {
        writeID3v2Frame(id3v2tag,i.value(),i.key());
    }

    //replaygain
    //first remove all old tags
    TagLib::ID3v2::FrameList::Iterator it;
    TagLib::ID3v2::FrameList frames = id3v2tag->frameList("TXXX");

    for (it = frames.begin(); it != frames.end(); ++it) {
        auto frame = dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);

        // this removes all variants of upper-/lower-/mixed-case tags
        if (frame && frame -> fieldList().size() >= 2) {
            TagLib::String desc = frame -> description().upper();
            for (unsigned i=0; i< sizeof(RG_STRING_UPPER); ++i) {
                if (desc == RG_STRING_UPPER[i]) id3v2tag->removeFrame(frame);
            }
        }
    }

    const char **RG_STRING = RG_STRING_UPPER;

    if (App->replaygainOptions.tagsCase == 1) {
        RG_STRING = RG_STRING_LOWER;
    }

    writeID3v2Frame(id3v2tag,tag->replayGainInfo().trackGain, RG_STRING[RG_TRACK_GAIN]);
    writeID3v2Frame(id3v2tag,tag->replayGainInfo().trackPeak, RG_STRING[RG_TRACK_PEAK]);
    writeID3v2Frame(id3v2tag,tag->replayGainInfo().albumGain, RG_STRING[RG_ALBUM_GAIN]);
    writeID3v2Frame(id3v2tag,tag->replayGainInfo().albumPeak, RG_STRING[RG_ALBUM_PEAK]);
    writeID3v2Frame(id3v2tag,tag->replayGainInfo().trackMinMax, RG_STRING[RG_TRACK_RANGE]);
    writeID3v2Frame(id3v2tag,tag->replayGainInfo().albumMinMax, RG_STRING[RG_ALBUM_RANGE]);
    writeID3v2Frame(id3v2tag,tag->replayGainInfo().loudness, RG_STRING[RG_REFERENCE_LOUDNESS]);
}

void TagsReaderWriter::parseTag(const QString &id, const TaggingScheme::TagType tagType, QVariant value)
{DD;
    if (!value.isValid()) return;
    QString stringValue = value.toString();
    if (stringValue.isEmpty()) return;

    if (id.startsWith("COMM")) {
        //hack for id3v2.4 COMM:MusicMatch_Preference
        if (id=="COMM:MusicMatch_Preference") {
            if (stringValue=="Poor") stringValue=QSL("1");
            else if (stringValue=="Fair") stringValue=QSL("2");
            else if (stringValue=="Good") stringValue=QSL("3");
            else if (stringValue=="Very Good") stringValue=QSL("4");
            else if (stringValue=="Excellent") stringValue=QSL("5");
        }
        else
            simplifyLineEnding(stringValue);
    }

    if (id=="TXXX:VideoKind" || id=="VIDEOKIND" || id=="stik" || id=="WM/VideoKind") {
        bool ok;
        int v = value.toInt(&ok);
        if (ok) {
            switch (v) {
                case 0: stringValue = QSL("Movie"); break;
                case 1: stringValue = QSL("Normal"); break;
                case 2: stringValue = QSL("Audiobook"); break;
                case 5: stringValue = QSL("Whacked Bookmark"); break;
                case 6: stringValue = QSL("Music Video"); break;
                case 9: stringValue = QSL("Short Film"); break;
                case 10: stringValue = QSL("TV Show"); break;
                case 11: stringValue = QSL("Booklet"); break;
                case 14: stringValue = QSL("Ringtone"); break;
                default: stringValue = QSL("Unknown");
            }
        }
    }

    if (id.contains(QSL("replaygain_album_gain"),Qt::CaseInsensitive)) {
        tag->setAlbumGain(stringValue, false);
        return;
    }
    if (id.contains(QSL("R128_TRACK_GAIN"),Qt::CaseInsensitive)) {
        tag->setTrackGain(stringValue, false);
        return;
    }
    if (id.contains(QSL("R128_ALBUM_GAIN"),Qt::CaseInsensitive)) {
        tag->setAlbumGain(stringValue, false);
        return;
    }
    if (id.contains(QSL("replaygain_album_peak"),Qt::CaseInsensitive))  {
        tag->setAlbumPeak(stringValue, false);
        return;
    }
    if (id.contains(QSL("replaygain_track_gain"),Qt::CaseInsensitive))  {
        tag->setTrackGain(stringValue, false);
        return;
    }
    if (id.contains(QSL("replaygain_track_peak"),Qt::CaseInsensitive))  {
        tag->setTrackPeak(stringValue, false);
        return;
    }
    if (id.contains(QSL("MP3GAIN_ALBUM_MINMAX"),Qt::CaseInsensitive)
            || id.contains(QSL("replaygain_album_range"),Qt::CaseInsensitive))  {
        tag->setAlbumMinMax(stringValue, false);
        return;
    }
    if (id.contains(QSL("MP3GAIN_MINMAX"),Qt::CaseInsensitive)
            || id.contains(QSL("replaygain_track_range"),Qt::CaseInsensitive))  {
        tag->setTrackMinMax(stringValue, false);
        return;
    }
    if (id.contains(QSL("replaygain_album_minmax"),Qt::CaseInsensitive)) {
        tag->setAlbumMinMax(stringValue, false);
        return;
    }
    if (id.contains(QSL("replaygain_track_minmax"),Qt::CaseInsensitive)) {
        tag->setTrackMinMax(stringValue, false);
        return;
    }
    if (id.contains(QSL("replaygain_minmax"),Qt::CaseInsensitive)) {
        tag->setTrackMinMax(stringValue, false);
        return;
    }
    if (id.contains(QSL("replaygain_reference_loudness"),Qt::CaseInsensitive)) {
        tag->setLoudness(stringValue, false);
        return;
    }
    if (id.contains(QSL("MP3GAIN_UNDO"),Qt::CaseInsensitive)) {
        tag->setReplayGainUndo(stringValue, false);
        return;
    }

    if (id=="USLT" || id=="SYLT") {
        //try optimizing lines with \r, \r\n or \n
        QStringList values = value.toStringList();
        for (QString &s: values) simplifyLineEnding(s);

        stringValue = values.join("\n");
    }

    QList<int> idPosition = App->currentScheme->search(tagType, id);
    if (!idPosition.isEmpty()) {
        QStringList l;
        if (idPosition.size() == 1) {
            l << stringValue;
        }
        else {
            // probably tracknumber/totaltracks or discnumber/totaldiscs
            l = stringValue.split(QChar('/'));
        }
        for (int i=0; i<l.size() && i<idPosition.size(); ++i) {
            if (!l.at(i).isEmpty())
                rawValues[idPosition.at(i)].append(l.at(i));
        }
//        qDebug()<<"reading"<<id<<idPosition<<l;
    }
    else {
        rawUserValues[id].append(stringValue);
    }
}

void TagsReaderWriter::readID3v1(TagLib::ID3v1::Tag *id3v1tag)
{DD;
    if (!id3v1tag) return;

    if (tag->tag(ALBUM).isEmpty())
        tag->d->tags[ALBUM] = QS(id3v1tag->album());
    if (tag->tag(ARTIST).isEmpty())
        tag->d->tags[ARTIST] = QS(id3v1tag->artist());
    if (tag->tag(TITLE).isEmpty())
        tag->d->tags[TITLE] = QS(id3v1tag->title());
    if (tag->tag(GENRE).isEmpty())
        tag->d->tags[GENRE] = QS(id3v1tag->genre());
    if (tag->tag(COMMENT).isEmpty())
        tag->d->tags[COMMENT] = QS(id3v1tag->comment());
    if (tag->tag(YEAR).isEmpty() || tag->tag(YEAR)=="0")
        tag->d->tags[YEAR] = QString::number(id3v1tag->year());
    if (tag->tag(TRACKNUMBER).isEmpty() || tag->tag(TRACKNUMBER)=="0")
        tag->d->tags[TRACKNUMBER] = QString::number(id3v1tag->track());
}

void TagsReaderWriter::writeID3v1(TagLib::ID3v1::Tag *id3v1tag)
{DD;
    if (!id3v1tag) return;

    if (App->id3v1Transliterate) {
        id3v1tag->setAlbum(TS(transliterate(tag->tag(ALBUM))));
        id3v1tag->setArtist(TS(transliterate(tag->tag(ARTIST))));
        id3v1tag->setComment(TS(transliterate(tag->tag(COMMENT))));
        id3v1tag->setGenre(TS(transliterate(tag->tag(GENRE))));
        id3v1tag->setTitle(TS(transliterate(tag->tag(TITLE))));
    }
    else {
        id3v1tag->setAlbum(TS(tag->tag(ALBUM)));
        id3v1tag->setArtist(TS(tag->tag(ARTIST)));
        id3v1tag->setComment(TS(tag->tag(COMMENT)));
        id3v1tag->setGenre(TS(tag->tag(GENRE)));
        id3v1tag->setTitle(TS(tag->tag(TITLE)));
    }
    id3v1tag->setTrack(tag->tag(TRACKNUMBER).toInt());
    id3v1tag->setYear(tag->tag(YEAR).toInt());
}

bool isFrontCover(TagLib::FLAC::Picture *p)
{DD;
    return p->type()==TagLib::FLAC::Picture::FrontCover;
}

void TagsReaderWriter::readFlacPicture(TagLib::FLAC::File *f)
{DD;
    TagLib::List<TagLib::FLAC::Picture *> pictures = f->pictureList();
    if (pictures.isEmpty()) return;

    TagLib::List<TagLib::FLAC::Picture *>::ConstIterator it = find_if(pictures.begin(),pictures.end(),isFrontCover);
    if (it==pictures.end())  it=pictures.begin();

    tag->d->image.setType((*it)->type());
    tag->d->image.setMimetype(QS((*it)->mimeType()));
    tag->d->image.setDescription(QS((*it)->description()));
    tag->d->image.setPixmap(QByteArray((*it)->data().data(),(*it)->data().size()));
}

TagLib::FLAC::Picture *createFlacPicture(const CoverImage &image)
{DD;
    TagLib::FLAC::Picture *picture = new TagLib::FLAC::Picture();
    QImage img;
    if (img.loadFromData(image.pixmap())) {
        picture->setData(TagLib::ByteVector(image.pixmap().data(),(unsigned int)image.pixmap().size()));
        picture->setDescription(TS(image.description()));
        picture->setMimeType(TS(image.mimetype()));
        picture->setType((TagLib::FLAC::Picture::Type)image.type());
        picture->setColorDepth(img.depth());
        picture->setHeight(img.height());
        picture->setWidth(img.width());
        picture->setNumColors(img.colorCount());
    }
    return picture;
}

void TagsReaderWriter::writeFlacPicture(TagLib::FLAC::File *f)
{DD;
    f->removePictures();
    if (!tag->image().pixmap().isEmpty()) {
        TagLib::FLAC::Picture *picture = createFlacPicture(tag->image());
        if (picture) f->addPicture(picture);
    }
}

void TagsReaderWriter::readXiph(TagLib::Ogg::XiphComment *xiph)
{DD;
    if (!xiph) return;

    const TagLib::Ogg::FieldListMap &map=xiph->fieldListMap();

    for (TagLib::Ogg::FieldListMap::ConstIterator it = map.begin(); it != map.end(); ++it)  {
        QString id=QS((*it).first).toUpper();
        QString value=QS((*it).second.toString(";"));
        if (id=="METADATA_BLOCK_PICTURE") {
            QByteArray rawData = QByteArray::fromBase64(value.toLatin1());
            TagLib::ByteVector data=TagLib::ByteVector(rawData.data(),rawData.length());
            TagLib::FLAC::Picture pic(data);
            tag->d->image.setDescription(QS(pic.description()));
            tag->d->image.setType(pic.type());
            tag->d->image.setMimetype(QS(pic.mimeType()));
            tag->d->image.setPixmap(QByteArray(pic.data().data(),pic.data().size()));
        }
        else if (id=="COVERARTMIME" && tag->fileType()!=Tag::FLAC_FILE)
            tag->d->image.setMimetype(value);
        else if (id=="COVERART" && tag->image().pixmap().isNull())
            tag->d->image.setPixmap(QByteArray::fromBase64(value.toLatin1()));
        else if (id=="COVERARTTYPE" && tag->fileType()!=Tag::FLAC_FILE)
            tag->d->image.setType(value.toInt());
        else if (id=="COVERARTDESCRIPTION" && tag->fileType()!=Tag::FLAC_FILE)
            tag->d->image.setDescription(value);
        else parseTag(id,TaggingScheme::VORBIS,value);
    }
}

void TagsReaderWriter::writeXiph(TagLib::Ogg::XiphComment *xiph, bool opusFile)
{DD;
    if (!xiph) return;
    renderTags(TaggingScheme::VORBIS, xiph);

    xiph->removeFields("COVERART");
    xiph->removeFields("COVERARTMIME");
    xiph->removeFields("COVERARTDESCRIPTION");
    xiph->removeFields("COVERARTTYPE");
    xiph->removeFields("METADATA_BLOCK_PICTURE");

    if (tag->fileType()!=Tag::FLAC_FILE && !tag->image().pixmap().isEmpty()) {
        if (App->oggPictureNew) {
            TagLib::FLAC::Picture *picture = createFlacPicture(tag->image());
            if (picture) {
                TagLib::ByteVector bv = picture->render();
                QByteArray result = QByteArray(bv.data(),bv.size());
                xiph->addField("METADATA_BLOCK_PICTURE",TS(QString(result.toBase64())),true);
            }
        }
        else {
            xiph->addField("COVERARTMIME",TS(tag->image().mimetype()),true);
            xiph->addField("COVERART",TS(QString(tag->image().pixmap().toBase64())),true);
            xiph->addField("COVERARTTYPE",TagLib::String::number(tag->image().type()),true);
            xiph->addField("COVERARTDESCRIPTION",TS(tag->image().description()),true);
        }
    }
    const QMap<QString, QString> &other = tag->userTags();
    QMap<QString, QString>::const_iterator it = other.constBegin();
    for (; it != other.constEnd(); ++it) {
        QStringList fields = App->writeFieldsSeparately ? it.value().split(QSL(";"),SKIP_EMPTY_PARTS)
                                                        : QStringList(it.value()) ;

        xiph->removeFields(TS(it.key().toUpper()));
        xiph->removeFields(TS(it.key().toLower()));
        Q_FOREACH (const QString &field, fields)
            xiph->addField(TS(it.key()),TS(field),false);
    }

    //ReplayGain
    //first remove old ones
    // RFC 7845 states:
    // To avoid confusion with multiple normalization schemes, an Opus
    // comment header SHOULD NOT contain any of the REPLAYGAIN_TRACK_GAIN,
    // REPLAYGAIN_TRACK_PEAK, REPLAYGAIN_ALBUM_GAIN, or
    // REPLAYGAIN_ALBUM_PEAK tags, […]"
    // so we remove these if present
    xiph -> removeFields(RG_STRING_UPPER[RG_TRACK_GAIN]);
    xiph -> removeFields(RG_STRING_UPPER[RG_TRACK_PEAK]);
    xiph -> removeFields(RG_STRING_UPPER[RG_TRACK_RANGE]);
    xiph -> removeFields(RG_STRING_UPPER[RG_ALBUM_GAIN]);
    xiph -> removeFields(RG_STRING_UPPER[RG_ALBUM_PEAK]);
    xiph -> removeFields(RG_STRING_UPPER[RG_ALBUM_RANGE]);
    xiph -> removeFields(RG_STRING_UPPER[RG_REFERENCE_LOUDNESS]);
    xiph -> removeFields("R128_TRACK_GAIN");
    xiph -> removeFields("R128_ALBUM_GAIN");
    //then write new ones
    const auto &rg = tag->replayGainInfo();
    if (opusFile) {
        xiph->addField("R128_TRACK_GAIN", TS(rg.trackGain));
        xiph->addField("R128_ALBUM_GAIN", TS(rg.albumGain));
    }
    else {
        xiph->addField("REPLAYGAIN_TRACK_GAIN",TS(rg.trackGain));
        xiph->addField("REPLAYGAIN_TRACK_PEAK",TS(rg.trackPeak));
        xiph->addField("REPLAYGAIN_ALBUM_GAIN",TS(rg.albumGain));
        xiph->addField("REPLAYGAIN_ALBUM_PEAK",TS(rg.albumPeak));
        xiph->addField("REPLAYGAIN_TRACK_RANGE", TS(rg.trackMinMax));
        xiph->addField("REPLAYGAIN_ALBUM_RANGE", TS(rg.albumMinMax));
        xiph->addField("REPLAYGAIN_REFERENCE_LOUDNESS",TS(rg.loudness));
    }
}

QStringList handleAttributeList(const TagLib::ASF::AttributeList &list)
{DD;
    QStringList result;
    for (TagLib::ASF::AttributeList::ConstIterator it=list.begin(); it!=list.end(); ++it) {
        QString s=QS((*it).toString());
        if (!s.isEmpty()) result.append(s);
    }
    return result;
}

void TagsReaderWriter::readAsf(TagLib::ASF::Tag *asftag)
{DD;
    if (!asftag) return;

    tag->d->tags[TITLE] = QS(asftag->title());
    tag->d->tags[ARTIST]= QS(asftag->artist());
    tag->d->tags[COPYRIGHT]= QS(asftag->copyright());
    tag->d->tags[RATING]=QS(asftag->rating());

    TagLib::ASF::AttributeListMap &map=asftag->attributeListMap();
    for (TagLib::ASF::AttributeListMap::ConstIterator it = map.begin(); it != map.end(); ++it)  {
        QString id=QS((*it).first);

        if (id=="WM/MediaPrimaryClassID" || id=="WMFSDKNeeded"
            || id=="WMFSDKVersion" || id=="DeviceConformanceTemplate")
            continue;
        if (id=="WM/Picture") {
            TagLib::ASF::Picture picture = (*it).second[0].toPicture();
            if (picture.isValid()) {
                tag->d->image.setPixmap(QByteArray(picture.picture().data(),picture.picture().size()));
                tag->d->image.setMimetype(QS(picture.mimeType()));
                tag->d->image.setDescription(QS(picture.description()));
                tag->d->image.setType((int)picture.type());
            }
            continue;
        }
        QString value=handleAttributeList((*it).second).join(QSL(";"));
        parseTag(id,TaggingScheme::ASF,value);
    }
    // two more fields that are stored internally in wma, so read them if
    // specified fields are empty.
    if (tag->tag(COMMENT).isEmpty())
        tag->d->tags[COMMENT]= QS(asftag->comment());
    if (tag->tag(TRACKNUMBER).isEmpty())
        tag->d->tags[TRACKNUMBER] = QString::number(asftag->track());
}

void writeAsfItem(TagLib::ASF::Tag *tag, const QString &s, const TagLib::String &key)
{DD;
    if (!s.isEmpty()) tag->addAttribute(key,TagLib::ASF::Attribute(TS(s)));
}

void TagsReaderWriter::writeAsf(TagLib::ASF::Tag *asftag)
{DD;
    if (!asftag) return;

    renderTags(TaggingScheme::ASF, asftag);

    asftag->setArtist(TS(tag->tag(ARTIST)));
    asftag->setTitle(TS(tag->tag(TITLE)));
    asftag->setComment(TS(tag->tag(COMMENT)));
    asftag->setCopyright(TS(tag->tag(COPYRIGHT)));
    asftag->removeItem("WM/SharedUserRating");
    asftag->setRating(TagLib::String::number(qMin(tag->tag(RATING).toInt(),5)));
    //tag->removeItem("WM/BPM");

    if (!tag->image().pixmap().isEmpty()) {
        TagLib::ASF::Picture picture;
        picture.setDescription(TS(tag->image().description()));
        picture.setMimeType(TS(tag->image().mimetype()));
        picture.setPicture(TagLib::ByteVector(tag->image().pixmap().data(),tag->image().pixmap().size()));
        picture.setType((TagLib::ASF::Picture::Type)tag->image().type());
        asftag->setAttribute("WM/Picture",TagLib::ASF::Attribute(picture));
    }
    else
        asftag->removeItem("WM/Picture");

    const QMap<QString, QString> &other = tag->userTags();
    QMap<QString, QString>::const_iterator it = other.constBegin();
    for (; it != other.constEnd(); ++it) {
        QStringList fields = App->writeFieldsSeparately ? it.value().split(QSL(";"),SKIP_EMPTY_PARTS)
                                                        : QStringList(it.value());
        asftag->removeItem(TS(it.key()));
        Q_FOREACH (const QString &field, fields)
            writeAsfItem(asftag,field,TS(it.key()));
    }

    //ReplayGain
    //first remove old RG tags
    TagLib::ASF::AttributeListMap &items = asftag->attributeListMap();
    for(TagLib::ASF::AttributeListMap::Iterator item = items.begin();
        item != items.end(); ++item) {
        TagLib::String desc = item->first.upper();

        for (unsigned i=0; i<sizeof(RG_STRING_UPPER); ++i) {
            if (desc==RG_STRING_UPPER[i]) asftag->removeItem(item->first);
        }
    }
    //then write new ones
    const char **RG_STRING = RG_STRING_UPPER;
    if (App->replaygainOptions.tagsCase==1) RG_STRING = RG_STRING_LOWER;

    const ReplayGainInfo &rg = tag->replayGainInfo();
    writeAsfItem(asftag, RG_STRING[RG_TRACK_GAIN], TS(rg.trackGain));
    writeAsfItem(asftag, RG_STRING[RG_TRACK_PEAK], TS(rg.trackPeak));
    writeAsfItem(asftag, RG_STRING[RG_ALBUM_GAIN], TS(rg.albumGain));
    writeAsfItem(asftag, RG_STRING[RG_ALBUM_PEAK], TS(rg.albumPeak));
    writeAsfItem(asftag, RG_STRING[RG_TRACK_RANGE], TS(rg.trackMinMax));
    writeAsfItem(asftag, RG_STRING[RG_ALBUM_RANGE], TS(rg.albumMinMax));
    writeAsfItem(asftag, RG_STRING[RG_REFERENCE_LOUDNESS], TS(rg.loudness));
}

void TagsReaderWriter::readMP4(TagLib::MP4::Tag *mp4tag)
{DD;
    if (!mp4tag)
        return;

    const TagLib::MP4::ItemListMap &map = mp4tag->itemListMap();

    for (TagLib::MP4::ItemListMap::ConstIterator it = map.begin(); it != map.end(); ++it)  {
        QString id=QS((*it).first);
        const auto &item = (*it).second;

//        qDebug()<<id << (*it).second.atomDataType() << QS((*it).second.toStringList().toString(";"));

        if (id=="trkn" || id=="disk") {
            int tn = item.toIntPair().first;
            int tt = item.toIntPair().second;
            parseTag(id,TaggingScheme::MP4,QString("%1/%2").arg(tn).arg(tt));
        }
        else if (id == "cpil" || id == "pgap" || id == "pcst" || id == "hdvd") {
            bool b=item.toBool();
            parseTag(id,TaggingScheme::MP4, b ? QSL("1") : QSL("0"));
        }
        else if (id=="tmpo") {
            parseTag(id, TaggingScheme::MP4, QString::number(item.toInt()));
        }
        else if (id == "tvsn" || id == "tves" || id == "cnID" ||
                 id == "sfID" || id == "atID" || id == "geID") {
            parseTag(id, TaggingScheme::MP4, QString::number(item.toUInt()));
        }
        else if (id == "akID") {
            parseTag(id, TaggingScheme::MP4, QString::number(item.toByte()));
        }
        else if (id == "plID") {
            parseTag(id, TaggingScheme::MP4, QString::number(item.toLongLong()));
        }
        else if (id == "rate") {
            auto stringValue = item.toStringList();
            if (!stringValue.isEmpty())
                parseTag(id, TaggingScheme::MP4, QS(stringValue[0]));
            else
                parseTag(id, TaggingScheme::MP4, QString::number(item.toInt()));
        }
        else if (id == "rati") {
            parseTag(id, TaggingScheme::MP4, QString::number(item.toInt()));
        }
        else if (id=="rtng") {
            int r=item.toByte();
            parseTag(id, TaggingScheme::MP4, QString::number(r / 20));
        }
        else if (id=="stik") {
            uchar stik = item.toByte();
//            QString val = QSL("Unknown");
//            switch (stik) {
//                case 1: val = QSL("Normal"); break;
//                case 2: val = QSL("Audiobook"); break;
//                case 5: val = QSL("Whacked Bookmark"); break;
//                case 6: val = QSL("Music Video"); break;
//                case 9: val = QSL("Short Film"); break;
//                case 10: val = QSL("TV Show"); break;
//                case 11: val = QSL("Booklet"); break;
//                case 14: val = QSL("Ringtone"); break;
//                default: break;
//            }
            parseTag(id, TaggingScheme::MP4, QString::number(stik));
        }
        else if (id=="covr") {
            TagLib::MP4::CoverArtList coverList=item.toCoverArtList();
            if (!coverList.isEmpty()) {
                TagLib::MP4::CoverArt cover=coverList[0];
                tag->d->image.setMimetype(cover.format()==TagLib::MP4::CoverArt::JPEG ? QSL("image/jpeg"):QSL("image/png"));
                tag->d->image.setDescription(QSL(""));
                TagLib::ByteVector data=cover.data();
                QByteArray bytes(data.data(),data.size());
                tag->d->image.setPixmap(bytes);
            }
        }
        else if (id==QSL("----:com.apple.iTunes:iTunNORM") || id==QSL("----:com.apple.iTunes:iTunSMPB")) {
            // ignore these tags
            continue;
        }
        else {
            QString value = QS(item.toStringList().toString(";"));
            parseTag(id,TaggingScheme::MP4,value);
        }
    }
}

void writeMp4Item(TagLib::MP4::Tag *tag, const QString &id, const QString &val, bool other=false)
{DD;
    if (!tag) return;
    TagLib::MP4::ItemListMap &map=tag->itemListMap();
    QString value = val;
    QString key=id;

    if (value.isEmpty()) {
        map.erase(TS(key));
        key.prepend(QLS("----:com.apple.iTunes:"));
        map.erase(TS(key));
        return;
    }
    if (key=="rtng") value = QString::number(value.toInt()*20);

    if (key=="trkn" || key=="disk") {
        QStringList list = value.split(QSL("/"));
        TagLib::MP4::Item pair(list.at(0).toInt(),list.value(1).toInt());
        map[TS(key)] = pair;
    }
    else if (key == "cpil" || key == "pgap" || key == "pcst" || key == "hdvd") {
        TagLib::MP4::Item item(value=="1" || value=="true");
        map[TS(key)] = item;
    }
    else if (key=="tmpo") {
        TagLib::MP4::Item item(value.toInt());
        map[TS(key)] = item;
    }
    else if (key == "tvsn" || key == "tves" || key == "cnID" ||
             key == "sfID" || key == "atID" || key == "geID") {
        TagLib::MP4::Item item(value.toUInt());
        map[TS(key)] = item;
    }
    else if (key == "rtng" || key == "akID") {
        TagLib::MP4::Item item(value.toUShort());
        map[TS(key)] = item;
    }
    else if (key == "stik") {
        bool ok;
        ushort v = value.toUShort(&ok);
        TagLib::MP4::Item item(v);
        map[TS(key)] = item;
    }
    else if (key == "plID") {
        TagLib::MP4::Item item(value.toLongLong());
        map[TS(key)] = item;
    }
    else if (key == "rati") {
        TagLib::MP4::Item item(value.toUShort()); item.setAtomDataType(TagLib::MP4::TypeImplicit);
        map[TS(key)] = item;
    }
    else {// text atoms
        if (other) {
            if (!QString(mp4tags).contains(key) && !key.startsWith(QLS("----:com.apple.iTunes:")))
                key.prepend(QLS("----:com.apple.iTunes:"));
        }
        map[TS(key)] = TagLib::StringList(TS(value));
    }
}

TagLib::String mp4tagname(TagLib::String s)
{
    s.append("----:com.apple.iTunes:");
    return s;
}

void TagsReaderWriter::writeMP4(TagLib::MP4::Tag *mp4tag)
{DD;
    if (!mp4tag) return;

    TagLib::MP4::ItemListMap &map=mp4tag->itemListMap();
    renderTags(TaggingScheme::MP4,mp4tag);
    map.erase("covr");
    if (!tag->image().pixmap().isEmpty()) {
        TagLib::MP4::CoverArt::Format format=TagLib::MP4::CoverArt::JPEG;
        if (tag->image().mimetype()=="image/png") format=TagLib::MP4::CoverArt::PNG;
        TagLib::MP4::CoverArt cover(format,TagLib::ByteVector(tag->image().pixmap().data(),(unsigned int)tag->image().pixmap().size()));

        TagLib::MP4::CoverArtList l;
        l.append(cover);
        map["covr"]=TagLib::MP4::Item(l);
    }

    const QMap<QString, QString> &other = tag->userTags();
    QMap<QString, QString>::const_iterator it = other.constBegin();
    for (; it != other.constEnd(); ++it) {
        writeMp4Item(mp4tag,it.key(),it.value(),true);
    }

    //remove old tags
    TagLib::String desc;
#if TAGLIB_VERSION >= 11200
    TagLib::MP4::ItemMap items = mp4tag->itemMap();
    for(TagLib::MP4::ItemMap::Iterator item = items.begin(); item != items.end(); ++item)
#else
    TagLib::MP4::ItemListMap &items = tag->itemListMap();
    for(TagLib::MP4::ItemListMap::Iterator item = items.begin(); item != items.end(); ++item)
#endif
    {
        desc = item->first.upper();
        for (unsigned i=0; i<sizeof(RG_STRING_UPPER); ++i) {
            if (desc==mp4tagname(RG_STRING_UPPER[i]).upper()) mp4tag->removeItem(item->first);
        }
    }
    const char **RG_STRING = RG_STRING_UPPER;

    if (App->replaygainOptions.tagsCase == 1) {
        RG_STRING = RG_STRING_LOWER;
    }

    const ReplayGainInfo &rg = tag->replayGainInfo();
    writeMp4Item(mp4tag, RG_STRING[RG_TRACK_GAIN], rg.trackGain, true);
    writeMp4Item(mp4tag, RG_STRING[RG_TRACK_PEAK], rg.trackPeak,true);
    writeMp4Item(mp4tag, RG_STRING[RG_ALBUM_GAIN], rg.albumGain,true);
    writeMp4Item(mp4tag, RG_STRING[RG_ALBUM_PEAK], rg.albumPeak,true);
    writeMp4Item(mp4tag, RG_STRING[RG_TRACK_RANGE], rg.trackMinMax,true);
    writeMp4Item(mp4tag, RG_STRING[RG_ALBUM_RANGE], rg.albumMinMax,true);
    //writeMp4Item(mp4tag, QSL("replaygain_mp3gain_undo"),rg.undo,true);
    writeMp4Item(mp4tag, RG_STRING[RG_REFERENCE_LOUDNESS], rg.loudness,true);
}

void TagsReaderWriter::readAPE(TagLib::APE::Tag *apetag)
{DD;
    if (!apetag) return;

    TagLib::APE::ItemListMap map=apetag->itemListMap();

    for (TagLib::APE::ItemListMap::ConstIterator it = map.begin(); it != map.end(); ++it)  {
        QString id=QS((*it).first).toUpper();

        if (id=="COVER ART (FRONT)") {
            TagLib::ByteVector v=(*it).second.binaryData();
            QByteArray data=QByteArray(v.data(),v.size());
            int pos=data.indexOf('\0',0);
            int pos1=data.lastIndexOf('.',pos);
            QByteArray descr=data.mid(0,pos);
            tag->d->image.setDescription(QString::fromUtf8(descr.constData()));
            QByteArray mime=data.mid(pos1+1,pos-pos1-1);
            tag->d->image.setMimetype(QSL("image/png"));
            if (mime.toUpper()=="JPG" || mime.toUpper()=="JPEG")
                tag->d->image.setMimetype(QSL("image/jpeg"));
            tag->d->image.setPixmap(data.mid(pos+1));
            tag->d->image.setType(3);
        }
        else if ((*it).second.type()==TagLib::APE::Item::Text) {
            QString value=QS((*it).second.toString());
            parseTag(id,TaggingScheme::APE,value);
        }
    }
}

void TagsReaderWriter::writeAPE(TagLib::APE::Tag *apetag)
{DD;
    if (!apetag) return;

    renderTags(TaggingScheme::APE,apetag);
    apetag->removeItem("Cover Art (front)");
    QByteArray bytes=tag->image().pixmap();
    if (!bytes.isEmpty()) {
        bytes.prepend((char)0x0);
        if (tag->image().description().isEmpty()) {
            if (tag->image().mimetype()=="image/jpeg")
                bytes.prepend("front.jpg");
            else
                bytes.prepend("front.png");
        }
        else
            bytes.prepend(tag->image().description().toUtf8());

        TagLib::ByteVector data;
        data.append(TagLib::ByteVector::fromUInt(bytes.size(), false));
        data.append(TagLib::ByteVector::fromUInt(2, false));
        data.append("Cover Art (front)");
        data.append(TagLib::ByteVector('\0'));
        data.append(TagLib::ByteVector(bytes.constData(),bytes.size()));

        TagLib::APE::Item item;
        item.parse(data);
        apetag->setItem("Cover Art (front)",item);
    }

    const QMap<QString, QString> &other = tag->userTags();
    QMap<QString, QString>::const_iterator it = other.constBegin();
    for (; it != other.constEnd(); ++it)
        apetag->setItem(TS(it.key()),TagLib::APE::Item(TS(it.key()),TS(it.value())));

    //replaygain
    if (tag->fileType()==Tag::MPC_FILE && App->mpcWriteRg)
        tag->removeReplayGainInfo(false);
    else {
        const ReplayGainInfo &rg = tag->replayGainInfo();

        apetag->addValue("REPLAYGAIN_TRACK_GAIN",  TS(rg.trackGain));
        apetag->addValue("REPLAYGAIN_TRACK_PEAK",  TS(rg.trackPeak));
        apetag->addValue("REPLAYGAIN_ALBUM_GAIN",  TS(rg.albumGain));
        apetag->addValue("REPLAYGAIN_ALBUM_PEAK",  TS(rg.albumPeak));
        apetag->addValue("REPLAYGAIN_TRACK_RANGE", TS(rg.trackMinMax));
        apetag->addValue("REPLAYGAIN_ALBUM_RANGE", TS(rg.albumMinMax));
        apetag->addValue("MP3GAIN_UNDO",           TS(rg.undo));
        apetag->addValue("REPLAYGAIN_REFERENCE_LOUDNESS",TS(rg.loudness));
    }
}


void TagsReaderWriter::renderTags(const TaggingScheme::TagType type, TagLib::Tag *taglibtag)
{DD;
    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<tagsCount; ++i) {
        QStringList fields = App->currentScheme->fields(i,type);

        QString value = tag->tag(i);
        if (i==RATING) {
            int r=qMin(value.toInt(),5);
            value=r>0?QString::number(r) : QSL("");
        }

        if (App->currentScheme->fieldName(i)==QSL("Music/Video Kind")) {
            bool ok;
            value.toUShort(&ok);
            if (!ok) {
                if (value == QSL("Movie")) value = "0";
                else if (value == QSL("Normal")) value = "1";
                else if (value == QSL("Audiobook")) value = "2";
                else if (value == QSL("Whacked Bookmark")) value = "5";
                else if (value == QSL("Music Video")) value = "6";
                else if (value == QSL("Short Film")) value = "9";
                else if (value == QSL("TV Show")) value = "10";
                else if (value == QSL("Booklet")) value = "11";
                else if (value == QSL("Ringtone")) value = "14";
                else value = "";
            }
        }

        Q_FOREACH (const QString &field, fields) {
            if (field == QSL("internal")) continue;

            QString value1 = value;
            QList<int> indexes = App->currentScheme->search(type, field);
            if (indexes.isEmpty())
                continue;
            else if (indexes.size()>1) {
                QStringList composedValue;
                for(int j=0; j<indexes.size()-1;++j) {
                    composedValue << tag->tag(indexes.at(j));
                }
                QString s = tag->tag(indexes.last());
                if (!s.isEmpty()) composedValue << s;

                value1 = composedValue.join("/");

            }
//            qDebug()<<"writing"<<field<<indexes<<value1;

            switch (type) {
                case TaggingScheme::VORBIS: {
                    TagLib::Ogg::XiphComment *t =  static_cast<TagLib::Ogg::XiphComment *>(taglibtag);
                    QStringList tags = App->writeFieldsSeparately ? value1.split(QSL(";"),SKIP_EMPTY_PARTS) : QStringList(value1);
                    t->removeFields(TS(field));

                    Q_FOREACH (const QString &v, tags) {
                        t->addField(TS(field),TS(v),false);
                    }
                    break;
                }
                case TaggingScheme::APE: {
                    TagLib::APE::Tag *t = static_cast<TagLib::APE::Tag *>(taglibtag);
                    if (t) {
                        t->setItem(TS(field),TagLib::APE::Item(TS(field),TS(value1)));
                    }
                    break;
                }
                case TaggingScheme::ID3:
                    writeID3v2Frame(static_cast<TagLib::ID3v2::Tag *>(taglibtag) ,value1, field);
                    break;
                case TaggingScheme::ASF: {
                    TagLib::ASF::Tag *t = static_cast<TagLib::ASF::Tag *>(taglibtag);
                    QStringList tags = App->writeFieldsSeparately ?  value1.split(QSL(";"),SKIP_EMPTY_PARTS) : QStringList(value1);
                    t->removeItem(TS(field));
                    Q_FOREACH (const QString &v, tags) {
                        writeAsfItem(t,v,TS(field));
                    }
                    break;
                }
                case TaggingScheme::MP4: {
                    TagLib::MP4::Tag *mp4= static_cast<TagLib::MP4::Tag *>(taglibtag);
                    writeMp4Item(mp4,field,value1);
                    break;
                }
                default: break;
            }
        }
    }
}

bool TagsReaderWriter::writeTags()
{DD;
    bool b=true;

    if (App->trim) {
        const int tagsCount = App->currentScheme->tagsCount();
        for (int i=0; i<tagsCount; ++i) {
            if (App->currentScheme->canBeSimplified(i))
                tag->d->tags[i] = tag->tag(i).simplified();
        }
        /*QMap<QString, QString> other = tag->userTags();
        QMutableMapIterator<QString, QString> i(other);
        while (i.hasNext()) {
            i.next();
            i.setValue(i.value().simplified());
        }
        tag->d->otherTags = other;*/
    }

    //adjustLineEnding(tag->d->tags[LYRICS]);
    //adjustLineEnding(tag->d->tags[COMMENT]);

    if (tag->d->image.mimetype().isEmpty())
        tag->d->image.setMimetype(QSL("image/png"));
    if (tag->d->image.type() < 0 || tag->image().type() > 0x14)
        tag->d->image.setType(3);



    switch (tag->fileType()) {
        case Tag::MP3_FILE: {
            TagLib::MPEG::File *f=new TagLib::MPEG::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                int savingTypes=0;
                if (App->mp3writeape) {
                    TagLib::APE::Tag *tag=f->APETag(true);
                    if (tag) {
                        writeAPE(tag);
                        savingTypes |=TagLib::MPEG::File::APE;
                    }
                }
                if (App->mp3writeid3) {
                    TagLib::ID3v2::Tag *tag=f->ID3v2Tag(true);
                    if (tag) {
                        writeID3v2(tag);
                        savingTypes |= TagLib::MPEG::File::ID3v2;
                    }
                }
                if (App->id3v1Synchro<2) {
                    TagLib::ID3v1::Tag *tag=f->ID3v1Tag(App->id3v1Synchro==0);
                    if (tag) {
                        writeID3v1(tag);
                        savingTypes |= TagLib::MPEG::File::ID3v1;
                    }
                }
                b=f->save(savingTypes,true,App->id3v2version);
            }
            delete f;
            break;
        }
        case Tag::DSF_FILE: {
            TagLib::DSF::File *f=new TagLib::DSF::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                TagLib::ID3v2::Tag *tag=f->tag();
                writeID3v2(tag);
                b = f->save();
            }
            delete f;
            break;
        }
        case Tag::TTA_FILE: {
            TagLib::TrueAudio::File *f=new TagLib::TrueAudio::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                writeID3v2(f->ID3v2Tag(true));

                if (App->id3v1Synchro<2) {//do write id3v1
                    writeID3v1(f->ID3v1Tag(App->id3v1Synchro==0));
                }
                else f->strip(TagLib::TrueAudio::File::ID3v1);
                b=f->save();
            }
            delete f;
            break;
        }
        case Tag::OGG_FILE: {
            TagLib::Ogg::Vorbis::File *f=new TagLib::Ogg::Vorbis::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                TagLib::Ogg::XiphComment *tag=f->tag();
                writeXiph(tag);
                b=f->save();
            }
            delete f;
            break;
        }
        case Tag::OPUS_FILE: {
            TagLib::Ogg::Opus::File *f=new TagLib::Ogg::Opus::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                TagLib::Ogg::XiphComment *tag=f->tag();
                writeXiph(tag, true);
                b=f->save();
            }
            delete f;
            break;
        }
        case Tag::FLAC_FILE: {
            TagLib::FLAC::File *f = new TagLib::FLAC::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
//                if (App->flacwriteogg)
                {
                    writeXiph(f->xiphComment(true));
                    writeFlacPicture(f);
                }
//                else if (!App->flacwriteogg) {
//                    TagLib::Ogg::XiphComment *tag=f->xiphComment(false);
//                    delete tag;
//                }
//                if (App->flacwriteid3) {
//                    TagLib::ID3v2::Tag *tag = f->ID3v2Tag(true);
//                    writeID3v2(tag);
//                }
//                else if (!App->flacwriteid3) {
//                    TagLib::ID3v2::Tag *tag = f->ID3v2Tag(false);
//                    delete tag;
//                }
                if (App->id3v1Synchro<2) {
                    writeID3v1(f->ID3v1Tag(App->id3v1Synchro==0));
                }
                else {//not possible to remove id3v1 tags from flac files
                    TagLib::ID3v1::Tag *tag=f->ID3v1Tag(false);
                    if (tag) {
                        tag->setAlbum(TagLib::String::null);
                        tag->setArtist(TagLib::String::null);
                        tag->setTrack(0);
                        tag->setTitle(TagLib::String::null);
                        tag->setComment(TagLib::String::null);
                        tag->setGenre(TagLib::String::null);
                        tag->setYear(0);
                    }
                }
                b=f->save();
            }
            delete f;
            break;
        }
        case Tag::OGA_FILE: {
            TagLib::Ogg::FLAC::File *f = new TagLib::Ogg::FLAC::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                writeXiph(f->tag());
                b=f->save();
            }
            delete f;
            TagLib::Ogg::Vorbis::File *ff=new TagLib::Ogg::Vorbis::File(FILE_NAME(tag->fullFileName()));
            if (ff->isValid()) {
                writeXiph(ff->tag());
                b=ff->save();
            }
            delete ff;
            break;
        }
        case Tag::SPX_FILE: {
            TagLib::Ogg::Speex::File *f=new TagLib::Ogg::Speex::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                TagLib::Ogg::XiphComment *tag=f->tag();
                writeXiph(tag);
                b=f->save();
            }
            delete f;
            break;
        }
        case Tag::MPC_FILE: {
            TagLib::MPC::File *f=new TagLib::MPC::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                writeAPE(f->APETag(true));
                if (App->id3v1Synchro<2) {//do write id3v1
                    writeID3v1(f->ID3v1Tag(App->id3v1Synchro==0));
                }
                else f->strip(TagLib::MPC::File::ID3v1);
                b=f->save();
            }
            delete f;
            break;
        }
        case Tag::WV_FILE: {
            TagLib::WavPack::File *f=new TagLib::WavPack::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                writeAPE(f->APETag(true));
                if (App->id3v1Synchro<2) {//do write id3v1
                    writeID3v1(f->ID3v1Tag(App->id3v1Synchro==0));
                }
                else f->strip(TagLib::WavPack::File::ID3v1);
                b=f->save();
            }
            delete f;
            break;
        }
        case Tag::WAV_FILE: {
            TagLib::RIFF::WAV::File *f = new TagLib::RIFF::WAV::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                writeID3v2(f->tag());
                b=f->save();
            }
            delete f;
            break;
        }

        case Tag::WMA_FILE: {
            TagLib::ASF::File *f=new TagLib::ASF::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                writeAsf(f->tag());
                b=f->save();
            }
            delete f;
            break;
        }

        case Tag::AIFF_FILE: {
            TagLib::RIFF::AIFF::File *f = new TagLib::RIFF::AIFF::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                writeID3v2(f->tag());
                b=f->save();
            }
            delete f;
            break;
        }

        case Tag::M4A_FILE: {
            TagLib::MP4::File *f=new TagLib::MP4::File(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {
                writeMP4(f->tag());
                b=f->save();
            }
            delete f;
            break;
        }
        case Tag::APE_FILE: {
            APEFILE *f=new APEFILE(FILE_NAME(tag->fullFileName()));
            if (f->isValid()) {//qDebug()<<tag->fullFileName();
                writeAPE(f->APETag(true));
                if (App->id3v1Synchro<2) {//do write id3v1
                    writeID3v1(f->ID3v1Tag(App->id3v1Synchro==0));
                }
                else {//qDebug()<<"attempting to remove id3v1";
                    f->strip(TagLib::APE::File::ID3v1);
                }
                b=f->save();
            }
            delete f;
            break;
        }
    }
    tag->setChanged(!b);
    return b;
}

//void TagsReaderWriter::cleanAfterReading(int tagID, QString &value)
//{
//    switch (tagID) {
//        case YEAR:
//        case TRACKNUMBER:
//        case TOTALTRACKS:
//        case RATING:
//        case DISCNUMBER:
//        case TOTALDISCS:
//        case TEMPO:
//            if (value == "0") value.clear();
//            break;
//        default:
//            break;
//    }

//}

/**
 * \file tagger.h
 * Class for reading, writing and storing tags.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 8 Dec 2009
 *
 * Copyright (C) 2009  Alex Novichkov
 *
 * This file is part of Qoobar.
 *
 * Qoobar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Qoobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TAGGER_H
#define TAGGER_H

#include <QVector>
#include <QString>
#include <QMap>
#include <QSharedData>
#include <QMetaType>
#include "enums.h"
#include "coverimage.h"

struct ReplayGainInfo {
    ReplayGainInfo() {}
    ReplayGainInfo(const ReplayGainInfo &rg) :
        albumPeak(rg.albumPeak),
        trackPeak(rg.trackPeak),
        albumGain(rg.albumGain),
        trackGain(rg.trackGain),
        albumMinMax(rg.albumMinMax),
        trackMinMax(rg.trackMinMax),
        loudness(rg.loudness),
        undo(rg.undo)
    {}
    void clear();
    bool isEmpty() const;
    bool operator ==(const ReplayGainInfo &rg) const {
        return (albumPeak == rg.albumPeak
                && albumGain == rg.albumGain
                && trackPeak == rg.trackPeak
                && trackGain == rg.trackGain
                && albumMinMax == rg.albumMinMax
                && trackMinMax == rg.trackMinMax
                && loudness == rg.loudness
                && undo == rg.undo);
    }
    bool operator !=(const ReplayGainInfo &rg) const {
        return (albumPeak!=rg.albumPeak
                || albumGain!=rg.albumGain
                || trackPeak!=rg.trackPeak
                || trackGain!=rg.trackGain
                || albumMinMax!=rg.albumMinMax
                || trackMinMax!=rg.trackMinMax
                || loudness!=rg.loudness
                || undo!=rg.undo);
    }

    QString albumPeak;
    QString trackPeak;
    QString albumGain;
    QString trackGain;
    QString albumMinMax;
    QString trackMinMax;
    QString loudness;
    QString undo;
};

class TagData : public QSharedData
{
public:
    TagData(int tagsCount);
    TagData(const TagData &other);
    ~TagData() {}

    QString icon;
    QString fullFileName; //full path to file
    QString fileNameExt; //file name with extension
    QString fileName; //file name without extension
    QString filePath;
    QString fileExt; //extension without dot
    QString bitrate;
    int length; //length in seconds
    int sampleRate;
    int channels;
    bool wasChanged;
    int filetype;
    bool readOnly;
    int tagsCount;
    int tagTypes;
    qint64 size;

    QVector<QString> tags;

    ReplayGainInfo replayGainInfo;
    CoverImage image;
    QMap<QString,QString> otherTags;
};

class Tag
{
public:
    enum FileType {
        MP3_FILE=0,
        OGG_FILE=1,
        WMA_FILE=2,
        FLAC_FILE=3,
        M4A_FILE=4,
        MPC_FILE=5,
        WV_FILE=6,
        WAV_FILE=7,
        SPX_FILE=8,
        TTA_FILE=9,
        OGA_FILE=10,
        APE_FILE=11,
        AIFF_FILE=12,
        OPUS_FILE=13,
        DSF_FILE=14,
        FILES_NUM
    };
    Tag(int tagsCount = TAGSCOUNT);
    Tag(const QString &filename, int tagsCount = TAGSCOUNT);
    Tag(const Tag &other) : d(other.d) {}
    Tag& operator=(const Tag &other);

    void clear();

    ReplayGainInfo replayGainInfo() const {return d->replayGainInfo;}
    void setReplayGainInfo(const ReplayGainInfo &rg);
    void removeReplayGainInfo(bool updateStatus=true);
    bool replayGainInfoIsEmpty() const {return d->replayGainInfo.isEmpty();}
    void setAlbumPeak(const QString &value, bool updateStatus=true);
    void setAlbumGain(const QString &value, bool updateStatus=true);
    void setTrackPeak(const QString &value, bool updateStatus=true);
    void setTrackGain(const QString &value, bool updateStatus=true);
    void setAlbumMinMax(const QString &value, bool updateStatus=true);
    void setTrackMinMax(const QString &value, bool updateStatus=true);
    void setLoudness(const QString &value, bool updateStatus=true);
    void setReplayGainUndo(const QString &value, bool updateStatus=true);

    CoverImage image() const;
    void setImage(const CoverImage &image);
    void removeImage(bool updateStatus=true);
    bool imageIsEmpty() const {return d->image.isEmpty();}
    void setImagePixmap(const QByteArray &pixmap);
    void setImageDescription(const QString &description);
    void setImageMimetype(const QString &mimetype);
    void setImageType(int type);


    void setFile(const QString & fullFileName);
    /**
     * @brief fullFileName
     * @return full path of file
     */
    QString fullFileName() const {return d->fullFileName;}

    int fileType() const {return d->filetype;}
    static QString fileTypeDescription(int fileType);
    /**
     * @brief fileNameExt
     * @return  file name with extension
     */
    QString fileNameExt() const {return d->fileNameExt;}
    /**
     * @brief filePath
     * @return file path without trailing slash
     */
    QString filePath() const {return d->filePath;}
    /**
     * @brief fileExt
     * @return  file name extension without leading dot
     */
    QString fileExt() const {return d->fileExt;}
    /**
     * @brief fileName
     * @return file name without extension
     */
    QString fileName() const {return d->fileName;}


    int length() const {return d->length;}
    QString bitrate() const {return d->bitrate;}
    int sampleRate() const {return d->sampleRate;}
    int channels() const {return d->channels;}

    QString icon() const {return d->icon;}

    bool wasChanged() const {return d->wasChanged;}
    void setChanged(const bool changed);

    bool readOnly() const {return d->readOnly;}

    int tagsCount() const {return d->tagsCount;}

    int tagTypes() const {return d->tagTypes;}

    qint64 size() const {return d->size;}

    QString tag(const int &) const;
    QString nonEmptyTag(const int & tagID,const QString &defaultVal) const;
    QString userTag(const QString &) const;
    QStringList userTagsKeys() const;
    QMap<QString,QString> userTags() const {return d->otherTags;}

    void setTag(const int tagID,const QString &value);
    void setUserTag(const QString &tagKey,const QString &value);

    void removeStandardTags(bool updateStatus=true);
    void removeAllTags(bool updateStatus=true);
    void removeUserTags(bool updateStatus=true);

    QString album() const {return d->tags[ALBUM];} //a
    QString artist() const {return d->tags[ARTIST];} //A
    QString title() const {return d->tags[TITLE];} //t
    QString performer() const {return d->tags[PERFORMER];} //p
    QString comment() const {return d->tags[COMMENT];} //C
    QString genre() const {return d->tags[GENRE];} //g
    QString composer() const {return d->tags[COMPOSER];} //c
    QString year() const {return d->tags[YEAR];} //y
    QString tracknumber() const {return d->tags[TRACKNUMBER];} //n
    QString totalTracks() const {return d->tags[TOTALTRACKS];} //T
    QString conductor() const {return d->tags[CONDUCTOR];} //d
    QString orchestra() const {return d->tags[ORCHESTRA];} //o
    QString subtitle() const {return d->tags[SUBTITLE];} //s
    QString key() const {return d->tags[KEY];} //k
    QString albumArtist() const {return d->tags[ALBUMARTIST];} //u
    QString category() const {return d->tags[CATEGORY];} //G
    QString publisher() const {return d->tags[PUBLISHER];} //P
    QString copyright() const {return d->tags[COPYRIGHT];} //X
    QString mood() const {return d->tags[MOOD];} //m
    QString tempo() const {return d->tags[TEMPO];} //B
    QString lyricist() const {return d->tags[LYRICIST];} //w
    QString lyrics() const {return d->tags[LYRICS];} //W
    QString discnumber() const {return d->tags[DISCNUMBER];} //D
    QString totaldiscs() const {return d->tags[TOTALDISCS];} //S
    QString encodedby() const {return d->tags[ENCODEDBY];} //e
    QString remixedby() const {return d->tags[REMIXEDBY];} //x
    QString rating() const {return d->tags[RATING];} //R
    QString originalalbum() const {return d->tags[ORIGINALALBUM];} //O
    QString originalartist() const {return d->tags[ORIGINALARTIST];} //v
    QString originallyricist() const {return d->tags[ORIGINALLYRICIST];} //i
    QString url() const {return d->tags[URL];} //U
    QString isrc() const {return d->tags[ISRC];} //I

    bool operator==(const Tag &t) const;
    bool operator!=(const Tag &t) const;
private:
    QSharedDataPointer<TagData> d;
    friend class TagsReaderWriter;

};

Q_DECLARE_METATYPE(Tag)


#endif

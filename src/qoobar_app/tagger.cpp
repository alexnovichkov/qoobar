/**
 * \file tagger.cpp
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


#include "tagger.h"
#include <QFileInfo>
#include <QDir>
#include "enums.h"
#include "qoobarglobals.h"
#include <QStringList>
#include <QtDebug>

//qRegisterMetaType<Tag>();

int fileTypeByExt(QString extension, QString &icon)
{
    if (extension.isEmpty()) return -1;

    if (extension=="asf") extension=QSL("wma");
    else if (extension=="m4b" || extension=="m4p" || extension=="mp4"
             || extension=="3g2") extension=QSL("m4a");
    else if (extension=="aif") extension=QSL("aiff");

    static const QStringList list=QString("mp3.ogg.wma.flac.m4a.mpc.wv.wav.spx.tta.oga.ape.aiff.opus.dsf").split(QSL("."));
    icon = QString("%1.png").arg(extension);
    return list.indexOf(extension);
}

void ReplayGainInfo::clear()
{
    albumPeak.clear();
    trackPeak.clear();
    albumGain.clear();
    trackGain.clear();
    albumMinMax.clear();
    trackMinMax.clear();
    loudness.clear();
    undo.clear();
}

bool ReplayGainInfo::isEmpty() const
{
    return albumPeak.isEmpty()
            && trackPeak.isEmpty()
            && albumGain.isEmpty()
            && trackGain.isEmpty()
            && albumMinMax.isEmpty()
            && trackMinMax.isEmpty()
            && loudness.isEmpty()
            && undo.isEmpty();
}

TagData::TagData(int tagsCount) :
    length(0),
    sampleRate(0),
    channels(0),
    wasChanged(false),
    filetype(-1),
    readOnly(false),
    tagsCount(tagsCount), size(0)
{
    tags.resize(tagsCount);
}

TagData::TagData(const TagData &other) :
    QSharedData(other),
    icon(other.icon),
    fullFileName(other.fullFileName),
    fileNameExt(other.fileNameExt),
    fileName(other.fileName),
    filePath(other.filePath),
    fileExt(other.fileExt),
    bitrate(other.bitrate),
    length(other.length),
    sampleRate(other.sampleRate),
    channels(other.channels),
    wasChanged(other.wasChanged),
    filetype(other.filetype),
    readOnly(other.readOnly),
    tags(other.tags),
    replayGainInfo(other.replayGainInfo),
    image(other.image),
    otherTags(other.otherTags),
    size(other.size)
{

}

Tag::Tag(int tagsCount)
{
    d = new TagData(tagsCount);
}

Tag::Tag(const QString &filename, int tagsCount)
{
    d = new TagData(tagsCount);
    setFile(filename);
}

QString Tag::tag(const int &tagID) const
{
    return tagID < d->tags.size() ? d->tags.at(tagID) : QString();
}

QString Tag::nonEmptyTag(const int &tagID,const QString &defaultVal) const
{
    QString s=tag(tagID);
    if (s.isEmpty()) return defaultVal;
    return s;
}

QString Tag::userTag(const QString &key) const
{
    return d->otherTags.value(key);
}

QStringList Tag::userTagsKeys() const
{
    return d->otherTags.keys();
}

void Tag::setFile(const QString &fileName)
{
    QFileInfo fi(fileName);
    d->fullFileName = fi.canonicalFilePath();
    d->fileNameExt  = fi.fileName();
    d->fileName     = fi.completeBaseName();
    d->fileExt      = fi.suffix();
    d->filePath     = fi.canonicalPath();
    d->filetype     = fileTypeByExt(d->fileExt.toLower(), d->icon);
    d->readOnly     = !fi.isWritable();
    d->size         = fi.size();
}

void Tag::setTag1(const int tagID,const QString &value)
{
    if (tagID < d->tags.size()) d->tags[tagID] = value;
}

void Tag::setTag1(const QString &tag, const QString &value)
{
    d->otherTags[tag] = value;
}

void Tag::setTag(const int tagID,const QString &value)
{
    if (tagID >= d->tags.size())
        d->tags.resize(tagID+1);
    if (d->tags[tagID] == value) return;
    d->wasChanged = true;
    d->tags[tagID] = value;
}


void Tag::setUserTag(const QString &id, const QString &s)
{
    if (d->otherTags.value(id)==s) return;
    d->wasChanged=true;
    d->otherTags.insert(id,s);
}

/* ================================================*/
CoverImage Tag::image() const
{
    return d->image;
}

void Tag::setImage(const CoverImage &image, bool updateStatus)
{
    if (d->image==image) return;
    if (updateStatus)  d->wasChanged = true;
    d->image = image;
}

void Tag::setImagePixmap(const QByteArray &pixmap, bool updateStatus)
{
    if (d->image.pixmap() == pixmap) return;
    if (updateStatus) d->wasChanged = true;
    d->image.setPixmap(pixmap);
}

void Tag::setImageDescription(const QString &description, bool updateStatus)
{
    if (d->image.description() == description) return;
    if (updateStatus) d->wasChanged = true;
    d->image.setDescription(description);
}

void Tag::setImageMimetype(const QString &mimetype, bool updateStatus)
{
    if (d->image.mimetype() == mimetype) return;
    if (updateStatus) d->wasChanged = true;
    d->image.setMimetype(mimetype);
}

void Tag::setImageType(int type, bool updateStatus)
{
    if (d->image.type() == type) return;
    if (updateStatus) d->wasChanged = true;
    d->image.setType(type);
}

void Tag::removeImage(bool updateStatus)
{
    if (d->image.isEmpty()) return;
    if (updateStatus) d->wasChanged = true;
    d->image.clear();
}

/* =========================================*/
bool Tag::operator==(const Tag &t) const
{
    if (d->icon != t.d->icon) return false;
    if (d->fullFileName != t.d->fullFileName) return false; //full path to file
    if (d->bitrate != t.d->bitrate) return false;
    if (d->length != t.d->length) return false; //length in seconds
    if (d->sampleRate != t.d->sampleRate) return false;
    if (d->channels != t.d->channels) return false;
    if (d->wasChanged != t.d->wasChanged) return false;
    if (d->filetype != t.d->filetype) return false;
    if (d->readOnly != t.d->readOnly) return false;
    if (d->tags != t.d->tags) return false;
    if (d->replayGainInfo != t.d->replayGainInfo) return false;
    if (d->image != t.d->image) return false;
    if (d->otherTags != t.d->otherTags) return false;
    return true;
}

bool Tag::operator!=(const Tag &t) const
{
    return ((d->icon != t.d->icon) ||
            (d->fullFileName != t.d->fullFileName) ||
            (d->bitrate != t.d->bitrate) ||
            (d->length != t.d->length) ||
            (d->sampleRate != t.d->sampleRate) ||
            (d->channels != t.d->channels) ||
            (d->wasChanged != t.d->wasChanged) ||
            (d->filetype != t.d->filetype) ||
            (d->readOnly != t.d->readOnly)  ||
            (d->tags != t.d->tags) ||
            (d->replayGainInfo != t.d->replayGainInfo) ||
            (d->image != t.d->image) ||
            (d->otherTags != t.d->otherTags));
}

void Tag::clear()
{
    removeAllTags();
    d->length=0;
    d->bitrate.clear();
    d->sampleRate=0;
    d->channels=0;
    d->wasChanged=false;
}

void Tag::setChanged(const bool changed)
{
    d->wasChanged = changed;
}

void Tag::removeReplayGainInfo(bool updateStatus)
{
    if (d->replayGainInfo.isEmpty()) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.clear();
}

void Tag::setReplayGainInfo(const ReplayGainInfo &rg, bool updateStatus)
{
    if (d->replayGainInfo == rg) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo = rg;
}

void Tag::setLength(int length)
{
    d->length=length;
}

void Tag::setAlbumPeak(const QString &value, bool updateStatus)
{
    if (d->replayGainInfo.albumPeak==value) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.albumPeak=value;
}

void Tag::setAlbumGain(const QString &value, bool updateStatus)
{
    if (d->replayGainInfo.albumGain==value) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.albumGain = value;
}

void Tag::setTrackPeak(const QString &value, bool updateStatus)
{
    if (d->replayGainInfo.trackPeak==value) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.trackPeak = value;
}

void Tag::setTrackGain(const QString &value, bool updateStatus)
{
    if (d->replayGainInfo.trackGain==value) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.trackGain = value;
}

void Tag::setAlbumMinMax(const QString &value, bool updateStatus)
{
    if (d->replayGainInfo.albumMinMax==value) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.albumMinMax = value;
}

void Tag::setTrackMinMax(const QString &value, bool updateStatus)
{
    if (d->replayGainInfo.trackMinMax==value) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.trackMinMax = value;
}

void Tag::setLoudness(const QString &value, bool updateStatus)
{
    if (d->replayGainInfo.loudness==value) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.loudness = value;
}

void Tag::setReplayGainUndo(const QString &value, bool updateStatus)
{
    if (d->replayGainInfo.undo==value) return;
    if (updateStatus) d->wasChanged = true;
    d->replayGainInfo.undo = value;
}

void Tag::setUserTags1(const QMap<QString, QString> &other)
{
    d->otherTags = other;
}

void Tag::removeStandardTags(bool updateStatus)
{
    const int tagsCount = d->tags.size();
    d->tags.clear();
    d->tags.resize(tagsCount);
    if (updateStatus) d->wasChanged=true;
}

void Tag::removeUserTags(bool updateStatus)
{
    if (d->otherTags.isEmpty()) return;
    if (updateStatus) d->wasChanged = true;
    d->otherTags.clear();
}

void Tag::removeAllTags(bool updateStatus)
{
    removeUserTags(updateStatus);
    removeStandardTags(updateStatus);
    removeImage(updateStatus);
    removeReplayGainInfo(updateStatus);
}



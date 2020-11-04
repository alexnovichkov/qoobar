#ifndef ABSTRACTPLAYLIST_H
#define ABSTRACTPLAYLIST_H

#include "tagger.h"
#include <QList>
#include <QtCore>
#include <QtDebug>
#include <QDateTime>

class AbstractPlaylist
{
public:
    explicit AbstractPlaylist(const QList<Tag> &tags) : tags(tags), relativePaths(false)
    {}
    void setCodec(const QString &codecName) {codec = codecName;}

    QStringList getRelativeFileNames(const QString &fileName, int pathType)
    {
        relativePaths = pathType != 0;
        QStringList l;
        Q_FOREACH(const Tag &tag, tags) {
            QString s = tag.fullFileName(); //absolute file name by default
            if (relativePaths) { // if relative needed
                QFileInfo fi(fileName);
                if (s.startsWith(fi.canonicalPath()+"/")) { // if can make relative path
                    s.remove(0, fi.canonicalPath().length()+1);
                }
            }
            l << QDir::toNativeSeparators(s);
        }
        return l;
    }

    virtual QStringList createContents(const QStringList &fileNames)
    {
        return fileNames;
    }

    void savePlayList(const QString &fileName, const QStringList &contents)
    {
        QFile f(fileName);
        if (f.open(QFile::WriteOnly | QFile::Text)) {
            if (!codec.isEmpty()) {//write with specified codec
                QTextStream stream(&f);
                stream.setCodec(codec.toLatin1());
                Q_FOREACH (const QString &s, contents)
                    stream << s << Qt::endl;
            }
            else {//write in system locale codec
                Q_FOREACH (const QString &s, contents)
                    f.write(QFile::encodeName(s)+"\n");
            }
        }
    }

    void createPlaylist(const QString &fileName, int pathType)
    {
        QStringList relativeFileNames = getRelativeFileNames(fileName, pathType);

        QStringList contents = createContents(relativeFileNames);

        savePlayList(fileName, contents);
    }

    QList<Tag> tags;
    QString codec;
    bool relativePaths;
};

class M3UPlaylist: public AbstractPlaylist
{
public:
    M3UPlaylist(const QList<Tag> &tags, bool unicode) : AbstractPlaylist(tags)
    {
        if (unicode) setCodec("UTF-8");
    }
};

class ExtendedM3UPlaylist: public M3UPlaylist
{
public:
    ExtendedM3UPlaylist(const QList<Tag> &tags, bool unicode) : M3UPlaylist(tags, unicode) {}

    virtual QStringList createContents(const QStringList &fileNames)
    {
        QStringList l;
        l<<"#EXTM3U";
        for (int i=0; i<tags.size(); ++i) {
            l << QString("#EXTINF:%1,%2: %3 - %4")
                 .arg(tags.at(i).length())
                 .arg(tags.at(i).artist())
                 .arg(tags.at(i).album())
                 .arg(tags.at(i).title());
            l<<fileNames.at(i);
        }
        return l;
    }
};

class PlsPlaylist: public AbstractPlaylist
{
public:
    PlsPlaylist(const QList<Tag> &tags) : AbstractPlaylist(tags) {}
};

class ExtendedPlsPlaylist: public PlsPlaylist
{
public:
    ExtendedPlsPlaylist(const QList<Tag> &tags) : PlsPlaylist(tags) {}

    virtual QStringList createContents(const QStringList &fileNames)
    {
        QStringList l;
        l<<"[playlist]";
        for (int i=0; i<tags.size(); ++i) {
            l << QString("File%1=%2").arg(i+1)
                                     .arg(fileNames.at(i));
            l << QString("Title%1=%2: %3 - %4").arg(i+1)
                 .arg(tags.at(i).artist())
                 .arg(tags.at(i).album())
                 .arg(tags.at(i).title());
            l << QString("Length%1=%2").arg(i+1)
                 .arg(tags.at(i).length());
        }
        l << QString("NumberOfEntries=%1").arg(tags.size());
        l << "Version=2";
        return l;
    }
};

bool oneTag(const QList<Tag> &tags, int tagID)
{
    QString tag = tags.first().tag(tagID);
    for (int i=1; i<tags.size(); ++i)
        if (tags.at(i).tag(tagID)!=tag) return false;
    return true;
}

class XspfPlaylist: public AbstractPlaylist
{
public:
    XspfPlaylist(const QList<Tag> &tags, bool extended) : AbstractPlaylist(tags),
        extended(extended)
    {
        setCodec("UTF-8");
    }

    virtual QStringList createContents(const QStringList &fileNames)
    {
        QStringList l;
        l<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        l<<"<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">";
        if (extended) {
            if (oneTag(tags,ALBUM))
                l << "  <title>" + tags.first().album() + "</title>";
            if (oneTag(tags, ARTIST))
                l << "  <creator>" + tags.first().artist() + "</creator>";
            QString date = QDate::currentDate().toString("yyyy-MM-dd")
                           +"T"
                           +QTime::currentTime().toString("hh:mm:ss");
            l<<"  <date>"+date+"</date>";
        }
        l<<"  <trackList>";

        for (int i=0; i<tags.size(); ++i) {
            l<<"    <track>";
            QString s = fileNames.at(i);
            if (!relativePaths) s.prepend("file://");
            l<<"      <location>"+s+"</location>";
            if (extended) {
                if (!tags.at(i).title().isEmpty()) {
                    l<<"      <title>"+tags.at(i).title()+"</title>";
                }
                if (!tags.at(i).album().isEmpty()) {
                    l<<"      <album>"+tags.at(i).album()+"</album>";
                }
                if (!tags.at(i).artist().isEmpty()) {
                    l<<"      <creator>"+tags.at(i).artist()+"</creator>";
                }
                if (!tags.at(i).tracknumber().isEmpty()) {
                    l<<"      <trackNum>"+tags.at(i).tracknumber()+"</trackNum>";
                }
                l<<"      <duration>"+QString::number(tags.at(i).length()*1000)+"</duration>";
            }
            l<<"    </track>";
        }

        l<<"  </trackList>";
        l<<"</playlist>";
        return l;
    }
private:
    bool extended;
};

#endif // ABSTRACTPLAYLIST_H

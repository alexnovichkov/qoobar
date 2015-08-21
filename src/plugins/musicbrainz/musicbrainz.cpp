#include "musicbrainz.h"

#include "discidhelper.h"
#include "qoobarglobals.h"
#include <QXmlStreamReader>

#ifndef HAVE_QT5
#include <QtPlugin>
Q_EXPORT_PLUGIN2(musicbrainz, MusicbrainzPlugin)
#endif

Request MusicbrainzPlugin::queryForManualSearch(const QStringList &list)
{
    QString artist=QString(QUrl::toPercentEncoding(list.at(0),"' "));
    QString album=QString(QUrl::toPercentEncoding(list.value(1),"' "));

    QString query="http://musicbrainz.org/ws/2/release?query=";
    if (!album.isEmpty()) {
        query = query + "release:" + album;
        if (!artist.isEmpty())
            query = query + " AND artist:" + artist;
    }
    else query = query + "artist:" + artist;
    query.replace(" ","+");
    return Request(query);
}

Request MusicbrainzPlugin::queryForSearchByFiles(const QVector<int> &lengths)
{
    return query(lengths);
}

Request MusicbrainzPlugin::queryForCD()
{
    return query(QVector<int>());
}

Request MusicbrainzPlugin::queryForPicture(const QString &url)
{
    return Request("http://ec1.images-amazon.com/images/P/" + url + ".01.LZZZZZZZ.jpg");
}

Request MusicbrainzPlugin::queryForRelease(const QString &url)
{
    return Request("http://musicbrainz.org/ws/2/release/"
                   + url
                   + "?inc="
                   "artists"
                   "+labels"
                   "+artist-rels"
                   "+recordings"
                   "+recording-rels"
                   "+release-rels"
                   "+recording-level-rels"
                   "+work-level-rels");
}

Request MusicbrainzPlugin::query(const QVector<int> &lengths)
{
    DiscIDHelper helper;

    if (!helper.isDiscidResolved()) {
        m_errorString = helper.errorString;
        return Request();
    }

    QString id = helper.getDiscID(lengths,1);
    id = "http://musicbrainz.org/ws/2/discid/" + id
            + "&inc="
            "artists"
            "+labels"
            ;
    return Request(id);
}


QString parseLabels(QXmlStreamReader &x /*label-info-list*/)
{
    QStringList results;

    while (!x.atEnd()) {
        x.readNext();
        if (x.isEndElement()) {
            if (x.name()=="label-info-list") return results.join("; ");
        }
        else if (x.isStartElement()) {
            if (x.name()=="catalog-number")
                results << x.readElementText();
            else if (x.name()=="name") {
                if (results.isEmpty()) results << x.readElementText();
                else results.last().prepend(x.readElementText()+", ");
            }
        }
    }
    return results.join("; ");
}

QList<Artist> parseArtists(QXmlStreamReader &x)
{
    QList<Artist> result;

    while (!x.atEnd()) {
        x.readNext();
        if (x.isEndElement()) {
            if (x.name()=="artist-credit") return result;
        }
        else if (x.isStartElement()) {
            QStringRef name=x.name();
            if (name=="artist")
                result.append(Artist());
            else if (name=="name") {
                if (result.isEmpty()) result << Artist();
                result.last().fields.insert("name", x.readElementText());
            }
        }
    }
    return result;
}

QString parseAttributes(QXmlStreamReader &x/*"attribute-list"*/)
{
    QStringList result;

    while (!x.atEnd()) {
        x.readNext();
        if (x.isEndElement()) {
            if (x.name()=="attribute-list") return result.join(", ");
        }
        else if (x.isStartElement()) {
            if (x.name()=="attribute")
                result.append(x.readElementText());
        }
    }
    return result.join(", ");
}

QList<Artist> parseRelations(QXmlStreamReader &x/*relation-list*/)
{
    QList<Artist> result;

    if (x.attributes().value("target-type")!="artist") return result;

    QString type;
    QString attributes;
    while (!x.atEnd()) {
        x.readNext();
        if (x.isEndElement()) {
            if (x.name()=="relation-list") return result;
            else if (x.name()=="relation") {
                if (type=="instrument" || type=="vocal" || type=="vocals") type=attributes;
                if (result.isEmpty()) result << Artist();
                result.last().fields.insert("role", type);
            }
        }
        else if (x.isStartElement()) {
            QStringRef name=x.name();
            if (name=="relation") {
                result.append(Artist());
                type = x.attributes().value("type").toString();
            }
            else if (name=="name") {
                if (result.isEmpty()) result << Artist();
                result.last().fields.insert("name", x.readElementText());
            }
            else if (name=="attribute-list")
                attributes = parseAttributes(x);
        }
    }
    return result;
}

QList<Track> parseTracks(QXmlStreamReader &trackList, int cdNum)
{
    QList<Track> result;

    while (!trackList.atEnd()) {
        trackList.readNext();
        if (trackList.isEndElement()) {
            if (trackList.name()=="track-list") return result;
        }
        else if (trackList.isStartElement()) {
            QStringRef name=trackList.name();

            if (name == "track") {
                Track t;
                t.cd = cdNum;
                result << t;
            }
            else if (name=="position")
                result.last().fields.insert("tracknumber", trackList.readElementText());
            else if (name=="length")
                result.last().fields.insert("length", Qoobar::formatLength(trackList.readElementText().toInt()/1000));
            else if (name=="title")
                result.last().fields.insert("title", trackList.readElementText());
            else if (name=="artist-credit")
                result.last().artists.append(parseArtists(trackList));
            else if (name=="relation-list")
                result.last().artists.append(parseRelations(trackList));
        }
    }

    return result;
}

QList<Track> parseMediums(QXmlStreamReader &mediumList)
{
    QList<Track> result;

    int cdNum = 0;
    while (!mediumList.atEnd()) {
        mediumList.readNext();
        if (mediumList.isEndElement()) {
            if (mediumList.name()=="medium-list") return result;
        }
        else if (mediumList.isStartElement()) {
            QStringRef name = mediumList.name();
            if (name=="track-list") {
                cdNum++;
                result.append(parseTracks(mediumList, cdNum));
            }
        }
    }

    return result;
}

void shrinkArtists(QList<Artist> &artists)
{
    QMap<QString, QString> artistsMap;
    Q_FOREACH (const Artist &a,artists) {
        artistsMap.insertMulti(a.fields.value("name"), a.fields.value("role"));
    }

    artists.clear();
    QStringList uniqueKeys = artistsMap.uniqueKeys();
    Q_FOREACH (const QString &key, uniqueKeys) {
        Artist a;
        a.fields.insert("name", key);
        QStringList values = artistsMap.values(key);
        values.removeDuplicates();
        a.fields.insert("role", values.join(", "));
        artists << a;
    }
}

QList<SearchResult> MusicbrainzPlugin::parseResponse(const QByteArray &response)
{
    QList<SearchResult> results;

    m_errorString.clear();
    QXmlStreamReader x(response);
    while (!x.atEnd()) {
        x.readNext();
        // do processing
        if (x.isStartElement()) {
            QStringRef name = x.name();
            if (name=="release") {
                SearchResult r;
                r.fields.insert("url", x.attributes().value("id").toString());
                results.append(r);
            }
            else if (name=="format") {
                results.last().fields.insert("format",x.readElementText());
            }
            else if (name=="title") {
                results.last().fields.insert("album",x.readElementText());
            }
            else if (name=="artist-credit") {
                results.last().fields.insert("album",
                                             results.last().fields.value("album")
                                             + " / "
                                             +SearchResult::artistsText(parseArtists(x)));
            }
            else if (name=="label-info-list") {
                results.last().fields.insert("label", parseLabels(x));
            }
            else if (name=="date") {
                results.last().fields.insert("year",x.readElementText());
            }
            else if (name=="medium-list") {
                results.last().cdCount = x.attributes().value("count").toString().toInt();
            }
        }
    }
    if (x.hasError()) {
        m_errorString = x.errorString();
    }

    for (int i=results.size()-1; i>=0; --i) {
        if (results.at(i).fields.value("format")=="CD"
                || results.at(i).fields.value("format").isEmpty()) continue;
        results.removeAt(i);
    }

    return results;
}

SearchResult MusicbrainzPlugin::parseRelease(const QByteArray &response)
{
    SearchResult r;
    r.loaded = true;
    m_errorString.clear();

    QXmlStreamReader x(response);

    while (!x.atEnd()) {
        x.readNext();
        // do processing
        if (x.isStartElement()) {
            QStringRef name = x.name();
            if (name=="title") {
                r.fields.insert("album",x.readElementText());
            }
            else if (name=="artist-credit") {
                r.artists = parseArtists(x);
            }
            else if (name=="date") {
                r.fields.insert("year",x.readElementText().left(4));
            }
            else if (name=="label-info-list") {
                r.fields.insert("label",parseLabels(x));
            }
            else if (name=="asin") {
                r.image.setDescription(x.readElementText());
                r.image.setType(3);
                r.image.setMimetype("image/jpeg");
            }
            else if (name=="relation-list") {
                r.artists.append(parseRelations(x));
            }
            else if (name=="medium-list") {
                r.tracks = parseMediums(x);
                r.cdCount = r.tracks.isEmpty()?0:r.tracks.last().cd;
            }
        }
    }
    if (x.hasError()) {
        m_errorString=x.errorString();
    }

    shrinkArtists(r.artists);
    for (int i=0; i<r.tracks.size(); ++i) {
        shrinkArtists(r.tracks[i].artists);
    }
    return r;
}

QStringList MusicbrainzPlugin::releaseToList(const SearchResult &r)
{
    QStringList result;
    result << r.fields.value("album")
           << r.fields.value("label") + " / "+r.fields.value("date");
    QString format = r.fields.value("format");
    if (!format.isEmpty()) {
        if (r.cdCount<=1) result << QString::number(r.cdCount)+" "+format;
        else result << QString::number(r.cdCount)+" "+format+"s";
    }
    return result;
}



int MusicbrainzPlugin::preferredPauseSize()
{
    return 0;
}

QMap<QString, QString> MusicbrainzPlugin::authenticationInfo()
{
    return QMap<QString, QString>();
}

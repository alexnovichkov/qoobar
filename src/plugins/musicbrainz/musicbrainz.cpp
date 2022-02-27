#include "musicbrainz.h"

#include "discidhelper.h"
#include "qoobarglobals.h"
#include <QXmlStreamReader>

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
            if (x.name().compare(QLS("label-info-list"))==0) return results.join("; ");
        }
        else if (x.isStartElement()) {
            if (x.name().compare(QLS("catalog-number"))==0)
                results << x.readElementText();
            else if (x.name().compare(QLS("name"))==0) {
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
            if (x.name().compare(QLS("artist-credit"))==0) return result;
        }
        else if (x.isStartElement()) {
            x.name();
            if (x.name().compare(QLS("artist"))==0)
                result << Artist();
            else if (x.name().compare(QLS("name"))==0) {
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
            if (x.name().compare(QLS("attribute-list"))==0) return result.join(", ");
        }
        else if (x.isStartElement()) {
            if (x.name().compare(QLS("attribute"))==0)
                result.append(x.readElementText());
        }
    }
    return result.join(", ");
}

QList<Artist> parseRelations(QXmlStreamReader &x/*relation-list*/)
{
    QList<Artist> result;

    if (x.attributes().value("target-type").compare(QLS("artist"))!=0) return result;

    QString type;
    QString attributes;
    while (!x.atEnd()) {
        x.readNext();
        if (x.isEndElement()) {
            if (x.name().compare(QLS("relation-list"))==0) return result;
            if (x.name().compare(QLS("relation"))==0) {
                if (type=="instrument" || type=="vocal" || type=="vocals") type=attributes;
                if (result.isEmpty()) result << Artist();
                result.last().fields.insert("role", type);
            }
        }
        else if (x.isStartElement()) {
            if (x.name().compare(QLS("relation"))==0) {
                result.append(Artist());
                type = x.attributes().value("type").toString();
            }
            else if (x.name().compare(QLS("name"))==0) {
                if (result.isEmpty()) result << Artist();
                result.last().fields.insert("name", x.readElementText());
            }
            else if (x.name().compare(QLS("attribute-list"))==0)
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
            if (trackList.name().compare(QLS("track-list"))==0) return result;
        }
        else if (trackList.isStartElement()) {
            auto name=trackList.name();

            if (name.compare(QLS("track"))==0) {
                Track t;
                t.cd = cdNum;
                result << t;
            }
            else if (name.compare(QLS("position"))==0)
                result.last().fields.insert("tracknumber", trackList.readElementText());
            else if (name.compare(QLS("length"))==0) {
                auto len = trackList.readElementText().toInt()/1000; //length in seconds
                result.last().fields.insert("length", Qoobar::formatLength(len));
                result.last().length = len;
            }
            else if (name.compare(QLS("title"))==0)
                result.last().fields.insert("title", trackList.readElementText());
            else if (name.compare(QLS("artist-credit"))==0)
                result.last().artists.append(parseArtists(trackList));
            else if (name.compare(QLS("relation-list"))==0)
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
            if (mediumList.name().compare(QLS("medium-list"))==0) return result;
        }
        else if (mediumList.isStartElement()) {
            auto name = mediumList.name();
            if (name.compare(QLS("track-list"))==0) {
                cdNum++;
                result.append(parseTracks(mediumList, cdNum));
            }
        }
    }

    return result;
}

void shrinkArtists(QList<Artist> &artists)
{
    QMultiMap<QString, QString> artistsMap;
    Q_FOREACH (const Artist &a,artists) {
        artistsMap.insert(a.fields.value("name"), a.fields.value("role"));
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
            auto name = x.name();
            if (name.compare(QLS("release"))==0) {
                SearchResult r;
                r.fields.insert("url", x.attributes().value("id").toString());
                results.append(r);
            }
            else if (name.compare(QLS("format"))==0) {
                results.last().fields.insert("format",x.readElementText());
            }
            else if (name.compare(QLS("title"))==0) {
                results.last().fields.insert("album",x.readElementText());
            }
            else if (name.compare(QLS("artist-credit"))==0) {
                results.last().fields.insert("album",
                                             results.last().fields.value("album")
                                             + " / "
                                             +artistsText(parseArtists(x)));
            }
            else if (name.compare(QLS("label-info-list"))==0) {
                results.last().fields.insert("label", parseLabels(x));
            }
            else if (name.compare(QLS("date"))==0) {
                results.last().fields.insert("year",x.readElementText());
            }
            else if (name.compare(QLS("medium-list"))==0) {
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
    for (auto &r : results) r.releaseInfo = releaseToList(r);

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
            auto name = x.name();
            if (name.compare(QLS("title"))==0) {
                r.fields.insert("album",x.readElementText());
            }
            else if (name.compare(QLS("artist-credit"))==0) {
                r.artists = parseArtists(x);
            }
            else if (name.compare(QLS("date"))==0) {
                r.fields.insert("year",x.readElementText().left(4));
            }
            else if (name.compare(QLS("label-info-list"))==0) {
                r.fields.insert("label",parseLabels(x));
            }
            else if (name.compare(QLS("asin"))==0) {
                r.image.setDescription(x.readElementText());
                r.image.setType(3);
                r.image.setMimetype("image/jpeg");
            }
            else if (name.compare(QLS("relation-list"))==0) {
                r.artists.append(parseRelations(x));
            }
            else if (name.compare(QLS("medium-list"))==0) {
                r.tracks = parseMediums(x);
                r.cdCount = r.tracks.isEmpty()?0:r.tracks.last().cd;
            }
        }
    }
    if (x.hasError()) {
        m_errorString=x.errorString();
    }

    shrinkArtists(r.artists);
    for (auto &track: r.tracks)
        shrinkArtists(track.artists);

    r.releaseInfo = releaseToList(r);

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

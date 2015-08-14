#include "discogs.h"

//#include "enums.h"

#include <QUrl>
#include <QVariant>
#ifdef HAVE_QT5
#include <QtConcurrent/QtConcurrent>
#include <QJsonDocument>
#else
#include <QtConcurrentMap>
#include <QtConcurrentFilter>
#include "ereilin/json.h"
#endif

#ifndef HAVE_QT5
#include <QtPlugin>

Q_EXPORT_PLUGIN2(discogs, DiscogsPlugin)
#endif

Artist toArtist(const QVariant &v)
{
    Artist a;
    QVariantMap artist=v.toMap();
    static const char *fields[] = {"name", "role", "tracks"};
    for (int i=0; i<3; ++i)
        a.fields.insert(fields[i],artist[fields[i]].toString());

    return a;
}

template<typename T>
QList<T> parseToList(const QVariantList &list, T (*f)(const QVariant &))
{
    return QtConcurrent::blockingMapped(list,f);
}

QString toString(const QVariantList &list, QString (*f)(const QVariant &v), const QString &separator)
{
    QStringList result = parseToList(list, f);
    return result.join(separator);
}

bool isCD(const QVariant &v)
{
    QVariantMap vMap = v.toMap();
    return vMap["format"].toList().contains("CD");
}

QString toPlain(const QVariant &variant)
{
    return variant.toString();
}

SearchResult toSearchResult(const QVariant &v)
{
    SearchResult r;
    QVariantMap vMap = v.toMap();

    r.fields.insert("url", vMap["resource_url"].toString());
    r.fields.insert("album", vMap["title"].toString());

    QString label = toString(vMap["label"].toList(),toPlain, ", ");
    QVariant v1=vMap["catno"];
    if (!v1.isNull()) label.append(", " + v1.toString());
    r.fields.insert("label", label);

    QStringList extra;
    v1=vMap["country"]; if (!v1.isNull()) extra << (QObject::tr("Country") + ": " + v1.toString());
    v1=vMap["year"]; if (!v1.isNull()) extra << (QObject::tr("Year") + ": " + v1.toString());
    v1=vMap["genre"]; if (!v1.isNull()) extra << (QObject::tr("Genre") + ": "
                                                + toString(v1.toList(),toPlain,", "));
    r.fields.insert("extraData", extra.join(", "));

    return r;
}

Track toTrack(const QVariant &v)
{
    Track t;
    QVariantMap track=v.toMap();
    t.fields.insert("title",track["title"].toString());
    t.fields.insert("length",track["duration"].toString());
    t.fields.insert("tracknumber",track["position"].toString());
    t.artists = parseToList(track["artists"].toList()+track["extraartists"].toList(),
                            toArtist);
    return t;
}

QString toLabel(const QVariant &variant)
{
    QVariantMap label=variant.toMap();
    QString result=label["name"].toString();
    QVariant catno=label["catno"];
    if (!catno.isNull() && !result.isEmpty())
        result.append(", " + catno.toString());
    return result;
}

Request DiscogsPlugin::queryForManualSearch(const QStringList &list)
{
    QString query="http://api.discogs.com/database/search?type=release";
    if (!list.first().isEmpty()) query.append("&artist=" + QString(QUrl::toPercentEncoding(list.first())));
    if (!list.last().isEmpty()) query.append("&release_title=" + QString(QUrl::toPercentEncoding(list.last())));

    return Request(QUrl(query));
}

Request DiscogsPlugin::queryForCD()
{
    return Request(); //cannot search by cd
}

Request DiscogsPlugin::queryForSearchByFiles(const QVector<int> &)
{
    return Request(); //cannot search by files
}

Request DiscogsPlugin::queryForPicture(const QString &url)
{
    return Request(url);
}

Request DiscogsPlugin::queryForRelease(const QString &url)
{
    return Request(url);
}

QList<SearchResult> DiscogsPlugin::parseResponse(const QByteArray &response)
{
    QList<SearchResult> results;
    m_errorString.clear();

#ifdef HAVE_QT5
    QVariant parsed = QJsonDocument::fromJson(response).toVariant();
    QVariantMap result = parsed.toMap();
    if (result.isEmpty()) {
        m_errorString = tr("Cannor parse response from Discogs server");
        return results;
    }
#else
    bool ok=true;
    QVariantMap result = QtJson::Json::parse(QString::fromUtf8(response),ok).toMap();
    if (!ok) {
        m_errorString = tr("Cannor parse response from Discogs server");
        return results;
    }
#endif

    QVariantList filtered=QtConcurrent::blockingFiltered(result["results"].toList(), isCD);
    results = parseToList(filtered,toSearchResult);

    return results;
}

SearchResult DiscogsPlugin::parseRelease(const QByteArray &response)
{
    SearchResult r;
    r.loaded = true;
    m_errorString.clear();

#ifdef HAVE_QT5
    QVariant parsed = QJsonDocument::fromJson(response).toVariant();
    QVariantMap result = parsed.toMap();
    if (result.isEmpty()) {
        m_errorString = tr("Cannor parse response from Discogs server");
        return r;
    }
#else
    bool ok=true;
    QVariantMap result = QtJson::Json::parse(QString::fromUtf8(response),ok).toMap();
    if (!ok) {
        m_errorString = tr("Cannor parse response from Discogs server");
        return r;
    }
#endif

    r.fields.insert("label", toString(result["labels"].toList(),toLabel,"; "));
    r.fields.insert("year", result["year"].toString());

    QVariant primaryImage;
    Q_FOREACH (QVariant i, result["images"].toList()) {
        if (i.toMap()["type"].toString()=="primary") {
            primaryImage=i;
            break;
        }
    }
    if (!primaryImage.isValid() && !result["images"].toList().isEmpty())
        primaryImage = result["images"].toList().value(0);
    if (primaryImage.isValid()) {
        QVariantMap image=primaryImage.toMap();
        QString s=image["resource_url"].toString();
        if (!s.isEmpty()) {
            r.image.setDescription(s);
            r.image.setType(3);
            r.image.setMimetype("image/jpeg");
        }
    }
    r.fields.insert("genre", toString(result["genres"].toList(), toPlain, "; "));
    r.fields.insert("album", result["title"].toString());
    r.artists = parseToList(result["artists"].toList()+result["extraartists"].toList(),
                            toArtist);
    r.tracks = parseToList(result["tracklist"].toList(),toTrack);
    r.fields.insert("comment", result["notes"].toString());

    return r;
}

QStringList DiscogsPlugin::releaseToList(const SearchResult &r)
{
    QStringList result;
    result <<r.fields.value("album")
          <<r.fields.value("label")
         <<r.fields.value("extraData");
    return result;
}

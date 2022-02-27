#include "discogs.h"

//#include "enums.h"

#include <QUrl>
#include <QVariant>
#include <QtConcurrent/QtConcurrent>
#include <QJsonDocument>

const char CONSUMER_KEY[] = "rNTSpiaQHGyopsmOuRHC";
const char CONSUMER_SECRET[] = "ZdhxWUelUWhHjAOkVRUIEzguggsJYVUT";
const int PREFERRED_PAUSE_SIZE = 201;

Artist toArtist(const QVariant &v)
{
    Artist a;
    QVariantMap artist=v.toMap();
    static const char *fields[] = {"name", "role", "tracks"};
    for (int i=0; i<3; ++i)
        a.fields.insert(fields[i],artist.value(fields[i]).toString());

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
    QString query="https://api.discogs.com/database/search?type=release";
    if (!list.first().isEmpty()) query.append("&artist=" + QString(QUrl::toPercentEncoding(list.first())));
    if (!list.last().isEmpty()) query.append("&release_title=" + QString(QUrl::toPercentEncoding(list.last())));

    query.append(QString("&key=%1&secret=%2").arg(CONSUMER_KEY, CONSUMER_SECRET));

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
    Request r(url);
    r.addRawHeader("Authorization",QString("Discogs key=%1, secret=%2").arg(CONSUMER_KEY, CONSUMER_SECRET).toLatin1());
    return r;
}

QList<SearchResult> DiscogsPlugin::parseResponse(const QByteArray &response)
{
    QList<SearchResult> results;
    m_errorString.clear();

    QVariant parsed = QJsonDocument::fromJson(response).toVariant();
    QVariantMap result = parsed.toMap();
    if (result.isEmpty()) {
        m_errorString = tr("Cannot parse response from Discogs server");
        return results;
    }

    QVariantList filtered = QtConcurrent::blockingFiltered(result["results"].toList(), isCD);
    results = parseToList(filtered, toSearchResult);
    for (auto &r : results) r.releaseInfo = releaseToList(r);

    return results;
}

SearchResult DiscogsPlugin::parseRelease(const QByteArray &response)
{
    SearchResult r;
    r.loaded = true;
    m_errorString.clear();

    QVariant parsed = QJsonDocument::fromJson(response).toVariant();
    QVariantMap result = parsed.toMap();
    if (result.isEmpty()) {
        m_errorString = tr("Cannor parse response from Discogs server");
        return r;
    }

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
    r.releaseInfo = releaseToList(r);
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


int DiscogsPlugin::preferredPauseSize()
{
    return PREFERRED_PAUSE_SIZE;
}

QMap<QString, QString> DiscogsPlugin::authenticationInfo()
{
    return QMap<QString, QString>();
}

//#include "o2/o1.h"
//#include "o2/o2settingsstore.h"
//#include <qdesktopservices.h>

//const char O2_CONSUMER_KEY[] = "rNTSpiaQHGyopsmOuRHC";
//const char O2_CONSUMER_SECRET[] = "ZdhxWUelUWhHjAOkVRUIEzguggsJYVUT";
//const char O2_KEY[] = "qoobar167";

//bool DiscogsPlugin::authenticate()
//{
//    O1 *o1 = new O1(this);
//    o1->setAccessTokenUrl(QUrl("https://api.discogs.com/oauth/access_token"));
//    o1->setRequestTokenUrl(QUrl("https://api.discogs.com/oauth/request_token"));
//    o1->setAuthorizeUrl(QUrl("https://www.discogs.com/oauth/authorize"));
//    o1->setClientId(O2_CONSUMER_KEY);
//    o1->setClientSecret(O2_CONSUMER_SECRET);


//    O2SettingsStore *store = new O2SettingsStore(O2_KEY);
//    store->setGroupKey("qoobar");
//    o1->setStore(store);

//    connect(o1, SIGNAL(linkedChanged()),
//            this, SLOT(onLinkedChanged()));
//    connect(o1, SIGNAL(linkingFailed()),
//            this, SIGNAL(linkingFailed()));
//    connect(o1, SIGNAL(linkingSucceeded()),
//            this, SLOT(onLinkingSucceeded()));
//    connect(o1, SIGNAL(openBrowser(QUrl)),
//            this, SLOT(onOpenBrowser(QUrl)));
//    connect(o1, SIGNAL(closeBrowser()),
//            this, SLOT(onCloseBrowser()));

//    qDebug() << "Starting OAuth...";
//    o1->link();

//    return true;
//}

//void DiscogsPlugin::onLinkedChanged()
//{
//    qDebug() << "Link changed!";
//}

//void DiscogsPlugin::onLinkingSucceeded()
//{
//    O1* o1 = qobject_cast<O1 *>(sender());

//    QVariantMap extraTokens = o1->extraTokens();

//    if (!extraTokens.isEmpty()) {
//        //emit extraTokensReady(extraTokens);

//        qDebug() << "Extra tokens in response:";
//        foreach (QString key, extraTokens.keys()) {
//            qDebug() << "\t" << key << ":" << extraTokens.value(key).toString();
//        }
//    }
//       // emit linkingSucceeded();
//}

//void DiscogsPlugin::onOpenBrowser(const QUrl &url)
//{
//    qDebug() << "Opening browser with url" << url.toString();
//    QDesktopServices::openUrl(url);
//}

//void DiscogsPlugin::onCloseBrowser()
//{
//    qDebug() << "Closing browser";
//}

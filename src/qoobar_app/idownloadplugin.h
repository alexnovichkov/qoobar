#ifndef IDOWNLOADPLUGIN_H
#define IDOWNLOADPLUGIN_H

#include <QStringList>
#include <QIcon>
#include <QUrl>
#include "searchresults.h"

struct Request {
    Request(const QUrl &url=QUrl(), const QByteArray &data=QByteArray())
        : url(url), data(data) {}
    bool isEmpty() const {return url.isEmpty();}
    void addRawHeader(const QByteArray &header, const QByteArray &data) {
        rawHeaders.insert(header,data);
    }

//    QString request;
    QUrl url;
    QByteArray data;
    QMap<QByteArray, QByteArray> rawHeaders;
};

class IDownloadPlugin
{
public:
    virtual ~IDownloadPlugin() {}

    /*returns request for search by artist and album*/
    virtual Request queryForManualSearch(const QStringList &) = 0;
    /*returns request for search by CD in CDROM*/
    virtual Request queryForCD() = 0;
    /*returns request for search by lengths of selected files*/
    virtual Request queryForSearchByFiles(const QVector<int> &) = 0;
    /*returns request for picture downloading*/
    virtual Request queryForPicture(const QString &) = 0;
    /*returns request for album/release downloading*/
    virtual Request queryForRelease(const QString &) = 0;

    /*returns true if the plugin can perform search by artist and album*/
    virtual bool canSearchManually() = 0;
    /*returns true if the plugin can perform search by CD in CDROM*/
    virtual bool canSearchByCD() = 0;
    /*returns true if the plugin can perform search by lengths of selected files*/
    virtual bool canSearchByFiles() = 0;

    /*does actual parsing of search response into a list of albums*/
    virtual QList<SearchResult> parseResponse(const QByteArray &) = 0;
    /*does actual parsing of a release response*/
    virtual SearchResult parseRelease(const QByteArray &) = 0;
    /*returns true if the plugin needs a pause between network requests*/
    virtual bool needsPause() = 0;
    /*preferred pause between network requests in milliseconds */
    virtual int preferredPauseSize() = 0;
    /* should contain authentication info (username and password and options) required by a server*/
    virtual QMap<QString, QString> authenticationInfo() = 0;
//    virtual bool authenticate() = 0;
    /*returns a string list containing info about an album*/
    virtual QStringList releaseToList(const SearchResult &r) = 0;

    virtual QString errorString() = 0;
};

Q_DECLARE_INTERFACE(IDownloadPlugin, "qoobar.IDownloadPlugin/2.0")

#endif // IDOWNLOADPLUGIN_H

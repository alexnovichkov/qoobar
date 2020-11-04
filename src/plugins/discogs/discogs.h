#ifndef DISCOGSPLUGIN_H
#define DISCOGSPLUGIN_H

#include <QObject>
#include "idownloadplugin.h"

class O1;

class DiscogsPlugin : public QObject, IDownloadPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qoobar.IDownloadPlugin/2.0" FILE "discogs.json")
    Q_INTERFACES(IDownloadPlugin)
public:
    Request queryForManualSearch(const QStringList &);
    Request queryForCD();
    Request queryForSearchByFiles(const QVector<int> &);
    Request queryForPicture(const QString &);
    Request queryForRelease(const QString &);

    bool canSearchManually() {return true;}
    bool canSearchByCD()     {return false;}
    bool canSearchByFiles()  {return false;}

    QList<SearchResult> parseResponse(const QByteArray &);
    SearchResult parseRelease(const QByteArray &);
    bool needsPause() {return true;}
    QStringList releaseToList(const SearchResult &r);
    QString errorString() {return m_errorString;}
private:
    QString m_errorString;

    // IDownloadPlugin interface
public:
    virtual int preferredPauseSize();
    virtual QMap<QString, QString> authenticationInfo();
//    virtual bool authenticate();
//private Q_SLOTS:
//    void onLinkedChanged();
//    void onLinkingSucceeded();
//    void onOpenBrowser(const QUrl&);
//    void onCloseBrowser();
};

#endif //DISCOGSPLUGIN_H

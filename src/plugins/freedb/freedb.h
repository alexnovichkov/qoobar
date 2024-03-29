#ifndef FREEDBPLUGIN_H
#define FREEDBPLUGIN_H

#include <QObject>
#include "idownloadplugin.h"

class DiscIDHelper;

class FreedbPlugin : public QObject, IDownloadPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qoobar.IDownloadPlugin/2.0" FILE "freedb.json")
    Q_INTERFACES(IDownloadPlugin)
public:
    Request queryForManualSearch(const QStringList &);
    Request queryForCD();
    Request queryForSearchByFiles(const QVector<int> &);
    Request queryForPicture(const QString &);
    Request queryForRelease(const QString &);

    bool canSearchManually() {return true;}
    bool canSearchByCD()     {return true;}
    bool canSearchByFiles()  {return true;}

    QList<SearchResult> parseResponse(const QByteArray &);
    SearchResult parseRelease(const QByteArray &);
    bool needsPause() {return false;}
    QStringList releaseToList(const SearchResult &r);
    QString errorString() {return m_errorString;}
private:
    Request query(const QVector<int> &);
    QString m_errorString;

    // IDownloadPlugin interface
public:
    virtual int preferredPauseSize();
    virtual QMap<QString, QString> authenticationInfo();
};

#endif

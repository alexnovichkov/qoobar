#ifndef MUSICBRAINZPLUGIN_H
#define MUSICBRAINZPLUGIN_H

#include <QObject>
#include "idownloadplugin.h"

class MusicbrainzPlugin : public QObject, IDownloadPlugin
{
    Q_OBJECT
#ifdef HAVE_QT5
    Q_PLUGIN_METADATA(IID "qoobar.IDownloadPlugin/2.0" FILE "musicbrainz.json")
#endif
    Q_INTERFACES(IDownloadPlugin)
public:
#ifndef HAVE_QT5
    QString text() {return QObject::tr("Musicbrainz");}
    QString description() {return QObject::tr("Fill tags from Musicbrainz database");}
    QString key() {return "musicbrainz";}
    QString version() {return "1.0.2";}
    QIcon icon() {return QIcon();}
#endif
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

#endif //MUSICBRAINZPLUGIN_H

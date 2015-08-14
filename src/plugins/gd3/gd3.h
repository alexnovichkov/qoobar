#ifndef GD3PLUGIN_H
#define GD3PLUGIN_H

#include <QObject>
#include "idownloadplugin.h"

class GD3Plugin : public QObject, IDownloadPlugin
{
    Q_OBJECT
#ifdef HAVE_QT5
    Q_PLUGIN_METADATA(IID "qoobar.IDownloadPlugin/1.0" FILE "gd3.json")
#endif
    Q_INTERFACES(IDownloadPlugin)
public:
#ifndef HAVE_QT5
    QString text() {return QObject::tr("GD3 database");}
    QString description() {return QObject::tr("Fill tags from GD3 database");}
    QString key() {return "gd3";}
    QString version() {return "1.0.0";}
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
    Request query(const QVector<int> &lengths);
    QStringList getTOC(const QVector<int> &lengths);
    QString getRequestString(const QString &operation, const QString &argument, const QString argument1=QString());
    QString m_errorString;

#ifndef Q_OS_MAC
    typedef void *DiscId;

    typedef DiscId * (*Discid_new)();
    typedef void (*Discid_free)(DiscId *);
    typedef int (*Discid_read)(DiscId *, const char *);
    typedef int (*Discid_get_first_track_num)(DiscId *);
    typedef int (*Discid_get_last_track_num)(DiscId *);
    typedef int (*Discid_get_track_offset)(DiscId *, int);

    Discid_new discid_new;
    Discid_free discid_free;
    Discid_read discid_read;
    Discid_get_first_track_num discid_get_first_track_num;
    Discid_get_last_track_num discid_get_last_track_num;
    Discid_get_track_offset discid_get_track_offset;
#endif
};

#include <QDialog>

class QLineEdit;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    LoginDialog(const QString &login, const QString &pass, QWidget *parent = 0);
    QStringList getLoginPass();
private:
    QLineEdit *loginEdit;
    QLineEdit *passEdit;
};


#endif //GD3PLUGIN_H

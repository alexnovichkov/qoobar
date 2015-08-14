#ifndef PLAYLISTSPLUGIN_H
#define PLAYLISTSPLUGIN_H

#include <QObject>
#ifdef HAVE_QT5
#include <QtWidgets/QDialog>
#else
#include <QDialog>
#endif
#include "iqoobarplugin.h"

class PlaylistsPlugin : public QObject, IQoobarPlugin
{
    Q_OBJECT
#ifdef HAVE_QT5
    Q_PLUGIN_METADATA(IID "qoobar.IQoobarPlugin/1.1" FILE "playlists.json")
#endif
    Q_INTERFACES(IQoobarPlugin)
public:
#ifndef HAVE_QT5
    QString text();
    QString description();
    QString key();
    QString version();
    QIcon icon();
#endif
    QList<Tag> getNewTags(const QList<Tag> &oldTags);
    bool canWorkWithNoFilesSelected() {return false;}
};

class QTreeWidget;
class QCheckBox;
class QSpinBox;
class QComboBox;

class Dialog : public QDialog
{
    Q_OBJECT
public:
    Dialog(const QList<Tag> &oldTags, QWidget *parent=0);
    virtual ~Dialog();
    QList<Tag> getNewTags();
private Q_SLOTS:
    void accept();
private:
    bool createPlaylist(const QString &format, int pathType, int type);
    QList<Tag> oldTags;
    QComboBox *formatComboBox;
    QComboBox *pathsComboBox;
    QComboBox *typeComboBox;
    QString dir;
};

#endif

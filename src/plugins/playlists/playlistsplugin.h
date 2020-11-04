#ifndef PLAYLISTSPLUGIN_H
#define PLAYLISTSPLUGIN_H

#include <QObject>
#include <QtWidgets/QDialog>
#include "iqoobarplugin.h"

class PlaylistsPlugin : public QObject, IQoobarPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qoobar.IQoobarPlugin/1.1" FILE "playlists.json")
    Q_INTERFACES(IQoobarPlugin)
public:
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

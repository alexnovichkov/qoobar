#ifndef AUTONUMBERPLUGIN_H
#define AUTONUMBERPLUGIN_H

#include <QObject>
#ifdef HAVE_QT5
#include <QtWidgets/QDialog>
#else
#include <QDialog>
#endif
#include "iqoobarplugin.h"

class AutonumberPlugin : public QObject, IQoobarPlugin
{
    Q_OBJECT
#ifdef HAVE_QT5
    Q_PLUGIN_METADATA(IID "qoobar.IQoobarPlugin/1.1" FILE "autonumber.json")
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
    void updateTrackNumbers();
    void accept();
private:
    QList<Tag> oldTags;
    QList<Tag> newTags;
    QSpinBox *startNumberEdit;
    QComboBox *formatComboBox;
    QComboBox *actionComboBox;
    QTreeWidget *tree;
    QCheckBox *addTotalTracksCheckBox;
    QCheckBox *resetFolderCheckBox;
    QCheckBox *resetAlbumCheckBox;

//    int startNumber;
//    int formatNumber;
//    bool addTotalTracks;
//    bool resetFolder;
//    bool resetAlbum;
//    int albumBehavior;
};

#endif // AutonumberPLUGIN_H

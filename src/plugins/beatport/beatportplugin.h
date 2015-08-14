#ifndef BEATPORTPLUGIN_H
#define BEATPORTPLUGIN_H

#include <QObject>
#ifdef HAVE_QT5
#include <QtWidgets/QDialog>
#else
#include <QDialog>
#endif

#include <QUrl>
#include "iqoobarplugin.h"

class BeatportPlugin : public QObject, IQoobarPlugin
{
    Q_OBJECT
#ifdef HAVE_QT5
    Q_PLUGIN_METADATA(IID "qoobar.IQoobarPlugin/1.1" FILE "beatport.json")
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

class QPushButton;
class QTableWidget;
class CheckableHeaderView;
class QLabel;

struct BeatportTrack
{
    BeatportTrack() : id(0), trackNumber(0) {}
    bool isEmpty() const;
    int id;
    //QString name;
    //QString mixName; //The mix/remix name of the track.
    //QString slug; //The url-friendly name of the track.
    QString title; //The full title of the track, as always displayed on Beatport.
    QString releaseDate; //The date that the track was made available to the world.
    QString length; //The time length in minutes and seconds of the track.
    QStringList artists; //A list of all artists and remixers involved in the production of the track.
    QStringList remixers;
    QStringList genres; //A list of all genres that the track is in.
    QStringList subgenres;
    QString release; //The parent release for the track.
    QString label; //The parent label for the track.
    QByteArray image;

    int trackNumber;
    QString isrc;
    QString bpm;
    QString key;
};

class BeatportWidget : public QWidget
{
    Q_OBJECT
public:
    BeatportWidget(QWidget *parent=0);
    int currentIndex() {return current;}
    BeatportTrack currentTrack();
    void setTracks(const QList<BeatportTrack> &);
private Q_SLOTS:
    void next();
    void previous();
private:
    void update(const BeatportTrack &);
    void clear();
    int current;
    QList<BeatportTrack> tracks;
    QLabel *pictureLabel;

    QLabel *titleLabel;
    QLabel *authorsLabel;
    QLabel *infoLabel;
    QPixmap emptyPixmap;
    QAction *prevAction;
    QAction *nextAction;
};

class CoreNetworkSearch;
class QProgressBar;
class QCheckBox;

class Dialog : public QDialog
{
    Q_OBJECT
public:
    Dialog(const QList<Tag> &oldTags, QWidget *parent=0);
    QList<Tag> getNewTags();
private Q_SLOTS:
    void startSearch();
    void headerToggled(int, Qt::CheckState);
    void checkBoxToggled(int,int);
    void accept();
private:
    void updateTags(Tag &tag, BeatportWidget *w);
    void parseTrack(const QByteArray &b, int index);
    void parseFileName(QString &title,QString &artists, QString &remixer, const int &n);
    QUrl createQuery(int index);
    QList<Tag> oldTags;
    QList<Tag> newTags;
    QList<BeatportWidget *> widgets;
    QTableWidget *table;
    QPushButton *searchButton;
    CheckableHeaderView *header;
    CoreNetworkSearch *search;
    QProgressBar *progressBar;
    QCheckBox *filterCheckBox;
    QCheckBox *skipCheckBox;
};

#endif // BEATPORTPLUGIN_H

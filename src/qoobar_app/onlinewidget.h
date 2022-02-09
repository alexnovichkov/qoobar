#ifndef ONLINEWIDGET_H
#define ONLINEWIDGET_H

#include <QWidget>

#include "tagger.h"
#include "searchresults.h"

class QComboBox;
class QRadioButton;
class ClearLineEdit;
class QPushButton;
class QTreeWidget;
class QLabel;
class CoreNetworkSearch;
class QProgressIndicatorSpinning;
class ReleaseInfoWidget;
class IDownloadPlugin;
class QTreeWidgetItem;
class QAction;

class OnlineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OnlineWidget(const QList<Tag> &oldTags, QWidget *parent = nullptr);
    QList<Tag> getTags();
public Q_SLOTS:
    void startSearch();
private Q_SLOTS:
    void handleSourceComboBox(int);
    void handleManualSearchRadioButton();
    void swapArtistAndAlbum();
    void handleAlbumSelection(QTreeWidgetItem*);
    void found(const QList<SearchResult> &);
    void resultFinished(const SearchResult &,int);
    void downloadRelease(QTreeWidgetItem*);
    void saveResults();
private:
    void downloadRelease(const QString &url, const int releaseIndex);
    void setNewTag(const QString &tagValue, Tag &tag, const QString &field, int fieldID);
    IDownloadPlugin *maybeLoadPlugin(const QString &path);

    QComboBox *sourceComboBox;
    QRadioButton *manualSearchRadioButton;
    QRadioButton *cdSearchRadioButton;
    QRadioButton *filesSearchRadioButton;
    ClearLineEdit *artistEdit;
    ClearLineEdit *albumEdit;
    QPushButton *startSearchButton;
    QTreeWidget *searchResultsList;
    QLabel *networkStatusInfo;
    QLabel *networkErrorInfo;
    CoreNetworkSearch *search;
    QProgressIndicatorSpinning *progress;
    QAction *saveResultsAct;

    int currentAlbum;
    QList<SearchResult> searchResults;
    ReleaseInfoWidget *releaseInfoWidget;
    QHash<QString, IDownloadPlugin *> loadedPlugins;
    QList<Tag> oldTags;
};

#endif // ONLINEWIDGET_H

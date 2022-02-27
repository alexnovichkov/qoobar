#ifndef ONLINEWIDGET_H
#define ONLINEWIDGET_H

#include <QWidget>

#include "tagger.h"
#include "searchresults.h"
#include <QItemSelection>

class QComboBox;
class QRadioButton;
class ClearLineEdit;
class QPushButton;
class QLabel;
class CoreNetworkSearch;
class QProgressIndicatorSpinning;
class ReleaseInfoWidget;
class IDownloadPlugin;
class QTreeView;
class QAction;
class QFile;
class OnlineModel;

#include <QJsonObject>

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
    void updateManualSearchEdits();
    void swapArtistAndAlbum();
    void onTreeItemClicked(const QModelIndex&, bool force = false);
//    void onTreeItemDoubleClicked(QTreeWidgetItem*);
    void onItemsSelectionChanged(const QItemSelection&, const QItemSelection&);
    void resultFinished(const SearchResult &,int);
    void cacheResult();
    void clearCache();
    void downloadRelease();
private:
    void downloadRelease(const QString &url, const int releaseIndex);
    void setNewTag(const QString &tagValue, Tag &tag, const QString &field, int fieldID);
    void setNewTag(const QString &tagValue, Tag &tag, int fieldID);
    IDownloadPlugin *maybeLoadPlugin(const QString &path);
    void addCachedResults();

    QComboBox *sourceComboBox;
    QRadioButton *manualSearchRadioButton;
    QRadioButton *cdSearchRadioButton;
    QRadioButton *filesSearchRadioButton;
    ClearLineEdit *artistEdit;
    ClearLineEdit *albumEdit;
    QPushButton *startSearchButton;
    QTreeView *tree;
    QLabel *networkStatusInfo;
    QLabel *networkErrorInfo;
    CoreNetworkSearch *search;
    QProgressIndicatorSpinning *progress;
    QAction *saveResultsAct;
    QAction *clearCacheAct;
    QAction *downloadReleaseAct;
    OnlineModel *model;

    ReleaseInfoWidget *releaseInfoWidget;
    QHash<QString, IDownloadPlugin *> loadedPlugins;
    QList<Tag> oldTags;
};

#endif // ONLINEWIDGET_H

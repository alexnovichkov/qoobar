#ifndef ONLINEMODEL_H
#define ONLINEMODEL_H

#include <QAbstractItemModel>

#include "searchresults.h"
#include "tagger.h"

class OnlineModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit OnlineModel(QObject *parent = nullptr);
    SearchResult &item(int index) {return results[index];}
    SearchResult &item(const QModelIndex &idx);
    QList<Tag> getTags();
    void loadCachedReleases();
    void setFoundReleases(const QList<SearchResult> &found);
    void setResult(const SearchResult &r, int n);
    bool isEmpty() const;
    SearchResult currentResult() const;
    void select(int index, int cd=-1);
Q_SIGNALS:
    void cacheEmpty(bool);
    void releaseDownloaded(const SearchResult &release);
public Q_SLOTS:
    void cacheResult(const SearchResult &r);
    void clearCache();

private:
    int row(quint64 id) const;

    void downloadRelease();

    int currentAlbum = -1;
    int currentCd = -1;
    QList<SearchResult> results;

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // ONLINEMODEL_H

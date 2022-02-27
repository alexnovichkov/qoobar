#include "onlinemodel.h"
#include <QIcon>
#include "qoobarglobals.h"
#include "application.h"
#include <QMessageBox>
#include "logging.h"

OnlineModel::OnlineModel(QObject *parent)
    : QAbstractItemModel(parent)
{DD;

}

SearchResult &OnlineModel::item(const QModelIndex &idx)
{DD;
    Q_ASSERT(idx.isValid());

    if (idx.parent().isValid()) {
        Q_ASSERT(results.size()>idx.parent().row());
        return results[idx.parent().row()];
    }
    Q_ASSERT(results.size()>idx.row());
    return results[idx.row()];
}



void OnlineModel::loadCachedReleases()
{
    QList<SearchResult> releases;
    QDir dir(ApplicationPaths::cachePath());
    if (!dir.exists()) return;
    auto list = dir.entryInfoList({"*.json"});
    if (list.isEmpty()) return;

    for (auto &entry: list) {
        QFile f(entry.canonicalFilePath());
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            auto data = f.readAll();
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data, &error);
            if (doc.isNull()) {
                qDebug()<<error.errorString();
                continue;
            }
            auto o = doc.object();
            auto r = SearchResult::fromJson(o["release"].toObject());
            releases.append(r);
        }
    }

    beginResetModel();
    releases.prepend(SearchResult::emptyResult());
    results = releases;
    endResetModel();
    Q_EMIT cacheEmpty(false);
}

void OnlineModel::setFoundReleases(const QList<SearchResult> &found)
{
    beginResetModel();

    //clear non-cached releases
    auto end = std::remove_if(results.begin(), results.end(), [](const SearchResult &r){return !r.cached;});
    results.erase(end, results.cend());
    //add new found releases
    results = found + results;

    endResetModel();
}

void OnlineModel::setResult(const SearchResult &r, int n)
{
    beginResetModel();

    const QString url = results.at(n).fields.value("url");
    auto releaseInfo = results.at(n).releaseInfo;
    results.replace(n,r);
    results[n].fields.insert("url", url);
    results[n].releaseInfo = releaseInfo;

    endResetModel();
}

bool OnlineModel::isEmpty() const
{
    return results.isEmpty();
}

SearchResult OnlineModel::currentResult() const
{
    if (currentAlbum < 0 || currentAlbum >= results.size())
        return SearchResult();
    auto r = results.at(currentAlbum);
    if (currentCd <= 1 || r.cdCount <= 1)
        return r;

    auto tracks = r.tracks;
    auto end=std::remove_if(tracks.begin(), tracks.end(), [this](const Track &t){return t.cd!=this->currentCd;});
    tracks.erase(end, tracks.cend());
    r.tracks = tracks;
    return r;
}

void OnlineModel::select(int index, int cd)
{
    currentAlbum = index;
    currentCd = cd;
}

void OnlineModel::cacheResult(const SearchResult &r)
{
    auto cacheDir = QDir(ApplicationPaths::cachePath());
    if (!cacheDir.exists()) {
        bool success = cacheDir.mkpath(ApplicationPaths::cachePath());
        if (!success) return;
    }

    QFile f(ApplicationPaths::cachePath()+"/"+QUuid::createUuid().toString()+".json");
    if (f.open(QFile::Text | QFile::WriteOnly)) {
        QJsonObject o;
        o.insert("query", "");
        o.insert("release", r.toJson());
        QJsonDocument doc(o);
        f.write(doc.toJson());
    }
    else return;


    //check for other cached releases
    int count = 1;
    if (!results.contains(SearchResult::emptyResult())) {
        count++;
    }

    beginInsertRows(QModelIndex(), results.size(), results.size()+count);

    if (count == 2) results.append(SearchResult::emptyResult());
    results.append(r);
    results.last().cached = true;

    endInsertRows();
    Q_EMIT cacheEmpty(false);
}

void OnlineModel::clearCache()
{
    beginResetModel();

    const auto end = std::remove_if(results.begin(), results.end(), [](const SearchResult &r)
    {
        return r.cached;
    });
    results.erase(end, results.cend());

    endResetModel();
    Q_EMIT cacheEmpty(true);
}

//void OnlineModel::downloadAndSetRelease(const QModelIndex &idx)
//{

//}

//returns index of top level item with id
int OnlineModel::row(quint64 id) const
{DD;
    auto f = std::find_if(results.cbegin(), results.cend(),
                          [id](const SearchResult &r){
        return r.index == id;
    });
    if (f!=results.cend()) return std::abs(std::distance(f,results.cbegin()));
    return -1;
}


QModelIndex OnlineModel::index(int row, int column, const QModelIndex &parent) const
{DD;
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    quint64 index = 0;

    if (parent.isValid()) {
        //add extra 1 since cdNumber always starts with 1
        index = results.at(parent.row()).index + row+1;
    }
    else
        index = results.at(row).index;
    auto result = createIndex(row, column, index);
    return result;
}

QModelIndex OnlineModel::parent(const QModelIndex &child) const
{DD;
    QModelIndex result;
    if (!child.isValid())
        return QModelIndex();

    const quint64 id = child.internalId();
    const quint64 childRow = id % 0x100000000;
    if (childRow == 0) {//top level item
        return QModelIndex();
    }
    auto r = row(id - childRow);
    if (r == -1) result = QModelIndex();
    else result = createIndex(r, 0, id - childRow);
    return result;
}

int OnlineModel::rowCount(const QModelIndex &parent) const
{DD;
    if (parent.isValid()) {
        const auto cdCount = results.at(parent.row()).cdCount;
        return cdCount > 1 ? cdCount : 0;
    }
    return results.size();
}

int OnlineModel::columnCount(const QModelIndex &parent) const
{DD;
    Q_UNUSED(parent);
    return 1;
}

QVariant OnlineModel::data(const QModelIndex &index, int role) const
{DD;
    if (!index.isValid()) return QVariant();

    auto row = index.row();
    auto releaseRow = row;

    if (index.parent().isValid()) {
         releaseRow = index.parent().row();
         const auto &r = results.at(releaseRow);
        //One of CDs in a release
        switch (role) {
            case Qt::DisplayRole: return QString("CD %1, %2, %3")
                        .arg(row+1)
                        .arg(tr("%n tracks","", r.tracksCount(row+1)))
                        .arg(Qoobar::formatLength(r.duration(row+1)));
            default: break;
        }
    }
    else {
        const auto &r = results.at(releaseRow);
        switch (role) {
            case Qt::DisplayRole: return r.releaseInfo;
            case Qt::DecorationRole: {
                if (r.cached && !(r == SearchResult::emptyResult()))
                    return QIcon::fromTheme("edit-download");
            }
            default: break;
        }
    }
    return QVariant();
}

QVariant OnlineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && section == 0) {
        if (role == Qt::DisplayRole)
            return tr("Search results");
        if (role == Qt::TextAlignmentRole) return Qt::AlignCenter;
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags OnlineModel::flags(const QModelIndex &index) const
{DD;
    if (!index.isValid()) return QAbstractItemModel::flags(index);

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.parent().isValid())
        f |= Qt::ItemNeverHasChildren;
    else {
        if (index.row()>=results.size()) return f;
        if (results.at(index.row()).cdCount <= 1) {
            f |= Qt::ItemNeverHasChildren;
        }
    }
    return f;
}



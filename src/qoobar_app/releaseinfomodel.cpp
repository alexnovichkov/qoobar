#include "releaseinfomodel.h"
#include <QtDebug>

ReleaseInfoModel::ReleaseInfoModel(QObject *parent) : CheckableTableModel(parent)
{
    setCheckable(0, true);
}

void ReleaseInfoModel::setReleaseInfo(const SearchResult &releaseInfo, int cdNumber)
{
    beginResetModel();
    this->cdNumber = cdNumber;
    info = releaseInfo;
    if (info.cdCount > 1) {
        for (int i=info.tracks.size()-1; i>=0; --i) {
            if (cdNumber != info.tracks.at(i).cd) info.tracks.removeAt(i);
            else info.tracks[i].fields.insert("checked","true");
        }
    }
//    update();
    endResetModel();
}

int ReleaseInfoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return info.tracks.size();
}

int ReleaseInfoModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 5;
}

QVariant ReleaseInfoModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role==Qt::DisplayRole) {
        const int row = index.row();
        const int col = index.column();
        //if (row < 0 || row >= info.tracks.size()) qDebug()<<"wrong row";
        const auto &track = info.tracks.at(row);
        switch (col) {
        case 0: return track.fields.value(QSL("tracknumber"));
        case 1: return track.fields.value(QSL("title"));
        case 2: return artistsText(track.artists);
        case 3: return track.fields.value(QSL("extraData"));
        case 4: return track.fields.value(QSL("length"));
        }
    }
    return CheckableTableModel::data(index, role);
}

QVariant ReleaseInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role==Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("No.");
        case 1: return tr("Title");
        case 2: return tr("Artists");
        case 3: return tr("Comment");
        case 4: return tr("Length");
        }
    }

    return CheckableTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ReleaseInfoModel::flags(const QModelIndex &index) const
{
    return CheckableTableModel::flags(index) | Qt::ItemIsEditable;
}

bool ReleaseInfoModel::checked(int row, int column) const
{
    Q_UNUSED(column)
    if (row>=0 && row < info.tracks.size())
        return info.tracks.at(row).fields.value("checked") == "true";
    return false;
}

void ReleaseInfoModel::setChecked(int row, int column, bool checked)
{
    Q_UNUSED(column)
    if (row>=0 && row < info.tracks.size())
        info.tracks[row].fields.insert("checked", checked?"true":"false");
}

void ReleaseInfoModel::setHeaderChecked(int section, bool checked)
{
    Q_UNUSED(section);
    for (auto &track: info.tracks)
        track.fields.insert("checked", checked?"true":"false");
}

void ReleaseInfoModel::update()
{

}

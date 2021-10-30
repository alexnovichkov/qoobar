#include "importmodel.h"

#include <QColor>
#include <QClipboard>

#include "application.h"
#include "tagparser.h"
#include "placeholders.h"

ImportModel::ImportModel(QObject *parent) : QAbstractTableModel(parent)
{

}

void ImportModel::setTags(const QList<Tag> &tags)
{
    beginResetModel();
    this->tags = tags;
    newTags = tags;
    checkedRows = QVector<bool>(tags.size(), true);
    update();
    endResetModel();
}

void ImportModel::setSource(int sourceId)
{
    beginResetModel();
    this->sourceId = sourceId;
    update();
    endResetModel();
}

void ImportModel::setPattern(const QString &pattern)
{
    beginResetModel();
    this->pattern = pattern;
    update();
    endResetModel();
}


int ImportModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return tags.size();
}

int ImportModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2+map.size();
}

QVariant ImportModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const int row = index.row();
        const int col = index.column();
        switch (role) {
        case Qt::DisplayRole: {
            switch (col) {
            case 0: break;
            case 1: return tagsSource.at(row);
            default:
                if (col <= map.size()+1)
                return checkedRows.value(row, true) ? newTags.at(row).tag(map.at(col-2)) : tags.at(row).tag(map.at(col-2));
            }
            break;
        }
        case Qt::CheckStateRole: {
            if (col==0) return checkedRows.value(row, true)?Qt::Checked:Qt::Unchecked;
            break;
        }
        case Qt::ForegroundRole: {
            if (!checkedRows.value(row, true)) return QVariant(QColor(Qt::gray));
            break;
        }
        default: break;
        }
    }

    return QVariant();
}

QVariant ImportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch(section) {
            case 0: return " ";
            case 1: return tr("Source");
            default: return App->currentScheme->localizedFieldName[map.value(section-2)];
            }
        }
        else if (role==Qt::CheckStateRole && section==0) {
            const auto checked = [](bool b){return b;};
            if (std::all_of(checkedRows.cbegin(), checkedRows.cend(), checked)) return Qt::Checked;
            if (std::none_of(checkedRows.cbegin(), checkedRows.cend(), checked)) return Qt::Unchecked;
            return Qt::PartiallyChecked;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void ImportModel::update()
{
    const int count = tags.size();
    tagsSource.resize(count);

    //updating tags source
    if (sourceId == 1) {
        tagsSource = qApp->clipboard()->text().split(QSL("\n")).toVector();
        tagsSource.resize(count);
    }
    else {
        for (int i=0; i<count; ++i) {
            if (sourceId==0) {
                tagsSource[i] = QString("%1/%2").arg(tags.at(i).filePath()).arg(tags.at(i).fileName());
                if (tagsSource[i]=="/") {//dont know why it happens, but still:
                    QStringList list = tags.at(i).fullFileName().split(".");
                    if (list.size()==1) tagsSource[i] = list.first();
                    else {
                        list.takeLast();
                        tagsSource[i] = list.join(".");
                    }
                }
            }
            else tagsSource[i] = tags.at(i).tag(sourceId-2);
        }
    }

    //parsing tags source and updating model
    if (TagParser::needParsing(pattern)) {
        QString p = pattern;
        newTags = tags;
        QStringList source = tagsSource;

        //truncate pattern according to the filename length
        if (sourceId==0)
            TagParser::truncateFileNamesToShortest(p, source);

        map.clear();

        for (int i=0; i<count; ++i) {
            if (!checkedRows.at(i)) continue;

            Tag &tag = newTags[i];
            PairList parsed = TagParser::parse(source.at(i), p);
            Q_FOREACH(const StringPair &pair, parsed) {
                int id = Placeholders::placeholderToId(pair.first);
                if (id<0)
                    id = App->currentScheme->tagIDBySimplifiedName(pair.first);
                if (id >= 0) {
                    if (!map.contains(id)) map << id;
                    tag.setTag(id, pair.second);
                }
            }
        }
    }
}

Qt::ItemFlags ImportModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return QAbstractTableModel::flags(index);

    auto f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column()==0) f |= Qt::ItemIsUserCheckable;
    return f;
}


bool ImportModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && index.column()==0 && role==Qt::CheckStateRole) {
        checkedRows[index.row()] = value.toInt()==Qt::Checked;
        Q_EMIT dataChanged(index, this->index(index.row(), columnCount(QModelIndex())-1),
                           {Qt::CheckStateRole, Qt::DisplayRole, Qt::ForegroundRole});
        Q_EMIT headerDataChanged(Qt::Horizontal, 0, 0);
        return true;
    }
    return false;
}


bool ImportModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation == Qt::Vertical || section != 0)
        return QAbstractItemModel::setHeaderData(section, orientation, value, role);

    if (role == Qt::CheckStateRole) {
        Qt::CheckState state = Qt::CheckState(value.toInt());
        switch (state) {
        case Qt::Checked:
            checkedRows = QVector<bool>(tags.size(), true);
            break;
        case Qt::Unchecked:
            checkedRows = QVector<bool>(tags.size(), false);
            break;
        default: break;
        }
        Q_EMIT dataChanged(index(0,0), index(rowCount(QModelIndex())-1, columnCount(QModelIndex())-1));
        Q_EMIT headerDataChanged(orientation, section, section);
        return true;
    }

    return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

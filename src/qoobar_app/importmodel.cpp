#include "importmodel.h"

#include <QColor>
#include <QClipboard>

#include "application.h"
#include "tagparser.h"
#include "placeholders.h"

ImportModel::ImportModel(QObject *parent) : CheckableTableModel(parent)
{
    setCheckable(0, true);
    setUpdatableColumns(QVector<int>());
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
        case Qt::ForegroundRole: {
            if (!checkedRows.value(row, true)) return QVariant(QColor(Qt::gray));
            break;
        }
        default: break;
        }
    }

    return CheckableTableModel::data(index, role);
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
    }

    return CheckableTableModel::headerData(section, orientation, role);
}

void resizeStringList(QStringList &list, int newSize)
{
#if QT_VERSION >=QT_VERSION_CHECK(6,0,0)
    list.resize(newSize);
#else
    list=list.mid(0,newSize);
    while(list.size()<newSize) list << "";
#endif
}

void ImportModel::update()
{
    const int count = tags.size();
    resizeStringList(tagsSource, count);

    //updating tags source
    if (sourceId == 1) {
        tagsSource = qApp->clipboard()->text().split(QSL("\n"));
        resizeStringList(tagsSource, count);
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
    return CheckableTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool ImportModel::checked(int row, int column) const
{
    Q_UNUSED(column);
    return checkedRows.value(row, true);
}

void ImportModel::setChecked(int row, int column, bool checked)
{
    Q_UNUSED(column);
    checkedRows[row] = checked;
}

void ImportModel::setHeaderChecked(int section, bool checked)
{
    Q_UNUSED(section);
    checkedRows = QVector<bool>(tags.size(), checked);
}

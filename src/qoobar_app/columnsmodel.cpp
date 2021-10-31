#include "columnsmodel.h"

#include <QTreeView>
#include <QHeaderView>

ColumnsModel::ColumnsModel(QTreeView *tree, QObject *parent)
    : CheckableTableModel(parent), tree(tree)
{
    setCheckable(0, true);
}

int ColumnsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return tree->header()->count();
}

int ColumnsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ColumnsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::DisplayRole) {
        return tree->model()->headerData(index.row(),Qt::Horizontal);
    }
    return CheckableTableModel::data(index, role);
}

QVariant ColumnsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section==0 && orientation==Qt::Horizontal && role==Qt::DisplayRole)
        return tr("Visibility");
    return CheckableTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ColumnsModel::flags(const QModelIndex &index) const
{
    return CheckableTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool ColumnsModel::checked(int row, int column) const
{
    if (column==0) return !tree->header()->isSectionHidden(row);
    return false;
}

void ColumnsModel::setChecked(int row, int column, bool checked)
{
    if (column==0) tree->header()->setSectionHidden(row, !checked);
}

void ColumnsModel::setHeaderChecked(int section, bool checked)
{
    if (section==0) {
        for (int i=0; i<tree->header()->count(); ++i)
            tree->header()->setSectionHidden(i, !checked);
    }
}

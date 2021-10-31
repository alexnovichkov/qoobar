#include "checkabletablemodel.h"

CheckableTableModel::CheckableTableModel(QObject *parent) : QAbstractTableModel(parent)
{

}

void CheckableTableModel::setCheckable(int column, bool checkable)
{
    if (checkable) checkableColumns.insert(column);
    else checkableColumns.remove(column);
}

void CheckableTableModel::setUpdatableColumns(const QVector<int> &columns)
{
    updatableColumns = columns;
}

Qt::CheckState CheckableTableModel::headerCheckState(int section) const
{
    return headerData(section, Qt::Horizontal, Qt::CheckStateRole).value<Qt::CheckState>();
}

QVariant CheckableTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const int row = index.row();
        const int col = index.column();
        switch (role) {
        case Qt::CheckStateRole: {
            if (checkableColumns.contains(col)) return checked(row, col) ? Qt::Checked : Qt::Unchecked;
            break;
        }
        default: break;
        }
    }

    return QVariant();
}

QVariant CheckableTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role==Qt::CheckStateRole && checkableColumns.contains(section)) {
            int count = 0;
            for (int i=0; i<rowCount(); ++i) if (checked(i, section)) count++;
            if (count == rowCount()) return Qt::Checked;
            if (count == 0) return Qt::Unchecked;
            return Qt::PartiallyChecked;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags CheckableTableModel::flags(const QModelIndex &index) const
{
    if (index.isValid() && checkableColumns.contains(index.column()))
        return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;

    return QAbstractTableModel::flags(index);
}

bool CheckableTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int col = index.column();
    if (index.isValid() && checkableColumns.contains(col) && role==Qt::CheckStateRole) {
        setChecked(index.row(), col, value.toInt()==Qt::Checked);
        for (int i: qAsConst(updatableColumns)) {
            const auto idx = this->index(index.row(), i);
            Q_EMIT dataChanged(idx, idx);
        }
        if (updatableColumns.isEmpty()) {//update all columns
            const auto idx1 = this->index(index.row(), 0);
            const auto idx2 = this->index(index.row(), columnCount()-1);
            Q_EMIT dataChanged(idx1, idx2);
        }

        Q_EMIT headerDataChanged(Qt::Horizontal, col, col);
        return true;
    }
    return false;
}

bool CheckableTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && checkableColumns.contains(section) && orientation == Qt::Horizontal) {
        Qt::CheckState state = Qt::CheckState(value.toInt());
        switch (state) {
        case Qt::Checked:
            setHeaderChecked(section, true);
            break;
        case Qt::Unchecked:
            setHeaderChecked(section, false);
            break;
        default: break;
        }
        for (int i: qAsConst(updatableColumns)) {
            const auto idx1 = this->index(0, i);
            const auto idx2 = this->index(rowCount()-1, i);
            Q_EMIT dataChanged(idx1, idx2);
        }
        if (updatableColumns.isEmpty()) {
            const auto idx1 = this->index(0, 0);
            const auto idx2 = this->index(rowCount()-1, columnCount()-1);
            Q_EMIT dataChanged(idx1, idx2);
        }

        Q_EMIT headerDataChanged(orientation, section, section);
        return true;
    }

    return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

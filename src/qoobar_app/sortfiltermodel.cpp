#include "sortfiltermodel.h"
#include <QtCore>
#include "logging.h"
#include "model.h"

SortFilterModel::SortFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{DD;
//    filters.resize(MODEL_COLUMNS_COUNT);
}

//void SortFilterModel::setFilter(const QString &text, int column)
//{DD;
//    if (filters.at(column) == text) return;
//    filters[column] = text;
//    invalidateFilter();
//}

//QString SortFilterModel::filter(int column) const
//{
//    return filters.value(column);
//}


bool SortFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QVariant leftData = sourceModel()->data(source_left);
    QVariant rightData = sourceModel()->data(source_right);

    if (leftData.type() == QVariant::DateTime)
        return leftData.toDateTime() < rightData.toDateTime();
    else if (leftData.type() == QVariant::Int)
        return leftData.toInt() < rightData.toInt();
    else if (leftData.type() == QVariant::Double)
        return leftData.toDouble() < rightData.toDouble();

    return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
}


bool SortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{DD;
    Q_UNUSED(source_parent);
    Q_UNUSED(source_row);
    return true;
}

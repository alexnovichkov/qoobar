#ifndef CHECKABLETABLEMODEL_H
#define CHECKABLETABLEMODEL_H

#include <QAbstractTableModel>
#include <QSet>

/**
 * @brief The CheckableTableModel class
 * This class provides the basic functionality for checkable tables. Must be used with HeaderView
 */

class CheckableTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CheckableTableModel(QObject *parent = nullptr);
    void setCheckable(int column, bool checkable);
    void setUpdatableColumns(const QVector<int> &columns);

    //returns if the item(row, column) is checked in the implementation
    virtual bool checked(int row, int column) const = 0;
    virtual void setChecked(int row, int column, bool checked) = 0;
    virtual void setHeaderChecked(int section, bool checked) = 0;

    // QAbstractItemModel interface
public:
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;
private:
    //contains columns numbers that should be updated when checkstate of an item is changed
    QVector<int> updatableColumns;
    //contains checkable columns numbers
    QSet<int> checkableColumns;
};

#endif // CHECKABLETABLEMODEL_H

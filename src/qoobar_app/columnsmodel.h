#ifndef COLUMNSMODEL_H
#define COLUMNSMODEL_H


#include "checkabletablemodel.h"

class QTreeView;

class ColumnsModel : public CheckableTableModel
{
    Q_OBJECT
public:
    explicit ColumnsModel(QTreeView *tree, QObject *parent=0);


    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // CheckableTableModel interface
public:
    virtual bool checked(int row, int column) const override;
    virtual void setChecked(int row, int column, bool checked) override;
    virtual void setHeaderChecked(int section, bool checked) override;
private:
    QTreeView *tree;
};

#endif // COLUMNSMODEL_H

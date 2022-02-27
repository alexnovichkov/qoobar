#ifndef RELEASEINFOMODEL_H
#define RELEASEINFOMODEL_H

#include "checkabletablemodel.h"
#include "searchresults.h"

class ReleaseInfoModel : public CheckableTableModel
{
    Q_OBJECT
public:
    ReleaseInfoModel(QObject *parent=nullptr);

    void setReleaseInfo(const SearchResult &releaseInfo, int cdNumber);
    void clear();

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    // CheckableTableModel interface
public:
    virtual bool checked(int row, int column) const override;
    virtual void setChecked(int row, int column, bool checked) override;
    virtual void setHeaderChecked(int section, bool checked) override;
private:
    void update();
    SearchResult info;
    int cdNumber = 1;
};

#endif // RELEASEINFOMODEL_H

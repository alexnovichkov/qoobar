#ifndef IMPORTMODEL_H
#define IMPORTMODEL_H

#include "checkabletablemodel.h"
#include "tagger.h"

class ImportModel : public CheckableTableModel
{
    Q_OBJECT
public:
    ImportModel(QObject *parent = nullptr);
    void setTags(const QList<Tag> &tags);
    void setSource(int sourceId);
    void setPattern(const QString &pattern);
    QList<Tag> getTags() const {return newTags;}

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
//    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
//    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;
private:
    void update();
    QList<Tag> tags;
    QList<Tag> newTags;
    QStringList tagsSource;
    QString pattern;
    QVector<int> map;
    QVector<bool> checkedRows;
    int sourceId = 0; //file names by default

    // CheckableTableModel interface
public:
    virtual bool checked(int row, int column) const override;
    virtual void setChecked(int row, int column, bool checked) override;
    virtual void setHeaderChecked(int section, bool checked) override;
};


#endif // IMPORTMODEL_H

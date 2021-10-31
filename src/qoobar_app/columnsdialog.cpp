/**
 * \file columnsdialog.cpp
 * A dialog for selecting/unselecting columns of TreeWidget
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 11 Apr 2013
 *
 * Copyright (C) 2013  Alex Novichkov
 *
 * This file is part of Qoobar.
 *
 * Qoobar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Qoobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "columnsdialog.h"

#include <QtWidgets>
#include "qoobarglobals.h"
#include "application.h"
#include "checkableheaderview.h"
#include "enums.h"
#include "checkabletablemodel.h"

class ColumnsModel : public CheckableTableModel
{
public:
    explicit ColumnsModel(QTreeView *tree, QObject *parent=0)
        : CheckableTableModel(parent), tree(tree)
    {
        setCheckable(0, true);
    }


    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override
    {
        Q_UNUSED(parent)
        return tree->header()->count();
    }
    virtual int columnCount(const QModelIndex &parent) const override
    {
        Q_UNUSED(parent)
        return 1;
    }
    virtual QVariant data(const QModelIndex &index, int role) const override
    {
        if (index.isValid() && role == Qt::DisplayRole) {
            return tree->model()->headerData(index.row(),Qt::Horizontal);
        }
        return CheckableTableModel::data(index, role);
    }
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (section==0 && orientation==Qt::Horizontal && role==Qt::DisplayRole)
            return tr("Visibility");
        return CheckableTableModel::headerData(section, orientation, role);
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return CheckableTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    // CheckableTableModel interface
public:
    virtual bool checked(int row, int column) const override
    {
        if (column==0) return !tree->header()->isSectionHidden(row);
        return false;
    }
    virtual void setChecked(int row, int column, bool checked) override
    {
        if (column==0) tree->header()->setSectionHidden(row, !checked);
    }
    virtual void setHeaderChecked(int section, bool checked) override
    {
        if (section==0) {
            for (int i=0; i<tree->header()->count(); ++i)
                tree->header()->setSectionHidden(i, !checked);
        }
    }
private:
    QTreeView *tree;
};

ColumnsDialog::ColumnsDialog(QTreeView *tree) :
    QDialog(tree), header(nullptr)
{DD;
    this->tree=tree;
    setWindowTitle(tr("Qoobar - Adjust columns visibility"));

    auto *layout = new QVBoxLayout;

    //const int columnCount = tree->header()->count();

    table = new QTableView(this);
    auto model = new ColumnsModel(tree, this);
    table->setModel(model);

//    connect(table,SIGNAL(cellChanged(int,int)),SLOT(checkBoxToggled(int,int)));

    header = new HeaderView(Qt::Horizontal, table);
    table->setHorizontalHeader(header);
    table->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(table);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    layout->addWidget(buttonBox);

    setLayout(layout);
    //TODO: this->devicePixelRatio()
    resize({App->screens().first()->availableSize().width()/4,
            App->screens().first()->availableSize().height()/3});
}


//void ColumnsDialog::checkBoxToggled(int row, int column)
//{DD;
//    QTableWidgetItem *item=table->item(row,column);
//    if (!item) return;

//    int hiddenCount = tree->header()->hiddenSectionCount();

//    //do not allow hiding of all columns
//    if (item->checkState()!=Qt::Checked && tree->header()->count()-hiddenCount<=1) {
//        item->setCheckState(Qt::Checked);
//        return;
//    }

//    tree->header()->setSectionHidden(row, item->checkState()!=Qt::Checked);
//}

//void ColumnsDialog::headerToggled(int column, Qt::CheckState checked)
//{DD;
//    if (column<0 || column >= table->columnCount()) return;

//    if (checked==Qt::PartiallyChecked) return;
//    for (int i=0; i<table->rowCount(); ++i)
//        table->item(i,column)->setCheckState(checked);
//}

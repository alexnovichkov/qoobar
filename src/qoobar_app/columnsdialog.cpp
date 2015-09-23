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

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "qoobarglobals.h"
#include "application.h"
#include "checkableheaderview.h"
#include "enums.h"

ColumnsDialog::ColumnsDialog(QTreeView *tree) :
    QDialog(tree), tree(tree), header(0)
{DD;
    setWindowTitle(tr("Qoobar - Adjust columns visibility"));

    QVBoxLayout *layout = new QVBoxLayout;

    const int columnCount = tree->header()->count();

    table = new QTableWidget(columnCount, 1);
    table->setHorizontalHeaderLabels(QStringList(tr("Visibility")));

    int hidden=tree->header()->hiddenSectionCount();
    int checked = columnCount - hidden;
    for (int i = 0; i < columnCount; ++i) {
        QTableWidgetItem *item = new QTableWidgetItem(tree->model()->headerData(i,Qt::Horizontal).toString());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setCheckState(tree->header()->isSectionHidden(i)?Qt::Unchecked:Qt::Checked);
        table->setItem(i,0,item);
    }
    connect(table,SIGNAL(cellChanged(int,int)),SLOT(checkBoxToggled(int,int)));

    header = new CheckableHeaderView(Qt::Horizontal,table);
    table->setHorizontalHeader(header);
    table->horizontalHeader()->setStretchLastSection(true);
    header->setCheckable(0,true);
    header->setCheckState(0,checked==0?Qt::Unchecked:(checked==columnCount?Qt::Checked : Qt::PartiallyChecked));
    connect(header,SIGNAL(toggled(int,Qt::CheckState)),this,SLOT(headerToggled(int,Qt::CheckState)));

    layout->addWidget(table);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    layout->addWidget(buttonBox);

    setLayout(layout);
    resize(500,400);
}


void ColumnsDialog::checkBoxToggled(int row, int column)
{DD;
    QTableWidgetItem *item=table->item(row,column);
    if (!item) return;

    int hiddenCount = tree->header()->hiddenSectionCount();

    //do not allow hiding of all columns
    if (item->checkState()!=Qt::Checked && tree->header()->count()-hiddenCount<=1) {
        item->setCheckState(Qt::Checked);
        return;
    }

    tree->header()->setSectionHidden(row, item->checkState()!=Qt::Checked);

    if (!header->isSectionCheckable(column)) return;
    int checked=0;
    for (int i=0; i<table->rowCount(); ++i)
        if (table->item(i,column)->checkState()==Qt::Checked) checked++;
    if (header) {
        if (checked==0) header->setCheckState(column,Qt::Unchecked);
        else if (checked==tree->header()->count()) header->setCheckState(column,Qt::Checked);
        else header->setCheckState(column,Qt::PartiallyChecked);
    }
}

void ColumnsDialog::headerToggled(int column, Qt::CheckState checked)
{DD;
    if (column<0 || column >= table->columnCount()) return;

    if (checked==Qt::PartiallyChecked) return;
    for (int i=0; i<table->rowCount(); ++i)
        table->item(i,column)->setCheckState(checked);
}

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
#include "headerview.h"
#include "enums.h"
#include "columnsmodel.h"

ColumnsDialog::ColumnsDialog(QTreeView *tree) :
    QDialog(tree), header(nullptr)
{DD;
    this->tree=tree;
    setWindowTitle(tr("Qoobar - Adjust columns visibility"));

    auto *layout = new QVBoxLayout;

    table = new QTableView(this);
    auto model = new ColumnsModel(tree, this);
    table->setModel(model);

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

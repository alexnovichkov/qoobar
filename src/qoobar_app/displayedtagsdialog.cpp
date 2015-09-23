/**
 * \file displayedtagsdialog.cpp
 * A dialog to set tags visibility in files table and tags table.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 11 Aug 2011
 *
 * Copyright (C) 2011  Alex Novichkov
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

#include "displayedtagsdialog.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "qoobarglobals.h"
#include "application.h"
#include "enums.h"

QWidget *ComboBoxItemDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &/* option */,
                                            const QModelIndex &/* index */) const
{DD;
    QComboBox *editor = new QComboBox(parent);
    if (editor) {
        for (int i=0; i<3; ++i) editor->addItem(tagStatusByID(i));
    }
    return editor;
}

void ComboBoxItemDelegate::setEditorData(QWidget *editor,
                                         const QModelIndex &index) const
{DD;
    int row = index.model()->data(index, Qt::UserRole).toInt();

    QComboBox *box = qobject_cast<QComboBox*>(editor);
    if (box) box->setCurrentIndex(row);
}

void ComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const
{DD;
    QComboBox *box = qobject_cast<QComboBox*>(editor);
    if (box) {
        model->setData(index, box->currentIndex(), Qt::UserRole);
        model->setData(index, box->currentText(), Qt::DisplayRole);
    }
}

QString tagStatusByID(const int id)
{DD;
    switch (id) {
    case ALWAYSVISIBLE: return QObject::tr("Always visible"); break;
    case VISIBLEIFNOTEMPTY: return QObject::tr("Visible if not empty"); break;
    case HIDDEN: return QObject::tr("Hidden"); break;
    }
    return QString();
}

DisplayedTagsDialog::DisplayedTagsDialog(QWidget *parent) :
    QDialog(parent)
{DD;
    setWindowTitle(tr("Qoobar - Adjust tags visibility"));

    QVBoxLayout *layout = new QVBoxLayout;

    const int tagsCount = App->currentScheme->tagsCount();
    table = new QTableWidget(tagsCount, 1);
    table->setHorizontalHeaderLabels(QStringList(tr("Visibility")));

    table->setItemDelegate(new ComboBoxItemDelegate(this));

    for (int i = 0; i < tagsCount; ++i) {
        QTableWidgetItem *statusItem = new QTableWidgetItem(tagStatusByID(App->tagStatus.value(i)));
        statusItem->setData(Qt::UserRole, App->tagStatus.value(i));
        table->setVerticalHeaderItem(i, new QTableWidgetItem(App->currentScheme->localizedFieldName[i]));
        table->setItem(i, 0, statusItem);
    }
    table->horizontalHeader()->setStretchLastSection(true);
    connect(table,SIGNAL(cellChanged(int,int)),SLOT(indexChanged(int,int)));
    connect(table,SIGNAL(cellClicked(int,int)),this,SLOT(cellClicked(int,int)));

    layout->addWidget(table);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    layout->addWidget(buttonBox);

    setLayout(layout);
    resize(500,400);
}

void DisplayedTagsDialog::cellClicked(const int row, const int col)
{DD;
    table->editItem(table->item(row,col));
}

void DisplayedTagsDialog::indexChanged(const int row,const int column)
{DD;
    int status = table->item(row,column)->data(Qt::UserRole).toInt();
    Q_EMIT tagStatusChanged(row,status);
}

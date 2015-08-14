/**
 * \file displayedtagsdialog.h
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

#ifndef DISPLAYEDTAGSDIALOG_H
#define DISPLAYEDTAGSDIALOG_H

#include <QDialog>
#include <QItemDelegate>

class QTableWidget;

QString tagStatusByID(int id);

class DisplayedTagsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DisplayedTagsDialog(QWidget *parent = 0);
Q_SIGNALS:
    void tagStatusChanged(int tagID,int status);
private Q_SLOTS:
    void indexChanged(const int, const  int);
    void cellClicked(const int row, const int col);
private:
    QTableWidget *table;
};

class ComboBoxItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ComboBoxItemDelegate(QObject *parent = 0)
        : QItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
};

#endif // DISPLAYEDTAGSDIALOG_H

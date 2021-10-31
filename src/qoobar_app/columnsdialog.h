/**
 * \file columnsdialog.h
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

#ifndef COLUMNSDIALOG_H
#define COLUMNSDIALOG_H

#include <QDialog>

class QTreeView;
class HeaderView;
class QTableView;

class ColumnsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ColumnsDialog(QTreeView *tree);

Q_SIGNALS:
    void tagStatusChanged(int tagID,int status);
private Q_SLOTS:
//    void checkBoxToggled(int row, int column);
//    void headerToggled(int column, Qt::CheckState checked);
private:
    QTreeView *tree;
    HeaderView *header;
    QTableView *table;
};

#endif // COLUMNSDIALOG_H

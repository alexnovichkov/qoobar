/**
 * \file newtagdialog.h
 * A dialog to choose new tag name
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
 * the Free Software Foundation; either version 3 of the License, or
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

#ifndef NEWTAGDIALOG_H
#define NEWTAGDIALOG_H

#include <QDialog>

class QLineEdit;

class NewTagDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewTagDialog(QWidget *parent = 0);
    QString newTagName;
public Q_SLOTS:
    void accept();
private:
    QLineEdit *edit;
};

#endif // NEWTAGDIALOG_H

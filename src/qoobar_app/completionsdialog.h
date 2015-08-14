/**
 * \file composersdialog.h
 * A dialog where you can edit composers names.
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

#ifndef COMPOSERSDIALOG_H
#define COMPOSERSDIALOG_H

#include <QDialog>
class QPlainTextEdit;

class CompletionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CompletionsDialog(const int &tagID, QWidget *parent = 0);
public Q_SLOTS:
    void accept();
private:
    QPlainTextEdit *edit;
    int m_tagID;
};

#endif // COMPOSERSDIALOG_H

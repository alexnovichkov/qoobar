/**
 * \file schemeeditor.h
 * A dialog where you can edit tagging schemes.
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

#ifndef SCHEMEEDITOR_H
#define SCHEMEEDITOR_H

#include <QDialog>
#include <QLineEdit>
#include <QKeyEvent>

class QTableWidget;
class TaggingScheme;
class QToolButton;

class SchemeEditor : public QDialog
{
    Q_OBJECT
public:
    enum Operation {
        Modify,
        Copy
    };

    explicit SchemeEditor(const QString &file, Operation operation, QWidget *parent = 0);
    ~SchemeEditor();
    TaggingScheme *scheme;
public Q_SLOTS:
    void accept();
private Q_SLOTS:
    void addField();
    void removeField();
    void yHeader(int section);
    void updateRemoveRowAct();
private:
    QLineEdit *edit;
    QTableWidget *table;
    QString fileName;
    Operation m_operation;
    QAction *addFieldAct;
    QAction *removeFieldAct;
};

#endif

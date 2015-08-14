/**
 * \file lineedit.h
 * QLineEdit with custom completer.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 8 Dec 2009
 *
 * Copyright (C) 2009  Alex Novichkov
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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>

class QMenu;
class QSignalMapper;
class QAction;

QMenu *createOperations(QSignalMapper *, QWidget *);

class MyCompleter : public QCompleter
{
Q_OBJECT
public:
    MyCompleter(int type,QObject * parent=0);
    void update(const QString &);
private:
    QStringListModel m_model;
    int m_type;
    QStringList map;
};

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(bool useInTable=false, QWidget *parent = 0);
    void setCompleter(MyCompleter *c);
    MyCompleter *completer() const;
protected:
    void keyPressEvent(QKeyEvent *e);
    void contextMenuEvent(QContextMenuEvent *);
Q_SIGNALS:
    void tagChanged(int,bool);
private Q_SLOTS:
    void tagChosen(int);
    void tagChosen1(int);
    void insertCompletion(const QString &completion);
    void handleOperation(const QString &);
private:
    bool useInTable_;
    QMenu *createOperationsMenu();
    QList<QAction *> acts;
    QList<QAction *> acts1;
    MyCompleter *c;
    QString completionPrefix;
};

#endif // LINEEDIT_H

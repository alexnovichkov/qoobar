/**
 * \file tabwidget.h
 * QTabWidget with context menu
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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QTabBar>
class QLineEdit;

class TabBar : public QTabBar
{
    Q_OBJECT
Q_SIGNALS:
    void newTab();
    void closeTab(int index);
    void closeOtherTabs(int index);
    void renameTab(int index);

    void tabTextChanged(const QString &text);
public:
    TabBar(QWidget *parent = 0);
protected:
    void mouseDoubleClickEvent(QMouseEvent * event);
    void contextMenuEvent(QContextMenuEvent * event);
private Q_SLOTS:
    void closeTab();
    void closeOtherTabs();
    void renameTab();

    void editTabName();
private:
    int index;
    int pressedTab;
    QLineEdit *editor;
};


class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);
    void hideTabBar(bool hide=true);
Q_SIGNALS:
    void newTab();
    void renameTab(int index);
    void closeTab(int index);
    void closeOtherTabs(int index);
    void tabTextChanged(const QString &text);
private:
    TabBar *m_tabBar;
};

#endif // TABWIDGET_H

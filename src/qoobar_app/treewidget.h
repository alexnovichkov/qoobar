/**
 * \file treewidget.h
 * QTreeWidget with some slots
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

#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>
class Tag;
class Settings;
class Tab;
class QSignalMapper;
#include "tagger.h"

class TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TreeWidget(Tab *parent = 0);
    void prepareItems(const int numItems);
protected:
    void contextMenuEvent(QContextMenuEvent *);
Q_SIGNALS:
    void moveToTab(int);
    void showMP3TagsDialog();
    void sortRequested(int section, Qt::SortOrder order, int sortType);
public Q_SLOTS:
    void updateFileSaveStatus(int,bool);
    void updateRow(int row, const Tag &tag, const QVector<int> &);
    void updateRG(int row, const Tag &tag);
    void updateImage(int row, bool imageEmpty);
    void updateFileName(int index, const QString &newFileName);
    void addRows(const QList<Tag> *,bool);
//    void deleteRows(const QVector<int> &rows);
    void deleteRow(int index);
    void clearRows();
    void synchronizeLine(int curRow);
    void retranslateUi();
    void editingDone();
    void resetHeader();

private Q_SLOTS:
    void updateTreeHeaderGeometry();
    void adjustDisplayedTags();
    void toggleFilesColumn(int,int);
    void sortIndicatorChanged(int section, Qt::SortOrder order);
private:
    void markRow(int row,const QFont &f);
    void updateSelection(int select);
    QIcon saveIcon;
    QIcon replayGainIcon;
    QIcon imageIcon;
    QFont uFont;
    QFont bFont;
    int currentLine;
    QSignalMapper *mapper;
    Tab *tab;
    QAction *showAct;
};

#endif // TREEWIDGET_H

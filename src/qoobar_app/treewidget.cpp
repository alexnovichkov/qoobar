/**
 * \file treewidget.cpp
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

#include "treewidget.h"
#include "qoobarglobals.h"
#include "application.h"
#include "columnsdialog.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "tab.h"
#include "tagsreaderwriter.h"
#include "enums.h"
#include "mainwindow.h"

int LENGTH = TAGSCOUNT;
int FILENAME = TAGSCOUNT+1;
int SAVEICON = TAGSCOUNT+2;
int REPLAYGAIN = TAGSCOUNT+3;
int IMAGE = TAGSCOUNT+4;

const int COLUMNCOUNT = TAGSCOUNT+5;

TreeWidget::TreeWidget(Tab *parent) :
    QTreeWidget(parent)
{DD;
    tab=parent;
    saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
    replayGainIcon = QIcon(App->iconThemeIcon("replaygain.png"));
    imageIcon = QIcon(App->iconThemeIcon("image.png"));
    setMouseTracking(true);
    setRootIsDecorated(false);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(true);

#ifdef Q_OS_MAC
    setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    //setAttribute(Qt::WA_MacShowFocusRect, false);
    setAutoFillBackground(true);
#endif

    showAct = new QAction(tr("Show/hide columns..."), this);
    connect(showAct,SIGNAL(triggered()),this,SLOT(adjustDisplayedTags()));

    resetHeader();

    if (!App->columns160.isEmpty()) {
        header()->restoreState(App->columns160);
    }
    connect(App,SIGNAL(settingsCleared()),SLOT(resetHeader()));

    uFont=font();
    bFont=uFont;
    bFont.setBold(true);
    currentLine=-1;
    mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(int)), this, SIGNAL(moveToTab(int)));
}

void TreeWidget::retranslateUi()
{DD;
    for (int i=0; i<TAGSCOUNT; ++i)
        headerItem()->setText(i, App->currentScheme->localizedFieldName[i]);
    headerItem()->setText(LENGTH,tr("Length"));
    headerItem()->setText(TRACKNUMBER,tr("No."));
    headerItem()->setText(FILENAME,tr("File"));
    headerItem()->setText(REPLAYGAIN,tr("RG"));
    headerItem()->setText(IMAGE,tr("Img"));
    showAct->setText(tr("Show/hide columns..."));
}

void TreeWidget::adjustDisplayedTags()
{DD;
    ColumnsDialog dialog(this);
    dialog.exec();
}

void TreeWidget::markRow(int row,const QFont &f)
{DD;
    if (row<0) return;

    QTreeWidgetItem *item=topLevelItem(row);
    if (item) {
        for (int i=0; i<item->columnCount(); ++i)
            item->setFont(i,f);
    }
}

void TreeWidget::editingDone()
{DD;
    markRow(currentLine,uFont);
}

void TreeWidget::resetHeader()
{DD;
    QHeaderView *h = new QHeaderView(Qt::Horizontal);

    setHeader(h);
    header()->setHidden(false);
    setHeaderLabels(QVector<QString>(COLUMNCOUNT,QString()).toList());

    //by default the last column containing the "save" icon is placed in the first position.
    header()->moveSection(TRACKNUMBER,0);//Track number is in the second pos.
    header()->moveSection(FILENAME,1);
    header()->moveSection(SAVEICON,0);

    header()->setSortIndicatorShown(true);

#ifdef Q_OS_LINUX
    header()->setMinimumSectionSize(fontMetrics().width(QSL("5555")));
#endif
    header()->SETSECTIONRESIZEMODE(TRACKNUMBER, QHeaderView::ResizeToContents); //No.
    header()->SETSECTIONRESIZEMODE(LENGTH, QHeaderView::ResizeToContents); //length
    header()->SETSECTIONRESIZEMODE(SAVEICON, QHeaderView::ResizeToContents); //saved
    header()->SETSECTIONRESIZEMODE(REPLAYGAIN, QHeaderView::ResizeToContents); //saved
    header()->SETSECTIONRESIZEMODE(IMAGE, QHeaderView::ResizeToContents); //saved
    header()->setStretchLastSection(false);
    header()->setContextMenuPolicy(Qt::ActionsContextMenu);
    header()->SETSECTIONSCLICKABLE(true);
    connect(header(),SIGNAL(geometriesChanged()),SLOT(updateTreeHeaderGeometry()));
    connect(header(),SIGNAL(sectionMoved(int,int,int)),SLOT(updateTreeHeaderGeometry()));
    connect(header(),SIGNAL(sectionResized(int,int,int)),SLOT(updateTreeHeaderGeometry()));
    connect(header(),SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),SLOT(sortIndicatorChanged(int,Qt::SortOrder)));

    header()->addAction(showAct);

    retranslateUi();
}

void TreeWidget::synchronizeLine(int curRow)
{DD;
    markRow(curRow, bFont);
    markRow(currentLine, uFont);
    currentLine=curRow;
    scrollToItem(topLevelItem(curRow), QAbstractItemView::PositionAtCenter);
}

void TreeWidget::toggleFilesColumn(int column,int status)
{DD;
    header()->setSectionHidden(column,status==HIDDEN);
}

void TreeWidget::sortIndicatorChanged(int section, Qt::SortOrder order)
{DD;
    int sortType = SortString;
    if (section==LENGTH) sortType = SortTime;
    Q_EMIT sortRequested(section, order, sortType);
}

void TreeWidget::updateTreeHeaderGeometry()
{DD;
    App->columns160=header()->saveState();
}

void TreeWidget::updateFileSaveStatus(int row, bool wasChanged)
{DD;
    const bool wC = !topLevelItem(row)->icon(SAVEICON).isNull();
    if (wC == wasChanged) return;
    topLevelItem(row)->setIcon(SAVEICON,wasChanged ? saveIcon : QIcon());
}

void TreeWidget::updateRow(int row, const Tag &tag, const QVector<int> &rows)
{DD;
    if (row<0 || row>=topLevelItemCount()) return;

    if (rows.isEmpty()) {
        for (int j=0; j<TAGSCOUNT; ++j)
            topLevelItem(row)->setText(j,tag.tag(j).simplified());
    }
    else {
        Q_FOREACH (const int &j, rows) {
            if (j<TAGSCOUNT) topLevelItem(row)->setText(j,tag.tag(j).simplified());
        }
    }
    //topLevelItem(row)->setIcon(REPLAYGAIN,tag.replayGainInfoIsEmpty() ? QIcon():replayGainIcon);
    //updateFileSaveStatus(row,tag.wasChanged());
}

void TreeWidget::updateRG(int row, const Tag &tag)
{DD;
    if (row<0 || row>=topLevelItemCount()) return;
    const bool wC = topLevelItem(row)->icon(REPLAYGAIN).isNull();
    if (wC == tag.replayGainInfoIsEmpty()) return;
    topLevelItem(row)->setIcon(REPLAYGAIN,tag.replayGainInfoIsEmpty() ? QIcon():replayGainIcon);
}

void TreeWidget::updateImage(int row, bool imageEmpty)
{DD;
    if (row<0 || row>=topLevelItemCount()) return;
    const bool wC = topLevelItem(row)->icon(IMAGE).isNull();
    if (wC == imageEmpty) return;
    topLevelItem(row)->setIcon(IMAGE,imageEmpty ? QIcon():imageIcon);
}

void TreeWidget::updateFileName(int index, const QString &newFileName)
{DD;
    topLevelItem(index)->setText(FILENAME, newFileName);
}

QTreeWidgetItem *getItem(const Tag &tag, bool &hasReadOnly)
{DD;
    hasReadOnly |= tag.readOnly();
    QStringList list=QVector<QString>(COLUMNCOUNT,QString()).toList();
    for (int i=0; i<TAGSCOUNT; ++i)
        list[i] = tag.tag(i).simplified();
    list[LENGTH]=Qoobar::formatLength(tag.length());
    list[FILENAME]=tag.fileNameExt();

    QTreeWidgetItem *item = new QTreeWidgetItem(list);
    if (tag.readOnly()) {
        item->setForeground(FILENAME, Qt::gray);
    }

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setIcon(FILENAME,QIcon(tag.icon()));
    item->setIcon(SAVEICON,tag.wasChanged()?qApp->style()->standardIcon(QStyle::SP_DialogSaveButton):QIcon());
    item->setIcon(REPLAYGAIN,tag.replayGainInfoIsEmpty() ? QIcon():QIcon(App->iconThemeIcon("replaygain.png")));
    item->setIcon(IMAGE,tag.imageIsEmpty() ? QIcon():QIcon(App->iconThemeIcon("image.png")));
    return item;
}

void TreeWidget::addRows(const QList<Tag> *tags, bool update)
{DD;
    QList<QTreeWidgetItem *> list;
    bool hasReadOnly = false;
    Q_FOREACH (const Tag &tag, *tags) list << getItem(tag, hasReadOnly);
    addTopLevelItems(list);
    if (update) updateSelection(tags->size());
    if (hasReadOnly) {
        warningMessage(tab->win, QSL("Qoobar"),tr("Some read-only files were added,\n"
                                              "all changes in them will not be saved!"));
    }
}

//void TreeWidget::deleteRows(const QVector<int> &rows)
//{DD
//    blockSignals(true);
//    const int count=rows.size()-1;
//    for (int i = count; i>=0; --i) {
//        delete takeTopLevelItem(rows.at(i));
//    }
//    blockSignals(false);
//}

void TreeWidget::deleteRow(int index)
{DD;
    blockSignals(true);
    delete takeTopLevelItem(index);
    blockSignals(false);
}

void TreeWidget::clearRows()
{DD;
    blockSignals(true);
    clear();
    blockSignals(false);
}

void TreeWidget::updateSelection(int select)
{DD;
    if (select<=0) return;
    if (select<100) {
        blockSignals(true);
        clearSelection();
        const int count = topLevelItemCount();
        for (int i=count-select; i<count-1; ++i)
            topLevelItem(i)->setSelected(true);
        blockSignals(false);
        topLevelItem(count-1)->setSelected(true);
    }
}

void TreeWidget::contextMenuEvent(QContextMenuEvent *event)
{DD;
    QMenu *menu = new QMenu(this);
    menu->insertActions(0,this->actions());

    if (tab) {
        QAction *moveToTabAct = new QAction(tr("Move to the tab..."),menu);
        moveToTabAct->setEnabled(this->selectionModel()->hasSelection());
        QMenu *moveToTabMenu = new QMenu(this);
        QMap<int, QString> tabNames = tab->allTabsNames();
        tabNames.insert(-1,tr("New tab"));
        Q_FOREACH (int tabID, tabNames.keys()) {
            QAction *a = new QAction(tabNames.value(tabID),moveToTabMenu);
            moveToTabMenu->insertAction(0,a);
            connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
            mapper->setMapping(a, tabID);
        }
        moveToTabAct->setMenu(moveToTabMenu);
        menu->insertAction(0,moveToTabAct);

        if (tab->allSelectedFilesAreMP3()) {
            QAction *a = new QAction(tr("MP3 tag types..."),menu);
            connect(a, SIGNAL(triggered()), this, SIGNAL(showMP3TagsDialog()));
            menu->insertAction(0,a);
        }
    }

    menu->exec(event->globalPos());
    menu->deleteLater();
}


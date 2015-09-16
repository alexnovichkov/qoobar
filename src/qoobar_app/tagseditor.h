/**
 * \file tagseditor.h
 * Editing tags dialog.
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

#ifndef TAGSEDITOR_H
#define TAGSEDITOR_H

#include <QDialog>
class QTableWidget;
class QLineEdit;
class QStringList;
class LineEdit;

class QTabWidget;
class QPlainTextEdit;
class Tag;
class LegendButton;
class QScrollArea;
class QSignalMapper;
class QToolButton;
class SearchPanel;
class SectionDelegateHighlighter;
class Model;

#include <QIcon>
#include <QToolButton>

class TagsEditDialog : public QDialog
{
Q_OBJECT

public:
    TagsEditDialog(int,const QString &,
               const QStringList &,
               const QString &,bool,
               QWidget *parent = 0);
    ~TagsEditDialog();
    QStringList getList();
    void setModel(Model *model);
Q_SIGNALS:
    void rowSelected(int currentRow);
    void tagsSent(int, const QStringList &);
public Q_SLOTS:
    void accept();
private Q_SLOTS:
    void spread(const QString &pattern);
    void cellClicked(int,int);
    void currentCellChanged(int);
    void insertFromCharsList();
    void insertLegend(const QString &);
    void toggleCharsWidget();
    void handleSentTag(int tagID, bool fromTable);
    void handleOperation(const QString &type);
    void collectTags();
    void collectTags(int tagID);
    void tableCellSelectionChanged();
    void delegateDidFinishEditing();

    void startSearch();
    void find(bool forward);
    void replace();
    void replaceAndFind();
    void replaceAll();
    void showHelp();
private:
    QTableWidget *table;
    SectionDelegateHighlighter *highlighter;

    LegendButton *legendButton;
    LineEdit *edit;
    int count;

    QPlainTextEdit *pedit;
    bool plain;
    int m_type;

    QScrollArea *scroll;
    QAction *toggleCharsAct;

    QIcon showIcon;
    QIcon hideIcon;

    SearchPanel *searchPanel;

    QStringList newValues;
    Model *model;
};

#endif


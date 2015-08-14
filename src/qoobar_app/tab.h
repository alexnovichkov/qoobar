/**
 * \file tab.h
 * Tab contains all widgets within a single tab.
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
#ifndef TAB_H
#define TAB_H

#include <QWidget>
#include <QIcon>
#include <QTreeWidget>
#include <QUndoStack>
#include <QItemDelegate>
#include "tagger.h"

class QTableWidgetItem;
class QTreeWidgetItem;
class QLabel;
class ClickableLabel;
class QPushButton;
class QToolButton;
class MainWindow;
class Settings;
class QUndoCommand;
class QDropEvent;
//class TreeWidget;
class TreeView;
class TableWidget;
class QSplitter;
class ImageBox;
class QSignalMapper;
//class DelayedExecutionTimer;
class ChangeFilesHelper;
struct BufferItem;
class IQoobarPlugin;
class Model;
class SearchBar;

struct BufferItem {
    BufferItem(int r,const QString &h, const QStringList &v)
        : row(r), header(h), values(v) {}
    int row;
    QString header;
    QStringList values;
};

class Tab : public QWidget
{
Q_OBJECT
public:
    explicit Tab(MainWindow *parent = 0);
    virtual ~Tab();
    void addFileNames(const QStringList &, bool clearBefore=false);
    void moveFiles(const QList<Tag> &filesToMove);

    void setTags(const QVector<int> &, const QList<Tag> &,const QVector<int> &);
    bool maybeSave(const QString &);
    bool allFilesSaved() const;
    void setTabNumber(int n) {tabnumber=n;}
    int tabNumber() {return tabnumber;}
    bool filesSelected();
    bool isEmpty();
    void setStatus();
    void retranslateUi();
    void doPlugin(IQoobarPlugin *);

    //void setTableUpdateMode(bool useDelayedUpdate);
    void updateDirView(bool visible);
    QMap<int, QString> allTabsNames();
    bool allSelectedFilesAreMP3() const;

    QUndoStack *undoStack();
    Model *model;
    MainWindow *win;

    bool wasRenamed;
Q_SIGNALS:
    void selectionChanged(bool);
    void filesChanged(bool);
    void bufferChanged(bool);
    void tagsSelectionChanged(bool tagsSelected);
    void filesCountChanged(int);
    void totalLengthChanged(int);
    void moveFilesToTab(int,const QList<Tag>&);
    void updateStatusBar(const Tag &);
public Q_SLOTS:
    void addFiles(int, bool);

    void copy();
    void paste();
    void cut();
    void newTag();
    void rereadTags();
    void renameFiles();
    void delFiles();
    void delAllFiles();
    void fill();
    void removeAllTags();
    void saveTags();
    void play();
    void handleCutCopy();
    void replaygain();

    void startSearch();
private Q_SLOTS:
    void filesSelectionChanged();
    void cellChanged(QTableWidgetItem *);
    void moveUp();
    void moveDown();
    void editCell();
    void pasteFromClipboard();
    void copyToClipboard();
    void remove();
    void onItemClicked(const QModelIndex &index);
    void updateStatusBar(const QModelIndex &index);
    void updateImageBox();
    void tagsChanged(int,const QStringList &);
    void saveSplitterState();
    void handleOperation(const QString &);
    void changeImage(const CoverImage &,const QString &);
    void collectTags();
    void updateTagsTableRow(int row, const QStringList &list);
    void moveToTab(int);
    void showMP3TagsDialog();
    void showMessage(int type,const QString &text);
    void sortColumn(int column, Qt::SortOrder order, int sortType);
private:
//    void updateFilesTable(const QVector<int> &, const QVector<int> &);
    void updateTagsTable(const QVector<int> & rows);


    void updateTags(QList<Tag> &oldTags, QList<Tag> &newTags, const QString &motivation);
    void delFiles(bool silently);
    QStringList getTags(int);
    void moveItems(bool up);
    void rename(const QStringList &);

    /*!
     * \brief saveWithProgress
     * \return list of indexes of successfully saved files
     */
    QVector<int> saveWithProgress();

    static QList<BufferItem> internalBuffer;

    TableWidget *table;
    //TreeWidget *tree;
    TreeView *tree;


    QAction *moveUpAct;
    QAction *moveDownAct;

    QAction *editAct;
    QAction *multilineEditAct;
    QAction *removeAct;
    QAction *operationsAct;

    QAction *collectAct;

    SearchBar *searchBar;


    ImageBox *imageBox;

    QSignalMapper *operationsMapper;
    QUndoStack *undoStack_;
    int tabnumber;
};

#endif // TAB_H

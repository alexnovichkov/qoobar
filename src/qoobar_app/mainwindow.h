/**
 * \file mainwindow.h
 * GUI class.
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QHash>
#include <QTimer>

class QAction;
class QLabel;
class TabWidget;
class Tab;
class QMenu;
class QUndoGroup;
class Tag;
class StatusBar;

class QFileSystemModel;
class QTreeView;
class QSplitter;

class SparkleUpdater;
class Toolbar;

struct Act {
    const char *key;
    const char *text;
    const char *tooltip;
    const char *shortText;
    const char *slot;
    const char *shortcut;
    const QKeySequence::StandardKey standardShortcut;
    const char *icon;
    const char *tabSlot;
};

struct Menu {
    const char *key;
    const char *text;
    const char *actions;
};

class IQoobarPlugin;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    QHash<QString, QAction *> actions;
    QList<QAction *> pluginsActions;
    QMap<int, QString> allTabsNames();
#ifdef Q_OS_WIN
    int allTabsSaved();
#endif
protected Q_SLOTS:
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    void closeEvent(QCloseEvent *);
private Q_SLOTS:
    void initRest();
    void addDir();
    void addFiles();
    void addFromDirViewWithSubfolders();
    void addFromDirView();
    void showHelp();
    void checkUpdates();
    void showSettingsDialog();
    void saveSplitterState();

    void closeTab(int);
    void closeCurrentTab();
    void closeOtherTabs(int);
    void closeOtherTabs();
    void newTab();
    void renameTab(int);
    void renameTab();
    void changeCurrentTab(int);
    void onTabTextChanged();

    void showAboutDialog();
    void showSplitDialog();
    void retranslateUi();
    void aboutQt();
    void saveAll();

    void moveFilesToTab(int tabID, const QList<Tag> &filesToMove);

    void onSelectionChanged(bool);
    void onFilesChanged(bool filesChanged);
    void onBufferChanged(bool bufferIsEmpty);
    void onClipboardChanged();
    void onTagsSelectionChanged(bool tagsSelected);
    void onFilesCountChanged(int count);
    void onPluginTriggered(const QString &);
//    void setTableUpdateMode(bool useDelayedUpdate);

    void searchFiles();
    void setAsTreeRoot();
private:
    void createActions();
    void createMenus();
    void createPluginsMenu();
    void retranslateActions();

    void handleArgs();
    void addDir(const QString &dir, bool withSubfolders, bool clearBefore);
    void init();
    void updateTabText(bool filesChanged, int index);
    void createUndoRedoActs();
    void createNewTab(bool quick);

    SparkleUpdater *updater;

    static const Act actionsDescr[];
    static const Menu menusDescr[];
    QHash<QString, QMenu *> menus;

    QHash<QString, IQoobarPlugin *> loadedPlugins;

    QUndoGroup *undoGroup;
    Toolbar *filesToolBar;

    TabWidget *tabWidget;
    Tab *currentTab;
    StatusBar *statusBar_;

    QFileSystemModel *dirModel;
    QTreeView *dirView;

    QSplitter *sp;

    QAction *undoAct;
    QAction *redoAct;

    QAction *separator;
    QAction *menuSeparator;
};

#endif

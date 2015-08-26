/**
 * \file mainwindow.cpp
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

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "mainwindow.h"
#include "application.h"
#include "settingsdialog.h"
#include "tab.h"
#include "splitdialog.h"
#include "tabwidget.h"
#include "statusbar.h"

#include "iqoobarplugin.h"
#include "taglib/toolkit/taglib.h"
#include "qoobarglobals.h"
#include "macsplitter.h"

#ifdef Q_OS_WIN32
#include "windows.h"
#endif

#include "sparkleupdater.h"
#include "model.h"
#include "mactoolbar.h"

// We mark these strings as translatable so not to deploy all of qt_xx.qm files
static const char *macapp_strings[] = {
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Services"),
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Hide %1"),
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Hide Others"),
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Show All"),
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Preferences..."),
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Quit %1"),
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "About %1")
};

const Act MainWindow::actionsDescr[] = {
    {"addDir", QT_TR_NOOP("&Add folder..."), QT_TR_NOOP("Add folder..."), QT_TR_NOOP("Add folder"),
     SLOT(addDir()), 0, QKeySequence::Open, "folder-new",0},
    {"addFiles", QT_TR_NOOP("Add &files..."), QT_TR_NOOP("Add files..."), QT_TR_NOOP("Add files"),
     SLOT(addFiles()), QT_TR_NOOP("Shift+Ctrl+O"), QKeySequence::UnknownKey, "document-new",0},
    {"save", QT_TR_NOOP("&Save current tab"), QT_TR_NOOP("Save current tab"), QT_TR_NOOP("Save"),
     0, 0, QKeySequence::Save, "document-save",SLOT(saveTags())},
    {"saveAll", QT_TR_NOOP("Save all &tabs"), QT_TR_NOOP("Save all tabs"), QT_TR_NOOP("Save all"),
     SLOT(saveAll()), 0, QKeySequence::UnknownKey, 0,0},
    {"rename", QT_TR_NOOP("&Rename files..."), QT_TR_NOOP("Rename files..."), QT_TR_NOOP("Rename"),
     0, QT_TR_NOOP("Ctrl+R"), QKeySequence::UnknownKey, "document-save-as",SLOT(renameFiles())},
    {"fill", QT_TR_NOOP("&Fill tags..."), QT_TR_NOOP("Fill tags..."), QT_TR_NOOP("Fill"),
     0, QT_TR_NOOP("Ctrl+F"), QKeySequence::UnknownKey, "fill", SLOT(fill())},
    {"rereadTags", QT_TR_NOOP("&Reread tags"), QT_TR_NOOP("Reread tags"), QT_TR_NOOP("Reread"),
     0, 0, QKeySequence::UnknownKey, "view-refresh", SLOT(rereadTags())},
    {"help", QT_TR_NOOP("&Help"), QT_TR_NOOP("Help on Qoobar"), QT_TR_NOOP("Help"),
     SLOT(showHelp()), 0, QKeySequence::HelpContents, "help-contents",0},
    {"about", QT_TR_NOOP("&About Qoobar"), QT_TR_NOOP("About Qoobar"), QT_TR_NOOP("About"),
     SLOT(showAboutDialog()), 0, QKeySequence::UnknownKey, "help-about",0},
    {"aboutQt", QT_TR_NOOP("About &Qt"), QT_TR_NOOP("About Qt"), QT_TR_NOOP("About Qt"),
     SLOT(aboutQt()), 0, QKeySequence::UnknownKey, 0,0},
    {"settings", QT_TR_NOOP("&Settings..."), QT_TR_NOOP("Settings..."), QT_TR_NOOP("Settings"),
     SLOT(showSettingsDialog()), 0, QKeySequence::UnknownKey, "preferences-system",0},
    {"delFiles", QT_TR_NOOP("Re&move from the list"), QT_TR_NOOP("Remove from the list"), QT_TR_NOOP("Remove"),
     0, QT_TR_NOOP("Delete"), QKeySequence::UnknownKey, "edit-delete",SLOT(delFiles())},
    {"delAllFiles", QT_TR_NOOP("C&lear the list"), QT_TR_NOOP("Clear the list"), QT_TR_NOOP("Remove"),
     0, QT_TR_NOOP("Ctrl+Delete"), QKeySequence::UnknownKey, "edit-clear",SLOT(delAllFiles())},
    {"play", QT_TR_NOOP("&Play selected"), QT_TR_NOOP("Play selected"), QT_TR_NOOP("Play"),
     0, QT_TR_NOOP("Ctrl+P"), QKeySequence::UnknownKey, "media-playback-start",SLOT(play())},
    {"removeTags", QT_TR_NOOP("Clear all tags"), QT_TR_NOOP("Clear all tags"), QT_TR_NOOP("Clear"),
     0, 0, QKeySequence::UnknownKey, "Delete", SLOT(removeAllTags())},
    {"cut", QT_TR_NOOP("Cu&t"), QT_TR_NOOP("Cut"), QT_TR_NOOP("Cut"),
     0, 0, QKeySequence::Cut, "edit-cut",SLOT(cut())},
    {"copy", QT_TR_NOOP("&Copy"), QT_TR_NOOP("Copy"), QT_TR_NOOP("Copy"),
     0, 0, QKeySequence::Copy, "edit-copy",SLOT(copy())},
    {"paste", QT_TR_NOOP("&Paste"), QT_TR_NOOP("Paste"), QT_TR_NOOP("Paste"),
     0, 0, QKeySequence::Paste, "edit-paste",SLOT(paste())},
    {"copyToClipboard", QT_TR_NOOP("Copy to clipboard"), QT_TR_NOOP("Copy to clipboard"), QT_TR_NOOP("Copy to clipboard"),
     0, QT_TR_NOOP("Shift+Ctrl+C"), QKeySequence::UnknownKey, 0,SLOT(copyToClipboard())},
    {"pasteFromClipboard", QT_TR_NOOP("Paste from clipboard"), QT_TR_NOOP("Paste from clipboard"), QT_TR_NOOP("Paste from clipboard"),
     0, QT_TR_NOOP("Shift+Ctrl+V"), QKeySequence::UnknownKey, 0,SLOT(pasteFromClipboard())},
    {"newTag", QT_TR_NOOP("&Add new tag..."), QT_TR_NOOP("Add new tag..."), QT_TR_NOOP("New tag"),
     0, 0, QKeySequence::New, "list-add",SLOT(newTag())},
    {"split", QT_TR_NOOP("Split &disc..."), QT_TR_NOOP("Split disc..."), QT_TR_NOOP("Split"),
     SLOT(showSplitDialog()), 0, QKeySequence::UnknownKey, "media-import-audio-cd",0},
    {"exit", QT_TR_NOOP("E&xit"), QT_TR_NOOP("Exit"), QT_TR_NOOP("Exit"),
     SLOT(close()), QT_TR_NOOP("Ctrl+Q"), QKeySequence::UnknownKey, "application-exit",0},
    {"newTab", QT_TR_NOOP("&New tab"), QT_TR_NOOP("New tab"), QT_TR_NOOP("New tab"),
     SLOT(newTab()), 0, QKeySequence::AddTab, "newTab", 0},
    {"closeTab", QT_TR_NOOP("&Close the tab"), QT_TR_NOOP("Close the tab"), QT_TR_NOOP("Close tab"),
     SLOT(closeCurrentTab()), 0, QKeySequence::Close, 0,0},
    {"closeOtherTabs", QT_TR_NOOP("Close &other tabs"), QT_TR_NOOP("Close other tabs"), QT_TR_NOOP("Close other tabs"),
     SLOT(closeOtherTabs()), 0, QKeySequence::UnknownKey, 0,0},
    {"renameTab", QT_TR_NOOP("&Rename the tab..."), QT_TR_NOOP("Rename the tab..."), QT_TR_NOOP("Rename tab"),
     SLOT(renameTab()), 0, QKeySequence::UnknownKey, 0,0},
    {"replaygain",QT_TR_NOOP("ReplayGain info..."), QT_TR_NOOP("Edit ReplayGain Info..."), QT_TR_NOOP("ReplayGain"),
     0, 0, QKeySequence::UnknownKey, 0, SLOT(replaygain())},

    {"addFromDirView", QT_TR_NOOP("List this folder"), QT_TR_NOOP("List this folder"), QT_TR_NOOP("List folder"),
     SLOT(addFromDirView()), 0, QKeySequence::UnknownKey, 0,0},
    {"addFromDirViewWithSubfolders", QT_TR_NOOP("List this folder with all subfolders"),
     QT_TR_NOOP("List this folder with all subfolders"), QT_TR_NOOP("List this folder with all subfolders"),
     SLOT(addFromDirViewWithSubfolders()), 0, QKeySequence::UnknownKey, 0,0},
    {"checkUpdates", QT_TR_NOOP("&Check for updates..."), QT_TR_NOOP("Check for updates..."), QT_TR_NOOP("Check updates"),
     SLOT(checkUpdates()), 0, QKeySequence::UnknownKey, 0,0},
    {"searchFiles", QT_TR_NOOP("S&earch files..."), QT_TR_NOOP("Search files..."), QT_TR_NOOP("Search"),
     SLOT(searchFiles()), QT_TR_NOOP("Shift+Ctrl+F"), QKeySequence::UnknownKey, 0,0},
    {0,0,0,0,0,0,QKeySequence::UnknownKey,0,0}
};

const Menu MainWindow::menusDescr[] = {
    {"file", QT_TR_NOOP("&File"), "addDir,addFiles,split,searchFiles,,save,"
                                   "saveAll,rereadTags,delFiles,"
                                   "delAllFiles,,exit"},
    {"edit", QT_TR_NOOP("&Edit"), ",cut,copy,paste,copyToClipboard,"
                                   "pasteFromClipboard,,removeTags,"
                                   "newTag,,settings"},
    {"tools", QT_TR_NOOP("&Tools"), "rename,fill,play,replaygain,"},
    {"tabs", QT_TR_NOOP("Tab&s"), "newTab,closeTab,closeOtherTabs,renameTab"},
    {"help", QT_TR_NOOP("&Help"),"help,about,aboutQt,,checkUpdates"},
    {0,0,0}
};

MainWindow::MainWindow()
{DD;
    init();
    show();

    updater = new SparkleUpdater(this);

    QTimer::singleShot(0,this,SLOT(initRest()));
}

MainWindow::~MainWindow()
{DD

}

void MainWindow::init()
{DD
    setWindowTitle(tr("Qoobar - Tag editor for classical music")+QSL("[*]"));
    setAcceptDrops(true);

    undoGroup = new QUndoGroup(this);
    undoAct=0;
    redoAct=0;
    currentTab=0;

    createActions();
    //menu
    createMenus();
    menuSeparator = menus[QSL("edit")]->actions().first();

    const QStringList toolBarActs =
            QString("addDir,addFiles,split,,save,rereadTags,,*,rename,fill,"
                    "removeTags,delAllFiles,,play,,cut,copy,paste,newTag").split(QSL(","));

    createUndoRedoActs();

    filesToolBar = new Toolbar(this);
    Q_FOREACH (const QString &a,toolBarActs) {
        if (a.isEmpty()) filesToolBar->addSeparator();
        else if (a=="*") {
            filesToolBar->addAction(undoAct);
            filesToolBar->addAction(redoAct);
        }
        else {
            if (actions.value(a)) {
                filesToolBar->addAction(actions.value(a));
            }
        }
    }
    filesToolBar->attachToWindow(this);


//========================================================================
    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    dirModel->setReadOnly(true);
    dirView = new QTreeView(this);
    dirView->setSortingEnabled(false);

    dirView->setModel(dirModel);
    dirView->setColumnHidden(1,true);
    dirView->setColumnHidden(2,true);
    dirView->setColumnHidden(3,true);
    dirView->setHeaderHidden(true);
    dirView->setContextMenuPolicy(Qt::ActionsContextMenu);
#ifdef Q_OS_MAC
    dirView->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
#endif

#ifndef Q_OS_WIN
    dirView->expandToDepth(100);
#endif
    connect(dirView,SIGNAL(pressed(QModelIndex)),SLOT(addFromDirView()));

    dirView->addAction(actions[QSL("addFromDirView")]);
    dirView->addAction(actions[QSL("addFromDirViewWithSubfolders")]);

    tabWidget = new TabWidget(this);
    connect(tabWidget,SIGNAL(newTab()),this, SLOT(newTab()));
    connect(tabWidget,SIGNAL(closeTab(int)),this, SLOT(closeTab(int)));
    connect(tabWidget,SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(tabWidget,SIGNAL(renameTab(int)),this, SLOT(renameTab(int)));
    connect(tabWidget,SIGNAL(currentChanged(int)),SLOT(changeCurrentTab(int)));
    connect(tabWidget,SIGNAL(tabTextChanged(QString)),SLOT(onTabTextChanged()));

#ifndef Q_OS_MAC
    sp = new QSplitter(Qt::Horizontal,this);
#else
    sp = new MacSplitter(Qt::Horizontal,this);
    sp->setChildrenCollapsible(false);
#endif
    sp->setOpaqueResize(false);
    sp->addWidget(dirView);
    sp->addWidget(tabWidget);
#ifndef Q_OS_MAC
    sp->setContentsMargins(5,5,5,5);
#endif
    //sp->setStretchFactor(0,1);
    //sp->setStretchFactor(1,3);

    connect(sp,SIGNAL(splitterMoved(int,int)),SLOT(saveSplitterState()));

    setCentralWidget(sp);

    statusBar_ = new StatusBar(this);
    setStatusBar(statusBar_);

    connect(qApp->clipboard(),SIGNAL(dataChanged()),SLOT(onClipboardChanged()));
}

void MainWindow::initRest()
{DD
    App->readGuiSettings();
    App->loadPlugins();
    createPluginsMenu();
    if (App->geometry.isEmpty())
        resize(1000,800);
    else
        restoreGeometry(App->geometry);
    createNewTab(true);
    tabWidget->hideTabBar(App->hideTabBar);

    if (!App->dirSplitterState.isEmpty())
        sp->restoreState(App->dirSplitterState);
    else {
        QList<int> sizes = sp->sizes();
        int total = sizes.at(0)+sizes.last();
        sizes[0]=200;
        sizes[1]=total-200;
        sp->setSizes(sizes);
    }


#ifdef Q_OS_WIN
    dirModel->setRootPath(QSL(""));
#else
    dirModel->setRootPath(QSL("/"));
#endif
    QString dir=App->lastTreeDirectory;
    if (dir.isEmpty()) {
        dir=App->lastDirectory;
        App->lastTreeDirectory=dir;
    }
    dirView->expand(dirModel->index(dir,0));
    dirView->scrollTo(dirModel->index(dir,0),QAbstractItemView::PositionAtCenter);
    dirView->setVisible(App->showDirView);

    handleArgs();
    retranslateUi();
}

void MainWindow::createMenus()
{DD
    int i=0;
    while (menusDescr[i].key) {
        QMenu *menu = menuBar()->addMenu(tr(menusDescr[i].text));
        menu->setProperty("key",i);
        QStringList acts = QString(menusDescr[i].actions).split(QSL(","));
        Q_FOREACH (const QString &text,acts) {
            if (text.isEmpty()) menu->addSeparator();
            else menu->addAction(actions[text]);
        }
        menus.insert(menusDescr[i].key,menu);
        i++;
    }
}

void MainWindow::createUndoRedoActs()
{DD
    delete undoAct;
    delete redoAct;
    undoAct = undoGroup->createUndoAction(this,tr("&Undo"));
    undoAct->setShortcutContext(Qt::ApplicationShortcut);
#if QT_VERSION >= 0x040600
    undoAct->setIcon(QIcon::fromTheme(QSL("edit-undo"),QIcon(QSL(":/src/icons/edit-undo.png"))));
#else
    undoAct->setIcon(QIcon(QSL(":/src/icons/edit-undo.png")));
#endif
    undoAct->setShortcut(QKeySequence(QKeySequence::Undo).toString());
    undoAct->setProperty("shortDescr",tr("Undo"));

    redoAct = undoGroup->createRedoAction(this,tr("&Redo"));
    redoAct->setShortcutContext(Qt::ApplicationShortcut);
#if QT_VERSION >= 0x040600
    redoAct->setIcon(QIcon::fromTheme(QSL("edit-redo"),QIcon(QSL(":/src/icons/edit-redo.png"))));
#else
    redoAct->setIcon(QIcon(QSL(":/src/icons/edit-redo.png")));
#endif
    redoAct->setShortcut(QKeySequence(QKeySequence::Redo).toString());
    redoAct->setProperty("shortDescr",tr("Redo"));

    menus[QSL("edit")]->insertAction(menuSeparator,undoAct);
    menus[QSL("edit")]->insertAction(menuSeparator,redoAct);
}

void MainWindow::retranslateUi()
{DD
    setWindowTitle(tr("Qoobar - Tag editor for classical music")+QSL("[*]"));

    App->currentScheme->retranslateUI();
    retranslateActions();

    Q_FOREACH (QMenu *m, menus) {
        int key = m->property("key").toInt();
        m->setTitle(tr(menusDescr[key].text));
    }

    for (int i=0; i<tabWidget->count(); ++i) {
        Tab *tab=qobject_cast<Tab *>(tabWidget->widget(i));
        if (tab) {
            tab->retranslateUi();
            if (!tab->wasRenamed)
                tabWidget->setTabText(i,tr("Tab %1").arg(tab->tabNumber()));
            updateTabText(!tab->allFilesSaved(),i);
        }
    }
}

void MainWindow::createActions()
{DD
    int i=0;
    while (actionsDescr[i].key) {
        QAction *a = new QAction(this);
        a->setProperty("key",i);

        if (actionsDescr[i].slot) connect(a,SIGNAL(triggered()),this,actionsDescr[i].slot);
        if (actionsDescr[i].icon) {
#if QT_VERSION >= 0x040600
            QIcon themeIcon=QIcon::fromTheme(actionsDescr[i].icon);
            if (themeIcon.isNull())
                themeIcon=QIcon(QString(":/src/icons/%1.png").arg(actionsDescr[i].icon));
            a->setIcon(themeIcon);
#else
            a->setIcon(QIcon(QString(":/src/icons/%1.png").arg(actionsDescr[i].icon)));

#endif
        }
        a->setText(tr(actionsDescr[i].text));
        a->setProperty("shortDescr",tr(actionsDescr[i].shortText));
        if (actionsDescr[i].standardShortcut>0)
            a->setShortcut(actionsDescr[i].standardShortcut);
        if (actionsDescr[i].shortcut) {
            a->setShortcut(QString(actionsDescr[i].shortcut));
        }
        if (a->shortcut().isEmpty()) a->setToolTip(tr(actionsDescr[i].tooltip));
        else a->setToolTip(QString("%1 <font color=gray size=-1>%2</font>")
                              .arg(tr(actionsDescr[i].tooltip))
                              .arg(a->shortcut().toString()));
        actions.insert(actionsDescr[i].key,a);
        i++;
    }

    actions[QSL("help")]->setShortcutContext(Qt::ApplicationShortcut);
    actions[QSL("delFiles")]->setShortcutContext(Qt::WidgetShortcut);
    actions[QSL("cut")]->setEnabled(false);
    actions[QSL("copy")]->setEnabled(false);
    actions[QSL("paste")]->setEnabled(false);
    actions[QSL("copyToClipboard")]->setEnabled(false);
#ifndef Q_OS_LINUX
    actions[QSL("rereadTags")]->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
#endif
    actions[QSL("about")]->setMenuRole(QAction::AboutRole);
    actions[QSL("aboutQt")]->setMenuRole(QAction::AboutQtRole);
    actions[QSL("settings")]->setMenuRole(QAction::PreferencesRole);
    actions[QSL("exit")]->setMenuRole(QAction::QuitRole);
    actions[QSL("checkUpdates")]->setMenuRole(QAction::ApplicationSpecificRole);
}


void MainWindow::retranslateActions()
{DD;
    Q_FOREACH(const QString &key,actions.keys()) {
        QAction *a = actions[key];
        if (a) {
            int index=a->property("key").toInt();
            a->setText(tr(actionsDescr[index].text));
            if (actionsDescr[index].shortcut) {
                a->setShortcut(tr(actionsDescr[index].shortcut));
                a->setToolTip(QString("%1 <font color=gray size=-1>%2</font>")
                                  .arg(tr(actionsDescr[index].tooltip))
                                  .arg(a->shortcut().toString()));
            }
            else a->setToolTip(tr(actionsDescr[index].tooltip));
            a->setProperty("shortDescr",tr(actionsDescr[index].shortText));
        }
    }
    if (undoAct) {
        undoAct->setText(tr("&Undo"));
        undoAct->setProperty("shortDescr",tr("Undo"));
    }
    if (redoAct) {
        redoAct->setText(tr("&Redo"));
        redoAct->setProperty("shortDescr",tr("Redo"));
    }

    filesToolBar->retranslateUI();

#ifdef HAVE_QT5
    Q_FOREACH (QAction *a, pluginsActions) {
        QJsonObject metaData = App->plugins.at(a->property("id").toInt());
        QString text = metaData.value(QSL("text")).toObject().value(App->langID).toString();
        if (text.isEmpty())
            text = metaData.value(QSL("text")).toObject().value(QSL("default")).toString();
        a->setText(text);
    }
#else
    Q_FOREACH (QAction *a, pluginsActions) {
        QString key=a->property("key").toString();
        a->setText(App->plugins.value(key)->text());
    }
#endif
}

void MainWindow::saveAll()
{DD
    for (int i=0; i<tabWidget->count(); ++i) {
        Tab *tab=qobject_cast<Tab *>(tabWidget->widget(i));
        if (tab) {
            tab->saveTags();
            updateTabText(!tab->allFilesSaved(),i);
        }
    }
}

void MainWindow::showSplitDialog()
{DD
    SplitDialog dialog(this);
    if (dialog.exec())
        currentTab->addFileNames(dialog.getNewFiles());
}

void MainWindow::saveSplitterState() /*SLOT*/
{DD
    QSplitter *sp=qobject_cast<QSplitter *>(sender());
    if (sp)
        App->dirSplitterState=sp->saveState();
}

void MainWindow::createNewTab(bool quick)
{DD;
    static int sequenceNumber=1;
    Tab *tab=new Tab(this);
    connect(tab,SIGNAL(selectionChanged(bool)),SLOT(onSelectionChanged(bool)));
    connect(tab,SIGNAL(filesChanged(bool)),SLOT(onFilesChanged(bool)));
    connect(tab,SIGNAL(bufferChanged(bool)),SLOT(onBufferChanged(bool)));
    connect(tab,SIGNAL(tagsSelectionChanged(bool)),SLOT(onTagsSelectionChanged(bool)));
    connect(tab,SIGNAL(filesCountChanged(int)),SLOT(onFilesCountChanged(int)));
    connect(tab,SIGNAL(moveFilesToTab(int,QList<Tag>)),SLOT(moveFilesToTab(int,QList<Tag>)));
    connect(tab,SIGNAL(updateStatusBar(Tag)),statusBar_,SLOT(update(Tag)));
    connect(tab,SIGNAL(totalLengthChanged(int)),statusBar_,SLOT(updateTotalLength(int)));

    int i=tabWidget->addTab(tab,tr("Tab %1").arg(sequenceNumber));
    tab->setTabNumber(sequenceNumber);
    tabWidget->setCurrentIndex(i);
    undoGroup->addStack(tab->undoStack());
    tab->undoStack()->setActive();
    sequenceNumber++;
    if (!quick) tab->retranslateUi();
    if (tabWidget->count()>1)
        tabWidget->hideTabBar(false);
}

void MainWindow::newTab()
{DD;
    createNewTab(false);
}

void MainWindow::changeCurrentTab(int currentIndex)
{DD
    if (currentIndex<0) return;
    Tab *tab = qobject_cast<Tab *>(tabWidget->currentWidget());

    if (!tab) return;
    currentTab = tab;

    onFilesChanged(!tab->allFilesSaved());
    onSelectionChanged(tab->filesSelected());
    actions[QSL("delAllFiles")]->setEnabled(!tab->isEmpty());
    filesToolBar->updateEnabled(actions[QSL("delAllFiles")]);
    tab->handleCutCopy();
    tab->undoStack()->setActive();

    int i=0;
    while (actionsDescr[i].key) {
        if (actionsDescr[i].tabSlot) {
            actions[actionsDescr[i].key]->disconnect();
            connect(actions[actionsDescr[i].key],SIGNAL(triggered()),tab,actionsDescr[i].tabSlot);
        }
        i++;
    }
}

void MainWindow::onTabTextChanged()
{DD
    Tab *tab=qobject_cast<Tab *>(tabWidget->widget(tabWidget->currentIndex()));
    if (!tab) return;

    tab->wasRenamed = true;
    updateTabText(!tab->allFilesSaved(),tabWidget->currentIndex());
}

void MainWindow::aboutQt()
{DD
    qApp->aboutQt();
}

void MainWindow::showAboutDialog()
{DD
    QString about=tr("<b>Qoobar, a Simple Tag Editor</b><br>released under the GPL 3"
                           "<br>Version: %1<br>Copyright 2009-2015 Alex Novichkov"
                           "<p>Web site: <a href=http://qoobar.sourceforge.net>http://qoobar.sourceforge.net</a>\n"
                           "<br>E-mail: <a href=mailto:novichkov.qoobar@gmail.com>novichkov.qoobar@gmail.com</a>"
                           "<br><hr>")
            .arg(QOOBAR_VERSION);

    // Libraries
    QString notInstalled = QString(" - <font color=red>%1</font>").arg(tr("not installed"));
    QString taglib=QString("TagLib %1.%2.%3")
                   .arg(TAGLIB_MAJOR_VERSION)
                   .arg(TAGLIB_MINOR_VERSION)
                   .arg(TAGLIB_PATCH_VERSION);

    QString libdiscid=QSL("libdiscid");
#ifndef Q_OS_MAC
    if (App->discidLibraryPath.isEmpty()) libdiscid.append(notInstalled);
#endif

    QString libraries=QString("%1<ul><li>Qt %2</li><li>%3</li><li>%4</li></ul>")
            .arg(tr("<b>Qoobar uses:</b><br><b>Libraries:</b>"))
            .arg(qVersion())
            .arg(taglib)
            .arg(libdiscid);

    QMessageBox::about(this, tr("&About Qoobar"), about + libraries);
}


void MainWindow::addFiles()
{DD
    QStringList filesToAdd = QFileDialog::getOpenFileNames(this,
        tr("Qoobar - adding files"), App->lastDirectory,
        tr("audio files (%1)").arg(Qoobar::musicalFilters().join(QSL(" "))));
    currentTab->addFileNames(filesToAdd);
}

void MainWindow::addDir()
{DD
    QString directory = QFileDialog::getExistingDirectory(this,
                                                          tr("Qoobar - adding folder"),
                                                          App->lastDirectory,
                                                          QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    if (directory.isEmpty()) return;
    App->lastDirectory=directory;
#ifdef Q_OS_LINUX
    QDir dir(directory);
    if (dir.cdUp())
        App->lastDirectory=dir.absolutePath();
#endif
    addDir(directory, true, false);
}

void MainWindow::addFromDirViewWithSubfolders() /*SLOT*/
{DD;
    App->lastTreeDirectory = dirModel->filePath(dirView->currentIndex());
    addDir(App->lastTreeDirectory, true, true);
}

void MainWindow::addFromDirView() /*SLOT*/
{DD
    if (App->mouseButtons() & Qt::RightButton) return;
    App->lastTreeDirectory = dirModel->filePath(dirView->currentIndex());
    addDir(App->lastTreeDirectory, false, true);
}

void MainWindow::addDir(const QString &dir, bool withSubfolders, bool clearBefore)
{DD
    if (dir.isEmpty()) return;

    QStringList filesToAdd;
    Qoobar::processDir(dir,filesToAdd, withSubfolders);
    currentTab->addFileNames(filesToAdd, clearBefore);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{DD
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{DD
    if (event->mimeData()->hasUrls()) {
        QStringList filters=Qoobar::musicalFilters();
        QList<QUrl> urlList = event->mimeData()->urls();
        QStringList filesToAdd;
        Q_FOREACH (const QUrl &url, urlList) {
            QString s=url.toLocalFile();
            QFileInfo f(s);
            if (f.isDir() || filters.contains(QString("*.%1").arg(f.suffix().toLower())))
                Qoobar::processDir(s,filesToAdd,true);
            else if (f.isFile() && f.suffix().toLower()=="cue") {
                SplitDialog dialog(this);
                dialog.setCueFile(s);
                if (dialog.exec())
                    filesToAdd.append(dialog.getNewFiles());
            }
        }
        currentTab->addFileNames(filesToAdd);
        event->acceptProposedAction();
    }
}

void MainWindow::handleArgs()
{DD
    currentTab->addFileNames(App->filesNames);
}

void MainWindow::closeEvent(QCloseEvent *event)
{DD
    for (int i=0; i<tabWidget->count(); ++i) {
        Tab *tab=qobject_cast<Tab *>(tabWidget->widget(i));
        if (tab) {
            if (tab->allFilesSaved()) continue;
            tabWidget->setCurrentIndex(i);
            QString tabText=tabWidget->tabText(i);
            if (tabText.endsWith(QLS("*"))) tabText.chop(1);
            if (!tab->maybeSave(tabText)) {
                event->ignore();
                return;
            }
        }
    }

    App->geometry=saveGeometry();
    event->accept();
}

#ifdef Q_OS_WIN
int MainWindow::allTabsSaved()
{
    for (int i=0; i<tabWidget->count(); ++i) {
        Tab *tab=qobject_cast<Tab *>(tabWidget->widget(i));
        if (tab) {
            if (!tab->allFilesSaved()) return FALSE;
        }
    }

    return TRUE;
}
#endif

void MainWindow::onSelectionChanged(bool filesSelected)
{DD
    actions[QSL("rereadTags")]->setEnabled(filesSelected);
    actions[QSL("rename")]->setEnabled(filesSelected);
    actions[QSL("fill")]->setEnabled(filesSelected);
    actions[QSL("delFiles")]->setEnabled(filesSelected);
    actions[QSL("removeTags")]->setEnabled(filesSelected);
    actions[QSL("play")]->setEnabled(filesSelected);
    actions[QSL("newTag")]->setEnabled(filesSelected);
    actions[QSL("replaygain")]->setEnabled(filesSelected);
    filesToolBar->updateEnabled(actions[QSL("rereadTags")]);
    filesToolBar->updateEnabled(actions[QSL("rename")]);
    filesToolBar->updateEnabled(actions[QSL("fill")]);
    filesToolBar->updateEnabled(actions[QSL("delFiles")]);
    filesToolBar->updateEnabled(actions[QSL("removeTags")]);
    filesToolBar->updateEnabled(actions[QSL("play")]);
    filesToolBar->updateEnabled(actions[QSL("newTag")]);
    filesToolBar->updateEnabled(actions[QSL("replaygain")]);
}

void MainWindow::onFilesCountChanged(int count)
{DD
    actions[QSL("delAllFiles")]->setEnabled(count>0);
    filesToolBar->updateEnabled(actions[QSL("delAllFiles")]);
}

void MainWindow::onFilesChanged(bool filesChanged)
{DD
    actions[QSL("save")]->setEnabled(filesChanged);
    filesToolBar->updateEnabled(actions[QSL("save")]);

    updateTabText(filesChanged,tabWidget->currentIndex());
    setWindowModified(filesChanged);
}

void MainWindow::updateTabText(bool filesChanged, int index)
{DD
    if (index<0 || index>=tabWidget->count()) return;
    QString s=tabWidget->tabText(index);
    Tab *tab=qobject_cast<Tab *>(tabWidget->widget(index));
    if (tab) {
        if (filesChanged) {
            if(!s.endsWith(QLS("*")))
                s.append("*");
        }
        else
            if (s.endsWith(QLS("*")))
                s.chop(1);
    }
    tabWidget->setTabText(index,s);
}

void MainWindow::onBufferChanged(bool bufferIsEmpty)
{DD
    actions[QSL("paste")]->setDisabled(bufferIsEmpty);
    filesToolBar->updateEnabled(actions[QSL("paste")]);
}

void MainWindow::onClipboardChanged()
{DD
    if (currentTab)
        actions[QSL("pasteFromClipboard")]->setDisabled(qApp->clipboard()->text().isEmpty());
    else
        actions[QSL("pasteFromClipboard")]->setDisabled(true);
    filesToolBar->updateEnabled(actions[QSL("pasteFromClipboard")]);
}

void MainWindow::onTagsSelectionChanged(bool tagsSelected)
{DD
    actions[QSL("cut")]->setEnabled(tagsSelected);
    actions[QSL("copy")]->setEnabled(tagsSelected);
    actions[QSL("copyToClipboard")]->setEnabled(tagsSelected);
    //actions[QSL("paste")]->setEnabled(tagsSelected);
    actions[QSL("pasteFromClipboard")]->setEnabled(tagsSelected && !qApp->clipboard()->text().isEmpty());

    filesToolBar->updateEnabled(actions[QSL("cut")]);
    filesToolBar->updateEnabled(actions[QSL("copy")]);
    filesToolBar->updateEnabled(actions[QSL("copyToClipboard")]);
    filesToolBar->updateEnabled(actions[QSL("pasteFromClipboard")]);
}

void MainWindow::closeCurrentTab()
{DD
    closeTab(tabWidget->currentIndex());
}

void MainWindow::closeTab(int i)
{DD
    if (tabWidget->count()==1) return;
    int index=i;
    if (i<0) index=tabWidget->currentIndex();

    tabWidget->setCurrentIndex(index);
    QString tabText=tabWidget->tabText(index);
    if (tabText.endsWith(QLS("*"))) tabText.chop(1);

    Tab *tab = qobject_cast<Tab *>(tabWidget->widget(index));
    if (tab && tab->maybeSave(tabText)) {
        undoGroup->removeStack(tab->undoStack());
        tabWidget->removeTab(index);
        tab->deleteLater();
    }
    tabWidget->hideTabBar(App->hideTabBar && tabWidget->count() <= 1);
}

void MainWindow::closeOtherTabs()
{DD
    closeOtherTabs(tabWidget->currentIndex());
}

void MainWindow::closeOtherTabs(int index)
{DD
    int count=tabWidget->count();
    if (count<=1) return;

    for (int i = count - 1; i > index; --i)
        closeTab(i);
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}

void MainWindow::renameTab()
{DD
    renameTab(tabWidget->currentIndex());
}

void MainWindow::renameTab(int i)
{DD
    Tab *tab=qobject_cast<Tab *>(tabWidget->widget(i));
    if (!tab) return;
    QString oldTabName = tabWidget->tabText(i);
    if (oldTabName.endsWith(QLS("*"))) oldTabName.chop(1);
    QString newTabName=QInputDialog::getText(this,
                                             tr("Qoobar - renaming the tab"),
                                             tr("Set a tab name"),
                                             QLineEdit::Normal,
                                             oldTabName);

    if (!newTabName.isEmpty()) {
        tabWidget->setTabText(i,newTabName);
        tab->wasRenamed = true;
        updateTabText(!tab->allFilesSaved(),i);
    }
}

void MainWindow::showHelp()
{DD;
    Qoobar::showHelp();
}

void MainWindow::checkUpdates()
{DD;
    updater->checkNow(false);
}

void MainWindow::showSettingsDialog()
{DD;
    SettingsDialog settingsDialog(this);
    connect(&settingsDialog,SIGNAL(retranslate()),SLOT(retranslateUi()));
    settingsDialog.exec();
    if (!App->useUndo)
        Q_FOREACH (QUndoStack *stack,undoGroup->stacks()) stack->clear();
    tabWidget->hideTabBar(App->hideTabBar && tabWidget->count() <= 1);

    dirView->setVisible(App->showDirView);
}

QMap<int, QString> MainWindow::allTabsNames()
{DD;
    QMap<int, QString> names;
    for (int i=0; i<tabWidget->count(); ++i) {
        if (i==tabWidget->currentIndex()) continue;
        QString tabName = tabWidget->tabText(i);
        if (tabName.endsWith(QLS("*"))) tabName.chop(1);
        names.insert(i,tabName);
    }
    return names;
}

void MainWindow::moveFilesToTab(int tabID, const QList<Tag> &filesToMove)
{DD
    if (tabID==-1) {//into new tab
        newTab();
        tabID = tabWidget->currentIndex();
    }
    Tab *tab = qobject_cast<Tab *>(tabWidget->widget(tabID));
    if (tab) {
        tab->moveFiles(filesToMove);
    }
}

void MainWindow::createPluginsMenu()
{DD
    QMenu *toolsMenu = menus[QSL("tools")];
    if (!App->plugins.isEmpty())
        toolsMenu->addSeparator();

    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(QString)), this, SLOT(onPluginTriggered(QString)));

#ifdef HAVE_QT5
    for (int it = 0; it < App->plugins.size(); ++it) {
        QJsonObject metaData = App->plugins.at(it);
        QString pluginInterface=metaData.value(QSL("interface")).toString();
        if (pluginInterface=="IQoobarPlugin") {
            QIcon icon(metaData.value(QSL("icon")).toString());
            QString text = metaData.value(QSL("text")).toObject().value(App->langID).toString();
            if (text.isEmpty())
                text = metaData.value(QSL("text")).toObject().value(QSL("default")).toString();
            QString key = metaData.value(QSL("path")).toString();
            QAction *a = new QAction(icon, text,this);
            a->setProperty("id", it);
            connect(a,SIGNAL(triggered()),mapper,SLOT(map()));
            mapper->setMapping(a, key);
            toolsMenu->insertAction(0,a);
            pluginsActions << a;
        }
    }
#else
    QMap<QString, IQoobarPlugin *>::const_iterator  it;
    for (it = App->plugins.constBegin(); it != App->plugins.constEnd(); ++it) {
        QString key=it.key();
        QAction *a = new QAction(it.value()->icon(), it.value()->text(),this);
        a->setProperty("key",key);
        connect(a,SIGNAL(triggered()),mapper,SLOT(map()));
        mapper->setMapping(a,key);
        toolsMenu->insertAction(0,a);
        pluginsActions << a;
    }
#endif
}

void MainWindow::onPluginTriggered(const QString &pluginKey)
{DD
#ifdef HAVE_QT5
    IQoobarPlugin *plugin = loadedPlugins.value(pluginKey, 0);
    if (!plugin) {
        QPluginLoader loader(pluginKey);
        QObject *o = loader.instance();
        if (o) plugin = qobject_cast<IQoobarPlugin *>(o);
        loadedPlugins.insert(pluginKey, plugin);
    }
#else
    IQoobarPlugin *plugin = App->plugins.value(pluginKey);
#endif

    if (!plugin) return;
    currentTab->doPlugin(plugin);
}

void MainWindow::searchFiles()
{DD;
    if (!currentTab) return;
    if (!currentTab->isEmpty())
        newTab();

    currentTab->startSearch();
}



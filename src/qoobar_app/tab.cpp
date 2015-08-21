/**
 * \file tab.cpp
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
#include "tab.h"
#include "tablewidget.h"
#include "application.h"
#include "mainwindow.h"
#include "filesrenamer.h"
#include "tagsfiller.h"
#include "tagseditor.h"
#include "lineedit.h"
#include "undoactions.h"
#include "qoobarglobals.h"
#include "imagebox.h"
#include "treeview.h"
#include "tablewidget.h"
#include "newtagdialog.h"
#include "macsplitter.h"
#include "replaygaindialog.h"
#include "mp3tagsdialog.h"
#include "iqoobarplugin.h"
#include "taskbarprogress.h"

#ifdef HAVE_QT5
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>
#else
#include <QtGui>
#endif
#include "enums.h"
#include "stringroutines.h"

#include "model.h"
#include "searchbar.h"

QList<BufferItem> Tab::internalBuffer = QList<BufferItem>();

Tab::Tab(MainWindow *parent) : QWidget(parent)
{DD;
    undoStack_ = new QUndoStack(this);
    wasRenamed = false;
    win = parent;
    searchBar = new SearchBar(this);



    //**************************************************************************
    table = new TableWidget(this);
    connect(table,SIGNAL(itemChanged(QTableWidgetItem *)),SLOT(cellChanged(QTableWidgetItem *)));
    connect(table,SIGNAL(cellClicked()),SLOT(editCell()));
    connect(table,SIGNAL(itemSelectionChanged()),SLOT(handleCutCopy()));
    connect(table,SIGNAL(tagsSent(int,QStringList)),SLOT(tagsChanged(int,QStringList)));
    //=====================================================
    imageBox=new ImageBox(this);
    connect(imageBox,SIGNAL(imageChanged(CoverImage,QString)),SLOT(changeImage(CoverImage,QString)));
    //=======================================================
    model = new Model(this);



    tree = new TreeView(this);
    tree->setModel(model);
    tree->resetHeader();
    connect(tree->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),SLOT(filesSelectionChanged()));
    connect(tree,SIGNAL(entered(const QModelIndex&)),SLOT(updateStatusBar(const QModelIndex&)));
    connect(tree,SIGNAL(clicked(const QModelIndex&)),SLOT(onItemClicked(const QModelIndex&)));
    connect(tree,SIGNAL(moveToTab(int)),this,SLOT(moveToTab(int)));
    connect(tree,SIGNAL(showMP3TagsDialog()),SLOT(showMP3TagsDialog()));
    connect(tree,SIGNAL(sortRequested(int,Qt::SortOrder,int)),SLOT(sortColumn(int,Qt::SortOrder,int)));

    connect(model,SIGNAL(message(int,QString)),SLOT(showMessage(int,QString)));
    connect(model,SIGNAL(filesAdded(int,bool)),SLOT(addFiles(int,bool)));
    connect(model,SIGNAL(modelChanged(bool)), SIGNAL(filesChanged(bool)));
    connect(model,SIGNAL(filesCountChanged(int)), SIGNAL(filesCountChanged(int)));
    connect(model,SIGNAL(selectionCleared()), table, SLOT(clearTable()));
    connect(model,SIGNAL(selectionCleared()), this, SLOT(updateImageBox()));
    connect(model,SIGNAL(tagValueChanged(int,QString,int)), SLOT(updateTableRow(int,QString)));



    connect(searchBar, SIGNAL(addFile(Tag)),model,SLOT(addFile(Tag)));
    connect(searchBar, SIGNAL(reset()), this, SLOT(delAllFiles()));

//#ifndef Q_OS_MAC
    QSplitter *innerSplitter = new QSplitter(Qt::Horizontal,this);
    innerSplitter->setContentsMargins(0,0,0,0);
//#else
//    QSplitter *innerSplitter = new MacSplitter();
//    innerSplitter->setOrientation(Qt::Horizontal);
//#endif
    innerSplitter->setProperty("id", "innerSplitter");
    innerSplitter->setOpaqueResize(false);
    innerSplitter->addWidget(table);
    innerSplitter->addWidget(imageBox);
    innerSplitter->setStretchFactor(0,10);
    innerSplitter->setStretchFactor(1,1);

    if (!App->innerSplitterState.isEmpty())
        innerSplitter->restoreState(App->innerSplitterState);
    connect(innerSplitter,SIGNAL(splitterMoved(int,int)),SLOT(saveSplitterState()));


#ifndef Q_OS_MAC
    QSplitter *sp = new QSplitter(Qt::Vertical,this);
    sp->setContentsMargins(0,0,0,0);
#else
    QSplitter *sp = new MacSplitter();
    sp->setOrientation(Qt::Vertical);
#endif
    sp->setProperty("id", "splitter");
    sp->setOpaqueResize(false);
    sp->addWidget(tree);
    sp->addWidget(innerSplitter);

    if (!App->splitterState.isEmpty())
        sp->restoreState(App->splitterState);
    connect(sp,SIGNAL(splitterMoved(int,int)),SLOT(saveSplitterState()));



    QVBoxLayout *la=new QVBoxLayout;
    la->setContentsMargins(0,0,0,0);
    la->addWidget(sp);
    la->setMenuBar(searchBar);
    setLayout(la);

    searchBar->hide();

    moveUpAct = new QAction(qApp->style()->standardIcon(QStyle::SP_ArrowUp),tr("Move up"),this);
    connect(moveUpAct,SIGNAL(triggered()),this,SLOT(moveUp()));
    moveUpAct->setShortcut(tr("Ctrl+Up"));
    moveUpAct->setDisabled(true);

    moveDownAct = new QAction(qApp->style()->standardIcon(QStyle::SP_ArrowDown),tr("Move down"),this);
    connect(moveDownAct,SIGNAL(triggered()),this,SLOT(moveDown()));
    moveDownAct->setShortcut(tr("Ctrl+Down"));
    moveDownAct->setDisabled(true);

    editAct = new QAction(tr("&Edit..."),this);
    connect(editAct,SIGNAL(triggered()),this,SLOT(editCell()));

    multilineEditAct  = new QAction(tr("Edit as &multiline text..."),this);
    connect(multilineEditAct,SIGNAL(triggered()),this,SLOT(editCell()));


    removeAct = new QAction(tr("C&lear tags"),this);
#if QT_VERSION >= 0x040600
    removeAct->setIcon(QIcon::fromTheme(QSL("edit-delete"),QIcon(QSL(":/src/icons/remove_tags.png"))));
#else
    removeAct->setIcon(QIcon(QSL(":/src/icons/remove_tags.png")));
#endif
    removeAct->setShortcut(Qt::Key_Delete);
    removeAct->setShortcutContext(Qt::WidgetShortcut);
    connect(removeAct,SIGNAL(triggered()),this,SLOT(remove()));

    collectAct = new QAction(tr("Collect for &future use"),this);
    collectAct->setShortcut(tr("Ctrl+H"));
    connect(collectAct,SIGNAL(triggered()),this,SLOT(collectTags()));

    tree->addAction(win->actions[QSL("rereadTags")]);
    tree->addAction(win->actions[QSL("rename")]);
    tree->addAction(win->actions[QSL("fill")]);
    tree->addAction(win->actions[QSL("delFiles")]);
    tree->addAction(win->actions[QSL("delAllFiles")]);
    tree->addAction(moveUpAct);
    tree->addAction(moveDownAct);
    tree->addAction(win->actions[QSL("play")]);
    tree->addAction(win->actions[QSL("replaygain")]);

    table->addAction(editAct);
    table->addAction(multilineEditAct);
    table->addAction(win->actions[QSL("cut")]);
    table->addAction(win->actions[QSL("copy")]);
    table->addAction(win->actions[QSL("paste")]);
    table->addAction(win->actions[QSL("copyToClipboard")]);
    table->addAction(win->actions[QSL("pasteFromClipboard")]);
    table->addAction(removeAct);
    table->addAction(win->actions[QSL("removeTags")]);
    table->addAction(win->actions[QSL("newTag")]);
    table->addAction(collectAct);

    operationsMapper = new QSignalMapper(this);
    connect(operationsMapper,SIGNAL(mapped(QString)),SLOT(handleOperation(QString)));
    operationsAct = new QAction(tr("Operations"),this);
    table->addAction(operationsAct);

    Q_EMIT totalLengthChanged(0);
    tree->setFocus();
}

Tab::~Tab()
{DD;

}


QVector<int> Tab::saveWithProgress()
{DD;
    QVector<int> saved;

    const int count = model->changedFilesCount();

    if (count>0) {
        TaskBarProgress *taskBarProgress = new TaskBarProgress(win, this);
        taskBarProgress->setRange(0, count-1);

        QProgressDialog *progress = new QProgressDialog(tr("Writing tags..."), tr("Abort"), 0, count);
        progress->setWindowTitle(tr("Qoobar"));
        progress->setWindowModality(Qt::ApplicationModal);

        QStringList errors;

        int index = model->firstChangedFileIndex();
        int i=0;
        while (index>-1) {
            qApp->processEvents();
            progress->setValue(++i);
            taskBarProgress->setValue(i);

            if (progress->wasCanceled()) {
                taskBarProgress->finalize();
                break;
            }

            //write tags
            QString error;
            if (model->saveAt(index, &error))
                saved << index;
            if (!error.isEmpty())
                errors << error;

            index = model->nextChangedFileIndex();
        }

        progress->setValue(count);
        progress->deleteLater();

        taskBarProgress->finalize();

        if (!errors.isEmpty())
            showMessage(MT_WARNING, tr("Cannot write tags to files:\n%1")
                        .arg(errors.join(QChar('\n'))));
    }
    return saved;
}

void Tab::retranslateUi()
{DD;
    moveUpAct->setText(tr("Move up"));
    moveUpAct->setShortcut(tr("Ctrl+Up"));
    moveDownAct->setText(tr("Move down"));
    moveDownAct->setShortcut(tr("Ctrl+Down"));
    editAct->setText(tr("&Edit..."));
    multilineEditAct->setText(tr("Edit as &multiline text..."));

    removeAct->setText(tr("C&lear tags"));
    collectAct->setText(tr("Collect for &future use"));
    collectAct->setShortcut(tr("Ctrl+H"));

    operationsAct->setText(tr("Operations"));
    QMenu *menu=operationsAct->menu();
    delete menu;
    operationsAct->setMenu(createOperations(operationsMapper,this));

    tree->retranslateUi();
    table->retranslateUi();
    imageBox->retranslateUi();
}



void Tab::saveSplitterState() /*SLOT*/
{DD;
    QSplitter *sp=qobject_cast<QSplitter *>(sender());
    if (sp) {
        if (sender()->property("id").toString() == "splitter")
            App->splitterState = sp->saveState();
        else
            App->innerSplitterState = sp->saveState();
    }
}

bool Tab::isEmpty()
{DD;
    return model->isEmpty();
}


/**
  Editing functions
  */

void Tab::handleOperation(const QString &type) /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    QVector<int> rows = table->selectedRows();
    if (rows.isEmpty()) return;

    ChangeFilesCommand *c = new ChangeFilesCommand(model->selectedFilesIndexes(),
                                                   model->selectedFiles(),
                                                   rows,
                                                   tr("changing tags"),
                                                   this);
    Q_FOREACH (const int &row, rows) {
        QStringList newValues = getTags(row); //tags to script
        newValues = QtConcurrent::blockingMapped(newValues, Operated(type));
        c->add(row, table->tag(row), newValues);
    }
    if (c->changed()) {
        if (App->useUndo)
            undoStack_->push(c);
        else {
            c->redo(); //void Tab::setTags through ChangeFilesCommand
            delete c;
        }
    }
}

void Tab::tagsChanged(int row,const QStringList &newValues) /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    ChangeFilesCommand *c = new ChangeFilesCommand(model->selectedFilesIndexes(),
                                                   model->selectedFiles(),
                                                   QVector<int>()<< row,
                                                   tr("changing tags"),
                                                   this);

    c->add(row, table->tag(row), newValues);

    if (c->changed()) {
        if (App->useUndo)
            undoStack_->push(c);
        else {
            c->redo(); //void Tab::setTags through ChangeFilesCommand
            delete c;
        }
    }

    if (App->autocompletions->collectSilently) {
        if (App->autocompletions->use(row))
            App->autocompletions->update(row, newValues);
    }
}




void Tab::setTags(const QVector<int> &inds, const QList<Tag> &ts, const QVector<int> &rows)
{DD;
    if (inds.isEmpty()) return;

    model->setTags(inds, ts);

    if (model->hasSelection())
        updateTagsTable(rows);
}

struct Reduce
{
    Reduce(int size)
        : m_size(size) { }

    typedef QString result_type;

    QString operator()(const QStringList &list)
    {
        if (list.isEmpty()) return QString();
        QStringList l;
        if (list.size()<=50) {
            l=list;
            l.removeDuplicates();
        }
        else {
            //reduce to a string with length not greater than table width
            Q_FOREACH (const QString &s, list) {
                if (!needToAdd(l)) break;
                if (!l.contains(s)) l<<s;
            }
        }
        if (l.size()==1) return l.first();
        return QObject::tr("<<multiple>>") + l.join(QSL(";"));
    }

    bool needToAdd(const QStringList &list) const {
        return qApp->fontMetrics().width(list.join(QSL(";"))) < m_size;
    }

    int m_size;
};

void Tab::updateTagsTable(const QVector<int> &rows)
{DD;
    const bool fullUpdate = (rows.isEmpty() || rows.last()>=App->currentScheme->tagsCount());

    const int indexesCount = model->selectedFilesCount();
    table->blockSignals(true);

    if (fullUpdate) {
        table->clearTable();
    }
    table->setCount(indexesCount);

    if (indexesCount>0) {
        QVector<int> _rows;
        if (fullUpdate) {
            QStringList otherid = model->userTagsIds();
            Q_FOREACH(const QString &s,otherid)
                table->addRow(s);
            for (int i=0; i<table->rowCount(); ++i) _rows << i;
        }
        else _rows=rows;

        QList<QStringList> tableTags;
        Q_FOREACH (const int &i,_rows)
            tableTags << getTags(i);
        QStringList reducedTags = QtConcurrent::blockingMapped(tableTags,Reduce(table->width()));

        table->blockSignals(true);
        for (int i=0; i<_rows.size(); ++i) {
            table->updateRow(_rows.at(i),reducedTags.at(i));
        }
        table->blockSignals(false);
    }

    table->blockSignals(false);
    table->resizeColumnsToContents();
    updateImageBox();
}

//void Tab::updateTagsTableRow(int row, const QStringList &list)
//{DD;
//    table->blockSignals(true);
//    QString text=Reduce(table->width())(list);
//    table->updateRow(row, text);
//    table->blockSignals(false);
//}

void Tab::remove() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;
    QVector<int> rows=table->selectedRows();
    if (rows.isEmpty()) return;

    QStringList newValues=QVector<QString>(model->selectedFilesCount()).toList();

    ChangeFilesCommand *c = new ChangeFilesCommand(model->selectedFilesIndexes(),
                                                   model->selectedFiles(),
                                                   rows,
                                                   tr("clearing tags"),
                                                   this);
    Q_FOREACH (const int &row,rows)
        c->add(row, table->tag(row), newValues);

    if (c->changed()) {
        if (App->useUndo)
            undoStack_->push(c);
        else {
            c->redo();
            delete c;
        }
    }
}

void Tab::removeAllTags() /*SLOT*/
{DD;
    table->selectAll();
    remove();
}

void Tab::editCell() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;
    int row = table->currentRow();

    bool plain=true;
    QAction *a=qobject_cast<QAction *>(sender());
    if (a) plain = (a!=multilineEditAct);

    QStringList oldValues = getTags(row);

    TagsEditDialog editor(row,table->verticalHeaderItem(row)->text(),
                          oldValues,table->item(row,0)->text(),plain,win);
    editor.setModel(model);

    connect(&editor,SIGNAL(tagsSent(int,QStringList)),SLOT(tagsChanged(int,QStringList)));
    connect(&editor,SIGNAL(rowSelected(int)),tree,SLOT(scrollToRow(int)));

    if (editor.exec()) {
        QStringList newValues=editor.getList();
        if (newValues==oldValues) return;

        tagsChanged(row,newValues);
    }
}

void Tab::updateTags(QList<Tag> &oldTags, QList<Tag> &newTags, const QString &motivation)
{DD;
    QVector<int> inds;
    QVector<int> indexes=model->selectedFilesIndexes();
    for (int i=oldTags.size()-1; i>=0; --i) {
        if (oldTags.at(i)==newTags.at(i)) {
            oldTags.removeAt(i);
            newTags.removeAt(i);
        }
        else
            inds.prepend(indexes.at(i));
    }
    if (inds.isEmpty()) return;


    ChangeFilesCommand *command = new ChangeFilesCommand(inds,
                                                         oldTags,
                                                         newTags,
                                                         QVector<int>(),
                                                         motivation,
                                                         this);
    if (App->useUndo)
        undoStack_->push(command);
    else {
        command->redo();
        delete command;
    }
}

void Tab::fill() /*SLOT*/
{DD;
    if (!model->hasSelection())
        return;

    QList<Tag> oldTags = model->selectedFiles();

    TagsFillDialog filler(oldTags, win);
    if (filler.exec()) {
        QList<Tag> newTags=filler.getNewTags();
        updateTags(oldTags, newTags, tr("filling tags"));
    }
}

void Tab::doPlugin(IQoobarPlugin *plugin)
{DD;
    if (!plugin) return;

    if (!model->hasSelection() && !plugin->canWorkWithNoFilesSelected())
        return;

    QList<Tag> oldTags=model->selectedFiles();
    QList<Tag> newTags = plugin->getNewTags(oldTags);
    if (newTags.size() <= oldTags.size()) {
        updateTags(oldTags, newTags, tr("changing tags"));
    }
    else {
        QList<Tag> newTags1 = newTags.mid(0, oldTags.size());
        updateTags(oldTags, newTags1, tr("changing tags"));

        newTags1 = newTags.mid(oldTags.size(), newTags.size()-oldTags.size());
        model->addFiles(newTags1, true);
    }
}

void Tab::rereadTags() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    QVector<int> indexes = model->selectedFilesIndexes();

    QList<Tag> oldTags = model->selectedFiles();
    QStringList fileNames = model->selectedFilesNames();
    ReadWorker w;
    w.setFiles(fileNames);
    w.run();
    QList<Tag> newTags = w.tags();

    QVector<int> inds;

    for (int i=oldTags.size()-1; i>=0; --i) {
        if (oldTags.at(i)==newTags.at(i)) {
            oldTags.removeAt(i);
            newTags.removeAt(i);
        }
        else
            inds.prepend(indexes.at(i));
    }
    if (inds.isEmpty()) return;

    ChangeFilesCommand *command = new ChangeFilesCommand(inds,
                                                         oldTags,
                                                         newTags,
                                                         QVector<int>(),
                                                         tr("rereading tags"),
                                                         this);
    if (App->useUndo)
        undoStack_->push(command);
    else {
        command->redo();
        delete command;
    }

    updateImageBox();
}

void Tab::cellChanged(QTableWidgetItem *item) /*SLOT*/
{DD;
    if (!model->hasSelection()) return;
    if (!item) return;
    int row=item->row();

    QStringList oldValues = getTags(row);
    QStringList newValues=model->tagsByPattern(row, item->text());
    table->closePersistentEditor(item);
    if (oldValues==newValues) {
        table->blockSignals(true);
        QString text=Reduce(table->width())(newValues);
        table->updateRow(row, text);
        table->blockSignals(false);
    }
    else {
        tagsChanged(row, newValues);
    }
}

void Tab::paste() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    ChangeFilesCommand *c = new ChangeFilesCommand(model->selectedFilesIndexes(),
                                                   model->selectedFiles(),
                                                   QVector<int>(),
                                                   tr("pasting tags"),
                                                   this);

    Q_FOREACH (const BufferItem &item, internalBuffer) {
        int row=-1;
        QString s=item.header;
        if (internalBuffer.size()==1) row = table->currentRow();
        else {
            row=item.row;
            if (row>=App->currentScheme->tagsCount()) {
                int found=-1;
                int c=table->rowCount();
                for (int j=0; j<c; ++j)
                    if (table->tag(j) == s) {
                        found=j;
                        break;
                    }
                if (found<0) row=c;
                else row=found;
            }
        }
        if (row==-1 || row>=table->rowCount()) continue;

        QStringList newValues=item.values;
        if (newValues.size()==1 && App->autoexpand)
            newValues = QVector<QString>(model->selectedFilesIndexes().size(),newValues.first()).toList();

        c->add(row, table->tag(row), newValues);
    }

    if (c->changed()) {
        if (App->useUndo)
            undoStack_->push(c);
        else {
            c->redo();
            delete c;
        }
    }
}

void Tab::pasteFromClipboard() /*SLOT*/
{DD;

    if (!model->hasSelection()) return;
    int row = table->currentRow();

    QVector<int> indexes = model->selectedFilesIndexes();
    QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    if (text.isEmpty()) return;
    QStringList newValues=text.split(QRegExp(QSL("\\n|\\r|\\n\\r|\\r\\n")));
    if (newValues.size()==1 && App->autoexpand) {
        while (indexes.size()>newValues.size()) newValues<<newValues.first();
    }
    while (indexes.size()<newValues.size()) newValues.removeLast();
    while (indexes.size()>newValues.size()) newValues<<QString();


    ChangeFilesCommand *c = new ChangeFilesCommand(model->selectedFilesIndexes(),
                                                   model->selectedFiles(),
                                                   QVector<int>()<< row,
                                                   tr("pasting tags"),
                                                   this);

    c->add(row, table->tag(row), newValues);

    if (c->changed()) {
        if (App->useUndo)
            undoStack_->push(c);
        else {
            c->redo();
            delete c;
        }
    }
}

void Tab::cut() /*SLOT*/
{DD;
    copy();
    remove();
}

void Tab::copy() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;
    internalBuffer.clear();
    for (int row=0; row<table->rowCount(); ++row) {
        if (table->item(row,0)->isSelected()) {
            internalBuffer.append(BufferItem(row,table->tag(row), getTags(row)));
        }
    }
    Q_EMIT bufferChanged(internalBuffer.isEmpty());
}

void Tab::copyToClipboard() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;
    QVector<int> indexes = model->selectedFilesIndexes();

    QClipboard *clipboard = QApplication::clipboard();
    QStringList result;
    const int tagsCount = App->currentScheme->tagsCount();
    Q_FOREACH (const int &index, indexes) {
        QStringList s;
        for (int row=0; row<table->rowCount(); ++row) {
            if (table->item(row,0)->isSelected()) {
                if (row<tagsCount)
                    s<<model->value(index,row);
                else s << model->value(index,table->tag(row));
            }
        }
        result << s.join(QSL("\t"));
    }
    clipboard->setText(result.join(QSL("\n")));
}


















/**
  Images
  */

void Tab::changeImage(const CoverImage &img,const QString &message) /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    if (App->useUndo) {
        ChangeCommand *command = new ChangeCommand(this);
        command->setText(message);

        QVector<int> indexes = model->selectedFilesIndexes();
        int added=0;
        Q_FOREACH(const int &i, indexes) {
            CoverImage oldImg = model->fileAt(i).image();
            if (img==oldImg) continue;
            added++;
            new ChangeImageCommand(i,oldImg,img,this,command);
        }
        if (added>0)
            undoStack_->push(command);
        else delete command;
    }
    else {
        model->setImage(img);
        setStatus();
    }
}

void Tab::setStatus()
{DD;
    Q_EMIT filesChanged(!allFilesSaved());
    updateImageBox();
}


void Tab::updateImageBox()
{DD;
    if (!imageBox->isVisible()) return; //do not paint image if it is not visible
    imageBox->clear();

    if (!model->hasSelection()) return;

    if (!model->imagesAreSame()) //images are different
        imageBox->updatePicture(tr("<<multiple>>"));
    else {//images are the same
        CoverImage img=model->fileAtSelection(0).image();
        imageBox->updatePicture(img,model->fileAtSelection(0).fullFileName());
    }
}

void Tab::updateTableRow(int row, const QString &value)
{
    table->blockSignals(true);
    table->updateRow(row, value);
    table->blockSignals(false);
}




/**
  ***********************************************
  */

void Tab::play() /*SLOT*/
{DD;
    QString s=App->player;
    if (s.isEmpty()) {
        warningMessage(win, QSL("Qoobar"),tr("Please set a player in the Settings dialog"));
        return;
    }
    QStringList list=model->selectedFilesNames();
    if (!list.isEmpty())
        QProcess::startDetached(s,list);
}


/**
  Rename files
  */

void Tab::renameFiles() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    QStringList oldFileNames = model->selectedFilesNames();

    FileRenameDialog renamer(model, win);
    if (renamer.exec()) {
        QStringList newFileNames = renamer.getNewFileNames();
        if (newFileNames == oldFileNames) return;

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        model->rename(newFileNames);

        QApplication::restoreOverrideCursor();
        undoStack_->clear();
    }
}



/**
  Add and del files
  */
void Tab::delFiles() /*SLOT*/
{DD;
    delFiles(false);
}

void Tab::delAllFiles() /*SLOT*/
{DD;
    if (model->isEmpty()) return;
    model->selectAll();
    delFiles(false);
}

void Tab::delFiles(bool deleteSilently)
{DD;
    if (!model->hasSelection()) return;

    if (!deleteSilently && !model->isSelectedFilesSaved()) {
        QMessageBox msgBox(QMessageBox::Question,tr("Qoobar"),
                           tr("Some files have been modified"),
                           QMessageBox::NoButton, win);
        msgBox.setInformativeText(tr("Remove them anyway?"));
        QPushButton *delB=msgBox.addButton(tr("Yes, remove without saving"),QMessageBox::YesRole);
        delB->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));
        QPushButton *saveB=msgBox.addButton(tr("No, save them and then remove"),QMessageBox::NoRole);
        saveB->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogDiscardButton));
        QPushButton *cancB=msgBox.addButton(QMessageBox::Cancel);
        cancB->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton));
        msgBox.setDefaultButton(saveB);
        msgBox.setEscapeButton(cancB);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.exec();

        if (msgBox.clickedButton() == saveB) {
            saveWithProgress();
        }
        else if (msgBox.clickedButton() == cancB) {
            return;
        }
    }

    model->delFiles();
    undoStack_->clear();


    Q_EMIT updateStatusBar(Tag());
    updateTagsTable(QVector<int>());

    Q_EMIT selectionChanged(false);
    Q_EMIT totalLengthChanged(0);

    moveUpAct->setEnabled(false);
    moveDownAct->setEnabled(false);

    handleCutCopy();
}

void Tab::addFileNames(const QStringList &filesToAdd, bool clearBefore)
{DD;
    if (filesToAdd.isEmpty()) return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (clearBefore) delAllFiles();

    TaskBarProgress *progress = new TaskBarProgress(win, this);
    progress->setRange(0, filesToAdd.size());
    connect(model,SIGNAL(fileAdded(int)),progress,SLOT(setValue(int)));
    connect(model,SIGNAL(allFilesAdded()), progress,SLOT(finalize()));

    model->addFiles(filesToAdd);
    win->statusBar()->showMessage(tr("Please wait while Qoobar is adding files"));
}

void Tab::addFiles(int addedCount, bool update) /*SLOT*/
{DD;
    if (addedCount==0) return;

    if (addedCount<100 && update) {
        tree->blockSignals(true);
        tree->clearSelection();
        for (int i=0; i<addedCount-1; ++i) {
            tree->selectionModel()->select(model->index(i+model->size()-addedCount,0),QItemSelectionModel::Rows | QItemSelectionModel::Select);
        }
        tree->blockSignals(false);
        tree->selectionModel()->select(model->index(model->size()-1,0),QItemSelectionModel::Rows | QItemSelectionModel::Select);
    }

    Q_EMIT filesChanged(!allFilesSaved());
    win->statusBar()->clearMessage();
    QApplication::restoreOverrideCursor();
}

void Tab::moveFiles(const QList<Tag> &filesToMove)
{DD;
    model->addFiles(filesToMove);
}




/**
Saving files
*/
void Tab::saveTags() /*SLOT*/
{DD;
    if (model->isFilesSaved()) return;
    const QVector<int> savedFilesIndexes = saveWithProgress();
    model->rereadTags(savedFilesIndexes);
    undoStack_->clear();
}

void Tab::onItemClicked(const QModelIndex &index) /*SLOT*/
{DD;
    if (!index.isValid()) return;
    int col = index.column();
    if (col == COL_SAVEICON) {
        QString msg;
        const int row = index.row();

        if (model->saveAt(row, &msg)) {
            model->rereadTags(QVector<int>()<<row);
            undoStack_->clear();

            if (!msg.isEmpty())
                showMessage(MT_WARNING,
                            tr("Cannot write tags to file:\n%1").arg(msg));
        }
    }
}

bool Tab::maybeSave(const QString &tabText)
{DD;

    if (App->saveChanges) {
        saveTags();
        return true;
    }

    if (!allFilesSaved()) {
        QMessageBox msgBox(QMessageBox::Question,
                           tr("Closing Qoobar"),
                           tr("Files in %1 have been modified.").arg(tabText),
                           QMessageBox::NoButton,
                           win);
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        QPushButton *saveB=msgBox.addButton(tr("Yes, save"),QMessageBox::YesRole);
        saveB->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));
        QPushButton *discB=msgBox.addButton(tr("No, don't save"),QMessageBox::NoRole);
        discB->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogDiscardButton));
        QPushButton *cancB=msgBox.addButton(QMessageBox::Cancel);
        cancB->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton));
        msgBox.setDefaultButton(saveB);
        msgBox.setEscapeButton(cancB);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.exec();

        if (msgBox.clickedButton() == cancB) return false;
        else {
            if (msgBox.clickedButton() == saveB) saveTags();
            return true;
        }
    }
    return true;
}

bool Tab::allFilesSaved() const
{DD;
    return model->isFilesSaved();
}

bool Tab::filesSelected()
{DD;
    return model->hasSelection();
}

/**
  Moving files up and down
  */

void Tab::moveUp() /*SLOT*/
{DD;
    moveItems(true);
}

void Tab::moveDown() /*SLOT*/
{DD;
    moveItems(false);
}

void Tab::moveItems(bool up)
{DD;
    model->move(up);
    undoStack_->clear();
}

void Tab::sortColumn(int column, Qt::SortOrder order, int sortType)
{DD;
    model->sort(column, order, sortType);
    filesSelectionChanged();
    undoStack_->clear();
}

void Tab::moveToTab(int tabID) /*SLOT*/
{DD;
    if (!model->hasSelection()) return;
    QList<Tag> filesToMove = model->selectedFiles();

    Q_EMIT moveFilesToTab(tabID, filesToMove);
    delFiles(true);
}


/**
  Other
  */

void Tab::filesSelectionChanged() /*SLOT*/
{DD;
    QVector<int> indexes;

    QItemSelection selectedRows = tree->selectionModel()->selection();

    for (int i=0; i<model->size(); ++i) {
        if (selectedRows.contains(model->index(i,0))) {
            indexes << i;
        }
    }
    indexes.squeeze();
    model->setSelection(indexes);

    Q_EMIT selectionChanged(indexes.size()>0);
    Q_EMIT totalLengthChanged(model->totalLength());

    moveUpAct->setEnabled(indexes.size()>0);
    moveDownAct->setEnabled(indexes.size()>0);

    updateTagsTable(QVector<int>());
    handleCutCopy();
}

QStringList Tab::getTags(int n)
{DD;
    if (n<App->currentScheme->tagsCount())
        return model->values(n);
    else
        return model->values(table->tag(n));
}

void Tab::newTag() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    QString id;
    NewTagDialog dialog(win);
    if (dialog.exec())
        id = dialog.newTagName.toUpper();
    if (id.isEmpty()) return;

    int found=-1;
    int c=table->rowCount();
    for (int i=0; i<c; ++i)
        if (table->verticalHeaderItem(i)->text()==id) {
            found=i;
            break;
        }
    if (found<0) {
        table->addRow(id);
        found=c;
    }

    table->setCurrentItem(table->item(found,0),QItemSelectionModel::ClearAndSelect);
    table->resizeColumnsToContents();
}

void Tab::handleCutCopy() /*SLOT*/
{DD;
    bool b=(!table->selectedItems().isEmpty() && model->hasSelection());

    Q_EMIT tagsSelectionChanged(b);

    editAct->setEnabled(b);
    multilineEditAct->setEnabled(b);
    removeAct->setEnabled(b);
    operationsAct->setEnabled(b);
    collectAct->setEnabled(b);
}

void Tab::replaygain() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    //we can only scan saved files, because we need to write
    //replaygain info
    if (!model->isSelectedFilesSaved()) {
        QMessageBox msgBox(QMessageBox::Question, tr("Qoobar - ReplayGain Info"),
                           tr("Qoobar can only scan saved files.\n"
                              "Do you want to save changes?"), QMessageBox::NoButton, win);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setWindowModality(Qt::WindowModal);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes) {
            const QVector<int> savedFilesIndexes = saveWithProgress();
            model->rereadTags(savedFilesIndexes);
        }
        else return;
    }

    ReplayGainDialog dialog(model, win);

    dialog.exec();
    model->saveSelected();
    model->rereadTags(model->selectedFilesIndexes());

    Q_EMIT filesChanged(!allFilesSaved());
}

void Tab::startSearch()
{
    searchBar->show();
}

void Tab::updateStatusBar(const QModelIndex &index) /*SLOT*/
{DD;
    int row = index.row();
    if (row<0)
        return;

    Q_EMIT updateStatusBar(model->fileAt(row));
}

void Tab::collectTags() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<tagsCount; ++i) {
        if (table->item(i,0)->isSelected()) {
            if (table->item(i,0)->text().isEmpty()) continue;
            App->autocompletions->update(i, getTags(i));
        }
    }
}

QMap<int, QString> Tab::allTabsNames()
{DD;
    return win->allTabsNames();
}

bool Tab::allSelectedFilesAreMP3() const
{DD;
    if (!model->hasSelection()) return false;
    for (int i=0; i<model->selectedFilesCount(); ++i) {
        if (model->fileAtSelection(i).fileType()!=Tag::MP3_FILE) return false;
    }
    return true;
}

QUndoStack *Tab::undoStack()
{DD;
    return undoStack_;
}

void Tab::showMP3TagsDialog() /*SLOT*/
{DD;
    if (!model->hasSelection()) return;

    MP3TagsDialog dialog(model->selectedFilesCount(),this);

    if (dialog.exec()) {
        //changing global settings just for this only operation
        int tagTypes = dialog.tagTypes();
        int id3v1Synchro = App->id3v1Synchro;
        App->id3v1Synchro = (tagTypes & MP3TagsDialog::ID3V1)
                ?ID3V1_UPDATE_ALWAYS
               :ID3V1_DELETE;
        bool mp3writeape = App->mp3writeape;
        App->mp3writeape = tagTypes & MP3TagsDialog::APE;
        bool mp3writeid3 = App->mp3writeid3;
        App->mp3writeid3 = tagTypes & MP3TagsDialog::ID3V2;

        //now we are ready to save files
        //we just have to ensure files will be saved
        model->forceSelectionToSave();
        const QVector<int> savedFilesIndexes = saveWithProgress();
        model->rereadTags(savedFilesIndexes);
        undoStack_->clear();



        //returning back global settings
        App->id3v1Synchro= id3v1Synchro;
        App->mp3writeape = mp3writeape;
        App->mp3writeid3 = mp3writeid3;
    }
}

void Tab::showMessage(int type, const QString &text)
{DD;
    switch (type) {
    case MT_ERROR: {
        criticalMessage(win, QSL("Qoobar"),text);
        break;
    }
        //do not show successful result, as we are not in console mode
    case MT_INFORMATION: {
        //QMessageBox::information(this, tr("Qoobar"), text);
        break;
    }
    case MT_WARNING: {
        warningMessage(win, QSL("Qoobar"),text);
        break;
    }
    }
}



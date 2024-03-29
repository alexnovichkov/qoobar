/**
 * \file replaygaindialog.cpp
 * A dialog to use with ReplayGainer
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date Nov 27 2012
 *
 * Copyright (C) 2012  Alex Novichkov
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

#include "replaygaindialog.h"

#include <QtWidgets>
#include "application.h"
#include "tagsreaderwriter.h"
#include "enums.h"
#include "model.h"
#include "replaygainer.h"
#include "qbutton.h"
#include "qoobarglobals.h"
#include "qoobarhelp.h"

enum {
    RG_NUMBER=0,
    RG_FILENAME=1,
    RG_ALBUMGAIN=2,
    RG_TRACKGAIN=3,
    RG_ALBUMPEAK=4,
    RG_TRACKPEAK=5,
    RG_ALBUMRANGE=6,
    RG_TRACKRANGE=7,
    RG_REFERENCELOUDNESS=8,
    RG_TOTAL
};

ReplayGainDialog::ReplayGainDialog(Model *model, QWidget *parent) :
    QDialog(parent), m(model), scanned(false)
{DD;
    setWindowTitle(tr("Qoobar - Changing ReplayGain Info"));
    setWindowModality(Qt::WindowModal);

    replayGainer = new ReplayGainer(m);
    connect(replayGainer, SIGNAL(finished()), this, SLOT(finalize()),Qt::QueuedConnection);
    connect(replayGainer, SIGNAL(message(int,QString)),SLOT(appendText(int,QString)),Qt::QueuedConnection);
    connect(replayGainer, SIGNAL(tick()), this, SLOT(tick()),Qt::QueuedConnection);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                       | QDialogButtonBox::Cancel);
    okButton=buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText(tr("Update file tags"));
    okButton->setEnabled(scanned);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
#ifdef OSX_SUPPORT_ENABLED
    QButton *helpButton = new QButton(this,QButton::HelpButton);
#else
    QPushButton *helpButton = buttonBox->addButton(QDialogButtonBox::Help);
#endif
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));

    scanFileButton = new QPushButton(tr("Scan per file track gain"),this);
    scanAlbumButton = new QPushButton(tr("Scan as a single album"),this);
    removeButton = new QPushButton(tr("Remove replaygain tags from files"),this);
    scanAlbumsByTagsButton = new QPushButton(tr("Scan as albums (by tags)"),this);
    scanAlbumsByFolderButton = new QPushButton(tr("Scan as albums (by folder)"),this);

    QSignalMapper *mapper = new QSignalMapper(this);
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    connect(mapper, &QSignalMapper::mappedInt, this, &ReplayGainDialog::operate);
#else
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(operate(int)));
#endif
    connect(scanFileButton,SIGNAL(clicked()), mapper, SLOT(map()));
    connect(scanAlbumButton,SIGNAL(clicked()), mapper, SLOT(map()));
    connect(removeButton,SIGNAL(clicked()), mapper, SLOT(map()));
    connect(scanAlbumsByTagsButton,SIGNAL(clicked()), mapper, SLOT(map()));
    connect(scanAlbumsByFolderButton,SIGNAL(clicked()), mapper, SLOT(map()));

    mapper->setMapping(scanFileButton, RG_SCAN_AS_FILES);
    mapper->setMapping(scanAlbumButton, RG_SCAN_AS_ALBUM);
    mapper->setMapping(removeButton, RG_SCAN_REMOVE);
    mapper->setMapping(scanAlbumsByTagsButton, RG_SCAN_AS_ALBUMS_BY_TAGS);
    mapper->setMapping(scanAlbumsByFolderButton, RG_SCAN_AS_ALBUMS_BY_FOLDERS);

    skipCheckBox = new QCheckBox(tr("Skip already scanned files"),this);
    connect(skipCheckBox,SIGNAL(stateChanged(int)),SLOT(setSkip()));
    skipCheckBox->setChecked(true);

    edit = new QPlainTextEdit(this);
    edit->setReadOnly(true);
    edit->setFont(QFont(QSL("Courier")));
    //edit->setCenterOnScroll(true);

    tree = new QTreeWidget(this);
    tree->setAlternatingRowColors(true);
    tree->setRootIsDecorated(false);
    tree->setAllColumnsShowFocus(true);
    tree->setUniformRowHeights(true);
    tree->setHeaderLabels(QStringList()<<tr("No.")<<tr("Name")<<tr("Album gain")<<tr("Track gain")
                          <<tr("Album peak")<<tr("Track peak")<<tr("Album range")<<tr("Track range")
                          <<tr("Reference loudness"));
    tree->header()->resizeSection(0,50);
    tree->header()->resizeSection(1,350);

#ifdef OSX_SUPPORT_ENABLED
    //tree->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    tree->setAutoFillBackground(true);
    tree->setAttribute(Qt::WA_MacShowFocusRect, false);
#endif

    tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    tree->header()->setStretchLastSection(false);
    tree->setTextElideMode(Qt::ElideLeft);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(tree,0,0,1,4);
    grid->addWidget(scanFileButton,1,0,1,1);
    grid->addWidget(scanAlbumButton,1,1,1,1);
    grid->addWidget(removeButton,1,2,1,1);
    grid->addWidget(scanAlbumsByTagsButton,2,0);
    grid->addWidget(scanAlbumsByFolderButton,2,1);
    grid->addWidget(skipCheckBox,3,0,1,4);
    grid->addWidget(edit,4,0,1,4);
#ifdef OSX_SUPPORT_ENABLED
    QHBoxLayout *boxL = new QHBoxLayout;
    boxL->addWidget(helpButton);
    boxL->addWidget(buttonBox);
    grid->addLayout(boxL,5,0,1,4,Qt::AlignRight);
#else
    grid->addWidget(buttonBox,5,0,1,4,Qt::AlignRight);
#endif
    setLayout(grid);
    resize(qApp->primaryScreen()->availableSize()/2);

    initTable();
}

ReplayGainDialog::~ReplayGainDialog()
{
    if (replayGainer) {
        replayGainer->deleteLater();
    }
    if (t) {
        t->quit();
        t->wait();
        t->deleteLater();
    }
}

void ReplayGainDialog::accept()
{DD;
    if (scanned && !rgList.isEmpty()) {
        for (int i=0; i<m->selectedFilesCount(); ++i)
            m->fileAtSelection(i).setReplayGainInfo(rgList.at(i));

        QDialog::accept();
    }
}

void ReplayGainDialog::reject()
{
    if (t)
        t->requestInterruption();
    //replayGainer->stop();
    QDialog::reject();
}

void ReplayGainDialog::appendText(const QString &text)
{DD;
    if (text.isEmpty() || text==" ") return;
    const bool atBottom = (edit->verticalScrollBar()->value()==edit->verticalScrollBar()->maximum());
    edit->appendHtml(text);
    if (atBottom)
        edit->verticalScrollBar()->setValue(edit->verticalScrollBar()->maximum());
}

void ReplayGainDialog::appendText(int type, const QString &text)
{DD;
    Q_UNUSED(type)
    appendText(text);
}

void ReplayGainDialog::tick()
{DD;
    edit->insertPlainText(QSL("."));
}

void ReplayGainDialog::setSkip()
{DD;
    replayGainer->setSkip(skipCheckBox->isChecked());
}

void ReplayGainDialog::showHelp()
{DD;
    Qoobar::showHelp(QSL("replaygain"));
}

void setRg(QTreeWidgetItem *item, const ReplayGainInfo &replayGainInfo)
{DD;
    item->setIcon(RG_NUMBER,replayGainInfo.isEmpty()?
                      QIcon():QIcon::fromTheme("tick"));
    item->setText(RG_ALBUMGAIN,replayGainInfo.albumGain);
    item->setText(RG_ALBUMPEAK,replayGainInfo.albumPeak);
    item->setText(RG_TRACKGAIN,replayGainInfo.trackGain);
    item->setText(RG_TRACKPEAK,replayGainInfo.trackPeak);
    item->setText(RG_TRACKRANGE,replayGainInfo.trackMinMax);
    item->setText(RG_ALBUMRANGE,replayGainInfo.albumMinMax);
    item->setText(RG_REFERENCELOUDNESS,replayGainInfo.loudness);
}

void ReplayGainDialog::operate(int operation)
{DD;
    edit->clear();

    scanFileButton->setEnabled(false);
    scanAlbumButton->setEnabled(false);
    scanAlbumsByTagsButton->setEnabled(false);
    scanAlbumsByFolderButton->setEnabled(false);
    removeButton->setEnabled(false);
    skipCheckBox->setEnabled(false);
    okButton->setEnabled(false);

    if (!t) {
        t = new QThread;
        replayGainer->moveToThread(t);
        connect(t, SIGNAL(started()), replayGainer, SLOT(start()));
        connect(replayGainer, SIGNAL(finished()), t, SLOT(quit()));
    }
    replayGainer->setOperation(operation);

    t->start();

    //scanned = replayGainer->start(operation);
}

void ReplayGainDialog::finalize()
{
    okButton->setEnabled(true);
    scanned = true;

    rgList = replayGainer->getNewRgInfo();
    for (int i=0; i< rgList.size(); ++i) {
        setRg(tree->topLevelItem(i), rgList.at(i));
    }

    scanFileButton->setEnabled(true);
    scanAlbumButton->setEnabled(true);
    scanAlbumsByTagsButton->setEnabled(true);
    scanAlbumsByFolderButton->setEnabled(true);
    removeButton->setEnabled(true);
    skipCheckBox->setEnabled(true);
}

void ReplayGainDialog::initTable()
{DD;
    for (int i=0; i<m->selectedFilesCount(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(tree);
        item->setText(RG_NUMBER,QString::number(i+1));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setTextAlignment(RG_NUMBER,Qt::AlignRight | Qt::AlignVCenter);
        tree->topLevelItem(i)->setText(RG_FILENAME,m->fileAtSelection(i).fullFileName());
        tree->topLevelItem(i)->setIcon(RG_FILENAME,QIcon(m->fileAtSelection(i).icon()));
        setRg(tree->topLevelItem(i), m->fileAtSelection(i).replayGainInfo());
    }
}

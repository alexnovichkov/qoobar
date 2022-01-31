/**
 * \file settingsdialog.cpp
 * Settings dialog.
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

#include "settingsdialog.h"
#include "configpages.h"
#include <QtWidgets>
#include "application.h"
#include "qoobarglobals.h"
#include "enums.h"

#ifdef OSX_SUPPORT_ENABLED
#include "mactoolbar.h"
#include "qocoa/qbutton.h"
//defines settings dialog that resizes each time page is changed
#define DYNAMICPAGES
#endif
#include "qoobarhelp.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
#ifdef OSX_SUPPORT_ENABLED
    // We define settings dialog as QMainWindow to obtain nice-looking OSX-style toolbar
    QMainWindow(parent)
#else
    QDialog(parent)
#endif
{DD;
    setWindowTitle(tr("Qoobar settings"));
#ifdef OSX_SUPPORT_ENABLED
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowModality(Qt::ApplicationModal);
#endif

    InterfacePage *page = new InterfacePage;
    connect(page,SIGNAL(retranslate()),this,SLOT(retranslateUI()));
    connect(page,SIGNAL(retranslate()),this,SIGNAL(retranslate()));
//    connect(page,SIGNAL(tableUpdateModeChanged(bool)),this,SIGNAL(tableUpdateModeChanged(bool)));
    configPages << page;
    configPages << new WritingPage;
    configPages << new TagsPage;
    configPages << new CompletionPage;
    configPages << new PatternsPage;
    configPages << new UtilitiesPage;
    configPages << new ReplaygainPage;
    configPages << new NetworkPage;
    configPages << new PluginsPage;

    pagesWidget = new QStackedWidget;
#ifdef DYNAMICPAGES
    pagesWidget->addWidget(page);
#else
    Q_FOREACH (ConfigPage *page, configPages)
        pagesWidget->addWidget(page);
#endif

    currentPage = 0;

#ifdef OSX_SUPPORT_ENABLED
    QSignalMapper *mapper = new QSignalMapper(this);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    connect(mapper, &QSignalMapper::mappedInt, this, &SettingsDialog::switchPage);
#else
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(switchPage(int)));
#endif

    setUnifiedTitleAndToolBarOnMac(true);
    toolBar = addToolBar("ttolbar");
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->setIconSize(QSize(LARGE_ICON_SIZE, LARGE_ICON_SIZE));

    QActionGroup *ag = new QActionGroup(this);
    for (int i=0; i<configPages.size(); ++i) {
        ConfigPage *page = configPages.at(i);
        QAction *a = new QAction(QIcon::fromTheme(page->iconFilename()), page->description(),this);
        a->setCheckable(true);
        a->setActionGroup(ag);
        if (i==0) a->setChecked(true);
        mapper->setMapping(a,i);
        connect(a,SIGNAL(triggered()),mapper,SLOT(map()));
        toolBar->addAction(a);
    }
#else
    pageList = new QListWidget(this);
    pageList->setIconSize(QSize(16,16));

    for (int i=0; i<configPages.size(); ++i) {
        ConfigPage *page = configPages.at(i);
        auto item = new QListWidgetItem(QIcon::fromTheme(page->iconFilename()), page->description(), pageList);

        pageList->addItem(item);
    }
//    pageList->setSizePolicy(QSizePolicy::Minimum, pageList->sizePolicy().verticalPolicy());
    connect(pageList, &QListWidget::currentRowChanged, this, &SettingsDialog::switchPage);
#endif

    resetSettingsButton = new QPushButton(tr("Reset Settings"),this);
    connect(resetSettingsButton,SIGNAL(clicked()),this,SLOT(resetSettings()));

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
#ifdef OSX_SUPPORT_ENABLED
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
#else
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
#endif

    QPushButton *helpButton = buttonBox->addButton(QDialogButtonBox::Help);
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));

    //laying out
    auto *horizontalLayout = new QHBoxLayout;
#ifndef OSX_SUPPORT_ENABLED
    horizontalLayout->addWidget(pageList);
#endif
    horizontalLayout->addWidget(pagesWidget);

    auto *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(resetSettingsButton);
    bottomLayout->addWidget(buttonBox);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    //mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(bottomLayout);
#ifdef OSX_SUPPORT_ENABLED
    auto *w = new QWidget(this);
    w->setLayout(mainLayout);
    setCentralWidget(w);
    mainLayout->setMenuBar(toolBar);
#else
//    mainLayout->setMargin(0);
    horizontalLayout->setContentsMargins(0,0,0,0);
    horizontalLayout->setStretch(0,1);
    horizontalLayout->setStretch(1,4);
    setLayout(mainLayout);
#endif
//    resize({qApp->primaryScreen()->availableSize().width()/3,
//            qApp->primaryScreen()->availableSize().height()/3});
    retranslateUI();
}

void SettingsDialog::retranslateUI()
{DD;
    setWindowTitle(tr("Qoobar settings"));
    Q_FOREACH (ConfigPage *page,configPages) page->retranslateUI();
#ifdef OSX_SUPPORT_ENABLED
    QList<QAction *> actions = toolBar->actions();
    for (int i=0; i<configPages.size(); ++i) {
        actions[i]->setText(configPages.at(i)->description());
    }
#else
    for (int i=0; i<configPages.size(); ++i) {
        pageList->item(i)->setText(configPages.at(i)->description());
    }
#endif
    resetSettingsButton->setText(tr("Reset Settings"));
}

void SettingsDialog::accept()
{DD;
    for (int i=0; i<configPages.count(); ++i) {
        configPages[i]->saveSettings();
    }

#ifdef OSX_SUPPORT_ENABLED
    close();
#else
    QDialog::accept();
#endif
}

void SettingsDialog::resetSettings()
{DD;
    if (QMessageBox::question(this,tr("Qoobar"),tr("Reset settings to defaults?"))!=QMessageBox::Yes) return;
    App->resetSettings();
    for (int i=0; i<configPages.count(); ++i) {
        configPages[i]->setSettings();
    }
}

void SettingsDialog::switchPage(int page)
{DD; 
#ifdef DYNAMICPAGES
    pagesWidget->removeWidget(pagesWidget->widget(0));
    pagesWidget->addWidget(configPages[page]);
    this->adjustSize();
#else
    pagesWidget->setCurrentIndex(page);
#endif
    currentPage = page;
}

void SettingsDialog::showHelp()
{
    QString anchor;
    switch (currentPage) {
    case 0: anchor="interface"; break;
    case 1: anchor="autocompletion-settings"; break;
    case 2: anchor="tagswriting"; break;
    case 3: anchor="patterns-settings"; break;
    case 4: anchor="utilities"; break;
    case 5: anchor="replaygain"; break;
    case 6: anchor="network"; break;
    case 7: anchor="plugins"; break;
    }
    Qoobar::showHelp(anchor);
}


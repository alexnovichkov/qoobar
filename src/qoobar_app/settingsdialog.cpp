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
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "application.h"


SettingsDialog::SettingsDialog(QWidget *parent) :
#ifdef Q_OS_MAC
    QMainWindow(parent)
#else
    QDialog(parent)
#endif
{
    setWindowTitle(tr("Qoobar settings"));
#ifdef Q_OS_MAC
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowModality(Qt::ApplicationModal);
#endif
    InterfacePage *page = new InterfacePage;
    connect(page,SIGNAL(retranslate()),this,SLOT(retranslateUI()));
    connect(page,SIGNAL(retranslate()),this,SIGNAL(retranslate()));
//    connect(page,SIGNAL(tableUpdateModeChanged(bool)),this,SIGNAL(tableUpdateModeChanged(bool)));
    configPages << page;
    configPages << new CompletionPage;
    configPages << new WritingPage;
    configPages << new PatternsPage;
    configPages << new UtilitiesPage;
    configPages << new NetworkPage;
    configPages << new PluginsPage;

    pagesWidget = new QStackedWidget;

    Q_FOREACH (ConfigPage *page, configPages)
        pagesWidget->addWidget(page);

#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac(true);
    toolBar = addToolBar("ttolbar");
#else
    toolBar = new QToolBar(this);
#endif

    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->setIconSize(QSize(48,48));
    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(int)),pagesWidget,SLOT(setCurrentIndex(int)));

    QActionGroup *ag = new QActionGroup(this);
    for (int i=0; i<configPages.size(); ++i) {
        ConfigPage *page = configPages.at(i);
        QAction *a = new QAction(QIcon(page->iconFilename()), page->description(),this);
        a->setCheckable(true);
        a->setActionGroup(ag);
        if (i==0) a->setChecked(true);
        mapper->setMapping(a,i);
        connect(a,SIGNAL(triggered()),mapper,SLOT(map()));
        toolBar->addAction(a);
    }





    resetSettingsButton = new QPushButton(tr("Reset Settings"),this);
    connect(resetSettingsButton,SIGNAL(clicked()),this,SLOT(resetSettings()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
#ifdef Q_OS_MAC
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
#else
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
#endif


    //laying out
    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(resetSettingsButton);
    bottomLayout->addWidget(buttonBox);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    //mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(bottomLayout);
#ifdef Q_OS_MAC
    QWidget *w = new QWidget(this);
    w->setLayout(mainLayout);
    setCentralWidget(w);
#else
    mainLayout->setMenuBar(toolBar);
    setLayout(mainLayout);
#endif
    resize(300,300);
    retranslateUI();
}

void SettingsDialog::retranslateUI()
{
    setWindowTitle(tr("Qoobar settings"));
    Q_FOREACH (ConfigPage *page,configPages) page->retranslateUI();

    QList<QAction *> actions = toolBar->actions();
    for (int i=0; i<configPages.size(); ++i) {
        actions[i]->setText(configPages.at(i)->description());
    }

    resetSettingsButton->setText(tr("Reset Settings"));
}

void SettingsDialog::accept()
{
    for (int i=0; i<pagesWidget->count(); ++i) {
        ConfigPage *page = qobject_cast<ConfigPage *>(pagesWidget->widget(i));
        if (page)
            page->saveSettings();
    }
#ifdef Q_OS_MAC
    close();
#else
    QDialog::accept();
#endif
}

void SettingsDialog::resetSettings()
{
    App->resetSettings();
    for (int i=0; i<pagesWidget->count(); ++i) {
        ConfigPage *page = qobject_cast<ConfigPage *>(pagesWidget->widget(i));
        if (page)
            page->setSettings();
    }
}

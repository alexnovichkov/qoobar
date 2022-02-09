/**
 * \file tagsfiller.cpp
 * Dialog for filling tags from files names.
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

#include "tagsfiller.h"

#include <QtWidgets>

#include "application.h"
#include "logging.h"
#include "qoobarhelp.h"
#include "generatorwidget.h"
#include "onlinewidget.h"

TagsFillDialog::TagsFillDialog(const QList<Tag> &oldTags, QWidget *parent)
    : QDialog(parent)
{DD;
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Fill tags"));

    newTags = oldTags;
    tab = new QTabWidget(this);

    generator = new GeneratorWidget(oldTags, this);
    tab->addTab(generator, tr("from other tags"));

    onlineWidget = new OnlineWidget(oldTags, this);
    tab->addTab(onlineWidget,tr("from network"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

#ifdef OSX_SUPPORT_ENABLED
    QButton *helpButton = new QButton(this, QButton::HelpButton);
#else
    QPushButton *helpButton = buttonBox->addButton(QDialogButtonBox::Help);
#endif
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));

    auto *layout = new QVBoxLayout;
    layout->addWidget(tab);
#ifdef OSX_SUPPORT_ENABLED
    QHBoxLayout *boxL = new QHBoxLayout;
    boxL->addWidget(helpButton);
    boxL->addWidget(buttonBox);
    layout->addLayout(boxL);
#else
    layout->addWidget(buttonBox);
#endif
    setLayout(layout);

    if (App->tagsFillerGeometry.isEmpty())
        resize(qApp->primaryScreen()->availableSize()/2);
    else
        restoreGeometry(App->tagsFillerGeometry);
}

TagsFillDialog::~TagsFillDialog()
{DD;
    App->tagsFillerGeometry = saveGeometry();
}

void TagsFillDialog::accept()
{DD;
    if (tab->currentIndex()==0) newTags = generator->getTags();
    if (tab->currentIndex()==1) newTags = onlineWidget->getTags();

    QDialog::accept();
}

void TagsFillDialog::showHelp()
{DD;
    Qoobar::showHelp(tab->currentIndex()==0?QSL("filling"):QSL("importing"));
}


void TagsFillDialog::keyPressEvent(QKeyEvent *event)
{DD;
    if ((event->key() == Qt::Key_Return || event->key()==Qt::Key_Enter)
            && tab->currentIndex()==1) {
        if (onlineWidget) onlineWidget->startSearch();
    }

    else QWidget::keyPressEvent(event);
}

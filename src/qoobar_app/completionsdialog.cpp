/**
 * \file composersdialog.cpp
 * A dialog where you can edit composers names.
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

#include "completionsdialog.h"
#include <QtWidgets>
#include "qoobarglobals.h"
#include "application.h"
#include "enums.h"

CompletionsDialog::CompletionsDialog(const int &tagID, QWidget *parent) :
    QDialog(parent) , m_tagID(tagID)
{DD;
    setWindowTitle(App->currentScheme->localizedFieldName[tagID]);


    edit = new QPlainTextEdit(App->autocompletions->variants(tagID).join(QSL("\n")),this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save |
                                                       QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    auto *layout = new QVBoxLayout;
    layout->addWidget(edit);
    layout->addWidget(buttonBox);
    setLayout(layout);

    //TODO: this->devicePixelRatio()
    resize(::dpiAwareSize(App->primaryScreen()->availableSize().width()/4,
                          App->primaryScreen()->availableSize().height()/3,this));
}

void CompletionsDialog::accept()
{DD;
    QString text = edit->toPlainText();
    App->autocompletions->set(m_tagID, text.split(QSL("\n")));
    QDialog::accept();
}

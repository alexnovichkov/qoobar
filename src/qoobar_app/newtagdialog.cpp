/**
 * \file newtagdialog.h
 * A dialog to choose new tag name
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
 * the Free Software Foundation; either version 3 of the License, or
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

#include "newtagdialog.h"
#include <QtWidgets>
#include "enums.h"
#include "qoobarglobals.h"

NewTagDialog::NewTagDialog(QWidget *parent) :
    QDialog(parent)
{DD;
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("New tag"));
    edit = new QLineEdit(this);
    edit->setValidator(new QRegExpValidator(QRegExp(QSL("[A-Z][A-Z0-9/ _]+"),Qt::CaseInsensitive),this));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Tag name:"),this));
    layout->addWidget(edit);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

void NewTagDialog::accept()
{DD;
    newTagName = edit->text();
    QDialog::accept();
}

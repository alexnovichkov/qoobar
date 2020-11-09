/**
 * \file schemeeditor.cpp
 * A dialog where you can edit tagging schemes.
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

#include "schemeeditor.h"
#include <QtWidgets>
#include "qoobarglobals.h"
#include "taggingscheme.h"
#include "application.h"
#include "applicationpaths.h"

#include "enums.h"

/**
 * @brief uniqueSchemeName
 * @param name - original scheme name
 * @return next unique file name with appended (%1)
 */
QString uniqueSchemeName(const QString &name)
{DD;
    QFileInfoList schemeFiles = QDir(ApplicationPaths::userSchemesPath()).entryInfoList(QStringList(QLS("*.xml")))
            +QDir(ApplicationPaths::schemesPath()).entryInfoList(QStringList(QLS("*.xml")));

    QStringList schemeFilesList;
    Q_FOREACH (const QFileInfo &schemeFile, schemeFiles) {
        TaggingScheme scheme(schemeFile.canonicalFilePath());
        scheme.read(true);
        QString schemeName = scheme.name();
        if (!schemeName.isEmpty()) {
            schemeFilesList << schemeName;
        }
    }

    schemeFilesList << QObject::tr("Default");
    schemeFilesList.removeDuplicates();

    int index = 0;

    // if the name ends in ' (1)' remove the suffix
    QString work = name;
    QRegExp re(" \\((\\d+)\\)$");
    if (re.indexIn(name) != -1) {
        work.chop(re.matchedLength());
        index = re.cap(1).toInt();
    }

    index++;
    while (schemeFilesList.contains(QString("%1 (%2)").arg(work).arg(index))) {
        index++;
    }
    return QString("%1 (%2)").arg(work).arg(index);
}

SchemeEditor::SchemeEditor(const QString &file, Operation operation, QWidget *parent) :
    QDialog(parent)
{DD;

    setWindowTitle(tr("Scheme editor"));

    m_operation = operation;

    scheme = new TaggingScheme(file);
    scheme->read();
    edit = new QLineEdit(this);

    if (m_operation==Modify) {
        edit->setText(scheme->name());
        edit->setReadOnly(true);
        fileName = file;
    }
    else if (m_operation==Copy) {
//        static int newSchemeIndex = 1;
        edit->setText(uniqueSchemeName(scheme->name()));
//        edit->setText(QString("%1 (%2)").arg(scheme->name()).arg(newSchemeIndex));
//        newSchemeIndex++;
    }

    const int tagsCount = scheme->tagsCount();
    table = new QTableWidget(tagsCount, 5, this);
    table->setHorizontalHeaderLabels(QStringList()<<"ID3v2"
                                     <<"APE"<<"Vorbis"<<"MP4"<<"ASF");

    connect(table->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(yHeader(int)));
    connect(table,SIGNAL(currentCellChanged(int,int,int,int)),SLOT(updateRemoveRowAct()));

    for (int tagID=0; tagID<tagsCount; ++tagID) {
        for (int tagType=0; tagType<5; ++tagType) {
            QString text = scheme->fields(tagID,(TaggingScheme::TagType)tagType).join(";");
            QTableWidgetItem *item = new QTableWidgetItem(text);
            if (text=="internal") item->setFlags(Qt::ItemIsSelectable);
            table->setItem(tagID,tagType,item);
        }
        table->setVerticalHeaderItem(tagID, new QTableWidgetItem(scheme->localizedFieldName[tagID]));
    }

    addFieldAct = new QAction("+",this);
    addFieldAct->setToolTip(tr("Add tag"));
    removeFieldAct = new QAction(QString(0x2012),this);
    removeFieldAct->setToolTip(tr("Remove tag"));
    connect(addFieldAct, SIGNAL(triggered()), SLOT(addField()));
    connect(removeFieldAct, SIGNAL(triggered()), SLOT(removeField()));
    QToolButton *addFieldButton = new QToolButton(this);
    addFieldButton->setDefaultAction(addFieldAct);
    QToolButton *removeFieldButton = new QToolButton(this);
    removeFieldButton->setDefaultAction(removeFieldAct);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save |
                                                       QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *hl = new QHBoxLayout;
    hl->setContentsMargins(0,0,0,0);
    hl->addWidget(addFieldButton);
    hl->addWidget(removeFieldButton);
    hl->addWidget(buttonBox);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(edit);
    layout->addWidget(table);
    layout->addLayout(hl);
    setLayout(layout);
    edit->selectAll();

    resize(qApp->primaryScreen()->availableSize()/2);
}

SchemeEditor::~SchemeEditor()
{DD;

    delete scheme;
}

void SchemeEditor::accept()
{DD;
    QString text = edit->text();
    scheme->setName(text);

    if (m_operation==Copy) {
        text.remove(QSL("\\"));
        text.remove(QSL("/"));
        text.remove(QSL(":"));
        text.remove(QSL("*"));
        text.remove(QSL("?"));
        text.remove(QSL("\""));
        text.remove(QSL("<"));
        text.remove(QSL(">"));
        text.remove(QSL("|"));
        scheme->setFilePath(ApplicationPaths::userSchemesPath() + "/" + text + ".xml");
    }
    //writing of scheme
    scheme->clear();


    for (int i=0; i<table->rowCount(); ++i) {
        scheme->addField(i<TAGSCOUNT?Qoobar::untranslatedTagByID(i):table->verticalHeaderItem(i)->text(), i);
        for (int j=0; j<5; ++j) {
            scheme->setFields(i,(TaggingScheme::TagType)j, table->item(i,j)->text().split(";"));
        }
    }

    if (scheme->save()) {
        QDialog::accept();
    }
    else
        QDialog::reject();
}

void SchemeEditor::addField()
{DD;
    bool ok;
    QString text = QInputDialog::getText(this, tr("Tag name"), "", QLineEdit::Normal,
                                         "", &ok);

    if (!ok) return;

    int row = table->rowCount();
    table->setRowCount(row+1);

    for (int tagType=0; tagType<5; ++tagType) {
        QTableWidgetItem *item = new QTableWidgetItem();
        table->setItem(row, tagType, item);
    }
    QTableWidgetItem *headerItem = new QTableWidgetItem(text);

    table->setVerticalHeaderItem(row, headerItem);
}

void SchemeEditor::removeField()
{DD;
    const int row = table->currentRow();
    if (row < TAGSCOUNT) return;

    table->removeRow(row);
}

void SchemeEditor::yHeader(int section)
{DD;
    if (section < TAGSCOUNT) return;

    bool ok;
    QString text = QInputDialog::getText(this, tr("Tag name"), "", QLineEdit::Normal,
                                         table->verticalHeaderItem(section)->text(), &ok);

    if (ok)
        table->verticalHeaderItem(section)->setText(text);
}

void SchemeEditor::updateRemoveRowAct()
{DD;
    int row = table->currentRow();
    removeFieldAct->setEnabled(row >= TAGSCOUNT);
}


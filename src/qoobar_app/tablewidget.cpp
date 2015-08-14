/**
 * \file tablewidget.cpp
 * QTableWidget with additional events.
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

#include "tablewidget.h"
#include "qoobarglobals.h"
#include "completerdelegate.h"
#include "application.h"
#include "displayedtagsdialog.h"



#include "enums.h"

TableWidget::TableWidget(QWidget* parent) : QTableWidget(App->currentScheme->tagsCount(),1,parent),
    count(0)
{DD;

    setContextMenuPolicy(Qt::ActionsContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setWordWrap(false);

#ifdef Q_OS_MAC
    setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    //setAttribute(Qt::WA_MacShowFocusRect, false);
    setAutoFillBackground(true);
#endif

    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<tagsCount; ++i) {
        addRow(i);
    }
    viewport()->setAttribute(Qt::WA_Hover);

    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setVisible(false);
    setVerticalHeaderLabels(QVector<QString>(tagsCount,QString()).toList());
//    if (!App->rows.isEmpty())
//        verticalHeader()->restoreState(App->rows);
    verticalHeader()->SETSECTIONRESIZEMODE(QHeaderView::ResizeToContents);
    int height=verticalHeader()->sectionSize(0);
    verticalHeader()->SETSECTIONRESIZEMODE(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(height);
    verticalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
#ifdef HAVE_QT5
    verticalHeader()->setSectionsMovable(true);
#else
    verticalHeader()->setMovable(true);
#endif

    for (int i=tagsCount-1; i>=0; --i) {
        verticalHeader()->setSectionHidden(i,App->tagStatus.value(i)==HIDDEN);
    }

    QAction *a=new QAction(this);
    connect(a,SIGNAL(triggered()),this,SLOT(adjustDisplayedTags()));
    verticalHeader()->addAction(a);
    QAction *a1=new QAction(this);
    connect(a1,SIGNAL(triggered()),this,SLOT(addTagToScheme()));
    verticalHeader()->addAction(a1);

    connect(verticalHeader(),SIGNAL(sectionMoved(int,int,int)),SLOT(updateTableHeaderGeometry()));
}

void TableWidget::retranslateUi()
{DD;
    const int tagscount = App->currentScheme->tagsCount();
    for (int i=0; i<tagscount; ++i)
        verticalHeaderItem(i)->setText(App->currentScheme->localizedFieldName[i]);
    verticalHeader()->actions().at(0)->setText(tr("Show/hide tags..."));
    verticalHeader()->actions().at(1)->setText(tr("Add this tag to the scheme"));
}

void TableWidget::updateRow(int row, const QString &text)
{DD;
    if (row<0 || row > rowCount()) return;
    item(row, 0)->setText(text);
    if (row<App->currentScheme->tagsCount())
        setRowHidden(row, App->tagStatus.value(row)==HIDDEN
                     || (App->tagStatus.value(row)==VISIBLEIFNOTEMPTY && text.isEmpty()));
}

void TableWidget::clearTable()
{DD;
    blockSignals(true);
    setCount(0);

    const int tagsCount = App->currentScheme->tagsCount();
    setRowCount(tagsCount);
    for (int i=0; i<tagsCount; ++i) {
        if (item(i,0)) {
            item(i,0)->setText(QString());
            verticalHeaderItem(i)->setText(App->currentScheme->localizedFieldName[i]);
        }
        else
            addRow(i);
    }

    blockSignals(false);
    resizeColumnsToContents();
}

void TableWidget::adjustDisplayedTags()
{DD;
    DisplayedTagsDialog dialog(this);
    connect(&dialog,SIGNAL(tagStatusChanged(int,int)),SLOT(toggleTableRow(int,int)));
    dialog.exec();
}

void TableWidget::mouseDoubleClickEvent(QMouseEvent *event)
{DD;

    if (!event || count==0) return;
    if (!itemAt(event->pos())) return;
    if (event->button()==Qt::LeftButton && count>1)
        Q_EMIT cellClicked();
    else
        QTableWidget::mouseDoubleClickEvent(event);
}

void TableWidget::updateTableHeaderGeometry()
{DD;

    App->rows=verticalHeader()->saveState();
}

void TableWidget::toggleTableRow(int row,int status)
{DD;

    verticalHeader()->setSectionHidden(row,status==HIDDEN
                                       || (status==VISIBLEIFNOTEMPTY
                                           && item(row,0)->text().isEmpty()));
    if (App->tagStatus.size()<=row) App->tagStatus.resize(row+1);
    App->tagStatus[row]=status;
    updateTableHeaderGeometry();
}

void TableWidget::handleSentTag(int tagID, bool fromTable)
{DD;
    Q_UNUSED(fromTable)

    QLineEdit *lineEdit=qobject_cast<QLineEdit *>(qApp->focusWidget());
    if (lineEdit) {
        const QString selectedText = lineEdit->selectedText();
        if (!selectedText.isEmpty()) {
            const QStringList destination = QVector<QString>(count, selectedText).toList();
            Q_EMIT tagsSent(tagID, destination);
        }
    }
}

void TableWidget::addTagToScheme()
{DD;
    int row = currentRow();
    if (row < TAGSCOUNT) {
        QMessageBox::warning(this, tr("Qoobar - adding tag"), tr("This tag is already present in the current tagging scheme"));
        return;
    }

    TaggingScheme *scheme = App->currentScheme;
    const int tagsCount = scheme->tagsCount();
    QString tagName = verticalHeaderItem(row)->text();
    int tagID = scheme->tagIDByName(tagName);
    if (tagID < 0) {
        scheme->addField(tagName, tagsCount);
        tagID = tagsCount;
        scheme->setFields(tagID, TaggingScheme::ID3, QStringList()<<("TXXX:"+tagName));
        scheme->setFields(tagID, TaggingScheme::APE, QStringList()<<tagName);
        scheme->setFields(tagID, TaggingScheme::ASF, QStringList()<<tagName);
        scheme->setFields(tagID, TaggingScheme::VORBIS, QStringList()<<tagName);
        scheme->setFields(tagID, TaggingScheme::MP4, QStringList()<<("----:com.apple.iTunes:"+tagName));
        scheme->save();
        if (scheme->name()==QSL("Default")) App->currentSchemeName = scheme->filePath();
        scheme->retranslateUI();
    }
    else
        QMessageBox::warning(this, tr("Qoobar - adding tag"), tr("This tag is already present in the current tagging scheme"));
}

QString TableWidget::tag(int row) const
{DD;

    return verticalHeaderItem(row)->text();
}

void TableWidget::addRow(const QString &header)
{DD;
    int c=rowCount();
    blockSignals(true);
    setRowCount(c+1);
    setItem(c,0,new QTableWidgetItem(QString()));
    setVerticalHeaderItem(c,new QTableWidgetItem(header));
    blockSignals(false);
}

void TableWidget::addRow(int row)
{
    blockSignals(true);
    setItem(row,0,new QTableWidgetItem(QString()));
    CompleterDelegate *d = new CompleterDelegate(row,false,this);
    connect(d,SIGNAL(tagChanged(int,bool)),SLOT(handleSentTag(int,bool)));
    setItemDelegateForRow(row,d);
    setVerticalHeaderItem(row,new QTableWidgetItem(App->currentScheme->localizedFieldName[row]));
    blockSignals(false);
}

QVector<int> TableWidget::selectedRows()
{DD;

    QVector<int> result;
    for (int i=0; i<rowCount(); ++i) {
        if (item(i,0)->isSelected()) result << i;
    }
    return result;
}

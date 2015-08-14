/**
 * \file completerdelegate.cpp
 * Delegate for tags table with completer support.
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

#include "completerdelegate.h"
#include "lineedit.h"
#include "application.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "delegatehighlighter.h"

CompleterDelegate::CompleterDelegate(int type, bool useInTagsEditor, QObject *parent)
    : QStyledItemDelegate(parent), m_type(type), useInTagsEditor_(useInTagsEditor), highlighter(0)
{

}

QWidget *CompleterDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    LineEdit *edit = new LineEdit(useInTagsEditor_, parent);

    connect(edit,SIGNAL(tagChanged(int,bool)),this,SIGNAL(tagChanged(int,bool)));
    connect(edit,SIGNAL(editingFinished()),SLOT(commitAndCloseEditor()));
    connect(edit,SIGNAL(selectionChanged()),this,SIGNAL(selectionChanged()));

    if (App->autocompletions->use(m_type))
        edit->setCompleter(new MyCompleter(m_type,edit));
    edit->setFocus();
    return edit;
}

void CompleterDelegate::commitAndCloseEditor()
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
    if (!edit) return;
    edit->setCompleter(0);
    Q_EMIT commitData(edit);
    Q_EMIT closeEditor(edit);
}

void CompleterDelegate::setHighlighter(DelegateHighlighter *highlighter)
{
    this->highlighter = highlighter;
}

void CompleterDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!highlighter || (option.state & QStyle::State_Selected)) {
        QStyledItemDelegate::paint(painter,option,index);
        return;
    }

    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);
    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);

    QList<QTextLayout::FormatRange> formats=highlighter->generateFormats(&optionV4,textRect,index);

    QTextLayout textLayout;
    textLayout.setText(optionV4.text);
    textLayout.setAdditionalFormats(formats);

    QTextOption textOption = textLayout.textOption();
    textOption.setWrapMode(QTextOption::NoWrap);
    textLayout.setTextOption(textOption);

    textLayout.beginLayout();
    QTextLine line = textLayout.createLine();
    if (line.isValid()) {
        int indent=3;
        line.setLineWidth(textRect.width()-2*indent);
        qreal vertPos=line.height();
        vertPos = vertPos <= textRect.height()?(textRect.height()-vertPos)/2.0 : 0.0;
        line.setPosition(QPointF(2,vertPos));
    }
    textLayout.endLayout();
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    textLayout.draw(painter,QPointF(0,0));
    painter->restore();
}

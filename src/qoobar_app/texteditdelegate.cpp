/**
 * \file texteditdelegate.cpp
 * A delegate with QPlainTextEdit
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

#include "texteditdelegate.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "qoobarglobals.h"

QWidget *TextEditDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem & option ,
                                        const QModelIndex & index ) const
{DD
    Q_UNUSED(option)
    Q_UNUSED(index)
    QPlainTextEdit *edit=new QPlainTextEdit(parent);
    return edit;
}

void TextEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{DD;
    if (!index.isValid() || !editor) return;

    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(editor);
    if (edit) {
        edit->setPlainText(index.data().toString());
        edit->moveCursor(QTextCursor::Start);
    }
}

void TextEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const
{DD;
    if (!index.isValid() || !editor || !model) return;
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(editor);
    if (edit)
        model->setData(index, edit->toPlainText(), Qt::DisplayRole);
}

void TextEditDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{DD;
    Q_UNUSED(index)
    QRect rect = option.rect;
    rect.setHeight(rect.height()*3);
    if (editor) editor->setGeometry(rect);
}

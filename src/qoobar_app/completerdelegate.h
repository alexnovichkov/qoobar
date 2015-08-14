/**
 * \file completerdelegate.h
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

#ifndef COMPLETERDELEGATE_H
#define COMPLETERDELEGATE_H

#include <QStyledItemDelegate>

class DelegateHighlighter;

class CompleterDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
    CompleterDelegate(int type, bool useInTagsEditor = false, QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setHighlighter(DelegateHighlighter *highlighter);
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
Q_SIGNALS:
    void tagChanged(int,bool);
    void selectionChanged();
private Q_SLOTS:
    void commitAndCloseEditor();
private:
    int m_type;
    bool useInTagsEditor_;
    DelegateHighlighter *highlighter;
};


#endif // COMPLETERDELEGATE_H

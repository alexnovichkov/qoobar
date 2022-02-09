/**
 * \file filedelegatehighlighter.cpp
 * A DelegateHighlighter for file names
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date Nov 27 2012
 *
 * Copyright (C) 2012  Alex Novichkov
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

#include "filedelegatehighlighter.h"

#include <QtWidgets>
#include "qoobarglobals.h"
#include "application.h"

FileDelegateHighlighter::FileDelegateHighlighter()
{
}

QList<QTextLayout::FormatRange> FileDelegateHighlighter::generateFormats(
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
        QStyleOptionViewItemV4
#else
        QStyleOptionViewItem
#endif
        *option,
        const QRect &textRect,
        const QModelIndex &index)
{DD;
    QList<QTextLayout::FormatRange> formats;

    QString text = index.data().toString();
    QString elidedText = option->fontMetrics.elidedText(text, Qt::ElideLeft, textRect.width());
    QString prefix = index.data(Qt::UserRole+1).toString();

    if (!text.startsWith(prefix)) {
        formats << formatRange(0,elidedText.length(),charFormat(option->font));
    }
    else {
        int length=elidedText.length()-text.length()+prefix.length()+1;
        if (length<0) length=0;
        if (length>0) {
            formats << formatRange(0,length, charFormat(option->font, QBrush(QColor(App->alternateTextColor))));
        }
        if (elidedText.length()-length>0) {
            formats << formatRange(length, elidedText.length()-length, charFormat(option->font));
        }
    }
    option->text = elidedText;

    return formats;
}

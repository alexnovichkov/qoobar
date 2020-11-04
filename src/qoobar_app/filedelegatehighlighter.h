/**
 * \file filedelegatehighlighter.h
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

#ifndef FILEDELEGATEHIGHLIGHTER_H
#define FILEDELEGATEHIGHLIGHTER_H

#include "delegatehighlighter.h"

class FileDelegateHighlighter : public DelegateHighlighter
{
public:
    FileDelegateHighlighter();
    QList<QTextLayout::FormatRange> generateFormats(
        #if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
            QStyleOptionViewItemV4
        #else
            QStyleOptionViewItem
        #endif
            *, const QRect &, const QModelIndex &);
};

#endif // FILEDELEGATEHIGHLIGHTER_H

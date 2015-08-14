/**
 * \file delegatehighlighter.cpp
 * A helper class for highlighting text in delegates
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

#include "delegatehighlighter.h"
#include <QApplication>

QTextCharFormat charFormat(const QFont &font, const QBrush &foreground)
{
    QTextCharFormat format;
    format.setFont(font);
    format.setForeground(foreground);
    return format;
}

QTextCharFormat selectionFormat(const QFont &font)
{
    QTextCharFormat format;
    format.setFont(font);
    format.setForeground(QApplication::palette().brush(QPalette::HighlightedText));
    format.setBackground(QApplication::palette().brush(QPalette::Highlight));
    return format;
}

QTextCharFormat charFormat(const QFont &font)
{
    QTextCharFormat format;
    format.setFont(font);
    return format;
}

QTextLayout::FormatRange formatRange(int start, int length, const QTextCharFormat &charFormat)
{
    QTextLayout::FormatRange format;
    format.start=start;
    format.length=length;
    format.format = charFormat;
    return format;
}

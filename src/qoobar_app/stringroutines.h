/**
 * \file stringroutines.h
 * Various string functions
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

#ifndef STRINGROUTINES_H
#define STRINGROUTINES_H

#include <QString>
#include <QHash>
#include "enums.h"
#include "qoobarglobals.h"

struct Operated
{
    Operated(const QString &type);
    typedef QString result_type;
    QString operator()(const QString &s);
    QString m_type;
};

typedef QString (*RenderFunction)(QStringList &, int);
typedef QHash<QString, RenderFunction> FunctionsHash;

QRegularExpression createRegExp(bool caseSensitive, bool useRegularExpressions,
                     bool wholeWord, QString pattern);

QString processFunction(const QString &functionName, QStringList &args, int index);

QString replaceWinChars(const QString &s, const QString &byWhat, bool useSmartReplacing);

QString removeDiacritics(const QString &s, bool replaceOthers = false);

QString changeCase(QString s, const Case::Case ca);

QString transliterate(const QString &s);

QString operatedString(const QString &s, const QString &type);

#endif // STRINGROUTINES_H

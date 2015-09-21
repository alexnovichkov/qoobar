/**
 * \file argsparser.h
 * Arguments parser and command line processor
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 10 Apr 2013
 *
 * Copyright (C) 2013  Alex Novichkov
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

#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#ifdef QOOBAR_ENABLE_CLI

#include <QStringList>
#include <QObject>

class CLParser;
class Model;

class CommandLineProcessor : public QObject
{
    Q_OBJECT
public:
    CommandLineProcessor(QObject *parent = 0);

Q_SIGNALS:
    void finished();
public Q_SLOTS:
    void parse();
private Q_SLOTS:
    void appendMessage(QString message, int indentSpaces=0, int secondLineOffset=0);
    void showMessage(int type, const QString &text);
    void addText(const QString &text);
    void tick();
private:
    void printFiles(const QStringList &files);
    void setBool(bool &dest, const QString &id, const QString &message, CLParser *parser);
    void setTag(CLParser *parser, Model *model, const QString &id, int row, const QString &mess, const QString &mess1);
    QStringList files;
};

#endif //QOOBAR_ENABLE_CLI

#endif // ARGSPARSER_H

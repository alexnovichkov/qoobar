/**
 * \file undoactions.h
 * Reimplementations of QUndoCommand.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 5 Aug 2010
 *
 * Copyright (C) 2009, 2010  Alex Novichkov
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

#ifndef UNDOACTIONS_H
#define UNDOACTIONS_H

#include <QUndoCommand>
#include <QList>
#include <QVector>
#include "tagger.h"

class Tab;

class ChangeFilesCommand : public QUndoCommand
{
public:
    ChangeFilesCommand(const QVector<int> &indexes,
                       const QList<Tag> &oldtags,
                       const QList<Tag> &newtags,
                       const QVector<int> &rows,
                       const QString &s,
                       Tab *t);
    ChangeFilesCommand(const QVector<int> &indexes,
                       const QList<Tag> &oldtags,
                       const QVector<int> &rows,
                       const QString &s,
                       Tab *t);
    void add(const int row, const QString &key, const QStringList &newValues);
    virtual void undo();
    virtual void redo();
    bool changed() const {return oldTags !=newTags;}
private:
    QVector<int> changedIndexes;
    QList<Tag> oldTags;
    QList<Tag> newTags;
    QVector<int> changedRows;
    Tab *tab;
};


class ChangeCommand : public QUndoCommand
{
public:
    ChangeCommand(Tab *t);
    virtual void undo();
    virtual void redo();
private:
    Tab *tab;
};

class ChangeImageCommand : public QUndoCommand
{
public:
    ChangeImageCommand(int index,
                       const CoverImage &oldImage,
                       const CoverImage &newImage,
                       Tab *tab,QUndoCommand *parent=0);
    virtual void undo();
    virtual void redo();
private:
    int ind;
    CoverImage oldimg;
    CoverImage newimg;
    Tab *t;
    bool status;
};

#endif // UNDOACTIONS_H

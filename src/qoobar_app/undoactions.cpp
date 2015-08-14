/**
 * \file undoactions.cpp
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

#include "undoactions.h"

#include "tab.h"
#include "model.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "qoobarglobals.h"
#include "application.h"

//change files
ChangeFilesCommand::ChangeFilesCommand(const QVector<int> &indexes,
                                       const QList<Tag> &oldtags,
                                       const QList<Tag> &newtags,
                                       const QVector<int> &rows,
                                       const QString &s,
                                       Tab *t)
: changedIndexes(indexes), oldTags(oldtags), newTags(newtags),
  changedRows(rows), tab(t)
{DD;
    setText(s);
}

ChangeFilesCommand::ChangeFilesCommand(const QVector<int> &indexes,
                                       const QList<Tag> &oldtags,
                                       const QVector<int> &rows,
                                       const QString &s, Tab *t)
    : changedIndexes(indexes), oldTags(oldtags),
      changedRows(rows), tab(t)
{DD;
    setText(s);
    newTags = oldTags;
}

void ChangeFilesCommand::add(const int row, const QString &key, const QStringList &newValues)
{
    int size=qMin(changedIndexes.size(), newValues.size());
    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<size; ++i) {
        if (row<tagsCount)
            newTags[i].setTag(row, newValues.at(i));
        else
            newTags[i].setUserTag(key, newValues.at(i));
    }
}

void ChangeFilesCommand::undo()
{DD;
    tab->setTags(changedIndexes,oldTags,changedRows);
}

void ChangeFilesCommand::redo()
{DD;
    tab->setTags(changedIndexes,newTags,changedRows);
}




//change images
ChangeCommand::ChangeCommand(Tab *t) : tab(t)
{DD;
}

void ChangeCommand::undo()
{DD
    QUndoCommand::undo();
    tab->setStatus();
}

void ChangeCommand::redo()
{DD
    QUndoCommand::redo();
    tab->setStatus();
}

ChangeImageCommand::ChangeImageCommand(int index,
                                       const CoverImage &oldImage,
                                       const CoverImage &newImage,
                                       Tab *tab, QUndoCommand *parent)
: QUndoCommand(parent), ind(index), oldimg(oldImage), newimg(newImage), t(tab)
{DD
}

void ChangeImageCommand::undo()
{DD
    t->model->setOldImage(ind,oldimg,status);
}

void ChangeImageCommand::redo()
{DD
    status=t->model->setNewImage(ind,newimg);
}

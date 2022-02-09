/**
 * \file tagsfiller.h
 * Dialog for filling tags from files names.
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

#ifndef TAGSFILLER_H
#define TAGSFILLER_H

#include <QDialog>

#include "tagger.h"

class GeneratorWidget;
class OnlineWidget;
class QTabWidget;

class TagsFillDialog : public QDialog
{
Q_OBJECT

public:
    explicit TagsFillDialog(const QList<Tag> &,QWidget *parent = 0);
    ~TagsFillDialog();
    QList<Tag> getNewTags() {return newTags;}
public Q_SLOTS:
    void accept() override;
private Q_SLOTS:
    void showHelp();
private:
    QList<Tag> newTags;
    QTabWidget *tab;

    GeneratorWidget *generator = nullptr;
    OnlineWidget *onlineWidget = nullptr;

    // QWidget interface
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};

#endif


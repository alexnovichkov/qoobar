/**
 * \file legendbutton.h
 * A button with drop-down menu and some signals
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

#include "legendbutton.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "enums.h"
#include "placeholders.h"
#include "application.h"

LegendButton::LegendButton(QWidget *parent) :
    QPushButton(parent), categories(NoPlaceholders), menu(new QMenu(this))
{
    setText(tr("Legend"));

    connect(menu,SIGNAL(triggered(QAction *)),SLOT(actionTriggered(QAction *)));
    setMenu(menu);
}

bool LegendButton::event(QEvent *e)
{
    if (e->type()==QEvent::LanguageChange) {
        retranslateUi();
        return false;
    }
    return QPushButton::event(e);
}

void LegendButton::retranslateUi()
{
    setText(tr("Legend"));
    menu->clear();

    if (categories & WritablePlaceholders) {
        QString s = Placeholders::getWritablePlaceholders();

        QMenu *otherMenu = new QMenu(tr("Other"),menu);
        for (int i=0; i<s.length(); ++i) {
            QAction *a = new QAction(Placeholders::placeholderToDescription(s.at(i)),menu);
            a->setData(s.at(i));
            if (Placeholders::placeholderToId(QString(s.at(i))) < CATEGORY)
                menu->addAction(a);
            else
                otherMenu->addAction(a);
        }
        menu->addMenu(otherMenu);

        const int tagsCount = App->currentScheme->tagsCount();
        if (tagsCount > TAGSCOUNT) {
            QMenu *userMenu = new QMenu(tr("User"),menu);
            for (int i=TAGSCOUNT; i<tagsCount; ++i) {
                QAction *a = new QAction(QString("<%1>").arg(App->currentScheme->localizedFieldName[i]),menu);
                //a->setData(s.at(i));
                userMenu->addAction(a);
            }
            menu->addMenu(userMenu);
        }
    }

    if (categories & ReadOnlyPlaceholders) {
        QString s = Placeholders::getReadOnlyPlaceholders();
        QMenu *otherMenu = new QMenu(tr("Properties"), menu);
        for (int i=0; i<5; ++i) {
            QAction *a = new QAction(Placeholders::placeholderToDescription(s.at(i)),menu);
            a->setData(s.at(i));
            otherMenu->addAction(a);
        }
        menu->addMenu(otherMenu);

        QMenu *otherMenu1 = new QMenu(tr("File name"), menu);
        for (int i=5; i<s.length(); ++i) {
            QAction *a = new QAction(Placeholders::placeholderToDescription(s.at(i)),menu);
            a->setData(s.at(i));
            otherMenu1->addAction(a);
        }
        menu->addMenu(otherMenu1);
    }
    if (categories & VoidPlaceholder) {
        QString s = Placeholders::getVoidPlaceholder();
        QAction *a=new QAction(Placeholders::placeholderToDescription(s.at(0)),menu);
        a->setData(s.at(0));
        menu->addAction(a);
    }
}

void LegendButton::actionTriggered(QAction *a)
{
    QString data = a->data().toString();
    if (!data.isEmpty()) Q_EMIT placeholderChosen(QString("%%1").arg(data));
    else Q_EMIT placeholderChosen(a->text());
}

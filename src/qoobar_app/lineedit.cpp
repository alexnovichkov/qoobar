/**
 * \file lineedit.cpp
 * QLineEdit with custom completer.
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

#include "lineedit.h"
#include "qoobarglobals.h"
#include "application.h"

#include <QtWidgets>

#include "stringroutines.h"

#include "enums.h"

QMenu *createOperations(QSignalMapper *mapper, QWidget *parent)
{DD
    QMenu *menu = new QMenu(parent);
    static const struct Operation{
        const char *text;
        const char *context;
        const int parent;
        const bool hasMenu;
    } operationsTexts[] = {
        {QT_TRANSLATE_NOOP("QObject","Change case"),         nullptr,        -1,true},
        {QT_TRANSLATE_NOOP("QObject","lower"),               "lower",         0,false},
        {QT_TRANSLATE_NOOP("QObject","UPPER"),               "upper",         0,false},
        {QT_TRANSLATE_NOOP("QObject","Only first up"),       "caps3",         0,false},
        {QT_TRANSLATE_NOOP("QObject","Every First Up"),      "caps",          0,false},
        {QT_TRANSLATE_NOOP("QObject","First Up PrESERVED"),  "caps2",         0,false},
        {QT_TRANSLATE_NOOP("QObject","Simplify whitespaces"),"simplify",     -1,false},
        {QT_TRANSLATE_NOOP("QObject","Remove diacritics"),   "ansi",         -1,false},
        {QT_TRANSLATE_NOOP("QObject","Transliterate"),       "transliterate",-1,false},
        {QT_TRANSLATE_NOOP("QObject","Fix encoding"),        "recode",       -1,false},
        {nullptr,nullptr,-1,false}
    };


    QList<QAction *> actions;
    int i=0;
    while (operationsTexts[i].text) {
        QAction *a = new QAction(QObject::tr(operationsTexts[i].text),parent);

        if (operationsTexts[i].hasMenu) {
            QMenu *m = new QMenu(parent);
            a->setMenu(m);
        }
        if (operationsTexts[i].context) {
            QObject::connect(a,SIGNAL(triggered()),mapper,SLOT(map()));
            mapper->setMapping(a,operationsTexts[i].context);
        }
        if (operationsTexts[i].parent>=0)
            actions[operationsTexts[i].parent]->menu()->addAction(a);
        else {
            actions << a;
            menu->addAction(a);
        }
        i++;
    }
    return menu;
}

MyCompleter::MyCompleter(int type, QObject * parent) :
        QCompleter(parent), m_type(type)
{DD
    setModel(&m_model);
    map = App->autocompletions->variants(type);
}

void MyCompleter::update(const QString &word)
{DD
    QStringList filtered = map.filter(word, Qt::CaseInsensitive);
    if (m_type==COMPOSER) {
        for (int i=0; i<filtered.size(); ++i) {
            if (filtered.at(i).contains(QSL("/"))) {
                filtered[i]=filtered.at(i).section(QSL("/"),1);
            }
        }
    }
    m_model.setStringList(filtered);
    complete();
}

LineEdit::LineEdit(bool useInTable, QWidget *parent)
    : QLineEdit(parent), useInTable_(useInTable), c(nullptr)
{DD
    QSignalMapper *mapper = new QSignalMapper(this);
    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<tagsCount; ++i) {
        QAction *a=new QAction(App->currentScheme->localizedFieldName[i], this);
        mapper->setMapping(a,i);
        connect(a,SIGNAL(triggered()),mapper,SLOT(map()));
        acts << a;
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(tagChosen(int)));

    if (useInTable_) {
        QSignalMapper *mapper1 = new QSignalMapper(this);
        for (int i=0; i<tagsCount; ++i) {
            QAction *a=new QAction(App->currentScheme->localizedFieldName[i],this);
            mapper1->setMapping(a,i);
            connect(a,SIGNAL(triggered()),mapper1,SLOT(map()));
            acts1 << a;
        }
        connect(mapper1,SIGNAL(mapped(int)),this,SLOT(tagChosen1(int)));
    }
}

void LineEdit::tagChosen(int i)
{DD
    if (hasSelectedText()) Q_EMIT tagChanged(i,false);
}

void LineEdit::tagChosen1(int i)
{DD
    if (hasSelectedText()) Q_EMIT tagChanged(i,true);
}

QMenu *LineEdit::createOperationsMenu()
{DD
    QSignalMapper *operationsMapper = new QSignalMapper(this);
    connect(operationsMapper,SIGNAL(mapped(QString)),SLOT(handleOperation(QString)));

    QMenu *menu = createOperations(operationsMapper,this);
    menu->setTitle(tr("Operations"));

    return menu;
}

void LineEdit::contextMenuEvent(QContextMenuEvent *event)
{DD

    QMenu *menu = createStandardContextMenu();

    menu->insertSeparator(menu->actions().at(0));
    QMenu *operationsMenu = createOperationsMenu();
    menu->insertMenu(menu->actions().at(0),operationsMenu);

    if (useInTable_) {
        QMenu *fancySetAsMenu = new QMenu(tr("Set this block into..."),menu);
        for (int i=0; i<CATEGORY; ++i)
            fancySetAsMenu->addAction(acts1[i]);
        QMenu *m = new QMenu(tr("Other"));
        for (int i=CATEGORY; i< acts1.size(); ++i)
            m->addAction(acts1[i]);
        fancySetAsMenu->addMenu(m);
        menu->insertMenu(menu->actions().at(0),fancySetAsMenu);
    }

    QMenu *setAsMenu = new QMenu(tr("Set this text into..."),menu);
    for (int i=0; i<CATEGORY; ++i)
        setAsMenu->addAction(acts[i]);
    QMenu *m = new QMenu(tr("Other"));
    for (int i=CATEGORY; i< acts.size(); ++i)
        m->addAction(acts[i]);
    setAsMenu->addMenu(m);

    menu->insertMenu(menu->actions().at(0),setAsMenu);



    menu->exec(event->globalPos());
    menu->deleteLater();
}

void LineEdit::handleOperation(const QString &type)
{DD
    if (!hasSelectedText()) return;
    insert(operatedString(selectedText(), type));
}

void LineEdit::setCompleter(MyCompleter *completer)
{DD
    if (c)
        QObject::disconnect(c, nullptr, this, nullptr);
    c = completer;
    if (!c)
        return;
    c->setWidget(this);
    connect(c, SIGNAL(activated(QString)),this,SLOT(insertCompletion(QString)));
}

MyCompleter *LineEdit::completer() const
{DD
    return c;
}

void LineEdit::insertCompletion(const QString& completion)
{DD
    if (App->autocompletions->completionStyle) {
        cursorBackward(true,completionPrefix.length());
        insert(completion);
    }
    else {
        setText(completion);
        selectAll();
    }
}

QString findLastWord(const QString &text, int cursorPosition)
{DD
    if (cursorPosition<=0) return QString();
    QString workString = text.left(cursorPosition);
    //now we search word break from the end of the workString
    int lastWordPos = workString.indexOf(QRegExp(QSL("\\b\\w+$")));
    if (lastWordPos>=0)
        return workString.right(workString.length()-lastWordPos);
    return QString();
}

void LineEdit::keyPressEvent(QKeyEvent *e)
{DD
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())  {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // Let the completer do default behavior
        }
    }


    bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
    if (!isShortcut)
        QLineEdit::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    if (!c)
        return;

    bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier) {
        c->popup()->hide();
        return;
    }

    if (App->autocompletions->completionStyle) {
        completionPrefix = findLastWord(text(),this->cursorPosition());
    }
    else
        completionPrefix = text();

    c->update(completionPrefix);
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
}

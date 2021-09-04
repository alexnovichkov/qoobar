/**
 * \file tabwidget.cpp
 * QTabWidget with context menu
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
 * the Free Software Foundation; either version 3 of the License, or
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

#include "tabwidget.h"
#include <QtWidgets>

#include "enums.h"
#include "qoobarglobals.h"
#include "application.h"

#ifdef OSX_SUPPORT_ENABLED
class TabButton : public QToolButton
{
public:
    explicit TabButton(QTabWidget *header, QWidget*parent=0) : QToolButton(parent),
        m_header(header)
    {}
    void paintEvent(QPaintEvent * pe)
    {
        QPainter p(this);

        QStyleOptionTabBarBaseV2 opt;
        opt.initFrom(m_header);
        opt.rect = pe->rect();

        //style()->drawPrimitive(QStyle::PE_FrameTabBarBase,&opt,&p);
        style()->drawItemPixmap(&p,opt.rect,Qt::AlignCenter,this->icon().pixmap(SMALL_ICON_SIZE,SMALL_ICON_SIZE));
        p.end();
    }
private:
    QTabWidget *m_header;
};
#endif

TabBar::TabBar(QWidget *parent) : QTabBar(parent)
{DD;
    setAcceptDrops(true);

    connect(this, SIGNAL(tabCloseRequested(int)), this, SIGNAL(closeTab(int)));
    setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    setMovable(true);
    setDocumentMode(true);

    editor = new QLineEdit(this);
    editor->hide();

    connect(editor, SIGNAL(editingFinished()), this, SLOT(editTabName()));
}

void TabBar::mouseDoubleClickEvent(QMouseEvent * event)
{DD;
    if (event->button()==Qt::LeftButton) {
        index = tabAt(event->pos());

        if (index==-1) {
            Q_EMIT newTab();
        }
        else {
            QRect rect = tabRect(index);

            if( rect.contains(event->pos()))
            {
                //TODO: check dpi awareness
                editor->setGeometry(rect.adjusted(2, 2, -2, -2));
                QString oldTabName = tabText(index);
                if (oldTabName.endsWith(QSL("*"))) oldTabName.chop(1);
                editor->setText(oldTabName);
                editor->selectAll();
                editor->show();
                editor->setFocus();
            }
        }
    }
    QTabBar::mouseDoubleClickEvent(event);
}

void TabBar::keyPressEvent(QKeyEvent *event)
{
    if (!editor->isHidden() && event->key() == Qt::Key_Return) {
        editTabName();
    }
    else QTabBar::keyPressEvent(event);
}

void TabBar::contextMenuEvent(QContextMenuEvent * event)
{DD;
    QMenu menu;
    menu.addAction(tr("New Tab"), this, SIGNAL(newTab()), QKeySequence::AddTab);
    QPoint position = event->pos();
    index = tabAt(position);

    if (index != -1) {
        menu.addAction(tr("Close tab"),this, SLOT(closeTab()), QKeySequence::Close);
        menu.addAction(tr("Close other tabs"), this, SLOT(closeOtherTabs()));
        menu.addSeparator();
        menu.addAction(tr("Rename tab..."), this, SLOT(renameTab()));
    }
    menu.exec(event->globalPos());
}

void TabBar::closeTab()
{DD;
    Q_EMIT closeTab(index);
}

void TabBar::closeOtherTabs()
{DD;
    Q_EMIT closeOtherTabs(index);
}

void TabBar::renameTab()
{DD;
    Q_EMIT renameTab(index);
}

void TabBar::editTabName()
{DD;
    if( index < 0 ) return;
    QString oldText = tabText(index);
    QString text = editor->text();
    editor->hide();
    if (text.isEmpty()) return;

    if (oldText!=text) {
        setTabText(index, text);
        Q_EMIT tabTextChanged(text);
    }
}




#include "qocoa/qbutton.h"

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
    , m_tabBar(new TabBar(this))
{DD;
    connect(m_tabBar, SIGNAL(newTab()), this, SIGNAL(newTab()));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SIGNAL(closeTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SIGNAL(closeOtherTabs(int)));
    connect(m_tabBar, SIGNAL(renameTab(int)), this, SIGNAL(renameTab(int)));
    connect(m_tabBar, SIGNAL(tabTextChanged(QString)), this, SIGNAL(tabTextChanged(QString)));

    setTabBar(m_tabBar);
#ifndef OSX_SUPPORT_ENABLED
    auto *newTabButton = new QToolButton(this);
    auto *a = new QAction(QIcon::fromTheme("list-add"),tr("New tab"),this);
    connect(a,SIGNAL(triggered()),this,SIGNAL(newTab()));
    newTabButton->setDefaultAction(a);
    newTabButton->setAutoRaise(true);
    setCornerWidget(newTabButton);
#endif
}

void TabWidget::hideTabBar(bool hide)
{DD;
    this->tabBar()->setHidden(hide);
}

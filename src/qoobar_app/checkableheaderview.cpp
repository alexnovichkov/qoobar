/**
 * \file checkableheaderview.cpp
 * A header view with checkable columns
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

#include "checkableheaderview.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "enums.h"

CheckableHeaderView::CheckableHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)

{
    SETSECTIONSCLICKABLE(true);
    connect(this,SIGNAL(sectionCountChanged(int,int)),SLOT(updateSectionCount(int,int)));
}

void CheckableHeaderView::setCheckState(int section, Qt::CheckState checkState)
{
    if (isEnabled() && m_isChecked.at(section) != checkState) {
        m_isChecked[section] = checkState;
        updateSection(section);
    }
}

void CheckableHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (m_isCheckable.at(logicalIndex)) {
        QStyleOptionButton option;
        if (isEnabled())
            option.state |= QStyle::State_Enabled;
        option.rect = checkBoxRect(rect);
        if (m_isChecked.at(logicalIndex)==Qt::Checked)
            option.state |= QStyle::State_On;
        else if (m_isChecked.at(logicalIndex)==Qt::Unchecked)
            option.state |= QStyle::State_Off;
        else
            option.state |= QStyle::State_NoChange;
        style()->drawControl(QStyle::CE_CheckBox, &option, painter);
    }
}

void CheckableHeaderView::mousePressEvent(QMouseEvent *event)
{
    int logicalIndex = logicalIndexAt(event->pos());
    if (isEnabled() && m_isCheckable.at(logicalIndex)) {
        if (m_isChecked.at(logicalIndex)==Qt::Checked)
            m_isChecked[logicalIndex] = Qt::Unchecked;
        else
            m_isChecked[logicalIndex] = Qt::Checked;
        updateSection(logicalIndex);
        Q_EMIT toggled(logicalIndex, m_isChecked.at(logicalIndex));
    }
    else QHeaderView::mousePressEvent(event);
}

QRect CheckableHeaderView::checkBoxRect(const QRect &sourceRect) const
{
    QStyleOptionButton checkBoxStyleOption;
    QRect checkBoxRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator,
                                                 &checkBoxStyleOption);
    QPoint checkBoxPoint(sourceRect.x() + 3,
                         sourceRect.y() +
                         sourceRect.height() / 2 -
                         checkBoxRect.height() / 2);
    return QRect(checkBoxPoint, checkBoxRect.size());
}

void CheckableHeaderView::setCheckable(int section, bool checkable)
{
    if (section<0 || section>=count()) return;
    m_isCheckable[section]=checkable;
}

void CheckableHeaderView::updateSectionCount(int oldCount, int newCount)
{
    Q_UNUSED(oldCount)
    m_isChecked.resize(newCount);
    m_isCheckable.resize(newCount);
}

bool CheckableHeaderView::isSectionCheckable(const int section) const
{
    return m_isCheckable.at(section);
}

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
#include <QtWidgets>

#include "enums.h"

CheckableHeaderView::CheckableHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)

{
    setSectionsClickable(true);
    connect(this,SIGNAL(sectionCountChanged(int,int)),SLOT(updateSectionCount(int,int)));
    if (auto parentView=qobject_cast<QAbstractItemView*>(parent)) {
        parentModel = parentView->model();
        connect(parentModel, &QAbstractItemModel::dataChanged, this, &CheckableHeaderView::modelDataChanged);

    }

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
        //first draw checkbox on a pixmap with pixel ratio set to the actual one
        //to respect high dpi screens
        QCheckBox cb;

        QPixmap img(rect.size() * this->devicePixelRatioF());
        img.setDevicePixelRatio(this->devicePixelRatioF());
        img.fill(Qt::transparent);

        QStylePainter p(&img, &cb);

        QStyleOptionButton option;
        option.initFrom(&cb);
        if (isEnabled())
            option.state |= QStyle::State_Enabled;
        option.rect = rect;
        option.rect.translate(3,0);
        if (m_isChecked.at(logicalIndex)==Qt::Checked)
            option.state |= QStyle::State_On;
        else if (m_isChecked.at(logicalIndex)==Qt::Unchecked)
            option.state |= QStyle::State_Off;
        else
            option.state |= QStyle::State_NoChange;

        p.drawControl(QStyle::CE_CheckBox, option);
        //next draw this pixmap on the screen
        style()->drawItemPixmap(painter, rect, Qt::AlignLeft | Qt::AlignTop, img);
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

//        if (parentModel) {
//            parentModel->blockSignals(true);
//            for (int i=0; i<parentModel->rowCount()-1; ++i) {
//                parentModel->setData(parentModel->index(i, logicalIndex), m_isChecked[logicalIndex], Qt::CheckStateRole);
//            }
//            parentModel->blockSignals(false);
//            parentModel->setData(parentModel->index(parentModel->rowCount()-1, logicalIndex),
//                                 m_isChecked[logicalIndex],
//                                 Qt::CheckStateRole);
//        }
    }
    else QHeaderView::mousePressEvent(event);
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

void CheckableHeaderView::modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (!parentModel) return;

    if (roles.contains(Qt::CheckStateRole) || roles.isEmpty()) {
        for (int column = topLeft.column(); column <= bottomRight.column(); ++column) {
            if (!isSectionCheckable(column)) continue;
            int checkedCount=0;
            for (int i=0; i<parentModel->rowCount(topLeft.parent()); ++i) {
                if (parentModel->data(parentModel->index(i,column,topLeft.parent()), Qt::CheckStateRole).toInt()==Qt::Checked)
                    checkedCount++;
            }
            if (checkedCount==0)
                setCheckState(column, Qt::Unchecked);
            else if (checkedCount == parentModel->rowCount(topLeft.parent()))
                setCheckState(column, Qt::Checked);
            else
                setCheckState(column, Qt::PartiallyChecked);
        }
    }
}

bool CheckableHeaderView::isSectionCheckable(const int section) const
{
    return m_isCheckable.at(section);
}

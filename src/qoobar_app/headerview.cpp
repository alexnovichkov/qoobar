/**
 * \file headerview.cpp
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

#include "headerview.h"
#include <QtWidgets>
#include "checkabletablemodel.h"
#include "enums.h"

HeaderView::HeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)

{
    setSectionsClickable(true);
    if (auto parentView=qobject_cast<QAbstractItemView*>(parent)) {
        if (auto m = qobject_cast<CheckableTableModel*>(parentView->model()))
            parentModel = m;
    }
}

void HeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

//    if (m_isCheckable.contains(logicalIndex)) {
    if (parentModel && parentModel->checkable(logicalIndex)) {
//        QStyleOptionButton option;
//        if (isEnabled())
//            option.state |= QStyle::State_Enabled;
//        option.rect = checkBoxRect(rect);

//        Qt::CheckState state = Qt::CheckState(model()->headerData(logicalIndex,Qt::Horizontal,Qt::CheckStateRole).toInt());
//        if (state==Qt::Checked)
//            option.state |= QStyle::State_On;
//        else if (state==Qt::Unchecked)
//            option.state |= QStyle::State_Off;
//        else
//            option.state |= QStyle::State_NoChange;
//        style()->drawControl(QStyle::CE_CheckBox, &option, painter);

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
        const auto state = parentModel->headerCheckState(logicalIndex);
        if (state==Qt::Checked)
            option.state |= QStyle::State_On;
        else if (state==Qt::Unchecked)
            option.state |= QStyle::State_Off;
        else
            option.state |= QStyle::State_NoChange;

        p.drawControl(QStyle::CE_CheckBox, option);
        //next draw this pixmap on the screen
        style()->drawItemPixmap(painter, rect, Qt::AlignLeft | Qt::AlignTop, img);
    }
}

void HeaderView::mousePressEvent(QMouseEvent *event)
{
    int logicalIndex = logicalIndexAt(event->pos());
    QRect r(0,0,sectionSize(logicalIndex), this->height());
    QRect rr = checkBoxRect(r);

    if (isEnabled()
        && parentModel && parentModel->checkable(logicalIndex)
        && rr.contains(event->pos())) {

        Qt::CheckState state = model()->headerData(logicalIndex,Qt::Horizontal,
                                                                  Qt::CheckStateRole).value<Qt::CheckState>();
        if (state==Qt::Checked)
            state = Qt::Unchecked;
        else
            state = Qt::Checked;
        model()->setHeaderData(logicalIndex, Qt::Horizontal, state, Qt::CheckStateRole);
    }
    else QHeaderView::mousePressEvent(event);
}

QRect HeaderView::checkBoxRect(const QRect &sourceRect) const
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

//void HeaderView::setCheckable(int section, bool checkable)
//{
//    if (checkable) m_isCheckable.insert(section);
//    else m_isCheckable.remove(section);
//}

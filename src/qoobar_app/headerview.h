/**
 * \file headerview.h
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

#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>

class CheckableTableModel;

class HeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit HeaderView(Qt::Orientation orientation, QWidget *parent = 0);

//    void setCheckable(int section, bool checkable);
//Q_SIGNALS:
//    void toggled(int section, Qt::CheckState checkState);
protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    void mousePressEvent(QMouseEvent *event);
private:
    QRect checkBoxRect(const QRect &sourceRect) const;

//    QSet<int> m_isCheckable;
    QRect _rect;
    CheckableTableModel *parentModel = nullptr;
};

#endif // HEADERVIEW_H

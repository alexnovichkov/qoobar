/**
 * \file tablewidget.h
 * QTableWidget with additional events.
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
#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QTableWidget>
class QMouseEvent;

class TableWidget : public QTableWidget
{
Q_OBJECT
public:
    TableWidget(QWidget* parent=0);
    inline void setCount(int i) {count=i;}
    QVector<int> selectedRows();
    void addRow(const QString &header);
    void addRow(int row);
    QString tag(int) const;
    void retranslateUi();
    void updateRow(int row, const QString &text);
Q_SIGNALS:
    void cellClicked();
    void tagsSent(int, const QStringList &);
public Q_SLOTS:
    void clearTable();
private Q_SLOTS:
    void updateTableHeaderGeometry();
    void adjustDisplayedTags();
    void toggleTableRow(int,int);
    void handleSentTag(int tagID, bool fromTable);
    void addTagToScheme();
protected:
    void mouseDoubleClickEvent(QMouseEvent *);
private:
    int count;
};

#endif


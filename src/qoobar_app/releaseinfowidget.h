/**
 * \file releaseinfowidget.h
 * Widget that shows release info.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 17 Jan 2012
 *
 * Copyright (C) 2012  Alex Novichkov
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

#ifndef RELEASEINFOWIDGET_H
#define RELEASEINFOWIDGET_H

#include <QWidget>
#include "searchresults.h"
class QCheckBox;
class QLineEdit;
class QTreeWidget;
class QLabel;
class CheckableHeaderView;
class QTreeWidgetItem;

class ReleaseInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReleaseInfoWidget(QWidget *parent = 0);
    void setSearchResult(SearchResult &r, int cdNum=-1);
    bool use(const int &);
    bool use(const QString &key);
    bool useTrack(const int track);
    int cd() {return _cd;}
private Q_SLOTS:
    void headerToggled(int,Qt::CheckState);
    void checkBoxToggled(QTreeWidgetItem*, int);
private:
    QTreeWidget *albumTable;

    QCheckBox *tracksCheckBox;
    QTreeWidget *tracksTable;
    CheckableHeaderView *header;

    QCheckBox *imageCheckBox;
    QLabel *imageLabel;
    QImage image;

    int _cd;
};

#endif // RELEASEINFOWIDGET_H

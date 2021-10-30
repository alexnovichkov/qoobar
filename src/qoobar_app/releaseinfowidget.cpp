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

#include "releaseinfowidget.h"
#include <QtWidgets>

#include "qoobarglobals.h"
#include "enums.h"
#include "application.h"
#include "checkableheaderview.h"

QPixmap previewIcon(const QPixmap &pixmap, double pixelRatio)
{DD;

    int max_dimension=100;
    int w=pixmap.width();
    int h = pixmap.height();
    QPixmap p = w>h?pixmap.scaledToWidth(max_dimension) : pixmap.scaledToHeight(max_dimension);
    //TODO: devicePixelRatio()
    p.setDevicePixelRatio(pixelRatio);
    return p;
}

ReleaseInfoWidget::ReleaseInfoWidget(QWidget *parent) : QWidget(parent)
{DD;
    tracksCheckBox = new QCheckBox(tr("Tracks"),this);
    imageCheckBox = new QCheckBox(tr("Cover art"),this);
    tracksCheckBox->setChecked(true);
    imageCheckBox->setChecked(true);

    albumTable = new QTreeWidget(this);
    albumTable->setAllColumnsShowFocus(true);
    albumTable->setAlternatingRowColors(true);
    albumTable->setColumnCount(2);
    albumTable->setHeaderLabels(QStringList()<<tr("Tag")<<tr("Value"));
    albumTable->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    albumTable->header()->setStretchLastSection(false);
    albumTable->setRootIsDecorated(false);

    tracksTable = new QTreeWidget(this);
    tracksTable->setAllColumnsShowFocus(true);
    tracksTable->setAlternatingRowColors(true);
    tracksTable->setColumnWidth(0,QFontMetrics(qApp->font()).HORIZONTAL_ADVANCE(QSL("999")));
    tracksTable->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tracksTable->setColumnWidth(4,QFontMetrics(qApp->font()).HORIZONTAL_ADVANCE(QSL("999:99")));
    tracksTable->header()->setStretchLastSection(false);
    tracksTable->setHeaderLabels(QStringList()<<tr("No.")<<tr("Title")<<tr("Artists")
                                 <<tr("Comment")<<tr("Length"));
    tracksTable->setRootIsDecorated(false);

    header = new CheckableHeaderView(Qt::Horizontal,tracksTable);
    tracksTable->setHeader(header);
    header->setCheckable(0, true);
    header->setCheckState(0,Qt::Checked);
    connect(header,SIGNAL(toggled(int,Qt::CheckState)),this,SLOT(headerToggled(int,Qt::CheckState)));


    imageLabel = new QLabel(this);

    const QSize size = {100,100};
    imageLabel->setMaximumSize(size);
    imageLabel->setMinimumSize(size);
    imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QGridLayout *l = new QGridLayout;
    l->addWidget(albumTable,0,0,2,1);
    l->addWidget(imageCheckBox,0,1);
    l->addWidget(imageLabel,1,1,1,1,Qt::AlignHCenter | Qt::AlignVCenter);
    l->addWidget(tracksCheckBox,2,0,1,1,Qt::AlignLeft | Qt::AlignTop);
    l->addWidget(tracksTable,3,0,1,2);

    l->setContentsMargins(10,0,0,0);

    setLayout(l);

    _cd=1;

    setTabOrder(albumTable,imageCheckBox);
    setTabOrder(imageCheckBox,tracksCheckBox);
    setTabOrder(tracksCheckBox,tracksTable);
}

void ReleaseInfoWidget::setSearchResult(SearchResult &r,int cdNum)
{DD;
    QString artists = artistsText(r.artists);
    if (!artists.isEmpty()) r.fields.insert("artist", artists);

    albumTable->clear();
    QMapIterator<QString,QString> it(r.fields);
    while (it.hasNext()) {
        it.next();
        QTreeWidgetItem *item = new QTreeWidgetItem(albumTable, QStringList()<<it.key()<<it.value());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable
                       | Qt::ItemIsEnabled);
        item->setCheckState(0,Qt::Checked);
    }

    tracksTable->clear();
    _cd=cdNum;
    for (int i=0; i<r.tracks.size(); ++i) {
        if (r.cdCount==1 || cdNum == r.tracks.at(i).cd) {
            QTreeWidgetItem *item = new QTreeWidgetItem(tracksTable, r.tracks.at(i).toStringList());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable
                           | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            item->setCheckState(0,Qt::Checked);
        }
    }

    imageLabel->clear();
    QPixmap p;
    if (p.loadFromData(r.image.pixmap())) imageLabel->setPixmap(previewIcon(p, devicePixelRatioF()));
}

bool ReleaseInfoWidget::use(const int &tagID)
{DD;
    Q_UNUSED(tagID)
//    switch (tagID) {
//    case ALBUM: return albumCheckBox->isChecked();
//    case ARTIST: return artistCheckBox->isChecked();
//    case YEAR: return yearCheckBox->isChecked();
//    case GENRE: return genreCheckBox->isChecked();
//    case COMMENT: return commentCheckBox->isChecked();
//    case PUBLISHER: return labelCheckBox->isChecked();
//    case TITLE: return tracksCheckBox->isChecked();
//    case -2: return imageCheckBox->isChecked();
//    default: return false;
//    }
    return true;
}

bool ReleaseInfoWidget::use(const QString &key)
{DD;
    for (int i=0; i<albumTable->topLevelItemCount(); ++i) {
        if (albumTable->topLevelItem(i)->text(0)==key
                && albumTable->topLevelItem(i)->checkState(0)==Qt::Checked)
            return true;
    }
    if (key=="image") return imageCheckBox->isChecked();
    if (key=="titles") return tracksCheckBox->isChecked();

    return false;
}

bool ReleaseInfoWidget::useTrack(const int track)
{DD;
    if (track <0 || track>=tracksTable->topLevelItemCount()) return false;
    return tracksTable->topLevelItem(track)->checkState(0)==Qt::Checked;
}

void ReleaseInfoWidget::headerToggled(int column, Qt::CheckState checked)
{DD;
    if (column<0 || column >= tracksTable->columnCount()) return;

    if (checked==Qt::PartiallyChecked) return;
    tracksTable->blockSignals(true);
    for (int i=0; i<tracksTable->topLevelItemCount(); ++i)
        tracksTable->topLevelItem(i)->setCheckState(column, checked);
    tracksTable->blockSignals(false);
}


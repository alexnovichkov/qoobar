/**
 * \file tagsfiller.h
 * Dialog for filling tags from files names.
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

#ifndef TAGSFILLER_H
#define TAGSFILLER_H

#include <QDialog>
#include <QItemDelegate>

class QTableWidget;
class QComboBox;
class QTreeWidget;
class QLabel;
class QRadioButton;
class LegendButton;
class CoreNetworkSearch;
class QListWidget;
class ReleaseInfoWidget;
class QProgressBar;
class CheckableHeaderView;

class DownloadHelper;
class IDownloadPlugin;
class ClearLineEdit;

#include "tagger.h"
#include "searchresults.h"

class Tab;
class QTreeWidgetItem;
class QProgressIndicatorSpinning;

class TagsFillDialog : public QDialog
{
Q_OBJECT

public:
    TagsFillDialog(const QList<Tag> &,QWidget *parent = 0);
    ~TagsFillDialog();
    QList<Tag> getNewTags() {return newTags;}
public Q_SLOTS:
    void accept();
private Q_SLOTS:
    void updateTags(bool alsoUpdateTable=true);
    void setSource(int n);
    void insertLegend(const QString &);
    void cellChanged(int row, int col);

    void handleSourceComboBox(int);
    void handleManualSearchRadioButton();
    void swapArtistAndAlbum();
    void startSearch();
    void handleAlbumSelection(QTreeWidgetItem*);
    void found(const QList<SearchResult> &);
    void resultFinished(const SearchResult &,int);
    //void downloadingImageFinished(const QByteArray &,int);
    void downloadRelease(QTreeWidgetItem*);
    void headerToggled(int,Qt::CheckState);

    void showHelp();
private:
    void updateTagsFromNetwork();
    void downloadRelease(const QString &url, const int releaseIndex);
    void setNewTag(const QString &tagValue, Tag &tag, const QString &field, int fieldID);
    IDownloadPlugin *maybeLoadPlugin(const QString &path);

    QList<Tag> newTags;
    QList<Tag> oldTags;
    QTabWidget *tab;
    int count;

    //from tags
    QComboBox *tagsSourceComboBox;
    QComboBox *patternEdit;
    QTableWidget *table;
    LegendButton *legendButton;
    QStringList tagsSource;
    CheckableHeaderView *header;

    //from Network
    QComboBox *sourceComboBox;
    QRadioButton *manualSearchRadioButton;
    QRadioButton *cdSearchRadioButton;
    QRadioButton *filesSearchRadioButton;
    ClearLineEdit *artistEdit;
    ClearLineEdit *albumEdit;
    QPushButton *startSearchButton;
    QTreeWidget *searchResultsList;
    QLabel *networkStatusInfo;
    QLabel *networkErrorInfo;
    CoreNetworkSearch *search;
    QProgressIndicatorSpinning *progress;
    //QProgressBar *progress;

    int currentAlbum;
    QList<SearchResult> searchResults;
    ReleaseInfoWidget *releaseInfoWidget;
#ifdef HAVE_QT5
    QHash<QString, IDownloadPlugin *> loadedPlugins;
#endif
};

#endif


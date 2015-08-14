/**
 * \file replaygaindialog.h
 * A dialog to use with ReplayGainer
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date Nov 27 2012
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

#ifndef REPLAYGAINDIALOG_H
#define REPLAYGAINDIALOG_H

#include <QDialog>

class QPushButton;
class QPlainTextEdit;
class QTreeWidget;
class QCheckBox;
class Model;
class ReplayGainer;

#include "tagger.h"

class ReplayGainDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplayGainDialog(Model *model, QWidget *parent = 0);
Q_SIGNALS:

public Q_SLOTS:
    void accept();
private Q_SLOTS:
    void operate(int);
    void appendText(const QString &text);
    void appendText(int type, const QString &text);
    void tick();
    void setSkip();
    void showHelp();
//    void update(const QVector<int> &indexes, const QList<ReplayGainInfo> &newRG);
private:
    void initTable();

    Model *m;

    QPushButton *scanFileButton;
    QPushButton *scanAlbumButton;
    QPushButton *scanAlbumsByTagsButton;
    QPushButton *scanAlbumsByFolderButton;
    QPushButton *removeButton;
    QPlainTextEdit *edit;
    QTreeWidget *tree;
    QCheckBox *skipCheckBox;

    ReplayGainer *replayGainer;
    QPushButton *okButton;

    QList<ReplayGainInfo> rgList;
    bool scanned;
};

#endif // REPLAYGAINDIALOG_H

/**
 * \file settingsdialog.h
 * Settings dialog.
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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QStackedWidget;
class CategoryListView;
class CategoryModel;
class QModelIndex;
class ConfigPage;
class QPushButton;

class SettingsDialog : public QDialog
{
Q_OBJECT

public:
    SettingsDialog(QWidget *parent = 0);
public Q_SLOTS:
    void accept();
Q_SIGNALS:
    void retranslate();
//    void tableUpdateModeChanged(bool);
private Q_SLOTS:
    void changePage(const QModelIndex &current);
    void retranslateUI();
    void resetSettings();
private:
    QStackedWidget *pagesWidget;
    CategoryModel *pagesModel;
    CategoryListView *contentsWidget;
    QList<ConfigPage *> configPages;
    QPushButton *resetSettingsButton;
};

#endif


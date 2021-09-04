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

#include <QMainWindow>
#include <QDialog>

class QStackedWidget;
class ConfigPage;
class QPushButton;
class QToolBar;


class SettingsDialog : public
        #ifdef OSX_SUPPORT_ENABLED
        QMainWindow
        #else
        QDialog
        #endif
{
Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
public Q_SLOTS:
    void accept();
Q_SIGNALS:
    void retranslate();
    void closed();
//    void tableUpdateModeChanged(bool);
private Q_SLOTS:
    void retranslateUI();
    void resetSettings();
    void switchPage(int page);
    void showHelp();
private:
    QStackedWidget *pagesWidget;
    QList<ConfigPage *> configPages;
    QPushButton *resetSettingsButton;

    QToolBar * toolBar;
    int currentPage;
};

#endif


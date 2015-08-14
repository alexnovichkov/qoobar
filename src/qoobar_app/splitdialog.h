/**
 * \file splitdialog.h
 * Dialog where you can split an image by cue file.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 13 Sep 2010
 *
 * Copyright (C) 2009, 2010  Alex Novichkov
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

#ifndef SPLITDIALOG_H
#define SPLITDIALOG_H

#include <QDialog>

class QPushButton;
class QCheckBox;
class QRadioButton;
class QLineEdit;
class QLabel;
class QPlainTextEdit;
class QProgressBar;
class CueSplitter;
class QComboBox;

class SplitDialog : public QDialog
{
Q_OBJECT
public:
    explicit SplitDialog(QWidget *parent = 0);
    ~SplitDialog();
    QStringList getNewFiles() {return splittedFiles;}
    void setCueFile(const QString &file);
public Q_SLOTS:
    void run();
    void cancel();
private Q_SLOTS:
    void openCue();
    void changeDir();
    void onNextFile();
    void onMessage(int type, const QString &text);
    void updateEdit(const QString &text);
    void changeCueEncoding(const QString &encoding);
    void showHelp();
private:
    QPushButton *openCueButton;
    QLineEdit *cueFileEdit;
    QLineEdit *folderEdit;
    QPushButton *folderButton;
    QPushButton *startButton;
    QLabel *warningLabel;
    QPlainTextEdit *edit;
    QProgressBar *progress;
    CueSplitter *splitter;
    QComboBox *formatBox;

    QWidget *cueEncodingWidget;

    QStringList splittedFiles;
};

#endif // SPLITDIALOG_H

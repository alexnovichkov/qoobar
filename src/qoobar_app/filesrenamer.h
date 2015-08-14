/**
 * \file filesrenamer.h
 * Rename files dialog.
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

#ifndef FILESRENAMER_H
#define FILESRENAMER_H

#include <QDialog>
class QTableWidget;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QRadioButton;
class Tag;
class FileNameRenderer;
class LegendButton;
class QSpinBox;
class QGroupBox;
class FileDelegateHighlighter;
class FancyLineEdit;
class Model;

class FileRenameDialog : public QDialog
{
Q_OBJECT

public:
    FileRenameDialog(Model *model, QWidget *parent);
    ~FileRenameDialog();
    inline QStringList getNewFileNames() {return newFileNames;}
private Q_SLOTS:
    void setExistingDirectory();
    void run();
    void insertLegend(const QString &);
    void handleRemoveFolders();
    void handleOperation(int);
    void updateCase(bool);

    void patternChanged(const QString &);
    void directoryChanged();
    void replaceWinCharsChanged();
    void replaceSpacesChanged();
    void diacriticsChanged();
    void caseChanged(const int value);

    void fileLengthChanged(int value);
    void updateFileLength(bool value);
    void applyToFoldersChanged();
    void removePattern();

    void showHelp();
private:
    void updateTable();
    Model *m;
    QStringList oldFileNames;
    QStringList newFileNames;
    QTableWidget *table;
    QComboBox *patternEdit;
    FancyLineEdit *directoryEdit;

    QCheckBox *directoryGroup;
    QCheckBox *charactersCheckBox;
    QLineEdit *replaceCharacterEdit;
    QComboBox *operationComboBox;
    LegendButton *legendButton;
    QCheckBox *removeFolderCheckBox;

    QCheckBox *spacesCheckBox;
    QLineEdit *spacesEdit;
    QCheckBox *diacriticsCheckBox;

    QComboBox *caseComboBox;
    QCheckBox *caseCheckBox;
    QSpinBox *fileLengthSpinBox;
    QCheckBox *fileLengthCheckBox;
    QCheckBox *applyToFoldersCheckBox;

    QPushButton *runButton;

    FileNameRenderer *filenameRenderer;
    FileDelegateHighlighter *highlighter;
};

#endif


/**
 * \file configpages.cpp
 * Class that contains pages for settings dialog.
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

#ifndef CONFIGPAGES_H
#define CONFIGPAGES_H

#include <QWidget>
class QGroupBox;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QListWidget;
class LegendButton;
class QMenu;
class QRadioButton;
class ClearLineEdit;
class FancyLineEdit;
class QFrame;

//#define USE_DELAYED_UPDATE

class ConfigPage : public QWidget
{
    Q_OBJECT
public:
    ConfigPage(QWidget *parent = 0);
    virtual QString description() {return "";}
    virtual QString iconFilename() {return "";}
    virtual void retranslateUI();
    virtual void saveSettings() {}
    virtual void setSettings() {}
    virtual void finalize(QLayout *layout);
protected:
    QFrame *contentsWidget;
};

class InterfacePage : public ConfigPage
{
    Q_OBJECT
public:
    InterfacePage(QWidget *parent = 0);
    QString description();
    QString iconFilename();
    void retranslateUI();
    void saveSettings();
    void setSettings();
Q_SIGNALS:
    void retranslate();
private Q_SLOTS:
    void changeCharsFont();
    void updateLanguage(const int);
    void chooseDirRoot();
private:
    QLabel *charsBox;
    FancyLineEdit *chars;
    QComboBox *lang;
    QCheckBox *autoexpand;
    QLabel *langLabel;
    QCheckBox *useUndo;
    QCheckBox *dirBox;
    QCheckBox *hideTabBar;
    FancyLineEdit *dirRoot;
    QLabel *dirRootLabel;
    QLabel *iconThemeLabel;
    QComboBox *iconTheme;
    QLabel *statusBarTrackLabel;
    QComboBox *statusBarTrack;
};

class QFormLayout;
class QTreeWidget;
class CompletionPage : public ConfigPage
{
    Q_OBJECT
public:
    CompletionPage(QWidget *parent = 0);
    QString description();
    QString iconFilename();
    void retranslateUI();
    void saveSettings();
    void setSettings();
private Q_SLOTS:
    void editList(const int);
private:
    QComboBox *completionStyle;
    QLabel *completionStyleLabel;

    QTreeWidget *completionTree;

    QLabel *collectTextLabel;
    QComboBox *collectText;
};

class QToolButton;
class WritingPage : public ConfigPage
{
    Q_OBJECT
public:
    WritingPage(QWidget *parent = 0);
    QString description();
    QString iconFilename();
    void retranslateUI();
    void saveSettings();
    void setSettings();
private:
    QCheckBox *saveChanges;
    QCheckBox *trim;
//    QCheckBox *readID3;
//    QCheckBox *writeID3;
//    QCheckBox *readAPE;
//    QCheckBox *writeAPE;
    QLabel *readMp3;
    QLabel *writeMp3;
    QToolButton *readID3;
    QToolButton *writeID3;
    QToolButton *readAPE;
    QToolButton *writeAPE;

    QGroupBox *box;
    QGroupBox *oggbox;
    QLabel *id3v1writeLabel;
    QComboBox *id3v1write;
    QCheckBox *id3v1transliterate;
    QComboBox *id3v1encoding;
    QComboBox *oggPicture;
    QComboBox *id3v2version;
    QLabel *id3v2versionLabel;

    QGroupBox *id3v1box;
    QLabel *id3Label;
    QLabel *oggLabel;

    QCheckBox *writeFieldsSeparately;
#ifdef Q_OS_LINUX
    QComboBox *mpcReplayGain;
    QGroupBox *mpcbox;
    QLabel *mpcLabel;
#endif
};

class PatternsPage : public ConfigPage
{
    Q_OBJECT
public:
    PatternsPage(QWidget *parent = 0);
    QString description();
    QString iconFilename();
    void retranslateUI();
    void saveSettings();
    void setSettings();
private Q_SLOTS:
    void addPattern();
    void removePattern();
    void updatePatterns();
    void insertLegend(const QString &);
    void removeScheme();
    void editScheme();
    void addScheme();
    void schemesComboBoxIndexChanged(const int);
private:
    QListWidget *patterns;
    QGroupBox *renamingPatternsBox;
    QGroupBox *schemesBox;
    QPushButton *addPatternButton;
    QPushButton *removePatternButton;
    LegendButton *legendButton;

    QLabel *schemesLabel;
    QComboBox *schemesComboBox;
    QPushButton *addSchemeButton;
    QPushButton *removeSchemeButton;
    QPushButton *editSchemeButton;
    QMenu *schemesMenu;
};


class UtilitiesPage : public ConfigPage
{
    Q_OBJECT
public:
    UtilitiesPage(QWidget *parent = 0);
    QString description();
    QString iconFilename();
    void retranslateUI();
    void saveSettings();
    void setSettings();
private Q_SLOTS:
    void choosePlayer();
    //void openExternalProgram(int i);
private:
    FancyLineEdit *player;
    ClearLineEdit *cdromDevice;
    QLabel *playerLabel;
    QLabel *cdromLabel;
    QLabel *encaLanguageLabel;
    QLabel *programsLabel;
    QComboBox *encaGuessLanguage;
    QCheckBox *copyFiles;

    QLabel *cueEncodingLabel;
    QComboBox *cueEncoding;
    QTreeWidget *tree;
};

class NetworkPage : public ConfigPage
{
    Q_OBJECT
public:
    NetworkPage(QWidget *parent = 0);
    QString description();
    QString iconFilename();
    void retranslateUI();
    void saveSettings();
    void setSettings();
private:
    QGroupBox *useProxy;
    ClearLineEdit *proxyServer;
    ClearLineEdit *proxyPort;
    ClearLineEdit *proxyLogin;
    ClearLineEdit *proxyPassword;
    QLabel *serverLabel;
    QLabel *portLabel;
    QLabel *loginLabel;
    QLabel *passwordLabel;
};

class PluginsPage : public ConfigPage
{
    Q_OBJECT
public:
    PluginsPage(QWidget *parent = 0);
    QString description();
    QString iconFilename();
    void retranslateUI();
    //void saveSettings();
    void setSettings();
private:
    QLabel *downloadLabel;
    QLabel *editingLabel;

    QTreeWidget *downloadTree;
    QTreeWidget *editingTree;
};

#endif // CONFIGPAGES_H

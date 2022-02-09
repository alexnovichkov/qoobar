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
    void updateLanguage(const int);
    void chooseDirRoot();
private:
    QCheckBox *useUndo;
    QCheckBox *hideTabBar;
    QCheckBox *dirBox;
    FancyLineEdit *dirRoot;
    QComboBox *sortOptions;
    QLabel *sortOptionsLabel;
    QLabel *statusBarTrackLabel;
    QComboBox *statusBarTrack;
    QComboBox *lang;
    QLabel *langLabel;
    QLabel *iconThemeLabel;
    QComboBox *iconTheme;
    QLabel *styleLabel;
    QComboBox *style;
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
private Q_SLOTS:
    void changeCharsFont();
private:
    QLabel *charsBox;
    FancyLineEdit *chars;
    QCheckBox *showChars;

    QCheckBox *autoexpand;

    QCheckBox *saveChanges;
    QCheckBox *trim;

    QCheckBox *writeFieldsSeparately;
};

class TagsPage : public ConfigPage
{
    Q_OBJECT
public:
    TagsPage(QWidget *parent = 0);

    // ConfigPage interface
public:
    virtual QString description() override;
    virtual QString iconFilename() override;
    virtual void retranslateUI() override;
    virtual void saveSettings() override;
    virtual void setSettings() override;
private Q_SLOTS:
    void removeScheme();
    void editScheme();
    void addScheme();
    void schemesComboBoxIndexChanged(const int);
private:
    QLabel *readMp3Label;
    QLabel *writeMp3Label;
    QComboBox *readMp3;
    QComboBox *writeMp3;

    QLabel *id3v1writeLabel;
    QComboBox *id3v1write;
    QCheckBox *id3v1transliterate;
    QComboBox *id3v1encoding;
    QComboBox *oggPicture;
    QComboBox *id3v2version;
    QLabel *id3v2versionLabel;

    QLabel *id3v2lineEndingLabel;
    QComboBox *id3v2LineEnding;

    QLabel *id3Label;
    QLabel *oggLabel;

    QComboBox *mpcReplayGain;
    QLabel *mpcLabel;

    QLabel *schemesLabel;
    QComboBox *schemesComboBox;
    QPushButton *addSchemeButton;
    QPushButton *removeSchemeButton;
    QPushButton *editSchemeButton;
    QMenu *schemesMenu;
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
private:
    QListWidget *patterns;
    QPushButton *addPatternButton;
    QPushButton *removePatternButton;
    LegendButton *legendButton;
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
    QLabel *cueEncodingLabel;
    QComboBox *cueEncoding;
    QTreeWidget *tree;
};

class ReplaygainPage : public ConfigPage
{
    Q_OBJECT
public:
    ReplaygainPage(QWidget *parent = 0);
    QString description();
    QString iconFilename();
    void retranslateUI();
    void saveSettings();
    void setSettings();
private Q_SLOTS:
    void updateModeInfo(int);
private:
    QLabel *tagsLabel;
    QComboBox *tagsCombo;
    QLabel *modeLabel;
    QComboBox *modeCombo;
    QLabel *modeInfoLabel;
    QLabel *loudnessLabel;
    QComboBox *loudnessCombo;
    QLabel *unitsLabel;
    QComboBox *unitsCombo;
    QCheckBox *clipping;
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

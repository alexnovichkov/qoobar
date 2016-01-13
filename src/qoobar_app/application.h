/**
 * \file application.h
 * QApplication with settings.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 17 May 2010
 *
 * Copyright (C) 2010  Alex Novichkov
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QStringList>
#include <QVector>
#include <QFont>
#include <QMap>
#include <QSettings>
#include "taggingscheme.h"
#include "applicationpaths.h"
#include "coverimage.h"

#define App (static_cast<Application *>(QCoreApplication::instance()))

class IQoobarPlugin;
class IDownloadPlugin;

struct RenameOptions
{
    /*!
     * \brief renamingOperation
     *  0 = "rename"
     *  1 = "copy"
     *  2 = "rename-folder"
     *  default value = 0
     */

    int renamingOperation;

    QString destinationFolder;
    bool replaceWinChars;
    QString winCharsReplacer;
    bool replaceSpaces;
    QString spacesReplacer;
    bool removeDiacritics;
    int ccase;
    bool changeCase;
    int maximumLength;
    bool trimFileLength;

    bool removeFolder;
    bool applyToFolders;
};

class Autocompletions : public QObject
{
    Q_OBJECT
public:
    class Completion
    {
    public:
        Completion() : use(false), wasChanged(false)
        {}
        QStringList variants;
        bool use;
        bool wasChanged;
        QString name;
    };

    explicit Autocompletions(QObject *parent = 0);

    void read(QSettings *se);
    bool write(QSettings *se);

    bool use(int tagID) const {
        return completions.value(tagID).use;
    }
    void setUse(int tagID, bool use) {
        if (tagID >= completions.size())
            completions.resize(tagID+1);
        completions[tagID].use = use;
    }
    QStringList variants(int tagID) const {
        return completions.value(tagID).variants;
    }
    int variantsCount(int tagID) const {
        return completions.value(tagID).variants.size();
    }

    void update(int tagID, const QStringList &newVariants);
    void set(int tagID, const QStringList &newVariants);

    // true:  Completer matches the previous word
    // false: Completer matches the whole string
    bool completionStyle;

    bool collectSilently;

    bool wasChanged;
private:
    QVector<Completion> completions;
};

void criticalMessage(QWidget *parent, const QString &caption, const QString &text);
void warningMessage(QWidget *parent, const QString &caption, const QString &text);

class Application : public QApplication
{
    Q_OBJECT
public:
    explicit Application(int & argc, char ** argv, bool useGui=true);
    ~Application();
    QSettings *globalSettings();
    QSettings *guiSettings();
    void loadTranslations();
    void setId3v1Encoding(const QString &s);
    void addPattern(const QString &s, QStringList &where);
    void removePattern(const QString &s, QStringList &where);
    void setFilesNames(const QStringList &names) {filesNames = names;}

    void loadPlugins();
#ifdef HAVE_QT5
    QList<QJsonObject> downloadPlugins;
    QList<QJsonObject> plugins;
#else
    QMap<QString, IDownloadPlugin *> downloadPlugins;
    QMap<QString, IQoobarPlugin *> plugins;
#endif
    void readGuiSettings();
    void readGlobalSettings();
    void clearSettings();
    void resetSettings();
    void writeGuiSettings();
    void writeGlobalSettings();
    QString themeIcon(const QString &icon);
    QIcon iconThemeIcon(const QString &icon);

    void onDockIconClick();
    bool consoleMode;
    QString langID; //language id of the app language
    QString chars; //characters with diacritics
    QString lastDirectory;  //last added directory
    QString lastTreeDirectory; //last directory added via dirs tree
    QString lastCueFile;
    bool mp3readape;
    bool mp3writeape;
    bool mp3readid3;
    bool mp3writeid3;

    bool flacreadogg;
    bool flacwriteogg;
    bool flacreadid3;
    bool flacwriteid3;

    QString player;
    QByteArray columns160; //horizontal header state
    QByteArray columns167; //horizontal header state since version 1.6.7

    QByteArray rows; //vertical header state
    bool saveChanges; //whether or not save all changes when closing app
    bool trim; //whether or not trim tags when saving files
    QStringList patterns; //file rename patterns
    bool autoexpand;

    bool oggPictureNew;

    QVector<int> tagStatus;

    bool useProxy;
    QString proxyServer;
    QString proxyPort;
    QString proxyLogin;
    QString proxyPassword;

    //rename options
    RenameOptions renameOptions;

    //autocompletion options
    Autocompletions *autocompletions;


    CoverImage imageBuffer;
    QString iconTheme;

    bool charsShown;
    QFont charsFont;

    QString cdromDevice;

    int id3v1Synchro;
    bool id3v1Transliterate;
    QString id3v1Encoding;

    QString cueEncoding;

    QByteArray splitterState;
    QByteArray innerSplitterState;
    QByteArray dirSplitterState;
    bool showDirView;
    QString dirViewRoot;


    QString currentSchemeName;
    TaggingScheme *currentScheme;

    bool useUndo;
//    bool useDelayedUpdate;
//    bool showFullFilesProperties;

    QString encaGuessLanguage;
    QByteArray geometry;
    bool writeFieldsSeparately;
    bool copyFiles;
    QString discidLibraryPath;

    QStringList fillPatterns;
    int id3v2version;

    QStringList filesNames;

    bool verbose;
    bool recursive;

    bool mpcWriteRg;
    QString delimiters;

    QStringList searchPaths;
    bool hideTabBar;
    QString defaultSplitFormat;
    int statusBarTrack;
Q_SIGNALS:
    void settingsCleared();
    void dockClicked();
private Q_SLOTS:
    void onApplicationStateChanged(Qt::ApplicationState);
private:
    class Private;
    Private *p;
    QTranslator *appTranslator;
    QTranslator *qtTranslator;
};

#endif // APPLICATION_H

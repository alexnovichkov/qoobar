/**
 * \file application.cpp
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

#include "application.h"
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

#include "id3v1stringhandler.h"


#include "applicationpaths.h"
#include "iqoobarplugin.h"
#include "idownloadplugin.h"
#include "enums.h"
#include "qoobarglobals.h"

bool isValidLibrary(const QFileInfo &path)
{DD;
    QLibrary lib(path.canonicalFilePath());

    if (lib.load()) {
        lib.unload();
        return true;
    }
    return false;
}

Application::Application(int &argc, char **argv, bool useGui)
    :  QApplication(argc,argv,useGui)
{DD;
#ifdef Q_OS_MAC
    connect(this,SIGNAL(applicationStateChanged(Qt::ApplicationState)),SLOT(onApplicationStateChanged(Qt::ApplicationState)));
#endif
    appTranslator = new QTranslator(this);
    qtTranslator = new QTranslator(this);
    installTranslator(appTranslator);
    installTranslator(qtTranslator);

    // Setting default values
    langID = QSL("en");

    chars = QString("ßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿāăąćĉċčďđēĕėęěĝğġģĥħĩīĭįıĳĵķĸĺļľŀłńņňŉŋōŏőœŕŗřśŝşšţťŧũūŭůűųŵŷźżž");
    lastDirectory = QDir::homePath();
    mp3readape = false;
    mp3writeape = false;
    mp3readid3 = true;
    mp3writeid3 = true;
    flacreadogg = true;
    flacwriteogg = true;
    flacreadid3 = false;
    flacwriteid3 = false;
    saveChanges = false;
    trim = true;
    autoexpand = true;
    oggPictureNew = true;
    delimiters = QSL("_~`!1234567890@#$%^&*()-_=+[{]};:'\"\\|,<.>/?");

    useProxy = false;
    renameOptions.replaceWinChars = true;
    renameOptions.winCharsReplacer = QSL("_");
    renameOptions.replaceSpaces = false;
    renameOptions.spacesReplacer = QSL("_");
    renameOptions.removeDiacritics = false;
    renameOptions.changeCase = false;
    renameOptions.ccase = Case::DontChange;
    renameOptions.maximumLength = 0;
    renameOptions.trimFileLength = false;
    renameOptions.renamingOperation = 0;
    renameOptions.removeFolder = true;
    renameOptions.applyToFolders = false;

    charsShown = true;
    id3v1Synchro = ID3V1_DELETE;
    id3v1Transliterate = true;
    id3v1Encoding = QSL("ISO-8859-1");
    cueEncoding = QSL("Locale");
    showDirView = true;
    currentSchemeName = QSL("default");
    currentScheme = nullptr;
    useUndo = false;
    encaGuessLanguage = QSL("russian");
    writeFieldsSeparately = false;
    copyFiles = false;
    id3v2version = 4;
    verbose = true;
    recursive = false;
    consoleMode = false;
    mpcWriteRg = true;
    iconTheme = "maia";
    statusBarTrack = 0; // tracks hovered file

    /*Testing for loadable libraries*/
    /*that is discid*/
    QStringList discidName = QStringList(QSL("*discid*"));

    QFileInfoList libFiles = QDir(QSL("/usr/lib")).entryInfoList(discidName, QDir::Files | QDir::NoSymLinks);
    libFiles << QDir(QSL("/usr/lib/i386-linux-gnu")).entryInfoList(discidName, QDir::Files | QDir::NoSymLinks);
    libFiles << QDir(QSL("/usr/lib64")).entryInfoList(discidName, QDir::Files | QDir::NoSymLinks);
    libFiles << QDir(QSL("/usr/lib/x86_64-linux-gnu")).entryInfoList(discidName, QDir::Files | QDir::NoSymLinks);
    libFiles << QDir(App->applicationDirPath()).entryInfoList(discidName, QDir::Files | QDir::NoSymLinks);
    Q_FOREACH(const QFileInfo &fi, libFiles) {
        if (isValidLibrary(fi)) {
            discidLibraryPath = fi.canonicalFilePath();
            break;
        }
    }

    QStringList themePaths = QIcon::themeSearchPaths();
    themePaths.prepend(ApplicationPaths::sharedPath()+"/icons");
    QIcon::setThemeSearchPaths(themePaths);
}

template <class T>
QVariantList makeList(const QVector<T> &list)
{DD;
    QVariantList result;
    Q_FOREACH(const T &t,list) result.append(QVariant(t));
    return result;
}

Application::~Application()
{DD;
    if (!consoleMode) {
        writeGuiSettings();
    }
    writeGlobalSettings();

    delete currentScheme;
    currentScheme=nullptr;
}

void Application::loadTranslations()
{DD;
    if (qtTranslator)
        qtTranslator->load(QSL("qt_") + langID, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    if (appTranslator)
        appTranslator->load(ApplicationPaths::translationsPath()+QSL("/qoobar_") + langID);
    if (currentScheme) currentScheme->retranslateUI();
}

void Application::setId3v1Encoding(const QString &s)
{DD;
    id3v1Encoding=s;
    auto *stringHandler = new ID3v1StringHandler;
    if (id3v1Encoding=="Locale")
        stringHandler->setCodec(QTextCodec::codecForLocale());
    else
        stringHandler->setCodec(QTextCodec::codecForName(id3v1Encoding.toLatin1()));
}

QSettings *Application::guiSettings()
{DD;
#ifdef QOOBAR_PORTABLE
    return new QSettings(ApplicationPaths::sharedPath()+QSL("/qoobar.ini"),QSettings::IniFormat);
#else
    return new QSettings(QSL("qoobar"),QSL("gui"));
#endif
}

void Application::readGuiSettings()
{DD;
    QSettings *se = guiSettings();

    //Interface
    useUndo = se->value(QSL("useUndo"), useUndo).toBool();
    autoexpand=se->value(QSL("autoexpand"), autoexpand).toBool();

    chars=se->value(QSL("chars"), chars).toString();
    QVariant charsFontV=se->value(QSL("charsFont"),font());
    charsFont=charsFontV.value<QFont>();

    langID=se->value(QSL("lang"),QSL("en")).toString();
    loadTranslations();

    player=se->value(QSL("player")).toString();
    saveChanges=se->value(QSL("saveChanges"),saveChanges).toBool();

    patterns = se->value(QSL("renamePatterns"), QStringList()<<"[%N. ]%t"
                        << "%a/[%N. ]%t"
                        << "[%N. ]%A - %t").toStringList();

    // old setting, contains patterns in a string separated by ;
    // use and then remove if present
    if (se->contains(QSL("patterns"))) {
        QStringList l = se->value(QSL("patterns")).toString().split(';');
        Q_FOREACH (const QString &s, l) addPattern(s, patterns);
        se->remove(QSL("patterns"));
    }
    QString pattern = se->value(QSL("pattern")).toString();
    if (!pattern.isEmpty()) {
        addPattern(pattern,patterns);
        se->remove(QSL("pattern"));
    }

    lastDirectory=se->value(QSL("lastDirectory"),lastDirectory).toString();
    lastTreeDirectory=se->value(QSL("lastTreeDirectory"),lastTreeDirectory).toString();
    splitterState = se->value(QSL("splitter")).toByteArray();
    innerSplitterState = se->value(QSL("innerSplitter")).toByteArray();
    dirSplitterState = se->value(QSL("dirSplitter")).toByteArray();
    showDirView = se->value(QSL("showDirView"), showDirView).toBool();
    dirViewRoot = se->value(QSL("dirViewRoot"), dirViewRoot).toString();
    if (dirViewRoot.isEmpty()) {
#ifdef Q_OS_WIN
        dirViewRoot = QLS("");
#else
        dirViewRoot = QLS("/");
#endif
    }

    useProxy = se->value(QSL("useProxy"), useProxy).toBool();
    proxyServer = se->value(QSL("proxyServer")).toString();
    proxyPort = se->value(QSL("proxyPort")).toString();
    proxyLogin = se->value(QSL("proxyLogin")).toString();
    proxyPassword = se->value(QSL("proxyPassword")).toString();
    cdromDevice=se->value(QSL("cdromDevice")).toString();

    lastCueFile = se->value(QSL("lastCueFile")).toString();

    columns167 = se->value(QSL("columns167_5.0")).toByteArray();
    rows=se->value(QSL("rows")).toByteArray();

    QVariantList variantList = se->value(QSL("tagStatus")).toList();
    for (const auto &status: variantList)
            tagStatus << status.toInt();
    tagStatus.resize(currentScheme->tagsCount());

/** Autocompletion options*/
    autocompletions = new Autocompletions(this);
    autocompletions->read(se);

    charsShown = se->value(QSL("charsShown"), charsShown).toBool();
    geometry = se->value(QSL("geometry")).toByteArray();

    fillPatterns = se->value(QSL("fill_Patterns"), QStringList("%n. %A - %a")).toStringList();

    if (se->contains(QSL("fillPatterns"))) {
        QStringList l = se->value(QSL("fillPatterns")).toString().split(';');
        Q_FOREACH (const QString &s, l) addPattern(s, fillPatterns);
        se->remove(QSL("fillPatterns"));
    }

    searchPaths = se->value(QSL("searchPaths")).toStringList();
    if (searchPaths.isEmpty()) {
        searchPaths = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    }

    hideTabBar = se->value(QSL("hideTabBar"), false).toBool();
    statusBarTrack = se->value(QSL("statusBarTrack"), 0).toInt();

//    showFullFilesProperties = se->value(QSL("showFullFilesProperties"),false).toBool();
    delete se;
}

QSettings *Application::globalSettings()
{DD;
#ifdef QOOBAR_PORTABLE
    return new QSettings(ApplicationPaths::sharedPath()+QSL("/qoobar.ini"),QSettings::IniFormat);
#else
#ifdef Q_OS_LINUX
    return new QSettings(QString("%1/.config/qoobar/global.ini").arg(QDir::homePath()), QSettings::IniFormat);
#elif defined(Q_OS_MAC)
    return new QSettings(QSL("qoobar"),QSL("global"));
#else
    return new QSettings(QSL("qoobar"),QSL("gui"));
#endif
#endif
}

void Application::readGlobalSettings()
{DD;
    QSettings *se = globalSettings();

    currentSchemeName = se->value("scheme", currentSchemeName).toString();
    if (!currentScheme)
        currentScheme = new TaggingScheme(currentSchemeName);
    else
        currentScheme->setFilePath(currentSchemeName);
    currentScheme->read();

    trim=se->value(QSL("simplify-whitespaces"),QSL("on")).toString()=="on";

    mp3readape=se->value(QSL("read-ape"), QSL("off")).toString()=="on";
    mp3writeape=se->value(QSL("write-ape"), QSL("off")).toString()=="on";
    mp3readid3=se->value(QSL("read-id3v2"), QSL("on")).toString()=="on";
    mp3writeid3=se->value(QSL("write-id3v2"), QSL("on")).toString()=="on";

    flacreadogg=se->value(QSL("flac-read-ogg"), QSL("on")).toString()=="on";
    flacwriteogg=se->value(QSL("flac-write-ogg"), QSL("on")).toString()=="on";
    flacreadid3=se->value(QSL("flac-read-id3v2"), QSL("off")).toString()=="on";
    flacwriteid3=se->value(QSL("flac-read-id3v2"), QSL("off")).toString()=="on";

    delimiters = se->value(QSL("delimiters"),delimiters).toString();

    QString id3v1 = se->value(QSL("id3v1-behavior"), QSL("delete")).toString();
    if (id3v1=="update-always") id3v1Synchro = ID3V1_UPDATE_ALWAYS;
    else if (id3v1=="update-existing") id3v1Synchro = ID3V1_UPDATE_ONLY_EXISTING;

    id3v1Transliterate = se->value("id3v1-transliterate", "on").toString()=="on";
    setId3v1Encoding(se->value("id3v1-encoding", id3v1Encoding).toString());

    cueEncoding = se->value(QSL("cue-encoding"), QSL("Locale")).toString();

    oggPictureNew = se->value("ogg-image-new","on").toString()=="on";

    /** rename options*/
    renameOptions.destinationFolder = se->value("output-dir","").toString();
    renameOptions.replaceWinChars   = se->value("replace-win-chars", "on").toString()=="on";
    renameOptions.winCharsReplacer  = se->value("win-chars-replacer", renameOptions.winCharsReplacer).toString();
    renameOptions.replaceSpaces     = se->value("replace-spaces", "off").toString()=="on";
    renameOptions.spacesReplacer    = se->value("spaces-replacer", renameOptions.spacesReplacer).toString();
    renameOptions.removeDiacritics  = se->value("remove-diacritics", "off").toString()=="on";

    QString s = se->value("change-case", "dont-change").toString();
    renameOptions.ccase = Case::caseByValue(s);
    renameOptions.changeCase = renameOptions.ccase!=Case::DontChange;

    int length = se->value("maximum-length", 0).toInt();
    if (length<=0) {
        renameOptions.maximumLength = MAXIMUM_FILENAME_LENGTH;
        renameOptions.trimFileLength = false;
    }
    else {
        renameOptions.maximumLength = length;
        renameOptions.trimFileLength = true;
    }

    QString renamingOperationStr = se->value("rename-operation", "rename").toString();
    if (renamingOperationStr == "rename")
        renameOptions.renamingOperation = 0;
    else if (renamingOperationStr == "copy")
        renameOptions.renamingOperation = 1;
    else if (renamingOperationStr == "rename-folder")
        renameOptions.renamingOperation = 2;

    renameOptions.removeFolder      = se->value("remove-folder", "on").toString()=="on";
    renameOptions.applyToFolders    = se->value("apply-to-folders", "off").toString()=="on";



    encaGuessLanguage  = se->value("enca-guess-language", encaGuessLanguage).toString();
    writeFieldsSeparately  = se->value("write-fields-separately","off").toString()=="on";
    copyFiles = se->value("rg-copy-files","off").toString()=="on";
    id3v2version = se->value("force-id3v23","off").toString()=="on"?3:4;

    verbose = se->value("verbose",false).toBool();

    //"header" - write rg into file header
    //"tags" - write rg into ape tags
    mpcWriteRg = se->value("mpc-replaygain-format", "header").toString()=="header";

#ifdef Q_OS_MAC
    defaultSplitFormat = se->value("default-split-format", "m4a").toString();
#else
    defaultSplitFormat = se->value("default-split-format", "flac").toString();
#endif

    iconTheme = se->value(QSL("iconTheme"),QSL("maia")).toString();
    QIcon::setThemeName(iconTheme);

    delete se;
}

void Application::writeGuiSettings()
{DD;
    QSettings *se = guiSettings();

    if (!se->isWritable()) {
        criticalMessage(nullptr,tr("Qoobar"),tr("Cannot write settings. The settings file is read-only"));
        delete se;
        return;
    }
    se->setValue("writeGuiSettings",true);
    se->setValue("lang",langID);
    se->setValue("chars",chars);
    se->setValue("lastDirectory",lastDirectory);
    se->setValue("lastTreeDirectory",lastTreeDirectory);
    se->setValue("player",player);
    se->setValue("saveChanges",saveChanges);
    se->setValue("renamePatterns", patterns);
    se->setValue("autoexpand",autoexpand);
    se->setValue("splitter",splitterState);
    se->setValue("innerSplitter",innerSplitterState);
    se->setValue("dirSplitter",dirSplitterState);
    se->setValue("showDirView",showDirView);
    se->setValue("dirViewRoot",dirViewRoot);
    se->setValue("useProxy",useProxy);
    se->setValue("proxyServer",proxyServer);
    se->setValue("proxyPort",proxyPort);
    se->setValue("proxyLogin",proxyLogin);
    se->setValue("proxyPassword",proxyPassword);
    se->setValue("cdromDevice",cdromDevice);
    se->setValue("charsFont",charsFont);
    se->setValue("lastCueFile",lastCueFile);
    se->setValue("columns167_5.0",columns167);

    se->setValue("rows",rows);
    se->setValue("tagStatus",QVariant(makeList(tagStatus)));


    se->setValue("charsShown", charsShown);
    se->setValue("useUndo",useUndo);
    se->setValue("geometry",geometry);
    se->setValue("fill_Patterns",fillPatterns);
    se->setValue(QSL("searchPaths"), searchPaths);
    se->setValue(QSL("hideTabBar"), hideTabBar);
    se->setValue(QSL("statusBarTrack"), statusBarTrack);

//    se->setValue(QSL("showFullFilesProperties"), showFullFilesProperties);

    autocompletions->write(se);
    delete se;
}

void Application::writeGlobalSettings()
{DD;
    QSettings *se = globalSettings();
    if (!se->isWritable()) {
        qCritical("Cannot write settings. The settings file is read-only");
        delete se;
        return;
    }
    se->setValue("writeGlobalSettings",true);
    se->setValue("delimiters", delimiters);

    se->setValue("simplify-whitespaces", trim?"on":"off");
    se->setValue("read-ape", mp3readape?"on":"off");
    se->setValue("write-ape", mp3writeape?"on":"off");
    se->setValue("read-id3v2",mp3readid3?"on":"off");
    se->setValue("write-id3v2", mp3writeid3?"on":"off");
    se->setValue(QSL("flac-read-ogg"), flacreadogg?"on":"off");
    se->setValue(QSL("flac-write-ogg"), flacwriteogg?"on":"off");
    se->setValue(QSL("flac-read-id3v2"), flacreadid3?"on":"off");
    se->setValue(QSL("flac-read-id3v2"), flacwriteid3?"on":"off");

    if (id3v1Synchro == ID3V1_DELETE) se->setValue("id3v1-behavior", "delete");
    else if (id3v1Synchro == ID3V1_UPDATE_ALWAYS) se->setValue("id3v1-behavior", "update-always");
    else if (id3v1Synchro == ID3V1_UPDATE_ONLY_EXISTING) se->setValue("id3v1-behavior", "update-existing");

    se->setValue("id3v1-transliterate", id3v1Transliterate?"on":"off");
    se->setValue("id3v1-encoding", id3v1Encoding);
    se->setValue("ogg-image-new", oggPictureNew?"on":"off");

    se->setValue("cue-encoding", cueEncoding);

    /** rename options*/
    se->setValue("output-dir", renameOptions.destinationFolder);
    se->setValue("replace-win-chars",renameOptions.replaceWinChars? "on":"off");
    se->setValue("win-chars-replacer", renameOptions.winCharsReplacer);
    se->setValue("replace-spaces", renameOptions.replaceSpaces? "on":"off");
    se->setValue("spaces-replacer", renameOptions.spacesReplacer);
    se->setValue("remove-diacritics", renameOptions.removeDiacritics? "on":"off");

    if (!renameOptions.changeCase) se->setValue("change-case", "dont-change");
    else se->setValue("change-case", Case::caseById(Case::Case(renameOptions.ccase)));

    se->setValue("maximum-length", renameOptions.trimFileLength ? renameOptions.maximumLength : 0);

    if (renameOptions.renamingOperation==0)
        se->setValue("rename-operation", "rename");
    else if (renameOptions.renamingOperation==1)
        se->setValue("rename-operation", "copy");
    else if (renameOptions.renamingOperation==2)
        se->setValue("rename-operation", "rename-folder");

    se->setValue("remove-folder", renameOptions.removeFolder?"on":"off");
    se->setValue("apply-to-folders", renameOptions.applyToFolders?"on":"off");
    se->setValue("scheme", currentSchemeName);
    se->setValue("enca-guess-language", encaGuessLanguage);
    se->setValue("write-fields-separately", writeFieldsSeparately?"on":"off");
    se->setValue("rg-copy-files", copyFiles?"on":"off");
    se->setValue("force-id3v23", id3v2version==3?"on":"off");
    se->setValue("verbose",verbose);
    se->setValue("mpc-replaygain-format", mpcWriteRg ? "header" : "tags");
    se->setValue("default-split-format", defaultSplitFormat);
    se->setValue(QSL("iconTheme"),iconTheme);
    delete se;
}

void Application::clearSettings()
{DD;
#ifdef Q_OS_LINUX
    QSettings se(QString("%1/.config/qoobar/global.ini").arg(QDir::homePath()), QSettings::IniFormat);
    se.clear();
#endif
#ifdef Q_OS_MAC
    QSettings se(QSL("qoobar"),QSL("global"));
    se.clear();
#endif
#ifdef QOOBAR_PORTABLE
    QSettings se1(ApplicationPaths::sharedPath()+QSL("/qoobar.ini"),QSettings::IniFormat);
#else
    QSettings se1(QSL("qoobar"),QSL("gui"));
#endif
    se1.clear();
}

void Application::resetSettings()
{DD;
    delete currentScheme;
    currentScheme=nullptr;
    clearSettings();
    Q_EMIT settingsCleared();
    readGlobalSettings();
    readGuiSettings();

    //to be sure settings are written
    writeGuiSettings();
    writeGlobalSettings();
}

void Application::addPattern(const QString &s, QStringList &where)
{DD;
    if (s.isEmpty()) return;
    if (!where.contains(s)) where.prepend(s);
}

void Application::removePattern(const QString &s, QStringList &where)
{DD;
    where.removeAll(s);
}

void Application::loadPlugins()
{DD;
    QDir pluginsDir = QDir(ApplicationPaths::pluginsPath());
    QFileInfoList potentialPlugins = pluginsDir.entryInfoList(QDir::Files);
    Q_FOREACH (const QFileInfo &fileName, potentialPlugins) {
        QString path=fileName.canonicalFilePath();
        QPluginLoader loader(path);
        QJsonObject metaData = loader.metaData().value("MetaData").toObject();

        if (metaData.isEmpty()) continue;
        metaData.insert("path",path);
        QString pluginInterface=metaData.value(QSL("interface")).toString();
        if (pluginInterface=="IDownloadPlugin")
            downloadPlugins << metaData;
        if (pluginInterface=="IQoobarPlugin")
            plugins << metaData;
    }
}

#ifdef Q_OS_MAC
void Application::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (state==Qt::ApplicationActive) {
        Q_EMIT dockClicked();
    }
}
#endif

void criticalMessage(QWidget *parent, const QString &caption, const QString &text)
{DD;
    QMessageBox msgBox(QMessageBox::Critical, caption, text, QMessageBox::NoButton, parent);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.exec();
}

void warningMessage(QWidget *parent, const QString &caption, const QString &text)
{DD;
    QMessageBox msgBox(QMessageBox::Warning, caption, text, QMessageBox::NoButton, parent);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.exec();
}


Autocompletions::Autocompletions(QObject *parent)
    : QObject(parent), completionStyle(false), collectSilently(false), wasChanged(false)
{DD;
    completions.resize(TAGSCOUNT);
    completions[COMPOSER].use = true;
    completions[GENRE].use = true;
    completions[MOOD].use = true;
}

void Autocompletions::read(QSettings *se)
{DD;
    completionStyle = se->value(QSL("completionStyle"), false).toBool();
    collectSilently = se->value(QSL("collectSilently"), false).toBool();

    QVariantList variantList = se->value(QSL("useCompletion")).toList();

    completions.resize(variantList.size());
    if (variantList.isEmpty()) {
        completions.clear();
        completions.resize(TAGSCOUNT);
        completions[COMPOSER].use = true;
        completions[GENRE].use = true;
        completions[MOOD].use = true;
    }
    else
    for (int i=0; i<variantList.size(); ++i) {
        completions[i].use = variantList.at(i).toBool();
        completions[i].name = App->currentScheme->fieldName(i);
    }

    const QString userPath = ApplicationPaths::userCompletionsPath();
    const QString path = ApplicationPaths::completionsPath();

    for (auto &completion: completions) {
        QString name = completion.name;
        name.remove("/").remove("\\");
        QString fileName = QString("%1/%2.txt").arg(userPath, name);
        if (!QFileInfo::exists(fileName))
            fileName = QString("%1/%2.txt").arg(path, name);
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&file);
            in.setCodec("UTF-8");
            completion.variants = in.readAll().split('\n');
        }
    }
}

bool Autocompletions::write(QSettings *se)
{DD;
    se->setValue("completionStyle", completionStyle);
    se->setValue("collectSilently", collectSilently);

    QVariantList useCompletion;
    for (const auto &completion: completions) {
        useCompletion.append(completion.use);
    }
    se->setValue("useCompletion", QVariant(useCompletion));

    if (!wasChanged) return true;

    QString path = ApplicationPaths::userCompletionsPath();
    if (!QDir().exists(path)) {
        if (!QDir().mkpath(path)) {
            criticalMessage(nullptr,tr("Qoobar"),tr("Cannot write resource files into\n%1.\n"
                                              "Please check the folder").arg(path));
            return false;
        }
    }

    Q_FOREACH (const Completion &c, completions) {
        if (!c.wasChanged) continue;

        QStringList list = c.variants;
        QString name = c.name;
        name.remove("/").remove("\\");

        QFile file(QString("%1/%2.txt").arg(path, name));
        if (!list.isEmpty()) {
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream out(&file);
                out.setCodec("UTF-8");
                out << list.join(QChar('\n'));
            }
            else {
                criticalMessage(nullptr,tr("Qoobar"),tr("Cannot write %1.\n"
                                                  "Please check the file").arg(file.fileName()));
                return false;
            }
        }
        else
            file.remove();
    }
    return true;
}

void Autocompletions::update(int tagID, const QStringList &newVariants)
{DD;
    if (tagID >= completions.size()) completions.resize(tagID+1);

    Completion &c = completions[tagID];
    c.use = true;

    QStringList list = c.variants;

    list.append(newVariants);
    list.removeDuplicates();

    for (int i=list.size()-1; i>=0; --i) {
        if (list.at(i).isEmpty()) {
            list.removeAt(i);
            break;
        }
    }

    if (list != c.variants) {
        c.variants = list;
        c.wasChanged = true;
        wasChanged = true;
    }
}

void Autocompletions::set(int tagID, const QStringList &newVariants)
{DD;
    if (tagID >= completions.size()) completions.resize(tagID+1);

    Completion &c = completions[tagID];
    c.use = true;

    if (newVariants != c.variants) {
        c.variants = newVariants;
        c.wasChanged = true;
        wasChanged = true;
    }
}

QSize dpiAwareSize(int width, int height, QPaintDevice *d)
{
    if (!d) return {width, height};

#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
    return {int(width*d->devicePixelRatioF()), int(height*d->devicePixelRatioF())};
#else
    return {width*d->devicePixelRatio(), height*d->devicePixelRatio()};
#endif
}

int dpiAwareSize(int size, QPaintDevice *d)
{
    if (!d) return size;

#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
    return int(size*d->devicePixelRatioF());
#else
    return size * d->devicePixelRatio();
#endif
}

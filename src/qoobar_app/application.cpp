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
#ifdef HAVE_QT5
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>
#else
#include <QtGui>
#include <QDesktopServices>
#endif

#include "id3v1stringhandler.h"


#include "applicationpaths.h"
#include "iqoobarplugin.h"
#include "idownloadplugin.h"
#include "enums.h"
#include "qoobarglobals.h"

//#include <QProxyStyle>

//class ProxyStyle : public QProxyStyle
//{
//public:
//    void drawPrimitive(PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0)
//    {
//        QRect rect = option->rect;
//        if (element == PE_PanelButtonTool /*&& widget->property("fancy").toBool()*/) {
//            qDebug()<<1;
//            bool pressed = option->state & State_Sunken || option->state & State_On;
//            if (pressed) {
//                painter->save();
//                QLinearGradient grad(rect.topLeft(), rect.topRight());
//                grad.setColorAt(0, QColor(255, 255, 255, 140));
//                grad.setColorAt(1, QColor(255, 255, 255, 210));
//                painter->fillRect(rect.adjusted(0, 0, 0, -1), grad);
//                painter->restore();

//                //shadows
//                painter->setPen(QColor(0, 0, 0, 110));
//                painter->drawLine(rect.topLeft() + QPoint(1,-1), rect.topRight() - QPoint(0,1));
//                painter->drawLine(rect.bottomLeft(), rect.bottomRight());
//                painter->setPen(QColor(0, 0, 0, 40));
//                painter->drawLine(rect.topLeft(), rect.bottomLeft());

//                //highlights
//                painter->setPen(QColor(255, 255, 255, 50));
//                painter->drawLine(rect.topLeft() + QPoint(0, -2), rect.topRight() - QPoint(0,2));
//                painter->drawLine(rect.bottomLeft() + QPoint(0, 1), rect.bottomRight() + QPoint(0,1));
//                painter->setPen(QColor(255, 255, 255, 40));
//                painter->drawLine(rect.topLeft() + QPoint(0, 0), rect.topRight());
//                painter->drawLine(rect.topRight() + QPoint(0, 1), rect.bottomRight() - QPoint(0, 1));
//                painter->drawLine(rect.bottomLeft() + QPoint(0,-1), rect.bottomRight()-QPoint(0,1));
//                painter->restore();
//            }
////            QColor shadow(0, 0, 0, 30);
////            painter->setPen(shadow);
////            if (pressed) {
////                QColor shade = option->palette.base().color();
////                shade.setHsv(shade.hue(), shade.saturation(), 255 - shade.value(), 40);
////                painter->fillRect(rect, shade);
////                painter->drawLine(rect.topLeft() + QPoint(1, 0), rect.topRight() - QPoint(1, 0));
////                painter->drawLine(rect.topLeft(), rect.bottomLeft());
////                painter->drawLine(rect.topRight(), rect.bottomRight());
////            } else if (option->state & State_Enabled && option->state & State_MouseOver) {
////                painter->fillRect(rect, QColor("#25ffffff"));
////            }
////            if (option->state & State_HasFocus && (option->state & State_KeyboardFocusChange)) {
////                QColor highlight = option->palette.highlight().color();
////                highlight.setAlphaF(0.4);
////                painter->setPen(QPen(highlight.lighter(), 1));
////                highlight.setAlphaF(0.3);
////                painter->setBrush(highlight);
////                painter->setRenderHint(QPainter::Antialiasing);
////                const QRectF rect = option->rect;
////                painter->drawRoundedRect(rect.adjusted(2.5, 2.5, -2.5, -2.5), 2, 2);
////            }
//        }
//        else
//            QProxyStyle::drawPrimitive(element, option, painter, widget);

//    }
//};

bool isValidLibrary(const QFileInfo &path)
{
    QLibrary lib(path.canonicalFilePath());

    if (lib.load()) {
        lib.unload();
        return true;
    }
    return false;
}

Application::Application(int &argc, char **argv, bool useGui)
    :  QApplication(argc,argv,useGui), autocompletions(0)
{DD;
    if (useGui) setWindowIcon(QIcon(QSL(":/src/icons/app/qoobar.ico")));
    appTranslator = new QTranslator(this);
    qtTranslator = new QTranslator(this);
    installTranslator(appTranslator);
    installTranslator(qtTranslator);

//    setStyle(new ProxyStyle);

    // Setting default values
    langID = QSL("en");
    static const QChar rawChars[] = {0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,       0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
             0x101,      0x103,      0x105,      0x107,      0x109,      0x10b,      0x10d,      0x10f,
             0x111,      0x113,      0x115,      0x117,      0x119,      0x11b,      0x11d,      0x11f,
             0x121,      0x123,      0x125,      0x127,      0x129,      0x12b,      0x12d,      0x12f,
             0x131,      0x133,      0x135,      0x137,0x138,      0x13a,      0x13c,      0x13e,
       0x140,      0x142,      0x144,      0x146,      0x148,0x149,      0x14b,      0x14d,      0x14f,
             0x151,      0x153,      0x155,      0x157,      0x159,      0x15b,      0x15d,      0x15f,
             0x161,      0x163,      0x165,      0x167,      0x169,      0x16b,      0x16d,      0x16f,
             0x171,      0x173,      0x175,      0x177,            0x17a,      0x17c,      0x17e
    };
    chars = QString(rawChars,96);
    lastDirectory = QDir::homePath();
    mp3readape = false;
    mp3writeape = false;
    mp3readid3 = true;
    mp3writeid3 = true;
    saveChanges = false;
    trim = true;
    autoexpand = true;
    oggPictureNew = true;

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
    currentScheme = 0;
    useUndo = false;
    encaGuessLanguage = QSL("russian");
    writeFieldsSeparately = false;
    copyFiles = false;
    id3v2version = 4;
    verbose = true;
    recursive = false;
    consoleMode = false;
    mpcWriteRg = true;
    iconTheme = "default";

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
    currentScheme=0;
}

void Application::loadTranslations()
{DD;
    if (qtTranslator)
        qtTranslator->load(QSL("qt_") + langID, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    if (appTranslator)
        appTranslator->load(ApplicationPaths::translationsPath()+QSL("/qoobar_") + langID);
}

void Application::setId3v1Encoding(const QString &s)
{DD;
    id3v1Encoding=s;
    ID3v1StringHandler *stringHandler = new ID3v1StringHandler;
    if (id3v1Encoding=="Locale")
        stringHandler->setCodec(QTextCodec::codecForLocale());
    else
        stringHandler->setCodec(QTextCodec::codecForName(id3v1Encoding.toLatin1()));
}

void Application::readGuiSettings()
{DD;
#ifdef QOOBAR_PORTABLE
    QSettings se(ApplicationPaths::sharedPath()+QSL("/qoobar.ini"),QSettings::IniFormat);
#else
    QSettings se(QSL("qoobar"),QSL("gui"));
#endif

    //Interface
    useUndo = se.value(QSL("useUndo"), useUndo).toBool();
    autoexpand=se.value(QSL("autoexpand"), autoexpand).toBool();

    chars=se.value(QSL("chars"), chars).toString();
    QVariant charsFontV=se.value(QSL("charsFont"),font());
    charsFont=charsFontV.value<QFont>();

    langID=se.value(QSL("lang"),QSL("en")).toString();
    loadTranslations();

    player=se.value(QSL("player")).toString();
    saveChanges=se.value(QSL("saveChanges"),saveChanges).toBool();

    patterns = se.value(QSL("renamePatterns"), QStringList()<<"[%N. ]%t"
                        << "%a/[%N. ]%t"
                        << "[%N. ]%A - %t").toStringList();

    // old setting, contains patterns in a string separated by ;
    // use and then remove if present
    if (se.contains(QSL("patterns"))) {
        QStringList l = se.value(QSL("patterns")).toString().split(';');
        Q_FOREACH (const QString &s, l) addPattern(s, patterns);
        se.remove(QSL("patterns"));
    }
    QString pattern = se.value(QSL("pattern")).toString();
    if (!pattern.isEmpty()) {
        addPattern(pattern,patterns);
        se.remove(QSL("pattern"));
    }

    lastDirectory=se.value(QSL("lastDirectory"),lastDirectory).toString();
    lastTreeDirectory=se.value(QSL("lastTreeDirectory"),lastTreeDirectory).toString();
    splitterState = se.value(QSL("splitter")).toByteArray();
    innerSplitterState = se.value(QSL("innerSplitter")).toByteArray();
    dirSplitterState = se.value(QSL("dirSplitter")).toByteArray();
    showDirView = se.value(QSL("showDirView"), showDirView).toBool();
    dirViewRoot = se.value(QSL("dirViewRoot"), dirViewRoot).toString();
    if (dirViewRoot.isEmpty()) {
#ifdef Q_OS_WIN
        dirViewRoot = QSL("");
#else
        dirViewRoot = QSL("/");
#endif
    }

    useProxy = se.value(QSL("useProxy"), useProxy).toBool();
    proxyServer = se.value(QSL("proxyServer")).toString();
    proxyPort = se.value(QSL("proxyPort")).toString();
    proxyLogin = se.value(QSL("proxyLogin")).toString();
    proxyPassword = se.value(QSL("proxyPassword")).toString();
    cdromDevice=se.value(QSL("cdromDevice")).toString();

    lastCueFile = se.value(QSL("lastCueFile")).toString();

#ifdef HAVE_QT5
    columns167 = se.value(QSL("columns167_5.0")).toByteArray();
#else
    columns167 = se.value(QSL("columns167")).toByteArray();
#endif
    rows=se.value(QSL("rows")).toByteArray();

    QVariantList variantList = se.value(QSL("tagStatus")).toList();
    for (int i=0; i<variantList.size(); ++i)
            tagStatus << variantList.at(i).toInt();
    tagStatus.resize(currentScheme->tagsCount());

/** Autocompletion options*/
    autocompletions = new Autocompletions(this);
    autocompletions->read(se);

    charsShown = se.value(QSL("charsShown"), charsShown).toBool();
    geometry = se.value(QSL("geometry")).toByteArray();

    fillPatterns = se.value(QSL("fill_Patterns"), QStringList("%n. %A - %a")).toStringList();

    if (se.contains(QSL("fillPatterns"))) {
        QStringList l = se.value(QSL("fillPatterns")).toString().split(';');
        Q_FOREACH (const QString &s, l) addPattern(s, fillPatterns);
        se.remove(QSL("fillPatterns"));
    }

    searchPaths = se.value(QSL("searchPaths")).toStringList();
    if (searchPaths.isEmpty()) {
#ifdef HAVE_QT5
        searchPaths = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
#else
        searchPaths << QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
#endif
    }

    hideTabBar = se.value(QSL("hideTabBar"), false).toBool();

//    showFullFilesProperties = se.value(QSL("showFullFilesProperties"),false).toBool();
}

void Application::readGlobalSettings()
{DD;
#ifdef QOOBAR_PORTABLE
    QSettings se(ApplicationPaths::sharedPath()+QSL("/qoobar.ini"),QSettings::IniFormat);
#else
#ifdef Q_OS_LINUX
    QSettings se(QString("%1/.config/qoobar/global.ini").arg(QDir::homePath()), QSettings::IniFormat);
#elif defined(Q_OS_MAC)
    QSettings se(QSL("qoobar"),QSL("global"));
#else
    QSettings se(QSL("qoobar"),QSL("gui"));
#endif
#endif

    currentSchemeName = se.value("scheme", currentSchemeName).toString();
    if (!currentScheme)
        currentScheme = new TaggingScheme(currentSchemeName);
    else
        currentScheme->setFilePath(currentSchemeName);
    currentScheme->read();

    trim=se.value(QSL("simplify-whitespaces"),QSL("on")).toString()=="on";

    mp3readape=se.value(QSL("read-ape"), QSL("off")).toString()=="on";
    mp3writeape=se.value(QSL("write-ape"), QSL("off")).toString()=="on";
    mp3readid3=se.value(QSL("read-id3v2"), QSL("on")).toString()=="on";
    mp3writeid3=se.value(QSL("write-id3v2"), QSL("on")).toString()=="on";

    QString id3v1 = se.value(QSL("id3v1-behavior"), QSL("delete")).toString();
    if (id3v1=="update-always") id3v1Synchro = ID3V1_UPDATE_ALWAYS;
    else if (id3v1=="update-existing") id3v1Synchro = ID3V1_UPDATE_ONLY_EXISTING;

    id3v1Transliterate = se.value("id3v1-transliterate", "on").toString()=="on";
    setId3v1Encoding(se.value("id3v1-encoding", id3v1Encoding).toString());

    cueEncoding = se.value(QSL("cue-encoding"), QSL("Locale")).toString();

    oggPictureNew = se.value("ogg-image-new","on").toString()=="on";

    /** rename options*/
    renameOptions.destinationFolder = se.value("output-dir","").toString();
    renameOptions.replaceWinChars   = se.value("replace-win-chars", "on").toString()=="on";
    renameOptions.winCharsReplacer  = se.value("win-chars-replacer", renameOptions.winCharsReplacer).toString();
    renameOptions.replaceSpaces     = se.value("replace-spaces", "off").toString()=="on";
    renameOptions.spacesReplacer    = se.value("spaces-replacer", renameOptions.spacesReplacer).toString();
    renameOptions.removeDiacritics  = se.value("remove-diacritics", "off").toString()=="on";

    QString s = se.value("change-case", "dont-change").toString();
    renameOptions.ccase = Case::caseByValue(s);
    renameOptions.changeCase = renameOptions.ccase!=Case::DontChange;

    int length = se.value("maximum-length", 0).toInt();
    if (length<=0) {
        renameOptions.maximumLength = 255;
        renameOptions.trimFileLength = false;
    }
    else {
        renameOptions.maximumLength = length;
        renameOptions.trimFileLength = true;
    }

    QString renamingOperationStr = se.value("rename-operation", "rename").toString();
    if (renamingOperationStr == "rename")
        renameOptions.renamingOperation = 0;
    else if (renamingOperationStr == "copy")
        renameOptions.renamingOperation = 1;
    else if (renamingOperationStr == "rename-folder")
        renameOptions.renamingOperation = 2;

    renameOptions.removeFolder      = se.value("remove-folder", "on").toString()=="on";
    renameOptions.applyToFolders    = se.value("apply-to-folders", "off").toString()=="on";



    encaGuessLanguage  = se.value("enca-guess-language", encaGuessLanguage).toString();
    writeFieldsSeparately  = se.value("write-fields-separately","off").toString()=="on";
    copyFiles = se.value("rg-copy-files","off").toString()=="on";
    id3v2version = se.value("force-id3v23","off").toString()=="on"?3:4;

    verbose = se.value("verbose",false).toBool();

    //"header" - write rg into file header
    //"tags" - write rg into ape tags
    mpcWriteRg = se.value("mpc-replaygain-format", "header").toString()=="header";

#ifdef Q_OS_MAC
    defaultSplitFormat = se.value("default-split-format", "m4a").toString();
#else
    defaultSplitFormat = se.value("default-split-format", "flac").toString();
#endif
    iconTheme = se.value(QSL("iconTheme"),QSL("default")).toString();
}

void Application::writeGuiSettings()
{DD;
#ifdef QOOBAR_PORTABLE
    QSettings se(ApplicationPaths::sharedPath()+QSL("/qoobar.ini"),QSettings::IniFormat);
#else
    QSettings se(QSL("qoobar"),QSL("gui"));
#endif
    if (!se.isWritable()) {
        criticalMessage(0,tr("Qoobar"),tr("Cannot write settings. The settings file is read-only"));
        return;
    }
    se.setValue("writeGuiSettings",true);
    se.setValue("lang",langID);
    se.setValue("chars",chars);
    se.setValue("lastDirectory",lastDirectory);
    se.setValue("lastTreeDirectory",lastTreeDirectory);
    se.setValue("player",player);
    se.setValue("saveChanges",saveChanges);
    se.setValue("renamePatterns", patterns);
    se.setValue("autoexpand",autoexpand);
    se.setValue("splitter",splitterState);
    se.setValue("innerSplitter",innerSplitterState);
    se.setValue("dirSplitter",dirSplitterState);
    se.setValue("showDirView",showDirView);
    se.setValue("dirViewRoot",dirViewRoot);
    se.setValue("useProxy",useProxy);
    se.setValue("proxyServer",proxyServer);
    se.setValue("proxyPort",proxyPort);
    se.setValue("proxyLogin",proxyLogin);
    se.setValue("proxyPassword",proxyPassword);
    se.setValue("cdromDevice",cdromDevice);
    se.setValue("charsFont",charsFont);
    se.setValue("lastCueFile",lastCueFile);
#ifdef HAVE_QT5
    se.setValue("columns167_5.0",columns167);
#else
    se.setValue("columns167",columns167);
#endif

    se.setValue("rows",rows);
    se.setValue("tagStatus",QVariant(makeList(tagStatus)));


    se.setValue("charsShown", charsShown);
    se.setValue("useUndo",useUndo);
    se.setValue("geometry",geometry);
    se.setValue("fill_Patterns",fillPatterns);
    se.setValue(QSL("searchPaths"), searchPaths);
    se.setValue(QSL("hideTabBar"), hideTabBar);

//    se.setValue(QSL("showFullFilesProperties"), showFullFilesProperties);

    autocompletions->write(se);
}

void Application::writeGlobalSettings()
{DD;
#ifdef QOOBAR_PORTABLE
    QSettings se(ApplicationPaths::sharedPath()+QSL("/qoobar.ini"),QSettings::IniFormat);
#else
#ifdef Q_OS_LINUX
    QSettings se(QString("%1/.config/qoobar/global.ini").arg(QDir::homePath()), QSettings::IniFormat);
#elif defined(Q_OS_MAC)
    QSettings se(QSL("qoobar"),QSL("global"));
#else
    QSettings se(QSL("qoobar"),QSL("gui"));
#endif
#endif
    if (!se.isWritable()) {
        qCritical("Cannot write settings. The settings file is read-only");
        return;
    }
    se.setValue("writeGlobalSettings",true);

    se.setValue("simplify-whitespaces", trim?"on":"off");
    se.setValue("read-ape", mp3readape?"on":"off");
    se.setValue("write-ape", mp3writeape?"on":"off");
    se.setValue("read-id3v2",mp3readid3?"on":"off");
    se.setValue("write-id3v2", mp3writeid3?"on":"off");

    if (id3v1Synchro == ID3V1_DELETE) se.setValue("id3v1-behavior", "delete");
    else if (id3v1Synchro == ID3V1_UPDATE_ALWAYS) se.setValue("id3v1-behavior", "update-always");
    else if (id3v1Synchro == ID3V1_UPDATE_ONLY_EXISTING) se.setValue("id3v1-behavior", "update-existing");

    se.setValue("id3v1-transliterate", id3v1Transliterate?"on":"off");
    se.setValue("id3v1-encoding", id3v1Encoding);
    se.setValue("ogg-image-new", oggPictureNew?"on":"off");

    se.setValue("cue-encoding", cueEncoding);

    /** rename options*/
    se.setValue("output-dir", renameOptions.destinationFolder);
    se.setValue("replace-win-chars",renameOptions.replaceWinChars? "on":"off");
    se.setValue("win-chars-replacer", renameOptions.winCharsReplacer);
    se.setValue("replace-spaces", renameOptions.replaceSpaces? "on":"off");
    se.setValue("spaces-replacer", renameOptions.spacesReplacer);
    se.setValue("remove-diacritics", renameOptions.removeDiacritics? "on":"off");

    if (!renameOptions.changeCase) se.setValue("change-case", "dont-change");
    else se.setValue("change-case", Case::caseById(Case::Case(renameOptions.ccase)));

    se.setValue("maximum-length", renameOptions.trimFileLength ? renameOptions.maximumLength : 0);

    if (renameOptions.renamingOperation==0)
        se.setValue("rename-operation", "rename");
    else if (renameOptions.renamingOperation==1)
        se.setValue("rename-operation", "copy");
    else if (renameOptions.renamingOperation==2)
        se.setValue("rename-operation", "rename-folder");

    se.setValue("remove-folder", renameOptions.removeFolder?"on":"off");
    se.setValue("apply-to-folders", renameOptions.applyToFolders?"on":"off");
    se.setValue("scheme", currentSchemeName);
    se.setValue("enca-guess-language", encaGuessLanguage);
    se.setValue("write-fields-separately", writeFieldsSeparately?"on":"off");
    se.setValue("rg-copy-files", copyFiles?"on":"off");
    se.setValue("force-id3v23", id3v2version==3?"on":"off");
    se.setValue("verbose",verbose);
    se.setValue("mpc-replaygain-format", mpcWriteRg ? "header" : "tags");
    se.setValue("default-split-format", defaultSplitFormat);
    se.setValue(QSL("iconTheme"),iconTheme);
}

QString Application::iconThemeIcon(const QString &icon)
{
    return QString("%1/icons/%2/%3")
            .arg(ApplicationPaths::sharedPath())
            .arg(App->iconTheme)
            .arg(icon);
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
    QSettings se(ApplicationPaths::sharedPath()+QSL("/qoobar.ini"),QSettings::IniFormat);
#else
    QSettings se1(QSL("qoobar"),QSL("gui"));
#endif
    se1.clear();
}

void Application::resetSettings()
{DD;
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
#ifdef HAVE_QT5
        QJsonObject metaData = loader.metaData().value("MetaData").toObject();

        if (metaData.isEmpty()) continue;
        metaData.insert("path",path);
        QString pluginInterface=metaData.value(QSL("interface")).toString();
        if (pluginInterface=="IDownloadPlugin")
            downloadPlugins << metaData;
        if (pluginInterface=="IQoobarPlugin")
            plugins << metaData;
#else
        QObject *plugin = loader.instance();
        if (plugin) {
            IQoobarPlugin *iplugin = qobject_cast<IQoobarPlugin *>(plugin);
            if (iplugin) plugins.insert(iplugin->key(),iplugin);
            IDownloadPlugin *iplugin1 = qobject_cast<IDownloadPlugin *>(plugin);
            if (iplugin1) downloadPlugins.insert(iplugin1->key(),iplugin1);
        }
#endif
    }
}


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

void Autocompletions::read(QSettings &se)
{DD;
    completionStyle = se.value(QSL("completionStyle"), false).toBool();
    collectSilently = se.value(QSL("collectSilently"), false).toBool();

    QVariantList variantList = se.value(QSL("useCompletion")).toList();
    completions.resize(variantList.size());

    for (int i=0; i<variantList.size(); ++i) {
        completions[i].use = variantList.at(i).toBool();
        completions[i].name = App->currentScheme->fieldName(i);
    }

    const QString userPath = ApplicationPaths::userCompletionsPath();
    const QString path = ApplicationPaths::completionsPath();

    for (int tagID=0; tagID<completions.size(); ++tagID) {
        QString name = completions.at(tagID).name;
        name.remove("/").remove("\\");
        QString fileName = QString("%1/%2.txt").arg(userPath).arg(name);
        if (!QFileInfo(fileName).exists())
            fileName = QString("%1/%2.txt").arg(path).arg(name);
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&file);
            in.setCodec("UTF-8");
            completions[tagID].variants = in.readAll().split('\n');
        }
    }
}

bool Autocompletions::write(QSettings &se)
{DD;
    se.setValue("completionStyle", completionStyle);
    se.setValue("collectSilently", collectSilently);

    QVariantList useCompletion;
    for (int tagID = 0; tagID < completions.size(); ++tagID) {
        useCompletion.append(completions.at(tagID).use);
    }
    se.setValue("useCompletion", QVariant(useCompletion));

    if (!wasChanged) return true;

    QString path = ApplicationPaths::userCompletionsPath();
    if (!QDir().exists(path)) {
        if (!QDir().mkpath(path)) {
            criticalMessage(0,tr("Qoobar"),tr("Cannot write resource files into\n%1.\n"
                                              "Please check the folder").arg(path));
            return false;
        }
    }

    Q_FOREACH (const Completion &c, completions) {
        if (!c.wasChanged) continue;

        QStringList list = c.variants;
        QString name = c.name;
        name.remove("/").remove("\\");

        QFile file(QString("%1/%2.txt").arg(path).arg(name));
        if (!list.isEmpty()) {
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream out(&file);
                out.setCodec("UTF-8");
                out << list.join(QChar('\n'));
            }
            else {
                criticalMessage(0,tr("Qoobar"),tr("Cannot write %1.\n"
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

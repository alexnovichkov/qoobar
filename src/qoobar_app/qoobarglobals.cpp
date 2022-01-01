/**
 * \file qoobarglobals.cpp
 * Contains types and enums that are used thoughout qoobar.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 8 Dec 2010
 *
 * Copyright (C) 2011  Alex Novichkov
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

#include "qoobarglobals.h"
#include <QApplication>

#include <QProcess>
#include <QHash>
#include <QDir>
#include "enums.h"
#include "applicationpaths.h"
#include "application.h"
#include <QDesktopServices>
#include <QUrl>
#include <QLocale>
#include <QStandardPaths>

//void Qoobar::showHelp(const QString &section)
//{DD;
//    QString path = ApplicationPaths::documentationPath();
//    if (!QFileInfo(path).exists())
//        path = QSL("http://qoobar.sourceforge.net/en/documentation.htm");
//    else path = QString("file:///%1").arg(path);
//    if (!section.isEmpty())
//        path.append(QSL("#") + section);
//    QDesktopServices::openUrl(QUrl(path));
//}

bool Qoobar::programInstalled(const QString &program, QString *path)
{DD;
#ifdef Q_OS_WIN
    if (path != nullptr) *path = qApp->applicationDirPath();

    //prefer the program from the installation folder
    QString programPath = QString("%1/%2.exe").arg(qApp->applicationDirPath(), program);
    bool exists = QFile::exists(programPath);
    if (exists) {
        if (path != nullptr) *path = programPath;
        return true;
    }

    //fallback to search in system paths
    programPath = QStandardPaths::findExecutable(program);
    if (!programPath.isEmpty()) {
        if (path != nullptr) *path = programPath;
        return true;
    }
    return false;
#endif
#ifdef Q_OS_OS2
    bool exists = QFile::exists(QString("%1/%2.exe").arg(qApp->applicationDirPath()).arg(program));
    if (exists) {
        if (path) *path = QString("%1/%2.exe").arg(qApp->applicationDirPath()).arg(program);
    }
    else {
        QString s = QStandardPaths::findExecutable(program);
        if (!s.isEmpty()) {
            if (path) *path = s;
            exists = true;
        }
        else if (path) *path = qApp->applicationDirPath();
    }
    return exists;
#endif
#ifdef Q_OS_LINUX
    QProcess p;
    p.start(QString("which %1").arg(program));
    p.waitForFinished();
    QByteArray b=p.readAll();
    QString s=QString(b).simplified();
    if (b.isEmpty()) return false;
    else if (QFile::exists(s)) {
        if (path) *path = s;
        return true;
    }
    return false;
#endif
#ifdef Q_OS_MACOS
    QDir dir(qApp->applicationDirPath());
    dir.cdUp();
    QString s = QString("%1/Resources/%2").arg(dir.canonicalPath()).arg(program);
    if (path) *path = s;
    if (QFile::exists(s)) {
        return true;
    }
    else {
        QProcess p;
        p.start(QString("which %1").arg(program));
        p.waitForFinished();
        QByteArray b=p.readAll();
        s=QString(b).simplified();
        if (path && path->isEmpty()) *path = s;
        if (b.isEmpty()) {
            return false;
        }
        else if (QFile::exists(s)) {
            return true;
        }
    }
    return false;
#endif
}


QString Qoobar::formatLength(int seconds)
{DD;
    if (seconds<3600)
        return QString("%1:%2").arg(int(seconds/60))
                           .arg(seconds % 60,2,10,QChar('0'));
    else return QString("%1:%2:%3").arg(int(seconds/3600))
            .arg((int(seconds/60)) % 60,2,10,QChar('0'))
            .arg(seconds % 60,2,10,QChar('0'));
}

static const char *tag_strings[] = {
    QT_TRANSLATE_NOOP("QObject","Composer"),
    QT_TRANSLATE_NOOP("QObject","Album"),
    QT_TRANSLATE_NOOP("QObject","Title"),
    QT_TRANSLATE_NOOP("QObject","Performer"),
    QT_TRANSLATE_NOOP("QObject","Artist"),
    QT_TRANSLATE_NOOP("QObject","Conductor"),
    QT_TRANSLATE_NOOP("QObject","Orchestra"),
    QT_TRANSLATE_NOOP("QObject","Subtitle"),
    QT_TRANSLATE_NOOP("QObject","Key"),
    QT_TRANSLATE_NOOP("QObject","Comment"),
    QT_TRANSLATE_NOOP("QObject","Genre"),
    QT_TRANSLATE_NOOP("QObject","Year"),
    QT_TRANSLATE_NOOP("QObject","Track number"),
    QT_TRANSLATE_NOOP("QObject","Total tracks"),
    QT_TRANSLATE_NOOP("QObject","Album artist"),
    QT_TRANSLATE_NOOP("QObject","Music category"),
    QT_TRANSLATE_NOOP("QObject","Publisher"),
    QT_TRANSLATE_NOOP("QObject","Copyright"),
    QT_TRANSLATE_NOOP("QObject","Mood"),
    QT_TRANSLATE_NOOP("QObject","Beats per minute"),
    QT_TRANSLATE_NOOP("QObject","Lyricist"),
    QT_TRANSLATE_NOOP("QObject","Lyrics"),
    QT_TRANSLATE_NOOP("QObject","Disc number"),
    QT_TRANSLATE_NOOP("QObject","Total discs"),
    QT_TRANSLATE_NOOP("QObject","Encoded by"),
    QT_TRANSLATE_NOOP("QObject","Remixed by"),
    QT_TRANSLATE_NOOP("QObject","Rating"),
    QT_TRANSLATE_NOOP("QObject","Original album"),
    QT_TRANSLATE_NOOP("QObject","Original artist"),
    QT_TRANSLATE_NOOP("QObject","Original lyricist"),
    QT_TRANSLATE_NOOP("QObject","URL"),
    QT_TRANSLATE_NOOP("QObject","ISRC")
};

QString Qoobar::tagByID(int id)
{DD;
    if (id>=0 && id<TAGSCOUNT) return QObject::tr(tag_strings[id]);
    return QString();
}

QString Qoobar::untranslatedTagByID(int id)
{DD;
    if (id>=0 && id<TAGSCOUNT) return QString(tag_strings[id]);
    return QString();
}

QStringList Qoobar::musicalFilters()
{DD;
    return QString("*.mp3,*.mpc,*.ogg,*.flac,*.oga,"
                   "*.wv,*.spx,*.tta,*.ape,*.opus"
                   ",*.wma,*.asf,*.m4a,*.m4b,*.m4p,*.3g2,*.mp4,*.wav,*.aiff,*.aif,*.dsf"
                   ).split(QSL(","));
}

void maybeAppend(const QString &s, QStringList &list)
{DD;
    if (!list.contains(s)) list.append(s);
}

void Qoobar::processDir(const QString &file, QStringList &files, bool includeSubfolders)
{DD;
    static QStringList filters=musicalFilters();
    if (QFileInfo(file).isDir()) {
        QFileInfoList dirLst = QDir(file).entryInfoList(filters,
                                                        QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot,
                                                        QDir::DirsFirst);
        for (int i=0; i<dirLst.count(); ++i) {
            if (dirLst.at(i).isDir()) {
                if (includeSubfolders)
                    processDir(dirLst.at(i).absoluteFilePath(),files,includeSubfolders);
            }
            else
                maybeAppend(dirLst.at(i).absoluteFilePath(), files);
        }
    }
    else
        maybeAppend(file, files);
}

QStringList Qoobar::expandFilesNames(const QStringList &filesNames, bool includeSubfolders)
{DD;
    QStringList filesToAdd;
    int count = filesNames.size();
    if (count <= 0)
        return filesToAdd;

    QStringList filters=musicalFilters();
    for (int i = 0; i < count; ++i) {
        QFileInfo f(filesNames.at(i));
        QString s = f.canonicalFilePath();

        if (f.isDir() || filters.contains(QString("*.%1").arg(f.suffix().toLower())))
            processDir(s,filesToAdd,includeSubfolders);
    }
    return filesToAdd;
}

namespace Case {
const struct Cases {
    std::string value;
    Case id;
} cases[6] = {
    {"lower", AllSmall},
    {"upper", AllCaps},
    {"first-up", FirstCaps},
    {"every-first-up", EveryFirstCaps},
    {"first-up-preserving", EveryFirstCapsPreserving},
    {"dont-change", DontChange},
};

Case caseByValue(const QString &value)
{DD;
    for (int i=0; i<6; ++i)
        if (cases[i].value==value.toStdString()) return cases[i].id;
    return DontChange;
}

QString caseById(const Case id)
{DD;
    for (int i=0; i<6; ++i)
        if (cases[i].id==id) return QString::fromStdString(cases[i].value);
    return "dont-change";
}

}


QString Qoobar::formatSize(qint64 size)
{DD;
    //size in bytes, format in 1024
    if (size < 1024) return QObject::tr("%n byte(s)","",size); // bytes
    float kib = float(size) / 1024.0;

    if (kib < 1024.0) return QObject::tr("%1 KiB").arg(QLocale(App->langID).toString(kib,'f',2)); //kibibytes
    kib /= 1024.0;
    return QObject::tr("%1 MiB").arg(QLocale(App->langID).toString(kib,'f',2));
}

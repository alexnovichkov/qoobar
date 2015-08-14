/**
 * \file qoobarglobals.h
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

#ifndef QOOBARGLOBALS_H
#define QOOBARGLOBALS_H

#include <QStringList>

namespace Qoobar {

QString formatLength(int i);
QString tagByID(int id);
QString untranslatedTagByID(int id);
bool programInstalled(const QString &program, QString *path);
QStringList musicalFilters();
void processDir(const QString &file, QStringList &files, bool includeSubfolders);
QStringList expandFilesNames(const QStringList &filesNames, bool includeSubfolders);

void showHelp(const QString &section = QString());

/**
 * @brief moveFolderInto - recursively moves all files and folders from
 * oldPath into newPath, deleting oldPath if it is empty
 * @param oldPath - full path to old folder
 * @param newPath - full path to new folder
 */
//void moveFolderInto(const QString &oldPath, const QString &newPath, QHash<QString, QString> &renamedFiles);

}

namespace Case {
enum Case {
    AllSmall=0,
    AllCaps=1,
    FirstCaps=2,
    EveryFirstCaps=3,
    EveryFirstCapsPreserving=4,
    DontChange=5
};

Case caseByValue(const QString &value);
QString caseById(const Case id);
}

#ifndef QT_NO_DEBUG
class QTime;
class Trace {
public:
    Trace(const QString &msg);
    ~Trace();
private:
    QString _msg;
    QTime *time;
};

#define DD  Trace trace(Q_FUNC_INFO);
#else
#define DD
#endif



#endif // QOOBARGLOBALS_H

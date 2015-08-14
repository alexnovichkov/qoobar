/* This file is part of the KDE project
   Copyright 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>
   Copyright 2007 Oswald Buddenhagen <ossi@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PROCESSLINEMAKER_H
#define PROCESSLINEMAKER_H

#include <QObject>

class QProcess;
class QStringList;

class ProcessLineMaker : public QObject
{
    Q_OBJECT
public:
    explicit ProcessLineMaker(QObject *parent = 0);
    explicit ProcessLineMaker(QProcess *process, QObject *parent = 0);
    void discardBuffers();
    void flushBuffers();
Q_SIGNALS:
    void receivedStdoutLines(const QStringList &lines);
    void receivedStderrLines(const QStringList &lines);
//public Q_SLOTS:
//    void slotReceivedStdout(const QByteArray &buffer);
//    void slotReceivedStderr(const QByteArray &buffer);
private Q_SLOTS:
    void slotReadyReadStdout();
    void slotReadyReadStderr();
private:
    void processStdOut();
    void processStdErr();
    QProcess* m_proc;
    QByteArray stdoutbuf;
    QByteArray stderrbuf;
};

#endif // PROCESSLINEMAKER_H

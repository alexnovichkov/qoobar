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

#include "processlinemaker.h"

#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include "qoobarglobals.h"

ProcessLineMaker::ProcessLineMaker(QObject *parent) :
    QObject(parent), m_proc(nullptr)
{DD;

}

ProcessLineMaker::ProcessLineMaker(QProcess *process, QObject *parent)
    : QObject(parent), m_proc(process)
{DD;
    if (m_proc) {
        m_proc->setTextModeEnabled(true);
        connect(m_proc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReadyReadStdout()));
        connect(m_proc, SIGNAL(readyReadStandardError()),
            this, SLOT(slotReadyReadStderr()));
    }
}

void ProcessLineMaker::slotReadyReadStdout()
{DD;
    if (m_proc) {
        stdoutbuf += m_proc->readAllStandardOutput();
        processStdOut();
    }
}

void ProcessLineMaker::slotReadyReadStderr()
{DD;
    if (m_proc) {
        stderrbuf += m_proc->readAllStandardError();
        processStdErr();
    }
}

void ProcessLineMaker::processStdOut()
{DD;
    QStringList lineList;
    int pos;
    while ((pos = stdoutbuf.indexOf('\n')) != -1) {
        if (pos > 0 && stdoutbuf.at(pos - 1) == '\r')
            lineList << QString::fromLocal8Bit(stdoutbuf, pos - 1);
        else
            lineList << QString::fromLocal8Bit(stdoutbuf, pos);
        stdoutbuf.remove(0, pos+1);
    }
    Q_EMIT receivedStdoutLines(lineList);
}

void ProcessLineMaker::processStdErr()
{DD;
    QStringList lineList;
    int pos;
    while ((pos = stderrbuf.indexOf('\n')) != -1) {
        if (pos > 0 && stderrbuf.at(pos - 1) == '\r')
            lineList << QString::fromLocal8Bit(stderrbuf, pos - 1);
        else
            lineList << QString::fromLocal8Bit(stderrbuf, pos);
        stderrbuf.remove(0, pos+1);
    }
    Q_EMIT receivedStderrLines(lineList);
}

void ProcessLineMaker::discardBuffers()
{DD;
    stderrbuf.clear();
    stdoutbuf.clear();
}

void ProcessLineMaker::flushBuffers()
{DD;
    if (!stdoutbuf.isEmpty())
        Q_EMIT receivedStdoutLines(QStringList(QString::fromLocal8Bit(stdoutbuf)));
    if (!stderrbuf.isEmpty())
        Q_EMIT receivedStderrLines(QStringList(QString::fromLocal8Bit(stderrbuf)));
    discardBuffers();
}


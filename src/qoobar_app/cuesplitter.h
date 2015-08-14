/**
 * \file cuesplitter.h
 * Splitter of audio files by cue file
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date Apr 15 2013
 *
 * Copyright (C) 2013  Alex Novichkov
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

#ifndef CUESPLITTER_H
#define CUESPLITTER_H

#include <QObject>
#include <QStringList>

class QProcess;
#include <QDateTime>


class CueSplitter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString outputDir READ outputDir WRITE setOutputDir NOTIFY outputDirChanged)
    Q_PROPERTY(QString cueFile READ cueFile WRITE setCueFile NOTIFY cueFileChanged)
    Q_PROPERTY(QString inputFile READ inputFile WRITE setInputFile NOTIFY inputFileChanged)
    Q_PROPERTY(QStringList files READ files)
    Q_PROPERTY(int trackCount READ trackCount)
    Q_PROPERTY(bool notLatin1 READ notLatin1)

public:
    static QStringList formats();

    explicit CueSplitter(QObject *parent = 0);
    ~CueSplitter();

    bool shntoolInstalled;
    bool flacInstalled;
    bool macInstalled;
    bool ffmpegInstalled;
    void check();

    QString outputDir() const;
    QString cueFile() const;
    QString inputFile() const;
    QStringList files() const;
    int trackCount() const;
    bool notLatin1() const;

    void setOutputDir(const QString &outputDir);
    void setCueFile(const QString &cueFile); // sets cue file and searches for inputFile in the same directory
    void setInputFile(const QString &inputFile);
    void setFormat(const QString &format); // sets format of splitted files

Q_SIGNALS:
    void message(int type, const QString &text); // warning and error messages
    void textReady(const QString &text); // shntool output data
    void next(); // next file created
    void outputDirChanged(const QString &);
    void cueFileChanged(const QString &);
    void inputFileChanged(const QString &);
public Q_SLOTS:
    void stop();
    void split();
private Q_SLOTS:
    void updateText();
    void onOutputDirChanged(const QString &);
private:
    void parseCue();
    void findInputFile();
    void findTrackCount();
    QProcess *process;
    QString _outputDir;
    QString _cueFile;
    QString _inputFile;
    int _trackCount;

    QStringList _files;
    QString formatExt;

    QStringList _cueFileContents;
    bool _notLatin1;

    QDateTime time;
};

#endif // CUESPLITTER_H

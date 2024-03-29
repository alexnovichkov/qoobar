/**
 * \file cuesplitter.cpp
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

#include "cuesplitter.h"

#include "qoobarglobals.h"
#include "enums.h"
#include "application.h"
#include "tagger.h"
#include "tagsreaderwriter.h"
#include "platformprocess.h"
#include "stringroutines.h"

#include <QFileInfo>
#include <QTextStream>
#include <QFileSystemWatcher>
#include <QProcessEnvironment>
#include <QProcess>
#include <QTextCodec>
#include <QDir>
#include <QtDebug>
#include <QRegularExpression>

QStringList CueSplitter::formats()
{DD;
    return {"flac","alac","m4a","ogg","mp3","wma","wv","spx","opus"};
}

CueSplitter::CueSplitter(QObject *parent) :
    QObject(parent), process(nullptr), _trackCount(0), _notLatin1(false)
{DD;
    shntoolInstalled = Qoobar::programInstalled(QSL("shntool"), nullptr);
    flacInstalled = Qoobar::programInstalled(QSL("flac"), nullptr);
    macInstalled = Qoobar::programInstalled(QSL("mac"), nullptr);
    ffmpegInstalled = Qoobar::programInstalled(QSL("ffmpeg"), nullptr) || Qoobar::programInstalled(QSL("avconv"), nullptr);
}

CueSplitter::~CueSplitter()
{DD;
    if (process) {
        process->kill();
        process->deleteLater();
    }
}

void CueSplitter::check()
{DD;
    if (!shntoolInstalled)
        Q_EMIT message(MT_ERROR, tr("Please install shntool."));
    if (!ffmpegInstalled) {
        if (!flacInstalled) Q_EMIT message(MT_WARNING, tr("Please install flac."));
    }
}

QString CueSplitter::outputDir() const
{DD;
    return _outputDir;
}

QString CueSplitter::cueFile() const
{DD;
    return _cueFile;
}

QString CueSplitter::inputFile() const
{DD;
    return _inputFile;
}

QStringList CueSplitter::files() const
{DD;
    return _files;
}

int CueSplitter::trackCount() const
{DD;
    return _trackCount;
}

bool CueSplitter::notLatin1() const
{DD;
    return _notLatin1;
}

void CueSplitter::setOutputDir(const QString &outputDir)
{DD;
    if (_outputDir == outputDir) return;
    _outputDir = outputDir;
    Q_EMIT outputDirChanged(_outputDir);
}

void CueSplitter::setCueFile(const QString &cueFile)
{DD;
    if (_cueFile == cueFile) return;
    Q_EMIT message(MT_INFORMATION, tr("Opening cue file: \"%1\"").arg(cueFile));
    _cueFile = cueFile;
    App->lastCueFile = cueFile;
    Q_EMIT cueFileChanged(_cueFile);
    setOutputDir(QFileInfo(cueFile).canonicalPath());

    // read cue file
    _cueFileContents.clear();
    QFile file(_cueFile);


    //first iteration - checking for non-Latin1 chars
    if (!file.open(QFile::ReadOnly)) {
        Q_EMIT message(MT_ERROR, tr("Cannot open cue file %1").arg(_cueFile));
        return;
    }
    QDataStream ind(&file);
    constexpr int blockSize = 1024;
    char data[blockSize];
    while(!ind.atEnd()) {
        int len = ind.readRawData(data, blockSize);
        for (int i=0; i<len; ++i) {
            if ((unsigned char)data[i] > 127) {
                _notLatin1 = true;
                goto notLatin;
            }
        }
    }
notLatin:
    file.close();

    // second iteration - actual reading of file using codec
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        Q_EMIT message(MT_ERROR, tr("Cannot open cue file %1").arg(_cueFile));
        return;
    }
    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QTextCodec *codec = QTextCodec::codecForLocale();
    if (App->cueEncoding != QSL("Locale"))
        codec = QTextCodec::codecForName(App->cueEncoding.toLatin1());
    if (codec)
        in.setCodec(codec);
#else
    //TODO: Add the full ICU encodings support
    if (App->cueEncoding == QSL("Locale"))
        in.setEncoding(QStringConverter::System);
#endif
    in.setAutoDetectUnicode(true);

    while(!in.atEnd()) {
        _cueFileContents << in.readLine();
    }

    findTrackCount();

    findInputFile();
}

void CueSplitter::setInputFile(const QString &inputFile)
{DD;
    if (_inputFile == inputFile) return;
    _inputFile = inputFile;
    Q_EMIT inputFileChanged(_inputFile);
}

void CueSplitter::setFormat(const QString &format)
{DD;
    formatExt = format;
}

void CueSplitter::stop()
{DD;
    if (process) process->kill();
}

void CueSplitter::split()
{DD;
    if (_inputFile.isEmpty()) {
        Q_EMIT message(MT_ERROR, tr("Nothing to split."));
        return; //nothing to split
    }

    if (formatExt.isEmpty()) formatExt = QSL("flac");

    //check the input file type and show warnings if needed
    QString inputFileExt = QFileInfo(_inputFile).suffix();
    if (inputFileExt.toLower() != QSL("wav") && !ffmpegInstalled) {
        if (inputFileExt.toLower()=="ape" && !macInstalled) {
            QString errorMessage;
#ifdef Q_OS_WIN
            errorMessage = tr("Make sure you have mac.exe "
                              "copied into the folder with qoobar.exe.<br>"
                              "You can download it from %1").arg(
                              " <a href=\"http://shnutils.freeshell.org/shntool/\">"
                              "http://shnutils.freeshell.org/shntool/</a>");
#elif defined(Q_OS_LINUX)
            errorMessage = tr("Please install ffmpeg.");
#else
            errorMessage = tr("Please install mac");
#endif
            Q_EMIT message(MT_ERROR, errorMessage);
            return;
        }
    }
    Q_EMIT message(MT_INFORMATION, tr("Please wait while Qoobar is splitting the file %1").arg(_inputFile));

    QDateTime dt=QDateTime::currentDateTime();

    if (!process) {
        process = new PlatformProcess(this);
        process->disconnect();
    }

    QString programToRun;
    QStringList arguments;

#if defined(Q_OS_LINUX) || defined (Q_OS_MACOS)
    QString pathToScript = qApp->applicationDirPath()+"/splitandconvert.sh";
    if (!QFile::exists(pathToScript)) {
        Q_EMIT message(MT_ERROR, tr("Cannot find the necessary script file: ").append(pathToScript));
        return;
    }
    arguments << QDir::toNativeSeparators(pathToScript);
//    arguments << QDir::toNativeSeparators(ApplicationPaths::sharedPath()+"/splitandconvert.sh");
    arguments << QDir::toNativeSeparators(_cueFile);
    arguments << QDir::toNativeSeparators(_outputDir);
    arguments << QDir::toNativeSeparators(_inputFile);
    arguments << formatExt;
    arguments << (ffmpegInstalled?"1":"0");
    programToRun = QSL("sh");
#elif defined(Q_OS_WIN)
    arguments << "/C";

    QString powershellPath;
    bool powerShell = Qoobar::programInstalled("powershell", &powershellPath);

    if (powerShell)
        programToRun = QDir::toNativeSeparators(powershellPath);
    else
        programToRun = QSL("c:\\windows\\system32\\cmd.exe");

    // We need to use short file names in Windows
    // because shntool cannot split files not in locale encoding

    QString cFile = getShortFileName(_cueFile);
    QString oDir = getShortFileName(_outputDir);
    QString iFile = getShortFileName(_inputFile);

    QString pathToScript = App->applicationDirPath()+"/splitandconvert.bat";
    if (!QFile::exists(pathToScript)) {
        Q_EMIT message(MT_ERROR, tr("Cannot find the necessary script file: ").append(pathToScript));
        return;
    }
    QString scriptFile = QDir::toNativeSeparators(App->applicationDirPath()+"/splitandconvert.bat");
    QString sFile = getShortFileName(scriptFile);
    arguments << (sFile.isEmpty()?scriptFile:sFile);

    //cmd.exe cannot handle &s at all, so escaping them
    cFile.replace("&","^^^&");
    oDir.replace("&","^^^&");
    iFile.replace("&","^^^&");

    if (programToRun != QSL("c:\\windows\\system32\\cmd.exe")) {
        arguments << "\""+QDir::toNativeSeparators(cFile.isEmpty()?_cueFile:cFile)+"\"";
        arguments << "\""+QDir::toNativeSeparators(oDir.isEmpty()?_outputDir:oDir)+"\"";
        arguments << "\""+QDir::toNativeSeparators(iFile.isEmpty()?_inputFile:iFile)+"\"";
    }
    else {
        arguments << QDir::toNativeSeparators(cFile.isEmpty()?_cueFile:cFile);
        arguments << QDir::toNativeSeparators(oDir.isEmpty()?_outputDir:oDir);
        arguments << QDir::toNativeSeparators(iFile.isEmpty()?_inputFile:iFile);
    }
    arguments << formatExt;
    arguments << (ffmpegInstalled?"1":"0");

#elif defined (Q_OS_OS2)
    /** TODO: 1. Find out how to invoke splitandconvert.cmd on OS/2
              2. Find out what commands are allowed on OS/2
              3. Rewrite splitandconvert.cmd maybe using REXX
              4. Replace the following stub with splitandconvert.cmd
    */

    arguments=QString("split.-P.dot.-f.cueFile.-d.outputDir.-O.always.-o.flac.file").split(QSL("."));
    arguments[11]=_inputFile;
    arguments[4]=_cueFile;
    arguments[6]=_outputDir;
    programToRun = QSL("shntool");
#else
    arguments=QString("split.-P.dot.-f.cueFile.-d.outputDir.-O.always.-o.flac.file").split(QSL("."));
    arguments[11]=_inputFile;
    arguments[4]=_cueFile;
    arguments[6]=_outputDir;
    arguments << (ffmpegInstalled?"1":"0");
    programToRun = QSL("shntool");
#endif

    if (formatExt == QSL("alac")) formatExt = QSL("m4a");

    QEventLoop q;
    connect(process,SIGNAL(finished(int)),&q,SLOT(quit()));
    connect(process,SIGNAL(error(QProcess::ProcessError)),&q,SLOT(quit()));
    connect(process,SIGNAL(readyReadStandardOutput()),SLOT(updateText()),Qt::QueuedConnection);
    connect(process,SIGNAL(readyRead()),SLOT(updateText()),Qt::QueuedConnection);

    time = QDateTime::currentDateTime();
    QFileSystemWatcher watcher(this);
    watcher.addPath(_outputDir);
    connect(&watcher,SIGNAL(directoryChanged(QString)),SLOT(onOutputDirChanged(QString)),Qt::QueuedConnection);

    Q_EMIT message(MT_INFORMATION, tr("Now invoking the script file with arguments:\n")
                   .append(programToRun + " "+arguments.join(" ")));
    process->start(programToRun, arguments);
    q.exec();

    int code=process->exitCode();
    if (code==0) {
        QFileInfoList newFiles = QDir(_outputDir).entryInfoList(QStringList()<<QString("*.%1").arg(formatExt),
                                                                QDir::Files);

        Q_FOREACH(const QFileInfo &newFile,newFiles) {
            if (newFile.BIRTH_TIME() > dt || newFile.lastModified() > dt)
                _files << newFile.canonicalFilePath();
        }
    }


    if (!_files.isEmpty()) {
        if (_files.at(0).contains(QRegularExpression(QSL("00\\b")))) {
            //remove pregap file
            QFile::remove(_files.at(0));
            _files.removeFirst();
        }
    }

    if (!_files.isEmpty()) {
        parseCue();
    }
}

void CueSplitter::parseCue()
{DD;
    if (_cueFileContents.isEmpty()) return; //nothing to parse
    QList<Tag> result;

    int currentTrack=-1; //top level corresponds to the whole disk
    QString albumArtist;
    QString comment;
    QString album;
    QString genre;
    QString date;

    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<_cueFileContents.count(); ++i) {
        QString s=_cueFileContents.at(i).simplified();
        if (s.contains(QRegularExpression(QSL("\\b(INDEX|FILE|PREGAP|POSTGAP|CDTEXTFILE|FLAGS)\\b"))))
            continue;
        if (s.startsWith(QLS("TRACK"))) {
            currentTrack+=1;
            Tag item(QLS(""), tagsCount);
            item.setTag(TRACKNUMBER,QString::number(currentTrack+1));
            item.setTag(ALBUM,album);
            item.setTag(ALBUMARTIST,albumArtist);
            item.setTag(COMMENT,comment);
            item.setTag(YEAR,date);
            item.setTag(GENRE,genre);
            result.append(item);
        }
        else {
            QString ss=s.section(QSL(" "),1);
            if (ss.startsWith(QLS("\""))) ss.remove(0,1);
            if (ss.endsWith(QLS("\""))) ss.chop(1);
            if (s.startsWith(QLS("TITLE"))) {
                if (currentTrack<0) album=ss;
                else {
                    result.last().setTag(TITLE,ss);
                }
            }
            else if (s.startsWith(QLS("PERFORMER"))) {
                if (currentTrack<0) albumArtist=ss;
                else {
                    result.last().setTag(ARTIST,ss);
                }
            }
            else if (s.startsWith(QLS("REM"))) {
                s=s.mid(4);
                ss=s.section(QSL(" "),1);
                if (ss.startsWith(QLS("\""))) ss.remove(0,1);
                if (ss.endsWith(QLS("\""))) ss.chop(1);
                if (s.startsWith(QLS("GENRE"))) {
                    if (currentTrack<0) genre=ss;
                    else {
                        result.last().setTag(GENRE,ss);
                    }
                }
                else if (s.startsWith(QLS("DATE"))) {
                    if (currentTrack<0) date=ss;
                    else {
                        result.last().setTag(YEAR,ss);
                    }
                }
                else if (s.startsWith(QLS("COMMENT"))) {
                    if (currentTrack<0) comment=ss;
                    else {
                        result.last().setTag(COMMENT,comment);
                    }
                }
            }
        }
    }
    //if all artists is empty, move albumArtist there
    bool isEmpty=true;
    for (const Tag &tag: result) {
        if (!tag.tag(ARTIST).isEmpty()) {
            isEmpty = false;
            break;
        }
    }

    if (isEmpty) {
        for (auto &tag: result) {
            tag.setTag(ARTIST, tag.tag(ALBUMARTIST));
            tag.setTag(ALBUMARTIST, QString());
        }
    }
    for (int i=0; i<result.size() && i<_files.size(); ++i) {
        result[i].setFile(_files.at(i));
        if (!result[i].wasChanged()) continue;
        TagsReaderWriter t(&result[i]);
        t.writeTags();
    }
}

void CueSplitter::findInputFile()
{DD;
    _inputFile.clear();

    QString inputFile;
    QRegularExpression r(QSL("FILE\\s+\"?([^\"]+)\"?"));
    QRegularExpressionMatch match;
    Q_FOREACH(const QString &s, _cueFileContents) {
        if (s.indexOf(r,0,&match)>-1) {
            inputFile=match.captured(1).trimmed();
            break;
        }
    }

    QFileInfo finfo = QFileInfo(inputFile);
    if (finfo.isRelative())
        inputFile = QString("%1/%2").arg(_outputDir, inputFile);

    if (inputFile.isEmpty() || !finfo.exists()) {
        //try to find file with the same name and another extension
        QStringList l = inputFile.split(".");

        l.replace(l.size()-1,"flac");
        if (!QFileInfo::exists(l.join("."))) {
            l.replace(l.size()-1,"ape");
            if (!QFileInfo::exists(l.join("."))) {
                l.replace(l.size()-1,"wav");
                if (!QFileInfo::exists(l.join("."))) {
                    Q_EMIT message(MT_ERROR, tr("Cannot find the file to split: %1").arg(inputFile));
                    return;
                }
            }
        }
        setInputFile(l.join("."));
    }
    else setInputFile(inputFile);
}

void CueSplitter::findTrackCount()
{DD;
    _trackCount=0;
    for (int i=_cueFileContents.size()-1; i>=0; --i) {
        QString s = _cueFileContents.at(i).simplified();
        if (s.startsWith(QLS("TRACK "))) {
            _trackCount = s.section(QSL(" "),1,1).toInt();
            break;
        }
    }
    Q_EMIT message(MT_INFORMATION, tr("Total track count: \"%1\"").arg(_trackCount));
}

void CueSplitter::updateText()
{DD;
    QByteArray b=process->readAll();

#ifdef Q_OS_WIN
    QString toRecode = QString::fromLocal8Bit(b.data(),b.size());
    QString text = processFunction("recode",QStringList()<<toRecode,0);
#else
    QString text = QString::fromUtf8(b.data(),b.size());
#endif
    Q_EMIT textReady(text);
}

void CueSplitter::onOutputDirChanged(const QString &path)
{DD;
    QFileInfoList newFiles = QDir(path).entryInfoList(QStringList()<<QString("*.%1").arg(formatExt),QDir::Files);
    Q_FOREACH(const QFileInfo &newFile,newFiles) {
        if (newFile.BIRTH_TIME()>time) {
#ifdef Q_OS_WIN
            Q_EMIT textReady(tr("Processing %1 ...\n\n").arg(newFile.canonicalFilePath()));
#endif
            Q_EMIT next();
        }
    }
    time = QDateTime::currentDateTime();
}

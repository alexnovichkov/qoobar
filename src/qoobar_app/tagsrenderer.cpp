/**
 * \file tagrender.cpp
 * Renders tags from placeholders.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 31 Dec 2010
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

#include "tagsrenderer.h"
#include "tagger.h"
#include "legendbutton.h"
#include "application.h"
#include <QFileInfo>

#include "enums.h"
#include "qoobarglobals.h"
#include "stringroutines.h"
#include "placeholders.h"
#include "model.h"

#include <QtDebug>

void fillPlaceholders(QString &currentPattern, Tag &tag, QStringList &args, bool remove)
{DD

    for (int i=0; i<args.size(); ++i)
        currentPattern = currentPattern.arg(args.at(i));

    //args.clear();
    const QStringList userTags = tag.userTagsKeys();

    if (currentPattern.contains(QSL("%"))) {
        int pos = 0;
        while ((pos = currentPattern.indexOf('%', pos)) != -1) {
            if (pos == currentPattern.length() - 1) break;
            QString cap = currentPattern.mid(pos+1,1);
            int index = Placeholders::placeholderToId(cap);
            if (index>=0) {
                QString tagValue=tag.nonEmptyTag(index, remove?QSL(""):QSL("_"));
                if (cap=="N" && tagValue!="_" && !tagValue.isEmpty())
                    tagValue=tagValue.rightJustified(2,QChar('0'));
                currentPattern.replace(pos,2,tagValue);
                pos += tagValue.length();
            }
            else pos += 2;
        }

        currentPattern.replace(QLS("%l"),Qoobar::formatLength(tag.length()));
        currentPattern.replace(QLS("%L"),QString::number(tag.length()));
        currentPattern.replace(QLS("%b"),tag.bitrate());
        currentPattern.replace(QLS("%f"),tag.fileName());
        currentPattern.replace(QLS("%r"),QString::number(tag.sampleRate()));
        currentPattern.replace(QLS("%h"),QString::number(tag.channels()));
        currentPattern.replace(QLS("%F"),tag.fileNameExt());
        currentPattern.replace(QLS("%z"),tag.fullFileName());
        currentPattern.replace(QLS("%Z"),tag.filePath());
        currentPattern.replace(QLS("%E"),tag.fileExt());
    }

    if (currentPattern.contains(QSL("<"))) {
        int pos1 = 0;
        while ((pos1 = currentPattern.indexOf('<', pos1)) != -1) {
            if (pos1 == currentPattern.length() - 1) break; //last symbol

            int pos2 = currentPattern.indexOf('>',pos1);
            if (pos2 != -1) {
                QString cap = currentPattern.mid(pos1+1, pos2-pos1-1);
                QString cap1 = cap.toLower().remove(QSL(" "));
                int index = App->currentScheme->tagIDBySimplifiedName(cap);
                if (index>=0) {
                    QString tagValue=tag.nonEmptyTag(index, remove?QSL(""):QSL("_"));
                    currentPattern.replace(pos1,pos2-pos1+1,tagValue);
                    pos1 += tagValue.length();
                }
                else if (userTags.contains(cap)) {
                    QString tagValue=tag.userTag(cap);
                    if (tagValue.isEmpty() && !remove) tagValue = QSL("_");
                    currentPattern.replace(pos1,pos2-pos1+1,tagValue);
                    pos1 += tagValue.length();
                }
                else if (cap1 == QSL("bitrate")) {
                    QString val = tag.bitrate();
                    currentPattern.replace(pos1,pos2-pos1+1, val);
                    pos1 += val.length();
                }
                else if (cap1 == QSL("length")) {
                    QString val = Qoobar::formatLength(tag.length());
                    currentPattern.replace(pos1,pos2-pos1+1, val);
                    pos1 += val.length();
                }
                else if (cap1 == QSL("samplerate")) {
                    QString val = QString::number(tag.sampleRate());
                    currentPattern.replace(pos1,pos2-pos1+1, val);
                    pos1 += val.length();
                }
                else if (cap1 == QSL("channels")) {
                    QString val = QString::number(tag.channels());
                    currentPattern.replace(pos1,pos2-pos1+1, val);
                    pos1 += val.length();
                }
                else if (cap1 == QSL("filename")) {
                    QString val = tag.fileName();
                    currentPattern.replace(pos1,pos2-pos1+1, val);
                    pos1 += val.length();
                }
                else if (cap1 == QSL("filepath")) {
                    QString val = tag.filePath();
                    currentPattern.replace(pos1,pos2-pos1+1, val);
                    pos1 += val.length();
                }
                else if (cap1 == QSL("fileextension")) {
                    QString val = tag.fileExt();
                    currentPattern.replace(pos1,pos2-pos1+1, val);
                    pos1 += val.length();
                }

                else pos1 = pos2+1;
            }
            else pos1++;
        }
    }
}

void processCurly(QStringList &args, QString &pattern, int index, int size)
{DD
    static QRegExp startNumRx(QSL("(\\d+)(?:\\s*-\\s*)*(\\d*)"));
    static QRegExp repeatRx(QSL("(\\d+)(?:\\s*:\\s*)(\\d+)"));

    int beginSec = pattern.indexOf("{",0);
    int endSec = 0;

    while (beginSec>=0) {
        QVector<int> wholeList;
        endSec = pattern.indexOf(QLS("}"),beginSec);
        if (endSec<0) endSec = pattern.length()-1;

        QString wholeSection = pattern.mid(beginSec+1, endSec-beginSec-1);
        if (wholeSection.isEmpty()) wholeSection=QSL("1");


        /**       {1-3,  1:3, 3:2; 2-3, 2:4, 3:3}
result: 1 1 1 2 3 3 2 2 2 2 3 3 3
*/
        QStringList sections = wholeSection.split(QSL(";"),QString::SkipEmptyParts);

        Q_FOREACH (const QString &sec, sections) {
            // 1-3,  1:3, 3:2
            QVector<int> list;

            int pos=0;
            pos = startNumRx.indexIn(sec, pos);
            if (pos!=-1) {
                const int startNum = startNumRx.cap(1).toInt(); //1
                int endNum = startNumRx.cap(2).toInt(); //3
                if (endNum==0) endNum=size+startNum-1;

                int repeatPos = pos + startNumRx.matchedLength();
                while ((repeatPos = repeatRx.indexIn(sec, repeatPos)) != -1) {
                    int whatToRepeat=repeatRx.cap(1).toInt();
                    if (whatToRepeat>0) {
                        int repeats=repeatRx.cap(2).toInt();
                        if (repeats>1) {
                            for (int i=0; i<repeats-1; ++i) list << whatToRepeat;
                        }
                        repeatPos += repeatRx.matchedLength();
                    }
                }
                for (int i=startNum; i<=endNum; ++i) list << i;
            }
            else
                for (int i=1; i<1+size; ++i) list << i;
            qSort(list);
            wholeList += list;
            if (index < wholeList.size()) {
                args << QString::number(wholeList.at(index));

                pattern.replace(beginSec, endSec - beginSec + 1, "%"+QString::number(args.size()));
                break;
            }
        }

        if (index >= wholeList.size()) {
            pattern.remove(beginSec, endSec - beginSec + 1);
            beginSec = pattern.indexOf(QLS("{"),beginSec);
        }
        else
            beginSec = pattern.indexOf(QLS("{"),beginSec+1);
    }
}

/**
the function removes empty [] sections
*/
void removeEmptySections(Tag &tag, QString &pattern)
{DD;
    const int tagsCount = App->currentScheme->tagsCount();
    while (pattern.contains(QSL("["))) {
        int begin = pattern.indexOf(QLS("["),0);
        int end = pattern.indexOf(QLS("]"),begin);
        if (end < begin)
            end = pattern.length()-1;
        QString sec = pattern.mid(begin+1,end-begin-1);

        for (int i=0; i<tagsCount; ++i) {
            if (tag.tag(i).isEmpty()) {
                if (i<TAGSCOUNT) sec.remove(QSL("%") + Placeholders::idToPlaceholder(i));
                sec.remove(QSL("<")+App->currentScheme->fieldName(i).toLower().remove(QSL(" "))+QSL(">"));
                sec.remove(QSL("<")+App->currentScheme->fieldName(i).toLower()+QSL(">"));
                sec.remove(QSL("<")+App->currentScheme->fieldName(i)+QSL(">"));

                sec.remove(QSL("<")+App->currentScheme->localizedFieldName[i].toLower().remove(QSL(" "))+QSL(">"));
                sec.remove(QSL("<")+App->currentScheme->localizedFieldName[i].toLower()+QSL(">"));
                sec.remove(QSL("<")+App->currentScheme->localizedFieldName[i]+QSL(">"));
            }
        }
        const QMap<QString, QString> &userTags = tag.userTags();
        QMapIterator<QString, QString> it(userTags);
        while (it.hasNext()) {
            it.next();
            if (it.value().isEmpty()) {
                sec.remove(QSL("<")+it.key()+QSL(">"));
            }
        }

        if (tag.tag(TRACKNUMBER).isEmpty()) sec.remove(QSL("%N"));
        if (tag.bitrate().isEmpty()) {
            sec.remove(QSL("%b"));
            QRegExp re(QSL("<\\s*bit\\s*rate\\s*>")); re.setMinimal(true); re.setCaseSensitivity(Qt::CaseInsensitive);
            sec.remove(re);
        }
        if (tag.length() <= 0) {
            sec.remove(QSL("%l"));
            sec.remove(QSL("%L"));
            QRegExp re(QSL("<\\s*length\\s*>")); re.setMinimal(true); re.setCaseSensitivity(Qt::CaseInsensitive);
            sec.remove(re);
        }
        if (tag.fileName().isEmpty()) {
            sec.remove(QSL("%f"));
            QRegExp re(QSL("<\\s*file\\s*name\\s*>")); re.setMinimal(true); re.setCaseSensitivity(Qt::CaseInsensitive);
            sec.remove(re);
        }
        if (tag.fileNameExt().isEmpty()) sec.remove(QSL("%F"));
        if (tag.fileExt().isEmpty()) {
            sec.remove(QSL("%E"));
            QRegExp re(QSL("<\\s*file\\s*extension\\s*>")); re.setMinimal(true); re.setCaseSensitivity(Qt::CaseInsensitive);
            sec.remove(re);
        }
        if (tag.fullFileName().isEmpty()) sec.remove(QSL("%z"));
        if (tag.filePath().isEmpty()) {
            sec.remove(QSL("%Z"));
            QRegExp re(QSL("<\\s*file\\s*path\\s*>")); re.setMinimal(true); re.setCaseSensitivity(Qt::CaseInsensitive);
            sec.remove(re);
        }
        if (tag.sampleRate() <= 0) {
            sec.remove(QSL("%r"));
            QRegExp re(QSL("<\\s*sample\\s*rate\\s*>")); re.setMinimal(true); re.setCaseSensitivity(Qt::CaseInsensitive);
            sec.remove(re);
        }
        if (tag.channels() <= 0) {
            sec.remove(QSL("%h"));
            QRegExp re(QSL("<\\s*channels\\s*>")); re.setMinimal(true); re.setCaseSensitivity(Qt::CaseInsensitive);
            sec.remove(re);
        }
        if (!sec.contains(QSL("%")) && !sec.contains(QSL("<")))
            sec.clear();
        pattern.replace(begin,end-begin+1,sec);
    }
}

QString getFunctionArg(const int pos, int &capturedLength, const QString &pattern)
{DD
    QString functionArg;

    int parenthesesCount = 1;
    int curPos = pos + capturedLength+1;
    while (curPos < pattern.length() && parenthesesCount > 0) {
        QChar currentChar = pattern[curPos];

        if (currentChar==')') parenthesesCount--;
        else if (currentChar=='(') parenthesesCount++;

        if (parenthesesCount>0)
            functionArg.append(currentChar);
        curPos++;
        capturedLength++;
    }
    capturedLength++;

    return functionArg;
}

void processFunctions(QStringList &args, QString &pattern, Tag &tag, int index, int size)
{DD
    static QRegExp re(QSL("\\$([a-zA-Z_0-9]+)(?=\\()"));

    re.setMinimal(true);
    int pos = 0;

    QStringList args1=args;

    while ((pos = re.indexIn(pattern, pos)) != -1) {
        int capturedLength = re.matchedLength();
        const QString functionName=re.cap(1);
        QString functionArg = getFunctionArg(pos, capturedLength, pattern);
        functionArg.replace(',',QChar(0x25a0));

        if (functionArg.contains("$"))
            processFunctions(args1, functionArg, tag, index, size);

        fillPlaceholders(functionArg, tag, args1, true);
        QStringList functionArgs=functionArg.split(QChar(0x25a0),QString::SkipEmptyParts);

        if (functionArgs.size()==1 && functionArgs.first().simplified()==QSL(""))
            functionArgs.clear();

        QString s = processFunction(functionName, functionArgs, index);

        if (!s.isEmpty()) {
            args << s;
            QString replaceBy = "%"+QString::number(args.size());
            pattern.replace(pos,capturedLength,replaceBy);
            pos += replaceBy.length()+1;
        }
        else pattern.remove(pos,capturedLength);
    }
}

TagsRenderer::TagsRenderer(QObject *parent) : QObject(parent), m(0)
{DD
    Model *mm = qobject_cast<Model *>(parent);
    if (mm) m=mm;
}

TagsRenderer::TagsRenderer(Model *model, QObject *parent) : QObject(parent), m(model)
{DD
}

bool TagsRenderer::setPattern(const QString &pattern)
{DD
    if (m_pattern==pattern) return false;
    m_pattern = pattern;
    Q_EMIT patternChanged(pattern);
    return true;
}

void TagsRenderer::updateTags()
{DD
    if (!m) return;
    const int size = m->selectedFilesCount();

    m_newTags.clear();
    preprocess();

    for (int index=0; index<size; ++index) {
        QStringList args;
        Tag tag = m->fileAtSelection(index);
        QString currentPattern=m_pattern;

        // 1. process {} functions, replace function by {0} or {1} etc.
        if (currentPattern.contains('{'))
            processCurly(args, currentPattern, index, size);

        // 2. process $ functions
        if (currentPattern.contains('$')) {
            processFunctions(args, currentPattern, tag, index, size);
        }

        if (currentPattern.contains('['))
            removeEmptySections(tag, currentPattern);

        if (currentPattern.contains('%') || currentPattern.contains('<'))
            fillPlaceholders(currentPattern, tag, args, false);

        m_newTags << currentPattern;
    }
    postprocess();
}

void TagsRenderer::preprocess()
{DD
    m_pattern.replace(QLS("\\["),QChar(0x2590));
    m_pattern.replace(QLS("\\]"),QChar(0x2591));
    m_pattern.replace(QLS("[["),QChar(0x2590));
    m_pattern.replace(QLS("]]"),QChar(0x2591));
    m_pattern.replace(QLS("\\{"),QChar(0x2592));
    m_pattern.replace(QLS("\\}"),QChar(0x2593));
    m_pattern.replace(QLS("{{"),QChar(0x2592));
    m_pattern.replace(QLS("}}"),QChar(0x2593));
    m_pattern.replace(QLS("\\$"),QChar(0x2594));
    m_pattern.replace(QLS("\\("),QChar(0x2595));
    m_pattern.replace(QLS("\\)"),QChar(0x2596));

    m_pattern.replace(QLS("\\<"),QChar(0x25a1));
    m_pattern.replace(QLS("\\>"),QChar(0x25a2));
}

void TagsRenderer::postprocess()
{DD
    m_newTags.replaceInStrings(QChar(0x2590),QSL("["));
    m_newTags.replaceInStrings(QChar(0x2591),QSL("]"));
    m_newTags.replaceInStrings(QChar(0x2592),QSL("{"));
    m_newTags.replaceInStrings(QChar(0x2593),QSL("}"));
    m_newTags.replaceInStrings(QChar(0x2594),QSL("$"));
    m_newTags.replaceInStrings(QChar(0x2595),QSL("("));
    m_newTags.replaceInStrings(QChar(0x2596),QSL(")"));

    m_newTags.replaceInStrings(QChar(0x25a1),QSL("<"));
    m_newTags.replaceInStrings(QChar(0x25a2),QSL(">"));
}




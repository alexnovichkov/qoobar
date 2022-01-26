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
#include <QRegularExpression>

void fillPlaceholders(QString &currentPattern, const Tag &tag, const QStringList &args, bool remove)
{DD;

    for (const QString &arg: args)
        currentPattern = currentPattern.arg(arg);

    //args.clear();
    const QStringList userTags = tag.userTagsKeys();

    const QString fillerString = remove?QSL(""):QSL("_");
    if (currentPattern.contains(QSL("%"))) {
        int pos = 0;
        while ((pos = currentPattern.indexOf('%', pos)) != -1) {
            if (pos == currentPattern.length() - 1) break;
            QString cap = currentPattern.mid(pos+1,1);
            int index = Placeholders::placeholderToId(cap);
            if (index>=0) {
                QString tagValue=tag.nonEmptyTag(index, fillerString);
                if (cap=="N" && tagValue!=fillerString && !tagValue.isEmpty())
                    tagValue = tagValue.rightJustified(2,QChar('0'));
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
                QString val;
                bool found = true;

                if (index>=0) val=tag.nonEmptyTag(index, fillerString);
                else if (userTags.contains(cap))    val = tag.userTag(cap);
                else if (cap1 == QSL("bitrate"))    val = tag.bitrate();
                else if (cap1 == QSL("length"))     val = Qoobar::formatLength(tag.length());
                else if (cap1 == QSL("samplerate")) val = QString::number(tag.sampleRate());
                else if (cap1 == QSL("channels"))   val = QString::number(tag.channels());
                else if (cap1 == QSL("filename"))   val = tag.fileName();
                else if (cap1 == QSL("filepath"))   val = tag.filePath();
                else if (cap1 == QSL("fileextension")) val = tag.fileExt();
                else found = false;
                if (found) {
                    if (val.isEmpty() && !remove) val = QSL("_");
                    currentPattern.replace(pos1, pos2-pos1+1, val);
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
    static QRegularExpression startNumRx(QSL("(\\d+)(?:\\s*-\\s*)*(\\d*)"));
    static QRegularExpression repeatRx(QSL("(\\d+)(?:\\s*:\\s*)(\\d+)"));

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
        const QStringList sections = wholeSection.split(QSL(";"),SKIP_EMPTY_PARTS);

        for (const QString &sec: sections) {
            // 1-3,  1:3, 3:2
            QVector<int> list;

            int pos=0;
            QRegularExpressionMatch match;
            pos = sec.indexOf(startNumRx, pos, &match);
            if (pos!=-1) {
                const int startNum = match.captured(1).toInt(); //1
                int endNum = match.captured(2).toInt(); //3
                if (endNum==0) endNum=size+startNum-1;

                int repeatPos = pos + match.capturedLength();
                while ((repeatPos = sec.indexOf(repeatRx, repeatPos, &match)) != -1) {
                    int whatToRepeat=match.captured(1).toInt();
                    if (whatToRepeat>0) {
                        int repeats=match.captured(2).toInt();
                        if (repeats>1) {
                            for (int i=0; i<repeats-1; ++i) list << whatToRepeat;
                        }
                        repeatPos += match.capturedLength();
                    }
                }
                for (int i=startNum; i<=endNum; ++i) list << i;
            }
            else
                for (int i=1; i<1+size; ++i) list << i;
            std::sort(list.begin(), list.end());
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
            QRegularExpression re(QSL("<\\s*bit\\s*rate\\s*>"), QRegularExpression::CaseInsensitiveOption);
            //re.setMinimal(true);
            sec.remove(re);
        }
        if (tag.length() <= 0) {
            sec.remove(QSL("%l"));
            sec.remove(QSL("%L"));
            QRegularExpression re(QSL("<\\s*length\\s*>"), QRegularExpression::CaseInsensitiveOption); //re.setMinimal(true);
            sec.remove(re);
        }
        if (tag.fileName().isEmpty()) {
            sec.remove(QSL("%f"));
            QRegularExpression re(QSL("<\\s*file\\s*name\\s*>"), QRegularExpression::CaseInsensitiveOption); //re.setMinimal(true);
            sec.remove(re);
        }
        if (tag.fileNameExt().isEmpty()) sec.remove(QSL("%F"));
        if (tag.fileExt().isEmpty()) {
            sec.remove(QSL("%E"));
            QRegularExpression re(QSL("<\\s*file\\s*extension\\s*>"), QRegularExpression::CaseInsensitiveOption); //re.setMinimal(true);
            sec.remove(re);
        }
        if (tag.fullFileName().isEmpty()) sec.remove(QSL("%z"));
        if (tag.filePath().isEmpty()) {
            sec.remove(QSL("%Z"));
            QRegularExpression re(QSL("<\\s*file\\s*path\\s*>"), QRegularExpression::CaseInsensitiveOption); //re.setMinimal(true);
            sec.remove(re);
        }
        if (tag.sampleRate() <= 0) {
            sec.remove(QSL("%r"));
            QRegularExpression re(QSL("<\\s*sample\\s*rate\\s*>"), QRegularExpression::CaseInsensitiveOption); //re.setMinimal(true);
            sec.remove(re);
        }
        if (tag.channels() <= 0) {
            sec.remove(QSL("%h"));
            QRegularExpression re(QSL("<\\s*channels\\s*>"), QRegularExpression::CaseInsensitiveOption); //re.setMinimal(true);
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
    static QRegularExpression re(QSL("\\$([a-zA-Z_0-9]+)(?=\\()"));

    //re.setMinimal(true);
    int pos = 0;

    QStringList args1=args;
    QRegularExpressionMatch match;

    while ((pos = pattern.indexOf(re, pos, &match)) != -1) {
        int capturedLength = match.capturedLength();
        const QString functionName=match.captured(1);
        QString functionArg = getFunctionArg(pos, capturedLength, pattern);
//        qDebug()<<functionArg;
        functionArg.replace("\\,",QChar(0x25da));
        functionArg.replace(',',QChar(0x25a0));
        functionArg.replace(QChar(0x25da),',');
//        qDebug()<<functionArg;

        if (functionArg.contains("$"))
            processFunctions(args1, functionArg, tag, index, size);

        fillPlaceholders(functionArg, tag, args1, true);
        QStringList functionArgs=functionArg.split(QChar(0x25a0),SKIP_EMPTY_PARTS);

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
//    qDebug()<<m_pattern;
    preprocess();
//    qDebug()<<m_pattern;

    for (int index=0; index<size; ++index) {
        QStringList args;
        Tag &tag = m->fileAtSelection(index);
        QString currentPattern=m_pattern;

        // 1. process {} functions, replace function by {0} or {1} etc.
        if (currentPattern.contains('{'))
            processCurly(args, currentPattern, index, size);

        // 2. process $ functions
        if (currentPattern.contains('$')) {
            processFunctions(args, currentPattern, tag, index, size);
        }

        // 3. empty []
        if (currentPattern.contains('['))
            removeEmptySections(tag, currentPattern);

        // 4. tags placeholders
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




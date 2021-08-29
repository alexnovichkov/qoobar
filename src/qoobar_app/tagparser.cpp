#include "tagparser.h"
#include <QtDebug>
#include <QRegularExpression>

void TagParser::truncateFileNamesToShortest(QString &pattern, QStringList &source)
{
    pattern.replace('\\','/');
    while (pattern.startsWith('/')) pattern.remove(0,1);
    const int count = source.size();

    int sourcesDirCount=100000;
    for (int i=0; i<count; ++i) {
        int j=source.at(i).count('/');
        if (j<sourcesDirCount) sourcesDirCount=j;
    }
    sourcesDirCount=qMin(sourcesDirCount, pattern.count('/'))+1;

    for (int i=0; i<count; ++i) {
        if (!source.at(i).isEmpty()) {
            source[i]=source.at(i).section('/',-1*sourcesDirCount);
        }
    }
    pattern=pattern.section('/',-1*sourcesDirCount);
}

PairList TagParser::parse(QString source, QString pattern)
{
    PairList result;

    QRegularExpression re("[%<]");

    int startPattern = 0;
    const int endPattern=pattern.length();
    int startSource=0;

    QString stack;

    while (startPattern < endPattern) {
        if (pattern[startPattern] == '%') {
            stack=pattern.mid(startPattern, 2);
            startPattern+=2;
        }
        else if (pattern[startPattern] == '<') {
            const int end = pattern.indexOf('>', startPattern);
            stack = pattern.mid(startPattern+1,end-startPattern-1);
            startPattern += (end<=0?1:stack.length()+2);
        }
        else {
            int next = pattern.indexOf(re, startPattern);
            QString delim = pattern.mid(startPattern, next-startPattern);
            int sourcePos=source.indexOf(delim, startSource);
            if (sourcePos!=-1) {
                QString s = source.mid(startSource, sourcePos-startSource);
                result << StringPair(stack.startsWith('%')?stack.mid(1,1):stack, s);
                startSource = sourcePos+delim.length();
                startPattern += delim.length();
            }
            else {
                QString s = source.mid(startSource, -1);
                result << StringPair(stack.startsWith('%')?stack.mid(1,1):stack, s);
                startPattern = endPattern;
            }
            stack.clear();
        }
    }
    if (!stack.isEmpty())
        result << StringPair(stack.startsWith('%')?stack.mid(1,1):stack, source.mid(startSource, -1));

    return result;
}

bool TagParser::needParsing(const QString &pattern)
{
    if (pattern.length()<2) return false;
    return (pattern.contains("%") || pattern.contains("<"));
}

#ifndef TAGPARSER_H
#define TAGPARSER_H

#include <QStringList>
#include <QPair>

typedef QPair<QString, QString> StringPair;
typedef QList<StringPair> PairList;

namespace TagParser
{
    bool needParsing(const QString &pattern);
    void truncateFileNamesToShortest(QString &pattern, QStringList &source);
    PairList parse(QString source, QString pattern);
}

#endif // TAGPARSER_H

#include "placeholders.h"
#include "enums.h"
#include "qoobarglobals.h"

#include <QCoreApplication>

namespace Placeholders {

int placeholderToId(const QString &placeholder)
{
    if (placeholder=="N") return TRACKNUMBER;
    for (int i=0; i < placeholdersCount; ++i) {
        if (placeholder == QString(QChar(placeholders[i].placeholder)))
            return placeholders[i].index;
    }
    return -1;
}

QString idToPlaceholder(const int id)
{
    for (int i=0; i< placeholdersCount; ++i) {
        if (placeholders[i].index==id) return QString(placeholders[i].placeholder);
    }
    return QString();
}

QString getWritablePlaceholders()
{
    QString result;
    for (int i=0; i< placeholdersCount; ++i) {
        if (placeholders[i].index>-2)
            result.append(QChar(placeholders[i].placeholder));
    }
    return result;
}

QString getReadOnlyPlaceholders()
{
    QString result;
    for (int i=0; i< placeholdersCount; ++i) {
        if (placeholders[i].index==-2)
            result.append(QChar(placeholders[i].placeholder));
    }
    return result;
}

QString getVoidPlaceholder()
{
    QString result;
    for (int i=0; i< placeholdersCount; ++i) {
        if (placeholders[i].index==-3)
            result.append(QChar(placeholders[i].placeholder));
    }
    return result;
}

QString placeholderToDescription(const QChar &id)
{
    char c = id.toLatin1();
    for (int i=0; i<placeholdersCount; ++i) {
        if (placeholders[i].placeholder==c)
            return QString("%2\t%1").arg(placeholders[i].text)
                    .arg(QCoreApplication::translate("Placeholders", placeholders[i].description));
    }
    return QString();
}



} //end of namespace Placeholders

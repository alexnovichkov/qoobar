#ifndef SECTIONS_H
#define SECTIONS_H

#include <QStringList>
#include <QPoint>

struct GetSection
{
    GetSection(const QString &baseString, int start, int end);
    typedef QPoint result_type;
    QPoint operator()(const QString &stringToCompare);

    QString m_baseString;
    int m_start;
    int m_end;
};

struct GetSectionString: public GetSection
{
    GetSectionString(const QString &baseString, int start, int end);
    typedef QString result_type;
    QString operator()(const QString &stringToCompare);
};



#endif // SECTIONS_H

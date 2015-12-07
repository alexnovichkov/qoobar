#include "sections.h"
#include <QList>
#include <QDebug>
#include <QVector>

struct section {
    section(int start=-1, int end=-1, bool common=false) {
        this->start = start;
        this->end = end;
        this->common = common;
    }
    bool operator==(const section &other) const {
        return (text==other.text);
    }

    int start;
    int end;
    bool common;
    QString text;
};

QDebug & operator<< (QDebug dbg, const section &t)
{
    dbg.nospace() << "("<<t.start<<","<<t.end<<","<<t.common<<")";
    return dbg.space();
}

int removeSingle(QVector<int> &seta, QVector<int> &setb, const QString &sa)
{//qDebug()<<sa<<sb;
    //qDebug()<<seta<<setb;
    int deleted = 0;
    if (seta.isEmpty() || setb.isEmpty()) return deleted;

    Q_ASSERT(seta.size()==setb.size());
    for (int i=seta.size()-1; i>=0; --i) {
        if (i==0) {
            if (seta.size()>1 && (seta.at(i)+1 != seta.at(i+1)) /*&& sa.at(i).isLetterOrNumber()*/) {
                seta.remove(i);
                setb.remove(i);
                deleted++;
            }
        }
        else if (i==seta.size()-1) {
            if (seta.size()>1 && (seta.at(i)-1 != seta.at(i-1)) /*&& sa.at(i).isLetterOrNumber()*/) {
                seta.remove(i);
                setb.remove(i);
                deleted++;
            }
        }
        else if (seta.size()>2 && (seta.at(i)-1 != seta.at(i-1) && seta.at(i)+1 != seta.at(i+1))
                  /*&& sa.at(i).isLetterOrNumber()*/) {
            seta.remove(i);
            setb.remove(i);
            deleted++;
        }
    }
    return deleted;
}

void createLongestCommonSubsequence(const QString & a, const QString & b,
                                    QVector<int> &seta, QVector<int> &setb)
{

    QVector<QVector<int> > max_len;
    max_len.resize(a.size() + 1);
    for(int i = 0; i <= a.size(); ++i)
        max_len[i].resize(b.size() + 1);
    for (int i = a.size() - 1; i >= 0; --i) {
        for (int j = b.size() - 1; j >= 0; --j) {
            if (a[i] == b[j])
                max_len[i][j] = 1 + max_len[i+1][j+1];
            else
                max_len[i][j] = qMax(max_len[i+1][j], max_len[i][j+1]);
        }
    }

    for (int i = 0, j = 0; i<a.size() && j<b.size() && max_len[i][j]!=0; ) {
        if (a[i] == b[j]) {
            seta << i;
            setb << j;
            i++;
            j++;
        }
        else {
            if(max_len[i][j] == max_len[i+1][j]) i++;
            else j++;
        }
    }

//qDebug()<<a<<b<<seta<<setb;
    int deleted = 0;
    do {
        deleted  = removeSingle(seta, setb, a);
        deleted += removeSingle(setb, seta, b);
    }
    while (deleted>0);
}

QList<section> createSectionsList(QVector<int> &set, int len)
{
    QList<section> list;
    if (set.isEmpty()) {
        list.append(section(0,len-1,false));
        return list;
    }
    list.append(section(0,-1,set.contains(0)));

    for (int i=0; i<len; ++i) {
        bool contains = set.contains(i);
        if (contains != list.last().common)
            list.append(section(i,-1,contains));
    }

    list.last().end = len-1;
    for (int i=0; i<list.size()-1; ++i)
        list[i].end = list[i+1].start-1;
    return list;
}

void divideLongestCommonSection(const section &sec1, const section &sec2,
                                QList<section> &list, int i)
{
    const int len=sec1.end-sec1.start;
    list[i].end = sec2.start + len;
    list.insert(i+1,section(sec2.start + len+1, sec2.end, true));
    list.insert(i+1,section(-1,-1,false));
}

void appendEmptySection(QList<section> &list)
{
    list.append(section(-1,-1,false));
}

void prependEmptySection(QList<section> &list)
{
    list.prepend(section(-1,-1,false));
}

void equalizeLists(QList<section> &baseList, QList<section> &listToCompare)
{
    while (baseList.size() != listToCompare.size()) {
        if (baseList.first().common && !listToCompare.first().common)
            prependEmptySection(baseList);
        else if (!baseList.first().common && listToCompare.first().common)
            prependEmptySection(listToCompare);

        for (int i=0; i<qMin(baseList.size(),listToCompare.size()); ++i) {
            section sec1=baseList.at(i);
            section sec2=listToCompare.at(i);

            if (sec1.common && sec2.common) {
                if (sec1.end-sec1.start < sec2.end-sec2.start)
                    divideLongestCommonSection(sec1,sec2,listToCompare,i);
                else if (sec1.end-sec1.start > sec2.end-sec2.start)
                    divideLongestCommonSection(sec2,sec1,baseList,i);
            }
        }
        if (baseList.last().common && !listToCompare.last().common)
            appendEmptySection(baseList);
        else if (!baseList.last().common && listToCompare.last().common)
            appendEmptySection(listToCompare);
    }
}


GetSectionString::GetSectionString(const QString &baseString, int start, int end)
    : GetSection(baseString, start, end)
{}

QString GetSectionString::operator()(const QString &stringToCompare)
{

    QPoint p = GetSection::operator ()(stringToCompare);
    return stringToCompare.mid(p.x(),p.y());
}

GetSection::GetSection(const QString &baseString, int start, int end)
    : m_baseString(baseString), m_start(start), m_end(end)
{}

QPoint GetSection::operator()(const QString &stringToCompare)
{
//qDebug()<<m_baseString << stringToCompare <<m_start<<m_end;
    if (stringToCompare==m_baseString)
        return QPoint(m_start,m_end-m_start+1);

    int targetStart = 0;
    int targetEnd = stringToCompare.length()-1;

    if (!stringToCompare.isEmpty()) {
        QVector<int> alist;
        QVector<int> blist;
        createLongestCommonSubsequence(m_baseString,stringToCompare,alist,blist);
        /*TODO: unite alist with baseList creation*/
//        qDebug()<<alist<<blist;

        QList<section> baseList=createSectionsList(alist, m_baseString.length());
        QList<section> listToCompare=createSectionsList(blist, stringToCompare.length());
        equalizeLists(baseList, listToCompare);
        Q_ASSERT(baseList.size()==listToCompare.size());
//qDebug()<<baseList<<listToCompare;
        for (int i=0; i<baseList.size(); ++i) {
            section sec1 = baseList.at(i);
            section sec2 = listToCompare.at(i);

            if (sec2.start<0 || m_start > sec1.end) continue;

            if (m_start > sec1.start) {// m rightmost chars
                int len=qMin(sec1.end - m_start, sec2.end-sec2.start) + 1;
                targetStart = sec2.end-len+1;
                break;
            }
            else if (m_start == sec1.start) {//section from the beginning
                targetStart = sec2.start;
                break;
            }
        }

        for (int i=baseList.size()-1; i>=0; --i) {
            const section &sec1 = baseList.at(i);
            const section &sec2 = listToCompare.at(i);

            if (sec2.start < 0 || m_end < sec1.start) continue;

            if (m_end == sec1.end) {//whole
                targetEnd = sec2.end;
                break;
            }
            else if (m_end < sec1.end) {//n leftmost
                if (targetStart <= sec2.start)
                    targetEnd = sec2.start + qMin(m_end-sec1.start,sec2.end-sec2.start);
                else {
                    int len = m_end - m_start + 1;
                    //take at most len chars skipping skip from the beginning
                    int skip = m_start - sec1.start;

                    if (len>=sec2.end-sec2.start+1) //whole section
                        targetEnd = sec2.end;
                    else {
                        while (sec2.start+skip+len-1>sec2.end) len--;
                        targetEnd = sec2.start+skip+len-1;
                    }
                }
                break;
            }
        }
    }
    return QPoint(targetStart,targetEnd-targetStart+1);
}


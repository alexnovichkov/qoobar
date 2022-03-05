#include "freedb.h"

#include "enums.h"

//#include <QUrl>
#include "discidhelper.h"
#include "qoobarglobals.h"

#include <QtDebug>
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtCore5Compat/QRegExp>
#else
#include <QRegExp>
#endif
#include <QRegularExpression>

const int FRREEDB_EXACT_MATCH = 200;
const int FREEDB_MULTIPLE_MATCHES = 210;
const int FREEDB_INEXACT_MATCH = 211;

const QString freedbHello = QString("&hello=novichkov+qoobar.sourceforge.io+freedbplugin+1.1.0&proto=6");

Request FreedbPlugin::queryForManualSearch(const QStringList &list)
{
    QString query=list.join("+");
    if (query.startsWith("+")) query.remove(0,1);
    if (query.endsWith("+")) query.chop(1);

    return Request(QString("http://www.gnudb.org/search/%1")
            .arg(QString(QUrl::toPercentEncoding(query,"+"))));
}

Request FreedbPlugin::queryForSearchByFiles(const QVector<int> &lengths)
{
    return query(lengths);
}

Request FreedbPlugin::queryForCD()
{
    return query(QVector<int>());
}

Request FreedbPlugin::queryForPicture(const QString &/*s*/)
{
    return Request(); //no pictures in freedb base
}

Request FreedbPlugin::queryForRelease(const QString &url)
{
    return Request(url);
}

Request FreedbPlugin::query(const QVector<int> &lengths)
{
    DiscIDHelper helper;
    if (!helper.isDiscidResolved()) {
        m_errorString = helper.errorString;
        return Request();
    }

    QString request = helper.getDiscID(lengths,0);
    m_errorString = helper.errorString;

    return Request(QString("http://gnudb.gnudb.org/~cddb/cddb.cgi?cmd=cddb+query+%1%2")
            .arg(request, freedbHello));
}

QList<SearchResult> FreedbPlugin::parseResponse(const QByteArray &response)
{
    QList<SearchResult> results;
    m_errorString.clear();

    QString frame="http://gnudb.gnudb.org/~cddb/cddb.cgi?cmd=cddb+read+%1"
            + freedbHello;
    QString s = QString::fromUtf8(response);
    bool ok;
    int code = response.left(3).toInt(&ok);
    if (ok) {
        if (code==FRREEDB_EXACT_MATCH) {//exact match
            if (s.simplified().contains("GnuDB is closed for Russia / In sympathy with the Ukrainian people")) {
                m_errorString = "GnuDB is closed for Russia / In sympathy with the Ukrainian people";
            }
            else {
                SearchResult r;
                r.fields.insert("url", frame.arg(s.section(" ",1,2)));
                r.fields.insert("album", s.section(" ",1).simplified());
                results << r;
            }
        }
        else if (code==FREEDB_MULTIPLE_MATCHES || code == FREEDB_INEXACT_MATCH) {//multiple exact matches or inexact matches
            QStringList list = s.split(QRegularExpression("[\\r\\n]+"));
            for (int i=1; i<list.size()-2; ++i) {
                SearchResult r;
                r.fields.insert("url", frame.arg(list.at(i).section(" ",0,1)));
                r.fields.insert("album", list.at(i).section(" ",0).simplified());
                results << r;
            }
        }
        else m_errorString=s.mid(4).simplified();
    }
    else {
//        taken from Kid3
//        <h2>Search Results, 1 albums found:</h2>
//        <br><br>
//        <a href="http://www.gnudb.org/cd/ro920b810c"><b>Catharsis / Imago</b></a><br>
//        Tracks: 12, total time: 49:07, year: 2002, genre: Metal<br>
//        <a href="http://www.gnudb.org/gnudb/rock/920b810c" target=_blank>Discid: rock / 920b810c</a><br>


        QRegExp numResultsRe("Search Results, ([0-9]+) albums found");
        QRegExp albumRe("<b>(.*)</b>"); albumRe.setMinimal(true);
        QRegExp extraRe("<br>(.*)<br>"); extraRe.setMinimal(true);
        QRegExp catIdRe("Discid: ([a-z]+)[\\s/]+([0-9a-f]+)");

        s=s.simplified();
        int numResults = 0;
        int pos=numResultsRe.indexIn(s);
        if (pos != -1)
            numResults = numResultsRe.cap(1).toInt();
        if (numResults>0) {
            for (int i=0; i < numResults; ++i) {
                SearchResult r;
                pos = albumRe.indexIn(s,pos);
                if (pos != -1) {
                    r.fields.insert("album", albumRe.cap(1));
                    pos+=albumRe.matchedLength();
                }
                pos = extraRe.indexIn(s,pos);
                if (pos != -1) {
                    r.fields.insert("extraData", extraRe.cap(1));
                    pos+=extraRe.matchedLength();
                }
                pos = catIdRe.indexIn(s,pos);
                if (pos != -1) {
                    r.fields.insert("url", frame.arg(catIdRe.cap(1) + "+" + catIdRe.cap(2)));
                    pos+=catIdRe.matchedLength();
                }
                r.releaseInfo = releaseToList(r);
                results << r;
            }
        }
    }
    return results;
}

SearchResult FreedbPlugin::parseRelease(const QByteArray &response)
{
    //# xcmd
    //#
    //# Track frame offsets:
    //#    150
    //#    400
    //...
    //# .
    //#
    //# Disc length: 1234 seconds
    //#
    //# Revision: 1
    //# Submitted via: ....
    //#
    //DISCID=disc_id
    //DTITLE=artist / title
    //DYEAR=year
    //DGENRE=genre
    //TTITLE0=[artist / ]title
    //TTITLE1=
    //...
    //TTITLEN-1=
    //EXTD=extd
    //EXTT0=
    //...
    //EXTTN-1=
    //PLAYORDER=
    SearchResult r; r.loaded = true;
    m_errorString.clear();

    if (response.isEmpty()) return r;
    int code=response.mid(0,3).toInt();
    if (code==FREEDB_MULTIPLE_MATCHES) {
        QStringList lines=QString::fromUtf8(response).split("\n");
        lines.removeFirst();

        lines.replaceInStrings("\\t","\t");
        lines.replaceInStrings("\\n","\n");

        QRegExp offsetRe = QRegExp("#\\s+(\\d+)");
        QRegExp lenRe = QRegExp("#\\s+Disc length:\\s+(\\d+)");

        QVector<int> offsets;
        int discLength=0;

        Q_FOREACH (const QString &s, lines) {
            if (s.startsWith("#")) { //comment
                if (offsetRe.indexIn(s)>-1)
                    offsets << offsetRe.cap(1).toInt();
                if (lenRe.indexIn(s)>-1)
                    discLength=lenRe.cap(1).toInt();
            }
            else break;
        }
        if (offsets.isEmpty()) return r;

        for (int i=0; i<offsets.count()-1; ++i) {
            offsets[i]=(offsets[i+1]-offsets[i])/75;
            discLength -= offsets[i];
        }
        offsets.last() = discLength;

        Q_FOREACH (const int &offset,offsets) {
            Track t; t.cd=-1;
            t.fields.insert("length",Qoobar::formatLength(offset));
            t.length = offset;
            r.tracks << t;
        }

        QRegExp ttitle = QRegExp("TTITLE(\\d+)=(.*)");
        QRegExp extt = QRegExp("EXTT(\\d+)=(.*)");

        QString album;

        for (int i=1; i<lines.count(); ++i) {
            QString s=lines.at(i).simplified();
            if (s==".") break;
            if (s.startsWith('#')) continue;
            QString field=s.section("=",0,0);
            QString value = s.section("=",1);

            if (field=="DTITLE")
                album.append(value);
            else if (field=="DYEAR")
                r.fields.insert("year", r.fields.value("year")+value);
            else if (field=="DGENRE")
                r.fields.insert("genre", r.fields.value("genre")+value);
            else if (field=="EXTD")
                r.fields.insert("extraData", r.fields.value("extraData")+value);
            else if (field=="DISCID")
                r.fields.insert("discid", r.fields.value("discid")+value);
            else if (field.startsWith("TTITLE")) {
                if (ttitle.indexIn(s) > -1) {
                    int pos = ttitle.cap(1).toInt();
                    r.tracks[pos].fields.insert("title",
                                                r.tracks[pos].fields.value("title")+ttitle.cap(2));
                    r.tracks[pos].fields.insert("tracknumber", QString::number(pos+1));
                }
            }
            else if (field.startsWith("EXTT")) {
                if (extt.indexIn(s) > -1) {
                    int pos = extt.cap(1).toInt();
                    r.tracks[pos].fields.insert("extraData",
                                                r.tracks[pos].fields.value("extraData")+extt.cap(2));
                }
            }
        }

        for (int i=0; i<r.tracks.size(); ++i) {
            QString title=r.tracks[i].fields.value("title");
            if (title.count(" / ") > 0) {
                Artist a; a.fields.insert("name", title.section(" / ", 0, 0));
                r.tracks[i].artists.append(a);
                r.tracks[i].fields.insert("title", title.section(" / ", 1));
            }
        }

        if (album.count(" / ") > 0) {
            Artist a; a.fields.insert("name", album.section(" / ", 0, 0));
            r.artists.append(a);
            r.fields.insert("album", album.section(" / ", 1));
        }
        else {
            Artist a; a.fields.insert("name", album);
            r.artists.append(a);
            r.fields.insert("album", album);
        }
    }
    else m_errorString=QString::fromUtf8(response).mid(4);
    r.releaseInfo = releaseToList(r);

    return r;
}

QStringList FreedbPlugin::releaseToList(const SearchResult &r)
{
    QString album=r.fields.value("album");
    QStringList l = album.split(" / ");
    return l;
}


int FreedbPlugin::preferredPauseSize()
{
    return 0;
}

QMap<QString, QString> FreedbPlugin::authenticationInfo()
{
    return QMap<QString, QString>();
}

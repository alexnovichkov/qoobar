/**
 * \file stringroutines.cpp
 * Various string functions
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 11 Aug 2011
 *
 * Copyright (C) 2011  Alex Novichkov
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

#include "stringroutines.h"
#include <QStringList>
#include <QTextCodec>
#include <QDateTime>
#include <QProcess>
#include <QtDebug>
#include "qoobarglobals.h"
#include "application.h"

struct Rus {
    int index;
    std::string l;
};

QRegExp createRegExp(bool caseSensitive, bool useRegularExpressions,
                     bool wholeWord, const QString &pattern)
{DD
    QRegExp rx(pattern,
               caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive,
               useRegularExpressions ? QRegExp::RegExp : QRegExp::FixedString);
    if (wholeWord) {
        rx.setPatternSyntax(QRegExp::RegExp);
        rx.setPattern(QString("\\b%1\\b").arg(pattern));
    }
    return rx;
}

QString replaceWinChars(const QString &source, const QString &byWhat, bool useSmartReplacing)
{DD
    QString s = source;
    static const struct ReplaceParams {
        const char *what;
        const char *byWhat;
    } params[] = {
        {"\\",0},
        {":",0},
        {"*",0},
        {"|",0},
        {"?",0},
        {"/",0},
        {"\"","''"},
        {"<","("},
        {">",")"}
    };
    for (int i=0; i<9; ++i) {
        if (useSmartReplacing && params[i].byWhat)
            s.replace(params[i].what,params[i].byWhat);
        else
            s.replace(params[i].what,byWhat);
    }
    return s;
}

bool isLatin(const QChar &c)
{DD;
    return (c.script() == QChar::Script_Latin);
}

QString removeDiacritics(const QString &source, bool replaceOthers)
{DD
    QString s = source;

    for(int charPos=0; charPos<s.length(); ++charPos) {
        QChar c=s.at(charPos);
        if (isLatin(c)) {//only latin chars
            QString decomposited=c.decomposition();
            if (!decomposited.isEmpty()) s.replace(charPos,1,decomposited.at(0));
        }
    }
    static const Rus stillToReplace[] = {
        {0x142,"l"},        {0x141,"L"},
        {0x153,"oe"},        {0x152,"OE"},
        {0xfe,"th"},        {0xde,"TH"},
        {0xf8,"o"},        {0xd8,"O"},
        {0xf0,"dh"},        {0xd0,"DH"},
        {0xe6,"ae"},        {0xc6,"AE"},
        {0xdf,"ss"},
        {0x131,"i"}
    };
    for (int i=0; i<14; ++i)
        s.replace(QChar(stillToReplace[i].index),QString::fromStdString(stillToReplace[i].l));
    if (replaceOthers) return QString(s.toLatin1());
    return s;
}

QString changeCase(QString source, const Case::Case ca)
{DD
    if (ca==Case::DontChange) return source;
    if (ca==Case::AllSmall) return source.toLower();
    if (ca==Case::AllCaps)  return source.toUpper();

    if (ca!=Case::EveryFirstCapsPreserving)
        source = source.toLower();
    int pos=0;
    QRegExp re(QSL("(\\b\\w)")); //beginning of the word
    while (pos>=0) {
        pos=source.indexOf(re,pos);
        if (pos>=0) {
            while (!source.at(pos).isLetter()) pos++;
            if (pos==source.length()) pos=-1;
            if (pos>=0) {
                source.replace(pos,1,source.at(pos).toUpper());
                if (ca==Case::FirstCaps) break;
                pos++;
            }
        }
    }
    if (ca==Case::EveryFirstCaps) {//apostrophes
        int pos=0;
        while (pos>=0) {
            pos=source.indexOf(QLS("'"),pos);
            if (pos>0 && pos<source.length()-1) {
                source.replace(pos+1,1,source.at(pos+1).toLower());
                pos++;
            }
            else if (pos==0) pos++;
            else pos=-1;
        }
    }

    return source;
}

QString transliterate(const QString &source)
{DD
    QString s = source;

    static const Rus capitalLetters[] = {
        {0x410,"A"},
        {0x411,"B"},
        {0x412,"V"},
        {0x413,"G"},
        {0x414,"D"},
        {0x415,"E"},
        {0x401,"E"},
        {0x417,"Z"},
        {0x418,"I"},
        {0x419,"J"},
        {0x41a,"K"},
        {0x41b,"L"},
        {0x41c,"M"},
        {0x41d,"N"},
        {0x41e,"O"},
        {0x41f,"P"},
        {0x420,"R"},
        {0x421,"S"},
        {0x422,"T"},
        {0x423,"U"},
        {0x424,"F"},
        {0x426,"C"},
        {0x429,"X"},
        {0x42a,"W"},
        {0x42b,"Y"},
        {0x42c,"J"}
    };

    static const Rus capitalMultipleLetters[] = {
        {0x416,"Zh"},
        {0x425,"Kh"},
        {0x427,"Ch"},
        {0x428,"Sh"},
        {0x42d,"Eh"},
        {0x42e,"Yu"},
        {0x42f,"Ya"}
    };

    static const Rus lowerCaseLetters[] = {
        {0x430,"a"},
        {0x431,"b"},
        {0x432,"v"},
        {0x433,"g"},
        {0x434,"d"},
        {0x435,"e"},
        {0x451,"e"},
        {0x436,"zh"},
        {0x437,"z"},
        {0x438,"i"},
        {0x439,"j"},
        {0x43a,"k"},
        {0x43b,"l"},
        {0x43c,"m"},
        {0x43d,"n"},
        {0x43e,"o"},
        {0x43f,"p"},
        {0x440,"r"},
        {0x441,"s"},
        {0x442,"t"},
        {0x443,"u"},
        {0x444,"f"},
        {0x445,"kh"},
        {0x446,"c"},
        {0x447,"ch"},
        {0x448,"sh"},
        {0x449,"x"},
        {0x44a,"w"},
        {0x44b,"y"},
        {0x44c,"j"},
        {0x44d,"eh"},
        {0x44e,"yu"},
        {0x44f,"ya"},
    };

    for (int i=0; i<7; ++i)
        s.replace(QRegExp(QString("%1([%2-%3%4])")
                          .arg(QChar(capitalMultipleLetters[i].index))
                          .arg(QChar(0x430))
                          .arg(QChar(0x44f))
                          .arg(QChar(0x451))),
                  QString("%1\\1").arg(QString::fromStdString(capitalMultipleLetters[i].l)));
    for (int i=0; i<33; ++i)
        s.replace(QChar(lowerCaseLetters[i].index),
                  QString::fromStdString(lowerCaseLetters[i].l));
    for (int i=0; i<26; ++i)
        s.replace(QChar(capitalLetters[i].index),
                  QString::fromStdString(capitalLetters[i].l));
    for (int i=0; i<7; ++i)
        s.replace(QChar(capitalMultipleLetters[i].index),
                  QString::fromStdString(capitalMultipleLetters[i].l).toUpper());
    return s;
}

Operated::Operated(const QString &type)
    : m_type(type) { }

QString Operated::operator()(const QString &s)
{DD
    QStringList l(s);
    return processFunction(m_type, l, 0);
}

// last word of the argument
//    $last_word(%c) <- last word of the composer tag
QString lastWord(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    QStringList words = args.first().split(QRegExp(QSL("\\W")), SKIP_EMPTY_PARTS);
    return words.isEmpty()?args.first():words.last();
}


// first word of the argument
QString firstWord(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    QStringList words = args.first().split(QRegExp(QSL("\\W")), SKIP_EMPTY_PARTS);
    return words.isEmpty()?args.first():words.first();
}

// n-th word of the argument, started from 1
QString nthWord(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");
    if (args.size()==1) args << "1";


    int wordPos = args.at(1).toInt();
    if (wordPos<=0) return args.first();

    QStringList words = args.first().split(QRegExp(QSL("\\W")), SKIP_EMPTY_PARTS);
    if (words.isEmpty()) return args.first();
    return words.at(qMin(wordPos,words.size())-1);
}

QString simplify(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");
    return args.join(QSL(",")).simplified();
}

// current time
QString currentTime(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    QString format=QSL("hh:mm:ss");
    if (!args.isEmpty()) format = args.first();
    return QTime::currentTime().toString(format);
}

// current date
QString currentDate(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    QString format=QSL("MMM d yyyy");
    if (!args.isEmpty()) format = args.join(QSL(","));
    return QDate::currentDate().toString(format);
}

// lowercase of the argument
QString lowerCase(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    return changeCase(args.first(), Case::AllSmall);
}

// uppercase of the argument
QString upperCase(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    return changeCase(args.first(), Case::AllCaps);
}

// Converts first letter in every word of X to uppercase, and all other letters to lowercase,
// e.g. “blah BLAH” ⇒ “Blah Blah”.
QString caps(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    return changeCase(args.first(), Case::EveryFirstCaps);
}

// Similar to $caps, but leaves uppercase letters as they are,
// e.g. “blah BLAH” ⇒ “Blah BLAH”.
QString caps2(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    return changeCase(args.first(), Case::EveryFirstCapsPreserving);
}

// Similar to $caps, but converts sentence-like:
// e.g. “blah BLAH” ⇒ “Blah blah”.
QString caps3(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    return changeCase(args.first(), Case::FirstCaps);
}

// Displays X as a number and pads with zeros up to Y characters
// (useful for track numbers etc).
QString num(QStringList &args, int index)
{DD
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");
    //if no length provided, pad to 2 chars
    if (args.size()==1) args << QSL("2");

    QString s=args.first().simplified();

    bool ok;
    int width = args.at(1).toInt(&ok);
    if (!ok) return args.first();

    return s.rightJustified(width, QChar('0'));
}

// Inserts Unicode character no. X.
QString charByNumber(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    bool ok;
    int num = args.first().toInt(&ok,0);

    if (ok) return QString(QChar(num));
    return QString();
}

// returns index of the file in the selected files
// by default starting from 1
// $index(N) = index + N
// $index() = $index(1)
QString indexOfFile(QStringList &args, int index) {
    if (args.isEmpty()) args << "1";
    return QString::number(index+args.first().toInt());
}

// $section(%c, str1, str2) <- section of %c from str1 through str2
// $section(%c, 4, 10) <- section of %c from 4th through 10th char
// $section(%c, str1) <- section of %c from str1 to the end of %c
// $section(%c) <- returns %c
QString stringSection(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    if (args.size()==1) return args.first();
    if (args.size()==2) args << QString::number(args.first().length());
    bool ok;
    int beg = args.at(1).toInt(&ok, 0)-1;
    if (!ok) beg = args.first().indexOf(args.at(1));
    int end = args.at(2).toInt(&ok, 0)-1;
    if (!ok) end = args.first().indexOf(args.at(2), beg+args.at(1).length());
    // section from beg to end plus end length
    int length = end-beg+(ok?1:args.at(2).length());
    return args.first().mid(beg, length);
}

// $section2(%c, str, 1, 2) <- splits %c by str and returns concatenated
//                             sections from 1 to 2 starting from 1
QString stringSection2(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");

    if (args.size()==1) return args.first();
    if (args.size()==2) return args.first();

    if (!args.first().contains(args.at(1))) return args.first();

    bool ok;
    int beg = args.at(2).toInt(&ok, 0)-1;
    if (!ok || beg<0) beg = 0;

    QStringList list = args.first().split(args.at(1));

    int end = -1; // whole string by default

    if (args.size()>3)
        end = args.at(3).toInt(&ok, 0)-1;
    if (!ok || end<beg) end = -1;


    list = list.mid(beg,end-beg+1);
    return list.join(args.at(1));
}

// $numsections(X,Y)
// Returns number of sections if X is splitted by Y
// This is the same as $occurrences(X,Y)+1
QString numsections(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("0");

    if (args.size()==1) return QSL("0");

    QStringList list = args.first().split(args.at(1));
    return QString::number(list.size());
}

// $occurrences(X,Y)
// Returns number of of Y in X
// This is the same as $numsections(X,Y)-1
QString occurrences(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("0");

    if (args.size()==1) return QSL("0");

    return QString::number(args.first().count(args.at(1)));
}

// $abbr(X,Y)
// Returns an abbreviation of X if X is longer than Y characters; otherwise returns full value of X.
// $abbr(X)
// Returns an abbreviation of X.
void testSection(QString &section, QString &result, QRegExp &re)
{
    bool ok;
    section.toInt(&ok, 0);
    if (ok)
        result.append(section);
    // roman numeral
    else if (re.exactMatch(section))
        result.append(section);
    else result.append(section.at(0));
    section.clear();
}

QString abbr(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QSL("");
    QString arg = args.first();

    bool ok;
    int maxSize = args.value(1).toInt(&ok, 0);
    if (maxSize == 0) arg = args.join(QSL(","));

    if (arg.length() <= maxSize) return arg;

    QString result;

    int pos = 0;
    QString section;
    QRegExp roman("[IVXLCDM]+");
    while (pos < arg.length()) {
        QCharRef c = arg[pos];
        if (c.isSpace() || c == '\\' || c == '/' || c == '|' || c == ',') {
            if (!section.isEmpty())
                testSection(section, result, roman);
        }
        else
            section.append(c);
        pos++;
    }
    if (!section.isEmpty())
        testSection(section, result, roman);

    return result;
}

// If X contains at least one present field, executes/displays Y, otherwise executes/displays Z.
QString ifFunction(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.size()<3) return QString();

    if (!args.first().simplified().isEmpty()) return args.at(1);

    return args.last();
}

// If X contains at least one present field, executes/displays X, otherwise executes/displays Y.
QString if2Function(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    if (args.size()<2) return args.first();

    if (!args.first().simplified().isEmpty()) return args.first();

    return args.at(1);
}

// Returns the first of parameters that contains at least one present field.
QString if3Function(QStringList &args, int index)
{
    Q_UNUSED(index);

    Q_FOREACH(const QString &arg, args)
        if (!arg.simplified().isEmpty()) return arg;

    return QString();
}

QString trimString(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    return args.first().trimmed();
}

QString replace(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    if (args.size()<2) return args.first();
    if (args.size()<3) args<<"";
    return args.first().replace(args.at(1), args.at(2));
}

// Repeats X N times.
QString repeat(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    if (args.size()<2) return args.first();
    bool ok;
    int n = args.at(1).toInt(&ok, 0);
    if (ok) return args.first().repeated(n);
    return args.first();
}

// Converts x to ASCII. Any characters that are not present in ASCII will be replaced.
QString ascii(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    return removeDiacritics(args.join(QSL(",")), true);
}

// Converts x to local codepage
QString ansi(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    return removeDiacritics(args.join(QSL(",")));
}

// Removes “A” and “The” prefixes from X plus 1 whitespace.
// Removes the specified prefixes from X.
QString stripPrefix(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    QStringList prefixes;
    for (int i=1; i<args.size(); ++i) {
        prefixes << QRegExp::escape(args.at(i));
    }
    QString prefix = prefixes.join(QSL(","));
    if (prefix.isEmpty()) prefix = QSL("the|a|an");
    QRegExp prefixRE("^(" + prefix + ")(?=\\s+\\w)", Qt::CaseInsensitive);
    if (prefixRE.indexIn(args.first())==0)
        return args.first().mid(prefixRE.matchedLength()+1);

    return args.first();
}

// Moves “A” and “The” prefixes to the end of X.
// Moves the specified prefixes to the end of X.
QString swapPrefix(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    QStringList prefixes;
    for (int i=1; i<args.size(); ++i) {
        prefixes << QRegExp::escape(args.at(i));
    }
    QString prefix = prefixes.join(QSL(","));
    if (prefix.isEmpty()) prefix = QSL("the|a|an");
    QRegExp prefixRE("^(" + prefix + ")(?=\\s+\\w)", Qt::CaseInsensitive);
    if (prefixRE.indexIn(args.first())==0)
        return args.first().mid(prefixRE.matchedLength()+1) + ", "+prefixRE.cap(1);

    return args.first();
}

QString transliterate(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    return transliterate(args.join(QSL(",")));
}

QString recode(QStringList &args, int index)
{
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();

    QTextCodec *codec = 0;
    QString source = args.first();
    QString encoding = args.value(1);

    if (encoding.isEmpty()) {
        if (!Qoobar::programInstalled(QSL("enca"),0))
            return source;

        QStringList arguments;
        arguments << QSL("-g") << QSL("-m") << QSL("-L") << App->encaGuessLanguage;

        QProcess p;
        p.start(QSL("enca"), arguments);
        if (p.waitForStarted()) {
            p.write(source.toLatin1());
            p.closeWriteChannel();
            if (p.waitForFinished()) {
                QByteArray b=p.readAll(); //qDebug()<<b;
                if (!b.isEmpty())
                    codec = QTextCodec::codecForName(b.simplified().split(' ').at(0));
            }
        }
    }
    else
        codec = QTextCodec::codecForName(encoding.toLatin1());
    if (codec) return codec->toUnicode(source.toLatin1());

    return source;
}

//$ifequal(n1,n2,then,else)
//Compares the integer numbers n1 and n2, if n1 is equal to n2,
//the then part is evaluated and its value returned. Otherwise the else part is evaluated and its value returned.
QString ifequal(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    while (args.size()<4) args << QSL("");
    bool ok;
    int n1 = args.first().toInt(&ok, 0);
    if (!ok) return QString();
    int n2 = args.at(1).toInt(&ok, 0);
    if (!ok) return QString();
    return (n1 == n2)?args.at(2):args.at(3);
}

// $ifgreater(n1,n2,then,else)
//Compares the integer numbers n1 and n2, if n1 is greater than n2, the then part
//is evaluated and its value returned. Otherwise the else part is evaluated and its value returned.
QString ifgreater(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    while (args.size()<4) args << QSL("");
    bool ok;
    int n1 = args.first().toInt(&ok, 0);
    if (!ok) return QString();
    int n2 = args.at(1).toInt(&ok, 0);
    if (!ok) return QString();
    return (n1 > n2)?args.at(2):args.at(3);
}

//$iflonger(s,n,then,else)
//Compares the length of the string s to the number n, if s is longer than n
//characters, the then part is evaluated and its value returned. Otherwise the else
//part is evaluated and its value returned.
QString iflonger(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    while (args.size()<4) args << QSL("");
    bool ok;
    int n = args.at(1).toInt(&ok, 0);
    if (!ok) return QString();
    return (args.first().length() > n)?args.at(2):args.at(3);
}

//$select(N,A,B,C,….)
//Returns N-th of A,B,C… parameters.
QString select(QStringList &args, int index) {
    Q_UNUSED(index);

    int n = args.value(0).toInt();
    return args.value(n+1);
}

//$cut(a,len)
//Returns first len characters from the left of the string a. This function is
//the same as $left(a,len). Negative numbers produce the entire string.
QString cut(QStringList &args, int index) {
    Q_UNUSED(index);
    int n = args.value(1).toInt();

    return args.value(0).left(n);
}

//$fix_eol(x,indicator)
//If x contains an end-of-line marker (CR-LF), the end-of-line marker and all
//text to the right of it is replaced by indicator. Otherwise x is returned unaltered.
QString fixEol(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();
    if (!args.at(0).contains('\n')) return args.at(0);
    if (args.size()<2) args << QSL(" (...)");
    return args.at(0).section('\n',0,0)+args.at(1);
}

//$insert(a,b,n)
//Inserts string B into string A after N characters.
QString insert(QStringList &args, int index) {
    Q_UNUSED(index);
    if (args.isEmpty()) return QString();

    QString arg = args.value(0);
    int pos = args.value(2).toInt();
    if (pos>arg.size()) arg.append(args.value(1));
    else arg.insert(pos, args.value(1));
    return arg;
}

//$len(a)
//Returns length of string a in characters.
QString len(QStringList &args, int index) {
    Q_UNUSED(index);
    return QString::number(args.value(0).length());
}

//$roman(n)
//Formats the integer number n in roman notation.
//Returns empty string if n <=0 or n > 3999
QString roman(QStringList &args, int index) {
    Q_UNUSED(index);
    int n = args.value(0).toInt();
    if (n < 0) return QString();
    if (n==0) return QSL("N");

    std::string result;

    static const int values[] = { 1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1 };
    static std::string numerals[] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" };
    for (int i = 0; i < 13; ++i) {
        // If the number being converted is less than the test value, append
        // the corresponding numeral or numeral pair to the resultant string
        while (n >= values[i]) {
            n -= values[i];
            result.append(numerals[i]);
        }
    }
    return QString::fromStdString(result);
}

// dummy default render function
QString dummyFunction(QStringList &args, int index) {
    Q_UNUSED(index);
    return args.isEmpty()?"":args.first();
}

FunctionsHash initRenderFunctions()
{
    FunctionsHash hash;
    hash.insert(QSL("last_word"), lastWord);
    hash.insert(QSL("lastword"), lastWord);
    hash.insert(QSL("lw"), lastWord);
    hash.insert(QSL("first_word"), firstWord);
    hash.insert(QSL("firstword"), firstWord);
    hash.insert(QSL("fw"), firstWord);
    hash.insert(QSL("nth_word"), nthWord);
    hash.insert(QSL("nthword"), nthWord);
    hash.insert(QSL("nw"), nthWord);
    hash.insert(QSL("time"), currentTime);
    hash.insert(QSL("date"), currentDate);
    hash.insert(QSL("lower"), lowerCase);
    hash.insert(QSL("upper"), upperCase);
    hash.insert(QSL("caps"), caps);
    hash.insert(QSL("caps2"), caps2);
    hash.insert(QSL("caps3"), caps3);
    hash.insert(QSL("num"), num);
    hash.insert(QSL("char"), charByNumber);
    hash.insert(QSL("index"), indexOfFile);
    hash.insert(QSL("section"), stringSection);
    hash.insert(QSL("substr"), stringSection);
    hash.insert(QSL("section2"), stringSection2);
    hash.insert(QSL("numsections"), numsections);
    hash.insert(QSL("occurrences"), occurrences);
    hash.insert(QSL("count"), occurrences);
    hash.insert(QSL("abbr"), abbr);
    hash.insert(QSL("if"), ifFunction);
    hash.insert(QSL("if2"), if2Function);
    hash.insert(QSL("if3"), if3Function);
    hash.insert(QSL("trim"), trimString);
    hash.insert(QSL("replace"), replace);
    hash.insert(QSL("repeat"), repeat);
    hash.insert(QSL("stripprefix"), stripPrefix);
    hash.insert(QSL("swapprefix"), swapPrefix);
    hash.insert(QSL("ascii"), ascii);
    hash.insert(QSL("ansi"), ansi);
    hash.insert(QSL("simplify"), simplify);
    hash.insert(QSL("transliterate"), transliterate);
    hash.insert(QSL("recode"), recode);
    hash.insert(QSL("ifequal"), ifequal);
    hash.insert(QSL("ifgreater"), ifgreater);
    hash.insert(QSL("iflonger"), iflonger);
    hash.insert(QSL("select"), select);
    hash.insert(QSL("cut"), cut);
    hash.insert(QSL("left"), cut);
    hash.insert(QSL("fix_eol"), fixEol);
    hash.insert(QSL("insert"), insert);
    hash.insert(QSL("len"), len);
    hash.insert(QSL("roman"), roman);
    return hash;
}

QString processFunction(const QString &name, QStringList &args, int index)
{
    static FunctionsHash renderFunctions = initRenderFunctions();
    return renderFunctions.value(name, dummyFunction)(args, index);
}

QString operatedString(const QString &s, const QString &type)
{
    QStringList args(s);
    return processFunction(type, args, 0);
}

#include "clparser.h"

#ifdef QOOBAR_ENABLE_CLI

#include <QJsonDocument>

#ifdef Q_OS_LINUX
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#endif

#include "enums.h"
#include "QFileInfo"

unsigned short consoleWidth()
{
#ifdef Q_OS_LINUX
    static unsigned short width = 0;

    if (width==0) {
#ifdef TIOCGSIZE
        struct ttysize ts;
        ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
        width = ts.ts_cols;
#elif defined(TIOCGWINSZ)
        struct winsize ts;
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
        width = ts.ws_col;
#endif /* TIOCGSIZE */
    }
    return width;
#else
    return 80;
#endif
}

void spacePrint(std::ostream& os, const QString &s, int indentSpaces, const int secondLineOffset)
{
    int len = s.length();
    int maxWidth = consoleWidth()-2; //2-symbol margin

    if ((len + indentSpaces > maxWidth) && maxWidth > 0) {
        int allowedLen = maxWidth - indentSpaces;
        int start = 0;
        while (start < len) {
            // find the substring length
            int stringLen = qMin(len - start, allowedLen);

            // trim the length so it doesn't end in middle of a word
            if (stringLen == allowedLen && stringLen+start<len) {
                while (stringLen >= 0 &&
                       s.at(stringLen+start) != ' ' &&
                       s.at(stringLen+start) != ',' &&
                       s.at(stringLen+start) != '|')
                    stringLen--;
            }
            // ok, the word is longer than the line, so just split
            // wherever the line ends
            if (stringLen <= 0) stringLen = allowedLen;

            // check for newlines
            for (int i = 0; i < stringLen; i++)
                if (s.at(start+i) == '\n') stringLen = i+1;

            // print the indent
            for (int i = 0; i < indentSpaces; i++)  os << " ";

            if (start == 0) {
                // handle second line offsets
                indentSpaces += secondLineOffset;

                // adjust allowed len
                allowedLen -= secondLineOffset;
            }

            os << qPrintable(s.mid(start,stringLen)) << std::endl;

            // so we don't start a line with a space
            while (start < len && (stringLen+start<len) && s.at(stringLen+start) == ' ')
                start++;

            start += stringLen;
        }
    }
    else {
        for (int i = 0; i < indentSpaces; i++)  os << " ";
        os << qPrintable(s) << std::endl;
    }
}

CLOption::CLOption(int type,
                   const QString &shortOption,
                   const QString &longOption,
                   const QString &description)
    : type(type),
      shortOption(shortOption),
      longOption(longOption),
      description(description),
      allowMore(false),
      alreadySet(false),
      argumentType(CL_NOARG),
      commandLineMode(false)
{
    unlabeled = (shortOption.isEmpty() && longOption.isEmpty());
}


bool CLOption::operator==(const CLOption& opt) const
{
    return ((shortOption != "" && shortOption == opt.shortOption) || longOption == opt.longOption);
}

QVariant CLOption::getValue() const
{
    if (alreadySet) return value;
    return defaultValue;
}

QString CLOption::typeDescr() const
{
    if (type!=CL_SWITCH) {
        if (!typeDescription.isEmpty())
            return  " <" + typeDescription  + ">";
        if (!constraint.isEmpty())
            return " <" + constraint  + ">";
    }
    return "";
}

QString CLOption::shortID() const
{
    if (unlabeled) return QString("<%1> ...").arg(typeDescription);

    QString id = "-" + shortOption;
    if (shortOption.isEmpty())
        id = "--" + longOption;
    id += typeDescr();
    id = "[" + id + "]";
    if (allowMore) id += " ... ";
    return id;
}

QString CLOption::longID() const
{
    if (unlabeled) return QString("<%1> (accepted multiple times)").arg(typeDescription);

    QString id;

    if (!shortOption.isEmpty()) {
        id += "-" + shortOption;
        id += typeDescr();
        id += ",  ";
    }

    id += "--" + longOption;
    id += typeDescr();
    if (allowMore) id += "  (accepted multiple times)";
    return id;
}

QString CLOption::toString() const {
    QString s;
    if (!shortOption.isEmpty())
        s += "-" + shortOption + " ";
    s += "(--" + longOption + ")";
    return s;
}

bool CLOption::argMatches(const QString &arg) const
{
    return ((arg == "-" + shortOption && !shortOption.isEmpty()) ||
            (arg == "--" + longOption && !longOption.isEmpty()));
}

int CLOption::combinedSwitchesMatch(QString &combinedSwitches)
{
    // make sure this is actually a combined switch
    if ((!combinedSwitches.isEmpty() && !combinedSwitches.startsWith('-'))
            || combinedSwitches.startsWith("--") || combinedSwitches.contains(' '))
        return 0;

    if (shortOption.isEmpty() || shortOption=="-") return 0; //ignore rest option

    int count = combinedSwitches.count(shortOption);
    combinedSwitches.replace(shortOption, QChar(CLOption::blankChar()));

    return count;
}


CLParser::CLParser(const QString &message, const QString &version)
    : m_message(message), m_version(version), m_isCommandLineMode(false) { }


CLOption *CLParser::option(const QString &id) {
    //        for (int i=0; i<optionList.size(); ++i)
    //            if (optionList[i].id==id) return &optionList[i];

    if (hash.contains(id)) {
        return hash[id];
    }
    return 0;
}

bool CLParser::isSet(const QString &id, QString &s)
{
    CLOption *opt = option(id);
    if (opt) {
        if (opt->alreadySet)
            s = opt->getValue().toString();
        return opt->alreadySet;
    }
    return false;
}
bool CLParser::isSet(const QString &id)
{
    CLOption *opt = option(id);
    if (opt) return opt->alreadySet;
    return false;
}

void CLParser::showHelp() const {
    std::cout << std::endl << "USAGE: " << std::endl << std::endl;
    shortUsage(std::cout);
    std::cout << std::endl << std::endl << "Where: " << std::endl << std::endl;
    longUsage(std::cout);
    std::cout << std::endl;
}

void CLParser::showVersion() const
{
    std::cout << std::endl << qPrintable(m_programName)
              << "  version: " << qPrintable(m_version)
              << std::endl << std::endl;
}

bool CLParser::setArgScheme(const QByteArray &json)
{
    bool ok=true;
    QVariantMap map;
    QJsonParseError error;
    QVariant parsed = QJsonDocument::fromJson(json, &error).toVariant();
    if (error.error!=QJsonParseError::NoError) return false;
    map = parsed.toMap();
    if (!ok || map.isEmpty()) return false;
    QVariantList list = map.value("args").toList();
    if (list.isEmpty()) return false;

    Q_FOREACH(const QVariant &v, list) {
        map = v.toMap();
        QString shortopt = map.value("shortopt").toString();
        QString longopt = map.value("longopt").toString();

        if (shortopt.length()>1)
            failure("Argument flag can only be one character long", shortopt);
        if (shortopt == "-" || shortopt == " ")
            failure("Argument flag cannot be either '-' or a space", shortopt);
        if (longopt.startsWith("-") || longopt.startsWith("--") || longopt.contains(" "))
            failure("Argument name begins with either '-' or '--' or contains space", longopt);

        int type = map.value("type").toString()=="switch"?CLOption::CL_SWITCH:CLOption::CL_VALUE;
        CLOption *opt = new CLOption(type, shortopt, longopt, map.value("description").toString());

        if (opt->unlabeled) opt->id = map.value("id").toString();
        else opt->id = longopt;

        opt->allowMore=map.value("allowMore").toBool();
        opt->group=map.value("group").toString();
        opt->defaultValue=map.value("defaultVal");
        QString argType = map.value("argType").toString();
        opt->argumentType=argType.isEmpty()?CLOption::CL_NOARG:(argType=="int"?CLOption::CL_ARGINT:CLOption::CL_ARGSTRING);
        opt->typeDescription=map.value("typeDescription").toString();
        opt->commandLineMode = map.value("commandLine").toBool();
        opt->constraint=map.value("constraint").toString();

        if (!hash.contains(opt->id)) {
            hash.insert(opt->id, opt);
            optionList << opt;
        }
    }
    if (optionList.isEmpty()) return false;

    optionList.prepend(new CLOption(CLOption::CL_IGNOREREST, "-", "--",
                                    "Ignores the rest of the labeled arguments following this flag."));

    // create m_groups
    Q_FOREACH (CLOption *opt, optionList) {
        m_groups << opt->group;
    }
    m_groups.removeDuplicates();
    if (m_groups.contains(QSL(""))) {
        m_groups.removeAll(QSL(""));
        m_groups << QSL("");
    }
    return true;
}

void CLParser::extractValue(CLOption *opt, const QString &val)
{
    if (opt->type==CLOption::CL_SWITCH) {
        opt->value = true;
        return;
    }
    if (!opt->constraint.isEmpty())
        if (!opt->constraint.contains(val))
            failure(QString("Value '%1' does not meet constraint: %2")
                    .arg(val).arg(opt->constraint), opt->toString());

    if (opt->argumentType==CLOption::CL_ARGINT) {
        bool ok;
        val.toInt(&ok);
        if (!ok)
            failure("The value for this argument should be int: ",opt->toString());
    }
    if (opt->allowMore) {
        QVariantList l = opt->value.toList();
        l << val;
        opt->value = l;
    }
    else opt->value = val;
}

bool CLParser::processArg(CLOption *opt, int *i, QStringList &args)
{
    if (!opt->unlabeled && ignoreRest())
        return false;

    QString testedString = args.at(*i);
    if (opt->argMatches(testedString)) { // <- matches -a or --arg
        if (opt->type==CLOption::CL_IGNOREREST) {
            beginIgnoring();
            return true;
        }

        if (opt->alreadySet && !opt->allowMore)
            failure(QSL("Argument already set!"), opt->toString());

        if (opt->type==CLOption::CL_VALUE) {//jump to next arg
            (*i)++;
            if ((*i) > args.size())
                failure("Missing a value for this argument!", opt->toString());
            testedString = args.at(*i);
        }

        extractValue(opt, testedString);
        opt->alreadySet = true;
        m_isCommandLineMode |= opt->commandLineMode;
        return true;
    }

    if (opt->type==CLOption::CL_SWITCH) {// check for a combination of flags
        int count = opt->combinedSwitchesMatch(args[*i]);
        if (count>0) {
            if (count>1 || opt->alreadySet)
                failure("Argument already set!", opt->toString());

            opt->alreadySet = true;
            opt->value = true;
            if (opt->type==CLOption::CL_IGNOREREST) beginIgnoring();

            // We only want to return true if we've found the last combined
            // match in the string, otherwise we return false so that other
            // switches in the combination will have a chance to match.
            return (args.at(*i)=="-"+QString(args.at(*i).length()-1,CLOption::blankChar()));
        }
    }

    if (opt->unlabeled) { // no other option matched, treat as unlabeled arg
        extractValue(opt, args.at(*i));
        opt->alreadySet = true;
        return true;
    }

    return false;
}

bool CLParser::parse(QStringList args)
{
    if (args.isEmpty() || optionList.isEmpty()) return false;

    m_programName = args.takeFirst();
    for (int i = 0; i < args.size(); ++i) {
        bool matched = false;
        for (int ind=0; ind<optionList.size(); ++ind) {
            if (processArg(optionList[ind], &i, args)) {
                matched = true;
                break;
            }
        }
        // checks to see if the argument is an empty combined
        // switch and if so, then we've actually matched it
        if (!matched && emptyCombined(args.at(i)))
            matched = true;

        if (!matched && !ignoreRest())
            failure("Couldn't find match for argument",args.at(i));
    }

    return true;
}
CLParser::~CLParser() {
    qDeleteAll(optionList);
}


void CLParser::shortUsage(std::ostream &os) const
{
    QString progName = QFileInfo(m_programName).fileName(); //short program name
    QStringList l(progName);

    Q_FOREACH (CLOption *opt, optionList)
        l << opt->shortID();

    // if the program name is too long, then adjust the second line offset
    int secondLineOffset = qMin(progName.length() + 2, consoleWidth()/2);
    spacePrint(os, " "+l.join(" "), 3, secondLineOffset);
}
void CLParser::longUsage(std::ostream &os) const
{
    Q_FOREACH (const QString &group, m_groups) {
        spacePrint(os, group.isEmpty()?"Other options":group, 0, 0);
        os << std::endl;
        Q_FOREACH (CLOption *opt, optionList) {
            if (opt->type==CLOption::CL_IGNOREREST) continue;
            if (opt->group!=group) continue;
            spacePrint(os, opt->longID(), 3, 3);
            spacePrint(os, opt->description, 5, 0);
            os << std::endl;
        }
    }
    os << std::endl;
    spacePrint(os, m_message, 3, 0);
}
bool CLParser::emptyCombined(const QString &s) const
{
    if (!s.isEmpty() && !s.startsWith('-'))
        return false;
    for (int i = 1; i < s.length(); ++i) {
        if (s.at(i) != CLOption::blankChar())
            return false;
    }
    return true;
}

QString CLParser::failure(const QString &mess, const QString &s) const
{
    std::cerr << "PARSE ERROR: " << qPrintable(mess) << std::endl
              << "             Argument: " << qPrintable(s) << std::endl << std::endl;
    if (!optionList.isEmpty()) {
        QString progName = QFileInfo(m_programName).fileName(); //short program name
        std::cerr << "Brief USAGE: " << std::endl;
        shortUsage(std::cerr);
        std::cerr << std::endl << "For complete USAGE and HELP type: "
                  << std::endl << "   " << qPrintable(progName) << " --help"
                  << std::endl << std::endl;
    }
    exit(1);
}

#endif //QOOBAR_ENABLE_CLI

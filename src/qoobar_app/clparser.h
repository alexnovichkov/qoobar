#ifndef CLPARSER_H
#define CLPARSER_H

#ifdef QOOBAR_ENABLE_CLI

#include <QStringList>
#include <QVariant>
#include <QHash>



#include <iostream>

/**
 * @brief consoleWidth computes the width of console in characters
 */
unsigned short consoleWidth();

/**
* This function inserts line breaks and indents long strings
* according the  params input. It will only break lines at spaces,
* commas and pipes.
* \param os - The stream to be printed to.
* \param s - The string to be printed.
* \param indentSpaces - The number of spaces to indent the first line.
* \param secondLineOffset - The number of spaces to indent the second
* and all subsequent lines in addition to indentSpaces.
*/
void spacePrint(std::ostream& os, const QString &s, int indentSpaces, const int secondLineOffset);

class CLOption
{
public:
    enum Type {
        CL_SWITCH, // switch, can be true or false
        CL_VALUE, // value, string or integer
        CL_IGNOREREST // -- option
    };
    enum ArgType {
        CL_ARGINT, // integer value
        CL_ARGSTRING, // string value
        CL_NOARG //no value
    };
    CLOption(int type,
             const QString &shortOption,
             const QString &longOption,
             const QString &description);
    int type; // {CL_SWITCH, CL_VALUE, CL_IGNOREREST};
    QString shortOption; //1 char long
    QString longOption;
    QString description;
    bool allowMore; // default = false
    bool alreadySet;
    QString constraint;
    QString group;
    int argumentType; // {CL_ARGINT, CL_ARGSTRING, CL_NOARG};
    bool commandLineMode;
    QString typeDescription;
    QVariant defaultValue; //false for switch, null for other or set to some val
    QVariant value;
    bool unlabeled;
    QString id;

    bool operator==(const CLOption& opt) const;

    QVariant getValue() const;

    QString typeDescr() const;

    QString shortID() const;

    QString longID() const;

    QString toString() const;

    bool argMatches(const QString &arg) const;

    int combinedSwitchesMatch(QString &combinedSwitches);
    static char blankChar() {return (char)7;}
};

//struct cloption {
//    char *longOption;
//    char *description;
//    char *typeDescription;
//    char *constraint;
//    char *group;
//    short type;
//    short argumentType;
//    char shortOption;
//    bool allowMore; // default = false
//    bool commandLineMode;
//};

//const cloption cloptions[] = {
//    {"","","","","",CLOption::CL_SWITCH,CLOption::CL_NOARG,'',false,false},

//    {0,0,0,0,0,CLOption::CL_SWITCH,CLOption::CL_NOARG,0,false,false},
//};

class CLParser
{

public:
    CLParser(const QString &message, const QString &version);

    bool isCommandLineMode() const {return m_isCommandLineMode;}

    CLOption *option(const QString &id);

    /**
     * @brief isSet tests whether option id is set and if set, returns its value in s
     * @param id - option id
     * @param s - reference to string to fill with option value
     * @return
     */
    bool isSet(const QString &id, QString &s);

    bool isSet(const QString &id);

    void showHelp() const;

    void showVersion() const;

    bool setArgScheme(const QByteArray &json);

    void extractValue(CLOption *opt, const QString &val);

    bool processArg(CLOption *opt, int *i, QStringList &args);

    bool parse(QStringList args);
    ~CLParser();

private:
    void shortUsage(std::ostream &os) const;
    void longUsage(std::ostream &os) const;
    bool emptyCombined(const QString &s) const;

    QString failure(const QString &mess, const QString &s) const;

    static void beginIgnoring() {ignoreRestRef() = true;}
    static bool ignoreRest() {return ignoreRestRef();}
    static bool& ignoreRestRef() {static bool ign = false; return ign;}

    QString m_message;
    QString m_version;
    QString m_programName;
    QList<CLOption*> optionList; // internal to traverse when parsing
    QHash<QString, CLOption*> hash; // helper hash for quick location of option

    QStringList m_groups; //contains names of groups of arguments
    bool m_isCommandLineMode;
};

#endif //QOOBAR_ENABLE_CLI

#endif // CLPARSER_H

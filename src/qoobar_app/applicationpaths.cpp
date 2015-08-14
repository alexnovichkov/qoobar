
#include "applicationpaths.h"

//namespace ApplicationPaths
//{
#ifdef Q_OS_MAC
const QString ApplicationPaths::bundlePath()
{
    static QString _bundlePath;
    if (_bundlePath.isEmpty()) {
        QDir dir(qApp->applicationDirPath());
        dir.cdUp();
        _bundlePath=dir.canonicalPath();
    }
    return _bundlePath;
}
#endif

const QString ApplicationPaths::sharedPath()
{
#ifdef Q_OS_LINUX
    return QString("/usr/share/qoobar");
#elif defined Q_OS_MAC
    return bundlePath() +"/Resources";
#else
    return qApp->applicationDirPath();
#endif
}

const QString ApplicationPaths::translationsPath()
{
    return sharedPath();
}

const QString ApplicationPaths::schemesPath()
{
    return sharedPath()+"/schemes";
}

const QString ApplicationPaths::userSchemesPath()
{
#ifdef Q_OS_LINUX
    return QDir::homePath() + "/.config/qoobar/schemes";
#else
    return schemesPath();
#endif
}

const QString ApplicationPaths::completionsPath()
{
    return sharedPath()+"/completions";
}

const QString ApplicationPaths::userCompletionsPath()
{
#ifdef Q_OS_LINUX
    return QDir::homePath() + "/.config/qoobar/completions";
#else
    return completionsPath();
#endif
}

const QString ApplicationPaths::documentationPath()
{
    return sharedPath()+"/html/en/documentation.htm";
}

const QString ApplicationPaths::pluginsPath()
{
#ifdef Q_OS_LINUX
    return QString("/usr/lib/qoobar/plugins");
#elif defined Q_OS_MAC
    return bundlePath() +"/PlugIns";
#else
    return sharedPath()+"/plugins";
#endif
}
//} // namespace


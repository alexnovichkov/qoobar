
#include "applicationpaths.h"
#include "qoobarglobals.h"

//namespace ApplicationPaths
//{
#ifdef Q_OS_MAC
const QString ApplicationPaths::bundlePath()
{DD;
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
{DD;
#ifdef Q_OS_LINUX
    return QString("/usr/share/qoobar");
#elif defined Q_OS_MAC
    return bundlePath() +"/Resources";
#else
    return qApp->applicationDirPath();
#endif
}

const QString ApplicationPaths::translationsPath()
{DD;
    return sharedPath();
}

const QString ApplicationPaths::schemesPath()
{DD;
    return sharedPath()+"/schemes";
}

const QString ApplicationPaths::userSchemesPath()
{DD;
#ifdef Q_OS_LINUX
    return QDir::homePath() + "/.config/qoobar/schemes";
#else
    return schemesPath();
#endif
}

const QString ApplicationPaths::completionsPath()
{DD;
    return sharedPath()+"/completions";
}

const QString ApplicationPaths::userCompletionsPath()
{DD;
#ifdef Q_OS_LINUX
    return QDir::homePath() + "/.config/qoobar/completions";
#else
    return completionsPath();
#endif
}

const QString ApplicationPaths::documentationPath()
{DD;
    return sharedPath()+"/html/en/documentation.htm";
}

const QString ApplicationPaths::pluginsPath()
{DD;
#ifdef Q_OS_LINUX
    return QString("/usr/lib/qoobar/plugins");
#elif defined Q_OS_MAC
    return bundlePath() +"/PlugIns";
#else
    return sharedPath()+"/plugins";
#endif
}
//} // namespace


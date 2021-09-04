
#include "applicationpaths.h"
#include "qoobarglobals.h"

#ifdef Q_OS_MACOS
QString ApplicationPaths::bundlePath()
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

QString ApplicationPaths::sharedPath()
{DD;
#ifdef Q_OS_MACOS
    return bundlePath() +"/Resources";
#endif
#ifdef Q_OS_LINUX
    return QString("/usr/share/qoobar");
#endif
    return qApp->applicationDirPath();
}

QString ApplicationPaths::translationsPath()
{DD;
    return sharedPath();
}

QString ApplicationPaths::schemesPath()
{DD;
    return sharedPath()+"/schemes";
}

QString ApplicationPaths::userSchemesPath()
{DD;
#ifdef Q_OS_LINUX
    return QDir::homePath() + "/.config/qoobar/schemes";
#else
    return schemesPath();
#endif
}

QString ApplicationPaths::completionsPath()
{DD;
    return sharedPath()+"/completions";
}

QString ApplicationPaths::userCompletionsPath()
{DD;
#ifdef Q_OS_LINUX
    return QDir::homePath() + "/.config/qoobar/completions";
#else
    return completionsPath();
#endif
}

QString ApplicationPaths::documentationPath()
{DD;
    return sharedPath()+"/html/en/documentation.htm";
}

QString ApplicationPaths::pluginsPath()
{DD;
#ifdef Q_OS_LINUX
    return QString("/usr/lib/qoobar/plugins");
#elif defined Q_OS_MACOS
    return bundlePath() +"/PlugIns";
#else
    return sharedPath()+"/plugins";
#endif
}
//} // namespace


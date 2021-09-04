#include "platformprocess.h"

#include "applicationpaths.h"
#include "qoobarglobals.h"

PlatformProcess::PlatformProcess(QObject *parent) :
    QProcess(parent)
{DD;
    setReadChannel(QProcess::StandardOutput);
    setProcessChannelMode(QProcess::MergedChannels);

#ifdef Q_OS_MACOS
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", env.value("PATH") + ":" + ApplicationPaths::sharedPath());
    setProcessEnvironment(env);
#endif
}

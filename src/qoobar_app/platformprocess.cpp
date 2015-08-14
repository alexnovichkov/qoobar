#include "platformprocess.h"

#include "applicationpaths.h"

PlatformProcess::PlatformProcess(QObject *parent) :
    QProcess(parent)
{
    setReadChannel(QProcess::StandardOutput);
    setProcessChannelMode(QProcess::MergedChannels);

#ifdef Q_OS_MAC
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", env.value("PATH") + ":" + ApplicationPaths::sharedPath());
    setProcessEnvironment(env);
#endif
}

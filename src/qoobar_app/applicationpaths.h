#ifndef APPLICATIONPATHS_H
#define APPLICATIONPATHS_H

#include <QCoreApplication>
#include <QDir>
#include <QString>

namespace ApplicationPaths
{
//#ifdef Q_OS_MAC
//QString bundlePath();
//#endif

QString sharedPath();

QString translationsPath();

QString schemesPath();

QString userSchemesPath();

QString completionsPath();

QString userCompletionsPath();

QString documentationPath();

QString pluginsPath();
} // namespace

#endif // APPLICATIONPATHS_H

#ifndef APPLICATIONPATHS_H
#define APPLICATIONPATHS_H

#include <QCoreApplication>
#include <QDir>
#include <QString>

namespace ApplicationPaths
{
#ifdef Q_OS_MAC
const QString bundlePath();
#endif

const QString sharedPath();

const QString translationsPath();

const QString schemesPath();

const QString userSchemesPath();

const QString completionsPath();

const QString userCompletionsPath();

const QString documentationPath();

const QString pluginsPath();
} // namespace

#endif // APPLICATIONPATHS_H

#ifndef APPLICATIONPATHS_H
#define APPLICATIONPATHS_H

#include <QCoreApplication>
#include <QDir>
#include <QString>

namespace ApplicationPaths
{
QString bundlePath();

QString sharedPath();

QString translationsPath();

QString schemesPath();

QString userSchemesPath();

QString completionsPath();

QString userCompletionsPath();

QString documentationPath();

QString pluginsPath();

QString iconsPath();

QString cachePath();
} // namespace

#endif // APPLICATIONPATHS_H

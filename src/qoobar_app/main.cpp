/**
 * \file main.cpp
 * Main program.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 8 Dec 2009
 *
 * Copyright (C) 2009  Alex Novichkov
 *
 * This file is part of Qoobar.
 *
 * Qoobar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Qoobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "application.h"
#include "mainwindow.h"

#ifdef Q_OS_WIN
#include "windows.h"
#include "winuser.h"
#endif

#include <QTimer>

#include <QNetworkProxyFactory>
#include <QUrl>

#ifdef QOOBAR_ENABLE_CLI
#include "argsparser.h"
#endif

#include "qoobarglobals.h"
#include "logging.h"
#include "enums.h"

#include <QProcess>

#include <QVariantMap>

#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

int main(int argc, char *argv[])
{DD;
#ifdef Q_OS_WIN
    qt_ntfs_permission_lookup++; // turn NTFS ownership and permissions checking on
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
#elif QT_VERSION >= QT_VERSION_CHECK(5,6,0)
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
#endif // QT_VERSION
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#ifdef Q_OS_WIN

    //This mutex is used to prevent the user from installing new versions
    //of Qoobar while Qoobar is still running, and to prevent
    //the user from uninstalling a running application.
    CreateMutex(NULL,false,L"QoobarMutex");

    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    QCoreApplication::setLibraryPaths(paths);
#endif

//    #ifdef QT_NO_DEBUG
    qSetMessagePattern("%{function}: %{type}: %{message}");
//    #endif

    Application app(argc, argv);
    QCoreApplication::setApplicationName(QSL("qoobar"));
    QCoreApplication::setOrganizationName(QSL("qoobar"));
    QCoreApplication::setApplicationVersion(QString(QOOBAR_VERSION));
    app.readGlobalSettings();

#ifdef QOOBAR_ENABLE_CLI
    CommandLineProcessor parser;
    QObject::connect(&parser,SIGNAL(finished()),&app,SLOT(quit()),Qt::QueuedConnection);

    //starts parsing argv and creates MainWindow if not in command line mode
    QTimer::singleShot(0,&parser,SLOT(parse()));
#else
    QStringList filesNames = QCoreApplication::arguments().mid(1);
    app.setFilesNames(Qoobar::expandFilesNames(filesNames, true));
    new MainWindow();
#endif

    // Make sure we honor the system's proxy settings
    //Stolen from Qt Creator
#if defined(Q_OS_UNIX)
    QUrl proxyUrl(QString::fromLatin1(qgetenv("http_proxy")));
    if (proxyUrl.isValid()) {
        QNetworkProxy proxy(QNetworkProxy::HttpProxy, proxyUrl.host(),
                            proxyUrl.port(), proxyUrl.userName(), proxyUrl.password());
        QNetworkProxy::setApplicationProxy(proxy);
    }
# if defined(Q_OS_MAC) // unix and mac
    else {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
    }
# endif
#else // windows and os/2
    QNetworkProxyFactory::setUseSystemConfiguration(true);
#endif

//    QList<QPair<QVariant, int>> map;
//    using P = QPair<QVariant, int>;
//    map.append({QString("aaa"),0});
//    map.append({QString("Aab"),1});
//    map.append({QString("Aac"),2});
//    map.append({QString("aad"),3});
//    for (auto it = map.begin(); it!=map.end(); ++it)
//        qDebug()<<*it;
//    std::sort(map.begin(), map.end(), [](const P &first, const P &second){
//        return first.first.toString().toLower() < second.first.toString().toLower();
//    });
//    for (auto it = map.begin(); it!=map.end(); ++it)
//        qDebug()<<*it;
    return app.exec();
}

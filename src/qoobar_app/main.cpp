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

#ifdef Q_OS_WIN32
#include "windows.h"
#endif

#include <QTimer>

#include <QNetworkProxyFactory>
#include <QUrl>

#ifdef QOOBAR_ENABLE_CLI
#include "argsparser.h"
#endif

#include "qoobarglobals.h"
#include "enums.h"

#include <QProcess>

int main(int argc, char *argv[])
{DD
#ifdef Q_OS_WIN32
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

#ifdef HAVE_QT5
    #ifdef QT_NO_DEBUG
    qSetMessagePattern("%{file} on %{line}: %{type}: %{message}");
    #endif
#endif

    Application app(argc, argv);
    app.setApplicationName(QSL("qoobar"));
    app.setApplicationVersion(QString(QOOBAR_VERSION));
    app.readGlobalSettings();

#ifdef QOOBAR_ENABLE_CLI
    CommandLineProcessor parser;
    QObject::connect(&parser,SIGNAL(finished()),&app,SLOT(quit()),Qt::QueuedConnection);

    //starts parsing argv and creates MainWindow if not in command line mode
    QTimer::singleShot(0,&parser,SLOT(parse()));
#else
    QStringList filesNames = app.arguments().mid(1);
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

    return app.exec();
}

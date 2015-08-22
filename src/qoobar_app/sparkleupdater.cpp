#include "sparkleupdater.h"

#include <winsparkle.h>
#include "mainwindow.h"

#include <QLibrary>

#include <QtDebug>

MainWindow *win;

static int canShutDownCallback()
{
    return win->allTabsSaved();
}

static void shutDownRequestCallback()
{
    win->close();
}

SparkleUpdater::SparkleUpdater(QObject *parent) :
    QObject(parent)
{
    win_sparkle_set_registry_path("Software\\qoobar\\updates");

    win_sparkle_init();

    win = static_cast<MainWindow*>(parent);

//    win_sparkle_set_can_shutdown_callback(canShutDownCallback);

//    win_sparkle_set_shutdown_request_callback(shutDownRequestCallback);
}

SparkleUpdater::~SparkleUpdater()
{
    win_sparkle_cleanup();
}

void SparkleUpdater::checkNow(bool silent)
{
    if (silent) {
//        win_sparkle_check_update_without_ui();
    }
    else {
        win_sparkle_check_update_with_ui();
    }
}

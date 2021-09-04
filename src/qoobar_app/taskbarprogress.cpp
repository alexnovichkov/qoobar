#include "taskbarprogress.h"

#ifdef Q_OS_WIN
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define WIN_TASKBAR
#include <QWinTaskbarProgress>
#include <QWinTaskbarButton>
#endif
#endif
#endif

#include <QtWidgets>

#ifdef Q_OS_LINUX
#include <QDBusMessage>
#include <QDBusConnection>
#endif

#include "qoobarglobals.h"

void sendMessage(const QVariantMap &map)
{DD
    Q_UNUSED(map)

#ifdef Q_OS_LINUX
    QDBusMessage message = QDBusMessage::createSignal(QString("/%1").arg(qAppName()),
                                                      "com.canonical.Unity.LauncherEntry", "Update");
    QVariantList args;
    args << QString("application://%1.desktop").arg(qAppName())
         << map;
    message.setArguments(args);
    if (!QDBusConnection::sessionBus().send(message)) {
        qWarning("Unable to send message");
    }
#endif
}

TaskBarProgress::TaskBarProgress(QWidget *window, QObject *parent) : QObject(parent),
    window(window)
{DD
#ifdef WIN_TASKBAR
    QWinTaskbarButton *button = new QWinTaskbarButton(this);
    button->setWindow(window->windowHandle());

    winProgress = button->progress();
    winProgress->setVisible(true);
#endif
}


void TaskBarProgress::setRange(int min, int max)
{DD
    range = qMakePair(min, max);
#ifdef WIN_TASKBAR
    winProgress->setRange(min, max);
#endif
}

void TaskBarProgress::setValue(int value)
{DD
    this->value = value;
#ifdef WIN_TASKBAR
    winProgress->setValue(value);
#endif

#ifdef Q_OS_LINUX
    QVariantMap map;
    double percent = double(value-range.first)/double(range.second-range.first);
    map.insert(QLatin1String("progress"), percent);
    map.insert(QLatin1String("progress-visible"), percent > 0.0001);
    sendMessage(map);
#endif
}

void TaskBarProgress::reset()
{DD
#ifdef WIN_TASKBAR
    winProgress->reset();
    winProgress->setVisible(false);
#endif

#ifdef Q_OS_LINUX
    QVariantMap map;
    map.insert(QLatin1String("progress-visible"), false);
    sendMessage(map);
#endif
    this->deleteLater();
}

void TaskBarProgress::finalize()
{DD
    QTimer::singleShot(500, this, SLOT(reset()));
}

#include "mactoolbar.h"
#include <QMainWindow>
#include <QToolBar>
#include <QAction>

#include "mainwindow.h"
#include "qoobarglobals.h"

class Impl
{
public:
    QToolBar *toolbar;
};

Toolbar::Toolbar(QMainWindow *parent) : QObject(parent)
{DD;
    d = new Impl;
    d->toolbar = parent->addToolBar("Files");
    d->toolbar->setIconSize(QSize(24,24));
    d->toolbar->setMovable(false);
}

Toolbar::~Toolbar()
{DD;
    delete d;
}

void Toolbar::addAction(QAction *act, const QString &key, bool showText)
{DD;
    Q_UNUSED(showText)
    Q_UNUSED(key)
    if (!act) return;

    d->toolbar->addAction(act);
}

void Toolbar::retranslateUI()
{DD;
    //do nothing
}

void Toolbar::writeState()
{DD;
    //do nothing
}

void Toolbar::addSeparator()
{DD;
    d->toolbar->addSeparator();
}

void Toolbar::attachToWindow(QMainWindow *window)
{DD;
    Q_UNUSED(window)
    //do nothing
}

void Toolbar::updateEnabled(QAction *act)
{DD;
    Q_UNUSED(act)
    //do nothing
}

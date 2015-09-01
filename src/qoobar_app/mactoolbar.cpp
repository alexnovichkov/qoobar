#include "mactoolbar.h"
#include <QMainWindow>
#include <QToolBar>
#include <QAction>

#include "mainwindow.h"


class Impl
{
public:
    QToolBar *toolbar;
};

Toolbar::Toolbar(QMainWindow *parent) : QObject(parent)
{
    d = new Impl;
    d->toolbar = parent->addToolBar("Files");
    d->toolbar->setIconSize(QSize(24,24));
    d->toolbar->setMovable(false);
}

Toolbar::~Toolbar()
{
    delete d;
}

void Toolbar::addAction(QAction *act, const QString &key, bool showText)
{
    Q_UNUSED(showText)
    Q_UNUSED(key)
    if (!act) return;

    d->toolbar->addAction(act);
}

void Toolbar::retranslateUI()
{
    //do nothing
}

void Toolbar::writeState()
{
    //do nothing
}

void Toolbar::addSeparator()
{
    d->toolbar->addSeparator();
}

void Toolbar::attachToWindow(QMainWindow *window)
{
    Q_UNUSED(window)
    //do nothing
}

void Toolbar::updateEnabled(QAction *act)
{
    Q_UNUSED(act)
    //do nothing
}

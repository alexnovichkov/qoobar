#include "mactoolbar.h"
#include <QMainWindow>
#include <QToolBar>
#include <QAction>



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

void Toolbar::addAction(QAction *act)
{
    if (!act) return;

    d->toolbar->addAction(act);
}

void Toolbar::retranslateUI()
{
    //do nothing
}

void Toolbar::addSeparator()
{
    d->toolbar->addSeparator();
}


#include "mactoolbar.h"
#include <QMainWindow>
#include <QtMacExtras>
#include <QAction>

class Impl
{
public:
    QMacToolBar *toolbar;
};

Toolbar::Toolbar(QMainWindow *parent) : QObject(parent)
{
    d = new Impl();
    d->toolbar = new QMacToolBar(parent);
}

Toolbar::~Toolbar()
{
    delete d;
}

void Toolbar::addAction(QAction *act)
{
    if (!act) return;

    QMacToolBarItem *toolBarItem = d->toolbar->addItem(act->icon(),"");
    connect(toolBarItem,SIGNAL(activated()),act,SLOT(trigger()));
}

void Toolbar::retranslateUI()
{
    //do nothing
}

void Toolbar::addSeparator()
{
    //no separators in Mac toolbar
}

void Toolbar::attachToWindow(QMainWindow *window)
{
    window->window()->winId(); // create window->windowhandle()
    d->toolbar->attachToWindow(window->window()->windowHandle());
}


#include "mactoolbar.h"
#include <QMainWindow>
#include <QtMacExtras>
#include <QAction>
#include <QHash>
#include "mainwindow.h"

class Impl
{
public:
    QMacToolBar *toolbar;
    QHash<QMacToolBarItem*, Act*> descriptions;
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

void Toolbar::addAction(QAction *act, const Act *descr)
{
    if (!act) return;

    QMacToolBarItem *toolBarItem = d->toolbar->addItem(act->icon(), descr->shortText?descr->shortText:descr->tooltip);
    d->descriptions.insert(toolBarItem,descr);
    connect(toolBarItem,SIGNAL(activated()),act,SLOT(trigger()));

    if (!act->isEnabled()) {
        toolBarItem->setDisabled();
    }
}

void Toolbar::retranslateUI()
{
    QList<QMacToolBarItem *> items = d->toolbar->items();
    Q_FOREACH(QMacToolBarItem *item, items) {
        Act *descr = d->descriptions.value(item);
        if (descr) {
            item->setText(descr->shortText?descr->shortText:descr->tooltip);
            //item->setToolTip()
        }
    }
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


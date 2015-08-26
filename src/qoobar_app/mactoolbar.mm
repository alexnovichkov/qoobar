#import <Foundation/NSString.h>
#import <AppKit/NSToolbarItem.h>

#include "mactoolbar.h"
#include <QMainWindow>
#include <QtMacExtras>
#include <QAction>
#include <QHash>
#include <QMessageBox>
#include "mainwindow.h"

class Impl
{
public:
    QMacToolBar *toolbar;
    QHash<QMacToolBarItem*, QAction*> toolbarItems;
};

Toolbar::Toolbar(QMainWindow *parent) : QObject(parent)
{
    d = new Impl();
    d->toolbar = new QMacToolBar(parent);
    NSToolbar *t = d->toolbar->nativeToolbar();
    [t setSizeMode: NSToolbarSizeModeSmall];
}

Toolbar::~Toolbar()
{
    delete d;
}

void Toolbar::addAction(QAction *act)
{
    if (!act) return;

    QMacToolBarItem *toolBarItem = d->toolbar->addItem(act->icon(),"");
    toolBarItem->setText(act->property("shortDescr").toString());
    connect(toolBarItem,SIGNAL(activated()),act,SLOT(trigger()));
    d->toolbarItems.insert(toolBarItem,act);
}

void Toolbar::retranslateUI()
{
    QList<QMacToolBarItem *> items = d->toolbar->items();
    Q_FOREACH(QMacToolBarItem *item, items) {
        QAction *act = d->toolbarItems.value(item);
        if (act) {
            item->setText(act->property("shortDescr").toString());
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

void Toolbar::updateEnabled(QAction *act)
{
//    QMacToolBarItem *item = d->toolbarItems.key(act);
//    if (item) {
//        NSToolbarItem *ns = item->nativeToolBarItem();
//        if (act->isEnabled())
//            [ns setEnabled:YES];
//        else
//            [ns setEnabled:NO];
//    }
}


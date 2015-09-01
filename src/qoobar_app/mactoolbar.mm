#import <Foundation/NSString.h>
#import <AppKit/NSToolbarItem.h>

#include "mactoolbar.h"
#include <QMainWindow>
#include <QtMacExtras>
#include <QAction>
#include <QHash>
#include <QMessageBox>
#include <QFile>
#include "mainwindow.h"
#include "applicationpaths.h"

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

//TODO: fix tollbuttons customization
void Toolbar::addAction(QAction *act, const QString &key, bool showText)
{
    if (!act) return;
    QMacToolBarItem *toolBarItem;
#if 0
    QSettings se("qoobar","gui");
    QVariantMap map = se.value("macToolbar").toMap();


    act->setProperty("key1",key);

    bool visible = map.value(key,true).toBool();

    if (visible) {
        toolBarItem = d->toolbar->addItem(act->icon(),"");
    }
    else {
        toolBarItem = d->toolbar->addAllowedItem(act->icon(),"");
    }
#else
    toolBarItem = d->toolbar->addItem(act->icon(),"");
#endif
    if (showText)
        toolBarItem->setText(act->property("shortDescr").toString());
    connect(toolBarItem,SIGNAL(activated()),act,SLOT(trigger()));

    d->toolbarItems.insert(toolBarItem,act);
}

// from QMacToolButton.cpp
QString qt_strippedText(QString s)
{
    s.remove( QString::fromLatin1("...") );
    int i = 0;
    while (i < s.size()) {
        ++i;
        if (s.at(i-1) != QLatin1Char('&'))
            continue;
        if (i < s.size() && s.at(i) == QLatin1Char('&'))
            ++i;
        s.remove(i-1,1);
    }
    return s.trimmed();
}

void Toolbar::retranslateUI()
{
    QList<QMacToolBarItem *> items = d->toolbar->items();
    Q_FOREACH(QMacToolBarItem *item, items) {
        QAction *act = d->toolbarItems.value(item);
        if (act) {
            if (!item->text().isEmpty())
                item->setText(act->property("shortDescr").toString());
            NSToolbarItem *it = item->nativeToolBarItem();
            QString s = qt_strippedText(act->text());
            [it setPaletteLabel:s.toNSString()];
            [it setToolTip:s.toNSString()];
            [it setAutovalidates:NO];
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

void Toolbar::writeState()
{
    ///TODO: fix adding actions from the palette

//    QSettings se("qoobar","gui");
//    QVariantMap map = se.value("macToolbar").toMap();
//    if (!se.isWritable()) return;

//    NSToolbar *t = d->toolbar->nativeToolbar();
//    NSArray *visible = [t visibleItems];

//    Q_FOREACH(QMacToolBarItem *item, d->toolbarItems.keys()) {
//        QString key = d->toolbarItems.value(item)->property("key1").toString();
//        NSToolbarItem *it = item->nativeToolBarItem();
//        if ([visible containsObject:it] == YES) {
//            map.insert(key,true);
//        }
//        else {
//            map.insert(key,false);
//        }
//    }

//    se.setValue("macToolbar",map);
}

void Toolbar::updateEnabled(QAction *act)
{
    Q_UNUSED(act)
    QMacToolBarItem *item = d->toolbarItems.key(act);
    if (item) {
        NSToolbarItem *ns = item->nativeToolBarItem();
        if (act->isEnabled())
            [ns setEnabled:YES];
        else
            [ns setEnabled:NO];
    }
}


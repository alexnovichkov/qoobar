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
#include "application.h"

class Impl
{
public:
    QMacToolBar *toolbar;
    QList<QAction*> addedItems;

    QMap<QAction *, QMacToolBarItem *> map;
    QHash<QString, QString> hash; // action->itemIdentifier <-> [NSToolbarItem itemIdentifier]
};

Toolbar::Toolbar(QMainWindow *parent) : QObject(parent)
{
    d = new Impl();
    d->toolbar = new QMacToolBar(parent);
    QSettings *se = App->guiSettings();

    NSToolbarSizeMode sizeMode = (NSToolbarSizeMode)se->value("macToolbarSizeMode", NSToolbarSizeModeSmall).toULongLong();
    NSToolbarDisplayMode displayMode = (NSToolbarDisplayMode)se->value("macToolbarDisplayMode", NSToolbarDisplayModeIconAndLabel).toULongLong();

    NSToolbar *t = d->toolbar->nativeToolbar();
    [t setSizeMode: sizeMode];
    [t setDisplayMode: displayMode];

    delete se;
}

Toolbar::~Toolbar()
{
    delete d;
}

//TODO: fix tollbuttons customization
void Toolbar::addAction(QAction *act, const QString &key, bool showText)
{
    if (!act) return;
    act->setProperty("itemIdentifier", key);
    act->setProperty("showText", showText);
    d->addedItems << act;
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
        NSToolbarItem *it = item->nativeToolBarItem();
        [it setAutovalidates:NO];
        QAction *act = d->map.key(item);
        if (act) {
            if (act->property("showText").toBool())
                item->setText(act->property("shortDescr").toString());
            QString s = qt_strippedText(act->text());
            [it setPaletteLabel:s.toNSString()];
            [it setToolTip:s.toNSString()];
        }
    }
}

void Toolbar::addSeparator()
{
    //no separators in Mac toolbar by default
}

QAction *findAction(QList<QAction*> &list, const QString &s)
{
    Q_FOREACH (QAction *a, list) {
        if (a->property("itemIdentifier").toString() == s) return a;
    }
    return 0;
}

void Toolbar::attachToWindow(QMainWindow *window)
{
    QSettings *se = App->guiSettings();
    QStringList macItems = se->value("macToolbarMacItems").toStringList();
    if (macItems.isEmpty()) {
        Q_FOREACH (QAction *a, d->addedItems) {
            macItems << a->property("itemIdentifier").toString();
        }
    }
    Q_FOREACH (const QString &itemIdentifier, macItems) {
        if (itemIdentifier=="NSToolbarSpaceItem") d->toolbar->addStandardItem(QMacToolBarItem::Space);
        else if (itemIdentifier=="NSToolbarFlexibleSpaceItem") d->toolbar->addStandardItem(QMacToolBarItem::FlexibleSpace);
        else {
            QAction *act = findAction(d->addedItems, itemIdentifier);
            if (act) {
                QMacToolBarItem *toolBarItem = d->toolbar->addItem(act->icon(),"");
                if (act->property("showText").toBool())
                    toolBarItem->setText(act->property("shortDescr").toString());
                d->addedItems.removeAll(act); //already added this action

                connect(toolBarItem,SIGNAL(activated()),act,SLOT(trigger()));
                d->hash.insert(itemIdentifier, QString::fromNSString([toolBarItem->nativeToolBarItem() itemIdentifier]));
                d->map.insert(act, toolBarItem);
                NSToolbarItem *it = toolBarItem->nativeToolBarItem();
                [it setAutovalidates:NO];
            }
        }
    }

    Q_FOREACH (QAction *act, d->addedItems) {
        //adding allowed items
        QMacToolBarItem *toolBarItem = d->toolbar->addAllowedItem(act->icon(),"");
        toolBarItem->setText(act->property("shortDescr").toString());
        connect(toolBarItem,SIGNAL(activated()),act,SLOT(trigger()));
        d->hash.insert(act->property("itemIdentifier").toString(),
                       QString::fromNSString([toolBarItem->nativeToolBarItem() itemIdentifier]));
        d->map.insert(act, toolBarItem);
    }

    d->toolbar->addAllowedStandardItem(QMacToolBarItem::Space);
    d->toolbar->addAllowedStandardItem(QMacToolBarItem::FlexibleSpace);
    window->window()->winId(); // create window->windowhandle()
    d->toolbar->attachToWindow(window->window()->windowHandle());
    delete se;
}

void Toolbar::writeState()
{
    QSettings *se = App->guiSettings();
    if (!se->isWritable()) {
        delete se;
        return;
    }
    NSToolbar *t = d->toolbar->nativeToolbar();
    NSToolbarSizeMode sizeMode = [t sizeMode];
    se->setValue("macToolbarSizeMode", (unsigned long long)sizeMode);

    NSToolbarDisplayMode displayMode = [t displayMode];
    se->setValue("macToolbarDisplayMode", (unsigned long long)displayMode);

    QStringList items;
    NSArray *allItems = [t items];
    for (uint i=0; i<[allItems count]; ++i) {
        NSString *label=[[allItems objectAtIndex:i] itemIdentifier];
        QString key=d->hash.key(QString::fromNSString(label));
        if (key.isEmpty()) items << QString::fromNSString(label);
        else items << key;
    }
    se->setValue("macToolbarMacItems", items);
    delete se;
}

void Toolbar::updateEnabled(QAction *act)
{
    Q_UNUSED(act)
    QMacToolBarItem *item = d->map.value(act);
    if (item) {
        NSToolbarItem *ns = item->nativeToolBarItem();
        if (act->isEnabled())
            [ns setEnabled:YES];
        else
            [ns setEnabled:NO];
    }
}


/*class Impl
{
public:
    QMacToolBar *toolbar;
    QHash<QMacToolBarItem*, QAction*> toolbarItems;
};

Toolbar::Toolbar(QMainWindow *parent) : QObject(parent)
{qDebug()<<Q_FUNC_INFO;
    d = new Impl();
    d->toolbar = new QMacToolBar(parent);
    NSToolbar *t = d->toolbar->nativeToolbar();
    [t setSizeMode: NSToolbarSizeModeSmall];
}

Toolbar::~Toolbar()
{qDebug()<<Q_FUNC_INFO;

    delete d;
}

//TODO: fix tollbuttons customization
void Toolbar::addAction(QAction *act, const QString &key, bool showText)
{qDebug()<<Q_FUNC_INFO;
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
{qDebug()<<Q_FUNC_INFO;
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
{qDebug()<<Q_FUNC_INFO;
    QList<QMacToolBarItem *> items = d->toolbar->items();
    Q_FOREACH(QMacToolBarItem *item, items) {
        NSToolbarItem *it = item->nativeToolBarItem();
        [it setAutovalidates:NO];
        QAction *act = d->toolbarItems.value(item);
        if (act) {
            if (!item->text().isEmpty())
                item->setText(act->property("shortDescr").toString());

            QString s = qt_strippedText(act->text());
            [it setPaletteLabel:s.toNSString()];
            [it setToolTip:s.toNSString()];

        }
    }
}

void Toolbar::addSeparator()
{qDebug()<<Q_FUNC_INFO;
    //no separators in Mac toolbar
}

void Toolbar::attachToWindow(QMainWindow *window)
{qDebug()<<Q_FUNC_INFO;
    window->window()->winId(); // create window->windowhandle()
    d->toolbar->attachToWindow(window->window()->windowHandle());
}

void Toolbar::writeState()
{qDebug()<<Q_FUNC_INFO;
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
{qDebug()<<Q_FUNC_INFO << act->text();
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
*/

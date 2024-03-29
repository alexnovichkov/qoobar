#include "mactoolbar.h"
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QtDebug>
#include <QSettings>
#include <QtWidgets>
#include <QContextMenuEvent>
#include "application.h"
#include "mainwindow.h"
#include "qoobarglobals.h"
#include "impl.h"
#include "enums.h"

QAction *findAction(const QList<QAction*> &list, const QString &s)
{
    Q_FOREACH (QAction *a, list) {
        if (a->property("itemIdentifier").toString() == s) return a;
    }
    return 0;
}


Impl::Impl(QWidget * parent) : QToolBar(parent)
{
    
}

void Impl::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu m;

    Q_FOREACH(QAction *a, addedItems) {
        QAction *act = new QAction(a->icon(), a->text(), this);
        act->setCheckable(true);
        act->setChecked(a->property("visible").toBool());
        act->setProperty("itemIdentifier", a->property("itemIdentifier"));
        connect(act, SIGNAL(toggled(bool)), this, SLOT(toggle(bool)));
        m.addAction(act);
    }

    m.exec(event->globalPos());
}

void Impl::toggle(bool on)
{
    QAction *a = static_cast<QAction*>(sender());
    if (a) {
        QAction *act = findAction(addedItems, a->property("itemIdentifier").toString());
        if (act) act->setProperty("visible", on);
    }
}


Toolbar::Toolbar(QMainWindow *parent) : QObject(parent), d{new Impl(parent)}
{DD;
    parent->addToolBar(d);
    d->setIconSize(QSize(LARGE_ICON_SIZE, LARGE_ICON_SIZE));
    d->setMovable(false);
    d->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

Toolbar::~Toolbar()
{DD;
   
}

void Toolbar::addAction(QAction *act, const QString &key, bool showText)
{DD;
    Q_UNUSED(showText)
    if (!act) return;

    auto *a = new QAction(act->icon(),act->text(),this);
    a->setText(act->text());
    a->setToolTip(act->toolTip());
    a->setEnabled(act->isEnabled());

    connect(a,SIGNAL(triggered()),act,SLOT(trigger()));
    a->setProperty("itemIdentifier", key);

    d->hash.insert(act, a);

    d->addedItems << a;
}

void Toolbar::retranslateUI()
{DD;
    Q_FOREACH(QAction *a, d->addedItems) {
        QAction *act = d->hash.key(a);
        if (act) {
            a->setText(act->text());
            a->setToolTip(act->toolTip());
            a->setEnabled(act->isEnabled());
        }
    }
}

void Toolbar::writeState()
{DD;
    QSettings *se = App->guiSettings();
    QStringList items;
    Q_FOREACH(QAction *a, d->addedItems) {
        if (a->isVisible())
            items << a->property("itemIdentifier").toString();
    }
    se->setValue("macToolbarItems", items);

    delete se;
}

void Toolbar::addSeparator()
{DD;
    static int s=0;
    QAction *a = new QAction(this);
    a->setSeparator(true);
    a->setProperty("itemIdentifier",QString("separator%1").arg(s++));
    d->addedItems << a;
}

void Toolbar::attachToWindow(QMainWindow *window)
{DD;
    Q_UNUSED(window)
    QSettings *se = App->guiSettings();

    QStringList items = se->value("macToolbarItems").toStringList();
    if (items.isEmpty()) {
        Q_FOREACH (QAction *a, d->addedItems) {
            items << a->property("itemIdentifier").toString();
        }
    }

    Q_FOREACH(QAction *a, d->addedItems) {
        d->addAction(a);
        if (items.contains(a->property("itemIdentifier").toString())) a->setVisible(true);
        else a->setVisible(false);
    }

    delete se;
}

void Toolbar::detachFromWindow()
{

}

void Toolbar::updateEnabled(QAction *act)
{DD;
    QAction *a = d->hash.value(act);
    if (a) {
        a->setEnabled(act->isEnabled());
    }
}

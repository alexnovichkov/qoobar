#ifndef IMPL_H
#define IMPL_H

#include <QToolBar>

class Impl : public QToolBar
{
    Q_OBJECT
public:
    explicit Impl(QWidget * parent = 0);
protected:
    void contextMenuEvent(QContextMenuEvent * event);
private Q_SLOTS:
    void toggle(bool);
public:
    QList<QAction*> addedItems;
    QHash<QAction*, QAction*> hash; //program action <-> toolbar action
};

#endif // IMPL_H


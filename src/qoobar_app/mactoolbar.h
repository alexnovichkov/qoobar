#ifndef MACTOOLBAR_H
#define MACTOOLBAR_H

#include <QObject>
class QMainWindow;
class QAction;
class Impl;
#include <QToolBar>
#include <QHash>

class Toolbar : public QObject
{
    Q_OBJECT
public:
    explicit Toolbar(QMainWindow *parent = 0);
    ~Toolbar();
    void addAction(QAction *act, const QString &key, bool showText=true);
    void retranslateUI();
    void addSeparator();
    void attachToWindow(QMainWindow *window);
    void writeState();
public Q_SLOTS:
    void updateEnabled(QAction *act);
private:
    Impl *d;
};

class CustomToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit CustomToolBar(Impl *impl, QWidget * parent = 0);
protected:
    void contextMenuEvent(QContextMenuEvent * event);
private Q_SLOTS:
    void toggle(bool);
private:
    Impl *impl;
    QHash<QAction *,QAction*> hash;
};

#endif // MACTOOLBAR_H

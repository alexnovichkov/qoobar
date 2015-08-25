#ifndef MACTOOLBAR_H
#define MACTOOLBAR_H

#include <QObject>
class QMainWindow;
class QAction;
class Impl;
class Act;

class Toolbar : public QObject
{
    Q_OBJECT
public:
    explicit Toolbar(QMainWindow *parent = 0);
    ~Toolbar();
    void addAction(QAction *act, const Act *descr=0);
    void retranslateUI();
    void addSeparator();
    void attachToWindow(QMainWindow *window);
private:
    Impl *d;
};

#endif // MACTOOLBAR_H

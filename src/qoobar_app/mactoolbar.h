#ifndef MACTOOLBAR_H
#define MACTOOLBAR_H

#include <QObject>
class QMainWindow;
class QAction;
class Impl;

class Toolbar : public QObject
{
    Q_OBJECT
public:
    explicit Toolbar(QMainWindow *parent = 0);
    ~Toolbar();
    void addAction(QAction *act);
    void retranslateUI();
    void addSeparator();
    void attachToWindow(QMainWindow *window);
public Q_SLOTS:
    void updateEnabled(QAction *act);
private:
    Impl *d;
};

#endif // MACTOOLBAR_H

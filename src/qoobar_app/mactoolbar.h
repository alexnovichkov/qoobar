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
    void addAction(QAction *act, const QString &key, bool showText=true);
    void retranslateUI();
    void addSeparator();
    void attachToWindow(QMainWindow *window);
    void detachFromWindow();
    void writeState();
public Q_SLOTS:
    void updateEnabled(QAction *act);
private:
    Impl *d;
};

#endif // MACTOOLBAR_H

#ifndef TASKBARPROGRESS_H
#define TASKBARPROGRESS_H

#include <QObject>
#include <QPair>

class QWidget;
class QWinTaskbarProgress;

class TaskBarProgress : public QObject
{
    Q_OBJECT
public:
    TaskBarProgress(QWidget *window, QObject *parent = 0);

public Q_SLOTS:
    void setRange(int min, int max);

    void setValue(int value);

    void reset();

    void finalize();

//Q_SIGNALS:


private:
    QWidget *window;
    QWinTaskbarProgress *winProgress;
    QPair<int, int> range;
    int value;
};

#endif // TASKBARPROGRESS_H

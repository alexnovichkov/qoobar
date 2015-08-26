#ifndef MACSPLITTER_H
#define MACSPLITTER_H

#include <QSplitter>
#include <QSplitterHandle>
#include <QCursor>

class MacSplitter : public QSplitter
{
Q_OBJECT
public:
    MacSplitter(Qt::Orientation orientation, QWidget * parent = 0) : QSplitter(orientation, parent) {}
    QSplitterHandle *createHandle();
};

class MacSplitterHandle : public QSplitterHandle
{
Q_OBJECT
public:
    MacSplitterHandle(Qt::Orientation orientation, QSplitter *parent);
    void paintEvent(QPaintEvent *);
    QSize sizeHint() const;
protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
};

#endif // MACSPLITTER_H

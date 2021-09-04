#ifndef MACSPLITTER_H
#define MACSPLITTER_H

#include <QSplitter>
#include <QSplitterHandle>
#include <QCursor>

class QEnterEvent;

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
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void enterEvent(QEnterEvent *event);
#else
    void enterEvent(QEvent *event);
#endif
    void leaveEvent(QEvent *e);
};

#endif // MACSPLITTER_H

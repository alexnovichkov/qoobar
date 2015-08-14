#ifndef MACSPLITTER_H
#define MACSPLITTER_H

#include <QSplitter>
#include <QSplitterHandle>

class MacSplitter : public QSplitter
{
Q_OBJECT
public:
    QSplitterHandle *createHandle();

};

class MacSplitterHandle : public QSplitterHandle
{
Q_OBJECT
public:
    MacSplitterHandle(Qt::Orientation orientation, QSplitter *parent);
    void paintEvent(QPaintEvent *);
    QSize sizeHint() const;
};

#endif // MACSPLITTER_H

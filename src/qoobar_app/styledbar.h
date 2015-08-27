#ifndef STYLEDBAR_H
#define STYLEDBAR_H

#include <QWidget>
#include <QToolButton>

class StyledBar : public QWidget
{
    Q_OBJECT
public:
    StyledBar(QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *event);
};

class StyledSeparator : public QWidget
{
    Q_OBJECT
public:
    StyledSeparator(QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *event);
};

class FancyToolButton : public QToolButton
{
    Q_OBJECT
public:
    FancyToolButton(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
};

#endif // STYLEDBAR_H


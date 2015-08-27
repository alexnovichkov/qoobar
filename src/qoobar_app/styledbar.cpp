#include "styledbar.h"

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <QPainter>
#include <QStyleOption>

StyledBar::StyledBar(QWidget *parent)
    : QWidget(parent)
{
}
void StyledBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QStyleOptionToolBar option;
    option.rect = rect();
    option.state = QStyle::State_Horizontal;
    style()->drawControl(QStyle::CE_ToolBar, &option, &painter, this);
}

StyledSeparator::StyledSeparator(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(10);
}

void StyledSeparator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QStyleOption option;
    option.rect = rect();
    option.state = QStyle::State_Horizontal;
    option.palette = palette();
    style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &option, &painter, this);
}

FancyToolButton::FancyToolButton(QWidget *parent)
    : QToolButton(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void FancyToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);

    QRect iconRect(0,0,iconSize().width(),iconSize().height());
    iconRect.moveCenter(rect().center());
    QPixmap px = icon().pixmap(iconRect.size());

    if (isDown()) {
        QLinearGradient grad(rect().topLeft(), rect().bottomLeft());
        grad.setColorAt(0, Qt::transparent);
        grad.setColorAt(1, Qt::lightGray);
        painter.fillRect(rect(),grad);
    }

    painter.drawPixmap(iconRect, px);
}

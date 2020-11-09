#include "styledbar.h"

#ifdef Q_OS_MAC
#include <QtWidgets>

#include <QPainter>
#include <QStyleOption>
#include "qoobarglobals.h"
#include "application.h"

StyledBar::StyledBar(QWidget *parent)
    : QWidget(parent)
{DD;
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
{DD;
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
{DD;
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
#endif

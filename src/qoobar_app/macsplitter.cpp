#include "macsplitter.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "qoobarglobals.h"

MacSplitterHandle::MacSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
: QSplitterHandle(orientation, parent)
{DD

}

// Paint the horizontal handle as a gradient, paint
// the vertical handle as a line.
void MacSplitterHandle::paintEvent(QPaintEvent *)
{DD
    QPainter painter(this);
    if (orientation() == Qt::Vertical) {
        QColor topColor(145, 145, 145);
        QColor bottomColor(142, 142, 142);
        QColor gradientStart(252, 252, 252);
        QColor gradientStop(223, 223, 223);
        painter.setPen(topColor);
        painter.drawLine(0, 0, width(), 0);
        painter.setPen(bottomColor);
        painter.drawLine(0, height() - 1, width(), height() - 1);

        QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, height() -3));
        linearGrad.setColorAt(0, gradientStart);
        linearGrad.setColorAt(1, gradientStop);
        painter.fillRect(QRect(QPoint(0,1), size() - QSize(0, 2)), QBrush(linearGrad));
    } else
    {
        QColor topColor(0, 0, 0);
        painter.setPen(topColor);
        painter.drawLine(0, 0, 0, height());
    }
}

QSize MacSplitterHandle::sizeHint() const
{DD
    QSize parent = QSplitterHandle::sizeHint();
    if (orientation() == Qt::Vertical)
        return parent + QSize(0, 3);
    else
        return QSize(1, parent.height());
}

QSplitterHandle *MacSplitter::createHandle()
{DD
    return new MacSplitterHandle(orientation(), this);
}

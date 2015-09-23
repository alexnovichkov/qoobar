/**
 * \file imagedialog.cpp
 * This dialog shows full-sized cover art picture in a separate window
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 11 Aug 2011
 *
 * Copyright (C) 2011  Alex Novichkov
 *
 * This file is part of Qoobar.
 *
 * Qoobar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Qoobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "imagedialog.h"

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "enums.h"
#include "application.h"
#include "qoobarglobals.h"

void ClickableLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button()==Qt::LeftButton)
        Q_EMIT clicked();
    QLabel::mouseReleaseEvent(ev);
}

void ClickableLabel::enterEvent(QEvent *event)
{
    QLabel::enterEvent(event);
    if (pixmap()) {
        if (cursor.isNull()) cursor = QPixmap(App->iconThemeIcon("zoom-in.png"));
        setCursor(cursor);
    }
}

void ClickableLabel::leaveEvent(QEvent *event)
{
    QLabel::leaveEvent(event);
    setCursor(Qt::ArrowCursor);
}

ImageDialog::ImageDialog(QWidget *parent) :
    QDialog(parent)
{DD;
    setWindowTitle(tr("Cover Art"));
    label = new ClickableLabel();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->addWidget(label);
    setLayout(vbox);
    connect(label, SIGNAL(clicked()), SLOT(close()));
}

void ImageDialog::setImage(const QPixmap &data,const QString &toolTip)
{DD;
    setWindowTitle(QString("%1, %2").arg(windowTitle()).arg(toolTip));
    QPixmap pixmap=data;

    QRect screenSize = qApp->desktop()->availableGeometry();
    int w=pixmap.width(); int h=pixmap.height();
    int W=screenSize.width(); int H=screenSize.height();
    if (w>W) {
        if (h>H) pixmap=pixmap.scaled(screenSize.size());
        else pixmap=pixmap.scaledToWidth(W);
    }
    else
        if (h>H) pixmap=pixmap.scaledToHeight(H);

    setMaximumSize(pixmap.size());
    setMinimumSize(pixmap.size());
    label->setPixmap(pixmap);
    resize(pixmap.size());
}

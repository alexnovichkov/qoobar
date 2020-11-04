/**
 * \file imagedialog.h
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

#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

#include <QDialog>
#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent=0)
        : QLabel(parent) {}
Q_SIGNALS:
    void clicked();
protected:
    void mouseReleaseEvent(QMouseEvent *ev);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
};



class ImageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImageDialog(QWidget *parent = 0);
    void setImage(const QPixmap &,const QString &);
private:
    ClickableLabel *label;
};

#endif // IMAGEDIALOG_H

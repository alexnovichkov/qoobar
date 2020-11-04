/**
 * \file imagebox.h
 * A widget that encapsulates operations with cover art pictures
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

#ifndef IMAGEBOX_H
#define IMAGEBOX_H

#include <QWidget>
#include <QBoxLayout>
#include "coverimage.h"

class QPushButton;
class QLabel;
class ClickableLabel;
class QAction;
class QToolButton;
class QGraphicsOpacityEffect;

//stolen from Qt Creator src
class FadingPanel : public QWidget
{
public:
    FadingPanel(QWidget *parent = 0);
    void fadeTo(float value);
protected:
    QGraphicsOpacityEffect *m_opacityEffect;
};

class Panel : public QWidget
{
    Q_OBJECT
public:
    enum PanelType{
        PanelText,
        PanelImage,
        PanelMixed
    };
    Panel(PanelType type, QWidget *parent = 0);
    void setText(QString text, bool clip=false);
    void setPixmap(QPixmap,const QString &);
    void setType(PanelType type) {this->type = type;}
    void addWidget(QWidget *);
    void clear();
protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *paintEvent);
private Q_SLOTS:
    void showFullImage();
private:
    void changeHoverState(bool hovered);
    QPixmap cacheBackground(const QSize &size);
    ClickableLabel *label;
    FadingPanel *panel;
    bool empty;
    QPixmap backgroundPixmap;
    QPixmap image;
    PanelType type = PanelMixed;
};


class ImageBox : public QWidget
{
    Q_OBJECT
public:
    explicit ImageBox(QWidget *parent = 0);
    void retranslateUi();
Q_SIGNALS:
    void imageChanged(const CoverImage &,const QString &);
public Q_SLOTS:
    void updatePicture(const QString &);
    void updatePicture(const CoverImage &, const QString &);
    void update(bool selected);
    void clear();
private Q_SLOTS:
    void addImage();
    void removeImage();
    void saveImage();

    void cutImage();
    void copyImage();
    void pasteImage();
    void resizeImage();

    void changeType();
    void changeDescription();
private:
    QToolButton *makeButton(QAction *act, const char *slot, const QString &icon);
    QPushButton *makeButton(const char *slot, const QString &icon);
    QAction *makeAction(const char *slot, const QString &icon);

    QPushButton *typeButton;
    QPushButton *descriptionButton;

    QAction *addImageAct;
    QAction *removeImageAct;
    QAction *saveImageAct;

    QAction *copyAct;
    QAction *cutAct;
    QAction *pasteAct;
    QAction *resizeAct;

    QAction *typeAct;
    QAction *descriptionAct;


    CoverImage image;
    QString filePath;



    Panel *typePanel;
    Panel *descriptionPanel;
    Panel *imagePanel;

    QStringList mp3CoverTypes;
};

#endif // IMAGEBOX_H

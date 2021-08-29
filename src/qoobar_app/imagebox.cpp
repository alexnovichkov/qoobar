/**
 * \file imagebox.cpp
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

#include "imagebox.h"
#include "imagedialog.h"
#include "application.h"
#include "imageresizedialog.h"

#include <QtWidgets>

#include "enums.h"
#include "qoobarglobals.h"

//#ifdef Q_OS_MAC
#define IMAGEBOX_TOOLBAR
//#endif

Panel::Panel(PanelType type, QWidget *parent) : QWidget(parent), type(type)
{DD
    label = new ClickableLabel(this);
    connect(label,SIGNAL(clicked()),SLOT(showFullImage()));
    //TODO: this->devicePixelRatio()
    label->setContentsMargins(5,5,0,5);
    panel = new FadingPanel(this);
    QHBoxLayout *layout = new QHBoxLayout;

    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(panel);
    setLayout(layout);
    clear();
}

void Panel::showFullImage()
{DD
    if (empty || image.isNull()) return;

    ImageDialog *dialog = new ImageDialog(this);
    dialog->setImage(image,toolTip());
    dialog->exec();
}

void Panel::setText(QString text, bool clip/*=false*/)
{DD;
    if (type == PanelImage) return;
    empty = false;
    image = QPixmap();
    if (clip) text=this->fontMetrics().elidedText(text,Qt::ElideRight,150);
    label->setText(text);
}

void Panel::setPixmap(QPixmap p, const QString &toolTip)
{DD;
    if (type == PanelText) return;
    setToolTip(toolTip);
    label->setText(QString());
    empty=false;
    image=p;
    //TODO: this->devicePixelRatio()
    int max=qMax(p.width(),p.height());
    if (max>150) {
        if (p.width()==max) p=p.scaledToWidth(150);
        else p=p.scaledToHeight(150);
    }
    label->setPixmap(p);
}

void Panel::clear()
{DD
    empty=true;
    label->clear();
    setToolTip("");
    if (type != PanelText) {
        label->setPixmap(QIcon::fromTheme("view-preview").pixmap(64,64));
        setToolTip(tr("No image"));
    }
}

void Panel::enterEvent(QEnterEvent * event)
{//DD
    QWidget::enterEvent(event);
//#ifndef Q_OS_MAC
    changeHoverState(true);
//#endif
}

void Panel::leaveEvent(QEvent * event)
{//DD
    QWidget::leaveEvent(event);
//#ifndef Q_OS_MAC
    changeHoverState(false);
//#endif
}

void Panel::paintEvent(QPaintEvent *paintEvent)
{//DD
    QWidget::paintEvent(paintEvent);
//#ifndef Q_OS_MAC
    QPainter p(this);

    QPoint topLeft(label->geometry().left(), contentsRect().top());
    const QRect paintArea(topLeft, contentsRect().bottomRight());

    if (backgroundPixmap.isNull() || backgroundPixmap.size() != size())
        backgroundPixmap = cacheBackground(paintArea.size());
    p.drawPixmap(paintArea, backgroundPixmap);
//#endif
}

QPixmap Panel::cacheBackground(const QSize &size)
{DD
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);

//    QRect fullRect(0, 0, size.width(), size.height());
//    p.fillRect(fullRect, QColor(255, 255, 255, 40));

//    QLinearGradient lg(fullRect.topLeft(), fullRect.bottomLeft());
//    lg.setColorAt(0, QColor(255, 255, 255, 130));
//    lg.setColorAt(1, QColor(255, 255, 255, 0));
//    p.fillRect(fullRect, lg);
//    p.setRenderHint(QPainter::Antialiasing, true);
//    p.translate(0.5, 0.5);
//    p.setPen(QColor(0, 0, 0, 40));
//    p.setBrush(Qt::NoBrush);
//    p.drawRoundedRect(fullRect.adjusted(0, 0, -1, -1), 2, 2);
//    p.setPen(QColor(255,255,255,140));
//    p.drawRoundedRect(fullRect.adjusted(1, 1, -2, -2), 2, 2);

    QRect fullRect(0, 0, size.width(), size.height());
    p.setPen(QColor(40,40,40,40));
    p.drawRect(fullRect.adjusted(1, 1, -1, -1));

    return pixmap;
}

void Panel::changeHoverState(bool hovered)
{DD
    panel->fadeTo(hovered ? 1.0 : 0);
}

void Panel::addWidget(QWidget *w)
{DD
    panel->layout()->addWidget(w);
}

FadingPanel::FadingPanel(QWidget *parent) :
    QWidget(parent)
{DD
//#ifndef Q_OS_MAC
    m_opacityEffect=new QGraphicsOpacityEffect;
    m_opacityEffect->setOpacity(0);
    setGraphicsEffect(m_opacityEffect);
    QPalette pal;
    pal.setBrush(QPalette::All, QPalette::Window, Qt::transparent);
    setPalette(pal);
//#endif
    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0,0,0,0);
    l->addStretch();
    setLayout(l);
}

void FadingPanel::fadeTo(float value)
{DD
    QPropertyAnimation *animation = new QPropertyAnimation(m_opacityEffect, "opacity");
    animation->setDuration(200);
    animation->setEndValue(value);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

ImageBox::ImageBox(QWidget *parent) : QWidget(parent)
{DD
    typePanel = new Panel(Panel::PanelText, this);
    descriptionPanel = new Panel(Panel::PanelText, this);
    imagePanel = new Panel(Panel::PanelMixed, this);

#ifdef IMAGEBOX_TOOLBAR
    addImageAct = makeAction(SLOT(addImage()), "list-add");
    removeImageAct = makeAction(SLOT(removeImage()), "list-remove");
    saveImageAct = makeAction(SLOT(saveImage()), "document-save");

    addImageAct->setEnabled(true);

    typeButton = makeButton(SLOT(changeType()),"TextEdit");
    descriptionButton = makeButton(SLOT(changeDescription()),"TextEdit");
    copyAct = makeAction(SLOT(copyImage()), "edit-copy");
    cutAct = makeAction(SLOT(cutImage()), "edit-cut");
    pasteAct = makeAction(SLOT(pasteImage()), "edit-paste");
    resizeAct = makeAction(SLOT(resizeImage()),"image-resize");
    typePanel->addWidget(typeButton);
    descriptionPanel->addWidget(descriptionButton);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));
    toolBar->addAction(addImageAct);
    toolBar->addAction(removeImageAct);
    toolBar->addAction(saveImageAct);
    toolBar->addSeparator();
    toolBar->addAction(cutAct);
    toolBar->addAction(copyAct);
    toolBar->addAction(pasteAct);
    toolBar->addAction(resizeAct);
#else
    addImageAct = new QAction(this);
    removeImageAct = new QAction(this);
    saveImageAct = new QAction(this);
    QToolButton *addImageButton = makeButton(addImageAct,SLOT(addImage()),"list-add");
    QToolButton *removeImageButton = makeButton(removeImageAct,SLOT(removeImage()),"list-remove");
    QToolButton *saveImageButton = makeButton(saveImageAct,SLOT(saveImage()),"document-save");


    addImageAct->setEnabled(false);

    typeAct = new QAction(this);
    descriptionAct = new QAction(this);
    cutAct = new QAction(this);
    copyAct = new QAction(this);
    pasteAct = new QAction(this);
    resizeAct = new QAction(this);

    typePanel->addWidget(makeButton(typeAct, SLOT(changeType()),"TextEdit"));
    descriptionPanel->addWidget(makeButton(descriptionAct, SLOT(changeDescription()),
                                           "TextEdit"));
    imagePanel->addWidget(makeButton(cutAct, SLOT(cutImage()),
                                     "edit-cut"));
    imagePanel->addWidget(makeButton(copyAct, SLOT(copyImage()),
                                     "edit-copy"));
    imagePanel->addWidget(makeButton(pasteAct, SLOT(pasteImage()),
                                     "edit-paste"));
    imagePanel->addWidget(makeButton(resizeAct, SLOT(resizeImage()),
                                     "image-resize"));
#endif

    QGridLayout *imageLayout=new QGridLayout;
#ifdef IMAGEBOX_TOOLBAR
    imageLayout->setContentsMargins(0,3,3,3);
//    imageLayout->setMargin(0);
    imageLayout->setVerticalSpacing(0);
    imageLayout->setMenuBar(toolBar);
    imageLayout->addWidget(typePanel,0,0);
    imageLayout->addWidget(descriptionPanel,1,0);
    imageLayout->addWidget(imagePanel,2,0,1,1,Qt::AlignTop);

    imageLayout->setRowStretch(0,1);
    imageLayout->setRowStretch(1,1);
    imageLayout->setRowStretch(2,20);
//    setFixedWidth(220);
    //setFixedWidth(toolBar->contentsRect().width());
#else
    imageLayout->setContentsMargins(0,0,0,0);
    imageLayout->setVerticalSpacing(0);
    imageLayout->addWidget(typePanel,0,0,1,3);
    imageLayout->addWidget(descriptionPanel,1,0,1,3);
    imageLayout->addWidget(imagePanel,2,0,1,3,Qt::AlignTop);
    imageLayout->addWidget(addImageButton,3,0,1,1,Qt::AlignCenter);
    imageLayout->addWidget(removeImageButton,3,1,1,1,Qt::AlignCenter);
    imageLayout->addWidget(saveImageButton,3,2,1,1,Qt::AlignCenter);


    imageLayout->setRowStretch(0,1);
    imageLayout->setRowStretch(1,1);
    imageLayout->setRowStretch(2,20);
    imageLayout->setRowStretch(3,1);
#endif

    setLayout(imageLayout);
}

void ImageBox::retranslateUi()
{DD
#ifdef IMAGEBOX_TOOLBAR
    typeButton->setToolTip(tr("Edit type"));
    descriptionButton->setToolTip(tr("Edit description"));
#else
    typeAct->setText(tr("Edit type"));
    descriptionAct->setText(tr("Edit description"));
#endif
    addImageAct->setText(tr("Add an image"));
    removeImageAct->setText(tr("Remove the image"));
    saveImageAct->setText(tr("Export the image"));

    cutAct->setText(tr("Cut"));
    copyAct->setText(tr("Copy"));
    pasteAct->setText(tr("Paste"));
    resizeAct->setText(tr("Resize"));

    mp3CoverTypes.clear();
    mp3CoverTypes << tr("Other");
    mp3CoverTypes << tr("File Icon");
    mp3CoverTypes << tr("Other File Icon");
    mp3CoverTypes << tr("Front Cover");
    mp3CoverTypes << tr("Back Cover");
    mp3CoverTypes << tr("Leaflet Page");
    mp3CoverTypes << tr("Media");
    mp3CoverTypes << tr("Lead Artist");
    mp3CoverTypes << tr("Artist");
    mp3CoverTypes << tr("Conductor");
    mp3CoverTypes << tr("Band");
    mp3CoverTypes << tr("Composer");
    mp3CoverTypes << tr("Lyricist");
    mp3CoverTypes << tr("Recording Location");
    mp3CoverTypes << tr("During Recording");
    mp3CoverTypes << tr("During Performance");
    mp3CoverTypes << tr("Movie Screen Capture");
    mp3CoverTypes << tr("Coloured Fish");
    mp3CoverTypes << tr("Illustration");
    mp3CoverTypes << tr("Band Logo");
    mp3CoverTypes << tr("Publisher Logo");

    updatePicture(image,filePath);
}

void ImageBox::changeType()
{DD
    if (image.pixmap().isEmpty()) return;
    int type = image.type();
    QString item = QInputDialog::getItem(this,tr("Qoobar"),
                                         tr("Cover art type"),
                                         mp3CoverTypes,type,false);
    if (item.isEmpty()) return;
    type = mp3CoverTypes.indexOf(item);

    if (image.type()!=type) {
        CoverImage img=image;
        img.setType(type);
        Q_EMIT imageChanged(img,tr("changing cover art"));
    }
}

void ImageBox::changeDescription()
{DD
    if (image.pixmap().isEmpty()) return;
    QString description = image.description();

    bool ok;
    QString item = QInputDialog::getText(this,tr("Qoobar"),
                                         tr("Cover art description"),QLineEdit::Normal,
                                         description,&ok);
    if (description!=item && ok) {
        CoverImage img=image;
        img.setDescription(item);
        Q_EMIT imageChanged(img,tr("changing cover art"));
    }
}

void ImageBox::addImage()
{DD
    //get the folder by the first selected file
    QString filename=QFileDialog::getOpenFileName(this,tr("Adding cover art"),filePath,
                                                  tr("Image files (*.png *.jpg *.jpeg)"));
    if (filename.isEmpty()) return;


    QFile f(filename);
    QFileInfo fi(filename);
    f.open(QFile::ReadOnly);
    QByteArray newPix=f.readAll();
    if (!newPix.isEmpty()) {
        CoverImage img(newPix,3,QSL("image/png"),fi.fileName());
        if (filename.endsWith(QLS("jpg"),Qt::CaseInsensitive)
                || filename.endsWith(QLS("jpeg"),Qt::CaseInsensitive))
            img.setMimetype(QSL("image/jpeg"));

        Q_EMIT imageChanged(img,tr("adding cover art"));
    }
}

void ImageBox::removeImage()
{DD
    CoverImage img;
    Q_EMIT imageChanged(img,tr("removing cover art"));
}

QToolButton *ImageBox::makeButton(QAction *act, const char *slot, const QString &icon)
{DD
    connect(act,SIGNAL(triggered()),slot);
    QToolButton *button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setDefaultAction(act);
    button->setIconSize(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));
    act->setIcon(QIcon::fromTheme(icon));
    act->setEnabled(false);
    return button;
}

QPushButton *ImageBox::makeButton(const char *slot, const QString &icon)
{DD
    QPushButton *button = new QPushButton(this);
    connect(button,SIGNAL(clicked()),slot);

    button->setFlat(true);
    button->setMaximumSize(MEDIUM_ICON_SIZE, MEDIUM_ICON_SIZE);
    button->setIcon(QIcon::fromTheme(icon));
    button->setEnabled(false);
    return button;
}

QAction *ImageBox::makeAction(const char *slot, const QString &icon)
{DD
    QAction *act = new QAction(this);
    connect(act,SIGNAL(triggered()),slot);
    act->setIcon(QIcon::fromTheme(icon));
    act->setEnabled(false);
    return act;
}

void ImageBox::saveImage()
{DD
    if (!image.pixmap().isNull()) {
        QString filter;
        QString filename=QFileDialog::getSaveFileName(this,
                         tr("Exporting cover art"),
                         QFileInfo(filePath).path(),
                         tr("JPEG files (*.jpg *.jpeg);;PNG files (*.png);;BMP files (*.bmp);;XPM files (*.xpm)"),
                         &filter);
        if (!filename.isEmpty()) {
            if (filter.isEmpty())
                filename.append(".png");
            QImage img;
            img.loadFromData(image.pixmap());
            img.save(filename);
        }
    }
}

void ImageBox::copyImage()
{DD
    App->imageBuffer = image;
    pasteAct->setEnabled(true);
}

void ImageBox::cutImage()
{DD
    copyImage();
    removeImage();
}

void ImageBox::pasteImage()
{DD
    Q_EMIT imageChanged(App->imageBuffer,tr("changing cover art"));
}

void ImageBox::resizeImage()
{DD
    ImageResizeDialog dialog(image);

    if (dialog.exec()) {
        QImage x;
        if (!x.loadFromData(image.pixmap())) return;
        if (x.width()==dialog.newWidth && x.height()==dialog.newHeight) return;
        x=x.scaled(dialog.newWidth, dialog.newHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QTemporaryFile tf("XXXXXX.jpg");
        if (tf.open()) {
            x.save(&tf);
            tf.close();
            tf.open();
            image.setPixmap(tf.readAll());
            Q_EMIT imageChanged(image, tr("resizing cover art"));
        }
    }
}

void ImageBox::clear()
{DD
    imagePanel->clear();
    image.clear();
    filePath.clear();
    typePanel->clear();
    descriptionPanel->clear();

#ifdef IMAGEBOX_TOOLBAR
    descriptionButton->setEnabled(false);
    typeButton->setEnabled(false);
#else
    descriptionAct->setEnabled(false);
    typeAct->setEnabled(false);
#endif
    removeImageAct->setEnabled(false);
    saveImageAct->setEnabled(false);
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    pasteAct->setEnabled(App->imageBuffer != CoverImage());
    resizeAct->setEnabled(false);
}

void ImageBox::updatePicture(const QString &text)
{DD
    imagePanel->setText(text);
}

void ImageBox::updatePicture(const CoverImage &img, const QString &filename)
{DD
    image=img;
    filePath=filename;
    if (!img.pixmap().isNull()) {
        QPixmap x;
        x.loadFromData(img.pixmap());

        QStringList l=img.mimetype().split(QSL("/"));
        QString tooltip=QString("%1x%2").arg(x.width()).arg(x.height());
        if (l.size()>1)
            tooltip.prepend(QString("%1, ").arg(l.at(1).toUpper()));

        typePanel->setText(mp3CoverTypes.value(img.type()));
        descriptionPanel->setText(img.description(),true);
        imagePanel->setPixmap(x,tooltip);
#ifdef IMAGEBOX_TOOLBAR
        descriptionButton->setEnabled(true);
        typeButton->setEnabled(true);
#else
        descriptionAct->setEnabled(true);
        typeAct->setEnabled(true);
#endif
        removeImageAct->setEnabled(true);
        saveImageAct->setEnabled(true);
        cutAct->setEnabled(true);
        copyAct->setEnabled(true);
        pasteAct->setEnabled(App->imageBuffer != CoverImage());
        resizeAct->setEnabled(true);
    }
}

void ImageBox::update(bool selected)
{DD;
    addImageAct->setEnabled(selected);
}


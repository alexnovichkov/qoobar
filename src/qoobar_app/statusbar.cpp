#include "statusbar.h"
#include "tagger.h"
#include "qoobarglobals.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "application.h"

/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include <QFontMetrics>
#include <QPainter>
#include <QStyle>

ElidingLabel::ElidingLabel(QWidget *parent)
    : QLabel(parent), m_elideMode(Qt::ElideRight)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred, QSizePolicy::LineEdit));
}

ElidingLabel::ElidingLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent), m_elideMode(Qt::ElideRight)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred, QSizePolicy::LineEdit));
}

Qt::TextElideMode ElidingLabel::elideMode() const
{
    return m_elideMode;
}

void ElidingLabel::setElideMode(const Qt::TextElideMode &elideMode)
{
    m_elideMode = elideMode;
    update();
}

void ElidingLabel::paintEvent(QPaintEvent *e)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    const int m = margin();
    QRect contents = contentsRect().adjusted(m, m, -m, -m);
    QFontMetrics fm = fontMetrics();
    QString txt = text();
    if (txt.length() > 4 && fm.width(txt) > contents.width()) {
        setToolTip(txt);
        txt = fm.elidedText(txt, m_elideMode, contents.width());
        int flags = QStyle::visualAlignment(layoutDirection(), alignment()) | Qt::TextSingleLine;

        QPainter painter(this);
        drawFrame(&painter);

        if (selectedText()==text()) {
            painter.fillRect(contents, palette().highlight());
            painter.setPen(palette().color(QPalette::HighlightedText));
        }
        painter.drawText(contents, flags, txt);
    } else {
        setToolTip(QString());
        QLabel::paintEvent(e);
    }
#else
    QLabel::paintEvent(e);
#endif
}
/*       end of Eliding label     */
/**********************************/



StatusBar::StatusBar(QWidget *parent) :
    QStatusBar(parent)
{DD;
    panel = new PropertiesPanel(this);
    addWidget(panel);
}

void StatusBar::retranslateUI()
{DD;
    panel->retranslateUI();
}

void StatusBar::update(const Tag &tag)
{DD;
    panel->updateHover(tag);
}

void StatusBar::updateTotalLength(int totalLength, int totalCount)
{DD;
    panel->overallCount = totalCount;
    panel->overallLength = totalLength;
    panel->updateSelected();
}

void StatusBar::updateSelectedLength(int length, int count)
{DD;
    panel->selectedCount = count;
    panel->selectedLength = length;
    panel->updateSelected();
}

PropertiesPanel::PropertiesPanel(QWidget *parent) : QWidget(parent)
{DD;
    overallCount=0;
    selectedCount=0;
    overallLength=0;
    selectedLength=0;
    currentHover = Tag();

    fileIconLabel = new QLabel(this);

    fileNameLabel = new ElidingLabel(this);
    fileNameLabel->setMinimumWidth(200);
    fileNameLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    fileNameLabel->setElideMode(Qt::ElideLeft);
    fileNameLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    fileNameLabel->setAttribute(Qt::WA_MacSmallSize);

    readOnlyLabel = new QLabel(QSL("<font color=#505050>RO</font>"),this);
    readOnlyLabel->setToolTip(tr("File is read only"));
    readOnlyLabel->setAttribute(Qt::WA_MacSmallSize);

    typeLabel = new QLabel(this);
    typeLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    typeLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    typeLabel->setAttribute(Qt::WA_MacSmallSize);

    sizeLabel = new QLabel(QString("<font color=#505050>%1:</font>").arg(tr("Size")),this);
    sizeLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    sizeLabel->setAttribute(Qt::WA_MacSmallSize);

    sizeContentsLabel = new QLabel(this);
    sizeContentsLabel->setFixedWidth(sizeContentsLabel->fontMetrics().width(QSL("000.000 Mib  ")));
    sizeContentsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    sizeContentsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    sizeContentsLabel->setAttribute(Qt::WA_MacSmallSize);

    lengthLabel = new QLabel(QString("<font color=#505050>%1:</font>").arg(tr("Length")),this);
    lengthLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    lengthLabel->setAttribute(Qt::WA_MacSmallSize);

    lengthContentsLabel = new QLabel(this);
    lengthContentsLabel->setFixedWidth(lengthContentsLabel->fontMetrics().width(QSL("h:mm:ss  ")));
    lengthContentsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lengthContentsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    lengthContentsLabel->setAttribute(Qt::WA_MacSmallSize);

    selectedLabel = new QLabel(this);
    selectedLabel->setAttribute(Qt::WA_MacSmallSize);
    selectedLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    selectedLengthLabel = new QLabel(this);
    selectedLengthLabel->setAttribute(Qt::WA_MacSmallSize);
    selectedLengthLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    updateSelected();

    QGridLayout *l = new QGridLayout(this);
#ifdef Q_OS_MAC
    l->setMargin(2);
    l->setContentsMargins(1,1,1,1);
    l->setVerticalSpacing(3);
#else
    l->setMargin(0);
    l->setContentsMargins(2,0,2,2);
#endif
    l->addWidget(selectedLabel,0,0,1,1,Qt::AlignLeft | Qt::AlignVCenter);
    l->addItem(new QSpacerItem(10,1),0,1,1,1,Qt::AlignLeft | Qt::AlignVCenter);
    l->addWidget(fileIconLabel,0,2,1,1,Qt::AlignLeft | Qt::AlignVCenter);
    l->addWidget(fileNameLabel,0,3,1,1,Qt::AlignLeft | Qt::AlignVCenter);
    l->addWidget(sizeLabel,0,4,1,1,Qt::AlignRight | Qt::AlignVCenter);
    l->addWidget(sizeContentsLabel,0,5,1,1,Qt::AlignLeft | Qt::AlignVCenter);

    l->addWidget(selectedLengthLabel,1,0,1,1,Qt::AlignLeft | Qt::AlignVCenter);
    l->addWidget(readOnlyLabel,1,2,1,1,Qt::AlignLeft | Qt::AlignVCenter);
    l->addWidget(typeLabel,1,3,1,1,Qt::AlignLeft | Qt::AlignVCenter);
    l->addWidget(lengthLabel,1,4,1,1,Qt::AlignRight | Qt::AlignVCenter);
    l->addWidget(lengthContentsLabel,1,5,1,1,Qt::AlignLeft | Qt::AlignVCenter);

    readOnlyLabel->hide();
    sizeLabel->hide();
    lengthLabel->hide();
}

void PropertiesPanel::retranslateUI()
{DD;
    readOnlyLabel->setToolTip(tr("File is read only"));
    sizeLabel->setText(QString("<font color=#505050>%1:</font>").arg(tr("Size")));
    lengthLabel->setText(QString("<font color=#505050>%1:</font>").arg(tr("Length")));
    updateSelected();
    updateFileName();
    updateLengthContents();
    updateSizeContents();
}
void PropertiesPanel::updateLengthContents()
{DD;
    if (currentHover.length()!=0) {
        lengthContentsLabel->setText(Qoobar::formatLength(currentHover.length()));
        lengthLabel->setVisible(true);
    }
    else {
        lengthContentsLabel->clear();
        lengthLabel->hide();
    }
}
void PropertiesPanel::updateFileName()
{DD;
    if (!currentHover.fileName().isEmpty()) {
        fileNameLabel->setText(currentHover.fullFileName());
        QString properties;

        if (!currentHover.bitrate().isEmpty() || currentHover.sampleRate()!=0 || currentHover.channels()!=0)
            properties = tr("%1 kbps, %2 Hz, %3 ch.").arg(currentHover.bitrate())
                         .arg(currentHover.sampleRate()).arg(currentHover.channels());
        if (currentHover.tagTypes()==0) properties.append(QString(", ")+tr("no tags"));
        else {
            int tagTypes = currentHover.tagTypes();
            QStringList tags;
            if (tagTypes & TAG_ID3V1) tags<<"ID3v1";
            if (tagTypes & TAG_ID3V2) tags<<"ID3v2";
            if (tagTypes & TAG_APE) tags<<"APE";
            if (tagTypes & TAG_ASF) tags<<"ASF";
            if (tagTypes & TAG_MP4) tags<<"MP4";
            if (tagTypes & TAG_VORBIS) tags<<"Vorbis";
            properties.append(QString(", ")+tags.join(", "));
        }
        typeLabel->setText(properties);
        readOnlyLabel->setVisible(currentHover.readOnly());
        fileIconLabel->setPixmap(QPixmap(App->themeIcon(currentHover.icon())));
    }
    else {
        fileNameLabel->clear();
        fileIconLabel->clear();
        readOnlyLabel->setVisible(false);
        typeLabel->clear();
    }
}
void PropertiesPanel::updateSizeContents()
{DD;
    if (currentHover.size()>0) {
        sizeContentsLabel->setText(Qoobar::formatSize(currentHover.size()));
        sizeLabel->show();
    }
    else {
        sizeContentsLabel->clear();
        sizeLabel->hide();
    }
}
void PropertiesPanel::updateHover(const Tag& tag)
{DD;
    currentHover = tag;
    updateFileName();
    updateLengthContents();
    updateSizeContents();
}
void PropertiesPanel::updateSelected()
{DD;
    if (overallCount==0) {//tab is empty
        selectedLabel->clear();
        selectedLengthLabel->clear();
    }
    else if (selectedCount==0) {//there are some files
        selectedLabel->setText(QString("<font color=#505050>")+tr("%n file(s)","",overallCount)+QString("</font>"));
        selectedLengthLabel->setText(QString("<font color=#505050>")
                                     +tr("Overall length %1").arg(Qoobar::formatLength(overallLength))
                                     +QString("</font>"));
    }
    else {
        selectedLabel->setText(QString("<font color=#505050>")
                               +tr("Selected %n file(s) of %1","",selectedCount).arg(overallCount)
                               +QString("</font>"));
        selectedLengthLabel->setText(QString("<font color=#505050>")
                                     +tr("Selected length %1 of %2").arg(Qoobar::formatLength(selectedLength))
                                     .arg(Qoobar::formatLength(overallLength))
                                     +QString("</font>"));
    }
}

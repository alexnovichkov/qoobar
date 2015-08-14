#include "statusbar.h"
#include "tagger.h"
#include "qoobarglobals.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif


StatusBar::StatusBar(QWidget *parent) :
    QStatusBar(parent)
{DD
    filenameLabel = new QLabel;
    filenameLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    filenameLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    typeLabel = new QLabel;
    typeLabel->setFixedWidth(typeLabel->fontMetrics().width(QSL("0000 kbps, 44100 Hz, 2 ch.")));
    typeLabel->setAlignment(Qt::AlignTop | Qt::AlignCenter);
    typeLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    lengthLabel = new QLabel;
    lengthLabel->setFixedWidth(lengthLabel->fontMetrics().width(QSL("h:mm:ss")));
    lengthLabel->setAlignment(Qt::AlignTop | Qt::AlignCenter);
    lengthLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    totalLengthLabel = new QLabel;
    totalLengthLabel->setFixedWidth(totalLengthLabel->fontMetrics().width(QSL("hhh:mm:ss")));
    totalLengthLabel->setAlignment(Qt::AlignTop | Qt::AlignCenter);
    totalLengthLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    addWidget(filenameLabel,10);
    addPermanentWidget(typeLabel,1);
    addPermanentWidget(lengthLabel,1);
    addPermanentWidget(totalLengthLabel,1);
}

void StatusBar::update(const Tag &tag)
{DD
    QString elided=filenameLabel->fontMetrics().elidedText(tag.fullFileName(), Qt::ElideLeft, filenameLabel->width());
    filenameLabel->setText(elided);
    if (tag.bitrate().isEmpty() && tag.sampleRate()==0 && tag.channels()==0)
        typeLabel->clear();
    else
        typeLabel->setText(tr("%1 kbps, %2 Hz, %3 ch.").arg(tag.bitrate())
                       .arg(tag.sampleRate()).arg(tag.channels()));
    if (tag.length()==0)
        lengthLabel->clear();
    else
        lengthLabel->setText(Qoobar::formatLength(tag.length()));
}

void StatusBar::updateTotalLength(int totalLength)
{DD
    totalLengthLabel->setText(totalLength==0?QString():Qoobar::formatLength(totalLength));
}

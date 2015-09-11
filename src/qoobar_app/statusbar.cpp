#include "statusbar.h"
#include "tagger.h"
#include "qoobarglobals.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "application.h"



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
{
    panel->selectedCount = count;
    panel->selectedLength = length;
    panel->updateSelected();
}

PropertiesPanel::PropertiesPanel(QWidget *parent) : QWidget(parent)
{
    overallCount=0;
    selectedCount=0;
    overallLength=0;
    selectedLength=0;
    currentHover = Tag();

    fileIconLabel = new QLabel(this);

    fileNameLabel = new QLabel(this);
    fileNameLabel->setMinimumWidth(200);
    fileNameLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    readOnlyLabel = new QLabel(QSL("<font color=#505050>RO</font>"),this);
    readOnlyLabel->setToolTip(tr("File is read only"));

    typeLabel = new QLabel(this);
    typeLabel->setFixedWidth(typeLabel->fontMetrics().width(QSL("0000 kbps, 44100 Hz, 2 ch.")));
    typeLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    typeLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    sizeLabel = new QLabel(QString("<font color=#505050>%1:</font>").arg(tr("Size")),this);
    sizeLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);

    sizeContentsLabel = new QLabel(this);
    sizeContentsLabel->setFixedWidth(sizeContentsLabel->fontMetrics().width(QSL("000.000 Mib  ")));
    sizeContentsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    sizeContentsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    lengthLabel = new QLabel(QString("<font color=#505050>%1:</font>").arg(tr("Length")),this);
    lengthLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);

    lengthContentsLabel = new QLabel(this);
    lengthContentsLabel->setFixedWidth(lengthContentsLabel->fontMetrics().width(QSL("h:mm:ss  ")));
    lengthContentsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lengthContentsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    selectedLabel = new QLabel(this);
    selectedLengthLabel = new QLabel(this);
    updateSelected();

    QGridLayout *l = new QGridLayout(this);
    l->setMargin(0);
    l->addWidget(selectedLabel,0,0);
    l->addItem(new QSpacerItem(10,1),0,1);
    l->addWidget(fileIconLabel,0,2);
    l->addWidget(fileNameLabel,0,3);
    l->addWidget(sizeLabel,0,4);
    l->addWidget(sizeContentsLabel,0,5);

    l->addWidget(selectedLengthLabel,1,0);
    l->addWidget(readOnlyLabel,1,2);
    l->addWidget(typeLabel,1,3);
    l->addWidget(lengthLabel,1,4);
    l->addWidget(lengthContentsLabel,1,5);


    readOnlyLabel->hide();
    sizeLabel->hide();
    lengthLabel->hide();
}

void PropertiesPanel::retranslateUI()
{
    readOnlyLabel->setToolTip(tr("File is read only"));
    sizeLabel->setText(QString("<font color=#505050>%1:</font>").arg(tr("Size")));
    lengthLabel->setText(QString("<font color=#505050>%1:</font>").arg(tr("Length")));
    updateSelected();
    updateFileName();
    updateLengthContents();
    updateSizeContents();
}
void PropertiesPanel::updateLengthContents()
{
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
{
    if (!currentHover.fileName().isEmpty()) {
        QString elided=fileNameLabel->fontMetrics().elidedText(currentHover.fullFileName(), Qt::ElideLeft, 800);
        fileNameLabel->setText(elided);
        if (currentHover.bitrate().isEmpty() && currentHover.sampleRate()==0 && currentHover.channels()==0)
            typeLabel->clear();
        else
            typeLabel->setText(tr("%1 kbps, %2 Hz, %3 ch.").arg(currentHover.bitrate())
                               .arg(currentHover.sampleRate()).arg(currentHover.channels()));
        readOnlyLabel->setVisible(currentHover.readOnly());
        fileIconLabel->setPixmap(QPixmap(App->iconThemeIcon(currentHover.icon())));
    }
    else {
        fileNameLabel->clear();
        fileIconLabel->clear();
        readOnlyLabel->setVisible(false);
        typeLabel->clear();
    }
}
void PropertiesPanel::updateSizeContents()
{
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
{
    currentHover = tag;
    updateFileName();
    updateLengthContents();
    updateSizeContents();
}
void PropertiesPanel::updateSelected()
{
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

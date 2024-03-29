#include "autonumber.h"

#include <QtWidgets>
#include "application.h"
#include "enums.h"
#include "tagger.h"
#include "qbutton.h"
#include "qoobarhelp.h"

constexpr int maximumTracksCount = 9999;

AutonumberDialog::AutonumberDialog(const QList<Tag> &oldTags, QWidget *parent)
    : QDialog(parent), oldTags(oldTags)
{
    setWindowTitle(tr("Qoobar - Autonumber files"));

#ifdef OSX_SUPPORT_ENABLED
    setWindowModality(Qt::WindowModal);
#endif

    QSettings *se = App->guiSettings();
    se->beginGroup("autonumberPlugin");
    int startNumber = se->value("startNumber",1).toInt();
    int formatNum = se->value("formatNumber",0).toInt();
    bool addTotalTracks = se->value("addTotalTracks", false).toBool();
    bool resetFolder = se->value("resetFolder", false).toBool();
    bool resetAlbum = se->value("resetAlbum", false).toBool();
    int albumBehavior = se->value("albumBehavior",0).toInt();
    se->endGroup();
    delete se;

    tree = new QTreeWidget(this);
    tree->setRootIsDecorated(false);
    tree->setAllColumnsShowFocus(true);
    tree->setAlternatingRowColors(true);
    tree->setSelectionMode(QAbstractItemView::SingleSelection);
    tree->setUniformRowHeights(true);
#ifdef OSX_SUPPORT_ENABLED
    //tree->setFrameStyle(/*QFrame::NoFrame |*/ QFrame::Plain);
    tree->setAttribute(Qt::WA_MacShowFocusRect, false);
    tree->setAutoFillBackground(true);
    tree->setAttribute(Qt::WA_MacSmallSize, true);
#endif
    tree->setHeaderLabels(QStringList()<<tr("Old #")
                                       <<tr("New #")
                                       <<tr("Old ##")
                                       <<tr("New ##")
                                       <<tr("File")
                                       <<tr("Artist")
                                       <<tr("Composer")
                                       <<tr("Album")
                                       <<tr("Title"));
    for (int i=0; i<4; ++i)
        tree->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    tree->header()->setStretchLastSection(false);


    startNumberEdit = new QSpinBox(this);
    startNumberEdit->setRange(1,maximumTracksCount);
    startNumberEdit->setValue(startNumber);
    connect(startNumberEdit,SIGNAL(valueChanged(int)),SLOT(updateTrackNumbers()));
    //startNumberEdit->setMaximumWidth(100);

    formatComboBox = new QComboBox(this);
    formatComboBox->addItems(QStringList()<<tr("1")
                             <<tr("01")
                             <<tr("001"));
    formatComboBox->setCurrentIndex(formatNum);
    connect(formatComboBox,SIGNAL(currentIndexChanged(int)),SLOT(updateTrackNumbers()));

    actionComboBox = new QComboBox(this);
    actionComboBox->addItems(QStringList()<<tr("Remove track number")
                             <<tr("Keep old track number")
                             <<tr("Set \"1\" as track number")
                             <<tr("Number tracks separately"));
    actionComboBox->setCurrentIndex(albumBehavior);
    connect(actionComboBox,SIGNAL(currentIndexChanged(int)),SLOT(updateTrackNumbers()));


    addTotalTracksCheckBox = new QCheckBox(tr("Add total tracks"),this);
    addTotalTracksCheckBox->setChecked(addTotalTracks);
    resetFolderCheckBox = new QCheckBox(tr("Reset counter for each folder"),this);
    resetFolderCheckBox->setChecked(resetFolder);
    resetAlbumCheckBox = new QCheckBox(tr("Reset counter for each album"),this);
    resetAlbumCheckBox->setChecked(resetAlbum);
    connect(addTotalTracksCheckBox,SIGNAL(clicked()),SLOT(updateTrackNumbers()));
    connect(resetFolderCheckBox,SIGNAL(clicked()),SLOT(updateTrackNumbers()));
    connect(resetAlbumCheckBox,SIGNAL(clicked()),SLOT(updateTrackNumbers()));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
#ifdef OSX_SUPPORT_ENABLED
    QButton *helpButton = new QButton(this,QButton::HelpButton);
#else
    QPushButton *helpButton = buttonBox->addButton(QDialogButtonBox::Help);
#endif
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));

    auto *l = new QGridLayout;
    l->addWidget(new QLabel(tr("Begin at number")),0,0);
    l->addWidget(startNumberEdit,0,1);
    l->addWidget(new QLabel(tr("Number format")),0,2,1,1,Qt::AlignRight);
    l->addWidget(formatComboBox,0,3);
    l->addWidget(addTotalTracksCheckBox,1,0,1,2);

    l->addWidget(resetFolderCheckBox,2,0,1,2);
    l->addWidget(resetAlbumCheckBox,3,0,1,2);
    l->addWidget(new QLabel(tr("If a file has no album"
                               "\nor album includes only one track")),1,2,3,1);
    l->addWidget(actionComboBox,1,3,3,1);
    l->addWidget(tree,5,0,1,5);
#ifdef OSX_SUPPORT_ENABLED
    l->addWidget(helpButton,6,0);
    l->addWidget(buttonBox,6,1,1,4);
#else
    l->addWidget(buttonBox,6,0,1,5);
#endif


    this->setLayout(l);

    resize(App->primaryScreen()->availableSize()/2);

    updateTrackNumbers();
}

AutonumberDialog::~AutonumberDialog()
{
    int  firstNumber    = startNumberEdit->value();
    int  numberFormat   = formatComboBox->currentIndex();
    bool resetFolder    = resetFolderCheckBox->isChecked();
    bool resetAlbum     = resetAlbumCheckBox->isChecked();
    bool addTotalTracks = addTotalTracksCheckBox->isChecked();
    int  actionIndex    = actionComboBox->currentIndex();

    QSettings *se = App->guiSettings();
    se->beginGroup("autonumberPlugin");
    se->setValue("startNumber", firstNumber);
    se->setValue("formatNumber", numberFormat);
    se->setValue("addTotalTracks", addTotalTracks);
    se->setValue("resetFolder", resetFolder);
    se->setValue("resetAlbum", resetAlbum);
    se->setValue("albumBehavior", actionIndex);
    se->endGroup();
    delete se;
}

QList<Tag> AutonumberDialog::getNewTags()
{
    return newTags;
}

QString formatNumber(int number, int format)
{
    QString s = QString::number(number).rightJustified(format+1,'0');
    return s;
}

void AutonumberDialog::updateTrackNumbers()
{
    int totalCount = oldTags.size();
    tree->blockSignals(true);
    tree->clear();

    QList<QTreeWidgetItem*> items;
    for (const auto &tag: oldTags) {
        QTreeWidgetItem *item = new QTreeWidgetItem({
                                        tag.tracknumber(),
                                        tag.tracknumber(),
                                        tag.totalTracks(),
                                        tag.totalTracks(),
                                        tag.fileNameExt(),
                                        tag.artist(),
                                        tag.composer(),
                                        tag.album(),
                                        tag.title()});
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        items << item;
    }

    const int  firstNumber    = startNumberEdit->value();
    const int  numberFormat   = formatComboBox->currentIndex();
    const bool resetFolder    = resetFolderCheckBox->isChecked();
    const bool resetAlbum     = resetAlbumCheckBox->isChecked();
    const bool addTotalTracks = addTotalTracksCheckBox->isChecked();
    const int  actionIndex    = actionComboBox->currentIndex();

    //the 1st layer - group byFolder
    QMap<QString, QVector<int> > byFolder;
    for (int i=0; i<totalCount; ++i) {
        if (resetFolder)
            byFolder[oldTags.at(i).filePath()].append(i);
        else
            byFolder[" "].append(i);
    }

    //folder by folder group in the next layers
    QMapIterator<QString, QVector<int> > it(byFolder);
    while (it.hasNext()) {
        it.next();
        const QVector<int> indexes = it.value();

        //the 2nd layer - group by album
        QMap<QString, QVector<int> > byAlbum;
        for (int i: indexes) {
            if (resetAlbum)
                byAlbum[oldTags.at(i).album()].append(i);
            else
                byAlbum[" "].append(i);
        }


        QVector<int> indexesLast;

        QMapIterator<QString, QVector<int> > itA(byAlbum);
        while (itA.hasNext()) {
            itA.next();
            const QVector<int> indexesA = itA.value();

            for (int i=0; i<indexesA.size(); ++i) {
                QString trackNumber = formatNumber(i+firstNumber, numberFormat);
                QString totalTracks = formatNumber(indexesA.size(), numberFormat);

                if (itA.key().isEmpty() || indexesA.size()<=1) {
                    switch (actionIndex) {
                    case 0: {//0 - Remove track number
                        trackNumber.clear();
                        if (addTotalTracks) totalTracks.clear();
                        break;
                    }
                    case 1: {//1 - Keep old track number
                        trackNumber = oldTags.at(indexesA.at(i)).tracknumber();
                        if (addTotalTracks)
                            totalTracks = oldTags.at(indexesA.at(i)).totalTracks();
                        break;
                    }
                    case 2: {//2 - Set "1" as track number
                        trackNumber = formatNumber(1,numberFormat);
                        if (addTotalTracks)
                            totalTracks = formatNumber(1, numberFormat);
                        break;
                    }
                    case 3: {//3 - Number separately
                        indexesLast << indexesA.at(i);
                    }
                    }
                }

                items[indexesA.at(i)]->setText(1, trackNumber);
                if (addTotalTracks)
                    items[indexesA.at(i)]->setText(3, totalTracks);
            }
        }

        if (!indexesLast.isEmpty()) {
            for (int i=0; i<indexesLast.size(); ++i) {
                QString trackNumber = formatNumber(i+firstNumber, numberFormat);
                QString totalTracks = formatNumber(indexesLast.size(), numberFormat);
                items[indexesLast.at(i)]->setText(1, trackNumber);
                if (addTotalTracks)
                    items[indexesLast.at(i)]->setText(3, totalTracks);
            }
        }
    }
    tree->addTopLevelItems(items);
    tree->blockSignals(false);

    bool tagsAreChanged = false;
    for (int i=0; i<tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = tree->topLevelItem(i);
        if (item->text(0) != item->text(1) || item->text(2) != item->text(3)) {
            tagsAreChanged = true;
            break;
        }
    }
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(tagsAreChanged);
}

void AutonumberDialog::accept()
{
    newTags = oldTags;
    for (int i=0; i<newTags.count(); ++i) {
        newTags[i].setTag(TRACKNUMBER, tree->topLevelItem(i)->text(1));
        newTags[i].setTag(TOTALTRACKS, tree->topLevelItem(i)->text(3));
    }
    QDialog::accept();
}

void AutonumberDialog::showHelp()
{
    Qoobar::showHelp("autonumbering");
}

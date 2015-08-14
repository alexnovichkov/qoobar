#include "autonumberplugin.h"

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "enums.h"

#ifndef HAVE_QT5
QString AutonumberPlugin::text()
{
    return tr("Autonumber selected files");
}

/*returns an icon for menus, tollbars etc.*/
QIcon AutonumberPlugin::icon()
{
    return QIcon();
}

/*returns a full localized description of a plugin*/
QString AutonumberPlugin::description()
{
    return tr("Automatically number selected files");
}

/*returns a short unique key that identifies a plugin*/
QString AutonumberPlugin::key()
{
    return "Autonumber";
}

QString AutonumberPlugin::version()
{
    return QString(PLUGIN_VERSION);
}

Q_EXPORT_PLUGIN2(Autonumber,AutonumberPlugin)
#endif

QList<Tag> AutonumberPlugin::getNewTags(const QList<Tag> &oldTags)
{
    Dialog dialog(oldTags);
    if (dialog.exec()) {
        return dialog.getNewTags();
    }
    return oldTags;
}

Dialog::Dialog(const QList<Tag> &oldTags, QWidget *parent)
    : QDialog(parent), oldTags(oldTags)
{
    setWindowTitle(tr("Autonumbering dialog"));

    QSettings se("qoobar","qoobar");
    se.beginGroup("autonumberPlugin");
    int startNumber = se.value("startNumber",1).toInt();
    int formatNumber = se.value("formatNumber",0).toInt();
    bool addTotalTracks = se.value("addTotalTracks", false).toBool();
    bool resetFolder = se.value("resetFolder", false).toBool();
    bool resetAlbum = se.value("resetAlbum", false).toBool();
    int albumBehavior = se.value("albumBehavior",0).toInt();
    se.endGroup();

    tree = new QTreeWidget(this);
    tree->setRootIsDecorated(false);
    tree->setAllColumnsShowFocus(true);
    tree->setAlternatingRowColors(true);
    tree->setSelectionMode(QAbstractItemView::SingleSelection);
    tree->setUniformRowHeights(true);
#ifdef Q_OS_MAC
    tree->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    //setAttribute(Qt::WA_MacShowFocusRect, false);
    tree->setAutoFillBackground(true);
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
        tree->header()->SETSECTIONRESIZEMODE(i, QHeaderView::ResizeToContents);
    tree->header()->setStretchLastSection(false);


    for (int i=0; i<oldTags.size(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(
                    QStringList()<<oldTags.at(i).tracknumber()
                    <<oldTags.at(i).tracknumber()
                    <<oldTags.at(i).totalTracks()
                    <<oldTags.at(i).totalTracks()
                    <<oldTags.at(i).fileNameExt()
                    <<oldTags.at(i).artist()
                    <<oldTags.at(i).composer()
                    <<oldTags.at(i).album()
                    <<oldTags.at(i).title());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        tree->addTopLevelItem(item);
    }

    startNumberEdit = new QSpinBox(this);
    startNumberEdit->setRange(1,9999);
    startNumberEdit->setValue(startNumber);
    connect(startNumberEdit,SIGNAL(valueChanged(int)),SLOT(updateTrackNumbers()));
    //startNumberEdit->setMaximumWidth(100);

    formatComboBox = new QComboBox(this);
    formatComboBox->addItems(QStringList()<<tr("1")
                             <<tr("01")
                             <<tr("001"));
    formatComboBox->setCurrentIndex(formatNumber);
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

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QGridLayout *l = new QGridLayout;
    l->addWidget(new QLabel(tr("Begin at number")),0,0);
    l->addWidget(startNumberEdit,0,1);
    l->addWidget(new QLabel(tr("Number format")),0,2,1,1,Qt::AlignRight);
    l->addWidget(formatComboBox,0,3);
    l->addWidget(addTotalTracksCheckBox,0,4,1,1,Qt::AlignRight);

    l->addWidget(resetFolderCheckBox,1,0,1,5);
    l->addWidget(resetAlbumCheckBox,2,0,1,1);
    l->addWidget(new QLabel(tr("If a file has no album"
                               "\nor album includes only one track")),2,1,1,1);
    l->addWidget(actionComboBox,2,2,1,3);
    l->addWidget(tree,3,0,1,5);
    l->addWidget(buttonBox,4,0,1,5);

    this->setLayout(l);
    resize(800,480);

    updateTrackNumbers();
}

Dialog::~Dialog()
{
    int  firstNumber    = startNumberEdit->value();
    int  numberFormat   = formatComboBox->currentIndex();
    bool resetFolder    = resetFolderCheckBox->isChecked();
    bool resetAlbum     = resetAlbumCheckBox->isChecked();
    bool addTotalTracks = addTotalTracksCheckBox->isChecked();
    int  actionIndex    = actionComboBox->currentIndex();

    QSettings se("qoobar","qoobar");
    se.beginGroup("autonumberPlugin");
    se.setValue("startNumber", firstNumber);
    se.setValue("formatNumber", numberFormat);
    se.setValue("addTotalTracks", addTotalTracks);
    se.setValue("resetFolder", resetFolder);
    se.setValue("resetAlbum", resetAlbum);
    se.setValue("albumBehavior", actionIndex);
    se.endGroup();
}

QList<Tag> Dialog::getNewTags()
{
    return newTags;
}

QString formatNumber(int number, int format)
{
    QString s = QString::number(number);
    while (s.length()<format+1) s.prepend('0');
    return s;
}

void Dialog::updateTrackNumbers()
{
    int totalCount = oldTags.size();

    for (int i=0; i<totalCount; ++i) {
        tree->topLevelItem(i)->setText(1, "");
        tree->topLevelItem(i)->setText(3, "");
    }

    int  firstNumber    = startNumberEdit->value();
    int  numberFormat   = formatComboBox->currentIndex();
    bool resetFolder    = resetFolderCheckBox->isChecked();
    bool resetAlbum     = resetAlbumCheckBox->isChecked();
    bool addTotalTracks = addTotalTracksCheckBox->isChecked();
    int  actionIndex    = actionComboBox->currentIndex();


    QMap<QString, QVector<int> > byFolder;
    Q_ASSERT_X(totalCount<=oldTags.size(), "autonumberplugin.cpp line 214","index out of bounds");
    if (resetFolder) {
        for (int i=0; i<totalCount; ++i) {
            QString path = oldTags.at(i).filePath();
            QVector<int> indexes = byFolder.value(path);
            indexes.append(i);
            byFolder.insert(path, indexes);
        }
    }
    else {
        QVector<int> indexes;
        for (int i=0; i<totalCount; ++i)
            indexes << i;
        byFolder.insert(" ", indexes);
    }

    QMapIterator<QString, QVector<int> > it(byFolder);
    while (it.hasNext()) {
        it.next();
        QVector<int> indexes = it.value();

        QMap<QString, QVector<int> > byAlbum;
        if (resetAlbum) {
            for (int i=0; i<indexes.size(); ++i) {
                QString album = oldTags.at(indexes.at(i)).album();
                QVector<int> indexesA = byAlbum.value(album);
                indexesA.append(indexes.at(i));
                byAlbum.insert(album, indexesA);
            }
        }
        else {
            byAlbum.insert(" ", indexes);
        }

        QVector<int> indexesLast;

        QMapIterator<QString, QVector<int> > itA(byAlbum);
        while (itA.hasNext()) {
            itA.next();
            QString album = itA.key();
            QVector<int> indexesA = itA.value();

            for (int i=0; i<indexesA.size(); ++i) {
                QString trackNumber = formatNumber(i+firstNumber, numberFormat);
                QString totalTracks = formatNumber(indexesA.size(), numberFormat);

                if (album.isEmpty() || indexesA.size()<=1) {
                    switch (actionIndex) {
                    case 0: {//0 - Remove track number
                        trackNumber.clear();
                        if (addTotalTracks) totalTracks.clear();
                        break;
                    }
                    case 1: {//1 - Keep old track number
                        trackNumber = tree->topLevelItem(indexesA.at(i))->text(0);
                        if (addTotalTracks)
                            totalTracks = tree->topLevelItem(indexesA.at(i))->text(2);
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

                tree->topLevelItem(indexesA.at(i))->setText(1, trackNumber);
                if (addTotalTracks)
                    tree->topLevelItem(indexesA.at(i))->setText(3, totalTracks);

            }
        }

        if (!indexesLast.isEmpty()) {
            for (int i=0; i<indexesLast.size(); ++i) {
                QString trackNumber = formatNumber(i+firstNumber, numberFormat);
                QString totalTracks = formatNumber(indexesLast.size(), numberFormat);
                tree->topLevelItem(indexesLast.at(i))->setText(1, trackNumber);
                if (addTotalTracks)
                    tree->topLevelItem(indexesLast.at(i))->setText(3, totalTracks);
            }
        }
    }
}

void Dialog::accept()
{
    newTags = oldTags;
    for (int i=0; i<newTags.count(); ++i) {
        newTags[i].setTag(TRACKNUMBER, tree->topLevelItem(i)->text(1));
        newTags[i].setTag(TOTALTRACKS, tree->topLevelItem(i)->text(3));
    }
    QDialog::accept();
}

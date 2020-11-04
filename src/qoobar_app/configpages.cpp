/**
 * \file configpages.cpp
 * Class that contains pages for settings dialog.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 17 Jan 2012
 *
 * Copyright (C) 2012  Alex Novichkov
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

#include "configpages.h"
#include "application.h"
#include "qoobarglobals.h"
#include "completionsdialog.h"
#include "legendbutton.h"
#include "schemeeditor.h"
#include <QtWidgets>
#include "enums.h"

#include <QJsonDocument>

#include "clearlineedit.h"
#include "fancylineedit.h"

#include <QVector>
#include <QtConcurrentRun>

constexpr int canSearchManuallyColumn = 3;
constexpr int canSearchByCDColumn = 4;
constexpr int canSearchByFilesColumn = 5;

ConfigPage::ConfigPage(QWidget *parent) : QWidget(parent)
{DD;
    contentsWidget = new QFrame(this);
    contentsWidget->setContentsMargins(0,0,0,0);
#ifdef Q_OS_WIN
    contentsWidget->setFrameShape(QFrame::StyledPanel);
#endif
    auto *contentsLayout = new QVBoxLayout;
    contentsLayout->setContentsMargins(0,0,0,0);

    contentsLayout->addWidget(contentsWidget);

    setLayout(contentsLayout);
}

void ConfigPage::retranslateUI()
{DD;

}

void ConfigPage::finalize(QLayout *layout)
{DD;
    contentsWidget->setLayout(layout);
    setSettings();
}

InterfacePage::InterfacePage(QWidget *parent) : ConfigPage(parent)
{DD;
    dirBox = new QCheckBox(tr("Show folder tree"),this);
    dirBox->setWhatsThis(tr("Check this box to show or hide the Folders navigation tree"));
    dirRoot = new FancyLineEdit(this);
    //TODO: this->devicePixelRatio()
    QPixmap pixmap(SMALL_ICON_SIZE, SMALL_ICON_SIZE);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.drawText(0,SMALL_ICON_SIZE-2,QSL("..."));
    dirRoot->setButtonPixmap(FancyLineEdit::Right, pixmap);
    dirRoot->setButtonVisible(FancyLineEdit::Right, true);
    dirRoot->setButtonToolTip(FancyLineEdit::Right, tr("Choose..."));
    dirRoot->setAutoHideButton(FancyLineEdit::Right, false);
    dirRoot->setWhatsThis(tr("Sets the top level folder for the Folders navigation tree"));
    dirRoot->setPlaceholderText(tr("All disks"));
    connect(dirRoot, SIGNAL(rightButtonClicked()), this, SLOT(chooseDirRoot()));

    dirRootLabel = new QLabel(tr("Folder tree root"),this);
    dirRootLabel->setBuddy(dirRoot);
    dirRootLabel->setWhatsThis(tr("Sets the top level folder for the Folders navigation tree"));
    useUndo = new QCheckBox(tr("Use undo / redo"),this);
    useUndo->setWhatsThis(tr("This box allows you to turn off the Undo/Redo system in Qoobar"));

    autoexpand = new QCheckBox(tr("Automatically fill a tag "
                                  "when pasting a single line"),this);
#ifndef Q_OS_MAC
    chars=new FancyLineEdit(this);
    chars->setButtonPixmap(FancyLineEdit::Right, QIcon::fromTheme("font").pixmap(SMALL_ICON_SIZE,SMALL_ICON_SIZE));
    chars->setButtonVisible(FancyLineEdit::Right, true);
    chars->setButtonToolTip(FancyLineEdit::Right, tr("Font..."));
    chars->setAutoHideButton(FancyLineEdit::Right, false);
    connect(chars, SIGNAL(rightButtonClicked()), this, SLOT(changeCharsFont()));
    chars->setWhatsThis(tr("Characters that will be shown in the Tags edit dialog"));
    charsBox = new QLabel(tr("Characters"),this);
    charsBox->setWhatsThis(tr("Characters that will be shown in the Tags edit dialog"));
#endif
    lang = new QComboBox(this);
    QStringList ts=QDir(ApplicationPaths::translationsPath()).entryList(QStringList(QSL("*.qm")), QDir::Files | QDir::Readable);
    QTranslator translator;
    int index=-1;
    Q_FOREACH(const QString &s,ts) {
        if (translator.load(ApplicationPaths::translationsPath() + "/" + s)) {
            QString ID=translator.translate("Settings","en");
            QString text=translator.translate("Settings","English");
            if (!ID.isEmpty()) {
                index++;
                lang->addItem(text,ID);
            }
        }
    }
    connect(lang,SIGNAL(currentIndexChanged(int)),this,SLOT(updateLanguage(int)));

    langLabel= new QLabel(tr("User interface language"),this);
    hideTabBar = new QCheckBox(tr("Hide Tab bar with only one tab"), this);

    iconThemeLabel = new QLabel(tr("Toolbar icons theme"),this);
    iconTheme = new QComboBox(this);
    QStringList iconThemes = QDir(ApplicationPaths::sharedPath()+"/icons").entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    Q_FOREACH(const QString &dir, iconThemes) {
        QFile jsonFile(ApplicationPaths::sharedPath()+"/icons/"+dir+"/properties.json");
        QVariantMap result;
        if (jsonFile.open(QFile::ReadOnly)) {
            QVariant parsed = QJsonDocument::fromJson(jsonFile.readAll()).toVariant();
            result = parsed.toMap();
        }
        if (result.isEmpty()) {
            iconTheme->addItem(dir);
            iconTheme->setItemData(iconTheme->count()-1, dir);
        }
        else {
            QString themeName = result["name"].toMap()[App->langID].toString();
            if (themeName.isEmpty()) themeName = result["name"].toMap()["default"].toString();
            if (themeName.isEmpty()) themeName = dir;
            iconTheme->addItem(themeName);
            iconTheme->setItemData(iconTheme->count()-1, dir);
        }
    }

    statusBarTrackLabel = new QLabel(tr("Status bar is tracking"), this);
    statusBarTrack = new QComboBox(this);
    statusBarTrack->addItem(tr("current hovered file"));
    statusBarTrack->addItem(tr("current selected file"));

    auto *UIlayout = new QFormLayout;
#ifdef Q_OS_MAC
    UIlayout->addRow("",useUndo);
    UIlayout->addRow("",autoexpand);
    UIlayout->addRow("",hideTabBar);
    UIlayout->addRow("",dirBox);
#else
    UIlayout->addRow(useUndo);
    UIlayout->addRow(autoexpand);
    UIlayout->addRow(hideTabBar);
    UIlayout->addRow(dirBox);
#endif
    UIlayout->addRow(dirRootLabel,dirRoot);
#ifndef Q_OS_MAC
    UIlayout->addRow(charsBox, chars);
#endif
    UIlayout->addRow(statusBarTrackLabel, statusBarTrack);
    UIlayout->addRow(langLabel, lang);
    UIlayout->addRow(iconThemeLabel, iconTheme);
    UIlayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto *UIML = new QVBoxLayout;
    UIML->addLayout(UIlayout);
    UIML->addStretch();

    contentsWidget->setFocusProxy(useUndo);
    finalize(UIML);
}

void InterfacePage::setSettings()
{DD;
    dirBox->setChecked(App->showDirView);
    dirRoot->setText(App->dirViewRoot);
    useUndo->setChecked(App->useUndo);
    autoexpand->setChecked(App->autoexpand);
#ifndef Q_OS_MAC
    chars->setText(App->chars);
    chars->setCursorPosition(0);
#endif
    int langIndex = lang->findData(App->langID);
    if (langIndex>=0) lang->setCurrentIndex(langIndex);

    int iconThemeIndex = iconTheme->findData(App->iconTheme);
    if (iconThemeIndex<0) iconThemeIndex = iconTheme->findData(QSL("default"));
    if (iconThemeIndex>=0) iconTheme->setCurrentIndex(iconThemeIndex);

    hideTabBar->setChecked(App->hideTabBar);

    statusBarTrack->setCurrentIndex(App->statusBarTrack);
}

QString InterfacePage::description()
{DD;
    return tr("Interface");
}
QString InterfacePage::iconFilename()
{DD;
    return "preferences-system-windows";
}
void InterfacePage::retranslateUI()
{DD;
    ConfigPage::retranslateUI();

    statusBarTrackLabel->setText(tr("Status bar is tracking"));
    statusBarTrack->setItemText(0, tr("current hovered file"));
    statusBarTrack->setItemText(1, tr("current selected file"));

    autoexpand->setText(tr("Automatically fill a tag "
                           "when pasting a single line"));
#ifndef Q_OS_MAC
    chars->setButtonToolTip(FancyLineEdit::Right, tr("Font..."));
    charsBox->setText(tr("Characters"));
    chars->setWhatsThis(tr("Characters that will be shown in the Tags edit dialog"));
    charsBox->setWhatsThis(tr("Characters that will be shown in the Tags edit dialog"));
#endif
    langLabel->setText(tr("User interface language"));
    useUndo->setText(tr("Use undo / redo"));
    dirBox->setText(tr("Show folder tree"));
    hideTabBar->setText(tr("Hide Tab bar with only one tab"));
    dirRoot->setPlaceholderText(tr("All disks"));
    dirRootLabel->setText(tr("Folder tree root"));
    iconThemeLabel->setText(tr("Toolbar icons theme"));
    dirBox->setWhatsThis(tr("Check this box to show or hide the Folders navigation tree"));
    dirRoot->setWhatsThis(tr("Sets the top level folder for the Folders navigation tree"));
    dirRootLabel->setWhatsThis(tr("Sets the top level folder for the Folders navigation tree"));
    useUndo->setWhatsThis(tr("This box allows you to turn off the Undo/Redo system in Qoobar"));

}
void InterfacePage::saveSettings()
{DD;
#ifndef Q_OS_MAC
    App->chars=chars->text();
#endif
    App->autoexpand=autoexpand->isChecked();
    App->useUndo = useUndo->isChecked();
    App->showDirView = dirBox->isChecked();
    App->hideTabBar = hideTabBar->isChecked();
    App->dirViewRoot = dirRoot->text();
    if (App->iconTheme != iconTheme->itemData(iconTheme->currentIndex()).toString())
        QMessageBox::information(this,tr("Qoobar"),tr("The toolbar icons theme will be changed\n"
                                                      "after you restart Qoobar"));
    App->iconTheme = iconTheme->itemData(iconTheme->currentIndex()).toString();
    App->statusBarTrack = statusBarTrack->currentIndex();
}

void InterfacePage::changeCharsFont()
{DD;
    bool ok;
    QFont font = QFontDialog::getFont(&ok, App->charsFont, this, tr("Choose a Chars list font"));
    if (ok) {
        App->charsFont=font;
    }
}

void InterfacePage::updateLanguage(const int index)
{DD;
    QString langID=lang->itemData(index).toString();
    App->langID=langID;
    App->loadTranslations();
    //App->currentScheme->retranslateUI();
    Q_EMIT retranslate();
}

void InterfacePage::chooseDirRoot()
{DD;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a folder tree root"),
                                                    App->dirViewRoot,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        dirRoot->setText(dir);
    }
}


CompletionPage::CompletionPage(QWidget *parent) : ConfigPage(parent)
{DD;
    collectText = new QComboBox(this);
    collectText->setEditable(false);
    collectText->addItem(tr("Automatically"));
    collectText->addItem(tr("Manually"));
    collectTextLabel = new QLabel(tr("Collect text"),this);
    collectText->setWhatsThis(tr("Sets the mode of remembering the text you are typing for future use.<br>"
                                 "<b>Automatically</b> - every text line you are typing will be remembered.<br>"
                                 "<b>Manually</b> - the text line will be remembered only after you press the"
                                 " <i>Ctrl+H</i> shortcut"));
    collectTextLabel->setWhatsThis(tr("Sets the mode of remembering the text you are typing for future use.<br>"
                                      "<b>Automatically</b> - every text line you are typing will be remembered.<br>"
                                      "<b>Manually</b> - the text line will be remembered only after you press the"
                                      " <i>Ctrl+H</i> shortcut"));

    completionStyle = new QComboBox(this);
    completionStyle->setEditable(false);
    completionStyle->addItem(tr("Completer matches the previous word"));
    completionStyle->addItem(tr("Completer matches the whole string"));
    completionStyleLabel = new QLabel(tr("Completion style"),this);

    auto *mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(int)),SLOT(editList(int)));

    completionTree = new QTreeWidget(this);
    completionTree->setRootIsDecorated(false);
    completionTree->setColumnCount(3);
    completionTree->setHeaderHidden(true);
    completionTree->header()->setStretchLastSection(false);
    completionTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    completionTree->setWhatsThis(tr("Check the tags for which you wish to use the autocompletion.<br>"
                                    "<br>The <i>Edit...</i> buttons allows you to manually change the remembered text lines"));
    completionTree->setAlternatingRowColors(true);
#ifdef Q_OS_MAC
    completionTree->setAttribute(Qt::WA_MacSmallSize, true);
#endif
    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<tagsCount; ++i) {
        auto *item = new QTreeWidgetItem(completionTree);

        auto *button = new QPushButton(this);
        connect(button,SIGNAL(clicked()),mapper,SLOT(map()));
        mapper->setMapping(button,i);
        completionTree->setItemWidget(item,2,button);
    }

    auto *l = new QFormLayout;
    l->addRow(collectTextLabel,collectText);
    l->addRow(completionStyleLabel,completionStyle);

    auto *autocompletionLayout = new QGridLayout;
    autocompletionLayout->addLayout(l,0,0);
    autocompletionLayout->addWidget(completionTree,1,0);

    finalize(autocompletionLayout);
}

void CompletionPage::setSettings()
{DD;
    collectText->setCurrentIndex(App->autocompletions->collectSilently?0:1);
    completionStyle->setCurrentIndex(App->autocompletions->completionStyle?0:1);

    for (int i=0; i<completionTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = completionTree->topLevelItem(i);
        item->setCheckState(0, App->autocompletions->use(i) ? Qt::Checked : Qt::Unchecked);
    }
}

void CompletionPage::editList(const int tagID)
{DD;
    CompletionsDialog dialog(tagID,this);
    if (dialog.exec()) {
        completionTree->topLevelItem(tagID)->setCheckState(0,Qt::Checked);
        int count = App->autocompletions->variantsCount(tagID);
        if (count==0) completionTree->topLevelItem(tagID)->setText(1,tr("Empty"));
        else completionTree->topLevelItem(tagID)->setText(1, tr("%n record(s)","",count));
    }
}

QString CompletionPage::description()
{DD;
    return tr("Autocompletion");
}
QString CompletionPage::iconFilename()
{DD;
    return "auto-type";
}
void CompletionPage::retranslateUI()
{DD;
    ConfigPage::retranslateUI();
    for (int i=0; i<completionTree->topLevelItemCount(); ++i) {
        completionTree->topLevelItem(i)->setText(0, App->currentScheme->localizedFieldName[i]);
        if (auto *b = qobject_cast<QPushButton *>(completionTree->itemWidget(completionTree->topLevelItem(i),2)))
            b->setText(tr("Edit..."));
        const int count = App->autocompletions->variantsCount(i);
        if (count==0) completionTree->topLevelItem(i)->setText(1,tr("Empty"));
        else completionTree->topLevelItem(i)->setText(1, tr("%n record(s)","",count));
    }
    completionStyleLabel->setText(tr("Completion style"));
    completionStyle->setItemText(0,tr("Completer matches the previous word"));
    completionStyle->setItemText(1,tr("Completer matches the whole string"));

    collectText->setItemText(0,tr("Automatically"));
    collectText->setItemText(1,tr("Manually"));
    collectTextLabel->setText(tr("Collect text"));
    collectText->setWhatsThis(tr("Sets the mode of remembering the text you are typing for future use.<br>"
                                 "<b>Automatically</b> - every text line you are typing will be remembered.<br>"
                                 "<b>Manually</b> - the text line will be remembered only after you press the"
                                 " <i>Ctrl+H</i> shortcut"));
    collectTextLabel->setWhatsThis(tr("Sets the mode of remembering the text you are typing for future use.<br>"
                                      "<b>Automatically</b> - every text line you are typing will be remembered.<br>"
                                      "<b>Manually</b> - the text line will be remembered only after you press the"
                                      " <i>Ctrl+H</i> shortcut"));
    completionTree->setWhatsThis(tr("Check the tags for which you wish to use the autocompletion.<br>"
                                    "<br>The <i>Edit...</i> buttons allows you to manually change the remembered text lines"));
}

void CompletionPage::saveSettings()
{DD;
    for (int i=0; i<completionTree->topLevelItemCount(); ++i)
        App->autocompletions->setUse(i, completionTree->topLevelItem(i)->checkState(0)==Qt::Checked);
    App->autocompletions->completionStyle = (completionStyle->currentIndex()==0);
    App->autocompletions->collectSilently = (collectText->currentIndex()==0);
}

WritingPage::WritingPage(QWidget *parent) : ConfigPage(parent)
{DD;
    saveChanges = new QCheckBox(tr("Save changes when closing Qoobar"),this);
    trim = new QCheckBox(tr("Simplify whitespaces when saving files"),this);
    writeFieldsSeparately = new QCheckBox(tr("Write tags separated by ; in different fields"),this);
//    readID3 = new QCheckBox(tr("Read ID3v2 tags"), this);
//    writeID3 = new QCheckBox(tr("Write ID3v2 tags"), this);
//    readAPE = new QCheckBox(tr("Read APE tags"), this);
//    writeAPE = new QCheckBox(tr("Write APE tags"), this);

    readMp3 = new QLabel(tr("Read:"), this);
    writeMp3 = new QLabel(tr("Write:"), this);
    readID3 = new QToolButton(this); readID3->setText(tr("ID3v2")); readID3->setCheckable(true);
    writeID3 = new QToolButton(this); writeID3->setText(tr("ID3v2")); writeID3->setCheckable(true);
    readAPE = new QToolButton(this); readAPE->setText(tr("APE")); readAPE->setCheckable(true);
    writeAPE = new QToolButton(this); writeAPE->setText(tr("APE")); writeAPE->setCheckable(true);

//    flacBox = new QGroupBox(tr("Flac files"),this);
//    readFlac = new QLabel(tr("Read:"),this);
//    writeFlac = new QLabel(tr("Write:"), this);
//    flacreadID3 = new QToolButton(this); flacreadID3->setText(tr("ID3v2")); flacreadID3->setCheckable(true);
//    flacwriteID3 = new QToolButton(this); flacwriteID3->setText(tr("ID3v2")); flacwriteID3->setCheckable(true);
//    flacreadOgg = new QToolButton(this); flacreadOgg->setText(tr("Ogg")); flacreadOgg->setCheckable(true);
//    flacwriteOgg = new QToolButton(this); flacwriteOgg->setText(tr("Ogg")); flacwriteOgg->setCheckable(true);
//    QHBoxLayout *flac1 = new QHBoxLayout;
//    flac1->addWidget(readFlac);
//    flac1->addWidget(flacreadOgg);
//    flac1->addWidget(flacreadID3);
//    flac1->addWidget(writeFlac);
//    flac1->addWidget(flacwriteOgg);
//    flac1->addWidget(flacwriteID3);
//    flac1->addStretch();
//    flacBox->setLayout(flac1);

    id3v2version = new QComboBox(this);
    id3v2version->setEditable(false);
    id3v2version->addItem(QSL("2.3"));
    id3v2version->addItem(QSL("2.4"));
    id3v2versionLabel = new QLabel(tr("ID3v2 tags version"),this);
    id3v2versionLabel->setBuddy(id3v2version);
    auto *id3v2versionLayout = new QHBoxLayout;
    id3v2versionLayout->addWidget(id3v2versionLabel);
    id3v2versionLayout->addWidget(id3v2version);

    box=new QGroupBox(tr("Mp3 files"));
    auto *mp3Layout=new QVBoxLayout;
    auto *mp3l1 = new QHBoxLayout;
    mp3l1->addWidget(readMp3);
    mp3l1->addWidget(readID3);
    mp3l1->addWidget(readAPE);
    mp3l1->addWidget(writeMp3);
    mp3l1->addWidget(writeID3);
    mp3l1->addWidget(writeAPE);
    mp3l1->addStretch();
    auto *mp3l2 = new QHBoxLayout;
    mp3l2->addWidget(id3v2versionLabel);
    mp3l2->addWidget(id3v2version);
    mp3l2->addStretch();
    mp3Layout->addLayout(mp3l1);
    mp3Layout->addLayout(mp3l2);
//    QFormLayout *mp3Layout=new QFormLayout;
//    mp3Layout->addRow(readID3,writeID3);
//    mp3Layout->addRow(readAPE,writeAPE);
//    mp3Layout->addRow(id3v2versionLabel,id3v2version);
//    mp3Layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    box->setLayout(mp3Layout);

    oggbox = new QGroupBox(tr("Ogg / Speex files"));
    oggLabel = new QLabel(tr("Write picture into tag"),this);
    auto *oggBoxLayout = new QFormLayout;
    oggPicture = new QComboBox(this);
    oggPicture->addItem(tr("COVERART (old standard)"));
    oggPicture->addItem(tr("METADATA_BLOCK_PICTURE (new standard)"));
    oggBoxLayout->addRow(oggLabel,oggPicture);
    oggBoxLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    oggbox->setLayout(oggBoxLayout);


    id3v1box=new QGroupBox(tr("ID3v1 tags"));
    id3v1writeLabel = new QLabel(tr("When writing tags"), this);
    id3v1write = new QComboBox(this);
    id3v1write->setEditable(false);
    id3v1write->addItem(tr("update ID3v1 tag"));
    id3v1write->addItem(tr("update ID3v1 tag only if it exists"));
    id3v1write->addItem(tr("delete ID3v1 tag"));
    id3v1transliterate = new QCheckBox(tr("Transliterate Russian words"),this);
    id3v1encoding = new QComboBox(this);
    QStringList codecs;
    QList<int> codecMibs = QTextCodec::availableMibs();
    Q_FOREACH(const int &mib,codecMibs)
        codecs << QString(QTextCodec::codecForMib(mib)->name());
    codecs.sort();
    id3v1encoding->addItem("Locale");
    id3v1encoding->addItems(codecs);
    id3v1encoding->setEditable(false);

    id3Label = new QLabel(tr("ID3v1 tag encoding"),this);
    auto *id3v1L = new QFormLayout;
    id3v1L->addRow(id3v1writeLabel, id3v1write);
    id3v1L->addRow(id3Label,id3v1encoding);
    id3v1L->addWidget(id3v1transliterate);
    id3v1L->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    id3v1box->setLayout(id3v1L);

#ifdef Q_OS_LINUX
    mpcbox = new QGroupBox(tr("Musepack files"), this);
    mpcLabel = new QLabel(tr("Write ReplayGain info of Musepack files into"), this);
    mpcReplayGain = new QComboBox(this);
    mpcReplayGain->setEditable(false);
    mpcReplayGain->addItem(tr("File header"));
    mpcReplayGain->addItem(tr("APE tags"));
    auto *mpcL = new QFormLayout;
    mpcL->addRow(mpcLabel, mpcReplayGain);
    mpcL->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    mpcbox->setLayout(mpcL);
#endif

    auto *rwLayout = new QVBoxLayout;
    rwLayout->addWidget(saveChanges);
    rwLayout->addWidget(trim);
    rwLayout->addWidget(writeFieldsSeparately);
    rwLayout->addWidget(box);
    rwLayout->addWidget(oggbox);
#ifdef Q_OS_LINUX
    rwLayout->addWidget(mpcbox);
#endif
    rwLayout->addWidget(id3v1box);
    rwLayout->addStretch();

    finalize(rwLayout);
}
void WritingPage::setSettings()
{DD;
    saveChanges->setCheckState(App->saveChanges ? Qt::Checked : Qt::Unchecked);
    trim->setCheckState(App->trim ? Qt::Checked : Qt::Unchecked);
    writeFieldsSeparately->setChecked(App->writeFieldsSeparately);
    readID3->setChecked(App->mp3readid3);
    writeID3->setChecked(App->mp3writeid3);
    readAPE ->setChecked(App->mp3readape);
    writeAPE->setChecked(App->mp3writeape);
    oggPicture->setCurrentIndex(App->oggPictureNew?1:0);
    id3v1write->setCurrentIndex(App->id3v1Synchro);
    id3v1transliterate->setChecked(App->id3v1Transliterate);
    id3v1encoding->setCurrentIndex(id3v1encoding->findText(App->id3v1Encoding));
    id3v2version->setCurrentIndex(App->id3v2version==4?1:0);
//    flacreadID3->setChecked(App->flacreadid3);
//    flacreadOgg->setChecked(App->flacreadogg);
//    flacwriteID3->setChecked(App->flacwriteid3);
//    flacwriteOgg->setChecked(App->flacwriteogg);
#ifdef Q_OS_LINUX
    mpcReplayGain->setCurrentIndex(App->mpcWriteRg?0:1);
#endif
}


QString WritingPage::description()
{DD;
    return tr("Tags Writing");
}
QString WritingPage::iconFilename()
{DD;
    return "tag-write";
}
void WritingPage::retranslateUI()
{DD;
    ConfigPage::retranslateUI();
    saveChanges->setText(tr("Save changes when closing Qoobar"));
    trim->setText(tr("Simplify whitespaces when saving files"));
    readMp3->setText(tr("Read:"));
    writeMp3->setText(tr("Write:"));
    readID3->setText(tr("ID3v2"));
    writeID3->setText(tr("ID3v2"));
    readAPE->setText(tr("APE"));
    writeAPE->setText(tr("APE"));
//    flacBox->setTitle(tr("Flac files"));
//    readFlac->setText(tr("Read:"));
//    writeFlac->setText(tr("Write:"));
//    flacreadID3->setText(tr("ID3v2"));
//    flacwriteID3->setText(tr("ID3v2"));
//    flacreadOgg->setText(tr("Ogg"));
//    flacwriteOgg->setText(tr("Ogg"));
    id3v1writeLabel->setText(tr("When writing tags"));

    box->setTitle(tr("Mp3 files"));
    oggbox->setTitle(tr("Ogg / Speex files"));
    oggPicture->setItemText(0,tr("COVERART (old standard)"));
    oggPicture->setItemText(1,tr("METADATA_BLOCK_PICTURE (new standard)"));

    oggLabel->setText(tr("Write picture into tag"));
    id3v1box->setTitle(tr("ID3v1 tags"));
    id3v1write->setItemText(0,tr("update ID3v1 tag"));
    id3v1write->setItemText(1,tr("update ID3v1 tag only if it exists"));
    id3v1write->setItemText(2,tr("delete ID3v1 tag"));
    id3v1transliterate->setText(tr("Transliterate Russian words"));
    id3Label->setText(tr("ID3v1 tag encoding"));
#ifdef Q_OS_LINUX
    mpcbox->setTitle(tr("Musepack files"));
    mpcLabel->setText(tr("Write ReplayGain info of Musepack files into"));
    mpcReplayGain->setItemText(0,tr("File header"));
    mpcReplayGain->setItemText(1,tr("APE tags"));
#endif
    writeFieldsSeparately->setText(tr("Write tags separated by ; in different fields"));

    id3v2versionLabel->setText(tr("ID3v2 tags version"));
}
void WritingPage::saveSettings()
{DD;
    App->saveChanges=saveChanges->isChecked();
    App->trim=trim->isChecked();
    App->mp3readape=readAPE->isChecked();
    App->mp3readid3=readID3->isChecked();
    App->mp3writeape=writeAPE->isChecked();
    App->mp3writeid3=writeID3->isChecked();
    App->oggPictureNew = (oggPicture->currentIndex()==1);
    App->id3v1Synchro=id3v1write->currentIndex();
    App->id3v1Transliterate=id3v1transliterate->isChecked();
    App->setId3v1Encoding(id3v1encoding->currentText());
    App->writeFieldsSeparately = writeFieldsSeparately->isChecked();
    App->id3v2version = id3v2version->currentIndex()==0?3:4;
#ifdef Q_OS_LINUX
    App->mpcWriteRg = mpcReplayGain->currentIndex()==0;
#endif
//    App->flacreadid3=flacreadID3->isChecked();
//    App->flacreadogg=flacreadOgg->isChecked();
//    App->flacwriteid3=flacwriteID3->isChecked();
//    App->flacwriteogg=flacwriteOgg->isChecked();
}


PatternsPage::PatternsPage(QWidget *parent) : ConfigPage(parent)
{DD;
    renamingPatternsBox = new QGroupBox(tr("Renaming patterns"),this);
    patterns = new QListWidget(this);
    patterns->setDragEnabled(true);
    patterns->viewport()->setAcceptDrops(true);
    patterns->setDropIndicatorShown(true);
    patterns->setDragDropMode(QAbstractItemView::InternalMove);
    //patterns->setMinimumHeight(200);
#ifdef Q_OS_MAC
    patterns->setAlternatingRowColors(true);
#endif

    addPatternButton = new QPushButton(tr("Add pattern"),this);
    connect(addPatternButton,SIGNAL(clicked()),this,SLOT(addPattern()));
    removePatternButton = new QPushButton(tr("Remove pattern"),this);
    connect(removePatternButton,SIGNAL(clicked()),this,SLOT(removePattern()));

    legendButton = new LegendButton(this);
    legendButton->setCategories(LegendButton::WritablePlaceholders | LegendButton::ReadOnlyPlaceholders);
    connect(legendButton,SIGNAL(placeholderChosen(QString)),SLOT(insertLegend(QString)));
    legendButton->setFocusPolicy(Qt::NoFocus);

    auto *renamingLayout = new QGridLayout;
    renamingLayout->addWidget(patterns,0,0,4,2);
    renamingLayout->addWidget(addPatternButton,0,2);
    renamingLayout->addWidget(removePatternButton,1,2);
    renamingLayout->addWidget(legendButton,2,2);
    renamingLayout->setColumnStretch(0,1);
    renamingPatternsBox->setLayout(renamingLayout);


    schemesBox = new QGroupBox(tr("Tagging schemes"),this);
    schemesLabel = new QLabel(tr("Current scheme"),this);
    schemesComboBox = new QComboBox(this);
    schemesComboBox->setEditable(false);

    QFileInfoList schemeFiles = QDir(ApplicationPaths::userSchemesPath()).entryInfoList(QStringList(QLS("*.xml")))
            +QDir(ApplicationPaths::schemesPath()).entryInfoList(QStringList(QLS("*.xml")));

    QStringList schemeFilesList;
    Q_FOREACH (const QFileInfo &schemeFile, schemeFiles) schemeFilesList << schemeFile.canonicalFilePath();
    schemeFilesList.removeDuplicates();
    schemesComboBox->addItem(tr("Default"),":/src/default.xml");
    Q_FOREACH (const QString &schemePath, schemeFilesList) {
        TaggingScheme scheme(schemePath);
        scheme.read(true);
        QString schemeName = scheme.name();
        if (!schemeName.isEmpty()) {
            schemesComboBox->addItem(schemeName,schemePath);
        }
    }
    connect(schemesComboBox,SIGNAL(currentIndexChanged(int)),SLOT(schemesComboBoxIndexChanged(int)));

    editSchemeButton  = new QPushButton(tr("Edit"),this);
    connect(editSchemeButton,SIGNAL(clicked()),SLOT(editScheme()));

    addSchemeButton = new QPushButton(tr("Add"),this);
    schemesMenu = new QMenu(this);
    QAction *a=schemesMenu->addAction(tr("Clone this"),this,SLOT(addScheme()));
    a->setData("clone");
    a=schemesMenu->addAction(tr("Using Default"),this,SLOT(addScheme()));
    a->setData("default");
    addSchemeButton->setMenu(schemesMenu);

    removeSchemeButton = new QPushButton(tr("Remove"),this);
    connect(removeSchemeButton,SIGNAL(clicked()),SLOT(removeScheme()));

    auto *schemesListLayout = new QGridLayout;
    schemesListLayout->addWidget(schemesLabel,0,0);
    schemesListLayout->addWidget(schemesComboBox,0,1);
    schemesListLayout->addWidget(editSchemeButton,0,2);
    schemesListLayout->addWidget(addSchemeButton,0,3);
    schemesListLayout->addWidget(removeSchemeButton,0,4);
    schemesBox->setLayout(schemesListLayout);

    auto *patternsLayout = new QVBoxLayout;
    patternsLayout->addWidget(renamingPatternsBox);
    patternsLayout->addWidget(schemesBox);
    patternsLayout->setStretch(0,1);
    //patternsLayout->addStretch();

    finalize(patternsLayout);
}

void PatternsPage::setSettings()
{DD;
    patterns->clear();
    patterns->addItems(App->patterns);
    for (int i=0; i<patterns->count();++i)
        patterns->item(i)->setFlags(patterns->item(i)->flags() | Qt::ItemIsEditable);

    int schemeIndex = schemesComboBox->findData(App->currentSchemeName);
    if (schemeIndex < 0) schemeIndex = 0;
    schemesComboBox->setCurrentIndex(schemeIndex);
    schemesComboBoxIndexChanged(schemeIndex);
}

QString PatternsPage::description()
{DD;
    return tr("Patterns");
}
QString PatternsPage::iconFilename()
{DD;
    return "patterns";
}
void PatternsPage::retranslateUI()
{DD;
    ConfigPage::retranslateUI();
    renamingPatternsBox->setTitle(tr("Renaming patterns"));
    addPatternButton->setText(tr("Add pattern"));
    removePatternButton->setText(tr("Remove pattern"));

    schemesBox->setTitle(tr("Tagging schemes"));
    schemesLabel->setText(tr("Current scheme"));
    schemesComboBox->setItemText(0,tr("Default"));
    addSchemeButton->setText(tr("Add"));
    editSchemeButton->setText(tr("Edit"));

    schemesMenu->actions()[0]->setText(tr("Clone this"));
    schemesMenu->actions()[1]->setText(tr("Using Default"));

    removeSchemeButton->setText(tr("Remove"));
}
void PatternsPage::saveSettings()
{DD;
    updatePatterns();
    int index = schemesComboBox->currentIndex();
    if (index>=0) {
        QString currentSchemeName=schemesComboBox->itemData(index).toString();
//        if (index != schemeIndex)
        //if (App->currentSchemeName != currentSchemeName || )
        App->currentSchemeName = currentSchemeName;
        delete App->currentScheme;
        App->currentScheme = new TaggingScheme(currentSchemeName);
        App->currentScheme->read();
    }
}
void PatternsPage::schemesComboBoxIndexChanged(const int index)
{DD;
    removeSchemeButton->setEnabled(index!=0);
}
void PatternsPage::removeScheme()
{DD;
    int curIndex = schemesComboBox->currentIndex();
    if (curIndex <= 0) return;

    QFile f(schemesComboBox->itemData(curIndex).toString());
    if (f.remove())
        schemesComboBox->removeItem(curIndex);
    else {
        criticalMessage(nullptr, QSL("Qoobar"), tr("Cannot delete the file\n%1.").arg(f.fileName()));
    }
}

void PatternsPage::editScheme()
{DD;
    int curIndex = schemesComboBox->currentIndex();
    if (curIndex < 0) return;

    QString schemePath=schemesComboBox->itemData(curIndex).toString();
    SchemeEditor editor(schemePath, (curIndex > 0) ? SchemeEditor::Modify : SchemeEditor::Copy, this);

    if (editor.exec()) {
        if (curIndex==0) {
            schemesComboBox->addItem(editor.scheme->name(), editor.scheme->filePath());
            schemesComboBox->setCurrentIndex(schemesComboBox->count()-1);
        }
    }
}

void PatternsPage::addScheme()
{DD;
    if (auto *a = qobject_cast<QAction *>(sender())) {
        int curIndex = schemesComboBox->currentIndex();
        if (a->data().toString()=="default")
            curIndex=0;
        if (curIndex<0) return;

        SchemeEditor editor(schemesComboBox->itemData(curIndex).toString(),SchemeEditor::Copy,this);

        if (editor.exec()) {
            schemesComboBox->addItem(editor.scheme->name(), editor.scheme->filePath());
            schemesComboBox->setCurrentIndex(schemesComboBox->count()-1);
        }
    }
}

void PatternsPage::updatePatterns()
{DD;
    QStringList l;
    for (int i=0; i<patterns->count(); ++i)
        l << patterns->item(i)->text();
    App->patterns = l;
}

void PatternsPage::insertLegend(const QString &s)
{DD;
    auto *edit = qobject_cast<QLineEdit *>(patterns->itemWidget(patterns->currentItem()));
    if (edit)
        edit->insert(s);
}

void PatternsPage::addPattern()
{DD;
    QListWidgetItem *item=new QListWidgetItem(QString(),patterns);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    patterns->setCurrentRow(patterns->count()-1);
    patterns->editItem(patterns->item(patterns->count()-1));
}

void PatternsPage::removePattern()
{DD;
    delete patterns->takeItem(patterns->currentRow());
}


UtilitiesPage::UtilitiesPage(QWidget *parent) : ConfigPage(parent)
{DD;
    player=new FancyLineEdit(this);
    QPixmap pixmap(SMALL_ICON_SIZE, SMALL_ICON_SIZE);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.drawText(0,SMALL_ICON_SIZE-2,QSL("..."));

    player->setButtonPixmap(FancyLineEdit::Right, pixmap);
    player->setButtonVisible(FancyLineEdit::Right, true);
    player->setButtonToolTip(FancyLineEdit::Right, tr("Choose..."));
    player->setAutoHideButton(FancyLineEdit::Right, false);
    player->setPlaceholderText(tr("Path/to/player"));
    player->setMinimumWidth(player->fontMetrics().HORIZONTAL_ADVANCE(QSL("Path/to/player"))*3/2);
    connect(player, SIGNAL(rightButtonClicked()), this, SLOT(choosePlayer()));

    cdromDevice = new ClearLineEdit(this);
    cdromDevice->setPlaceholderText(tr("default device"));
    cdromDevice->setMinimumWidth(cdromDevice->fontMetrics().HORIZONTAL_ADVANCE(QSL("default device"))*3/2);
    playerLabel = new QLabel(tr("Player command"),this);
    cdromLabel = new QLabel(tr("CD-ROM device"),this);

    encaLanguageLabel = new QLabel(tr("Default language to use with\nthe enca utility"),this);
    encaGuessLanguage = new QComboBox(this);
    encaGuessLanguage->setEditable(false);
    encaGuessLanguage->addItems(QString("none.belarussian.bulgarian.chinese.croatian.czech."
                                        "estonian.hungarian.lithuanian.latvian.polish."
                                        "russian.slovak.slovene.ukrainian").split(QSL(".")));
    programsLabel = new QLabel(tr("External programs"),this);
    cueEncoding = new QComboBox(this);
    QStringList codecs;
    QList<int> codecMibs = QTextCodec::availableMibs();
    Q_FOREACH(const int &mib, codecMibs)
        codecs << QString(QTextCodec::codecForMib(mib)->name());
    codecs.sort();
    cueEncoding->addItem("Locale");
    cueEncoding->addItems(codecs);
    cueEncoding->setEditable(false);
    cueEncodingLabel = new QLabel(tr("Cue files encoding"), this);

    copyFiles = new QCheckBox(tr("Copy files into temp folder before replaygaining them"),this);
    copyFiles->setToolTip(tr("Enable this if you are encountering \"File not found\" messages\n"
                             "in the ReplayGain dialog"));

    tree = new QTreeWidget(this);
    tree->header()->hide();
    tree->setColumnCount(3);
    tree->setRootIsDecorated(false);
    tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QVector<QString> external {"mac","flac","shntool","enca",
                       #ifndef Q_OS_WIN
                               "mp3gain",
                       #endif
                               "aacgain","vorbisgain","metaflac","wvgain","mpcgain", "replaygain",
                               "mppdec"};
    Q_FOREACH(const QString &s, external) {
        auto *item = new QTreeWidgetItem(tree);
        item->setText(1,s);
    }

    auto *utilitiesl = new QFormLayout;
    utilitiesl->addRow(playerLabel,player);
    utilitiesl->addRow(cdromLabel,cdromDevice);
    utilitiesl->addRow(cueEncodingLabel, cueEncoding);
    utilitiesl->addRow(encaLanguageLabel,encaGuessLanguage);
    utilitiesl->addRow(copyFiles);
    utilitiesl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto *utilitieslayout = new QVBoxLayout;
    utilitieslayout->addWidget(programsLabel);
    utilitieslayout->addWidget(tree);
    utilitieslayout->addLayout(utilitiesl);
    utilitieslayout->setStretch(1,1);

    finalize(utilitieslayout);
}

void UtilitiesPage::setSettings()
{DD;
    player->setText(App->player);
    cdromDevice->setText(App->cdromDevice);
    encaGuessLanguage->setCurrentIndex(encaGuessLanguage->findText(App->encaGuessLanguage));
    copyFiles->setChecked(App->copyFiles);
    cueEncoding->setCurrentIndex(cueEncoding->findText(App->cueEncoding));
}

QString UtilitiesPage::description()
{DD;
    return tr("Utilities");
}

QString UtilitiesPage::iconFilename()
{DD;
    return "applications-utilities";
}

void UtilitiesPage::retranslateUI()
{DD;
    ConfigPage::retranslateUI();
    player->setButtonToolTip(FancyLineEdit::Right, tr("Choose..."));
    playerLabel->setText(tr("Player command"));
    cdromLabel->setText(tr("CD-ROM device"));
    encaLanguageLabel->setText(tr("Default language to use with\nthe enca utility"));
    cueEncodingLabel->setText(tr("Cue files encoding"));
    programsLabel->setText(tr("External programs"));
    cdromDevice->setPlaceholderText(tr("default device"));
    player->setPlaceholderText(tr("Path/to/player"));
    copyFiles->setText(tr("Copy files into temp folder before replaygaining them"));
    copyFiles->setToolTip(tr("Enable this if you are encountering \"File not found\" messages\n"
                             "in the ReplayGain dialog"));

    for (int i=0; i<tree->topLevelItemCount(); ++i) {
        QString programPath;
        bool installed = Qoobar::programInstalled(tree->topLevelItem(i)->text(1),&programPath);
        if (installed) {
            tree->topLevelItem(i)->setIcon(0,QIcon::fromTheme("tick"));
            tree->topLevelItem(i)->setText(2,programPath);
        }
        else {
            tree->topLevelItem(i)->setIcon(0,QIcon::fromTheme("exclamation"));
            tree->topLevelItem(i)->setText(2,tr("Cannot find in ")+programPath);
        }
        tree->topLevelItem(i)->setToolTip(2, tree->topLevelItem(i)->text(2));
    }
}

void UtilitiesPage::saveSettings()
{DD;
    App->player=player->text();
    App->cdromDevice=cdromDevice->text();
    App->encaGuessLanguage = encaGuessLanguage->currentText();
    App->copyFiles = copyFiles->isChecked();
    App->cueEncoding = cueEncoding->currentText();
}

void UtilitiesPage::choosePlayer()
{DD;
    QString filter;
#ifdef Q_OS_WIN
    filter=tr("Executable files (*.exe)");
#endif
#ifdef Q_OS_OS2
    filter=tr("Executable files (*.exe)");
#endif
    QString s=QFileDialog::getOpenFileName(this,tr("Choose a player file"),App->player,filter);
    if (!s.isEmpty())
        player->setText(s);
}

NetworkPage::NetworkPage(QWidget *parent) : ConfigPage(parent)
{DD;
    useProxy = new QGroupBox(tr("Use proxy server"),this);
    useProxy->setCheckable(true);
    proxyServer = new ClearLineEdit(this);
    proxyPort = new ClearLineEdit(this);
    proxyLogin = new ClearLineEdit(this);
    proxyPassword = new ClearLineEdit(this);
    //proxyPassword->setEchoMode(QLineEdit::Password);

    serverLabel = new QLabel(tr("Proxy server"),this);
    portLabel = new QLabel(tr("Port"),this);
    loginLabel = new QLabel(tr("Login"),this);
    passwordLabel = new QLabel(tr("Password"),this);
    auto *proxyLayout = new QFormLayout;
    proxyLayout->addRow(serverLabel,proxyServer);
    proxyLayout->addRow(portLabel,proxyPort);
    proxyLayout->addRow(loginLabel,proxyLogin);
    proxyLayout->addRow(passwordLabel,proxyPassword);
    proxyLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    useProxy->setLayout(proxyLayout);

    auto *networkLayout = new QVBoxLayout;
    networkLayout->addWidget(useProxy);
    networkLayout->addStretch();

    finalize(networkLayout);
}

void NetworkPage::setSettings()
{DD;
    useProxy->setChecked(App->useProxy);
    proxyServer->setText(App->proxyServer);
    proxyPort->setText(App->proxyPort);
    proxyLogin->setText(App->proxyLogin);
    proxyPassword->setText(App->proxyPassword);
}

QString NetworkPage::description()
{DD;
    return tr("Network");
}
QString NetworkPage::iconFilename()
{DD;
    return "applications-internet";
}
void NetworkPage::retranslateUI()
{DD;
    ConfigPage::retranslateUI();
    useProxy->setTitle(tr("Use proxy server"));

    serverLabel->setText(tr("Proxy server"));
    portLabel->setText(tr("Port"));
    loginLabel->setText(tr("Login"));
    passwordLabel->setText(tr("Password"));
}
void NetworkPage::saveSettings()
{DD;
    App->useProxy = useProxy->isChecked();
    App->proxyServer = proxyServer->text();
    App->proxyPort = proxyPort->text();
    App->proxyLogin = proxyLogin->text();
    App->proxyPassword = proxyPassword->text();
}


PluginsPage::PluginsPage(QWidget *parent) : ConfigPage(parent)
{DD;
    downloadLabel = new QLabel(tr("Download plugins:"),this);
    editingLabel = new QLabel(tr("Tags editing plugins:"),this);

    downloadTree = new QTreeWidget(this);
    downloadTree->setRootIsDecorated(false);
    downloadTree->setColumnCount(6);
    downloadTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    downloadTree->header()->setStretchLastSection(false);
#ifdef Q_OS_MAC
    //TODO: this->devicePixelRatioF()
    downloadTree->setFixedHeight(::dpiAwareSize(120,this));
#endif

    editingTree = new QTreeWidget(this);
    editingTree->setRootIsDecorated(false);
    editingTree->setColumnCount(3);
    editingTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    editingTree->header()->setStretchLastSection(false);
#ifdef Q_OS_MAC
    //TODO: this->devicePixelRatio()
    editingTree->setFixedHeight(::dpiAwareSize(120,this));
#endif

    auto *l = new QVBoxLayout;
#ifndef Q_OS_WIN
    QMargins m=l->contentsMargins();
    m.setBottom(0);
    l->setContentsMargins(m);
#endif
    l->addWidget(downloadLabel);
    l->addWidget(downloadTree);
    l->addWidget(editingLabel);
    l->addWidget(editingTree);

    finalize(l);
}

QString PluginsPage::description()
{DD;
    return tr("Plugins");
}

QString PluginsPage::iconFilename()
{DD;
    return "preferences-plugin";
}

void PluginsPage::retranslateUI()
{DD;
    ConfigPage::retranslateUI();
    downloadLabel->setText(tr("Download plugins:"));
    editingLabel->setText(tr("Tags editing plugins:"));
    downloadTree->setHeaderLabels(QStringList()<<tr("Name")<<tr("Version")<<tr("Description")
                                  <<tr("Manual")
                                  <<tr("By CD")
                                  <<tr("By files"));

    editingTree->setHeaderLabels(QStringList()<<tr("Name")<<tr("Version")<<tr("Description"));

    for (int j=0; j<downloadTree->topLevelItemCount(); ++j) {
        QJsonObject metaData = downloadTree->topLevelItem(j)->data(0, Qt::UserRole).toJsonObject();
        QString text = metaData.value(QSL("text")).toObject().value(App->langID).toString();
        if (!text.isEmpty())
            downloadTree->topLevelItem(j)->setText(0, text);
        QString description = metaData.value(QSL("description")).toObject().value(App->langID).toString();
        if (!description.isEmpty())
            downloadTree->topLevelItem(j)->setText(2, description);
    }

    for (int j=0; j<editingTree->topLevelItemCount(); ++j) {
        QJsonObject metaData = editingTree->topLevelItem(j)->data(0, Qt::UserRole).toJsonObject();
        QString text = metaData.value(QSL("text")).toObject().value(App->langID).toString();
        if (!text.isEmpty())
            editingTree->topLevelItem(j)->setText(0, text);
        QString description = metaData.value(QSL("description")).toObject().value(App->langID).toString();
        if (!description.isEmpty())
            editingTree->topLevelItem(j)->setText(2, description);
    }
}

void PluginsPage::setSettings()
{DD;
    for (const auto &metaData: App->downloadPlugins) {
        QString text = metaData.value(QSL("text")).toObject().value(App->langID).toString();
        if (text.isEmpty())
            text = metaData.value(QSL("text")).toObject().value(QSL("default")).toString();
        QString description = metaData.value(QSL("description")).toObject().value(App->langID).toString();
        if (description.isEmpty())
            description = metaData.value(QSL("description")).toObject().value(QSL("default")).toString();
        QString version = metaData.value(QSL("version")).toString();

        QTreeWidgetItem *item = new QTreeWidgetItem(downloadTree, QStringList()<<text
                                                    <<version <<description
                                                    <<QString()<<QString()<<QString());
        item->setData(0, Qt::UserRole, metaData);
        if (metaData.value(QSL("canSearchManually")).toBool()) item->setIcon(canSearchManuallyColumn, QIcon::fromTheme("tick"));
        if (metaData.value(QSL("canSearchByCD")).toBool())     item->setIcon(canSearchByCDColumn, QIcon::fromTheme("tick"));
        if (metaData.value(QSL("canSearchByFiles")).toBool())  item->setIcon(canSearchByFilesColumn, QIcon::fromTheme("tick"));
    }

    for (const auto &metaData: App->plugins) {
        QString text = metaData.value(QSL("text")).toObject().value(App->langID).toString();
        if (text.isEmpty())
            text = metaData.value(QSL("text")).toObject().value(QSL("default")).toString();
        QString description = metaData.value(QSL("description")).toObject().value(App->langID).toString();
        if (description.isEmpty())
            description = metaData.value(QSL("description")).toObject().value(QSL("default")).toString();
        QString version = metaData.value(QSL("version")).toString();

        QTreeWidgetItem *item = new QTreeWidgetItem(editingTree, QStringList()<<text
                                                    <<version <<description);
        item->setData(0, Qt::UserRole, metaData);
    }
}

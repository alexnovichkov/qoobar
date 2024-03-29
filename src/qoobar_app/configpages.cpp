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

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QtCore5Compat/QTextCodec>
#else
#include <QTextCodec>
#endif

#include <QJsonDocument>

#include "clearlineedit.h"
#include "fancylineedit.h"

#include <QVector>
#include <QtConcurrentRun>
#include "qeasysettings.hpp"

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
    dirRoot->setPlaceholderText(tr("Folder tree root"));
    dirRoot->setWhatsThis(tr("Sets the top level folder for the Folders navigation tree"));
    connect(dirRoot, SIGNAL(rightButtonClicked()), this, SLOT(chooseDirRoot()));

    useUndo = new QCheckBox(tr("Use undo / redo"),this);
    useUndo->setWhatsThis(tr("This box allows you to turn off the Undo/Redo system in Qoobar"));

    lang = new QComboBox(this);
    const QStringList ts=QDir(ApplicationPaths::translationsPath()).entryList(QStringList(QSL("*.qm")), QDir::Files | QDir::Readable);
    QTranslator translator;
    int index=-1;
    for (const QString &s: ts) {
        if (translator.load(ApplicationPaths::translationsPath() + "/" + s)) {
            QString ID=translator.translate("Settings","en");
            if (!ID.isEmpty()) {
                QString text=translator.translate("Settings","English");
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
    iconThemes.replaceInStrings("[dark]","");
    iconThemes.replaceInStrings("[light]","");
    iconThemes.removeDuplicates();
    for (const QString &dir: iconThemes) {
        //if (QFile::exists(ApplicationPaths::sharedPath()+"/icons/"+dir+"/index.theme")) {
            iconTheme->addItem(dir);
       // }
    }

#ifdef Q_OS_WIN
    styleLabel = new QLabel(tr("Style"), this);
    style = new QComboBox(this);
    styleLabel->setBuddy(style);
    style->addItems(QEasySettings::supportedStyles());
#endif

    statusBarTrackLabel = new QLabel(tr("Status bar is tracking"), this);
    statusBarTrack = new QComboBox(this);
    statusBarTrack->addItem(tr("current hovered file"));
    statusBarTrack->addItem(tr("current selected file"));

    sortOptionsLabel = new QLabel(tr("Sort file names and other text fields"), this);
    sortOptions = new QComboBox(this);
    sortOptions->addItem(tr("case sensitively"));
    sortOptions->addItem(tr("case insensitively"));

    auto *UIlayout = new QFormLayout;
    UIlayout->addRow(langLabel, lang);
#ifdef Q_OS_WIN
    UIlayout->addRow(styleLabel, style);
#endif
    UIlayout->addRow(iconThemeLabel, iconTheme);
    UIlayout->addRow(statusBarTrackLabel, statusBarTrack);
    UIlayout->addRow(sortOptionsLabel, sortOptions);
    UIlayout->addRow(dirBox,dirRoot);
    UIlayout->addRow(useUndo);
    UIlayout->addRow(hideTabBar);






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
    sortOptions->setCurrentIndex(App->sortOption);

    int langIndex = lang->findData(App->langID);
    if (langIndex>=0) lang->setCurrentIndex(langIndex);

    int iconThemeIndex = iconTheme->findText(App->iconTheme);
    if (iconThemeIndex<0) iconThemeIndex = 0;
    if (iconThemeIndex>=0) iconTheme->setCurrentIndex(iconThemeIndex);
#ifdef Q_OS_WIN
    int styleIndex = static_cast<int>(QEasySettings::readStyle());
    style->setCurrentIndex(styleIndex);
#endif
    hideTabBar->setChecked(App->hideTabBar);

    statusBarTrack->setCurrentIndex(App->statusBarTrack);
}

QString InterfacePage::description()
{DD;
    return tr("General");
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

    sortOptionsLabel->setText(tr("Sort file names and other text fields"));
    sortOptions->setItemText(0, tr("case sensitively"));
    sortOptions->setItemText(1, tr("case insensitively"));

    langLabel->setText(tr("User interface language"));
    useUndo->setText(tr("Use undo / redo"));
    dirBox->setText(tr("Show folder tree"));
    hideTabBar->setText(tr("Hide Tab bar with only one tab"));
    dirRoot->setPlaceholderText(tr("Folder tree root"));
    iconThemeLabel->setText(tr("Toolbar icons theme"));
#ifdef Q_OS_WIN
    styleLabel->setText(tr("Style"));
#endif
    dirBox->setWhatsThis(tr("Check this box to show or hide the Folders navigation tree"));
    dirRoot->setWhatsThis(tr("Sets the top level folder for the Folders navigation tree"));
    useUndo->setWhatsThis(tr("This box allows you to turn off the Undo/Redo system in Qoobar"));
}
void InterfacePage::saveSettings()
{DD;
    App->useUndo = useUndo->isChecked();
    App->showDirView = dirBox->isChecked();
    App->hideTabBar = hideTabBar->isChecked();
    App->dirViewRoot = dirRoot->text();
    if (App->iconTheme != iconTheme->itemText(iconTheme->currentIndex()))
        QMessageBox::information(this,tr("Qoobar"),tr("The toolbar icons theme will be changed\n"
                                                      "after you restart Qoobar"));
    App->iconTheme = iconTheme->itemText(iconTheme->currentIndex());
#ifdef Q_OS_WIN
    auto st = static_cast<QEasySettings::Style>(style->currentIndex());
    if (st != QEasySettings::readStyle())
        QMessageBox::information(this,tr("Qoobar"),tr("The style theme will be changed\n"
                                                      "after you restart Qoobar"));
    QEasySettings::writeStyle(st);
#endif
    App->statusBarTrack = statusBarTrack->currentIndex();
    App->sortOption = sortOptions->currentIndex();
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
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    connect(mapper, &QSignalMapper::mappedInt, this, &CompletionPage::editList);
#else
    connect(mapper,SIGNAL(mapped(int)),SLOT(editList(int)));
#endif
    completionTree = new QTreeWidget(this);
    completionTree->setRootIsDecorated(false);
    completionTree->setColumnCount(3);
    completionTree->setHeaderHidden(true);
    completionTree->header()->setStretchLastSection(false);
    completionTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    completionTree->setWhatsThis(tr("Check the tags for which you wish to use the autocompletion.<br>"
                                    "<br>The <i>Edit...</i> buttons allows you to manually change the remembered text lines"));
    completionTree->setAlternatingRowColors(true);
#ifdef OSX_SUPPORT_ENABLED
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
    autoexpand = new QCheckBox(tr("Automatically fill a tag "
                                  "when pasting a single line"),this);
#ifndef OSX_SUPPORT_ENABLED
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

    saveChanges = new QCheckBox(tr("Save changes when closing Qoobar"),this);
    trim = new QCheckBox(tr("Simplify whitespaces when saving files"),this);
    writeFieldsSeparately = new QCheckBox(tr("Write tags separated by ; in different fields"),this);

    auto *rwLayout = new QFormLayout;
    rwLayout->addRow(saveChanges);
    rwLayout->addRow(trim);
    rwLayout->addRow(writeFieldsSeparately);
    rwLayout->addRow(autoexpand);
    rwLayout->addRow(charsBox, chars);


    finalize(rwLayout);
}
void WritingPage::setSettings()
{DD;
    autoexpand->setChecked(App->autoexpand);
#ifndef OSX_SUPPORT_ENABLED
    chars->setText(App->chars);
    chars->setCursorPosition(0);
#endif

    saveChanges->setCheckState(App->saveChanges ? Qt::Checked : Qt::Unchecked);
    trim->setCheckState(App->trim ? Qt::Checked : Qt::Unchecked);
    writeFieldsSeparately->setChecked(App->writeFieldsSeparately);
}

void WritingPage::changeCharsFont()
{DD;
    bool ok;
    QFont font = QFontDialog::getFont(&ok, App->charsFont, this, tr("Choose a Chars list font"));
    if (ok) {
        App->charsFont=font;
    }
}


QString WritingPage::description()
{DD;
    return tr("Edit & Save");
}
QString WritingPage::iconFilename()
{DD;
    return "document-save";
}
void WritingPage::retranslateUI()
{DD;
    ConfigPage::retranslateUI();

    autoexpand->setText(tr("Automatically fill a tag "
                           "when pasting a single line"));
#ifndef OSX_SUPPORT_ENABLED
    chars->setButtonToolTip(FancyLineEdit::Right, tr("Font..."));
    charsBox->setText(tr("Characters"));
    chars->setWhatsThis(tr("Characters that will be shown in the Tags edit dialog"));
    charsBox->setWhatsThis(tr("Characters that will be shown in the Tags edit dialog"));
#endif
    saveChanges->setText(tr("Save changes when closing Qoobar"));
    trim->setText(tr("Simplify whitespaces when saving files"));
    writeFieldsSeparately->setText(tr("Write tags separated by ; in different fields"));
}
void WritingPage::saveSettings()
{DD;
#ifndef OSX_SUPPORT_ENABLED
    App->chars=chars->text();
#endif
    App->autoexpand=autoexpand->isChecked();

    App->saveChanges=saveChanges->isChecked();
    App->trim=trim->isChecked();
    App->writeFieldsSeparately = writeFieldsSeparately->isChecked();
}


PatternsPage::PatternsPage(QWidget *parent) : ConfigPage(parent)
{DD;
    patterns = new QListWidget(this);
    patterns->setDragEnabled(true);
    patterns->viewport()->setAcceptDrops(true);
    patterns->setDropIndicatorShown(true);
    patterns->setDragDropMode(QAbstractItemView::InternalMove);
    //patterns->setMinimumHeight(200);
    patterns->setAlternatingRowColors(true);

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

//    auto *patternsLayout = new QVBoxLayout;
//    patternsLayout->addWidget(renamingPatternsBox);
//    patternsLayout->setStretch(0,1);
//    //patternsLayout->addStretch();

//    finalize(patternsLayout);
    finalize(renamingLayout);
}

void PatternsPage::setSettings()
{DD;
    patterns->clear();
    patterns->addItems(App->patterns);
    for (int i=0; i<patterns->count();++i)
        patterns->item(i)->setFlags(patterns->item(i)->flags() | Qt::ItemIsEditable);
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
    addPatternButton->setText(tr("Add pattern"));
    removePatternButton->setText(tr("Remove pattern"));
}
void PatternsPage::saveSettings()
{DD;
    updatePatterns();
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
    if (auto *edit = qobject_cast<QLineEdit *>(patterns->itemWidget(patterns->currentItem())))
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

    tree = new QTreeWidget(this);
    tree->header()->hide();
    tree->setColumnCount(3);
    tree->setRootIsDecorated(false);
    tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QVector<QString> external {"ffmpeg","avconv","flac","shntool","enca",
//                       #ifndef Q_OS_WIN
//                               "mp3gain",
//                       #endif
//                               "aacgain","vorbisgain","metaflac","wvgain",
//                               "mpcgain", "replaygain", "mppdec"
    };
    Q_FOREACH(const QString &s, external) {
        auto *item = new QTreeWidgetItem(tree);
        item->setText(1,s);
    }

    auto *utilitiesl = new QFormLayout;
    utilitiesl->addRow(playerLabel,player);
    utilitiesl->addRow(cdromLabel,cdromDevice);
    utilitiesl->addRow(cueEncodingLabel, cueEncoding);
    utilitiesl->addRow(encaLanguageLabel,encaGuessLanguage);
//    utilitiesl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

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

    cacheSearchResults = new QCheckBox(tr("Cache search results"), this);
    searchInCachedResults = new QCheckBox(tr("Perform search in cached results"), this);
    clearCacheBtn = new QPushButton(tr("Clear cache"), this);
    connect(clearCacheBtn, &QPushButton::clicked, this, &NetworkPage::clearCache);


    auto *networkLayout = new QVBoxLayout;
    networkLayout->addWidget(useProxy);
    networkLayout->addWidget(cacheSearchResults);
    networkLayout->addWidget(searchInCachedResults);
    networkLayout->addWidget(clearCacheBtn);
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
    cacheSearchResults->setChecked(App->cacheSearchResults);
    searchInCachedResults->setChecked(App->searchInCachedResults);
}

void NetworkPage::clearCache()
{DD;
    QDir dir(ApplicationPaths::cachePath());
    if (!dir.exists()) return;
    auto list = dir.entryInfoList({"*.json"});
    if (list.isEmpty()) return;
    if (QMessageBox::question(this, tr("Clearing search cache"),
                              tr("Delete %n downloaded release(s) from the cache?").arg(list.size()))
        ==QMessageBox::Yes) {
        for (auto &f: list) QFile(f.canonicalFilePath()).remove();
    }
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
    cacheSearchResults->setText(tr("Cache search results"));
    searchInCachedResults->setText(tr("Perform search in cached results"));
    clearCacheBtn->setText(tr("Clear cache"));
}
void NetworkPage::saveSettings()
{DD;
    App->useProxy = useProxy->isChecked();
    App->proxyServer = proxyServer->text();
    App->proxyPort = proxyPort->text();
    App->proxyLogin = proxyLogin->text();
    App->proxyPassword = proxyPassword->text();
    App->cacheSearchResults = cacheSearchResults->isChecked();
    App->searchInCachedResults = searchInCachedResults->isChecked();
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

#ifdef OSX_SUPPORT_ENABLED
    const int height = 120;
    downloadTree->setFixedHeight(height);
#endif

    editingTree = new QTreeWidget(this);
    editingTree->setRootIsDecorated(false);
    editingTree->setColumnCount(3);
    editingTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    editingTree->header()->setStretchLastSection(false);
#ifdef OSX_SUPPORT_ENABLED
    editingTree->setFixedHeight(height);
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
    for (const auto &metaData: qAsConst(App->downloadPlugins)) {
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

    for (const auto &metaData: qAsConst(App->plugins)) {
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

ReplaygainPage::ReplaygainPage(QWidget *parent) : ConfigPage(parent)
{
    tagsLabel = new QLabel(tr("Replaygain tags"), this);
    tagsCombo = new QComboBox(this);
    tagsCombo->addItems(QStringList()<<tr("UPPERCASE")<<tr("lowercase"));
    tagsLabel->setBuddy(tagsCombo);

    modeLabel = new QLabel(tr("Replaygain mode"), this);
    modeCombo = new QComboBox(this);
    modeCombo->addItems(QStringList()<<tr("Standard")<<tr("Enhanced"));
    modeLabel->setBuddy(modeCombo);
    modeInfoLabel = new QLabel(this);
    connect(modeCombo, SIGNAL(currentIndexChanged(int)),this,SLOT(updateModeInfo(int)));

    loudnessLabel = new QLabel(tr("Loudness target"), this);
    loudnessCombo = new QComboBox(this);
    loudnessCombo->addItems(QStringList()<<tr("-18 LUFS (Replaygain 2.0)")<<tr("-23 LUFS (EBU R128)"));
    loudnessLabel->setBuddy(loudnessCombo);

    unitsLabel = new QLabel(tr("Loudness units"), this);
    unitsCombo = new QComboBox(this);
    unitsCombo->addItems(QStringList()<<tr("dB")<<tr("LU"));
    unitsLabel->setBuddy(unitsCombo);

    clipping = new QCheckBox(tr("Prevent clipping"), this);

    auto *l = new QFormLayout;
    l->addRow(tagsLabel, tagsCombo);
    l->addRow(modeLabel, modeCombo);
    l->addRow("", modeInfoLabel);
    l->addRow(loudnessLabel, loudnessCombo);
    l->addRow(unitsLabel, unitsCombo);
    l->addRow(clipping);

    finalize(l);
}

QString ReplaygainPage::description()
{
    return tr("Replaygain");
}

QString ReplaygainPage::iconFilename()
{
    return "replay-gain";
}

void ReplaygainPage::retranslateUI()
{
    tagsLabel->setText(tr("Replaygain tags"));
    tagsCombo->setItemText(0,tr("UPPERCASE"));
    tagsCombo->setItemText(1,tr("lowercase"));
    modeLabel->setText(tr("Replaygain mode"));
    modeCombo->setItemText(0,tr("Standard"));
    modeCombo->setItemText(1,tr("Enhanced"));
    updateModeInfo(modeCombo->currentIndex());
    loudnessLabel->setText(tr("Loudness target"));
    unitsLabel->setText(tr("Loudness units"));
    unitsLabel->setBuddy(unitsCombo);

    clipping->setText(tr("Prevent clipping"));
}

void ReplaygainPage::saveSettings()
{
    App->replaygainOptions.tagsCase = tagsCombo->currentIndex();
    App->replaygainOptions.mode = modeCombo->currentIndex();
    App->replaygainOptions.loudness = loudnessCombo->currentIndex();
    App->replaygainOptions.units = unitsCombo->currentIndex();
    App->replaygainOptions.preventClipping = clipping->isChecked();
}

void ReplaygainPage::setSettings()
{
    tagsCombo->setCurrentIndex(App->replaygainOptions.tagsCase);
    modeCombo->setCurrentIndex(App->replaygainOptions.mode);
    loudnessCombo->setCurrentIndex(App->replaygainOptions.loudness);
    unitsCombo->setCurrentIndex(App->replaygainOptions.units);
    clipping->setChecked(App->replaygainOptions.preventClipping);
}

void ReplaygainPage::updateModeInfo(int index)
{
    if (index == -1) modeInfoLabel->hide();
    else {
        modeInfoLabel->show();
        if (index==0) //standard mode
            modeInfoLabel->setText("<small>"+tr("In this mode only the standard\nGAIN and PEAK tags are written")
                                   +"</small>");
        if (index==1) //enhanced mode
            modeInfoLabel->setText("<small>"+tr("In this mode the additional RANGE and LOUDNESS tags are written")
                                   +"</small>");
    }
}


TagsPage::TagsPage(QWidget *parent) : ConfigPage(parent)
{
    readMp3Label = new QLabel(tr("Read in MP3 files"), this);
    writeMp3Label = new QLabel(tr("Write to MP3 files"), this);

    readMp3 = new QComboBox(this);
    readMp3->addItems(QStringList()<<tr("ID3v2 and APE tags")<<tr("ID3v2 tags only")<<tr("APE tags only"));
    writeMp3 = new QComboBox(this);
    writeMp3->addItems(QStringList()<<tr("ID3v2 and APE tags")<<tr("ID3v2 tags only")<<tr("APE tags only"));

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

    id3v2lineEndingLabel = new QLabel(tr("ID3v2 frames line ending"), this);

    id3v2LineEnding = new QComboBox(this);
    id3v2LineEnding->addItem(tr("CRLF - Windows style"));
    id3v2LineEnding->addItem(tr("LF - Unix and OS X style"));
    id3v2LineEnding->addItem(tr("CR - pre-OS X style"));
    id3v2lineEndingLabel->setBuddy(id3v2LineEnding);

    id3v2version = new QComboBox(this);
    id3v2version->setEditable(false);
    id3v2version->addItem(QSL("ID3 v2.3"));
    id3v2version->addItem(QSL("ID3 v2.4"));
    id3v2versionLabel = new QLabel(tr("ID3v2 tags version"),this);
    id3v2versionLabel->setBuddy(id3v2version);
    auto *id3v2versionLayout = new QHBoxLayout;
    id3v2versionLayout->addWidget(id3v2versionLabel);
    id3v2versionLayout->addWidget(id3v2version);

    oggLabel = new QLabel(tr("Ogg & Speex picture tag"),this);
    oggPicture = new QComboBox(this);
    oggPicture->addItem(tr("COVERART (old standard)"));
    oggPicture->addItem(tr("METADATA_BLOCK_PICTURE (new standard)"));

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

    mpcLabel = new QLabel(tr("Write ReplayGain info of Musepack files into"), this);
    mpcReplayGain = new QComboBox(this);
    mpcReplayGain->setEditable(false);
    mpcReplayGain->addItem(tr("File header"));
    mpcReplayGain->addItem(tr("APE tags"));

    schemesLabel = new QLabel(tr("Current tagging scheme"),this);
    schemesComboBox = new QComboBox(this);
    schemesComboBox->setEditable(false);
    schemesComboBox->addItem(tr("Default"),":/src/default.xml");
    const QFileInfoList schemeFiles = QDir(ApplicationPaths::userSchemesPath()).entryInfoList(QStringList(QLS("*.xml")))
            +QDir(ApplicationPaths::schemesPath()).entryInfoList(QStringList(QLS("*.xml")));

    for (const auto &file: schemeFiles) {
        const QString schemePath = file.canonicalFilePath();
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

    QFormLayout *mainL =new QFormLayout;
    mainL->addRow(readMp3Label, readMp3);
    mainL->addRow(writeMp3Label, writeMp3);
    mainL->addRow(id3v2versionLabel, id3v2version);
    mainL->addRow(id3v2lineEndingLabel, id3v2LineEnding);
    mainL->addItem(new QSpacerItem(10,10));
    mainL->addRow(id3v1writeLabel, id3v1write);
    mainL->addRow(new QLabel("<small>"+tr("(for mp3, flac, tta, mpc, wv, ape files)")
                              +"</small>", this));
    mainL->addRow(id3Label,id3v1encoding);
    mainL->addWidget(id3v1transliterate);
    mainL->addItem(new QSpacerItem(10,10));
    mainL->addRow(oggLabel,oggPicture);
    mainL->addRow(mpcLabel, mpcReplayGain);
    mainL->addItem(new QSpacerItem(10,10));
    mainL->addRow(schemesListLayout);

//    mainL->addStretch();
    finalize(mainL);
}

QString TagsPage::description()
{
    return tr("Tags");
}

QString TagsPage::iconFilename()
{
    return "tag-write";
}

void TagsPage::retranslateUI()
{
    ConfigPage::retranslateUI();
    readMp3Label->setText(tr("Read in MP3 files"));
    writeMp3Label->setText(tr("Write to MP3 files"));
    readMp3->setItemText(0, tr("ID3v2 and APE tags"));
    readMp3->setItemText(1, tr("ID3v2 tags only"));
    readMp3->setItemText(2, tr("APE tags only"));
    writeMp3->setItemText(0, tr("ID3v2 and APE tags"));
    writeMp3->setItemText(1, tr("ID3v2 tags only"));
    writeMp3->setItemText(2, tr("APE tags only"));
//    flacBox->setTitle(tr("Flac files"));
//    readFlac->setText(tr("Read:"));
//    writeFlac->setText(tr("Write:"));
//    flacreadID3->setText(tr("ID3v2"));
//    flacwriteID3->setText(tr("ID3v2"));
//    flacreadOgg->setText(tr("Ogg"));
//    flacwriteOgg->setText(tr("Ogg"));
    id3v1writeLabel->setText(tr("When writing tags"));

    oggPicture->setItemText(0,tr("COVERART (old standard)"));
    oggPicture->setItemText(1,tr("METADATA_BLOCK_PICTURE (new standard)"));

    oggLabel->setText(tr("Ogg & Speex picture tag"));
    id3v1write->setItemText(0,tr("update ID3v1 tag"));
    id3v1write->setItemText(1,tr("update ID3v1 tag only if it exists"));
    id3v1write->setItemText(2,tr("delete ID3v1 tag"));
    id3v1transliterate->setText(tr("Transliterate Russian words"));
    id3Label->setText(tr("ID3v1 tag encoding"));
    id3v2LineEnding->setItemText(0,tr("CRLF - Windows style"));
    id3v2LineEnding->setItemText(1,tr("LF - Unix and OS X style"));
    id3v2LineEnding->setItemText(2,tr("CR - pre-OS X style"));
    id3v2lineEndingLabel->setText(tr("ID3v2 frames line ending"));

    mpcLabel->setText(tr("Write ReplayGain info of Musepack files into"));
    mpcReplayGain->setItemText(0,tr("File header"));
    mpcReplayGain->setItemText(1,tr("APE tags"));

    id3v2versionLabel->setText(tr("ID3v2 tags version"));

    schemesLabel->setText(tr("Current tagging scheme"));
    schemesComboBox->setItemText(0,tr("Default"));
    addSchemeButton->setText(tr("Add"));
    editSchemeButton->setText(tr("Edit"));

    const auto schemesActions = schemesMenu->actions();
    schemesActions[0]->setText(tr("Clone this"));
    schemesActions[1]->setText(tr("Using Default"));

    removeSchemeButton->setText(tr("Remove"));
}

void TagsPage::saveSettings()
{
    App->mp3readape = readMp3->currentIndex()==0 || readMp3->currentIndex()==2;
    App->mp3readid3 = readMp3->currentIndex()==0 || readMp3->currentIndex()==1;
    App->mp3writeape = writeMp3->currentIndex()==0 || writeMp3->currentIndex()==2;
    App->mp3writeid3 = writeMp3->currentIndex()==0 || writeMp3->currentIndex()==1;
    App->oggPictureNew = (oggPicture->currentIndex()==1);
    App->id3v1Synchro=id3v1write->currentIndex();
    App->id3v1Transliterate=id3v1transliterate->isChecked();
    App->setId3v1Encoding(id3v1encoding->currentText());
    App->id3v2version = id3v2version->currentIndex()==0?3:4;
    App->id3v2LineEnding = id3v2LineEnding->currentIndex();
    App->mpcWriteRg = mpcReplayGain->currentIndex()==0;

//    App->flacreadid3=flacreadID3->isChecked();
//    App->flacreadogg=flacreadOgg->isChecked();
//    App->flacwriteid3=flacwriteID3->isChecked();
//    App->flacwriteogg=flacwriteOgg->isChecked();

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

void TagsPage::setSettings()
{
    if (App->mp3readid3 && App->mp3readape) readMp3->setCurrentIndex(0);
    else if (App->mp3readid3) readMp3->setCurrentIndex(1);
    else if (App->mp3readape) readMp3->setCurrentIndex(2);

    if (App->mp3writeid3 && App->mp3writeape) writeMp3->setCurrentIndex(0);
    else if (App->mp3writeid3) writeMp3->setCurrentIndex(1);
    else if (App->mp3writeape) writeMp3->setCurrentIndex(2);

    oggPicture->setCurrentIndex(App->oggPictureNew?1:0);
    id3v1write->setCurrentIndex(App->id3v1Synchro);
    id3v1transliterate->setChecked(App->id3v1Transliterate);
    id3v1encoding->setCurrentIndex(id3v1encoding->findText(App->id3v1Encoding));
    id3v2version->setCurrentIndex(App->id3v2version==4?1:0);
    id3v2LineEnding->setCurrentIndex(App->id3v2LineEnding);
//    flacreadID3->setChecked(App->flacreadid3);
//    flacreadOgg->setChecked(App->flacreadogg);
//    flacwriteID3->setChecked(App->flacwriteid3);
//    flacwriteOgg->setChecked(App->flacwriteogg);
    mpcReplayGain->setCurrentIndex(App->mpcWriteRg?0:1);

    int schemeIndex = schemesComboBox->findData(App->currentSchemeName);
    if (schemeIndex < 0) schemeIndex = 0;
    schemesComboBox->setCurrentIndex(schemeIndex);
    schemesComboBoxIndexChanged(schemeIndex);
}

void TagsPage::schemesComboBoxIndexChanged(const int index)
{DD;
    removeSchemeButton->setEnabled(index!=0);
}
void TagsPage::removeScheme()
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

void TagsPage::editScheme()
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

void TagsPage::addScheme()
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

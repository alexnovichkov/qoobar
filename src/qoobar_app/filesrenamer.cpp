/**
 * \file filesrenamer.cpp
 * Rename files dialog.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 8 Dec 2009
 *
 * Copyright (C) 2009  Alex Novichkov
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

#include "filesrenamer.h"
#include "application.h"
#include "tagger.h"
#include "filenamerenderer.h"
#include "legendbutton.h"
#include <QtWidgets>
#include <QResizeEvent>
#include "highlightdelegate.h"
#include "filedelegatehighlighter.h"
#include "fancylineedit.h"
#include "model.h"
#include "qoobarglobals.h"
#include "logging.h"
#include "qoobarhelp.h"

#include "qbutton.h"

class RenameTableWidget : public QTableWidget {
public:
    RenameTableWidget(int rows, int cols, QWidget *parent=0) :
        QTableWidget(rows, cols, parent) {}

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

void RenameTableWidget::resizeEvent(QResizeEvent *event)
{
    QTableWidget::resizeEvent(event);
    setColumnWidth(0, event->size().width()*0.45);
}

QWidgetAction *createWidgetAction(QWidget *widget1, QWidget *widget2, QWidget *parent=nullptr)
{DD;
    auto *a = new QWidgetAction(parent);
    auto *inner = new QWidget(parent);
    auto *l = new QHBoxLayout;
    //TODO: this->devicePixelRatio()
#ifdef OSX_SUPPORT_ENABLED
    l->setContentsMargins(5,1,5,1);
#else
    l->setContentsMargins(0,1,0,1);
#endif
    if (widget1) l->addWidget(widget1);
    if (widget2) l->addWidget(widget2);
    inner->setLayout(l);
    a->setDefaultWidget(inner);
    return a;
}

FileRenameDialog::FileRenameDialog(Model *model, QWidget *parent)
    : QDialog(parent), m(model)
{DD;
    setWindowTitle(tr("Move/Copy/Rename files"));
#ifdef OSX_SUPPORT_ENABLED
    setWindowModality(Qt::WindowModal);
#endif
    if (!m || !m->hasSelection()) return;

    oldFileNames = m->selectedFilesNames();

    filenameRenderer = new FileNameRenderer(m,this);
    highlighter = new FileDelegateHighlighter();

    ////////////////////////////////////////////////////////////////////////////

    operationComboBox = new QComboBox(this);
    operationComboBox->addItem(tr("rename/move files"));
    operationComboBox->addItem(tr("copy files"));
    operationComboBox->addItem(tr("rename folder"));
    operationComboBox->setEditable(false);
    operationComboBox->setCurrentIndex(App->renameOptions.renamingOperation);

    ////////////////////////////////////////////////////////////////////////////
    QString destinationFolder = App->renameOptions.destinationFolder;
    directoryGroup = new QCheckBox(tr("Output folder"),this);
    directoryGroup->setChecked(!destinationFolder.isEmpty());
    directoryGroup->setEnabled(App->renameOptions.renamingOperation != 2);

    directoryEdit = new FancyLineEdit(this);
    QPixmap pixmap(SMALL_ICON_SIZE, SMALL_ICON_SIZE);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.drawText(0,SMALL_ICON_SIZE-2,QSL("..."));

    directoryEdit->setButtonPixmap(FancyLineEdit::Right, pixmap);
    directoryEdit->setButtonVisible(FancyLineEdit::Right, true);
    directoryEdit->setButtonToolTip(FancyLineEdit::Right, tr("Open existing folder"));
    directoryEdit->setAutoHideButton(FancyLineEdit::Right, false);
    directoryEdit->setText(destinationFolder);
    directoryEdit->setEnabled(App->renameOptions.renamingOperation != 2 && directoryGroup->isChecked());

    ////////////////////////////////////////////////////////////////////////////

    patternEdit = new QComboBox(this);

    auto *patternLineEdit = new FancyLineEdit(this);
    QIcon icon = QIcon::fromTheme("editclear");
    QPixmap buttonPix = icon.pixmap(SMALL_ICON_SIZE);

    patternLineEdit->setButtonPixmap(FancyLineEdit::Right, buttonPix);
    patternLineEdit->setButtonVisible(FancyLineEdit::Right, true);
    patternLineEdit->setButtonToolTip(FancyLineEdit::Right, tr("Remove this pattern"));
    patternLineEdit->setAutoHideButton(FancyLineEdit::Right, true);
    connect(patternLineEdit, SIGNAL(rightButtonClicked()), this, SLOT(removePattern()));

    patternEdit->setLineEdit(patternLineEdit);

    patternEdit->setEditable(true);
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
     patternEdit->setAutoCompletion(true);
     patternEdit->setAutoCompletionCaseSensitivity(Qt::CaseSensitive);
#else
    patternEdit->completer()->setCaseSensitivity(Qt::CaseSensitive);
#endif
    if (!App->patterns.isEmpty()) {
        patternEdit->insertItems(0,App->patterns);
        filenameRenderer->setPattern(App->patterns.first());
        patternEdit->setEditText(App->patterns.first());
    }

    legendButton = new LegendButton(this);
    legendButton->setCategories(LegendButton::WritablePlaceholders | LegendButton::ReadOnlyPlaceholders);
    legendButton->retranslateUi();
    connect(legendButton,SIGNAL(placeholderChosen(QString)),SLOT(insertLegend(QString)));


    auto *patternLayout = new QHBoxLayout;
    patternLayout->addWidget(patternEdit,1);
    patternLayout->addWidget(legendButton);

    ////////////////////////////////////////////////////////////////////////////
    removeFolderCheckBox = new QCheckBox(tr("Delete empty folder after renaming"),this);
    removeFolderCheckBox->setChecked(App->renameOptions.removeFolder);
    removeFolderCheckBox->setEnabled(App->renameOptions.renamingOperation != 2);

    charactersCheckBox = new QCheckBox(tr("Replace special Windows characters \\/:*?\"<>| with"),this);
    charactersCheckBox->setToolTip(tr("Characters \\/:*| will be replaced by this text\n"
                                      "character ? will be removed\n"
                                      "characters <> will be replaced by (),\n"
                                      "and \" by two single quotes ''"));
    charactersCheckBox->setChecked(App->renameOptions.replaceWinChars);

    replaceCharacterEdit = new QLineEdit(this);
    replaceCharacterEdit->setText(App->renameOptions.winCharsReplacer);
    replaceCharacterEdit->setEnabled(App->renameOptions.replaceWinChars);

    spacesCheckBox = new QCheckBox(tr("Replace spaces with"));
    spacesCheckBox->setChecked(App->renameOptions.replaceSpaces);

    spacesEdit = new QLineEdit(this);
    spacesEdit->setText(App->renameOptions.spacesReplacer);
    spacesEdit->setEnabled(App->renameOptions.replaceSpaces);

    fileLengthSpinBox = new QSpinBox(this);
    fileLengthSpinBox->setMinimum(5);
    fileLengthSpinBox->setMaximum(25000);
    fileLengthSpinBox->setValue(App->renameOptions.maximumLength);
    fileLengthSpinBox->setEnabled(App->renameOptions.trimFileLength);
    fileLengthCheckBox = new QCheckBox(tr("Maximum file name length"),this);
    fileLengthCheckBox->setChecked(App->renameOptions.trimFileLength);

    caseCheckBox = new QCheckBox(tr("Case"),this);
    caseCheckBox->setChecked(App->renameOptions.changeCase);

    caseComboBox = new QComboBox(this);
    caseComboBox->setEditable(false);
    caseComboBox->addItem(tr("lower"));
    caseComboBox->addItem(tr("UPPER"));
    caseComboBox->addItem(tr("Only first up"));
    caseComboBox->addItem(tr("Every First Up"));
    caseComboBox->setCurrentIndex(App->renameOptions.ccase);
    caseComboBox->setEnabled(App->renameOptions.changeCase);

    diacriticsCheckBox = new QCheckBox(tr("Remove diacritics"),this);
    diacriticsCheckBox->setChecked(App->renameOptions.removeDiacritics);

    applyToFoldersCheckBox = new QCheckBox(tr("Apply these options to created folders as well"),this);
    applyToFoldersCheckBox->setChecked(App->renameOptions.applyToFolders);
    //applyToFoldersCheckBox->setEnabled(App->renameOptions.renamingOperation != 2);

    auto *optionsButton = new QPushButton(tr("Options"), this);
    auto *optionsMenu = new QMenu(this);

    QWidgetAction *deleteFolderWidget = createWidgetAction(removeFolderCheckBox, 0, this);
    QWidgetAction *charactersWidget = createWidgetAction(charactersCheckBox, replaceCharacterEdit, this);
    QWidgetAction *spacesWidget = createWidgetAction(spacesCheckBox,spacesEdit,this);
    QWidgetAction *fileLengthWidget = createWidgetAction(fileLengthCheckBox,fileLengthSpinBox,this);
    QWidgetAction *caseWidget = createWidgetAction(caseCheckBox,caseComboBox,this);
    QWidgetAction *diacriticsWidget = createWidgetAction(diacriticsCheckBox,0,this);
    QWidgetAction *foldersWidget = createWidgetAction(applyToFoldersCheckBox,0,this);

    optionsMenu->addAction(deleteFolderWidget);
    optionsMenu->addAction(charactersWidget);
    optionsMenu->addAction(spacesWidget);
    optionsMenu->addAction(fileLengthWidget);
    optionsMenu->addAction(caseWidget);
    optionsMenu->addAction(diacriticsWidget);
    optionsMenu->addAction(foldersWidget);

    optionsButton->setMenu(optionsMenu);

    ////////////////////////////////////////////////////////////////////////////
    table = new RenameTableWidget(oldFileNames.count(),2,this);
    table->setHorizontalHeaderLabels(QStringList() << tr("Old names") << tr("New names"));
    table->horizontalHeader()->setStretchLastSection(true);

    table->setTextElideMode(Qt::ElideLeft);
#ifdef OSX_SUPPORT_ENABLED
    table->setAttribute(Qt::WA_MacSmallSize, true);
#endif
    HighlightDelegate *delegate = new HighlightDelegate(this);
    delegate->setHighlighter(highlighter);
    table->setItemDelegateForColumn(1, delegate);
    for (int i=0; i<oldFileNames.count(); ++i) {
        QString s = QFileInfo(oldFileNames.at(i)).fileName();
        QTableWidgetItem *item = new QTableWidgetItem(s);
        item->setFlags(Qt::ItemIsSelectable  | Qt::ItemIsEnabled);
        table->setItem(i,0,item);
        QTableWidgetItem *item1 = new QTableWidgetItem(oldFileNames.at(i));
        item1->setFlags(Qt::ItemIsSelectable  | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        table->setItem(i,1,item1);
    }

    ////////////////////////////////////////////////////////////////////////////
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    runButton = buttonBox->addButton(tr("Run"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(run()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
#ifdef OSX_SUPPORT_ENABLED
    QButton *helpButton = new QButton(this,QButton::HelpButton);
#else
    QPushButton *helpButton = buttonBox->addButton(QDialogButtonBox::Help);
#endif
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));
    //////////////////////////////////////////////////////////////////////////////

#ifdef OSX_SUPPORT_ENABLED
    QVBoxLayout *l = new QVBoxLayout;
    QFormLayout *mainLayout = new QFormLayout;
    mainLayout->addRow(tr("Do what:"),operationComboBox);
    //TODO: this->devicePixelRatio()
    directoryEdit->setMinimumWidth(200);
    patternEdit->setMinimumWidth(200);
    mainLayout->addRow(directoryGroup, directoryEdit);
    mainLayout->addRow(tr("Output file name pattern"), patternLayout);
    mainLayout->addWidget(optionsButton);
    mainLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    l->addLayout(mainLayout);
    l->addWidget(table);
    QHBoxLayout *boxL = new QHBoxLayout;
    boxL->addWidget(helpButton);
    boxL->addWidget(buttonBox);
    l->addLayout(boxL);
    setLayout(l);
#else
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(new QLabel(tr("Do what:")),0,0);
    mainLayout->addWidget(operationComboBox,0,1);
    mainLayout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed),0,2,1,2);
    mainLayout->addWidget(directoryGroup,1,0);
    mainLayout->addWidget(directoryEdit,1,1,1,3);
    mainLayout->addWidget(new QLabel(tr("Output file name pattern")),2,0);
    mainLayout->addLayout(patternLayout,2,1,1,3);
    mainLayout->addWidget(optionsButton,3,1);
    mainLayout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed),3,2,1,2);
    mainLayout->addWidget(table,4,0,1,4);
    mainLayout->addWidget(buttonBox,5,0,1,4);
    mainLayout->setColumnStretch(0,1);
    mainLayout->setColumnStretch(1,1);
    mainLayout->setColumnStretch(2,10);
    setLayout(mainLayout);
#endif

    resize(qApp->primaryScreen()->availableSize()/2);

    table->setFocusPolicy(Qt::NoFocus);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //table->horizontalHeader()->resizeSection(0, std::round(table->width()*0.4));

    connect(operationComboBox,SIGNAL(currentIndexChanged(int)), SLOT(handleOperation(int)));
    connect(removeFolderCheckBox,SIGNAL(toggled(bool)),SLOT(handleRemoveFolders()));
    connect(directoryGroup,SIGNAL(clicked()),SLOT(directoryChanged()));
    connect(directoryEdit,SIGNAL(textChanged(QString)),this,SLOT(directoryChanged()));
    connect(directoryEdit, SIGNAL(rightButtonClicked()), this, SLOT(setExistingDirectory()));
    connect(patternEdit,SIGNAL(currentTextChanged(QString)),this,SLOT(patternChanged(QString)));
    connect(patternEdit,SIGNAL(editTextChanged(QString)),this,SLOT(patternChanged(QString)));
    connect(charactersCheckBox,SIGNAL(stateChanged(int)),SLOT(replaceWinCharsChanged()));
    connect(replaceCharacterEdit,SIGNAL(textEdited(QString)),SLOT(replaceWinCharsChanged()));
    connect(applyToFoldersCheckBox,SIGNAL(stateChanged(int)),SLOT(applyToFoldersChanged()));
    connect(diacriticsCheckBox,SIGNAL(stateChanged(int)),SLOT(diacriticsChanged()));
    connect(caseComboBox,SIGNAL(currentIndexChanged(int)),SLOT(caseChanged(int)));
    connect(caseCheckBox,SIGNAL(toggled(bool)),SLOT(updateCase(bool)));
    connect(fileLengthSpinBox,SIGNAL(valueChanged(int)),SLOT(fileLengthChanged(int)));
    connect(fileLengthCheckBox,SIGNAL(toggled(bool)),SLOT(updateFileLength(bool)));
    connect(spacesEdit,SIGNAL(textEdited(QString)),SLOT(replaceSpacesChanged()));
    connect(spacesCheckBox,SIGNAL(stateChanged(int)),SLOT(replaceSpacesChanged()));

    updateTable();
}

FileRenameDialog::~FileRenameDialog()
{DD;
    delete highlighter;
}

void FileRenameDialog::patternChanged(const QString &pattern)
{DD;
    if (filenameRenderer->setPattern(pattern))
        updateTable();
}

void FileRenameDialog::directoryChanged()
{DD;
    directoryEdit->setEnabled(directoryGroup->isChecked());
    if (directoryGroup->isChecked())
        App->renameOptions.destinationFolder = directoryEdit->text();
    else
        App->renameOptions.destinationFolder.clear();
    updateTable();
}

void FileRenameDialog::replaceWinCharsChanged()
{DD;
    App->renameOptions.replaceWinChars = charactersCheckBox->isChecked();
    App->renameOptions.winCharsReplacer = replaceCharacterEdit->text();
    replaceCharacterEdit->setEnabled(charactersCheckBox->isChecked());
    updateTable();
}

void FileRenameDialog::replaceSpacesChanged()
{DD;
    App->renameOptions.replaceSpaces = spacesCheckBox->isChecked();
    App->renameOptions.spacesReplacer = spacesEdit->text();
    spacesEdit->setEnabled(spacesCheckBox->isChecked());
    updateTable();
}

void FileRenameDialog::diacriticsChanged()
{DD;
    App->renameOptions.removeDiacritics = diacriticsCheckBox->isChecked();
    updateTable();
}

void FileRenameDialog::caseChanged(const int value)
{DD;
    App->renameOptions.ccase = value;
    updateTable();
}

void FileRenameDialog::updateCase(bool b)
{DD;
    caseComboBox->setEnabled(b);
    App->renameOptions.changeCase = b;
    updateTable();
}

void FileRenameDialog::fileLengthChanged(int value)
{DD;
    App->renameOptions.maximumLength = value;
    updateTable();
}

void FileRenameDialog::updateFileLength(bool value)
{DD;
    fileLengthSpinBox->setEnabled(value);
    App->renameOptions.trimFileLength = value;
    updateTable();
}

void FileRenameDialog::applyToFoldersChanged()
{DD;
    App->renameOptions.applyToFolders = applyToFoldersCheckBox->isChecked();
    updateTable();
}

void FileRenameDialog::removePattern()
{DD;
    QString pattern = patternEdit->currentText();
    patternEdit->removeItem(patternEdit->currentIndex());
    App->removePattern(pattern, App->patterns);
}

void FileRenameDialog::showHelp()
{DD;
    Qoobar::showHelp(QSL("renaming"));
}

void FileRenameDialog::handleOperation(int index)
{DD;
    removeFolderCheckBox->setEnabled(index == 0);
    directoryGroup->setEnabled(index != 2);
    directoryEdit->setEnabled(index != 2 && directoryGroup->isChecked());
    if (index == 2)
        applyToFoldersCheckBox->setChecked(true);
    //applyToFoldersCheckBox->setEnabled(index != 2);

    App->renameOptions.renamingOperation = index;

    updateTable();
}

void FileRenameDialog::handleRemoveFolders()
{DD;
    App->renameOptions.removeFolder = removeFolderCheckBox->isChecked();
}

void FileRenameDialog::insertLegend(const QString &s)
{DD;
    patternEdit->lineEdit()->insert(s);
}

void FileRenameDialog::setExistingDirectory()
{DD;
    QString directory = QFileDialog::getExistingDirectory(this,
            tr("Folder to move/copy files"),
            oldFileNames.isEmpty() ? QDir::homePath() : QDir(oldFileNames.at(0)).canonicalPath(),
            QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        directoryEdit->setText(directory);
}

void FileRenameDialog::updateTable()
{DD;
    filenameRenderer->setPattern(patternEdit->currentText());
    QStringList newFileNames = filenameRenderer->newTags();
    QStringList prefixes = filenameRenderer->getPrefixes();
    Q_ASSERT(newFileNames.size()==prefixes.size());
    for (int i=0; i<newFileNames.size(); ++i) {
        table->item(i,1)->setText(newFileNames.at(i));
        table->item(i,1)->setData(Qt::UserRole+1,prefixes.at(i));
    }
    runButton->setEnabled(newFileNames!=oldFileNames);
    runButton->setToolTip(newFileNames!=oldFileNames ? "":tr("Nothing to rename, file names not changed."));
}

void FileRenameDialog::run()
{DD;
    //add pattern to file renaming patterns
    App->addPattern(patternEdit->currentText(), App->patterns);

    newFileNames.clear();
    for (int i=0; i<oldFileNames.count(); ++i) newFileNames << table->item(i,1)->text();
    accept();
}

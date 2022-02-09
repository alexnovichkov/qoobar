/**
 * \file splitdialog.h
 * Dialog where you can split an image by cue file.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 13 Sep 2010
 *
 * Copyright (C) 2009, 2010  Alex Novichkov
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

#include "splitdialog.h"
#include "application.h"
#include <QtWidgets>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QtCore5Compat/QTextCodec>
#endif
#include "enums.h"

#include "cuesplitter.h"
#include "qoobarglobals.h"
#include "qbutton.h"
#include "qoobarhelp.h"

SplitDialog::SplitDialog(QWidget *parent) :
    QDialog(parent)
{DD;
    setWindowTitle(tr("Split CD image file by cue"));
    setWindowModality(Qt::WindowModal);

    splitter = new CueSplitter(this);
    connect(splitter, SIGNAL(message(int,QString)),SLOT(onMessage(int,QString)),Qt::QueuedConnection);
    connect(splitter, SIGNAL(textReady(QString)),this,SLOT(updateEdit(QString)),Qt::QueuedConnection);
    connect(splitter, SIGNAL(next()), SLOT(onNextFile()),Qt::QueuedConnection);

    bool ffmpegInstalled = splitter->ffmpegInstalled;

    warningLabel = new QLabel(this);
    warningLabel->setMaximumWidth(500);
    warningLabel->setOpenExternalLinks(true);
    warningLabel->setWordWrap(true);

    cueFileEdit = new QLineEdit(this);
    cueFileEdit->setReadOnly(true);
    openCueButton = new QPushButton(tr("Open"),this);
    connect(openCueButton,SIGNAL(clicked()),SLOT(openCue()));

    folderEdit = new QLineEdit(this);
    folderEdit->setReadOnly(true);
    folderButton = new QPushButton(tr("Change"),this);
    connect(folderButton,SIGNAL(clicked()),SLOT(changeDir()));

    edit = new QPlainTextEdit(this);
    edit->setReadOnly(true);
    edit->setCenterOnScroll(true);
    edit->setMaximumBlockCount(1000);

    progress = new QProgressBar(this);
    progress->setMinimum(0);
    progress->setTextVisible(false);


    QComboBox *cueEncoding = new QComboBox(this);
    QStringList codecs;

    QList<int> codecMibs = QTextCodec::availableMibs();
    Q_FOREACH(const int &mib, codecMibs)
        codecs << QString(QTextCodec::codecForMib(mib)->name());

    codecs.sort();
    cueEncoding->addItem("Locale");
    cueEncoding->addItems(codecs);
    cueEncoding->setEditable(false);
    cueEncoding->setCurrentIndex(cueEncoding->findText(App->cueEncoding));
    connect(cueEncoding, SIGNAL(currentTextChanged(QString)), this, SLOT(changeCueEncoding(QString)));

    QLabel *cueEncodingLabel = new QLabel(tr("Warning! Cue file contains non-Latin characters!\n"
                                             "Please choose the encoding:"), this);

    cueEncodingWidget = new QWidget(this);
    QHBoxLayout *cueEncodingLayout = new QHBoxLayout;
    cueEncodingLayout->setContentsMargins(0,0,0,0);
    cueEncodingLayout->addWidget(cueEncodingLabel);
    cueEncodingLayout->addWidget(cueEncoding);
    cueEncodingWidget->setLayout(cueEncodingLayout);
    cueEncodingWidget->hide();


    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    startButton = buttonBox->addButton(tr("Start"), QDialogButtonBox::AcceptRole);
    QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(startButton,SIGNAL(clicked()),SLOT(run()));
    connect(cancelButton,SIGNAL(clicked()),SLOT(cancel()));
#ifdef OSX_SUPPORT_ENABLED
    QButton *helpButton = new QButton(this,QButton::HelpButton);
#else
    QPushButton *helpButton = buttonBox->addButton(QDialogButtonBox::Help);
#endif
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));


    formatBox = new QComboBox(this);
    formatBox->setEditable(false);
    QStringList formats = splitter->formats();
    formatBox->addItems(formats);
    formatBox->setCurrentIndex(formatBox->findText(ffmpegInstalled?App->defaultSplitFormat:QSL("flac")));
    formatBox->setEnabled(ffmpegInstalled);

    splitter->check();

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(warningLabel,0,0,1,3);
    grid->addWidget(new QLabel(tr("Cue file:"),this),2,0);
    grid->addWidget(cueFileEdit,2,1);
    grid->addWidget(openCueButton,2,2);
    grid->addWidget(new QLabel(tr("Output folder:"),this),3,0);
    grid->addWidget(folderEdit,3,1);
    grid->addWidget(folderButton,3,2);
    grid->addWidget(new QLabel("Output format:", this),4,0);
    grid->addWidget(formatBox,4,1);

    if (!ffmpegInstalled) {
        QString warningString = tr("If you want to change the default output format, please install ffmpeg or avconv");
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
        warningString = tr("If you want to change the default output format, please copy "
                           "<br>ffmpeg.exe into the Qoobar installation folder. "
                           "<br>You can download it from %1."
                           "<br>OS/2 users can find ffmpeg.exe %2here%3")
                        .arg("<a href=\"https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full.7z\">"
                             "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full.7z</a>")
                        .arg("<a href=\"http://hobbes.nmsu.edu/download/pub/os2/apps/mmedia/ffmpeg-os2-r2_5_1-Fourier.zip\">")
                        .arg("</a>");
#endif
        formatBox->setToolTip(warningString);
        grid->itemAtPosition(4,0)->widget()->setToolTip(warningString);

        edit->appendHtml(warningString);
    }
    grid->addWidget(edit,5,0,1,3);
    grid->addWidget(progress,6,0,1,3);
    grid->addWidget(cueEncodingWidget, 8,0,1,3);
#ifdef OSX_SUPPORT_ENABLED
    QHBoxLayout *boxL = new QHBoxLayout;
    boxL->addWidget(helpButton);
    boxL->addWidget(buttonBox);
    grid->addLayout(boxL,10,0,1,3,Qt::AlignRight);
#else
    grid->addWidget(buttonBox,10,0,1,3,Qt::AlignRight);
#endif
    const int dpisize = 10;
    grid->setRowMinimumHeight(1,dpisize);
    grid->setRowMinimumHeight(4,dpisize);
    grid->setRowMinimumHeight(8,dpisize);

    setLayout(grid);
    warningLabel->setHidden(warningLabel->text().isEmpty());
    startButton->setEnabled(warningLabel->text().isEmpty());

    resize(qApp->primaryScreen()->availableSize()/2);
}

SplitDialog::~SplitDialog()
{DD;
    if (formatBox)
        App->defaultSplitFormat = formatBox->currentText();
}

void SplitDialog::changeCueEncoding(const QString &encoding)
{DD;
    App->cueEncoding = encoding;
}

void SplitDialog::showHelp()
{DD;
    Qoobar::showHelp(QSL("split"));
}

void SplitDialog::cancel()
{DD;
    splitter->stop();
    reject();
}

void SplitDialog::changeDir()
{DD;
    QString s=QFileDialog::getExistingDirectory(this,tr("Choose a folder to save files"),splitter->outputDir());
    if (!s.isEmpty()) {
        splitter->setOutputDir(s);
        folderEdit->setText(s);
    }
}

void SplitDialog::onNextFile()
{DD;
    progress->setValue(progress->value()+1);
}

void SplitDialog::setCueFile(const QString &s)
{DD;
    splitter->setCueFile(s);
    cueFileEdit->setText(s);
    if (splitter->trackCount()>0)
        progress->setMaximum(splitter->trackCount());

    cueEncodingWidget->setHidden(!splitter->notLatin1());
    folderEdit->setText(splitter->outputDir());
}

void SplitDialog::openCue()
{DD;
    QString s=QFileDialog::getOpenFileName(this, tr("Choose a cue file"),
                                           App->lastCueFile, QSL("*.cue"));
    if (!s.isEmpty()) setCueFile(s);
}

void SplitDialog::run()
{DD;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    openCueButton->setEnabled(false);
    cueFileEdit->setEnabled(false);
    folderEdit->setEnabled(false);
    folderButton->setEnabled(false);
    startButton->setEnabled(false);

    splitter->setFormat(formatBox->itemText(formatBox->currentIndex()));
    splitter->split();
    splittedFiles = splitter->files();

    QApplication::restoreOverrideCursor();
    progress->setValue(progress->maximum());

    if (!splittedFiles.isEmpty()) {
        accept();
    }
}


void SplitDialog::onMessage(int type, const QString &text)
{DD;
    QString prefix;
    switch (type) {
        case MT_ERROR: prefix = QSL("<font color=red>"); break;
        case MT_WARNING: prefix = QSL("<font color=red>"); break;
        case MT_INFORMATION: prefix = QSL("<font color=blue>"); break;
    }
    edit->appendHtml(prefix+text+QSL("</font><p>"));
}

void SplitDialog::updateEdit(const QString &text)
{DD;
    edit->moveCursor(QTextCursor::End);
    if (text.contains("\033[")) {
        QString s = text;

        s = s.replace("\033[34m", "<font color=blue>");
        s = s.replace("\033[31m", "<font color=red>");
        s = s.replace("\033[32m", "<font color=green>");
        s = s.replace("\033[0m", "</font>");
        s = s.replace("\n", "<br>");
        edit->appendHtml(s);
    }
    else {
        edit->insertPlainText(text);
    }
}

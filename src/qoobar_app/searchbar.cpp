#include "searchbar.h"

#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <QThread>

#include "enums.h"
#include "qoobarglobals.h"
#include "qocoa/qprogressindicatorspinning.h"
#include "application.h"
#include "fancylineedit.h"
#include "stringroutines.h"
#include "tagsreaderwriter.h"

Searcher::Searcher()
{DD
    paths = App->searchPaths;
    stopped = false;
    filters = Qoobar::musicalFilters();
}

Searcher::~Searcher()
{DD

}

void Searcher::setSearchParametres(bool caseSensitive, bool useRegularExpressions, bool wholeWord, const QString &text)
{DD
    this->text = removeDiacritics(text);
    regExp = createRegExp(caseSensitive, useRegularExpressions, wholeWord, this->text);
}

void Searcher::process()
{DD
    stopped = false;
    Q_FOREACH (const QString &path, paths) {
        searchDir(path);
    }
    stopped = false;
    Q_EMIT finished();
}

void Searcher::stop()
{DD
    stopped = true;
}

void Searcher::reset()
{DD
    stopped = false;
}

void Searcher::searchDir(const QString &file)
{DD
    if (stopped) {
        Q_EMIT finished();
        return;
    }

    if (QFileInfo(file).isDir()) {
        Q_EMIT pathChanged(file);

        QFileInfoList dirLst = QDir(file).entryInfoList(filters,
                                                        QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot,
                                                        QDir::DirsFirst);
        for (int i=0; i<dirLst.count(); ++i) {
            if (dirLst.at(i).isDir()) {
                searchDir(dirLst.at(i).absoluteFilePath());
            }
            else
                maybeAdd(dirLst.at(i).absoluteFilePath());
        }
    }
    else {
        maybeAdd(file);
    }
}

void Searcher::maybeAdd(const QString &file)
{DD
    if (stopped) {
        Q_EMIT finished();
        return;
    }

//    QEventLoop loop;
//    QTimer::singleShot(200, &loop, SLOT(quit()));
//    loop.exec();

    const int tagsCount = App->currentScheme->tagsCount();
    Tag tag(file, tagsCount);
    TagsReaderWriter trw(&tag);
    trw.readTags();

    if (regExp.indexIn(removeDiacritics(file), 0) >= 0) {
         Q_EMIT found(tag);
         return;
    }

    for (int i=0; i<tagsCount; ++i) {
        if (regExp.indexIn(removeDiacritics(tag.tag(i)), 0) >= 0) {
            Q_EMIT found(tag);
            return;
        }
    }

    QMap<QString,QString> userTags = tag.userTags();
    Q_FOREACH(const QString &val, userTags.values()) {
        if (regExp.indexIn(removeDiacritics(val), 0) >= 0) {
            Q_EMIT found(tag);
            return;
        }
    }
}

SearchBar::SearchBar(QWidget *parent) :
    QToolBar(parent)
{DD;
    progress=0;
    qRegisterMetaType<Tag>("Tag");

    pathsButton = new QPushButton(this);
#ifndef Q_OS_MAC
    pathsButton->setFlat(true);
#endif
    pathsMenu = createPathsMenu();
    pathsButton->setMenu(pathsMenu);
    addWidget(pathsButton);

    startSearchButton = new QPushButton(this);
    connect(startSearchButton, SIGNAL(clicked()), SLOT(toggleSearch()));

    textEdit = new SearchLineEdit(this);
    textEdit->setMaximumWidth(200);
    connect(textEdit, SIGNAL(textChanged(QString)), this, SLOT(updateStartSearchButton()));
    updateStartSearchButton();
    addWidget(textEdit);
    addWidget(startSearchButton);

#ifndef Q_OS_MAC
    QWidget* spacer = new QWidget(this);
    spacer->setFixedSize(5,1);
    addWidget(spacer);
#endif
    progress = new QProgressIndicatorSpinning(this);
    progress->animate(false);
    addWidget(progress);
    progress->hide();
#ifndef Q_OS_MAC
    QWidget* spacer1 = new QWidget();
    spacer1->setFixedSize(5,1);
    addWidget(spacer1);
#endif

    currentPathLabel = new QLineEdit(this);
    currentPathLabel->setReadOnly(true);
    QPalette palette = currentPathLabel->palette();
    palette.setColor(QPalette::Base, Qt::transparent);
    currentPathLabel->setPalette(palette);
    currentPathLabel->setFrame(false);
    addWidget(currentPathLabel);

    closeSearchPanel = new QAction(this);
    closeSearchPanel->setIcon(style()->standardPixmap(QStyle::SP_DockWidgetCloseButton));
    connect(closeSearchPanel,SIGNAL(triggered()),this,SLOT(hide()));
    addAction(closeSearchPanel);


    textEdit->setFocus();

    thread = new QThread;
    searcher = new Searcher();
    searcher->moveToThread(thread);

    connect(thread, SIGNAL(started()), searcher, SLOT(process()), Qt::QueuedConnection);
    connect(searcher, SIGNAL(found(Tag)), this, SIGNAL(addFile(Tag)), Qt::QueuedConnection);
    connect(searcher, SIGNAL(pathChanged(QString)), currentPathLabel, SLOT(setText(QString)), Qt::QueuedConnection);
    connect(searcher, SIGNAL(finished()), thread, SLOT(quit()));
    connect(searcher, SIGNAL(finished()), this, SLOT(resetSearch()));
    connect(searcher, SIGNAL(finished()), this, SLOT(quitSearch()));

    retranslateUI();
}

SearchBar::~SearchBar()
{DD;
    thread->quit();
    thread->wait();
    thread->deleteLater();

    searcher->stop();
    searcher->deleteLater();
}

void SearchBar::retranslateUI()
{DD;
    pathsButton->setText(tr("Paths"));
    startSearchButton->setText(tr("Start search"));
#if QT_VERSION >= 0x040700
    textEdit->setPlaceholderText(tr("text to search"));
#endif
    closeSearchPanel->setToolTip(tr("Close search bar"));
}

void SearchBar::removePath()
{DD;
    if (App->searchPaths.size()<=1) return;

    delete pathsMenu;
    App->searchPaths.removeAll(sender()->property("path").toString());


    pathsMenu = createPathsMenu();
    pathsButton->setMenu(pathsMenu);
}

void SearchBar::addPath()
{DD;
    QString newPath = QFileDialog::getExistingDirectory(this, tr("Choose a folder with music"));
    if (newPath.isEmpty() || App->searchPaths.contains(newPath)) return;

    bool add = true;
    Q_FOREACH(const QString &path, App->searchPaths) {
        if (newPath.startsWith(path)) {
            add = false;
            break;
        }
    }

    if (add) {
        App->searchPaths << newPath;
        delete pathsMenu;
        pathsMenu = createPathsMenu();
        pathsButton->setMenu(pathsMenu);
    }
}

void SearchBar::updateStartSearchButton()
{DD;
    startSearchButton->setDisabled(textEdit->text().isEmpty());
}

void SearchBar::toggleSearch()
{DD;
    if (startSearchButton->text() == tr("Stop search")) {
        stopSearch();
    }
    else {
        startSearch();
    }
}

void SearchBar::startSearch()
{DD;
    if (App->searchPaths.isEmpty()) {
        addPath();
    }

    if (textEdit->text().isEmpty()) {
        return;
    }
    Q_EMIT reset();
    currentPathLabel->clear();

    startSearchButton->setText(tr("Stop search"));
    if (progress) {
        progress->show();
        progress->animate(true);
    }
    searcher->stop();
    searcher->setSearchParametres(textEdit->caseSensitive(), textEdit->useRegularExpressions(),
                      textEdit->wholeWord(), textEdit->text());
    searcher->setPaths(App->searchPaths);
    thread->start();
}

void SearchBar::stopSearch()
{DD;
    if (searcher) {
        searcher->stop();
    }
    resetSearch();
}

void SearchBar::resetSearch()
{DD;
    startSearchButton->setText(tr("Start search"));
    if (progress) {
        progress->animate(false);
        progress->hide();
    }
}

void SearchBar::quitSearch()
{DD;
    currentPathLabel->setText(tr("Done"));
}

QMenu *SearchBar::createPathsMenu()
{DD;
    QMenu *menu = new QMenu(this);

    Q_FOREACH(const QString &path, App->searchPaths) {
        menu->addAction(createAction(path));
    }

    addPathAct = new QAction(tr("Add path"), this);
    connect(addPathAct, SIGNAL(triggered()), SLOT(addPath()));
    menu->addAction(addPathAct);

    return menu;
}

QWidgetAction *SearchBar::createAction(const QString &text)
{DD;
    QWidgetAction *act = new QWidgetAction(this);

    QLabel *label = new QLabel(text, this);
    QPushButton *button = new QPushButton(this);
    button->setIcon(QIcon(App->iconThemeIcon("editclear.png")));
    button->setIconSize(QSize(16,16));
    button->setFlat(true);
    button->setProperty("path", text);
    button->setToolTip(tr("Remove"));
    connect(button,SIGNAL(clicked()),SLOT(removePath()));
    QWidget *w = new QWidget(this);
    QHBoxLayout *l = new QHBoxLayout;
    l->setContentsMargins(2,2,2,2);
    l->addWidget(label);
    l->addStretch();
    l->addWidget(button);
    w->setLayout(l);
    act->setDefaultWidget(w);
    return act;
}

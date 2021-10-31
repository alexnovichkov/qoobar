/**
 * \file tagsfiller.cpp
 * Dialog for filling tags from files names.
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

#include "tagsfiller.h"

#include <QtWidgets>

#include "corenetworksearch.h"
#include "application.h"
#include "legendbutton.h"
#include "releaseinfowidget.h"
#include "macsplitter.h"
#include "checkableheaderview.h"

#include "enums.h"
#include "qoobarglobals.h"
#include "logging.h"
#include "qoobarhelp.h"
#include "tagparser.h"

#include "idownloadplugin.h"
#include "clearlineedit.h"
#include "placeholders.h"
#include "qprogressindicatorspinning.h"
#include "qbutton.h"
#include "importmodel.h"

class SearchResultsListDelegate : public QItemDelegate
{
public:
    SearchResultsListDelegate(QObject *parent) : QItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (!painter) return;

        painter->save();

        if (index.isValid()) {
            if (!index.parent().isValid()) {
                QStringList albumInfo = index.data(Qt::DisplayRole).toStringList();
                if (albumInfo.isEmpty()) {
                    QItemDelegate::paint(painter,option,index);
                    painter->restore();
                    return;
                }
                drawBackground(painter, option, index);
                drawFocus(painter, option, option.rect);

                QRectF albumRect=option.rect;
                albumRect.setLeft(albumRect.left()+5);
                int num = albumInfo.size();
                int h=albumRect.height()/num;
                albumRect.setHeight(h);
                int w = albumRect.width();

                QFont albumFont = option.font;
                albumFont.setBold(true);
                painter->setFont(albumFont);
                if (option.state & QStyle::State_Selected)
                    painter->setPen(option.palette.color(QPalette::HighlightedText));
                painter->drawText(albumRect,Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                                  option.fontMetrics.elidedText(albumInfo.at(0),Qt::ElideRight,w));

                painter->setFont(option.font);
                QRectF extraDataRect=albumRect;
                for (int i=1; i<num; ++i) {
                    extraDataRect=extraDataRect.translated(0,h);
                    painter->drawText(extraDataRect,Qt::AlignLeft | Qt::TextSingleLine,
                                      option.fontMetrics.elidedText(albumInfo.at(i),Qt::ElideRight,w));
                }
            }
            else QItemDelegate::paint(painter,option,index);
        }
        else QItemDelegate::paint(painter,option,index);

        painter->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (!index.isValid()) return option.rect.size();
        QStringList albumInfo = index.data(Qt::DisplayRole).toStringList();
        return QSize(option.rect.width(),option.fontMetrics.height()*albumInfo.size());
    }
};

QStringList excludeByRole(QList<Artist> &l, const QStringList &patterns)
{DD;
    QStringList result;

    //I ignore artist tracks because I don't know its pattern.
    QMutableListIterator<Artist> i(l);
    while (i.hasNext()) {
        Artist a = i.next();
        QString role = a.fields.value(QSL("role"));
        if (role.isEmpty()) continue;

        Q_FOREACH (const QString &s,patterns)
        if (role.contains(s,Qt::CaseInsensitive)) {
            if (!result.contains(a.fields.value(QSL("name"))))
                result << a.fields.value(QSL("name"));
            i.remove();
        }
    }
    result.removeDuplicates();
    return result;
}

TagsFillDialog::TagsFillDialog(const QList<Tag> &oldTags, QWidget *parent)
    : QDialog(parent)
{DD;
    setWindowModality(Qt::WindowModal);
    this->oldTags=oldTags;
    newTags = this->oldTags;
    setWindowTitle(tr("Fill tags"));
    tab = new QTabWidget(this);

//import from files names
    count = oldTags.size();
    QLabel *patternLabel = new QLabel(tr("Pattern:"),this);

    tagsSourceComboBox = new QComboBox(this);
    tagsSourceComboBox->setEditable(false);
    tagsSourceComboBox->addItem(tr("File name"));
    tagsSourceComboBox->addItem(tr("Clipboard"));
    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<tagsCount; ++i)
        tagsSourceComboBox->addItem(App->currentScheme->localizedFieldName[i]);
    tagsSourceComboBox->adjustSize();
    connect(tagsSourceComboBox,SIGNAL(currentIndexChanged(int)),SLOT(setSource(int)));


    patternEdit = new QComboBox(this);
    patternEdit->setSizePolicy(QSizePolicy::Expanding, patternEdit->sizePolicy().verticalPolicy());
    patternEdit->setEditable(true);
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    patternEdit->setAutoCompletion(true);
    patternEdit->setAutoCompletionCaseSensitivity(Qt::CaseSensitive);
#else
    patternEdit->completer()->setCaseSensitivity(Qt::CaseSensitive);
#endif
    if (!App->fillPatterns.isEmpty()) {
        patternEdit->insertItems(0,App->fillPatterns);
        patternEdit->setEditText(App->fillPatterns.first());
    }
    connect(patternEdit,SIGNAL(currentTextChanged(QString)),this, SLOT(updateTags()));
    connect(patternEdit,SIGNAL(editTextChanged(QString)),this, SLOT(updateTags()));

    legendButton = new LegendButton(this);
    legendButton->setCategories(LegendButton::WritablePlaceholders | LegendButton::VoidPlaceholder);
    connect(legendButton,SIGNAL(placeholderChosen(QString)),SLOT(insertLegend(QString)));
    legendButton->setFocusPolicy(Qt::NoFocus);
    legendButton->retranslateUi();

    table = new QTableView(this);

#ifdef OSX_SUPPORT_ENABLED
    table->setAttribute(Qt::WA_MacSmallSize, true);
    table->setWordWrap(true);
#else
    table->setWordWrap(false);
#endif
    importModel = new ImportModel(table);
    importModel->setTags(oldTags);
    table->setModel(importModel);

    header = new HeaderView(Qt::Horizontal,table);
    table->setHorizontalHeader(header);
    table->setColumnWidth(0,25);
    table->setColumnWidth(1,400);
//    header->setCheckable(0,true);
    table->resizeColumnToContents(0);

    setSource(0);
    updateTags();

    QHBoxLayout *patternLayout = new QHBoxLayout;
    patternLayout->addWidget(new QLabel(tr("Source:"),this));
    patternLayout->addWidget(tagsSourceComboBox);
    patternLayout->addWidget(patternLabel);
    patternLayout->addWidget(patternEdit);
    patternLayout->addWidget(legendButton);

    QVBoxLayout *flayout = new QVBoxLayout;
    flayout->addLayout(patternLayout);
    flayout->addWidget(table);
    QWidget *fw = new QWidget;
    fw->setLayout(flayout);
    tab->addTab(fw,tr("from other tags"));

//*************************************************************
    //from Internet

    networkStatusInfo = new QLabel(this);
    networkErrorInfo = new QLabel(this);
    networkErrorInfo->setWordWrap(true);

    sourceComboBox = new QComboBox(this);
    sourceComboBox->setEditable(false);
    sourceComboBox->setIconSize(QSize(50,17));

    for (int it = 0; it < App->downloadPlugins.size(); ++it) {
        QJsonObject metaData = App->downloadPlugins.at(it);

        QIcon icon(metaData.value(QSL("icon")).toString());
        //QIcon icon("K:/My/build/qoobar/src/plugins/freedb/freedb.png");
        QString text = metaData.value(QSL("text")).toObject().value(App->langID).toString();
        if (text.isEmpty())
            text = metaData.value(QSL("text")).toObject().value(QSL("default")).toString();
        QString key = metaData.value(QSL("path")).toString();
        sourceComboBox->addItem(icon, text, key);
    }
    sourceComboBox->adjustSize();
    connect(sourceComboBox,SIGNAL(currentIndexChanged(int)),SLOT(handleSourceComboBox(int)));


    artistEdit = new ClearLineEdit(this);
    albumEdit = new ClearLineEdit(this);

    artistEdit->setEnabled(false);
    albumEdit->setEnabled(false);

    manualSearchRadioButton = new QRadioButton(tr("manually"),this);
    connect(manualSearchRadioButton,SIGNAL(clicked()),SLOT(handleManualSearchRadioButton()));

    cdSearchRadioButton = new QRadioButton(tr("by CD in CD-ROM"),this);
    connect(cdSearchRadioButton,SIGNAL(clicked()),SLOT(handleManualSearchRadioButton()));
    filesSearchRadioButton = new QRadioButton(tr("by selected files"),this);
    connect(filesSearchRadioButton,SIGNAL(clicked()),SLOT(handleManualSearchRadioButton()));
    filesSearchRadioButton->setChecked(true);


    QPushButton *swapButton = new QPushButton(QIcon::fromTheme("flip"),QString(),this);
    swapButton->setFlat(true);
    connect(swapButton,SIGNAL(clicked()),SLOT(swapArtistAndAlbum()));

    startSearchButton = new QPushButton(tr("Search"),this);
    connect(startSearchButton,SIGNAL(clicked()),SLOT(startSearch()));

    searchResultsList = new QTreeWidget(this);
    searchResultsList->setAlternatingRowColors(true);
    searchResultsList->setColumnCount(1);
    searchResultsList->setRootIsDecorated(false);
#ifdef OSX_SUPPORT_ENABLED
    searchResultsList->setHeaderLabels(QStringList()<<tr("Search results"));
    searchResultsList->setAlternatingRowColors(true);
    searchResultsList->header()->setDefaultAlignment(Qt::AlignCenter);
#else
    searchResultsList->header()->hide();
#endif
    searchResultsList->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(searchResultsList,SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(handleAlbumSelection(QTreeWidgetItem*)));
    connect(searchResultsList,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(downloadRelease(QTreeWidgetItem*)));
    searchResultsList->setItemDelegate(new SearchResultsListDelegate(this));

    releaseInfoWidget = new ReleaseInfoWidget(this);


    progress = new QProgressIndicatorSpinning(this);
    progress->animate(false);


    search = new CoreNetworkSearch(this);
    connect(search,SIGNAL(error(QString)),networkErrorInfo,SLOT(setText(QString)));
    search->setUserAgent(QString("Qoobar/%1+http://qoobar.sourceforge.net").arg(QOOBAR_VERSION));


    QHBoxLayout *dLayout = new QHBoxLayout;
    dLayout->addWidget(new QLabel(tr("artist:"),this));
    dLayout->addWidget(artistEdit);
    dLayout->addWidget(swapButton);
    dLayout->addWidget(new QLabel(tr("album:"),this));
    dLayout->addWidget(albumEdit);

    QHBoxLayout *frdbgb = new QHBoxLayout;
    frdbgb->addWidget(new QLabel(tr("Search in"),this));
    frdbgb->addWidget(sourceComboBox);
    frdbgb->addWidget(filesSearchRadioButton);
    frdbgb->addWidget(cdSearchRadioButton);
    frdbgb->addWidget(manualSearchRadioButton);
    frdbgb->addWidget(startSearchButton);
    frdbgb->addStretch(1);

    QHBoxLayout *nl = new QHBoxLayout;
    nl->addWidget(progress);
    nl->addWidget(networkStatusInfo);
    nl->addSpacing(50);
    nl->addWidget(networkErrorInfo);
    nl->addStretch();

    QSplitter *splitter = new QSplitter(Qt::Horizontal,this);
    splitter->addWidget(searchResultsList);
    splitter->addWidget(releaseInfoWidget);

    QVBoxLayout *nLayout = new QVBoxLayout;
    nLayout->addLayout(frdbgb);
    nLayout->addLayout(dLayout);
    nLayout->addWidget(splitter);
    nLayout->addLayout(nl);
    nLayout->setStretch(2,1);

    QWidget *dw = new QWidget(this);
    dw->setLayout(nLayout);
    tab->addTab(dw,tr("from network"));

    setTabOrder(sourceComboBox,filesSearchRadioButton);
    setTabOrder(filesSearchRadioButton,cdSearchRadioButton);
    setTabOrder(cdSearchRadioButton,manualSearchRadioButton);
    setTabOrder(manualSearchRadioButton,artistEdit);
    setTabOrder(artistEdit,albumEdit);
    setTabOrder(albumEdit,startSearchButton);
    setTabOrder(startSearchButton,releaseInfoWidget);

    if (App->downloadPlugins.isEmpty()) {
        networkErrorInfo->setText(tr("No download plugins found. Internet search is inaccessible"));
        artistEdit->setEnabled(false);
        albumEdit->setEnabled(false);
        swapButton->setEnabled(false);
        manualSearchRadioButton->setEnabled(false);
        cdSearchRadioButton->setEnabled(false);
        filesSearchRadioButton->setEnabled(false);
        startSearchButton->setEnabled(false);
    }
    else {
        sourceComboBox->setCurrentIndex(0);
        handleSourceComboBox(sourceComboBox->currentIndex());
    }

    //******************************************************************************************
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    setTabOrder(releaseInfoWidget,buttonBox);

#ifdef OSX_SUPPORT_ENABLED
    QButton *helpButton = new QButton(this, QButton::HelpButton);
#else
    QPushButton *helpButton = buttonBox->addButton(QDialogButtonBox::Help);
#endif
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));

    auto *layout = new QVBoxLayout;
    layout->addWidget(tab);
#ifdef OSX_SUPPORT_ENABLED
    QHBoxLayout *boxL = new QHBoxLayout;
    boxL->addWidget(helpButton);
    boxL->addWidget(buttonBox);
    layout->addLayout(boxL);
#else
    layout->addWidget(buttonBox);
#endif
    setLayout(layout);
    resize(App->primaryScreen()->availableSize()/2);

    currentAlbum=-1;
}

TagsFillDialog::~TagsFillDialog()
{DD

}

void TagsFillDialog::swapArtistAndAlbum()
{DD
    QString s = artistEdit->text();
    artistEdit->setText(albumEdit->text());
    albumEdit->setText(s);
}

void TagsFillDialog::startSearch()
{DD
    QString path=sourceComboBox->itemData(sourceComboBox->currentIndex()).toString();
    IDownloadPlugin *plugin = maybeLoadPlugin(path);

    if (!plugin) return;

    networkErrorInfo->clear();
    networkStatusInfo->clear();
    searchResults.clear();
    searchResultsList->clear();
    currentAlbum=-1;

    bool fromFiles = filesSearchRadioButton->isChecked();
    bool manually = manualSearchRadioButton->isChecked();

    QString artist = artistEdit->text();
    QString album = albumEdit->text();
    if (manually && artist.isEmpty() && album.isEmpty()) {
        networkErrorInfo->setText(tr("Please specify an artist and an album for the manual search"));
        return;
    }

    progress->animate(true);
    networkStatusInfo->setText(tr("Searching %1...").arg(sourceComboBox->currentText()));

    Request query;
    if (manually) {
        query = plugin->queryForManualSearch(QStringList()<<artist<<album);
    }
    else if (fromFiles) {//search by files;
        QVector<int> filesLengths;
        Q_FOREACH (const Tag &tag,oldTags) filesLengths << tag.length();
        query = plugin->queryForSearchByFiles(filesLengths);
    }
    else {//try to search by CD
        query = plugin->queryForCD();
    }
    if (!query.isEmpty()) {
        QByteArray response = search->get(query);
        QList<SearchResult> releases = plugin->parseResponse(response);

        found(releases);
    }
    progress->animate(false);
    networkErrorInfo->setText(plugin->errorString());
}

void TagsFillDialog::found(const QList<SearchResult> &releases)
{DD
    QString path=sourceComboBox->itemData(sourceComboBox->currentIndex()).toString();
    IDownloadPlugin *plugin = maybeLoadPlugin(path);

    if (releases.isEmpty()) {
        networkStatusInfo->setText(tr("Nothing found"));
        return;
    }
    searchResultsList->clear();
    searchResults = releases;
    networkStatusInfo->setText(tr("Found %n album(s)","",releases.size()));

    if (!plugin) return;
    for (int i=0; i<releases.size(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setData(0,Qt::DisplayRole, plugin->releaseToList(releases.at(i)));
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        searchResultsList->addTopLevelItem(item);
    }
}

void TagsFillDialog::downloadRelease(QTreeWidgetItem *item)
{DD
    int row = -1;
    if (item->parent()==0) //top-level item, release
        row = searchResultsList->indexOfTopLevelItem(item);
    else // a CD in a release
        row = searchResultsList->indexOfTopLevelItem(item->parent());
    if (row<0) return;

    currentAlbum = row;
    downloadRelease(searchResults.at(row).fields.value("url"),row);
}

void TagsFillDialog::downloadRelease(const QString &url, const int releaseIndex)
{DD
    QString path=sourceComboBox->itemData(sourceComboBox->currentIndex()).toString();
    IDownloadPlugin *plugin = maybeLoadPlugin(path);
    if (!plugin) return;

    progress->animate(true);

    if (plugin->needsPause()) {
        QEventLoop loop;
        QTimer::singleShot(plugin->preferredPauseSize(), &loop, SLOT(quit()));
        loop.exec();
    }
    networkStatusInfo->setText(tr("Retrieving release info..."));
    networkErrorInfo->clear();

    QByteArray response = search->get(plugin->queryForRelease(url));
    SearchResult release = plugin->parseRelease(response);
    QString imageUrl = release.image.description();
    if (!imageUrl.isEmpty()) {
        QByteArray data = search->get(plugin->queryForPicture(imageUrl));
        QPixmap img;
        if (img.loadFromData(data))
            release.image.setPixmap(data);
    }
    resultFinished(release, releaseIndex);
    networkStatusInfo->setText(tr("Done"));
    networkErrorInfo->setText(plugin->errorString());
    progress->animate(false);
}

IDownloadPlugin *TagsFillDialog::maybeLoadPlugin(const QString &path)
{DD
    IDownloadPlugin *plugin = loadedPlugins.value(path, 0);
    if (!plugin) {
        QPluginLoader loader(path);
        QObject *o = loader.instance();
        if (o) plugin = qobject_cast<IDownloadPlugin *>(o);
        if (plugin) loadedPlugins.insert(path, plugin);
    }
    return plugin;
}

void TagsFillDialog::resultFinished(const SearchResult &r, int n)
{DD
    if (n<0 || n>=searchResults.size()) return;

    const QString url = searchResults.at(n).fields.value("url");
    searchResults.replace(n,r);
    searchResults[n].fields.insert("url", url);
    releaseInfoWidget->setSearchResult(searchResults[n]);

    const int cdCount = r.cdCount;
    if (cdCount>1) {
        for (int i=1; i<cdCount+1; ++i) {
            QTreeWidgetItem *it = new QTreeWidgetItem(searchResultsList->topLevelItem(n),
                                                      QStringList(QString("CD %1").arg(i)));
            it->setData(0,Qt::UserRole+1,i);
        }
        searchResultsList->topLevelItem(n)->setExpanded(true);
    }
}

void TagsFillDialog::handleAlbumSelection(QTreeWidgetItem *item)
{DD
    int row = -1;
    int cd = 1;
    if (item->parent()==0) //top-level item, release
        row = searchResultsList->indexOfTopLevelItem(item);
    else {// a CD in a release
        row = searchResultsList->indexOfTopLevelItem(item->parent());
        cd = item->data(0,Qt::UserRole+1).toInt();
    }
    if (row<0) return;

    currentAlbum = row;
    if (!searchResults.isEmpty()) {
        if (!searchResults.at(row).loaded)
            downloadRelease(searchResults.at(row).fields.value(QSL("url")),row);
        else
            releaseInfoWidget->setSearchResult(searchResults[row],cd);
    }
}

void TagsFillDialog::handleSourceComboBox(int row)
{DD
    if (row<0 || row>=App->downloadPlugins.size()) return;
    QJsonObject meta = App->downloadPlugins.at(row);
    bool canSearchByFiles=meta.value(QSL("canSearchByFiles")).toBool(false);
    bool canSearchByCD=meta.value(QSL("canSearchByCD")).toBool(false);
    bool canSearchManually=meta.value(QSL("canSearchManually")).toBool(false);

    filesSearchRadioButton->setEnabled(canSearchByFiles);
    cdSearchRadioButton->setEnabled(canSearchByCD);
    manualSearchRadioButton->setEnabled(canSearchManually);

    if (filesSearchRadioButton->isEnabled()) filesSearchRadioButton->setChecked(true);
    else if (cdSearchRadioButton->isEnabled()) cdSearchRadioButton->setChecked(true);
    else  manualSearchRadioButton->setChecked(true);
    handleManualSearchRadioButton();
}

void TagsFillDialog::handleManualSearchRadioButton()
{DD
    artistEdit->setEnabled(manualSearchRadioButton->isChecked());
    albumEdit->setEnabled(manualSearchRadioButton->isChecked());
    if (manualSearchRadioButton->isChecked()) artistEdit->setFocus();
}

void TagsFillDialog::setNewTag(const QString &tagValue, Tag &tag, const QString &field, int fieldID)
{DD
    if (!tagValue.isEmpty() && releaseInfoWidget->use(field)) tag.setTag(fieldID, tagValue);
}

void TagsFillDialog::updateTagsFromNetwork()
{DD

    newTags=oldTags;
    if (searchResults.isEmpty() || currentAlbum==-1) return;

    SearchResult &res = searchResults[currentAlbum];

    int c = 0;
    QString prefix;
    QList<Track> cdTracks;
    if (res.cdCount <= 1) cdTracks = res.tracks;
    else {
        for (int i=0; i<res.tracks.size(); ++i) {
            if (res.tracks[i].cd == releaseInfoWidget->cd()) cdTracks << res.tracks[i];
        }
    }
    for (int i=0; i<cdTracks.size(); ++i) {
        if (!releaseInfoWidget->useTrack(i)) continue;

        const QString no = cdTracks[i].fields.value(QSL("tracknumber"));
        QString title = cdTracks[i].fields.value(QSL("title"));

        //there is three possible cases:
        //1. track title is not empty and not equals '-', track number is empty
        if (!title.isEmpty() && title != "-" && no.isEmpty())
            prefix = title;

        //2. track title is not empty and not equals '-', track number is not empty
        else if (!title.isEmpty() && title != "-" && !no.isEmpty()) {
            if (c>=newTags.size()) break;

            QList<Artist> artists = cdTracks[i].artists + res.artists;

            QStringList trackComposers = excludeByRole(artists, QStringList()<<QSL("compose") << QSL("music by"));
            QStringList trackOrchestras = excludeByRole(artists,QStringList(QSL("orchestra")));
            QStringList trackConductors = excludeByRole(artists,QStringList(QSL("conduct")));
            QStringList tracklyricists = excludeByRole(artists,QStringList()<<QSL("lyrics by") << QSL("words by")<< QSL("lyricist"));

            QString artist = artistsText(artists);

            Tag &tag = newTags[c];

            setNewTag(res.fields.value(QSL("album")), tag, QSL("album"), ALBUM);
            setNewTag(res.fields.value(QSL("genre")), tag, QSL("genre"), GENRE);
            setNewTag(res.fields.value(QSL("year")), tag, QSL("year"), YEAR);
            setNewTag(res.fields.value(QSL("label")), tag, QSL("label"), PUBLISHER);
            setNewTag(res.fields.value(QSL("extraData")), tag, QSL("extraData"), COMMENT);
            setNewTag(trackComposers.join(QSL("; ")), tag, QSL("artist"), COMPOSER);
            setNewTag(trackOrchestras.join(QSL("; ")), tag, QSL("artist"), ORCHESTRA);
            setNewTag(trackConductors.join(QSL("; ")), tag, QSL("artist"), CONDUCTOR);
            setNewTag(tracklyricists.join(QSL("; ")), tag, QSL("artist"), LYRICIST);
            setNewTag(artist, tag, QSL("artist"), ARTIST);

            if (!res.image.pixmap().isNull() && releaseInfoWidget->use("image")) tag.setImage(res.image);
            if (releaseInfoWidget->use("titles")) tag.setTag(TRACKNUMBER,no);

            if (!prefix.isEmpty())
                title = QString("%1. %2").arg(prefix).arg(title);
            setNewTag(title, tag, QSL("titles"), TITLE);
            c++;
        }
        //3. track title is empty or equals '-'
        else if (title.isEmpty() || title=="-")
            prefix.clear();
    }
}


/* Filling from tags */

void TagsFillDialog::insertLegend(const QString &s)
{DD
    patternEdit->lineEdit()->insert(s);
    patternEdit->setFocus();
}

void TagsFillDialog::setSource(int sourceId)
{DD
    if (sourceId < 0) return;
    importModel->setSource(sourceId);
}

void TagsFillDialog::updateTags()
{DD;
    importModel->setPattern(patternEdit->currentText());
}

void TagsFillDialog::accept()
{DD
    if (tab->currentIndex()==0) {
        //add pattern to file renaming patterns
        App->addPattern(patternEdit->currentText(), App->fillPatterns);
        newTags = importModel->getTags();
    }
    if (tab->currentIndex()==1) updateTagsFromNetwork();

    QDialog::accept();
}

void TagsFillDialog::showHelp()
{DD;
    Qoobar::showHelp(tab->currentIndex()==0?QSL("filling"):QSL("importing"));
}


void TagsFillDialog::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return || event->key()==Qt::Key_Enter)
            && tab->currentIndex()==1)
        startSearchButton->click();
    else QWidget::keyPressEvent(event);
}


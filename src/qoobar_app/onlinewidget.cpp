#include "onlinewidget.h"

#include <QtWidgets>

#include "application.h"
#include "clearlineedit.h"
#include "releaseinfowidget.h"
#include "qprogressindicatorspinning.h"
#include "corenetworksearch.h"
#include "logging.h"
#include "idownloadplugin.h"

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

OnlineWidget::OnlineWidget(const QList<Tag> &oldTags, QWidget *parent)
    : QWidget(parent), oldTags(oldTags)
{DD;
    networkStatusInfo = new QLabel(this);
    networkErrorInfo = new QLabel(this);
    networkErrorInfo->setWordWrap(true);

    sourceComboBox = new QComboBox(this);
    sourceComboBox->setEditable(false);

    for (const auto &metaData: qAsConst(App->downloadPlugins)) {
        QString text = metaData.value(QSL("text")).toObject().value(App->langID).toString();
        if (text.isEmpty())
            text = metaData.value(QSL("text")).toObject().value(QSL("default")).toString();
        sourceComboBox->addItem(text, metaData.value(QSL("path")));
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
    search->setUserAgent(QString("Qoobar/%1+https://qoobar.sourceforge.io").arg(QOOBAR_VERSION));

    saveResultsAct = new QAction(tr("Export search results"), this);
    saveResultsAct->setIcon(QIcon::fromTheme("document-save"));
    saveResultsAct->setEnabled(false);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->addAction(saveResultsAct);
    toolBar->setIconSize(QSize(16,16));


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
    QVBoxLayout *leftSplitterL = new QVBoxLayout;
    leftSplitterL->addWidget(searchResultsList);
    leftSplitterL->setContentsMargins(0,0,10,0);
    leftSplitterL->setMenuBar(toolBar);
    auto *leftSplitterW = new QWidget(this);
    leftSplitterW->setLayout(leftSplitterL);
    splitter->addWidget(leftSplitterW);
    splitter->addWidget(releaseInfoWidget);

    QVBoxLayout *nLayout = new QVBoxLayout;
    nLayout->addLayout(frdbgb);
    nLayout->addLayout(dLayout);
    nLayout->addWidget(splitter);
    nLayout->addLayout(nl);
    nLayout->setStretch(2,1);

    setTabOrder(sourceComboBox,filesSearchRadioButton);
    setTabOrder(filesSearchRadioButton,cdSearchRadioButton);
    setTabOrder(cdSearchRadioButton,manualSearchRadioButton);
    setTabOrder(manualSearchRadioButton,artistEdit);
    setTabOrder(artistEdit,albumEdit);
    setTabOrder(albumEdit,startSearchButton);
    setTabOrder(startSearchButton,releaseInfoWidget);

    setLayout(nLayout);

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
        sourceComboBox->setCurrentIndex(App->lastSearchServer);
    }

    currentAlbum = -1;
}

QList<Tag> OnlineWidget::getTags()
{DD;
    QList<Tag> newTags = oldTags;

    if (searchResults.isEmpty() || currentAlbum==-1)
        return newTags;

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

    return newTags;
}

void OnlineWidget::swapArtistAndAlbum()
{DD;
    QString s = artistEdit->text();
    artistEdit->setText(albumEdit->text());
    albumEdit->setText(s);
}

void OnlineWidget::startSearch()
{DD;
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
        networkErrorInfo->setText(tr("Please specify an album and/or an artist for the manual search"));
        return;
    }

    progress->animate(true);
    networkStatusInfo->setText(tr("Searching %1...").arg(sourceComboBox->currentText()));

    Request query;
    if (manually) {
        query = plugin->queryForManualSearch({artist, album});
    }
    else if (fromFiles) {//search by files;
        QVector<int> filesLengths;
        for (const Tag &tag: qAsConst(oldTags)) filesLengths << tag.length();
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

void OnlineWidget::found(const QList<SearchResult> &releases)
{DD;
    QString path=sourceComboBox->itemData(sourceComboBox->currentIndex()).toString();
    IDownloadPlugin *plugin = maybeLoadPlugin(path);

    if (releases.isEmpty()) {
        networkStatusInfo->setText(tr("Nothing found"));
        return;
    }
    saveResultsAct->setEnabled(true);
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

void OnlineWidget::downloadRelease(QTreeWidgetItem *item)
{DD;
    int row = -1;
    if (item->parent()==0) //top-level item, release
        row = searchResultsList->indexOfTopLevelItem(item);
    else // a CD in a release
        row = searchResultsList->indexOfTopLevelItem(item->parent());
    if (row<0) return;

    currentAlbum = row;
    downloadRelease(searchResults.at(row).fields.value("url"),row);
}

void OnlineWidget::saveResults()
{

}

void OnlineWidget::downloadRelease(const QString &url, const int releaseIndex)
{DD;
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

IDownloadPlugin *OnlineWidget::maybeLoadPlugin(const QString &path)
{DD;
    IDownloadPlugin *plugin = loadedPlugins.value(path, 0);
    if (!plugin) {
        QPluginLoader loader(path);
        QObject *o = loader.instance();
        if (o) plugin = qobject_cast<IDownloadPlugin *>(o);
        if (plugin) loadedPlugins.insert(path, plugin);
    }
    return plugin;
}

void OnlineWidget::resultFinished(const SearchResult &r, int n)
{DD;
    if (n<0 || n>=searchResults.size()) return;

    const QString url = searchResults.at(n).fields.value("url");
    searchResults.replace(n,r);
    searchResults[n].fields.insert("url", url);
    releaseInfoWidget->setSearchResult(searchResults[n]);

    const int cdCount = r.cdCount;
    if (cdCount>1) {
        for (int i=1; i<=cdCount; ++i) {
            QTreeWidgetItem *it = new QTreeWidgetItem(searchResultsList->topLevelItem(n),
                                                      QStringList(QString("CD %1").arg(i)));
            it->setData(0,Qt::UserRole+1,i);
        }
        searchResultsList->topLevelItem(n)->setExpanded(true);
    }
}

void OnlineWidget::handleAlbumSelection(QTreeWidgetItem *item)
{DD;
    int row = -1;
    int cd = 1;
    if (item->parent()==nullptr) //top-level item, release
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

void OnlineWidget::handleSourceComboBox(int row)
{DD;
    if (row<0 || row>=App->downloadPlugins.size()) return;
    App->lastSearchServer = row;

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

void OnlineWidget::handleManualSearchRadioButton()
{DD
    artistEdit->setEnabled(manualSearchRadioButton->isChecked());
    albumEdit->setEnabled(manualSearchRadioButton->isChecked());
    if (manualSearchRadioButton->isChecked()) artistEdit->setFocus();
}

void OnlineWidget::setNewTag(const QString &tagValue, Tag &tag, const QString &field, int fieldID)
{DD
    if (!tagValue.isEmpty() && releaseInfoWidget->use(field)) tag.setTag(fieldID, tagValue);
}

#include "onlinewidget.h"

#include <QtWidgets>

#include "application.h"
#include "clearlineedit.h"
#include "releaseinfowidget.h"
#include "qprogressindicatorspinning.h"
#include "corenetworksearch.h"
#include "logging.h"
#include "idownloadplugin.h"
#include "onlinemodel.h"

class SearchResultsListDelegate : public QItemDelegate
{
public:
    explicit SearchResultsListDelegate(QObject *parent) : QItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (!painter) return;

        painter->save();

        if (index.isValid()) {
            if (!index.parent().isValid() /*|| index.row()!=1*/) {
                QStringList albumInfo = index.data(Qt::DisplayRole).toStringList();
                if (albumInfo.isEmpty()) {
                    QItemDelegate::paint(painter,option,index);
                    painter->restore();
                    return;
                }

                drawBackground(painter, option, index);
                drawFocus(painter, option, option.rect);

                QRect albumRect = option.rect;
                albumRect.setLeft(albumRect.left()+25);
                albumRect.setWidth(option.rect.width()-25);
                int num = albumInfo.size();
                int h = albumRect.height()/num;
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
                auto icon = index.data(Qt::DecorationRole);
                if (icon.isValid()) {
                    QRect iconRect = option.rect;
                    iconRect.setWidth(20);
                    iconRect.setHeight(20);
                    iconRect.translate(10,10);
                    drawDecoration(painter, option, iconRect, icon.value<QIcon>().pixmap(16,16));
                }
            }
            else {
                QItemDelegate::paint(painter,option,index);
            }
        }
        else
            QItemDelegate::paint(painter,option,index);

        painter->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (!index.isValid()) return option.rect.size();
        QStringList albumInfo = index.data(Qt::DisplayRole).toStringList();
        return QSize(option.rect.width()+20,option.fontMetrics.height()*albumInfo.size()+5);
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

    model = new OnlineModel(this);

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
    connect(manualSearchRadioButton,SIGNAL(clicked()),SLOT(updateManualSearchEdits()));
    cdSearchRadioButton = new QRadioButton(tr("by CD in CD-ROM"),this);
    connect(cdSearchRadioButton,SIGNAL(clicked()),SLOT(updateManualSearchEdits()));
    filesSearchRadioButton = new QRadioButton(tr("by selected files"),this);
    connect(filesSearchRadioButton,SIGNAL(clicked()),SLOT(updateManualSearchEdits()));
    filesSearchRadioButton->setChecked(true);

    QPushButton *swapButton = new QPushButton(QIcon::fromTheme("flip"),QString(),this);
    swapButton->setFlat(true);
    connect(swapButton,SIGNAL(clicked()),SLOT(swapArtistAndAlbum()));

    startSearchButton = new QPushButton(tr("Search"),this);
    connect(startSearchButton,SIGNAL(clicked()),SLOT(startSearch()));

    tree = new QTreeView(this);
    tree->setModel(model);
    tree->setAlternatingRowColors(true);
    tree->setAllColumnsShowFocus(true);
    tree->setRootIsDecorated(true);
    tree->setItemsExpandable(true);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
#ifndef OSX_SUPPORT_ENABLED
    tree->header()->hide();
#endif
    tree->setSelectionMode(QAbstractItemView::SingleSelection);



    connect(tree,SIGNAL(clicked(QModelIndex)), this, SLOT(onTreeItemClicked(QModelIndex)));
    connect(tree->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onItemsSelectionChanged(QItemSelection,QItemSelection)));
    tree->setItemDelegate(new SearchResultsListDelegate(this));

    releaseInfoWidget = new ReleaseInfoWidget(this);


    progress = new QProgressIndicatorSpinning(this);
    progress->animate(false);

    search = new CoreNetworkSearch(this);
    connect(search,SIGNAL(error(QString)),networkErrorInfo,SLOT(setText(QString)));
    search->setUserAgent(QString("Qoobar/%1+https://qoobar.sourceforge.io").arg(QOOBAR_VERSION));

    saveResultsAct = new QAction(tr("Cache this release"), this);
    saveResultsAct->setIcon(QIcon::fromTheme("document-save"));
    saveResultsAct->setEnabled(false);
    connect(saveResultsAct, SIGNAL(triggered()), this, SLOT(cacheResult()));

    clearCacheAct = new QAction(tr("Delete cached releases"), this);
    clearCacheAct->setIcon(QIcon::fromTheme("edit-delete"));
    connect(model, SIGNAL(cacheEmpty(bool)), clearCacheAct, SLOT(setDisabled(bool)));
    connect(clearCacheAct, SIGNAL(triggered()), this, SLOT(clearCache()));
    model->loadCachedReleases();

    downloadReleaseAct = new QAction(tr("Download this release"), this);
    downloadReleaseAct->setIcon(QIcon::fromTheme("edit-download"));
    connect(downloadReleaseAct, SIGNAL(triggered()), this, SLOT(downloadRelease()));

    tree->addAction(saveResultsAct);
    tree->addAction(downloadReleaseAct);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->addAction(saveResultsAct);
    toolBar->addAction(clearCacheAct);
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
    leftSplitterL->addWidget(tree);
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
}

void OnlineWidget::swapArtistAndAlbum()
{DD;
    QString s = artistEdit->text();
    artistEdit->setText(albumEdit->text());
    albumEdit->setText(s);
}

void OnlineWidget::onTreeItemClicked(const QModelIndex &idx, bool force)
{
    if (idx.isValid()) {
        int row = -1;
        int cd = -1;
        if (!idx.parent().isValid()) //top-level item, release
            row = idx.row();
        else {// a CD in a release
            row = idx.parent().row();
            cd = idx.row()+1;
        }
        if (row<0) return;

        if (model->item(row)==SearchResult::emptyResult())
            return;

        if (!model->item(row).loaded || force)
            downloadRelease(model->item(row).fields.value(QSL("url")),row);
        else
            releaseInfoWidget->setSearchResult(model->item(row), cd);
        model->select(row, cd);
    }
    else model->select(-1, -1);
}

QList<int> lengths(const QList<Tag> &tags)
{
    QList<int> result;
    for (const auto &t: tags) result << t.length();
    return result;
}

void OnlineWidget::startSearch()
{DD;
    QString path = sourceComboBox->itemData(sourceComboBox->currentIndex()).toString();
    IDownloadPlugin *plugin = maybeLoadPlugin(path);

    if (!plugin) {
        networkErrorInfo->setText(tr("Could not load plugin %1").arg(path));
        return;
    }

    networkErrorInfo->clear();
    networkStatusInfo->clear();

    const SearchType searchType = manualSearchRadioButton->isChecked() ? SearchManually :
                 (filesSearchRadioButton->isChecked() ? SearchByFiles : SearchByCD);
    const QString artist = artistEdit->text();
    const QString album = albumEdit->text();

    if (searchType == SearchManually && artist.isEmpty() && album.isEmpty()) {
        networkErrorInfo->setText(tr("Please specify an album and/or an artist for the manual search"));
        return;
    }

    QProgressIndicatorSpinningHandle progressHandle(progress);
    networkStatusInfo->setText(tr("Searching %1...").arg(sourceComboBox->currentText()));

    Request query;
    switch (searchType) {
        case SearchManually: query = plugin->queryForManualSearch({artist, album});
            break;
        case SearchByFiles: {
            QList<int> filesLengths = lengths(oldTags);
            QJsonArray array;
            for (int i: qAsConst(filesLengths)) {
                array.append(i);
            }
            query = plugin->queryForSearchByFiles(filesLengths);
            break;
        }
        case SearchByCD: query = plugin->queryForCD();
            break;
    }

    if (!query.isEmpty()) {
        QByteArray response = search->get(query);
        qDebug()<<response;
        QList<SearchResult> releases = plugin->parseResponse(response);

        model->setFoundReleases(releases);
        if (releases.isEmpty()) {
            networkStatusInfo->setText(tr("Nothing found"));
        }
        else
            networkStatusInfo->setText(tr("Found %n album(s)", "", releases.size()));
    }
    networkErrorInfo->setText(plugin->errorString());
}

void OnlineWidget::onItemsSelectionChanged(const QItemSelection& selected, const QItemSelection&)
{DD;
    if (selected.isEmpty()) saveResultsAct->setDisabled(true);
    else {
        auto r = model->item(selected.indexes().first());
        saveResultsAct->setEnabled(r.loaded && !r.cached);
    }
}

void OnlineWidget::downloadRelease(const QString &url, const int releaseIndex)
{DD;
    QString path=sourceComboBox->itemData(sourceComboBox->currentIndex()).toString();
    IDownloadPlugin *plugin = maybeLoadPlugin(path);
    if (!plugin) return;

    QProgressIndicatorSpinningHandle handle(progress);

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

    networkStatusInfo->setText(tr("Done"));
    networkErrorInfo->setText(plugin->errorString());

    resultFinished(release, releaseIndex);
}

void OnlineWidget::resultFinished(const SearchResult &r, int n)
{DD;
    model->setResult(r, n);
    releaseInfoWidget->setSearchResult(model->item(n));
    tree->setExpanded(model->index(n,0,QModelIndex()), true);
    tree->selectionModel()->select(model->index(n,0,QModelIndex()), QItemSelectionModel::ClearAndSelect);
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

void OnlineWidget::cacheResult()
{
    if (!tree->selectionModel()->hasSelection()) return;
    auto r = model->item(tree->selectionModel()->selectedIndexes().first());

    if (r.cached) return;

    model->cacheResult(r);
}

void OnlineWidget::clearCache()
{
    QDir dir(ApplicationPaths::cachePath());
    if (!dir.exists()) return;
    auto list = dir.entryInfoList({"*.json"});
    if (list.isEmpty()) return;

    const bool cached = model->currentResult().cached;

    if (QMessageBox::question(this, tr("Qoobar - Clearing download cache"),
                              tr("Do you want to delete all of %n cached releases?", "", list.size()))==QMessageBox::Yes) {
        for (auto &fi: list) QFile::remove(fi.canonicalFilePath());
    }
    clearCacheAct->setDisabled(dir.isEmpty());
    model->clearCache();
    //clear releaseinfowidget if there was a cached release displayed there
    if (cached)
        releaseInfoWidget->clear();
}

void OnlineWidget::downloadRelease()
{
    auto selection = tree->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    onTreeItemClicked(selection.first(), true);
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
    updateManualSearchEdits();
}

void OnlineWidget::updateManualSearchEdits()
{DD;
    artistEdit->setEnabled(manualSearchRadioButton->isChecked());
    albumEdit->setEnabled(manualSearchRadioButton->isChecked());
    if (manualSearchRadioButton->isChecked()) artistEdit->setFocus();
}

QList<Tag> OnlineWidget::getTags()
{
    QList<Tag> newTags = oldTags;

    if (model->isEmpty())
        return newTags;

    SearchResult res = model->currentResult();

    int c = 0;
    QString prefix;
    QList<Track> cdTracks = res.tracks;

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
            setNewTag(QString::number(cdTracks[i].cd), tag, DISCNUMBER);
            setNewTag(QString::number(res.cdCount), tag, TOTALDISCS);

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


void OnlineWidget::setNewTag(const QString &tagValue, Tag &tag, const QString &field, int fieldID)
{DD;
    if (!tagValue.isEmpty() && releaseInfoWidget->use(field)) tag.setTag(fieldID, tagValue);
}

void OnlineWidget::setNewTag(const QString &tagValue, Tag &tag, int fieldID)
{DD;
    if (!tagValue.isEmpty()) tag.setTag(fieldID, tagValue);
}

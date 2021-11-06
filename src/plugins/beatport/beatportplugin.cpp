#include "beatportplugin.h"

#include <QtWidgets>
#include <QJsonDocument>
#include <QApplication>

#include "corenetworksearch.h"
#include "headerview.h"
#include "enums.h"
#include "tagparser.h"
#include "downloadhelper.h"

QList<Tag> BeatportPlugin::getNewTags(const QList<Tag> &oldTags)
{
    Dialog dialog(oldTags);
    if (dialog.exec()) {
        return dialog.getNewTags();
    }
    return oldTags;
}

CheckableHeaderView::CheckableHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)

{
    setSectionsClickable(true);
    connect(this,SIGNAL(sectionCountChanged(int,int)),SLOT(updateSectionCount(int,int)));
    if (auto parentView=qobject_cast<QAbstractItemView*>(parent)) {
        parentModel = parentView->model();
        connect(parentModel, &QAbstractItemModel::dataChanged, this, &CheckableHeaderView::modelDataChanged);

    }

}

void CheckableHeaderView::setCheckState(int section, Qt::CheckState checkState)
{
    if (isEnabled() && m_isChecked.at(section) != checkState) {
        m_isChecked[section] = checkState;
        updateSection(section);
    }
}

void CheckableHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (m_isCheckable.at(logicalIndex)) {
        //first draw checkbox on a pixmap with pixel ratio set to the actual one
        //to respect high dpi screens
        QCheckBox cb;

        QPixmap img(rect.size() * this->devicePixelRatioF());
        img.setDevicePixelRatio(this->devicePixelRatioF());
        img.fill(Qt::transparent);

        QStylePainter p(&img, &cb);

        QStyleOptionButton option;
        option.initFrom(&cb);
        if (isEnabled())
            option.state |= QStyle::State_Enabled;
        option.rect = rect;
        option.rect.translate(3,0);
        if (m_isChecked.at(logicalIndex)==Qt::Checked)
            option.state |= QStyle::State_On;
        else if (m_isChecked.at(logicalIndex)==Qt::Unchecked)
            option.state |= QStyle::State_Off;
        else
            option.state |= QStyle::State_NoChange;

        p.drawControl(QStyle::CE_CheckBox, option);
        //next draw this pixmap on the screen
        style()->drawItemPixmap(painter, rect, Qt::AlignLeft | Qt::AlignTop, img);
    }
}

void CheckableHeaderView::mousePressEvent(QMouseEvent *event)
{
    int logicalIndex = logicalIndexAt(event->pos());
    if (isEnabled() && m_isCheckable.at(logicalIndex)) {
        if (m_isChecked.at(logicalIndex)==Qt::Checked)
            m_isChecked[logicalIndex] = Qt::Unchecked;
        else
            m_isChecked[logicalIndex] = Qt::Checked;
        updateSection(logicalIndex);

        Q_EMIT toggled(logicalIndex, m_isChecked.at(logicalIndex));

//        if (parentModel) {
//            parentModel->blockSignals(true);
//            for (int i=0; i<parentModel->rowCount()-1; ++i) {
//                parentModel->setData(parentModel->index(i, logicalIndex), m_isChecked[logicalIndex], Qt::CheckStateRole);
//            }
//            parentModel->blockSignals(false);
//            parentModel->setData(parentModel->index(parentModel->rowCount()-1, logicalIndex),
//                                 m_isChecked[logicalIndex],
//                                 Qt::CheckStateRole);
//        }
    }
    else QHeaderView::mousePressEvent(event);
}

void CheckableHeaderView::setCheckable(int section, bool checkable)
{
    if (section<0 || section>=count()) return;
    m_isCheckable[section]=checkable;
}

void CheckableHeaderView::updateSectionCount(int oldCount, int newCount)
{
    Q_UNUSED(oldCount)
    m_isChecked.resize(newCount);
    m_isCheckable.resize(newCount);
}

void CheckableHeaderView::modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (!parentModel) return;

    if (roles.contains(Qt::CheckStateRole) || roles.isEmpty()) {
        for (int column = topLeft.column(); column <= bottomRight.column(); ++column) {
            if (!isSectionCheckable(column)) continue;
            int checkedCount=0;
            for (int i=0; i<parentModel->rowCount(topLeft.parent()); ++i) {
                if (parentModel->data(parentModel->index(i,column,topLeft.parent()), Qt::CheckStateRole).toInt()==Qt::Checked)
                    checkedCount++;
            }
            if (checkedCount==0)
                setCheckState(column, Qt::Unchecked);
            else if (checkedCount == parentModel->rowCount(topLeft.parent()))
                setCheckState(column, Qt::Checked);
            else
                setCheckState(column, Qt::PartiallyChecked);
        }
    }
}

bool CheckableHeaderView::isSectionCheckable(const int section) const
{
    return m_isCheckable.at(section);
}


/* Dialog */

bool BeatportTrack::isEmpty() const
{
    return (id==0 &&
            title.isEmpty() &&
            releaseDate.isEmpty() &&
            length.isEmpty() &&
            artists.isEmpty() &&
            remixers.isEmpty() &&
            genres.isEmpty() &&
            subgenres.isEmpty() &&
            release.isEmpty() &&
            label.isEmpty() &&
            image.isEmpty() &&
            trackNumber==0 &&
            isrc.isEmpty() &&
            bpm.isEmpty() &&
            key.isEmpty());
}

BeatportWidget::BeatportWidget(QWidget *parent)
    : QWidget(parent), emptyPixmap{QPixmap(100,100)}
{
    current = 0;

    emptyPixmap.setDevicePixelRatio(devicePixelRatioF());
    emptyPixmap.fill(Qt::gray);

    pictureLabel = new QLabel(this);
    pictureLabel->setPixmap(emptyPixmap);

    prevAction = new QAction(this);
    prevAction->setEnabled(false);
    connect(prevAction,SIGNAL(triggered()),SLOT(previous()));
    auto *prevButton = new QToolButton(this);
    prevButton->setDefaultAction(prevAction);
    prevButton->setArrowType(Qt::LeftArrow);
    prevButton->setAutoRaise(true);
    prevButton->setIconSize(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));

    nextAction = new QAction(this);
    nextAction->setEnabled(false);
    connect(nextAction,SIGNAL(triggered()),SLOT(next()));
    auto *nextButton = new QToolButton(this);
    nextButton->setDefaultAction(nextAction);
    nextButton->setArrowType(Qt::RightArrow);
    nextButton->setAutoRaise(true);
    nextButton->setIconSize(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));

    titleLabel = new QLabel(this);
    titleLabel->setMaximumWidth(280*devicePixelRatio());

    authorsLabel = new QLabel(this);
    authorsLabel->setMaximumWidth(280*devicePixelRatio());
    authorsLabel->setWordWrap(true);

    infoLabel = new QLabel(this);
    infoLabel->setMaximumWidth(280*devicePixelRatio());
    infoLabel->setWordWrap(true);

    auto *l = new QGridLayout;
    l->addWidget(prevButton,0,0,3,1,Qt::AlignCenter | Qt::AlignVCenter);
    l->addWidget(titleLabel,0,1,1,2);
    l->addWidget(pictureLabel,1,1,2,1);
    l->addWidget(authorsLabel,1,2);
    l->addWidget(infoLabel,2,2);
    l->addWidget(nextButton,0,3,3,1,Qt::AlignCenter | Qt::AlignVCenter);
    setLayout(l);
#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
    resize(int(400*devicePixelRatioF()), int(100*devicePixelRatioF()));
#else
    resize(400*devicePixelRatio(), 100*devicePixelRatio());
#endif
    update(BeatportTrack());
}

void BeatportWidget::next()
{
    if (current < tracks.size()-1) {
        current++;
        prevAction->setEnabled(current>0);
        update(tracks.at(current));
    }
    nextAction->setEnabled(current < tracks.size()-1);
}

void BeatportWidget::previous()
{
    if (current > 0) {
        current--;
        nextAction->setEnabled(current < tracks.size()-1);
        update(tracks.at(current));
    }
    prevAction->setEnabled(current>0);
}

void BeatportWidget::setTracks(const QList<BeatportTrack> &tracks)
{
    this->tracks = tracks;
    if (tracks.isEmpty()) {
        prevAction->setDisabled(true);
        nextAction->setDisabled(true);
        clear();
    }
    else {
        current=0;
        prevAction->setEnabled(false);
        nextAction->setEnabled(tracks.size()>1);
        update(tracks.at(0));
    }
}

void BeatportWidget::clear()
{
    pictureLabel->setPixmap(emptyPixmap);
    titleLabel->clear();
    authorsLabel->clear();
    infoLabel->clear();
}

void BeatportWidget::update(const BeatportTrack &track)
{
    clear();
    if (track.isEmpty())
        return;

    if (!track.image.isNull()) {
        QPixmap p;
        if (p.loadFromData(track.image))
            pictureLabel->setPixmap(p.scaled(100,100));
    }
    else pictureLabel->setPixmap(emptyPixmap);

    titleLabel->setText("<b>"+track.release+"</b>: "+track.title);
    QString authors=track.artists.join(", ");
    if (!track.remixers.isEmpty())
        authors.append(", <b>Remixer</b> "+track.remixers.join(", "));
    authorsLabel->setText(authors);

    QStringList info;
    if (!track.label.isEmpty()) info << track.label;
    if (!track.releaseDate.isEmpty()) info << track.releaseDate;
    if (!track.genres.isEmpty()) info << track.genres;
    if (!track.bpm.isEmpty()) info << (track.bpm+" bpm");
    if (!track.key.isEmpty()) info << track.key;
    infoLabel->setText(info.join(", "));
}

BeatportTrack BeatportWidget::currentTrack()
{
    if (tracks.isEmpty() || current<0 || current>=tracks.size())
        return BeatportTrack();
    return tracks.at(current);
}



Dialog::Dialog(const QList<Tag> &oldTags, QWidget *parent)
    : QDialog(parent), oldTags(oldTags)
{
    setWindowTitle(tr("Search in Beatport database"));
    search = new CoreNetworkSearch(this);
    search->setUserAgent("Qoobar/1.6.0+http://qoobar.sourceforge.net");
    searchButton = new QPushButton(tr("Search"),this);
    connect(searchButton,SIGNAL(clicked()),this,SLOT(startSearch()));

    table = new QTableWidget(0,4,this);
    table->setHorizontalHeaderLabels(QStringList()<<tr("Original track")
                                     <<tr("Authors / Remixers")
                                     <<tr("Track name")
                                     <<tr("Beatport variants"));


    for (int i=0; i<oldTags.size(); ++i) {
        table->insertRow(i);
        auto *item0 = new QTableWidgetItem();
        item0->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        item0->setCheckState(Qt::Checked);
        table->setItem(i,0,item0);

        auto *item1 = new QTableWidgetItem();
        item1->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        table->setItem(i,1,item1);

        auto *item2 = new QTableWidgetItem();
        item2->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        table->setItem(i,2,item2);

        auto *item3 = new QTableWidgetItem();
        item3->setFlags(Qt::ItemIsEnabled);
        table->setItem(i,3,item3);

        auto *w = new BeatportWidget(this);
        widgets << w;
        table->setCellWidget(i,3,w);
    }

    //filling in the table with tracks
    for (int i=0; i<oldTags.size(); ++i) {
        table->item(i,0)->setText(oldTags.at(i).fileNameExt());
        QString title,artists,remixer;
        parseFileName(title, artists, remixer, i);
        table->item(i,1)->setData(Qt::UserRole+1,remixer);
        table->item(i,1)->setText(artists);
        table->item(i,2)->setText(title);
    }

    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    header = new CheckableHeaderView(Qt::Horizontal,table);
    table->setHorizontalHeader(header);
    header->setCheckState(0,Qt::Checked);
    header->setCheckable(0,true);
    header->setCheckState(0,Qt::Checked);
//    table->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
    //table->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
    table->horizontalHeader()->resizeSection(3, 400);
    connect(header,SIGNAL(toggled(int,Qt::CheckState)),this,SLOT(headerToggled(int,Qt::CheckState)));

    progressBar = new QProgressBar(this);
    progressBar->setEnabled(false);
    progressBar->setTextVisible(false);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    filterCheckBox = new QCheckBox(tr("Filter tracks by length"),this);
    filterCheckBox->setChecked(true);
    //skipCheckBox = new QCheckBox(tr("Skip already filled tracks"),this);
    //skipCheckBox->setChecked(true);

    QGridLayout *l = new QGridLayout;
    l->addWidget(searchButton,0,0,1,1,Qt::AlignLeft);
    l->addWidget(filterCheckBox,0,1,1,1,Qt::AlignRight);
    l->addWidget(table,1,0,1,2);
    l->addWidget(progressBar,2,0,1,2);
    l->addWidget(buttonBox,3,0,1,2);

    this->setLayout(l);
    resize(800*devicePixelRatio(),480*devicePixelRatio());
}

QList<Tag> Dialog::getNewTags()
{
    return newTags;
}

void Dialog::startSearch()
{
    newTags = oldTags;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    progressBar->setEnabled(true);
    progressBar->setMinimum(0);
    progressBar->setMaximum(newTags.size());

    for (int i=0; i<newTags.size(); ++i) {
        if (table->item(i,0)->checkState()!=Qt::Checked) continue;
        QUrl query = createQuery(i);
        if (query.isEmpty() || !query.isValid()) continue;

        QByteArray b=search->get(Request(query));

        parseTrack(b,i);
        progressBar->setValue(i);
        QApplication::processEvents();
    }
    progressBar->setValue(newTags.size());
    QApplication::restoreOverrideCursor();
}

void Dialog::accept()
{
    if (newTags.isEmpty()) newTags = oldTags;
    for (int i=0; i<newTags.size(); ++i) {
        if (table->item(i,0)->checkState()!=Qt::Checked) continue;
        updateTags(newTags[i],widgets[i]);
    }
    QDialog::accept();
}

void Dialog::headerToggled(int column, Qt::CheckState checked)
{
    if (column<0 || column >= table->columnCount()) return;

    if (checked==Qt::PartiallyChecked) return;
    for (int i=0; i<table->rowCount(); ++i)
        table->item(i,column)->setCheckState(checked);
}

QString makeKey(QVariantMap map)
{
    //"key":{"standard":{"letter":"F","sharp":true,"flat":false,"chord":"minor"}},
    QVariant standardKey = map.value("standard");
    if (standardKey.isNull()) return QString(); //what other keys can be?

    map = standardKey.toMap();
    if (map.isEmpty()) return QString();
    QString key = map.value("letter").toString();
    if (map.value("sharp").toBool()) key.append("#");
    if (map.value("flat").toBool()) key.append("b");
    QString chord = map.value("chord").toString();
    if (!chord.isEmpty()) key.append(" "+chord);
    return key;
}

QStringList makeList(const QVariantList &list, const QString &type)
{
    QStringList result;
    Q_FOREACH (const QVariant &v,list) {
        QVariantMap map = v.toMap();
        if (!map.isEmpty()) {
            if (map.value("type").toString()==type)
                result << map.value("name").toString();
        }
    }
    result.removeDuplicates();

    return result;
}

int parseLength(const QString &length)
{
    return length.section(":",0,0).toInt()*60+length.section(":",1,1).toInt();
}

void Dialog::parseTrack(const QByteArray &b, int index)
{
    QList<BeatportTrack> tracks;
    bool ok;

    QVariant parsed = QJsonDocument::fromJson(b).toVariant();
    QVariantMap map = parsed.toMap();
    ok = !map.isEmpty();

    if (ok) {
        QVariantList results = map.value("results").toList();
        Q_FOREACH (const QVariant &v, results) {
            QVariantMap result = v.toMap();
            if (!result.isEmpty()) {
                BeatportTrack track;
                track.id = result.value("id").toInt();
                //track.mixName = result.value("mixName").toString();
                track.title = result.value("title").toString();
                track.isrc = result.value("isrc").toString();
                track.releaseDate = result.value("releaseDate").toString();
                track.length = result.value("length").toString();
                track.bpm = QString::number(result.value("bpm").toInt());
                track.key = makeKey(result.value("key").toMap());
                track.release = result.value("release").toMap().value("name").toString();
                QVariantList artists = result.value("artists").toList();
                track.artists = makeList(artists,"artist");
                track.remixers = makeList(artists,"remixer");
                track.genres = makeList(result.value("genres").toList(),"genre");
                track.subgenres = makeList(result.value("subgenres").toList(),"subgenre");
                track.label = result.value("label").toString(); //The parent label for the track.
                QVariantMap images = result.value("images").toMap();
                if (!images.isEmpty()) {
                    QVariantMap image = images.value("large").toMap();
                    if (image.isEmpty())
                        image = images.value("medium").toMap();
                    if (image.isEmpty())
                        image = images.value("small").toMap();
                    if (!image.isEmpty()) {
                        QByteArray imageData = search->get(Request(QUrl(image.value("url").toString())));
                        QPixmap p;
                        if (p.loadFromData(imageData)) track.image=imageData;
                    }
                }
                track.trackNumber = result.value("trackNumber").toInt();
                tracks << track;
            }
        }
    }

    if (filterCheckBox->isChecked()) {
        //filter tracks by length with 5% margins
        int supposedLength = newTags.at(index).length();
        if (supposedLength>0) {
            for (int i=tracks.size()-1; i>=0; --i) {
                int length = parseLength(tracks.at(i).length);
                if (float(qAbs(length-supposedLength))/supposedLength*100.0 > 5) tracks.removeAt(i);
            }
        }
    }

    if (widgets.at(index))
        widgets.at(index)->setTracks(tracks);
}

QUrl Dialog::createQuery(int index)
{
    QUrl result("http://api.beatport.com/catalog/3/search");

    //Available facets:
    //fieldType, artistName, artistId, labelName,
    //labelId, releaseTypeName, subGenreName,
    //subGenreId, currentStatus, genreName, genreId

    // http://api.beatport.com/catalog/search?query=Freedom&facets[]=fieldType:track&facets[]=performerName:Vigil%20Coma&format=xml&v=2.0
    QString title = table->item(index,2)->text();
    QString performer = table->item(index,1)->text();
    QString remixer=table->item(index,1)->data(Qt::UserRole+1).toString();

    if (title.isEmpty() && performer.isEmpty() && remixer.isEmpty()) return QUrl();

    QList<QPair<QString, QString> > queryItems;

    if (title.isEmpty()) {
        queryItems.append(QPair<QString, QString>("query",performer));
    }
    else {
        queryItems.append(QPair<QString, QString>("query",title));
        queryItems.append(QPair<QString, QString>("facets[]",performer.prepend("performerName:")));
//        if (!remixer.isEmpty())
//            queryItems.append(QPair<QString, QString>("facets[]",remixer.prepend("performerName:")));
    }
    queryItems.append(QPair<QString, QString>("facets[]","fieldType:track"));
    //queryItems.append(QPair<QString, QString>("page","1"));
    //queryItems.append(QPair<QString, QString>("perPage","20"));

    QUrlQuery urlQuery;
    urlQuery.setQueryItems(queryItems);
    result.setQuery(urlQuery);
    return result;
}

void Dialog::updateTags(Tag &tag, BeatportWidget *w)
{
    BeatportTrack track = w->currentTrack();
    if (track.id!=0) {
        if (!track.bpm.isEmpty()) tag.setTag(TEMPO,track.bpm);
        if (!track.key.isEmpty()) {
            tag.setTag(KEY,track.key);
            tag.setTag(COMMENT,track.key);
        }
        if (!track.releaseDate.isEmpty()) tag.setTag(YEAR,track.releaseDate.left(4));
        if (!track.artists.isEmpty()) tag.setTag(ARTIST,track.artists.join(", "));
        if (!track.remixers.isEmpty()) tag.setTag(REMIXEDBY,track.remixers.join(", "));

        QStringList genres=track.genres;
        genres.append(track.subgenres);
        genres.removeDuplicates();
        if (!genres.isEmpty())
            tag.setTag(GENRE,genres.join(", "));

        if (!track.label.isEmpty()) tag.setTag(PUBLISHER,track.label);
        if (!track.title.isEmpty()) tag.setTag(TITLE,track.title);
        if (!track.image.isNull()) {
            CoverImage img = tag.image();
            img.setPixmap(track.image);
            tag.setImage(img);
        }
        if (!track.isrc.isEmpty()) tag.setTag(ISRC,track.isrc);
        if (track.trackNumber != 0) tag.setTag(TRACKNUMBER,QString::number(track.trackNumber));
        if (!track.release.isEmpty()) tag.setTag(ALBUM,track.release);

        //QString mixName; //The mix/remix name of the track.
    }
}

QString getFromPairList(char id, const PairList &list)
{
    Q_FOREACH (const StringPair &p, list) {
        if (p.first.at(0)==id) return p.second;
    }
    return QString();
}

void Dialog::parseFileName(QString &title, QString &artists, QString &remixer, const int &n)
{
    const Tag &tag = oldTags.at(n);

    QStringList l;
    l << tag.artist();
    l << tag.performer();
    l << tag.albumArtist();
    l.removeDuplicates();
    l.removeAll(QString(""));
    artists = l.join("; ");

    remixer=tag.remixedby();

    title = tag.title();

    if (title.isEmpty() || artists.isEmpty()) {
        QString pattern = "%A-%t (%x)%%";

        PairList list = TagParser::parse(tag.fileNameExt(), pattern);

        if (title.isEmpty()) title=getFromPairList('t',list).simplified();

        if (artists.isEmpty()) artists = getFromPairList('A',list).simplified();
        if (remixer.isEmpty()) remixer = getFromPairList('x',list).simplified();
        remixer.remove("remix",Qt::CaseInsensitive);
        remixer=remixer.simplified();
        if (remixer.compare("original mix",Qt::CaseInsensitive)==0) remixer.clear();
    }
}


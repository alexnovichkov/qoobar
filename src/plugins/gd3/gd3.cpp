#include "gd3.h"

#include <QtWidgets>
#include <QLibrary>
#include <QUrl>
#include <QSettings>
#include <QXmlStreamReader>
#include "qoobarglobals.h"

#ifdef Q_OS_MACOS
#include "discid.h"
#endif

constexpr int maximumCDTracks = 99;
constexpr int firstTrackOffset = 150;

Request GD3Plugin::queryForManualSearch(const QStringList &list)
{
    QString requestString = getRequestString("SearchAlbum", list.at(0), list.value(1));
    if (requestString.isEmpty()) return Request();

    QByteArray requestArray = requestString.toUtf8();
    QByteArray postDataSize = QByteArray::number(requestArray.size());

    Request r(QUrl("https://gdwebservice.getdigitaldata.com/gd3.asmx"), requestArray);
    r.addRawHeader("Content-Type", "text/xml; charset=utf-8");
    r.addRawHeader("Content-Length", postDataSize);
    r.addRawHeader("SOAPAction", "\"https://gdwebservice.getdigitaldata.com/GD3/SearchAlbum\"");

    return r;
}

Request GD3Plugin::queryForSearchByFiles(const QVector<int> &lengths)
{
    return query(lengths);
}

Request GD3Plugin::queryForCD()
{
    return query(QVector<int>());
}

Request GD3Plugin::queryForPicture(const QString &/*url*/)
{
    return Request();
}

Request GD3Plugin::queryForRelease(const QString &url)
{
    QString requestString = getRequestString("RetrieveAlbum", url);
    if (requestString.isEmpty()) return Request();

    QByteArray requestArray = requestString.toUtf8();
    QByteArray postDataSize = QByteArray::number(requestArray.size());

    Request r(QUrl("https://gdwebservice.getdigitaldata.com/gd3.asmx"), requestArray);
    r.addRawHeader("Content-Type", "text/xml; charset=utf-8");
    r.addRawHeader("Content-Length", postDataSize);
    r.addRawHeader("SOAPAction", "\"https://gdwebservice.getdigitaldata.com/GD3/RetrieveAlbum\"");

    return r;
}

QStringList GD3Plugin::getTOC(const QVector<int> &lengths)
{
    QStringList offsetsInFrames;
    m_errorString.clear();

    if (lengths.isEmpty()) {//get discID by CD
#ifndef Q_OS_MACOS
        /*Testing for loadable libraries*/
        QString libName;
#ifdef Q_OS_LINUX
        QFileInfoList libFiles = QDir("/usr/lib").entryInfoList(QStringList()<<"*discid*", QDir::Files | QDir::NoSymLinks);
        libFiles << QDir("/usr/lib/i386-linux-gnu").entryInfoList(QStringList()<<"*discid*", QDir::Files | QDir::NoSymLinks);
        if (!libFiles.isEmpty()) libName = libFiles.at(0).canonicalFilePath();
#else
        libName = "libdiscid";
#endif

        QLibrary lib(libName);
        if (!lib.load()) {
            libName = "discid";
            lib.setFileName(libName);
        }

        if (!lib.load()) {
            m_errorString=tr("Please install libdiscid");
            return offsetsInFrames;
        }

        discid_new = reinterpret_cast<Discid_new>(lib.resolve("discid_new"));
        discid_free = reinterpret_cast<Discid_free>(lib.resolve("discid_free"));

        discid_read = (Discid_read)lib.resolve("discid_read");
        discid_get_first_track_num = reinterpret_cast<Discid_get_first_track_num>(lib.resolve("discid_get_first_track_num"));
        discid_get_last_track_num = reinterpret_cast<Discid_get_last_track_num>(lib.resolve("discid_get_last_track_num"));
        discid_get_track_offset = reinterpret_cast<Discid_get_track_offset>(lib.resolve("discid_get_track_offset"));
#endif

        DiscId *disc = discid_new();
        if (!disc) {
            m_errorString=tr("Cannot read CD");
            return offsetsInFrames;
        }

        int disc_read_result=discid_read(disc, nullptr);
        if (disc_read_result==0) {
            m_errorString=tr("Cannot read CD");
            discid_free(disc);
            return offsetsInFrames;
        }

        int firstTrack = discid_get_first_track_num(disc);
        int lastTrack = discid_get_last_track_num(disc);

        for (int i=firstTrack; i<=lastTrack; ++i)
            offsetsInFrames << QString::number(discid_get_track_offset(disc, i));

        discid_free(disc);
#ifndef Q_OS_MACOS
        lib.unload();
#endif
    }
    else {//get TOC by files
        const int size = qMin(lengths.size(), maximumCDTracks);
        QVector<int> offsets(size + 1, 0);

        offsets[0] = firstTrackOffset;//by default the first track has offset 150

        for (int i=0; i<size-1; ++i)
            offsets[i+1] = offsets.at(i) + lengths.at(i)*(firstTrackOffset/2);

        for (int i=0; i<size; ++i)
            offsetsInFrames << QString::number(offsets.at(i));
    }

    return offsetsInFrames;
}

Request GD3Plugin::query(const QVector<int> &lengths)
{
    QStringList tocList = getTOC(lengths);
    if (tocList.isEmpty()) return Request();
    QString toc=tocList.join(" ");
    //toc="150 19255 37050 57477 77585 95100 111452 132267 154292 171860 189527 203747 233967";

    QString requestString = getRequestString("MatchCDID", toc);
    if (requestString.isEmpty()) return Request();

    QByteArray requestArray = requestString.toUtf8();
    QByteArray postDataSize = QByteArray::number(requestArray.size());

    Request r(QUrl("https://gdwebservice.getdigitaldata.com/gd3.asmx"), requestArray);
    r.addRawHeader("Content-Type", "text/xml; charset=utf-8");
    r.addRawHeader("Content-Length", postDataSize);
    r.addRawHeader("SOAPAction", "\"https://gdwebservice.getdigitaldata.com/GD3/MatchCDID\"");

    return r;
}

QString GD3Plugin::getRequestString(const QString &operation, const QString &argument, const QString &argument1)
{
    QMap<QString, QString> auth = authenticationInfo();
    QString login = auth.value("user");
    if (login.isEmpty()) login = auth.value("login");
    QString password = auth.value("password");

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox msgBox(QMessageBox::Critical, tr("GD3 database"), tr("GD3 user name or password is incorrect"),QMessageBox::Ok);
       // msgBox.setWindowModality(Qt::WindowModal);
        msgBox.exec();

        return QString();
    }

    QString body;
    if (operation=="MatchCDID")
        body = QString("    <MatchCDID xmlns=\"https://gdwebservice.getdigitaldata.com/GD3\">\n"
                       "      <CDID>%3</CDID>\n"
                       "    </MatchCDID>\n").arg(argument);
    else if (operation=="GetNumberOfLookupsRemaining")
        body = "<GetNumberOfLookupsRemaining xmlns=\"https://gdwebservice.getdigitaldata.com/GD3\" />";
    else if (operation=="RetrieveAlbum")
        body = QString("    <RetrieveAlbum xmlns=\"https://gdwebservice.getdigitaldata.com/GD3\">\n"
                       "      <AlbumID>%1</AlbumID>\n"
                       "      <LanguageCode>0</LanguageCode>\n"
                       "    </RetrieveAlbum>\n").arg(argument);
    else if (operation=="SearchAlbum") {
        body = QString("    <SearchAlbum xmlns=\"https://gdwebservice.getdigitaldata.com/GD3\">\n"
                       "      <Artist>%1</Artist>\n"
                       "      <AlbumTitle>%2</AlbumTitle>\n"
                       "      <NumberResults>20</NumberResults>\n"
                       "    </SearchAlbum>\n").arg(argument, argument1);
    }

    QString requestString = QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                    "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
                                    "xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
                                    "  <soap:Header>\n"
                                    "    <AuthHeader xmlns=\"https://gdwebservice.getdigitaldata.com/GD3\">\n"
                                    "      <Username>%1</Username>\n"
                                    "      <Password>%2</Password>\n"
                                    //"      <ProductCode></ProductCode>\n"
                                   // "      <RequestID></RequestID>\n"
                                    "    </AuthHeader>\n"
                                    "  </soap:Header>\n"
                                    "  <soap:Body>\n%3"
                                    "  </soap:Body>\n"
                                    "</soap:Envelope>")
            .arg(login, password, body);

    return requestString;
}

QList<SearchResult> GD3Plugin::parseResponse(const QByteArray &response)
{
    QList<SearchResult> results;
    m_errorString.clear();

    QXmlStreamReader x(response);
    bool withinAlbum=false;

    while (!x.atEnd()) {
        x.readNext();
        // do processing
        if (x.isStartElement()) {
            auto name = x.name();
            if (name.compare(QSL("SearchResult"))==0) {
                results.append(SearchResult());
            }
            else if (name.compare(QSL("AlbumID"))==0) {
                results.last().fields.insert("url",x.readElementText());
            }
            else if (name.compare(QSL("Publisher"))==0) {
                results.last().fields.insert("label",x.readElementText());
            }
            else if (name.compare(QSL("NumberOfTracks"))==0) {
                results.last().fields.insert("numberoftracks",x.readElementText());
            }
            else if (name.compare(QSL("AlbumCode"))==0) {
                if (withinAlbum) {
                    results.last().fields.insert("url",x.readElementText());
                    withinAlbum=false;
                }
                else {
                    results.append(SearchResult());
                    withinAlbum=true;
                }
            }
            else if (name.compare(QSL("Album"))==0) {
                results.last().fields.insert("album",x.readElementText());
            }
            else if (name.compare(QSL("Artist"))==0) {
                results.last().fields.insert("artist",x.readElementText());
            }
        }
    }
    if (x.hasError()) {
        m_errorString=x.errorString();
    }
    if (!results.isEmpty())
        if (results.last().fields.value("url").isEmpty())
            results.removeLast();
    for (auto &r : results) r.releaseInfo = releaseToList(r);
    m_errorString="Sample error from gd3 plugin";
    return results;
}

void insertToMap(QMap<QString, QString> &map, const QString &key, const QString &s)
{
    if (!s.isEmpty()) map.insert(key,s);
}

SearchResult GD3Plugin::parseRelease(const QByteArray &response)
{
    SearchResult r; r.loaded = true;
    m_errorString.clear();

    QHash<QString, QString> fieldsMap;
    fieldsMap.insert("Album", "album");
    fieldsMap.insert("Genre", "genre");
    fieldsMap.insert("AlbumYear", "year");
    fieldsMap.insert("Publisher", "label");
    fieldsMap.insert("Conductor", "conductor");
    fieldsMap.insert("CatalogNumber", "catno");

    QXmlStreamReader x(response);
    while (!x.atEnd()) {
        x.readNext();
        // do processing
        if (x.isStartElement()) {
            auto name = x.name();
            if (fieldsMap.contains(name.toString()))
                insertToMap(r.fields,
                            fieldsMap.value(name.toString()),
                            x.readElementText());
            else if (name.compare(QSL("Artist"))==0) {
                QString s = x.readElementText();
                if (!s.isEmpty()) {
                    Artist a; a.fields.insert("name", s);
                    r.artists.append(a);
                }
            }
            else if (name.compare(QSL("AlbumImage"))==0) {
                QByteArray data = QByteArray::fromBase64(x.readElementText().toLatin1());
                QPixmap img;
                if (img.loadFromData(data))
                    r.image.setPixmap(data);
            }
            else if (name.compare(QSL("TrackMeta"))==0) {
                Track t;
                t.cd=-1;
                r.tracks.append(t);
            }
            else if (name.compare(QSL("TrackName"))==0) {
                insertToMap(r.tracks.last().fields,"title", x.readElementText());
            }
            else if (name.compare(QSL("TrackLength"))==0) {
                auto len = x.readElementText().toInt() / 1000;
                insertToMap(r.tracks.last().fields,"length",
                            Qoobar::formatLength(len));
                r.tracks.last().length = len;
            }
            else if (name.compare(QSL("TrackNumber"))==0) {
                insertToMap(r.tracks.last().fields,"tracknumber", x.readElementText());
            }
            else if (name.compare(QSL("TrackArtist"))==0) {
                QString s = x.readElementText();
                if (!s.isEmpty()) {
                    Artist a;
                    a.fields.insert("name", s);
                    r.tracks.last().artists.append(a);
                }
            }
            else if (name.compare(QSL("ComposerName"))==0) {
                QString s = x.readElementText();
                if (!s.isEmpty()) {
                    Artist a;
                    a.fields.insert("name", s);
                    a.fields.insert("role", "composer");
                    r.tracks.last().artists.append(a);
                }
            }
            else if (name.compare(QSL("ConductorName"))==0) {
                QString s = x.readElementText();
                if (!s.isEmpty()) {
                    Artist a;
                    a.fields.insert("name", s);
                    a.fields.insert("role", "conductor");
                    r.tracks.last().artists.append(a);
                }
            }
            else if (name.compare(QSL("Orchestra"))==0) {
                QString s = x.readElementText();
                if (!s.isEmpty()) {
                    Artist a;
                    a.fields.insert("name", s);
                    a.fields.insert("role", "orchestra");
                    r.tracks.last().artists.append(a);
                }
            }
            else if (name.compare(QSL("Chorus"))==0) {
                QString s = x.readElementText();
                if (!s.isEmpty()) {
                    Artist a;
                    a.fields.insert("name", s);
                    a.fields.insert("role", "chorus");
                    r.tracks.last().artists.append(a);
                }
            }
        }
    }
    if (x.hasError()) {
        m_errorString=x.errorString();
    }
    r.releaseInfo = releaseToList(r);

    return r;
}

QStringList GD3Plugin::releaseToList(const SearchResult &r)
{
    QStringList result;
    result << r.fields.value("album")
           << r.fields.value("artist");
    QString s=r.fields.value("label");
    if (!r.fields.value("numberoftracks").isEmpty()) {
        s.append(", "+tr("%n track(s)",nullptr,r.fields.value("numberoftracks").toInt()));
    }
    if (!s.isEmpty()) result << s;
    return result;
}

LoginDialog::LoginDialog(const QString &login, const QString &pass, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("GD3 database login and password"));
    loginEdit = new QLineEdit(this);
    loginEdit->setText(login);
    passEdit = new QLineEdit(this);
    passEdit->setText(pass);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    auto *ll = new QVBoxLayout;
    ll->addWidget(new QLabel(tr("Please set your GD3 login and password"),this));
    auto *l = new QHBoxLayout;
    l->addWidget(new QLabel(tr("Login:"),this));
    l->addWidget(loginEdit);
    l->addWidget(new QLabel(tr("Password:"),this));
    l->addWidget(passEdit);
    ll->addLayout(l);
    ll->addWidget(buttonBox);

    setLayout(ll);
}

QStringList LoginDialog::getLoginPass()
{
    return QStringList()<<loginEdit->text() << passEdit->text();
}


int GD3Plugin::preferredPauseSize()
{
    return 0;
}

QMap<QString, QString> GD3Plugin::authenticationInfo()
{
#ifdef QOOBAR_PORTABLE
    QSettings settings(QSL("qoobar.ini"),QSettings::IniFormat);
#else
    QSettings settings("qoobar","qoobar");
#endif
    QString login = settings.value("gd3login").toString();
    QString password = settings.value("gd3password").toString();

    if (login.isEmpty() || password.isEmpty()) {
        LoginDialog dialog(login, password);
        if (dialog.exec()) {
            QStringList l = dialog.getLoginPass();
            login = l.at(0);
            password = l.at(1);
            settings.setValue("gd3login",login);
            settings.setValue("gd3password",password);
        }
    }
    QMap<QString, QString> map;
    map.insert("user",login);
    map.insert("password", password);
    return map;
}

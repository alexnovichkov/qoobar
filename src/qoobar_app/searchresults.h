/**
 * \file searchresults.h
 * Some classes for search in the internet
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 11 Aug 2011
 *
 * Copyright (C) 2011  Alex Novichkov
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

#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H
#include <QStringList>
#include "coverimage.h"
#include <QMap>
#include "enums.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <QJsonArray>
#include <QVariant>

struct Artist {
    QMap<QString,QString> fields;

    const QString toString() const
    {
        QStringList artist;
        artist << fields.value(QSL("name"));
        artist << fields.value(QSL("role"));
        artist << fields.value(QSL("tracks"));

        artist.removeAll("");

        if (artist.size()==3)
            return QString("%1 (%2, %3)")
                      .arg(artist.at(0), artist.at(1), artist.at(2));
        if (artist.size()==2)
            return QString("%1 (%2)")
                      .arg(artist.at(0), artist.at(1));
        return artist.at(0);
    }
    QJsonValue toJson() const {
        QJsonObject v;
        for (const auto &a: fields) v.insert(a, fields.value(a));
        return v;
    }
    static Artist fromJson(const QJsonObject &json)
    {
        Artist a;
        const auto map = json.toVariantMap();
        for (const auto &key: map.keys()) a.fields.insert(key, map.value(key).toString());

        return a;
    }
};

static QString artistsText(const QList<Artist> &l)
    {
        QStringList result;
        Q_FOREACH(const Artist &a,l)
            result << a.toString();
        return result.join(QSL("; "));
    }

struct Track {
    QMap<QString,QString> fields;
    QList<Artist> artists;
    int cd;

    Track() : cd(1) {}
    const QStringList toStringList() const
    {
        QStringList result;

        result << fields.value(QSL("tracknumber"))
               << fields.value(QSL("title"))
               << artistsText(artists)
               << fields.value(QSL("extraData"))
               << fields.value(QSL("length"));
        return result;
    }

    QJsonValue toJson() const {
        QJsonObject v;
        v.insert("cd", cd);

        for (const auto &a: fields) v.insert(a, fields.value(a));

        //if (!artists.isEmpty())
        {
            QJsonArray artistsList;
            for(const auto &a: artists) artistsList.append(a.toJson());
            v.insert("artists", artistsList);
        }
        return v;
    }
    static Track fromJson(const QJsonObject &json)
    {
        Track t;
        t.cd = json["cd"].toInt();
        if (json.contains("artists")) {
            auto list = json["artists"].toArray();
            for (const auto &l: list) t.artists.append(Artist::fromJson(l.toObject()));
        }

        const auto map = json["fields"].toObject().toVariantMap();
        for (const auto &key: map.keys()) t.fields.insert(key, map.value(key).toString());

        return t;
    }
};

struct SearchResult {
    SearchResult() : loaded(false), cdCount(1) {}

    const QStringList toStringList() const
    {
        QStringList l;
        l << fields.value(QSL("album"));
        l << fields.value(QSL("label"));
        l << fields.value(QSL("extraData"));
        l.removeAll(QLS(""));
        return l;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        o.insert("loaded", loaded);
        o.insert("cdCount", cdCount);
        o.insert("cached", true);
        if (!artists.isEmpty())
        {
            QJsonArray artistsList;
            for(const auto &a: this->artists) artistsList.append(a.toJson());
            o.insert("artists", artistsList);
        }
        if (!tracks.isEmpty())
        {
            QJsonArray tracksList;
            for(const auto &a: this->tracks) tracksList.append(a.toJson());
            o.insert("tracks", tracksList);
        }
        QJsonObject f;
        for (const auto &a: fields.keys())
            f.insert(a, fields.value(a));
        o.insert("fields", f);
        if (!image.isEmpty())
        {
            QJsonObject im;
            im.insert("type", image.type());
            im.insert("mimetype", image.mimetype());
            im.insert("description", image.description());
            im.insert("data", QString::fromLatin1(image.pixmap().toBase64()));
            o.insert("image", im);
        }

        return o;
    }
    static SearchResult fromJson(const QJsonObject &json)
    {
        SearchResult r;
        r.query = json["query"].toString();
        r.cached = json["cached"].toBool();
        r.loaded = json["loaded"].toBool();
        r.cdCount = json["cdCount"].toInt();

        if (json.contains("image")) {
            CoverImage image;
            auto img = json["image"].toObject();
            image.setType(img["type"].toInt());
            image.setMimetype(img["mimetype"].toString());
            image.setDescription(img["description"].toString());
            image.setPixmap(QByteArray::fromBase64(img["data"].toString().toLatin1()));
            r.image = image;
        }

        if (json.contains("fields")) {
            auto map = json["fields"].toObject().toVariantMap();
            for (const auto &key: map.keys()) r.fields.insert(key, map.value(key).toString());
        }
        if (json.contains("artists")) {
            auto list = json["artists"].toArray();
            for (const auto &l: list) r.artists.append(Artist::fromJson(l.toObject()));
        }
        if (json.contains("tracks")) {
            auto list = json["tracks"].toArray();
            for (const auto &l: list) r.tracks.append(Track::fromJson(l.toObject()));
        }
        return r;
    }
    QString lengths() const
    {
        QList<int> lengths;
    }

    bool loaded;
    int cdCount;
    QMap<QString,QString> fields;
    QList<Artist> artists;
    QList<Track> tracks;
    CoverImage image;
    bool cached = false;
    QString query;
};


#endif // SEARCHRESULTS_H

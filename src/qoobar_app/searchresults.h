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
                      .arg(artist.at(0))
                      .arg(artist.at(1))
                      .arg(artist.at(2));
        if (artist.size()==2)
            return QString("%1 (%2)")
                      .arg(artist.at(0))
                      .arg(artist.at(1));
        return artist.at(0);
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

    int cd;
};

struct SearchResult {
    SearchResult() : loaded(false), cdCount(1) {}

    const QStringList toStringList() const
    {
        QStringList l;
        l << fields.value(QSL("album"));
        l << fields.value(QSL("label"));
        l << fields.value(QSL("extraData"));
        l.removeAll(QSL(""));
        return l;
    }

    bool loaded;
    int cdCount;
    QMap<QString,QString> fields;
    QList<Artist> artists;
    QList<Track> tracks;
    CoverImage image;
};


#endif // SEARCHRESULTS_H

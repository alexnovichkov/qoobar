/**
 * \file downloadhelper.h
 * An abstract class to provide queries
 * for internet search and to parse responses.
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

#ifndef DOWNLOADHELPER_H
#define DOWNLOADHELPER_H

#include <QObject>
#include <QUrl>
#include "searchresults.h"

struct Request {
    Request(const QUrl &url=QUrl(), const QByteArray &data=QByteArray())
        : url(url), data(data) {}
    bool isEmpty() const {return url.isEmpty();}
    void addRawHeader(const QByteArray &header, const QByteArray &data) {
        rawHeaders.insert(header,data);
    }

    QUrl url;
    QByteArray data;
    QMap<QByteArray, QByteArray> rawHeaders;
};

class DownloadHelper : public QObject
{
    Q_OBJECT
public:
    explicit DownloadHelper(QObject *parent = 0) : QObject(parent)
    {}
    virtual ~DownloadHelper() {}
    /*returns localized text for menus, tollbars etc.*/
    virtual QString text() = 0;

    /*returns a short unique key that identifies a plugin*/
    virtual QString key() = 0;

    /*returns a version string*/
    virtual QString version() = 0;

    virtual Request queryForManualSearch(const QStringList &) = 0;
    virtual Request queryForCD() = 0;
    virtual Request queryForSearchByFiles(const QList<int> &) = 0;
    virtual Request queryForPicture(const QString &) = 0;
    virtual Request queryForRelease(const QString &) = 0;

    virtual bool canSearchManually() = 0;
    virtual bool canSearchByCD() = 0;
    virtual bool canSearchByFiles() = 0;

    virtual QList<SearchResult> parseResponse(const QByteArray &) = 0;
    virtual SearchResult parseRelease(const QByteArray &) = 0;
    virtual bool needsPause() = 0;
    virtual QStringList releaseToList(const SearchResult &r) = 0;
Q_SIGNALS:
    void error(const QString &);
};

#endif // DOWNLOADHELPER_H

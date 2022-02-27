/**
 * \file corenetworksearch.h
 * An object to synchronously download data from remote URL
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date Nov 27 2012
 *
 * Copyright (C) 2012  Alex Novichkov
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

#ifndef CORENETWORKSEARCH_H
#define CORENETWORKSEARCH_H

#include <QObject>
class QNetworkAccessManager;
class DownloadHelper;
class QNetworkProxy;
class QAuthenticator;
struct Request;
//#include <QUrl>

class CoreNetworkSearch : public QObject
{
    Q_OBJECT
public:
    explicit CoreNetworkSearch(QObject *parent = 0);
    void setProxy(const QString &proxyServer,
                  const QString &proxyPort,
                  const QString &proxyLogin,
                  const QString &proxyPassword);
    void setUserAgent(const QString &userAgent);
    QByteArray get(const Request &request);
    QByteArray get(const QUrl &url, const QByteArray &data, const QMap<QByteArray, QByteArray> &rawHeaders);
Q_SIGNALS:
    void error(const QString &);
private Q_SLOTS:
    void setProxyAuthentication(const QNetworkProxy &, QAuthenticator*);
    void analyseError();
private:
    QNetworkAccessManager *m;
    QString userAgent;
};

#endif // CORENETWORKSEARCH_H

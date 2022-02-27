/**
 * \file corenetworksearch.cpp
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

#include "corenetworksearch.h"
#include <QTemporaryFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QAuthenticator>
#include <QEventLoop>
#include <QTimer>
#include <QPixmap>
#include "zlib.h"
#include "zconf.h"
#include "downloadhelper.h"
#include "application.h"

const int bufferSize = 4096;

void unzipResponse(QByteArray& response)
{
    QTemporaryFile file;
    if (file.open()) {
        file.write(response);
        QByteArray fname = file.fileName().toLatin1();
        file.close();

        response.clear();

        gzFile gzDoc = gzopen(fname.constData(), "rb");
        if (gzDoc != nullptr) {
            char buff[bufferSize+1];
            int i;
            while ((i = gzread(gzDoc, &buff, bufferSize)) > 0) {
                buff[i] = '\0';
                response.append(buff);
            }
        }
        gzclose(gzDoc);
    }
}

CoreNetworkSearch::CoreNetworkSearch(QObject *parent) :  QObject(parent)
{
    if (!QSslSocket::supportsSsl()) Q_EMIT error(tr("This Qt version doesn't support SSL.\n"
                                                    "Please compile Qoobar with another Qt version"));

    m = new QNetworkAccessManager(this);
    if (App->useProxy)
        setProxy(App->proxyServer,
                 App->proxyPort,
                 App->proxyLogin,
                 App->proxyPassword);
}

void CoreNetworkSearch::setProxy(const QString &proxyServer,
                                 const QString &proxyPort,
                                 const QString &proxyLogin,
                                 const QString &proxyPassword)
{
    if (!proxyServer.isEmpty() && !proxyPort.isEmpty()) {
        QNetworkProxy proxy(QNetworkProxy::HttpProxy,
                            proxyServer,
                            proxyPort.toInt(),
                            proxyLogin,
                            proxyPassword);
        m->setProxy(proxy);

        connect(m,SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
                SLOT(setProxyAuthentication(QNetworkProxy,QAuthenticator*)));
    }
}

void CoreNetworkSearch::setProxyAuthentication(const QNetworkProxy &proxy, QAuthenticator *a)
{
    a->setPassword(proxy.password());
    a->setUser(proxy.user());
}

QByteArray CoreNetworkSearch::get(const Request &request)
{
    QByteArray response;
    auto *loop = new QEventLoop();

    QNetworkRequest r;

    QMapIterator<QByteArray,QByteArray> i(request.rawHeaders);
    while (i.hasNext()) {
        i.next();
        r.setRawHeader(i.key(), i.value());
    }
    //if (acceptGzipped)
    //    r.setRawHeader("Accept-Encoding", "gzip");
    r.setRawHeader("User-Agent",userAgent.toLatin1());
//    qDebug()<<request.url;
    r.setUrl(request.url);

    QNetworkReply * reply=nullptr;
    if (request.data.isEmpty())
        reply = m->get(r);
    else
        reply = m->post(r, request.data);

    connect(reply, SIGNAL(finished()),loop, SLOT(quit()));
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(analyseError()));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(analyseError()));
    loop->exec();

//qDebug()<<reply->error()<<reply->errorString();

    if (reply->error()==QNetworkReply::NoError) {
        response = reply->readAll();
        if (reply->hasRawHeader("Content-Encoding") && reply->rawHeader("Content-Encoding")=="gzip") {
            //we have to unzip the response
            unzipResponse(response);
//            qDebug()<<response;
        }
    }
    else {

    }
    reply->deleteLater();
    loop->deleteLater();

    return response;
}

QByteArray CoreNetworkSearch::get(const QUrl &url, const QByteArray &data, const QMap<QByteArray, QByteArray> &rawHeaders)
{
    QByteArray response;
    auto *loop = new QEventLoop();

    QNetworkRequest r;

    QMapIterator<QByteArray,QByteArray> i(rawHeaders);
    while (i.hasNext()) {
        i.next();
        r.setRawHeader(i.key(), i.value());
    }
    //if (acceptGzipped)
    //    r.setRawHeader("Accept-Encoding", "gzip");
    r.setRawHeader("User-Agent",userAgent.toLatin1());
//    qDebug()<<url;
    r.setUrl(url);

    QNetworkReply * reply=nullptr;
    if (data.isEmpty())
        reply = m->get(r);
    else
        reply = m->post(r, data);

    connect(reply, SIGNAL(finished()),loop, SLOT(quit()));
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(analyseError()));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(analyseError()));
    loop->exec();

//qDebug()<<reply->error()<<reply->errorString();

    if (reply->error()==QNetworkReply::NoError) {
        response = reply->readAll();
        if (reply->hasRawHeader("Content-Encoding") && reply->rawHeader("Content-Encoding")=="gzip") {
            //we have to unzip the response
            unzipResponse(response);
//            qDebug()<<response;
        }
    }
    else {

    }
    reply->deleteLater();
    loop->deleteLater();

    return response;
}

void CoreNetworkSearch::analyseError()
{
    if (auto *reply = qobject_cast<QNetworkReply *>(sender())) {
        Q_EMIT error(reply->errorString());
    }
}

void CoreNetworkSearch::setUserAgent(const QString &userAgent)
{
    this->userAgent = userAgent;
}



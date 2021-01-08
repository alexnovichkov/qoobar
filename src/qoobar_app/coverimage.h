/**
 * \file coverimage.h
 * Implicitly shared cover image data
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

#ifndef COVERIMAGE_H
#define COVERIMAGE_H

#include <QByteArray>
#include <QSharedDataPointer>
#include <QString>
#include <QSharedData>

class CoverImageData : public QSharedData
{
public:
    CoverImageData();
    CoverImageData(const CoverImageData &other);

    QByteArray pixmap;
    int type;
    QString mimetype;
    QString description;
};

class CoverImage
{
public:
    CoverImage();
    CoverImage(const QByteArray &pixmap,int type,const QString &mimetype,const QString &description);
    CoverImage(const CoverImage &other);
    CoverImage& operator=(const CoverImage &other);

    void setType(int type);
    void setPixmap(const QByteArray &pixmap);
    void setMimetype(const QString &mimetype);
    void setDescription(const QString &description);

    int type() const;
    QByteArray pixmap() const;
    QString mimetype() const;
    QString description() const;
    bool operator==(const CoverImage &image) const;
    bool operator!=(const CoverImage &image) const;
    void clear();
    bool isEmpty() const;
private:
    QSharedDataPointer<CoverImageData> d;
};

#endif // COVERIMAGE_H

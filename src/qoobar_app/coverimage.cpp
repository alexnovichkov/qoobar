/**
 * \file coverimage.cpp
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

#include "coverimage.h"

CoverImageData::CoverImageData(): type(-1)
{ }

CoverImageData::CoverImageData(const CoverImageData &other)
    : QSharedData(other),
      pixmap(other.pixmap),
      type(other.type),
      mimetype(other.mimetype),
      description(other.description)
{ }

CoverImage::CoverImage() : d{new CoverImageData}
{

}

CoverImage::CoverImage(const QByteArray &pixmap,int type,const QString &mimetype,const QString &description)
    : d{new CoverImageData}
{
    setPixmap(pixmap);
    setType(type);
    setMimetype(mimetype);
    setDescription(description);
}

CoverImage::CoverImage(const CoverImage &other)
    : d(other.d)
{}

CoverImage& CoverImage::operator=(const CoverImage &other)
{
    d = other.d;
    return *this;
}

void CoverImage::setType(int type)
{
    d->type = type;
}
void CoverImage::setPixmap(const QByteArray &pixmap)
{
    d->pixmap = pixmap;
}
void CoverImage::setMimetype(const QString &mimetype)
{
    d->mimetype = mimetype;
}
void CoverImage::setDescription(const QString &description)
{
    d->description = description;
}

int CoverImage::type() const
{
    return d->type;
}
QByteArray CoverImage::pixmap() const
{
    return d->pixmap;
}
QString CoverImage::mimetype() const
{
    return d->mimetype;
}
QString CoverImage::description() const
{
    return d->description;
}
bool CoverImage::operator==(const CoverImage &image) const
{
    return (d->type==image.type() &&
            d->pixmap==image.pixmap() &&
            d->mimetype==image.mimetype() &&
            d->description==image.description());
}
bool CoverImage::operator!=(const CoverImage &image) const
{
    return (d->type!=image.type() ||
            d->pixmap!=image.pixmap() ||
            d->mimetype!=image.mimetype() ||
            d->description!=image.description());
}
void CoverImage::clear()
{
    d->pixmap.clear();
    d->type=-1;
    d->mimetype.clear();
    d->description.clear();
}

bool CoverImage::isEmpty() const
{
    return (d->type==-1 &&
            d->pixmap.isEmpty() &&
            d->mimetype.isEmpty() &&
            d->description.isEmpty());
}

/**
 * \file id3v1stringhandler.cpp
 * A Qt implementation of TagLib::ID3v1::StringHandler
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

#include "id3v1stringhandler.h"
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QtCore5Compat/QTextCodec>
#else
#include <QTextCodec>
#endif

ID3v1StringHandler::ID3v1StringHandler() : codec(0)
{

}

TagLib::String ID3v1StringHandler::parse(const TagLib::ByteVector &data) const
{
    QString s=codec->toUnicode(QByteArray(data.data(),qstrnlen(data.data(),data.size())));
    return TagLib::String(s.toStdWString());
}

TagLib::ByteVector ID3v1StringHandler::render(const TagLib::String &s) const
{
    QByteArray a=codec->fromUnicode(QString::fromStdWString(s.toWString()));
    TagLib::ByteVector b=TagLib::ByteVector(a.constData(),a.length());
    return b;
}

void ID3v1StringHandler::setCodec(QTextCodec *codec)
{
    this->codec = codec;
    TagLib::ID3v1::Tag::setStringHandler(this);
}

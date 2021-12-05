/**
 * \file id3v1stringhandler.h
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

#ifndef ID3V1STRINGHANDLER_H
#define ID3V1STRINGHANDLER_H

class QTextCodec;
#include "taglib/id3v1tag.h"

class ID3v1StringHandler : public TagLib::ID3v1::StringHandler
{
public:
    ID3v1StringHandler();
    void setCodec(QTextCodec *codec);
    TagLib::String parse(const TagLib::ByteVector &data) const;
    TagLib::ByteVector render(const TagLib::String &s) const;
private:
    QTextCodec *codec;
};

#endif // ID3V1STRINGHANDLER_H

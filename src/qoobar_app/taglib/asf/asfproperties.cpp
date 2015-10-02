/**************************************************************************
    copyright            : (C) 2005-2007 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <toolkit/tdebug.h>
#include <toolkit/tstring.h>
#include "asfproperties.h"

using namespace TagLib;

class ASF::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate(): 
    length(0), 
    bitrate(0), 
    sampleRate(0), 
    channels(0), 
    bitsPerSample(0),
    codec(ASF::Properties::Unknown),
    encrypted(false) {}

  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int bitsPerSample;
  ASF::Properties::Codec codec;
  String codecName;
  String codecDescription;
  bool encrypted;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ASF::Properties::Properties() : AudioProperties(AudioProperties::Average)
{
  d = new PropertiesPrivate;
}

ASF::Properties::~Properties()
{
    delete d;
}

int ASF::Properties::length() const
{
  return d->length / 1000;
}

int ASF::Properties::bitrate() const
{
  return d->bitrate;
}

int ASF::Properties::sampleRate() const
{
  return d->sampleRate;
}

int ASF::Properties::channels() const
{
  return d->channels;
}

int ASF::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

ASF::Properties::Codec ASF::Properties::codec() const
{
  return d->codec;
}

String ASF::Properties::codecName() const
{
  return d->codecName;
}

String ASF::Properties::codecDescription() const
{
  return d->codecDescription;
}

bool ASF::Properties::isEncrypted() const
{
  return d->encrypted;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void ASF::Properties::setLength(int length)
{
  //d->length = length;
}

void ASF::Properties::setLengthInMilliseconds(int value)
{
  d->length = value;
}

void ASF::Properties::setBitrate(int length)
{
  d->bitrate = length;
}

void ASF::Properties::setSampleRate(int length)
{
  d->sampleRate = length;
}

void ASF::Properties::setChannels(int length)
{
  d->channels = length;
}

void ASF::Properties::setBitsPerSample(int value)
{
  d->bitsPerSample = value;
}

void ASF::Properties::setCodec(int value)
{
  switch(value)
  {
  case 0x0160:
    d->codec = WMA1;
    break;
  case 0x0161:
    d->codec = WMA2;
    break;
  case 0x0162:
    d->codec = WMA9Pro;
    break;
  case 0x0163:
    d->codec = WMA9Lossless;
    break;
  default:
    d->codec = Unknown;
    break;
  }
}

void ASF::Properties::setCodecName(const String &value)
{
  d->codecName = value;
}

void ASF::Properties::setCodecDescription(const String &value)
{
  d->codecDescription = value;
}

void ASF::Properties::setEncrypted(bool encrypted)
{
  d->encrypted = encrypted;
}


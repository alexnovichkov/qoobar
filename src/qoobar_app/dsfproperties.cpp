/***************************************************************************
    copyright            : (C) 2013 by Stephen F. Booth
    email                : me@sbooth.org
 ***************************************************************************/

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

#include <taglib/tstring.h>
//#include <taglib/tdebug.h>

#include "dsfproperties.h"
#include "dsffile.h"

using namespace TagLib;



class DSF::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    formatVersion(0),
    formatID(0),
    channelType(0),
    channelNum(0),
    samplingFrequency(0),
    bitsPerSample(0),
    sampleCount(0),
    blockSizePerChannel(0),
    bitrate(0),
    length(0)
  {

  }

  // Nomenclature is from DSF file format specification
  uint formatVersion;
  uint formatID;
  uint channelType;
  uint channelNum;
  uint samplingFrequency;
  uint bitsPerSample;
  long long sampleCount;
  uint blockSizePerChannel;

  // Computed
  uint bitrate;
  uint length;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSF::Properties::Properties(const ByteVector &data) : AudioProperties(Fast)
{
  d = new PropertiesPrivate;
  read(data);
}

DSF::Properties::~Properties()
{
  delete d;
}

int DSF::Properties::length() const
{
    return d->length;
}

int DSF::Properties::lengthInMilliseconds() const
{
    return d->length * 1000;
}

int DSF::Properties::bitrate() const
{
  return d->bitrate;
}

int DSF::Properties::sampleRate() const
{
  return d->samplingFrequency;
}

int DSF::Properties::channels() const
{
  return d->channelNum;
}

// DSF specific
int DSF::Properties::formatVersion() const
{
  return d->formatVersion;
}

int DSF::Properties::formatID() const
{
  return d->formatID;
}

int DSF::Properties::channelType() const
{
  return d->channelType;
}

int DSF::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

long long DSF::Properties::sampleCount() const
{
  return d->sampleCount;
}

int DSF::Properties::blockSizePerChannel() const
{
  return d->blockSizePerChannel;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSF::Properties::read(const ByteVector &data)
{
  d->formatVersion         = data.mid(0, 4).toUInt(0, false);
  d->formatID              = data.mid(4, 4).toUInt(0, false);
  d->channelType           = data.mid(8, 4).toUInt(0, false);
  d->channelNum            = data.mid(12, 4).toUInt(0, false);
  d->samplingFrequency     = data.mid(16, 4).toUInt(0, false);
  d->bitsPerSample         = data.mid(20, 4).toUInt(0, false);
  d->sampleCount           = data.mid(24, 8).toLongLong(0, false);
  d->blockSizePerChannel   = data.mid(32, 4).toUInt(0, false);

  d->bitrate               = (d->samplingFrequency * d->bitsPerSample * d->channelNum) / 1000.0;
  d->length                = d->samplingFrequency > 0 ? d->sampleCount / d->samplingFrequency : 0;
}


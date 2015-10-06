/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#include "tfile.h"
#include "tfilestream.h"
#include "tstring.h"
#include "toolkit/tdebug.h"
#include "toolkit/tpropertymap.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
# include <wchar.h>
# include <windows.h>
# include <io.h>
# define ftruncate _chsize
#else
# include <unistd.h>
#endif

#include <stdlib.h>

#ifndef R_OK
# define R_OK 4
#endif
#ifndef W_OK
# define W_OK 2
#endif

#include "asf/asffile.h"
#include "mpeg/mpegfile.h"
#include "ogg/vorbis/vorbisfile.h"
#include "flac/flacfile.h"
#include "ogg/flac/oggflacfile.h"
#include "mpc/mpcfile.h"
#include "mp4/mp4file.h"
#include "wavpack/wavpackfile.h"
#include "ogg/speex/speexfile.h"
#include "trueaudio/trueaudiofile.h"
#include "riff/aiff/aifffile.h"
#include "riff/wav/wavfile.h"
#include "ape/apefile.h"

using namespace TagLib;

namespace
{
#ifdef _WIN32
  const TagLib::uint BufferSize = 8192;
#else
  const TagLib::uint BufferSize = 1024;
#endif
}

class File::FilePrivate
{
public:
  FilePrivate(IOStream *stream, bool owner);

  IOStream *stream;
  bool streamOwner;
  bool valid;
};

File::FilePrivate::FilePrivate(IOStream *stream, bool owner) :
  stream(stream),
  streamOwner(owner),
  valid(true)
{
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

File::File(FileName fileName)
{
  IOStream *stream = new FileStream(fileName);
  d = new FilePrivate(stream, true);
}

File::File(IOStream *stream)
{
  d = new FilePrivate(stream, false);
}

File::~File()
{
  if(d->stream && d->streamOwner)
    delete d->stream;
  delete d;
}

FileName File::name() const
{
  return d->stream->name();
}

ByteVector File::readBlock(ulong length)
{
  return d->stream->readBlock(length);
}

void File::writeBlock(const ByteVector &data)
{
  d->stream->writeBlock(data);
}

long File::find(const ByteVector &pattern, long fromOffset, const ByteVector &before)
{
  if(!d->stream || pattern.size() > bufferSize())
      return -1;

  // The position in the file that the current buffer starts at.

  long bufferOffset = fromOffset;
  ByteVector buffer;

  // These variables are used to keep track of a partial match that happens at
  // the end of a buffer.

  int previousPartialMatch = -1;
  int beforePreviousPartialMatch = -1;

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.

  long originalPosition = tell();

  // Start the search at the offset.

  seek(fromOffset);

  // This loop is the crux of the find method.  There are three cases that we
  // want to account for:
  //
  // (1) The previously searched buffer contained a partial match of the search
  // pattern and we want to see if the next one starts with the remainder of
  // that pattern.
  //
  // (2) The search pattern is wholly contained within the current buffer.
  //
  // (3) The current buffer ends with a partial match of the pattern.  We will
  // note this for use in the next itteration, where we will check for the rest
  // of the pattern.
  //
  // All three of these are done in two steps.  First we check for the pattern
  // and do things appropriately if a match (or partial match) is found.  We
  // then check for "before".  The order is important because it gives priority
  // to "real" matches.

  for(buffer = readBlock(bufferSize()); buffer.size() > 0; buffer = readBlock(bufferSize())) {

    // (1) previous partial match

    if(previousPartialMatch >= 0 && int(bufferSize()) > previousPartialMatch) {
      const int patternOffset = (bufferSize() - previousPartialMatch);
      if(buffer.containsAt(pattern, 0, patternOffset)) {
        seek(originalPosition);
        return bufferOffset - bufferSize() + previousPartialMatch;
      }
    }

    if(!before.isNull() && beforePreviousPartialMatch >= 0 && int(bufferSize()) > beforePreviousPartialMatch) {
      const int beforeOffset = (bufferSize() - beforePreviousPartialMatch);
      if(buffer.containsAt(before, 0, beforeOffset)) {
        seek(originalPosition);
        return -1;
      }
    }

    // (2) pattern contained in current buffer

    long location = buffer.find(pattern);
    if(location >= 0) {
      seek(originalPosition);
      return bufferOffset + location;
    }

    if(!before.isNull() && buffer.find(before) >= 0) {
      seek(originalPosition);
      return -1;
    }

    // (3) partial match

    previousPartialMatch = buffer.endsWithPartialMatch(pattern);

    if(!before.isNull())
      beforePreviousPartialMatch = buffer.endsWithPartialMatch(before);

    bufferOffset += bufferSize();
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();

  seek(originalPosition);

  return -1;
}


long File::rfind(const ByteVector &pattern, long fromOffset, const ByteVector &before)
{
  if(!d->stream || pattern.size() > bufferSize())
      return -1;

  // The position in the file that the current buffer starts at.

  ByteVector buffer;

  // These variables are used to keep track of a partial match that happens at
  // the end of a buffer.

  /*
  int previousPartialMatch = -1;
  int beforePreviousPartialMatch = -1;
  */

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.

  long originalPosition = tell();

  // Start the search at the offset.

  long bufferOffset;
  if(fromOffset == 0) {
    seek(-1 * int(bufferSize()), End);
    bufferOffset = tell();
  }
  else {
    seek(fromOffset + -1 * int(bufferSize()), Beginning);
    bufferOffset = tell();
  }

  // See the notes in find() for an explanation of this algorithm.

  for(buffer = readBlock(bufferSize()); buffer.size() > 0; buffer = readBlock(bufferSize())) {

    // TODO: (1) previous partial match

    // (2) pattern contained in current buffer

    long location = buffer.rfind(pattern);
    if(location >= 0) {
      seek(originalPosition);
      return bufferOffset + location;
    }

    if(!before.isNull() && buffer.find(before) >= 0) {
      seek(originalPosition);
      return -1;
    }

    // TODO: (3) partial match

    bufferOffset -= bufferSize();
    seek(bufferOffset);
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();

  seek(originalPosition);

  return -1;
}

void File::insert(const ByteVector &data, ulong start, ulong replace)
{
  d->stream->insert(data, start, replace);
}

void File::removeBlock(ulong start, ulong length)
{
  d->stream->removeBlock(start, length);
}

bool File::readOnly() const
{
  return d->stream->readOnly();
}

bool File::isOpen() const
{
  return d->stream->isOpen();
}

bool File::isValid() const
{
  return isOpen() && d->valid;
}

void File::seek(long offset, Position p)
{
  d->stream->seek(offset, IOStream::Position(p));
}

void File::truncate(long length)
{
  d->stream->truncate(length);
}

void File::clear()
{
  d->stream->clear();
}

long File::tell() const
{
  return d->stream->tell();
}

long File::length()
{
  return d->stream->length();
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

TagLib::uint File::bufferSize()
{
  return BufferSize;
}

void File::setValid(bool valid)
{
  d->valid = valid;
}


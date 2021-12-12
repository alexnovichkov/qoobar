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

#include <taglib/tbytevector.h>
//#include <toolkit/tdebug.h>
#include <QtDebug>
#include <taglib/id3v2tag.h>
#include <taglib/tstringlist.h>
#include <taglib/tpropertymap.h>

#include "dsffile.h"

using namespace TagLib;

// The DSF specification is located at http://dsd-guide.com/sites/default/files/white-papers/DSFFileFormatSpec_E.pdf

class DSF::File::FilePrivate
{
public:
  FilePrivate() :
    properties(0),
    tag(0)
  {

  }

  ~FilePrivate()
  {
    delete properties;
    delete tag;
  }

  long long fileSize;
  long long metadataOffset;
  DSF::Properties *properties;
  ID3v2::Tag *tag;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSF::File::File(FileName file, bool readProperties) : TagLib::File(file)
{
  d = new FilePrivate;
  if(isOpen())
    read(readProperties);
}

DSF::File::~File()
{
  delete d;
}

ID3v2::Tag *DSF::File::tag() const
{
  return d->tag;
}

PropertyMap DSF::File::properties() const
{
  return d->tag->properties();
}

PropertyMap DSF::File::setProperties(const PropertyMap &properties)
{
  return d->tag->setProperties(properties);
}

DSF::Properties *DSF::File::audioProperties() const
{
  return d->properties;
}

bool DSF::File::save()
{
  if(readOnly()) {
    qDebug("DSF::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    qDebug("DSF::File::save() -- Trying to save invalid file.");
    return false;
  }

  // Three things must be updated: the file size, the tag data, and the metadata offset

  ByteVector tagData = d->tag->render();

  long long metadataLocation = d->metadataOffset ? d->metadataOffset : d->fileSize;
  long long newFileSize = metadataLocation + tagData.size();
  long long newMetadataOffset = tagData.size() ? metadataLocation : 0;
  long long oldTagSize = d->fileSize - metadataLocation;

  // Update the file size
  if(d->fileSize != newFileSize) {
    insert(ByteVector::fromLongLong(newFileSize, false), 12, 8);
    d->fileSize = newFileSize;
  }

  // Update the metadata offset
  if(d->metadataOffset != newMetadataOffset) {
    insert(ByteVector::fromLongLong(newMetadataOffset, false), 20, 8);
    d->metadataOffset = newMetadataOffset;
  }

  // Delete the old tag and write the new one
  insert(tagData, metadataLocation, oldTagSize);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSF::File::read(bool readProperties)
{
    (void)readProperties;
  // A DSF file consists of four chunks: DSD chunk, format chunk, data chunk, and metadata chunk
  // The file format is not chunked in the sense of a RIFF File, though

  // DSD chunk
  ByteVector chunkName = readBlock(4);
  if(chunkName != "DSD ") {
    qDebug("DSF::File::read() -- Not a DSF file.");
    return;
  }

  long long chunkSize = readBlock(8).toLongLong(0, false);

  // Integrity check
  if(28 != chunkSize) {
    qDebug("DSF::File::read() -- File is corrupted.");
    return;
  }

  d->fileSize = readBlock(8).toLongLong(0, false);
  d->metadataOffset = readBlock(8).toUInt(0, false);

  // File is malformed or corrupted
  if(d->metadataOffset > d->fileSize) {
    qDebug("DSF::File::read() -- Invalid metadata offset.");
    return;
  }

  // Format chunk
  chunkName = readBlock(4);
  if(chunkName != "fmt ") {
    qDebug("DSF::File::read() -- Missing 'fmt ' chunk.");
    return;
  }

  chunkSize = readBlock(8).toLongLong(0, false);

  d->properties = new Properties(readBlock(chunkSize));

  // Skip the data chunk

  // A metadata offset of 0 indicates the absence of an ID3v2 tag
  if(0 == d->metadataOffset)
    d->tag = new ID3v2::Tag();
  else
    d->tag = new ID3v2::Tag(this, d->metadataOffset);
}

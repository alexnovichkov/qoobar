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

#ifndef DSFPROPERTIES_H
#define DSFPROPERTIES_H

#include "taglib/audioproperties.h"
#include "taglib/tbytevector.h"

namespace TagLib {

  namespace DSF {

    class File;

    //! An implementation of audio property reading for DSF

    /*!
     * This reads the data from a DSF stream found in the AudioProperties
     * API.
     */

    class Properties : public AudioProperties
    {
    public:
      /*!
       * Create an instance of DSF::AudioProperties with the data read from the
       * ByteVector \a data.
       */
      Properties(const ByteVector &data);

      /*!
       * Destroys this DSF::AudioProperties instance.
        */
      virtual ~Properties();

      // Reimplementations.

      virtual int length() const;
      virtual int lengthInMilliseconds() const;
      virtual int bitrate() const;
      virtual int sampleRate() const;
      virtual int channels() const;

      int formatVersion() const;
      int formatID() const;
      int channelType() const;
      int bitsPerSample() const;
      long long sampleCount() const;
      int blockSizePerChannel() const;

    private:
      Properties(const Properties &);
      Properties &operator=(const Properties &);

      void read(const ByteVector &data);

      class PropertiesPrivate;
      PropertiesPrivate *d;
    };
  }
}


#endif // DSFPROPERTIES_H

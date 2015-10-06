/***************************************************************************
    copyright            : (C) 2008 by Scott Wheeler
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

#ifndef TAGLIB_WAVPROPERTIES_H
#define TAGLIB_WAVPROPERTIES_H

#include "toolkit/taglib.h"
#include "audioproperties.h"

namespace TagLib {

  class ByteVector;

  namespace RIFF {

    namespace WAV {

      class File;

      //! An implementation of audio property reading for WAV

      /*!
       * This reads the data from an WAV stream found in the AudioProperties
       * API.
       */

      class Properties : public AudioProperties
      {
      public:
        Properties(File *file);
        /*!
         * Destroys this WAV::Properties instance.
         */
        virtual ~Properties();

        // Reimplementations.

        virtual int length() const;
        virtual int lengthInMilliseconds() const;
        virtual int bitrate() const;
        virtual int sampleRate() const;
        virtual int channels() const;

        int sampleWidth() const;
        uint sampleFrames() const;

        /*!
         * Returns the format ID of the file.
         * 0 for unknown, 1 for PCM, 2 for ADPCM, 3 for 32/64-bit IEEE754, and
         * so forth.
         *
         * \note For further information, refer to the WAVE Form Registration
         * Numbers in RFC 2361.
         */
        int format() const;
      private:
        Properties(const Properties &);
        Properties &operator=(const Properties &);

        void read(File *file);

        class PropertiesPrivate;
        PropertiesPrivate *d;
      };
    }
  }
}

#endif

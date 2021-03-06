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

#ifndef TAGLIB_AIFFFILE_H
#define TAGLIB_AIFFFILE_H

#include "riff/rifffile.h"
#include "mpeg/id3v2/id3v2tag.h"
#include "aiffproperties.h"

namespace TagLib {

  namespace RIFF {

    //! An implementation of AIFF metadata

    /*!
     * This is implementation of AIFF metadata.
     *
     * This supports an ID3v2 tag as well as reading stream from the ID3 RIFF
     * chunk as well as properties from the file.
     */

    namespace AIFF {

      //! An implementation of TagLib::File with AIFF specific methods

      /*!
       * This implements and provides an interface for AIFF files to the
       * TagLib::Tag and TagLib::AudioProperties interfaces by way of implementing
       * the abstract TagLib::File API as well as providing some additional
       * information specific to AIFF files.
       */

      class File : public TagLib::RIFF::File
      {
      public:
        /*!
         * Contructs an AIFF file from \a file.  If \a readProperties is true the
         * file's audio properties will also be read using \a propertiesStyle.  If
         * false, \a propertiesStyle is ignored.
         */
        File(FileName file, bool readProperties = true);

        /*!
         * Destroys this instance of the File.
         */
        virtual ~File();

        /*!
         * Returns the Tag for this file.
         */
        virtual ID3v2::Tag *tag() const;
#ifndef QOOBAR_NO_PROPERTY_MAPS
        /*!
         * Implements the unified property interface -- export function.
         * This method forwards to ID3v2::Tag::properties().
         */
        PropertyMap properties() const;

        /*!
         * Implements the unified property interface -- import function.
         * This method forwards to ID3v2::Tag::setProperties().
         */
        PropertyMap setProperties(const PropertyMap &);
#endif
        /*!
         * Returns the AIFF::Properties for this file.  If no audio properties
         * were read then this will return a null pointer.
         */
        virtual Properties *audioProperties() const;

        /*!
         * Saves the file.
         */
        virtual bool save();
        /*!
         * Returns whether or not the file on disk actually has an ID3v2 tag.
         *
         * \see ID3v2Tag()
         */
        bool hasID3v2Tag() const;

      private:
        File(const File &);
        File &operator=(const File &);

        void read(bool readProperties);
        friend class Properties;
        class FilePrivate;
        FilePrivate *d;
      };
    }
  }
}

#endif

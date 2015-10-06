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

#ifndef TAGLIB_VORBISFILE_H
#define TAGLIB_VORBISFILE_H

#include "ogg/oggfile.h"
#include "ogg/xiphcomment.h"

#include "vorbisproperties.h"

namespace TagLib {

/*
 * This is just to make this appear to be in the Ogg namespace in the
 * documentation.  The typedef below will make this work with the current code.
 * In the next BIC version of TagLib this will be really moved into the Ogg
 * namespace.
 */

  //! A namespace containing classes for Vorbis metadata

  namespace Vorbis {


    //! An implementation of Ogg::File with Vorbis specific methods

    /*!
     * This is the central class in the Ogg Vorbis metadata processing collection
     * of classes.  It's built upon Ogg::File which handles processing of the Ogg
     * logical bitstream and breaking it down into pages which are handled by
     * the codec implementations, in this case Vorbis specifically.
     */

    class File : public Ogg::File
    {
    public:
      /*!
       * Contructs a Vorbis file from \a file.  If \a readProperties is true the
       * file's audio properties will also be read using \a propertiesStyle.  If
       * false, \a propertiesStyle is ignored.
       */
      File(FileName file, bool readProperties = true);

      /*!
       * Destroys this instance of the File.
       */
      virtual ~File();

      /*!
       * Returns the XiphComment for this file.  XiphComment implements the tag
       * interface, so this serves as the reimplementation of
       * TagLib::File::tag().
       */
      virtual Ogg::XiphComment *tag() const;

#ifndef QOOBAR_NO_PROPERTY_MAPS
      /*!
       * Implements the unified property interface -- export function.
       * This forwards directly to XiphComment::properties().
       */
      PropertyMap properties() const;

      /*!
       * Implements the unified tag dictionary interface -- import function.
       * Like properties(), this is a forwarder to the file's XiphComment.
       */
      PropertyMap setProperties(const PropertyMap &);
#endif
      /*!
       * Returns the Vorbis::Properties for this file.  If no audio properties
       * were read then this will return a null pointer.
       */
      virtual Properties *audioProperties() const;

      virtual bool save();

    private:
      File(const File &);
      File &operator=(const File &);

      void read(bool readProperties);

      class FilePrivate;
      FilePrivate *d;
    };
  }

  namespace Ogg { namespace Vorbis { typedef TagLib::Vorbis::File File; } }
}

#endif

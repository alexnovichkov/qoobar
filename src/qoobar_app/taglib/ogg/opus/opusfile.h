/***************************************************************************
    copyright            : (C) 2012 by Lukas Lalinsky
    email                : lalinsky@gmail.com

    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
                           (original Vorbis implementation)
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

#ifndef TAGLIB_OPUSFILE_H
#define TAGLIB_OPUSFILE_H

#include "ogg/oggfile.h"
#include "ogg/xiphcomment.h"

#include "opusproperties.h"
namespace TagLib {
  namespace Ogg {
    //! A namespace containing classes for Opus metadata
    namespace Opus {
      class File : public Ogg::File
      {
      public:
        File(FileName file, bool readProperties = true);
        virtual ~File();
        virtual Ogg::XiphComment *tag() const;
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
  }
}
#endif

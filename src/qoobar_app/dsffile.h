#ifndef DSFFILE_H
#define DSFFILE_H

#include "taglib/tfile.h"
#include "taglib/id3v2tag.h"
#include "dsfproperties.h"

namespace TagLib {

    //! An implementation of DSF metadata

    /*!
     * This is implementation of DSF metadata.
     *
     * This supports an ID3v2 tag as well as properties from the file.
     */

  namespace DSF {

   //! An implementation of TagLib::File with DSF specific methods

   /*!
     * This implements and provides an interface for DSF files to the
     * TagLib::Tag and TagLib::AudioProperties interfaces by way of implementing
     * the abstract TagLib::File API as well as providing some additional
     * information specific to DSF files.
     */

    class File : public TagLib::File
    {
    public:
      /*!
       * Contructs an DSF file from \a file.  If \a readProperties is true the
       * file's audio properties will also be read using \a propertiesStyle.  If
       * false, \a propertiesStyle is ignored.
       */
      explicit File(FileName file, bool readProperties = true);

      /*!
       * Destroys this instance of the File.
       */
      virtual ~File();

      /*!
       * Returns the Tag for this file.
       */
      virtual ID3v2::Tag *tag() const;

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

      /*!
       * Returns the DSF::AudioProperties for this file.  If no audio properties
       * were read then this will return a null pointer.
       */
      virtual Properties *audioProperties() const;

      /*!
       * Saves the file.
       */
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

#endif // DSFFILE_H

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

#ifndef TAGLIB_AUDIOPROPERTIES_H
#define TAGLIB_AUDIOPROPERTIES_H


namespace TagLib {

  //! A simple, abstract interface to common audio properties

  /*!
   * The values here are common to most audio formats.  For more specific, codec
   * dependant values, please see see the subclasses APIs.  This is meant to
   * compliment the TagLib::File and TagLib::Tag APIs in providing a simple
   * interface that is sufficient for most applications.
   */

  class AudioProperties
  {
  public:

    /*!
     * Destroys this AudioProperties instance.
     */
    virtual ~AudioProperties();

    /*!
     * Returns the length of the file in seconds.
     */
    virtual int length() const = 0;

    /*!
     * \brief lengthInMilliseconds
     * \return the length of the file in milliseconds.
     */
    virtual int lengthInMilliseconds() const = 0;

    /*!
     * Returns the most appropriate bit rate for the file in kb/s.  For constant
     * bitrate formats this is simply the bitrate of the file.  For variable
     * bitrate formats this is either the average or nominal bitrate.
     */
    virtual int bitrate() const = 0;

    /*!
     * Returns the sample rate in Hz.
     */
    virtual int sampleRate() const = 0;

    /*!
     * Returns the number of audio channels.
     */
    virtual int channels() const = 0;

  protected:

    /*!
     * Construct an audio properties instance.  This is protected as this class
     * should not be instantiated directly, but should be instantiated via its
     * subclasses and can be fetched from the FileRef or File APIs.
     *
     * \see ReadStyle
     */
    AudioProperties();

  private:
    AudioProperties(const AudioProperties &);
    AudioProperties &operator=(const AudioProperties &);

    class AudioPropertiesPrivate;
    AudioPropertiesPrivate *d;
  };

}

#endif

/**
 * \file discidhelper.cpp
 * A helper class to use libdiscid.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 11 Aug 2011
 *
 * Copyright (C) 2011  Alex Novichkov
 *
 * This file is part of Qoobar.
 *
 * Qoobar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Qoobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "discidhelper.h"
#include "application.h"
#include <QLibrary>
#include <QtDebug>

#ifdef Q_OS_MAC
#include "discid.h"
#endif

#include "enums.h"

DiscIDHelper::DiscIDHelper()
{
#ifdef Q_OS_MAC
    libdiscidResolved = true;
#else
    libdiscidResolved = resolveLibdiscid();
    if (!libdiscidResolved)
        errorString = tr("Please install libdiscid");
#endif
}

#ifndef Q_OS_MAC
bool DiscIDHelper::resolveLibdiscid()
{
    QString libName=App->discidLibraryPath;
    discid_new = (Discid_new)QLibrary::resolve(libName,"discid_new");
    discid_free = (Discid_free)QLibrary::resolve(libName,"discid_free");

    discid_get_freedb_id = (Discid_get_freedb_id)QLibrary::resolve(libName,"discid_get_freedb_id");
    discid_get_id = (Discid_get_id)QLibrary::resolve(libName,"discid_get_id");
    discid_read = (Discid_read)QLibrary::resolve(libName,"discid_read");
    discid_get_first_track_num = (Discid_get_first_track_num)QLibrary::resolve(libName,"discid_get_first_track_num");
    discid_get_last_track_num = (Discid_get_last_track_num)QLibrary::resolve(libName,"discid_get_last_track_num");
    discid_get_sectors = (Discid_get_sectors)QLibrary::resolve(libName,"discid_get_sectors");
    discid_get_track_offset = (Discid_get_track_offset)QLibrary::resolve(libName,"discid_get_track_offset");
    discid_put = (Discid_put)QLibrary::resolve(libName,"discid_put");

    return (discid_new && discid_free);
}
#endif

int * computeTrackFrameOffsets(const QVector<int> &tracksLengths, const int size)
{
    int *offsets = new int[100];

    offsets[1]=150;//by default the first track has offset 150

    for (int i=0; i<size-1; ++i)
        offsets[i+2]=offsets[i+1]+tracksLengths.at(i)*75;
    offsets[0]=offsets[size]+tracksLengths.at(size-1)*75;

    return offsets;
}

QString DiscIDHelper::getDiscID(const QVector<int> &lengths,int type)
{
    //type = 0 - freedb
    //type = 1 - musicbrainz
    QString q;
    if (!libdiscidResolved) return q;

    errorString.clear();

    DiscId *disc = discid_new();
    if (!disc) return q;

    int tracksCount=0;
    int discLength=0;
    int firstTrack=0;
    int lastTrack=0;
    QStringList offsetsInFrames;

    if (lengths.isEmpty()) {//get discID by CD
        int disc_read_result=0;
        if (App->cdromDevice.isEmpty())
            disc_read_result=discid_read(disc, NULL);
        else
            disc_read_result=discid_read(disc, App->cdromDevice.toLatin1().data());
        if (disc_read_result==0) {
            errorString = tr("Cannot read CD");
            discid_free(disc);
            return q;
        }

        firstTrack = discid_get_first_track_num(disc);
        lastTrack = discid_get_last_track_num(disc);
        tracksCount = lastTrack-firstTrack+1;

        discLength = discid_get_sectors(disc);
        for (int i=firstTrack; i<=lastTrack; ++i)
            offsetsInFrames << QString::number(discid_get_track_offset(disc, i));
    }
    else {//get discid by files
        int size=qMin(lengths.size(),99);
        int *offsets = computeTrackFrameOffsets(lengths,size);

        discid_put(disc, 1, size, offsets);

        firstTrack = 1;
        lastTrack = size;
        tracksCount = lastTrack;
        discLength = offsets[0]; //int discLength = int((offsets[0]-150)/75);

        for (int i=0; i<size;++i)
            offsetsInFrames << QString::number(offsets[i+1]);
        delete [] offsets;
    }
    if (type==1)
        q=QString("%1?toc=%2+%3+%4+%5").arg(discid_get_id(disc))
            .arg(firstTrack)
            .arg(lastTrack)
            .arg(discLength)
            .arg(offsetsInFrames.join(QSL("+")));
    else q=QString("%1+%2+%3+%4")
            .arg(discid_get_freedb_id(disc))
            .arg(tracksCount)
            .arg(offsetsInFrames.join(QSL("+")))
            .arg(discLength/75);
    discid_free(disc);
    return q;
}



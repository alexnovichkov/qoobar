/**
 * \file discidhelper.h
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

#ifndef DISCIDHELPER_H
#define DISCIDHELPER_H

#include <QObject>

// if discid is not linked we need to resolve its symbols
//#define DISCID_LOADABLE

#ifdef DISCID_LOADABLE
typedef void *DiscId;

typedef DiscId * (*Discid_new)();
typedef void (*Discid_free)(DiscId *);
typedef int (*Discid_read)(DiscId *, const char *);
typedef int (*Discid_put)(DiscId *, int, int, int *);
typedef char * (*Discid_get_id)(DiscId *);
typedef char * (*Discid_get_freedb_id)(DiscId *);
typedef int (*Discid_get_first_track_num)(DiscId *);
typedef int (*Discid_get_last_track_num)(DiscId *);
typedef int (*Discid_get_sectors)(DiscId *);
typedef int (*Discid_get_track_offset)(DiscId *, int);
#endif

class DiscIDHelper : public QObject
{
    Q_OBJECT
public:
    DiscIDHelper();
    QString getDiscID(const QVector<int> &, int);
    bool isDiscidResolved() {return libdiscidResolved;}
    QString errorString;
private:
    bool libdiscidResolved = false;
#ifdef DISCID_LOADABLE
    bool resolveLibdiscid();
    Discid_new discid_new = nullptr;
    Discid_free discid_free = nullptr;
    Discid_get_freedb_id discid_get_freedb_id = nullptr;
    Discid_read discid_read = nullptr;
    Discid_get_first_track_num discid_get_first_track_num = nullptr;
    Discid_get_last_track_num discid_get_last_track_num = nullptr;
    Discid_get_sectors discid_get_sectors = nullptr;
    Discid_get_track_offset discid_get_track_offset = nullptr;
    Discid_put discid_put = nullptr;
    Discid_get_id discid_get_id = nullptr;
#endif
};

#endif // DISCIDHELPER_H

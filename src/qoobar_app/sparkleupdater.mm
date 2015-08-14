/**
 * Copyright (c) 2012, WebItUp <contact@webitup.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sparkleupdater.h"
#include "cocoainit.h"

#include <Cocoa/Cocoa.h>
#ifdef HAVE_QT5
#include <Sparkle.h>
#else
#include <Sparkle/Sparkle.h>
#endif
#include <QtCore/qdebug.h>

class SparkleUpdater::Private
{
	public:
		SUUpdater* updater;
};

SparkleUpdater::SparkleUpdater(QObject *parent):
    QObject(parent)
{
    CocoaInitializer initializer;
    d = new Private;

    d->updater = [SUUpdater sharedUpdater];
    [d->updater retain];

    // Unfortunately, it's not possible to specify any of the additional parameters

    // XXX these are obviously not implemented on windows, so, not a separate / activable method for now
    //- (void)setSendsSystemProfile:(BOOL)sendsSystemProfile;
    //- (BOOL)sendsSystemProfile;
    [d->updater setSendsSystemProfile: true];
    //- (void)setAutomaticallyDownloadsUpdates:(BOOL)automaticallyDownloadsUpdates;
    //- (BOOL)automaticallyDownloadsUpdates;
    [d->updater setAutomaticallyDownloadsUpdates: true];
}

SparkleUpdater::~SparkleUpdater()
{
    [d->updater release];
//    d->cinit->~CocoaInitializer();
	delete d;
}

void SparkleUpdater::checkNow(bool silent)
{
    if (silent)
        [d->updater checkForUpdatesInBackground];
    else
        [d->updater checkForUpdates: 0];
}


#include "qoobarhelp.h"

//#import <NSHelp
//#import <AppKit/NSApplication.h>
//#import <AppKit/NSImageView.h>
//#import <AppKit/NSCIImageRep.h>
//#import <AppKit/NSBezierPath.h>
//#import <AppKit/NSColor.h>
//#import <Foundation/NSString.h>

#import "Foundation/NSAutoreleasePool.h"
#import <AppKit/NSHelpManager.h>
#import <Foundation/NSBundle.h>

#include "cocoainit.h"
#include "enums.h"
#include <QtDebug>
#include <QApplication>
#include <qplatformdefs.h>

void Qoobar::showHelp(const QString &section)
{//DD;
    Q_UNUSED(section)
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSHelpManager *help = [NSHelpManager sharedHelpManager];
    NSString *locBookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    //qDebug()<<QString::fromNSString(locBookName);
   // qDebug()<<[help registerBooksInBundle:[NSBundle mainBundle]];
    if (section.isEmpty())
        [[NSApplication sharedApplication] showHelp:[NSApplication sharedApplication]];
    else {
        NSString *anchor=section.toNSString();
        [help openHelpAnchor:anchor  inBook:locBookName];
    }

    [pool release];
}

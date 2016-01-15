#include "machelper.h"

#import <AppKit/NSApplication.h>

void openCharacterPalette()
{
    [[NSApplication sharedApplication] orderFrontCharacterPalette:nil];
}

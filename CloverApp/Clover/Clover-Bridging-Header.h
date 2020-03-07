//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

#ifndef CLOVERAPPLICATION
#define CLOVERAPPLICATION 1
#endif

#ifdef DEBUG
#define DEBUG_BACKUP DEBUG // backup original
#undef DEBUG
#endif

#include "../../rEFIt_UEFI/Platform/Platform.h"

#ifdef DEBUG_BACKUP
#undef DEBUG
#define DEBUG DEBUG_BACKUP // restore original
#endif

#import "NSWindowFix.h"
#import "PNG8Image.h"
#import "gfxutil.h"
#import "efidevp.h"


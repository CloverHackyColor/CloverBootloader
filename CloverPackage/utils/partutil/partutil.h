//
//  partutil.h
//  partutil
//
//  Created by Yves Blusseau on 28/12/2014.
//  Copyright (c) 2014-2015 JrCs. All rights reserved.
//

#ifndef partutil_partutil_h
#define partutil_partutil_h

// macros
#define STRINGIFY(s) #s
#define STRINGIFY2(s) STRINGIFY(s)
#define PROGNAME_S STRINGIFY2(PROGNAME)

#define VERSION_S  STRINGIFY(0.15)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <sys/param.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOCDTypes.h>
#include <IOKit/storage/IOMediaBSDClient.h>

#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

void print_version(void);
void usage (int status);
CFDictionaryRef getDescriptionsFrom(char const* diskOrMountPoint);
CFArrayRef findEFIDisks();
char* getESPFor(char const* diskOrMountPoint);
void releaseESPArray(CFArrayRef esps);
#endif

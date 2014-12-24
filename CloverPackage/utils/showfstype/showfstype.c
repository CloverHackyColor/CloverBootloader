/*
 *  showfstype.c
 *  showfstype
 *
 *  Created by JrCs on December 24th, 2014.
 *  Base on boot1-install by Zenith432.
 *  Copyright (c) 2014-2015 JrCs. All rights reserved.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

#define ENUM_TABLE \
X(msdos),          \
X(hfs),            \
X(exfat),          \
X(ntfs),           \
X(unknown)

#define X(a)    a
typedef enum fsType {
    ENUM_TABLE
} fsType_t;
#undef X

#define X(a)    #a
static char * const fsTypeStr[] = {
    ENUM_TABLE
};
#undef X

static char const devrdisk[] = "/dev/rdisk";
static char const devdisk[] = "/dev/disk";
static __used char const copyright[] = "Copyright 2014-2015 JrCs";

#pragma mark -
#pragma mark DiskArbitration Helpers
#pragma mark -

static
char const* toBSDName(char const* pathName)
{
	assert(pathName);
	return strncmp(pathName, &devrdisk[0], 10) ? pathName : &pathName[6];
}

static
int getDASessionAndDisk(char const* pathName, DASessionRef* pSession, DADiskRef* pDisk)
{
	DASessionRef session;
	DADiskRef disk;

	assert(pathName);
	session = DASessionCreate(kCFAllocatorDefault);
	if (!session) {
		fprintf(stderr, "DASessionCreate returned NULL\n");
		return -1;
	}

	disk = DADiskCreateFromBSDName(kCFAllocatorDefault, session, toBSDName(pathName));
	if (!disk) {
		CFRelease(session);
		fprintf(stderr, "DADiskCreateFromBSDName(%s) returned NULL\n", pathName);
		return -1;
	}
	if (pDisk)
		*pDisk = disk;
	else
		CFRelease(disk);
	if (pSession)
		*pSession = session;
	else
		CFRelease(session);
	return 0;
}

static
int checkDevice(char const* pathName)
{
    struct stat buf;
    DADiskRef disk;
    CFDictionaryRef descDict;
    CFStringRef s_ref;
    CFBooleanRef b_ref;
    int result = -1;
    int isMediaWhole = 0;
    int isMediaLeaf = 0;

    assert(pathName);

    if (strncmp(pathName, &devdisk[0], 9) != 0 &&
        strncmp(pathName, &devrdisk[0], 10) != 0) {
        fprintf(stderr, "device must be of form /dev/rdiskUsS or /dev/diskUsS\n");
        return -1;
    }
    if (stat(pathName, &buf) < 0) {
        fprintf(stderr, "stat failed on %s: %s\n", pathName, strerror(errno));
        return -1;
    }
    if (!(buf.st_mode & (S_IFCHR | S_IFBLK))) {
        fprintf(stderr, "%s is not a block or character special device\n", pathName);
        return -1;
    }

	if (getDASessionAndDisk(pathName, NULL, &disk) < 0)
		return -1;

    descDict = DADiskCopyDescription(disk);
    CFRelease(disk);

	if (!descDict) {
		fprintf(stderr, "DADiskCopyDescription(%s) returned NULL\n", pathName);
		return -1;
	}
	if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionMediaWholeKey, (void const**) &b_ref) &&
		CFBooleanGetValue(b_ref))
		isMediaWhole = 1;
	if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionMediaLeafKey, (void const**) &b_ref) &&
		CFBooleanGetValue(b_ref))
		isMediaLeaf = 1;

    if (isMediaWhole && !isMediaLeaf) {
        fprintf(stderr, "%s is a whole disk\n", pathName);
    }
	else if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionVolumeKindKey, (void const**) &s_ref)) {
		static char cstr_buffer[64];
		char const* cstr = CFStringGetCStringPtr(s_ref, kCFStringEncodingUTF8);
		if (!cstr) {
			CFStringGetCString(s_ref, &cstr_buffer[0], (CFIndex) sizeof cstr_buffer, kCFStringEncodingUTF8);
			cstr = &cstr_buffer[0];
		}

        fsType_t checkFsType;
        for (checkFsType=0; checkFsType < unknown; checkFsType++) {
            if (!strcmp(cstr, fsTypeStr[checkFsType])) {
                result = 0;
                break;
            }
        }
        printf("%s\n", fsTypeStr[checkFsType]);
	}

#if 0
	CFShow(descDict);
#endif
	CFRelease(descDict);
	return result;
}

#pragma mark -
#pragma mark Usage
#pragma mark -

static
void usage(char const* self)
{
	assert(self);
	fprintf(stderr, "Usage: %s device\n", self);
	fprintf(stderr, "device is of the form /dev/rdiskUsS or /dev/diskUsS\n");
}

#pragma mark -
#pragma mark Main
#pragma mark -

int main(int argc, char* const argv[])
{
	char const* devicePath = NULL;

    if (argc != 2) {
        if (argc > 2) fprintf(stderr, "Error: too many arguments !\n");
        usage(argv[0]);
        return -1;
    }

    devicePath = argv[optind];

    return checkDevice(devicePath);
}
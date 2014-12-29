/*
 *  partutil.c
 *  partutil
 *
 *  Created by JrCs on December 28th, 2014.
 *  Copyright (c) 2014-2015 JrCs. All rights reserved.
 */

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

#include "partutil.h"

static char const diskStr[] = "disk";
static __used char const copyright[] = "Copyright 2014-2015 JrCs.";
int verbosity = 0;

typedef enum searchType {
    search_uuid=300, // must be > 255
    search_undefined
} searchType_t;
searchType_t search = search_undefined;
char const *searchValue = NULL;

typedef enum queryType {
    query_fstype=search_undefined+1,
    query_volumename,
    query_uuid,
    query_blocksize,
    query_dump,
    query_undefined
} queryType_t;
queryType_t query = query_undefined;

#pragma mark -
#pragma mark DiskArbitration Helpers
#pragma mark -

static
char const* toBSDName(char const* pathName)
{
	assert(pathName);
    char const* bsdName = pathName;
    if (strncmp(pathName, _PATH_DEV, sizeof(_PATH_DEV)-1) == 0) {
        // Remove /dev/
        bsdName=&pathName[sizeof(_PATH_DEV)-1];
    }
    if (bsdName[0] == 'r') {
        // Remove the r (raw device)
        bsdName=&bsdName[1];
    }
    // the devicename must start by the string "disk"
    if (strncmp(bsdName, diskStr, sizeof(diskStr)-1) != 0) {
        fprintf(stderr,"Error: invalid device name '%s'\n", bsdName);
        exit(EXIT_FAILURE);
    }
	return bsdName;
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
int queryDevice(char const* deviceName)
{
    struct stat buf;
    DADiskRef disk;
    CFDictionaryRef descDict;
    CFBooleanRef b_ref;
    int result = EXIT_FAILURE;

    assert(deviceName);

    char devFilePath[ MAXPATHLEN ];
    size_t maxPathSize = sizeof( devFilePath );
    char *deviceFilePath = &devFilePath[0];

    strncpy( deviceFilePath, _PATH_DEV, maxPathSize );
    strncat( deviceFilePath, deviceName, maxPathSize - strlen(deviceFilePath) - 1);

    if (stat(deviceFilePath, &buf) < 0) {
        fprintf(stderr, "Error: stat failed on %s: %s\n", deviceFilePath, strerror(errno));
        return EXIT_FAILURE;
    }
    if (!(buf.st_mode & (S_IFCHR | S_IFBLK))) {
        fprintf(stderr, "Error: %s is not a block or character special device\n", deviceFilePath);
        return EXIT_FAILURE;
    }

    if (getDASessionAndDisk(deviceName, NULL, &disk) < 0)
		return EXIT_FAILURE;

    descDict = DADiskCopyDescription(disk);
    CFRelease(disk);

	if (!descDict) {
		return EXIT_FAILURE;
	}

    // If not query the blocksize, check if the device is not a whole disk
    if (query != query_blocksize) {
        int isMediaWhole = 0;
        int isMediaLeaf = 0;
        if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionMediaWholeKey, (void const**) &b_ref) &&
            CFBooleanGetValue(b_ref))
            isMediaWhole = 1;
        if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionMediaLeafKey, (void const**) &b_ref) &&
            CFBooleanGetValue(b_ref))
            isMediaLeaf = 1;

        if (isMediaWhole && !isMediaLeaf) {
            fprintf(stderr, "Error: %s is a whole disk\n", deviceName);
            CFRelease(descDict);
            return EXIT_FAILURE;
        }
    }

    CFStringRef key = NULL;
    switch (query) {
        case query_fstype:
            key = kDADiskDescriptionVolumeKindKey;
            break;
        case query_volumename:
            key = kDADiskDescriptionVolumeNameKey;
            break;
        case query_uuid:
            key = kDADiskDescriptionMediaUUIDKey;
            break;
        case query_blocksize:
            key = kDADiskDescriptionMediaBlockSizeKey;
            break;
        case query_dump:
            CFShow(descDict);
            CFRelease(descDict);
            return EXIT_SUCCESS;
            break;
        case query_undefined:
            break;
    }
    if (!key) {
        CFRelease(descDict);
        return EXIT_FAILURE;
    }

    CFTypeRef valueRef;
	if (CFDictionaryGetValueIfPresent(descDict, key, (void const**) &valueRef)) {
        // Get the OF variable's type.
        CFTypeID typeID = CFGetTypeID(valueRef);

        CFStringRef cfstr_value = NULL;
        if (typeID == CFStringGetTypeID())
            cfstr_value = CFStringCreateCopy(NULL, valueRef); // same string
        else if (typeID == CFUUIDGetTypeID())
            cfstr_value = CFUUIDCreateString(NULL, valueRef); // convert CFUUIDRef to CFStringRef
        else if (typeID == CFNumberGetTypeID())
            cfstr_value = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@"), valueRef); // convert CFNumber to CFStringRef
        if (cfstr_value) {
            static char value_buffer[64];
            char const* value = CFStringGetCStringPtr(cfstr_value, kCFStringEncodingUTF8); // Convert to CString
            if (!value) {
                CFStringGetCString(cfstr_value, &value_buffer[0], (CFIndex) sizeof value_buffer, kCFStringEncodingUTF8);
                value = &value_buffer[0];
            }
            printf("%s\n", value);
            CFRelease(cfstr_value);
            result = EXIT_SUCCESS;
        }
    }

#if 0
	CFShow(descDict);
#endif
	CFRelease(descDict);
	return result;
}

#pragma mark -
#pragma mark Test
#pragma mark -

int doSearch()
{
    mach_port_t         masterPort;
    kern_return_t       kernResult;
    CFMutableDictionaryRef   classesToMatch;
    io_iterator_t mediaIterator;

    assert(searchValue);

    kernResult = IOMasterPort( MACH_PORT_NULL, &masterPort );
    if ( kernResult != KERN_SUCCESS ) {
        fprintf(stderr, "IOMasterPort returned %d\n", kernResult );
        return EXIT_FAILURE;
    }

    classesToMatch = IOServiceMatching( kIOMediaClass ); // All Storage Media
    if ( classesToMatch == NULL ) {
        fprintf( stderr, "IOServiceMatching returned a NULL dictionary.\n" );
        return EXIT_FAILURE;
    }
    CFDictionarySetValue( classesToMatch,
                         CFSTR( kIOMediaWholeKey ), kCFBooleanFalse ); // Not a Whole disk
    CFDictionarySetValue( classesToMatch,
                         CFSTR( kIOMediaLeafKey ), kCFBooleanTrue ); // Is a leaf device

    CFStringRef searchCFString = CFStringCreateWithCString(NULL, searchValue, kCFStringEncodingASCII);
    switch (search) {
        case search_uuid:
            if (strlen(searchValue) != 36 || searchValue[8] != '-' ||
                searchValue[13] != '-' || searchValue[18] != '-' || searchValue[23] != '-') {
                fprintf(stderr, "Error: invalid UUID value '%s'\n", searchValue);
                CFRelease(searchCFString);
                CFRelease(classesToMatch); // Not use so release it
                return EXIT_FAILURE;
            }
            CFDictionarySetValue( classesToMatch, CFSTR( kIOMediaUUIDKey ), searchCFString);
            break;

        case search_undefined:
            CFRelease(searchCFString);
            CFRelease(classesToMatch); // Not use so release it
            return EXIT_FAILURE;
            break;
    }
    CFRelease(searchCFString);

    // classesToMatch is consume by IOServiceGetMatchingServices
    kernResult = IOServiceGetMatchingServices( masterPort,
                                               classesToMatch, &mediaIterator );

    if ( kernResult != KERN_SUCCESS ) {
        fprintf(stderr, "No Block Storage media found.\n kernResult = %d\n", kernResult );
        return EXIT_FAILURE;
    }

    io_service_t service;
    CFMutableDictionaryRef	serviceProperties = NULL;

    while ((service = IOIteratorNext( mediaIterator ))) {

        kernResult = IORegistryEntryCreateCFProperties(service, &serviceProperties, NULL, 0);
        if ( kernResult != KERN_SUCCESS ) {
            fprintf(stderr, "Error: IORegistryEntryCreateCFProperties returned %d\n", kernResult );
            IOObjectRelease( service );
            return EXIT_FAILURE;
        }

        CFStringRef s_ref;
        // get the BSO device name
        if (CFDictionaryGetValueIfPresent(serviceProperties, CFSTR( kIOBSDNameKey ), (void const**) &s_ref)) {
            static char bsd_name_buffer[64];
            char const* bsd_name = CFStringGetCStringPtr(s_ref, kCFStringEncodingASCII);
            if (!bsd_name) {
                CFStringGetCString(s_ref, &bsd_name_buffer[0], (CFIndex) sizeof bsd_name_buffer, kCFStringEncodingUTF8);
                bsd_name = &bsd_name_buffer[0];
            }
            printf( "%s\n", bsd_name );
            CFRelease(serviceProperties);
            IOObjectRelease( service );
            return EXIT_SUCCESS;
        }

        CFRelease(serviceProperties);
        IOObjectRelease( service );
    }
    return EXIT_FAILURE;
}

#pragma mark -
#pragma mark Usage
#pragma mark -

//
// display version string
//
void print_version(void) {
    printf ("%s " VERSION_S "\n%s\n", PROGNAME_S, copyright);
}

//
// display usage
//
static void
usage (int status)
{
    if (status)
        fprintf (stderr, "Try ``" PROGNAME_S " --help'' for more information.\n");
    else {
        print_version();
        printf ("\n\
Usage: " PROGNAME_S " [QUERY OPTION] DEVICE\n\
       " PROGNAME_S " [SEARCH OPTION]\n\
\n\
Query options:\n\
\t--show-fstype           display the filesytem type of the partition\n\
\t--show-volumename       display the volume name of the partition\n\
\t--show-uuid             display the UUID of the partition\n\
\t--show-blocksize        display the prefer blocksize of the partition\n\
\t--dump                  dump properties of the partition\n\
\n\
Search options: \n\
\t--search-uuid UUID      search a partition with the specific UUID\n\
\n\
Other options:\n\
\t-h, --help              display this message and exit\n\
\t-V, --version           print version information and exit\n\
\t-v, --verbose           print verbose messages\n\
\n\
example: " PROGNAME_S " --show-fstype disk0s4\n\
         " PROGNAME_S " --search-uuid 2C97F84A-F488-4917-A312-5D64BAE5BCFC\n");
    }
    exit (status);
}


#pragma mark -
#pragma mark Main
#pragma mark -

// Options
static struct option options[] =
{
    {"search-uuid", required_argument, 0, search_uuid},
    {"show-fstype", no_argument, 0, query_fstype},
    {"show-volumename", no_argument, 0, query_volumename},
    {"show-uuid", no_argument, 0, query_uuid},
    {"show-blocksize", no_argument, 0, query_blocksize},
    {"dump", no_argument, 0, query_dump},
    {"help",    no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

void define_search(searchType_t search_option, const char* value) {
    if (search != search_undefined) {
        fprintf(stderr, "Error: only one search can be done !\n");
        exit(EXIT_FAILURE);
    }
    if (query != query_undefined) {
        fprintf(stderr, "Error: can't mix query and search !\n");
        exit(EXIT_FAILURE);
    }
    search = search_option;
    searchValue = value;
}

void define_query(queryType_t query_option) {
    if (query != query_undefined) {
        fprintf(stderr, "Error: only one query can be done !\n");
        exit(EXIT_FAILURE);
    }
    if (search != search_undefined) {
        fprintf(stderr, "Error: can't mix query and search !\n");
        exit(EXIT_FAILURE);
    }
    query = query_option;
}

int main(int argc, char* const argv[])
{
    //test();

    /* Check for options.  */
    while (1) {
        int c = getopt_long (argc, argv, "d:r:hVv", options, 0);
        if (c == -1)
            break;
        else
            switch (c) {
                case search_uuid:
                    define_search(c, optarg);
                    break;

                case query_fstype:
                case query_volumename:
                case query_uuid:
                case query_blocksize:
                case query_dump:
                    define_query(c);
                    break;

                case 'h':
                    usage (0);
                    break;

                case 'V':
                    print_version();
                    return 0;
                    
                case 'v':
                    verbosity++;
                    break;
                    
                default:
                    usage (1);
                    break;
            }
    }
    
    argc -= optind;

    /* check arguments.  */
    if (query == query_undefined && search == search_undefined)
        usage(EXIT_SUCCESS);

    if (query != query_undefined && argc < 1) {
        fprintf (stderr, "Error: you must specify a device for the query !\n");
        usage(EXIT_FAILURE);
    }

    if ( (query  != query_undefined  && argc != 1) ||
         (search != search_undefined && argc != 0)) {
        fprintf(stderr, "Error: too many arguments !\n");
        usage(EXIT_FAILURE);
    }

    if (query != query_undefined) {
        char const* device = toBSDName(argv[optind++]);
        return queryDevice(device);
    }
    else
        return doSearch();
}
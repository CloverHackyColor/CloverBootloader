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

typedef enum {
    search_uuid=300, // must be > 255
    search_undefined
} searchType;

typedef enum {
    query_fstype=search_undefined+1,
    query_bsdname,
    query_mountpoint,
    query_volumename,
    query_uuid,
    query_blocksize,
    query_partitionscheme,
    query_pbrtype,
    query_wholedisk,
    query_contenttype,
    query_dump,
    query_undefined
} queryType;

static char const fat16ID[] = "FAT16   ";
static char const fat32ID[] = "FAT32   ";
static char const exfatID[] = "EXFAT   ";

static char const fat16[] = "fat16";
static char const fat32[] = "fat32";
static char const msdos[] = "msdos";
static char const hfs[]   = "hfs";
static char const exfat[] = "exfat";


#pragma mark -
#pragma mark DiskArbitration and PBR Helpers
#pragma mark -

static
bool isUUID(const char* string) {
    if (strlen(string) == 36 &&
        string[8]  == '-' && string[13] == '-' &&
        string[18] == '-' && string[23] == '-')
        return true;
    return false;
}

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
bool getPBRType(char const* pathName, char *answer, size_t answer_maxsize) {
    const unsigned int bytes_to_read = 4096;
    unsigned char buffer[bytes_to_read];
    char rawPathName[ MAXPATHLEN ];

    if (geteuid() != 0) {
        fprintf(stderr, "Error: this program must be run as root to get Filesystem Type on this platform !\n");
        return false;
    }

    assert(pathName);

    snprintf(rawPathName,MAXPATHLEN, "%sr%s", _PATH_DEV, toBSDName(pathName));
    int fd = open(rawPathName, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error: unable to open %s: %s\n", rawPathName, strerror(errno));
        return false;
    }
    ssize_t rc = read(fd, (void *)buffer, bytes_to_read);
    close(fd);

    if (rc != bytes_to_read) {
        fprintf(stderr, "Error: unable to read from %s: %s\n", rawPathName, strerror(errno));
        return false;
    }

    uint16_t fat_bytesPerSector = OSSwapLittleToHostInt16(*(uint16_t const*)&buffer[0xb]);
    uint8_t fat_sectorsPerCluster = buffer[0xd];

    if (!(fat_bytesPerSector & (fat_bytesPerSector - 1U)) &&
        fat_bytesPerSector >= 0x200U && fat_bytesPerSector <= 0x1000U &&
        fat_sectorsPerCluster && !(fat_sectorsPerCluster & (fat_sectorsPerCluster - 1U)))
    {
        if (memcmp(&buffer[0x52], fat32ID, sizeof(fat32ID)-1) == 0) {
            strlcpy(answer, fat32, answer_maxsize);
            return true;
        }
        if (memcmp(&buffer[0x36], fat16ID, sizeof(fat16ID)-1) == 0) {
            strlcpy(answer, fat16, answer_maxsize);
            return true;
        }
        fprintf(stderr, "Error: unknown %s format\n", msdos);
        return false;
    }

    uint16_t hfs_sig = OSSwapBigToHostInt16(*(uint16_t const*)&buffer[1024]);
    if (hfs_sig == 0x4244 /*'BD'*/ ||
        hfs_sig == 0x482B /*'H+'*/ ||
        hfs_sig == 0x4858 /*'HX'*/) {
        strlcpy(answer, hfs, answer_maxsize);
        return true;
    }

    if (memcmp(&buffer[0x3], exfatID, sizeof(exfatID)) == 0) {
        strlcpy(answer, exfat, answer_maxsize);
        return true;
    }
    return false;
}

static
bool getFSTypeFromPBR(char const* pathName, char *answer, size_t answer_maxsize) {
    bool isSuccess = getPBRType(pathName, answer, answer_maxsize);
    if (isSuccess &&
        (strncmp(answer, fat32, answer_maxsize) == 0 ||
         strncmp(answer, fat16, answer_maxsize) == 0))
        strlcpy(answer, msdos, answer_maxsize); // return msdos for fat32 or fat16
    return isSuccess;
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
bool queryDevice(char const* deviceName, queryType query, char *answer, size_t answer_maxsize)
{
    struct stat buf;
    DADiskRef disk;
    CFDictionaryRef descDict;
    CFBooleanRef b_ref;
    bool result = false;
    int isMediaWhole = 0;

    assert(deviceName);

    char devFilePath[ MAXPATHLEN ];
    char *deviceFilePath = &devFilePath[0];

    strlcpy( deviceFilePath, _PATH_DEV, sizeof( devFilePath ) );
    strlcat( deviceFilePath, deviceName, sizeof( devFilePath ) );

    if (stat(deviceFilePath, &buf) < 0) {
        fprintf(stderr, "Error: stat failed on %s: %s\n", deviceFilePath, strerror(errno));
        return false;
    }
    if (!(buf.st_mode & (S_IFCHR | S_IFBLK))) {
        fprintf(stderr, "Error: %s is not a block or character special device\n", deviceFilePath);
        return false;
    }

    if (getDASessionAndDisk(deviceName, NULL, &disk) < 0)
		return false;

    descDict = DADiskCopyDescription(disk);
    CFRelease(disk);

	if (!descDict) {
		return false;
	}

    if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionMediaWholeKey, (void const**) &b_ref) &&
        CFBooleanGetValue(b_ref))
        isMediaWhole = 1;

    // If query partition scheme, the device must be a whole disk
    if (query == query_partitionscheme && !isMediaWhole) {
        fprintf(stderr, "Error: device must be a whole disk to query about partition scheme !\n");
        CFRelease(descDict);
        return false;
    }

    CFStringRef key = NULL;
    switch (query) {
        case query_fstype:
            key = kDADiskDescriptionVolumeKindKey;
            break;
        case query_bsdname:
            key = kDADiskDescriptionMediaBSDNameKey;
            break;
        case query_mountpoint:
            key = kDADiskDescriptionVolumePathKey;
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
        case query_partitionscheme:
            key = kDADiskDescriptionMediaContentKey;
            break;
        case query_wholedisk:
            key = kDADiskDescriptionMediaBSDUnitKey;
            break;
        case query_contenttype:
            key = kDADiskDescriptionMediaContentKey;
            break;
        case query_dump:
            CFShow(descDict);
            CFRelease(descDict);
            return EXIT_SUCCESS;
            break;
        case query_pbrtype: // never used here
        case query_undefined:
            break;
    }
    if (!key) {
        CFRelease(descDict);
        return false;
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
        else if (typeID == CFURLGetTypeID())
            cfstr_value = CFURLCopyFileSystemPath ( valueRef, kCFURLPOSIXPathStyle ); // convert CFURL to CFStringRef
        if (cfstr_value) {
            result = CFStringGetCString(cfstr_value, answer, answer_maxsize, kCFStringEncodingUTF8);
            CFRelease(cfstr_value);
        }
    }
	CFRelease(descDict);

    if (query == query_wholedisk && result == true) {
        // append the string "disk" after the BSD unit number
        char bsd_unit_number[answer_maxsize];
        strlcpy(bsd_unit_number, answer, sizeof(bsd_unit_number));
        snprintf(answer, answer_maxsize, "disk%s", bsd_unit_number);
    }
    if (result == false && query == query_fstype) {
        // Try to find the fstype by analysing the PBR of the partition
        return getFSTypeFromPBR(deviceName, answer, answer_maxsize);
    }
    return result;
}

#pragma mark -
#pragma mark Search
#pragma mark -

static
bool doSearch(searchType search, char const* searchValue, char *answer, size_t answer_maxsize)
{
    mach_port_t         masterPort;
    kern_return_t       kernResult;
    CFMutableDictionaryRef   classesToMatch;
    io_iterator_t mediaIterator;

    assert(searchValue);

    kernResult = IOMasterPort( MACH_PORT_NULL, &masterPort );
    if ( kernResult != KERN_SUCCESS ) {
        fprintf(stderr, "IOMasterPort returned %d\n", kernResult );
        return false;
    }

    classesToMatch = IOServiceMatching( kIOMediaClass ); // All Storage Media
    if ( classesToMatch == NULL ) {
        fprintf( stderr, "IOServiceMatching returned a NULL dictionary.\n" );
        return false;
    }
    CFDictionarySetValue( classesToMatch,
                         CFSTR( kIOMediaLeafKey ), kCFBooleanTrue ); // Is a leaf device

    CFStringRef searchCFString = CFStringCreateWithCString(NULL, searchValue, kCFStringEncodingASCII);
    switch (search) {
        case search_uuid:
            if ( ! isUUID(searchValue) ) {
                fprintf(stderr, "Error: invalid UUID value '%s'\n", searchValue);
                CFRelease(searchCFString);
                CFRelease(classesToMatch); // Not use so release it
                return false;
            }
            CFMutableStringRef tmpMutableStr = CFStringCreateMutableCopy(0, 36, searchCFString);
            CFStringUppercase(tmpMutableStr, 0); // Convert to Uppercase
            CFDictionarySetValue( classesToMatch, CFSTR( kIOMediaUUIDKey ), tmpMutableStr);
            CFRelease(tmpMutableStr);
            break;

        case search_undefined:
            CFRelease(searchCFString);
            CFRelease(classesToMatch); // Not use so release it
            return false;
            break;
    }
    CFRelease(searchCFString);

    // classesToMatch is consume by IOServiceGetMatchingServices
    kernResult = IOServiceGetMatchingServices( masterPort,
                                               classesToMatch, &mediaIterator );

    if ( kernResult != KERN_SUCCESS ) {
        fprintf(stderr, "No Block Storage media found.\n kernResult = %d\n", kernResult );
        return false;
    }

    io_service_t service;
    CFMutableDictionaryRef	serviceProperties = NULL;

    while ((service = IOIteratorNext( mediaIterator ))) {

        kernResult = IORegistryEntryCreateCFProperties(service, &serviceProperties, NULL, 0);
        if ( kernResult != KERN_SUCCESS ) {
            fprintf(stderr, "Error: IORegistryEntryCreateCFProperties returned %d\n", kernResult );
            IOObjectRelease( service );
            return false;
        }

        CFStringRef s_ref;
        // get the BSO device name
        if (CFDictionaryGetValueIfPresent(serviceProperties, CFSTR( kIOBSDNameKey ), (void const**) &s_ref)) {
            CFStringGetCString(s_ref, answer, answer_maxsize, kCFStringEncodingUTF8);
            CFRelease(serviceProperties);
            IOObjectRelease( service );
            return true;
        }

        CFRelease(serviceProperties);
        IOObjectRelease( service );
    }
    return false;
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
Usage: " PROGNAME_S " [QUERY OPTION] [DEVICE|UUID]\n\
       " PROGNAME_S " [SEARCH OPTION]\n\
\n\
Query options:\n\
\t--show-fstype           display the filesytem type of the partition\n\
\t--show-bsdname          display the device name of the partition\n\
\t--show-mountpoint       display the mount point of the partition\n\
\t--show-volumename       display the volume name of the partition\n\
\t--show-uuid             display the UUID of the partition\n\
\t--show-blocksize        display the prefer blocksize of the partition\n\
\t--show-partitionscheme  display the partition scheme of a disk\n\
\t--show-contenttype      display the content type of the device\n\
\t--show-pbrtype          display the filesystem type from the PBR of the device\n\
\t--show-wholedisk        display the whole disk of the device\n\
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
         " PROGNAME_S " --show-bsdname 6A9017D9-2B9E-4786-B0A5-A75BD2264239\n\
         " PROGNAME_S " --show-blocksize disk0s4\n\
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
    {"show-bsdname", no_argument, 0, query_bsdname},
    {"show-mountpoint", no_argument, 0, query_mountpoint},
    {"show-volumename", no_argument, 0, query_volumename},
    {"show-uuid", no_argument, 0, query_uuid},
    {"show-blocksize", no_argument, 0, query_blocksize},
    {"show-partitionscheme", no_argument, 0, query_partitionscheme},
    {"show-contenttype", no_argument, 0, query_contenttype},
    {"show-pbrtype", no_argument, 0, query_pbrtype},
    {"show-wholedisk", no_argument, 0, query_wholedisk},
    {"dump", no_argument, 0, query_dump},
    {"help",    no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

int main(int argc, char* const argv[])
{
    queryType query = query_undefined;
    searchType search = search_undefined;
    char const *searchValue = NULL;
    char answer[64] = "\0";
    bool isSuccess = false;

    /* Check for options.  */
    while (1) {
        int c = getopt_long (argc, argv, "d:r:hVv", options, 0);
        if (c == -1)
            break;
        else
            switch (c) {
                case search_uuid:
                    if (search != search_undefined) {
                        fprintf(stderr, "Error: only one search can be done !\n");
                        exit(EXIT_FAILURE);
                    }
                    if (query != query_undefined) {
                        fprintf(stderr, "Error: can't mix query and search !\n");
                        exit(EXIT_FAILURE);
                    }
                    search = c;
                    searchValue = optarg;
                    break;

                case query_fstype:
                case query_bsdname:
                case query_mountpoint:
                case query_volumename:
                case query_uuid:
                case query_blocksize:
                case query_partitionscheme:
                case query_contenttype:
                case query_pbrtype:
                case query_wholedisk:
                case query_dump:
                    if (query != query_undefined) {
                        fprintf(stderr, "Error: only one query can be done !\n");
                        exit(EXIT_FAILURE);
                    }
                    if (search != search_undefined) {
                        fprintf(stderr, "Error: can't mix query and search !\n");
                        exit(EXIT_FAILURE);
                    }
                    query = c;
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

    if (search != search_undefined) {
        isSuccess = doSearch(search, searchValue, answer, sizeof(answer));
    }
    else {
        char const* argument = argv[optind++];
        char device[ MAXPATHLEN ];
        if (strlen(argument) >= 30) { // It's must be a uuid
            if (!isUUID(argument)) {
                fprintf(stderr, "Error: invalid UUID value '%s'\n", argument);
                return EXIT_FAILURE;
            }
            // it's an UUID try to get the devicename
            isSuccess = doSearch(search_uuid, argument, answer, sizeof(answer));
            if (!isSuccess) {
                fprintf(stderr,"Error: can't find device with UUID '%s'\n",argument);
                return EXIT_FAILURE;
            }
            strlcpy(device, answer, sizeof(device));
        }
        else {
            strlcpy(device, toBSDName(argument), sizeof(device));
        }

        if (query == query_pbrtype)
            isSuccess = getPBRType(device, answer, sizeof(answer));
        else
            isSuccess = queryDevice(device, query, answer, sizeof(answer));
    }


    if (isSuccess)
        printf("%s\n", answer);
    return isSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}
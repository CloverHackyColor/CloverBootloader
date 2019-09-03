/*
 *  boot1-install.c
 *  boot1-install
 *
 *  Created by Zenith432 on November 19th, 2014.
 *  Copyright (c) 2014 Zenith432. All rights reserved.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

struct buffer_t
{
	unsigned char* _b;
	size_t _s;
};

enum volume_kind_t
{
	_undetected = 0,
	_exfat = 1,
	_hfs = 2,
	_msdos = 3,
  _ntfs = 4,
  _ext4 = 5,
	_other = 255
};

static int isVolumeMounted = 0;
static int isMediaWhole = 0;
static int isMediaLeaf = 0;
static enum volume_kind_t daVolumeKind = _undetected;

static struct buffer_t bpbBlob = { NULL, 0 };
static struct buffer_t bootBlob = { NULL, 0 };
static struct buffer_t outputBlob = { NULL, 0 };

static char const UnsupportedMessage[] = "Only exFAT, FAT32 or HFS+ volumes are supported\n";
static char const exfatID[] = "EXFAT   ";
static char const fat32ID[] = "FAT32   ";
static char const devrdisk[] = "/dev/rdisk";
static char const devdisk[] = "/dev/disk";
static char const defaultBootFile_exfat[] = "./boot1x";
static char const defaultBootFile_hfs[] = "./boot1h";
static char const defaultBootFile_fat32[] = "./boot1f32";

static __used char const copyright[] = "Copyright 2014 Zenith432";

static int checkExfat(struct buffer_t const*);
static int checkFat32(struct buffer_t const*);
static int loadChunk(char const*, off_t, off_t, struct buffer_t*);
static void unsupported(void);

#pragma mark -
#pragma mark Cleaners
#pragma mark -

static
void free_buffer(struct buffer_t* pBuffer)
{
	assert(pBuffer);
	if (pBuffer->_b) {
		free(pBuffer->_b);
		pBuffer->_b = NULL;
		pBuffer->_s = 0;
	}
}

/*
 * Uses statics
 */
static
void cleanup(void)
{
	free_buffer(&outputBlob);
	free_buffer(&bootBlob);
	free_buffer(&bpbBlob);
}

#pragma mark -
#pragma mark ExFAT Processor
#pragma mark -

static
unsigned VBRChecksum(unsigned char const* octets, size_t NumberOfBytes)
{
	unsigned Checksum = 0;
	size_t Index;
	for (Index = 0; Index != NumberOfBytes; ++Index)
	{
		if (Index == 106 || Index == 107 || Index == 112)
			continue;
		Checksum = ((Checksum << 31) | (Checksum >> 1)) + (unsigned) octets[Index];
	}
	return Checksum;
}

static
int calcSum(struct buffer_t const* pBootBlob,
			struct buffer_t const* pBpbBlob,
			struct buffer_t* pOutputBlob,
			char const* pathName)
{
	unsigned char *outBuffer, *p, *q;
	size_t outSize, toCopy, leftOver;
	unsigned Checksum;

	assert(pBootBlob && pBpbBlob);
	if (pBootBlob->_s > 9U * 512U) {
		fprintf(stderr, "Boot Code must be at most 4608 bytes\n");
		return -1;
	}
	if (pBpbBlob->_s < 113U) {
		fprintf(stderr, "BPB must be at least 113 bytes\n");
		return -1;
	}
	if (!checkExfat(pBpbBlob)) {
		fprintf(stderr, "BPB does not contain proper exFAT signature\n");
		return -1;
	}
	outSize = 12U * 512U;
	outBuffer = malloc(outSize);
	if (!outBuffer) {
		fprintf(stderr, "%s: Memory allocation failed\n", __FUNCTION__);
		return -1;
	}
	memset(outBuffer, 0, outSize);
	memcpy(outBuffer, pBootBlob->_b, pBootBlob->_s);
	memcpy(&outBuffer[3], &pBpbBlob->_b[3], 8);
	memset(&outBuffer[11], 0, 53);
	toCopy = 120;
	if (pBpbBlob->_s < toCopy)
		toCopy = pBpbBlob->_s;
	leftOver = 120 - toCopy;
	memcpy(&outBuffer[64], &pBpbBlob->_b[64], toCopy - 64);
	if (leftOver)
		memset(&outBuffer[120 - leftOver], 0, leftOver);
	for (toCopy = 0; toCopy != 9; ++toCopy) {
		p = outBuffer + toCopy * 512U + 508U;
		p[2] = 0x55U;
		p[3] = 0xAAU;
		if (toCopy) {
			p[0] = 0U;
			p[1] = 0U;
		}
	}
	if (pathName) {
		/*
		 * Copy OEM Parameters record
		 */
		struct buffer_t auxBlob = { NULL, 0 };
		if (loadChunk(pathName, 9 * 512 , 512, &auxBlob) >= 0) {
			memcpy(&outBuffer[9 * 512], &auxBlob._b[0], 512);
			free_buffer(&auxBlob);
		}
	}
	Checksum = VBRChecksum(outBuffer, 11U * 512U);
	p = outBuffer + 11U * 512U;
	q = p + 512U;
	for (; p < q; p += 4) {
		*(unsigned*) p = Checksum;
	}
	if (pOutputBlob) {
		pOutputBlob->_b = outBuffer;
		pOutputBlob->_s = outSize;
	} else
		free(outBuffer);
	return 0;
}

#pragma mark -
#pragma mark FAT32 Processor
#pragma mark -

static
int fat32Layout(struct buffer_t const* pBootBlob,
				struct buffer_t const* pBpbBlob,
				struct buffer_t* pOutputBlob)
{
	unsigned char *outBuffer;
	size_t outSize;

	assert(pBootBlob && pBpbBlob);
	if (pBootBlob->_s > 512U) {
		fprintf(stderr, "Boot Code must be at most 512 bytes\n");
		return -1;
	}
	if (pBpbBlob->_s < 90U) {
		fprintf(stderr, "BPB must be at least 90 bytes\n");
		return -1;
	}
	if (!checkFat32(pBpbBlob)) {
		fprintf(stderr, "BPB does not contain proper FAT32 signature\n");
		return -1;
	}
	outSize = 512U;
	outBuffer = malloc(outSize);
	if (!outBuffer) {
		fprintf(stderr, "%s: Memory allocation failed\n", __FUNCTION__);
		return -1;
	}
	memset(outBuffer, 0, outSize);
	memcpy(outBuffer, pBootBlob->_b, pBootBlob->_s);
	memcpy(&outBuffer[3], &pBpbBlob->_b[3], 87);
	outBuffer[510] = 0x55U;
	outBuffer[511] = 0xAAU;
	if (pOutputBlob) {
		pOutputBlob->_b = outBuffer;
		pOutputBlob->_s = outSize;
	} else
		free(outBuffer);
	return 0;
}

#pragma mark -
#pragma mark File Operations
#pragma mark -

static
void writeVBR(char const* pathName,
			  struct buffer_t const* pBuffer,
			  int numCopies,
			  size_t expectedSize,
			  char const* volumeType)
{
	int fd, j;

	assert(pathName && pBuffer && volumeType);
	if (pBuffer->_s != expectedSize) {
		fprintf(stderr, "Unexpected %s VBR size %lu (expected %lu)\n", volumeType, pBuffer->_s, expectedSize);
		return;
	}
	fd = open(pathName, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Unable to write boot record to %s, %s\n", pathName, strerror(errno));
	}
	for (j = 0; j != numCopies; ++j)
		write(fd, pBuffer->_b, pBuffer->_s);
	close(fd);
}

static
int loadChunk(char const* pathName, off_t startOffset, off_t bytesToRead, struct buffer_t* pBuffer)
{
	int fd;
	ssize_t rc;
	unsigned char* p;
	struct stat buf;

	assert(pathName);
	fd = open(pathName, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Unable to open %s, %s\n", pathName, strerror(errno));
		return -1;
	}
	if (bytesToRead > 0)
		buf.st_size = bytesToRead;
	else if (fstat(fd, &buf) < 0) {
		fprintf(stderr, "Unable to fstat %s, %s\n", pathName, strerror(errno));
		close(fd);
		return -1;
	}
	if (startOffset > 0) {
		off_t t = lseek(fd, startOffset, SEEK_SET);
		if (t < 0) {
			fprintf(stderr, "Unable to lseek %s, %s\n", pathName, strerror(errno));
			close(fd);
			return -1;
		}
		if (t != startOffset) {
			fprintf(stderr, "lseek %s returned wrong value %lld instead of %lld\n", pathName, t, startOffset);
			close(fd);
			return -1;
		}
		if (bytesToRead <= 0)
			buf.st_size -= t;
	}
	p = malloc((size_t) buf.st_size);
	if (!p) {
		fprintf(stderr, "%s: Memory allocation failed\n", __FUNCTION__);
		close(fd);
		return -1;
	}
	rc = read(fd, p, (size_t) buf.st_size);
	if (rc < 0) {
		fprintf(stderr, "Unable to read from %s, %s\n", pathName, strerror(errno));
		free(p);
		close(fd);
		return -1;
	}
	close(fd);
	if (rc != buf.st_size) {
		fprintf(stderr, "Unable to read entire chunk from %s, read %ld/%lld\n", pathName, rc, buf.st_size);
		free(p);
		return -1;
	}
	if (pBuffer) {
		pBuffer->_b = p;
		pBuffer->_s = (size_t) rc;
	} else
		free(p);
	return 0;
}

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
char const* daReturnStr(DAReturn v)
{
	if (unix_err(err_get_code(v)) == v)
		return strerror(err_get_code(v));
	switch (v) {
		case kDAReturnError:
			return "Error";
		case kDAReturnBusy:
			return "Busy";
		case kDAReturnBadArgument:
			return "Bad Argument";
		case kDAReturnExclusiveAccess:
			return "Exclusive Access";
		case kDAReturnNoResources:
			return "No Resources";
		case kDAReturnNotFound:
			return "Not Found";
		case kDAReturnNotMounted:
			return "Not Mounted";
		case kDAReturnNotPermitted:
			return "Not Permitted";
		case kDAReturnNotPrivileged:
			return "Not Privileged";
		case kDAReturnNotReady:
			return "Not Ready";
		case kDAReturnNotWritable:
			return "Not Writable";
		case kDAReturnUnsupported:
			return "Unsupported";
		default:
			return "Unknown";
	}
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

#pragma mark -
#pragma mark Mount/UMount
#pragma mark -

static
void umountCallback(DADiskRef disk __unused,
					DADissenterRef dissenter,
					void *context)
{
	if (context && dissenter != NULL) {
		*(int*) context = -1;
		fprintf(stderr, "umount unsuccessful, status %s\n", daReturnStr(DADissenterGetStatus(dissenter)));
	}
	CFRunLoopStop(CFRunLoopGetCurrent());
}

static
int umount(char const* pathName)
{
	DASessionRef session;
	DADiskRef disk;
	int rc;

	assert(pathName);
	if (getDASessionAndDisk(pathName, &session, &disk) < 0)
		return -1;
	rc = 0;
	DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	DADiskUnmount(disk, kDADiskUnmountOptionDefault, umountCallback, &rc);
	CFRunLoopRun();
	DASessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	CFRelease(disk);
	CFRelease(session);
	return rc;
}

static
void mountCallback(DADiskRef disk __unused,
				   DADissenterRef dissenter,
				   void *context)
{
	if (context && dissenter != NULL) {
		*(int*) context = -1;
		fprintf(stderr, "mount unsuccessful, status %s\n", daReturnStr(DADissenterGetStatus(dissenter)));
	}
	CFRunLoopStop(CFRunLoopGetCurrent());
}

static
int mount(char const* pathName)
{
	DASessionRef session;
	DADiskRef disk;
	int rc;

	assert(pathName);
	if (getDASessionAndDisk(pathName, &session, &disk) < 0)
		return -1;
	rc = 0;
	DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	DADiskMount(disk, NULL, kDADiskMountOptionDefault, mountCallback, &rc);
	CFRunLoopRun();
	DASessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	CFRelease(disk);
	CFRelease(session);
	return rc;
}

#pragma mark -
#pragma mark Analyze Volume
#pragma mark -

static
int checkExfat(struct buffer_t const* pBpbBlob)
{
	assert(pBpbBlob);
	return !memcmp(&pBpbBlob->_b[3], &exfatID[0], 8);
}

static
int checkHFS(struct buffer_t const* pBpbBlob)
{
	uint16_t sig;

	assert(pBpbBlob);
	sig = OSSwapBigToHostInt16(*(uint16_t const*)&pBpbBlob->_b[0]);
	return sig == 0x4244 || sig == 0x482B || sig == 0x4858;		/* 'BD', 'H+', 'HX' */
}

static
int checkFat32(struct buffer_t const* pBpbBlob)
{
	uint16_t bytesPerSector, rootEntCnt;
	uint8_t sectorsPerCluster;

	assert(pBpbBlob);
	bytesPerSector = OSSwapLittleToHostInt16(*(uint16_t const*)&pBpbBlob->_b[11]);
	if ((bytesPerSector & (bytesPerSector - 1U)) ||
		bytesPerSector < 0x200U ||
		bytesPerSector > 0x1000U)
		return 0;
	sectorsPerCluster = pBpbBlob->_b[13];
	if (!sectorsPerCluster ||
		(sectorsPerCluster & (sectorsPerCluster - 1U)))
		return 0;
	rootEntCnt = OSSwapLittleToHostInt16(*(uint16_t const*)&pBpbBlob->_b[17]);
	if (rootEntCnt)
		return 0;
	return !memcmp(&pBpbBlob->_b[82], &fat32ID[0], 8);
}

static
int checkSupportedVolume(enum volume_kind_t* pKind, struct buffer_t const* pBpbBlob, char const* pathName)
{
	int rc;

	assert(pKind && pBpbBlob);
	rc = -1;
	switch (*pKind) {
		case _undetected:
			if (checkExfat(pBpbBlob)) {
				*pKind = _exfat;
				rc = 0;
			} else if (checkFat32(pBpbBlob)) {
				*pKind = _msdos;
				rc = 0;
			} else if (pathName) {
				struct buffer_t auxBlob = { NULL, 0 };
				if (loadChunk(pathName, 1024 , 512, &auxBlob) >= 0) {
					if (checkHFS(&auxBlob)) {
						*pKind = _hfs;
						rc = 0;
					}
					free_buffer(&auxBlob);
				}
			}
			break;
		case _exfat:
			if (checkExfat(pBpbBlob))
				rc = 0;
			else
				*pKind = _other;
			break;
		case _hfs:
			if (checkHFS(pBpbBlob))
				rc = 0;
			else
				*pKind = _other;
			break;
		case _msdos:
			if (checkFat32(pBpbBlob))
				rc = 0;
			else
				*pKind = _other;
			break;
    case _ntfs:
      rc = 0;
      break;
		default:
			break;
	}
	if (rc < 0)
		unsupported();
	return rc;
}

/*
 * Uses statics
 */
static
int checkDevicePath2(char const* pathName)
{
	DASessionRef session;
	DADiskRef disk;
	CFDictionaryRef descDict;
	CFStringRef s_ref;
	CFBooleanRef b_ref;

	assert(pathName);
	if (getDASessionAndDisk(pathName, &session, &disk) < 0)
		return -1;
	descDict = DADiskCopyDescription(disk);
	if (!descDict) {
		CFRelease(disk);
		CFRelease(session);
		fprintf(stderr, "DADiskCopyDescription(%s) returned NULL\n", pathName);
		return -1;
	}
	if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionMediaWholeKey, (void const**) &b_ref) &&
		CFBooleanGetValue(b_ref))
		isMediaWhole = 1;
	if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionMediaLeafKey, (void const**) &b_ref) &&
		CFBooleanGetValue(b_ref))
		isMediaLeaf = 1;
	if (CFDictionaryContainsKey(descDict, kDADiskDescriptionVolumePathKey))
		isVolumeMounted = 1;
	if (CFDictionaryGetValueIfPresent(descDict, kDADiskDescriptionVolumeKindKey, (void const**) &s_ref)) {
		static char cstr_buffer[64];
		char const* cstr = CFStringGetCStringPtr(s_ref, kCFStringEncodingUTF8);
		if (!cstr) {
			CFStringGetCString(s_ref, &cstr_buffer[0], (CFIndex) sizeof cstr_buffer, kCFStringEncodingUTF8);
			cstr = &cstr_buffer[0];
		}
#if 0
		printf("DAVolumeKind %s\n", cstr);
#endif
		if (!strcmp(cstr, "exfat"))
			daVolumeKind = _exfat;
		else if (!strcmp(cstr, "hfs"))
			daVolumeKind = _hfs;
		else if (!strcmp(cstr, "msdos"))
			daVolumeKind = _msdos;
		else if (!strcmp(cstr, "ntfs"))
			daVolumeKind = _ntfs;
		else
			daVolumeKind = _other;
	}
#if 0
	printf(stderr, "whole %c, leaf %c, mounted %c\n",
		   isMediaWhole ? 'Y' : 'N',
		   isMediaLeaf ? 'Y' : 'N',
		   isVolumeMounted ? 'Y' : 'N');
#endif
#if 0
	CFShow(descDict);
#endif
	CFRelease(descDict);
	CFRelease(disk);
	CFRelease(session);
	return 0;
}

static
int checkDevicePath(char const* pathName)
{
	struct stat buf;

	assert(pathName);
	if (strncmp(pathName, &devdisk[0], 9) != 0 &&
		strncmp(pathName, &devrdisk[0], 10) != 0) {
		fprintf(stderr, "disk must be of form /dev/rdiskUsS or /dev/diskUsS\n");
		return -1;
	}
	if (stat(pathName, &buf) < 0) {
		fprintf(stderr, "stat on %s failed, %s\n", pathName, strerror(errno));
		return -1;
	}
	if (!(buf.st_mode & (S_IFCHR | S_IFBLK))) {
		fprintf(stderr, "%s is not a block or character special device\n", pathName);
		return -1;
	}
	/*
	 * FIXME: milk information from st_rdev - what's in it?
	 */
#if 0
	printf("size of buf is %lu\n", sizeof buf);
	printf("st_dev %#x\n", buf.st_dev);
	printf("st_ino %llu\n", buf.st_ino);
	printf("st_mode %#o\n", buf.st_mode);
	printf("st_nlink %u\n", buf.st_nlink);
	printf("st_uid %u\n", buf.st_uid);
	printf("st_gid %u\n", buf.st_gid);
	printf("st_rdev %#x\n", buf.st_rdev);
	printf("st_size %llu\n", buf.st_size);
	printf("st_blocks %llu\n", buf.st_blocks);
	printf("st_blksize %u\n", buf.st_blksize);
	printf("st_flags %#x\n", buf.st_flags);
	printf("st_gen %u\n", buf.st_gen);
#endif
	return 0;
}

#pragma mark -
#pragma mark Usage
#pragma mark -

static
void usage(char const* self)
{
	assert(self);
	fprintf(stderr, "Usage: %s [-yM] [-f boot_code_file] disk\n", self);
	fprintf(stderr, "  boot_code_file is an optional boot template\n");
	fprintf(stderr, "  -y: don't ask any questions\n");
	fprintf(stderr, "  -M: keep volume mounted while proceeding (useful for root filesystem)\n");
	fprintf(stderr, "disk is of the form /dev/rdiskUsS or /dev/diskUsS\n");
	fprintf(stderr, "default boot files are\n");
	fprintf(stderr, "  boot1h for HFS+\n");
	fprintf(stderr, "  boot1f32 for FAT32\n");
	fprintf(stderr, "  boot1x for exFAT\n");
}

static
void unsupported(void)
{
	fprintf(stderr, "%s", &UnsupportedMessage[0]);
}

#pragma mark -
#pragma mark Main
#pragma mark -

int main(int argc, char* const argv[])
{
	int ch;
	char const* bootFile = NULL;
	char const* devicePath = NULL;
	int dontAsk = 0;
	int keepMounted = 0;

	while ((ch = getopt(argc, argv, "yMf:")) != -1)
		switch (ch) {
			case 'y':
				dontAsk = 1;
				break;
			case 'M':
				keepMounted = 1;
				break;
			case 'f':
				bootFile = optarg;
				break;
			default:
				goto usage_and_error;
		}
	if (optind + 1 > argc)
		goto usage_and_error;
	devicePath = argv[optind];
	if (geteuid() != 0) {
		fprintf(stderr, "This program must be run as root\n");
		return -1;
	}
#if 0
	printf("bootFile %s, devicePath %s, dontAsk %d\n", bootFile, devicePath, dontAsk);
#endif
	if (checkDevicePath(devicePath) < 0)
		return -1;
	if (checkDevicePath2(devicePath) >= 0) {
		if (isMediaWhole && !isMediaLeaf) {
			fprintf(stderr, "%s is a whole disk\n", devicePath);
			return -1;
		}
		switch (daVolumeKind) {
			case _undetected:
			case _exfat:
			case _hfs:
			case _msdos:
      case _ntfs:
				break;
			default:
				unsupported();
				return -1;
		}
		if (isVolumeMounted && keepMounted)
			isVolumeMounted = 0;
		if (isVolumeMounted && umount(devicePath) < 0) {
			fprintf(stderr, "Unable to umount %s, please 'diskutil umount' manually before running this program\n", devicePath);
			return -1;
		}
	}
	/*
	 * Note:
	 *   Reading a non-multiple of 512 does not work on /dev/rdisk
	 */
	if (loadChunk(devicePath, daVolumeKind == _hfs ? 1024 : 0, 512, &bpbBlob) < 0)
		goto remount_and_error;
	if (checkSupportedVolume(&daVolumeKind, &bpbBlob, devicePath) < 0)
		goto cleanup_and_error;
	if (!bootFile) {
		switch (daVolumeKind) {
			case _exfat:
				bootFile = &defaultBootFile_exfat[0];
				break;
			case _hfs:
				bootFile = &defaultBootFile_hfs[0];
				break;
			case _msdos:
				bootFile = &defaultBootFile_fat32[0];
				break;
			default:
				unsupported();
				return -1;
		}
		printf("Using %s as default boot template\n", bootFile);
	}

	if (loadChunk(bootFile, 0, 0, &bootBlob) < 0)
		goto cleanup_and_error;
  
	switch (daVolumeKind) {
		case _exfat:
			if (calcSum(&bootBlob, &bpbBlob, &outputBlob, devicePath) < 0)
				goto cleanup_and_error;
			break;
		case _hfs:
			free_buffer(&bpbBlob);
			if (bootBlob._s != 1024U) {
				fprintf(stderr, "Boot Code size must be 1024 bytes\n");
				goto cleanup_and_error;
			}
			break;
		case _msdos:
			if (fat32Layout(&bootBlob, &bpbBlob, &outputBlob) < 0)
				goto cleanup_and_error;
			break;
		default:
			assert(0);
			break;
	}
	if (!dontAsk) {
		printf("About to write new boot record on %s, Are You Sure (Y/N)?", devicePath);
		ch = 0;
		while (ch != 'Y' && ch != 'N')
			ch = getchar();
		if (ch != 'Y') {
			printf("Aborted due to user request\n");
			goto cleanup_and_exit;
		}
	}
	switch (daVolumeKind) {
		case _exfat:
			writeVBR(devicePath, &outputBlob, 2, 12U * 512U, "exFAT");
			break;
		case _hfs:
			writeVBR(devicePath, &bootBlob, 1, 1024U, "HFS+");
			break;
		case _msdos:
			writeVBR(devicePath, &outputBlob, 1, 512U, "FAT32");
			break;
		default:
			assert(0);
			break;
	}

cleanup_and_exit:
	cleanup();
	if (isVolumeMounted)
		mount(devicePath);
	return 0;

cleanup_and_error:
	cleanup();
remount_and_error:
	if (isVolumeMounted)
		mount(devicePath);
	return -1;

usage_and_error:
	usage(argv[0]);
	return -1;
}

/*
 *  Hibernate.c
 *
 *  Created by dmazar, 01.2014.
 *
 *  Hibernate support.
 *
 */


#include <Platform.h>


#ifndef DEBUG_ALL
#define DEBUG_HIB 1
#else
#define DEBUG_HIB DEBUG_ALL
#endif

#if DEBUG_HIB == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_HIB, __VA_ARGS__);
#endif

#define CREATE_NEW_BOOT_IMAGE 1

extern XObjArray<REFIT_VOLUME> Volumes;

#pragma pack(push, 1)

//
// Just the first part of HFS+ volume header from where we can take modification time
//
typedef struct _HFSPlusVolumeHeaderMin {
  UINT16              signature;
  UINT16              version;
  UINT32              attributes;
  UINT32              lastMountedVersion;
  UINT32              journalInfoBlock;
  
  UINT32              createDate;
  UINT32              modifyDate;
  UINT32              backupDate;
  UINT32              checkedDate;
  
  UINT32              fileCount;
  UINT32              folderCount;
  
  UINT32              blockSize;
  UINT32              totalBlocks;
  UINT32              freeBlocks;
} HFSPlusVolumeHeaderMin;

// IOHibernateImageHeader.signature
enum
{
  kIOHibernateHeaderSignature        = 0x73696d65,
  kIOHibernateHeaderInvalidSignature = 0x7a7a7a7a
};

typedef struct _IOHibernateImageHeaderMin
{
  UINT64  imageSize;
  UINT64  image1Size;
  
  UINT32  restore1CodePhysPage;
  UINT32    reserved1;
  UINT64    restore1CodeVirt;
  UINT32  restore1PageCount;
  UINT32  restore1CodeOffset;
  UINT32  restore1StackOffset;
  
  UINT32  pageCount;
  UINT32  bitmapSize;
  
  UINT32  restore1Sum;
  UINT32  image1Sum;
  UINT32  image2Sum;
  
  UINT32  actualRestore1Sum;
  UINT32  actualImage1Sum;
  UINT32  actualImage2Sum;
  
  UINT32  actualUncompressedPages;
  UINT32  conflictCount;
  UINT32  nextFree;
  
  UINT32  signature;
  UINT32  processorFlags;
  UINT32    runtimePages;
  UINT32    runtimePageCount;
  UINT64    runtimeVirtualPages;
  
  UINT32    performanceDataStart;
  UINT32    performanceDataSize;
  
  UINT64  encryptStart;
  UINT64  machineSignature;
  
  UINT32    previewSize;
  UINT32    previewPageListSize;
  
  UINT32  diag[4];
  
  UINT32    handoffPages;
  UINT32    handoffPageCount;
  
  UINT32    systemTableOffset;
  
  UINT32  debugFlags;
  UINT32  options;
  UINT32  sleepTime;
  UINT32    compression;
  
} IOHibernateImageHeaderMin;

typedef struct _IOHibernateImageHeaderMinSnow
{
  UINT64  imageSize;
  UINT64  image1Size;
  
  UINT32  restore1CodePhysPage;
  UINT32  restore1PageCount;
  UINT32  restore1CodeOffset;
  UINT32  restore1StackOffset;
  
  UINT32  pageCount;
  UINT32  bitmapSize;
  
  UINT32  restore1Sum;
  UINT32  image1Sum;
  UINT32  image2Sum;
  
  UINT32  actualRestore1Sum;
  UINT32  actualImage1Sum;
  UINT32  actualImage2Sum;
  
  UINT32  actualUncompressedPages;
  UINT32  conflictCount;
  UINT32  nextFree;
  
  UINT32  signature;
  UINT32  processorFlags;
} IOHibernateImageHeaderMinSnow;


typedef struct _AppleRTCHibernateVars
{
  UINT8     signature[4];
  UINT32    revision;
  UINT8    booterSignature[20];
  UINT8    wiredCryptKey[16];
} AppleRTCHibernateVars;

#pragma pack(pop)

//
// Taken from VBoxFsDxe
//

//
// time conversion
//
// Adopted from public domain code in FreeBSD libc.
//

#define SECSPERMIN      60
#define MINSPERHOUR     60
#define HOURSPERDAY     24
#define DAYSPERWEEK     7
#define DAYSPERNYEAR    365
#define DAYSPERLYEAR    366
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY      ((INTN) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR     12

#define EPOCH_YEAR      1970
#define EPOCH_WDAY      TM_THURSDAY

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define LEAPS_THRU_END_OF(y)    ((y) / 4 - (y) / 100 + (y) / 400)

INT32 mon_lengths[2][MONSPERYEAR] = {
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
INT32 year_lengths[2] = {
  DAYSPERNYEAR, DAYSPERLYEAR
};

//static fsw_u32 mac_to_posix(fsw_u32 mac_time)
INT32 mac_to_posix(UINT32 mac_time)
{
  /* Mac time is 1904 year based */
  return mac_time ?  mac_time - 2082844800 : 0;
}
/* not used
 VOID fsw_efi_decode_time(OUT EFI_TIME *EfiTime, IN UINT32 UnixTime)
 {
 INT32        days, rem;
 INT32        y, newy, yleap;
 INT32        *ip;
 
 ZeroMem(EfiTime, sizeof(EFI_TIME));
 
 days = ((INTN)UnixTime) / SECSPERDAY;
 rem = ((INTN)UnixTime) % SECSPERDAY;
 
 EfiTime->Hour = (UINT8) (rem / SECSPERHOUR);
 rem = rem % SECSPERHOUR;
 EfiTime->Minute = (UINT8) (rem / SECSPERMIN);
 EfiTime->Second = (UINT8) (rem % SECSPERMIN);
 
 y = EPOCH_YEAR;
 while (days < 0 || days >= (INT64) year_lengths[yleap = isleap(y)]) {
 newy = y + days / DAYSPERNYEAR;
 if (days < 0)
 --newy;
 days -= (newy - y) * DAYSPERNYEAR +
 LEAPS_THRU_END_OF(newy - 1) -
 LEAPS_THRU_END_OF(y - 1);
 y = newy;
 }
 EfiTime->Year = (UINT16)y;
 ip = mon_lengths[yleap];
 for (EfiTime->Month = 0; days >= (INT64) ip[EfiTime->Month]; ++(EfiTime->Month))
 days = days - (INT64) ip[EfiTime->Month];
 EfiTime->Month++;  // adjust range to EFI conventions
 EfiTime->Day = (UINT8) (days + 1);
 }
 */


EFI_BLOCK_READ OrigBlockIoRead = NULL;
UINT64  gSleepImageOffset = 0;
UINT32  gSleepTime = 0;

//
// Available on all platforms, requires NMI bit handling.
//
#define R_PCH_RTC_INDEX           0x70
#define R_PCH_RTC_TARGET          0x71
#define R_PCH_RTC_EXT_INDEX       0x72
#define R_PCH_RTC_EXT_TARGET      0x73

//
// Available on Ivy Bridge and newer. Ignores NMI bit.
//
#define R_PCH_RTC_INDEX_ALT       0x74
#define R_PCH_RTC_TARGET_ALT      0x75
#define R_PCH_RTC_EXT_INDEX_ALT   0x76
#define R_PCH_RTC_EXT_TARGET_ALT  0x77

//
// RTC Memory bank size
//
#define RTC_BANK_SIZE             0x80

//
// RTC INDEX bit mask
//
#define RTC_DATA_MASK             0x7F
#define RTC_NMI_MASK              0x80

STATIC
UINT8
SimpleRtcRead (
               IN  UINT8  Offset
               )
{
  UINT8  RtcIndexPort;
  UINT8  RtcDataPort;
  UINT8  RtcIndexNmi;
  
  if (Offset < RTC_BANK_SIZE) {
    RtcIndexPort  = R_PCH_RTC_INDEX;
    RtcDataPort   = R_PCH_RTC_TARGET;
  } else {
    RtcIndexPort  = R_PCH_RTC_EXT_INDEX;
    RtcDataPort   = R_PCH_RTC_EXT_TARGET;
  }
  
  RtcIndexNmi = IoRead8 (RtcIndexPort) & RTC_NMI_MASK;
  IoWrite8 (RtcIndexPort, (Offset & RTC_DATA_MASK) | RtcIndexNmi);
  return IoRead8 (RtcDataPort);
}

STATIC
VOID
SimpleRtcWrite (
                IN UINT8 Offset,
                IN UINT8 Value
                )
{
  UINT8  RtcIndexPort;
  UINT8  RtcDataPort;
  UINT8  RtcIndexNmi;
  
  if (Offset < RTC_BANK_SIZE) {
    RtcIndexPort  = R_PCH_RTC_INDEX;
    RtcDataPort   = R_PCH_RTC_TARGET;
  } else {
    RtcIndexPort  = R_PCH_RTC_EXT_INDEX;
    RtcDataPort   = R_PCH_RTC_EXT_TARGET;
  }
  
  RtcIndexNmi = IoRead8 (RtcIndexPort) & RTC_NMI_MASK;
  IoWrite8 (RtcIndexPort, (Offset & RTC_DATA_MASK) | RtcIndexNmi);
  IoWrite8 (RtcDataPort, Value);
}

/** BlockIo->Read() override. */
EFI_STATUS
EFIAPI OurBlockIoRead (
                       IN EFI_BLOCK_IO_PROTOCOL          *This,
                       IN UINT32                         MediaId,
                       IN EFI_LBA                        Lba,
                       IN UINTN                          BufferSize,
                       OUT VOID                          *Buffer
                       )
{
  EFI_STATUS          Status;
  Status = OrigBlockIoRead(This, MediaId, Lba, BufferSize, Buffer);
  
  // Enter special processing only when gSleepImageOffset == 0, to avoid recursion when Boot/Log=true
  if (gSleepImageOffset == 0 && Status == EFI_SUCCESS && BufferSize >= sizeof(IOHibernateImageHeaderMin)) { //sizeof(IOHibernateImageHeaderMin)==96
    IOHibernateImageHeaderMin *Header;
    IOHibernateImageHeaderMinSnow *Header2;
    UINT32 BlockSize = 0;
    
    // Mark that we are executing, to avoid entering above phrase again, and don't add DBGs outside this scope, to avoid recursion
    gSleepImageOffset = (UINT64)-1;
    
    if (This->Media != NULL) {
      BlockSize = This->Media->BlockSize;
    } else {
      BlockSize = 512;
    }
    
    //   DBG("    OurBlockIoRead: Lba=%lx, Offset=%lx (BlockSize=%d)\n", Lba, MultU64x32(Lba, BlockSize), BlockSize);
    
    Header = (IOHibernateImageHeaderMin *) Buffer;
    Header2 = (IOHibernateImageHeaderMinSnow *) Buffer;
    //   DBG("    sig lion: %X\n", Header->signature);
    //   DBG("    sig snow: %X\n", Header2->signature);
    // DBG(" sig swap: %X\n", SwapBytes32(Header->signature));
    
    if (Header->signature == kIOHibernateHeaderSignature ||
        Header2->signature == kIOHibernateHeaderSignature) {
      gSleepImageOffset = MultU64x32(Lba, BlockSize);
      DBG("    got sleep image offset\n");
      machineSignature = ((IOHibernateImageHeaderMin*)Buffer)->machineSignature;
		DBG("     image has machineSignature =0x%llX\n", machineSignature);
      
      //save sleep time as lvs1974 suggested
      if (Header->signature == kIOHibernateHeaderSignature) {
        gSleepTime = Header->sleepTime;
      } else
        gSleepTime = 0;
      // return invalid parameter in case of success in order to prevent driver from caching our buffer
      return EFI_INVALID_PARAMETER;
    } else {
      DBG("    no valid sleep image offset was found\n");
      gSleepImageOffset = 0;
    }
  }
  
  return Status;
}

/** Get sleep image location (volume and name) */
VOID
GetSleepImageLocation(IN REFIT_VOLUME *Volume, REFIT_VOLUME **SleepImageVolume, CHAR16 **SleepImageName)
{
  EFI_STATUS          Status = EFI_NOT_FOUND;
  UINT8               *PrefBuffer = NULL;
  UINTN               PrefBufferLen = 0;
  TagPtr              PrefDict, dict, dict2, prop;
  CONST CHAR16              *PrefName = L"\\Library\\Preferences\\SystemConfiguration\\com.apple.PowerManagement.plist";
  CONST CHAR16              *PrefName2 = L"\\Library\\Preferences\\com.apple.PowerManagement.plist";
  CHAR16              *PrefName3 = NULL;
  CHAR16              *ImageName = NULL;
  REFIT_VOLUME        *ImageVolume = Volume;
  
  
  if (Volume->RootDir) {
    // find sleep image entry from plist
    Status = egLoadFile(Volume->RootDir, PrefName, &PrefBuffer, &PrefBufferLen);
    if (EFI_ERROR(Status)) {
      PrefName3 = PoolPrint(L"\\Library\\Preferences\\com.apple.PowerManagement.%s.plist", GuidBeToStr(&gUuid));
      Status = egLoadFile(Volume->RootDir, PrefName3, &PrefBuffer, &PrefBufferLen);
      if (EFI_ERROR(Status)) {
        Status = egLoadFile(Volume->RootDir, PrefName2, &PrefBuffer, &PrefBufferLen);
        if (!EFI_ERROR(Status)) {
          DBG("    read prefs %ls status=%s\n", PrefName2, strerror(Status));
        }
      } else {
        DBG("    read prefs %ls status=%s\n", PrefName3, strerror(Status));
      }
    } else {
      DBG("    read prefs %ls status=%s\n", PrefName, strerror(Status));
    }
  }
  
  if (!EFI_ERROR(Status)) {
    Status = ParseXML((const CHAR8*)PrefBuffer, &PrefDict, (UINT32)PrefBufferLen);
    if (!EFI_ERROR(Status)) {
      dict = GetProperty(PrefDict, "Custom Profile");
      if (dict) {
        dict2 = GetProperty(dict, "AC Power");
        if (dict2) {
          prop = GetProperty(dict2, "Hibernate File");
          if (prop && prop->type == kTagTypeString ) {
            CHAR16 *p;
            if (AsciiStrStr(prop->string, "/Volumes/")) {
              CHAR8 *VolNameStart = NULL, *VolNameEnd = NULL;
              CHAR16 *VolName = NULL;
              UINTN VolNameSize = 0;
              // Extract Volumes Name
              VolNameStart = AsciiStrStr(prop->string + 1, "/") + 1;
              if (VolNameStart) {
                VolNameEnd = AsciiStrStr(VolNameStart, "/");
                if (VolNameEnd) {
                  VolNameSize = (VolNameEnd - VolNameStart + 1) * sizeof(CHAR16);
                  if (VolNameSize > 0) {
                    VolName = (__typeof__(VolName))AllocateZeroPool(VolNameSize);
                  }
                }
              }
              if (VolName) {
                UnicodeSPrint(VolName, VolNameSize, L"%a", VolNameStart);
                ImageVolume = FindVolumeByName(VolName);
                if (ImageVolume) {
                  ImageName = PoolPrint(L"%a", VolNameEnd);
                } else {
                  ImageVolume = Volume;
                }
                FreePool(VolName);
              }
            } else if (AsciiStrStr(prop->string, "/var") && !AsciiStrStr(prop->string, "private")) {
              ImageName = PoolPrint(L"\\private%a", prop->string);
            } else {
              ImageName = PoolPrint(L"%a", prop->string);
            }
            p = ImageName;
            while (*p) {
              if (*p == L'/') {
                *p = L'\\';
              }
              p++;
            }
            DBG("    SleepImage name from pref: ImageVolume = '%ls', ImageName = '%ls'\n", ImageVolume->VolName, ImageName);
          }
        }
      }
    }
  }
  
  if (!ImageName) {
    ImageName = PoolPrint(L"\\private\\var\\vm\\sleepimage");
    DBG("    using default sleep image name = %ls\n", ImageName);
  }
  if (PrefBuffer) {
    FreePool(PrefBuffer); //allocated by egLoadFile
  }
  if (PrefName3) {
    FreePool(PrefName3); //allocated by PoolPrint
  }
  *SleepImageVolume = ImageVolume;
  *SleepImageName = ImageName;
}


/** Returns byte offset of sleepimage on the whole disk or 0 if not found or error.
 *
 * To avoid messing with HFS+ format, we'll use the trick with overriding
 * BlockIo->Read() of the disk and then read first bytes of the sleepimage
 * through file system driver. And then we'll detect block delivered by BlockIo
 * and calculate position from there.
 * It's for hack after all :)
 */
UINT64
GetSleepImagePosition (IN REFIT_VOLUME *Volume, REFIT_VOLUME **SleepImageVolume)
{
  EFI_STATUS          Status = EFI_SUCCESS;
  EFI_FILE            *File = NULL;
  VOID                *Buffer;
  UINTN               BufferSize;
  CHAR16              *ImageName;
  REFIT_VOLUME        *ImageVolume;
  
  if (!Volume) {
    DBG("    no volume to get sleepimage\n");
    return 0;
  }
  
  if (Volume->WholeDiskBlockIO == NULL) {
    DBG("    no disk BlockIo\n");
    return 0;
  }
  
  // If IsSleepImageValidBySignature() was used, then we already have that offset
  if (Volume->SleepImageOffset != 0) {
    if (SleepImageVolume != NULL) {
      // Update caller's SleepImageVolume when requested
      GetSleepImageLocation(Volume, SleepImageVolume, &ImageName);
    }
	  DBG("    returning previously calculated offset: %llx\n", Volume->SleepImageOffset);
    return Volume->SleepImageOffset;
  }
  
  // Get sleepimage name and volume
  GetSleepImageLocation(Volume, &ImageVolume, &ImageName);
  
  if (ImageVolume->RootDir) {
    // Open sleepimage
    Status = ImageVolume->RootDir->Open(ImageVolume->RootDir, &File, ImageName, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
      DBG("    sleepimage not found -> %s\n", strerror(Status));
      return 0;
    }
  }
  
  // We want to read the first 512 bytes from sleepimage
  BufferSize = 512;
  Buffer = (__typeof__(Buffer))AllocatePool(BufferSize);
  if (Buffer == NULL) {
    DBG("    could not allocate buffer for sleepimage\n");
    return 0;
  }
  
  //  DBG("    Reading first %d bytes of sleepimage ...\n", BufferSize);
  
  if (!ImageVolume->WholeDiskBlockIO) {
    DBG("     can not get whole disk\n");
    if (Buffer) {
      FreePool(Buffer);
    }
    return 0;
  }
  
  // Override disk BlockIo
  OrigBlockIoRead = ImageVolume->WholeDiskBlockIO->ReadBlocks;
  ImageVolume->WholeDiskBlockIO->ReadBlocks = OurBlockIoRead;
  gSleepImageOffset = 0; //used as temporary global variable to pass our value
  Status = File->Read(File, &BufferSize, Buffer);
  
  // Restore original disk BlockIo
  ImageVolume->WholeDiskBlockIO->ReadBlocks = OrigBlockIoRead;
  
  // OurBlockIoRead always returns invalid parameter in order to avoid driver caching, so that is a good value
  if (Status == EFI_INVALID_PARAMETER) {
    Status = EFI_SUCCESS;
  }
  //  DBG("    Reading completed -> %s\n", strerror(Status));
  
  // Close sleepimage
  File->Close(File);
  
  // We don't use the buffer, as actual signature checking is being done by OurBlockIoRead
  if (Buffer) {
    FreePool(Buffer);
  }
  
  if (EFI_ERROR(Status)) {
    DBG("     can not read sleepimage -> %s\n", strerror(Status));
    return 0;
  }
  
  // We store SleepImageOffset, in case our BlockIoRead does not execute again on next read due to driver caching.
  if (gSleepImageOffset != 0) {
	  DBG("     sleepimage offset acquired successfully: %llx\n", gSleepImageOffset);
    ImageVolume->SleepImageOffset = gSleepImageOffset;
  } else {
    DBG("     sleepimage offset could not be acquired\n");
  }
  
  if (SleepImageVolume != NULL) {
    // Update caller's SleepImageVolume when requested
    *SleepImageVolume = ImageVolume;
  }
  return gSleepImageOffset;
}


/** Returns TRUE if /private/var/vm/sleepimage exists
 *  and it's modification time is close to volume modification time).
 */
BOOLEAN
IsSleepImageValidBySleepTime (IN REFIT_VOLUME *Volume)
{
  EFI_STATUS          Status;
  VOID                *Buffer;
  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  HFSPlusVolumeHeaderMin  *HFSHeader;
  UINT32              HFSVolumeModifyDate;
  INTN                TimeDelta;
  INTN                Pages; // = 1;
  //EFI_TIME            ImageModifyTime;
  //EFI_TIME            *TimePtr;
  //EFI_TIME            HFSVolumeModifyTime;
  
  DBG("     gSleepTime: %d\n", gSleepTime);
  //fsw_efi_decode_time(&ImageModifyTime, gSleepTime);
  //TimePtr = &ImageModifyTime;
  //DBG(" in EFI: %d-%d-%d %d:%d:%d\n", TimePtr->Year, TimePtr->Month, TimePtr->Day, TimePtr->Hour, TimePtr->Minute, TimePtr->Second);
  
  //
  // Get HFS+ volume nodification time
  //
  // use 4KB aligned page to avoid possible issues with BlockIo buffer alignment
  BlockIo = Volume->BlockIO;
  Pages = EFI_SIZE_TO_PAGES(BlockIo->Media->BlockSize);
  Buffer = (__typeof__(Buffer))AllocatePages(Pages);
  if (Buffer == NULL) {
    return FALSE;
  }
  Status = BlockIo->ReadBlocks(BlockIo, BlockIo->Media->MediaId, 2, BlockIo->Media->BlockSize, Buffer);
  if (EFI_ERROR(Status)) {
    DBG("     can not read HFS+ header -> %s\n", strerror(Status));
    FreePages(Buffer, Pages);
    return FALSE;
  }
  HFSHeader = (HFSPlusVolumeHeaderMin *)Buffer;
  HFSVolumeModifyDate = SwapBytes32(HFSHeader->modifyDate);
  HFSVolumeModifyDate = mac_to_posix(HFSVolumeModifyDate);
  DBG("     HFS+ volume modifyDate: %d\n", HFSVolumeModifyDate);
  //fsw_efi_decode_time(&HFSVolumeModifyTime, mac_to_posix(HFSVolumeModifyDate));
  //TimePtr = &HFSVolumeModifyTime;
  //DBG(" in EFI: %d-%d-%d %d:%d:%d\n", TimePtr->Year, TimePtr->Month, TimePtr->Day, TimePtr->Hour, TimePtr->Minute, TimePtr->Second);
  
  
  //
  // Check that sleepimage is not more then 5 secs older then volume modification date
  // Idea is from Chameleon
  //
  TimeDelta = HFSVolumeModifyDate - (INTN)gSleepTime;
	DBG("     image older then volume: %lld sec\n", TimeDelta);
  if (TimeDelta > 5 /*|| TimeDelta < -5 */) {
    //Slice - if image newer then volume it should be OK
    DBG("     image too old\n");
    FreePages(Buffer, Pages);
    return FALSE;
  }
	DBG("     machineSignature from FACS =0x%llX\n", machineSignature);
  //  machineSignature = ((IOHibernateImageHeaderMin*)Buffer)->machineSignature;
  //  DBG("     image has machineSignature =0x%X\n", machineSignature);
  FreePages(Buffer, Pages);
  return TRUE;
}

/** Returns TRUE if /private/var/vm/sleepimage exists
 *  and it's signature is kIOHibernateHeaderSignature.
 */
BOOLEAN
IsSleepImageValidBySignature (IN REFIT_VOLUME *Volume)
{
  // We'll have to detect offset here also in case driver caches
  // some data and stops us from detecting offset later.
  // So, make first call to GetSleepImagePosition() now.
  DBG("    Check sleep image 'by signature':\n");
  return (GetSleepImagePosition (Volume, NULL) != 0);
}

UINT16 PartNumForVolume(REFIT_VOLUME *Volume)
{
  UINT16 PartNum = 0; //if not found then zero mean whole disk
  HARDDRIVE_DEVICE_PATH       *HdPath     = NULL;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath = Volume->DevicePath;
  
  while (DevicePath && !IsDevicePathEnd (DevicePath)) {
    if ((DevicePathType (DevicePath) == MEDIA_DEVICE_PATH) &&
        (DevicePathSubType (DevicePath) == MEDIA_HARDDRIVE_DP)) {
      HdPath = (HARDDRIVE_DEVICE_PATH *)DevicePath;
      break;
    }
    DevicePath = NextDevicePathNode (DevicePath);
  }
  
  if (HdPath != NULL) {
    PartNum = (UINT16)(HdPath->PartitionNumber);
  }
  return PartNum;
}

REFIT_VOLUME *FoundParentVolume(REFIT_VOLUME *Volume)
{
  UINTN         VolumeIndex;
  REFIT_VOLUME  *Volume1 = NULL;
  INT16         SearchPartNum = PartNumForVolume(Volume);
  
  if (SearchPartNum < 3) {
    // 0 - whole disk
    // 1 - ESP
    // 2 - a partition to search
    // 3 - minimum # for recovery
    DBG("    the volume has wrong partition number %d\n", SearchPartNum);
    return NULL; //don't search!
  }
  
  for (VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++) {
    Volume1 = &Volumes[VolumeIndex];
    if (Volume1 != Volume &&
        Volume1->WholeDiskBlockIO == Volume->WholeDiskBlockIO) {
      if (PartNumForVolume(Volume1) == SearchPartNum - 1) {
        return Volume1;
      }
    }
  }
  return NULL;
}


STATIC CHAR16 OffsetHexStr[100];

/** Returns TRUE if given OSX on given volume is hibernated. */
BOOLEAN
IsOsxHibernated (IN LOADER_ENTRY *Entry)
{
  EFI_STATUS      Status  = EFI_SUCCESS;
  UINTN           Size            = 0;
  UINT8           *Data           = NULL;
  //  REFIT_VOLUME    *ThisVolume     = Entry->Volume;
  REFIT_VOLUME    *Volume         = Entry->Volume;
  EFI_GUID        *BootGUID       = NULL;
  BOOLEAN         ret             = FALSE;
  UINT8           *Value          = NULL;
  
  //  UINTN           VolumeIndex;
  EFI_GUID        *VolumeUUID;
  //  CHAR16          *VolumeUUIDStr  = NULL;
  
  if (!Volume) {
    return FALSE;
  }
  /*
   Status = GetRootUUID(ThisVolume);  // already done
   if (!EFI_ERROR(Status)) { //this is set by scan loaders only for Recovery volumes
   FP.1EE01920[\].Open('com.apple.boot.R', 1, 0) = Not Found
   FP.1EE01920[\].Open('com.apple.boot.P', 1, 0) = Not Found
   FP.1EE01920[\].Open('com.apple.boot.S', 1, 0) = EFI_SUCCESS -> FP.1EE01A20[\com.apple.boot.S]
   FP.1EE01A20[\com.apple.boot.S].Open('Library\Preferences\SystemConfiguration\com.apple.Boot.plist', 1, 0) = EFI_SUCCESS -> FP.1F7F7F20[\com.apple.boot.S\Library\Preferences\SystemConfiguration\com.apple.Boot.plist]
   FP.1F7F7F20[\com.apple.boot.S\Library\Preferences\SystemConfiguration\com.apple.Boot.plist].GetInfo(gEfiFileInfoGuid, 122, 1F7FAE18) = Success
   FP.1F7F7F20[\com.apple.boot.S\Library\Preferences\SystemConfiguration\com.apple.Boot.plist].Read(309, 1CF27018) = Success
   FP.1F7F7F20[\com.apple.boot.S\Library\Preferences\SystemConfiguration\com.apple.Boot.plist].Close() = Success
   
   <dict>
   <key>Kernel Flags</key>
   <string></string>
   <key>Root UUID</key>
   <string>D6E74829-F4A5-3CBA-B8EE-D0B6E40E4D53</string>
   </dict>
   
   //   Volume = from UUID
   //   We can obtain Partition UUID but not Volume UUID
   
   Status = EFI_NOT_FOUND;
   for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
   Volume = Volumes[VolumeIndex];
   VolumeUUID = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
   DBG("Volume[%d] has UUID=%ls\n", VolumeIndex, GuidLEToStr(VolumeUUID));
   if (CompareGuid(&ThisVolume->RootUUID, VolumeUUID)) {
   DBG("found root volume at path: %ls\n", FileDevicePathToStr(Volume->DevicePath));
   Status = EFI_SUCCESS;
   break;
   }
   }
   if (EFI_ERROR(Status)) {
   Volume = ThisVolume;
   DBG("cant find volume with UUID=%ls\n", GuidLEToStr(&ThisVolume->RootUUID));
   }
   
   DBG("    got RootUUID %s\n", strguid(&ThisVolume->RootUUID));
   
   VolumeUUIDStr = GuidLEToStr(&ThisVolume->RootUUID);
   DBG("    Search for Volume with UUID: %ls\n", VolumeUUIDStr);
   if (VolumeUUIDStr) {
   FreePool(VolumeUUIDStr);
   }
   
   Volume = FoundParentVolume(ThisVolume);
   if (Volume) {
   DBG("    Found parent Volume with name %ls\n", Volume->VolName);
   if (Volume->RootDir == NULL) {
   return FALSE;
   }
   } else {
   DBG("    Parent Volume not found, use this one\n");
   Volume = ThisVolume;
   }
   }
   */
  //For tests
  /*  Status = GetRootUUID(Volume);
   if (!EFI_ERROR(Status)) {
   EFI_GUID TmpGuid;
   CHAR16 *TmpStr = NULL;
   CHAR16 *Ptr = GuidLEToStr(&Volume->RootUUID);
   DBG("got str=%ls\n", Ptr);
   Status = StrToGuidLE (Ptr, &TmpGuid);
   if (EFI_ERROR(Status)) {
   DBG("    cant convert Str %ls to GUID\n", Ptr);
   } else {
   TmpStr = PoolPrint(L"%s", strguid(&TmpGuid));  //PoolPrint не работает!!!
   DBG("got the guid %ls\n", TmpStr);
   CopyMem((VOID*)Ptr, TmpStr, StrSize(TmpStr));
   DBG("fter CopyMem: %ls\n", Ptr);
   FreePool(TmpStr);
   }
   }
   */
  
  //if sleep image is good but OSX was not hibernated.
  //or we choose "cancel hibernate wake" then it must be canceled
  if (GlobalConfig.NeverHibernate) {
    DBG("    hibernated: set as never\n");
    return FALSE;
  }
  
  DBG("    Check if volume Is Hibernated:\n");
  
  if (!GlobalConfig.StrictHibernate) {
    // CloverEFI or UEFI with EmuVariable
    if (IsSleepImageValidBySignature(Volume)) {
      if ((gSleepTime == 0) || IsSleepImageValidBySleepTime(Volume)) {
        DBG("    hibernated: yes\n");
        ret = TRUE;
      } else {
        DBG("    hibernated: no - time\n");
        return FALSE;
      }
      //    IsHibernate = TRUE;
    } else {
      DBG("    hibernated: no - sign\n");
      return FALSE; //test
    }
  }
  
  if (!gFirmwareClover &&
      (!gDriversFlags.EmuVariableLoaded || GlobalConfig.HibernationFixup)) {
    DBG("    UEFI with NVRAM? ");
    Status = GetVariable2 (L"Boot0082", &gEfiGlobalVariableGuid, (VOID**)&Data, &Size);
    if (EFI_ERROR(Status))  {
      DBG(" no, Boot0082 not exists\n");
      ret = FALSE;
    } else {
      DBG("yes\n");
      ret = TRUE;
      //1. Parse Media Device Path from Boot0082 load option
      //Cut Data pointer by 0x08 up to DevicePath
      // Data += 0x08;
      // Size -= 0x08;
      //We get starting offset of media device path, and then jumping 24 bytes to GUID start
      // BootGUID = (EFI_GUID*)(Data + NodeParser(Data, Size, 0x04) + 0x18);
      
      /* APFS Hibernation support*/
      //Check that current volume is APFS
      if ((VolumeUUID = APFSPartitionUUIDExtract(Volume->DevicePath)) != NULL) {
        //BootGUID = (EFI_GUID*)(Data + Size - 0x14);
        BootGUID = (EFI_GUID*)ScanGuid(Data, Size, VolumeUUID);
        //DBG("    APFS Boot0082 points to UUID:%s\n", strguid(BootGUID));
      } else {
        //BootGUID = (EFI_GUID*)(Data + Size - 0x16);
        VolumeUUID = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
        if (VolumeUUID != NULL) {
          BootGUID = (EFI_GUID*)ScanGuid(Data, Size, VolumeUUID);
          //DBG("    Boot0082 points to UUID:%s\n", strguid(BootGUID));
        }
      }
      //DBG("    Volume has PartUUID=%s\n", strguid(VolumeUUID));
      if (BootGUID != NULL && VolumeUUID != NULL && !CompareGuid(BootGUID, VolumeUUID)) {
        ret = FALSE;
      } else  {
        DBG("    Boot0082 points to Volume with UUID:%s\n", strguid(BootGUID));
        
        //3. Checks for boot-image exists
        if (GlobalConfig.StrictHibernate) {
          /*
           Variable NV+RT+BS '7C436110-AB2A-4BBB-A880-FE41995C9F82:boot-image' DataSize = 0x3A
           00000000: 02 01 0C 00 D0 41 03 0A-00 00 00 00 01 01 06 00  *.....A..........*
           00000010: 02 1F 03 12 0A 00 00 00-00 00 00 00 04 04 1A 00  *................*
           00000020: 33 00 36 00 63 00 34 00-64 00 64 00 63 00 30 00  *3.6.c.4.d.d.c.0.*
           00000030: 30 00 30 00 00 00 7F FF-04 00                    *0.0.......*
           02 - ACPI_DEVICE_PATH
           01 - ACPI_DP
           0C - 4 bytes
           00 D0 41 03 - PNP0A03
           
           // FileVault2
           4:609  0:000      Boot0082 points to Volume with UUID:BA92975E-E2FB-48E6-95CC-8138B286F646
           4:609  0:000      boot-image before: PciRoot(0x0)\Pci(0x1F,0x2)\Sata(0x5,0x0,0x0)\25593c7000:A82E84C6-9DD6-49D6-960A-0F4C2FE4851C
           */
          Status = GetVariable2 (L"boot-image", &gEfiAppleBootGuid, (VOID**)&Value, &Size);
          if (EFI_ERROR(Status)) {
            // leave it as is
            DBG("    boot-image not found while we want StrictHibernate\n");
            ret = FALSE;
          } else {
            
            EFI_DEVICE_PATH_PROTOCOL    *BootImageDevPath;
            //              UINTN                       Size;
            CHAR16                      *Ptr = (CHAR16*)&OffsetHexStr[0];
            
            DBG("    boot-image before: %ls\n", FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL*)Value));
            UnicodeSPrint(OffsetHexStr, sizeof(OffsetHexStr), L"%s", (CHAR16 *)(Value + 0x20));
            //      DBG("OffsetHexStr=%ls\n", OffsetHexStr);
            while ((*Ptr != L':') && (*Ptr != 0)) {
              Ptr++;
            }
            //       DBG(" have ptr=%p, in Str=%p, text:%ls\n", Ptr, &OffsetHexStr, Ptr);
            if (*Ptr++ == L':') {
              //Convert BeUUID to LeUUID
              //Ptr points to begin L"A82E84C6-9DD6-49D6-960A-0F4C2FE4851C"
              EFI_GUID TmpGuid;
              CHAR16 *TmpStr = NULL;
              
              ResumeFromCoreStorage = TRUE;
              //         DBG("got str=%ls\n", Ptr);
              Status = StrToGuidLE (Ptr, &TmpGuid);
              if (EFI_ERROR(Status)) {
                DBG("    cant convert Str %ls to GUID\n", Ptr);
              } else {
                //TmpStr = PoolPrint(L"%s", strguid(&TmpGuid));
                TmpStr =  GuidLEToStr(&TmpGuid);
                //           DBG("got the guid %ls\n", TmpStr);
                CopyMem((VOID*)Ptr, TmpStr, StrSize(TmpStr));
                FreePool(TmpStr);
              }
            }
            if (StrCmp(gST->FirmwareVendor, L"INSYDE Corp.") != 0) {
              // skip this on INSYDE UEFI
              UINT8 SataNum = Value[22];
              FreePool(Value);
              BootImageDevPath = FileDevicePath(Volume->WholeDiskDeviceHandle, OffsetHexStr);
              //  DBG(" boot-image device path:\n");
              Size = GetDevicePathSize(BootImageDevPath);
              Value = (UINT8*)BootImageDevPath;
              DBG("    boot-image after: %ls\n", FileDevicePathToStr(BootImageDevPath));
              //Apple's device path differs from UEFI BIOS device path that will be used by boot.efi
              //Value[6] = 8; //Acpi(PNP0A08,0)
              Value[22] = SataNum;
              Value[24] = 0xFF;
              Value[25] = 0xFF;
              DBG("    boot-image corrected: %ls\n", FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL*)Value));
              PrintBytes(Value, Size);
              
              Status = gRT->SetVariable(L"boot-image", &gEfiAppleBootGuid,
                                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                        Size , Value);
              if (EFI_ERROR(Status)) {
                DBG(" can not write boot-image -> %s\n", strerror(Status));
                ret = FALSE;
              }
            }
          }
        } //else boot-image will be created
      }
      FreePool(Data);
    }
  }
  if (Value) {
    FreePool(Value);
  }
  return ret;
}

/** Prepares nvram vars needed for boot.efi to wake from hibernation:
 *  boot-switch-vars and boot-image.
 *
 * Normally those vars should be set by kernel
 * boot-switch-vars: structure with image encription key
 * boot-image: device path like Acpi(PNP0A03,0)/Pci(1f,2)/Sata(2,0)/File(56b99e000)
 *  where Acpi(PNP0A03,0)/Pci(1f,2)/Sata(2,0) points to the disk containing sleepimage
 *  and File(56b99e000) contains hex position (in bytes) of the beginning of the sleepimage
 *
 * Since boot-switch-vars is not present in CloverEFI or with EmuVar driver (no real NVRAM) but also not on UEFI hack
 * (not written by the kernel for some reason), and since boot-image is also not present in CloverEFI
 * and on UEFI hack device path as set by kernel can be different in some bytes from the device path
 * reported by UEFI, we'll compute and set both vars here.
 *
 * That's the only way for CloverEFI and should be OK for UEFI hack also.
 */

BOOLEAN
PrepareHibernation (IN REFIT_VOLUME *Volume)
{
  EFI_STATUS                  Status;
  UINT64                      SleepImageOffset;
  EFI_DEVICE_PATH_PROTOCOL    *BootImageDevPath;
  UINTN                       Size = 0;
  VOID                        *Value = NULL;
  AppleRTCHibernateVars       RtcVars;
  UINT8                       *VarData = NULL;
  REFIT_VOLUME                *SleepImageVolume;
  UINT32                      Attributes;
  BOOLEAN                     HasIORTCVariables = FALSE;
  BOOLEAN                     HasHibernateInfo = FALSE;
  BOOLEAN                     HasHibernateInfoInRTC = FALSE;
  
  DBG("PrepareHibernation:\n");
  
  if (!GlobalConfig.StrictHibernate) {
    // Find sleep image offset
    SleepImageOffset = GetSleepImagePosition (Volume, &SleepImageVolume);
	  DBG(" SleepImageOffset: %llx\n", SleepImageOffset);
    if (SleepImageOffset == 0 || SleepImageVolume == NULL) {
      DBG(" sleepimage offset not found\n");
      return FALSE;
    }
    
    // Set boot-image var
    UnicodeSPrint(OffsetHexStr, sizeof(OffsetHexStr), L"%lx", SleepImageOffset);
    BootImageDevPath = FileDevicePath(SleepImageVolume->WholeDiskDeviceHandle, OffsetHexStr);
    //  DBG(" boot-image device path:\n");
    Size = GetDevicePathSize(BootImageDevPath);
    VarData = (UINT8*)BootImageDevPath;
    PrintBytes(VarData, Size);
    DBG("boot-image before: %ls\n", FileDevicePathToStr(BootImageDevPath));
    //      VarData[6] = 8;
    
    //  VarData[24] = 0xFF;
    //  VarData[25] = 0xFF;
    //  DBG("boot-image corrected: %ls\n", FileDevicePathToStr(BootImageDevPath));
    
    Status = gRT->SetVariable(L"boot-image", &gEfiAppleBootGuid,
                              EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                              Size , BootImageDevPath);
    if (EFI_ERROR(Status)) {
      DBG(" can not write boot-image -> %s\n", strerror(Status));
      return FALSE;
    }
  }
  
  // now we should delete boot0082 to do hibernate only once
  Status = DeleteBootOption(0x82);
  if (EFI_ERROR(Status)) {
    DBG("Options 0082 was not deleted: %s\n", strerror(Status));
  }
  
  //
  // If legacy boot-switch-vars exists (NVRAM working), then use it.
  //
  Status = GetVariable2 (L"boot-switch-vars", &gEfiAppleBootGuid, &Value, &Size);
  if (!EFI_ERROR (Status)) {
    //
    // Leave it as is.
    //
    DBG(" boot-switch-vars present\n");
    ZeroMem (Value, Size);
    gBS->FreePool (Value);
    return TRUE;
  }
  
  //
  // Work with RTC memory if allowed.
  //
  if (GlobalConfig.RtcHibernateAware) {
    UINT8  Index;
    UINT8 *RtcRawVars = (UINT8 *)&RtcVars;
    for (Index = 0; Index < sizeof(AppleRTCHibernateVars); Index++) {
      RtcRawVars[Index] = SimpleRtcRead (Index + 128);
    }
    
    HasHibernateInfoInRTC = (RtcVars.signature[0] == 'A' && RtcVars.signature[1] == 'A' &&
    RtcVars.signature[2] == 'P' && RtcVars.signature[3] == 'L');
    HasHibernateInfo = HasHibernateInfoInRTC;
    //
    // If RTC variables is still written to NVRAM (and RTC is broken).
    // Prior to 10.13.6.
    //
    Status = GetVariable2 (L"IOHibernateRTCVariables", &gEfiAppleBootGuid, &Value, &Size);
	  DBG("get IOHR variable status=%s, size=%llu, RTC info=%d\n", strerror(Status), Size, HasHibernateInfoInRTC);
    if (!HasHibernateInfo && !EFI_ERROR (Status) && Size == sizeof (RtcVars)) {
      CopyMem (RtcRawVars, Value, sizeof (RtcVars));
      HasHibernateInfo = (RtcVars.signature[0] == 'A' && RtcVars.signature[1] == 'A' &&
      RtcVars.signature[2] == 'P' && RtcVars.signature[3] == 'L');
    }
    
    //
    // Erase RTC variables in NVRAM.
    //
    if (!EFI_ERROR (Status)) {
      Status = gRT->SetVariable (L"IOHibernateRTCVariables", &gEfiAppleBootGuid,
                                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                 0, NULL);
      ZeroMem (Value, Size);
      gBS->FreePool (Value);
    }
    
    //
    // Convert RTC data to boot-key and boot-signature
    //
    if (HasHibernateInfo) {
      gRT->SetVariable (L"boot-image-key", &gEfiAppleBootGuid,
                        EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (RtcVars.wiredCryptKey), RtcVars.wiredCryptKey);
      gRT->SetVariable (L"boot-signature", &gEfiAppleBootGuid,
                        EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (RtcVars.booterSignature), RtcVars.booterSignature);
      DBG("variables boot-image-key and boot-signature saved\n");
    }
    
    //
    // Erase RTC memory similarly to AppleBds.
    //
    if (HasHibernateInfoInRTC) {
      ZeroMem (RtcRawVars, sizeof(AppleRTCHibernateVars));
      RtcVars.signature[0] = 'D';
      RtcVars.signature[1] = 'E';
      RtcVars.signature[2] = 'A';
      RtcVars.signature[3] = 'D';
      
      for (Index = 0; Index < sizeof(AppleRTCHibernateVars); Index++) {
        SimpleRtcWrite (Index + 128, RtcRawVars[Index]);
      }
    }
    
    //
    // We have everything we need now.
    //
    if (HasHibernateInfo) {
      return TRUE;
    }
  }
  
  //
  // Fallback to legacy hibernation support if any.
  // if IOHibernateRTCVariables exists (NVRAM working), then copy it to boot-switch-vars
  // else (no NVRAM) set boot-switch-vars to dummy one
  //
  Value = NULL;
  Status = GetVariable2 (L"IOHibernateRTCVariables", &gEfiAppleBootGuid, &Value, &Size);
  if (!EFI_ERROR (Status)) {
    DBG(" IOHibernateRTCVariables found - will be used as boot-switch-vars\n");
    //
    // Delete IOHibernateRTCVariables.
    //
    Status = gRT->SetVariable(L"IOHibernateRTCVariables", &gEfiAppleBootGuid,
                              EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                              0, NULL);
    HasIORTCVariables = TRUE;
  } else {
    //
    // No NVRAM support, trying unencrypted.
    //
    DBG(" setting dummy boot-switch-vars\n");
    Size = sizeof(RtcVars);
    Value = &RtcVars;
    SetMem(&RtcVars, Size, 0);
    RtcVars.signature[0] = 'A';
    RtcVars.signature[1] = 'A';
    RtcVars.signature[2] = 'P';
    RtcVars.signature[3] = 'L';
    RtcVars.revision     = 1;
  }
  
  //
  // boot-switch-vars should not be non volatile for security reasons
  // For now let's preserve old behaviour without RtcHibernateAware for compatibility reasons.
  //
  Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  if (!GlobalConfig.RtcHibernateAware) {
    Attributes |= EFI_VARIABLE_NON_VOLATILE;
  }
  
  Status = gRT->SetVariable(L"boot-switch-vars", &gEfiAppleBootGuid,
                            Attributes,
                            Size, Value);
  
  //
  // Erase written boot-switch-vars buffer.
  //
  ZeroMem (Value, Size);
  if (HasIORTCVariables) {
    gBS->FreePool (Value);
  }
  
  if (EFI_ERROR(Status)) {
    DBG(" can not write boot-switch-vars -> %s\n", strerror(Status));
    return FALSE;
  }
  
  return TRUE;
}


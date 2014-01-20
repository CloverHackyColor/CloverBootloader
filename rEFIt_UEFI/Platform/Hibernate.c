/*
 *  Hibernate.c
 *
 *  Created by dmazar, 01.2014.
 *
 *  Hibernate support.
 *
 */


#include "Platform.h"


#ifndef DEBUG_ALL
#define DEBUG_HIB 1
#else
#define DEBUG_HIB DEBUG_ALL
#endif

#if DEBUG_HIB == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_HIB, __VA_ARGS__);
//#define DBG(...) AsciiPrint(__VA_ARGS__);
#endif


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
    UINT64	imageSize;
    UINT64	image1Size;
    
    UINT32	restore1CodePhysPage;
    UINT32    reserved1;
    UINT64	restore1CodeVirt;
    UINT32	restore1PageCount;
    UINT32	restore1CodeOffset;
    UINT32	restore1StackOffset;
    
    UINT32	pageCount;
    UINT32	bitmapSize;
    
    UINT32	restore1Sum;
    UINT32	image1Sum;
    UINT32	image2Sum;
    
    UINT32	actualRestore1Sum;
    UINT32	actualImage1Sum;
    UINT32	actualImage2Sum;
    
    UINT32	actualUncompressedPages;
    UINT32	conflictCount;
    UINT32	nextFree;
    
    UINT32	signature;
    UINT32	processorFlags;
} IOHibernateImageHeaderMin;

typedef struct _IOHibernateImageHeaderMinSnow
{
  UINT64	imageSize;
  UINT64	image1Size;

  UINT32	restore1CodePhysPage;
  UINT32	restore1PageCount;
  UINT32	restore1CodeOffset;
  UINT32	restore1StackOffset;

  UINT32	pageCount;
  UINT32	bitmapSize;

  UINT32	restore1Sum;
  UINT32	image1Sum;
  UINT32	image2Sum;

  UINT32	actualRestore1Sum;
  UINT32	actualImage1Sum;
  UINT32	actualImage2Sum;

  UINT32	actualUncompressedPages;
  UINT32	conflictCount;
  UINT32	nextFree;

  UINT32	signature;
  UINT32	processorFlags;
} IOHibernateImageHeaderMinSnow;


typedef struct _AppleRTCHibernateVars
{
    UINT8     signature[4];
    UINT32    revision;
    UINT8	  booterSignature[20];
    UINT8	  wiredCryptKey[16];
} AppleRTCHibernateVars;


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

VOID fsw_efi_decode_time(OUT EFI_TIME *EfiTime, IN UINT32 UnixTime)
{
    INTN         days, rem;
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




/** Prints Number of bytes in a row (hex and ascii). Row size is MaxNumber. */
VOID
EFIAPI
PrintBytesRow(IN CHAR8 *Bytes, IN UINTN Number, IN UINTN MaxNumber)
{
	UINTN	Index;
	
	// print hex vals
	for (Index = 0; Index < Number; Index++) {
		DBG("%02x ", (UINT8)Bytes[Index]);
	}
	
	// pad to MaxNumber if needed
	for (; Index < MaxNumber; Index++) {
		DBG("   ");
	}
	
	DBG("| ");
	
	// print ASCII
	for (Index = 0; Index < Number; Index++) {
		if (Bytes[Index] >= 0x20 && Bytes[Index] <= 0x7e) {
			DBG("%c", (CHAR16)Bytes[Index]);
		} else {
			DBG("%c", L'.');
		}
	}
	
	DBG("\n");
}

/** Prints series of bytes. */
VOID
EFIAPI
PrintBytes(IN CHAR8 *Bytes, IN UINTN Number)
{
	UINTN	Index;
	
	for (Index = 0; Index < Number; Index += 16) {
		PrintBytesRow(Bytes + Index, (Index + 16 < Number ? 16 : Number - Index), 16);
	}
}




/** Returns TRUE if given OSX on given volume is hibernated
 *  (/private/var/vm/sleepimage exists and it's modification time is close to volume modification time).
 */
BOOLEAN
IsOsxHibernated (IN REFIT_VOLUME *Volume)
{
  EFI_STATUS          Status;
  EFI_FILE            *File;
  EFI_FILE_INFO       *FileInfo;
  EFI_TIME            *TimePtr;
  EFI_TIME            ImageModifyTime;
  VOID                *Buffer;
  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  HFSPlusVolumeHeaderMin  *HFSHeader;
  EFI_TIME            HFSVolumeModifyTime;
  UINT32              HFSVolumeModifyDate;
  INTN                TimeDiffMs;
  UINTN                     dataSize            = 0;
  UINT8                     *data               = NULL;
    
  //
  // Check for sleepimage and get it's info
  //
  DBG("OsxIsHibernated:\n");
  Status = Volume->RootDir->Open(Volume->RootDir, &File, L"\\private\\var\\vm\\sleepimage", EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    DBG(" sleepimage not found -> %r\n", Status);
    return FALSE;
  }
  FileInfo = EfiLibFileInfo(File);
  if (FileInfo == NULL) {
    DBG(" sleepimage info error\n");
    File->Close(File);
    return FALSE;
  }
  //CopyMem(&ImageModifyTime, &FileInfo->ModificationTime, sizeof(EFI_TIME));
  CopyMem(&ImageModifyTime, &FileInfo->LastAccessTime, sizeof(EFI_TIME));
  TimePtr = &FileInfo->CreateTime;
  DBG(" CreateTime: %d-%d-%d %d:%d:%d\n", TimePtr->Year, TimePtr->Month, TimePtr->Day, TimePtr->Hour, TimePtr->Minute, TimePtr->Second);
  TimePtr = &FileInfo->LastAccessTime;
  DBG(" LastAccessTime: %d-%d-%d %d:%d:%d\n", TimePtr->Year, TimePtr->Month, TimePtr->Day, TimePtr->Hour, TimePtr->Minute, TimePtr->Second);
  TimePtr = &ImageModifyTime;
  DBG(" ModificationTime: %d-%d-%d %d:%d:%d\n", TimePtr->Year, TimePtr->Month, TimePtr->Day, TimePtr->Hour, TimePtr->Minute, TimePtr->Second);
  File->Close(File);
  FreePool(FileInfo);
  
  //
  // Get HFS+ volume nodification time
  //
  // use 4KB aligned page to not have issues with BlockIo buffer alignment
  Buffer = AllocatePages(1);
  if (Buffer == NULL) {
    return FALSE;
  }
  // Note: assuming 512K blocks
  BlockIo = Volume->BlockIO;
  Status = BlockIo->ReadBlocks(BlockIo, BlockIo->Media->MediaId, 2, 512, Buffer);
  if (EFI_ERROR(Status)) {
    DBG(" can not read HFS+ header -> %r\n", Status);
    FreePages(Buffer, 1);
    return FALSE;
  }
  HFSHeader = (HFSPlusVolumeHeaderMin *)Buffer;
  HFSVolumeModifyDate = SwapBytes32(HFSHeader->modifyDate);
  DBG(" HFS+ volume modifyDate: %x\n", HFSVolumeModifyDate);
  fsw_efi_decode_time(&HFSVolumeModifyTime, mac_to_posix(HFSVolumeModifyDate));
  TimePtr = &HFSVolumeModifyTime;
  DBG(" in EFI: %d-%d-%d %d:%d:%d\n", TimePtr->Year, TimePtr->Month, TimePtr->Day, TimePtr->Hour, TimePtr->Minute, TimePtr->Second);
  FreePages(Buffer, 1);
  
  //
  // Check that sleepimage is not more then 2 mins older then volume modification date
  // Idea is from Chameleon
  //
  TimeDiffMs = (INTN)(GetEfiTimeInMs(&HFSVolumeModifyTime) - GetEfiTimeInMs(&ImageModifyTime));
  DBG(" image old: %d sec\n", TimeDiffMs / 1000);
  if (TimeDiffMs > 120000) {
    DBG(" image too old\n");
    // just test - always accepts sleepimage
 //   return TRUE;
    return FALSE;
  }
  
  DBG(" volume is hibernated\n");
  if (!gFirmwareClover && 
      !gDriversFlags.EmuVariableLoaded &&
      !GlobalConfig.IgnoreNVRAMBoot) {
    Status = gRT->GetVariable (L"Boot0082", &gEfiGlobalVariableGuid, NULL, &dataSize, data);
    if (!EFI_ERROR(Status)) {
      return TRUE;
    } else {
      return FALSE;
    }
  }

  return TRUE;
}


EFI_BLOCK_READ OrigBlockIoRead = NULL;
UINT64  gSleepImageOffset = 0;

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
  IOHibernateImageHeaderMin *Header;
  IOHibernateImageHeaderMinSnow *Header2;

  DBG(" OurBlockIoRead: Lba=%lx, Offset=%lx\n", Lba, Lba * 512);
  Status = OrigBlockIoRead(This, MediaId, Lba, BufferSize, Buffer);

  if (Status == EFI_SUCCESS && BufferSize >= sizeof(IOHibernateImageHeaderMin)) {
    Header = (IOHibernateImageHeaderMin *) Buffer;
    DBG(" sig lion: %x\n", Header->signature);
    DBG(" sig snow: %x\n", Header2->signature);
   // DBG(" sig swap: %x\n", SwapBytes32(Header->signature));
    if (Header->signature == kIOHibernateHeaderSignature
        // just for tests
        //|| Header->signature == kIOHibernateHeaderInvalidSignature
        || Header2->signature == kIOHibernateHeaderSignature
        )
    {
      gSleepImageOffset = Lba * 512;
      DBG(" got sleep image offset\n");
    }

  }

  return Status;
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
GetSleepImagePosition (IN REFIT_VOLUME *Volume)
{
    EFI_STATUS          Status;
    EFI_FILE            *File;
    VOID                *Buffer;
    UINTN               BufferSize;
    
    if (Volume->WholeDiskBlockIO == NULL) {
        DBG(" no disk BlockIo\n");
        return 0;
    }
    
    // Open sleepimage
    Status = Volume->RootDir->Open(Volume->RootDir, &File, L"\\private\\var\\vm\\sleepimage", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        DBG(" sleepimage not found -> %r\n", Status);
        return 0;
    }
    
    // Override disk BlockIo
    gSleepImageOffset = 0;
    OrigBlockIoRead = Volume->WholeDiskBlockIO->ReadBlocks;
    Volume->WholeDiskBlockIO->ReadBlocks = OurBlockIoRead;
    
    // Read the first block from sleepimage
    BufferSize = 512;
    Buffer = AllocatePool(BufferSize);
    if (Buffer == NULL) {
        return 0;
    }
    Status = File->Read(File, &BufferSize, Buffer);
    
    // Return original disk BlockIo
    Volume->WholeDiskBlockIO->ReadBlocks = OrigBlockIoRead;
    
    if (EFI_ERROR(Status)) {
        DBG(" can not read sleepimage -> %r\n", Status);
        return 0;
    }
    
    return gSleepImageOffset;
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
  CHAR16                      OffsetHexStr[17];
  EFI_DEVICE_PATH_PROTOCOL    *BootImageDevPath;
  UINT8                       VarData[256];
  UINTN                       Size;
  AppleRTCHibernateVars       RtcVars;
  
  DBG("PrepareHibernation:\n");
  
  // Find sleep image offset
  SleepImageOffset = GetSleepImagePosition (Volume);
  DBG(" SleepImageOffset: %lx\n", SleepImageOffset);
  if (SleepImageOffset == 0) {
    DBG(" sleepimage offset not found\n");
    return FALSE;
  }
  
  UnicodeSPrint(OffsetHexStr, sizeof(OffsetHexStr), L"%lx", SleepImageOffset);
  BootImageDevPath = FileDevicePath(Volume->WholeDiskDeviceHandle, OffsetHexStr);
  Size = GetDevicePathSize(BootImageDevPath);
  PrintBytes((CHAR8*) BootImageDevPath, Size);
  DBG(" boot-image device path: %s\n", FileDevicePathToStr(BootImageDevPath));

  Status = gRT->GetVariable (
                             L"boot-image",
                             &gEfiAppleBootGuid,
                             NULL,
                             &Size,
                             VarData
                             );
  if (Status == EFI_SUCCESS) {
    DBG("boot-image before: %s\n", FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL*)&VarData[0]));
    //      VarData[6] = 8;
    VarData[24] = 0xFF;
    VarData[25] = 0xFF;
    DBG("boot-image corrected: %s\n", FileDevicePathToStr((EFI_DEVICE_PATH_PROTOCOL*)&VarData[0]));
    gRT->SetVariable(L"boot-image", &gEfiAppleBootGuid,
                     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                     Size , VarData);
    // now we should delete boot0082 to do hibernate only once
    Status = DeleteBootOption(0x82);
    if (EFI_ERROR(Status)) {
      DBG("Options 0082 was not deleted: %r\n", Status);
    }
  } else {
    //we have no such variable so created new one
    // Set boot-image var
    
    Status = gRT->SetVariable(L"boot-image", &gEfiAppleBootGuid,
                              EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                              Size , BootImageDevPath);
    if (EFI_ERROR(Status)) {
      DBG(" can not write boot-image var -> %r\n", Status);
      return FALSE;
    }
  }
  
  // Set boot-switch-vars to dummy header without encription keys
  // TODO: check for existance first, and if exists then leave it as is
  // maybe somebody is lucky and kernel will set it properly
  SetMem(&RtcVars, sizeof(AppleRTCHibernateVars), 0);
  RtcVars.signature[0] = 'A';
  RtcVars.signature[1] = 'A';
  RtcVars.signature[2] = 'P';
  RtcVars.signature[3] = 'L';
  RtcVars.revision     = 1;
  Status = gRT->SetVariable(L"boot-switch-vars", &gEfiAppleBootGuid,
                            EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            sizeof(RtcVars) ,&RtcVars);
  if (EFI_ERROR(Status)) {
    DBG(" can not write boot-switch-vars -> %r\n", Status);
    return FALSE;
  }
  return TRUE;
//  DoHibernateWake = TRUE;
}


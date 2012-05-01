/*
 * refit/lib.c
 * General library functions
 *
 * Copyright (c) 2006-2009 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Platform.h"

#define DEBUG_LIB 1

#if DEBUG_LIB == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_LIB == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

// variables

EFI_HANDLE       SelfImageHandle;
EFI_HANDLE       SelfDeviceHandle;
EFI_LOADED_IMAGE *SelfLoadedImage;
EFI_FILE         *SelfRootDir;
EFI_FILE         *SelfDir;
CHAR16           *SelfDirPath;
EFI_DEVICE_PATH  *SelfDevicePath;
EFI_FILE         *ThemeDir;
CHAR16           *ThemePath;
EFI_FILE         *OEMDir;
CHAR16           *OEMPath;


REFIT_VOLUME     *SelfVolume = NULL;
REFIT_VOLUME     **Volumes = NULL;
UINTN            VolumesCount = 0;
//
// Unicode collation protocol interface
//
EFI_UNICODE_COLLATION_PROTOCOL *mUnicodeCollation = NULL;

// functions

static EFI_STATUS FinishInitRefitLib(VOID);

static VOID UninitVolumes(VOID);


BOOLEAN MetaiMatch (
			IN CHAR16   *String,
			IN CHAR16   *Pattern
			);

EFI_STATUS GetRootFromPath(IN EFI_DEVICE_PATH_PROTOCOL* DevicePath, OUT EFI_FILE **Root)
{
  EFI_STATUS  Status;
  EFI_HANDLE                NewHandle;
  EFI_DEVICE_PATH_PROTOCOL* TmpDevicePath;
//  DBG("Try to duplicate DevicePath\n");
  TmpDevicePath = DuplicateDevicePath(DevicePath);
//  DBG("TmpDevicePath found\n");
  NewHandle = NULL;
	Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid,
                                  &TmpDevicePath,
                                  &NewHandle);
//   DBG("volume handle found =%x\n", NewHandle);
  CheckError(Status, L"while reopening volume handle");
  *Root = EfiLibOpenRoot(NewHandle);  
  if (*Root == NULL) {
//    DBG("volume Root Dir can't be reopened\n");
    return EFI_NOT_FOUND;
  }
  if (FileExists(*Root, L"mach_kernel")) {
    DBG("mach_kernel exists\n");
  } else {
    DBG("mach_kernel not exists\n");
  }

  return Status;
}
//
// self recognition stuff
//

EFI_STATUS InitRefitLib(IN EFI_HANDLE ImageHandle)
{
    EFI_STATUS  Status;
    CHAR16      *FilePathAsString;
    CHAR16      BaseDirectory[256];
    UINTN       i;
    UINTN                     DevicePathSize;
    EFI_DEVICE_PATH_PROTOCOL* TmpDevicePath;
    
    SelfImageHandle = ImageHandle;
    Status = gBS->HandleProtocol(SelfImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &SelfLoadedImage);
    if (CheckFatalError(Status, L"while getting a LoadedImageProtocol handle"))
        return Status;
    
    SelfDeviceHandle = SelfLoadedImage->DeviceHandle;
    TmpDevicePath = DevicePathFromHandle (SelfDeviceHandle);
    DevicePathSize = GetDevicePathSize (TmpDevicePath);
    SelfDevicePath = AllocateAlignedPages(EFI_SIZE_TO_PAGES(DevicePathSize), 64);
    CopyMem(SelfDevicePath, TmpDevicePath, DevicePathSize);
  
    DBG("SelfDevicePath=%s @%x\n", DevicePathToStr(SelfDevicePath), SelfDeviceHandle);
  
  // find the current directory
    FilePathAsString = FileDevicePathToStr(SelfLoadedImage->FilePath);
    if (FilePathAsString != NULL) {
        StrCpy(BaseDirectory, FilePathAsString);
        FreePool(FilePathAsString);
        for (i = StrLen(BaseDirectory); i > 0 && BaseDirectory[i] != '\\'; i--) ;
        BaseDirectory[i] = 0;
    } else
        BaseDirectory[0] = 0;
    SelfDirPath = EfiStrDuplicate(BaseDirectory);

 //   Print(L"SelfDirPath = %s\n", SelfDirPath);  //result=\EFI\BOOT
  
    return FinishInitRefitLib();
}

VOID UninitRefitLib(VOID)
{
    // called before running external programs to close open file handles
    
    UninitVolumes();
    
    if (SelfDir != NULL) {
        SelfDir->Close(SelfDir);
        SelfDir = NULL;
    }
    
    if (SelfRootDir != NULL) {
        SelfRootDir->Close(SelfRootDir);
        SelfRootDir = NULL;
    }
  
  if (ThemeDir != NULL) {
    ThemeDir->Close(ThemeDir);
    ThemeDir = NULL;
  }

  if (OEMDir != NULL) {
    OEMDir->Close(OEMDir);
    OEMDir = NULL;
  }
  
  
}

EFI_STATUS ReinitRefitLib(VOID)
{
    // called after running external programs to re-open file handles
    //
    ReinitVolumes();
    
    if (SelfVolume != NULL && SelfVolume->RootDir != NULL)
        SelfRootDir = SelfVolume->RootDir;
    
    return FinishInitRefitLib();
}

EFI_STATUS ReinitSelfLib(VOID)
{
  // called after reconnect drivers to re-open file handles
  
  EFI_STATUS  Status;
  EFI_HANDLE                NewSelfHandle;
	EFI_DEVICE_PATH_PROTOCOL* TmpDevicePath;

  if (!SelfDevicePath) {
    return EFI_NOT_FOUND;
  }

  TmpDevicePath = DuplicateDevicePath(SelfDevicePath);
  DBG("reinit: self device path=%s\n", DevicePathToStr(TmpDevicePath));
  if(TmpDevicePath == NULL)
		return EFI_NOT_FOUND;

  NewSelfHandle = NULL;
	Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid,
                                  &TmpDevicePath,
                                  &NewSelfHandle);
  CheckError(Status, L"while reopening our self handle");
  DBG("new SelfHandle=%x\n", NewSelfHandle);

  SelfRootDir = EfiLibOpenRoot(NewSelfHandle);  
  if (SelfRootDir == NULL) {
    DBG("SelfRootDir can't be reopened\n");
    return EFI_NOT_FOUND;
  }
  SelfDeviceHandle = NewSelfHandle;
  Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath, EFI_FILE_MODE_READ, 0);
  Status = SelfRootDir->Open(SelfRootDir, &OEMDir, OEMPath, EFI_FILE_MODE_READ, 0);
  Status = SelfRootDir->Open(SelfRootDir, &SelfDir, SelfDirPath, EFI_FILE_MODE_READ, 0);
  CheckFatalError(Status, L"while reopening our installation directory");
  return Status;
}


static
EFI_STATUS FinishInitRefitLib(VOID)
{ 
	EFI_STATUS                Status;

  if (SelfRootDir == NULL) {
      SelfRootDir = EfiLibOpenRoot(SelfLoadedImage->DeviceHandle);
     if (SelfRootDir != NULL) {
       SelfDeviceHandle = SelfLoadedImage->DeviceHandle;
     } else {
       return EFI_LOAD_ERROR;
     }
  }
  Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath, EFI_FILE_MODE_READ, 0);
  Status = SelfRootDir->Open(SelfRootDir, &OEMDir, OEMPath, EFI_FILE_MODE_READ, 0);
  Status = SelfRootDir->Open(SelfRootDir, &SelfDir, SelfDirPath, EFI_FILE_MODE_READ, 0);
  CheckFatalError(Status, L"while opening our installation directory");
  return Status;
}

//
// list functions
//

VOID CreateList(OUT VOID ***ListPtr, OUT UINTN *ElementCount, IN UINTN InitialElementCount)
{
    UINTN AllocateCount;
    
    *ElementCount = InitialElementCount;
    if (*ElementCount > 0) {
        AllocateCount = (*ElementCount + 7) & ~7;   // next multiple of 8
        *ListPtr = AllocatePool(sizeof(VOID *) * AllocateCount);
    } else {
        *ListPtr = NULL;
    }
}

VOID AddListElement(IN OUT VOID ***ListPtr, IN OUT UINTN *ElementCount, IN VOID *NewElement)
{
    UINTN AllocateCount;
    
    if ((*ElementCount & 7) == 0) {
        AllocateCount = *ElementCount + 8;
        if (*ElementCount == 0)
            *ListPtr = AllocatePool(sizeof(VOID *) * AllocateCount);
        else
            *ListPtr =  EfiReallocatePool((VOID *)*ListPtr, sizeof(VOID *) * (*ElementCount), sizeof(VOID *) * AllocateCount);
    }
    (*ListPtr)[*ElementCount] = NewElement;
    (*ElementCount)++;
}

VOID FreeList(IN OUT VOID ***ListPtr, IN OUT UINTN *ElementCount)
{
    UINTN i;
    
    if (*ElementCount > 0) {
        for (i = 0; i < *ElementCount; i++) {
            // TODO: call a user-provided routine for each element here
            FreePool((*ListPtr)[i]);
        }
        FreePool(*ListPtr);
    }
}

//
// firmware device path discovery
//

static UINT8 LegacyLoaderMediaPathData[] = {
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
static EFI_DEVICE_PATH *LegacyLoaderMediaPath = (EFI_DEVICE_PATH *)LegacyLoaderMediaPathData;

EFI_STATUS ExtractLegacyLoaderPaths(EFI_DEVICE_PATH **PathList, UINTN MaxPaths, EFI_DEVICE_PATH **HardcodedPathList)
{
    EFI_STATUS          Status;
    UINTN               HandleCount = 0;
    UINTN               HandleIndex, HardcodedIndex;
    EFI_HANDLE          *Handles;
    EFI_HANDLE          Handle;
    UINTN               PathCount = 0;
    UINTN               PathIndex;
    EFI_LOADED_IMAGE    *LoadedImage;
    EFI_DEVICE_PATH     *DevicePath;
    BOOLEAN             Seen;
    
    MaxPaths--;  // leave space for the terminating NULL pointer
    
    // get all LoadedImage handles
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiLoadedImageProtocolGuid, NULL,
                             &HandleCount, &Handles);
    if (CheckError(Status, L"while listing LoadedImage handles")) {
        if (HardcodedPathList) {
            for (HardcodedIndex = 0; HardcodedPathList[HardcodedIndex] && PathCount < MaxPaths; HardcodedIndex++)
                PathList[PathCount++] = HardcodedPathList[HardcodedIndex];
        }
        PathList[PathCount] = NULL;
        return Status;
    }
    for (HandleIndex = 0; HandleIndex < HandleCount && PathCount < MaxPaths; HandleIndex++) {
        Handle = Handles[HandleIndex];
        
        Status = gBS->HandleProtocol(Handle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
        if (EFI_ERROR(Status))
            continue;  // This can only happen if the firmware scewed up, ignore it.
        
        Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiDevicePathProtocolGuid, (VOID **) &DevicePath);
        if (EFI_ERROR(Status))
            continue;  // This happens, ignore it.
        
        // Only grab memory range nodes
        if (DevicePathType(DevicePath) != HARDWARE_DEVICE_PATH || DevicePathSubType(DevicePath) != HW_MEMMAP_DP)
            continue;
        
        // Check if we have this device path in the list already
        // WARNING: This assumes the first node in the device path is unique!
        Seen = FALSE;
        for (PathIndex = 0; PathIndex < PathCount; PathIndex++) {
            if (DevicePathNodeLength(DevicePath) != DevicePathNodeLength(PathList[PathIndex]))
                continue;
            if (CompareMem(DevicePath, PathList[PathIndex], DevicePathNodeLength(DevicePath)) == 0) {
                Seen = TRUE;
                break;
            }
        }
        if (Seen)
            continue;
        
        PathList[PathCount++] = AppendDevicePath(DevicePath, LegacyLoaderMediaPath);
    }
    FreePool(Handles);
    
    if (HardcodedPathList) {
        for (HardcodedIndex = 0; HardcodedPathList[HardcodedIndex] && PathCount < MaxPaths; HardcodedIndex++)
            PathList[PathCount++] = HardcodedPathList[HardcodedIndex];
    }
    PathList[PathCount] = NULL;
  return (PathCount > 0)?EFI_SUCCESS:EFI_NOT_FOUND;
}

//
// volume functions
//

static VOID ScanVolumeBootcode(IN OUT REFIT_VOLUME *Volume, OUT BOOLEAN *Bootable)
{
    EFI_STATUS              Status;
    UINT8                   *SectorBuffer;
    UINTN                   i;
    MBR_PARTITION_INFO      *MbrTable;
    BOOLEAN                 MbrTableFound;
    UINTN       BlockSize = 0;  
    CHAR16      volumeName[255];
    
    Volume->HasBootCode = FALSE;
    Volume->OSIconName = NULL;
    Volume->OSName = NULL;
    *Bootable = FALSE;
    
    if (Volume->BlockIO == NULL)
        return;
    BlockSize = Volume->BlockIO->Media->BlockSize;
    if (BlockSize > 2048)
        return;   // our buffer is too small...
    SectorBuffer = AllocateAlignedPages(EFI_SIZE_TO_PAGES (2048), 16); //align to 16 byte?!
    // look at the boot sector (this is used for both hard disks and El Torito images!)
    Status = Volume->BlockIO->ReadBlocks(Volume->BlockIO, Volume->BlockIO->Media->MediaId,
                                         Volume->BlockIOOffset /*start lba*/,
                                         2048, SectorBuffer);
    if (!EFI_ERROR(Status)) {
      // calc crc checksum of first 2 sectors - it's used later for legacy boot BIOS drive num detection
      // note: possible future issues with AF 4K disks
      gBS->CalculateCrc32 (SectorBuffer, 2 * 512, &Volume->DriveCRC32);
      
      if (Volume->DiskKind == DISK_KIND_OPTICAL) { //CDROM
        CHAR8* p = (CHAR8*)&SectorBuffer[8];
				while (*p == 0x20) {
					p++;
				}
				if (*p != 0) {
					AsciiStrToUnicodeStr(p, volumeName);
				}
				for (i=8; i<2000; i++) { //vendor search
					if (SectorBuffer[i] == 'A') {
						if (AsciiStrStr((CHAR8*)&SectorBuffer[i], "APPLE")) {
							StrCpy(Volume->VolName, volumeName);
							Volume->OSType = OSTYPE_OSX;
							Volume->BootType = BOOTING_BY_CD;
							break;
						}
					} else if (SectorBuffer[i] == 'M') {
						if (AsciiStrStr((CHAR8*)&SectorBuffer[i], "MICROSOFT")) {
							StrCpy(Volume->VolName, volumeName);
							Volume->OSType = OSTYPE_WIN;
							Volume->BootType = BOOTING_BY_CD;
							break;
						}
						
					} else if (SectorBuffer[i] == 'L') {
						if (AsciiStrStr((CHAR8*)&SectorBuffer[i], "LINUX")) {
					//		Volume->DevicePath = DuplicateDevicePath(DevicePath);
              
							StrCpy(Volume->VolName, volumeName);
							Volume->OSType = OSTYPE_LIN;
							Volume->BootType = BOOTING_BY_CD;
							break;
						}						
					}
				}				
        
      }
      else { //HDD
        if (*((UINT16 *)(SectorBuffer + 510)) == 0xaa55 && SectorBuffer[0] != 0) {
          *Bootable = TRUE;
          Volume->HasBootCode = TRUE;
      //    DBG("The volume has bootcode\n");
          Volume->OSIconName = L"legacy";
          Volume->OSName = L"Legacy";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
        }
        
        // detect specific boot codes
        if (CompareMem(SectorBuffer + 2, "LILO", 4) == 0 ||
            CompareMem(SectorBuffer + 6, "LILO", 4) == 0 ||
            CompareMem(SectorBuffer + 3, "SYSLINUX", 8) == 0 ||
            FindMem(SectorBuffer, 2048, "ISOLINUX", 8) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"linux";
          Volume->OSName = L"Linux";
          Volume->OSType = OSTYPE_LIN;
          Volume->BootType = BOOTING_BY_PBR;

          
        } else if (FindMem(SectorBuffer, 512, "Geom\0Hard Disk\0Read\0 Error", 26) >= 0) {   // GRUB
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"grub,linux";
          Volume->OSName = L"Linux";
          
 /*       } else if ((*((UINT32 *)(SectorBuffer)) == 0x8ec031fa &&
                    *((UINT16 *)(SectorBuffer + 510)) == 0xaa55) ||
                   FindMem(SectorBuffer, 2048, "boot      ", 10) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"mac";
          Volume->OSName = L"MacOSX";
          Volume->OSType = OSTYPE_OSX;
          Volume->BootType = BOOTING_BY_EFI;
   //       DBG("Detected MacOSX HFS+ bootcode\n"); */

        } else if ((*((UINT32 *)(SectorBuffer)) == 0x4d0062e9 &&
                    *((UINT16 *)(SectorBuffer + 510)) == 0xaa55) ||
                   FindMem(SectorBuffer, 2048, "BOOT      ", 10) >= 0) { //reboot Clover
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"clover";
          Volume->OSName = L"MacOSX";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
  //        DBG("Detected Clover FAT32 bootcode\n");
          
          
        } else if ((*((UINT32 *)(SectorBuffer + 502)) == 0 &&
                    *((UINT32 *)(SectorBuffer + 506)) == 50000 &&
                    *((UINT16 *)(SectorBuffer + 510)) == 0xaa55) ||
                   FindMem(SectorBuffer, 2048, "Starting the BTX loader", 23) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"freebsd";
          Volume->OSName = L"FreeBSD";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;

          
        } else if (FindMem(SectorBuffer, 512, "!Loading", 8) >= 0 ||
                   FindMem(SectorBuffer, 2048, "/cdboot\0/CDBOOT\0", 16) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"openbsd";
          Volume->OSName = L"OpenBSD";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else if (FindMem(SectorBuffer, 512, "Not a bootxx image", 18) >= 0 ||
                   *((UINT32 *)(SectorBuffer + 1028)) == 0x7886b6d1) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"netbsd";
          Volume->OSName = L"NetBSD";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else if (FindMem(SectorBuffer, 2048, "NTLDR", 5) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"win";
          Volume->OSName = L"Windows";
          Volume->OSType = OSTYPE_WIN;
          Volume->BootType = BOOTING_BY_PBR;

          
        } else if (FindMem(SectorBuffer, 2048, "BOOTMGR", 7) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"vista";
          Volume->OSName = L"Windows";
          Volume->OSType = OSTYPE_WIN;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else if (FindMem(SectorBuffer, 512, "CPUBOOT SYS", 11) >= 0 ||
                   FindMem(SectorBuffer, 512, "KERNEL  SYS", 11) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"freedos";
          Volume->OSName = L"FreeDOS";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else if (FindMem(SectorBuffer, 512, "OS2LDR", 6) >= 0 ||
                   FindMem(SectorBuffer, 512, "OS2BOOT", 7) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"ecomstation";
          Volume->OSName = L"eComStation";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else if (FindMem(SectorBuffer, 512, "Be Boot Loader", 14) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"beos";
          Volume->OSName = L"BeOS";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else if (FindMem(SectorBuffer, 512, "yT Boot Loader", 14) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"zeta";
          Volume->OSName = L"ZETA";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else if (FindMem(SectorBuffer, 512, "\x04" "beos\x06" "system\x05" "zbeos", 18) >= 0 ||
                   FindMem(SectorBuffer, 512, "\x06" "system\x0c" "haiku_loader", 20) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"haiku";
          Volume->OSName = L"Haiku";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;          
      }
      }
      
      if (Volume->OSIconName) {
        CHAR16          FileName[256];
        UnicodeSPrint(FileName, 255, L"icons\\os_%s.icns", Volume->OSIconName);
        Volume->OSImage = egLoadIcon(ThemeDir, FileName, 128);
        //LoadOSIcon(Volume->OSIconName, L"mac", FALSE);
      }

        
        // NOTE: If you add an operating system with a name that starts with 'W' or 'L', you
        //  need to fix AddLegacyEntry in main.c.
        
#if 0 // REFIT_DEBUG > 0
        Print(L"  Result of bootcode detection: %s %s (%s)\n",
              Volume->HasBootCode ? L"bootable" : L"non-bootable",
              Volume->OSName, Volume->OSIconName);
#endif
        
        if (FindMem(SectorBuffer, 512, "Non-system disk", 15) >= 0)   // dummy FAT boot sector
            Volume->HasBootCode = FALSE;
        
        // check for MBR partition table
        if (*((UINT16 *)(SectorBuffer + 510)) == 0xaa55) {
            MbrTableFound = FALSE;
            MbrTable = (MBR_PARTITION_INFO *)(SectorBuffer + 446);
            for (i = 0; i < 4; i++)
                if (MbrTable[i].StartLBA && MbrTable[i].Size)
                    MbrTableFound = TRUE;
            for (i = 0; i < 4; i++)
                if (MbrTable[i].Flags != 0x00 && MbrTable[i].Flags != 0x80)
                    MbrTableFound = FALSE;
            if (MbrTableFound) {
                Volume->MbrPartitionTable = AllocatePool(4 * 16);
                CopyMem(Volume->MbrPartitionTable, MbrTable, 4 * 16);
                Volume->BootType = BOOTING_BY_MBR;
            }
        }
    }
}

EG_IMAGE* ScanVolumeDefaultIcon(IN UINT8 DiskKind)
{  
    // default volume icon based on disk kind
  switch (DiskKind) {
    case DISK_KIND_INTERNAL:
      return BuiltinIcon(BUILTIN_ICON_VOL_INTERNAL);
    case DISK_KIND_EXTERNAL:
      return BuiltinIcon(BUILTIN_ICON_VOL_EXTERNAL);
    case DISK_KIND_OPTICAL:
      return BuiltinIcon(BUILTIN_ICON_VOL_OPTICAL);
    case DISK_KIND_FIREWIRE:
      return BuiltinIcon(BUILTIN_ICON_VOL_FIREWIRE);
    case DISK_KIND_BOOTER:
      return BuiltinIcon(BUILTIN_ICON_VOL_BOOTER);
    default:
      break;
  }
  return NULL;
}

//at start we have only Volume->DeviceHandle
static EFI_STATUS ScanVolume(IN OUT REFIT_VOLUME *Volume)
{
  EFI_STATUS              Status = EFI_NOT_FOUND;
  EFI_DEVICE_PATH         *DevicePath, *NextDevicePath;
  EFI_DEVICE_PATH         *DiskDevicePath, *RemainingDevicePath = NULL;
  HARDDRIVE_DEVICE_PATH   *HdPath     = NULL; 
  EFI_HANDLE              WholeDiskHandle;
  UINTN                   PartialLength = 0;
  UINTN                   DevicePathSize;
//  UINTN                   BufferSize = 255;
  EFI_FILE_SYSTEM_VOLUME_LABEL *VolumeInfo;
  EFI_FILE_SYSTEM_INFO    *FileSystemInfoPtr;
  EFI_FILE_INFO           *RootInfo = NULL;
  BOOLEAN                 Bootable;
//  EFI_INPUT_KEY           Key;
  CHAR16                  *tmpName;
  
    // get device path
  DiskDevicePath = DevicePathFromHandle(Volume->DeviceHandle);
  DevicePathSize = GetDevicePathSize (DiskDevicePath);
  Volume->DevicePath = AllocateAlignedPages(EFI_SIZE_TO_PAGES(DevicePathSize), 64);
  CopyMem(Volume->DevicePath, DiskDevicePath, DevicePathSize);
  
//    Volume->DevicePath = DuplicateDevicePath(DevicePathFromHandle(Volume->DeviceHandle));
#if 0 //REFIT_DEBUG > 0
    if (Volume->DevicePath != NULL) {
        Print(L"* %s\n", DevicePathToStr(Volume->DevicePath));
#if REFIT_DEBUG >= 2
 //       DumpHex(1, 0, GetDevicePathSize(Volume->DevicePath), Volume->DevicePath);
#endif
    }
#endif
    
    Volume->DiskKind = DISK_KIND_INTERNAL;  // default
    
    // get block i/o
    Status = gBS->HandleProtocol(Volume->DeviceHandle, &gEfiBlockIoProtocolGuid, (VOID **) &(Volume->BlockIO));
    if (EFI_ERROR(Status)) {
      Volume->BlockIO = NULL;
      DBG("Warning: Can't get BlockIO protocol.\n");
 //     WaitForSingleEvent (gST->ConIn->WaitForKey, 0);
 //     gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);

      return Status;
    } else {
      if (Volume->BlockIO->Media->BlockSize == 2048){
        Volume->DiskKind = DISK_KIND_OPTICAL;
        Volume->BlockIOOffset = 0; //0x10; //no offset already applyed!
      } else {
        Volume->BlockIOOffset = 0;
      }
    }
    
    // scan for bootcode and MBR table
    Bootable = FALSE;
    ScanVolumeBootcode(Volume, &Bootable);
//  DBG("ScanVolumeBootcode success\n");
    // detect device type
    DevicePath = DuplicateDevicePath(Volume->DevicePath);
    while (DevicePath != NULL && !IsDevicePathEndType(DevicePath)) {
        NextDevicePath = NextDevicePathNode(DevicePath);
      
      if ((DevicePathType (DevicePath) == MESSAGING_DEVICE_PATH) &&
          (DevicePathSubType (DevicePath) == MSG_SATA_DP)) {
//        DBG("HDD volume\n");
        Volume->DiskKind = DISK_KIND_INTERNAL; 
        break;
      }
      if (DevicePathType(DevicePath) == MESSAGING_DEVICE_PATH && 
          (DevicePathSubType(DevicePath) == MSG_USB_DP || DevicePathSubType(DevicePath) == MSG_USB_CLASS_DP)) {
//        DBG("USB volume\n");
        Volume->DiskKind = DISK_KIND_EXTERNAL; 
        break;
      }
      // FIREWIRE Devices
      if (DevicePathType(DevicePath) == MESSAGING_DEVICE_PATH && 
          (DevicePathSubType(DevicePath) == MSG_1394_DP || DevicePathSubType(DevicePath) == MSG_FIBRECHANNEL_DP)) {
//        DBG("FireWire volume\n");
        Volume->DiskKind = DISK_KIND_FIREWIRE; 
        break;
      }
      // CD-ROM Devices
      if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && 
          DevicePathSubType(DevicePath) == MEDIA_CDROM_DP) {
//        DBG("CD-ROM volume\n");
        Volume->DiskKind = DISK_KIND_OPTICAL;     
        break;
      }
      // VENDOR Specific Path
      if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && 
          DevicePathSubType(DevicePath) == MEDIA_VENDOR_DP) {
//        DBG("Vendor volume\n");
        Volume->DiskKind = DISK_KIND_NODISK;
        break;
      }
      // LEGACY CD-ROM
      if (DevicePathType(DevicePath) == BBS_DEVICE_PATH && 
          (DevicePathSubType(DevicePath) == BBS_BBS_DP || DevicePathSubType(DevicePath) == BBS_TYPE_CDROM)) {
//        DBG("Legacy CD-ROM volume\n");
        Volume->DiskKind = DISK_KIND_OPTICAL;
        break;
      }
      // LEGACY HARDDISK
      if (DevicePathType(DevicePath) == BBS_DEVICE_PATH && 
          (DevicePathSubType(DevicePath) == BBS_BBS_DP || DevicePathSubType(DevicePath) == BBS_TYPE_HARDDRIVE)) {
//        DBG("Legacy HDD volume\n");
        Volume->DiskKind = DISK_KIND_INTERNAL;
        break;
      }
      DevicePath = NextDevicePath;
    }

  /*  what is the bread?
     // Bootable = TRUE;
      
      if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH &&
            DevicePathSubType(DevicePath) == MEDIA_VENDOR_DP) {
            Volume->IsAppleLegacy = TRUE;             // legacy BIOS device entry
            // TODO: also check for Boot Camp GUID
        //gEfiPartTypeSystemPartGuid
            Bootable = FALSE;   // this handle's BlockIO is just an alias for the whole device
          Print(L"AppleLegacy device\n");
        }
 */   

  DevicePath = DuplicateDevicePath(Volume->DevicePath);
  RemainingDevicePath = DevicePath; //initial value
	//
	// find the partition device path node
	//
	while (DevicePath && !IsDevicePathEnd (DevicePath)) {
		if ((DevicePathType (DevicePath) == MEDIA_DEVICE_PATH) &&
        (DevicePathSubType (DevicePath) == MEDIA_HARDDRIVE_DP)) {
      HdPath = (HARDDRIVE_DEVICE_PATH *)DevicePath;
//			break;
		}		
		DevicePath = NextDevicePathNode (DevicePath);
	}
//  DBG("DevicePath scanned\n");
    if (HdPath) {
//      Print(L"Partition found %s\n", DevicePathToStr((EFI_DEVICE_PATH *)HdPath));
      
      PartialLength = (UINTN)((UINT8 *)HdPath - (UINT8 *)(RemainingDevicePath));
      if (PartialLength > 0x1000) {        
        PartialLength = sizeof(EFI_DEVICE_PATH); //something wrong here but I don't want to be freezed
        //       return EFI_SUCCESS;
      }
      DiskDevicePath = (EFI_DEVICE_PATH *)AllocatePool(PartialLength + sizeof(EFI_DEVICE_PATH));
      CopyMem(DiskDevicePath, Volume->DevicePath, PartialLength);
      CopyMem((UINT8 *)DiskDevicePath + PartialLength, DevicePath, sizeof(EFI_DEVICE_PATH)); //EndDevicePath
//      Print(L"WholeDevicePath  %s\n", DevicePathToStr(DiskDevicePath));
//      DBG("WholeDevicePath  %s\n", DevicePathToStr(DiskDevicePath));
      RemainingDevicePath = DiskDevicePath;
      Status = gBS->LocateDevicePath(&gEfiDevicePathProtocolGuid, &RemainingDevicePath, &WholeDiskHandle);
      if (EFI_ERROR(Status)) {
        DBG("Can't find WholeDevicePath: %r\n", Status);
      } else {
        Volume->WholeDiskDevicePath = DuplicateDevicePath(RemainingDevicePath);
        // look at the BlockIO protocol
        Status = gBS->HandleProtocol(WholeDiskHandle, &gEfiBlockIoProtocolGuid, (VOID **) &Volume->WholeDiskBlockIO);
        if (!EFI_ERROR(Status)) {
 //          DBG("WholeDiskBlockIO %x BlockSize=%d\n", Volume->WholeDiskBlockIO, Volume->WholeDiskBlockIO->Media->BlockSize);
          // check the media block size
          if (Volume->WholeDiskBlockIO->Media->BlockSize == 2048)
            Volume->DiskKind = DISK_KIND_OPTICAL;
          
        } else {
          Volume->WholeDiskBlockIO = NULL;
          DBG("no WholeDiskBlockIO: %r\n", Status);
          
          //CheckError(Status, L"from HandleProtocol");
        }
      }
      FreePool(DiskDevicePath);
    }
/*  else {
    DBG("HD path is not found\n");
  }*/

    if (!Bootable) {
#if REFIT_DEBUG > 0
      if (Volume->HasBootCode){
        DBG("  Volume considered non-bootable, but boot code is present\n");
//        WaitForSingleEvent (gST->ConIn->WaitForKey, 0);
//        gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
      }
#endif
        Volume->HasBootCode = FALSE;
    }
//  DBG("default volume icon based on disk kind\n");
    // default volume icon based on disk kind
    Volume->DriveImage = ScanVolumeDefaultIcon(Volume->DiskKind);
//  DBG("default volume icon OK\n");
    // open the root directory of the volume
    Volume->RootDir = EfiLibOpenRoot(Volume->DeviceHandle);
//  DBG("Volume->RootDir OK\n");
    if (Volume->RootDir == NULL) {
        //Print(L"Error: Can't open volume.\n");
        // TODO: signal that we had an error
      //Slice - there is LegacyBoot volume
      //properties are set before
//        DBG("LegacyBoot volume\n");

      if (HdPath) {
        tmpName = (CHAR16*)AllocateZeroPool(60);
//        DBG("Create legacyName\n");
        UnicodeSPrint(tmpName, 60, L"Legacy HD%d", HdPath->PartitionNumber);
        Volume->VolName = EfiStrDuplicate(tmpName);
        FreePool(tmpName);
      }
     //Volume->VolName =  L"Legacy OS";
      return EFI_SUCCESS;
    }
  
    // get volume name
  Volume->VolName = NULL;
  if (Volume->RootDir) {
    RootInfo = EfiLibFileInfo (Volume->RootDir);
    if (RootInfo) {
      //        DBG("  Volume name from RootFile\n");
      Volume->VolName = EfiStrDuplicate(RootInfo->FileName);
      FreePool(RootInfo);
    }
  }
  if (!Volume->VolName) {
    FileSystemInfoPtr = EfiLibFileSystemInfo(Volume->RootDir);
    if (FileSystemInfoPtr) {
 //     DBG("  Volume name from FileSystem\n");
      Volume->VolName = EfiStrDuplicate(FileSystemInfoPtr->VolumeLabel);
      FreePool(FileSystemInfoPtr);
    }
    if (!Volume->VolName) {
      VolumeInfo = EfiLibFileSystemVolumeLabelInfo(Volume->RootDir);
      if (VolumeInfo) {
//        DBG("  Volume name from VolumeLabel\n");
        Volume->VolName = EfiStrDuplicate(VolumeInfo->VolumeLabel);
        FreePool(VolumeInfo); 
      }  
    }
  }
  if (!Volume->VolName) {
    DBG("Create unknown name\n");
    //        WaitForSingleEvent (gST->ConIn->WaitForKey, 0);
    //        gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);

    if (HdPath) {      
      
      tmpName = (CHAR16*)AllocateZeroPool(128);
      UnicodeSPrint(tmpName, 60, L"Unknown HD%d", HdPath->PartitionNumber);
      Volume->VolName = EfiStrDuplicate(tmpName);
      FreePool(tmpName);
      // NOTE: this is normal for Apple's VenMedia device paths
    } else {
      Volume->VolName = L"Unknown HD";
    }
  }

//  DBG("GetOSVersion\n");
  Status = GetOSVersion(Volume); //here we set tiger,leo,snow,lion and cougar
  if (!EFI_ERROR(Status)) {
    Volume->OSImage = egLoadIcon(ThemeDir, PoolPrint(L"icons\\os_%s.icns", Volume->OSIconName), 128);
  }
  
    // get custom volume icon if present
/*  if (FileExists(Volume->RootDir, L".VolumeIcon.icns")){
        Volume->OSImage = LoadIcns(Volume->RootDir, L".VolumeIcon.icns", 32);
  }*/
  return EFI_SUCCESS;
}

static VOID ScanExtendedPartition(REFIT_VOLUME *WholeDiskVolume, MBR_PARTITION_INFO *MbrEntry)
{
    EFI_STATUS              Status;
    REFIT_VOLUME            *Volume;
    UINT32                  ExtBase, ExtCurrent, NextExtCurrent;
    UINTN                   i;
    UINTN                   LogicalPartitionIndex = 4;
    UINT8                   *SectorBuffer;
    BOOLEAN                 Bootable;
    MBR_PARTITION_INFO      *EMbrTable;
    
    ExtBase = MbrEntry->StartLBA;
    SectorBuffer = AllocateAlignedPages (EFI_SIZE_TO_PAGES (512), WholeDiskVolume->BlockIO->Media->IoAlign);
  
    for (ExtCurrent = ExtBase; ExtCurrent; ExtCurrent = NextExtCurrent) {
        // read current EMBR
        Status = WholeDiskVolume->BlockIO->ReadBlocks(WholeDiskVolume->BlockIO,
                                                      WholeDiskVolume->BlockIO->Media->MediaId,
                                                      ExtCurrent, 512, SectorBuffer);
        if (EFI_ERROR(Status))
            break;
        if (*((UINT16 *)(SectorBuffer + 510)) != 0xaa55)
            break;
        EMbrTable = (MBR_PARTITION_INFO *)(SectorBuffer + 446);
        
        // scan logical partitions in this EMBR
        NextExtCurrent = 0;
        for (i = 0; i < 4; i++) {
            if ((EMbrTable[i].Flags != 0x00 && EMbrTable[i].Flags != 0x80) ||
                EMbrTable[i].StartLBA == 0 || EMbrTable[i].Size == 0)
                break;
            if (IS_EXTENDED_PART_TYPE(EMbrTable[i].Type)) {
                // set next ExtCurrent
                NextExtCurrent = ExtBase + EMbrTable[i].StartLBA;
                break;
            } else {
                
                // found a logical partition
                Volume = AllocateZeroPool(sizeof(REFIT_VOLUME));
                Volume->DiskKind = WholeDiskVolume->DiskKind;
                Volume->IsMbrPartition = TRUE;
                Volume->MbrPartitionIndex = LogicalPartitionIndex++;
                Volume->VolName = PoolPrint(L"Partition %d", Volume->MbrPartitionIndex + 1);
                Volume->BlockIO = WholeDiskVolume->BlockIO;
                Volume->BlockIOOffset = ExtCurrent + EMbrTable[i].StartLBA;
                Volume->WholeDiskBlockIO = WholeDiskVolume->BlockIO;
                
                Bootable = FALSE;
                ScanVolumeBootcode(Volume, &Bootable);
                if (!Bootable)
                    Volume->HasBootCode = FALSE;
                
                Volume->DriveImage = ScanVolumeDefaultIcon(Volume->DiskKind);                
                AddListElement((VOID ***) &Volumes, &VolumesCount, Volume);                
            }
        }
    }
}

VOID ScanVolumes(VOID)
{
    EFI_STATUS              Status;
    UINTN                   HandleCount = 0;
    UINTN                   HandleIndex;
    EFI_HANDLE              *Handles = NULL;
    REFIT_VOLUME            *Volume, *WholeDiskVolume;
    UINTN                   VolumeIndex, VolumeIndex2;
    MBR_PARTITION_INFO      *MbrTable;
    UINTN                   PartitionIndex;
    UINT8                   *SectorBuffer1, *SectorBuffer2;
    UINTN                   SectorSum, i;
//  EFI_INPUT_KEY Key;
    
//    DBG("Scanning volumes...\n");
    
    // get all BlockIo handles
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &Handles);
    if (Status == EFI_NOT_FOUND)
        return;  
    
    // first pass: collect information about all handles
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        
      Volume = AllocateZeroPool(sizeof(REFIT_VOLUME));
      Volume->DeviceHandle = Handles[HandleIndex];
      if (Volume->DeviceHandle == SelfDeviceHandle){
//        DBG("this is SelfVolume at index %d\n", HandleIndex);
        SelfVolume = Volume;  
      }
      
      Status = ScanVolume(Volume);
      if (!EFI_ERROR(Status)) {
//        DBG("Found Volume %s at index=%d\n", Volume->VolName, HandleIndex);
        AddListElement((VOID ***) &Volumes, &VolumesCount, Volume);
        
      } else {
        DBG("wrong volume Nr%d?!\n", HandleIndex);
        FreePool(Volume);
      }
    }
    FreePool(Handles);
  DBG("Found %d volumes\n", VolumesCount);
  if (SelfVolume == NULL){
    DBG("WARNING: SelfVolume not found"); //Slice - and what?
    SelfVolume = AllocateZeroPool(sizeof(REFIT_VOLUME));
    SelfVolume->DeviceHandle = SelfDeviceHandle;
    SelfVolume->DevicePath = SelfDevicePath;
    SelfVolume->RootDir = SelfRootDir;
    SelfVolume->DiskKind = DISK_KIND_BOOTER;
    SelfVolume->VolName = L"Clover";
    SelfVolume->OSType = OSTYPE_EFI;
    SelfVolume->HasBootCode = TRUE;
    SelfVolume->BootType = BOOTING_BY_PBR;
 //   AddListElement((VOID ***) &Volumes, &VolumesCount, SelfVolume);
//    DBG("SelfVolume Nr %d created\n", VolumesCount);
  }
    
    // second pass: relate partitions and whole disk devices
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        Volume = Volumes[VolumeIndex];
        
        // check MBR partition table for extended partitions
        if (Volume->BlockIO != NULL && Volume->WholeDiskBlockIO != NULL &&
            Volume->BlockIO == Volume->WholeDiskBlockIO && Volume->BlockIOOffset == 0 &&
            Volume->MbrPartitionTable != NULL) {
            DBG("Volume %d has MBR\n", VolumeIndex);
            MbrTable = Volume->MbrPartitionTable;
            for (PartitionIndex = 0; PartitionIndex < 4; PartitionIndex++) {
                if (IS_EXTENDED_PART_TYPE(MbrTable[PartitionIndex].Type)) {
                    ScanExtendedPartition(Volume, MbrTable + PartitionIndex);
                }
            }
        }
        
        // search for corresponding whole disk volume entry
        WholeDiskVolume = NULL;
        if (Volume->BlockIO != NULL && Volume->WholeDiskBlockIO != NULL &&
            Volume->BlockIO != Volume->WholeDiskBlockIO) {
            for (VolumeIndex2 = 0; VolumeIndex2 < VolumesCount; VolumeIndex2++) {
                if (Volumes[VolumeIndex2]->BlockIO == Volume->WholeDiskBlockIO &&
                    Volumes[VolumeIndex2]->BlockIOOffset == 0)
                    WholeDiskVolume = Volumes[VolumeIndex2];
            }
        }
        if (WholeDiskVolume != NULL && WholeDiskVolume->MbrPartitionTable != NULL) {
            // check if this volume is one of the partitions in the table
            MbrTable = WholeDiskVolume->MbrPartitionTable;
            SectorBuffer1 = AllocateAlignedPages (EFI_SIZE_TO_PAGES (512), 16);
            SectorBuffer2 = AllocateAlignedPages (EFI_SIZE_TO_PAGES (512), 16);
            
            for (PartitionIndex = 0; PartitionIndex < 4; PartitionIndex++) {
                // check size
                if ((UINT64)(MbrTable[PartitionIndex].Size) != Volume->BlockIO->Media->LastBlock + 1)
                    continue;
                
                // compare boot sector read through offset vs. directly
                Status = Volume->BlockIO->ReadBlocks(Volume->BlockIO, Volume->BlockIO->Media->MediaId,
                                                     Volume->BlockIOOffset, 512, SectorBuffer1);
                if (EFI_ERROR(Status))
                    break;
                Status = Volume->WholeDiskBlockIO->ReadBlocks(Volume->WholeDiskBlockIO, Volume->WholeDiskBlockIO->Media->MediaId,
                                                              MbrTable[PartitionIndex].StartLBA, 512, SectorBuffer2);
                if (EFI_ERROR(Status))
                    break;
                if (CompareMem(SectorBuffer1, SectorBuffer2, 512) != 0)
                    continue;
                SectorSum = 0;
                for (i = 0; i < 512; i++)
                    SectorSum += SectorBuffer1[i];
                if (SectorSum < 1000)
                    continue;
                
                // TODO: mark entry as non-bootable if it is an extended partition
                
                // now we're reasonably sure the association is correct...
                Volume->IsMbrPartition = TRUE;
                Volume->MbrPartitionIndex = PartitionIndex;
                if (Volume->VolName == NULL)
                    Volume->VolName = PoolPrint(L"Partition %d", PartitionIndex + 1);
                break;
            }
            
            FreePool(SectorBuffer1);
            FreePool(SectorBuffer2);
        }
        
    }
}

static VOID UninitVolumes(VOID)
{
    REFIT_VOLUME            *Volume;
    UINTN                   VolumeIndex;
    
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        Volume = Volumes[VolumeIndex];
        
        if (Volume->RootDir != NULL) {
            Volume->RootDir->Close(Volume->RootDir);
            Volume->RootDir = NULL;
        }
        
        Volume->DeviceHandle = NULL;
        Volume->BlockIO = NULL;
        Volume->WholeDiskBlockIO = NULL;
    }
}

VOID ReinitVolumes(VOID)
{
  EFI_STATUS              Status;
  REFIT_VOLUME            *Volume;
  UINTN                   VolumeIndex;
  UINTN           VolumesFound = 0;
  EFI_DEVICE_PATH         *RemainingDevicePath;
  EFI_HANDLE              DeviceHandle, WholeDiskHandle;
  
  for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
    Volume = Volumes[VolumeIndex];
    if (!Volume) {
      continue;
    }
    VolumesFound++;
    if (Volume->DevicePath != NULL) {
      // get the handle for that path
      RemainingDevicePath = Volume->DevicePath;
      Status = gBS->LocateDevicePath(&gEfiBlockIoProtocolGuid, &RemainingDevicePath, &DeviceHandle);
      
      if (!EFI_ERROR(Status)) {
        Volume->DeviceHandle = DeviceHandle;
        
        // get the root directory
        Volume->RootDir = EfiLibOpenRoot(Volume->DeviceHandle);
        
      } else
        CheckError(Status, L"from LocateDevicePath");
    }
    
    if (Volume->WholeDiskDevicePath != NULL) {
      // get the handle for that path
      RemainingDevicePath = Volume->WholeDiskDevicePath;
      Status = gBS->LocateDevicePath(&gEfiBlockIoProtocolGuid, &RemainingDevicePath, &WholeDiskHandle);
      
      if (!EFI_ERROR(Status)) {
        // get the BlockIO protocol
        Status = gBS->HandleProtocol(WholeDiskHandle, &gEfiBlockIoProtocolGuid, (VOID **) &Volume->WholeDiskBlockIO);
        if (EFI_ERROR(Status)) {
          Volume->WholeDiskBlockIO = NULL;
          CheckError(Status, L"from HandleProtocol");
        }
      } else
        CheckError(Status, L"from LocateDevicePath");
    }
  }
  VolumesCount = VolumesFound;
}

//
// file and dir functions
//

BOOLEAN FileExists(IN EFI_FILE *Root, IN CHAR16 *RelativePath)
{
    EFI_STATUS  Status;
    EFI_FILE    *TestFile;
    
    Status = Root->Open(Root, &TestFile, RelativePath, EFI_FILE_MODE_READ, 0);
    if (Status == EFI_SUCCESS) {
        TestFile->Close(TestFile);
        return TRUE;
    }
    return FALSE;
}

EFI_STATUS DirNextEntry(IN EFI_FILE *Directory, IN OUT EFI_FILE_INFO **DirEntry, IN UINTN FilterMode)
{
    EFI_STATUS Status;
    VOID *Buffer;
    UINTN LastBufferSize, BufferSize;
    INTN IterCount;
    
    for (;;) {
        
        // free pointer from last call
        if (*DirEntry != NULL) {
            FreePool(*DirEntry);
            *DirEntry = NULL;
        }
        
        // read next directory entry
        LastBufferSize = BufferSize = 256;
        Buffer = AllocateAlignedPages (EFI_SIZE_TO_PAGES (BufferSize), 16);
        for (IterCount = 0; ; IterCount++) {
            Status = Directory->Read(Directory, &BufferSize, Buffer);
            if (Status != EFI_BUFFER_TOO_SMALL || IterCount >= 4)
                break;
            if (BufferSize <= LastBufferSize) {
                DBG("FS Driver requests bad buffer size %d (was %d), using %d instead\n", BufferSize, LastBufferSize, LastBufferSize * 2);
                BufferSize = LastBufferSize * 2;
#if REFIT_DEBUG > 0
            } else {
                DBG("Reallocating buffer from %d to %d\n", LastBufferSize, BufferSize);
#endif
            }
            Buffer = EfiReallocatePool(Buffer, LastBufferSize, BufferSize);
            LastBufferSize = BufferSize;
        }
        if (EFI_ERROR(Status)) {
            FreePool(Buffer);
            break;
        }
        
        // check for end of listing
        if (BufferSize == 0) {    // end of directory listing
            FreePool(Buffer);
            break;
        }
        
        // entry is ready to be returned
        *DirEntry = (EFI_FILE_INFO *)Buffer;
        
        // filter results
        if (FilterMode == 1) {   // only return directories
            if (((*DirEntry)->Attribute & EFI_FILE_DIRECTORY))
                break;
        } else if (FilterMode == 2) {   // only return files
            if (((*DirEntry)->Attribute & EFI_FILE_DIRECTORY) == 0)
                break;
        } else                   // no filter or unknown filter -> return everything
            break;
        
    }
    return Status;
}

VOID DirIterOpen(IN EFI_FILE *BaseDir, IN CHAR16 *RelativePath OPTIONAL, OUT REFIT_DIR_ITER *DirIter)
{
    if (RelativePath == NULL) {
        DirIter->LastStatus = EFI_SUCCESS;
        DirIter->DirHandle = BaseDir;
        DirIter->CloseDirHandle = FALSE;
    } else {
        DirIter->LastStatus = BaseDir->Open(BaseDir, &(DirIter->DirHandle), RelativePath, EFI_FILE_MODE_READ, 0);
        DirIter->CloseDirHandle = EFI_ERROR(DirIter->LastStatus) ? FALSE : TRUE;
    }
    DirIter->LastFileInfo = NULL;
}

BOOLEAN DirIterNext(IN OUT REFIT_DIR_ITER *DirIter, IN UINTN FilterMode, IN CHAR16 *FilePattern OPTIONAL,
                    OUT EFI_FILE_INFO **DirEntry)
{
    if (DirIter->LastFileInfo != NULL) {
        FreePool(DirIter->LastFileInfo);
        DirIter->LastFileInfo = NULL;
    }
    
    if (EFI_ERROR(DirIter->LastStatus))
        return FALSE;   // stop iteration
    
    for (;;) {
        DirIter->LastStatus = DirNextEntry(DirIter->DirHandle, &(DirIter->LastFileInfo), FilterMode);
        if (EFI_ERROR(DirIter->LastStatus))
            return FALSE;
        if (DirIter->LastFileInfo == NULL)  // end of listing
            return FALSE;
        if (FilePattern != NULL) {
            if ((DirIter->LastFileInfo->Attribute & EFI_FILE_DIRECTORY))
                break;
            if (MetaiMatch(DirIter->LastFileInfo->FileName, FilePattern))
                break;
            // else continue loop
        } else
            break;
    }
    
    *DirEntry = DirIter->LastFileInfo;
    return TRUE;
}

EFI_STATUS DirIterClose(IN OUT REFIT_DIR_ITER *DirIter)
{
    if (DirIter->LastFileInfo != NULL) {
        FreePool(DirIter->LastFileInfo);
        DirIter->LastFileInfo = NULL;
    }
    if (DirIter->CloseDirHandle)
        DirIter->DirHandle->Close(DirIter->DirHandle);
    return DirIter->LastStatus;
}

//
// file name manipulation
//
BOOLEAN 
MetaiMatch (
			IN CHAR16   *String,
			IN CHAR16   *Pattern
			)
{
	if (!mUnicodeCollation) {
		// quick fix for driver loading on UEFIs without UnicodeCollation
		//return FALSE;
		return TRUE;
	}
	return mUnicodeCollation->MetaiMatch (mUnicodeCollation, String, Pattern);
}


EFI_STATUS
InitializeUnicodeCollationProtocol (VOID)
{
	EFI_STATUS  Status;
	
	if (mUnicodeCollation != NULL) {
		return EFI_SUCCESS;
	}
	
	//
	// BUGBUG: Proper impelmentation is to locate all Unicode Collation Protocol
	// instances first and then select one which support English language.
	// Current implementation just pick the first instance.
	//
	Status = gBS->LocateProtocol (
								  &gEfiUnicodeCollation2ProtocolGuid,
								  NULL,
								  (VOID **) &mUnicodeCollation
								  );
	return Status;
}



CHAR16 * Basename(IN CHAR16 *Path)
{
    CHAR16  *FileName;
    UINTN   i;
    
    FileName = Path;
    
    if (Path != NULL) {
        for (i = StrLen(Path); i > 0; i--) {
            if (Path[i-1] == '\\' || Path[i-1] == '/') {
                FileName = Path + i;
                break;
            }
        }
    }
    
    return FileName;
}

VOID ReplaceExtension(IN OUT CHAR16 *Path, IN CHAR16 *Extension)
{
    UINTN i;
    
    for (i = StrLen(Path); i >= 0; i--) {
        if (Path[i] == '.') {
            Path[i] = 0;
            break;
        }
        if (Path[i] == '\\' || Path[i] == '/')
            break;
    }
    StrCat(Path, Extension);
}

//
// memory string search
//

INTN FindMem(IN VOID *Buffer, IN UINTN BufferLength, IN VOID *SearchString, IN UINTN SearchStringLength)
{
    UINT8 *BufferPtr;
    UINTN Offset;
    
    BufferPtr = Buffer;
    BufferLength -= SearchStringLength;
    for (Offset = 0; Offset < BufferLength; Offset++, BufferPtr++) {
        if (CompareMem(BufferPtr, SearchString, SearchStringLength) == 0)
            return (INTN)Offset;
    }
    
    return -1;
}

//
// Aptio UEFI returns File DevPath as 2 nodes (dir, file)
// and DevicePathToStr connects them with /, but we need '\\'
CHAR16 *FileDevicePathToStr(IN EFI_DEVICE_PATH_PROTOCOL *DevPath)
{
    CHAR16      *FilePath;
    CHAR16      *Char;
    
    FilePath = DevicePathToStr(DevPath);
    // fix / into '\\'
    if (FilePath != NULL) {
        for (Char = FilePath; *Char != L'\0'; Char++) {
            if (*Char == L'/') {
                *Char = L'\\';
            }
        }
    }
    return FilePath;
}

// EOF

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

#define DEBUG_LIB 2

#if DEBUG_LIB == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_LIB == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

// variables

EFI_HANDLE       SelfImageHandle;
EFI_LOADED_IMAGE *SelfLoadedImage;
EFI_FILE         *SelfRootDir;
EFI_FILE         *SelfDir;
CHAR16           *SelfDirPath;

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
static VOID ReinitVolumes(VOID);

BOOLEAN MetaiMatch (
			IN CHAR16   *String,
			IN CHAR16   *Pattern
			);
//
// self recognition stuff
//

EFI_STATUS InitRefitLib(IN EFI_HANDLE ImageHandle)
{
    EFI_STATUS  Status;
    CHAR16      *DevicePathAsString;
    CHAR16      BaseDirectory[256];
    UINTN       i;
    
    SelfImageHandle = ImageHandle;
    Status = gBS->HandleProtocol(SelfImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &SelfLoadedImage);
    if (CheckFatalError(Status, L"while getting a LoadedImageProtocol handle"))
        return EFI_LOAD_ERROR;
    
    /*
    if (SelfLoadedImage->LoadOptionsSize > 0) {
        CHAR16 Buffer[1024];
        UINTN Length = SelfLoadedImage->LoadOptionsSize / 2;
        if (Length > 1023)
            Length = 1023;
        CopyMem(Buffer, SelfLoadedImage->LoadOptions, SelfLoadedImage->LoadOptionsSize);
        Buffer[Length] = 0;
        Print(L"Load options: '%s'\n", Buffer);
        CheckError(EFI_LOAD_ERROR, L"FOR DEBUGGING");
    }
    */
    
    // find the current directory
    DevicePathAsString = DevicePathToStr(SelfLoadedImage->FilePath);
    if (DevicePathAsString != NULL) {
        StrCpy(BaseDirectory, DevicePathAsString);
        FreePool(DevicePathAsString);
        for (i = StrLen(BaseDirectory); i > 0 && BaseDirectory[i] != '\\'; i--) ;
        BaseDirectory[i] = 0;
    } else
        BaseDirectory[0] = 0;
    SelfDirPath = EfiStrDuplicate(BaseDirectory);
    
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
}

EFI_STATUS ReinitRefitLib(VOID)
{
    // called after running external programs to re-open file handles
    
    //EFI_STATUS  Status;
    
    ReinitVolumes();
    
    if (SelfVolume != NULL && SelfVolume->RootDir != NULL)
        SelfRootDir = SelfVolume->RootDir;
    
    return FinishInitRefitLib();
}

static EFI_STATUS FinishInitRefitLib(VOID)
{    
    EFI_STATUS  Status;
    
    if (SelfRootDir == NULL) {
        SelfRootDir = EfiLibOpenRoot(SelfLoadedImage->DeviceHandle);
        if (SelfRootDir == NULL) {
            CheckError(EFI_LOAD_ERROR, L"while (re)opening our installation volume");
            return EFI_LOAD_ERROR;
        }
    }
    
    Status = SelfRootDir->Open(SelfRootDir, &SelfDir, SelfDirPath, EFI_FILE_MODE_READ, 0);
    if (CheckFatalError(Status, L"while opening our installation directory"))
        return EFI_LOAD_ERROR;
    
    return EFI_SUCCESS;
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

VOID ExtractLegacyLoaderPaths(EFI_DEVICE_PATH **PathList, UINTN MaxPaths, EFI_DEVICE_PATH **HardcodedPathList)
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
        return;
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
}

//
// volume functions
//

static VOID ScanVolumeBootcode(IN OUT REFIT_VOLUME *Volume, OUT BOOLEAN *Bootable)
{
    EFI_STATUS              Status;
    UINT8                   SectorBuffer[2048];
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
    
    // look at the boot sector (this is used for both hard disks and El Torito images!)
    Status = Volume->BlockIO->ReadBlocks(Volume->BlockIO, Volume->BlockIO->Media->MediaId,
                                         Volume->BlockIOOffset /*start lba*/,
                                         2048, SectorBuffer);
    if (!EFI_ERROR(Status)) {
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
        
      } else { //HDD
        if (*((UINT16 *)(SectorBuffer + 510)) == 0xaa55 && SectorBuffer[0] != 0) {
          *Bootable = TRUE;
          Volume->HasBootCode = TRUE;
          DBG("The volume has bootcode\n");
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
          Volume->BootType = BOOTING_BY_EFI;

          
        } else if (FindMem(SectorBuffer, 512, "Geom\0Hard Disk\0Read\0 Error", 26) >= 0) {   // GRUB
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"grub,linux";
          Volume->OSName = L"Linux";
          
        } else if ((*((UINT32 *)(SectorBuffer)) == 0x8ec031fa &&
                    *((UINT16 *)(SectorBuffer + 510)) == 0xaa55) ||
                   FindMem(SectorBuffer, 2048, L"boot", 10) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"mac";
          Volume->OSName = L"MacOSX";
          Volume->OSType = OSTYPE_OSX;
          Volume->BootType = BOOTING_BY_EFI;
          DBG("Detected MacOSX bootcode\n");

          
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
          Volume->OSIconName = L"winvista,win";
          Volume->OSName = L"Windows";
          Volume->OSType = OSTYPE_WIN;
          Volume->BootType = BOOTING_BY_EFI;
          
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
          Volume->OSIconName = L"zeta,beos";
          Volume->OSName = L"ZETA";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else if (FindMem(SectorBuffer, 512, "\x04" "beos\x06" "system\x05" "zbeos", 18) >= 0 ||
                   FindMem(SectorBuffer, 512, "\x06" "system\x0c" "haiku_loader", 20) >= 0) {
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"haiku,beos";
          Volume->OSName = L"Haiku";
          Volume->OSType = OSTYPE_VAR;
          Volume->BootType = BOOTING_BY_PBR;
          
        } else {
          DBG("unknown bootcode %x\n", *(UINT32*)&SectorBuffer[0]);
          Volume->HasBootCode = TRUE;
          Volume->OSIconName = L"mac";
          Volume->OSName = L"MacOSX";
          Volume->OSType = OSTYPE_OSX;
          Volume->BootType = BOOTING_BY_EFI;          
        }
      
      }

        
        // NOTE: If you add an operating system with a name that starts with 'W' or 'L', you
        //  need to fix AddLegacyEntry in main.c.
        
#if REFIT_DEBUG > 0
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
            }
        }
        
    } else {
#if REFIT_DEBUG > 0
        CheckError(Status, L"while reading boot sector");
#endif
    }
}

static VOID ScanVolumeDefaultIcon(IN OUT REFIT_VOLUME *Volume)
{
    // default volume icon based on disk kind
    if (Volume->DiskKind == DISK_KIND_INTERNAL)
        Volume->VolBadgeImage = BuiltinIcon(BUILTIN_ICON_VOL_INTERNAL);
    else if (Volume->DiskKind == DISK_KIND_EXTERNAL)
        Volume->VolBadgeImage = BuiltinIcon(BUILTIN_ICON_VOL_EXTERNAL);
    else if (Volume->DiskKind == DISK_KIND_OPTICAL)
        Volume->VolBadgeImage = BuiltinIcon(BUILTIN_ICON_VOL_OPTICAL);
}

EFI_FILE_SYSTEM_INFO * LibFileSystemInfo (IN EFI_FILE_HANDLE   Root)
{
	EFI_STATUS                Status = EFI_NOT_FOUND;
	EFI_FILE_SYSTEM_INFO*			FileSystemInfo = NULL;
	UINT32                    BufferSizeVolume;
		
	BufferSizeVolume =	SIZE_OF_EFI_FILE_SYSTEM_INFO + 255;
	FileSystemInfo = AllocateZeroPool(BufferSizeVolume);
	
	//
	// check file system info
	//
	if(FileSystemInfo)
	{
    while (EfiGrowBuffer (&Status, (VOID **) &FileSystemInfo, BufferSizeVolume)) {
      Status = Root->GetInfo(Root, &gEfiFileSystemInfoGuid, (UINTN*)&BufferSizeVolume, FileSystemInfo);
    }
		
	}
	return EFI_ERROR(Status)?NULL:FileSystemInfo;
}
	
static VOID ScanVolume(IN OUT REFIT_VOLUME *Volume)
{
    EFI_STATUS              Status;
    EFI_DEVICE_PATH         *DevicePath, *NextDevicePath;
    EFI_DEVICE_PATH         *DiskDevicePath, *RemainingDevicePath;
    EFI_HANDLE              WholeDiskHandle;
    UINTN                   PartialLength;
    EFI_FILE_SYSTEM_INFO    *FileSystemInfoPtr;
    BOOLEAN                 Bootable;
    
    // get device path
    Volume->DevicePath = DuplicateDevicePath(DevicePathFromHandle(Volume->DeviceHandle));
#if REFIT_DEBUG > 0
    if (Volume->DevicePath != NULL) {
        Print(L"* %s\n", DevicePathToStr(Volume->DevicePath));
#if REFIT_DEBUG >= 2
        DumpHex(1, 0, DevicePathSize(Volume->DevicePath), Volume->DevicePath);
#endif
    }
#endif
    
    Volume->DiskKind = DISK_KIND_INTERNAL;  // default
    
    // get block i/o
    Status = gBS->HandleProtocol(Volume->DeviceHandle, &gEfiBlockIoProtocolGuid, (VOID **) &(Volume->BlockIO));
    if (EFI_ERROR(Status)) {
        Volume->BlockIO = NULL;
        DBG("Warning: Can't get BlockIO protocol.\n");
    } else {
      if (Volume->BlockIO->Media->BlockSize == 2048){
        Volume->DiskKind = DISK_KIND_OPTICAL;
        Volume->BlockIOOffset = 0x10;
      } else {
        Volume->BlockIOOffset = 0;
      }

    }
    
    // scan for bootcode and MBR table
    Bootable = FALSE;
    ScanVolumeBootcode(Volume, &Bootable);
  DBG("ScanVolumeBootcode success\n");
    // detect device type
    DevicePath = Volume->DevicePath;
    while (DevicePath != NULL && !IsDevicePathEndType(DevicePath)) {
        NextDevicePath = NextDevicePathNode(DevicePath);
        
      if (DevicePathType(DevicePath) == MESSAGING_DEVICE_PATH && 
          (DevicePathSubType(DevicePath) == MSG_USB_DP || DevicePathSubType(DevicePath) == MSG_USB_CLASS_DP))
      {
        DBG("USB volume\n");
        Volume->DiskKind = DISK_KIND_EXTERNAL; 
      }
      // FIREWIRE Devices
      if (DevicePathType(DevicePath) == MESSAGING_DEVICE_PATH && 
          (DevicePathSubType(DevicePath) == MSG_1394_DP || DevicePathSubType(DevicePath) == MSG_FIBRECHANNEL_DP))
      {
        Volume->DiskKind = DISK_KIND_FIREWIRE; 
      }
      // CD-ROM Devices
      if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && 
          DevicePathSubType(DevicePath) == MEDIA_CDROM_DP) 
      {
        Volume->DiskKind = DISK_KIND_OPTICAL;     
      }
      // VENDOR Specific Path
      if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && 
          DevicePathSubType(DevicePath) == MEDIA_VENDOR_DP) 
      {
        Volume->DiskKind = DISK_KIND_NODISK;
      }
      // LEGACY CD-ROM
      if (DevicePathType(DevicePath) == BBS_DEVICE_PATH && 
          (DevicePathSubType(DevicePath) == BBS_BBS_DP || DevicePathSubType(DevicePath) == BBS_TYPE_CDROM)) 
      {
        Volume->DiskKind = DISK_KIND_OPTICAL;
      }
      // LEGACY HARDDISK
      if (DevicePathType(DevicePath) == BBS_DEVICE_PATH && 
          (DevicePathSubType(DevicePath) == BBS_BBS_DP || DevicePathSubType(DevicePath) == BBS_TYPE_HARDDRIVE)) 
      {
        DBG("LEGACY HARDDISK\n");
        Volume->DiskKind = DISK_KIND_INTERNAL;
      }
      Bootable = TRUE;
      
      if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH &&
            DevicePathSubType(DevicePath) == MEDIA_VENDOR_DP) {
            Volume->IsAppleLegacy = TRUE;             // legacy BIOS device entry
            // TODO: also check for Boot Camp GUID
            Bootable = FALSE;   // this handle's BlockIO is just an alias for the whole device
          Print(L"AppleLegacy device\n");
        }
        
        if (DevicePathType(DevicePath) == MESSAGING_DEVICE_PATH) {
            // make a device path for the whole device
            PartialLength = (UINT8 *)NextDevicePath - (UINT8 *)(Volume->DevicePath);
            DiskDevicePath = (EFI_DEVICE_PATH *)AllocatePool(PartialLength + sizeof(EFI_DEVICE_PATH));
            CopyMem(DiskDevicePath, Volume->DevicePath, PartialLength);
            CopyMem((UINT8 *)DiskDevicePath + PartialLength, NextDevicePath, sizeof(EFI_DEVICE_PATH)); //EndDevicePath
            
            // get the handle for that path
            RemainingDevicePath = DiskDevicePath;
            //Print(L"  * looking at %s\n", DevicePathToStr(RemainingDevicePath));
            Status = gBS->LocateDevicePath(&gEfiBlockIoProtocolGuid, &RemainingDevicePath, &WholeDiskHandle);
            //Print(L"  * remaining: %s\n", DevicePathToStr(RemainingDevicePath));
            FreePool(DiskDevicePath);
            
            if (!EFI_ERROR(Status)) {
                //Print(L"  - original handle: %08x - disk handle: %08x\n", (UINT32)DeviceHandle, (UINT32)WholeDiskHandle);
                
                // get the device path for later
                Status = gBS->HandleProtocol(WholeDiskHandle, &gEfiDevicePathProtocolGuid, (VOID **) &DiskDevicePath);
                if (!EFI_ERROR(Status)) {
                    Volume->WholeDiskDevicePath = DuplicateDevicePath(DiskDevicePath);
                }
                
                // look at the BlockIO protocol
                Status = gBS->HandleProtocol(WholeDiskHandle, &gEfiBlockIoProtocolGuid, (VOID **) &Volume->WholeDiskBlockIO);
                if (!EFI_ERROR(Status)) {
                    
                    // check the media block size
                    if (Volume->WholeDiskBlockIO->Media->BlockSize == 2048)
                        Volume->DiskKind = DISK_KIND_OPTICAL;
                    
                } else {
                    Volume->WholeDiskBlockIO = NULL;
                  DBG("no WholeDiskBlockIO\n");
                    //CheckError(Status, L"from HandleProtocol");
                }
            } //else
              //  CheckError(Status, L"from LocateDevicePath");
        }
      DBG("NextDevicePath\n");
        DevicePath = NextDevicePath;
    }
    
    if (!Bootable) {
#if REFIT_DEBUG > 0
        if (Volume->HasBootCode)
            Print(L"  Volume considered non-bootable, but boot code is present\n");
#endif
        Volume->HasBootCode = FALSE;
    }
  DBG("search volume icon\n");
    // default volume icon based on disk kind
    ScanVolumeDefaultIcon(Volume);
    DBG("search volume icon finished\n");
    // open the root directory of the volume
    Volume->RootDir = EfiLibOpenRoot(Volume->DeviceHandle);
    if (Volume->RootDir == NULL) {
        //Print(L"Error: Can't open volume.\n");
        // TODO: signal that we had an error
      //Slice - there is LegacyBoot volume
      //properties are set before
        DBG("LegacyBoot volume\n");
  //    Volume->VolName =  L"Legacy OS";
      return;
    }
    DBG("RootDir found\n");
    // get volume name
    FileSystemInfoPtr = LibFileSystemInfo(Volume->RootDir);
    if (FileSystemInfoPtr != NULL) {
      Print(L"  Volume %s\n", FileSystemInfoPtr->VolumeLabel);
      Volume->VolName = EfiStrDuplicate(FileSystemInfoPtr->VolumeLabel);
      Status = GetOSVersion(Volume); //here we set tiger,leo,snow,lion
      if (EFI_ERROR(Status)) {
       // Volume->OSType = 0; //TODO - other criteria?
       // Other EFI systems?
      }
      
      FreePool(FileSystemInfoPtr);
    } else {
        Print(L"Warning: Can't get volume info.\n");
        // NOTE: this is normal for Apple's VenMedia device paths
    }
    DBG("VolName found\n");
    // TODO: if no official volume name is found or it is empty, use something else, e.g.:
    //   - name from bytes 3 to 10 of the boot sector
    //   - partition number
    //   - name derived from file system type or partition type
    
    // get custom volume icon if present
  if (FileExists(Volume->RootDir, L".VolumeIcon.icns")){
        Volume->VolBadgeImage = LoadIcns(Volume->RootDir, L".VolumeIcon.icns", 32);
    DBG("VolBadgeImage found\n");
  }
  DBG("ScanVolume finished\n");
}

static VOID ScanExtendedPartition(REFIT_VOLUME *WholeDiskVolume, MBR_PARTITION_INFO *MbrEntry)
{
    EFI_STATUS              Status;
    REFIT_VOLUME            *Volume;
    UINT32                  ExtBase, ExtCurrent, NextExtCurrent;
    UINTN                   i;
    UINTN                   LogicalPartitionIndex = 4;
    UINT8                   SectorBuffer[512];
    BOOLEAN                 Bootable;
    MBR_PARTITION_INFO      *EMbrTable;
    
    ExtBase = MbrEntry->StartLBA;
    
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
                
                ScanVolumeDefaultIcon(Volume);
                
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
    EFI_HANDLE              *Handles;
    REFIT_VOLUME            *Volume, *WholeDiskVolume;
    UINTN                   VolumeIndex, VolumeIndex2;
    MBR_PARTITION_INFO      *MbrTable;
    UINTN                   PartitionIndex;
    UINT8                   *SectorBuffer1, *SectorBuffer2;
    UINTN                   SectorSum, i;
    
    DBG("Scanning volumes...\n");
    
    // get all filesystem handles
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &Handles);
    // was: &FileSystemProtocol
    if (Status == EFI_NOT_FOUND)
        return;  
    if (CheckError(Status, L"while listing all BlockIo"))
      //Slice - what is the case for that?
        return;
    
    // first pass: collect information about all handles
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        
        Volume = AllocateZeroPool(sizeof(REFIT_VOLUME));
        Volume->DeviceHandle = Handles[HandleIndex];
        ScanVolume(Volume);
      DBG("Found Volume at index=%x\n", HandleIndex);
        AddListElement((VOID ***) &Volumes, &VolumesCount, Volume);
        
        if (Volume->DeviceHandle == SelfLoadedImage->DeviceHandle)
            SelfVolume = Volume;
        
    }
    FreePool(Handles);
  DBG("Fount %d volumes\n", VolumesCount);
    if (SelfVolume == NULL)
        Print(L"WARNING: SelfVolume not found"); //Slice - and what?
    
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
            SectorBuffer1 = AllocatePool(512);
            SectorBuffer2 = AllocatePool(512);
            
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

static VOID ReinitVolumes(VOID)
{
    EFI_STATUS              Status;
    REFIT_VOLUME            *Volume;
    UINTN                   VolumeIndex;
    EFI_DEVICE_PATH         *RemainingDevicePath;
    EFI_HANDLE              DeviceHandle, WholeDiskHandle;
    
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        Volume = Volumes[VolumeIndex];
        
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
}

//
// file and dir functions
//

BOOLEAN FileExists(IN EFI_FILE *BaseDir, IN CHAR16 *RelativePath)
{
    EFI_STATUS  Status;
    EFI_FILE    *TestFile;
    
    Status = BaseDir->Open(BaseDir, &TestFile, RelativePath, EFI_FILE_MODE_READ, 0);
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
        Buffer = AllocatePool(BufferSize);
        for (IterCount = 0; ; IterCount++) {
            Status = Directory->Read(Directory, &BufferSize, Buffer);
            if (Status != EFI_BUFFER_TOO_SMALL || IterCount >= 4)
                break;
            if (BufferSize <= LastBufferSize) {
                Print(L"FS Driver requests bad buffer size %d (was %d), using %d instead\n", BufferSize, LastBufferSize, LastBufferSize * 2);
                BufferSize = LastBufferSize * 2;
#if REFIT_DEBUG > 0
            } else {
                Print(L"Reallocating buffer from %d to %d\n", LastBufferSize, BufferSize);
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
		return FALSE;
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

// EOF

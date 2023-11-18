/*
 * Volume.h
 *
 *  Created on: Apr 21, 2021
 *      Author: jief
 */

#ifndef PLATFORM_VOLUME_H_
#define PLATFORM_VOLUME_H_

#include "../include/VolumeTypes.h"
#include <Efi.h>
#include "../cpp_foundation/apd.h"
#include "../cpp_foundation/XString.h"
#include "../libeg/libeg.h"
#include "../Platform/guid.h"

class LEGACY_OS
{
public:
  UINT8         Type = 0;
  XStringW      IconName = XStringW();
  XStringW      Name = XStringW();
};

class REFIT_VOLUME {
public:
  EFI_DEVICE_PATH     *DevicePath = 0;
  EFI_HANDLE          DeviceHandle = 0;
  EFI_FILE            *RootDir = 0;
  XStringW            DevicePathString = XStringW();
  XStringW            VolName = XStringW(); // comes from EfiLibFileSystemInfo, EfiLibFileSystemVolumeLabelInfo, "EFI" if gEfiPartTypeSystemPartGuid or "Unknown HD"
  XStringW            VolLabel = XStringW(); // comes from \\.VolumeLabel.txt, or empty.
  UINT8               DiskKind = 0;
  LEGACY_OS           LegacyOS = LEGACY_OS();
  XBool               Hidden = false;
  UINT8               BootType = 0;
  XBool               IsAppleLegacy = false;
  XBool               HasBootCode = false;
  XBool               IsMbrPartition = false;
  UINTN               MbrPartitionIndex = 0;
  EFI_BLOCK_IO        *BlockIO = 0;
  UINT64              BlockIOOffset = 0;
  EFI_BLOCK_IO        *WholeDiskBlockIO = 0;
  apd<EFI_DEVICE_PATH*> WholeDiskDevicePath = NULL;
  EFI_HANDLE          WholeDiskDeviceHandle = 0;
  MBR_PARTITION_INFO  *MbrPartitionTable = 0;
  UINT32              DriveCRC32 = 0;
  EFI_GUID       RootUUID = EFI_GUID(); //for recovery it is UUID of parent partition
  UINT64              SleepImageOffset = 0;
  XStringW            osxVolumeName = XStringW(); // comes from \\System\\Library\\CoreServices\\.disk_label.contentDetails, or empty.
  EFI_GUID       ApfsFileSystemUUID = EFI_GUID(); // apfs file system UUID of that partition. It's not the UUID of subfolder like in Preboot.
  EFI_GUID       ApfsContainerUUID = EFI_GUID();
  APPLE_APFS_VOLUME_ROLE  ApfsRole = 0;
  XObjArray<EFI_GUID> ApfsTargetUUIDArray = XObjArray<EFI_GUID>(); // this is the array of folders that are named as UUID
  UINTN              Index = 0;

  REFIT_VOLUME() {};
  REFIT_VOLUME(const REFIT_VOLUME& other) = delete; // Can be defined if needed
  const REFIT_VOLUME& operator = ( const REFIT_VOLUME & ) = delete; // Can be defined if needed
  ~REFIT_VOLUME() {}

  const XStringW getVolLabelOrOSXVolumeNameOrVolName() {
    if ( VolLabel.notEmpty() ) return VolLabel;
    if ( osxVolumeName.notEmpty() ) return osxVolumeName;
    return VolName;
  }
};




EFI_STATUS GetRootUUID(IN OUT REFIT_VOLUME *Volume);


#endif /* PLATFORM_VOLUME_H_ */

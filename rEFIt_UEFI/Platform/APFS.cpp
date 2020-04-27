/*
 * APFS.cpp
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#include "Platform.h"
#include "guid.h"


/*  S. Mitrofanov 08.06.2016
 *  APFS Container introduced new partitions structure
 *  Now we have, for example:
 * /dev/disk0 (internal, physical):
 *   #:                       TYPE NAME                    SIZE       IDENTIFIER
 *   0:      GUID_partition_scheme                        *240.1 GB   disk0
 *   1:                        EFI EFI                     209.7 MB   disk0s1
 *   2:                 Apple_APFS Container disk1         239.2 GB   disk0s2
 *   3:       Apple_KernelCoreDump                         655.4 MB   disk0s3
 *
 * /dev/disk1 (synthesized):
 *   #:                       TYPE NAME                    SIZE       IDENTIFIER
 *   0:      APFS Container Scheme -                      +239.2 GB   disk1
 *                                 Physical Store disk0s2
 *   1:                APFS Volume Macintosh SSD           170.8 GB   disk1s1
 *   2:                APFS Volume Preboot                 17.9 MB    disk1s2
 *   3:                APFS Volume Recovery                521.1 MB   disk1s3
 *   4:                APFS Volume VM                      1.1 GB     disk1s4
 */
UINTN    APFSUUIDBankCounter = 0;
UINT8   *APFSUUIDBank        = NULL;
//Vednor APFS device path signature
//BE74FCF7-0B7C-49F3-9147-01F4042E6842
EFI_GUID APFSSignature       = {0xBE74FCF7, 0x0B7C, 0x49F3, { 0x91, 0x47, 0x01, 0xf4, 0x04, 0x2E, 0x68, 0x42 }};
BOOLEAN                 APFSSupport     = FALSE;



//Function for obtaining unique part id from APFS partition
//IN DevicePath
//Out: EFI_GUID
//null if it is not APFS part
EFI_GUID *APFSPartitionUUIDExtract(
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath
  )
{
  while (!IsDevicePathEndType(DevicePath) &&
         !(DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (DevicePath) == MEDIA_VENDOR_DP)) {
    DevicePath = NextDevicePathNode(DevicePath);
  }
  if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (DevicePath) == MEDIA_VENDOR_DP) {
    //Check that vendor-assigned GUID defines APFS Container Partition
    if ( GuidLEToStr((EFI_GUID *)((UINT8 *)DevicePath+0x04)).equalIC(GuidLEToStr(&APFSSignature)) ) {
      return (EFI_GUID *)((UINT8 *)DevicePath+0x14);
    }
  }
  return NULL;
}

UINT8 *APFSContainer_Support(VOID) {
  /*
   * S. Mtr 2017
   * APFS Container partition support
   * Gather System PartitionUniqueGUID
   * edit: 17.06.2017
   * Fiil UUIDBank only with APFS container UUIDs
   */
  UINTN                     VolumeIndex;
  REFIT_VOLUME             *Volume;
  EFI_GUID                 *TmpUUID    = NULL;

  //Fill APFSUUIDBank
  APFSUUIDBank = (__typeof__(APFSUUIDBank))AllocateZeroPool(0x10*Volumes.size());
  for (VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++) {
    Volume = &Volumes[VolumeIndex];
    //Check that current volume - apfs partition
    if ((TmpUUID = APFSPartitionUUIDExtract(Volume->DevicePath)) != NULL){
      CopyMem(APFSUUIDBank+APFSUUIDBankCounter*0x10,(UINT8 *)TmpUUID,0x10);
      APFSUUIDBankCounter++;
    }
  }
  return APFSUUIDBank;
}


/*
 * APFS.cpp
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

#include "guid.h"
#include "APFS.h"


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


/*
 * Function for obtaining unique part id from APFS partition
 *   IN: DevicePath
 *   OUT: EFI_GUID
 *   returns null if it is not APFS part
 */
EFI_GUID APFSPartitionUUIDExtract(
    const EFI_DEVICE_PATH_PROTOCOL *DevicePath
  )
{
  while (!IsDevicePathEndType(DevicePath) &&
         !(DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (DevicePath) == MEDIA_VENDOR_DP)) {
    DevicePath = NextDevicePathNode(DevicePath);
  }
  if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType (DevicePath) == MEDIA_VENDOR_DP) {
    //Check that vendor-assigned EFI_GUID defines APFS Container Partition
    if ( ApfsSignatureUUID == *(const EFI_GUID *)((UINT8 *)DevicePath+0x04) ) {
      return *(EFI_GUID *)((UINT8 *)DevicePath+0x14);
    }
  }
  return nullGuid;
}




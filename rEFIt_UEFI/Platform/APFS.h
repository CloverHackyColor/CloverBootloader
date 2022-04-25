/*
 * APFS.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_APFS_H_
#define PLATFORM_APFS_H_

extern "C" {
#include <Protocol/DevicePath.h>
}

// The conversion from string litteral to EFI_GUID is made at compile time !!! Thanks constexpr.
constexpr const EFI_GUID ApfsSignatureUUID = "BE74FCF7-0B7C-49F3-9147-01F4042E6842"_guid;

/*
 * Function for obtaining unique part id from APFS partition
 *   IN: DevicePath
 *   OUT: EFI_GUID
 *   returns null if it is not APFS part
 */
EFI_GUID APFSPartitionUUIDExtract(
    const EFI_DEVICE_PATH_PROTOCOL *DevicePath
  );


#endif /* PLATFORM_APFS_H_ */

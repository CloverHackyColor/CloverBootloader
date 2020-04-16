/*
 * APFS.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_APFS_H_
#define PLATFORM_APFS_H_

/* Switch for APFS support */
extern UINTN 						  APFSUUIDBankCounter;
extern UINT8 						 *APFSUUIDBank;
extern EFI_GUID                        APFSSignature;
extern BOOLEAN                         APFSSupport;


UINT8 *APFSContainer_Support(VOID);

//Function for obtaining unique part id from APFS partition
//IN DevicePath
//Out: EFI_GUID
//null if it is not APFS part
EFI_GUID *APFSPartitionUUIDExtract(
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath
  );



#endif /* PLATFORM_APFS_H_ */

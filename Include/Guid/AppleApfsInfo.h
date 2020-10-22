/** @file
  Copyright (C) 2017, CupertinoNet.  All rights reserved.<BR>

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
**/

#ifndef APPLE_APFS_INFO_H_
#define APPLE_APFS_INFO_H_

#define APPLE_APFS_PARTITION_TYPE_GUID  \
  { 0x7C3457EF, 0x0000, 0x11AA,         \
    { 0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC } }

extern EFI_GUID gAppleApfsPartitionTypeGuid;

#define APPLE_APFS_CONTAINER_INFO_GUID  \
  { 0x3533CF0D, 0x685F, 0x5EBF,         \
    { 0x8D, 0xC6, 0x73, 0x93, 0x48, 0x5B, 0xAF, 0xA2 } }

typedef struct {
  UINT32 Always1;
  GUID   Uuid;
} APPLE_APFS_CONTAINER_INFO;

extern EFI_GUID gAppleApfsContainerInfoGuid;

#define APPLE_APFS_VOLUME_INFO_GUID  \
  { 0x900C7693, 0x8C14, 0x58BA,      \
    { 0xB4, 0x4E, 0x97, 0x45, 0x15, 0xD2, 0x7C, 0x78 } }


#define APPLE_APFS_VOLUME_ROLE_RECOVERY  BIT2  // 0x04
#define APPLE_APFS_VOLUME_ROLE_PREBOOT   BIT4  // 0x10
#define APPLE_APFS_VOLUME_ROLE_DATA      BIT6  // 0x40
#define APPLE_APFS_VOLUME_ROLE_VM        BIT3  // 0x08
#define APPLE_APFS_VOLUME_ROLE_SYSTEM    BIT0  // 0x01 Main partition. Joint with DATA.
//#define APPLE_APFS_VOLUME_ROLE_UPDATE    0xC0  // I got 0xC0, maybe the meaning is DATA + UPDATE ?

typedef UINT32 APPLE_APFS_VOLUME_ROLE;

typedef struct {
  UINT32                 Always1;
  GUID                   Uuid;
  APPLE_APFS_VOLUME_ROLE Role;
} APPLE_APFS_VOLUME_INFO;

extern EFI_GUID gAppleApfsVolumeInfoGuid;

#endif // APPLE_APFS_H_

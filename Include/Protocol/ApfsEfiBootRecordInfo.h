/** @file
  Apple FileSystem EfiBootRecord container info.

  Copyright (C) 2018, savvas.  All rights reserved.<BR>
  Copyright (C) 2018, CupertinoNet.  All rights reserved.<BR>

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

#ifndef APFS_EFIBOOTRECORD_INFO_PROTOCOL_H_
#define APFS_EFIBOOTRECORD_INFO_PROTOCOL_H_

#define APFS_EFIBOOTRECORD_INFO_PROTOCOL_GUID \
  { 0x03B8D751, 0xA02F, 0x4FF8,               \
    {0x9B, 0x1A, 0x55, 0x24, 0xAF, 0xA3, 0x94, 0x5F } }

typedef struct  _APFS_EFIBOOTRECORD_LOCATION_INFO
{
  //
  // Handle of partition which contain EfiBootRecord section
  //
  EFI_HANDLE  ControllerHandle;
  //
  // UUID of GPT container partition
  //
  EFI_GUID    ContainerUuid;
} APFS_EFIBOOTRECORD_LOCATION_INFO;

extern EFI_GUID gApfsEfiBootRecordInfoProtocolGuid;

#endif // APFS_EFIBOOTRECORD_INFO_PROTOCOL_H_

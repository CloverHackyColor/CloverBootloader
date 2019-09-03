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

#ifndef APPLE_PARTITION_INFO_H_
#define APPLE_PARTITION_INFO_H_

// APPLE_PARTITION_INFO_PROTOCOL_GUID
#define APPLE_PARTITION_INFO_PROTOCOL_GUID  \
  { 0x68425EE5, 0x1C43, 0x4BAA,             \
    { 0x84, 0xF7, 0x9A, 0xA8, 0xA4, 0xD8, 0xE1, 0x1E } }

// APPLE_PARTITION_INFO_PROTOCOL
typedef struct {
  UINT32   Revision;
  UINT32   PartitionNumber;
  UINT64   PartitionStart;
  UINT64   PartitionSize;
  UINT8    Signature[16];
  UINT8    MBRType;
  UINT8    SignatureType;
  UINT64   Attributes;
  CHAR16   PartitionName[36];
  UINT8    PartitionType[16];
} APPLE_PARTITION_INFO_PROTOCOL;

// gApplePartitionInfoProtocolGuid
extern EFI_GUID gApplePartitionInfoProtocolGuid;

#endif // APPLE_PARTITION_INFO_H_

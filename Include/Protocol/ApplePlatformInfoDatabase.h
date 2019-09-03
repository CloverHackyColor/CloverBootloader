/** @file
  Copyright (C) 2005 - 2015, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#ifndef APPLE_PLATFORM_INFO_DATABASE_H_
#define APPLE_PLATFORM_INFO_DATABASE_H_

// APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_GUID
#define APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_GUID        \
  { 0xAC5E4829, 0xA8FD, 0x440B,                           \
    { 0xAF, 0x33, 0x9F, 0xFE, 0x01, 0x3B, 0x12, 0xD8 } }

typedef struct _APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL;

// PLATFORM_INFO_GET_FIRST_DATA
typedef
EFI_STATUS
(EFIAPI *PLATFORM_INFO_GET_FIRST_DATA)(
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT VOID                                   *Data, OPTIONAL
  IN OUT UINTN                                  *Size
  );

// PLATFORM_INFO_GET_FIRST_DATA_SIZE 
typedef
EFI_STATUS
(EFIAPI *PLATFORM_INFO_GET_FIRST_DATA_SIZE)(
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT UINTN                                  *Size
  );

// PLATFORM_INFO_GET_DATA
typedef
EFI_STATUS
(EFIAPI *PLATFORM_INFO_GET_DATA)(
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  Index, OPTIONAL
  IN OUT VOID                                   *Data, OPTIONAL
  IN OUT UINTN                                  *Size
  );

// PLATFORM_INFO_GET_DATA_SIZE
typedef
EFI_STATUS
(EFIAPI *PLATFORM_INFO_GET_DATA_SIZE)(
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid, OPTIONAL
  IN     UINTN                                  Index, OPTIONAL
  IN OUT UINTN                                  *Size
  );

// APPLE_PLATFORM_INFO_DATABASE_PROTOCOL
struct _APPLE_PLATFORM_INFO_DATABASE_PROTOCOL {
  UINTN                             Revision;          ///<
  PLATFORM_INFO_GET_FIRST_DATA      GetFirstData;      ///<
  PLATFORM_INFO_GET_FIRST_DATA_SIZE GetFirstDataSize;  ///<
  PLATFORM_INFO_GET_DATA            GetData;           ///<
  PLATFORM_INFO_GET_DATA_SIZE       GetDataSize;       ///<
};

// gApplePlatformInfoDatabaseProtocolGuid
extern EFI_GUID gApplePlatformInfoDatabaseProtocolGuid;

#endif // APPLE_PLATFORM_INFO_DATABASE_H_

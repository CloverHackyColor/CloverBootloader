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

#ifndef EFI_OS_INFO_H_
#define EFI_OS_INFO_H_

// EFI_OS_INFO_PROTOCOL_GUID
#define EFI_OS_INFO_PROTOCOL_GUID                         \
  { 0xC5C5DA95, 0x7D5C, 0x45E6,                           \
    { 0xB2, 0xF1, 0x3F, 0xD5, 0x2B, 0xB1, 0x00, 0x77 } }

typedef struct _EFI_OS_INFO_PROTOCOL EFI_OS_INFO_PROTOCOL;

// OS_INFO_OS_VENDOR
typedef
VOID
(EFIAPI *OS_INFO_OS_VENDOR)(

  IN CHAR8  *OSName
  );

// OS_INFO_OS_NAME
typedef
VOID
(EFIAPI *OS_INFO_OS_NAME)(

  IN CHAR8  *OSName
  );

typedef
VOID
(EFIAPI *OS_INFO_EMPTY)(

IN CHAR8  *OSName
);


// EFI_OS_INFO_PROTOCOL
struct _EFI_OS_INFO_PROTOCOL {
  UINT64            Revision;  ///<
  OS_INFO_OS_NAME   OSName;    ///<
  OS_INFO_OS_VENDOR OSVendor;  ///<
  OS_INFO_EMPTY     OSEmpty;    ///<
};

// gEfiOSInfoProtocolGuid
extern EFI_GUID gEfiOSInfoProtocolGuid;

#endif // EFI_OS_INFO_H_

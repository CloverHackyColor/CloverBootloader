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

#ifndef EFI_KEYBOARD_INFORMATION_H_
#define EFI_KEYBOARD_INFORMATION_H_

// EFI_KEYBOARD_INFO_PROTOCOL_GUID
#define EFI_KEYBOARD_INFO_PROTOCOL_GUID                   \
  { 0xE82A0A1E, 0x0E4D, 0x45AC,                           \
    { 0xA6, 0xDC, 0x2A, 0xE0, 0x58, 0x00, 0xD3, 0x11 } }

// KEYBOARD_INFO_GET_INFO
typedef
EFI_STATUS
(EFIAPI *KEYBOARD_INFO_GET_INFO)(
  OUT UINT16  *IdVendor,
  OUT UINT16  *IdProduct,
  OUT UINT8   *CountryCode
  );

// EFI_KEYBOARD_INFO_PROTOCOL
typedef struct {
  KEYBOARD_INFO_GET_INFO GetInfo;  ///< 
} EFI_KEYBOARD_INFO_PROTOCOL;

// gEfiKeyboardInfoProtocolGuid
extern EFI_GUID gEfiKeyboardInfoProtocolGuid;

#endif // EFI_KEYBOARD_INFORMATION_H_

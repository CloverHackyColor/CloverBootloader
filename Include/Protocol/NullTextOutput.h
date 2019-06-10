/** @file

APFS Driver Loader - loads apfs.efi from EfiBootRecord block

Copyright (c) 2017-2018, savvas
Copyright (c) 2018, vit9696

All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#ifndef NULL_TEXT_OUTPUT_PROTOCOL_H_
#define NULL_TEXT_OUTPUT_PROTOCOL_H_

STATIC
EFI_STATUS
EFIAPI
NullTextReset (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN BOOLEAN                         ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NullTextOutputString (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN CHAR16                          *String
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NullTextTestString (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN CHAR16                          *String
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NullTextQueryMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN  UINTN                           ModeNumber,
  OUT UINTN                           *Columns,
  OUT UINTN                           *Rows
  )
{
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
NullTextSetMode (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN UINTN                           ModeNumber
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NullTextSetAttribute (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN UINTN                           Attribute
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NullTextClearScreen (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NullTextSetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN UINTN                           Column,
  IN UINTN                           Row
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
NullTextEnableCursor (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN BOOLEAN                         Visible
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_SIMPLE_TEXT_OUTPUT_MODE
mNullTextOutputMode;

STATIC
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
mNullTextOutputProtocol = {
  NullTextReset,
  NullTextOutputString,
  NullTextTestString,
  NullTextQueryMode,
  NullTextSetMode,
  NullTextSetAttribute,
  NullTextClearScreen,
  NullTextSetCursorPosition,
  NullTextEnableCursor,
  &mNullTextOutputMode
};

extern EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL mNullTextOutputProtocol;

#endif //NULL_TEXT_OUTPUT_PROTOCOL_H_

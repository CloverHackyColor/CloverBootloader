/** @file
  Utility routines used by boot maintenance modules.

Copyright (c) 2004 - 2009, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>
//#include "IO.h"

/**

  Find the first instance of this Protocol
  in the system and return it's interface.


  @param ProtocolGuid    Provides the protocol to search for
  @param Interface       On return, a pointer to the first interface
                         that matches ProtocolGuid

  @retval  EFI_SUCCESS      A protocol instance matching ProtocolGuid was found
  @retval  EFI_NOT_FOUND    No protocol instances were found that match ProtocolGuid

**/
EFI_STATUS
EfiLibLocateProtocol (
  IN  EFI_GUID    *ProtocolGuid,
  OUT void        **Interface
  )
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (
                  ProtocolGuid,
                  NULL,
                  (void **) Interface
                  );
  return Status;
}

/**

  Function opens and returns a file handle to the root directory of a volume.

  @param DeviceHandle    A handle for a device

  @return A valid file handle or NULL is returned

**/
EFI_FILE*
EfiLibOpenRoot (
  IN EFI_HANDLE                   DeviceHandle
  )
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
  EFI_FILE*                 File;

  File = NULL;

  //
  // File the file system interface to the device
  //
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (void **) &Volume
                  );

  //
  // Open the root directory of the volume
  //
  if (!EFI_ERROR(Status)) {
    Status = Volume->OpenVolume (
                      Volume,
                      &File
                      );
  }
  //
  // Done
  //
  return EFI_ERROR(Status) ? NULL : File;
}

/**

  Function gets the file system information from an open file descriptor.


  @param FHand           The file handle.

  @return                A pointer to a buffer with file information.
  @retval                NULL is returned if failed to get Volume Label Info.

**/
XStringW
EfiLibFileSystemVolumeLabelInfo (
  const EFI_FILE*      FHand
  )
{
  EFI_STATUS    Status;
  EFI_FILE_SYSTEM_VOLUME_LABEL *VolumeInfo = NULL;
  UINTN         Size = 0;
  XStringW returnValue;
  
  Status = FHand->GetInfo(FHand, &gEfiFileSystemVolumeLabelInfoIdGuid, &Size, VolumeInfo);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    // inc size by 2 because some drivers (HFSPlus.efi) do not count 0 at the end of file name
    Size += 2;
    VolumeInfo = (__typeof__(VolumeInfo))AllocateZeroPool(Size);
    Status = FHand->GetInfo (FHand, &gEfiFileSystemVolumeLabelInfoIdGuid, &Size, VolumeInfo);
    // Check to make sure this isn't actually EFI_FILE_SYSTEM_INFO
    if (!EFI_ERROR(Status))
    {
       EFI_FILE_SYSTEM_INFO *FSInfo = (EFI_FILE_SYSTEM_INFO *)VolumeInfo;
       returnValue.takeValueFrom(FSInfo->VolumeLabel);
    }
    FreePool(VolumeInfo);
  }
  return returnValue;
}


//Compare strings case insensitive
// return 0 if strings are equal not accounting match case
INTN
StriCmp (
		IN      CONST CHAR16              *FirstS,
		IN      CONST CHAR16              *SecondS
		)
{
	
	while (*FirstS != L'\0') {
		if ( (((*FirstS >= 'a') && (*FirstS <= 'z')) ? (*FirstS - ('a' - 'A')) : *FirstS ) !=
            (((*SecondS >= 'a') && (*SecondS <= 'z')) ? (*SecondS - ('a' - 'A')) : *SecondS ) ) return 1;
		FirstS++;
		SecondS++;
	}
	return *SecondS; //if all chars equal then compare sizes, last char in SecondS will be 0
}

// If Null-terminated strings are case insensitive equal or its sSize symbols are equal then TRUE
BOOLEAN
AsciiStriNCmp(
              IN      CONST CHAR8              *FirstS,
              IN      CONST CHAR8              *SecondS,
              IN      CONST UINTN               sSize
              )
{
    INTN i = sSize;
	while ( i && (*FirstS != '\0') ) {
		if ( (((*FirstS >= 'a') && (*FirstS <= 'z')) ? (*FirstS - ('a' - 'A')) : *FirstS ) !=
            (((*SecondS >= 'a') && (*SecondS <= 'z')) ? (*SecondS - ('a' - 'A')) : *SecondS ) ) return FALSE;
		FirstS++;
		SecondS++;
        i--;
	}
	return TRUE;
}

// Case insensitive search of WhatString in WhereString
BOOLEAN
AsciiStrStriN (
               IN      CONST CHAR8              *WhatString,
               IN      CONST UINTN               sWhatSize,
               IN      CONST CHAR8              *WhereString,
               IN      CONST UINTN               sWhereSize
              )
{
  INTN i = sWhereSize;
  BOOLEAN Found = FALSE;
  if (sWhatSize > sWhereSize) return FALSE;
  for (; i && !Found; i--) {
    Found = AsciiStriNCmp(WhatString, WhereString, sWhatSize);
    WhereString++;
  }
	return Found;
}

/**

  Function gets the file information from an open file descriptor, and stores it
  in a buffer allocated from pool.

  @param FHand           File Handle.

  @return                A pointer to a buffer with file information or NULL is returned

**/
EFI_FILE_INFO *
EfiLibFileInfo (
  const EFI_FILE*      FHand
  )
{
  EFI_STATUS    Status;
  EFI_FILE_INFO *FileInfo = NULL;
  UINTN         Size = 0;
  
  Status = FHand->GetInfo (FHand, &gEfiFileInfoGuid, &Size, FileInfo);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    // inc size by 2 because some drivers (HFSPlus.efi) do not count 0 at the end of file name
    Size += 2;
    FileInfo = (__typeof__(FileInfo))AllocateZeroPool(Size);
    Status = FHand->GetInfo (FHand, &gEfiFileInfoGuid, &Size, FileInfo);
  }
  
  return EFI_ERROR(Status)?NULL:FileInfo;
}

EFI_FILE_SYSTEM_INFO *
EfiLibFileSystemInfo (
                const EFI_FILE*      FHand
                )
{
  EFI_STATUS    Status;
  EFI_FILE_SYSTEM_INFO *FileSystemInfo = NULL;
  UINTN         Size = 0;
  
  Status = FHand->GetInfo (FHand, &gEfiFileSystemInfoGuid, &Size, FileSystemInfo);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    // inc size by 2 because some drivers (HFSPlus.efi) do not count 0 at the end of file name
    Size += 2;
    FileSystemInfo = (__typeof__(FileSystemInfo))AllocateZeroPool(Size);
    Status = FHand->GetInfo (FHand, &gEfiFileSystemInfoGuid, &Size, FileSystemInfo);
  }
  
  return EFI_ERROR(Status)?NULL:FileSystemInfo;
}

/**
  Function is used to determine the number of device path instances
  that exist in a device path.


  @param DevicePath      A pointer to a device path data structure.

  @return This function counts and returns the number of device path instances
          in DevicePath.

**/
UINTN
EfiDevicePathInstanceCount (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
{
  UINTN Count;
  UINTN Size;

  Count = 0;
  while (GetNextDevicePathInstance (&DevicePath, &Size) != NULL) {
    Count += 1;
  }

  return Count;
}

/**
  Adjusts the size of a previously allocated buffer.


  @param OldPool         - A pointer to the buffer whose size is being adjusted.
  @param OldSize         - The size of the current buffer.
  @param NewSize         - The size of the new buffer.

  @return   The newly allocated buffer.
  @retval   NULL  Allocation failed.

**/
void *
EfiReallocatePool (
  IN void                 *OldPool,
  IN UINTN                OldSize,
  IN UINTN                NewSize
  )
{
  void  *NewPool;

  NewPool = NULL;
  if (NewSize != 0) {
    NewPool = (__typeof__(NewPool))AllocateZeroPool(NewSize);
  }

  if (OldPool != NULL) {
    if (NewPool != NULL) {
      CopyMem(NewPool, OldPool, OldSize < NewSize ? OldSize : NewSize);
    }

    FreePool(OldPool);
  }

  return NewPool;
}

/**
  Compare two EFI_TIME data.


  @param FirstTime       - A pointer to the first EFI_TIME data.
  @param SecondTime      - A pointer to the second EFI_TIME data.

  @retval  TRUE              The FirstTime is not later than the SecondTime.
  @retval  FALSE             The FirstTime is later than the SecondTime.

**/
BOOLEAN
TimeCompare (
  IN EFI_TIME               *FirstTime,
  IN EFI_TIME               *SecondTime
  )
{
  if (FirstTime->Year != SecondTime->Year) {
    return (BOOLEAN) (FirstTime->Year < SecondTime->Year);
  } else if (FirstTime->Month != SecondTime->Month) {
    return (BOOLEAN) (FirstTime->Month < SecondTime->Month);
  } else if (FirstTime->Day != SecondTime->Day) {
    return (BOOLEAN) (FirstTime->Day < SecondTime->Day);
  } else if (FirstTime->Hour != SecondTime->Hour) {
    return (BOOLEAN) (FirstTime->Hour < SecondTime->Hour);
  } else if (FirstTime->Minute != SecondTime->Minute) {
    return (BOOLEAN) (FirstTime->Minute < SecondTime->Minute);
  } else if (FirstTime->Second != SecondTime->Second) {
    return (BOOLEAN) (FirstTime->Second < SecondTime->Second);
  }

  return (BOOLEAN) (FirstTime->Nanosecond <= SecondTime->Nanosecond);
}

/*
 Translate VT-UTF8 characters into one Unicode character.

 UTF8 Encoding Table
 Bits per Character | Unicode Character Range | Unicode Binary  Encoding |  UTF8 Binary Encoding
 0-7                |     0x0000 - 0x007F     |     00000000 0xxxxxxx    |   0xxxxxxx
 8-11               |     0x0080 - 0x07FF     |     00000xxx xxxxxxxx    |   110xxxxx 10xxxxxx
 12-16              |     0x0800 - 0xFFFF     |     xxxxxxxx xxxxxxxx    |   1110xxxx 10xxxxxx 10xxxxxx

 $  U+0024    10 0100             00100100                    24
 ¢  U+00A2  1010 0010             11000010 10100010           C2 A2
 €  U+20AC  0010 0000 1010 1100   11100010 10000010 10101100  E2 82 AC
 𐍈  U+10348 1 0000 0011 0100 1000  11110000 10010000 10001101 10001000  F0 90 8D 88
 */

CHAR8* GetUnicodeChar(CHAR8 *s, CHAR16* UnicodeChar)
{
  INT8 ValidBytes = 0;
  UINT8 Byte0, Byte1, Byte2;
  UINT8 UnicodeByte0, UnicodeByte1;
  CHAR16 A = L'\0';
  if (*s == '&') {
    if (AsciiStrStr(s, "&#x") !=0 ) {
      s += 3;
      while (IS_HEX(*s) || IS_DIGIT(*s)) {
        A <<= 4;
        if (IS_DIGIT(*s)) {
          A += *s - 0x30;
        } else if (IS_UPPER(*s)) {
          A += *s - 0x41 + 10;
        } else {
          A += *s - 0x61 + 10;
        }
        s++;
      }
    } else if (AsciiStrStr(s, "&amp;") !=0 ) {
      A = 0x26; //&
      s += 5;
    } else if (AsciiStrStr(s, "&quot;") !=0 ) {
      A = 0x22; //"
      s += 6;
    } else if (AsciiStrStr(s, "&lt;") !=0 ) {
      A = 0x3C; //<
      s += 4;
    } else if (AsciiStrStr(s, "&gt;") !=0 ) {
      A = 0x3E; //>
      s += 4;
    } else if (AsciiStrStr(s, "&nbsp;") !=0 ) {
      A = 0xA0; //>
      s += 6;
    }
    *UnicodeChar = A;
  } else {
    if ((*s & 0x80) == 0) {
      ValidBytes = 1;
    } else if ((*s & 0xe0) == 0xc0) {
      ValidBytes = 2;
    } else if ((*s & 0xf0) == 0xe0) {
      ValidBytes = 3;
    }
    switch (ValidBytes) {
      case 1:
        //
        // one-byte utf8 code
        //
        *UnicodeChar = (UINT16) (*s++);
        break;

      case 2:
        //
        // two-byte utf8 code
        //
        Byte1         = *s++;  //c2
        Byte0         = *s++;  //a2

        UnicodeByte0  = (UINT8) ((Byte1 << 6) | (Byte0 & 0x3f));
        UnicodeByte1  = (UINT8) ((Byte1 >> 2) & 0x07);
        *UnicodeChar  = (UINT16) (UnicodeByte0 | (UnicodeByte1 << 8));
        break;

      case 3:
        //
        // three-byte utf8 code
        // sample E3 90 A1 = 0x3421
        //
        Byte2         = *s++;
        Byte1         = *s++;
        Byte0         = *s++;

        UnicodeByte0  = (UINT8) ((Byte1 << 6) | (Byte0 & 0x3f));
        UnicodeByte1  = (UINT8) ((Byte2 << 4) | ((Byte1 >> 2) & 0x0f));
        *UnicodeChar  = (UINT16) (UnicodeByte0 | (UnicodeByte1 << 8));

      default:
        break;
    }
  }
  return s;
}


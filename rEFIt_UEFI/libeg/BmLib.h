/*
 * BmLib.h
 *
 *  Created on: 10 Apr 2020
 *      Author: jief
 */

#ifndef LIBEG_BMLIB_H_
#define LIBEG_BMLIB_H_

#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>

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
  OUT VOID        **Interface
  );

/**

  Function opens and returns a file handle to the root directory of a volume.

  @param DeviceHandle    A handle for a device

  @return A valid file handle or NULL is returned

**/
EFI_FILE_HANDLE
EfiLibOpenRoot (
  IN EFI_HANDLE                   DeviceHandle
  );

/**

  Function gets the file system information from an open file descriptor,
  and stores it in a buffer allocated from pool.


  @param FHand           The file handle.

  @return                A pointer to a buffer with file information.
  @retval                NULL is returned if failed to get Volume Label Info.

**/
EFI_FILE_SYSTEM_VOLUME_LABEL *
EfiLibFileSystemVolumeLabelInfo (
  IN EFI_FILE_HANDLE      FHand
  );

/**
  Duplicate a string.

  @param Src             The source.

  @return A new string which is duplicated copy of the source.
  @retval NULL If there is not enough memory.

**/
CHAR16 *
EfiStrDuplicate (
  IN CONST CHAR16   *Src
				 );

//Compare strings case insensitive
// return 0 if strings are equal not accounting match case
INTN
StriCmp (
		IN      CONST CHAR16              *FirstS,
		IN      CONST CHAR16              *SecondS
		);

// If Null-terminated strings are case insensitive equal or its sSize symbols are equal then TRUE
BOOLEAN
AsciiStriNCmp (
              IN      CONST CHAR8              *FirstS,
              IN      CONST CHAR8              *SecondS,
              IN      CONST UINTN               sSize
              );

// Case insensitive search of WhatString in WhereString
BOOLEAN
AsciiStrStriN (
               IN      CONST CHAR8              *WhatString,
               IN      CONST UINTN               sWhatSize,
               IN      CONST CHAR8              *WhereString,
               IN      CONST UINTN               sWhereSize
              );

/**

  Function gets the file information from an open file descriptor, and stores it
  in a buffer allocated from pool.

  @param FHand           File Handle.

  @return                A pointer to a buffer with file information or NULL is returned

**/
EFI_FILE_INFO *
EfiLibFileInfo (
  IN EFI_FILE_HANDLE      FHand
  );

EFI_FILE_SYSTEM_INFO *
EfiLibFileSystemInfo (
                IN EFI_FILE_HANDLE      FHand
                );

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
  );


/**
  Adjusts the size of a previously allocated buffer.


  @param OldPool         - A pointer to the buffer whose size is being adjusted.
  @param OldSize         - The size of the current buffer.
  @param NewSize         - The size of the new buffer.

  @return   The newly allocated buffer.
  @retval   NULL  Allocation failed.

**/
VOID *
EfiReallocatePool (
  IN VOID                 *OldPool,
  IN UINTN                OldSize,
  IN UINTN                NewSize
  );

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
  );

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

CHAR8* GetUnicodeChar(CHAR8 *s, CHAR16* UnicodeChar);




#endif /* LIBEG_BMLIB_H_ */

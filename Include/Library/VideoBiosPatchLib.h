/** @file
    Functions for video bios patches.
**/

#ifndef __VIDEO_BIOS_PATCH_LIB_H__
#define __VIDEO_BIOS_PATCH_LIB_H__



/**
  Unlocks video bios area for writing.
 
  @retval EFI_SUCCESS   If area is unlocked.
  @retval other         In case of error.
 
**/
EFI_STATUS
EFIAPI
VideoBiosUnlock (
  VOID
  );


/**
  Locks video bios area for writing.
 
  @retval EFI_SUCCESS   If area is locked.
  @retval other         In case of error.
 
**/
EFI_STATUS
EFIAPI
VideoBiosLock (
  VOID
  );


/**
  Searches video bios for SearchBytes (Size bytes) and replaces them with ReplaceBytes (Size bytes).
 
  @param  SearchBytes   Bytes to search for.
  @param  ReplaceBytes  Bytes that should replace SearchBytes.
  @param  Size          Number of SearchBytes and ReplaceBytes.
 
  @retval EFI_SUCCESS   If no error occured.
  @retval other         In case of error.
 
**/
EFI_STATUS
EFIAPI
VideoBiosPatchBytes (
  IN  UINT8         *SearchBytes,
  IN  UINT8         *ReplaceBytes,
  IN  UINTN         Size
  );


/**
  Determines "native" resolution from Edid detail timing descriptor
  and patches first video mode with that timing/resolution info.
 
  @param  Edid          Edid to use. If NULL, then Edid will be read from EFI_EDID_ACTIVE_PROTOCOL
 
  @retval EFI_SUCCESS   If no error occured.
  @retval other         In case of error.
 
**/
EFI_STATUS
EFIAPI
VideoBiosPatchNativeFromEdid (
  IN  UINT8         *Edid  OPTIONAL
  );


#endif // __VIDEO_BIOS_PATCH_LIB_H__

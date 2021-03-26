/** @file
    Functions for video bios patches.
**/

#ifndef __VIDEO_BIOS_PATCH_LIB_H__
#define __VIDEO_BIOS_PATCH_LIB_H__



/**
  Set of Search & replace bytes for VideoBiosPatchBytes().
**/
typedef struct _VBIOS_PATCH_BYTES {
  VOID    *Find;
  VOID    *Replace;
  UINTN   NumberOfBytes;
} VBIOS_PATCH_BYTES;


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
  Performs mutltiple Find&Replace operations on the video bios memory.
 
  @param  FindAndReplace      Pointer to array of VBIOS_PATCH_BYTES.
  @param  FindAndReplaceCount Number of VBIOS_PATCH_BYTES elements in a FindAndReplace array.
 
  @retval EFI_SUCCESS   If no error occured.
  @retval other         In case of error.
 
**/
EFI_STATUS
EFIAPI
VideoBiosPatchBytes (
  IN  JCONST VBIOS_PATCH_BYTES   *FindAndReplace,
  IN  UINTN               FindAndReplaceCount
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

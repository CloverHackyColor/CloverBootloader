/** @file
  Default instance of VideoBiosPatchLib library for video bios patches.
 
  Ported from Chameleon's Resolution module (created by Evan Lojewski)
  which is a version of 915resolution (created by steve tomljenovic).

  Ported to UEFI by usr-sse2, tweaked and added as VideoBiosPatchLib by dmazar.

**/

#include "VideoBiosPatchLibInternal.h"


//
// Internal pointers to LegacyRegion protocols
//
EFI_LEGACY_REGION_PROTOCOL    *mLegacyRegion = NULL;
EFI_LEGACY_REGION2_PROTOCOL   *mLegacyRegion2 = NULL;

//
// Temp var for passing Edid to readEDID() in edid.c
//
UINT8     *mEdid = NULL;


/**
  Searches Source for Search pattern of size SearchSize
  and replaces it with Replace up to MaxReplaces times.
 
  @param  Source      Source bytes that will be searched.
  @param  SourceSize  Number of bytes in Source region.
  @param  Search      Bytes to search for.
  @param  SearchSize  Number of bytes in Search.
  @param  Replace     Bytes that will replace found bytes in Source (size is SearchSize).
  @param  MaxReplaces Maximum number of replaces. If MaxReplaces <= 0, then there is no restriction.

  @retval Number of replaces done.

 **/
UINTN
VideoBiosPatchSearchAndReplace (
  IN  UINT8       *Source,
  IN  UINTN       SourceSize,
  IN  UINT8       *Search,
  IN  UINTN       SearchSize,
  IN  UINT8       *Replace,
  IN  INTN        MaxReplaces
  )
{
  UINTN     NumReplaces = 0;
  BOOLEAN   NoReplacesRestriction = MaxReplaces <= 0;
  UINT8     *End = Source + SourceSize;
  
  while (Source < End && (NoReplacesRestriction || MaxReplaces > 0)) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
      CopyMem(Source, Replace, SearchSize);
      NumReplaces++;
      MaxReplaces--;
      Source += SearchSize;
    } else {
      Source++;
    }
  }
  return NumReplaces;
}


/**
  Inits mLegacyRegion or mLegacyRegion2 protocols.
 
**/
EFI_STATUS
VideoBiosPatchInit (
  VOID
  )
{
  EFI_STATUS        Status;
  
  //
  // Return if we are already inited
  //
  if (mLegacyRegion != NULL || mLegacyRegion2 != NULL) {
    return EFI_SUCCESS;
  }
  
  DBG(" VideoBiosPatchInit(");
  //
  // Check for EfiLegacyRegionProtocol and/or EfiLegacyRegion2Protocol
  //
  Status = gBS->LocateProtocol (&gEfiLegacyRegionProtocolGuid, NULL, (VOID **) &mLegacyRegion);
  DBG("LegacyRegion = %r", Status);
  if (EFI_ERROR(Status)) {
    mLegacyRegion = NULL;
    Status = gBS->LocateProtocol (&gEfiLegacyRegion2ProtocolGuid, NULL, (VOID **) &mLegacyRegion2);
    DBG(", LegacyRegion2 = %r", Status);
    if (EFI_ERROR(Status)) {
      mLegacyRegion2 = NULL;
    }
  }
  DBG(") = %r\n", Status);
  
  return Status;
}


/**
  Unlocks video bios area for writing.
 
  @retval EFI_SUCCESS   If area is unlocked.
  @retval other         In case of error.
 
**/
EFI_STATUS
EFIAPI
VideoBiosUnlock (
  VOID
  )
{
  EFI_STATUS        Status;
  UINT32            Granularity;
  UINT8             *TstPtr;
  UINT8             TstVar;
  
  Status = VideoBiosPatchInit ();
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = EFI_NOT_FOUND;
  
  DBG(" VideoBiosUnlock: ");
  if (mLegacyRegion != NULL) {
    Status = mLegacyRegion->UnLock (mLegacyRegion, VBIOS_START, VBIOS_SIZE, &Granularity);
  } else if (mLegacyRegion2 != NULL) {
    Status = mLegacyRegion2->UnLock (mLegacyRegion2, VBIOS_START, VBIOS_SIZE, &Granularity);
  }
  //DBG("%r\n", Status);
  
  //
  // Test some vbios address for writing
  //
  TstPtr = (UINT8*)(UINTN)(VBIOS_START + 0xA110);
  TstVar = *TstPtr;
  *TstPtr = *TstPtr + 1;
  if (TstVar == *TstPtr) {
    DBG(" Test unlock: Error, not unlocked!\n");
    Status = EFI_DEVICE_ERROR;
  } else {
    DBG(" unlocked\n");
  }
  *TstPtr = TstVar;
  
  return Status;
}


/**
  Locks video bios area for writing.
 
  @retval EFI_SUCCESS   If area is locked.
  @retval other         In case of error.
 
**/
EFI_STATUS
EFIAPI
VideoBiosLock (
  VOID
  )
{
  EFI_STATUS        Status;
  UINT32            Granularity;
  
  Status = VideoBiosPatchInit ();
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = EFI_NOT_FOUND;
  
  DBG(" VideoBiosLock: ");
  if (mLegacyRegion != NULL) {
    Status = mLegacyRegion->Lock (mLegacyRegion, VBIOS_START, VBIOS_SIZE, &Granularity);
  } else if (mLegacyRegion2 != NULL) {
    Status = mLegacyRegion2->Lock (mLegacyRegion2, VBIOS_START, VBIOS_SIZE, &Granularity);
  }
  DBG("%r\n", Status);
  
  return Status;
}


/**
  Performs mutltiple Search&Replace operations on the video bios memory.

  @param  FindAndReplace      Pointer to array of VBIOS_PATCH_BYTES.
  @param  FindAndReplaceCount Number of VBIOS_PATCH_BYTES elements in a FindAndReplace array.

  @retval EFI_SUCCESS   If no error occured.
  @retval other         In case of error.

 **/
EFI_STATUS
EFIAPI
VideoBiosPatchBytes (
  IN  VBIOS_PATCH_BYTES   *FindAndReplace,
  IN  UINTN               FindAndReplaceCount
  )
{
  EFI_STATUS        Status;
  UINTN             Index;
  UINTN             NumReplaces;
  UINTN             NumReplacesTotal;
  
  if (FindAndReplace == NULL || FindAndReplaceCount == 0) {
    return EFI_INVALID_PARAMETER;
  }
  
  DBG("VideoBiosPatchBytes(%d patches):\n", FindAndReplaceCount);
  Status = VideoBiosUnlock ();
  if (EFI_ERROR(Status)) {
    DBG(" = not done.\n");
    return Status;
  }
  
  NumReplaces = 0;
  NumReplacesTotal = 0;
  for (Index = 0; Index < FindAndReplaceCount; Index++) {
    NumReplaces = VideoBiosPatchSearchAndReplace (
                                                  (UINT8*)(UINTN)VBIOS_START,
                                                  VBIOS_SIZE,
                                                  FindAndReplace[Index].Find,
                                                  FindAndReplace[Index].NumberOfBytes,
                                                  FindAndReplace[Index].Replace,
                                                  -1
                                                  );
    NumReplacesTotal += NumReplaces;
    DBG(" patch %d: patched %d time(s)\n", Index, NumReplaces);
  }
  
  DBG(" patched %d time(s)\n", NumReplacesTotal);
  
  VideoBiosLock ();
  
  return EFI_SUCCESS;
}


/**
  Reads and returns Edid from EFI_EDID_ACTIVE_PROTOCOL.
 
  @retval Edid          If Edid found.
  @retval NULL          If Edid not found.
 
**/
UINT8* VideoBiosPatchGetEdid (VOID)
{
  EFI_STATUS                      Status;
  EFI_EDID_ACTIVE_PROTOCOL        *EdidProtocol;
  UINT8                           *Edid;
  
  DBG(" Edid:");
  Edid = NULL;
  Status = gBS->LocateProtocol (&gEfiEdidActiveProtocolGuid, NULL, (VOID**)&EdidProtocol);
  if (!EFI_ERROR(Status)) {
    DBG(" size=%d", EdidProtocol->SizeOfEdid);
    if (EdidProtocol->SizeOfEdid > 0) {
      Edid = AllocateCopyPool(EdidProtocol->SizeOfEdid, EdidProtocol->Edid);
    }
  }
  DBG(" %a\n", Edid != NULL ? "found" : "not found");

  return Edid;
}


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
  )
{
  EFI_STATUS          Status;
  BOOLEAN             ReleaseEdid;
  vbios_map           *map;
  
  DBG("VideoBiosPatchNativeFromEdid:\n");
  
  ReleaseEdid = FALSE;
  if (Edid == NULL) {
    Edid = VideoBiosPatchGetEdid ();
    if (Edid == NULL) {
      return EFI_UNSUPPORTED;
    }
    ReleaseEdid = TRUE;
  }
  
  map = open_vbios(CT_UNKNOWN);
  if (map == NULL) {
    DBG(" = unknown video bios.\n");
    if (ReleaseEdid) {
      FreePool(Edid);
    }
    return EFI_UNSUPPORTED;
  }
  
  Status = VideoBiosUnlock ();
  if (EFI_ERROR(Status)) {
    DBG(" = not done.\n");
    return Status;
  }
  
  mEdid = Edid;
  set_mode (map, 0, 0, 0, 0, 0);
  mEdid = NULL;
  if (ReleaseEdid) {
    FreePool(Edid);
  }
  
  VideoBiosLock ();
  
  close_vbios (map);
  
  return EFI_SUCCESS;
  
}


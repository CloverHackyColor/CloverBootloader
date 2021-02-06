//Slice 2013

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "Settings.h"
#include "AcpiPatcher.h"

#ifndef DEBUG_ALL
#define DEBUG_EDID 1
#else
#define DEBUG_EDID DEBUG_ALL
#endif

#if DEBUG_EDID == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_EDID, __VA_ARGS__)
#endif

EFI_STATUS EFIAPI GetEdidImpl(
                              IN  EFI_EDID_OVERRIDE_PROTOCOL          *This,
                              IN  EFI_HANDLE                          *ChildHandle,
                              OUT UINT32                              *Attributes,
                              IN OUT UINTN                            *EdidSize,
                              IN OUT UINT8                            **Edid
                              )
{
  *Edid = gSettings.CustomEDID;
  *EdidSize = 128;
  *Attributes = 0;
  if (*Edid) {
    return EFI_SUCCESS;
  }
  return EFI_NOT_FOUND;
}

EFI_EDID_OVERRIDE_PROTOCOL gEdidOverride =
{
  GetEdidImpl
};

EFI_STATUS
InitializeEdidOverride ()
{
  EFI_STATUS              Status;
  EFI_EDID_OVERRIDE_PROTOCOL *EdidOverride;

  EdidOverride = (__typeof__(EdidOverride))AllocateCopyPool(sizeof(EFI_EDID_OVERRIDE_PROTOCOL), &gEdidOverride);
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                                                   &gImageHandle,
                                                   &gEfiEdidOverrideProtocolGuid,
                                                   EdidOverride,
                                                   NULL
                                                   );
  if (EFI_ERROR(Status)) {
    DBG("Can't install EdidOverride on ImageHandle\n");
  }
  return Status;
}

//used only if VBiosPatchNeeded and if no CustomEDID
UINT8* getCurrentEdid (void)
{
  EFI_STATUS                      Status;
  EFI_EDID_ACTIVE_PROTOCOL        *EdidProtocol;
  UINT8                           *Edid;
  
  DBG("EdidActive:");
  Edid = NULL;
  Status = gBS->LocateProtocol (&gEfiEdidActiveProtocolGuid, NULL, (void**)&EdidProtocol);
  if (!EFI_ERROR(Status)) {
    DBG(" size=%d", EdidProtocol->SizeOfEdid);
    if (EdidProtocol->SizeOfEdid > 0) {
      Edid = (__typeof__(Edid))AllocateCopyPool(EdidProtocol->SizeOfEdid, EdidProtocol->Edid);
    }
  }
  DBG(" %s\n", Edid != NULL ? "found" : "not found");
  
  return Edid;
}

void DebugDumpEDID(CONST CHAR8 *Message, INTN N)
{
  INTN i,j;
  // Don't dump in the case of debug logging because of too slow output
  if (gSettings.Boot.DebugLog) {
    return;
  }
	DBG("%s size:%lld\n", Message, N);
  for (i=0; i<N; i+=10) {
	  DBG("%03lld  |", i);
    for (j=0; j<10; j++) {
      if (i+j > N-1) break;
      DBG("  %02hhX", gSettings.CustomEDID[i+j]);
    }
    DBG("\n");
  }
}

//Used at OS start
// if EFI_SUCCESS then result in gSettings.CustomEDID != NULL
// first priority is CustomEDID
// second is UEFI EDID from EdidDiscoveredProtocol
EFI_STATUS GetEdidDiscovered(void)
{
  EFI_STATUS  Status = EFI_SUCCESS;
  UINTN       N = 0;
  UINT8       NewChecksum;
  //gEDID       = NULL;

  if (gSettings.CustomEDID) {
    N = gSettings.CustomEDIDsize;
    DebugDumpEDID("--- Custom EDID Table", N);
  } else {
    Status = gBS->LocateProtocol (&gEfiEdidDiscoveredProtocolGuid, NULL, (void **)&EdidDiscovered);
    if (!EFI_ERROR(Status)) { //discovered
      N = EdidDiscovered->SizeOfEdid;
      if (!gSettings.Boot.DebugLog) {
		  DBG("EdidDiscovered size=%llu\n", N);
      }
      if (N == 0) {
        return EFI_NOT_FOUND;
      }
      gSettings.CustomEDID = (__typeof__(gSettings.CustomEDID))AllocateAlignedPages(EFI_SIZE_TO_PAGES(N), 128);
      CopyMem(gSettings.CustomEDID, EdidDiscovered->Edid, N);
      DebugDumpEDID("--- Discovered EDID Table", N);
    }
  }

  if (gSettings.CustomEDID) {
    // begin patching result
    if (gSettings.VendorEDID) {
		DBG("    VendorID = 0x%04hx changed to 0x%04hx\n", ((UINT16*)gSettings.CustomEDID)[4], gSettings.VendorEDID);
      ((UINT16*)gSettings.CustomEDID)[4] = gSettings.VendorEDID;
    }

    if (gSettings.ProductEDID) {
		DBG("    ProductID = 0x%04hx changed to 0x%04hx\n", ((UINT16*)gSettings.CustomEDID)[5], gSettings.ProductEDID);
      ((UINT16*)gSettings.CustomEDID)[5] = gSettings.ProductEDID;
    }

    if (gSettings.EdidFixHorizontalSyncPulseWidth) {
		DBG("    HorizontalSyncPulseWidth = 0x%02hhx changed to 0x%02hx\n", ((UINT8*)gSettings.CustomEDID)[63], gSettings.EdidFixHorizontalSyncPulseWidth);
      UINT8 LsBits, MsBits;
      LsBits = gSettings.EdidFixHorizontalSyncPulseWidth & 0xff;
      MsBits = (gSettings.EdidFixHorizontalSyncPulseWidth >> 8) & 0x03;
      ((UINT8*)gSettings.CustomEDID)[63] = LsBits;
      LsBits = ((UINT8*)gSettings.CustomEDID)[65] & ~0x30;
      ((UINT8*)gSettings.CustomEDID)[65] = LsBits | (MsBits << 4);
    }

    if (gSettings.EdidFixVideoInputSignal) {
		DBG("    VideoInputSignal = 0x%02hhx changed to 0x%02hhx\n", ((UINT8*)gSettings.CustomEDID)[20], gSettings.EdidFixVideoInputSignal);
      ((UINT8*)gSettings.CustomEDID)[20] = gSettings.EdidFixVideoInputSignal;
    }

    NewChecksum = (UINT8)(256 - Checksum8(gSettings.CustomEDID, 127));
    if ((gSettings.VendorEDID) || (gSettings.ProductEDID) || (gSettings.EdidFixHorizontalSyncPulseWidth) || (gSettings.EdidFixVideoInputSignal)) {
      ((UINT8*)gSettings.CustomEDID)[127] = NewChecksum;
      DebugDumpEDID("--- Patched EDID Table", N);
    } else if (((UINT8*)gSettings.CustomEDID)[127] != NewChecksum) {
		DBG("    Fix wrong checksum = 0x%02hhx changed to ", ((UINT8*)gSettings.CustomEDID)[127]);
      ((UINT8*)gSettings.CustomEDID)[127] = NewChecksum;
		DBG("0x%02hhx\n", ((UINT8*)gSettings.CustomEDID)[127]);
      DebugDumpEDID("--- Patched EDID Table", N);
    }
  }
  return Status;
}

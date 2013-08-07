//Slice 2013

#include "Platform.h"

#ifndef DEBUG_ALL
#define DEBUG_EDID 1
#else
#define DEBUG_EDID DEBUG_ALL
#endif

#if DEBUG_SET == 0
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

  EdidOverride = AllocateCopyPool(sizeof(EFI_EDID_OVERRIDE_PROTOCOL), &gEdidOverride);
  

  Status = gBS->InstallMultipleProtocolInterfaces (
                                                   &gImageHandle,
                                                   &gEfiEdidOverrideProtocolGuid,
                                                   EdidOverride,
                                                   NULL
                                                   );
  if (EFI_ERROR (Status)) {
    DBG("Can't install EdidOverride on ImageHandle\n");
  }
  return Status;
}

UINT8* getCurrentEdid (VOID)
{
  EFI_STATUS                      Status;
  EFI_EDID_ACTIVE_PROTOCOL        *EdidProtocol;
  UINT8                           *Edid;
  
  DBG ("Edid:");
  Edid = NULL;
  Status = gBS->LocateProtocol (&gEfiEdidActiveProtocolGuid, NULL, (VOID**)&EdidProtocol);
  if (!EFI_ERROR (Status)) {
    DBG(" size=%d", EdidProtocol->SizeOfEdid);
    if (EdidProtocol->SizeOfEdid > 0) {
      Edid = AllocateCopyPool (EdidProtocol->SizeOfEdid, EdidProtocol->Edid);
    }
  }
  DBG(" %a\n", Edid != NULL ? "found" : "not found");
  
  return Edid;
}

EFI_STATUS GetEdidDiscovered(VOID)
{
	EFI_STATUS						Status;
	UINTN i, j;
  UINTN N;
  gEDID = NULL;

	Status = gBS->LocateProtocol (&gEfiEdidDiscoveredProtocolGuid, NULL, (VOID **)&EdidDiscovered);

	if (!EFI_ERROR (Status))
	{
		N = EdidDiscovered->SizeOfEdid;
    MsgLog("EdidDiscovered size=%d\n", N);
		if (N == 0) {
			return EFI_NOT_FOUND;
		}
    gEDID = AllocateAlignedPages(EFI_SIZE_TO_PAGES(N), 128);
    if (!gSettings.CustomEDID) {
      gSettings.CustomEDID = gEDID; //copy pointer but data if no CustomEDID
    }
    CopyMem(gEDID, EdidDiscovered->Edid, N);
    if (!GlobalConfig.DebugLog) {
      for (i=0; i<N; i+=10) {
        MsgLog("%02d | ", i);
        for (j=0; j<10; j++) {
          MsgLog("%02x ", EdidDiscovered->Edid[i+j]);
        }
        MsgLog("\n");
      }
    }
	}
  return Status;
}



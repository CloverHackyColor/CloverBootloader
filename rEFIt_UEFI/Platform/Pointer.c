//
//  Pointer.c
//  
//
//  Created by Slice on 23.09.12.
//
// Initial idea comes from iBoot project by OS_Ninja and Ujen
// their sources are under GNU License but I don't know what is the subject for licensing here.
// my sources are quite different while Mouse/Events interfaces comes from Tiano,
// for example ConSplitterDxe or BdsDxe/FrontPage
// anyway thanks for good tutorial how to do and how not to do
// 

#include "Platform.h"

typedef struct _pointers {
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointerProtocol;
  EG_IMAGE *Pointer;
  EG_IMAGE *Mask;
  EG_IMAGE *Shadow;

  EG_RECT  PlaceNow;
  EG_RECT  PlaceOld;
  
  EFI_TIME	LastClickTime;
  EFI_SIMPLE_POINTER_STATE    State;
} POINTERS;

static POINTERS gPointer;

EFI_STATUS MouseInit()
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  
  Status = gBS->LocateProtocol (&gEfiSimplePointerProtocolGuid,NULL, (VOID**)&gPointer.SimplePointerProtocol);
	if(EFI_ERROR(Status))
	{
		MsgLog("No mouse!\n");
    returm Status;
	}
  
  gPointer.Pointer = egLoadIcon(ThemeDir, L"pointer.icns", 32);
  gPointer.Mask = NULL;
  gPointer.Shadow = NULL;
  gPointer.PlaceNow.XPos = UGAWidth >> 2;
  gPointer.PlaceNow.YPos = UGAHeight >> 2;
  
  
  returm Status;
}

EFI_STATUS WaitForMouseRelease()
{
  EFI_STATUS Status = EFI_SUCCESS;
  
}

#if UNDERCONSTRUCTION

TimeoutRemain = TimeoutDefault;
while (TimeoutRemain != 0) {
  
  Status = WaitForSingleEvent (gST->ConIn->WaitForKey, ONE_SECOND);
  if (Status != EFI_TIMEOUT) {
    break;
  }
  TimeoutRemain--;
  
  if (!FeaturePcdGet(PcdBootlogoOnlyEnable)) {
    //
    // Show progress
    //
    if (TmpStr != NULL) {
      PlatformBdsShowProgress (
                               Foreground,
                               Background,
                               TmpStr,
                               Color,
                               ((TimeoutDefault - TimeoutRemain) * 100 / TimeoutDefault),
                               0
                               );
    }
  }
}

#endif
//
//  Pointer.c
//  
//
//  Created by Slice on 23.09.12.
//
// Initial idea comes from iBoot project by OS_Ninja and Ujen
// their sources are under GNU License but I don't know what is the subject for licensing here.
// my sources are quite different while Mouse interfaces comes from Tiano,
// for example ConSplitterDxe
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

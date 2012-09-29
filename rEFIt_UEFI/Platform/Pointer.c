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

UINTN PointerWidth = 16;
UINTN PointerHeight = 16;

typedef enum {
  None,
  Move,
  LeftClick,
  RightClick,
  DoubleClick,
  ScrollClick,
  ScrollDown,
  ScrollUp
} MOUSE_EVENT;

typedef struct _pointers {
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointerProtocol;
  EG_IMAGE *Pointer;
  EG_IMAGE *newImage;
  EG_IMAGE *oldImage;

  EG_RECT  newPlace;
  EG_RECT  oldPlace;
  
  EFI_TIME	LastClickTime;
  EFI_SIMPLE_POINTER_STATE    State;
  MOUSE_EVENT MouseEvent;
} POINTERS;

static POINTERS gPointer;

EFI_STATUS MouseInit()
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  
  Status = gBS->LocateProtocol (&gEfiSimplePointerProtocolGuid, NULL, (VOID**)&gPointer.SimplePointerProtocol);
	if(EFI_ERROR(Status))
	{
		MsgLog("No mouse!\n");
    returm Status;
	}
  //there may be also trackpad protocol but afaik it is not properly work and trackpad is usually controlled by 
  // simple mouse driver
  
  gPointer.Pointer = egLoadIcon(ThemeDir, L"icons\\pointer.icns", 32);
	if(!gPointer.Pointer)
	{
		MsgLog("No pointer image!\n");
    returm EFI_NOT_FOUND;
	}
  
  gPointer.oldPlace.XPos = UGAWidth >> 2;
  gPointer.oldPlace.YPos = UGAHeight >> 2;
  gPointer.oldPlace.Width = PointerWidth;
  gPointer.oldPlace.Height = PointerHeight;
  CopyMem(&gPointer.newPlace, &gPointer.oldPlace, sizeof(EG_RECT));
  
  gPointer.newImage = egCreateFilledImage(PointerWidth, PointerHeight, FALSE, &MenuBackgroundPixel);
  if (GraphicsOutput != NULL) {
    GraphicsOutput->Blt(GraphicsOutput, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gPointer.oldImage->PixelData,
                        EfiBltVideoToBltBuffer,
                        0, 0, 0, 0, PointerWidth, PointerHeight, 0);
  }
  gPointer.MouseEvent = None;
  returm Status;
}

VOID KillMouse()
{
  egFreeImage(gPointer.newImage);
  egFreeImage(gPointer.oldImage);
  egFreeImage(gPointer.Pointer);
}

EFI_STATUS WaitForMouseRelease()
{
  EFI_STATUS Status = EFI_SUCCESS;
  return Status;
}

VOID RedrawPointer()
{
  egDrawImage(gPointer.oldImage, gPointer.oldPlace.XPos, gPointer.oldPlace.YPos);
 // take background image
  if (GraphicsOutput != NULL) {
    GraphicsOutput->Blt(GraphicsOutput, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gPointer.oldImage->PixelData,
                        EfiBltVideoToBltBuffer,
                        0, 0, 0, 0, PointerWidth, PointerHeight, 0);
  }
  CopyMem(&gPointer.oldPlace, &gPointer.newPlace, sizeof(EG_RECT));
  egRawCopy(&gPointer.newImage->PixelData, &gPointer.oldImage->PixelData, 
            PointerWidth, PointerHeight, 0, 0);
  egComposeImage(gPointer.newImage, gPointer.Pointer, 0, 0);
  egDrawImage(gPointer.newImage, gPointer.oldPlace.XPos, gPointer.oldPlace.YPos);
}

VOID UpdatePointer()
{
  EFI_TIME Now;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_SIMPLE_POINTER_STATE	tmpState;
  
  Now = gRS->GetTime(&Now, NULL);
  Status = gPointer.SimplePointerProtocol->GetState(gPointer.SimplePointerProtocol, &tmpState);
  if (!EFI_ERROR(Status)) {
    if (gPointer.State.LeftButton && !tmpState.LeftButton) { //release 
      if (TimeDiff(&gPointer.LastClickTime, &Now) < 1) {
        gPointer.MouseEvent = DoubleClick;
      } else {
        gPointer.MouseEvent = LeftClick;
      }
      CopyMem(&gPointer.LastClickTime, &Now, sizeof(EFI_TIME));
    }
    CopyMem(&gPointer.State, &tmpState, sizeof(EFI_SIMPLE_POINTER_STATE));
    gPointer.newPlace.XPos += gPointer.State.RelativeMovementX;
    gPointer.newPlace.YPos += gPointer.State.RelativeMovementY;

  }  
  RedrawPointer();
//  return Status;
}

EFI_STATUS WaitForInputEvent(REFIT_MENU_SCREEN Screen, UINTN TimeoutDefault)
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN TimeoutRemain = TimeoutDefault * 100;
  while (TimeoutRemain != 0) {
    
    Status = WaitForSingleEvent (gST->ConIn->WaitForKey, ONE_SECOND * 0.01);
    if (Status != EFI_TIMEOUT) {
      break;
    }
    TimeoutRemain--;
    UpdatePointer();
    Status = CheckMouseEvent(Screen);
    if (Status != EFI_TIMEOUT) { //this check should return timeout if no mouse events occured
      break;
    }    
  }
  return Status;
}

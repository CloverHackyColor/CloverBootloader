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
// Any usage for SMBIOS here?
/// Built-in Pointing Device (Type 21).

//#include "Platform.h"
#include "libegint.h"   //this includes platform.h 

#ifndef DEBUG_ALL
#define DEBUG_MOUSE 1
#else
#define DEBUG_MOUSE DEBUG_ALL
#endif

#if DEBUG_MOUSE == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_MOUSE, __VA_ARGS__)
#endif

extern EFI_AUDIO_IO_PROTOCOL *AudioIo;

// make them theme dependent? No, 32 is good value for all.
#define POINTER_WIDTH  32
#define POINTER_HEIGHT 32

ACTION gAction;
UINTN  gItemID;

POINTERS gPointer = {NULL, NULL, NULL, NULL,
                     {0, 0, POINTER_WIDTH, POINTER_HEIGHT},
                     {0, 0, POINTER_WIDTH, POINTER_HEIGHT}, 0,
                     {0, 0, 0, FALSE, FALSE}, NoEvents};

VOID HidePointer()
{
  if (gPointer.SimplePointerProtocol) {
    egDrawImageArea(gPointer.oldImage, 0, 0, 0, 0, gPointer.oldPlace.XPos, gPointer.oldPlace.YPos);
  }  
}

VOID DrawPointer()
{
  // thanks lllevelll for the patch, I move it to egTakeImage and egDrawImageArea
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*  UINTN  PointerHCrop = POINTER_HEIGHT;
  UINTN  PointerWCrop = POINTER_WIDTH;  
  UINTN  Var = 0;
  
  PointerHCrop = POINTER_HEIGHT;
  PointerWCrop = POINTER_WIDTH;
  
  Var = UGAWidth - gPointer.newPlace.XPos;      
  
  If (Var < POINTER_WIDTH) {
    PointerWCrop = Var;
  }
  
  Var = UGAHeight - gPointer.newPlace.YPos;     
  
  If Var < (POINTER_HEIGHT) {
    PointerHCrop = Var;
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
*/  
  // take background image
  egTakeImage(gPointer.oldImage, gPointer.newPlace.XPos, gPointer.newPlace.YPos,
              POINTER_WIDTH, POINTER_HEIGHT);
  CopyMem(&gPointer.oldPlace, &gPointer.newPlace, sizeof(EG_RECT));
  CopyMem(gPointer.newImage->PixelData, gPointer.oldImage->PixelData, (UINTN)(POINTER_WIDTH * POINTER_HEIGHT * sizeof(EG_PIXEL))); // Should be faster
  egComposeImage(gPointer.newImage, gPointer.Pointer, 0, 0);
  egDrawImageArea(gPointer.newImage, 0, 0,
                  POINTER_WIDTH, POINTER_HEIGHT,
                  gPointer.oldPlace.XPos, gPointer.oldPlace.YPos);
  
}

VOID RedrawPointer()
{
  //always assumed
  if (!gPointer.SimplePointerProtocol) {
   return;
  }
  HidePointer();
  DrawPointer();
}

EFI_STATUS MouseBirth()
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  if (!gSettings.PointerEnabled) {
    return EFI_SUCCESS;
  }
  
  if (gPointer.SimplePointerProtocol) { //do not double
//    DBG("DrawPointer\n");
    DrawPointer();
    return EFI_SUCCESS;
  }
  //Status = gBS->LocateProtocol (&gEfiSimplePointerProtocolGuid, NULL, (VOID**)&gPointer.SimplePointerProtocol);

  // Try first to use mouse from System Table
  Status = gBS->HandleProtocol (gST->ConsoleInHandle, &gEfiSimplePointerProtocolGuid, (VOID**)&gPointer.SimplePointerProtocol);
  if (EFI_ERROR (Status)) {
      // not found, so use the first found device
      DBG("MouseBirth: No mouse at ConIn, checking if any other device exists\n");
      Status = gBS->LocateProtocol (&gEfiSimplePointerProtocolGuid, NULL, (VOID**)&gPointer.SimplePointerProtocol);
  }
  /*else {
      DBG("MouseBirth: Mouse located at ConIn\n");
  }*/

  if(EFI_ERROR(Status)) {
    gPointer.Pointer = NULL;
    gPointer.MouseEvent = NoEvents;
    gPointer.SimplePointerProtocol = NULL;
    MsgLog("No mouse!\n");
    gSettings.PointerEnabled = FALSE;
    return Status;
  }

  //there may be also trackpad protocol but afaik it is not properly work and
  // trackpad is usually controlled by simple mouse driver
  
  gPointer.Pointer = BuiltinIcon(BUILTIN_ICON_POINTER);
	if(!gPointer.Pointer) {
    //this is impossible after BuiltinIcon
		DBG("No pointer image!\n");
    gPointer.SimplePointerProtocol = NULL;
    return EFI_NOT_FOUND;
	}
  gPointer.LastClickTime = 0; //AsmReadTsc();
  gPointer.oldPlace.XPos = (INTN)(UGAWidth >> 2);
  gPointer.oldPlace.YPos = (INTN)(UGAHeight >> 2);
  gPointer.oldPlace.Width = POINTER_WIDTH;
  gPointer.oldPlace.Height = POINTER_HEIGHT;
  CopyMem(&gPointer.newPlace, &gPointer.oldPlace, sizeof(EG_RECT));
  
  gPointer.oldImage = egCreateImage(POINTER_WIDTH, POINTER_HEIGHT, FALSE);
  gPointer.newImage = egCreateFilledImage(POINTER_WIDTH, POINTER_HEIGHT, FALSE, &MenuBackgroundPixel);
//  egTakeImage(gPointer.oldImage, gPointer.oldPlace.XPos, gPointer.oldPlace.YPos,
//              POINTER_WIDTH, POINTER_HEIGHT); // DrawPointer repeats it
  DrawPointer();
  gPointer.MouseEvent = NoEvents;
  return Status;
}

VOID KillMouse()
{
//  EG_PIXEL pi;
  if (!gPointer.SimplePointerProtocol) {
    return;
  }
//  pi = gPointer.oldImage->PixelData[0];
//  DBG("Mouse death\n");
//  DBG(" Blue=%x Green=%x Red=%x Alfa=%x\n\n", pi.b, pi.g, pi.r, pi.a);

  
  egFreeImage(gPointer.newImage); gPointer.newImage = NULL;
  egFreeImage(gPointer.oldImage); gPointer.oldImage = NULL;

  // Free Pointer only if it is not builtin icon
  if (gPointer.Pointer != BuiltinIcon(BUILTIN_ICON_POINTER)) {
    egFreeImage(gPointer.Pointer);
  }

  gPointer.Pointer = NULL;
  gPointer.MouseEvent = NoEvents;
  gPointer.SimplePointerProtocol = NULL;
}

// input - tsc
// output - milliseconds
// the caller is responsible for t1 > t0
UINT64 TimeDiff(UINT64 t0, UINT64 t1) 
{
  return DivU64x64Remainder((t1 - t0), DivU64x32(gCPUStructure.TSCFrequency, 1000), 0);
}

VOID PrintPointerVars(
                      INT32     RelX,
                      INT32     RelY,
                      INTN      ScreenRelX,
                      INTN      ScreenRelY,
                      INTN      XPosPrev,
                      INTN      YPosPrev,
                      INTN      XPos,
                      INTN      YPos
                      )
{
  EFI_SIMPLE_POINTER_MODE  *CurrentMode;
//  UINT64   Now;
  
  CurrentMode = gPointer.SimplePointerProtocol->Mode;
//  Now = AsmReadTsc();
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);
//  DBG("%ld                           \n", Now);
  DBG("Resolution X, Y: %ld, %ld           \n", CurrentMode->ResolutionX, CurrentMode->ResolutionY);
  DBG("Relative X, Y: %d, %d (%ld, %ld millimeters)           \n",
        RelX, RelY,
        (INTN)RelX / (INTN)CurrentMode->ResolutionX,
        (INTN)RelY / (INTN)CurrentMode->ResolutionY
        );
  DBG("X: %d + %d = %d -> %d               \n", XPosPrev, ScreenRelX, (XPosPrev + ScreenRelX), XPos);
  DBG("Y: %d + %d = %d -> %d               \n", YPosPrev, ScreenRelY, (YPosPrev + ScreenRelY), YPos);
}

//static INTN PrintCount = 0;

VOID UpdatePointer()
{
//  EFI_TIME Now;
  UINT64                    Now;
  EFI_STATUS                Status; // = EFI_SUCCESS;
  EFI_SIMPLE_POINTER_STATE	tmpState;
  EFI_SIMPLE_POINTER_MODE   *CurrentMode;
//  INTN                      XPosPrev;
//  INTN                      YPosPrev;
  INTN                      ScreenRelX;
  INTN                      ScreenRelY;
  
//  Now = gRT->GetTime(&Now, NULL);
  Now = AsmReadTsc();
  Status = gPointer.SimplePointerProtocol->GetState(gPointer.SimplePointerProtocol, &tmpState);
  if (!EFI_ERROR(Status)) {
    if (!gPointer.State.LeftButton && tmpState.LeftButton) // press left
      gPointer.MouseEvent = LeftMouseDown;
    else if (!gPointer.State.RightButton && tmpState.RightButton) // press right
      gPointer.MouseEvent = RightMouseDown;
    else if (gPointer.State.LeftButton && !tmpState.LeftButton) { //release left
      // time for double click 500ms into menu
      if (TimeDiff(gPointer.LastClickTime, Now) < gSettings.DoubleClickTime)
        gPointer.MouseEvent = DoubleClick;
      else
        gPointer.MouseEvent = LeftClick;
      //     CopyMem(&gPointer.LastClickTime, &Now, sizeof(EFI_TIME));
      gPointer.LastClickTime = Now;
    } else if (gPointer.State.RightButton && !tmpState.RightButton) //release right
      gPointer.MouseEvent = RightClick;
    else if (gPointer.State.RelativeMovementZ > 0)
      gPointer.MouseEvent = ScrollDown;
    else if (gPointer.State.RelativeMovementZ < 0)
      gPointer.MouseEvent = ScrollUp;
    else if (gPointer.State.RelativeMovementX || gPointer.State.RelativeMovementY)
      gPointer.MouseEvent = MouseMove;
    else
      gPointer.MouseEvent = NoEvents;
    
    CopyMem(&gPointer.State, &tmpState, sizeof(EFI_SIMPLE_POINTER_STATE));
    CurrentMode = gPointer.SimplePointerProtocol->Mode;
    
//    XPosPrev = gPointer.newPlace.XPos;
    ScreenRelX = ((UGAWidth * gPointer.State.RelativeMovementX / (INTN)CurrentMode->ResolutionX) * gSettings.PointerSpeed) >> 10;
    if (gSettings.PointerMirror) {
      gPointer.newPlace.XPos -= ScreenRelX;
    } else {
      gPointer.newPlace.XPos += ScreenRelX;
    }
    if (gPointer.newPlace.XPos < 0) gPointer.newPlace.XPos = 0;
    if (gPointer.newPlace.XPos > UGAWidth - 1) gPointer.newPlace.XPos = UGAWidth - 1;
    
//    YPosPrev = gPointer.newPlace.YPos;
    ScreenRelY = ((UGAHeight * gPointer.State.RelativeMovementY / (INTN)CurrentMode->ResolutionY) * gSettings.PointerSpeed) >> 10;
    gPointer.newPlace.YPos += ScreenRelY;
    if (gPointer.newPlace.YPos < 0) gPointer.newPlace.YPos = 0;
    if (gPointer.newPlace.YPos > UGAHeight - 1) gPointer.newPlace.YPos = UGAHeight - 1;
 /*
    if (PrintCount < 1) {
      PrintPointerVars(gPointer.State.RelativeMovementX,
                       gPointer.State.RelativeMovementY,
                       ScreenRelX,
                       ScreenRelY,
                       XPosPrev,
                       YPosPrev,
                       gPointer.newPlace.XPos,
                       gPointer.newPlace.YPos
                       );
      
      PrintCount++;
    }
 */   
    RedrawPointer();
  }
//  return Status;
}

BOOLEAN MouseInRect(EG_RECT *Place)
{
  return  ((gPointer.newPlace.XPos >= Place->XPos) &&
           (gPointer.newPlace.XPos < (Place->XPos + (INTN)Place->Width)) &&
           (gPointer.newPlace.YPos >= Place->YPos) &&
           (gPointer.newPlace.YPos < (Place->YPos + (INTN)Place->Height)));
}

EFI_STATUS CheckMouseEvent(REFIT_MENU_SCREEN *Screen)
{
  EFI_STATUS Status = EFI_TIMEOUT;
  INTN EntryId;
  
  gAction = ActionNone;
  
  if (!Screen) {
    return EFI_TIMEOUT;
  }
  
  if (!IsDragging && gPointer.MouseEvent == MouseMove)
    gPointer.MouseEvent = NoEvents;

// if (gPointer.MouseEvent != NoEvents){
    if (ScrollEnabled && MouseInRect(&UpButton) && gPointer.MouseEvent == LeftClick)
      gAction = ActionScrollUp;
    else if (ScrollEnabled && MouseInRect(&DownButton) && gPointer.MouseEvent == LeftClick)
      gAction = ActionScrollDown;
    else if (ScrollEnabled && MouseInRect(&Scrollbar) && gPointer.MouseEvent == LeftMouseDown) {
      IsDragging = TRUE;
      ScrollbarYMovement = 0;
      ScrollbarOldPointerPlace.XPos = ScrollbarNewPointerPlace.XPos = gPointer.newPlace.XPos;
      ScrollbarOldPointerPlace.YPos = ScrollbarNewPointerPlace.YPos = gPointer.newPlace.YPos;
    } else if (ScrollEnabled && IsDragging && gPointer.MouseEvent == LeftClick) {
      IsDragging = FALSE;
    } else if (ScrollEnabled && IsDragging && gPointer.MouseEvent == MouseMove) {
      gAction = ActionMoveScrollbar;
      ScrollbarNewPointerPlace.XPos = gPointer.newPlace.XPos;
      ScrollbarNewPointerPlace.YPos = gPointer.newPlace.YPos;
    } else if (ScrollEnabled && MouseInRect(&ScrollbarBackground) &&
               gPointer.MouseEvent == LeftClick) {
      if (gPointer.newPlace.YPos < Scrollbar.YPos) // up
        gAction = ActionPageUp;
      else // down
        gAction = ActionPageDown;
      // page up/down, like in OS X
    } else if (ScrollEnabled &&
               gPointer.MouseEvent == ScrollDown) {
      gAction = ActionScrollDown;
    } else if (ScrollEnabled &&
               gPointer.MouseEvent == ScrollUp) {
      gAction = ActionScrollUp;
    } else {
      for (EntryId = 0; EntryId < Screen->EntryCount; EntryId++) {
        if (MouseInRect(&(Screen->Entries[EntryId]->Place))) {
          switch (gPointer.MouseEvent) {
            case LeftClick:
              gAction = Screen->Entries[EntryId]->AtClick;
              //          DBG("Click\n");
              break;
            case RightClick:
              gAction = Screen->Entries[EntryId]->AtRightClick;
              break;
            case DoubleClick:
              gAction = Screen->Entries[EntryId]->AtDoubleClick;
              break;
            case ScrollDown:
              gAction = ActionScrollDown;
              break;
            case ScrollUp:
              gAction = ActionScrollUp;
              break;
            case MouseMove:
              gAction = Screen->Entries[EntryId]->AtMouseOver;
              //how to do the action once?
              break;
            default:
              gAction = ActionNone;
              break;
          }
          gItemID = EntryId;
          break;
        } else { //click in milk
          switch (gPointer.MouseEvent) {
            case LeftClick:
              gAction = ActionDeselect;
              break;
            case RightClick:
              gAction = ActionFinish;
              break;
            case ScrollDown:
              gAction = ActionScrollDown;
              break;
            case ScrollUp:
              gAction = ActionScrollUp;
              break;
            default:
              gAction = ActionNone;
              break;
          }
          gItemID = 0xFFFF;
        }
      }
    }
// }
  if (gAction != ActionNone) {
    Status = EFI_SUCCESS;
    gPointer.MouseEvent = NoEvents; //clear event as set action
  }
  return Status;
}

#define ONE_SECOND  10000000
#define ONE_MSECOND    10000

// TimeoutDefault for a wait in seconds
// return EFI_TIMEOUT if no inputs
EFI_STATUS WaitForInputEventPoll(REFIT_MENU_SCREEN *Screen, UINTN TimeoutDefault)
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN TimeoutRemain = TimeoutDefault * 100;

  while (TimeoutRemain != 0) {
    
//    Status = WaitForSingleEvent (gST->ConIn->WaitForKey, ONE_MSECOND * 10);
    Status = WaitFor2EventWithTsc (gST->ConIn->WaitForKey, NULL, 10);
    
    if (Status != EFI_TIMEOUT) {
      break;
    }
    UpdateAnime(Screen, &(Screen->FilmPlace));
    if (gSettings.PlayAsync) {
      CheckSyncSound();
    }
/*    if ((INTN)gItemID < Screen->EntryCount) {
      UpdateAnime(Screen->Entries[gItemID]->SubScreen, &(Screen->Entries[gItemID]->Place));
    } */
    TimeoutRemain--;
    if (gPointer.SimplePointerProtocol) {
      UpdatePointer();
      Status = CheckMouseEvent(Screen); //out: gItemID, gAction
      if (Status != EFI_TIMEOUT) { //this check should return timeout if no mouse events occured
        break;
      }
    }
  }
  return Status;
}


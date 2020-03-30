/*
 * a class for mouse support
 */

#include <Platform.h>

#include "XPointer.h"
#include "libegint.h"   //this includes platform.h 
#include "../refit/screen.h"
#include "../refit/menu.h"

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

// Initial value, but later it will be theme dependent
#define POINTER_WIDTH  64
#define POINTER_HEIGHT 64

XPointer::XPointer()
            : SimplePointerProtocol(NULL), PointerImage(NULL),
//              newImage(POINTER_WIDTH, POINTER_HEIGHT),
              oldImage(0, 0), Alive(false)
{

}

XPointer::~XPointer()
{
}

void XPointer::Hide()
{
  if (Alive) {
    oldImage.DrawWithoutCompose(oldPlace.XPos, oldPlace.YPos);
  }
}

bool XPointer::isAlive()
{
  return Alive;
}

EFI_STATUS XPointer::MouseBirth()
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  if (!gSettings.PointerEnabled) {
    return EFI_SUCCESS;
  }

  if (SimplePointerProtocol) { //do not double
//    DBG("Mouse is already here\n");
    Draw();
    return EFI_SUCCESS;
  }

  // Try first to use mouse from System Table
  Status = gBS->HandleProtocol(gST->ConsoleInHandle, &gEfiSimplePointerProtocolGuid, (VOID**)&SimplePointerProtocol);
  if (EFI_ERROR(Status)) {
    // not found, so use the first found device
    DBG("MouseBirth: No mouse at ConIn, checking if any other device exists\n");
    Status = gBS->LocateProtocol(&gEfiSimplePointerProtocolGuid, NULL, (VOID**)&SimplePointerProtocol);
  }

  if (EFI_ERROR(Status)) {
    MsgLog("No mouse driver found!\n");
    if (PointerImage) {
      delete PointerImage;
      PointerImage = NULL;
    }
    MouseEvent = NoEvents;
    SimplePointerProtocol = NULL;
    gSettings.PointerEnabled = FALSE;
    return Status;
  }

  if (PointerImage && !PointerImage->isEmpty() ) {
    delete PointerImage;
    PointerImage = nullptr;
  }
//  Now update image because of other theme has other image
#if USE_XTHEME
//  XImage PointerImageX = ThemeX.GetIcon(BUILTIN_ICON_POINTER);
//  PointerImage = &PointerImageX;
  PointerImage = new XImage(ThemeX.GetIcon(BUILTIN_ICON_POINTER));
#else
  PointerImage = new XImage(BuiltinIcon(BUILTIN_ICON_POINTER));
#endif


  oldImage.setSizeInPixels(PointerImage->GetWidth(), PointerImage->GetHeight());
  LastClickTime = 0;
  oldPlace.XPos = (INTN)(UGAWidth >> 2);
  oldPlace.YPos = (INTN)(UGAHeight >> 2);
  oldPlace.Width = PointerImage->GetWidth();
  oldPlace.Height = PointerImage->GetHeight();
//  CopyMem(&newPlace, &oldPlace, sizeof(EG_RECT));
  newPlace = oldPlace;
  Draw();
  MouseEvent = NoEvents;
  Alive = true;
  return Status;
}

VOID XPointer::Draw()
{
  oldPlace = newPlace;
//  CopyMem(&oldPlace, &newPlace, sizeof(EG_RECT));  //can we use oldPlace = newPlace; ?
// take background image for later to restore background
  oldImage.GetArea(newPlace);
  PointerImage->Draw(newPlace.XPos, newPlace.YPos); //zero means no scale
}

VOID XPointer::KillMouse()
{

  Alive = false;
  if (!SimplePointerProtocol) {
    return;
  }
//  DBG("KillMouse\n");

  if (PointerImage) {
    delete PointerImage;
    PointerImage = nullptr;
  }

  MouseEvent = NoEvents;
  SimplePointerProtocol = NULL;
}

VOID XPointer::UpdatePointer()
{
  UINT64                    Now;
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE	tmpState;
  EFI_SIMPLE_POINTER_MODE   *CurrentMode;
  INTN                      ScreenRelX;
  INTN                      ScreenRelY;

  //  Now = gRT->GetTime(&Now, NULL);
  Now = AsmReadTsc();
  Status = SimplePointerProtocol->GetState(SimplePointerProtocol, &tmpState);
  if (!EFI_ERROR(Status)) {
    if (!State.LeftButton && tmpState.LeftButton) // press left
      MouseEvent = LeftMouseDown;
    else if (!State.RightButton && tmpState.RightButton) // press right
      MouseEvent = RightMouseDown;
    else if (State.LeftButton && !tmpState.LeftButton) { //release left
      // time for double click 500ms into menu
      if (TimeDiff(LastClickTime, Now) < gSettings.DoubleClickTime)
        MouseEvent = DoubleClick;
      else
        MouseEvent = LeftClick;
      LastClickTime = Now;
    }
    else if (State.RightButton && !tmpState.RightButton) //release right
      MouseEvent = RightClick;
    else if (State.RelativeMovementZ > 0)
      MouseEvent = ScrollDown;
    else if (State.RelativeMovementZ < 0)
      MouseEvent = ScrollUp;
    else if (State.RelativeMovementX || State.RelativeMovementY)
      MouseEvent = MouseMove;
    else
      MouseEvent = NoEvents;

    CopyMem(&State, &tmpState, sizeof(State));
    CurrentMode = SimplePointerProtocol->Mode;

    ScreenRelX = (UGAWidth * State.RelativeMovementX * gSettings.PointerSpeed / (INTN)CurrentMode->ResolutionX) >> 10;
    if (gSettings.PointerMirror) {
      newPlace.XPos -= ScreenRelX;
    }
    else {
      newPlace.XPos += ScreenRelX;
    }
    if (newPlace.XPos < 0) newPlace.XPos = 0;
    if (newPlace.XPos > UGAWidth - 1) newPlace.XPos = UGAWidth - 1;

    //    YPosPrev = newPlace.YPos;
    ScreenRelY = (UGAHeight * State.RelativeMovementY * gSettings.PointerSpeed / (INTN)CurrentMode->ResolutionY) >> 10;
    newPlace.YPos += ScreenRelY;
    if (newPlace.YPos < 0) newPlace.YPos = 0;
    if (newPlace.YPos > UGAHeight - 1) newPlace.YPos = UGAHeight - 1;

    if ( oldPlace != newPlace ) {
      Hide();
      Draw();
    }
  }
}

bool XPointer::MouseInRect(EG_RECT *Place)
{
  return  ((newPlace.XPos >= Place->XPos) &&
    (newPlace.XPos < (Place->XPos + (INTN)Place->Width)) &&
    (newPlace.YPos >= Place->YPos) &&
    (newPlace.YPos < (Place->YPos + (INTN)Place->Height)));
}

EFI_STATUS XPointer::CheckMouseEvent(REFIT_MENU_SCREEN *Screen)
{
  if (!Screen) {
    return EFI_TIMEOUT;
  }
  EFI_STATUS Status = EFI_TIMEOUT;
  Screen->mAction = ActionNone;
  bool Move = false;

  if (!IsDragging && MouseEvent == MouseMove)
    MouseEvent = NoEvents;

  if (Screen->ScrollEnabled){
    if (MouseInRect(&UpButton) && MouseEvent == LeftClick)
      Screen->mAction = ActionScrollUp;
    else if (MouseInRect(&DownButton) && MouseEvent == LeftClick)
      Screen->mAction = ActionScrollDown;
    else if (MouseInRect(&Scrollbar) && MouseEvent == LeftMouseDown) {
      IsDragging = TRUE;
      Move = true;
//      Screen->mAction = ActionMoveScrollbar;
      ScrollbarYMovement = 0;
      ScrollbarOldPointerPlace.XPos = ScrollbarNewPointerPlace.XPos = newPlace.XPos;
      ScrollbarOldPointerPlace.YPos = ScrollbarNewPointerPlace.YPos = newPlace.YPos;
    }
    else if (IsDragging && MouseEvent == LeftClick) {
      IsDragging = FALSE;
      Move = true;
//      Screen->mAction = ActionMoveScrollbar;
    }
    else if (IsDragging && MouseEvent == MouseMove) {
      Screen->mAction = ActionMoveScrollbar;
      ScrollbarNewPointerPlace.XPos = newPlace.XPos;
      ScrollbarNewPointerPlace.YPos = newPlace.YPos;
    }
    else if (MouseInRect(&ScrollbarBackground) &&
             MouseEvent == LeftClick) {
      if (newPlace.YPos < Scrollbar.YPos) // up
        Screen->mAction = ActionPageUp;
      else // down
        Screen->mAction = ActionPageDown;
    // page up/down, like in OS X
    }
    else if (MouseEvent == ScrollDown) {
      Screen->mAction = ActionScrollDown;
    }
    else if (MouseEvent == ScrollUp) {
      Screen->mAction = ActionScrollUp;
    }
  }
  if (!Screen->ScrollEnabled || (Screen->mAction == ActionNone && !Move) ) {
      for (UINTN EntryId = 0; EntryId < Screen->Entries.size(); EntryId++) {
        if (MouseInRect(&(Screen->Entries[EntryId].Place))) {
          switch (MouseEvent) {
            case LeftClick:
              Screen->mAction = Screen->Entries[EntryId].AtClick;
              //          DBG("Click\n");
              break;
            case RightClick:
              Screen->mAction = Screen->Entries[EntryId].AtRightClick;
              break;
            case DoubleClick:
              Screen->mAction = Screen->Entries[EntryId].AtDoubleClick;
              break;
            case ScrollDown:
              Screen->mAction = ActionScrollDown;
              break;
            case ScrollUp:
              Screen->mAction = ActionScrollUp;
              break;
            case MouseMove:
              Screen->mAction = Screen->Entries[EntryId].AtMouseOver;
              //how to do the action once?
              break;
            default:
              Screen->mAction = ActionNone;
              break;
          }
          Screen->mItemID = EntryId;
          break;
        }
        else { //click in milk
          switch (MouseEvent) {
            case LeftClick:
              Screen->mAction = ActionDeselect;
              break;
            case RightClick:
              Screen->mAction = ActionFinish;
              break;
            case ScrollDown:
              Screen->mAction = ActionScrollDown;
              break;
            case ScrollUp:
              Screen->mAction = ActionScrollUp;
              break;
            default:
              Screen->mAction = ActionNone;
              break;
          }
          Screen->mItemID = 0xFFFF;
        }
      }

  }

  if (Screen->mAction != ActionNone) {
    Status = EFI_SUCCESS;
    MouseEvent = NoEvents; //clear event as set action
  }
  return Status;
}


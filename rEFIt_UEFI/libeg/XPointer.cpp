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
  newPlace.Width = PointerImage->GetWidth();
  newPlace.Height = PointerImage->GetHeight();
  oldImage.GetArea(newPlace); //GetArea will resize oldImage, so correct newPlace
  newPlace.Width = oldImage.GetWidth();
  newPlace.Height = oldImage.GetHeight();
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

MOUSE_EVENT XPointer::GetEvent()
{
  return MouseEvent;
}

bool XPointer::MouseInRect(EG_RECT *Place)
{
  return  ((newPlace.XPos >= Place->XPos) &&
    (newPlace.XPos < (Place->XPos + (INTN)Place->Width)) &&
    (newPlace.YPos >= Place->YPos) &&
    (newPlace.YPos < (Place->YPos + (INTN)Place->Height)));
}

EFI_STATUS REFIT_MENU_SCREEN::CheckMouseEvent()
{
  EFI_STATUS Status = EFI_TIMEOUT;
  mAction = ActionNone;
  MOUSE_EVENT Event = mPointer.GetEvent();
  bool Move = false;

  if (!IsDragging && Event == MouseMove)
    Event = NoEvents;

  if (ScrollEnabled){
    if (mPointer.MouseInRect(&UpButton) && Event == LeftClick)
      mAction = ActionScrollUp;
    else if (mPointer.MouseInRect(&DownButton) && Event == LeftClick)
      mAction = ActionScrollDown;
    else if (mPointer.MouseInRect(&Scrollbar) && Event == LeftMouseDown) {
      IsDragging = TRUE;
      Move = true;
//      mAction = ActionMoveScrollbar;
      ScrollbarYMovement = 0;
      ScrollbarOldPointerPlace.XPos = ScrollbarNewPointerPlace.XPos = mPointer.GetPlace().XPos;
      ScrollbarOldPointerPlace.YPos = ScrollbarNewPointerPlace.YPos = mPointer.GetPlace().YPos;
    }
    else if (IsDragging && Event == LeftClick) {
      IsDragging = FALSE;
      Move = true;
//      mAction = ActionMoveScrollbar;
    }
    else if (IsDragging && Event == MouseMove) {
      mAction = ActionMoveScrollbar;
      ScrollbarNewPointerPlace.XPos = mPointer.GetPlace().XPos;
      ScrollbarNewPointerPlace.YPos = mPointer.GetPlace().YPos;
    }
    else if (mPointer.MouseInRect(&ScrollbarBackground) &&
             Event == LeftClick) {
      if (mPointer.GetPlace().YPos < Scrollbar.YPos) // up
        mAction = ActionPageUp;
      else // down
        mAction = ActionPageDown;
    // page up/down, like in OS X
    }
    else if (Event == ScrollDown) {
      mAction = ActionScrollDown;
    }
    else if (Event == ScrollUp) {
      mAction = ActionScrollUp;
    }
  }
  if (!ScrollEnabled || (mAction == ActionNone && !Move) ) {
      for (UINTN EntryId = 0; EntryId < Entries.size(); EntryId++) {
        if (mPointer.MouseInRect(&(Entries[EntryId].Place))) {
          switch (Event) {
            case LeftClick:
              mAction = Entries[EntryId].AtClick;
              //          DBG("Click\n");
              break;
            case RightClick:
              mAction = Entries[EntryId].AtRightClick;
              break;
            case DoubleClick:
              mAction = Entries[EntryId].AtDoubleClick;
              break;
            case ScrollDown:
              mAction = ActionScrollDown;
              break;
            case ScrollUp:
              mAction = ActionScrollUp;
              break;
            case MouseMove:
              mAction = Entries[EntryId].AtMouseOver;
              //how to do the action once?
              break;
            default:
              mAction = ActionNone;
              break;
          }
          mItemID = EntryId;
          break;
        }
        else { //click in milk
          switch (Event) {
            case LeftClick:
              mAction = ActionDeselect;
              break;
            case RightClick:
              mAction = ActionFinish;
              break;
            case ScrollDown:
              mAction = ActionScrollDown;
              break;
            case ScrollUp:
              mAction = ActionScrollUp;
              break;
            default:
              mAction = ActionNone;
              break;
          }
          mItemID = 0xFFFF;
        }
      }

  }

  if (mAction != ActionNone) {
    Status = EFI_SUCCESS;
 //   Event = NoEvents; //clear event as set action
    mPointer.ClearEvent();
  }
  return Status;
}


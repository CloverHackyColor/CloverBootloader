/*
 * a class for mouse support
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

#include "XPointer.h"
#include "libegint.h"   //this includes platform.h 
#include "../refit/screen.h"
#include "../Platform/Settings.h"
#include "../libeg/XTheme.h"

#ifndef DEBUG_ALL
#define DEBUG_MOUSE 0
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

  if (!gSettings.GUI.Mouse.PointerEnabled) {
    return EFI_SUCCESS;
  }

  if (SimplePointerProtocol) { //do not double
//    DBG("Mouse is already here\n");
    Draw();
    return EFI_SUCCESS;
  }

  // Try first to use mouse from System Table
  Status = gBS->HandleProtocol(gST->ConsoleInHandle, &gEfiSimplePointerProtocolGuid, (void**)&SimplePointerProtocol);
  if (EFI_ERROR(Status)) {
    // not found, so use the first found device
    DBG("MouseBirth: No mouse at ConIn, checking if any other device exists\n");
    Status = gBS->LocateProtocol(&gEfiSimplePointerProtocolGuid, NULL, (void**)&SimplePointerProtocol);
  }

  if (EFI_ERROR(Status)) {
    MsgLog("No mouse driver found!\n");
    if (PointerImage) {
      delete PointerImage;
      PointerImage = NULL;
    }
    MouseEvent = NoEvents;
    SimplePointerProtocol = NULL;
    gSettings.GUI.Mouse.PointerEnabled = FALSE;
    return Status;
  }

  if (PointerImage && !PointerImage->isEmpty() ) {
    delete PointerImage;
    PointerImage = nullptr;
  }
//  Now update image because of other theme has other image
  PointerImage = new XImage(*ThemeX.GetIconP(BUILTIN_ICON_POINTER)->GetBest(night));

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

void XPointer::Draw()
{
  oldPlace = newPlace;

  // take background image for later to restore background
  newPlace.Width = PointerImage->GetWidth();
  newPlace.Height = PointerImage->GetHeight();
  oldImage.GetArea(newPlace); //GetArea will resize oldImage, so correct newPlace
  newPlace.Width = oldImage.GetWidth();
  newPlace.Height = oldImage.GetHeight();
  PointerImage->Draw(newPlace.XPos, newPlace.YPos); //zero means no scale
}

void XPointer::KillMouse()
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

void XPointer::UpdatePointer(bool isDaylight) // cannot be called daylight because of a global var on macOS
{
  UINT64                    Now;
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE	tmpState;
  EFI_SIMPLE_POINTER_MODE   *CurrentMode;
  INTN                      ScreenRelX;
  INTN                      ScreenRelY;
  
  night = !isDaylight;

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
      if (TimeDiff(LastClickTime, Now) < gSettings.GUI.Mouse.DoubleClickTime)
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

    ScreenRelX = (UGAWidth * State.RelativeMovementX * gSettings.GUI.Mouse.PointerSpeed / (INTN)CurrentMode->ResolutionX) >> 10;
    if (gSettings.GUI.Mouse.PointerMirror) {
      newPlace.XPos -= ScreenRelX;
    }
    else {
      newPlace.XPos += ScreenRelX;
    }
    if (newPlace.XPos < 0) newPlace.XPos = 0;
    if (newPlace.XPos > UGAWidth - 1) newPlace.XPos = UGAWidth - 1;

    //    YPosPrev = newPlace.YPos;
    ScreenRelY = (UGAHeight * State.RelativeMovementY * gSettings.GUI.Mouse.PointerSpeed / (INTN)CurrentMode->ResolutionY) >> 10;
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

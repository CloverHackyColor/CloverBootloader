#pragma once

#include "XImage.h"
//#include "../refit/IO.h"

class REFIT_MENU_SCREEN;
class XImage;

class XPointer
{
public:
  XPointer();
  ~XPointer();

protected:
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointerProtocol;
  XImage* PointerImage;
//  XImage newImage;
  XImage oldImage;

  EG_RECT  newPlace;
  EG_RECT  oldPlace;

  UINT64	LastClickTime;  //not EFI_TIME
  EFI_SIMPLE_POINTER_STATE    State;
  MOUSE_EVENT MouseEvent;
  bool Alive;

public:
  void Hide();
  bool isAlive();
  EFI_STATUS MouseBirth();
  VOID KillMouse();
  VOID UpdatePointer();
  bool MouseInRect(EG_RECT *Place);
  EFI_STATUS CheckMouseEvent(REFIT_MENU_SCREEN *Screen);

  bool isEmpty() const { return PointerImage->isEmpty(); }

protected:
  VOID Draw();
  VOID DrawPointer();
};

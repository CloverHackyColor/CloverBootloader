#pragma once

#include "XImage.h"
//#include "../refit/IO.h"
#include "libeg.h"

class XImage;

class XPointer
{
public:
  XPointer() : SimplePointerProtocol(NULL), PointerImage(NULL),
               oldImage(0, 0), newPlace(), oldPlace(), LastClickTime(0), State{0}, MouseEvent(NoEvents), Alive(false), night(false)
             {}
  XPointer(const XPointer&) = delete;
  XPointer& operator=(const XPointer&) = delete;

  ~XPointer() {};

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
  bool night;

public:
  void Hide();
  bool isAlive();
  EFI_STATUS MouseBirth();
  void KillMouse();
  void UpdatePointer(bool daylight);
  bool MouseInRect(EG_RECT *Place);

  bool isEmpty() const { return PointerImage->isEmpty(); }
  void ClearEvent() { MouseEvent = NoEvents; }
  MOUSE_EVENT GetEvent();
  EG_RECT& GetPlace() { return newPlace; }

protected:
  void Draw();
  void DrawPointer();
};

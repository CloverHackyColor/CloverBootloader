/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __REFIT_MENU_SCREEN_H__
#define __REFIT_MENU_SCREEN_H__


#include "../Platform/Settings.h"
#include "../libeg/libegint.h"
//#include "../libeg/libeg.h"
#include "../refit/lib.h"


#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
#include "../libeg/XPointer.h"
#include "../libeg/XCinema.h"
#include "menu_items/menu_items.h"


#ifdef _MSC_VER
#define __attribute__(x)
#endif

//some unreal values
#define FILM_CENTRE   40000
//#define FILM_LEFT     50000
//#define FILM_TOP      50000
//#define FILM_RIGHT    60000
//#define FILM_BOTTOM   60000
//#define FILM_PERCENT 100000
#define INITVALUE       40000

class REFIT_MENU_ENTRY_ITEM_ABSTRACT;
class REFIT_MENU_ENTRY;
class REFIT_ABSTRACT_MENU_ENTRY;

typedef void (REFIT_MENU_SCREEN::*MENU_STYLE_FUNC)(IN UINTN Function, IN CONST CHAR16 *ParamText);

class EntryArray : public XObjArray<REFIT_ABSTRACT_MENU_ENTRY>
{
public:
  bool includeHidden = false;

  size_t sizeIncludingHidden() const { return XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size(); }

  size_t size() const
  {
    if ( includeHidden ) return sizeIncludingHidden();
    size_t size = 0;
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      const REFIT_ABSTRACT_MENU_ENTRY& entry = XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i);
      if ( !entry.Hidden ) {
        size++;
      }
    }
    return size;
  }
  size_t length() const { return size(); }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  REFIT_ABSTRACT_MENU_ENTRY& operator[](IntegralType nIndex)
  {
    if ( includeHidden ) return XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::operator [] (nIndex);
    if (nIndex < 0) {
      panic("EntryArray::operator[] : i < 0. System halted\n");
    }
    size_t size = 0;
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( !XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).Hidden ) {
        if ( size == (unsigned_type(IntegralType))nIndex ) return XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i);
        size++;
      }
    }
    panic("EntryArray::operator[] nIndex > size()");
  }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  const REFIT_ABSTRACT_MENU_ENTRY& operator[](IntegralType nIndex) const
  {
    if ( includeHidden ) return XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::operator [] (nIndex);
    if (nIndex < 0) {
      panic("EntryArray::operator[] : i < 0. System halted\n");
    }
    size_t size = 0;
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( !XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).Hidden ) {
        if ( size == (unsigned_type(IntegralType))nIndex ) return XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i);
        size++;
      }
    }
    panic("EntryArray::operator[] nIndex > size()");
  }
};

class REFIT_MENU_SCREEN
{
public:
  static   XPointer mPointer;
//  XPointer mPointer;
  UINTN             ID;
  XStringW          Title;
  XIcon             TitleImage;
  XStringWArray     InfoLines;

  EntryArray Entries;
  
  INTN              TimeoutSeconds;
  bool              Daylight;
  XStringW          TimeoutText;
  XStringW          ThemeName;  //?
  EG_RECT           OldTextBufferRect;
  XImage            OldTextBufferImage;
  BOOLEAN           isBootScreen;
  FILM              *FilmC;

  ACTION          mAction;
  UINTN           mItemID;
  SCROLL_STATE    ScrollState;
  BOOLEAN         ScrollEnabled;
  INTN            TextStyle;
  BOOLEAN         IsDragging;

  //TODO scroll positions should depends on REFIT_SCREEN?
  // Or it just currently calculated to be global variables?
  EG_RECT BarStart;
  EG_RECT BarEnd;
  EG_RECT ScrollStart;
  EG_RECT ScrollEnd;
  EG_RECT ScrollTotal;
  EG_RECT UpButton;
  EG_RECT DownButton;
  EG_RECT ScrollbarBackground;
  EG_RECT Scrollbar;
  EG_RECT ScrollbarOldPointerPlace;
  EG_RECT ScrollbarNewPointerPlace;



  REFIT_MENU_SCREEN()
      : ID(0), Title(), TitleImage(), InfoLines(), Entries(),
        TimeoutSeconds(0), Daylight(true), TimeoutText(), ThemeName(),
        OldTextBufferRect(), OldTextBufferImage(), isBootScreen(false), FilmC(),
        mAction(ActionNone), mItemID(0), ScrollState{0,0,0,0}, ScrollEnabled(0), TextStyle(0), IsDragging(0),
        BarStart(), BarEnd(), ScrollStart(), ScrollEnd(), ScrollTotal(), UpButton(), DownButton(), ScrollbarBackground(), Scrollbar(), ScrollbarOldPointerPlace(), ScrollbarNewPointerPlace()
  {
    EFI_TIME          Now;
    gRT->GetTime(&Now, NULL);
    if (GlobalConfig.Timezone != 0xFF) {
      INT32 NowHour = Now.Hour + GlobalConfig.Timezone;
      if (NowHour <  0 ) NowHour += 24;
      if (NowHour >= 24 ) NowHour -= 24;
      Daylight = (NowHour > 8) && (NowHour < 20);  //this is the screen member
    } else {
      Daylight = true;
    }
  };

  REFIT_MENU_SCREEN(UINTN ID, XStringW TTitle, XStringW TTimeoutText)
      : ID(ID), Title(TTitle), TitleImage(), InfoLines(), Entries(),
        TimeoutSeconds(0), Daylight(true), TimeoutText(TTimeoutText), ThemeName(),
        OldTextBufferRect(), OldTextBufferImage(), isBootScreen(false), FilmC(),
        mAction(ActionNone), mItemID(0), ScrollState{0,0,0,0}, ScrollEnabled(0), TextStyle(0), IsDragging(0),
        BarStart(), BarEnd(), ScrollStart(), ScrollEnd(), ScrollTotal(), UpButton(), DownButton(), ScrollbarBackground(), Scrollbar(), ScrollbarOldPointerPlace(), ScrollbarNewPointerPlace()
  {};

  //TODO exclude CHAR16
  REFIT_MENU_SCREEN(UINTN ID, CONST CHAR16* TitleC, CONST CHAR16* TimeoutTextC)
      : ID(ID), Title(), TitleImage(), InfoLines(), Entries(),
        TimeoutSeconds(0), Daylight(true), TimeoutText(), ThemeName(),
        OldTextBufferRect(), OldTextBufferImage(), isBootScreen(false), FilmC(),
        mAction(ActionNone), mItemID(0), ScrollState{0,0,0,0}, ScrollEnabled(0), TextStyle(0), IsDragging(0),
        BarStart(), BarEnd(), ScrollStart(), ScrollEnd(), ScrollTotal(), UpButton(), DownButton(), ScrollbarBackground(), Scrollbar(), ScrollbarOldPointerPlace(), ScrollbarNewPointerPlace()
  {
    Title.takeValueFrom(TitleC);
    TimeoutText.takeValueFrom(TimeoutTextC);
  };

  REFIT_MENU_SCREEN(UINTN ID, XStringW  TTitle, XStringW  TTimeoutText, REFIT_ABSTRACT_MENU_ENTRY* entry1, REFIT_ABSTRACT_MENU_ENTRY* entry2)
      : ID(ID), Title(TTitle), TitleImage(), InfoLines(), Entries(),
        TimeoutSeconds(0), Daylight(true), TimeoutText(TTimeoutText), ThemeName(),
        OldTextBufferRect(), OldTextBufferImage(), isBootScreen(false), FilmC(),
        mAction(ActionNone), mItemID(0), ScrollState{0,0,0,0}, ScrollEnabled(0), TextStyle(0), IsDragging(0),
        BarStart(), BarEnd(), ScrollStart(), ScrollEnd(), ScrollTotal(), UpButton(), DownButton(), ScrollbarBackground(), Scrollbar(), ScrollbarOldPointerPlace(), ScrollbarNewPointerPlace()
  {
    Entries.AddReference(entry1, false);
    Entries.AddReference(entry2, false);
  };

  REFIT_MENU_SCREEN(const REFIT_MENU_SCREEN&) = delete;
  REFIT_MENU_SCREEN& operator=(const REFIT_MENU_SCREEN&) = delete;


  //Scroll functions
  void InitScroll(IN INTN ItemCount, IN UINTN MaxCount,
                  IN UINTN VisibleSpace, IN INTN Selected);
  void UpdateScroll(IN UINTN Movement);
//  void InitBar();
  void ScrollingBar();
  void SetBar(INTN PosX, INTN UpPosY, INTN DownPosY, IN SCROLL_STATE *State);

  //mouse functions and event
  void HidePointer();
  EFI_STATUS MouseBirth();
  void KillMouse();
  EFI_STATUS CheckMouseEvent();
  EFI_STATUS WaitForInputEventPoll(UINTN TimeoutDefault);

  //menu functions
  void AddMenuItem_(REFIT_MENU_ENTRY_ITEM_ABSTRACT* InputBootArgs, INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  void AddMenuInfo_f(CONST char *format, ...) __attribute__((format(printf, 2, 3)));
  void AddMenuInfoLine_f(CONST char *format, ...) __attribute__((format(printf, 2, 3)));
  void AddMenuEntry(IN REFIT_ABSTRACT_MENU_ENTRY *Entry, bool freeIt);
  void AddMenuItemSwitch(INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  void AddMenuCheck(CONST CHAR8 *Text, UINTN Bit, INTN ItemNum);
  void AddMenuItemInput(INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  void FreeMenu();
  INTN FindMenuShortcutEntry(IN CHAR16 Shortcut);
  UINTN RunGenericMenu(IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN RunMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN RunMainMenu(IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN InputDialog(IN MENU_STYLE_FUNC StyleFunc);


  void DrawMainMenuEntry(REFIT_ABSTRACT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos);
  void DrawMainMenuLabel(IN CONST XStringW& Text, IN INTN XPos, IN INTN YPos);
  INTN DrawTextXY(IN CONST XStringW& Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);
  void EraseTextXY();
  void DrawTextCorner(UINTN TextC, UINT8 Align);
  void DrawMenuText(IN const XStringW& Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN UINTN Cursor, IN INTN MaxWidth);
  void DrawBCSText(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);
  void CountItems();
  void InitAnime();
  void GetAnime(); //same for xcinema
  void UpdateFilm(); 

  //Style functions

  virtual void MainMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual void MainMenuVerticalStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual void GraphicsMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual void TextMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);

  virtual ~REFIT_MENU_SCREEN() {};
};

#endif
/*

 EOF */

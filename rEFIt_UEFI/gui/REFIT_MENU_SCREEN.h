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

#define SCROLL_LINE_UP        (0)
#define SCROLL_LINE_DOWN      (1)
#define SCROLL_PAGE_UP        (2)
#define SCROLL_PAGE_DOWN      (3)
#define SCROLL_FIRST          (4)
#define SCROLL_LAST           (5)
#define SCROLL_NONE           (6)
#define SCROLL_SCROLL_DOWN    (7)
#define SCROLL_SCROLL_UP      (8)
#define SCROLL_SCROLLBAR_MOVE (9)

//
#define TEXT_CORNER_REVISION  (1)
#define TEXT_CORNER_HELP      (2)
#define TEXT_CORNER_OPTIMUS   (3)
//
#define TITLE_MAX_LEN (SVALUE_MAX_SIZE / sizeof(CHAR16) + 128)

//TODO spacing must be a part of layout in XTheme
#define TITLEICON_SPACING (16)
//#define ROW0__TILESIZE (144)
//#define ROW1_TILESIZE (64)
#define TILE1_XSPACING (8)
//#define TILE_YSPACING (24)
#define ROW0_SCROLLSIZE (100)

#define MENU_FUNCTION_INIT            (0)
#define MENU_FUNCTION_CLEANUP         (1)
#define MENU_FUNCTION_PAINT_ALL       (2)
#define MENU_FUNCTION_PAINT_SELECTION (3)
#define MENU_FUNCTION_PAINT_TIMEOUT   (4)

//some unreal values
#define FILM_CENTRE   40000
//#define FILM_LEFT     50000
//#define FILM_TOP      50000
//#define FILM_RIGHT    60000
//#define FILM_BOTTOM   60000
//#define FILM_PERCENT 100000
#define INITVALUE       40000

#define CONSTRAIN_MIN(Variable, MinValue) if (Variable < MinValue) Variable = MinValue
#define CONSTRAIN_MAX(Variable, MaxValue) if (Variable > MaxValue) Variable = MaxValue



extern INTN row0Count, row0PosX;
extern INTN row1Count, row1PosX;
extern INTN row0PosY;

extern INTN OldX, OldY;
extern INTN OldTextWidth, OldTextHeight;
extern UINTN OldRow;
extern INTN MenuWidth , TimeoutPosY;
extern UINTN MenuMaxTextLen;
extern INTN EntriesPosX, EntriesPosY;



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
#ifdef DEBUG
      panic("EntryArray::operator[] : i < 0. System halted\n");
#else
      return 0;
#endif
    }
    size_t size = 0;
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( !XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).Hidden ) {
        if ( size == (unsigned_type(IntegralType))nIndex ) return XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i);
        size++;
      }
    }
    
#ifdef DEBUG
    panic("EntryArray::operator[] nIndex > size()");
#else
    return 0;
#endif

  }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  const REFIT_ABSTRACT_MENU_ENTRY& operator[](IntegralType nIndex) const
  {
    if ( includeHidden ) return XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::operator [] (nIndex);
    if (nIndex < 0) {
#ifdef DEBUG
      panic("EntryArray::operator[] : i < 0. System halted\n");
#else
      return 0;
#endif
    }
    size_t size = 0;
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( !XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).Hidden ) {
        if ( size == (unsigned_type(IntegralType))nIndex ) return XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i);
        size++;
      }
    }
#ifdef DEBUG
    panic("EntryArray::operator[] nIndex > size()");
#else
    return 0;
#endif
  }

  size_t getIdx(const REFIT_ABSTRACT_MENU_ENTRY* entry)
  {
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( &XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i) == entry ) {
        return i;
      }
    }
    return SIZE_T_MAX;
  }

  size_t getApfsLoaderIdx(const XString8& ApfsContainerUUID, const XString8& ApfsFileSystemUUID)
  {
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY() ) {
        if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->Volume->ApfsContainerUUID == ApfsContainerUUID ) {
          if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->Volume->ApfsFileSystemUUID == ApfsFileSystemUUID ) {
            return i;
          }
        }
      }
    }
    return SIZE_T_MAX;
  }

  size_t getApfsLoaderIdx(const XString8& ApfsContainerUUID, const XString8& ApfsFileSystemUUID, uint8_t osType)
  {
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY() ) {
        if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->LoaderType == osType ) {
          if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->Volume->ApfsContainerUUID == ApfsContainerUUID ) {
            if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->Volume->ApfsFileSystemUUID == ApfsFileSystemUUID ) {
              return i;
            }
          }
        }
      }
    }
    return SIZE_T_MAX;
  }

  size_t getApfsPrebootLoaderIdx(const XString8& ApfsContainerUUID, const XString8& ApfsFileSystemUUID)
  {
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY() ) {
        if ( (XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->Volume->ApfsRole & APPLE_APFS_VOLUME_ROLE_PREBOOT) != 0 ) {
          if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->Volume->ApfsContainerUUID == ApfsContainerUUID ) {
            if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->APFSTargetUUID == ApfsFileSystemUUID ) {
              return i;
            }
          }
        }
      }
    }
    return SIZE_T_MAX;
  }

  size_t getApfsPrebootLoaderIdx(const XString8& ApfsContainerUUID, const XString8& ApfsFileSystemUUID, uint8_t osType)
  {
    for ( size_t i=0 ; i < XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size() ; i++ ) {
      if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY() ) {
        if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->LoaderType == osType ) {
          if ( (XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->Volume->ApfsRole & APPLE_APFS_VOLUME_ROLE_PREBOOT) != 0 ) {
            if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->Volume->ApfsContainerUUID == ApfsContainerUUID ) {
              if ( XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::ElementAt(i).getLOADER_ENTRY()->APFSTargetUUID == ApfsFileSystemUUID ) {
                return i;
              }
            }
          }
        }
      }
    }
    return SIZE_T_MAX;
  }

  template<typename IntegralType1, typename IntegralType2, enable_if(is_integral(IntegralType1) && is_integral(IntegralType2))>
  void moveBefore(IntegralType1 idxFrom, IntegralType2 idxTo)
  {
#ifdef DEBUG
    if (idxFrom < 0) panic("EntryArray::move(IntegralType1, IntegralType2) : idxFrom < 0. System halted\n");
    if ((unsigned_type(IntegralType1))idxFrom >= XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size()) panic("EntryArray::move(IntegralType1, IntegralType2) : idxFrom > size(). System halted\n");
    if (idxTo < 0) panic("EntryArray::move(IntegralType1, IntegralType2) : idxTo < 0. System halted\n");
    if ((unsigned_type(IntegralType2))idxTo >= XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size()) panic("EntryArray::move(IntegralType1, IntegralType2) : idxTo > size(). System halted\n");
#else
    if (idxFrom < 0) return;
    if ((unsigned_type(IntegralType1))idxFrom >= XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size()) return;
    if (idxTo < 0) return;
    if ((unsigned_type(IntegralType2))idxTo >= XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size()) return;
#endif


    REFIT_ABSTRACT_MENU_ENTRY* entry = &ElementAt(idxFrom);
    RemoveWithoutFreeingAtIndex(idxFrom);
    if ( idxTo > idxFrom ) {
      InsertRef(entry, idxTo-1, true);
    }else{
      InsertRef(entry, idxTo, true);
    }
  }

  template<typename IntegralType1, typename IntegralType2, enable_if(is_integral(IntegralType1) && is_integral(IntegralType2))>
  void moveAfter(IntegralType1 idxFrom, IntegralType2 idxTo)
  {
#ifdef DEBUG
    if (idxFrom < 0) panic("EntryArray::move(IntegralType1, IntegralType2) : idxFrom < 0. System halted\n");
    if ((unsigned_type(IntegralType1))idxFrom >= XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size()) panic("EntryArray::move(IntegralType1, IntegralType2) : idxFrom > size(). System halted\n");
    if (idxTo < 0) panic("EntryArray::move(IntegralType1, IntegralType2) : idxTo < 0. System halted\n");
    if ((unsigned_type(IntegralType2))idxTo >= XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size()) panic("EntryArray::move(IntegralType1, IntegralType2) : idxTo > size(). System halted\n");
#else
    if (idxFrom < 0) return;
    if ((unsigned_type(IntegralType1))idxFrom >= XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size()) return;
    if (idxTo < 0) return;
    if ((unsigned_type(IntegralType2))idxTo >= XObjArray<REFIT_ABSTRACT_MENU_ENTRY>::size()) return;
#endif

    REFIT_ABSTRACT_MENU_ENTRY* entry = &ElementAt(idxFrom);
    RemoveWithoutFreeingAtIndex(idxFrom);
    if ( idxTo > idxFrom ) {
      InsertRef(entry, idxTo, true);
    }else{
      InsertRef(entry, idxTo+1, true);
    }
  }
};

class REFIT_MENU_SCREEN
{
public:
  static   XPointer mPointer;
//  XPointer mPointer;
  UINTN             ID = 0;
  XStringW          Title = XStringW();
  XIcon             TitleImage = XIcon();
  XStringWArray     InfoLines = XStringWArray();

  EntryArray Entries = EntryArray();
  
  INTN              TimeoutSeconds = 0;
  bool              Daylight = true;
  XStringW          TimeoutText = XStringW();
  XStringW          ThemeName = XStringW();  //?
  EG_RECT           OldTextBufferRect = EG_RECT();
  XImage            OldTextBufferImage = XImage();
  BOOLEAN           isBootScreen = 0;
  FILM              *FilmC = 0;

  ACTION          mAction = ActionNone;
  UINTN           mItemID = 0;
  SCROLL_STATE    ScrollState = {0,0,0,0,0,0,0,0,0,0,0};
  BOOLEAN         ScrollEnabled = 0;
  INTN            TextStyle = 0;
  BOOLEAN         IsDragging = 0;

  //TODO scroll positions should depends on REFIT_SCREEN?
  // Or it just currently calculated to be global variables?
  EG_RECT BarStart = EG_RECT();
  EG_RECT BarEnd = EG_RECT();
  EG_RECT ScrollStart = EG_RECT();
  EG_RECT ScrollEnd = EG_RECT();
  EG_RECT ScrollTotal = EG_RECT();
  EG_RECT UpButton = EG_RECT();
  EG_RECT DownButton = EG_RECT();
  EG_RECT ScrollbarBackground = EG_RECT();
  EG_RECT Scrollbar = EG_RECT();
  EG_RECT ScrollbarOldPointerPlace = EG_RECT();
  EG_RECT ScrollbarNewPointerPlace = EG_RECT();


  void common_init() {
#ifdef CLOVER_BUILD
    EFI_TIME          Now;
    gRT->GetTime(&Now, NULL);
    if (gSettings.GUI.Timezone != 0xFF) {
      INT32 NowHour = Now.Hour + gSettings.GUI.Timezone;
      if (NowHour <  0 ) NowHour += 24;
      if (NowHour >= 24 ) NowHour -= 24;
      Daylight = (NowHour > 8) && (NowHour < 20);  //this is the screen member
    } else {
      Daylight = true;
    }
#endif
  }

  REFIT_MENU_SCREEN()
  {
    common_init();
  };

  REFIT_MENU_SCREEN(UINTN ID, XStringW TTitle, XStringW TTimeoutText) : ID(ID), Title(TTitle), TimeoutText(TTimeoutText) { common_init(); };

  //TODO exclude CHAR16
  REFIT_MENU_SCREEN(UINTN ID, CONST CHAR16* TitleC, CONST CHAR16* TimeoutTextC) : ID(ID)
  {
    common_init();
    Title.takeValueFrom(TitleC);
    TimeoutText.takeValueFrom(TimeoutTextC);
  };

  REFIT_MENU_SCREEN(UINTN ID, XStringW  TTitle, XStringW  TTimeoutText, REFIT_ABSTRACT_MENU_ENTRY* entry1, REFIT_ABSTRACT_MENU_ENTRY* entry2) : ID(ID), Title(TTitle), TimeoutText(TTimeoutText)
  {
    common_init();
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
  UINTN RunGenericMenu(IN OUT INTN *DefaultEntryIndex, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN RunMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN InputDialog();


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

  virtual void GraphicsMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual void TextMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual void MenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText) {
    if (AllowGraphicsMode) GraphicsMenuStyle(Function, ParamText);
    else                   TextMenuStyle(Function, ParamText);
  }

//  MENU_STYLE_FUNC  m_StyleFunc = NULL;

  virtual void call_MENU_FUNCTION_INIT(IN CONST CHAR16 *ParamText)              { MenuStyle(MENU_FUNCTION_INIT, ParamText); }
  virtual void call_MENU_FUNCTION_PAINT_ALL(IN CONST CHAR16 *ParamText)         { MenuStyle(MENU_FUNCTION_PAINT_ALL, ParamText); }
  virtual void call_MENU_FUNCTION_PAINT_SELECTION(IN CONST CHAR16 *ParamText)   { MenuStyle(MENU_FUNCTION_PAINT_SELECTION, ParamText); }
  virtual void call_MENU_FUNCTION_PAINT_TIMEOUT(IN CONST CHAR16 *ParamText)     { MenuStyle(MENU_FUNCTION_PAINT_TIMEOUT, ParamText); }
  virtual void call_MENU_FUNCTION_CLEANUP(IN CONST CHAR16 *ParamText)           { MenuStyle(MENU_FUNCTION_CLEANUP, ParamText); }

  virtual ~REFIT_MENU_SCREEN() {};
};


#endif
/*

 EOF */

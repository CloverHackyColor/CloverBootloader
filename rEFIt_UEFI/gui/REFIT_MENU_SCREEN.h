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


#include "libeg.h"
#include "../refit/lib.h"

#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XStringWArray.h"
#include "../cpp_foundation/XStringW.h"
#include "../libeg/XPointer.h"
#include "menu_items/menu_items.h"


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

typedef VOID (REFIT_MENU_SCREEN::*MENU_STYLE_FUNC)(IN UINTN Function, IN CONST CHAR16 *ParamText);

class REFIT_MENU_SCREEN
{
public:
  static   XPointer mPointer;
//  XPointer mPointer;
  UINTN             ID;
  CONST  CHAR16      *Title;  //Title is not const, but *Title is. It will be better to make it XStringW
  EG_IMAGE          *TitleImage;
  XStringWArray     InfoLines;
  XObjArray<REFIT_ABSTRACT_MENU_ENTRY> Entries;
  INTN              TimeoutSeconds;
  CONST CHAR16     *TimeoutText;
  CONST CHAR16     *Theme;
  BOOLEAN           AnimeRun;
  BOOLEAN           Once;
  UINT64            LastDraw;
  INTN              CurrentFrame;
  INTN              Frames;
  UINTN             FrameTime; //ms
  EG_RECT           FilmPlace;
  EG_IMAGE        **Film;
  ACTION      mAction;
  UINTN       mItemID;
  SCROLL_STATE ScrollState;
//  MENU_STYLE_FUNC StyleFunc;


  REFIT_MENU_SCREEN()
						: ID(0), Title(0), TitleImage(0),
						  TimeoutSeconds(0), TimeoutText(0), Theme(0), AnimeRun(0),
						  Once(0), LastDraw(0), CurrentFrame(0),
						  Frames(0), FrameTime(0),
						  Film(0), mAction(ActionNone), mItemID(0)//, mPointer(NULL) //, StyleFunc(&REFIT_MENU_SCREEN::TextMenuStyle)
						{};

  REFIT_MENU_SCREEN(UINTN ID, CONST CHAR16* Title, CONST CHAR16* TimeoutText)
						: ID(ID), Title(Title), TitleImage(0),
						  TimeoutSeconds(0), TimeoutText(TimeoutText), Theme(0), AnimeRun(0),
						  Once(0), LastDraw(0), CurrentFrame(0),
						  Frames(0), FrameTime(0),
						  Film(0), mAction(ActionNone), mItemID(0)//, mPointer(NULL) //, StyleFunc(&REFIT_MENU_SCREEN::TextMenuStyle)
						{};
  REFIT_MENU_SCREEN(UINTN ID, CONST CHAR16* Title, CONST CHAR16* TimeoutText, REFIT_ABSTRACT_MENU_ENTRY* entry1, REFIT_ABSTRACT_MENU_ENTRY* entry2)
						: ID(ID), Title(Title), TitleImage(0),
						  TimeoutSeconds(0), TimeoutText(TimeoutText), Theme(0), AnimeRun(0),
						  Once(0), LastDraw(0), CurrentFrame(0),
						  Frames(0), FrameTime(0),
						  Film(0), mAction(ActionNone), mItemID(0)//, mPointer(NULL) //, StyleFunc(&REFIT_MENU_SCREEN::TextMenuStyle)
						{
	  	  	  	  	  	  	  Entries.AddReference(entry1, false);
	  	  	  	  	  	  	  Entries.AddReference(entry2, false);
						};

  //Scroll functions
  VOID InitScroll(IN INTN ItemCount, IN UINTN MaxCount,
                  IN UINTN VisibleSpace, IN INTN Selected);
  VOID UpdateScroll(IN UINTN Movement);
  VOID HidePointer();
  EFI_STATUS MouseBirth();
  VOID KillMouse();
  VOID AddMenuItem_(REFIT_MENU_ENTRY_ITEM_ABSTRACT* InputBootArgs, INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  VOID AddMenuInfo(CONST CHAR16 *Line);
  VOID AddMenuInfoLine(IN CONST CHAR16 *InfoLine);
  VOID AddMenuEntry(IN REFIT_ABSTRACT_MENU_ENTRY *Entry, bool freeIt);
  VOID AddMenuItemSwitch(INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  VOID AddMenuCheck(CONST CHAR8 *Text, UINTN Bit, INTN ItemNum);
  VOID AddMenuItemInput(INTN Inx, CONST CHAR8 *Title, BOOLEAN Cursor);
  VOID FreeMenu();
  INTN FindMenuShortcutEntry(IN CHAR16 Shortcut);
  UINTN RunGenericMenu(IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN RunMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN RunMainMenu(IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);
  UINTN InputDialog(IN MENU_STYLE_FUNC StyleFunc);

  VOID DrawMainMenuLabel(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos);
  VOID CountItems();
  VOID InitAnime();
  BOOLEAN GetAnime();
  VOID UpdateAnime();


  //Style functions
  virtual VOID MainMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual VOID MainMenuVerticalStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual VOID GraphicsMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);
  virtual VOID TextMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText);

  ~REFIT_MENU_SCREEN() {};
};

#endif
/*

 EOF */

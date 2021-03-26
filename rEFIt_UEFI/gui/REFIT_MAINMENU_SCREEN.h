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

#ifndef __REFIT_MAINMENU_SCREEN_H__
#define __REFIT_MAINMENU_SCREEN_H__

#include "REFIT_MENU_SCREEN.h"
#include "../libeg/XTheme.h"
//#include "../Platform/Settings.h"
//#include "../libeg/libegint.h"
////#include "../libeg/libeg.h"
//#include "../refit/lib.h"
//
//
//#include "../cpp_foundation/XObjArray.h"
//#include "../cpp_foundation/XString.h"
//#include "../cpp_foundation/XStringArray.h"
//#include "../libeg/XPointer.h"
//#include "../libeg/XCinema.h"
//#include "menu_items/menu_items.h"
//
//
//#ifdef _MSC_VER
//#define __attribute__(x)
//#endif


class REFIT_MAINMENU_SCREEN : public REFIT_MENU_SCREEN
{
public:
  typedef void (REFIT_MAINMENU_SCREEN::*MAINMENU_STYLE_FUNC)(IN UINTN Function, IN CONST CHAR16 *ParamText);
//  MAINMENU_STYLE_FUNC m_MainStyle = NULL;

  REFIT_MAINMENU_SCREEN(UINTN ID, XStringW TTitle, XStringW TTimeoutText);

  UINTN RunMainMenu(IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);

  virtual void TextMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText) { REFIT_MENU_SCREEN::TextMenuStyle(Function, ParamText); }
  virtual void MainMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText); // cannot remove from here because the use of pointer to member function
  virtual void MainMenuVerticalStyle(IN UINTN Function, IN CONST CHAR16 *ParamText); // cannot remove from here because the use of pointer to member function
  virtual void MenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText) {
    if (AllowGraphicsMode) {
      if (ThemeX.VerticalLayout) {
        MainMenuVerticalStyle(Function, ParamText);
      } else {
        MainMenuStyle(Function, ParamText);
      }
    }else {
      TextMenuStyle(Function, ParamText);
    }
  }

  void DrawMainMenuEntry(REFIT_ABSTRACT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos);
  void DrawMainMenuLabel(IN CONST XStringW& Text, IN INTN XPos, IN INTN YPos);

  virtual void call_MENU_FUNCTION_INIT(IN CONST CHAR16 *ParamText)              { MenuStyle(MENU_FUNCTION_INIT, ParamText); }
  virtual void call_MENU_FUNCTION_PAINT_ALL(IN CONST CHAR16 *ParamText)         { MenuStyle(MENU_FUNCTION_PAINT_ALL, ParamText); }
  virtual void call_MENU_FUNCTION_PAINT_SELECTION(IN CONST CHAR16 *ParamText)   { MenuStyle(MENU_FUNCTION_PAINT_SELECTION, ParamText); }
  virtual void call_MENU_FUNCTION_PAINT_TIMEOUT(IN CONST CHAR16 *ParamText)     { MenuStyle(MENU_FUNCTION_PAINT_TIMEOUT, ParamText); }
  virtual void call_MENU_FUNCTION_CLEANUP(IN CONST CHAR16 *ParamText)           { MenuStyle(MENU_FUNCTION_CLEANUP, ParamText); }
};

#endif
/*

 EOF */

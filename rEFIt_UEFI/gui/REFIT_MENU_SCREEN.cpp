/*
 * refit/menu.c
 * Menu functions
 *
 * Copyright (c) 2006 Christoph Pfisterer
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

//#include "Platform.h"
#include "../libeg/libegint.h"   //this includes platform.h
//#include "../include/scroll_images.h"

#include "../../Version.h"
//#include "colors.h"

#include "../libeg/nanosvg.h"
#include "../libeg/FloatLib.h"
#include "HdaCodecDump.h"
#include "REFIT_MENU_SCREEN.h"
//#include "screen.h"
#include "../cpp_foundation/XString.h"
#include "../libeg/XTheme.h"
#include "../libeg/VectorGraphics.h" // for testSVG
#include "shared_with_menu.h"
#include "../refit/menu.h"  // for DrawTextXY. Must disappear soon.
#ifndef DEBUG_ALL
#define DEBUG_MENU 1
#else
#define DEBUG_MENU DEBUG_ALL
#endif

#if DEBUG_MENU == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_MENU, __VA_ARGS__)
#endif

XPointer REFIT_MENU_SCREEN::mPointer;
//
//
////#define PREBOOT_LOG L"EFI\\CLOVER\\misc\\preboot.log"
//
////#define LSTR(s) L##s
//
//// scrolling definitions
static INTN MaxItemOnScreen = -1;


#include "../Platform/Settings.h"
#include "../Platform/StartupSound.h" // for audioIo

//extern REFIT_MENU_ITEM_RETURN MenuEntryReturn;
//extern UINTN            ThemesNum;
//extern CONST CHAR16           *ThemesList[];
//extern UINTN            ConfigsNum;
//extern CHAR16           *ConfigsList[];
//extern UINTN            DsdtsNum;
//extern CHAR16           *DsdtsList[];
//extern UINTN            AudioNum;
//extern HDA_OUTPUTS      AudioList[20];
//extern CONST CHAR8      *AudioOutputNames[];
//extern CHAR8            NonDetected[];
//extern BOOLEAN          GetLegacyLanAddress;
//extern UINT8            gLanMac[4][6]; // their MAC addresses
//extern EFI_AUDIO_IO_PROTOCOL *AudioIo;
//
////layout must be in XTheme
//#if !USE_XTHEME
//INTN LayoutBannerOffset = 64;
//INTN LayoutButtonOffset = 0;
//INTN LayoutTextOffset = 0;
//INTN LayoutMainMenuHeight = 376;
//INTN LayoutAnimMoveForMenuX = 0;
//#endif
//
//BOOLEAN SavePreBootLog = FALSE;

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
//
// other menu definitions

#define MENU_FUNCTION_INIT            (0)
#define MENU_FUNCTION_CLEANUP         (1)
#define MENU_FUNCTION_PAINT_ALL       (2)
#define MENU_FUNCTION_PAINT_SELECTION (3)
#define MENU_FUNCTION_PAINT_TIMEOUT   (4)


//
static CHAR16 ArrowUp[2]   = { ARROW_UP, 0 };
static CHAR16 ArrowDown[2] = { ARROW_DOWN, 0 };
//
BOOLEAN MainAnime = FALSE;
//
////TODO Scroll variables must be a part of REFIT_SCREEN
////BOOLEAN ScrollEnabled = FALSE;
BOOLEAN IsDragging = FALSE;
//#if !USE_XTHEME
//INTN ScrollWidth = 16;
//INTN ScrollButtonsHeight = 20;
//INTN ScrollBarDecorationsHeight = 5;
//INTN ScrollScrollDecorationsHeight = 7;
//#endif
INTN ScrollbarYMovement;
//
//
////#define TextHeight (FONT_CELL_HEIGHT + TEXT_YMARGIN * 2)
//

//
////TODO spacing must be a part of layout in XTheme
#define TITLEICON_SPACING (16)
////#define ROW0__TILESIZE (144)
////#define ROW1_TILESIZE (64)
#define TILE1_XSPACING (8)
////#define TILE_YSPACING (24)
#define ROW0_SCROLLSIZE (100)
//
////EG_IMAGE *SelectionImages[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
////EG_IMAGE *Buttons[4] = {NULL, NULL, NULL, NULL};
//#if !USE_XTHEME
//static EG_IMAGE *TextBuffer = NULL;
//#endif
//
////EG_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff }; //non-trasparent
//
////INTN row0TileSize = 144;
////INTN row1TileSize = 64;
//
static INTN row0Count, row0PosX, row0PosXRunning;
static INTN row1Count, row1PosX, row1PosXRunning;
static INTN *itemPosX = NULL;
static INTN *itemPosY = NULL;
static INTN row0PosY, row1PosY, textPosY, FunctextPosY;
////static EG_IMAGE* MainImage;
static INTN OldX = 0, OldY = 0;
static INTN OldTextWidth = 0;
static UINTN OldRow = 0;
static INTN OldTimeoutTextWidth = 0;
static INTN MenuWidth , TimeoutPosY;
static INTN EntriesPosX, EntriesPosY;
static INTN EntriesWidth, EntriesHeight, EntriesGap;
//#if !USE_XTHEME
//static EG_IMAGE* ScrollbarImage = NULL;
//static EG_IMAGE* ScrollbarBackgroundImage = NULL;
//static EG_IMAGE* UpButtonImage = NULL;
//static EG_IMAGE* DownButtonImage = NULL;
//static EG_IMAGE* BarStartImage = NULL;
//static EG_IMAGE* BarEndImage = NULL;
//static EG_IMAGE* ScrollStartImage = NULL;
//static EG_IMAGE* ScrollEndImage = NULL;
//EG_RECT BarStart;
//EG_RECT BarEnd;
//EG_RECT ScrollStart;
//EG_RECT ScrollEnd;
//EG_RECT ScrollTotal;
//EG_RECT UpButton;
//EG_RECT DownButton;
//EG_RECT ScrollbarBackground;
//EG_RECT Scrollbar;
//EG_RECT ScrollbarOldPointerPlace;
//EG_RECT ScrollbarNewPointerPlace;
//#endif
//
//INPUT_ITEM *InputItems = NULL;
//UINTN  InputItemsCount = 0;
//
//INTN OldChosenTheme;
//INTN OldChosenConfig;
//INTN OldChosenDsdt;
//UINTN OldChosenAudio;
//UINT8 DefaultAudioVolume = 70;
////INTN NewChosenTheme;
//#if !USE_XTHEME
//INTN TextStyle; //why global? It will be class SCREEN member
//#endif
BOOLEAN mGuiReady = FALSE;


//
////REFIT_MENU_ITEM_OPTIONS(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, ACTION AtClick_)
//REFIT_MENU_ITEM_OPTIONS  MenuEntryOptions (L"Options"_XSW,          1, 0, 'O', ActionEnter);
//REFIT_MENU_ITEM_ABOUT    MenuEntryAbout   (L"About Clover"_XSW,     1, 0, 'A', ActionEnter);
//REFIT_MENU_ITEM_RESET    MenuEntryReset   (L"Restart Computer"_XSW, 1, 0, 'R', ActionSelect);
//REFIT_MENU_ITEM_SHUTDOWN MenuEntryShutdown(L"Exit Clover"_XSW,      1, 0, 'U', ActionSelect);
//REFIT_MENU_ITEM_RETURN   MenuEntryReturn  (L"Return"_XSW,           0, 0,  0,  ActionEnter);
//
//
//#if USE_XTHEME
//REFIT_MENU_SCREEN MainMenu(1, L"Main Menu"_XSW, L"Automatic boot"_XSW);
//REFIT_MENU_SCREEN AboutMenu(2, L"About"_XSW, L""_XSW);
//REFIT_MENU_SCREEN HelpMenu(3, L"Help"_XSW, L""_XSW);
//#else
//REFIT_MENU_SCREEN MainMenu(1, L"Main Menu", L"Automatic boot");
//REFIT_MENU_SCREEN AboutMenu(2, L"About", NULL);
//REFIT_MENU_SCREEN HelpMenu(3, L"Help", NULL);
//#endif
//
//
//CONST CHAR16* ArgOptional[NUM_OPT] = {
//  L"arch=i386",       //0
//  L"arch=x86_64",     //1
//  L"-v ",             //2
//  L"-f ",             //3
//  L"-s ",             //4
//  L"-x ",             //5
//  L"nv_disable=1",    //6
//  L"slide=0",         //7
//  L"darkwake=0",      //8
//  L"-xcpm",           //9
//  L"-gux_no_idle",    //10
//  L"-gux_nosleep",    //11
//  L"-gux_nomsi",      //12
//  L"-gux_defer_usb2", //13
//  L"keepsyms=1",      //14
//  L"debug=0x100",     //15
//  L"kextlog=0xffff",  //16
//  L"-alcoff",         //17
//  L"-shikioff",       //18
//  L"nvda_drv=1"       //19
//};

//UINTN RunGenericMenu(IN REFIT_MENU_SCREEN *Screen, IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);



//
//VOID REFIT_MENU_SCREEN::AddMenuInfo(CONST char *Line)
//{
//
////DBG("%s, %s : Line=%s\n", __FILE__, __LINE__, XString(Line).c);
//  REFIT_INFO_DIALOG *InputBootArgs;
//
//  InputBootArgs = new REFIT_INFO_DIALOG;
//  InputBootArgs->Title.takeValueFrom(Line);
//  InputBootArgs->AtClick = ActionLight;
//  AddMenuEntry(InputBootArgs, true);
//}

VOID REFIT_MENU_SCREEN::AddMenuInfo_f(CONST char *format, ...)
{

//DBG("%s, %s : Line=%s\n", __FILE__, __LINE__, XString(Line).c);
  REFIT_INFO_DIALOG *InputBootArgs;

  InputBootArgs = new REFIT_INFO_DIALOG;
  VA_LIST va;
	VA_START(va, format);
  InputBootArgs->Title.vSWPrintf(format, va);
  VA_END(va);
  InputBootArgs->AtClick = ActionLight;
  AddMenuEntry(InputBootArgs, true);
}



//
// Graphics helper functions
//

/*
  SelectionImages:
    [0] SelectionBig
    [2] SelectionSmall
    [4] SelectionIndicator
  Buttons:
    [0] radio_button
    [1] radio_button_selected
    [2] checkbox
    [3] checkbox_checked
*/


//
// Scrolling functions
//
#define CONSTRAIN_MIN(Variable, MinValue) if (Variable < MinValue) Variable = MinValue
#define CONSTRAIN_MAX(Variable, MaxValue) if (Variable > MaxValue) Variable = MaxValue

VOID REFIT_MENU_SCREEN::InitScroll(IN INTN ItemCount, IN UINTN MaxCount,
                       IN UINTN VisibleSpace, IN INTN Selected)
{
  //ItemCount - a number to scroll (Row0)
  //MaxCount - total number (Row0 + Row1)
  //VisibleSpace - a number to fit

  ScrollState.LastSelection = ScrollState.CurrentSelection = Selected;
  //MaxIndex, MaxScroll, MaxVisible are indexes, 0..N-1
  ScrollState.MaxIndex = (INTN)MaxCount - 1;
  ScrollState.MaxScroll = ItemCount - 1;

  if (VisibleSpace == 0) {
    ScrollState.MaxVisible = ScrollState.MaxScroll;
  } else {
    ScrollState.MaxVisible = (INTN)VisibleSpace - 1;
  }

  if (ScrollState.MaxVisible >= ItemCount) {
      ScrollState.MaxVisible = ItemCount - 1;
  }

  ScrollState.MaxFirstVisible = ScrollState.MaxScroll - ScrollState.MaxVisible;
  CONSTRAIN_MIN(ScrollState.MaxFirstVisible, 0);
  ScrollState.FirstVisible = MIN(Selected, ScrollState.MaxFirstVisible);


  ScrollState.IsScrolling = (ScrollState.MaxFirstVisible > 0);
  ScrollState.PaintAll = TRUE;
  ScrollState.PaintSelection = FALSE;

  ScrollState.LastVisible = ScrollState.FirstVisible + ScrollState.MaxVisible;

#if USE_XTHEME
  //scroll bar geometry
  if (!ThemeX.TypeSVG) {
    UpButton.Width      = ThemeX.ScrollWidth; // 16
    UpButton.Height     = ThemeX.ScrollButtonsHeight; // 20
    DownButton.Width    = UpButton.Width;
    DownButton.Height   = ThemeX.ScrollButtonsHeight;
    BarStart.Height     = ThemeX.ScrollBarDecorationsHeight; // 5
    BarEnd.Height       = ThemeX.ScrollBarDecorationsHeight;
    ScrollStart.Height  = ThemeX.ScrollScrollDecorationsHeight; // 7
    ScrollEnd.Height    = ThemeX.ScrollScrollDecorationsHeight;

  } else {
    UpButton.Width      = ThemeX.ScrollWidth; // 16
    UpButton.Height     = 0; // 20
    DownButton.Width    = UpButton.Width;
    DownButton.Height   = 0;
    BarStart.Height     = ThemeX.ScrollBarDecorationsHeight; // 5
    BarEnd.Height       = ThemeX.ScrollBarDecorationsHeight;
    ScrollStart.Height  = 0; // 7
    ScrollEnd.Height    = 0;
  }
#endif

}

VOID REFIT_MENU_SCREEN::UpdateScroll(IN UINTN Movement)
{
  INTN Lines;
  UINTN ScrollMovement = SCROLL_SCROLL_DOWN;
  ScrollState.LastSelection = ScrollState.CurrentSelection;

  switch (Movement) {
    case SCROLL_SCROLLBAR_MOVE:
      ScrollbarYMovement += ScrollbarNewPointerPlace.YPos - ScrollbarOldPointerPlace.YPos;
      ScrollbarOldPointerPlace.XPos = ScrollbarNewPointerPlace.XPos;
      ScrollbarOldPointerPlace.YPos = ScrollbarNewPointerPlace.YPos;
      Lines = ScrollbarYMovement * ScrollState.MaxIndex / ScrollbarBackground.Height;
      ScrollbarYMovement = ScrollbarYMovement - Lines * (ScrollState.MaxVisible * TextHeight - 16 - 1) / ScrollState.MaxIndex;
      if (Lines < 0) {
        Lines = -Lines;
        ScrollMovement = SCROLL_SCROLL_UP;
      }
      for (INTN i = 0; i < Lines; i++)
        UpdateScroll(ScrollMovement);
      break;

    case SCROLL_LINE_UP: //of left = decrement
      if (ScrollState.CurrentSelection > 0) {
        ScrollState.CurrentSelection --;
        if (ScrollState.CurrentSelection < ScrollState.FirstVisible) {
          ScrollState.PaintAll = TRUE;
          ScrollState.FirstVisible = ScrollState.CurrentSelection;
        }
        if (ScrollState.CurrentSelection == ScrollState.MaxScroll) {
          ScrollState.PaintAll = TRUE;
        }
        if ((ScrollState.CurrentSelection < ScrollState.MaxScroll) &&
             (ScrollState.CurrentSelection > ScrollState.LastVisible)) {
          ScrollState.PaintAll = TRUE;
          ScrollState.LastVisible = ScrollState.CurrentSelection;
          ScrollState.FirstVisible = ScrollState.LastVisible - ScrollState.MaxVisible;
        }
      }
      break;

    case SCROLL_LINE_DOWN: //or right -- increment
      if (ScrollState.CurrentSelection < ScrollState.MaxIndex) {
        ScrollState.CurrentSelection++;
        if ((ScrollState.CurrentSelection > ScrollState.LastVisible) &&
            (ScrollState.CurrentSelection <= ScrollState.MaxScroll)){
          ScrollState.PaintAll = TRUE;
          ScrollState.FirstVisible++;
          CONSTRAIN_MAX(ScrollState.FirstVisible, ScrollState.MaxFirstVisible);
        }
        if (ScrollState.CurrentSelection == ScrollState.MaxScroll + 1) {
          ScrollState.PaintAll = TRUE;
        }
      }
      break;

    case SCROLL_SCROLL_DOWN:
      if (ScrollState.FirstVisible < ScrollState.MaxFirstVisible) {
        if (ScrollState.CurrentSelection == ScrollState.FirstVisible)
          ScrollState.CurrentSelection++;
        ScrollState.FirstVisible++;
        ScrollState.LastVisible++;
        ScrollState.PaintAll = TRUE;
      }
      break;

    case SCROLL_SCROLL_UP:
      if (ScrollState.FirstVisible > 0) {
        if (ScrollState.CurrentSelection == ScrollState.LastVisible)
          ScrollState.CurrentSelection--;
        ScrollState.FirstVisible--;
        ScrollState.LastVisible--;
        ScrollState.PaintAll = TRUE;
      }
      break;

    case SCROLL_PAGE_UP:
      if (ScrollState.CurrentSelection > 0) {
        if (ScrollState.CurrentSelection == ScrollState.MaxIndex) {   // currently at last entry, special treatment
          if (ScrollState.IsScrolling)
            ScrollState.CurrentSelection -= ScrollState.MaxVisible - 1;  // move to second line without scrolling
          else
            ScrollState.CurrentSelection = 0;                // move to first entry
        } else {
          if (ScrollState.FirstVisible > 0)
            ScrollState.PaintAll = TRUE;
          ScrollState.CurrentSelection -= ScrollState.MaxVisible;          // move one page and scroll synchronously
          ScrollState.FirstVisible -= ScrollState.MaxVisible;
        }
        CONSTRAIN_MIN(ScrollState.CurrentSelection, 0);
        CONSTRAIN_MIN(ScrollState.FirstVisible, 0);
        if (ScrollState.CurrentSelection < ScrollState.FirstVisible) {
          ScrollState.PaintAll = TRUE;
          ScrollState.FirstVisible = ScrollState.CurrentSelection;
        }
      }
      break;

    case SCROLL_PAGE_DOWN:
      if (ScrollState.CurrentSelection < ScrollState.MaxIndex) {
        if (ScrollState.CurrentSelection == 0) {   // currently at first entry, special treatment
          if (ScrollState.IsScrolling)
            ScrollState.CurrentSelection += ScrollState.MaxVisible - 1;  // move to second-to-last line without scrolling
          else
            ScrollState.CurrentSelection = ScrollState.MaxIndex;         // move to last entry
        } else {
          if (ScrollState.FirstVisible < ScrollState.MaxFirstVisible)
            ScrollState.PaintAll = TRUE;
          ScrollState.CurrentSelection += ScrollState.MaxVisible;          // move one page and scroll synchronously
          ScrollState.FirstVisible += ScrollState.MaxVisible;
        }
        CONSTRAIN_MAX(ScrollState.CurrentSelection, ScrollState.MaxIndex);
        CONSTRAIN_MAX(ScrollState.FirstVisible, ScrollState.MaxFirstVisible);
        if ((ScrollState.CurrentSelection > ScrollState.LastVisible) &&
            (ScrollState.CurrentSelection <= ScrollState.MaxScroll)){
          ScrollState.PaintAll = TRUE;
          ScrollState.FirstVisible+= ScrollState.MaxVisible;
          CONSTRAIN_MAX(ScrollState.FirstVisible, ScrollState.MaxFirstVisible);
        }
      }
      break;

    case SCROLL_FIRST:
      if (ScrollState.CurrentSelection > 0) {
        ScrollState.CurrentSelection = 0;
        if (ScrollState.FirstVisible > 0) {
          ScrollState.PaintAll = TRUE;
          ScrollState.FirstVisible = 0;
        }
      }
      break;

    case SCROLL_LAST:
      if (ScrollState.CurrentSelection < ScrollState.MaxIndex) {
        ScrollState.CurrentSelection = ScrollState.MaxIndex;
        if (ScrollState.FirstVisible < ScrollState.MaxFirstVisible) {
          ScrollState.PaintAll = TRUE;
          ScrollState.FirstVisible = ScrollState.MaxFirstVisible;
        }
      }
      break;

    case SCROLL_NONE:
      // The caller has already updated CurrentSelection, but we may
      // have to scroll to make it visible.
      if (ScrollState.CurrentSelection < ScrollState.FirstVisible) {
        ScrollState.PaintAll = TRUE;
        ScrollState.FirstVisible = ScrollState.CurrentSelection; // - (ScrollState.MaxVisible >> 1);
        CONSTRAIN_MIN(ScrollState.FirstVisible, 0);
      } else if ((ScrollState.CurrentSelection > ScrollState.LastVisible) &&
                 (ScrollState.CurrentSelection <= ScrollState.MaxScroll)) {
        ScrollState.PaintAll = TRUE;
        ScrollState.FirstVisible = ScrollState.CurrentSelection - ScrollState.MaxVisible;
        CONSTRAIN_MAX(ScrollState.FirstVisible, ScrollState.MaxFirstVisible);
      }
      break;

  }

  if (!ScrollState.PaintAll && ScrollState.CurrentSelection != ScrollState.LastSelection)
    ScrollState.PaintSelection = TRUE;
  ScrollState.LastVisible = ScrollState.FirstVisible + ScrollState.MaxVisible;

  //ycr.ru
  if ((ScrollState.PaintAll) && (Movement != SCROLL_NONE))
    HidePointer();
}

VOID REFIT_MENU_SCREEN::HidePointer()
{
  if ( mPointer.isAlive() ) mPointer.Hide();
}

EFI_STATUS REFIT_MENU_SCREEN::MouseBirth()
{

  //if ( !mPointer ) mPointer = new XPointer();
  return mPointer.MouseBirth();
}

VOID REFIT_MENU_SCREEN::KillMouse()
{
  /*if ( mPointer ) */mPointer.KillMouse();
}

VOID REFIT_MENU_SCREEN::AddMenuInfoLine_f(CONST char *format, ...)
{
  XStringW* s = new XStringW();
  VA_LIST va;
	VA_START(va, format);
  s->vSWPrintf(format, va);
  VA_END(va);
  InfoLines.AddReference(s, true);
}

VOID REFIT_MENU_SCREEN::AddMenuEntry(IN REFIT_ABSTRACT_MENU_ENTRY *Entry, bool freeIt)
{
	if ( !Entry ) return;
	Entries.AddReference(Entry, freeIt);
}

// This is supposed to be a destructor ?
VOID REFIT_MENU_SCREEN::FreeMenu()
{
//  INTN i;
  REFIT_ABSTRACT_MENU_ENTRY *Tentry = NULL;
//TODO - here we must FreePool for a list of Entries, Screens, InputBootArgs
  if (Entries.size() > 0) {
    for (UINTN i = 0; i < Entries.size(); i++) {
      Tentry = &Entries[i];
      if (Tentry->SubScreen) {
#if USE_XTHEME
#else
        if (Tentry->SubScreen->Title) {
          FreePool(Tentry->SubScreen->Title);
          Tentry->SubScreen->Title = NULL;
        }
#endif
        // don't free image because of reusing them
     //   FreeMenu(Tentry->SubScreen);
        Tentry->SubScreen->FreeMenu();
        Tentry->SubScreen = NULL;
      }
// Title is a XStringW. It'll be destroyed in REFIT_MENU_SCREEN dtor
//      if (Tentry->getREFIT_MENU_ITEM_RETURN()) { //can't free constants
//        if (Tentry->Title) {
//          FreePool(Tentry->Title);
//          Tentry->Title = NULL;
//        }
//      }
// Tentry is an object inserted in a XArray. It'll deleted at Entries.Empty()
//      FreePool(Tentry);
    }
    Entries.Empty();
//    FreePool(Screen->Entries);
//    Screen->Entries = NULL;
  }
// Infolines will deleted at InfoLines.Empty()
//  if (InfoLines.size() > 0) {
//    for (UINTN i = 0; i < InfoLines.size(); i++) {
//      // TODO: call a user-provided routine for each element here
//      FreePool(InfoLines[i]);
//    }
//    Screen->InfoLines.size() = 0;
//    FreePool(Screen->InfoLines);
//    Screen->InfoLines = NULL;
//  }
    InfoLines.Empty();

}

INTN REFIT_MENU_SCREEN::FindMenuShortcutEntry(IN CHAR16 Shortcut)
{
  if (Shortcut >= 'a' && Shortcut <= 'z')
    Shortcut -= ('a' - 'A');
  if (Shortcut) {
    for (UINTN i = 0; i < Entries.size(); i++) {
      if (Entries[i].ShortcutDigit == Shortcut ||
          Entries[i].ShortcutLetter == Shortcut) {
        return i;
      }
    }
  }
  return -1;
}

//
// generic input menu function
// usr-sse2
//
UINTN REFIT_MENU_SCREEN::InputDialog(IN MENU_STYLE_FUNC  StyleFunc)
{
	if ( !Entries[ScrollState.CurrentSelection].getREFIT_MENU_ITEM_IEM_ABSTRACT() ) {
		DebugLog(2, "BUG : InputDialog called with !Entries[ScrollState.CurrentSelection].REFIT_MENU_ENTRY_ITEM_ABSTRACT()\n");
		return 0; // is it the best thing to do ? CpuDeadLog ?
	}

  EFI_STATUS    Status;
  EFI_INPUT_KEY key;
  UINTN         ind = 0;
  UINTN         i = 0;
  UINTN         MenuExit = 0;
  //UINTN         LogSize;
  UINTN         Pos = (Entries[ScrollState.CurrentSelection]).Row;
  REFIT_MENU_ENTRY_ITEM_ABSTRACT& selectedEntry = *Entries[ScrollState.CurrentSelection].getREFIT_MENU_ITEM_IEM_ABSTRACT();
  INPUT_ITEM    *Item = selectedEntry.Item;
  CHAR16        *Backup = EfiStrDuplicate(Item->SValue);
  UINTN         BackupPos, BackupShift;
  CHAR16        *Buffer;
  //SCROLL_STATE  StateLine;

  /*
    I would like to see a LineSize that depends on the Title width and the menu width so
    the edit dialog does not extend beyond the menu width.
    There are 3 cases:
    1) Text menu where MenuWidth is min of ConWidth - 6 and max of 50 and all StrLen(Title)
    2) Graphics menu where MenuWidth is measured in pixels and font is fixed width.
       The following works well in my case but depends on font width and minimum screen size.
         LineSize = 76 - StrLen(Screen->Entries[State->CurrentSelection].Title);
    3) Graphics menu where font is proportional. In this case LineSize would depend on the
       current width of the displayed string which would need to be recalculated after
       every change.
    Anyway, the above will not be implemented for now, and LineSize will remain at 38
    because it works.
  */
  UINTN         LineSize = 38;
#define DBG_INPUTDIALOG 0
#if DBG_INPUTDIALOG
  UINTN         Iteration = 0;
#endif


  if ((Item->ItemType != BoolValue) &&
      (Item->ItemType != RadioSwitch) &&
      (Item->ItemType != CheckBit)) {
    // Grow Item->SValue to SVALUE_MAX_SIZE if we want to edit a text field
    Item->SValue = (__typeof__(Item->SValue))ReallocatePool(StrSize(Item->SValue), SVALUE_MAX_SIZE, Item->SValue);
  }

  Buffer = Item->SValue;
  BackupShift = Item->LineShift;
  BackupPos = Pos;

  do {

    if (Item->ItemType == BoolValue) {
      Item->BValue = !Item->BValue;
      MenuExit = MENU_EXIT_ENTER;
    } else if (Item->ItemType == RadioSwitch) {
      if (Item->IValue == 3) {
        OldChosenTheme = Pos? Pos - 1: 0xFFFF;
      } else if (Item->IValue == 90) {
        OldChosenConfig = Pos;
      } else if (Item->IValue == 116) {
        OldChosenDsdt = Pos? Pos - 1: 0xFFFF;
      } else if (Item->IValue == 119) {
        OldChosenAudio = Pos;
      }
      MenuExit = MENU_EXIT_ENTER;
    } else if (Item->ItemType == CheckBit) {
      Item->IValue ^= Pos;
      MenuExit = MENU_EXIT_ENTER;
    } else {

      Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);

      if (Status == EFI_NOT_READY) {
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &ind);
        continue;
      }

      switch (key.ScanCode) {
        case SCAN_RIGHT:
          if (Pos + Item->LineShift < StrLen(Buffer)) {
            if (Pos < LineSize)
              Pos++;
            else
              Item->LineShift++;
          }
          break;
        case SCAN_LEFT:
          if (Pos > 0)
            Pos--;
          else if (Item->LineShift > 0)
            Item->LineShift--;
          break;
        case SCAN_HOME:
          Pos = 0;
          Item->LineShift=0;
          break;
        case SCAN_END:
          if (StrLen(Buffer) < LineSize)
            Pos = StrLen(Buffer);
          else {
            Pos = LineSize;
            Item->LineShift = StrLen(Buffer) - LineSize;
          }
          break;
        case SCAN_ESC:
          MenuExit = MENU_EXIT_ESCAPE;
          continue;
          break;
        case SCAN_F2:
          SavePreBootLog = TRUE;
          break;
          //not used here
/*        case SCAN_F6:
          Status = egSaveFile(SelfRootDir, VBIOS_BIN, (UINT8*)(UINTN)0xc0000, 0x20000);
          if (EFI_ERROR(Status)) {
            Status = egSaveFile(NULL, VBIOS_BIN, (UINT8*)(UINTN)0xc0000, 0x20000);
          }
          break; */
        case SCAN_F10:
          egScreenShot();
          break;

        case SCAN_DELETE:
          // forward delete
          if (Pos + Item->LineShift < StrLen(Buffer)) {
            for (i = Pos + Item->LineShift; i < StrLen(Buffer); i++) {
               Buffer[i] = Buffer[i+1];
            }
            /*
            // Commented this out because it looks weird - Forward Delete should not
            // affect anything left of the cursor even if it's just to shift more of the
            // string into view.
            if (Item->LineShift > 0 && Item->LineShift + LineSize > StrLen(Buffer)) {
              Item->LineShift--;
              Pos++;
            }
            */
          }
          break;
      }

      switch (key.UnicodeChar) {
        case CHAR_BACKSPACE:
          if (Buffer[0] != CHAR_NULL && Pos != 0) {
            for (i = Pos + Item->LineShift; i <= StrLen(Buffer); i++) {
               Buffer[i-1] = Buffer[i];
            }
            Item->LineShift > 0 ? Item->LineShift-- : Pos--;
          }

          break;

        case CHAR_LINEFEED:
        case CHAR_CARRIAGE_RETURN:
          MenuExit = MENU_EXIT_ENTER;
          Pos = 0;
          Item->LineShift = 0;
          break;
        default:
          if ((key.UnicodeChar >= 0x20) &&
              (key.UnicodeChar < 0x80)){
            if (StrSize(Buffer) < SVALUE_MAX_SIZE) {
              for (i = StrLen(Buffer)+1; i > Pos + Item->LineShift; i--) {
                 Buffer[i] = Buffer[i-1];
              }
              Buffer[i] = key.UnicodeChar;
              Pos < LineSize ? Pos++ : Item->LineShift++;
            }
          }
          break;
      }
    }
    // Redraw the field
    (Entries[ScrollState.CurrentSelection]).Row = Pos;
    ((*this).*(StyleFunc))(MENU_FUNCTION_PAINT_SELECTION, NULL);
  } while (!MenuExit);

  switch (MenuExit) {
    case MENU_EXIT_ENTER:
      Item->Valid = TRUE;
      ApplyInputs();
      break;

    case MENU_EXIT_ESCAPE:
      if (StrCmp(Item->SValue, Backup) != 0) {
		  snwprintf(Item->SValue, SVALUE_MAX_SIZE, "%ls", Backup);
        if (Item->ItemType != BoolValue) {
          Item->LineShift = BackupShift;
          (Entries[ScrollState.CurrentSelection]).Row = BackupPos;
        }
        ((*this).*(StyleFunc))( MENU_FUNCTION_PAINT_SELECTION, NULL);
      }
      break;
  }
  Item->Valid = FALSE;
  FreePool(Backup);
  if (Item->SValue) {
    MsgLog("EDITED: %ls\n", Item->SValue);
  }
  return 0;
}


UINTN REFIT_MENU_SCREEN::RunGenericMenu(IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry)
{
  EFI_STATUS    Status;
  EFI_INPUT_KEY key;
  //    UINTN         Index;
  INTN          ShortcutEntry;
  BOOLEAN       HaveTimeout = FALSE;
  INTN          TimeoutCountdown = 0;
  UINTN         MenuExit;

  if (ChosenEntry == NULL) {
    TextStyle = 0;
  } else {
    TextStyle = 2;
  }
#if USE_XTHEME
  if (ThemeX.TypeSVG) {
    if (!textFace[TextStyle].valid) {
      if (textFace[0].valid) {
        TextStyle = 0;
      } else if (textFace[2].valid) {
        TextStyle = 2;
      } else if (textFace[1].valid) {
        TextStyle = 1;
      } else {
        DBG("no valid text style\n");
        textFace[TextStyle].size = TextHeight - 4;
      }
    }
    if (textFace[TextStyle].valid) {
      // TextHeight = (int)((textFace[TextStyle].size + 4) * GlobalConfig.Scale);
      //clovy - row height / text size factor
      TextHeight = (int)((textFace[TextStyle].size * RowHeightFromTextHeight) * ThemeX.Scale);
    }
  }
#else
  if (GlobalConfig.TypeSVG) {
    if (!textFace[TextStyle].valid) {
      if (textFace[0].valid) {
        TextStyle = 0;
      } else if (textFace[2].valid) {
        TextStyle = 2;
      } else if (textFace[1].valid) {
        TextStyle = 1;
      } else {
        DBG("no valid text style\n");
        textFace[TextStyle].size = TextHeight - 4;
      }
    }
    if (textFace[TextStyle].valid) {
      // TextHeight = (int)((textFace[TextStyle].size + 4) * GlobalConfig.Scale);
      //clovy - row height / text size factor
      TextHeight = (int)((textFace[TextStyle].size * RowHeightFromTextHeight) * GlobalConfig.Scale);
    }
  }
#endif

  //no default - no timeout!
  if ((*DefaultEntryIndex != -1) && (TimeoutSeconds > 0)) {
    //      DBG("have timeout\n");
    HaveTimeout = TRUE;
    TimeoutCountdown =  TimeoutSeconds;
  }
  MenuExit = 0;

  ((*this).*(StyleFunc))(MENU_FUNCTION_INIT, NULL);
  //  DBG("scroll inited\n");
  // override the starting selection with the default index, if any
  if (*DefaultEntryIndex >= 0 && *DefaultEntryIndex <=  ScrollState.MaxIndex) {
     ScrollState.CurrentSelection = *DefaultEntryIndex;
     UpdateScroll(SCROLL_NONE);
  }
  //  DBG("RunGenericMenu CurrentSelection=%d MenuExit=%d\n",
  //      State.CurrentSelection, MenuExit);

  // exhaust key buffer and be sure no key is pressed to prevent option selection
  // when coming with a key press from timeout=0, for example
  while (ReadAllKeyStrokes()) gBS->Stall(500 * 1000);
  while (!MenuExit) {
    // update the screen
    if (ScrollState.PaintAll) {
      ((*this).*(StyleFunc))(MENU_FUNCTION_PAINT_ALL, NULL);
      ScrollState.PaintAll = FALSE;
    } else if (ScrollState.PaintSelection) {
      ((*this).*(StyleFunc))(MENU_FUNCTION_PAINT_SELECTION, NULL);
      ScrollState.PaintSelection = FALSE;
    }

    if (HaveTimeout) {
#if USE_XTHEME
      //TimeoutMessage = PoolPrint(L"%s in %d seconds", TimeoutText.data(), TimeoutCountdown);
      XStringW TOMessage = SWPrintf("%ls in %lld seconds", TimeoutText.wc_str(), TimeoutCountdown);
      ((*this).*(StyleFunc))(MENU_FUNCTION_PAINT_TIMEOUT, TOMessage.data());
//      FreePool(TimeoutMessage);
#else
      CHAR16        *TimeoutMessage = PoolPrint(L"%s in %d seconds", TimeoutText, TimeoutCountdown);
      ((*this).*(StyleFunc))(MENU_FUNCTION_PAINT_TIMEOUT, TimeoutMessage);
      FreePool(TimeoutMessage);
#endif
    }

    if (gEvent) { //for now used at CD eject.
      MenuExit = MENU_EXIT_ESCAPE;
      ScrollState.PaintAll = TRUE;
      gEvent = 0; //to prevent looping
      break;
    }
    key.UnicodeChar = 0;
    key.ScanCode = 0;
    if (!mGuiReady) {
      mGuiReady = TRUE;
      DBG("GUI ready\n");
    }
    Status = WaitForInputEventPoll(this, 1); //wait for 1 seconds.
    if (Status == EFI_TIMEOUT) {
      if (HaveTimeout) {
        if (TimeoutCountdown <= 0) {
          // timeout expired
          MenuExit = MENU_EXIT_TIMEOUT;
          break;
        } else {
          //     gBS->Stall(100000);
          TimeoutCountdown--;
        }
      }
      continue;
    }

    switch (mAction) {
      case ActionSelect:
        ScrollState.LastSelection = ScrollState.CurrentSelection;
        ScrollState.CurrentSelection = mItemID;
        ScrollState.PaintAll = TRUE;
        HidePointer();
        break;
      case ActionEnter:
        ScrollState.LastSelection = ScrollState.CurrentSelection;
        ScrollState.CurrentSelection = mItemID;
        if ( Entries[mItemID].getREFIT_INPUT_DIALOG() ||  Entries[mItemID].getREFIT_MENU_CHECKBIT() ) {
          MenuExit = InputDialog(StyleFunc);
        } else if (Entries[mItemID].getREFIT_MENU_SWITCH()) {
          MenuExit = InputDialog(StyleFunc);
          ScrollState.PaintAll = TRUE;
          HidePointer();
        } else if (!Entries[mItemID].getREFIT_INFO_DIALOG()) {
          MenuExit = MENU_EXIT_ENTER;
        }
        break;
      case ActionHelp:
        MenuExit = MENU_EXIT_HELP;
        break;
      case ActionOptions:
        ScrollState.LastSelection = ScrollState.CurrentSelection;
        ScrollState.CurrentSelection = mItemID;
        MenuExit = MENU_EXIT_OPTIONS;
        break;
      case ActionDetails:
        ScrollState.LastSelection = ScrollState.CurrentSelection;
        // Index = State.CurrentSelection;
        ScrollState.CurrentSelection = mItemID;
        if ((Entries[mItemID].getREFIT_INPUT_DIALOG()) ||
            (Entries[mItemID].getREFIT_MENU_CHECKBIT())) {
          MenuExit = InputDialog(StyleFunc);
        } else if (Entries[mItemID].getREFIT_MENU_SWITCH()) {
          MenuExit = InputDialog(StyleFunc);
          ScrollState.PaintAll = TRUE;
          HidePointer();
        } else if (!Entries[mItemID].getREFIT_INFO_DIALOG()) {
          MenuExit = MENU_EXIT_DETAILS;
        }
        break;
      case ActionDeselect:
        ScrollState.LastSelection = ScrollState.CurrentSelection;
        ScrollState.PaintAll = TRUE;
        HidePointer();
        break;
      case ActionFinish:
        MenuExit = MENU_EXIT_ESCAPE;
        break;
      case ActionScrollDown:
        UpdateScroll(SCROLL_SCROLL_DOWN);
        break;
      case ActionScrollUp:
        UpdateScroll(SCROLL_SCROLL_UP);
        break;
      case ActionMoveScrollbar:
        UpdateScroll(SCROLL_SCROLLBAR_MOVE);
        break;
      default:
        break;
    }

    // read key press (and wait for it if applicable)
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
    if ((Status == EFI_NOT_READY) && (mAction == ActionNone)) {
      continue;
    }
    if (mAction == ActionNone) {
      ReadAllKeyStrokes(); //clean to avoid doubles
    }
    if (HaveTimeout) {
      // the user pressed a key, cancel the timeout
      ((*this).*(StyleFunc))(MENU_FUNCTION_PAINT_TIMEOUT, L"");
      HidePointer(); //ycr.ru
      HaveTimeout = FALSE;
    }

    mAction = ActionNone; //do action once
    // react to key press
    switch (key.ScanCode) {
      case SCAN_UP:
      case SCAN_LEFT:
        UpdateScroll(SCROLL_LINE_UP);
        break;
      case SCAN_DOWN:
      case SCAN_RIGHT:
        UpdateScroll(SCROLL_LINE_DOWN);
        break;
      case SCAN_HOME:
        UpdateScroll(SCROLL_FIRST);
        break;
      case SCAN_END:
        UpdateScroll(SCROLL_LAST);
        break;
      case SCAN_PAGE_UP:
        UpdateScroll(SCROLL_PAGE_UP);
    //    SetNextScreenMode(1);
        ((*this).*(StyleFunc))(MENU_FUNCTION_INIT, NULL);
        break;
      case SCAN_PAGE_DOWN:
        UpdateScroll(SCROLL_PAGE_DOWN);
     //   SetNextScreenMode(-1);
        ((*this).*(StyleFunc))(MENU_FUNCTION_INIT, NULL);
        break;
      case SCAN_ESC:
        MenuExit = MENU_EXIT_ESCAPE;
        break;
      case SCAN_INSERT:
        MenuExit = MENU_EXIT_OPTIONS;
        break;

      case SCAN_F1:
        MenuExit = MENU_EXIT_HELP;
        break;
      case SCAN_F2:
        SavePreBootLog = TRUE;
        //let it be twice
        Status = SaveBooterLog(SelfRootDir, PREBOOT_LOG);
        if (EFI_ERROR(Status)) {
          Status = SaveBooterLog(NULL, PREBOOT_LOG);
        }
        break;
      case SCAN_F3:
         MenuExit = MENU_EXIT_HIDE_TOGGLE;
         break;
      case SCAN_F4:
        SaveOemTables();
        break;
      case SCAN_F5:
        SaveOemDsdt(TRUE); //full patch
        break;
      case SCAN_F6:
        Status = egSaveFile(SelfRootDir, VBIOS_BIN, (UINT8*)(UINTN)0xc0000, 0x20000);
        if (EFI_ERROR(Status)) {
          Status = egSaveFile(NULL, VBIOS_BIN, (UINT8*)(UINTN)0xc0000, 0x20000);
        }
        break;
/* just a sample code
      case SCAN_F7:
        Status = egMkDir(SelfRootDir,  L"EFI\\CLOVER\\new_folder");
        DBG("create folder %s\n", strerror(Status));
        if (!EFI_ERROR(Status)) {
          Status = egSaveFile(SelfRootDir,  L"EFI\\CLOVER\\new_folder\\new_file.txt", (UINT8*)SomeText, sizeof(*SomeText)+1);
          DBG("create file %s\n", strerror(Status));
        }
        break;
*/
      case SCAN_F7:
        if (OldChosenAudio > AudioNum) {
              OldChosenAudio = 0; //security correction
        }
        Status = gBS->HandleProtocol(AudioList[OldChosenAudio].Handle, &gEfiAudioIoProtocolGuid, (VOID**)&AudioIo);
			DBG("open %llu audio handle status=%s\n", OldChosenAudio, strerror(Status));
        if (!EFI_ERROR(Status)) {
          StartupSoundPlay(SelfRootDir, NULL); //play embedded sound
        }
        break;
      case SCAN_F8:
        testSVG();
        SaveHdaDumpBin();
        SaveHdaDumpTxt();
        break;

      case SCAN_F9:
        SetNextScreenMode(1);
        InitTheme(FALSE, NULL);
        break;
      case SCAN_F10:
        egScreenShot();
        break;
      case SCAN_F11:
        ResetNvram ();
        break;
      case SCAN_F12:
        MenuExit = MENU_EXIT_EJECT;
        ScrollState.PaintAll = TRUE;
        break;

    }
    switch (key.UnicodeChar) {
      case CHAR_LINEFEED:
      case CHAR_CARRIAGE_RETURN:
        if ((Entries[ScrollState.CurrentSelection].getREFIT_INPUT_DIALOG()) ||
            (Entries[ScrollState.CurrentSelection].getREFIT_MENU_CHECKBIT())) {
          MenuExit = InputDialog(StyleFunc);
        } else if (Entries[ScrollState.CurrentSelection].getREFIT_MENU_SWITCH()){
          MenuExit = InputDialog(StyleFunc);
          ScrollState.PaintAll = TRUE;
        } else if (Entries[ScrollState.CurrentSelection].getREFIT_MENU_ENTRY_CLOVER()){
          MenuExit = MENU_EXIT_DETAILS;
        } else if (!Entries[ScrollState.CurrentSelection].getREFIT_INFO_DIALOG()) {
          MenuExit = MENU_EXIT_ENTER;
        }
        break;
      case ' ': //CHAR_SPACE
        if ((Entries[ScrollState.CurrentSelection].getREFIT_INPUT_DIALOG()) ||
            (Entries[ScrollState.CurrentSelection].getREFIT_MENU_CHECKBIT())) {
          MenuExit = InputDialog(StyleFunc);
        } else if (Entries[ScrollState.CurrentSelection].getREFIT_MENU_SWITCH()){
          MenuExit = InputDialog(StyleFunc);
          ScrollState.PaintAll = TRUE;
          HidePointer();
        } else if (!Entries[ScrollState.CurrentSelection].getREFIT_INFO_DIALOG()) {
          MenuExit = MENU_EXIT_DETAILS;
        }
        break;

      default:
        ShortcutEntry = FindMenuShortcutEntry(key.UnicodeChar);
        if (ShortcutEntry >= 0) {
          ScrollState.CurrentSelection = ShortcutEntry;
          MenuExit = MENU_EXIT_ENTER;
        }
        break;
    }
  }

  ((*this).*(StyleFunc))(MENU_FUNCTION_CLEANUP, NULL);

	if (ChosenEntry) {
    *ChosenEntry = &Entries[ScrollState.CurrentSelection];
	}

  *DefaultEntryIndex = ScrollState.CurrentSelection;

  return MenuExit;
}

/**
 * Text Mode menu.
 */
VOID REFIT_MENU_SCREEN::TextMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
  INTN i = 0, j = 0;
  static UINTN TextMenuWidth = 0,ItemWidth = 0, MenuHeight = 0;
  static UINTN MenuPosY = 0;
  //static CHAR16 **DisplayStrings;
  CHAR16 *TimeoutMessage;
  CHAR16 ResultString[TITLE_MAX_LEN]; // assume a title max length of around 128
	UINTN OldChosenItem = ~(UINTN)0;

  switch (Function) {

    case MENU_FUNCTION_INIT:
      // vertical layout
      MenuPosY = 4;

	  if (InfoLines.size() > 0) {
        MenuPosY += InfoLines.size() + 1;
	  }

      MenuHeight = ConHeight - MenuPosY;

	  if (TimeoutSeconds > 0) {
        MenuHeight -= 2;
	  }

      InitScroll(Entries.size(), Entries.size(), MenuHeight, 0);

      // determine width of the menu
      TextMenuWidth = 50;  // minimum
      for (i = 0; i <= ScrollState.MaxIndex; i++) {
        ItemWidth = StrLen(Entries[i].Title);

		if (TextMenuWidth < ItemWidth) {
          TextMenuWidth = ItemWidth;
        }
      }

	  if (TextMenuWidth > ConWidth - 6) {
        TextMenuWidth = ConWidth - 6;
	  }

    if (Entries[0].getREFIT_MENU_SWITCH() && Entries[0].getREFIT_MENU_SWITCH()->Item->IValue == 90) {
      j = OldChosenConfig;
    } else if (Entries[0].getREFIT_MENU_SWITCH() && Entries[0].getREFIT_MENU_SWITCH()->Item->IValue == 116) {
      j = OldChosenDsdt;
    } else if (Entries[0].getREFIT_MENU_SWITCH() && Entries[0].getREFIT_MENU_SWITCH()->Item->IValue == 119) {
      j = OldChosenAudio;
    }

    break;

    case MENU_FUNCTION_CLEANUP:
      // release temporary memory

			// reset default output colours
	  gST->ConOut->SetAttribute(gST->ConOut, ATTR_BANNER);

      break;

    case MENU_FUNCTION_PAINT_ALL:
        // paint the whole screen (initially and after scrolling)
      gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_BASIC);
      for (i = 0; i < (INTN)ConHeight - 4; i++) {
        gST->ConOut->SetCursorPosition(gST->ConOut, 0, 4 + i);
        gST->ConOut->OutputString(gST->ConOut, BlankLine);
      }

        BeginTextScreen(Title);

        if (InfoLines.size() > 0) {
          gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);

          for (i = 0; i < (INTN)InfoLines.size(); i++) {
            gST->ConOut->SetCursorPosition (gST->ConOut, 3, 4 + i);
            gST->ConOut->OutputString (gST->ConOut, InfoLines[i].data());
          }
        }

        for (i = ScrollState.FirstVisible; i <= ScrollState.LastVisible && i <= ScrollState.MaxIndex; i++) {
      gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (i - ScrollState.FirstVisible));

      if (i == ScrollState.CurrentSelection) {
        gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_CURRENT);
      } else {
        gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_BASIC);
      }

      StrCpyS(ResultString, TITLE_MAX_LEN, Entries[i].Title);

      if (Entries[i].getREFIT_INPUT_DIALOG()) {
        REFIT_INPUT_DIALOG& entry = (REFIT_INPUT_DIALOG&) Entries[i];
        if (entry.getREFIT_INPUT_DIALOG()) {
          if ((entry).Item->ItemType == BoolValue) {
            StrCatS(ResultString, TITLE_MAX_LEN, (entry).Item->BValue ? L": [+]" : L": [ ]");
          } else {
            StrCatS(ResultString, TITLE_MAX_LEN, (entry).Item->SValue);
          }
        } else if (entry.getREFIT_MENU_CHECKBIT()) {
          // check boxes
          StrCatS(ResultString, TITLE_MAX_LEN, ((entry).Item->IValue & (entry.Row)) ? L": [+]" : L": [ ]");
        } else if (entry.getREFIT_MENU_SWITCH()) {
          // radio buttons

          // update chosen config
          if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 90) {
            OldChosenItem = OldChosenConfig;
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 116) {
            OldChosenItem = OldChosenDsdt;
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 119) {
            OldChosenItem = OldChosenAudio;
          }

          StrCatS(ResultString, TITLE_MAX_LEN, (entry.Row == OldChosenItem) ? L": (*)" : L": ( )");
        }
      }

      for (j = StrLen(ResultString); j < (INTN)TextMenuWidth; j++) {
        ResultString[j] = L' ';
      }

      ResultString[j] = 0;
        gST->ConOut->OutputString(gST->ConOut, ResultString);
      }

      // scrolling indicators
      gST->ConOut->SetAttribute (gST->ConOut, ATTR_SCROLLARROW);
      gST->ConOut->SetCursorPosition (gST->ConOut, 0, MenuPosY);

      if (ScrollState.FirstVisible > 0) {
          gST->ConOut->OutputString (gST->ConOut, ArrowUp);
      } else {
          gST->ConOut->OutputString (gST->ConOut, L" ");
      }

      gST->ConOut->SetCursorPosition (gST->ConOut, 0, MenuPosY + ScrollState.MaxVisible);

      if (ScrollState.LastVisible < ScrollState.MaxIndex) {
          gST->ConOut->OutputString (gST->ConOut, ArrowDown);
      } else {
          gST->ConOut->OutputString (gST->ConOut, L" ");
      }

      break;

    case MENU_FUNCTION_PAINT_SELECTION:
			// last selection
      // redraw selection cursor
      gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (ScrollState.LastSelection - ScrollState.FirstVisible));
      gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_BASIC);
      //gST->ConOut->OutputString (gST->ConOut, DisplayStrings[ScrollState.LastSelection]);
			StrCpyS(ResultString, TITLE_MAX_LEN, Entries[ScrollState.LastSelection].Title);
      if (Entries[ScrollState.LastSelection].getREFIT_INPUT_DIALOG()) {
        REFIT_INPUT_DIALOG& entry = (REFIT_INPUT_DIALOG&) Entries[ScrollState.LastSelection];
        if (entry.getREFIT_INPUT_DIALOG()) {
          if (entry.Item->ItemType == BoolValue) {
            StrCatS(ResultString, TITLE_MAX_LEN, entry.Item->BValue ? L": [+]" : L": [ ]");
          } else {
            StrCatS(ResultString, TITLE_MAX_LEN, entry.Item->SValue);
          }
        } else if (entry.getREFIT_MENU_CHECKBIT()) {
          // check boxes
          StrCatS(ResultString, TITLE_MAX_LEN, (entry.Item->IValue & (entry.Row)) ? L": [+]" : L": [ ]");
        } else if (entry.getREFIT_MENU_SWITCH()) {
          // radio buttons

          if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 90) {
            OldChosenItem = OldChosenConfig;
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 116) {
            OldChosenItem = OldChosenDsdt;
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 119) {
            OldChosenItem = OldChosenAudio;
          }

          StrCatS(ResultString, TITLE_MAX_LEN, (entry.Row == OldChosenItem) ? L": (*)" : L": ( )");
        }
      }

      for (j = StrLen(ResultString); j < (INTN) TextMenuWidth; j++) {
        ResultString[j] = L' ';
      }

      ResultString[j] = 0;
      gST->ConOut->OutputString (gST->ConOut, ResultString);

        // current selection
      gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (ScrollState.CurrentSelection - ScrollState.FirstVisible));
      gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_CURRENT);
      StrCpyS(ResultString, TITLE_MAX_LEN, Entries[ScrollState.CurrentSelection].Title);
      if (Entries[ScrollState.CurrentSelection].getREFIT_INPUT_DIALOG()) {
        REFIT_INPUT_DIALOG& entry = (REFIT_INPUT_DIALOG&) Entries[ScrollState.CurrentSelection];
        if (entry.getREFIT_INPUT_DIALOG()) {
          if (entry.Item->ItemType == BoolValue) {
            StrCatS(ResultString, TITLE_MAX_LEN, entry.Item->BValue ? L": [+]" : L": [ ]");
          } else {
            StrCatS(ResultString, TITLE_MAX_LEN, entry.Item->SValue);
          }
        } else if (entry.getREFIT_MENU_CHECKBIT()) {
          // check boxes
          StrCatS(ResultString, TITLE_MAX_LEN, (entry.Item->IValue & (entry.Row)) ? L": [+]" : L": [ ]");
        } else if (entry.getREFIT_MENU_SWITCH()) {
          // radio buttons

          if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 90) {
            OldChosenItem = OldChosenConfig;
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 116) {
            OldChosenItem = OldChosenDsdt;
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 119) {
            OldChosenItem = OldChosenAudio;
          }

          StrCatS(ResultString, TITLE_MAX_LEN, (entry.Row == OldChosenItem) ? L": (*)" : L": ( )");
        }
      }

      for (j = StrLen(ResultString); j < (INTN) TextMenuWidth; j++) {
        ResultString[j] = L' ';
      }

      ResultString[j] = 0;
      gST->ConOut->OutputString (gST->ConOut, ResultString);
        //gST->ConOut->OutputString (gST->ConOut, DisplayStrings[ScrollState.CurrentSelection]);

      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      if (ParamText[0] == 0) {
        // clear message
        gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
        gST->ConOut->SetCursorPosition (gST->ConOut, 0, ConHeight - 1);
        gST->ConOut->OutputString (gST->ConOut, BlankLine + 1);
      } else {
        // paint or update message
        gST->ConOut->SetAttribute (gST->ConOut, ATTR_ERROR);
        gST->ConOut->SetCursorPosition (gST->ConOut, 3, ConHeight - 1);
        TimeoutMessage = PoolPrint(L"%s  ", ParamText);
        gST->ConOut->OutputString (gST->ConOut, TimeoutMessage);
        FreePool(TimeoutMessage);
      }

      break;
  }
}

/**
 * Draw text with specific coordinates.
 */


#if USE_XTHEME
INTN REFIT_MENU_SCREEN::DrawTextXY(IN const XStringW& Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign)
{
  INTN      TextWidth = 0;
  INTN      XText = 0;
  INTN      Height;
  INTN      TextXYStyle = 1;
//  EG_IMAGE  *TextBufferXY = NULL;
  XImage TextBufferXY(0,0);

  if (Text.isEmpty()) {
    return 0;
  }
  //TODO assume using embedded font for BootScreen
  //messages must be TextXYStyle = 1 if it is provided by theme
  if (!textFace[1].valid) {
    if (textFace[2].valid) {
      TextXYStyle = 2;
    } else {
      TextXYStyle = 0;
    }
  }
/*
 * here we want to know how many place is needed for the graphical text
 * the procedure worked for fixed width font but the problem appears with proportional fonts
 * as well we not know yet the font using but egRenderText calculate later real width
 * so make a place to be large enoungh
 */
  egMeasureText(Text.data(), &TextWidth, NULL);

  if (XAlign == X_IS_LEFT) {
    TextWidth = UGAWidth - XPos - 1;
    XText = XPos;
  }

  if (!isBootScreen && ThemeX.TypeSVG) {
    TextWidth += TextHeight * 2; //give more place for buffer
    if (!textFace[TextXYStyle].valid) {
      DBG("no vaid text face for message!\n");
      Height = TextHeight;
    } else {
      Height = (int)(textFace[TextXYStyle].size * RowHeightFromTextHeight * ThemeX.Scale);
    }
  } else {
    Height = TextHeight;
  }

//  TextBufferXY = egCreateFilledImage(TextWidth, Height, TRUE, &MenuBackgroundPixel);
  TextBufferXY.setSizeInPixels(TextWidth, Height);
//  TextBufferXY.Fill(MenuBackgroundPixel);

  // render the text
  INTN TextWidth2 = egRenderText(Text, &TextBufferXY, 0, 0, 0xFFFF, TextXYStyle);
  // there is real text width but we already have an array with Width = TextWidth
  //
//  TextBufferXY.EnsureImageSize(TextWidth2, Height); //assume color = MenuBackgroundPixel

  if (XAlign != X_IS_LEFT) {
    // shift 64 is prohibited
    XText = XPos - (TextWidth2 >> XAlign);  //X_IS_CENTER = 1
  }

  OldTextBufferRect.XPos = XText;
  OldTextBufferRect.YPos = YPos;
  OldTextBufferRect.Width = TextWidth2;
  OldTextBufferRect.Height = Height;

  OldTextBufferImage.GetArea(OldTextBufferRect);
  //GetArea may change sizes
  OldTextBufferRect.Width = OldTextBufferImage.GetWidth();
  OldTextBufferRect.Height = OldTextBufferImage.GetHeight();

  //  DBG("draw text %ls\n", Text);
  //  DBG("pos=%d width=%d xtext=%d Height=%d Y=%d\n", XPos, TextWidth, XText, Height, YPos);
 // TextBufferXY.Draw(XText, YPos, 0, false);
//  TextBufferXY.DrawWithoutCompose(XText, YPos);
  TextBufferXY.DrawOnBack(XText, YPos, ThemeX.Background);
  return TextWidth2;
}

void REFIT_MENU_SCREEN::EraseTextXY()
{
  OldTextBufferImage.Draw(OldTextBufferRect.XPos, OldTextBufferRect.YPos);
}
#else


#endif
/**
 * Helper function to draw text for Boot Camp Style.
 * @author: Needy
 */
#if USE_XTHEME
VOID REFIT_MENU_SCREEN::DrawBCSText(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign)
{
  // check if text was provided
  if (!Text) {
    return;
  }

  INTN TextLen = StrLen(Text);

  // number of chars to be drawn on the screen
  INTN MaxTextLen = 13;
  INTN EllipsisLen = 2;
  
  CHAR16 *BCSText = NULL;

  // more space, more characters
  if (ThemeX.TileXSpace >= 25 && ThemeX.TileXSpace < 30) {
    MaxTextLen = 14;
  } else if (ThemeX.TileXSpace >= 30 && ThemeX.TileXSpace < 35) {
    MaxTextLen = 15;
  } else if (ThemeX.TileXSpace >= 35 && ThemeX.TileXSpace < 40) {
    MaxTextLen = 16;
  } else if (ThemeX.TileXSpace >= 40 && ThemeX.TileXSpace < 45) {
    MaxTextLen = 17;
  } else if (ThemeX.TileXSpace >= 45 && ThemeX.TileXSpace < 50) {
    MaxTextLen = 18;
  } else if (ThemeX.TileXSpace >= 50) {
    MaxTextLen = 19;
  }

  MaxTextLen += EllipsisLen;

  // if the text exceeds the given limit
  if (TextLen > MaxTextLen) {
    BCSText = (__typeof__(BCSText))AllocatePool((sizeof(CHAR16) * MaxTextLen) + 1);

    // error check, not enough memory
    if (!BCSText) {
      return;
    }

    // copy the permited amound of chars minus the ellipsis
    StrnCpyS(BCSText, (MaxTextLen - EllipsisLen) + 1, Text, MaxTextLen - EllipsisLen);
    BCSText[MaxTextLen - EllipsisLen] = '\0';

    // add ellipsis
    StrnCatS(BCSText, MaxTextLen + 1, L"..", EllipsisLen);
    // redundant, used for safety measures
    BCSText[MaxTextLen] = '\0';
    XStringW BCSTextX;
    BCSTextX.takeValueFrom(BCSText);
    DrawTextXY(BCSTextX, XPos, YPos, XAlign);

    FreePool(BCSText);
  } else {
		// draw full text
    XStringW TextX;
    TextX.takeValueFrom(Text);
    DrawTextXY(TextX, XPos, YPos, XAlign);
  }
}
#else
//remains in menu.cpp
#endif

#if USE_XTHEME
VOID REFIT_MENU_SCREEN::DrawMenuText(IN XStringW& Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor)
{
  XImage TextBufferX(UGAWidth-XPos, TextHeight);
  XImage SelectionBar(UGAWidth-XPos, TextHeight);
/*
  if (Cursor == 0xFFFF) { //InfoLine = 0xFFFF
    TextBufferX.Fill(MenuBackgroundPixel);
  } else {
    TextBufferX.Fill(InputBackgroundPixel);
  }
*/

  if (SelectedWidth > 0) {
    // fill selection bar background
    EG_RECT TextRect;
    TextRect.Width = SelectedWidth;
    TextRect.Height = TextHeight;
    TextBufferX.FillArea(SelectionBackgroundPixel, TextRect);
//    SelectionBar.Fill(SelectionBackgroundPixel);
  }
  SelectionBar.CopyRect(ThemeX.Background, XPos, YPos);
//  SelectionBar.DrawWithoutCompose(XPos, YPos);
//  TextBufferX.Compose(0, 0, ThemeX.Background, true);
  // render the text
  if (ThemeX.TypeSVG) {
    //clovy - text vertically centred on Height
    egRenderText(Text, &TextBufferX, 0,
                 (INTN)((TextHeight - (textFace[TextStyle].size * ThemeX.Scale)) / 2),
                 Cursor, TextStyle);
  } else {
    egRenderText(Text, &TextBufferX, TEXT_XMARGIN, TEXT_YMARGIN, Cursor, TextStyle);
  }
  SelectionBar.Compose(0, 0, TextBufferX, true);
//  TextBufferX.DrawWithoutCompose(XPos, YPos);
  SelectionBar.DrawWithoutCompose(XPos, YPos);
}

#else

#endif

VOID REFIT_MENU_SCREEN::SetBar(INTN PosX, INTN UpPosY, INTN DownPosY, IN SCROLL_STATE *State)
{
//  DBG("SetBar <= %d %d %d %d %d\n", UpPosY, DownPosY, State->MaxVisible, State->MaxIndex, State->FirstVisible);
//SetBar <= 302 722 19 31 0
  UpButton.XPos = PosX;
  UpButton.YPos = UpPosY;

  DownButton.XPos = UpButton.XPos;
  DownButton.YPos = DownPosY;

  ScrollbarBackground.XPos = UpButton.XPos;
  ScrollbarBackground.YPos = UpButton.YPos + UpButton.Height;
  ScrollbarBackground.Width = UpButton.Width;
  ScrollbarBackground.Height = DownButton.YPos - (UpButton.YPos + UpButton.Height);

  BarStart.XPos = ScrollbarBackground.XPos;
  BarStart.YPos = ScrollbarBackground.YPos;
  BarStart.Width = ScrollbarBackground.Width;

  BarEnd.Width = ScrollbarBackground.Width;
  BarEnd.XPos = ScrollbarBackground.XPos;
  BarEnd.YPos = DownButton.YPos - BarEnd.Height;

  ScrollStart.XPos = ScrollbarBackground.XPos;
  ScrollStart.YPos = ScrollbarBackground.YPos + ScrollbarBackground.Height * State->FirstVisible / (State->MaxIndex + 1);
  ScrollStart.Width = ScrollbarBackground.Width;

  Scrollbar.XPos = ScrollbarBackground.XPos;
  Scrollbar.YPos = ScrollStart.YPos + ScrollStart.Height;
  Scrollbar.Width = ScrollbarBackground.Width;
  Scrollbar.Height = ScrollbarBackground.Height * (State->MaxVisible + 1) / (State->MaxIndex + 1) - ScrollStart.Height;

  ScrollEnd.Width = ScrollbarBackground.Width;
  ScrollEnd.XPos = ScrollbarBackground.XPos;
  ScrollEnd.YPos = Scrollbar.YPos + Scrollbar.Height - ScrollEnd.Height;

  Scrollbar.Height -= ScrollEnd.Height;

  ScrollTotal.XPos = UpButton.XPos;
  ScrollTotal.YPos = UpButton.YPos;
  ScrollTotal.Width = UpButton.Width;
  ScrollTotal.Height = DownButton.YPos + DownButton.Height - UpButton.YPos;
//  DBG("ScrollTotal.Height = %d\n", ScrollTotal.Height);  //ScrollTotal.Height = 420
}
#if USE_XTHEME
VOID REFIT_MENU_SCREEN::ScrollingBar()
{
  ScrollEnabled = (ScrollState.MaxFirstVisible != 0);
  if (!ScrollEnabled) {
    return;
  }
#if 1 //use compose instead of Draw
  //this is a copy of old algorithm
  // but we can not use Total and Draw all parts separately assumed they composed on background
  // it is #else

  XImage Total(ScrollTotal.Width, ScrollTotal.Height);
//  Total.Fill(&MenuBackgroundPixel);
  Total.CopyRect(ThemeX.Background, ScrollTotal.XPos, ScrollTotal.YPos);
  if (!ThemeX.ScrollbarBackgroundImage.isEmpty()) {
    for (INTN i = 0; i < ScrollbarBackground.Height; i+=ThemeX.ScrollbarBackgroundImage.GetHeight()) {
      Total.Compose(ScrollbarBackground.XPos - ScrollTotal.XPos, ScrollbarBackground.YPos + i - ScrollTotal.YPos, ThemeX.ScrollbarBackgroundImage, FALSE);
    }
  }
  Total.Compose(BarStart.XPos - ScrollTotal.XPos, BarStart.YPos - ScrollTotal.YPos, ThemeX.BarStartImage, FALSE);
  Total.Compose(BarEnd.XPos - ScrollTotal.XPos, BarEnd.YPos - ScrollTotal.YPos, ThemeX.BarEndImage, FALSE);
  if (!ThemeX.ScrollbarImage.isEmpty()) {
    for (INTN i = 0; i < Scrollbar.Height; i+=ThemeX.ScrollbarImage.GetHeight()) {
      Total.Compose(Scrollbar.XPos - ScrollTotal.XPos, Scrollbar.YPos + i - ScrollTotal.YPos, ThemeX.ScrollbarImage, FALSE);
    }
  }
  Total.Compose(UpButton.XPos - ScrollTotal.XPos, UpButton.YPos - ScrollTotal.YPos, ThemeX.UpButtonImage, FALSE);
  Total.Compose(DownButton.XPos - ScrollTotal.XPos, DownButton.YPos - ScrollTotal.YPos, ThemeX.DownButtonImage, FALSE);
  Total.Compose(ScrollStart.XPos - ScrollTotal.XPos, ScrollStart.YPos - ScrollTotal.YPos, ThemeX.ScrollStartImage, FALSE);
  Total.Compose(ScrollEnd.XPos - ScrollTotal.XPos, ScrollEnd.YPos - ScrollTotal.YPos, ThemeX.ScrollEndImage, FALSE);
  Total.Draw(ScrollTotal.XPos, ScrollTotal.YPos, ThemeX.ScrollWidth / 16.f); //ScrollWidth can be set in theme.plist but usually=16
#else
  for (INTN i = 0; i < ScrollbarBackground.Height; i += ThemeX.ScrollbarBackgroundImage.GetHeight()) {
    ThemeX.ScrollbarBackgroundImage.Draw(ScrollbarBackground.XPos - ScrollTotal.XPos, ScrollbarBackground.YPos + i - ScrollTotal.YPos, 1.f);
  }
  ThemeX.BarStartImage.Draw(BarStart.XPos - ScrollTotal.XPos, BarStart.YPos - ScrollTotal.YPos, 1.f);
  ThemeX.BarEndImage.Draw(BarEnd.XPos - ScrollTotal.XPos, BarEnd.YPos - ScrollTotal.YPos, 1.f);
  for (INTN i = 0; i < Scrollbar.Height; i += ThemeX.ScrollbarImage.GetHeight()) {
    ThemeX.ScrollbarImage.Draw(Scrollbar.XPos - ScrollTotal.XPos, Scrollbar.YPos + i - ScrollTotal.YPos, 1.f);
  }
  ThemeX.UpButtonImage.Draw(UpButton.XPos - ScrollTotal.XPos, UpButton.YPos - ScrollTotal.YPos, 1.f);
  ThemeX.DownButtonImage.Draw(DownButton.XPos - ScrollTotal.XPos, DownButton.YPos - ScrollTotal.YPos, 1.f);
  ThemeX.ScrollStartImage.Draw(ScrollStart.XPos - ScrollTotal.XPos, ScrollStart.YPos - ScrollTotal.YPos, 1.f);
  ThemeX.ScrollEndImage.Draw(ScrollEnd.XPos - ScrollTotal.XPos, ScrollEnd.YPos - ScrollTotal.YPos, 1.f);
#endif
}
#else
VOID REFIT_MENU_SCREEN::ScrollingBar()
{
  EG_IMAGE* Total;
//  INTN  i;

  ScrollEnabled = (ScrollState.MaxFirstVisible != 0);
  if (ScrollEnabled) {
    Total = egCreateFilledImage(ScrollTotal.Width, ScrollTotal.Height, TRUE, &MenuBackgroundPixel);

    if (ScrollbarBackgroundImage && ScrollbarBackgroundImage->Height) {
      for (INTN i = 0; i < ScrollbarBackground.Height; i+=ScrollbarBackgroundImage->Height) {
        egComposeImage(Total, ScrollbarBackgroundImage, ScrollbarBackground.XPos - ScrollTotal.XPos, ScrollbarBackground.YPos + i - ScrollTotal.YPos);
      }
    }

    egComposeImage(Total, BarStartImage, BarStart.XPos - ScrollTotal.XPos, BarStart.YPos - ScrollTotal.YPos);
    egComposeImage(Total, BarEndImage, BarEnd.XPos - ScrollTotal.XPos, BarEnd.YPos - ScrollTotal.YPos);

    if (ScrollbarImage && ScrollbarImage->Height) {
      for (INTN i = 0; i < Scrollbar.Height; i+=ScrollbarImage->Height) {
        egComposeImage(Total, ScrollbarImage, Scrollbar.XPos - ScrollTotal.XPos, Scrollbar.YPos + i - ScrollTotal.YPos);
      }
    }

    egComposeImage(Total, UpButtonImage, UpButton.XPos - ScrollTotal.XPos, UpButton.YPos - ScrollTotal.YPos);
    egComposeImage(Total, DownButtonImage, DownButton.XPos - ScrollTotal.XPos, DownButton.YPos - ScrollTotal.YPos);
    egComposeImage(Total, ScrollStartImage, ScrollStart.XPos - ScrollTotal.XPos, ScrollStart.YPos - ScrollTotal.YPos);
    egComposeImage(Total, ScrollEndImage, ScrollEnd.XPos - ScrollTotal.XPos, ScrollEnd.YPos - ScrollTotal.YPos);

    BltImageAlpha(Total, ScrollTotal.XPos, ScrollTotal.YPos, &MenuBackgroundPixel, ScrollWidth);
    egFreeImage(Total);
  }
}
#endif
/**
 * Graphical menu.
 */
#if USE_XTHEME
VOID REFIT_MENU_SCREEN::GraphicsMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
  INTN Chosen = 0;
  INTN ItemWidth = 0;
  INTN t1, t2;
  INTN VisibleHeight = 0; //assume vertical layout
  XStringW ResultString;
  INTN PlaceCentre = 0; //(TextHeight / 2) - 7;
  INTN PlaceCentre1 = 0;
  UINTN OldChosenItem = ~(UINTN)0;
  INTN TitleLen = 0;
  INTN ScaledWidth = (INTN)(ThemeX.CharWidth * ThemeX.Scale);
  // clovy
  INTN ctrlX, ctrlY, ctrlTextX;

  HidePointer();

  switch (Function) {

    case MENU_FUNCTION_INIT:
    {
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime();
      SwitchToGraphicsAndClear();

      EntriesPosY = ((UGAHeight - (int)(LAYOUT_TOTAL_HEIGHT * ThemeX.Scale)) >> 1) + (int)(ThemeX.LayoutBannerOffset * ThemeX.Scale) + (TextHeight << 1);

      VisibleHeight = ((UGAHeight - EntriesPosY) / TextHeight) - InfoLines.size() - 2;/* - GlobalConfig.PruneScrollRows; */
      //DBG("MENU_FUNCTION_INIT 1 EntriesPosY=%d VisibleHeight=%d\n", EntriesPosY, VisibleHeight);
      if ( Entries[0].getREFIT_INPUT_DIALOG() ) {
        REFIT_INPUT_DIALOG& entry = (REFIT_INPUT_DIALOG&)Entries[0];
        if (entry.getREFIT_MENU_SWITCH()) {
          if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 3) {
            Chosen = (OldChosenTheme == 0xFFFF) ? 0: (OldChosenTheme + 1);
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 90) {
            Chosen = OldChosenConfig;
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 116) {
            Chosen = (OldChosenDsdt == 0xFFFF) ? 0: (OldChosenDsdt + 1);
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 119) {
            Chosen = OldChosenAudio;
          }
        }
      }
      InitScroll(Entries.size(), Entries.size(), VisibleHeight, Chosen);
      // determine width of the menu - not working
      //MenuWidth = 80;  // minimum
      MenuWidth = (int)(LAYOUT_TEXT_WIDTH * ThemeX.Scale); //500


      if (!TitleImage.isEmpty()) {
        if (MenuWidth > (INTN)(UGAWidth - (int)(TITLEICON_SPACING * ThemeX.Scale) - TitleImage.GetWidth())) {
          MenuWidth = UGAWidth - (int)(TITLEICON_SPACING * ThemeX.Scale) - TitleImage.GetWidth() - 2;
        }
        EntriesPosX = (UGAWidth - (TitleImage.GetWidth() + (int)(TITLEICON_SPACING * ThemeX.Scale) + MenuWidth)) >> 1;
        //       DBG("UGAWIdth=%lld TitleImage=%lld MenuWidth=%lld\n", UGAWidth,
        //           TitleImage.GetWidth(), MenuWidth);
        MenuWidth += TitleImage.GetWidth();
      } else {
        EntriesPosX = (UGAWidth - MenuWidth) >> 1;
      }
      TimeoutPosY = EntriesPosY + (Entries.size() + 1) * TextHeight;

      // initial painting
      egMeasureText(Title.data(), &ItemWidth, NULL);
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_MENU_TITLE)) {
        DrawTextXY(Title, (UGAWidth >> 1), EntriesPosY - TextHeight * 2, X_IS_CENTER);
      }

      if (!TitleImage.isEmpty()) {
        INTN FilmXPos = (INTN)(EntriesPosX - (TitleImage.GetWidth() + (int)(TITLEICON_SPACING * ThemeX.Scale)));
        INTN FilmYPos = (INTN)EntriesPosY;
        //    BltImageAlpha(TitleImage, FilmXPos, FilmYPos, &MenuBackgroundPixel, 16);
        TitleImage.Draw(FilmXPos, FilmYPos);

        // update FilmPlace only if not set by InitAnime
        if (FilmPlace.Width == 0 || FilmPlace.Height == 0) {
          FilmPlace.XPos = FilmXPos;
          FilmPlace.YPos = FilmYPos;
          FilmPlace.Width = TitleImage.GetWidth();
          FilmPlace.Height = TitleImage.GetHeight();
        }
      }

      if (InfoLines.size() > 0) {
        //       DrawMenuText(NULL, 0, 0, 0, 0);
        //EraseTextXY(); //but we should make it complementare to DrawMenuText
        for (UINTN i = 0; i < InfoLines.size(); i++) {
          DrawMenuText(InfoLines[i], 0, EntriesPosX, EntriesPosY, 0xFFFF);
          EntriesPosY += TextHeight;
        }
        EntriesPosY += TextHeight;  // also add a blank line
      }
      ThemeX.InitBar();

      break;
    }
    case MENU_FUNCTION_CLEANUP:
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:
    {
      //         DBG("PAINT_ALL: EntriesPosY=%lld MaxVisible=%lld\n", EntriesPosY, ScrollState.MaxVisible);
      //          DBG("DownButton.Height=%lld TextHeight=%lld MenuWidth=%lld\n", DownButton.Height, TextHeight, MenuWidth);
      t2 = EntriesPosY + (ScrollState.MaxVisible + 1) * TextHeight - DownButton.Height;
      t1 = EntriesPosX + TextHeight + MenuWidth  + (INTN)((TEXT_XMARGIN + 16) * ThemeX.Scale);
      //          DBG("PAINT_ALL: X=%lld Y=%lld\n", t1, t2);
      SetBar(t1, EntriesPosY, t2, &ScrollState); //823 302 554
      /*
       48:307  39:206  UGAWIdth=800 TitleImage=48 MenuWidth=333
       48:635  0:328  PAINT_ALL: EntriesPosY=259 MaxVisible=13
       48:640  0:004  DownButton.Height=0 TextHeight=21 MenuWidth=381
       48:646  0:006  PAINT_ALL: X=622 Y=553
       */

      // blackosx swapped this around so drawing of selection comes before drawing scrollbar.

      for (INTN i = ScrollState.FirstVisible, j = 0; i <= ScrollState.LastVisible; i++, j++) {
        REFIT_ABSTRACT_MENU_ENTRY *Entry = &Entries[i];
        TitleLen = StrLen(Entry->Title);

        Entry->Place.XPos = EntriesPosX;
        Entry->Place.YPos = EntriesPosY + j * TextHeight;
        Entry->Place.Width = TitleLen * ScaledWidth;
        Entry->Place.Height = (UINTN)TextHeight;
        ResultString = Entry->Title; //create a copy to modify later
        PlaceCentre = (INTN)((TextHeight - (INTN)(ThemeX.Buttons[2].GetHeight())) * ThemeX.Scale / 2);
        PlaceCentre1 = (INTN)((TextHeight - (INTN)(ThemeX.Buttons[0].GetHeight())) * ThemeX.Scale / 2);
        // clovy

        if (ThemeX.TypeSVG)
          ctrlX = EntriesPosX;
        else ctrlX = EntriesPosX + (INTN)(TEXT_XMARGIN * ThemeX.Scale);
        ctrlTextX = ctrlX + ThemeX.Buttons[0].GetWidth() + (INTN)(TEXT_XMARGIN * ThemeX.Scale / 2);
        ctrlY = Entry->Place.YPos + PlaceCentre;

        if ( Entry->getREFIT_INPUT_DIALOG() ) {
          REFIT_INPUT_DIALOG* inputDialogEntry = Entry->getREFIT_INPUT_DIALOG();
          if (inputDialogEntry->Item && inputDialogEntry->Item->ItemType == BoolValue) {
            Entry->Place.Width = ResultString.length() * ScaledWidth;
            //possible artefacts
            DrawMenuText(ResultString, (i == ScrollState.CurrentSelection) ? (MenuWidth) : 0,
                         ctrlTextX,
                         Entry->Place.YPos, 0xFFFF);
            ThemeX.Buttons[(((REFIT_INPUT_DIALOG*)(Entry))->Item->BValue)?3:2].DrawOnBack(ctrlX, ctrlY, ThemeX.Background);
          } else {
            // text input
            ResultString += ((REFIT_INPUT_DIALOG*)(Entry))->Item->SValue;
            ResultString += L" ";
            Entry->Place.Width = ResultString.length() * ScaledWidth;
            // Slice - suppose to use Row as Cursor in text
            DrawMenuText(ResultString, (i == ScrollState.CurrentSelection) ? MenuWidth : 0,
                         EntriesPosX,
                         Entry->Place.YPos, TitleLen + Entry->Row);
          }
        } else if (Entry->getREFIT_MENU_CHECKBIT()) {
          ThemeX.FillRectAreaOfScreen(ctrlTextX, Entry->Place.YPos, MenuWidth, TextHeight);
          DrawMenuText(ResultString, (i == ScrollState.CurrentSelection) ? (MenuWidth) : 0,
                       ctrlTextX,
                       Entry->Place.YPos, 0xFFFF);
          ThemeX.Buttons[(((REFIT_INPUT_DIALOG*)(Entry))->Item->IValue & Entry->Row)?3:2].DrawOnBack(ctrlX, ctrlY, ThemeX.Background);
        } else if (Entry->getREFIT_MENU_SWITCH()) {
          if (Entry->getREFIT_MENU_SWITCH()->Item->IValue == 3) {
            //OldChosenItem = OldChosenTheme;
            OldChosenItem = (OldChosenTheme == 0xFFFF) ? 0: (OldChosenTheme + 1);
          } else if (Entry->getREFIT_MENU_SWITCH()->Item->IValue == 90) {
            OldChosenItem = OldChosenConfig;
          } else if (Entry->getREFIT_MENU_SWITCH()->Item->IValue == 116) {
            OldChosenItem =  (OldChosenDsdt == 0xFFFF) ? 0: (OldChosenDsdt + 1);
          } else if (Entry->getREFIT_MENU_SWITCH()->Item->IValue == 119) {
            OldChosenItem = OldChosenAudio;
          }

          DrawMenuText(ResultString,
                       (i == ScrollState.CurrentSelection) ? MenuWidth : 0,
                       // clovy                  EntriesPosX + (TextHeight + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale)),
                       ctrlTextX,
                       Entry->Place.YPos, 0xFFFF);
          ThemeX.Buttons[(Entry->Row == OldChosenItem)?1:0].DrawOnBack(ctrlX, ctrlY, ThemeX.Background);
        } else {
          //DBG("paint entry %d title=%ls\n", i, Entries[i]->Title);
          DrawMenuText(ResultString,
                       (i == ScrollState.CurrentSelection) ? MenuWidth : 0,
                       EntriesPosX, Entry->Place.YPos, 0xFFFF);
        }
      }

      ScrollingBar(); //&ScrollState - inside the class
      //MouseBirth();
      break;
    }
    case MENU_FUNCTION_PAINT_SELECTION:
    {
      // last selection
      REFIT_ABSTRACT_MENU_ENTRY *EntryL = &Entries[ScrollState.LastSelection];
      REFIT_ABSTRACT_MENU_ENTRY *EntryC = &Entries[ScrollState.CurrentSelection];
      TitleLen = EntryL->Title.length();
      ResultString = EntryL->Title;
      //clovy//PlaceCentre = (TextHeight - (INTN)(Buttons[2]->Height * GlobalConfig.Scale)) / 2;
      //clovy//PlaceCentre = (PlaceCentre>0)?PlaceCentre:0;
      //clovy//PlaceCentre1 = (TextHeight - (INTN)(Buttons[0]->Height * GlobalConfig.Scale)) / 2;
      PlaceCentre = (INTN)((TextHeight - (INTN)(ThemeX.Buttons[2].GetHeight())) * ThemeX.Scale / 2);
      PlaceCentre1 = (INTN)((TextHeight - (INTN)(ThemeX.Buttons[0].GetHeight())) * ThemeX.Scale / 2);

      // clovy
      if (ThemeX.TypeSVG)
        ctrlX = EntriesPosX;
      else ctrlX = EntriesPosX + (INTN)(TEXT_XMARGIN * ThemeX.Scale);
      ctrlTextX = ctrlX + ThemeX.Buttons[0].GetWidth() + (INTN)(TEXT_XMARGIN * ThemeX.Scale / 2);

      // redraw selection cursor
      // 1. blackosx swapped this around so drawing of selection comes before drawing scrollbar.
      // 2. usr-sse2
      if ( EntryL->getREFIT_INPUT_DIALOG() ) {
        REFIT_INPUT_DIALOG* inputDialogEntry = (REFIT_INPUT_DIALOG*)EntryL;
        if (inputDialogEntry->Item->ItemType == BoolValue) { //this is checkbox
          //clovy
          DrawMenuText(ResultString, 0,
                       ctrlTextX,
                       EntryL->Place.YPos, 0xFFFF);
          ThemeX.Buttons[(inputDialogEntry->Item->BValue)?3:2].DrawOnBack(ctrlX, EntryL->Place.YPos + PlaceCentre, ThemeX.Background);
        } else {
          ResultString += (((REFIT_INPUT_DIALOG*)(EntryL))->Item->SValue + ((REFIT_INPUT_DIALOG*)(EntryL))->Item->LineShift);
          ResultString += L" ";
          DrawMenuText(ResultString, 0, EntriesPosX,
                       EntriesPosY + (ScrollState.LastSelection - ScrollState.FirstVisible) * TextHeight,
                       TitleLen + EntryL->Row);
        }
      } else if (EntryL->getREFIT_MENU_SWITCH()) { //radio buttons 0,1

        if (EntryL->getREFIT_MENU_SWITCH()->Item->IValue == 3) {
          OldChosenItem = (OldChosenTheme == 0xFFFF) ? 0: OldChosenTheme + 1;
        } else if (EntryL->getREFIT_MENU_SWITCH()->Item->IValue == 90) {
          OldChosenItem = OldChosenConfig;
        } else if (EntryL->getREFIT_MENU_SWITCH()->Item->IValue == 116) {
          OldChosenItem = (OldChosenDsdt == 0xFFFF) ? 0: OldChosenDsdt + 1;
        } else if (EntryL->getREFIT_MENU_SWITCH()->Item->IValue == 119) {
          OldChosenItem = OldChosenAudio;
        }

        // clovy
        DrawMenuText(ResultString, 0, ctrlTextX,
                     EntriesPosY + (ScrollState.LastSelection - ScrollState.FirstVisible) * TextHeight, 0xFFFF);
        ThemeX.Buttons[(EntryL->Row == OldChosenItem)?1:0].DrawOnBack(ctrlX, EntryL->Place.YPos + PlaceCentre1, ThemeX.Background);
      } else if (EntryL->getREFIT_MENU_CHECKBIT()) {
        // clovy
        DrawMenuText(ResultString, 0, ctrlTextX,
                     EntryL->Place.YPos, 0xFFFF);
        ThemeX.Buttons[(EntryL->getREFIT_MENU_CHECKBIT()->Item->IValue & EntryL->Row) ?3:2].DrawOnBack(ctrlX, EntryL->Place.YPos + PlaceCentre, ThemeX.Background);
      } else {
        DrawMenuText(EntryL->Title, 0, EntriesPosX,
                     EntriesPosY + (ScrollState.LastSelection - ScrollState.FirstVisible) * TextHeight, 0xFFFF);
      }

      // current selection
      ResultString = EntryC->Title;
      TitleLen = EntryC->Title.length();
      if ( EntryC->getREFIT_MENU_SWITCH() ) {
        if (EntryC->getREFIT_MENU_SWITCH()->Item->IValue == 3) {
          OldChosenItem = (OldChosenTheme == 0xFFFF) ? 0: OldChosenTheme + 1;;
        } else if (EntryC->getREFIT_MENU_SWITCH()->Item->IValue == 90) {
          OldChosenItem = OldChosenConfig;
        } else if (EntryC->getREFIT_MENU_SWITCH()->Item->IValue == 116) {
          OldChosenItem = (OldChosenDsdt == 0xFFFF) ? 0: OldChosenDsdt + 1;
        } else if (EntryC->getREFIT_MENU_SWITCH()->Item->IValue == 119) {
          OldChosenItem = OldChosenAudio;
        }
      }

      if ( EntryC->getREFIT_INPUT_DIALOG() ) {
        REFIT_INPUT_DIALOG* inputDialogEntry = (REFIT_INPUT_DIALOG*)EntryC;
        if (inputDialogEntry->Item->ItemType == BoolValue) { //checkbox
          DrawMenuText(ResultString, MenuWidth,
                       ctrlTextX,
                       inputDialogEntry->Place.YPos, 0xFFFF);
          ThemeX.Buttons[(inputDialogEntry->Item->BValue)?3:2].DrawOnBack(ctrlX, inputDialogEntry->Place.YPos + PlaceCentre, ThemeX.Background);
        } else {
          ResultString += (inputDialogEntry->Item->SValue +
                           inputDialogEntry->Item->LineShift);
          ResultString += L" ";
          DrawMenuText(ResultString, MenuWidth, EntriesPosX,
                       EntriesPosY + (ScrollState.CurrentSelection - ScrollState.FirstVisible) * TextHeight,
                       TitleLen + inputDialogEntry->Row);
        }
      } else if (EntryC->getREFIT_MENU_SWITCH()) { //radio
        DrawMenuText(EntryC->Title, MenuWidth,
                     ctrlTextX,
                     EntriesPosY + (ScrollState.CurrentSelection - ScrollState.FirstVisible) * TextHeight,
                     0xFFFF);
        ThemeX.Buttons[(EntryC->Row == OldChosenItem)?1:0].DrawOnBack(ctrlX, EntryC->Place.YPos + PlaceCentre1, ThemeX.Background);
      } else if (EntryC->getREFIT_MENU_CHECKBIT()) {
        DrawMenuText(ResultString, MenuWidth,
                     ctrlTextX,
                     EntryC->Place.YPos, 0xFFFF);
        ThemeX.Buttons[(EntryC->getREFIT_MENU_CHECKBIT()->Item->IValue & EntryC->Row)?3:2].DrawOnBack(ctrlX, EntryC->Place.YPos + PlaceCentre, ThemeX.Background);
      }else{
        DrawMenuText(EntryC->Title, MenuWidth, EntriesPosX,
                     EntriesPosY + (ScrollState.CurrentSelection - ScrollState.FirstVisible) * TextHeight,
                     0xFFFF);
      }

      ScrollStart.YPos = ScrollbarBackground.YPos + ScrollbarBackground.Height * ScrollState.FirstVisible / (ScrollState.MaxIndex + 1);
      Scrollbar.YPos = ScrollStart.YPos + ScrollStart.Height;
      ScrollEnd.YPos = Scrollbar.YPos + Scrollbar.Height; // ScrollEnd.Height is already subtracted
      ScrollingBar(); //&ScrollState);

      break;
    }

    case MENU_FUNCTION_PAINT_TIMEOUT: //ParamText should be XStringW
      ResultString.takeValueFrom(ParamText);
      INTN X = (UGAWidth - StrLen(ParamText) * ScaledWidth) >> 1;
      DrawMenuText(ResultString, 0, X, TimeoutPosY, 0xFFFF);
      break;
  }

  MouseBirth();
}

#else


VOID REFIT_MENU_SCREEN::GraphicsMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
  INTN i;
  INTN j = 0;
  INTN ItemWidth = 0;
  INTN X, t1, t2;
  INTN VisibleHeight = 0; //assume vertical layout
  CHAR16 ResultString[TITLE_MAX_LEN]; // assume a title max length of around 128
  INTN PlaceCentre = 0; //(TextHeight / 2) - 7;
  INTN PlaceCentre1 = 0;
  UINTN OldChosenItem = ~(UINTN)0;
	INTN TitleLen = 0;
  INTN ScaledWidth = (INTN)(GlobalConfig.CharWidth * GlobalConfig.Scale);
// clovy
	INTN ctrlX, ctrlY, ctrlTextX;

  HidePointer();

  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime();
			SwitchToGraphicsAndClear();

      EntriesPosY = ((UGAHeight - (int)(LAYOUT_TOTAL_HEIGHT * GlobalConfig.Scale)) >> 1) + (int)(LayoutBannerOffset * GlobalConfig.Scale) + (TextHeight << 1);

      VisibleHeight = ((UGAHeight - EntriesPosY) / TextHeight) - InfoLines.size() - 2;/* - GlobalConfig.PruneScrollRows; */
      //DBG("MENU_FUNCTION_INIT 1 EntriesPosY=%d VisibleHeight=%d\n", EntriesPosY, VisibleHeight);
      if ( Entries[0].getREFIT_INPUT_DIALOG() ) {
        REFIT_INPUT_DIALOG& entry = (REFIT_INPUT_DIALOG&)Entries[0];
        if (entry.getREFIT_MENU_SWITCH()) {
          if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 3) {
            j = (OldChosenTheme == 0xFFFF) ? 0: (OldChosenTheme + 1);
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 90) {
            j = OldChosenConfig;
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 116) {
            j = (OldChosenDsdt == 0xFFFF) ? 0: (OldChosenDsdt + 1);
          } else if (entry.getREFIT_MENU_SWITCH()->Item->IValue == 119) {
            j = OldChosenAudio;
          }
        }
      }
      InitScroll(Entries.size(), Entries.size(), VisibleHeight, j);
      // determine width of the menu - not working
      //MenuWidth = 80;  // minimum
      MenuWidth = (int)(LAYOUT_TEXT_WIDTH * GlobalConfig.Scale); //500
      DrawMenuText(NULL, 0, 0, 0, 0);

      if (TitleImage) {
        if (MenuWidth > (INTN)(UGAWidth - (int)(TITLEICON_SPACING * GlobalConfig.Scale) - TitleImage->Width)) {
          MenuWidth = UGAWidth - (int)(TITLEICON_SPACING * GlobalConfig.Scale) - TitleImage->Width - 2;
        }
        EntriesPosX = (UGAWidth - (TitleImage->Width + (int)(TITLEICON_SPACING * GlobalConfig.Scale) + MenuWidth)) >> 1;
        //DBG("UGAWIdth=%d TitleImage=%d MenuWidth=%d\n", UGAWidth,
        //TitleImage->Width, MenuWidth);
        MenuWidth += TitleImage->Width;
      } else {
        EntriesPosX = (UGAWidth - MenuWidth) >> 1;
      }
      TimeoutPosY = EntriesPosY + (Entries.size() + 1) * TextHeight;

      // initial painting
      egMeasureText(Title, &ItemWidth, NULL);
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE)) {
        DrawTextXY(Title, (UGAWidth >> 1), EntriesPosY - TextHeight * 2, X_IS_CENTER);
      }

      if (TitleImage) {
        INTN FilmXPos = (INTN)(EntriesPosX - (TitleImage->Width + (int)(TITLEICON_SPACING * GlobalConfig.Scale)));
        INTN FilmYPos = (INTN)EntriesPosY;
        BltImageAlpha(TitleImage, FilmXPos, FilmYPos, &MenuBackgroundPixel, 16);

        // update FilmPlace only if not set by InitAnime
        if (FilmPlace.Width == 0 || FilmPlace.Height == 0) {
          FilmPlace.XPos = FilmXPos;
          FilmPlace.YPos = FilmYPos;
          FilmPlace.Width = TitleImage->Width;
          FilmPlace.Height = TitleImage->Height;
        }
      }

      if (InfoLines.size() > 0) {
        DrawMenuText(NULL, 0, 0, 0, 0);
        for (i = 0; i < (INTN)InfoLines.size(); i++) {
          DrawMenuText(InfoLines[i], 0, EntriesPosX, EntriesPosY, 0xFFFF);
          EntriesPosY += TextHeight;
        }
        EntriesPosY += TextHeight;  // also add a blank line
      }
      InitBar();

      break;

    case MENU_FUNCTION_CLEANUP:
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:
      DrawMenuText(NULL, 0, 0, 0, 0); //should clean every line to avoid artefacts
	//		DBG("PAINT_ALL: EntriesPosY=%d MaxVisible=%d\n", EntriesPosY, ScrollState.MaxVisible);
	//		DBG("DownButton.Height=%d TextHeight=%d\n", DownButton.Height, TextHeight);
      t2 = EntriesPosY + (ScrollState.MaxVisible + 1) * TextHeight - DownButton.Height;
      t1 = EntriesPosX + TextHeight + MenuWidth  + (INTN)((TEXT_XMARGIN + 16) * GlobalConfig.Scale);
	//		DBG("PAINT_ALL: %d %d\n", t1, t2);
      SetBar(t1, EntriesPosY, t2, &ScrollState); //823 302 554

      // blackosx swapped this around so drawing of selection comes before drawing scrollbar.

      for (i = ScrollState.FirstVisible, j = 0; i <= ScrollState.LastVisible; i++, j++) {
        REFIT_ABSTRACT_MENU_ENTRY *Entry = &Entries[i];
        TitleLen = StrLen(Entry->Title);

        Entry->Place.XPos = EntriesPosX;
        Entry->Place.YPos = EntriesPosY + j * TextHeight;
        Entry->Place.Width = TitleLen * ScaledWidth;
        Entry->Place.Height = (UINTN)TextHeight;
        StrCpyS(ResultString, TITLE_MAX_LEN, Entry->Title);
        //clovy//PlaceCentre1 = (TextHeight - (INTN)(Buttons[2]->Height * GlobalConfig.Scale)) / 2;
        //clovy//PlaceCentre = (PlaceCentre>0)?PlaceCentre:0;
        //clovy//PlaceCentre1 = (TextHeight - (INTN)(Buttons[0]->Height * GlobalConfig.Scale)) / 2;
        PlaceCentre = (INTN)((TextHeight - (INTN)(Buttons[2]->Height)) * GlobalConfig.Scale / 2);
        PlaceCentre1 = (INTN)((TextHeight - (INTN)(Buttons[0]->Height)) * GlobalConfig.Scale / 2);
// clovy
        ctrlX = EntriesPosX + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale);
        if (GlobalConfig.TypeSVG)
          ctrlX = EntriesPosX;
        ctrlTextX = ctrlX + Buttons[0]->Width + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale / 2);
        ctrlY = Entry->Place.YPos + PlaceCentre;

        if ( Entry->getREFIT_INPUT_DIALOG() ) {
          REFIT_INPUT_DIALOG* inputDialogEntry = Entry->getREFIT_INPUT_DIALOG();
          if (inputDialogEntry->Item->ItemType == BoolValue) {
            Entry->Place.Width = StrLen(ResultString) * ScaledWidth;
            DrawMenuText(L" ", 0, EntriesPosX, Entry->Place.YPos, 0xFFFF);
            DrawMenuText(ResultString, (i == ScrollState.CurrentSelection) ? (MenuWidth) : 0,
// clovy                    EntriesPosX + (TextHeight + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale)),
                         ctrlTextX,
                         Entry->Place.YPos, 0xFFFF);
            BltImageAlpha( (((REFIT_INPUT_DIALOG*)(Entry))->Item->BValue) ? Buttons[3] :Buttons[2],
                          ctrlX, ctrlY,
                          &MenuBackgroundPixel, 16);
//            DBG("X=%d, Y=%d, ImageY=%d\n", EntriesPosX + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale),
//                Entry->Place.YPos, Entry->Place.YPos + PlaceCentre);

          } else {
            // text input
            StrCatS(ResultString, TITLE_MAX_LEN, ((REFIT_INPUT_DIALOG*)(Entry))->Item->SValue);
            StrCatS(ResultString, TITLE_MAX_LEN, L" ");
            Entry->Place.Width = StrLen(ResultString) * ScaledWidth;
            // Slice - suppose to use Row as Cursor in text
            DrawMenuText(ResultString, (i == ScrollState.CurrentSelection) ? MenuWidth : 0,
                         EntriesPosX,
                         Entry->Place.YPos, TitleLen + Entry->Row);
          }
        } else if (Entry->getREFIT_MENU_CHECKBIT()) {
          DrawMenuText(L" ", 0, EntriesPosX, Entry->Place.YPos, 0xFFFF);
          DrawMenuText(ResultString, (i == ScrollState.CurrentSelection) ? (MenuWidth) : 0,
// clovy                  EntriesPosX + (TextHeight + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale)),
                       ctrlTextX,
                       Entry->Place.YPos, 0xFFFF);
          BltImageAlpha((((REFIT_INPUT_DIALOG*)(Entry))->Item->IValue & Entry->Row) ? Buttons[3] :Buttons[2],
                        ctrlX,
                        ctrlY,
                        &MenuBackgroundPixel, 16);

        } else if (Entry->getREFIT_MENU_SWITCH()) {
          if (Entry->getREFIT_MENU_SWITCH()->Item->IValue == 3) {
            //OldChosenItem = OldChosenTheme;
            OldChosenItem = (OldChosenTheme == 0xFFFF) ? 0: (OldChosenTheme + 1);
          } else if (Entry->getREFIT_MENU_SWITCH()->Item->IValue == 90) {
            OldChosenItem = OldChosenConfig;
          } else if (Entry->getREFIT_MENU_SWITCH()->Item->IValue == 116) {
            OldChosenItem =  (OldChosenDsdt == 0xFFFF) ? 0: (OldChosenDsdt + 1);
          } else if (Entry->getREFIT_MENU_SWITCH()->Item->IValue == 119) {
            OldChosenItem = OldChosenAudio;
          }

          DrawMenuText(ResultString,
                       (i == ScrollState.CurrentSelection) ? MenuWidth : 0,
// clovy                  EntriesPosX + (TextHeight + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale)),
                       ctrlTextX,
                       Entry->Place.YPos, 0xFFFF);
          BltImageAlpha((Entry->Row == OldChosenItem) ? Buttons[1] : Buttons[0],
                        ctrlX,
                        ctrlY,
                        &MenuBackgroundPixel, 16);
        } else {
					//DBG("paint entry %d title=%ls\n", i, Entries[i]->Title);
          DrawMenuText(ResultString,
                       (i == ScrollState.CurrentSelection) ? MenuWidth : 0,
                       EntriesPosX, Entry->Place.YPos, 0xFFFF);
        }
      }

      ScrollingBar(); //&ScrollState - inside the class
      //MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
    {
			// last selection
      REFIT_ABSTRACT_MENU_ENTRY *EntryL = &Entries[ScrollState.LastSelection];
      REFIT_ABSTRACT_MENU_ENTRY *EntryC = &Entries[ScrollState.CurrentSelection];
      TitleLen = StrLen(EntryL->Title);
      StrCpyS(ResultString, TITLE_MAX_LEN, EntryL->Title);
      //clovy//PlaceCentre = (TextHeight - (INTN)(Buttons[2]->Height * GlobalConfig.Scale)) / 2;
      //clovy//PlaceCentre = (PlaceCentre>0)?PlaceCentre:0;
      //clovy//PlaceCentre1 = (TextHeight - (INTN)(Buttons[0]->Height * GlobalConfig.Scale)) / 2;
      PlaceCentre = (INTN)((TextHeight - (INTN)(Buttons[2]->Height)) * GlobalConfig.Scale / 2);
      PlaceCentre1 = (INTN)((TextHeight - (INTN)(Buttons[0]->Height)) * GlobalConfig.Scale / 2);

// clovy
			ctrlX = EntriesPosX + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale);
      if (GlobalConfig.TypeSVG)
        ctrlX = EntriesPosX;
			ctrlTextX = ctrlX + Buttons[0]->Width + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale / 2);

      // redraw selection cursor
      // 1. blackosx swapped this around so drawing of selection comes before drawing scrollbar.
      // 2. usr-sse2
      if ( EntryL->getREFIT_INPUT_DIALOG() ) {
        REFIT_INPUT_DIALOG* inputDialogEntry = (REFIT_INPUT_DIALOG*)EntryL;
        if (inputDialogEntry->Item->ItemType == BoolValue) {
          //clovy//DrawMenuText(ResultString, 0, EntriesPosX + (TextHeight + TEXT_XMARGIN),
          //clovy//             EntryL->Place.YPos, 0xFFFF);
          DrawMenuText(ResultString, 0,
                        ctrlTextX,
                       EntryL->Place.YPos, 0xFFFF);
          BltImageAlpha((inputDialogEntry->Item->BValue)? Buttons[3] : Buttons[2],
                        ctrlX,
                        EntryL->Place.YPos + PlaceCentre,
                        &MenuBackgroundPixel, 16);
//          DBG("se:X=%d, Y=%d, ImageY=%d\n", EntriesPosX + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale),
//              EntryL->Place.YPos, EntryL->Place.YPos + PlaceCentre);
        } else {
          StrCatS(ResultString, TITLE_MAX_LEN, ((REFIT_INPUT_DIALOG*)(EntryL))->Item->SValue +
                 ((REFIT_INPUT_DIALOG*)(EntryL))->Item->LineShift);
          StrCatS(ResultString, TITLE_MAX_LEN, L" ");
          DrawMenuText(ResultString, 0, EntriesPosX,
                       EntriesPosY + (ScrollState.LastSelection - ScrollState.FirstVisible) * TextHeight,
                       TitleLen + EntryL->Row);
        }
      } else if (EntryL->getREFIT_MENU_SWITCH()) {

        if (EntryL->getREFIT_MENU_SWITCH()->Item->IValue == 3) {
          OldChosenItem = (OldChosenTheme == 0xFFFF) ? 0: OldChosenTheme + 1;
        } else if (EntryL->getREFIT_MENU_SWITCH()->Item->IValue == 90) {
          OldChosenItem = OldChosenConfig;
        } else if (EntryL->getREFIT_MENU_SWITCH()->Item->IValue == 116) {
          OldChosenItem = (OldChosenDsdt == 0xFFFF) ? 0: OldChosenDsdt + 1;
        } else if (EntryL->getREFIT_MENU_SWITCH()->Item->IValue == 119) {
          OldChosenItem = OldChosenAudio;
        }

// clovy
//         DrawMenuText(ResultString, 0, EntriesPosX + (TextHeight + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale)),
//                      EntriesPosY + (ScrollState.LastSelection - ScrollState.FirstVisible) * TextHeight, 0xFFFF);
        DrawMenuText(ResultString, 0, ctrlTextX,
                     EntriesPosY + (ScrollState.LastSelection - ScrollState.FirstVisible) * TextHeight, 0xFFFF);
        BltImageAlpha((EntryL->Row == OldChosenItem) ? Buttons[1] : Buttons[0],
                     ctrlX,
                      EntryL->Place.YPos + PlaceCentre1,
                      &MenuBackgroundPixel, 16);
      } else if (EntryL->getREFIT_MENU_CHECKBIT()) {
// clovy
//         DrawMenuText(ResultString, 0, EntriesPosX + (TextHeight + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale)),
//                      EntryL->Place.YPos, 0xFFFF);
        DrawMenuText(ResultString, 0, ctrlTextX,
                     EntryL->Place.YPos, 0xFFFF);
        BltImageAlpha((EntryL->getREFIT_MENU_CHECKBIT()->Item->IValue & EntryL->Row) ? Buttons[3] : Buttons[2],
                     ctrlX,
                      EntryL->Place.YPos + PlaceCentre,
                      &MenuBackgroundPixel, 16);
//        DBG("ce:X=%d, Y=%d, ImageY=%d\n", EntriesPosX + (INTN)(TEXT_XMARGIN * GlobalConfig.Scale),
//            EntryL->Place.YPos, EntryL->Place.YPos + PlaceCentre);
      } else {
        DrawMenuText(EntryL->Title, 0, EntriesPosX,
                     EntriesPosY + (ScrollState.LastSelection - ScrollState.FirstVisible) * TextHeight, 0xFFFF);
      }

      // current selection
      StrCpyS(ResultString, TITLE_MAX_LEN, EntryC->Title);
      TitleLen = StrLen(EntryC->Title);
      if ( EntryC->getREFIT_MENU_SWITCH() ) {
		  if (EntryC->getREFIT_MENU_SWITCH()->Item->IValue == 3) {
			OldChosenItem = (OldChosenTheme == 0xFFFF) ? 0: OldChosenTheme + 1;;
		  } else if (EntryC->getREFIT_MENU_SWITCH()->Item->IValue == 90) {
			OldChosenItem = OldChosenConfig;
		  } else if (EntryC->getREFIT_MENU_SWITCH()->Item->IValue == 116) {
			OldChosenItem = (OldChosenDsdt == 0xFFFF) ? 0: OldChosenDsdt + 1;
		  } else if (EntryC->getREFIT_MENU_SWITCH()->Item->IValue == 119) {
			OldChosenItem = OldChosenAudio;
		  }
      }

      if ( EntryC->getREFIT_INPUT_DIALOG() ) {
        REFIT_INPUT_DIALOG* inputDialogEntry = (REFIT_INPUT_DIALOG*)EntryC;
        if (inputDialogEntry->Item->ItemType == BoolValue) {
          DrawMenuText(ResultString, MenuWidth,
                       ctrlTextX,
                       inputDialogEntry->Place.YPos, 0xFFFF);
          BltImageAlpha((inputDialogEntry->Item->BValue)? Buttons[3] : Buttons[2],
                        ctrlX,
                        inputDialogEntry->Place.YPos + PlaceCentre,
                        &MenuBackgroundPixel, 16);
        } else {
          StrCatS(ResultString, TITLE_MAX_LEN, inputDialogEntry->Item->SValue +
                               inputDialogEntry->Item->LineShift);
          StrCatS(ResultString, TITLE_MAX_LEN, L" ");
          DrawMenuText(ResultString, MenuWidth, EntriesPosX,
                       EntriesPosY + (ScrollState.CurrentSelection - ScrollState.FirstVisible) * TextHeight,
                       TitleLen + inputDialogEntry->Row);
        }
      } else if (EntryC->getREFIT_MENU_SWITCH()) {
        StrCpyS(ResultString, TITLE_MAX_LEN, EntryC->Title);
        DrawMenuText(ResultString, MenuWidth,
                     ctrlTextX,
                     EntriesPosY + (ScrollState.CurrentSelection - ScrollState.FirstVisible) * TextHeight,
                     0xFFFF);
        BltImageAlpha((EntryC->Row == OldChosenItem) ? Buttons[1]:Buttons[0],
                      ctrlX,
                      EntryC->Place.YPos + PlaceCentre1,
                      &MenuBackgroundPixel, 16);
      } else if (EntryC->getREFIT_MENU_CHECKBIT()) {
        DrawMenuText(ResultString, MenuWidth,
                     ctrlTextX,
                     EntryC->Place.YPos, 0xFFFF);
        BltImageAlpha((EntryC->getREFIT_MENU_CHECKBIT()->Item->IValue & EntryC->Row) ? Buttons[3] :Buttons[2],
                      ctrlX,
                      EntryC->Place.YPos + PlaceCentre,
                      &MenuBackgroundPixel, 16);
      }else{
        DrawMenuText(EntryC->Title, MenuWidth, EntriesPosX,
                     EntriesPosY + (ScrollState.CurrentSelection - ScrollState.FirstVisible) * TextHeight,
                     0xFFFF);
      }

      ScrollStart.YPos = ScrollbarBackground.YPos + ScrollbarBackground.Height * ScrollState.FirstVisible / (ScrollState.MaxIndex + 1);
      Scrollbar.YPos = ScrollStart.YPos + ScrollStart.Height;
      ScrollEnd.YPos = Scrollbar.YPos + Scrollbar.Height; // ScrollEnd.Height is already subtracted
      ScrollingBar(); //&ScrollState);

      break;
    }

    case MENU_FUNCTION_PAINT_TIMEOUT: //ever be here?
      X = (UGAWidth - StrLen(ParamText) * ScaledWidth) >> 1;
      DrawMenuText(ParamText, 0, X, TimeoutPosY, 0xFFFF);
      break;
  }

  MouseBirth();
}
#endif
/**
 * Draw entries for GUI.
 */
#if USE_XTHEME

#else

#endif

#if USE_XTHEME
VOID REFIT_MENU_SCREEN::DrawMainMenuLabel(IN CONST XStringW& Text, IN INTN XPos, IN INTN YPos)
{
  INTN TextWidth = 0;
  INTN BadgeDim = (INTN)(BADGE_DIMENSION * ThemeX.Scale);

  egMeasureText(Text.wc_str(), &TextWidth, NULL);

  //Clear old text
//  if (OldTextWidth > TextWidth) {
    ThemeX.FillRectAreaOfScreen(OldX, OldY, OldTextWidth, TextHeight);
//  }

  if (!(ThemeX.BootCampStyle)
      && (ThemeX.HideBadges & HDBADGES_INLINE) && (!OldRow)
//      && (OldTextWidth) && (OldTextWidth != TextWidth)
      ) {
    //Clear badge
    ThemeX.FillRectAreaOfScreen((OldX - (OldTextWidth >> 1) - (BadgeDim + 16)),
                                (OldY - ((BadgeDim - TextHeight) >> 1)), 128, 128);
  }
//  XStringW TextX;
//  TextX.takeValueFrom(Text);
  DrawTextXY(Text, XPos, YPos, X_IS_CENTER);

  //show inline badge
  if (!(ThemeX.BootCampStyle) &&
      (ThemeX.HideBadges & HDBADGES_INLINE) &&
      (Entries[ScrollState.CurrentSelection].Row == 0)) {
    // Display Inline Badge: small icon before the text
    Entries[ScrollState.CurrentSelection].Image.Draw((XPos - (TextWidth >> 1) - (BadgeDim + 16)),
                                                     (YPos - ((BadgeDim - TextHeight) >> 1)));
  }

  OldX = XPos;
  OldY = YPos;
  OldTextWidth = TextWidth;
  OldRow = Entries[ScrollState.CurrentSelection].Row;
}
#else
VOID REFIT_MENU_SCREEN::DrawMainMenuLabel(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos)
{
  INTN TextWidth;
  INTN BadgeDim = (INTN)(BADGE_DIMENSION * GlobalConfig.Scale);

  egMeasureText(Text, &TextWidth, NULL);

  //Clear old text
  if (OldTextWidth > TextWidth) {
    FillRectAreaOfScreen(OldX, OldY, OldTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
  }

  if (!(GlobalConfig.BootCampStyle)
      && (GlobalConfig.HideBadges & HDBADGES_INLINE) && (!OldRow)
      && (OldTextWidth) && (OldTextWidth != TextWidth)) {
    //Clear badge
    BltImageAlpha(NULL, (OldX - (OldTextWidth >> 1) - (BadgeDim + 16)),
                  (OldY - ((BadgeDim - TextHeight) >> 1)), &MenuBackgroundPixel, BadgeDim >> 3);
  }
  DrawTextXY(Text, XPos, YPos, X_IS_CENTER);

  //show inline badge
  if (!(GlobalConfig.BootCampStyle) &&
       (GlobalConfig.HideBadges & HDBADGES_INLINE) &&
       (Entries[ScrollState.CurrentSelection].Row == 0)) {
    // Display Inline Badge: small icon before the text
    BltImageAlpha(Entries[ScrollState.CurrentSelection].Image,
                  (XPos - (TextWidth >> 1) - (BadgeDim + 16)),
                  (YPos - ((BadgeDim - TextHeight) >> 1)), &MenuBackgroundPixel, BadgeDim >> 3);
  }

  OldX = XPos;
  OldY = YPos;
  OldTextWidth = TextWidth;
  OldRow = Entries[ScrollState.CurrentSelection].Row;
}
#endif


VOID REFIT_MENU_SCREEN::CountItems()
{
  row0PosX = 0;
  row1PosX = Entries.size();
  // layout
  row0Count = 0; //Nr items in row0
  row1Count = 0;
  for (INTN i = 0; i < (INTN)Entries.size(); i++) {
    if (Entries[i].Row == 0) {
      row0Count++;
      CONSTRAIN_MIN(row0PosX, i);
    } else {
      row1Count++;
      CONSTRAIN_MAX(row1PosX, i);
    }
  }
}

#if USE_XTHEME
VOID REFIT_MENU_SCREEN::DrawTextCorner(UINTN TextC, UINT8 Align)
{
  INTN    Xpos;
//  CHAR16  *Text = NULL;
  XStringW Text;

  if (
      // HIDEUI_ALL - included
      ((TextC == TEXT_CORNER_REVISION) && ((ThemeX.HideUIFlags & HIDEUI_FLAG_REVISION) != 0)) ||
      ((TextC == TEXT_CORNER_HELP) && ((ThemeX.HideUIFlags & HIDEUI_FLAG_HELP) != 0)) ||
      ((TextC == TEXT_CORNER_OPTIMUS) && (GlobalConfig.ShowOptimus == FALSE))
      ) {
    return;
  }

  switch (TextC) {
    case TEXT_CORNER_REVISION:
      // Display Clover boot volume
      if (SelfVolume->VolLabel && SelfVolume->VolLabel[0] != L'#') {
   //     Text = PoolPrint(L"%s, booted from %s", gFirmwareRevision, SelfVolume->VolLabel);
        Text = XStringW() + gFirmwareRevision + L", booted from " + SelfVolume->VolLabel;
      }
      if (Text.isEmpty()) {
        Text = XStringW() + gFirmwareRevision + L" " + SelfVolume->VolName;
      }
      break;
    case TEXT_CORNER_HELP:
      Text = XStringW() + L"F1:Help";
      break;
    case TEXT_CORNER_OPTIMUS:
      if (gGraphics[0].Vendor != Intel) {
        Text = XStringW() + L"Discrete";
      } else {
        Text = XStringW() + L"Intel";
      }
      //      Text = (NGFX == 2)?L"Intel":L"Discrete";
      break;
    default:
      return;
  }

  switch (Align) {
    case X_IS_LEFT:
      Xpos = (INTN)(TextHeight * 0.75f);
      break;
    case X_IS_RIGHT:
      Xpos = UGAWidth - (INTN)(TextHeight * 0.75f);//2
      break;
    case X_IS_CENTER:
      Xpos = UGAWidth >> 1;
      break;
    default:
      Text.setEmpty();
      return;
  }
  //  DBG("draw text %ls at (%d, %d)\n", Text, Xpos, UGAHeight - 5 - TextHeight),
  // clovy  DrawTextXY(Text, Xpos, UGAHeight - 5 - TextHeight, Align);
  DrawTextXY(Text, Xpos, UGAHeight - (INTN)(TextHeight * 1.5f), Align);
}
#else

#endif



#if USE_XTHEME

VOID REFIT_MENU_SCREEN::DrawMainMenuEntry(REFIT_ABSTRACT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos)
{
  INTN MainSize = ThemeX.MainEntriesSize;
  XImage MainImage(MainSize, MainSize);
  XImage* BadgeImage = NULL;

  if (Entry->Row == 0 && Entry->getDriveImage()  &&  !(ThemeX.HideBadges & HDBADGES_SWAP)) {
    MainImage = *Entry->getDriveImage();
  } else {
    MainImage = Entry->Image; //XImage
  }
  //this should be inited by the Theme
  if (MainImage.isEmpty()) {
    if (!IsEmbeddedTheme()) {
      MainImage = ThemeX.GetIcon("os_mac");
    }
    if (MainImage.isEmpty()) {
      MainImage.DummyImage(MainSize);
    }
  }
  INTN CompWidth = (Entry->Row == 0) ? ThemeX.row0TileSize : ThemeX.row1TileSize;
  INTN CompHeight = CompWidth;
  //  DBG("Entry title=%ls; Width=%d\n", Entry->Title, MainImage->Width);
  float fScale;
  if (ThemeX.TypeSVG) {
    fScale = (selected ? 1.f : -1.f);
  } else {
    fScale = ((Entry->Row == 0) ? (ThemeX.MainEntriesSize/128.f * (selected ? 1.f : -1.f)): 1.f) ;
  }

  if (Entry->Row == 0) {
    BadgeImage = Entry->getBadgeImage();
  } //else null

  XImage TopImage = ThemeX.SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)];
  XImage Back(CompWidth, CompHeight);
//  Back.GetArea(XPos, YPos, 0, 0); // this is background at this place
  Back.CopyRect(ThemeX.Background, XPos, YPos);

  INTN OffsetX = (CompWidth - MainImage.GetWidth()) / 2;
  OffsetX = (OffsetX > 0) ? OffsetX: 0;
  INTN OffsetY = (CompHeight - MainImage.GetHeight()) / 2;
  OffsetY = (OffsetY > 0) ? OffsetY: 0;

  if(ThemeX.SelectionOnTop) {
    //place main image in centre. It may be OS or Drive
    Back.Compose(OffsetX, OffsetY, MainImage, false);
  } else {
    Back.Compose(0, 0, TopImage, false); //selection first
    Back.Compose(OffsetX, OffsetY, MainImage, false);
  }

  // place the badge image
  if (BadgeImage &&
      ((INTN)BadgeImage->GetWidth() + 8) < CompWidth &&
      ((INTN)BadgeImage->GetHeight() + 8) < CompHeight) {

    // Check for user badge x offset from theme.plist
    if (ThemeX.BadgeOffsetX != 0xFFFF) {
      // Check if value is between 0 and ( width of the main icon - width of badge )
      if (ThemeX.BadgeOffsetX < 0 || ThemeX.BadgeOffsetX > (CompWidth - (INTN)BadgeImage->GetWidth())) {
        DBG("User offset X %lld is out of range\n", ThemeX.BadgeOffsetX);
        ThemeX.BadgeOffsetX = CompWidth  - 8 - BadgeImage->GetWidth();
        DBG("   corrected to default %lld\n", ThemeX.BadgeOffsetX);
      }
      OffsetX += ThemeX.BadgeOffsetX;
    } else {
      // Set default position
      OffsetX += CompWidth  - 8 - BadgeImage->GetWidth();
    }
    // Check for user badge y offset from theme.plist
    if (ThemeX.BadgeOffsetY != 0xFFFF) {
      // Check if value is between 0 and ( height of the main icon - height of badge )
      if (ThemeX.BadgeOffsetY < 0 || ThemeX.BadgeOffsetY > (CompHeight - (INTN)BadgeImage->GetHeight())) {
        DBG("User offset Y %lld is out of range\n",ThemeX.BadgeOffsetY);
        ThemeX.BadgeOffsetY = CompHeight - 8 - BadgeImage->GetHeight();
        DBG("   corrected to default %lld\n", ThemeX.BadgeOffsetY);
      }
      OffsetY += ThemeX.BadgeOffsetY;
    } else {
      // Set default position
      OffsetY += CompHeight - 8 - BadgeImage->GetHeight();
    }
    Back.Compose(OffsetX, OffsetY, *BadgeImage, false);
  }

  if(ThemeX.SelectionOnTop) {
    Back.Compose(0, 0, TopImage, false); //selection at the top
  }
  Back.DrawWithoutCompose(XPos, YPos);


  // draw BCS indicator
  // Needy: if Labels (Titles) are hidden there is no point to draw the indicator
  if (ThemeX.BootCampStyle && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
//    ThemeX.SelectionImages[4]->HasAlpha = TRUE;

    // indicator is for row 0, main entries, only
    if (Entry->Row == 0) {
//      BltImageAlpha(SelectionImages[4 + (selected ? 0 : 1)],
//                    XPos + (row0TileSize / 2) - (INTN)(INDICATOR_SIZE * 0.5f * GlobalConfig.Scale),
//                    row0PosY + row0TileSize + TextHeight + (INTN)((BCSMargin * 2) * GlobalConfig.Scale),
//                    &MenuBackgroundPixel, Scale);
      TopImage = ThemeX.SelectionImages[4 + (selected ? 0 : 1)];
      TopImage.Draw(XPos + (ThemeX.row0TileSize / 2) - (INTN)(INDICATOR_SIZE * 0.5f * ThemeX.Scale),
                    row0PosY + ThemeX.row0TileSize + TextHeight + (INTN)((BCSMargin * 2) * ThemeX.Scale), fScale, false);
    }
  }

  Entry->Place.XPos = XPos;
  Entry->Place.YPos = YPos;
  Entry->Place.Width = MainImage.GetWidth();
  Entry->Place.Height = MainImage.GetHeight();

}


VOID REFIT_MENU_SCREEN::MainMenuVerticalStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
//  INTN i;
  INTN row0PosYRunning;
  INTN VisibleHeight = 0; //assume vertical layout
  INTN MessageHeight = 20;

  if (ThemeX.TypeSVG && textFace[1].valid) {
    MessageHeight = (INTN)(textFace[1].size * RowHeightFromTextHeight * ThemeX.Scale);
  } else {
    MessageHeight = (INTN)(TextHeight * RowHeightFromTextHeight * ThemeX.Scale);
  }

  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime();
			SwitchToGraphicsAndClear();
			//BltClearScreen(FALSE);
      //adjustable by theme.plist?
      EntriesPosY = (int)(LAYOUT_Y_EDGE * ThemeX.Scale);
      EntriesGap = (int)(ThemeX.TileYSpace * ThemeX.Scale);
      EntriesWidth = ThemeX.MainEntriesSize + (int)(16 * ThemeX.Scale);
      EntriesHeight = ThemeX.MainEntriesSize + (int)(16 * ThemeX.Scale);
      //
      VisibleHeight = (UGAHeight - EntriesPosY - (int)(LAYOUT_Y_EDGE * ThemeX.Scale) + EntriesGap) / (EntriesHeight + EntriesGap);
      EntriesPosX = UGAWidth - EntriesWidth - (int)((BAR_WIDTH + LAYOUT_X_EDGE) * ThemeX.Scale);
      TimeoutPosY = UGAHeight - (int)(LAYOUT_Y_EDGE * ThemeX.Scale) - MessageHeight * 2; //optimus + timeout texts

      CountItems();
      InitScroll(row0Count, Entries.size(), VisibleHeight, 0);
      row0PosX = EntriesPosX;
      row0PosY = EntriesPosY;
      row1PosX = (UGAWidth + EntriesGap - (ThemeX.row1TileSize + (int)(TILE1_XSPACING * ThemeX.Scale)) * row1Count) >> 1;
      textPosY = TimeoutPosY - (int)(ThemeX.TileYSpace * ThemeX.Scale) - MessageHeight; //message text
      row1PosY = textPosY - ThemeX.row1TileSize - (int)(ThemeX.TileYSpace * ThemeX.Scale) - ThemeX.LayoutTextOffset;
      if (!itemPosX) {
        itemPosX = (__typeof__(itemPosX))AllocatePool(sizeof(UINT64) * Entries.size());
        itemPosY = (__typeof__(itemPosY))AllocatePool(sizeof(UINT64) * Entries.size());
      }
      row0PosYRunning = row0PosY;
      row1PosXRunning = row1PosX;

      //     DBG("EntryCount =%d\n", Entries.size());
      for (INTN i = 0; i < (INTN)Entries.size(); i++) {
        if (Entries[i].Row == 0) {
          itemPosX[i] = row0PosX;
          itemPosY[i] = row0PosYRunning;
          row0PosYRunning += EntriesHeight + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          itemPosY[i] = row1PosY;
          row1PosXRunning += ThemeX.row1TileSize + (int)(ThemeX.TileXSpace * ThemeX.Scale);
          //         DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
      ThemeX.InitSelection();

      // Update FilmPlace only if not set by InitAnime
      if (FilmPlace.Width == 0 || FilmPlace.Height == 0) {
     //   CopyMem(&FilmPlace, &BannerPlace, sizeof(BannerPlace));
        FilmPlace = ThemeX.BannerPlace;
      }

      ThemeX.InitBar();
      break;

    case MENU_FUNCTION_CLEANUP:
      FreePool(itemPosX);
      itemPosX = NULL;
      FreePool(itemPosY);
      itemPosY = NULL;
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:
      SetBar(EntriesPosX + EntriesWidth + (int)(10 * ThemeX.Scale),
             EntriesPosY, UGAHeight - (int)(LAYOUT_Y_EDGE * ThemeX.Scale), &ScrollState);
      for (INTN i = 0; i <= ScrollState.MaxIndex; i++) {
        if (Entries[i].Row == 0) {
          if ((i >= ScrollState.FirstVisible) && (i <= ScrollState.LastVisible)) {
            DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                              itemPosX[i - ScrollState.FirstVisible], itemPosY[i - ScrollState.FirstVisible]);
          }
        } else { //row1
          DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                            itemPosX[i], itemPosY[i]);
        }
      }
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)){
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      ScrollingBar(); //&ScrollState);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      if (Entries[ScrollState.LastSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection - ScrollState.FirstVisible],
                          itemPosY[ScrollState.LastSelection - ScrollState.FirstVisible]);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection],
                          itemPosY[ScrollState.LastSelection]);
      }

      if (Entries[ScrollState.CurrentSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection - ScrollState.FirstVisible],
                          itemPosY[ScrollState.CurrentSelection - ScrollState.FirstVisible]);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection],
                          itemPosY[ScrollState.CurrentSelection]);
      }
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      ScrollingBar(); //&ScrollState);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      INTN hi = MessageHeight * ((ThemeX.HideBadges & HDBADGES_INLINE)?3:1);
      HidePointer();
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        ThemeX.FillRectAreaOfScreen((UGAWidth >> 1), textPosY + hi,
                             OldTimeoutTextWidth, TextHeight);
        XStringW TextX;
        TextX.takeValueFrom(ParamText);
        OldTimeoutTextWidth = DrawTextXY(TextX, (UGAWidth >> 1), textPosY + hi, X_IS_CENTER);
      }

      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      break;

  }
}
#else
VOID REFIT_MENU_SCREEN::MainMenuVerticalStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
  INTN i;
  INTN row0PosYRunning;
  INTN VisibleHeight = 0; //assume vertical layout
  INTN MessageHeight = 20;

  if (GlobalConfig.TypeSVG && textFace[1].valid) {
    MessageHeight = (INTN)(textFace[1].size * RowHeightFromTextHeight * GlobalConfig.Scale);
  } else {
    MessageHeight = (INTN)(TextHeight * RowHeightFromTextHeight * GlobalConfig.Scale);
  }


  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime();
      SwitchToGraphicsAndClear();
      //BltClearScreen(FALSE);
      //adjustable by theme.plist?
      EntriesPosY = (int)(LAYOUT_Y_EDGE * GlobalConfig.Scale);
      EntriesGap = (int)(GlobalConfig.TileYSpace * GlobalConfig.Scale);
      EntriesWidth = GlobalConfig.MainEntriesSize + (int)(16 * GlobalConfig.Scale);
      EntriesHeight = GlobalConfig.MainEntriesSize + (int)(16 * GlobalConfig.Scale);
      //
      VisibleHeight = (UGAHeight - EntriesPosY - (int)(LAYOUT_Y_EDGE * GlobalConfig.Scale) + EntriesGap) / (EntriesHeight + EntriesGap);
      EntriesPosX = UGAWidth - EntriesWidth - (int)((BAR_WIDTH + LAYOUT_X_EDGE) * GlobalConfig.Scale);
      TimeoutPosY = UGAHeight - (int)(LAYOUT_Y_EDGE * GlobalConfig.Scale) - MessageHeight * 2; //optimus + timeout texts

      CountItems();
      InitScroll(row0Count, Entries.size(), VisibleHeight, 0);
      row0PosX = EntriesPosX;
      row0PosY = EntriesPosY;
      row1PosX = (UGAWidth + EntriesGap - (row1TileSize + (int)(TILE1_XSPACING * GlobalConfig.Scale)) * row1Count) >> 1;
      textPosY = TimeoutPosY - (int)(GlobalConfig.TileYSpace * GlobalConfig.Scale) - MessageHeight; //message text
      row1PosY = textPosY - row1TileSize - (int)(GlobalConfig.TileYSpace * GlobalConfig.Scale) - LayoutTextOffset;
      if (!itemPosX) {
        itemPosX = (__typeof__(itemPosX))AllocatePool(sizeof(UINT64) * Entries.size());
        itemPosY = (__typeof__(itemPosY))AllocatePool(sizeof(UINT64) * Entries.size());
      }
      row0PosYRunning = row0PosY;
      row1PosXRunning = row1PosX;

      //     DBG("EntryCount =%d\n", Entries.size());
      for (i = 0; i < (INTN)Entries.size(); i++) {
        if (Entries[i].Row == 0) {
          itemPosX[i] = row0PosX;
          itemPosY[i] = row0PosYRunning;
          row0PosYRunning += EntriesHeight + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          itemPosY[i] = row1PosY;
          row1PosXRunning += row1TileSize + (int)(TILE1_XSPACING* GlobalConfig.Scale);
          //         DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
      InitSelection();

      // Update FilmPlace only if not set by InitAnime
      if (FilmPlace.Width == 0 || FilmPlace.Height == 0) {
  //      CopyMem(&FilmPlace, &BannerPlace, sizeof(BannerPlace));
        FilmPlace = BannerPlace;
      }

      InitBar();
      break;

    case MENU_FUNCTION_CLEANUP:
      FreePool(itemPosX);
      itemPosX = NULL;
      FreePool(itemPosY);
      itemPosY = NULL;
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:
      SetBar(EntriesPosX + EntriesWidth + (int)(10 * GlobalConfig.Scale),
             EntriesPosY, UGAHeight - (int)(LAYOUT_Y_EDGE * GlobalConfig.Scale), &ScrollState);
      for (i = 0; i <= ScrollState.MaxIndex; i++) {
        if (Entries[i].Row == 0) {
          if ((i >= ScrollState.FirstVisible) && (i <= ScrollState.LastVisible)) {
            DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                              itemPosX[i - ScrollState.FirstVisible], itemPosY[i - ScrollState.FirstVisible]);
          }
        } else { //row1
          DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                            itemPosX[i], itemPosY[i]);
        }
      }
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      ScrollingBar(); //&ScrollState);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      if (Entries[ScrollState.LastSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection - ScrollState.FirstVisible],
                          itemPosY[ScrollState.LastSelection - ScrollState.FirstVisible]);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection],
                          itemPosY[ScrollState.LastSelection]);
      }

      if (Entries[ScrollState.CurrentSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection - ScrollState.FirstVisible],
                          itemPosY[ScrollState.CurrentSelection - ScrollState.FirstVisible]);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection],
                          itemPosY[ScrollState.CurrentSelection]);
      }
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      ScrollingBar(); //&ScrollState);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      i = (GlobalConfig.HideBadges & HDBADGES_INLINE)?3:1;
      HidePointer();
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        FillRectAreaOfScreen((UGAWidth >> 1), textPosY + MessageHeight * i,
                             OldTimeoutTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
        OldTimeoutTextWidth = DrawTextXY(ParamText, (UGAWidth >> 1), textPosY + MessageHeight * i, X_IS_CENTER);
      }

      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      break;

  }
}
#endif

/**
 * Main screen text.
 */
#if USE_XTHEME
VOID REFIT_MENU_SCREEN::MainMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
  EFI_STATUS Status = EFI_SUCCESS;
//  INTN i = 0;
  INTN MessageHeight = 0;
// clovy
	if (ThemeX.TypeSVG && textFace[1].valid) {
		MessageHeight = (INTN)(textFace[1].size * RowHeightFromTextHeight * ThemeX.Scale);
	} else {
		MessageHeight = (INTN)(TextHeight * RowHeightFromTextHeight * ThemeX.Scale);
	}

  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime();
			SwitchToGraphicsAndClear();
			//BltClearScreen(FALSE);

      EntriesGap = (int)(ThemeX.TileXSpace * ThemeX.Scale);
      EntriesWidth = ThemeX.row0TileSize;
      EntriesHeight = ThemeX.MainEntriesSize + (int)(16.f * ThemeX.Scale);

      MaxItemOnScreen = (UGAWidth - (int)((ROW0_SCROLLSIZE * 2)* ThemeX.Scale)) / (EntriesWidth + EntriesGap); //8
      CountItems();
      InitScroll(row0Count, Entries.size(), MaxItemOnScreen, 0);

      row0PosX = EntriesWidth + EntriesGap;
      row0PosX = row0PosX * ((MaxItemOnScreen < row0Count)?MaxItemOnScreen:row0Count);
      row0PosX = row0PosX - EntriesGap;
      row0PosX = UGAWidth - row0PosX;
      row0PosX = row0PosX >> 1;

      row0PosY = (int)(((float)UGAHeight - ThemeX.LayoutHeight * ThemeX.Scale) * 0.5f +
                  ThemeX.LayoutBannerOffset * ThemeX.Scale);

      row1PosX = (UGAWidth + 8 - (ThemeX.row1TileSize + (INTN)(8.0f * ThemeX.Scale)) * row1Count) >> 1;

      if (ThemeX.BootCampStyle && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        row1PosY = row0PosY + ThemeX.row0TileSize + (INTN)((BCSMargin * 2) * ThemeX.Scale) + TextHeight +
            (INTN)(INDICATOR_SIZE * ThemeX.Scale) +
            (INTN)((ThemeX.LayoutButtonOffset + ThemeX.TileYSpace) * ThemeX.Scale);
      } else {
        row1PosY = row0PosY + EntriesHeight +
            (INTN)((ThemeX.TileYSpace + ThemeX.LayoutButtonOffset) * ThemeX.Scale);
      }

      if (row1Count > 0) {
          textPosY = row1PosY + MAX(ThemeX.row1TileSize, MessageHeight) + (INTN)((ThemeX.TileYSpace + ThemeX.LayoutTextOffset) * ThemeX.Scale);
        } else {
          textPosY = row1PosY;
        }

      if (ThemeX.BootCampStyle) {
        textPosY = row0PosY + ThemeX.row0TileSize + (INTN)((TEXT_YMARGIN + BCSMargin) * ThemeX.Scale);
      }

      FunctextPosY = row1PosY + ThemeX.row1TileSize + (INTN)((ThemeX.TileYSpace + ThemeX.LayoutTextOffset) * ThemeX.Scale);
      
      if (!itemPosX) {
        itemPosX = (__typeof__(itemPosX))AllocatePool(sizeof(UINT64) * Entries.size());
      }

      row0PosXRunning = row0PosX;
      row1PosXRunning = row1PosX;
      //DBG("EntryCount =%d\n", Entries.size());
      for (INTN i = 0; i < (INTN)Entries.size(); i++) {
        if (Entries[i].Row == 0) {
          itemPosX[i] = row0PosXRunning;
          row0PosXRunning += EntriesWidth + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          row1PosXRunning += ThemeX.row1TileSize + (INTN)(TILE1_XSPACING * ThemeX.Scale);
          //DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
      ThemeX.InitSelection();

      // Update FilmPlace only if not set by InitAnime
      if (FilmPlace.Width == 0 || FilmPlace.Height == 0) {
//        CopyMem(&FilmPlace, &BannerPlace, sizeof(BannerPlace));
        FilmPlace = ThemeX.BannerPlace;
      }

      //DBG("main menu inited\n");
      break;

    case MENU_FUNCTION_CLEANUP:
      FreePool(itemPosX);
      itemPosX = NULL;
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:
    
      for (INTN i = 0; i <= ScrollState.MaxIndex; i++) {
        if (Entries[i].Row == 0) {
          if ((i >= ScrollState.FirstVisible) && (i <= ScrollState.LastVisible)) {
            DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                              itemPosX[i - ScrollState.FirstVisible], row0PosY);
            // draw static text for the boot options, BootCampStyle

            if (ThemeX.BootCampStyle && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
              INTN textPosX = itemPosX[i - ScrollState.FirstVisible] + (ThemeX.row0TileSize / 2);
              // clear the screen

              ThemeX.FillRectAreaOfScreen(textPosX, textPosY, EntriesWidth + ThemeX.TileXSpace,
                                   MessageHeight);
              DrawBCSText(Entries[i].Title.data(), textPosX, textPosY, X_IS_CENTER);
            }
          }
        } else {
          DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                            itemPosX[i], row1PosY);
        }
      }

      // clear the text from the second row, required by the BootCampStyle
      if ((ThemeX.BootCampStyle) && (Entries[ScrollState.LastSelection].Row == 1)
          && (Entries[ScrollState.CurrentSelection].Row == 0) && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        ThemeX.FillRectAreaOfScreen((UGAWidth >> 1), FunctextPosY,
                             OldTextWidth, MessageHeight);
      }

      if ((Entries[ScrollState.LastSelection].Row == 0) && (Entries[ScrollState.CurrentSelection].Row == 1)
          && ThemeX.BootCampStyle && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), FunctextPosY);
      }
      if (!(ThemeX.BootCampStyle) && !(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at all! Status=%s\n", strerror(Status));
      }
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      if (Entries[ScrollState.LastSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                      itemPosX[ScrollState.LastSelection - ScrollState.FirstVisible], row0PosY);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection], row1PosY);
      }

      if (Entries[ScrollState.CurrentSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                      itemPosX[ScrollState.CurrentSelection - ScrollState.FirstVisible], row0PosY);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection], row1PosY);
      }

      if ((ThemeX.BootCampStyle) && (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL))
          && Entries[ScrollState.CurrentSelection].Row == 1) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), FunctextPosY);
      }
      if ((!(ThemeX.BootCampStyle)) && (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL))) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at sel! Status=%s\n", strerror(Status));
      }
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      INTN hi = MessageHeight * ((ThemeX.HideBadges & HDBADGES_INLINE)?3:1);
      HidePointer();
      if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_LABEL)){
        ThemeX.FillRectAreaOfScreen((UGAWidth >> 1), FunctextPosY + hi,
                             OldTimeoutTextWidth, MessageHeight);
        XStringW TextX;
        TextX.takeValueFrom(ParamText);
        OldTimeoutTextWidth = DrawTextXY(TextX, (UGAWidth >> 1), FunctextPosY + hi, X_IS_CENTER);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at timeout! Status=%s\n", strerror(Status));
      }
      break;

  }
}
#else
VOID REFIT_MENU_SCREEN::MainMenuStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
  EFI_STATUS Status = EFI_SUCCESS;
  INTN i = 0;
  INTN MessageHeight = 0;
  // clovy
  if (GlobalConfig.TypeSVG && textFace[1].valid) {
    MessageHeight = (INTN)(textFace[1].size * RowHeightFromTextHeight * GlobalConfig.Scale);
  } else {
    MessageHeight = (INTN)(TextHeight * RowHeightFromTextHeight * GlobalConfig.Scale);
  }

  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime();
      SwitchToGraphicsAndClear();
      //BltClearScreen(FALSE);

      EntriesGap = (int)(GlobalConfig.TileXSpace * GlobalConfig.Scale);
      EntriesWidth = row0TileSize;
      EntriesHeight = GlobalConfig.MainEntriesSize + (int)(16.f * GlobalConfig.Scale);

      MaxItemOnScreen = (UGAWidth - (int)((ROW0_SCROLLSIZE * 2)* GlobalConfig.Scale)) / (EntriesWidth + EntriesGap); //8
      CountItems();
      InitScroll(row0Count, Entries.size(), MaxItemOnScreen, 0);

      row0PosX = EntriesWidth + EntriesGap;
      row0PosX = row0PosX * ((MaxItemOnScreen < row0Count)?MaxItemOnScreen:row0Count);
      row0PosX = row0PosX - EntriesGap;
      row0PosX = UGAWidth - row0PosX;
      row0PosX = row0PosX >> 1;

      row0PosY = (int)(((float)UGAHeight - LayoutMainMenuHeight * GlobalConfig.Scale) * 0.5f +
                       LayoutBannerOffset * GlobalConfig.Scale);

      row1PosX = (UGAWidth + 8 - (row1TileSize + (INTN)(8.0f * GlobalConfig.Scale)) * row1Count) >> 1;

      if (GlobalConfig.BootCampStyle && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        row1PosY = row0PosY + row0TileSize + (INTN)((BCSMargin * 2) * GlobalConfig.Scale) + TextHeight +
        (INTN)(INDICATOR_SIZE * GlobalConfig.Scale) +
        (INTN)((LayoutButtonOffset + GlobalConfig.TileYSpace) * GlobalConfig.Scale);
      } else {
        row1PosY = row0PosY + EntriesHeight +
        (INTN)((GlobalConfig.TileYSpace + LayoutButtonOffset) * GlobalConfig.Scale);
      }

      if (row1Count > 0) {
        textPosY = row1PosY + MAX(row1TileSize, MessageHeight) + (INTN)((GlobalConfig.TileYSpace + LayoutTextOffset) * GlobalConfig.Scale);
      } else {
        textPosY = row1PosY;
      }

      if (GlobalConfig.BootCampStyle) {
        textPosY = row0PosY + row0TileSize + (INTN)((TEXT_YMARGIN + BCSMargin) * GlobalConfig.Scale);
      }

      FunctextPosY = row1PosY + row1TileSize + (INTN)((GlobalConfig.TileYSpace + LayoutTextOffset) * GlobalConfig.Scale);

      if (!itemPosX) {
        itemPosX = (__typeof__(itemPosX))AllocatePool(sizeof(UINT64) * Entries.size());
      }

      row0PosXRunning = row0PosX;
      row1PosXRunning = row1PosX;
      //DBG("EntryCount =%d\n", Entries.size());
      for (i = 0; i < (INTN)Entries.size(); i++) {
        if (Entries[i].Row == 0) {
          itemPosX[i] = row0PosXRunning;
          row0PosXRunning += EntriesWidth + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          row1PosXRunning += row1TileSize + (INTN)(TILE1_XSPACING * GlobalConfig.Scale);
          //DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
      InitSelection();

      // Update FilmPlace only if not set by InitAnime
      if (FilmPlace.Width == 0 || FilmPlace.Height == 0) {
        CopyMem(&FilmPlace, &BannerPlace, sizeof(BannerPlace));
      }

      //DBG("main menu inited\n");
      break;

    case MENU_FUNCTION_CLEANUP:
      FreePool(itemPosX);
      itemPosX = NULL;
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:

      for (i = 0; i <= ScrollState.MaxIndex; i++) {
        if (Entries[i].Row == 0) {
          if ((i >= ScrollState.FirstVisible) && (i <= ScrollState.LastVisible)) {
            DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                              itemPosX[i - ScrollState.FirstVisible], row0PosY);
            // draw static text for the boot options, BootCampStyle
            if (GlobalConfig.BootCampStyle && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
              INTN textPosX = itemPosX[i - ScrollState.FirstVisible] + (row0TileSize / 2);
              FillRectAreaOfScreen(textPosX, textPosY, EntriesWidth + GlobalConfig.TileXSpace,
                                   MessageHeight, &MenuBackgroundPixel, X_IS_CENTER);
              // draw the text
              DrawBCSText(Entries[i].Title.data(), textPosX, textPosY, X_IS_CENTER);
            }
          }
        } else {
          DrawMainMenuEntry(&Entries[i], (i == ScrollState.CurrentSelection)?1:0,
                            itemPosX[i], row1PosY);
        }
      }
      // clear the text from the second row, required by the BootCampStyle
      if ((GlobalConfig.BootCampStyle) && (Entries[ScrollState.LastSelection].Row == 1)
          && (Entries[ScrollState.CurrentSelection].Row == 0) && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        FillRectAreaOfScreen((UGAWidth >> 1), FunctextPosY,
                             // clovy
                             OldTextWidth, MessageHeight, &MenuBackgroundPixel, X_IS_CENTER);
      }

      if ((Entries[ScrollState.LastSelection].Row == 0) && (Entries[ScrollState.CurrentSelection].Row == 1)
          && GlobalConfig.BootCampStyle && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), FunctextPosY);
      }

      // something is wrong with the DrawMainMenuLabel or Entries[ScrollState.CurrentSelection]
      // and it's required to create the first selection text from here
      // used for all the entries
      if (!(GlobalConfig.BootCampStyle) && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at all! Status=%s\n", strerror(Status));
      }
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      if (Entries[ScrollState.LastSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection - ScrollState.FirstVisible], row0PosY);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.LastSelection], FALSE,
                          itemPosX[ScrollState.LastSelection], row1PosY);
      }

      if (Entries[ScrollState.CurrentSelection].Row == 0) {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection - ScrollState.FirstVisible], row0PosY);
      } else {
        DrawMainMenuEntry(&Entries[ScrollState.CurrentSelection], TRUE,
                          itemPosX[ScrollState.CurrentSelection], row1PosY);
      }

      // create dynamic text for the second row if BootCampStyle is used
      if ((GlobalConfig.BootCampStyle) && (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL))
          && Entries[ScrollState.CurrentSelection].Row == 1) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), FunctextPosY);
      }

      // create dynamic text for all the entries
      if ((!(GlobalConfig.BootCampStyle)) && (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL))) {
        DrawMainMenuLabel(Entries[ScrollState.CurrentSelection].Title,
                          (UGAWidth >> 1), textPosY);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at sel! Status=%s\n", strerror(Status));
      }
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      i = (GlobalConfig.HideBadges & HDBADGES_INLINE)?3:1;
      HidePointer();
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        FillRectAreaOfScreen((UGAWidth >> 1), FunctextPosY + MessageHeight * i,
                             OldTimeoutTextWidth, MessageHeight, &MenuBackgroundPixel, X_IS_CENTER);
        OldTimeoutTextWidth = DrawTextXY(ParamText, (UGAWidth >> 1), FunctextPosY + MessageHeight * i, X_IS_CENTER);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_OPTIMUS, X_IS_CENTER);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      Status = MouseBirth();
      if(EFI_ERROR(Status)) {
        DBG("can't bear mouse at timeout! Status=%s\n", strerror(Status));
      }
      break;

  }
}
#endif

//
// user-callable dispatcher functions
//

UINTN REFIT_MENU_SCREEN::RunMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry)
{
  INTN Index = -1;

  if (AllowGraphicsMode)

    return RunGenericMenu(&REFIT_MENU_SCREEN::GraphicsMenuStyle, &Index, ChosenEntry);
  else
    return RunGenericMenu(&REFIT_MENU_SCREEN::TextMenuStyle, &Index, ChosenEntry);
}


VOID REFIT_MENU_SCREEN::AddMenuCheck(CONST CHAR8 *Text, UINTN Bit, INTN ItemNum)
{
  REFIT_MENU_CHECKBIT *InputBootArgs;

//  InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs = new REFIT_MENU_CHECKBIT;
  InputBootArgs->Title.takeValueFrom(Text);
//  InputBootArgs->Tag = TAG_CHECKBIT_OLD;
  InputBootArgs->Row = Bit;
  InputBootArgs->Item = &InputItems[ItemNum];
  InputBootArgs->AtClick = ActionEnter;
  InputBootArgs->AtRightClick = ActionDetails;
  AddMenuEntry(InputBootArgs, true);
}


VOID REFIT_MENU_SCREEN::AddMenuItem_(REFIT_MENU_ENTRY_ITEM_ABSTRACT* InputBootArgs, INTN Inx, CONST CHAR8 *Line, BOOLEAN Cursor)
{
  InputBootArgs->Title.takeValueFrom(Line);
  if (Inx == 3 || Inx == 116) {
    InputBootArgs->Row          = 0;
  } else {
    InputBootArgs->Row          = Cursor?StrLen(InputItems[Inx].SValue):0xFFFF;
  }
  InputBootArgs->Item           = &InputItems[Inx];
  InputBootArgs->AtClick        = Cursor?ActionSelect:ActionEnter;
  InputBootArgs->AtRightClick   = Cursor?ActionNone:ActionDetails;
  InputBootArgs->AtDoubleClick  = Cursor?ActionEnter:ActionNone;

  AddMenuEntry(InputBootArgs, true);
}
//
//VOID AddMenuItem(REFIT_MENU_SCREEN  *SubScreen, INTN Inx, CONST CHAR8 *Title, UINTN Tag, BOOLEAN Cursor)
//{
////  REFIT_INPUT_DIALOG *InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
//  REFIT_INPUT_DIALOG *InputBootArgs = new REFIT_INPUT_DIALOG;
//  AddMenuItem_(SubScreen, InputBootArgs, Inx, Title, Tag, Cursor);
//}

VOID REFIT_MENU_SCREEN::AddMenuItemInput(INTN Inx, CONST CHAR8 *Line, BOOLEAN Cursor)
{
//  REFIT_INPUT_DIALOG *InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  REFIT_INPUT_DIALOG *InputBootArgs = new REFIT_INPUT_DIALOG;
  AddMenuItem_(InputBootArgs, Inx, Line, Cursor);
}

VOID REFIT_MENU_SCREEN::AddMenuItemSwitch(INTN Inx, CONST CHAR8 *Line, BOOLEAN Cursor)
{
  REFIT_MENU_SWITCH *InputBootArgs = new REFIT_MENU_SWITCH;
  AddMenuItem_(InputBootArgs, Inx, Line, Cursor);
}


UINTN REFIT_MENU_SCREEN::RunMainMenu(IN INTN DefaultSelection, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry)
{

  MENU_STYLE_FUNC     Style             = &REFIT_MENU_SCREEN::TextMenuStyle;
  MENU_STYLE_FUNC     MainStyle         = &REFIT_MENU_SCREEN::TextMenuStyle;

  REFIT_ABSTRACT_MENU_ENTRY    *TempChosenEntry  = 0;
  REFIT_ABSTRACT_MENU_ENTRY    *MainChosenEntry  = 0;
  REFIT_ABSTRACT_MENU_ENTRY    *NextChosenEntry  = NULL;
  UINTN               MenuExit = 0, SubMenuExit = 0;
  INTN                DefaultEntryIndex = DefaultSelection;
  INTN                SubMenuIndex;

  if (AllowGraphicsMode) {
    Style = &REFIT_MENU_SCREEN::GraphicsMenuStyle;
#if USE_XTHEME
    if (ThemeX.VerticalLayout) {
      MainStyle = &REFIT_MENU_SCREEN::MainMenuVerticalStyle;
    } else {
      MainStyle = &REFIT_MENU_SCREEN::MainMenuStyle;
    }
#else
    if (GlobalConfig.VerticalLayout) {
      MainStyle = &REFIT_MENU_SCREEN::MainMenuVerticalStyle;
    } else {
      MainStyle = &REFIT_MENU_SCREEN::MainMenuStyle;
    }
#endif
  }

  while (!MenuExit) {
    AnimeRun = MainAnime;
    MenuExit = RunGenericMenu(MainStyle, &DefaultEntryIndex, &MainChosenEntry);
    TimeoutSeconds = 0;

    if (MenuExit == MENU_EXIT_DETAILS && MainChosenEntry->SubScreen != NULL) {
      CHAR16 *TmpArgs = NULL;
      if (AsciiStrLen(gSettings.BootArgs) > 0) {
        TmpArgs = PoolPrint(L"%a", gSettings.BootArgs);
      }
      SubMenuIndex = -1;

      gSettings.OptionsBits = EncodeOptions(TmpArgs);
//      DBG("main OptionsBits = 0x%X\n", gSettings.OptionsBits);
      if (MainChosenEntry->getLOADER_ENTRY())
        gSettings.OptionsBits |= EncodeOptions(MainChosenEntry->getLOADER_ENTRY()->LoadOptions);
//      DBG("add OptionsBits = 0x%X\n", gSettings.OptionsBits);
      if (MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM())
        DecodeOptions(MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM());
      //      DBG(" enter menu with LoadOptions: %ls\n", ((LOADER_ENTRY*)MainChosenEntry)->LoadOptions);
      if (MainChosenEntry->getLOADER_ENTRY()) {
        // Only for non-legacy entries, as LEGACY_ENTRY doesn't have Flags
        gSettings.FlagsBits = MainChosenEntry->getLOADER_ENTRY()->Flags;
      }
 //          DBG(" MainChosenEntry with FlagsBits = 0x%X\n", gSettings.FlagsBits);

      if (TmpArgs) {
        FreePool(TmpArgs);
        TmpArgs = NULL;
      }
      SubMenuExit = 0;
      while (!SubMenuExit) {
        //running details menu
        SubMenuExit = MainChosenEntry->SubScreen->RunGenericMenu(Style, &SubMenuIndex, &TempChosenEntry);
        if ( MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM() ) DecodeOptions(MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM());
//        DBG("get OptionsBits = 0x%X\n", gSettings.OptionsBits);
//        DBG(" TempChosenEntry FlagsBits = 0x%X\n", ((LOADER_ENTRY*)TempChosenEntry)->Flags);
        if (SubMenuExit == MENU_EXIT_ESCAPE || TempChosenEntry->getREFIT_MENU_ITEM_RETURN() ) {
          SubMenuExit = MENU_EXIT_ENTER;
          MenuExit = 0;
          break;
        }
        if (MainChosenEntry->getREFIT_MENU_ENTRY_CLOVER()) {
          MainChosenEntry->getREFIT_MENU_ENTRY_CLOVER()->LoadOptions = EfiStrDuplicate(((REFIT_MENU_ENTRY_CLOVER*)TempChosenEntry)->LoadOptions);
        }
        //       DBG(" exit menu with LoadOptions: %ls\n", ((LOADER_ENTRY*)MainChosenEntry)->LoadOptions);
        if (SubMenuExit == MENU_EXIT_ENTER && MainChosenEntry->getLOADER_ENTRY() && TempChosenEntry->getLOADER_ENTRY()) {
          // Only for non-legacy entries, as LEGACY_ENTRY doesn't have Flags
          MainChosenEntry->getLOADER_ENTRY()->Flags = TempChosenEntry->getLOADER_ENTRY()->Flags;
//           DBG(" get MainChosenEntry FlagsBits = 0x%X\n", ((LOADER_ENTRY*)MainChosenEntry)->Flags);
        }
        if (/*MenuExit == MENU_EXIT_ENTER &&*/ MainChosenEntry->getLOADER_ENTRY()) {
          if (MainChosenEntry->getLOADER_ENTRY()->LoadOptions) {
            snprintf(gSettings.BootArgs, 255, "%ls", MainChosenEntry->getLOADER_ENTRY()->LoadOptions);
          } else {
            ZeroMem(&gSettings.BootArgs, 255);
          }
          DBG(" boot with args: %s\n", gSettings.BootArgs);
        }
        //---- Details submenu (kexts disabling etc)
        if (SubMenuExit == MENU_EXIT_ENTER || MenuExit == MENU_EXIT_DETAILS) {
          if (TempChosenEntry->SubScreen != NULL) {
            UINTN NextMenuExit = 0;
            INTN NextEntryIndex = -1;
            while (!NextMenuExit) {
              NextMenuExit = TempChosenEntry->SubScreen->RunGenericMenu(Style, &NextEntryIndex, &NextChosenEntry);
              if (NextMenuExit == MENU_EXIT_ESCAPE || NextChosenEntry->getREFIT_MENU_ITEM_RETURN() ) {
                SubMenuExit = 0;
                NextMenuExit = MENU_EXIT_ENTER;
                break;
              }
 //             DBG(" get NextChosenEntry FlagsBits = 0x%X\n", ((LOADER_ENTRY*)NextChosenEntry)->Flags);
              //---- Details submenu (kexts disabling etc) second level
              if (NextMenuExit == MENU_EXIT_ENTER || MenuExit == MENU_EXIT_DETAILS) {
                if (NextChosenEntry->SubScreen != NULL) {
                  UINTN DeepMenuExit = 0;
                  INTN DeepEntryIndex = -1;
                  REFIT_ABSTRACT_MENU_ENTRY    *DeepChosenEntry  = NULL;
                  while (!DeepMenuExit) {
                    DeepMenuExit = NextChosenEntry->SubScreen->RunGenericMenu(Style, &DeepEntryIndex, &DeepChosenEntry);
                    if (DeepMenuExit == MENU_EXIT_ESCAPE || DeepChosenEntry->getREFIT_MENU_ITEM_RETURN() ) {
                      DeepMenuExit = MENU_EXIT_ENTER;
                      NextMenuExit = 0;
                      break;
                    }
 //                   DBG(" get DeepChosenEntry FlagsBits = 0x%X\n", ((LOADER_ENTRY*)DeepChosenEntry)->Flags);
                  } //while(!DeepMenuExit)
                }
              }

            } //while(!NextMenuExit)
          }
        }
        //---------
      }
    }
  }

  if (ChosenEntry) {
    *ChosenEntry = MainChosenEntry;
  }
  return MenuExit;
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

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

#include "Platform.h"

#include "egemb_back_selected_small.h"


#define DEBUG_MENU 0

#if DEBUG_MENU == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_MENU == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

// scrolling definitions

static INTN MaxItemOnScreen = -1;

typedef struct {
  INTN    CurrentSelection, LastSelection;
  INTN    MaxScroll, MaxIndex;
  INTN    FirstVisible, LastVisible, MaxVisible, MaxFirstVisible;
  BOOLEAN IsScrolling, PaintAll, PaintSelection;
} SCROLL_STATE;

#define SCROLL_LINE_UP    (0)
#define SCROLL_LINE_DOWN  (1)
#define SCROLL_PAGE_UP    (2)
#define SCROLL_PAGE_DOWN  (3)
#define SCROLL_FIRST      (4)
#define SCROLL_LAST       (5)
#define SCROLL_NONE       (6)

// other menu definitions

#define MENU_FUNCTION_INIT            (0)
#define MENU_FUNCTION_CLEANUP         (1)
#define MENU_FUNCTION_PAINT_ALL       (2)
#define MENU_FUNCTION_PAINT_SELECTION (3)
#define MENU_FUNCTION_PAINT_TIMEOUT   (4)

typedef VOID (*MENU_STYLE_FUNC)(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText);

static CHAR16 ArrowUp[2] = { ARROW_UP, 0 };
static CHAR16 ArrowDown[2] = { ARROW_DOWN, 0 };

//#define TextHeight (FONT_CELL_HEIGHT + TEXT_YMARGIN * 2)
#define TITLEICON_SPACING (16)

#define ROW0_TILESIZE (144)
#define ROW1_TILESIZE (64)
#define TILE_XSPACING (8)
#define TILE_YSPACING (16)
#define ROW0_SCROLLSIZE (100)

static EG_IMAGE *SelectionImages[4] = { NULL, NULL, NULL, NULL };
static EG_PIXEL SelectionBackgroundPixel = { 0xff, 0xff, 0xff, 0 };
static EG_IMAGE *TextBuffer = NULL;

static UINTN row0Count, row0PosX, row0PosXRunning;
static UINTN row1Count, row1PosX, row1PosXRunning;
static UINTN *itemPosX;
static UINTN row0PosY, row1PosY, textPosY;

INPUT_ITEM *InputItems;
UINTN  InputItemsCount = 0;


VOID FillInputs(VOID)
{
  InputItemsCount = 0; 
  InputItems = AllocateZeroPool(20 * sizeof(INPUT_ITEM)); //XXX
  InputItems[InputItemsCount].ItemType = ASString;
  //even though Ascii we will keep value as Unicode to convert later
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%a", gSettings.BootArgs);
  InputItems[InputItemsCount].ItemType = BoolValue;
  InputItems[InputItemsCount].BValue = gSettings.UseDSDTmini;
  InputItems[InputItemsCount++].SValue = gSettings.UseDSDTmini?L"[X]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Decimal;
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%d", gSettings.HDALayoutId);
  //and so on  
}

VOID ApplyInputs(VOID)
{
  if (InputItems[0].Valid) {
    AsciiSPrint(gSettings.BootArgs, 255, "%s", InputItems[0].SValue);
  }
  if (InputItems[1].Valid) {
    gSettings.UseDSDTmini = InputItems[1].BValue;
  }
  if (InputItems[2].Valid) {
    gSettings.HDALayoutId = StrDecimalToUintn((CHAR16*)&(InputItems[0].SValue));
  }
}

VOID FreeItems(VOID)
{
/*  UINTN i;
  for (i=0; i<InputItemsCount; i++) {
    FreePool(InputItems[i].AValue);
    FreePool(InputItems[i].SValue);
  } */
  FreePool(InputItems);
}

//
// Graphics helper functions
//

static VOID InitSelection(VOID)
{
  UINTN       x, y, src_x, src_y;
  EG_PIXEL    *DestPtr, *SrcPtr;
  
  if (!AllowGraphicsMode)
    return;
  if (SelectionImages[0] != NULL)
    return;
  
  // load small selection image
  if (GlobalConfig.SelectionSmallFileName != NULL)
    SelectionImages[2] = egLoadImage(ThemeDir, GlobalConfig.SelectionSmallFileName, FALSE);
  if (SelectionImages[2] == NULL)
    SelectionImages[2] = egPrepareEmbeddedImage(&egemb_back_selected_small, FALSE);
  SelectionImages[2] = egEnsureImageSize(SelectionImages[2],
                                         ROW1_TILESIZE, ROW1_TILESIZE, &MenuBackgroundPixel);
  if (SelectionImages[2] == NULL)
    return;
  
  // load big selection image
  if (GlobalConfig.SelectionBigFileName != NULL) {
    SelectionImages[0] = egLoadImage(ThemeDir, GlobalConfig.SelectionBigFileName, FALSE);
    SelectionImages[0] = egEnsureImageSize(SelectionImages[0],
                                           ROW0_TILESIZE, ROW0_TILESIZE, &MenuBackgroundPixel);
  }
  if (SelectionImages[0] == NULL) {
    // calculate big selection image from small one
    
    SelectionImages[0] = egCreateImage(ROW0_TILESIZE, ROW0_TILESIZE, FALSE);
    if (SelectionImages[0] == NULL) {
      egFreeImage(SelectionImages[2]);
      SelectionImages[2] = NULL;
      return;
    }
    
    DestPtr = SelectionImages[0]->PixelData;
    SrcPtr  = SelectionImages[2]->PixelData;
    for (y = 0; y < ROW0_TILESIZE; y++) {
      if (y < (ROW1_TILESIZE >> 1))
        src_y = y;
      else if (y < (ROW0_TILESIZE - (ROW1_TILESIZE >> 1)))
        src_y = (ROW1_TILESIZE >> 1);
      else
        src_y = y - (ROW0_TILESIZE - ROW1_TILESIZE);
      
      for (x = 0; x < ROW0_TILESIZE; x++) {
        if (x < (ROW1_TILESIZE >> 1))
          src_x = x;
        else if (x < (ROW0_TILESIZE - (ROW1_TILESIZE >> 1)))
          src_x = (ROW1_TILESIZE >> 1);
        else
          src_x = x - (ROW0_TILESIZE - ROW1_TILESIZE);
        
        *DestPtr++ = SrcPtr[src_y * ROW1_TILESIZE + src_x];
      }
    }
  }
  
  // non-selected background images
  SelectionImages[1] = egCreateFilledImage(ROW0_TILESIZE, ROW0_TILESIZE, FALSE, &MenuBackgroundPixel);
  SelectionImages[3] = egCreateFilledImage(ROW1_TILESIZE, ROW1_TILESIZE, FALSE, &MenuBackgroundPixel);
}

//
// Scrolling functions
//
#define CONSTRAIN_MIN(Variable, MinValue) if (Variable < MinValue) Variable = MinValue
#define CONSTRAIN_MAX(Variable, MaxValue) if (Variable > MaxValue) Variable = MaxValue

static VOID InitScroll(OUT SCROLL_STATE *State, IN UINTN ItemCount, IN UINTN MaxCount, IN UINTN VisibleSpace)
{
  State->LastSelection = State->CurrentSelection = 0;
  State->MaxIndex = (INTN)MaxCount - 1;
  State->MaxScroll = (INTN)ItemCount - 1;
  State->FirstVisible = 0;
  
  if (VisibleSpace == 0)
    State->MaxVisible = State->MaxScroll;
  else
    State->MaxVisible = (INTN)VisibleSpace - 1;
  
  State->MaxFirstVisible = State->MaxScroll - State->MaxVisible;
  CONSTRAIN_MIN(State->MaxFirstVisible, 0);
  
  State->IsScrolling = (State->MaxFirstVisible > 0);
  State->PaintAll = TRUE;
  State->PaintSelection = FALSE;
  
  State->LastVisible = State->FirstVisible + State->MaxVisible;
  DBG("InitScroll: MaxIndex=%d, FirstVisible=%d, MaxVisible=%d, MaxFirstVisible=%d\n",
      State->MaxIndex, State->FirstVisible, State->MaxVisible, State->MaxFirstVisible);
  // 14 0 7 2 => MaxScroll = 9 ItemCount=10 MaxCount=15
}

static VOID UpdateScroll(IN OUT SCROLL_STATE *State, IN UINTN Movement)
{
  State->LastSelection = State->CurrentSelection;
  DBG("UpdateScroll on %d\n", Movement);
  switch (Movement) {
    case SCROLL_LINE_UP: //of left = decrement
      if (State->CurrentSelection > 0) {
        State->CurrentSelection --;
        if (State->CurrentSelection < State->FirstVisible) {
          State->PaintAll = TRUE;
          State->FirstVisible = State->CurrentSelection; // - (State->MaxVisible >> 1);
          //      CONSTRAIN_MIN(State->FirstVisible, 0);
        }
        if (State->CurrentSelection == State->MaxScroll) {
          State->PaintAll = TRUE;
        }
        if ((State->CurrentSelection < State->MaxScroll) && 
             (State->CurrentSelection > State->LastVisible)) {
          State->PaintAll = TRUE;
          State->LastVisible = State->CurrentSelection;
          State->FirstVisible = State->LastVisible - State->MaxVisible;
        }
      }
      break;
      
    case SCROLL_LINE_DOWN: //or right -- increment
      if (State->CurrentSelection < State->MaxIndex) {
        State->CurrentSelection++;
        if ((State->CurrentSelection > State->LastVisible) &&
            (State->CurrentSelection <= State->MaxScroll)){
          State->PaintAll = TRUE;
          State->FirstVisible++;
          CONSTRAIN_MAX(State->FirstVisible, State->MaxFirstVisible);
        }
        if (State->CurrentSelection == State->MaxScroll + 1) {
          State->PaintAll = TRUE;
        }        
      }
      break;
      
    case SCROLL_PAGE_UP:
      if (State->CurrentSelection > 0) {
        if (State->CurrentSelection == State->MaxIndex) {   // currently at last entry, special treatment
          if (State->IsScrolling)
            State->CurrentSelection -= State->MaxVisible - 1;  // move to second line without scrolling
          else
            State->CurrentSelection = 0;                // move to first entry
        } else {
          if (State->FirstVisible > 0)
            State->PaintAll = TRUE;
          State->CurrentSelection -= State->MaxVisible;          // move one page and scroll synchronously
          State->FirstVisible -= State->MaxVisible;
        }
        CONSTRAIN_MIN(State->CurrentSelection, 0);
        CONSTRAIN_MIN(State->FirstVisible, 0);
        if (State->CurrentSelection < State->FirstVisible) {
          State->PaintAll = TRUE;
          State->FirstVisible = State->CurrentSelection; 
        }        
      }
      break;
      
    case SCROLL_PAGE_DOWN:
      if (State->CurrentSelection < State->MaxIndex) {
        if (State->CurrentSelection == 0) {   // currently at first entry, special treatment
          if (State->IsScrolling)
            State->CurrentSelection += State->MaxVisible - 1;  // move to second-to-last line without scrolling
          else
            State->CurrentSelection = State->MaxIndex;         // move to last entry
        } else {
          if (State->FirstVisible < State->MaxFirstVisible)
            State->PaintAll = TRUE;
          State->CurrentSelection += State->MaxVisible;          // move one page and scroll synchronously
          State->FirstVisible += State->MaxVisible;
        }
        CONSTRAIN_MAX(State->CurrentSelection, State->MaxIndex);
        CONSTRAIN_MAX(State->FirstVisible, State->MaxFirstVisible);
        if ((State->CurrentSelection > State->LastVisible) &&
            (State->CurrentSelection <= State->MaxScroll)){
          State->PaintAll = TRUE;
          State->FirstVisible+= State->MaxVisible;
          CONSTRAIN_MAX(State->FirstVisible, State->MaxFirstVisible);
        }
        
      }
      break;
      
    case SCROLL_FIRST:
      if (State->CurrentSelection > 0) {
        State->CurrentSelection = 0;
        if (State->FirstVisible > 0) {
          State->PaintAll = TRUE;
          State->FirstVisible = 0;
        }
      }
      break;
      
    case SCROLL_LAST:
      if (State->CurrentSelection < State->MaxIndex) {
        State->CurrentSelection = State->MaxIndex;
        if (State->FirstVisible < State->MaxFirstVisible) {
          State->PaintAll = TRUE;
          State->FirstVisible = State->MaxFirstVisible;
        }
      }
      break;
      
    case SCROLL_NONE:
      // The caller has already updated CurrentSelection, but we may
      // have to scroll to make it visible.
      if (State->CurrentSelection < State->FirstVisible) {
        State->PaintAll = TRUE;
        State->FirstVisible = State->CurrentSelection; // - (State->MaxVisible >> 1);
        CONSTRAIN_MIN(State->FirstVisible, 0);
      } else if ((State->CurrentSelection > State->LastVisible) &&
                 (State->CurrentSelection <= State->MaxScroll)) {
        State->PaintAll = TRUE;
        State->FirstVisible = State->CurrentSelection - State->MaxVisible;
        CONSTRAIN_MAX(State->FirstVisible, State->MaxFirstVisible);
      }
      break;
      
  }
  
  if (!State->PaintAll && State->CurrentSelection != State->LastSelection)
    State->PaintSelection = TRUE;
  State->LastVisible = State->FirstVisible + State->MaxVisible;
  DBG("Scroll to: CurrentSelection=%d, FirstVisible=%d, LastVisible=%d, LastSelection=%d\n",
      State->CurrentSelection, State->FirstVisible, State->LastVisible, State->LastSelection);  
  //result 0 0 4 0
}

//
// menu helper functions
//

VOID AddMenuInfoLine(IN REFIT_MENU_SCREEN *Screen, IN CHAR16 *InfoLine)
{
    AddListElement((VOID ***) &(Screen->InfoLines), &(Screen->InfoLineCount), InfoLine);
}

VOID AddMenuEntry(IN REFIT_MENU_SCREEN *Screen, IN REFIT_MENU_ENTRY *Entry)
{
    AddListElement((VOID ***) &(Screen->Entries), &(Screen->EntryCount), Entry);
}

VOID FreeMenu(IN REFIT_MENU_SCREEN *Screen)
{
    if (Screen->Entries)
        FreePool(Screen->Entries);
}

static INTN FindMenuShortcutEntry(IN REFIT_MENU_SCREEN *Screen, IN CHAR16 Shortcut)
{
    UINTN i;
    if (Shortcut >= 'a' && Shortcut <= 'z')
        Shortcut -= ('a' - 'A');
    if (Shortcut) {
        for (i = 0; i < Screen->EntryCount; i++) {
            if (Screen->Entries[i]->ShortcutDigit == Shortcut ||
                Screen->Entries[i]->ShortcutLetter == Shortcut) {
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
static UINTN InputDialog(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN MENU_STYLE_FUNC  StyleFunc)
{
	EFI_STATUS    Status;
	EFI_INPUT_KEY key;
	UINTN         ind = 0;
	UINTN         i = 0;
	UINTN         MenuExit = 0;
	UINTN         LogSize;
  UINTN         Pos = (Screen->Entries[State->CurrentSelection])->Row;
  INPUT_ITEM    *Item = ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item;
  CHAR16        *Backup = EfiStrDuplicate(Item->SValue);
  CHAR16        *Buffer = Item->SValue;
  CHAR16        *TempString = AllocateZeroPool(255);
  
	
	do {
    if (Item->ItemType == BoolValue) {
      Item->BValue = !Item->BValue;
      Item->SValue = Item->BValue?L"[X]":L"[ ]";
      MenuExit = MENU_EXIT_ENTER;
    } else {
    
      Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
      if (Status == EFI_NOT_READY) {
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &ind);
        continue;
      }
      
      switch (key.ScanCode) {
        case SCAN_RIGHT:
          if (Pos < StrLen(Buffer)) Pos++;
          break;
        case SCAN_LEFT:
          if (Pos>0) Pos--;
          break;
        case SCAN_HOME:
          Pos = 0;
          break;
        case SCAN_END:
          Pos = StrLen(Buffer);
          break;
        case SCAN_ESC:
          MenuExit = MENU_EXIT_ESCAPE;
          continue;
          break;
        case SCAN_F2:
          LogSize = msgCursor - msgbuf;
          Status = egSaveFile(SelfRootDir, L"EFI\\misc\\preboot.log", (UINT8*)msgbuf, LogSize);
          break;
        case SCAN_F10:
          egScreenShot();
          break;
      }
      
      switch (key.UnicodeChar) {
        case CHAR_BACKSPACE:  
          if (Buffer[0] != CHAR_NULL && Pos != 0) {
            for (i = 0; i < Pos - 1; i++) {
              TempString[i] = Buffer[i];
            }           
            for (i = Pos - 1; i < StrLen(Buffer); i++) {
              TempString[i] = Buffer[i+1];
            }
            TempString[i] = CHAR_NULL;
            StrCpy (Buffer, TempString);
            Pos--;
          }
          
          break;
          
        case CHAR_LINEFEED:
        case CHAR_CARRIAGE_RETURN:
          MenuExit = MENU_EXIT_ENTER;
          break;
        default:
          if (Pos < 254) {
            for (i = 0; i < Pos; i++) {
              TempString[i] = Buffer[i];
            }           
            TempString[Pos++] = key.UnicodeChar;
            for (i = Pos; i < StrLen(Buffer); i++) {
              TempString[i] = Buffer[i-1];
            }           
            StrCpy (Buffer, TempString);
          }
          Buffer[Pos++] = key.UnicodeChar;
          Buffer[i] = '\0';
          break;
      }
    }
    (Screen->Entries[State->CurrentSelection])->Row = Pos;
    StyleFunc(Screen, State, MENU_FUNCTION_PAINT_SELECTION, NULL);
	} while (!MenuExit);
	switch (MenuExit) {
		case MENU_EXIT_ENTER:
      Item->Valid = TRUE;      
			break;
		case MENU_EXIT_ESCAPE:
			Item->Valid = FALSE;
      Item->SValue = EfiStrDuplicate(Backup);
      StyleFunc(Screen, State, MENU_FUNCTION_PAINT_SELECTION, NULL);
			break;
	}
  FreePool(TempString);
	
  return 0;
}

static UINTN RunGenericMenu(IN REFIT_MENU_SCREEN *Screen, IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_MENU_ENTRY **ChosenEntry)
{
    SCROLL_STATE  State;
    EFI_STATUS    Status;
    EFI_INPUT_KEY key;
    UINTN         index;
    INTN          ShortcutEntry;
    BOOLEAN       HaveTimeout = FALSE;
    UINTN         TimeoutCountdown = 0;
    CHAR16        *TimeoutMessage;
    UINTN         MenuExit;
    UINTN         LogSize;
  
    //no default - no timeout!
    if ((*DefaultEntryIndex != -1) && (Screen->TimeoutSeconds > 0)) {
        HaveTimeout = TRUE;
        TimeoutCountdown = Screen->TimeoutSeconds * 10;
    }
    MenuExit = 0;
    
    StyleFunc(Screen, &State, MENU_FUNCTION_INIT, NULL);
    // override the starting selection with the default index, if any
    if (*DefaultEntryIndex >= 0 && *DefaultEntryIndex <= State.MaxIndex) {
        State.CurrentSelection = *DefaultEntryIndex;
        UpdateScroll(&State, SCROLL_NONE);
    }
    
  while (!MenuExit) {
    // update the screen
    if (State.PaintAll) {
      StyleFunc(Screen, &State, MENU_FUNCTION_PAINT_ALL, NULL);
      State.PaintAll = FALSE;
    } else if (State.PaintSelection) {
      StyleFunc(Screen, &State, MENU_FUNCTION_PAINT_SELECTION, NULL);
      State.PaintSelection = FALSE;
    }
    
    if (HaveTimeout) {
      TimeoutMessage = PoolPrint(L"%s in %d seconds", Screen->TimeoutText, (TimeoutCountdown + 5) / 10);
      StyleFunc(Screen, &State, MENU_FUNCTION_PAINT_TIMEOUT, TimeoutMessage);
      FreePool(TimeoutMessage);
    }
    
    // read key press (and wait for it if applicable)
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
    if (Status == EFI_NOT_READY) {
      if (HaveTimeout && TimeoutCountdown == 0) {
        // timeout expired
        MenuExit = MENU_EXIT_TIMEOUT;
        break;
      } else if (HaveTimeout) {
        gBS->Stall(100000);
        TimeoutCountdown--;
      } else
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
      continue;
    }
    if (HaveTimeout) {
      // the user pressed a key, cancel the timeout
      StyleFunc(Screen, &State, MENU_FUNCTION_PAINT_TIMEOUT, L"");
      HaveTimeout = FALSE;
    }
    
    // react to key press
    switch (key.ScanCode) {
      case SCAN_UP:
      case SCAN_LEFT:
        UpdateScroll(&State, SCROLL_LINE_UP);
        break;
      case SCAN_DOWN:
      case SCAN_RIGHT:
        UpdateScroll(&State, SCROLL_LINE_DOWN);
        break;
      case SCAN_HOME:
        UpdateScroll(&State, SCROLL_FIRST);
        break;
      case SCAN_END:
        UpdateScroll(&State, SCROLL_LAST);
        break;
      case SCAN_PAGE_UP:
        UpdateScroll(&State, SCROLL_PAGE_UP);
        break;
      case SCAN_PAGE_DOWN:
        UpdateScroll(&State, SCROLL_PAGE_DOWN);
        break;
      case SCAN_ESC:
        MenuExit = MENU_EXIT_ESCAPE;
        break;
      case SCAN_INSERT:
        MenuExit = MENU_EXIT_OPTIONS;
        break;
        
      case SCAN_F2:
        LogSize = msgCursor - msgbuf;
        Status = egSaveFile(SelfRootDir, L"EFI\\misc\\preboot.log", (UINT8*)msgbuf, LogSize);
        if (EFI_ERROR(Status)) {
          Status = egSaveFile(NULL, L"EFI\\misc\\preboot.log", (UINT8*)msgbuf, LogSize);
        }
        break;
      case SCAN_F10:
        egScreenShot();
        break;
      case SCAN_F12:
        MenuExit = MENU_EXIT_EJECT;
        State.PaintAll = TRUE;
        break;
        
    }
    switch (key.UnicodeChar) {
      case CHAR_LINEFEED:
      case CHAR_CARRIAGE_RETURN:
        
        if (Screen->Entries[State.CurrentSelection]->Tag == TAG_INPUT)
          MenuExit = InputDialog(Screen, &State, StyleFunc);
                      //((REFIT_INPUT_DIALOG*)(Screen->Entries[State.CurrentSelection]))->Value);
        else 
          MenuExit = MENU_EXIT_ENTER;
        break;
      case ' ':
        MenuExit = MENU_EXIT_DETAILS;
        break;
        
      default:
        ShortcutEntry = FindMenuShortcutEntry(Screen, key.UnicodeChar);
        if (ShortcutEntry >= 0) {
          State.CurrentSelection = ShortcutEntry;
          MenuExit = MENU_EXIT_ENTER;
        }
        break;
    }
  }
  
  StyleFunc(Screen, &State, MENU_FUNCTION_CLEANUP, NULL);
    
  if (ChosenEntry)
    *ChosenEntry = Screen->Entries[State.CurrentSelection];
  *DefaultEntryIndex = State.CurrentSelection;
  return MenuExit;
}

//
// text-mode generic style
//

static VOID TextMenuStyle(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText)
{
    INTN i;
    UINTN MenuWidth, ItemWidth, MenuHeight;
    static UINTN MenuPosY;
    static CHAR16 **DisplayStrings;
    CHAR16 *TimeoutMessage;
    
    switch (Function) {
        
        case MENU_FUNCTION_INIT:
            // vertical layout
            MenuPosY = 4;
            if (Screen->InfoLineCount > 0)
                MenuPosY += Screen->InfoLineCount + 1;
            MenuHeight = ConHeight - MenuPosY;
            if (Screen->TimeoutSeconds > 0)
                MenuHeight -= 2;
            InitScroll(State, Screen->EntryCount, Screen->EntryCount, MenuHeight);
            
            // determine width of the menu
            MenuWidth = 20;  // minimum
            for (i = 0; i <= State->MaxIndex; i++) {
                ItemWidth = StrLen(Screen->Entries[i]->Title);
                if (MenuWidth < ItemWidth)
                    MenuWidth = ItemWidth;
            }
            if (MenuWidth > ConWidth - 6)
                MenuWidth = ConWidth - 6;
            
            // prepare strings for display
            DisplayStrings = AllocatePool(sizeof(CHAR16 *) * Screen->EntryCount);
            for (i = 0; i <= State->MaxIndex; i++)
                DisplayStrings[i] = PoolPrint(L" %-.*s ", MenuWidth, Screen->Entries[i]->Title);
            // TODO: shorten strings that are too long (PoolPrint doesn't do that...)
            // TODO: use more elaborate techniques for shortening too long strings (ellipses in the middle)
            // TODO: account for double-width characters
                
            // initial painting
            BeginTextScreen(Screen->Title);
            if (Screen->InfoLineCount > 0) {
                gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
                for (i = 0; i < (INTN)Screen->InfoLineCount; i++) {
                    gST->ConOut->SetCursorPosition (gST->ConOut, 3, 4 + i);
                    gST->ConOut->OutputString (gST->ConOut, Screen->InfoLines[i]);
                }
            }
            
            break;
            
        case MENU_FUNCTION_CLEANUP:
            // release temporary memory
            for (i = 0; i <= State->MaxIndex; i++)
                FreePool(DisplayStrings[i]);
            FreePool(DisplayStrings);
            break;
            
        case MENU_FUNCTION_PAINT_ALL:
            // paint the whole screen (initially and after scrolling)
            for (i = 0; i <= State->MaxIndex; i++) {
                if (i >= State->FirstVisible && i <= State->LastVisible) {
                    gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (i - State->FirstVisible));
                    if (i == State->CurrentSelection)
                        gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_CURRENT);
                    else
                        gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_BASIC);
                    gST->ConOut->OutputString (gST->ConOut, DisplayStrings[i]);
                }
            }
            // scrolling indicators
            gST->ConOut->SetAttribute (gST->ConOut, ATTR_SCROLLARROW);
            gST->ConOut->SetCursorPosition (gST->ConOut, 0, MenuPosY);
            if (State->FirstVisible > 0)
                gST->ConOut->OutputString (gST->ConOut, ArrowUp);
            else
                gST->ConOut->OutputString (gST->ConOut, L" ");
            gST->ConOut->SetCursorPosition (gST->ConOut, 0, MenuPosY + State->MaxVisible);
            if (State->LastVisible < State->MaxIndex)
                gST->ConOut->OutputString (gST->ConOut, ArrowDown);
            else
                gST->ConOut->OutputString (gST->ConOut, L" ");
            break;
            
        case MENU_FUNCTION_PAINT_SELECTION:
            // redraw selection cursor
            gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (State->LastSelection - State->FirstVisible));
            gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_BASIC);
            gST->ConOut->OutputString (gST->ConOut, DisplayStrings[State->LastSelection]);
            gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (State->CurrentSelection - State->FirstVisible));
            gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_CURRENT);
            gST->ConOut->OutputString (gST->ConOut, DisplayStrings[State->CurrentSelection]);
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

//
// graphical generic style
//

VOID DrawMenuText(IN CHAR16 *Text, IN UINTN SelectedWidth, IN UINTN XPos, IN UINTN YPos, IN UINTN Cursor)
{
    if (TextBuffer == NULL)
        TextBuffer = egCreateImage(LAYOUT_TEXT_WIDTH, TextHeight, FALSE);
    
    egFillImage(TextBuffer, &MenuBackgroundPixel);
    if (SelectedWidth > 0) {
        // draw selection bar background
        egFillImageArea(TextBuffer, 0, 0, SelectedWidth, TextBuffer->Height,
                        &SelectionBackgroundPixel);
    }
    
    // render the text
    egRenderText(Text, TextBuffer, TEXT_XMARGIN, TEXT_YMARGIN, Cursor);
    BltImage(TextBuffer, XPos, YPos);
}

static UINTN MenuWidth, EntriesPosX, EntriesPosY, TimeoutPosY;

static VOID GraphicsMenuStyle(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText)
{
    INTN i;
    UINTN ItemWidth = 0;

    UINTN VisibleHeight = 0; //assume vertical layout
    
    switch (Function) {
        
        case MENU_FUNCTION_INIT:
        // TODO: calculate available screen space
        //
       
            EntriesPosY = ((UGAHeight - LAYOUT_TOTAL_HEIGHT) >> 1) + LAYOUT_BANNER_YOFFSET + TextHeight * 2;
            VisibleHeight = LAYOUT_TOTAL_HEIGHT / TextHeight;
        
            InitScroll(State, Screen->EntryCount, Screen->EntryCount, VisibleHeight);              
            // determine width of the menu
            MenuWidth = 20;  // minimum
            for (i = 0; i < (INTN)Screen->InfoLineCount; i++) {
                ItemWidth = StrLen(Screen->InfoLines[i]);
                if (MenuWidth < ItemWidth)
                    MenuWidth = ItemWidth;
            }
            for (i = 0; i <= State->MaxIndex; i++) {
                ItemWidth = StrLen(Screen->Entries[i]->Title) + 
                    StrLen(((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->SValue);
                if (MenuWidth < ItemWidth)
                    MenuWidth = ItemWidth;
            }
            MenuWidth = TEXT_XMARGIN * 2 + MenuWidth * GlobalConfig.CharWidth; // FontWidth;
            if (MenuWidth > LAYOUT_TEXT_WIDTH)
                MenuWidth = LAYOUT_TEXT_WIDTH;
            
            if (Screen->TitleImage)
                EntriesPosX = (UGAWidth + (Screen->TitleImage->Width + TITLEICON_SPACING) - MenuWidth) >> 1;
            else
                EntriesPosX = (UGAWidth - MenuWidth) >> 1;
         //   EntriesPosY = ((UGAHeight - LAYOUT_TOTAL_HEIGHT) >> 1) + LAYOUT_BANNER_YOFFSET + TextHeight * 2;
            TimeoutPosY = EntriesPosY + (Screen->EntryCount + 1) * TextHeight;
            
            // initial painting
            SwitchToGraphicsAndClear();
            egMeasureText(Screen->Title, &ItemWidth, NULL);
            DrawMenuText(Screen->Title, 0, ((UGAWidth - ItemWidth) >> 1) - TEXT_XMARGIN, EntriesPosY - TextHeight * 2, 0xFFFF);
            if (Screen->TitleImage)
                BltImageAlpha(Screen->TitleImage,
                              EntriesPosX - (Screen->TitleImage->Width + TITLEICON_SPACING), EntriesPosY,
                              &MenuBackgroundPixel);
            if (Screen->InfoLineCount > 0) {
                for (i = 0; i < (INTN)Screen->InfoLineCount; i++) {
                    DrawMenuText(Screen->InfoLines[i], 0, EntriesPosX, EntriesPosY, 0xFFFF);
                    EntriesPosY += TextHeight;
                }
                EntriesPosY += TextHeight;  // also add a blank line
            }
            
            break;
            
        case MENU_FUNCTION_CLEANUP:
            // nothing to do
            break;
            
        case MENU_FUNCTION_PAINT_ALL:
            for (i = 0; i <= State->MaxIndex; i++) {
              if (Screen->Entries[i]->Tag == TAG_INPUT) {
                CHAR16 ResultString[255];
                StrCpy(ResultString, Screen->Entries[i]->Title);
                StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->SValue);
                StrCat(ResultString, L" ");
                //Slice - suppose to use Row as Cursor in text
                DrawMenuText(ResultString, (i == State->CurrentSelection) ? MenuWidth : 0,
                             EntriesPosX, EntriesPosY + i * TextHeight, Screen->Entries[i]->Row);
              }
              else
                DrawMenuText(Screen->Entries[i]->Title, (i == State->CurrentSelection) ? MenuWidth : 0,
                             EntriesPosX, EntriesPosY + i * TextHeight, 0xFFFF);
            }
            break;
            
        case MENU_FUNCTION_PAINT_SELECTION:
            // redraw selection cursor
        //Last selection
        if (Screen->Entries[State->LastSelection]->Tag == TAG_INPUT) {
          CHAR16 ResultString[255];
          StrCpy(ResultString, Screen->Entries[State->LastSelection]->Title);
          StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->SValue);
          StrCat(ResultString, L" ");
          DrawMenuText(ResultString, 0,
                       EntriesPosX, EntriesPosY + State->LastSelection * TextHeight,
                       Screen->Entries[State->LastSelection]->Row);
        }
        else {
            DrawMenuText(Screen->Entries[State->LastSelection]->Title, 0,
                         EntriesPosX, EntriesPosY + State->LastSelection * TextHeight, 0xFFFF);
        }
        
        
        //Current selection
        if (Screen->Entries[State->CurrentSelection]->Tag == TAG_INPUT) {
          CHAR16 ResultString[255];
          StrCpy(ResultString, Screen->Entries[State->CurrentSelection]->Title);
          StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->SValue);
          StrCat(ResultString, L" ");
          DrawMenuText(ResultString, MenuWidth,
                       EntriesPosX, EntriesPosY + State->CurrentSelection * TextHeight,
                       Screen->Entries[State->CurrentSelection]->Row);
        }
        else {
            DrawMenuText(Screen->Entries[State->CurrentSelection]->Title, MenuWidth,
                         EntriesPosX, EntriesPosY + State->CurrentSelection * TextHeight, 0xFFFF);
        }

            break;
            
        case MENU_FUNCTION_PAINT_TIMEOUT:
            DrawMenuText(ParamText, 0, EntriesPosX, TimeoutPosY, 0xFFFF);
            break;
            
    }
}

//
// graphical main menu style
//

static VOID DrawMainMenuEntry(REFIT_MENU_ENTRY *Entry, BOOLEAN selected, UINTN XPos, UINTN YPos)
{
  LOADER_ENTRY* LEntry = (LOADER_ENTRY*)Entry;
  EG_IMAGE* MainImage;
  if (LEntry->Volume) {
    MainImage = LEntry->Volume->DriveImage;
  } else {
    MainImage = Entry->Image;
  }
  if (!MainImage) {
    MainImage = LoadIcns(ThemeDir, L"icons\\osx.icns", 128);
  }
  
    BltImageCompositeBadge(SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
                           MainImage, Entry->BadgeImage, XPos, YPos);
}

static VOID DrawMainMenuText(IN CHAR16 *Text, IN UINTN XPos, IN UINTN YPos)
{
    UINTN TextWidth;
    
    if (TextBuffer == NULL)
        TextBuffer = egCreateImage(LAYOUT_TEXT_WIDTH, TextHeight, FALSE);
    
    egFillImage(TextBuffer, &MenuBackgroundPixel);
    
    // render the text
    egMeasureText(Text, &TextWidth, NULL);
    egRenderText(Text, TextBuffer, (TextBuffer->Width - TextWidth) >> 1, 0, 0xFFFF);
    BltImage(TextBuffer, XPos, YPos);
}

static VOID MainMenuStyle(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText)
{
  INTN i; 
  
  switch (Function) {
      
    case MENU_FUNCTION_INIT:
      MaxItemOnScreen = (UGAWidth - ROW0_SCROLLSIZE * 2) / (ROW0_TILESIZE + TILE_XSPACING); //8
      row0PosX = 0;
      row1PosX = Screen->EntryCount;
      // layout
      row0Count = 0; //Nr items in row0
      row1Count = 0;
      for (i = 0; i < Screen->EntryCount; i++) {
        if (Screen->Entries[i]->Row == 0) {
          row0Count++;
          CONSTRAIN_MIN(row0PosX, i);
        } else {
          row1Count++;
          CONSTRAIN_MAX(row1PosX, i);
        }
      }
      if (row0PosX > row1PosX) { //9<10
        MsgLog("BUG! (index_row0 > index_row1) Needed sorting\n");
      }
      InitScroll(State, row0Count, Screen->EntryCount, MaxItemOnScreen);
      row0PosX = (UGAWidth + TILE_XSPACING - (ROW0_TILESIZE + TILE_XSPACING) *
                  ((MaxItemOnScreen < row0Count)?MaxItemOnScreen:row0Count)) >> 1;
      row0PosY = ((UGAHeight - LAYOUT_TOTAL_HEIGHT) >> 1) + LAYOUT_BANNER_YOFFSET;
      row1PosX = (UGAWidth + TILE_XSPACING - (ROW1_TILESIZE + TILE_XSPACING) * row1Count) >> 1;
      row1PosY = row0PosY + ROW0_TILESIZE + TILE_YSPACING;
      if (row1Count > 0)
        textPosY = row1PosY + ROW1_TILESIZE + TILE_YSPACING;
      else
        textPosY = row1PosY;
      
      if (!itemPosX) {
        itemPosX = AllocatePool(sizeof(UINTN) * Screen->EntryCount);
      }
      
      row0PosXRunning = row0PosX;
      row1PosXRunning = row1PosX;
      for (i = 0; i <= Screen->EntryCount; i++) {
        if (Screen->Entries[i]->Row == 0) {
          itemPosX[i] = row0PosXRunning;
          row0PosXRunning += ROW0_TILESIZE + TILE_XSPACING;
        } else {
          itemPosX[i] = row1PosXRunning;
          row1PosXRunning += ROW1_TILESIZE + TILE_XSPACING;
        }
      }
      
      // initial painting
      //InitSelection(); //Slice - I changed order because of background pixel
      SwitchToGraphicsAndClear();
      InitSelection();
      break;
      
    case MENU_FUNCTION_CLEANUP:
      FreePool(itemPosX);
      break;
      
    case MENU_FUNCTION_PAINT_ALL:
       for (i = 0; i <= State->MaxIndex; i++) {
        if (Screen->Entries[i]->Row == 0) {
          if ((i >= State->FirstVisible) && (i <= State->LastVisible)) {
            DBG("Draw at x=%d y=%d\n", itemPosX[i - State->FirstVisible], row0PosY);
            DrawMainMenuEntry(Screen->Entries[i], (i == State->CurrentSelection),
                              itemPosX[i - State->FirstVisible], row0PosY);
          }
        } else {
          DrawMainMenuEntry(Screen->Entries[i], (i == State->CurrentSelection),
                            itemPosX[i], row1PosY);
        }
      }
      
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL))
        DrawMainMenuText(Screen->Entries[State->CurrentSelection]->Title,
                         (UGAWidth - LAYOUT_TEXT_WIDTH) >> 1, textPosY);
      break;
      
    case MENU_FUNCTION_PAINT_SELECTION:
      if ((Screen->Entries[State->LastSelection]->Row == 0)) {
        DrawMainMenuEntry(Screen->Entries[State->LastSelection], FALSE,
                      itemPosX[State->LastSelection - State->FirstVisible], row0PosY);
      } else {
        DrawMainMenuEntry(Screen->Entries[State->LastSelection], FALSE,
                          itemPosX[State->LastSelection], row1PosY);
      }
      
      if (Screen->Entries[State->CurrentSelection]->Row == 0) {
        DrawMainMenuEntry(Screen->Entries[State->CurrentSelection], TRUE,
                      itemPosX[State->CurrentSelection - State->FirstVisible], row0PosY);
      } else {
        DrawMainMenuEntry(Screen->Entries[State->CurrentSelection], TRUE,
                          itemPosX[State->CurrentSelection], row1PosY);
      }

      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL))
        DrawMainMenuText(Screen->Entries[State->CurrentSelection]->Title,
                         (UGAWidth - LAYOUT_TEXT_WIDTH) >> 1, textPosY);
      break;
      
    case MENU_FUNCTION_PAINT_TIMEOUT:
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        //screen centering
        UINTN TextLen = StrLen(ParamText) * GlobalConfig.CharWidth;
        DrawMainMenuText(ParamText, (UGAWidth - TextLen) >> 1, textPosY + TextHeight);
      }
      break;
      
  }
}

//
// user-callable dispatcher functions
//

UINTN RunMenu(IN REFIT_MENU_SCREEN *Screen, OUT REFIT_MENU_ENTRY **ChosenEntry)
{
    INTN index = -1;
    MENU_STYLE_FUNC Style = TextMenuStyle;
    
    if (AllowGraphicsMode)
        Style = GraphicsMenuStyle;
    
    return RunGenericMenu(Screen, Style, &index, ChosenEntry);
}

UINTN RunMainMenu(IN REFIT_MENU_SCREEN *Screen, IN INTN DefaultSelection, OUT REFIT_MENU_ENTRY **ChosenEntry)
{
    MENU_STYLE_FUNC Style = TextMenuStyle;
    MENU_STYLE_FUNC MainStyle = TextMenuStyle;
    REFIT_MENU_ENTRY *TempChosenEntry;
    UINTN MenuExit = 0;
    INTN DefaultEntryIndex = DefaultSelection;
    INTN SubMenuIndex;
    
    if (AllowGraphicsMode) {
        Style = GraphicsMenuStyle;
        MainStyle = MainMenuStyle;
    }
    
    while (!MenuExit) {
        MenuExit = RunGenericMenu(Screen, MainStyle, &DefaultEntryIndex, &TempChosenEntry);
        Screen->TimeoutSeconds = 0;
        
        if (MenuExit == MENU_EXIT_DETAILS && TempChosenEntry->SubScreen != NULL) {
            SubMenuIndex = -1;
            MenuExit = RunGenericMenu(TempChosenEntry->SubScreen, Style, &SubMenuIndex, &TempChosenEntry);
            if (MenuExit == MENU_EXIT_ESCAPE || TempChosenEntry->Tag == TAG_RETURN)
                MenuExit = 0;
        }
    }
    
    if (ChosenEntry)
        *ChosenEntry = TempChosenEntry;
    return MenuExit;
}

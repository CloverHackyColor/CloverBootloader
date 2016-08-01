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
#include "libegint.h"   //this includes platform.h
#include "../include/scroll_images.h"
#include "Version.h"

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


//#define PREBOOT_LOG L"EFI\\CLOVER\\misc\\preboot.log"
#define VBIOS_BIN L"EFI\\CLOVER\\misc\\c0000.bin"

//#define LSTR(s) L##s

// scrolling definitions
static INTN MaxItemOnScreen = -1;
REFIT_MENU_SCREEN OptionMenu  = {4, L"Options", NULL, 0, NULL, 0, NULL, 0, NULL, NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL };
extern REFIT_MENU_ENTRY MenuEntryReturn;
extern UINTN            ThemesNum;
extern CHAR16           *ThemesList[];
extern CHAR8            *NonDetected;
extern BOOLEAN          GetLegacyLanAddress;
extern UINT8            gLanMac[4][6]; // their MAC addresses

INTN LayoutBannerOffset = 64;
INTN LayoutButtonOffset = 0;
INTN LayoutTextOffset = 0;
INTN LayoutMainMenuHeight = 376;
INTN LayoutAnimMoveForMenuX = 0;
BOOLEAN SavePreBootLog = FALSE;

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


#define TEXT_CORNER_REVISION  (1)
#define TEXT_CORNER_HELP      (2)

// other menu definitions

#define MENU_FUNCTION_INIT            (0)
#define MENU_FUNCTION_CLEANUP         (1)
#define MENU_FUNCTION_PAINT_ALL       (2)
#define MENU_FUNCTION_PAINT_SELECTION (3)
#define MENU_FUNCTION_PAINT_TIMEOUT   (4)

typedef VOID (*MENU_STYLE_FUNC)(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText);

static CHAR16 ArrowUp[2]   = { ARROW_UP, 0 };
static CHAR16 ArrowDown[2] = { ARROW_DOWN, 0 };

BOOLEAN MainAnime = FALSE;

BOOLEAN ScrollEnabled = FALSE;
BOOLEAN IsDragging = FALSE;

INTN ScrollWidth = 16;
INTN ScrollButtonsHeight = 20;
INTN ScrollBarDecorationsHeight = 5;
INTN ScrollScrollDecorationsHeight = 7;
INTN ScrollbarYMovement;


//#define TextHeight (FONT_CELL_HEIGHT + TEXT_YMARGIN * 2)
#define TITLEICON_SPACING (16)

//#define ROW0__TILESIZE (144)
#define ROW1_TILESIZE (64)
#define TILE_XSPACING (8)
#define TILE_YSPACING (24)
#define ROW0_SCROLLSIZE (100)
#define INDICATOR_SIZE (52)

EG_IMAGE *SelectionImages[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
EG_IMAGE *Buttons[4] = {NULL, NULL, NULL, NULL};
static EG_IMAGE *TextBuffer = NULL;

EG_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff }; //non-trasparent

INTN row0TileSize = 144;
INTN row1TileSize = 64;

static INTN row0Count, row0PosX, row0PosXRunning;
static INTN row1Count, row1PosX, row1PosXRunning;
static INTN *itemPosX = NULL;
static INTN *itemPosY = NULL;
static INTN row0PosY, row1PosY, textPosY, FunctextPosY;
static EG_IMAGE* MainImage;
static INTN OldX = 0, OldY = 0;
static INTN OldTextWidth = 0;
static UINTN OldRow = 0;
static INTN OldTimeoutTextWidth = 0;
static INTN MenuWidth, EntriesPosX, EntriesPosY, TimeoutPosY;
static INTN EntriesWidth, EntriesHeight, EntriesGap;
static EG_IMAGE* ScrollbarImage = NULL;
static EG_IMAGE* ScrollbarBackgroundImage = NULL;
static EG_IMAGE* UpButtonImage = NULL;
static EG_IMAGE* DownButtonImage = NULL;
static EG_IMAGE* BarStartImage = NULL;
static EG_IMAGE* BarEndImage = NULL;
static EG_IMAGE* ScrollStartImage = NULL;
static EG_IMAGE* ScrollEndImage = NULL;
static EG_RECT BarStart;
static EG_RECT BarEnd;
static EG_RECT ScrollStart;
static EG_RECT ScrollEnd;
static EG_RECT ScrollTotal;
EG_RECT UpButton;
EG_RECT DownButton;
EG_RECT ScrollbarBackground;
EG_RECT Scrollbar;
EG_RECT ScrollbarOldPointerPlace;
EG_RECT ScrollbarNewPointerPlace;


INPUT_ITEM *InputItems = NULL;
UINTN  InputItemsCount = 0;

INTN OldChosenTheme;
//INTN NewChosenTheme;

BOOLEAN mGuiReady = FALSE;

#if defined(ADVICON)
REFIT_MENU_ENTRY MenuEntryOptions  = { L"Options", TAG_OPTIONS, 1, 0, 'O', NULL, NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone, NULL };
REFIT_MENU_ENTRY MenuEntryAbout    = { L"About Clover", TAG_ABOUT, 1, 0, 'A', NULL, NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryReset    = { L"Restart Computer", TAG_RESET, 1, 0, 'R', NULL, NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionSelect, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryShutdown = { L"Exit Clover", TAG_SHUTDOWN, 1, 0, 'U', NULL, NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionSelect, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryReturn   = { L"Return", TAG_RETURN, 0, 0, 0, NULL, NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryHelp    = { L"Help", TAG_HELP, 1, 0, 'H', NULL, NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionSelect, ActionEnter, ActionNone, ActionNone,  NULL };
#else //ADVICON
REFIT_MENU_ENTRY MenuEntryOptions  = { L"Options", TAG_OPTIONS, 1, 0, 'O', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone, NULL };
REFIT_MENU_ENTRY MenuEntryAbout    = { L"About Clover", TAG_ABOUT, 1, 0, 'A', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryReset    = { L"Restart Computer", TAG_RESET, 1, 0, 'R', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionSelect, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryShutdown = { L"Exit Clover", TAG_SHUTDOWN, 1, 0, 'U', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionSelect, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryReturn   = { L"Return", TAG_RETURN, 0, 0, 0, NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryHelp    = { L"Help", TAG_HELP, 1, 0, 'H', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionSelect, ActionEnter, ActionNone, ActionNone,  NULL };
#endif //ADVICON

REFIT_MENU_SCREEN MainMenu    = {1, L"Main Menu", NULL, 0, NULL, 0, NULL, 0, L"Automatic boot", NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL};
REFIT_MENU_SCREEN AboutMenu   = {2, L"About",     NULL, 0, NULL, 0, NULL, 0, NULL,              NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL};
REFIT_MENU_SCREEN HelpMenu    = {3, L"Help",      NULL, 0, NULL, 0, NULL, 0, NULL,              NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL};

UINTN RunGenericMenu(IN REFIT_MENU_SCREEN *Screen, IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_MENU_ENTRY **ChosenEntry);


VOID FillInputs(BOOLEAN New)
{
  UINTN i,j; //for loops
  CHAR8 tmp[41];
  BOOLEAN bit;

  tmp[40] = 0;  //make it null-terminated

  InputItemsCount = 0;
  if (New) {
    InputItems = AllocateZeroPool(128 * sizeof(INPUT_ITEM)); //XXX
  }

  InputItems[InputItemsCount].ItemType = ASString;  //0
  //even though Ascii we will keep value as Unicode to convert later
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(SVALUE_MAX_SIZE);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, SVALUE_MAX_SIZE, L"%a ", gSettings.BootArgs);
  InputItems[InputItemsCount].ItemType = UNIString; //1
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(63);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 63, L"%s", gSettings.DsdtName); // 1-> 2
  InputItems[InputItemsCount].ItemType = UNIString; //2
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(63);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 63, L"%s", gSettings.BlockKexts);
/*
  InputItems[InputItemsCount].ItemType = UNIString; //3
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(53);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 53, L"%s",
                (GlobalConfig.Theme == NULL)?L"embedded":GlobalConfig.Theme);
*/
  InputItems[InputItemsCount++].ItemType = RadioSwitch;  //3
//  InputItems[InputItemsCount].SValue   = (InputItems[InputItemsCount].IValue == OldChosenTheme)?L"(*)":L"( )";
//  InputItems[InputItemsCount++].IValue = OldChosenTheme;

  InputItems[InputItemsCount].ItemType = BoolValue; //4
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.DropSSDT;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.DropSSDT?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue;  //5
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.GeneratePStates;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.GeneratePStates?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue;  //6
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.SlpSmiEnable;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.SlpSmiEnable?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = Decimal;  //7
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.PLimitDict);
  InputItems[InputItemsCount].ItemType = Decimal;  //8
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.UnderVoltStep);
  InputItems[InputItemsCount].ItemType = BoolValue; //9
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.GenerateCStates;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.GenerateCStates?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //10
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.EnableC2;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.EnableC2?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //11
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.EnableC4;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.EnableC4?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //12
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.EnableC6;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.EnableC6?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //13
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.EnableISS;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.EnableISS?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = Decimal;  //14
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gSettings.QPI);
  InputItems[InputItemsCount].ItemType = BoolValue; //15
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.PatchNMI;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.PatchNMI?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //16
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.PatchVBios;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.PatchVBios?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = Decimal;  //17
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%08d", gSettings.PlatformFeature);
  InputItems[InputItemsCount].ItemType = Hex;  //18
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(36);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 36, L"0x%X", gSettings.BacklightLevel); // Download-Fritz: There is no GUI element for BacklightLevel; please revise
  InputItems[InputItemsCount].ItemType = Decimal;  //19
  if (gSettings.BusSpeed > 20000) {
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gSettings.BusSpeed);
  } else {
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gCPUStructure.ExternalClock);
  }
  InputItemsCount = 20;
  for (i=0; i<NGFX; i++) {
    InputItems[InputItemsCount].ItemType = ASString;  //20+i*6
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%a", gGraphics[i].Model);

    if (gGraphics[i].Vendor == Ati) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.InjectATI;
      if (GlobalConfig.TextOnly) {
      InputItems[InputItemsCount++].SValue = gSettings.InjectATI?L"[+]":L"[ ]";
      }
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      if (New) {
        InputItems[InputItemsCount].SValue = AllocateZeroPool(20);
      }
      if (StrLen(gSettings.FBName) > 2) { //fool proof: cfg_name is 3 character or more.
        UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%s", gSettings.FBName);
      } else {
        UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%a", gGraphics[i].Config);
      }
    } else if (gGraphics[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.InjectNVidia;
      if (GlobalConfig.TextOnly) {
      InputItems[InputItemsCount++].SValue = gSettings.InjectNVidia?L"[+]":L"[ ]";
      }
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      for (j=0; j<8; j++) {
        AsciiSPrint((CHAR8*)&tmp[2*j], 3, "%02x", gSettings.Dcfg[j]);
      }
      if (New) {
        InputItems[InputItemsCount].SValue = AllocateZeroPool(40);
      }
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 40, L"%a", tmp);

      //InputItems[InputItemsCount++].SValue = PoolPrint(L"%08x",*(UINT64*)&gSettings.Dcfg[0]);
    } else /*if (gGraphics[i].Vendor == Intel) */ {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.InjectIntel;
      if (GlobalConfig.TextOnly) {
      InputItems[InputItemsCount++].SValue = gSettings.InjectIntel?L"[+]":L"[ ]";
      }
      InputItems[InputItemsCount].ItemType = Hex; //22+6i
      InputItems[InputItemsCount++].SValue = PoolPrint(L"%08lx", gSettings.IgPlatform);;
    }

    if (gGraphics[i].Vendor == Intel) {
      InputItemsCount += 3;
      continue;
    }

    InputItems[InputItemsCount].ItemType = Decimal;  //23+6i
    if (gSettings.VideoPorts > 0) {
      InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.VideoPorts);
    } else {
      InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gGraphics[i].Ports);
    }


    InputItems[InputItemsCount].ItemType = ASString; //24+6i
    for (j=0; j<20; j++) {
      AsciiSPrint((CHAR8*)&tmp[2*j], 3, "%02x", gSettings.NVCAP[j]);
    }
    if (New) {
      InputItems[InputItemsCount].SValue = AllocateZeroPool(84);
    }
    UnicodeSPrint(InputItems[InputItemsCount++].SValue, 84, L"%a", tmp);

    InputItems[InputItemsCount].ItemType = BoolValue; //25+6i
    InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gGraphics[i].LoadVBios;
    if (GlobalConfig.TextOnly) {
    InputItems[InputItemsCount++].SValue = gGraphics[i].LoadVBios?L"[+]":L"[ ]";
  }
  }
  //and so on

  InputItemsCount = 43;
    // ErmaC: NvidiaGeneric menu selector y/n
  InputItems[InputItemsCount].ItemType = BoolValue; //26+6i
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.NvidiaGeneric;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.NvidiaGeneric?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //44
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.KextPatchesAllowed;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.KextPatchesAllowed ? L"[+]" : L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //45
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPKernelCpu;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.KernelAndKextPatches.KPKernelCpu ? L"[+]" : L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //46
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPAsusAICPUPM;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.KernelAndKextPatches.KPAsusAICPUPM ? L"[+]" : L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //47
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPAppleRTC;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.KernelAndKextPatches.KPAppleRTC ? L"[+]" : L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //48
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPKernelPm;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.KernelAndKextPatches.KPKernelPm ? L"[+]" : L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //49
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.DropMCFG;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.DropMCFG?L"[+]":L"[ ]";
  }

  InputItems[InputItemsCount].ItemType = Decimal;  //50
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%d", gSettings.RefCLK);

  InputItems[InputItemsCount].ItemType = ASString;  //51
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(SVALUE_MAX_SIZE);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, SVALUE_MAX_SIZE, L"%a ", NonDetected);

  InputItems[InputItemsCount].ItemType = BoolValue; //52
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.InjectEDID;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.InjectEDID?L"[+]":L"[ ]";
  }

  for (j=0; j<16; j++) {
    InputItems[InputItemsCount].ItemType = BoolValue; //53+j
    bit = (gSettings.FixDsdt & (1<<j)) != 0;
    InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = bit;
    if (GlobalConfig.TextOnly) {
    InputItems[InputItemsCount++].SValue = bit?L"[+]":L"[ ]";
  }
  }

  InputItemsCount = 70;
  InputItems[InputItemsCount].ItemType = Decimal;  //70
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.PointerSpeed);
  InputItems[InputItemsCount].ItemType = Decimal;  //71
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%04d", gSettings.DoubleClickTime);
  InputItems[InputItemsCount].ItemType = BoolValue; //72
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.PointerMirror;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.PointerMirror?L"[+]":L"[ ]";
  }
  
  //reserve for mouse and continue

  InputItemsCount = 74;
  InputItems[InputItemsCount].ItemType = BoolValue; //74
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.USBFixOwnership;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.USBFixOwnership?L"[+]":L"[ ]";
  }

  InputItems[InputItemsCount].ItemType = Hex;  //75
  InputItems[InputItemsCount++].SValue = PoolPrint(L"0x%04x", gSettings.C3Latency);
  InputItems[InputItemsCount].ItemType = Decimal;  //76
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.EnabledCores);
  InputItems[InputItemsCount].ItemType = Decimal;  //77
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%d", gSettings.SavingMode);

  InputItems[InputItemsCount].ItemType = ASString;  //78
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.ProductName);
  InputItems[InputItemsCount].ItemType = ASString;  //79
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.VersionNr);
  InputItems[InputItemsCount].ItemType = ASString;  //80
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.SerialNr);
  InputItems[InputItemsCount].ItemType = ASString;  //81
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardNumber);
  InputItems[InputItemsCount].ItemType = ASString;  //82
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardSerialNumber);
  InputItems[InputItemsCount].ItemType = Decimal;  //83
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.BoardType);
  InputItems[InputItemsCount].ItemType = ASString;  //84
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardVersion);
  InputItems[InputItemsCount].ItemType = Decimal;  //85
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.ChassisType);
  InputItems[InputItemsCount].ItemType = ASString;  //86
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.RomVersion);
  InputItems[InputItemsCount].ItemType = ASString;  //87
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.ReleaseDate);

  InputItems[InputItemsCount].ItemType = BoolValue; //88
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue   = gSettings.DoubleFirstState;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.DoubleFirstState?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //89
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.EnableC7;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.EnableC7?L"[+]":L"[ ]";
  }

  InputItems[InputItemsCount].ItemType = UNIString; //90
  if (New) {
    InputItems[InputItemsCount].SValue   = AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%s", gSettings.ConfigName);

  InputItems[InputItemsCount].ItemType = BoolValue; //91
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPLapicPanic;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.KernelAndKextPatches.KPLapicPanic ? L"[+]" : L"[ ]";
  }

  InputItems[InputItemsCount].ItemType = BoolValue; //92
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue   = gSettings.USBInjection;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.USBInjection?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //93
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue   = gSettings.InjectClockID;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.InjectClockID?L"[+]":L"[ ]";
  }

  InputItems[InputItemsCount].ItemType = Hex;  //94
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeATI);
  InputItems[InputItemsCount].ItemType = Hex;  //95
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeNVidia);
  InputItems[InputItemsCount].ItemType = Hex;  //96
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeIntel);

  InputItems[InputItemsCount].ItemType = Hex;  //97
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeLAN);
  InputItems[InputItemsCount].ItemType = Hex;  //98
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeWIFI);
  InputItems[InputItemsCount].ItemType = Hex;  //99
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeSATA);
  InputItems[InputItemsCount].ItemType = Hex;  //100
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeXHCI);
  InputItems[InputItemsCount].ItemType = Hex;  //101
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%04X", dropDSM);
  InputItems[InputItemsCount].ItemType = BoolValue; //102
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.DebugDSDT;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.DebugDSDT?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = Hex;  //103
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeIMEI);
  InputItems[InputItemsCount].ItemType = Hex;  //104
  if (New) {
    InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.KernelAndKextPatches.FakeCPUID);

  InputItems[InputItemsCount].ItemType = BoolValue; //105
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPHaswellE;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.KernelAndKextPatches.KPHaswellE ? L"[+]" : L"[ ]";
  }

  InputItems[InputItemsCount].ItemType = BoolValue; //106
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.StringInjector;
  if (GlobalConfig.TextOnly) {
  InputItems[InputItemsCount++].SValue = gSettings.StringInjector?L"[+]":L"[ ]";
  }
  InputItems[InputItemsCount].ItemType = BoolValue; //107
  InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = gSettings.NoDefaultProperties;
  if (GlobalConfig.TextOnly) {
    InputItems[InputItemsCount].SValue = gSettings.NoDefaultProperties?L"[+]":L"[ ]";
  }

  InputItemsCount = 110;
  for (j=0; j<16; j++) {
    InputItems[InputItemsCount].ItemType = BoolValue; //110+j
    bit = (gSettings.FixDsdt & (1<<(j+16))) != 0;
    InputItems[(GlobalConfig.TextOnly) ? InputItemsCount : InputItemsCount++].BValue = bit;
    if (GlobalConfig.TextOnly) {
    InputItems[InputItemsCount++].SValue = bit?L"[+]":L"[ ]";
  }
  }

  //menu for drop table
  if (gSettings.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
    while (DropTable) {
      DropTable->MenuItem.ItemType = BoolValue;
      if (GlobalConfig.TextOnly) {
      DropTable->MenuItem.SValue = DropTable->MenuItem.BValue?L"[+]":L"[ ]";
      }
      DropTable = DropTable->Next;
    }
  }

  if (ACPIPatchedAML) {
    ACPI_PATCHED_AML *ACPIPatchedAMLTmp = ACPIPatchedAML;
    while (ACPIPatchedAMLTmp) {
      ACPIPatchedAMLTmp->MenuItem.ItemType = BoolValue;
      if (GlobalConfig.TextOnly) {
      ACPIPatchedAMLTmp->MenuItem.SValue = ACPIPatchedAMLTmp->MenuItem.BValue?L"[+]":L"[ ]";
      }
      ACPIPatchedAMLTmp = ACPIPatchedAMLTmp->Next;
    }
  }
}


VOID ApplyInputs(VOID)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  MACHINE_TYPES Model;
  BOOLEAN NeedSave = TRUE;
  INTN i = 0;
  UINTN j;
  UINT32 k;
  CHAR16 *ch;
  CHAR8  AString[256];
  TagPtr dict;
//  DBG("ApplyInputs\n");
  if (InputItems[i].Valid) {
    ZeroMem(&gSettings.BootArgs, 256);
    gBootChanged = TRUE;
    ch = InputItems[i].SValue;
    do {
      if (*ch == L'\\') {
        *ch = L'_';
      }
    } while (*(++ch));

    AsciiSPrint(gSettings.BootArgs, 255, "%s ", InputItems[i].SValue);
  }
  i++; //1
  if (InputItems[i].Valid) {
    UnicodeSPrint(gSettings.DsdtName, 120, L"%s", InputItems[i].SValue);
  }
  i++; //2
  if (InputItems[i].Valid) {
    UnicodeSPrint(gSettings.BlockKexts, 120, L"%s", InputItems[i].SValue);
  }
  i++; //3
  if (InputItems[i].Valid) {
    if (GlobalConfig.Theme) {
      FreePool(GlobalConfig.Theme);
    }
    GlobalConfig.Theme = PoolPrint(L"%s", ThemesList[OldChosenTheme]);

    //will change theme after ESC
    gThemeChanged = TRUE;
    //will change '\' to '_' because of underscore has a problem with some keyboards
/*    ch = GlobalConfig.Theme;
    do {
      if (*ch == L'\\') {
        *ch = L'_';
      }
    } while (*(++ch));
 */
  }
  i++; //4
  if (InputItems[i].Valid) {
    gSettings.DropSSDT = InputItems[i].BValue;
  }
  i++; //5
  if (InputItems[i].Valid) {
    gSettings.GeneratePStates = InputItems[i].BValue;
  }
  i++; //6
  if (InputItems[i].Valid) {
    gSettings.SlpSmiEnable = InputItems[i].BValue;
  }
  i++; //7
  if (InputItems[i].Valid) {
//    DBG("InputItems[i]: %s\n", InputItems[i].SValue);
    gSettings.PLimitDict = (UINT8)(StrDecimalToUintn(InputItems[i].SValue) & 0x7F);
//    DBG("Item 7=PLimitDict %d\n", gSettings.PLimitDict);
 }
  i++; //8
  if (InputItems[i].Valid) {
    gSettings.UnderVoltStep = (UINT8)(StrDecimalToUintn(InputItems[i].SValue) & 0x3F);
//    DBG("Item 8=UnderVoltStep %d\n", gSettings.UnderVoltStep);
  }
  i++; //9
  if (InputItems[i].Valid) {
    gSettings.GenerateCStates = InputItems[i].BValue;
  }
  i++; //10
  if (InputItems[i].Valid) {
    gSettings.EnableC2 = InputItems[i].BValue;
  }
  i++; //11
  if (InputItems[i].Valid) {
    gSettings.EnableC4 = InputItems[i].BValue;
  }
  i++; //12
  if (InputItems[i].Valid) {
    gSettings.EnableC6 = InputItems[i].BValue;
  }
  i++; //13
  if (InputItems[i].Valid) {
    gSettings.EnableISS = InputItems[i].BValue;
  }
  i++; //14
  if (InputItems[i].Valid) {
    gSettings.QPI = (UINT16)StrDecimalToUintn(InputItems[i].SValue);
    //DBG("Apply ProcessorInterconnectSpeed=%d\n", gSettings.QPI);
  }
  i++; //15
  if (InputItems[i].Valid) {
    gSettings.PatchNMI = InputItems[i].BValue;
  }
  i++; //16
  if (InputItems[i].Valid) {
    gSettings.PatchVBios = InputItems[i].BValue;
  }
  i++; //17
  if (InputItems[i].Valid) {
    gSettings.PlatformFeature = (UINT64)StrDecimalToUintn(InputItems[i].SValue);
    DBG("Apply PlatformFeature=%d\n", gSettings.PlatformFeature);
  }
  i++; //18 | Download-Fritz: There is no GUI element for BacklightLevel; please revise
  if (InputItems[i].Valid) {
    gSettings.BacklightLevel = (UINT16)StrHexToUint64(InputItems[i].SValue);
    gSettings.BacklightLevelConfig = TRUE;
  }
  i++; //19
  if (InputItems[i].Valid) {
    gSettings.BusSpeed = (UINT32)StrDecimalToUintn(InputItems[i].SValue);
    DBG("Apply BusSpeed=%d\n", gSettings.BusSpeed);
  }

  i = 19;
  for (j = 0; j < NGFX; j++) {
    i++; //20
    if (InputItems[i].Valid) {
      AsciiSPrint(gGraphics[j].Model, 64, "%s",  InputItems[i].SValue);
    }
    i++; //21
    if (InputItems[i].Valid) {
      if (gGraphics[j].Vendor == Ati) {
        gSettings.InjectATI = InputItems[i].BValue;
      } else if (gGraphics[j].Vendor == Nvidia) {
        gSettings.InjectNVidia = InputItems[i].BValue;
      } else if (gGraphics[j].Vendor == Intel) {
        gSettings.InjectIntel = InputItems[i].BValue;
      }
    }
    i++; //22
    if (InputItems[i].Valid) {
      if (gGraphics[j].Vendor == Ati) {
        UnicodeSPrint(gSettings.FBName, 32, L"%s", InputItems[i].SValue);
      } else if (gGraphics[j].Vendor == Nvidia) {
        ZeroMem(AString, 256);
        AsciiSPrint(AString, 255, "%s", InputItems[i].SValue);
        hex2bin(AString, (UINT8*)&gSettings.Dcfg[0], 8);
      } else if (gGraphics[j].Vendor == Intel) {
        //ig-platform-id for Ivy+ and snb-platform-id for Sandy
        gSettings.IgPlatform = (UINT32)StrHexToUint64(InputItems[i].SValue);
      }
    }

    if (gGraphics[i].Vendor == Intel) {
      i += 3;
      continue;
    }

    i++; //23
    if (InputItems[i].Valid) {
      gGraphics[j].Ports = (UINT8)(StrDecimalToUintn(InputItems[i].SValue) & 0x0F);
    }
    i++; //24
    if (InputItems[i].Valid) {
      ZeroMem(AString, 256);
      if (StrLen(InputItems[i].SValue) > 0) {
        AsciiSPrint(AString, 255, "%s", InputItems[i].SValue);
        hex2bin(AString, (UINT8*)&gSettings.NVCAP[0], 20);
      }
    }
    i++; //25
    if (InputItems[i].Valid) {
      gGraphics[j].LoadVBios = InputItems[i].BValue;
    }
  }  //end of Graphics Cards
  // next number == 42
  i = 43; //26
  // ErmaC: NvidiaGeneric bool(Y/N)
  if (InputItems[i].Valid) {
    gSettings.NvidiaGeneric = InputItems[i].BValue;
  }

  i = 44;
  if (InputItems[i].Valid) {
    gSettings.KextPatchesAllowed = InputItems[i].BValue;
    gBootChanged = TRUE;
  }
  i++; //45
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPKernelCpu = InputItems[i].BValue;
    gBootChanged = TRUE;
  }
  i++; //46
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPAsusAICPUPM = InputItems[i].BValue;
    gBootChanged = TRUE;
  }
  i++; //47
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPAppleRTC = InputItems[i].BValue;
    gBootChanged = TRUE;
  }
  i++; //48
  if (InputItems[i].Valid) {
     gSettings.KernelAndKextPatches.KPKernelPm = InputItems[i].BValue;
     gBootChanged = TRUE;
  }
  i++; //49
  if (InputItems[i].Valid) {
    gSettings.DropMCFG = InputItems[i].BValue;
  }

  i++; //50
  if (InputItems[i].Valid) {
    gSettings.RefCLK = (UINT32)StrDecimalToUintn(InputItems[i].SValue);
  }

  i++; //51
  if (InputItems[i].Valid) {
    AsciiSPrint(NonDetected, 64, "%s", InputItems[i].SValue);
  }

  i++; //52
  if (InputItems[i].Valid) {
    gSettings.InjectEDID = InputItems[i].BValue;
  }
  k=0;
  for (j=0; j<16; j++) {
    i++; //53-68
    if (InputItems[i].BValue) {
      k += (1<<j);
    }
  }
  i=110;
  for (j=16; j<32; j++) {
    if (InputItems[i++].BValue) {
      k += (1<<j);
    }
  }

  if (gSettings.FixDsdt != k) {
    DBG("applied FixDsdt=%08x\n", k);
    gSettings.FixDsdt = k;
  }

  i = 70;
  if (InputItems[i].Valid) {
    INTN Minus = 0;
    if (InputItems[i].SValue[0] == '-') {
      Minus = 1;
    }
    gSettings.PointerSpeed = StrDecimalToUintn(&InputItems[i].SValue[Minus]);
    if (Minus) {
      gSettings.PointerSpeed = -gSettings.PointerSpeed;
    }
//    DBG("Pointer Speed=%d\n", gSettings.PointerSpeed);
  }
  i++;
  if (InputItems[i].Valid) {
    gSettings.DoubleClickTime = StrDecimalToUintn(InputItems[i].SValue);
//    DBG("DoubleClickTime=%d ms\n", gSettings.DoubleClickTime);
  }
  i++; //72
  if (InputItems[i].Valid) {
    gSettings.PointerMirror = InputItems[i].BValue;
  }


  i = 74;
  if (InputItems[i].Valid) {
    gSettings.USBFixOwnership = InputItems[i].BValue;
  }
  i++; //75
  if (InputItems[i].Valid) {
    gSettings.C3Latency = (UINT16)StrHexToUint64(InputItems[i].SValue);
  }

  i++; //76
  if (InputItems[i].Valid) {
    gSettings.EnabledCores = (UINT8)StrDecimalToUintn(InputItems[i].SValue);
  }
  i++; //77
  if (InputItems[i].Valid) {
    gSettings.SavingMode = (UINT8)StrDecimalToUintn(InputItems[i].SValue);
  }

  i++; //78
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.ProductName, 64, "%s", InputItems[i].SValue);
    // let's fill all other fields based on this ProductName
    // to serve as default
    Model = GetModelFromString(gSettings.ProductName);
    if (Model != MaxMachineType) {
      SetDMISettingsForModel(Model, FALSE);
    }
  }

  i++; //79
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.VersionNr, 64, "%s", InputItems[i].SValue);
  }
  i++; //80
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.SerialNr, 64, "%s", InputItems[i].SValue);
  }
  i++; //81
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.BoardNumber, 64, "%s", InputItems[i].SValue);
  }
  i++; //82
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.BoardSerialNumber, 64, "%s", InputItems[i].SValue);
  }
  i++; //83
  if (InputItems[i].Valid) {
    gSettings.BoardType = (UINT8)(StrDecimalToUintn(InputItems[i].SValue) & 0x0F);
  }
  i++; //84
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.BoardVersion, 64, "%s", InputItems[i].SValue);
  }
  i++; //85
  if (InputItems[i].Valid) {
    gSettings.ChassisType = (UINT8)(StrDecimalToUintn(InputItems[i].SValue) & 0x0F);
  }
  i++; //86
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.RomVersion, 64, "%s", InputItems[i].SValue);
  }
  i++; //87
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.ReleaseDate, 64, "%s", InputItems[i].SValue);
  }

  i++; //88
  if (InputItems[i].Valid) {
    gSettings.DoubleFirstState = InputItems[i].BValue;
  }
  i++; //89
  if (InputItems[i].Valid) {
    gSettings.EnableC7 = InputItems[i].BValue;
  }

  i=90; //90
  if (InputItems[i].Valid) {
    if (StriCmp(InputItems[i].SValue, gSettings.ConfigName) != 0) {
      gBootChanged = TRUE;
      gThemeChanged = TRUE;
      if ((StrLen(InputItems[i].SValue) == 0) ||
          (StriCmp(InputItems[i].SValue, gSettings.MainConfigName) == 0)) {
        for (i=0; i<2; i++) {
          if (gConfigDict[i]) {
            Status = GetUserSettings(SelfRootDir, gConfigDict[i]);
            if (!EFI_ERROR(Status)) {
              if (gSettings.ConfigName) FreePool(gSettings.ConfigName);
              gSettings.ConfigName = EfiStrDuplicate(gSettings.MainConfigName);
              if (gConfigDict[2]) FreeTag(gConfigDict[2]);
              gConfigDict[2] = NULL;
            }
            DBG("Main settings%d from menu: %r\n", i, Status);
          }
        }
      } else {
        Status = LoadUserSettings(SelfRootDir, InputItems[i].SValue, &dict);
        if (!EFI_ERROR(Status)) {
          if (gSettings.ConfigName) FreePool(gSettings.ConfigName);
          //gSettings.ConfigName  = PoolPrint(L"");
          GetUserSettings(SelfRootDir, dict);
          if (gConfigDict[2]) FreeTag(gConfigDict[2]);
          gConfigDict[2] = dict;
          //if (gSettings.ConfigName) FreePool(gSettings.ConfigName);
          gSettings.ConfigName = EfiStrDuplicate(InputItems[i].SValue);
        }
        DBG("Main settings3 from menu: %r\n", Status);
      }

      FillInputs(FALSE);
      NeedSave = FALSE;
    }
  }
  i++; //91
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPLapicPanic = InputItems[i].BValue;
    gBootChanged = TRUE;
  }
  i++; //92
  if (InputItems[i].Valid) {
    gSettings.USBInjection = InputItems[i].BValue;
  }
  i++; //93
  if (InputItems[i].Valid) {
    gSettings.InjectClockID = InputItems[i].BValue;
  }
  i++; //94
  if (InputItems[i].Valid) {
    gSettings.FakeATI = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //95
  if (InputItems[i].Valid) {
    gSettings.FakeNVidia = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //96
  if (InputItems[i].Valid) {
    gSettings.FakeIntel = (UINT32)StrHexToUint64(InputItems[i].SValue);
    DBG("applied FakeIntel=0x%x\n", gSettings.FakeIntel);
  }
  i++; //97
  if (InputItems[i].Valid) {
    gSettings.FakeLAN = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //98
  if (InputItems[i].Valid) {
    gSettings.FakeWIFI = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //99
  if (InputItems[i].Valid) {
    gSettings.FakeSATA = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //100
  if (InputItems[i].Valid) {
    gSettings.FakeXHCI = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }

  i++; //101
  if (InputItems[i].Valid) {
    gSettings.DropOEM_DSM = (UINT16)StrHexToUint64(InputItems[i].SValue);
    dropDSM = gSettings.DropOEM_DSM;
//    defDSM = TRUE;
  }
  i++; //102
  if (InputItems[i].Valid) {
    gSettings.DebugDSDT = InputItems[i].BValue;
  }
  i++; //103
  if (InputItems[i].Valid) {
    gSettings.FakeIMEI = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
//FakeCPUID
  i++; //104
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.FakeCPUID = (UINT32)StrHexToUint64(InputItems[i].SValue);
    DBG("applied FakeCPUID=%06x\n", gSettings.KernelAndKextPatches.FakeCPUID);
    gBootChanged = TRUE;
  }
  i++; //105
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPHaswellE = InputItems[i].BValue;
    gBootChanged = TRUE;
  }

  i++; //106
  if (InputItems[i].Valid) {
    gSettings.StringInjector = InputItems[i].BValue;
  }

  i++; //107
  if (InputItems[i].Valid) {
    gSettings.NoDefaultProperties = InputItems[i].BValue;
  }

  i++; //108
  if (InputItems[i].Valid) {
    gSettings.KernelPatchesAllowed = InputItems[i].BValue;
    gBootChanged = TRUE;
  }

  if (SysVariables) {
    SYSVARIABLES *SysVariablesTmp = SysVariables;
    CHAR8 *SysVarsTmp = NULL;
    while (SysVariablesTmp) {
      if (SysVariablesTmp->MenuItem.Valid) {
        if (StrCmp(SysVariablesTmp->Key, L"CsrActiveConfig") == 0) {
          gSettings.CsrActiveConfig = (UINT32)StrHexToUint64(SysVariablesTmp->MenuItem.SValue);
        } else if (StrCmp(SysVariablesTmp->Key, L"BooterConfig") == 0) {
          gSettings.BooterConfig = (UINT16)StrHexToUint64(SysVariablesTmp->MenuItem.SValue);
        } else if (StrCmp(SysVariablesTmp->Key, L"MLB") == 0) {
          gSettings.RtMLB = AllocateZeroPool(StrSize(SysVariablesTmp->MenuItem.SValue));
          UnicodeStrToAsciiStr(SysVariablesTmp->MenuItem.SValue, gSettings.RtMLB);
        } else if (StrCmp(SysVariablesTmp->Key, L"ROM") == 0) {
          UINT32 len;
          SysVarsTmp = AllocateZeroPool(StrSize(SysVariablesTmp->MenuItem.SValue));
          UnicodeStrToAsciiStr(SysVariablesTmp->MenuItem.SValue, SysVarsTmp);
          if (AsciiStriCmp(SysVarsTmp, "UseMacAddr0") == 0) {
            gSettings.RtROM     = &gLanMac[0][0];
            gSettings.RtROMLen  = 6;
          } else if (AsciiStriCmp(SysVarsTmp, "UseMacAddr1") == 0) {
            gSettings.RtROM     = &gLanMac[1][0];
            gSettings.RtROMLen  = 6;
          } else {
            len = (UINT32)(AsciiStrLen(SysVarsTmp) >> 1);
            gSettings.RtROM = (UINT8*)AllocateZeroPool(len);
            gSettings.RtROMLen = hex2bin(SysVarsTmp, gSettings.RtROM, len);
          }
        } else if (StrCmp(SysVariablesTmp->Key, L"CustomUUID") == 0) {
          BOOLEAN IsValidCustomUUID = FALSE;
          SysVarsTmp = AllocateZeroPool(StrSize(SysVariablesTmp->MenuItem.SValue));
          UnicodeStrToAsciiStr(SysVariablesTmp->MenuItem.SValue, SysVarsTmp);
          if (IsValidGuidAsciiString (SysVarsTmp)) {
            AsciiStrToUnicodeStr (SysVarsTmp, gSettings.CustomUuid);
            Status = StrToGuidLE (gSettings.CustomUuid, &gUuid);
            if (!EFI_ERROR (Status)) {
              IsValidCustomUUID = TRUE;
              // if CustomUUID specified, then default for InjectSystemID=FALSE
              // to stay compatibile with previous Clover behaviour
              gSettings.InjectSystemID = FALSE;
            }
          }

          if (!IsValidCustomUUID && (AsciiStrLen(SysVarsTmp) > 0)) {
            DBG ("Error: invalid CustomUUID '%a' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", SysVarsTmp);
          }
        } else if (StrCmp(SysVariablesTmp->Key, L"InjectSystemID") == 0) {
          gSettings.InjectSystemID = SysVariablesTmp->MenuItem.BValue;
        }
      }
      SysVariablesTmp = SysVariablesTmp->Next;
    }

    if (SysVarsTmp != NULL) {
      FreePool(SysVarsTmp);
    }
  }

  if (NeedSave) {
    SaveSettings();
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


VOID AddMenuInfo(  REFIT_MENU_SCREEN  *SubScreen, CHAR16 *Line)
{
  REFIT_INPUT_DIALOG *InputBootArgs;

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"%s", Line);
  InputBootArgs->Entry.Tag = TAG_INFO;
  InputBootArgs->Item = NULL;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
}


VOID AboutRefit(VOID)
{
  //  CHAR8* Revision = NULL;
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    AboutMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_ABOUT);
  } else {
    AboutMenu.TitleImage = NULL;
  }
  if (AboutMenu.EntryCount == 0) {
    AddMenuInfo(&AboutMenu, PoolPrint(L"Clover Version 2.3k rev %s", FIRMWARE_REVISION)); // by Slice, dmazar, apianti, JrCs, pene and others");
#ifdef FIRMWARE_BUILDDATE
    AddMenuInfo(&AboutMenu, PoolPrint(L" Build: %a", FIRMWARE_BUILDDATE));
#else
    AddMenuInfo(&AboutMenu, L" Build: unknown");
#endif
    AddMenuInfo(&AboutMenu, L"");
    AddMenuInfo(&AboutMenu, L"Based on rEFIt (c) 2006-2010 Christoph Pfisterer");
    AddMenuInfo(&AboutMenu, L"Portions Copyright (c) Intel Corporation");
    AddMenuInfo(&AboutMenu, L"Developers:");
    AddMenuInfo(&AboutMenu, L"  Slice, dmazar, apianti, JrCs, pene, usrsse2");
    AddMenuInfo(&AboutMenu, L"Credits also:");
    AddMenuInfo(&AboutMenu, L"  Kabyl, pcj, jadran, Blackosx, STLVNUB, ycr.ru");
    AddMenuInfo(&AboutMenu, L"  FrodoKenny, skoczi, crazybirdy, Oscar09, xsmile");
    AddMenuInfo(&AboutMenu, L"  cparm, rehabman, nms42, sherlocks, Zenith432");
    AddMenuInfo(&AboutMenu, L"  stinga11, TheRacerMaster, solstice, SoThOr, DF");
    AddMenuInfo(&AboutMenu, L"  cecekpawon, Micky1979, Needy, joevt");
    AddMenuInfo(&AboutMenu, L"  projectosx.com, applelife.ru, insanelymac.com");
    AddMenuInfo(&AboutMenu, L"");
    AddMenuInfo(&AboutMenu, L"Running on:");
    AddMenuInfo(&AboutMenu, PoolPrint(L" EFI Revision %d.%02d",
                                      gST->Hdr.Revision >> 16, gST->Hdr.Revision & ((1 << 16) - 1)));
#if defined(MDE_CPU_IA32)
    AddMenuInfo(&AboutMenu, L" Platform: i386 (32 bit)");
#elif defined(MDE_CPU_X64)
    AddMenuInfo(&AboutMenu, L" Platform: x86_64 (64 bit)");
#else
    AddMenuInfo(&AboutMenu, L" Platform: unknown");
#endif
    AddMenuInfo(&AboutMenu, PoolPrint(L" Firmware: %s rev %d.%d", gST->FirmwareVendor, gST->FirmwareRevision >> 16, gST->FirmwareRevision & ((1 << 16) - 1)));
    AddMenuInfo(&AboutMenu, PoolPrint(L" Screen Output: %s", egScreenDescription()));
    AboutMenu.AnimeRun = GetAnime(&AboutMenu);
    AddMenuEntry(&AboutMenu, &MenuEntryReturn);
  } /* else {
    FreePool(AboutMenu.InfoLines[AboutMenu.InfoLineCount-1]);
    AboutMenu.InfoLines[AboutMenu.InfoLineCount-1]=PoolPrint(L" Screen Output: %s", egScreenDescription());
  } */ else if (AboutMenu.EntryCount >= 2) {
    /*
      EntryCount instead of InfoLineCount. Lastline == return/back. Is necessary recheck screen res here?
    */
    FreePool(AboutMenu.Entries[AboutMenu.EntryCount-2]->Title);
    AboutMenu.Entries[AboutMenu.EntryCount-2]->Title = PoolPrint(L" Screen Output: %s", egScreenDescription());
  }

  RunMenu(&AboutMenu, NULL);
}

VOID HelpRefit(VOID)
{
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    HelpMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_HELP);
  } else {
    HelpMenu.TitleImage = NULL;
  }
  if (HelpMenu.EntryCount == 0) {
    switch (gLanguage)
    {
      case russian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Выход из подменю, обновление главного меню");
        AddMenuInfoLine(&HelpMenu, L"F1  - Помощь по горячим клавишам");
        AddMenuInfoLine(&HelpMenu, L"F2  - Сохранить отчет в preboot.log (только если FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Показать скрытые значки в меню");
        AddMenuInfoLine(&HelpMenu, L"F4  - Родной DSDT сохранить в EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Патченный DSDT сохранить в EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Сохранить ВидеоБиос в EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Снимок экрана в папку EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Извлечь указанный DVD");
        AddMenuInfoLine(&HelpMenu, L"Пробел - Подробнее о выбранном пункте");
        AddMenuInfoLine(&HelpMenu, L"Цифры 1-9 - Быстрый запуск тома по порядку в меню");
        AddMenuInfoLine(&HelpMenu, L"A - О загрузчике");
        AddMenuInfoLine(&HelpMenu, L"O - Дополнительные настройки");
        AddMenuInfoLine(&HelpMenu, L"R - Теплый перезапуск");
        AddMenuInfoLine(&HelpMenu, L"U - Завершить работу в Кловере");
        break;
      case ukrainian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Вийти з меню, оновити головне меню");
        AddMenuInfoLine(&HelpMenu, L"F1  - Ця довідка");
        AddMenuInfoLine(&HelpMenu, L"F2  - Зберегти preboot.log (тiльки FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Відображати приховані розділи");
        AddMenuInfoLine(&HelpMenu, L"F4  - Зберегти OEM DSDT в EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Зберегти патчений DSDT в EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Зберегти VideoBios в EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Зберегти знімок екрану в EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Відкрити обраний диск (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Пробіл - докладніше про обраний пункт меню");
        AddMenuInfoLine(&HelpMenu, L"Клавіші 1-9 -  клавіші пунктів меню");
        AddMenuInfoLine(&HelpMenu, L"A - Про систему");
        AddMenuInfoLine(&HelpMenu, L"O - Опції меню");
        AddMenuInfoLine(&HelpMenu, L"R - Перезавантаження");
        AddMenuInfoLine(&HelpMenu, L"U - Відключити ПК");
        break;
      case spanish:
        AddMenuInfoLine(&HelpMenu, L"ESC - Salir de submenu o actualizar el menu principal");
        AddMenuInfoLine(&HelpMenu, L"F1  - Esta Ayuda");
        AddMenuInfoLine(&HelpMenu, L"F2  - Guardar preboot.log (Solo FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Guardar DSDT oem en EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Guardar DSDT parcheado en EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Guardar VideoBios en EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Guardar Captura de pantalla en EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Expulsar volumen seleccionado (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Espacio - Detalles acerca selected menu entry");
        AddMenuInfoLine(&HelpMenu, L"Digitos 1-9 - Atajo a la entrada del menu");
        AddMenuInfoLine(&HelpMenu, L"A - Menu Acerca de");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Optiones");
        AddMenuInfoLine(&HelpMenu, L"R - Reiniciar Equipo");
        AddMenuInfoLine(&HelpMenu, L"U - Apagar");
        break;
      case portuguese:
      case brasil:
        AddMenuInfoLine(&HelpMenu, L"ESC - Sai do submenu, atualiza o menu principal");
        AddMenuInfoLine(&HelpMenu, L"F1  - Esta ajuda");
        AddMenuInfoLine(&HelpMenu, L"F2  - Salva preboot.log (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Salva oem DSDT em EFI/CLOVER/ACPI/origin/ (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Salva DSDT corrigido em EFI/CLOVER/ACPI/origin/ (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Salva VideoBios em EFI/CLOVER/misc/ (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Salva screenshot em EFI/CLOVER/misc/ (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Ejeta o volume selecionado (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Espaco - Detalhes sobre a opcao do menu selecionada");
        AddMenuInfoLine(&HelpMenu, L"Tecle 1-9 - Atalho para as entradas do menu");
        AddMenuInfoLine(&HelpMenu, L"A - Sobre o Menu");
        AddMenuInfoLine(&HelpMenu, L"O - Opcoes do Menu");
        AddMenuInfoLine(&HelpMenu, L"R - Reiniciar");
        AddMenuInfoLine(&HelpMenu, L"U - Desligar");
        break;
      case italian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Esci dal submenu, Aggiorna menu principale");
        AddMenuInfoLine(&HelpMenu, L"F1  - Aiuto");
        AddMenuInfoLine(&HelpMenu, L"F2  - Salva il preboot.log (solo su FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Salva il DSDT oem in EFI/CLOVER/ACPI/origin/ (solo suFAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Salva il patched DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Salva il VideoBios in EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Salva screenshot in EFI/CLOVER/misc/ (solo su FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Espelli il volume selezionato (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spazio - Dettagli sul menu selezionato");
        AddMenuInfoLine(&HelpMenu, L"Digita 1-9 - Abbreviazioni per il menu");
        AddMenuInfoLine(&HelpMenu, L"A - Informazioni");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Opzioni");
        AddMenuInfoLine(&HelpMenu, L"R - Riavvio");
        AddMenuInfoLine(&HelpMenu, L"U - Spegnimento");
        break;
      case german:
        AddMenuInfoLine(&HelpMenu, L"ESC - Zurueck aus Untermenue, Hauptmenue erneuern");
        AddMenuInfoLine(&HelpMenu, L"F1  - Diese Hilfe");
        AddMenuInfoLine(&HelpMenu, L"F2  - Sichere preboot.log (nur mit FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Sichere OEM DSDT in EFI/CLOVER/ACPI/origin/ (nur mit FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Sichere gepatchtes DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Sichere VideoBios in EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Sichere Bildschirmfoto in EFI/CLOVER/misc/ (nur mit FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Volume auswerfen (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Leertaste - Details über den gewählten Menue Eintrag");
        AddMenuInfoLine(&HelpMenu, L"Zahlen 1-9 - Kurzwahl zum Menue Eintrag");
        AddMenuInfoLine(&HelpMenu, L"A - Menue Informationen");
        AddMenuInfoLine(&HelpMenu, L"O - Menue Optionen");
        AddMenuInfoLine(&HelpMenu, L"R - Neustart");
        AddMenuInfoLine(&HelpMenu, L"U - Ausschalten");
        break;
      case dutch:
        AddMenuInfoLine(&HelpMenu, L"ESC - Verlaat submenu, Vernieuwen hoofdmenu");
        AddMenuInfoLine(&HelpMenu, L"F1  - Onderdeel hulp");
        AddMenuInfoLine(&HelpMenu, L"F2  - preboot.log opslaan (FAT32 only)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Opslaan oem DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Opslaan gepatchte DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Opslaan VideoBios in EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Opslaan schermafdruk in EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Uitwerpen geselecteerd volume (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spatie - Details over geselecteerd menuoptie");
        AddMenuInfoLine(&HelpMenu, L"Cijfers 1-9 - Snelkoppeling naar menuoptie");
        AddMenuInfoLine(&HelpMenu, L"A - Menu Over");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Opties");
        AddMenuInfoLine(&HelpMenu, L"R - Soft Reset");
        AddMenuInfoLine(&HelpMenu, L"U - Verlaten");
        break;
      case french:
        AddMenuInfoLine(&HelpMenu, L"ESC - Quitter sous-menu, Retour menu principal");
        AddMenuInfoLine(&HelpMenu, L"F1  - Aide");
        AddMenuInfoLine(&HelpMenu, L"F2  - Enregistrer preboot.log (FAT32 only)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Enregistrer oem DSDT dans EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Enregistrer DSDT modifié dans EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Enregistrer VideoBios dans EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Enregistrer la capture d'écran dans EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Ejecter le volume (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - Détails a propos du menu selectionné");
        AddMenuInfoLine(&HelpMenu, L"Digits 1-9 - Raccourci vers entrée menu");
        AddMenuInfoLine(&HelpMenu, L"A - A propos");
        AddMenuInfoLine(&HelpMenu, L"O - Options Menu");
        AddMenuInfoLine(&HelpMenu, L"R - Redémarrer");
        AddMenuInfoLine(&HelpMenu, L"U - Eteindre");
        break;
      case indonesian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Keluar submenu, Refresh main menu");
        AddMenuInfoLine(&HelpMenu, L"F1  - Help");
        AddMenuInfoLine(&HelpMenu, L"F2  - Simpan preboot.log ke EFI/CLOVER/ACPI/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Simpan oem DSDT ke EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Simpan patched DSDT ke EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Simpan VideoBios ke EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Simpan screenshot ke EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Eject volume (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spasi - Detail dari menu yang dipilih");
        AddMenuInfoLine(&HelpMenu, L"Tombol 1-9 - Shortcut pilihan menu");
        AddMenuInfoLine(&HelpMenu, L"A - About");
        AddMenuInfoLine(&HelpMenu, L"O - Opsi");
        AddMenuInfoLine(&HelpMenu, L"R - Soft Reset");
        AddMenuInfoLine(&HelpMenu, L"U - Shutdown");
        break;
      case polish:
        AddMenuInfoLine(&HelpMenu, L"ESC - Wyjscie z podmenu, Odswiezenie glownego menu");
        AddMenuInfoLine(&HelpMenu, L"F1  - Pomoc");
        AddMenuInfoLine(&HelpMenu, L"F2  - Zapis preboot.log (tylko FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Zapis DSDT do EFI/CLOVER/ACPI/origin/ (tylko FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Zapis poprawionego DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Zapis BIOSu k. graficznej do EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Zapis zrzutu ekranu do EFI/CLOVER/misc/ (tylko FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Wysuniecie zaznaczonego dysku (tylko dla DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spacja - Informacje nt. dostepnych opcji dla zaznaczonego dysku");
        AddMenuInfoLine(&HelpMenu, L"Znaki 1-9 - Skroty opcji dla wybranego dysku");
        AddMenuInfoLine(&HelpMenu, L"A - Menu Informacyjne");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Opcje");
        AddMenuInfoLine(&HelpMenu, L"R - Restart komputera");
        AddMenuInfoLine(&HelpMenu, L"U - Wylaczenie komputera");
        break;
      case croatian:
        AddMenuInfoLine(&HelpMenu, L"ESC - izlaz iz podizbornika, Osvježi glavni izbornik");
        AddMenuInfoLine(&HelpMenu, L"F1  - Ovaj izbornik");
        AddMenuInfoLine(&HelpMenu, L"F2  - Spremi preboot.log (samo na FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Spremi oem DSDT u EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Spremi patched DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Spremi VideoBios into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Spremi screenshot into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Izbaci izabrai (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - Detalji o odabranom sistemu");
        AddMenuInfoLine(&HelpMenu, L"Brojevi 1 do 9 su prečac do izbora");
        AddMenuInfoLine(&HelpMenu, L"A - Izbornik o meni");
        AddMenuInfoLine(&HelpMenu, L"O - Izbornik opcije");
        AddMenuInfoLine(&HelpMenu, L"R - Restart računala");
        AddMenuInfoLine(&HelpMenu, L"U - Isključivanje računala");
        break;
      case czech:
        AddMenuInfoLine(&HelpMenu, L"ESC - Vrátit se do hlavní nabídky");
        AddMenuInfoLine(&HelpMenu, L"F1  - Tato Nápověda");
        AddMenuInfoLine(&HelpMenu, L"F2  - Uložit preboot.log (FAT32 only)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Uložit oem DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Uložit patchnuté DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Uložit VideoBios do EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Uložit snímek obrazovky do EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Vysunout vybranou mechaniku (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Mezerník - Podrobnosti o vybraném disku");
        AddMenuInfoLine(&HelpMenu, L"čísla 1-9 - Klávesové zkratky pro disky");
        AddMenuInfoLine(&HelpMenu, L"A - Menu O Programu");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Možnosti");
        AddMenuInfoLine(&HelpMenu, L"R - Částečný restart");
        AddMenuInfoLine(&HelpMenu, L"U - Odejít");
        break;
      case korean:
        AddMenuInfoLine(&HelpMenu, L"ESC - 하위메뉴에서 나감, 메인메뉴 새로 고침");
        AddMenuInfoLine(&HelpMenu, L"F1  - 이 도움말");
        AddMenuInfoLine(&HelpMenu, L"F2  - preboot.log를 저장합니다. (FAT32방식에만 해당됨)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - oem DSDT를 EFI/CLOVER/ACPI/origin/에 저장합니다. (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - 패치된 DSDT를 EFI/CLOVER/ACPI/origin/에 저장합니다. (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - VideoBios를 EFI/CLOVER/misc/에 저장합니다. (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - 스크린샷을 EFI/CLOVER/misc/에 저장합니다. (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - 선택한 볼륨을 제거합니다. (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - 선택한 메뉴의 상세 설명");
        AddMenuInfoLine(&HelpMenu, L"Digits 1-9 - 메뉴 단축 번호");
        AddMenuInfoLine(&HelpMenu, L"A - 단축키 - 이 부트로더에 관하여");
        AddMenuInfoLine(&HelpMenu, L"O - 단축키 - 부트 옵션");
        AddMenuInfoLine(&HelpMenu, L"R - 단축키 - 리셋");
        AddMenuInfoLine(&HelpMenu, L"U - 단축키 - 시스템 종료");
        break;
      case romanian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Iesire din sub-meniu, Refresh meniul principal");
        AddMenuInfoLine(&HelpMenu, L"F1  - Ajutor");
        AddMenuInfoLine(&HelpMenu, L"F2  - Salvare preboot.log (doar pentru FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F4  - Salvare oem DSDT in EFI/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Salvare DSDT modificat in EFI/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Salvare VideoBios in EFI/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Salvare screenshot in EFI/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Scoatere volum selectat (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - Detalii despre item-ul selectat");
        AddMenuInfoLine(&HelpMenu, L"Cifre 1-9 - Scurtaturi pentru itemele meniului");
        AddMenuInfoLine(&HelpMenu, L"A - Despre");
        AddMenuInfoLine(&HelpMenu, L"O - Optiuni");
        AddMenuInfoLine(&HelpMenu, L"R - Soft Reset");
        AddMenuInfoLine(&HelpMenu, L"U - Inchidere");
        break;
      case english:
      default:
        AddMenuInfoLine(&HelpMenu, L"ESC - Escape from submenu, Refresh main menu");
        AddMenuInfoLine(&HelpMenu, L"F1  - This help");
        AddMenuInfoLine(&HelpMenu, L"F2  - Save preboot.log into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Save oem DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Save patched DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Save VideoBios into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Save screenshot into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Eject selected volume (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - Details about selected menu entry");
        AddMenuInfoLine(&HelpMenu, L"Digits 1-9 - Shortcut to menu entry");
        AddMenuInfoLine(&HelpMenu, L"A - Menu About");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Options");
        AddMenuInfoLine(&HelpMenu, L"R - Soft Reset");
        AddMenuInfoLine(&HelpMenu, L"U - Exit");
        break;
    }
    HelpMenu.AnimeRun = GetAnime(&HelpMenu);
    AddMenuEntry(&HelpMenu, &MenuEntryReturn);
  }

  RunMenu(&HelpMenu, NULL);
}

//
// Graphics helper functions
//

VOID InitSelection(VOID)
{

  if (!AllowGraphicsMode)
    return;
  SelectionBackgroundPixel.r = (GlobalConfig.SelectionColor >> 24) & 0xFF;
  SelectionBackgroundPixel.g = (GlobalConfig.SelectionColor >> 16) & 0xFF;
  SelectionBackgroundPixel.b = (GlobalConfig.SelectionColor >> 8) & 0xFF;
  SelectionBackgroundPixel.a = (GlobalConfig.SelectionColor >> 0) & 0xFF;

  if (SelectionImages[0] != NULL)
    return;
  // load small selection image
  if (GlobalConfig.SelectionSmallFileName != NULL){
    SelectionImages[2] = egLoadImage(ThemeDir, GlobalConfig.SelectionSmallFileName, FALSE);
  }
  if (SelectionImages[2] == NULL){
    SelectionImages[2] = BuiltinIcon(BUILTIN_SELECTION_SMALL);
    CopyMem(&BlueBackgroundPixel, &StdBackgroundPixel, sizeof(EG_PIXEL));
  }
  SelectionImages[2] = egEnsureImageSize(SelectionImages[2],
                                         row1TileSize, row1TileSize, &MenuBackgroundPixel);
  if (SelectionImages[2] == NULL)
    return;
  // load big selection image
  if (GlobalConfig.SelectionBigFileName != NULL) {
    SelectionImages[0] = egLoadImage(ThemeDir, GlobalConfig.SelectionBigFileName, FALSE);
    SelectionImages[0] = egEnsureImageSize(SelectionImages[0],
                                           row0TileSize, row0TileSize,
                                           &MenuBackgroundPixel);
  }
  if (SelectionImages[0] == NULL) {
    // calculate big selection image from small one
    SelectionImages[0] = BuiltinIcon(BUILTIN_SELECTION_BIG);
    CopyMem(&BlueBackgroundPixel, &StdBackgroundPixel, sizeof(EG_PIXEL));
    if (SelectionImages[0] == NULL) {
      egFreeImage(SelectionImages[2]);
      SelectionImages[2] = NULL;
      return;
    }
    if (GlobalConfig.SelectionOnTop) {
      SelectionImages[0]->HasAlpha = TRUE;
      SelectionImages[2]->HasAlpha = TRUE;
    }
  }
    
  // BootCampStyle indicator image
  if (GlobalConfig.BootCampStyle) {
    // load indicator selection image
    if (GlobalConfig.SelectionIndicatorName != NULL) {
      SelectionImages[4] = egLoadImage(ThemeDir, GlobalConfig.SelectionIndicatorName, TRUE);
    }
    if (!SelectionImages[4]) {
      SelectionImages[4] = egDecodePNG(&emb_selection_indicator[0], sizeof(emb_selection_indicator), 52, TRUE);
      
    }
    SelectionImages[4] = egEnsureImageSize(SelectionImages[4], INDICATOR_SIZE, INDICATOR_SIZE, &MenuBackgroundPixel);
    SelectionImages[5] = egCreateFilledImage(INDICATOR_SIZE, INDICATOR_SIZE,
                                             TRUE, &MenuBackgroundPixel);
  }
  
  // Radio buttons
#if defined(ADVICON)
  Buttons[0] = egLoadImage(ThemeDir, GetIconsExt(L"radio_button", L"png"), TRUE);
  Buttons[1] = egLoadImage(ThemeDir, GetIconsExt(L"radio_button_selected", L"png"), TRUE);
#else //ADVICON
  Buttons[0] = egLoadImage(ThemeDir, L"radio_button.png", TRUE);
  Buttons[1] = egLoadImage(ThemeDir, L"radio_button_selected.png", TRUE);
#endif //ADVICON
  if (!Buttons[0]) {
    Buttons[0] = egDecodePNG(&emb_radio_button[0], sizeof(emb_radio_button), 20, TRUE);
  }
  Buttons[0] = egEnsureImageSize(Buttons[0], TextHeight, TextHeight, &MenuBackgroundPixel);
  if (!Buttons[1]) {
    Buttons[1] = egDecodePNG(&emb_radio_button_selected[0], sizeof(emb_radio_button_selected), 20, TRUE);
  }
  Buttons[1] = egEnsureImageSize(Buttons[1], TextHeight, TextHeight, &MenuBackgroundPixel);
    
  // Checkbox
  Buttons[2] = egLoadImage(ThemeDir, L"checkbox.png", TRUE);
  Buttons[3] = egLoadImage(ThemeDir, L"checkbox_checked.png", TRUE);
  if (!Buttons[2]) {
    Buttons[2] = egDecodePNG(&emb_checkbox[0], sizeof(emb_checkbox), 15, TRUE);
  }
  Buttons[2] = egEnsureImageSize(Buttons[2], TextHeight, TextHeight, &MenuBackgroundPixel);
  if (!Buttons[3]) {
    Buttons[3] = egDecodePNG(&emb_checkbox_checked[0], sizeof(emb_checkbox_checked), 15, TRUE);
  }
  Buttons[3] = egEnsureImageSize(Buttons[3], TextHeight, TextHeight, &MenuBackgroundPixel);
    
  // non-selected background images
  //TODO FALSE -> TRUE
  if (GlobalConfig.SelectionBigFileName != NULL) {
    SelectionImages[1] = egCreateFilledImage(row0TileSize, row0TileSize,
                                             TRUE, &MenuBackgroundPixel);
    SelectionImages[3] = egCreateFilledImage(row1TileSize, row1TileSize,
                                             TRUE, &MenuBackgroundPixel);
  } else { // using embedded theme (this is an assumption but a better check is required)
    SelectionImages[1] = egCreateFilledImage(row0TileSize, row0TileSize,
                                             TRUE, &StdBackgroundPixel);
    SelectionImages[3] = egCreateFilledImage(row1TileSize, row1TileSize,
                                             TRUE, &StdBackgroundPixel);
  }
//  DBG("selections inited\n");
}

//
// Scrolling functions
//
#define CONSTRAIN_MIN(Variable, MinValue) if (Variable < MinValue) Variable = MinValue
#define CONSTRAIN_MAX(Variable, MaxValue) if (Variable > MaxValue) Variable = MaxValue

static VOID InitScroll(OUT SCROLL_STATE *State, IN INTN ItemCount, IN UINTN MaxCount,
                       IN UINTN VisibleSpace, IN INTN Selected)
{
  State->LastSelection = State->CurrentSelection = Selected;
  State->MaxIndex = (INTN)MaxCount - 1;
  State->MaxScroll = ItemCount - 1;
//  State->FirstVisible = 0;

  if (VisibleSpace == 0)
    State->MaxVisible = State->MaxScroll;
  else
    State->MaxVisible = (INTN)VisibleSpace - 1;

  if (State->MaxVisible >= (INTN)ItemCount)
      State->MaxVisible = (INTN)ItemCount - 1;

  State->MaxFirstVisible = State->MaxScroll - State->MaxVisible;
  CONSTRAIN_MIN(State->MaxFirstVisible, 0);
  State->FirstVisible = (Selected > State->MaxFirstVisible)?State->MaxFirstVisible:Selected;

  State->IsScrolling = (State->MaxFirstVisible > 0);
  State->PaintAll = TRUE;
  State->PaintSelection = FALSE;

  State->LastVisible = State->FirstVisible + State->MaxVisible;
//  DBG("InitScroll: MaxIndex=%d, FirstVisible=%d, MaxVisible=%d, MaxFirstVisible=%d\n",
//      State->MaxIndex, State->FirstVisible, State->MaxVisible, State->MaxFirstVisible);
}

static VOID UpdateScroll(IN OUT SCROLL_STATE *State, IN UINTN Movement)
{
  INTN Lines;
  UINTN ScrollMovement = SCROLL_SCROLL_DOWN;
  INTN i;
  State->LastSelection = State->CurrentSelection;
//  DBG("UpdateScroll on %d\n", Movement);
  switch (Movement) {
    case SCROLL_SCROLLBAR_MOVE:
      ScrollbarYMovement += ScrollbarNewPointerPlace.YPos - ScrollbarOldPointerPlace.YPos;
      ScrollbarOldPointerPlace.XPos = ScrollbarNewPointerPlace.XPos;
      ScrollbarOldPointerPlace.YPos = ScrollbarNewPointerPlace.YPos;
      Lines = ScrollbarYMovement * State->MaxIndex / ScrollbarBackground.Height;
      ScrollbarYMovement = ScrollbarYMovement - Lines * (State->MaxVisible * TextHeight - 16 - 1) / State->MaxIndex;
      if (Lines < 0) {
        Lines = -Lines;
        ScrollMovement = SCROLL_SCROLL_UP;
      }
      for (i = 0; i < Lines; i++)
        UpdateScroll(State, ScrollMovement);
      break;

    case SCROLL_LINE_UP: //of left = decrement
      if (State->CurrentSelection > 0) {
        State->CurrentSelection --;
        if (State->CurrentSelection < State->FirstVisible) {
          State->PaintAll = TRUE;
          State->FirstVisible = State->CurrentSelection;
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

    case SCROLL_SCROLL_DOWN:
      if (State->FirstVisible < State->MaxFirstVisible) {
        if (State->CurrentSelection == State->FirstVisible)
          State->CurrentSelection++;
        State->FirstVisible++;
        State->LastVisible++;
        State->PaintAll = TRUE;
      }
      break;

    case SCROLL_SCROLL_UP:
      if (State->FirstVisible > 0) {
        if (State->CurrentSelection == State->LastVisible)
          State->CurrentSelection--;
        State->FirstVisible--;
        State->LastVisible--;
        State->PaintAll = TRUE;
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

  //ycr.ru
  if ((State->PaintAll) && (Movement != SCROLL_NONE))
    HidePointer();
}

//
// menu helper functions
//

VOID AddMenuInfoLine(IN REFIT_MENU_SCREEN *Screen, IN CHAR16 *InfoLine)
{
  AddListElement((VOID ***) &(Screen->InfoLines), (UINTN*)&(Screen->InfoLineCount), InfoLine);
}

VOID AddMenuEntry(IN REFIT_MENU_SCREEN *Screen, IN REFIT_MENU_ENTRY *Entry)
{
  AddListElement((VOID ***) &(Screen->Entries), (UINTN*)&(Screen->EntryCount), Entry);
}

VOID FreeMenu(IN REFIT_MENU_SCREEN *Screen)
{
  UINTN i;
  REFIT_MENU_ENTRY *Tentry = NULL;
//TODO - here we must FreePool for a list of Entries, Screens, InputBootArgs  
  if (Screen->EntryCount > 0) {
    for (i = 0; i < Screen->EntryCount; i++) {
      Tentry = Screen->Entries[i];
      if (Tentry->SubScreen) {
        if (Tentry->SubScreen->Title) {
          FreePool(Tentry->SubScreen->Title);
          Tentry->SubScreen->Title = NULL;          
        }
        // don't free image because of reusing them
   //     egFreeImage(Tentry->SubScreen->Image);
        FreeMenu(Tentry->SubScreen);
        Tentry->SubScreen = NULL;
      }
      if (Tentry->Tag != TAG_RETURN) { //can't free constants
        if (Tentry->Title) {
          FreePool(Tentry->Title);
          Tentry->Title = NULL;
        }
      }
      FreePool(Tentry);
    }
    Screen->EntryCount = 0;
    FreePool(Screen->Entries);
    Screen->Entries = NULL;
  }
  if (Screen->InfoLineCount > 0) {
    for (i = 0; i < Screen->InfoLineCount; i++) {
      // TODO: call a user-provided routine for each element here
      FreePool(Screen->InfoLines[i]);
    }
    Screen->InfoLineCount = 0;
    FreePool(Screen->InfoLines);
    Screen->InfoLines = NULL;
  }

}

static INTN FindMenuShortcutEntry(IN REFIT_MENU_SCREEN *Screen, IN CHAR16 Shortcut)
{
  INTN i;
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
static UINTN InputDialog(IN REFIT_MENU_SCREEN *Screen, IN MENU_STYLE_FUNC  StyleFunc, IN SCROLL_STATE *State)
{

  EFI_STATUS    Status;
  EFI_INPUT_KEY key;
  UINTN         ind = 0;
  UINTN         i = 0;
  UINTN         MenuExit = 0;
  //UINTN         LogSize;
  UINTN         Pos = (Screen->Entries[State->CurrentSelection])->Row;
  INPUT_ITEM    *Item = ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item;
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
         LineSize = 76 - StrLen(Screen->Entries[State->CurrentSelection]->Title);
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


  if ((Item->ItemType != BoolValue) && (Item->ItemType != RadioSwitch)) {
    // Grow Item->SValue to SVALUE_MAX_SIZE if we want to edit a text field
    Item->SValue = EfiReallocatePool(Item->SValue, StrSize(Item->SValue), SVALUE_MAX_SIZE);
  }

  Buffer = Item->SValue;
  BackupShift = Item->LineShift;
  BackupPos = Pos;

  do {

    if (Item->ItemType == BoolValue) {
      Item->BValue = !Item->BValue;
      if (GlobalConfig.TextOnly) {
      Item->SValue = Item->BValue?L"[+] ":L"[ ] ";
      }
      MenuExit = MENU_EXIT_ENTER;
    } else if (Item->ItemType == RadioSwitch) {
      OldChosenTheme = Pos;
      MenuExit = MENU_EXIT_ENTER;
    } else {

      Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);

#if DBG_INPUTDIALOG
      // For debugging the InputDialog
      PrintAt(0, 0, L"%5d: Buffer:%x MaxSize:%d Line:%3d", Iteration, Buffer, SVALUE_MAX_SIZE, LineSize);
      PrintAt(0, 1, L"%5d: Size:%3d Len:%3d", Iteration, StrSize(Buffer), StrLen(Buffer));
      PrintAt(0, 2, L"%5d: Pos:%3d Shift:%3d AbsPos:%3d", Iteration, Pos, Item->LineShift, Pos+Item->LineShift);
      PrintAt(0, 3, L"%5d: KeyCode:%4d KeyChar:%4d", Iteration, key.ScanCode, (UINTN)key.UnicodeChar);
      PrintAt(0, 4, L"%5d: Title:\"%s\"", Iteration, Screen->Entries[State->CurrentSelection]->Title);
      Iteration++;
#endif

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
          /*
          Status = SaveBooterLog(SelfRootDir, PREBOOT_LOG);
          if (EFI_ERROR(Status)) {
            Status = SaveBooterLog(NULL, PREBOOT_LOG);
          } */
          /*
          LogSize = msgCursor - msgbuf;
          Status = egSaveFile(SelfRootDir, PREBOOT_LOG, (UINT8*)msgbuf, LogSize);
          if (EFI_ERROR(Status)) {
            Status = egSaveFile(NULL, PREBOOT_LOG, (UINT8*)msgbuf, LogSize);
          }
          */
          break;
        case SCAN_F6:
          Status = egSaveFile(SelfRootDir, VBIOS_BIN, (UINT8*)(UINTN)0xc0000, 0x20000);
          if (EFI_ERROR(Status)) {
            Status = egSaveFile(NULL, VBIOS_BIN, (UINT8*)(UINTN)0xc0000, 0x20000);
          }
          break;
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
    (Screen->Entries[State->CurrentSelection])->Row = Pos;
    StyleFunc(Screen, State, MENU_FUNCTION_PAINT_SELECTION, NULL);
  } while (!MenuExit);

  switch (MenuExit) {
    case MENU_EXIT_ENTER:
      Item->Valid = TRUE;
      ApplyInputs();
      break;

    case MENU_EXIT_ESCAPE:
      if (StrCmp(Item->SValue, Backup) != 0) {
        UnicodeSPrint(Item->SValue, SVALUE_MAX_SIZE, L"%s", Backup);
        if (Item->ItemType != BoolValue) {
          Item->LineShift = BackupShift;
          (Screen->Entries[State->CurrentSelection])->Row = BackupPos;
        }
        StyleFunc(Screen, State, MENU_FUNCTION_PAINT_SELECTION, NULL);
      }
      break;
  }
  Item->Valid = FALSE;
  FreePool(Backup);
  MsgLog("EDITED: %s\n", Item->SValue);

  return 0;
}

UINTN RunGenericMenu(IN REFIT_MENU_SCREEN *Screen, IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_MENU_ENTRY **ChosenEntry)
{
  SCROLL_STATE  State;
  EFI_STATUS    Status;
  EFI_INPUT_KEY key;
  //    UINTN         Index;
  INTN          ShortcutEntry;
  BOOLEAN       HaveTimeout = FALSE;
  INTN          TimeoutCountdown = 0;
  CHAR16        *TimeoutMessage;
  UINTN         MenuExit;

  //no default - no timeout!
  if ((*DefaultEntryIndex != -1) && (Screen->TimeoutSeconds > 0)) {
    //      DBG("have timeout\n");
    HaveTimeout = TRUE;
    TimeoutCountdown = Screen->TimeoutSeconds;
  }
  MenuExit = 0;

  StyleFunc(Screen, &State, MENU_FUNCTION_INIT, NULL);
  //  DBG("scroll inited\n");
  // override the starting selection with the default index, if any
  if (*DefaultEntryIndex >= 0 && *DefaultEntryIndex <= State.MaxIndex) {
    State.CurrentSelection = *DefaultEntryIndex;
    UpdateScroll(&State, SCROLL_NONE);
  }
  //  DBG("RunGenericMenu CurrentSelection=%d MenuExit=%d\n",
  //      State.CurrentSelection, MenuExit);

  // exhaust key buffer and be sure no key is pressed to prevent option selection
  // when coming with a key press from timeout=0, for example
  while (ReadAllKeyStrokes()) gBS->Stall(500 * 1000);
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
      TimeoutMessage = PoolPrint(L"%s in %d seconds", Screen->TimeoutText, TimeoutCountdown);
      StyleFunc(Screen, &State, MENU_FUNCTION_PAINT_TIMEOUT, TimeoutMessage);
      FreePool(TimeoutMessage);
    }

    if (gEvent) { //for now used at CD eject.
      MenuExit = MENU_EXIT_ESCAPE;
      State.PaintAll = TRUE;
      gEvent = 0; //to prevent looping
      break;
    }
    key.UnicodeChar = 0;
    key.ScanCode = 0;
    if (!mGuiReady) {
      mGuiReady = TRUE;
      DBG("GUI ready\n");
    }
    Status = WaitForInputEventPoll(Screen, 1); //wait for 1 seconds.
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

    switch (gAction) {
      case ActionSelect:
        State.LastSelection = State.CurrentSelection;
        State.CurrentSelection = gItemID;
        State.PaintAll = TRUE;
        HidePointer();
        break;
      case ActionEnter:
        State.LastSelection = State.CurrentSelection;
        State.CurrentSelection = gItemID;
        if ((Screen->Entries[gItemID])->Tag == TAG_INPUT) {
          MenuExit = InputDialog(Screen, StyleFunc, &State);
        } else if ((Screen->Entries[gItemID])->Tag == TAG_SWITCH) {
          MenuExit = InputDialog(Screen, StyleFunc, &State);
          State.PaintAll = TRUE;
          HidePointer();
        } else {
          MenuExit = MENU_EXIT_ENTER;
        }
        break;
      case ActionHelp:
        MenuExit = MENU_EXIT_HELP;
        break;
      case ActionOptions:
        State.LastSelection = State.CurrentSelection;
        State.CurrentSelection = gItemID;
        MenuExit = MENU_EXIT_OPTIONS;
        break;
      case ActionDetails:
        State.LastSelection = State.CurrentSelection;
        // Index = State.CurrentSelection;
        State.CurrentSelection = gItemID;
        if ((Screen->Entries[gItemID])->Tag == TAG_INPUT) {
          MenuExit = InputDialog(Screen, StyleFunc, &State);
        } else if ((Screen->Entries[gItemID])->Tag == TAG_SWITCH) {
          MenuExit = InputDialog(Screen, StyleFunc, &State);
          State.PaintAll = TRUE;
          HidePointer();
        } else {
          MenuExit = MENU_EXIT_DETAILS;
        }
        break;
      case ActionDeselect:
        State.LastSelection = State.CurrentSelection;
        State.PaintAll = TRUE;
        HidePointer();
        break;
      case ActionFinish:
        MenuExit = MENU_EXIT_ESCAPE;
        break;
      case ActionScrollDown:
        UpdateScroll(&State, SCROLL_SCROLL_DOWN);
        break;
      case ActionScrollUp:
        UpdateScroll(&State, SCROLL_SCROLL_UP);
        break;
      case ActionMoveScrollbar:
        UpdateScroll(&State, SCROLL_SCROLLBAR_MOVE);
        break;
      default:
        break;
    }


    // read key press (and wait for it if applicable)
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
    if ((Status == EFI_NOT_READY) && (gAction == ActionNone)) {
      continue;
    }
    if (gAction == ActionNone) {
      ReadAllKeyStrokes(); //clean to avoid doubles
    }
    if (HaveTimeout) {
      // the user pressed a key, cancel the timeout
      StyleFunc(Screen, &State, MENU_FUNCTION_PAINT_TIMEOUT, L"");
      HidePointer(); //ycr.ru
      HaveTimeout = FALSE;
    }

    gAction = ActionNone; //do action once
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
    //    SetNextScreenMode(1);
        StyleFunc(Screen, &State, MENU_FUNCTION_INIT, NULL);
        break;
      case SCAN_PAGE_DOWN:
        UpdateScroll(&State, SCROLL_PAGE_DOWN);
     //   SetNextScreenMode(-1);
        StyleFunc(Screen, &State, MENU_FUNCTION_INIT, NULL);
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
        DBG("create folder %r\n", Status);
        if (!EFI_ERROR(Status)) {
          Status = egSaveFile(SelfRootDir,  L"EFI\\CLOVER\\new_folder\\new_file.txt", (UINT8*)SomeText, sizeof(*SomeText)+1);
          DBG("create file %r\n", Status);
        }
        break;

      case SCAN_F8:
        do {
          CHAR16 *Str = PoolPrint(L"%s\n%s\n%s", L"ABC", L"123456", L"xy");
          if (Str != NULL) {
            AlertMessage(L"Sample message", Str);
            FreePool(Str);
          }
        } while (0);
 //this way screen is dirty
        break;
 */
      case SCAN_F9:
        SetNextScreenMode(1);
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
        if ((Screen->Entries[State.CurrentSelection])->Tag == TAG_INPUT) {
          MenuExit = InputDialog(Screen, StyleFunc, &State);
        } else if ((Screen->Entries[State.CurrentSelection])->Tag == TAG_SWITCH){
          MenuExit = InputDialog(Screen, StyleFunc, &State);
          State.PaintAll = TRUE;
        } else if ((Screen->Entries[State.CurrentSelection])->Tag == TAG_CLOVER){
          MenuExit = MENU_EXIT_DETAILS;
        } else {
          MenuExit = MENU_EXIT_ENTER;
        }
        break;
      case ' ':
        if ((Screen->Entries[State.CurrentSelection])->Tag == TAG_INPUT) {
          MenuExit = InputDialog(Screen, StyleFunc, &State);
        } else if ((Screen->Entries[State.CurrentSelection])->Tag == TAG_SWITCH){
          MenuExit = InputDialog(Screen, StyleFunc, &State);
          State.PaintAll = TRUE;
          HidePointer();
        } else {
          MenuExit = MENU_EXIT_DETAILS;
        }
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
  INTN i = 0, j = 0;
  static UINTN TextMenuWidth = 0,ItemWidth = 0, MenuHeight = 0;
  static UINTN MenuPosY = 0;
  //static CHAR16 **DisplayStrings;
  CHAR16 *TimeoutMessage;
  CHAR16 ResultString[SVALUE_MAX_SIZE / sizeof(CHAR16) + 128]; // assume a title max length of around 128

  switch (Function) {

    case MENU_FUNCTION_INIT:
      // vertical layout
      MenuPosY = 4;
      if (Screen->InfoLineCount > 0)
        MenuPosY += Screen->InfoLineCount + 1;
      MenuHeight = ConHeight - MenuPosY;
      if (Screen->TimeoutSeconds > 0)
        MenuHeight -= 2;
      InitScroll(State, Screen->EntryCount, Screen->EntryCount, MenuHeight, 0);

      // determine width of the menu
      TextMenuWidth = 50;  // minimum
      for (i = 0; i <= State->MaxIndex; i++) {
        ItemWidth = StrLen(Screen->Entries[i]->Title);
        if (TextMenuWidth < ItemWidth)
          TextMenuWidth = ItemWidth;
      }
      if (TextMenuWidth > ConWidth - 6)
        TextMenuWidth = ConWidth - 6;

      break;

    case MENU_FUNCTION_CLEANUP:
      // release temporary memory
      break;

    case MENU_FUNCTION_PAINT_ALL:
      // paint the whole screen (initially and after scrolling)
			gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_BASIC);
			for (i = 0; i < (INTN)ConHeight - 4; i++) {
				gST->ConOut->SetCursorPosition (gST->ConOut, 0, 4 + i);
				gST->ConOut->OutputString (gST->ConOut, BlankLine);
			}

			BeginTextScreen(Screen->Title);
      if (Screen->InfoLineCount > 0) {
        gST->ConOut->SetAttribute (gST->ConOut, ATTR_BASIC);
        for (i = 0; i < (INTN)Screen->InfoLineCount; i++) {
          gST->ConOut->SetCursorPosition (gST->ConOut, 3, 4 + i);
          gST->ConOut->OutputString (gST->ConOut, Screen->InfoLines[i]);
        }
      }

      for (i = State->FirstVisible; i <= State->LastVisible && i <= State->MaxIndex; i++) {
				gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (i - State->FirstVisible));
				if (i == State->CurrentSelection)
					gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_CURRENT);
				else
					gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_BASIC);

				StrCpy(ResultString, Screen->Entries[i]->Title);
        if (Screen->Entries[i]->Tag == TAG_INPUT) {
          if (((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->ItemType == BoolValue) {
            StrCat(ResultString, L":");
            StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->SValue);
          } else {
					StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->SValue);
          }
        }
				for (j = StrLen(ResultString); j < (INTN)TextMenuWidth; j++)
					ResultString[j] = L' ';
				ResultString[j] = 0;
				gST->ConOut->OutputString (gST->ConOut, ResultString);
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
      //gST->ConOut->OutputString (gST->ConOut, DisplayStrings[State->LastSelection]);
			StrCpy(ResultString, Screen->Entries[State->LastSelection]->Title);
      if (Screen->Entries[State->LastSelection]->Tag == TAG_INPUT) {
        if (((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->ItemType == BoolValue) {
          StrCat(ResultString, L":");
          StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->SValue);
        } else {
				StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->SValue);
        }
      }
			for (j = StrLen(ResultString); j < (INTN)TextMenuWidth; j++)
				ResultString[j] = L' ';
			ResultString[j] = 0;
			gST->ConOut->OutputString (gST->ConOut, ResultString);

			gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (State->CurrentSelection - State->FirstVisible));
      gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_CURRENT);
			StrCpy(ResultString, Screen->Entries[State->CurrentSelection]->Title);
      if (Screen->Entries[State->CurrentSelection]->Tag == TAG_INPUT) {
        if (((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->ItemType == BoolValue) {
          StrCat(ResultString, L":");
				StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->SValue);
        } else {
          StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->SValue);
        }
      }
			for (j = StrLen(ResultString); j < (INTN)TextMenuWidth; j++)
				ResultString[j] = L' ';
			ResultString[j] = 0;
			gST->ConOut->OutputString (gST->ConOut, ResultString);
      //gST->ConOut->OutputString (gST->ConOut, DisplayStrings[State->CurrentSelection]);
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

INTN DrawTextXY(IN CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign)
{
  INTN      TextWidth = 0;
  INTN      XText = 0;
  EG_IMAGE  *TextBufferXY = NULL;

  if (!Text) {
    return 0;
  }
    
  egMeasureText(Text, &TextWidth, NULL);
  if (XAlign == X_IS_LEFT) {
    TextWidth = UGAWidth - XPos - 1;
    XText = XPos;
  }
  TextBufferXY = egCreateImage(TextWidth, TextHeight, TRUE);

  egFillImage(TextBufferXY, &MenuBackgroundPixel);
  // render the text
  TextWidth = egRenderText(Text, TextBufferXY, 0, 0, 0xFFFF);
  if (XAlign != X_IS_LEFT) { // shift 64 is prohibited
    XText = XPos - (TextWidth >> XAlign);
  }
  BltImageAlpha(TextBufferXY, XText, YPos,  &MenuBackgroundPixel, 16);
  egFreeImage(TextBufferXY);
  return TextWidth;
}

VOID DrawBCSText(IN CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign)
{
  INTN      ChrsNum = 16;
  INTN      FntChrsNum = 12;
  INTN      TextWidth = 0;
  INTN      XText = 0;
  INTN      i = 0;
  EG_IMAGE  *TextBufferXY = NULL;
  CHAR16    *BCSText = NULL;
  
  if (!Text) {
    return;
  }
  
  if (GlobalConfig.TileXSpace >= 25 && GlobalConfig.TileXSpace < 30) {
    ChrsNum = 17;
    FntChrsNum = 13;
  } else if (GlobalConfig.TileXSpace >= 30 && GlobalConfig.TileXSpace < 35) {
    ChrsNum = 18;
    FntChrsNum = 14;
  } else if (GlobalConfig.TileXSpace >= 35 && GlobalConfig.TileXSpace < 40) {
    ChrsNum = 19;
    FntChrsNum = 15;
  } else if (GlobalConfig.TileXSpace >= 40 && GlobalConfig.TileXSpace < 45) {
    ChrsNum = 20;
    FntChrsNum = 16;
  } else if (GlobalConfig.TileXSpace >= 45 && GlobalConfig.TileXSpace < 50) {
    ChrsNum = 21;
    FntChrsNum = 17;
  } else if (GlobalConfig.TileXSpace >= 50 && GlobalConfig.TileXSpace < 55) {
    ChrsNum = 22;
    FntChrsNum = 18;
  } else {
    ChrsNum = 16;
    FntChrsNum = 12;
  }
  
  TextWidth = ((StrLen(Text) <= ((GlobalConfig.Font == FONT_LOAD) ? (FntChrsNum - 3) : (ChrsNum - 3))) ?
               StrLen(Text) : ((GlobalConfig.Font == FONT_LOAD) ? FntChrsNum : ChrsNum)) *
  ((FontWidth > GlobalConfig.CharWidth) ? FontWidth : GlobalConfig.CharWidth);
  
  TextBufferXY = egCreateImage(TextWidth, FontHeight, TRUE);
  
  egFillImage(TextBufferXY, &MenuBackgroundPixel);
  
  // render the text
  if (StrLen(Text) > ((GlobalConfig.Font == FONT_LOAD) ? (FntChrsNum - 3) : (ChrsNum - 3))) {
    BCSText = AllocatePool(sizeof(CHAR16) * ((GlobalConfig.Font == FONT_LOAD) ? FntChrsNum : ChrsNum));
    
    for (i = 0; i < ((GlobalConfig.Font == FONT_LOAD) ? FntChrsNum : ChrsNum); i++) {
      if (i < ((GlobalConfig.Font == FONT_LOAD) ? (FntChrsNum - 3) : (ChrsNum - 3))) {
        BCSText[i] = Text[i];
      } else {
        BCSText[i] = L'.';
      }
    }
    BCSText[((GlobalConfig.Font == FONT_LOAD) ? FntChrsNum : ChrsNum)] = '\0';
    
    if (!BCSText) {
      return;
    }
    
    TextWidth = egRenderText(BCSText, TextBufferXY, 0, 0, 0xFFFF);
    
    FreePool(BCSText);
  } else {
    TextWidth = egRenderText(Text, TextBufferXY, 0, 0, 0xFFFF);
  }
  
  if (XAlign == X_IS_LEFT) {
    TextWidth = UGAWidth - XPos - 1;
    XText = XPos;
  } else { // shift 64 is prohibited
    XText = XPos - (TextWidth >> XAlign);
  }
  
  BltImageAlpha(TextBufferXY, XText, YPos,  &MenuBackgroundPixel, 16);
    
  egFreeImage(TextBufferXY);
}

VOID DrawMenuText(IN CHAR16 *Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor)
{
  //use Text=null to reinit the buffer
  if (!Text) {
    if (TextBuffer) {
      egFreeImage(TextBuffer);
      TextBuffer = NULL;
    }
    return;
  }

  if (TextBuffer && (TextBuffer->Height != TextHeight)) {
    egFreeImage(TextBuffer);
    TextBuffer = NULL;
  }

  if (TextBuffer == NULL) {
    TextBuffer = egCreateImage(UGAWidth-XPos, TextHeight, TRUE);
  }

  if (Cursor != 0xFFFF) {
    egFillImage(TextBuffer, &MenuBackgroundPixel);
  } else {
    egFillImage(TextBuffer, &InputBackgroundPixel);
  }


  if (SelectedWidth > 0) {
    // draw selection bar background
    egFillImageArea(TextBuffer, 0, 0, (INTN)SelectedWidth, TextBuffer->Height,
                    &SelectionBackgroundPixel);
  }

  // render the text
  egRenderText(Text, TextBuffer, TEXT_XMARGIN, TEXT_YMARGIN, Cursor);
  BltImageAlpha(TextBuffer, (INTN)XPos, (INTN)YPos, &MenuBackgroundPixel, 16);
}


VOID FreeScrollBar(VOID)
{
  if (ScrollbarBackgroundImage) {
    egFreeImage(ScrollbarBackgroundImage);
    ScrollbarBackgroundImage = NULL;
  }
  if (BarStartImage) {
    egFreeImage(BarStartImage);
    BarStartImage = NULL;
  }
  if (BarEndImage) {
    egFreeImage(BarEndImage);
    BarEndImage = NULL;
  }
  if (ScrollbarImage) {
    egFreeImage(ScrollbarImage);
    ScrollbarImage = NULL;
  }
  if (ScrollStartImage) {
    egFreeImage(ScrollStartImage);
    ScrollStartImage = NULL;
  }
  if (ScrollEndImage) {
    egFreeImage(ScrollEndImage);
    ScrollEndImage = NULL;
  }
  if (UpButtonImage) {
    egFreeImage(UpButtonImage);
    UpButtonImage = NULL;
  }
  if (DownButtonImage) {
    egFreeImage(DownButtonImage);
    DownButtonImage = NULL;
  }
}


VOID InitBar(VOID)
{
  if (ThemeDir) {
    if (!ScrollbarBackgroundImage) {
#if defined(ADVICON)
      ScrollbarBackgroundImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_fill", L"png"), FALSE);
#else //ADVICON
      ScrollbarBackgroundImage = egLoadImage(ThemeDir, L"scrollbar\\bar_fill.png", FALSE);
#endif //ADVICON
    }
    if (!BarStartImage) {
#if defined(ADVICON)
      BarStartImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_start", L"png"), TRUE);
#else //ADVICON
      BarStartImage = egLoadImage(ThemeDir, L"scrollbar\\bar_start.png", TRUE);
#endif //ADVICON
    }
    if (!BarEndImage) {
#if defined(ADVICON)
      BarEndImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_end", L"png"), TRUE);
#else //ADVICON
      BarEndImage = egLoadImage(ThemeDir, L"scrollbar\\bar_end.png", TRUE);
#endif //ADVICON
    }
    if (!ScrollbarImage) {
#if defined(ADVICON)
      ScrollbarImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_fill", L"png"), FALSE);
#else //ADVICON
      ScrollbarImage = egLoadImage(ThemeDir, L"scrollbar\\scroll_fill.png", FALSE);
#endif //ADVICON
    }
    if (!ScrollStartImage) {
#if defined(ADVICON)
      ScrollStartImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_start", L"png"), TRUE);
#else //ADVICON
      ScrollStartImage = egLoadImage(ThemeDir, L"scrollbar\\scroll_start.png", TRUE);
#endif //ADVICON
    }
    if (!ScrollEndImage) {
#if defined(ADVICON)
      ScrollEndImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_end", L"png"), TRUE);
#else //ADVICON
      ScrollEndImage = egLoadImage(ThemeDir, L"scrollbar\\scroll_end.png", TRUE);
#endif //ADVICON
    }
    if (!UpButtonImage) {
#if defined(ADVICON)
      UpButtonImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\up_button", L"png"), TRUE);
#else //ADVICON
      UpButtonImage = egLoadImage(ThemeDir, L"scrollbar\\up_button.png", TRUE);
#endif //ADVICON
    }
    if (!DownButtonImage) {
#if defined(ADVICON)
      DownButtonImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\down_button", L"png"), TRUE);
#else //ADVICON
      DownButtonImage = egLoadImage(ThemeDir, L"scrollbar\\down_button.png", TRUE);
#endif //ADVICON
    }
  }

  if (!BarStartImage) {
    BarStartImage = egDecodePNG(&emb_scroll_bar_start[0], sizeof(emb_scroll_bar_start), 5, TRUE);
  }
  if (!BarEndImage) {
    BarEndImage = egDecodePNG(&emb_scroll_bar_end[0], sizeof(emb_scroll_bar_end), 5, TRUE);
  }
  if (!ScrollbarBackgroundImage) {
    ScrollbarBackgroundImage = egDecodePNG(&emb_scroll_bar_fill[0], sizeof(emb_scroll_bar_fill), 1, TRUE);
  }
  if (!ScrollbarImage) {
    ScrollbarImage = egDecodePNG(&emb_scroll_scroll_fill[0], sizeof(emb_scroll_scroll_fill), 5, TRUE);
  }
  if (!ScrollStartImage) {
    ScrollStartImage = egDecodePNG(&emb_scroll_scroll_start[0], sizeof(emb_scroll_scroll_start), 7, TRUE);
  }
  if (!ScrollEndImage) {
    ScrollEndImage = egDecodePNG(&emb_scroll_scroll_end[0], sizeof(emb_scroll_scroll_end), 7, TRUE);
  }
  if (!UpButtonImage) {
    UpButtonImage = egDecodePNG(&emb_scroll_up_button[0], sizeof(emb_scroll_up_button), 20, TRUE);
  }
  if (!DownButtonImage) {
    DownButtonImage = egDecodePNG(&emb_scroll_down_button[0], sizeof(emb_scroll_down_button), 20, TRUE);
  }
}

VOID SetBar(INTN PosX, INTN UpPosY, INTN DownPosY, IN SCROLL_STATE *State)
{

  UpButton.XPos = PosX;
  UpButton.YPos = UpPosY;
  UpButton.Width = ScrollWidth; // 16
  UpButton.Height = ScrollButtonsHeight; // 20

  DownButton.Width = UpButton.Width;
  DownButton.Height = ScrollButtonsHeight;
  DownButton.XPos = UpButton.XPos;
  DownButton.YPos = DownPosY;

  ScrollbarBackground.XPos = UpButton.XPos;
  ScrollbarBackground.YPos = UpButton.YPos + UpButton.Height;
  ScrollbarBackground.Width = UpButton.Width;
  ScrollbarBackground.Height = DownButton.YPos - (UpButton.YPos + UpButton.Height);

  BarStart.XPos = ScrollbarBackground.XPos;
  BarStart.YPos = ScrollbarBackground.YPos;
  BarStart.Width = ScrollbarBackground.Width;
  BarStart.Height = ScrollBarDecorationsHeight; // 5

  BarEnd.Width = ScrollbarBackground.Width;
  BarEnd.Height = ScrollBarDecorationsHeight;
  BarEnd.XPos = ScrollbarBackground.XPos;
  BarEnd.YPos = DownButton.YPos - BarEnd.Height;

  ScrollStart.XPos = ScrollbarBackground.XPos;
  ScrollStart.YPos = ScrollbarBackground.YPos + ScrollbarBackground.Height * State->FirstVisible / (State->MaxIndex + 1);
  ScrollStart.Width = ScrollbarBackground.Width;
  ScrollStart.Height = ScrollScrollDecorationsHeight; // 7


  Scrollbar.XPos = ScrollbarBackground.XPos;
  Scrollbar.YPos = ScrollStart.YPos + ScrollStart.Height;
  Scrollbar.Width = ScrollbarBackground.Width;
  Scrollbar.Height = ScrollbarBackground.Height * (State->MaxVisible + 1) / (State->MaxIndex + 1) - ScrollStart.Height;

  ScrollEnd.Width = ScrollbarBackground.Width;
  ScrollEnd.Height = ScrollScrollDecorationsHeight;
  ScrollEnd.XPos = ScrollbarBackground.XPos;
  ScrollEnd.YPos = Scrollbar.YPos + Scrollbar.Height - ScrollEnd.Height;

  Scrollbar.Height -= ScrollEnd.Height;

  ScrollTotal.XPos = UpButton.XPos;
  ScrollTotal.YPos = UpButton.YPos;
  ScrollTotal.Width = UpButton.Width;
  ScrollTotal.Height = DownButton.YPos + DownButton.Height - UpButton.YPos;
}

VOID ScrollingBar(IN SCROLL_STATE *State)
{
  EG_IMAGE* Total;
  INTN  i;

  ScrollEnabled = (State->MaxFirstVisible != 0);
  if (ScrollEnabled) {
    Total = egCreateFilledImage(ScrollTotal.Width, ScrollTotal.Height, TRUE, &MenuBackgroundPixel);
    for (i = 0; i < ScrollbarBackground.Height; i++) {
      egComposeImage(Total, ScrollbarBackgroundImage, ScrollbarBackground.XPos - ScrollTotal.XPos, ScrollbarBackground.YPos + i - ScrollTotal.YPos);
    }

    egComposeImage(Total, BarStartImage, BarStart.XPos - ScrollTotal.XPos, BarStart.YPos - ScrollTotal.YPos);
    egComposeImage(Total, BarEndImage, BarEnd.XPos - ScrollTotal.XPos, BarEnd.YPos - ScrollTotal.YPos);

    for (i = 0; i < Scrollbar.Height; i++) {
      egComposeImage(Total, ScrollbarImage, Scrollbar.XPos - ScrollTotal.XPos, Scrollbar.YPos + i - ScrollTotal.YPos);
    }

    egComposeImage(Total, UpButtonImage, UpButton.XPos - ScrollTotal.XPos, UpButton.YPos - ScrollTotal.YPos);
    egComposeImage(Total, DownButtonImage, DownButton.XPos - ScrollTotal.XPos, DownButton.YPos - ScrollTotal.YPos);
    egComposeImage(Total, ScrollStartImage, ScrollStart.XPos - ScrollTotal.XPos, ScrollStart.YPos - ScrollTotal.YPos);
    egComposeImage(Total, ScrollEndImage, ScrollEnd.XPos - ScrollTotal.XPos, ScrollEnd.YPos - ScrollTotal.YPos);

    BltImageAlpha(Total, ScrollTotal.XPos, ScrollTotal.YPos, &MenuBackgroundPixel, ScrollWidth);
    egFreeImage(Total);
  }
}


VOID GraphicsMenuStyle(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText)
{
  INTN i;
  INTN j = 0;
  INTN ItemWidth = 0;
  INTN X;
  INTN VisibleHeight = 0; //assume vertical layout
  CHAR16 ResultString[SVALUE_MAX_SIZE / sizeof(CHAR16) + 128]; // assume a title max length of around 128
  INTN PlaceCentre = (GlobalConfig.Font == FONT_LOAD) ? ((TextHeight / 2) - 7) : 0;
  
  HidePointer();

  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime(Screen);
      SwitchToGraphicsAndClear();

      EntriesPosY = ((UGAHeight - LAYOUT_TOTAL_HEIGHT) >> 1) + LayoutBannerOffset + (TextHeight << 1);

      //VisibleHeight = (UGAHeight - EntriesPosY) / TextHeight - Screen->InfoLineCount - 1;
      VisibleHeight = ((UGAHeight - EntriesPosY) / TextHeight) - Screen->InfoLineCount - 1 - GlobalConfig.PruneScrollRows;
      //DBG("MENU_FUNCTION_INIT 1 EntriesPosY=%d VisibleHeight=%d\n", EntriesPosY, VisibleHeight);
      if (Screen->Entries[0]->Tag == TAG_SWITCH) {
        j = OldChosenTheme;
      }
      InitScroll(State, Screen->EntryCount, Screen->EntryCount, VisibleHeight, j);
      // determine width of the menu -- not working
      //MenuWidth = 80;  // minimum
      MenuWidth = LAYOUT_TEXT_WIDTH;
      DrawMenuText(NULL, 0, 0, 0, 0);

      if (Screen->TitleImage) {
        if (MenuWidth > (INTN)(UGAWidth - TITLEICON_SPACING - Screen->TitleImage->Width)) {
          MenuWidth = UGAWidth - TITLEICON_SPACING - Screen->TitleImage->Width - 2;
        }
        EntriesPosX = (UGAWidth - (Screen->TitleImage->Width + TITLEICON_SPACING + MenuWidth)) >> 1;
        //DBG("UGAWIdth=%d TitleImage=%d MenuWidth=%d\n", UGAWidth,
        //Screen->TitleImage->Width, MenuWidth);
        MenuWidth += Screen->TitleImage->Width;
      } else {
        EntriesPosX = (UGAWidth - MenuWidth) >> 1;
      }
      TimeoutPosY = EntriesPosY + (Screen->EntryCount + 1) * TextHeight;

      // initial painting
      egMeasureText(Screen->Title, &ItemWidth, NULL);
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE)) {
        DrawMenuText(Screen->Title, 0, ((UGAWidth - ItemWidth) >> 1) - TEXT_XMARGIN, EntriesPosY - TextHeight * 2, 0xFFFF);
      }

      if (Screen->TitleImage) {
        INTN FilmXPos = (INTN)(EntriesPosX - (Screen->TitleImage->Width + TITLEICON_SPACING));
        INTN FilmYPos = (INTN)EntriesPosY;
        BltImageAlpha(Screen->TitleImage, FilmXPos, FilmYPos, &MenuBackgroundPixel, 16);

        // Update FilmPlace only if not set by InitAnime
        if (Screen->FilmPlace.Width == 0 || Screen->FilmPlace.Height == 0) {
          Screen->FilmPlace.XPos = FilmXPos;
          Screen->FilmPlace.YPos = FilmYPos;
          Screen->FilmPlace.Width = Screen->TitleImage->Width;
          Screen->FilmPlace.Height = Screen->TitleImage->Height;
        }
      }

      if (Screen->InfoLineCount > 0) {
        DrawMenuText(NULL, 0, 0, 0, 0);
        for (i = 0; i < (INTN)Screen->InfoLineCount; i++) {
          DrawMenuText(Screen->InfoLines[i], 0, EntriesPosX, EntriesPosY, 0xFFFF);
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
      DrawMenuText(NULL, 0, 0, 0, 0); 
      SetBar(EntriesPosX + MenuWidth + 16, EntriesPosY,
             EntriesPosY + (State->MaxVisible + 1) * TextHeight - DownButton.Height, State);

      // blackosx swapped this around so drawing of selection comes before drawing scrollbar.

      for (i = State->FirstVisible, j = 0; i <= State->LastVisible; i++, j++) {
        INTN  TitleLen;        

        TitleLen = StrLen(Screen->Entries[i]->Title);
        Screen->Entries[i]->Place.XPos = EntriesPosX;
        Screen->Entries[i]->Place.YPos = EntriesPosY + j * TextHeight;
        Screen->Entries[i]->Place.Width = TitleLen * GlobalConfig.CharWidth;
        Screen->Entries[i]->Place.Height = (UINTN)TextHeight;

        if (Screen->Entries[i]->Tag == TAG_INPUT) {
          StrCpy(ResultString, Screen->Entries[i]->Title);
          // Slice - suppose to use Row as Cursor in text
          if (((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->ItemType == BoolValue) {
          Screen->Entries[i]->Place.Width = StrLen(ResultString) * GlobalConfig.CharWidth;
          DrawMenuText(ResultString,
                       (i == State->CurrentSelection)?(MenuWidth /* Screen->Entries[i]->Place.Width */):0,
                         EntriesPosX + (TextHeight + TEXT_XMARGIN), Screen->Entries[i]->Place.YPos,
                       TitleLen + Screen->Entries[i]->Row);
            BltImageCompositeIndicator((((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->BValue) ? Buttons[3] :
                                       Buttons[2], Buttons[2], EntriesPosX + TEXT_XMARGIN,
                                       Screen->Entries[i]->Place.YPos + PlaceCentre, 16);
          } else {
            StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->SValue);
            StrCat(ResultString, L" ");
            Screen->Entries[i]->Place.Width = StrLen(ResultString) * GlobalConfig.CharWidth;
            DrawMenuText(ResultString, (i == State->CurrentSelection) ? MenuWidth : 0, EntriesPosX,
                         Screen->Entries[i]->Place.YPos, TitleLen + Screen->Entries[i]->Row);
          }
        } else if (Screen->Entries[i]->Tag == TAG_SWITCH) {
          StrCpy(ResultString, Screen->Entries[i]->Title);
          DrawMenuText(ResultString,
                       (i == State->CurrentSelection) ? MenuWidth : 0,
                       EntriesPosX + (TextHeight + TEXT_XMARGIN), Screen->Entries[i]->Place.YPos, 0xFFFF);
          BltImageCompositeIndicator((Screen->Entries[i]->Row == OldChosenTheme) ? Buttons[1] : Buttons[0], Buttons[0],
                                     EntriesPosX + TEXT_XMARGIN, Screen->Entries[i]->Place.YPos + PlaceCentre, 16);
        } else {
//          DBG("paint entry %d title=%s\n", i, Screen->Entries[i]->Title);
          DrawMenuText(Screen->Entries[i]->Title,
                       (i == State->CurrentSelection) ? MenuWidth : 0,
                       EntriesPosX, Screen->Entries[i]->Place.YPos, 0xFFFF);
        }
      }

      ScrollingBar(State);
      //MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      // blackosx swapped this around so drawing of selection comes before drawing scrollbar.

      // redraw selection cursor
      //usr-sse2
      if (Screen->Entries[State->LastSelection]->Tag == TAG_INPUT) {
        UINTN  TitleLen = StrLen(Screen->Entries[State->LastSelection]->Title);
        StrCpy(ResultString, Screen->Entries[State->LastSelection]->Title);
        if (((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->ItemType == BoolValue) {
          DrawMenuText(ResultString, 0, EntriesPosX + (TextHeight + TEXT_XMARGIN),
                       EntriesPosY + (State->LastSelection - State->FirstVisible) * TextHeight,
                       TitleLen + Screen->Entries[State->LastSelection]->Row);
          BltImageCompositeIndicator((((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->BValue)
                                     ? Buttons[3] : Buttons[2], Buttons[2], EntriesPosX + TEXT_XMARGIN,
                                     Screen->Entries[State->LastSelection]->Place.YPos + PlaceCentre, 16);
        } else {
          StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->SValue +
                 ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->LineShift);
        StrCat(ResultString, L" ");
          DrawMenuText(ResultString, 0, EntriesPosX,
                       EntriesPosY + (State->LastSelection - State->FirstVisible) * TextHeight,
                     TitleLen + Screen->Entries[State->LastSelection]->Row);
        }
      } else if (Screen->Entries[State->LastSelection]->Tag == TAG_SWITCH) {
        StrCpy(ResultString, Screen->Entries[State->LastSelection]->Title);
        DrawMenuText(ResultString, 0, EntriesPosX + (TextHeight + TEXT_XMARGIN),
                     EntriesPosY + (State->LastSelection - State->FirstVisible) * TextHeight, 0xFFFF);
        BltImageCompositeIndicator((Screen->Entries[State->LastSelection]->Row == OldChosenTheme) ? Buttons[1] :
                                   Buttons[0], Buttons[0], EntriesPosX + TEXT_XMARGIN,
                                   Screen->Entries[State->LastSelection]->Place.YPos + PlaceCentre, 16);
        
      } else {
        DrawMenuText(Screen->Entries[State->LastSelection]->Title, 0,
                     EntriesPosX, EntriesPosY + (State->LastSelection - State->FirstVisible) * TextHeight, 0xFFFF);
      }
      // Current selection
      if (Screen->Entries[State->CurrentSelection]->Tag == TAG_INPUT) {
        UINTN  TitleLen = StrLen(Screen->Entries[State->CurrentSelection]->Title);
        StrCpy(ResultString, Screen->Entries[State->CurrentSelection]->Title);
        if (((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->ItemType == BoolValue) {
          DrawMenuText(ResultString,
                       MenuWidth /* StrLen(ResultString) * GlobalConfig.CharWidth */,
                       EntriesPosX + (TextHeight + TEXT_XMARGIN),
                       EntriesPosY + (State->CurrentSelection - State->FirstVisible) * TextHeight,
                       TitleLen + Screen->Entries[State->CurrentSelection]->Row);
          BltImageCompositeIndicator((((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->BValue)
                                     ? Buttons[3] : Buttons[2], Buttons[2], EntriesPosX + TEXT_XMARGIN,
                                     Screen->Entries[State->CurrentSelection]->Place.YPos + PlaceCentre, 16);
        } else {
        StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->SValue + ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->LineShift);
        StrCat(ResultString, L" ");
          DrawMenuText(ResultString,
                       MenuWidth /* StrLen(ResultString) * GlobalConfig.CharWidth */,
                     EntriesPosX, EntriesPosY + (State->CurrentSelection - State->FirstVisible) * TextHeight,
                     TitleLen + Screen->Entries[State->CurrentSelection]->Row);
        }
      } else if (Screen->Entries[State->CurrentSelection]->Tag == TAG_SWITCH) {
        StrCpy(ResultString, Screen->Entries[State->CurrentSelection]->Title);
        DrawMenuText(ResultString, MenuWidth, EntriesPosX + (TextHeight + TEXT_XMARGIN),
                     EntriesPosY + (State->CurrentSelection - State->FirstVisible) * TextHeight, 0xFFFF);
        BltImageCompositeIndicator((Screen->Entries[State->CurrentSelection]->Row == OldChosenTheme) ? Buttons[1] :
                                   Buttons[0], Buttons[0], EntriesPosX + TEXT_XMARGIN,
                                   Screen->Entries[State->CurrentSelection]->Place.YPos + PlaceCentre, 16);
      } else {
        DrawMenuText(Screen->Entries[State->CurrentSelection]->Title, MenuWidth,
                     EntriesPosX, EntriesPosY + (State->CurrentSelection - State->FirstVisible) * TextHeight, 0xFFFF);
      }

      ScrollStart.YPos = ScrollbarBackground.YPos + ScrollbarBackground.Height * State->FirstVisible / (State->MaxIndex + 1);
      Scrollbar.YPos = ScrollStart.YPos + ScrollStart.Height;
      ScrollEnd.YPos = Scrollbar.YPos + Scrollbar.Height; // ScrollEnd.Height is already subtracted
      ScrollingBar(State);

      //MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      X = (UGAWidth - StrLen(ParamText) * GlobalConfig.CharWidth) >> 1;
      DrawMenuText(ParamText, 0, X, TimeoutPosY, 0xFFFF);
      break;
  }
  
  MouseBirth();
}

//
// graphical main menu style
//

static VOID DrawMainMenuEntry(REFIT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos)
{
//  EG_IMAGE *TmpBuffer = NULL;
  INTN Scale = GlobalConfig.MainEntriesSize >> 3;
/*  
  if (GlobalConfig.BootCampStyle && (Entry->Row == 1)) {
    return;
  }
*/
  if (((Entry->Tag == TAG_LOADER) || (Entry->Tag == TAG_LEGACY)) &&
      !(GlobalConfig.HideBadges & HDBADGES_SWAP) &&
      (Entry->Row == 0)) {
    MainImage = Entry->DriveImage;
  } else {
    MainImage = Entry->Image;
  }

  if (!MainImage) {
    if (ThemeDir) {
#if defined(ADVICON)
      MainImage = egLoadIcon(ThemeDir, GetIconsExt(L"icons\\mac", L"icns"), Scale << 3);
#else //ADVICON
      MainImage = egLoadIcon(ThemeDir, L"icons\\osx.icns", Scale << 3);
#endif //ADVICON
    }
    if (!MainImage) {
      MainImage = DummyImage(Scale << 3);
    }
  }
  //  DBG("Entry title=%s; Width=%d\n", Entry->Title, MainImage->Width);
  Scale = ((Entry->Row == 0) ? (Scale * (selected ? 1 : -1)): 16) ;
  if (GlobalConfig.SelectionOnTop) {
    SelectionImages[0]->HasAlpha = TRUE;
    SelectionImages[2]->HasAlpha = TRUE;
    SelectionImages[4]->HasAlpha = TRUE;
    //MainImage->HasAlpha = TRUE;
#if defined(ADVICON)
    BltImageCompositeBadge(
      MainImage,
      SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
      selected
        ? (Entry->ImageHover ? Entry->ImageHover : ((Entry->Row == 0) ? Entry->BadgeImage : NULL))
        : ((Entry->Row == 0) ? Entry->BadgeImage : NULL),
      XPos, YPos, Scale);
#else //ADVICON
    BltImageCompositeBadge(MainImage,
                           SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
                           (Entry->Row == 0) ? Entry->BadgeImage:NULL,
                           XPos, YPos, Scale);
#endif //ADVICON

  } else {
#if defined(ADVICON)
    BltImageCompositeBadge(
      SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
      MainImage,
      selected
        ? (Entry->ImageHover ? Entry->ImageHover : ((Entry->Row == 0) ? Entry->BadgeImage : NULL))
        : ((Entry->Row == 0) ? Entry->BadgeImage : NULL),
      XPos, YPos, Scale);
#else //ADVICON
    BltImageCompositeBadge(SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
                           MainImage, (Entry->Row == 0) ? Entry->BadgeImage:NULL,
                           XPos, YPos, Scale);
#endif //ADVICON
  }
    
  if (GlobalConfig.BootCampStyle) {
    if (Entry->Row == 0) {
      BltImageCompositeIndicator(SelectionImages[(4) + (selected ? 0 : 1)], SelectionImages[5],
                                     XPos + (row0TileSize / 2) - (INDICATOR_SIZE / 2),
                                     row0PosY + row0TileSize
                                     + ((GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL) ? 10 :
                                        (FontHeight - TEXT_YMARGIN + 20)), Scale);
    }
  }
    
  Entry->Place.XPos = XPos;
  Entry->Place.YPos = YPos;
  Entry->Place.Width = MainImage->Width;
  Entry->Place.Height = MainImage->Height;
}

static VOID FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height, IN EG_PIXEL *Color, IN UINT8 XAlign)
{
  EG_IMAGE *TmpBuffer = NULL;
  INTN X = XPos - (Width >> XAlign);

  if (!Width || !Height) return;

  TmpBuffer = egCreateImage(Width, Height, FALSE);
  if (!BackgroundImage) {
    egFillImage(TmpBuffer, Color);
  } else {
    egRawCopy(TmpBuffer->PixelData,
              BackgroundImage->PixelData + YPos * BackgroundImage->Width + X,
              Width, Height,
              TmpBuffer->Width,
              BackgroundImage->Width);
  }
  BltImage(TmpBuffer, X, YPos);
  egFreeImage(TmpBuffer);
}

static VOID DrawMainMenuLabel(IN CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State)
{
  INTN TextWidth;

  egMeasureText(Text, &TextWidth, NULL);
  
  //Clear old text
  if (OldTextWidth > TextWidth) {
    FillRectAreaOfScreen(OldX, OldY, OldTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
  }
    
  if (!(GlobalConfig.BootCampStyle)
      && (GlobalConfig.HideBadges & HDBADGES_INLINE) && (!OldRow)
      && (OldTextWidth) && (OldTextWidth != TextWidth)) {
    //Clear badge
    BltImageAlpha(NULL, (OldX - (OldTextWidth >> 1) - (BADGE_DIMENSION + 16)),
                  (OldY - ((BADGE_DIMENSION - TextHeight) >> 1)), &MenuBackgroundPixel, BADGE_DIMENSION >> 3);
  }
  DrawTextXY(Text, XPos, YPos, X_IS_CENTER);

  //show inline badge
  if (!(GlobalConfig.BootCampStyle) &&
       (GlobalConfig.HideBadges & HDBADGES_INLINE) &&
       (Screen->Entries[State->CurrentSelection]->Row == 0)) {
    // Display Inline Badge: small icon before the text
    BltImageAlpha(((LOADER_ENTRY*)Screen->Entries[State->CurrentSelection])->me.Image,
                  (XPos - (TextWidth >> 1) - (BADGE_DIMENSION + 16)),
                  (YPos - ((BADGE_DIMENSION - TextHeight) >> 1)), &MenuBackgroundPixel, BADGE_DIMENSION >> 3);
  }

  OldX = XPos;
  OldY = YPos;
  OldTextWidth = TextWidth;
  OldRow = Screen->Entries[State->CurrentSelection]->Row;
}

VOID CountItems(IN REFIT_MENU_SCREEN *Screen)
{
  INTN i;
  row0PosX = 0;
  row1PosX = Screen->EntryCount;
  // layout
  row0Count = 0; //Nr items in row0
  row1Count = 0;
  for (i = 0; i < (INTN)Screen->EntryCount; i++) {
    if (Screen->Entries[i]->Row == 0) {
      row0Count++;
      CONSTRAIN_MIN(row0PosX, i);
    } else {
      row1Count++;
      CONSTRAIN_MAX(row1PosX, i);
    }
  }
}

VOID DrawTextCorner(UINTN TextC, UINT8 Align)
{
  INTN    Xpos;
  CHAR16  *Text;

  if (
    // HIDEUI_ALL - included
    ((TextC == TEXT_CORNER_REVISION) && ((GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION) != 0)) ||
    ((TextC == TEXT_CORNER_HELP) && ((GlobalConfig.HideUIFlags & HIDEUI_FLAG_HELP) != 0))
  ) {
    return;
  }

  switch (TextC) {
    case TEXT_CORNER_REVISION:
#ifdef FIRMWARE_REVISION
      Text = FIRMWARE_REVISION;
#else
      Text = gST->FirmwareRevision;
#endif
      break;
    case TEXT_CORNER_HELP:
      Text = L"F1:Help";
      break;
    default:
      return;
  }

  switch (Align) {
    case X_IS_LEFT:
      Xpos = 5;
      break;
    case X_IS_RIGHT:
      Xpos = UGAWidth - 5;//2
      break;
    case X_IS_CENTER: //not used
      Xpos = UGAWidth >> 1;
      break;
    default:
      return;
  }

  DrawTextXY(Text, Xpos, UGAHeight - 5 - TextHeight, Align);
}

VOID MainMenuVerticalStyle(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText)
{
  INTN i;
  INTN row0PosYRunning;
  INTN VisibleHeight = 0; //assume vertical layout

  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime(Screen);
      SwitchToGraphicsAndClear();
      //adjustable by theme.plist?
      EntriesPosY = LAYOUT_Y_EDGE;
      EntriesGap = GlobalConfig.TileYSpace;
      EntriesWidth = GlobalConfig.MainEntriesSize + 16;
      EntriesHeight = GlobalConfig.MainEntriesSize + 16;
      //
      VisibleHeight = (UGAHeight - EntriesPosY - LAYOUT_Y_EDGE + EntriesGap) / (EntriesHeight + EntriesGap);
      EntriesPosX = UGAWidth - EntriesWidth - BAR_WIDTH - LAYOUT_X_EDGE;
      TimeoutPosY = UGAHeight - LAYOUT_Y_EDGE - TextHeight;

      CountItems(Screen);
      InitScroll(State, row0Count, Screen->EntryCount, VisibleHeight, 0);
      row0PosX = EntriesPosX;
      row0PosY = EntriesPosY;
      row1PosX = (UGAWidth + EntriesGap - (row1TileSize + TILE_XSPACING) * row1Count) >> 1;
      textPosY = TimeoutPosY - GlobalConfig.TileYSpace - TextHeight;
      row1PosY = textPosY - row1TileSize - GlobalConfig.TileYSpace - LayoutTextOffset;
      if (!itemPosX) {
        itemPosX = AllocatePool(sizeof(UINT64) * Screen->EntryCount);
        itemPosY = AllocatePool(sizeof(UINT64) * Screen->EntryCount);
      }
      row0PosYRunning = row0PosY;
      row1PosXRunning = row1PosX;
      //     DBG("EntryCount =%d\n", Screen->EntryCount);
      for (i = 0; i < (INTN)Screen->EntryCount; i++) {
        if (Screen->Entries[i]->Row == 0) {
          itemPosX[i] = row0PosX;
          itemPosY[i] = row0PosYRunning;
          row0PosYRunning += EntriesHeight + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          itemPosY[i] = row1PosY;
          row1PosXRunning += row1TileSize + TILE_XSPACING;
          //         DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
      InitSelection();

      // Update FilmPlace only if not set by InitAnime
      if (Screen->FilmPlace.Width == 0 || Screen->FilmPlace.Height == 0) {
        CopyMem(&Screen->FilmPlace, &BannerPlace, sizeof(BannerPlace));
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
      SetBar(EntriesPosX + EntriesWidth + 10, EntriesPosY, UGAHeight - LAYOUT_Y_EDGE, State);
      for (i = 0; i <= State->MaxIndex; i++) {
        if (Screen->Entries[i]->Row == 0) {
          if ((i >= State->FirstVisible) && (i <= State->LastVisible)) {
            DrawMainMenuEntry(Screen->Entries[i], (i == State->CurrentSelection)?1:0,
                              itemPosX[i - State->FirstVisible], itemPosY[i - State->FirstVisible]);
          }
        } else { //row1
          DrawMainMenuEntry(Screen->Entries[i], (i == State->CurrentSelection)?1:0,
                            itemPosX[i], itemPosY[i]);
        }
      }
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        DrawMainMenuLabel(Screen->Entries[State->CurrentSelection]->Title,
                          (UGAWidth >> 1), textPosY, Screen, State);
      }

      ScrollingBar(State);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      if (Screen->Entries[State->LastSelection]->Row == 0) {
        DrawMainMenuEntry(Screen->Entries[State->LastSelection], FALSE,
                          itemPosX[State->LastSelection - State->FirstVisible],
                          itemPosY[State->LastSelection - State->FirstVisible]);
      } else {
        DrawMainMenuEntry(Screen->Entries[State->LastSelection], FALSE,
                          itemPosX[State->LastSelection],
                          itemPosY[State->LastSelection]);
      }

      if (Screen->Entries[State->CurrentSelection]->Row == 0) {
        DrawMainMenuEntry(Screen->Entries[State->CurrentSelection], TRUE,
                          itemPosX[State->CurrentSelection - State->FirstVisible],
                          itemPosY[State->CurrentSelection - State->FirstVisible]);
      } else {
        DrawMainMenuEntry(Screen->Entries[State->CurrentSelection], TRUE,
                          itemPosX[State->CurrentSelection],
                          itemPosY[State->CurrentSelection]);
      }
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Screen->Entries[State->CurrentSelection]->Title,
                          (UGAWidth >> 1), textPosY, Screen, State);
      }

      ScrollingBar(State);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      i = (GlobalConfig.HideBadges & HDBADGES_INLINE)?3:1;
      HidePointer();
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        FillRectAreaOfScreen((UGAWidth >> 1), textPosY + TextHeight * i,
                             OldTimeoutTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
        OldTimeoutTextWidth = DrawTextXY(ParamText, (UGAWidth >> 1), textPosY + TextHeight * i, X_IS_CENTER);
      }

      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_LEFT);
      break;
  }
}

VOID MainMenuStyle(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText)
{
  INTN i;

  switch (Function) {

    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      InitAnime(Screen);
      SwitchToGraphicsAndClear();

      EntriesGap = GlobalConfig.TileXSpace;
      EntriesWidth = GlobalConfig.MainEntriesSize + (16 * row0TileSize) / 144;
      EntriesHeight = GlobalConfig.MainEntriesSize + 16;

      MaxItemOnScreen = (UGAWidth - ROW0_SCROLLSIZE * 2) / (EntriesWidth + EntriesGap); //8
      CountItems(Screen);
      InitScroll(State, row0Count, Screen->EntryCount, MaxItemOnScreen, 0);
      row0PosX = (UGAWidth + 8 - (EntriesWidth + EntriesGap) *
                  ((MaxItemOnScreen < row0Count)?MaxItemOnScreen:row0Count)) >> 1;
      row0PosY = ((UGAHeight - LayoutMainMenuHeight) >> 1) + LayoutBannerOffset; //LAYOUT_BANNER_YOFFSET;

      row1PosX = (UGAWidth + 8 - (row1TileSize + TILE_XSPACING) * row1Count) >> 1;
          
      if (GlobalConfig.BootCampStyle) {
        row1PosY = row0PosY + row0TileSize + LayoutButtonOffset + GlobalConfig.TileYSpace + INDICATOR_SIZE
                     + ((GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL) ? 15 : (FontHeight + 30));
      } else {
        row1PosY = row0PosY + EntriesHeight + GlobalConfig.TileYSpace + LayoutButtonOffset;
      }
      
      if (row1Count > 0) {
        if (GlobalConfig.BootCampStyle) {
          textPosY = row0PosY + row0TileSize + 10;
        } else {
          textPosY = row1PosY + row1TileSize + GlobalConfig.TileYSpace + LayoutTextOffset;
        }
      } else {
        if (GlobalConfig.BootCampStyle) {
          textPosY = row0PosY + row0TileSize + 10;
        } else {
          textPosY = row1PosY;
        }
      }
      
      FunctextPosY = row1PosY + row1TileSize + GlobalConfig.TileYSpace + LayoutTextOffset;
      if (!itemPosX) {
        itemPosX = AllocatePool(sizeof(UINT64) * Screen->EntryCount);
      }

      row0PosXRunning = row0PosX;
      row1PosXRunning = row1PosX;
      //DBG("EntryCount =%d\n", Screen->EntryCount);
      for (i = 0; i < (INTN)Screen->EntryCount; i++) {
        if (Screen->Entries[i]->Row == 0) {
          itemPosX[i] = row0PosXRunning;
          row0PosXRunning += EntriesWidth + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          row1PosXRunning += row1TileSize + TILE_XSPACING;
          //DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
      InitSelection();

      // Update FilmPlace only if not set by InitAnime
      if (Screen->FilmPlace.Width == 0 || Screen->FilmPlace.Height == 0) {
        CopyMem(&Screen->FilmPlace, &BannerPlace, sizeof(BannerPlace));
      }

      //DBG("main menu inited\n");
      break;

    case MENU_FUNCTION_CLEANUP:
      FreePool(itemPosX);
      itemPosX = NULL;
      HidePointer();
      break;

    case MENU_FUNCTION_PAINT_ALL:
      for (i = 0; i <= State->MaxIndex; i++) {
        if (Screen->Entries[i]->Row == 0) {
          if ((i >= State->FirstVisible) && (i <= State->LastVisible)) {
            DrawMainMenuEntry(Screen->Entries[i], (i == State->CurrentSelection)?1:0,
                              itemPosX[i - State->FirstVisible], row0PosY);
          // create static text for the boot options if the BootCampStyle is used
          if (GlobalConfig.BootCampStyle && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
              FillRectAreaOfScreen(itemPosX[i - State->FirstVisible] + (row0TileSize / 2), textPosY,
                                   EntriesWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
              DrawBCSText(Screen->Entries[i]->Title, itemPosX[i - State->FirstVisible] + (row0TileSize / 2),
                           textPosY, X_IS_CENTER);
          }
          }
        } else {
          DrawMainMenuEntry(Screen->Entries[i], (i == State->CurrentSelection)?1:0,
                            itemPosX[i], row1PosY);
        }
      }
      
      // clear the text from the second row, required by the BootCampStyle
      if ((GlobalConfig.BootCampStyle) && (Screen->Entries[State->LastSelection]->Row == 1)
          && (Screen->Entries[State->CurrentSelection]->Row == 0) && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
          FillRectAreaOfScreen((UGAWidth >> 1), FunctextPosY,
                               OldTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
      }
      
      // something is wrong with the DrawMainMenuLabel or Screen->Entries[State->CurrentSelection]
      // and it's required to create the first selection text from here
      // used for the second row entries, when BootCampStyle is used
      if ((Screen->Entries[State->LastSelection]->Row == 0) && (Screen->Entries[State->CurrentSelection]->Row == 1)
          && GlobalConfig.BootCampStyle && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
          DrawMainMenuLabel(Screen->Entries[State->CurrentSelection]->Title,
                            (UGAWidth >> 1), FunctextPosY, Screen, State);
      }
          
      // something is wrong with the DrawMainMenuLabel or Screen->Entries[State->CurrentSelection]
      // and it's required to create the first selection text from here
      // used for all the entries
      if (!(GlobalConfig.BootCampStyle) && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
        DrawMainMenuLabel(Screen->Entries[State->CurrentSelection]->Title,
                            (UGAWidth >> 1), textPosY, Screen, State);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      if (Screen->Entries[State->LastSelection]->Row == 0) {
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
      
      // create dynamic text for the second row if BootCampStyle is used
      if ((GlobalConfig.BootCampStyle) && (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL))
          && Screen->Entries[State->CurrentSelection]->Row == 1) {
          DrawMainMenuLabel(Screen->Entries[State->CurrentSelection]->Title,
                            (UGAWidth >> 1), FunctextPosY, Screen, State);
      }
      
      // create dynamic text for all the entries
      if ((!(GlobalConfig.BootCampStyle)) && (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL))) {
          DrawMainMenuLabel(Screen->Entries[State->CurrentSelection]->Title,
                            (UGAWidth >> 1), textPosY, Screen, State);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      MouseBirth();
      break;

    case MENU_FUNCTION_PAINT_TIMEOUT:
      i = (GlobalConfig.HideBadges & HDBADGES_INLINE)?3:1;
      HidePointer();
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        FillRectAreaOfScreen((UGAWidth >> 1), FunctextPosY + TextHeight * i,
                                   OldTimeoutTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
        OldTimeoutTextWidth = DrawTextXY(ParamText, (UGAWidth >> 1), FunctextPosY + TextHeight * i, X_IS_CENTER);
      }

      DrawTextCorner(TEXT_CORNER_HELP, X_IS_LEFT);
      DrawTextCorner(TEXT_CORNER_REVISION, X_IS_RIGHT);
      MouseBirth();
      break;

  }
}


REFIT_MENU_ENTRY  *SubMenuGraphics()
{
  UINTN  i, N, Ven = 97;
  REFIT_MENU_ENTRY   *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"Graphics Injector ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_GRAPHICS;
  SubScreen->AnimeRun = GetAnime(SubScreen);
  AddMenuInfoLine(SubScreen, PoolPrint(L"Number of VideoCards=%d", NGFX));

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"InjectEDID");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[52];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  for (i = 0; i < NGFX; i++) {
    AddMenuInfoLine(SubScreen, PoolPrint(L"Card DeviceID=%04x", gGraphics[i].DeviceID));
    N = 20 + i * 6;
    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs->Entry.Title = PoolPrint(L"Model:");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[N].SValue); //cursor
    InputBootArgs->Item = &InputItems[N];
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    if (gGraphics[i].Vendor == Nvidia) {
      InputBootArgs->Entry.Title = PoolPrint(L"InjectNVidia");
    } else if (gGraphics[i].Vendor == Ati) {
      InputBootArgs->Entry.Title = PoolPrint(L"InjectATI");
    } else if (gGraphics[i].Vendor == Intel) {
      InputBootArgs->Entry.Title = PoolPrint(L"InjectIntel");
    } else {
      InputBootArgs->Entry.Title = PoolPrint(L"InjectX3");
    }
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = 0xFFFF; //cursor
    InputBootArgs->Item = &InputItems[N+1];
    InputBootArgs->Entry.AtClick = ActionEnter;
    InputBootArgs->Entry.AtRightClick = ActionDetails;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    if (gGraphics[i].Vendor == Nvidia) {
      Ven = 95;
    } else if (gGraphics[i].Vendor == Ati) {
      Ven = 94;
    } else /*if (gGraphics[i].Vendor == Intel)*/ {
      Ven = 96;
    }
    InputBootArgs->Entry.Title = PoolPrint(L"FakeID:");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[Ven].SValue); //cursor
    InputBootArgs->Item = &InputItems[Ven];
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    if (gGraphics[i].Vendor == Nvidia) {
      InputBootArgs->Entry.Title = PoolPrint(L"DisplayCFG:");
    } else if (gGraphics[i].Vendor == Ati) {
      InputBootArgs->Entry.Title = PoolPrint(L"FBConfig:");
    } else /*if (gGraphics[i].Vendor == Intel)*/{
      InputBootArgs->Entry.Title = PoolPrint(L"*-platform-id:");
    }
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[N+2].SValue); //cursor
    InputBootArgs->Item = &InputItems[N+2];
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

    // ErmaC: NvidiaGeneric entry
    if (gGraphics[i].Vendor == Nvidia) {
       InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
       InputBootArgs->Entry.Title = PoolPrint(L"Generic NVIDIA name");
       InputBootArgs->Entry.Tag = TAG_INPUT;
       InputBootArgs->Entry.Row = 0xFFFF; //cursor
       InputBootArgs->Item = &InputItems[43];
       InputBootArgs->Entry.AtClick = ActionEnter;
       InputBootArgs->Entry.AtRightClick = ActionDetails;
       AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
    }

    if (gGraphics[i].Vendor == Intel) {
      continue;
    }

    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs->Entry.Title = PoolPrint(L"Ports:");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[N+3].SValue); //cursor
    InputBootArgs->Item = &InputItems[N+3];
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

    if (gGraphics[i].Vendor == Nvidia) {
      InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputBootArgs->Entry.Title = PoolPrint(L"NVCAP:");
      InputBootArgs->Entry.Tag = TAG_INPUT;
      InputBootArgs->Entry.Row = StrLen(InputItems[N+4].SValue); //cursor
      InputBootArgs->Item = &InputItems[N+4];
      InputBootArgs->Entry.AtClick = ActionSelect;
      InputBootArgs->Entry.AtDoubleClick = ActionEnter;
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
    } else {
      InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputBootArgs->Entry.Title = PoolPrint(L"RefCLK:");
      InputBootArgs->Entry.Tag = TAG_INPUT;
      InputBootArgs->Entry.Row = StrLen(InputItems[50].SValue); //cursor
      InputBootArgs->Item = &InputItems[50];
      InputBootArgs->Entry.AtClick = ActionSelect;
      InputBootArgs->Entry.AtDoubleClick = ActionEnter;
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
    }

    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs->Entry.Title = PoolPrint(L"LoadVideoBios");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = 0xFFFF; //cursor
    InputBootArgs->Item = &InputItems[N+5];
    InputBootArgs->Entry.AtClick = ActionEnter;
    InputBootArgs->Entry.AtRightClick = ActionDetails;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  }

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Backlight Level:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[18].SValue); //cursor
  InputBootArgs->Item = &InputItems[18];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);


  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}

#define nya(x) x/10,x%10

REFIT_MENU_ENTRY  *SubMenuSpeedStep()
{
  REFIT_MENU_ENTRY   *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"CPU tuning ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_CPU;
  SubScreen->AnimeRun = GetAnime(SubScreen);
  AddMenuInfoLine(SubScreen, PoolPrint(L"%a", gCPUStructure.BrandString));
  AddMenuInfoLine(SubScreen, PoolPrint(L"Model: %2x/%2x/%2x",
      gCPUStructure.Family, gCPUStructure.Model, gCPUStructure.Stepping));
  AddMenuInfoLine(SubScreen, PoolPrint(L"Cores: %d Threads: %d",
                  gCPUStructure.Cores, gCPUStructure.Threads));
  AddMenuInfoLine(SubScreen, PoolPrint(L"FSB speed MHz: %d",
                  DivU64x32(gCPUStructure.FSBFrequency, Mega)));
  AddMenuInfoLine(SubScreen, PoolPrint(L"CPU speed MHz: %d",
                  DivU64x32(gCPUStructure.CPUFrequency, Mega)));
  AddMenuInfoLine(SubScreen, PoolPrint(L"Ratio: Min=%d.%d Max=%d.%d Turbo=%d.%d/%d.%d/%d.%d/%d.%d",
     nya(gCPUStructure.MinRatio), nya(gCPUStructure.MaxRatio),
     nya(gCPUStructure.Turbo4), nya(gCPUStructure.Turbo3), nya(gCPUStructure.Turbo2), nya(gCPUStructure.Turbo1)));

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Cores enabled:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[76].SValue); //cursor
  InputBootArgs->Entry.ShortcutLetter = 'E';
  InputBootArgs->Item = &InputItems[76];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtRightClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"GeneratePStates");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = 'G';
  InputBootArgs->Item = &InputItems[5];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Halt Enabler");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = 'H';
  InputBootArgs->Item = &InputItems[6];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"PLimitDict:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[7].SValue); //cursor
  InputBootArgs->Entry.ShortcutLetter = 'P';
  InputBootArgs->Item = &InputItems[7];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"UnderVoltStep:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[8].SValue); //cursor
  InputBootArgs->Entry.ShortcutLetter = 'U';
  InputBootArgs->Item = &InputItems[8];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"DoubleFirstState");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = 'D';
  InputBootArgs->Item = &InputItems[88];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtDoubleClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"GenerateCStates");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = 'C';
  InputBootArgs->Item = &InputItems[9];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"EnableC2");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = '2';
  InputBootArgs->Item = &InputItems[10];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"EnableC4");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = '4';
  InputBootArgs->Item = &InputItems[11];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"EnableC6");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = '6';
  InputBootArgs->Item = &InputItems[12];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"EnableC7");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = '7';
  InputBootArgs->Item = &InputItems[89];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Use SystemIO");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = 'S';
  InputBootArgs->Item = &InputItems[13];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"C3Latency:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[75].SValue); //cursor
  InputBootArgs->Entry.ShortcutLetter = 'L';
  InputBootArgs->Item = &InputItems[75];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"BusSpeed [kHz]:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[19].SValue); //cursor
  InputBootArgs->Entry.ShortcutLetter = 'B';
  InputBootArgs->Item = &InputItems[19];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"QPI:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[14].SValue); //cursor
  InputBootArgs->Entry.ShortcutLetter = 'Q';
  InputBootArgs->Item = &InputItems[14];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Saving Mode:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[77].SValue); //cursor
  InputBootArgs->Entry.ShortcutLetter = 'V';
  InputBootArgs->Item = &InputItems[77];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  //15
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"PatchAPIC");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  //InputBootArgs->Entry.ShortcutDigit = 0;
  InputBootArgs->Item = &InputItems[15];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}

REFIT_MENU_ENTRY  *SubMenuBinaries()
{
  REFIT_MENU_ENTRY   *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"Binaries patching ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_BINARIES;
  SubScreen->AnimeRun = GetAnime(SubScreen);
  AddMenuInfoLine(SubScreen, PoolPrint(L"%a", gCPUStructure.BrandString));
  AddMenuInfoLine(SubScreen, PoolPrint(L"Real CPUID: 0x%06x", gCPUStructure.Signature));


  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fake CPUID:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[104].SValue); //cursor
//  InputBootArgs->Entry.ShortcutLetter = 'I';
  InputBootArgs->Item = &InputItems[104];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtRightClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kext patching allowed");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[44];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kernel Support CPU");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[45];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kernel Lapic Patch");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[91];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kernel Haswell-E Patch");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[105];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kernel PM Patch");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[48];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"AppleIntelCPUPM patch");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[46];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"AppleRTC patch");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[47];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}

REFIT_MENU_ENTRY  *SubMenuDropTables()
{
  CHAR8               sign[5];
  CHAR8               OTID[9];
 // CHAR16*             Flags;
  REFIT_MENU_ENTRY    *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN   *SubScreen;
  REFIT_INPUT_DIALOG  *InputBootArgs;

  sign[4] = 0;
  OTID[8] = 0;
//  Flags = AllocateZeroPool(255);

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"Tables dropping ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_TABLES;
  SubScreen->AnimeRun = GetAnime(SubScreen);

  //AddMenuInfoLine(SubScreen, L"OEM ACPI TABLE:");
  if (gSettings.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
    while (DropTable) {
      CopyMem((CHAR8*)&sign, (CHAR8*)&(DropTable->Signature), 4);
      CopyMem((CHAR8*)&OTID, (CHAR8*)&(DropTable->TableId), 8);

      //MsgLog("adding to menu %a (%x) %a (%lx) L=%d(0x%x)\n",
      //       sign, DropTable->Signature,
      //       OTID, DropTable->TableId,
      //       DropTable->Length, DropTable->Length);
      InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputBootArgs->Entry.Title = PoolPrint(L"Drop \"%4.4a\" \"%8.8a\" %d", sign, OTID, DropTable->Length);
      InputBootArgs->Entry.Tag = TAG_INPUT;
      InputBootArgs->Entry.Row = 0xFFFF; //cursor
      InputBootArgs->Item = &(DropTable->MenuItem);
      InputBootArgs->Entry.AtClick = ActionEnter;
      InputBootArgs->Entry.AtRightClick = ActionDetails;
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

      DropTable = DropTable->Next;
    }
  }

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Drop all OEM SSDT");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  //InputBootArgs->Entry.ShortcutDigit = 0;
  InputBootArgs->Entry.ShortcutLetter = 'S';
  InputBootArgs->Item = &InputItems[4];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  //AddMenuInfoLine(SubScreen, L"PATCHED AML:");
  if (ACPIPatchedAML) {
    ACPI_PATCHED_AML *ACPIPatchedAMLTmp = ACPIPatchedAML;
    while (ACPIPatchedAMLTmp) {
      InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputBootArgs->Entry.Title = PoolPrint(L"Drop \"%s\"", ACPIPatchedAMLTmp->FileName);
      InputBootArgs->Entry.Tag = TAG_INPUT;
      InputBootArgs->Entry.Row = 0xFFFF; //cursor
      InputBootArgs->Item = &(ACPIPatchedAMLTmp->MenuItem);
      InputBootArgs->Entry.AtClick = ActionEnter;
      InputBootArgs->Entry.AtRightClick = ActionDetails;
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
      ACPIPatchedAMLTmp = ACPIPatchedAMLTmp->Next;
    }
  }

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}

REFIT_MENU_ENTRY  *SubMenuSysVariables()
{
  REFIT_MENU_ENTRY    *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN   *SubScreen;
  REFIT_INPUT_DIALOG  *InputRT;

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"System Variables ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;

  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_SYSVARS;
  SubScreen->AnimeRun = GetAnime(SubScreen);

  AddMenuInfoLine(SubScreen, L"More: SystemParameters -> ExposeSysVariables = TRUE");
  if (SysVariables) {
    SYSVARIABLES *SysVariablesTmp = SysVariables;
    while (SysVariablesTmp) {
      InputRT = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputRT->Entry.Title = PoolPrint((SysVariablesTmp->MenuItem.ItemType == BoolValue) ? L"%s" : L"%s:",
                                       SysVariablesTmp->Key);
      InputRT->Entry.Tag = TAG_INPUT;
      InputRT->Entry.Row = (SysVariablesTmp->MenuItem.ItemType == BoolValue) ? 0xFFFF : StrLen(SysVariablesTmp->MenuItem.SValue); //cursor
      InputRT->Item = &(SysVariablesTmp->MenuItem);
      InputRT->Entry.AtClick = ActionEnter;
      InputRT->Entry.AtRightClick = ActionDetails;
      AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputRT);
      SysVariablesTmp = SysVariablesTmp->Next;
    }
  }

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}


REFIT_MENU_ENTRY  *SubMenuSmbios()
{
  REFIT_MENU_ENTRY   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"SMBIOS ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_SMBIOS;
  SubScreen->AnimeRun = GetAnime(SubScreen);
  AddMenuInfoLine(SubScreen, PoolPrint(L"%a", gCPUStructure.BrandString));
  AddMenuInfoLine(SubScreen, PoolPrint(L"%a", gSettings.OEMProduct));
  AddMenuInfoLine(SubScreen, PoolPrint(L"with board %a", gSettings.OEMBoard));

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Product name:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[78].SValue);
  InputBootArgs->Entry.ShortcutDigit = 0xF1;
  InputBootArgs->Item = &InputItems[78];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Product version:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[79].SValue);
  InputBootArgs->Item = &InputItems[79];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Product sn:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[80].SValue);
  InputBootArgs->Item = &InputItems[80];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Board ID:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[81].SValue);
  InputBootArgs->Item = &InputItems[81];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Board sn:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[82].SValue);
  InputBootArgs->Item = &InputItems[82];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Board type:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[83].SValue);
  InputBootArgs->Item = &InputItems[83];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Board version:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[84].SValue);
  InputBootArgs->Item = &InputItems[84];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Chassis type:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[85].SValue);
  InputBootArgs->Item = &InputItems[85];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"ROM version:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[86].SValue);
  InputBootArgs->Item = &InputItems[86];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"ROM release date:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[87].SValue);
  InputBootArgs->Item = &InputItems[87];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"PlatformFeature:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[17].SValue); //cursor
  InputBootArgs->Item = &InputItems[17];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}

REFIT_MENU_ENTRY  *SubMenuDsdtFix()
{
  REFIT_MENU_ENTRY   *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"DSDT fix mask [0x%08x]->", gSettings.FixDsdt);
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_DSDT;
  SubScreen->AnimeRun = GetAnime(SubScreen);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Debug DSDT");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[102];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"DSDT name:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[1].SValue);
  //InputBootArgs->Entry.ShortcutDigit = 0;
  InputBootArgs->Entry.ShortcutLetter = 'D';
  InputBootArgs->Entry.Image = NULL;
  InputBootArgs->Entry.BadgeImage = NULL;
  InputBootArgs->Entry.SubScreen = NULL;
  InputBootArgs->Item = &InputItems[1];    //1
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Drop _DSM:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[101].SValue);; //cursor
  InputBootArgs->Item = &InputItems[101];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add DTGP");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[53];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix Darwin");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[54];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix shutdown");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[55];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add MCHC");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[56];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix HPET");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[57];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fake LPC");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[58];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix IPIC");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[59];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add SMBUS");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[60];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix display");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[61];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix IDE");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[62];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix SATA");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[63];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix Firewire");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[64];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix USB");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[65];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix LAN");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[66];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix Airport");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[67];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix sound");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[68];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix new way");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[125];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix RTC");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[111];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix TMR");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[112];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add IMEI");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[113];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix IntelGFX");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[114];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix _WAK");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[115];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Del unused");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[116];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix ADP1");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[117];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add PNLF");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[118];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix S3D");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[119];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Rename ACST");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[120];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add HDMI");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[121];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix Regions");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[122];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}

REFIT_MENU_ENTRY  *SubMenuPCI()
{
  REFIT_MENU_ENTRY   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"PCI devices ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;

  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_USB;
  SubScreen->AnimeRun = GetAnime(SubScreen);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"USB Ownership");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  InputBootArgs->Item = &InputItems[74];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"USB Injection");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  InputBootArgs->Item = &InputItems[92];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Inject ClockID");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  InputBootArgs->Item = &InputItems[93];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Inject EFI Strings");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  InputBootArgs->Item = &InputItems[106];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"No Default Properties");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  InputBootArgs->Item = &InputItems[107];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID LAN:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[97].SValue); //cursor
  InputBootArgs->Item = &InputItems[97];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID WIFI:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[98].SValue); //cursor
  InputBootArgs->Item = &InputItems[98];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID SATA:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[99].SValue); //cursor
  InputBootArgs->Item = &InputItems[99];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID XHCI:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[100].SValue); //cursor
  InputBootArgs->Item = &InputItems[100];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID IMEI:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[103].SValue); //cursor
  InputBootArgs->Item = &InputItems[103];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}


REFIT_MENU_ENTRY  *SubMenuThemes()
{
  REFIT_MENU_ENTRY   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;
  UINTN               i;

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"Themes ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;

  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_THEME;
  SubScreen->AnimeRun = GetAnime(SubScreen);

  AddMenuInfoLine(SubScreen, L"Installed themes:");
  for (i = 0; i < ThemesNum; i++) {
    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs->Entry.Title = PoolPrint(L"%s", ThemesList[i]);
    InputBootArgs->Entry.Tag = TAG_SWITCH;
    InputBootArgs->Entry.Row = i;
    InputBootArgs->Item = &InputItems[3];
    InputBootArgs->Entry.AtClick = ActionEnter;
    InputBootArgs->Entry.AtRightClick = ActionDetails;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  }
  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  return Entry;
}


VOID  OptionsMenu(OUT REFIT_MENU_ENTRY **ChosenEntry)
{
  REFIT_MENU_ENTRY  *TmpChosenEntry = NULL;
  UINTN             MenuExit = 0;
  UINTN             SubMenuExit;
  //CHAR16*           Flags;
  MENU_STYLE_FUNC   Style = TextMenuStyle;
  MENU_STYLE_FUNC   SubStyle;
  INTN              EntryIndex = 0;
  INTN              SubMenuIndex;
  REFIT_INPUT_DIALOG* InputBootArgs;
  BOOLEAN           OldFontStyle = GlobalConfig.Proportional;

  GlobalConfig.Proportional = FALSE; //temporary disable proportional

  if (AllowGraphicsMode)
    Style = GraphicsMenuStyle;

  // remember, if you extended this menu then change procedures
  // FillInputs and ApplyInputs

  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    OptionMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_OPTIONS);
  } else {
    OptionMenu.TitleImage = NULL;
  }

  gThemeOptionsChanged = FALSE;

  if (OptionMenu.EntryCount == 0) {
    gThemeOptionsChanged = TRUE;
    OptionMenu.ID = SCREEN_OPTIONS;
    OptionMenu.AnimeRun = GetAnime(&OptionMenu); //FALSE;
    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    *ChosenEntry = (REFIT_MENU_ENTRY*)InputBootArgs;

    InputBootArgs->Entry.Title = PoolPrint(L"Config:");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[90].SValue);
    InputBootArgs->Entry.ShortcutDigit = 0xF1;
    InputBootArgs->Item = &InputItems[90];    //0
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);

    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs->Entry.Title = PoolPrint(L"Boot Args:");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[0].SValue);
    InputBootArgs->Item = &InputItems[0];
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);

    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs->Entry.Title = PoolPrint(L"Block kext:");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[2].SValue);
    InputBootArgs->Item = &InputItems[2];
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);

    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs->Entry.Title = PoolPrint(L"Set OS version if not:");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[51].SValue);
    InputBootArgs->Item = &InputItems[51];
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);

    if (AllowGraphicsMode) {
      InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputBootArgs->Entry.Title = PoolPrint(L"Pointer speed:");
      InputBootArgs->Entry.Tag = TAG_INPUT;
      InputBootArgs->Entry.Row = StrLen(InputItems[70].SValue); //cursor
      InputBootArgs->Entry.ShortcutLetter = 'P';
      InputBootArgs->Item = &InputItems[70];
      InputBootArgs->Entry.AtClick = ActionSelect;
      InputBootArgs->Entry.AtDoubleClick = ActionEnter;
      AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);

      InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputBootArgs->Entry.Title = PoolPrint(L"Mirror move");
      InputBootArgs->Entry.Tag = TAG_INPUT;
      InputBootArgs->Entry.Row = 0xFFFF;
      InputBootArgs->Item = &InputItems[72];
      InputBootArgs->Entry.AtClick = ActionEnter;
      InputBootArgs->Entry.AtRightClick = ActionDetails;
      AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);

      AddMenuEntry(&OptionMenu, SubMenuThemes());
    }

    AddMenuEntry(&OptionMenu, SubMenuDropTables());
    AddMenuEntry(&OptionMenu, SubMenuDsdtFix());
    AddMenuEntry(&OptionMenu, SubMenuSmbios());
    AddMenuEntry(&OptionMenu, SubMenuPCI());
    AddMenuEntry(&OptionMenu, SubMenuSpeedStep());
    AddMenuEntry(&OptionMenu, SubMenuGraphics());
    AddMenuEntry(&OptionMenu, SubMenuBinaries());
    if (SysVariables) {
      AddMenuEntry(&OptionMenu, SubMenuSysVariables());
    }

    AddMenuEntry(&OptionMenu, &MenuEntryReturn);
    //    DBG("option menu created entries=%d\n", OptionMenu.EntryCount);
  }

  while (!MenuExit) {

    MenuExit = RunGenericMenu(&OptionMenu, Style, &EntryIndex, ChosenEntry);
    if (MenuExit == MENU_EXIT_ESCAPE || (*ChosenEntry)->Tag == TAG_RETURN)
      break;
    if (MenuExit == MENU_EXIT_ENTER) {
      //enter input dialog or subscreen
      if ((*ChosenEntry)->SubScreen != NULL) {
        SubMenuIndex = -1;
        SubMenuExit = 0;
        SubStyle = Style;
        while (!SubMenuExit) {
          SubMenuExit = RunGenericMenu((*ChosenEntry)->SubScreen, SubStyle, &SubMenuIndex, &TmpChosenEntry);
          if (SubMenuExit == MENU_EXIT_ESCAPE || TmpChosenEntry->Tag == TAG_RETURN){
            ApplyInputs();
            if ((*ChosenEntry)->SubScreen->ID == SCREEN_DSDT) {
              UnicodeSPrint((*ChosenEntry)->Title, 255, L"DSDT fix mask [0x%08x]->", gSettings.FixDsdt);
       //       MsgLog("@ESC: %s\n", (*ChosenEntry)->Title);
            }
            break;
          }
          if (SubMenuExit == MENU_EXIT_ENTER) {
            //enter input dialog
            SubMenuExit = 0;
            if ((*ChosenEntry)->SubScreen->ID == SCREEN_DSDT) {
              CHAR16 *TmpTitle;
              ApplyInputs();
              TmpTitle = PoolPrint(L"DSDT fix mask [0x%08x]->", gSettings.FixDsdt);
      //        MsgLog("@ENTER: tmp=%s\n", TmpTitle);
              while (*TmpTitle) {
                *((*ChosenEntry)->Title)++ = *TmpTitle++;
              }
      //        MsgLog("@ENTER: chosen=%s\n", (*ChosenEntry)->Title);
            }
            if (TmpChosenEntry->ShortcutDigit == 0xF1) {
              MenuExit = MENU_EXIT_ENTER;
              //     DBG("Escape menu from input dialog\n");
              goto exit;
            } //if F1
          }
        } //while(!SubMenuExit)
      }
      MenuExit = 0;
      if ((*ChosenEntry)->ShortcutDigit == 0xF1) {
        MenuExit = MENU_EXIT_ENTER;
        //     DBG("Escape options menu\n");
        break;
      } //if F1
    } // if MENU_EXIT_ENTER
  }
exit:
  GlobalConfig.Proportional = OldFontStyle;
  ApplyInputs();
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
  REFIT_MENU_ENTRY *TempChosenEntry = 0;
  UINTN MenuExit = 0;
  INTN DefaultEntryIndex = DefaultSelection;
  INTN SubMenuIndex;

  if (AllowGraphicsMode) {
    Style = GraphicsMenuStyle;
    if (GlobalConfig.VerticalLayout) {
      MainStyle = MainMenuVerticalStyle;
    } else {
      MainStyle = MainMenuStyle;
    }
  }

  while (!MenuExit) {
    Screen->AnimeRun = MainAnime;
    MenuExit = RunGenericMenu(Screen, MainStyle, &DefaultEntryIndex, &TempChosenEntry);
    Screen->TimeoutSeconds = 0;

    if (MenuExit == MENU_EXIT_DETAILS && TempChosenEntry->SubScreen != NULL) {
      SubMenuIndex = -1;
      MenuExit = RunGenericMenu(TempChosenEntry->SubScreen, Style, &SubMenuIndex, &TempChosenEntry);
      if (MenuExit == MENU_EXIT_ENTER && TempChosenEntry->Tag == TAG_LOADER) {
        AsciiSPrint(gSettings.BootArgs, 255, "%s", ((LOADER_ENTRY*)TempChosenEntry)->LoadOptions);
      }
      if (MenuExit == MENU_EXIT_ESCAPE || TempChosenEntry->Tag == TAG_RETURN)
        MenuExit = 0;
    }
  }

  if (ChosenEntry) {
    *ChosenEntry = TempChosenEntry;
  }
  return MenuExit;
}

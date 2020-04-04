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

#include "menu.h"
//#include "Platform.h"
#include "../libeg/libegint.h"   //this includes platform.h
//#include "../include/scroll_images.h"

#include "../Platform/Settings.h"
#include "../../Version.h"
//#include "colors.h"

#include "../libeg/nanosvg.h"
#include "../libeg/FloatLib.h"
#include "HdaCodecDump.h"
#include "menu.h"
#include "screen.h"
#include "../cpp_foundation/XString.h"
#include "../libeg/XTheme.h"
#include "../libeg/VectorGraphics.h" // for testSVG
#include "../gui/shared_with_menu.h"

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
//#define VBIOS_BIN L"EFI\\CLOVER\\misc\\c0000.bin"
//CONST CHAR16 *VBIOS_BIN = L"EFI\\CLOVER\\misc\\c0000.bin"; // tmporarily moved in shared_wint_menu

//#define LSTR(s) L##s

// scrolling definitions
//static INTN MaxItemOnScreen = -1;

//#if USE_XTHEME
//REFIT_MENU_SCREEN OptionMenu(4, L"Options"_XSW, L""_XSW);
//#else
REFIT_MENU_SCREEN OptionMenu(4, L"Options", L""_XSW);
//#endif

//
//extern REFIT_MENU_ITEM_RETURN MenuEntryReturn;
//extern UINTN            ThemesNum;
//extern CONST CHAR16           *ThemesList[];
//extern UINTN            ConfigsNum;
//extern CHAR16           *ConfigsList[];
//extern UINTN            DsdtsNum;
//extern CHAR16           *DsdtsList[];
//extern UINTN            AudioNum;
//extern HDA_OUTPUTS      AudioList[20];
extern CONST CHAR8      *AudioOutputNames[];
//extern CHAR8            NonDetected[];
#include "../Platform/string.h"
//extern BOOLEAN          GetLegacyLanAddress;
//extern UINT8            gLanMac[4][6]; // their MAC addresses
//extern EFI_AUDIO_IO_PROTOCOL *AudioIo;
//
////layout must be in XTheme
//#if !USE_XTHEME
//INTN LayoutBannerOffset = 64;
//INTN LayoutButtonOffset = 0;
//INTN LayoutTextOffset = 0;
INTN LayoutMainMenuHeight = 376;
INTN LayoutAnimMoveForMenuX = 0;
//#endif
//
//BOOLEAN SavePreBootLog = FALSE;
//
//#define SCROLL_LINE_UP        (0)
//#define SCROLL_LINE_DOWN      (1)
//#define SCROLL_PAGE_UP        (2)
//#define SCROLL_PAGE_DOWN      (3)
//#define SCROLL_FIRST          (4)
//#define SCROLL_LAST           (5)
//#define SCROLL_NONE           (6)
//#define SCROLL_SCROLL_DOWN    (7)
//#define SCROLL_SCROLL_UP      (8)
//#define SCROLL_SCROLLBAR_MOVE (9)
//
//
#define TEXT_CORNER_REVISION  (1)
#define TEXT_CORNER_HELP      (2)
#define TEXT_CORNER_OPTIMUS   (3)
//
//#define TITLE_MAX_LEN (SVALUE_MAX_SIZE / sizeof(CHAR16) + 128)
//
//// other menu definitions
//
//#define MENU_FUNCTION_INIT            (0)
//#define MENU_FUNCTION_CLEANUP         (1)
//#define MENU_FUNCTION_PAINT_ALL       (2)
//#define MENU_FUNCTION_PAINT_SELECTION (3)
//#define MENU_FUNCTION_PAINT_TIMEOUT   (4)
//
//
//
//static CHAR16 ArrowUp[2]   = { ARROW_UP, 0 };
//static CHAR16 ArrowDown[2] = { ARROW_DOWN, 0 };
//
//BOOLEAN MainAnime = FALSE;
//
////TODO Scroll variables must be a part of REFIT_SCREEN
////BOOLEAN ScrollEnabled = FALSE;
//BOOLEAN IsDragging = FALSE;
//#if !USE_XTHEME
INTN ScrollWidth = 16;
//INTN ScrollButtonsHeight = 20;
//INTN ScrollBarDecorationsHeight = 5;
//INTN ScrollScrollDecorationsHeight = 7;
//#endif
//INTN ScrollbarYMovement;
//
//
////#define TextHeight (FONT_CELL_HEIGHT + TEXT_YMARGIN * 2)
//
//// clovy - set row height based on text size
//#define RowHeightFromTextHeight (1.35f)
//
////TODO spacing must be a part of layout in XTheme
//#define TITLEICON_SPACING (16)
////#define ROW0__TILESIZE (144)
////#define ROW1_TILESIZE (64)
//#define TILE1_XSPACING (8)
////#define TILE_YSPACING (24)
//#define ROW0_SCROLLSIZE (100)
//
////EG_IMAGE *SelectionImages[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
////EG_IMAGE *Buttons[4] = {NULL, NULL, NULL, NULL};
#if !USE_XTHEME
static EG_IMAGE *TextBuffer = NULL;
#endif
//
////EG_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff }; //non-trasparent
//
////INTN row0TileSize = 144;
////INTN row1TileSize = 64;
//
//static INTN row0Count, row0PosX, row0PosXRunning;
//static INTN row1Count, row1PosX, row1PosXRunning;
//static INTN *itemPosX = NULL;
//static INTN *itemPosY = NULL;
#if !USE_XTHEME
static INTN row0PosY /*, row1PosY, textPosY, FunctextPosY*/;
#endif
////static EG_IMAGE* MainImage;
//static INTN OldX = 0, OldY = 0;
//static INTN OldTextWidth = 0;
//static UINTN OldRow = 0;
//static INTN OldTimeoutTextWidth = 0;
//static INTN MenuWidth, TimeoutPosY;
//static INTN EntriesPosX, EntriesPosY;
//static INTN EntriesWidth, EntriesHeight, EntriesGap;
//#if !USE_XTHEME
EG_IMAGE* ScrollbarImage = NULL;
EG_IMAGE* ScrollbarBackgroundImage = NULL;
EG_IMAGE* UpButtonImage = NULL;
EG_IMAGE* DownButtonImage = NULL;
EG_IMAGE* BarStartImage = NULL;
EG_IMAGE* BarEndImage = NULL;
EG_IMAGE* ScrollStartImage = NULL;
EG_IMAGE* ScrollEndImage = NULL;
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
////BOOLEAN mGuiReady = FALSE;
//


//REFIT_MENU_ITEM_OPTIONS(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, ACTION AtClick_)
REFIT_MENU_ITEM_OPTIONS  MenuEntryOptions (L"Options"_XSW,          1, 0, 'O', ActionEnter);
REFIT_MENU_ITEM_ABOUT    MenuEntryAbout   (L"About Clover"_XSW,     1, 0, 'A', ActionEnter);
REFIT_MENU_ITEM_RESET    MenuEntryReset   (L"Restart Computer"_XSW, 1, 0, 'R', ActionSelect);
REFIT_MENU_ITEM_SHUTDOWN MenuEntryShutdown(L"Exit Clover"_XSW,      1, 0, 'U', ActionSelect);
REFIT_MENU_ITEM_RETURN   MenuEntryReturn  (L"Return"_XSW,           0, 0,  0,  ActionEnter);


#if USE_XTHEME
REFIT_MENU_SCREEN MainMenu(1, L"Main Menu"_XSW, L"Automatic boot"_XSW);
REFIT_MENU_SCREEN AboutMenu(2, L"About"_XSW, L""_XSW);
REFIT_MENU_SCREEN HelpMenu(3, L"Help"_XSW, L""_XSW);
#else
REFIT_MENU_SCREEN MainMenu(1, L"Main Menu", L"Automatic boot");
REFIT_MENU_SCREEN AboutMenu(2, L"About", NULL);
REFIT_MENU_SCREEN HelpMenu(3, L"Help", NULL);
#endif


// input - tsc
// output - milliseconds
// the caller is responsible for t1 > t0
UINT64 TimeDiff(UINT64 t0, UINT64 t1)
{
  return DivU64x64Remainder((t1 - t0), DivU64x32(gCPUStructure.TSCFrequency, 1000), 0);
}


VOID FillInputs(BOOLEAN New)
{
  UINTN i,j; //for loops
  CHAR8 tmp[41];
//  BOOLEAN bit;

  tmp[40] = 0;  //make it null-terminated

  UINTN InputItemsCount = 0;
  if (New) {
    InputItems = (__typeof__(InputItems))AllocateZeroPool(130 * sizeof(INPUT_ITEM)); //XXX
  }

  InputItems[InputItemsCount].ItemType = ASString;  //0
  //even though Ascii we will keep value as Unicode to convert later
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(SVALUE_MAX_SIZE);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, SVALUE_MAX_SIZE, "%s ", gSettings.BootArgs);
  InputItems[InputItemsCount].ItemType = UNIString; //1
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(32);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 32, "%ls", gSettings.DsdtName); // 1-> 2
  InputItems[InputItemsCount].ItemType = UNIString; //2
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(63);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 63, "%ls", gSettings.BlockKexts);

  InputItems[InputItemsCount].ItemType = RadioSwitch;  //3 - Themes chooser
  InputItems[InputItemsCount++].IValue = 3;

  InputItems[InputItemsCount].ItemType = BoolValue; //4
  InputItems[InputItemsCount++].BValue = gSettings.DropSSDT;
  InputItems[InputItemsCount].ItemType = BoolValue;  //5
  InputItems[InputItemsCount++].BValue = gSettings.GeneratePStates;
  InputItems[InputItemsCount].ItemType = BoolValue;  //6
  InputItems[InputItemsCount++].BValue = gSettings.SlpSmiEnable;
  InputItems[InputItemsCount].ItemType = Decimal;  //7
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(8);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 8, "%02d", gSettings.PLimitDict);
  InputItems[InputItemsCount].ItemType = Decimal;  //8
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(8);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 8, "%02d", gSettings.UnderVoltStep);
  InputItems[InputItemsCount].ItemType = BoolValue; //9
  InputItems[InputItemsCount++].BValue = gSettings.GenerateCStates;
  InputItems[InputItemsCount].ItemType = BoolValue; //10
  InputItems[InputItemsCount++].BValue = gSettings.EnableC2;
  InputItems[InputItemsCount].ItemType = BoolValue; //11
  InputItems[InputItemsCount++].BValue = gSettings.EnableC4;
  InputItems[InputItemsCount].ItemType = BoolValue; //12
  InputItems[InputItemsCount++].BValue = gSettings.EnableC6;
  InputItems[InputItemsCount].ItemType = BoolValue; //13
  InputItems[InputItemsCount++].BValue = gSettings.EnableISS;
  InputItems[InputItemsCount].ItemType = Decimal;  //14
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "%06d", gSettings.QPI);
  InputItems[InputItemsCount].ItemType = BoolValue; //15
  InputItems[InputItemsCount++].BValue = gSettings.PatchNMI;
  InputItems[InputItemsCount].ItemType = BoolValue; //16
  InputItems[InputItemsCount++].BValue = gSettings.PatchVBios;
  InputItems[InputItemsCount].ItemType = Decimal;  //17
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(20);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 16, "0x%llX", gPlatformFeature);
  InputItems[InputItemsCount].ItemType = Hex;  //18
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(36);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 36, "0x%X", gSettings.BacklightLevel);
  InputItems[InputItemsCount].ItemType = Decimal;  //19
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  if (gSettings.BusSpeed > 20000) {
    snwprintf(InputItems[InputItemsCount++].SValue, 16, "%06d", gSettings.BusSpeed);
  } else {
	  snwprintf(InputItems[InputItemsCount++].SValue, 16, "%06llu", gCPUStructure.ExternalClock);
  }
  InputItemsCount = 20;
  for (i=0; i<NGFX; i++) {
    InputItems[InputItemsCount].ItemType = ASString;  //20+i*6
    if (New) {
      InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
    }
	  snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gGraphics[i].Model);

    if (gGraphics[i].Vendor == Ati) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.InjectATI;
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(20);
      }
      if (StrLen(gSettings.FBName) > 2) { //fool proof: cfg_name is 3 character or more.
		  snwprintf(InputItems[InputItemsCount++].SValue, 20, "%ls", gSettings.FBName);
      } else {
		  snwprintf(InputItems[InputItemsCount++].SValue, 20, "%s", gGraphics[i].Config);
      }
    } else if (gGraphics[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.InjectNVidia;
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      for (j=0; j<8; j++) {
        snprintf((CHAR8*)&tmp[2*j], 3, "%02X", gSettings.Dcfg[j]);
      }
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(40);
      }
		snwprintf(InputItems[InputItemsCount++].SValue, 40, "%s", tmp);

      //InputItems[InputItemsCount++].SValue = PoolPrint(L"%08x",*(UINT64*)&gSettings.Dcfg[0]);
    } else /*if (gGraphics[i].Vendor == Intel) */ {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.InjectIntel;
      InputItems[InputItemsCount].ItemType = Hex; //22+6i
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(20);
      }
      snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.IgPlatform);
 //     InputItemsCount += 3;
 //     continue;
    }

    InputItems[InputItemsCount].ItemType = Decimal;  //23+6i
    if (New) {
      InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(8);
    }
    if (gSettings.VideoPorts > 0) {
      snwprintf(InputItems[InputItemsCount++].SValue, 8, "%02d", gSettings.VideoPorts);
    } else {
      snwprintf(InputItems[InputItemsCount++].SValue, 8, "%02d", gGraphics[i].Ports);
    }

    if (gGraphics[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = ASString; //24+6i
      for (j=0; j<20; j++) {
        snprintf((CHAR8*)&tmp[2*j], 3, "%02X", gSettings.NVCAP[j]);
      }
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(84);
      }
		snwprintf(InputItems[InputItemsCount++].SValue, 84, "%s", tmp);
    } else { //ATI and others there will be connectors
      InputItems[InputItemsCount].ItemType = Hex; //24+6i
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(20);
      }
		snwprintf(InputItems[InputItemsCount++].SValue, 20, "%08x", gGraphics[i].Connectors);
    }

    InputItems[InputItemsCount].ItemType = BoolValue; //25+6i
    InputItems[InputItemsCount++].BValue = gGraphics[i].LoadVBios;
  }
  //and so on

  InputItemsCount = 44;
  InputItems[InputItemsCount].ItemType = BoolValue; //44
  InputItems[InputItemsCount++].BValue = gSettings.KextPatchesAllowed;
  InputItems[InputItemsCount].ItemType = BoolValue; //45
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPKernelCpu;
  InputItems[InputItemsCount].ItemType = BoolValue; //46
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPAppleIntelCPUPM;
  InputItems[InputItemsCount].ItemType = BoolValue; //47
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPAppleRTC;
  InputItems[InputItemsCount].ItemType = BoolValue; //48
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPKernelPm;
  InputItems[InputItemsCount].ItemType = BoolValue; //49
  InputItems[InputItemsCount++].BValue = gSettings.FixMCFG;

  InputItems[InputItemsCount].ItemType = Decimal;  //50
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "%06d", gSettings.RefCLK);

  InputItems[InputItemsCount].ItemType = ASString;  //51 OS version if non-detected
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(SVALUE_MAX_SIZE);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, SVALUE_MAX_SIZE, "%s ", NonDetected);

  InputItems[InputItemsCount].ItemType = BoolValue; //52
  InputItems[InputItemsCount++].BValue = gSettings.InjectEDID;

  //VendorEDID & ProductEDID 53, 54
  InputItems[InputItemsCount].ItemType = Decimal;  //53
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "0x%04X", gSettings.VendorEDID);
  InputItems[InputItemsCount].ItemType = Decimal;  //54
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "0x%04X", gSettings.ProductEDID);

  // ErmaC: NvidiaGeneric menu selector y/n
  InputItems[InputItemsCount].ItemType = BoolValue; //55
  InputItems[InputItemsCount++].BValue = gSettings.NvidiaGeneric;
  InputItems[InputItemsCount].ItemType = BoolValue; //56
  InputItems[InputItemsCount++].BValue = gSettings.NvidiaWeb;
  InputItems[InputItemsCount].ItemType = BoolValue; //57
  InputItems[InputItemsCount++].BValue = gSettings.ResetHDA;
  InputItems[InputItemsCount].ItemType = BoolValue; //58
  InputItems[InputItemsCount++].BValue = gSettings.AFGLowPowerState;
  InputItems[InputItemsCount].ItemType = BoolValue; //59
  InputItems[InputItemsCount++].BValue = gSettings.HDAInjection;
  InputItems[InputItemsCount].ItemType = Decimal;  // 60
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 64, "%d", gSettings.HDALayoutId);

  // syscl change here
  InputItems[InputItemsCount].ItemType = BoolValue; //61
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPDELLSMBIOS;
  // end of change

  InputItems[InputItemsCount].ItemType = Hex;  //62
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(24);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 24, "0x%08X", gFwFeatures);

  InputItems[InputItemsCount].ItemType = Hex;  //63
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(24);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 24, "0x%08X", gFwFeaturesMask);

  // Debug for KernelAndKextPatches
  InputItems[InputItemsCount].ItemType = BoolValue; //64
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPDebug;


  // CSR - aka System Integrity Protection configuration
  InputItems[InputItemsCount].ItemType = CheckBit; //65
  InputItems[InputItemsCount++].IValue = gSettings.BooterConfig;
  InputItems[InputItemsCount].ItemType = CheckBit; //66
  InputItems[InputItemsCount++].IValue = gSettings.CsrActiveConfig;


  InputItems[InputItemsCount].ItemType = CheckBit; //67
  InputItems[InputItemsCount++].IValue = gSettings.FixDsdt;
  InputItems[InputItemsCount].ItemType = CheckBit; //68
  InputItems[InputItemsCount++].IValue = gSettings.OptionsBits;
  InputItems[InputItemsCount].ItemType = CheckBit; //69
  InputItems[InputItemsCount++].IValue = gSettings.FlagsBits;

  InputItems[InputItemsCount].ItemType = Decimal;  //70
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(8);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 8, "%02lld", gSettings.PointerSpeed);
  InputItems[InputItemsCount].ItemType = Decimal;  //71
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 16, "%04llu", gSettings.DoubleClickTime);
  InputItems[InputItemsCount].ItemType = BoolValue; //72
  InputItems[InputItemsCount++].BValue = gSettings.PointerMirror;

  //reserve for mouse and continue

  InputItemsCount = 74;
  InputItems[InputItemsCount].ItemType = BoolValue; //74
  InputItems[InputItemsCount++].BValue = gSettings.USBFixOwnership;

  InputItems[InputItemsCount].ItemType = Hex;  //75
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "0x%04X", gSettings.C3Latency);
  InputItems[InputItemsCount].ItemType = Decimal;  //76
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "%02d", gSettings.EnabledCores);
  InputItems[InputItemsCount].ItemType = Decimal;  //77
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "%02d", gSettings.SavingMode);

  InputItems[InputItemsCount].ItemType = ASString;  //78
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.ProductName);
  InputItems[InputItemsCount].ItemType = ASString;  //79
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.VersionNr);
  InputItems[InputItemsCount].ItemType = ASString;  //80
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.SerialNr);
  InputItems[InputItemsCount].ItemType = ASString;  //81
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.BoardNumber);
  InputItems[InputItemsCount].ItemType = ASString;  //82
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.BoardSerialNumber);
  InputItems[InputItemsCount].ItemType = Decimal;  //83
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 64, "%d", gSettings.BoardType);
  InputItems[InputItemsCount].ItemType = ASString;  //84
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.BoardVersion);
  InputItems[InputItemsCount].ItemType = Decimal;  //85
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 64, "%d", gSettings.ChassisType);
  InputItems[InputItemsCount].ItemType = ASString;  //86
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.RomVersion);
  InputItems[InputItemsCount].ItemType = ASString;  //87
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.ReleaseDate);

  InputItems[InputItemsCount].ItemType = BoolValue; //88
  InputItems[InputItemsCount++].BValue = gSettings.DoubleFirstState;
  InputItems[InputItemsCount].ItemType = BoolValue; //89
  InputItems[InputItemsCount++].BValue = gSettings.EnableC7;
  InputItems[InputItemsCount].ItemType = RadioSwitch; //90
  InputItems[InputItemsCount++].IValue = 90;

  InputItems[InputItemsCount].ItemType = BoolValue; //91
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPKernelLapic;

  InputItems[InputItemsCount].ItemType = BoolValue; //92
  InputItems[InputItemsCount++].BValue = gSettings.USBInjection;
  InputItems[InputItemsCount].ItemType = BoolValue; //93
  InputItems[InputItemsCount++].BValue = gSettings.InjectClockID;

  InputItems[InputItemsCount].ItemType = Hex;  //94
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.FakeATI);
  InputItems[InputItemsCount].ItemType = Hex;  //95
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.FakeNVidia);
  InputItems[InputItemsCount].ItemType = Hex;  //96
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.FakeIntel);

  InputItems[InputItemsCount].ItemType = Hex;  //97
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.FakeLAN);
  InputItems[InputItemsCount].ItemType = Hex;  //98
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.FakeWIFI);
  InputItems[InputItemsCount].ItemType = Hex;  //99
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.FakeSATA);
  InputItems[InputItemsCount].ItemType = Hex;  //100
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.FakeXHCI);
  InputItems[InputItemsCount].ItemType = CheckBit;  //101
  InputItems[InputItemsCount++].IValue = dropDSM;

  InputItems[InputItemsCount].ItemType = BoolValue; //102
  InputItems[InputItemsCount++].BValue = gSettings.DebugDSDT;
  InputItems[InputItemsCount].ItemType = Hex;  //103
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.FakeIMEI);
  InputItems[InputItemsCount].ItemType = Hex;  //104
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 26, "0x%08X", gSettings.KernelAndKextPatches.FakeCPUID);


  InputItems[InputItemsCount].ItemType = BoolValue; //105
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPKernelXCPM;

  InputItems[InputItemsCount].ItemType = BoolValue; //106
  InputItems[InputItemsCount++].BValue = gSettings.StringInjector;
  InputItems[InputItemsCount].ItemType = BoolValue; //107
  InputItems[InputItemsCount++].BValue = gSettings.NoDefaultProperties;
  InputItems[InputItemsCount].ItemType = BoolValue; //108
  InputItems[InputItemsCount++].BValue = gSettings.KernelPatchesAllowed;

  InputItems[InputItemsCount].ItemType = Hex; //109
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "%01X", gSettings.DualLink);

  InputItems[InputItemsCount].ItemType = BoolValue; //110
  InputItems[InputItemsCount++].BValue = gSettings.NvidiaNoEFI;
  InputItems[InputItemsCount].ItemType = BoolValue; //111
  InputItems[InputItemsCount++].BValue = gSettings.NvidiaSingle;

  InputItems[InputItemsCount].ItemType = Hex;  //112
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "0x%04X", gSettings.IntelMaxValue);

  InputItems[InputItemsCount].ItemType = BoolValue; //113
  InputItems[InputItemsCount++].BValue = gSettings.AutoMerge;
  InputItems[InputItemsCount].ItemType = BoolValue; //114
  InputItems[InputItemsCount++].BValue = gSettings.DeInit;
  InputItems[InputItemsCount].ItemType = BoolValue; //115
  InputItems[InputItemsCount++].BValue = gSettings.NoCaches;
  InputItems[InputItemsCount].ItemType = RadioSwitch;  //116 - DSDT chooser
  InputItems[InputItemsCount++].IValue = 116;

  InputItems[InputItemsCount].ItemType = ASString;  //117
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.EfiVersion);
  InputItems[InputItemsCount].ItemType = ASString;  //118
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
	snwprintf(InputItems[InputItemsCount++].SValue, 64, "%s", gSettings.BooterCfgStr);

  InputItems[InputItemsCount].ItemType = RadioSwitch;  //119 - Audio chooser
  InputItems[InputItemsCount++].IValue = 119;
  InputItems[InputItemsCount].ItemType = Decimal;  //120
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  snwprintf(InputItems[InputItemsCount++].SValue, 16, "%04d", DefaultAudioVolume);
  
  InputItems[InputItemsCount].ItemType = BoolValue; //121
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPPanicNoKextDump;




  //menu for drop table
  if (gSettings.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
    while (DropTable) {
      DropTable->MenuItem.ItemType = BoolValue;
      DropTable = DropTable->Next;
    }
  }

  if (ACPIPatchedAML) {
    ACPI_PATCHED_AML *ACPIPatchedAMLTmp = ACPIPatchedAML;
    while (ACPIPatchedAMLTmp) {
      ACPIPatchedAMLTmp->MenuItem.ItemType = BoolValue;
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
//  UINT32 k;
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

	  snprintf(gSettings.BootArgs, 255, "%ls ", InputItems[i].SValue);
  }
  i++; //1
  if (InputItems[i].Valid) {
	  snwprintf(gSettings.DsdtName, sizeof(gSettings.DsdtName), "%ls", InputItems[i].SValue);
  }
  i++; //2
  if (InputItems[i].Valid) {
	  snwprintf(gSettings.BlockKexts, sizeof(gSettings.BlockKexts), "%ls", InputItems[i].SValue);
  }
  i++; //3
  if (InputItems[i].Valid) {
#if USE_XTHEME
    if (OldChosenTheme == 0xFFFF) {
      ThemeX.Theme.takeValueFrom("embedded");
    } else {
      ThemeX.Theme.takeValueFrom(ThemesList[OldChosenTheme]);
      ThemeX.DarkEmbedded = FALSE;
      ThemeX.Font = FONT_ALFA;
    }
#else
    if (GlobalConfig.Theme) {
      FreePool(GlobalConfig.Theme);
    }
    if (OldChosenTheme == 0xFFFF) {
      GlobalConfig.Theme = PoolPrint(L"embedded");
    } else {
      GlobalConfig.Theme = PoolPrint(L"%s", ThemesList[OldChosenTheme]);
      GlobalConfig.DarkEmbedded = FALSE;
      GlobalConfig.Font = FONT_ALFA;
    }
#endif

    //will change theme after ESC
    gThemeChanged = TRUE;
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
//    DBG("InputItems[i]: %ls\n", InputItems[i].SValue);
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
    DBG("applied QPI=%d\n", gSettings.QPI);
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
    gPlatformFeature = (UINT64)StrHexToUint64(InputItems[i].SValue);
	  DBG("applied PlatformFeature=0x%llX\n", gPlatformFeature);
  }
  i++; //18 | Download-Fritz: There is no GUI element for BacklightLevel; please revise
  if (InputItems[i].Valid) {
    gSettings.BacklightLevel = (UINT16)StrHexToUint64(InputItems[i].SValue);
    gSettings.BacklightLevelConfig = TRUE;
  }
  i++; //19
  if (InputItems[i].Valid) {
    gSettings.BusSpeed = (UINT32)StrDecimalToUintn(InputItems[i].SValue);
    DBG("applied BusSpeed=%d\n", gSettings.BusSpeed);
  }

  i = 19;
  for (j = 0; j < NGFX; j++) {
    i++; //20
    if (InputItems[i].Valid) {
      snprintf(gGraphics[j].Model, 64, "%ls",  InputItems[i].SValue);
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
		  snwprintf(gSettings.FBName, 32, "%ls", InputItems[i].SValue);
      } else if (gGraphics[j].Vendor == Nvidia) {
        ZeroMem(AString, 256);
        snprintf(AString, 255, "%ls", InputItems[i].SValue);
        hex2bin(AString, (UINT8*)&gSettings.Dcfg[0], 8);
      } else if (gGraphics[j].Vendor == Intel) {
        //ig-platform-id for Ivy+ and snb-platform-id for Sandy
        gSettings.IgPlatform = (UINT32)StrHexToUint64(InputItems[i].SValue);
        DBG("applied *-platform-id=0x%X\n", gSettings.IgPlatform);
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
      if (gGraphics[j].Vendor == Nvidia) {
        ZeroMem(AString, 256);
        if (StrLen(InputItems[i].SValue) > 0) {
          snprintf(AString, 255, "%ls", InputItems[i].SValue);
          hex2bin(AString, (UINT8*)&gSettings.NVCAP[0], 20);
        }
      } else {
        gGraphics[j].Connectors = (UINT32)StrHexToUint64(InputItems[i].SValue);
        gGraphics[j].ConnChanged = TRUE;
      }
    }
    i++; //25
    if (InputItems[i].Valid) {
      gGraphics[j].LoadVBios = InputItems[i].BValue;
    }
  }  //end of Graphics Cards
  // next number == 42

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
    gSettings.KernelAndKextPatches.KPAppleIntelCPUPM = InputItems[i].BValue;
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
    gSettings.FixMCFG = InputItems[i].BValue;
  }

  i++; //50
  if (InputItems[i].Valid) {
    gSettings.RefCLK = (UINT32)StrDecimalToUintn(InputItems[i].SValue);
  }

  i++; //51
  if (InputItems[i].Valid) {
	  snprintf(NonDetected, 64, "%ls", InputItems[i].SValue);
  }

  i++; //52
  if (InputItems[i].Valid) {
    gSettings.InjectEDID = InputItems[i].BValue;
  }
  i++; //53
  if (InputItems[i].Valid) {
    gSettings.VendorEDID = (UINT16)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //54
  if (InputItems[i].Valid) {
    gSettings.ProductEDID = (UINT16)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //55
  // ErmaC: NvidiaGeneric bool(Y/N)
  if (InputItems[i].Valid) {
    gSettings.NvidiaGeneric = InputItems[i].BValue;
  }
  i++; //56
  if (InputItems[i].Valid) {
    gSettings.NvidiaWeb = InputItems[i].BValue;
  }
  i++; //57
  if (InputItems[i].Valid) {
    gSettings.ResetHDA = InputItems[i].BValue;
  }
  i++; //58
  if (InputItems[i].Valid) {
    gSettings.AFGLowPowerState = InputItems[i].BValue;
  }
  i++; //59
  if (InputItems[i].Valid) {
    gSettings.HDAInjection = InputItems[i].BValue;
  }
  i++; //60
  if (InputItems[i].Valid) {
    gSettings.HDALayoutId = (UINT32)(StrDecimalToUintn(InputItems[i].SValue));
  }
  i++; //61
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPDELLSMBIOS = InputItems[i].BValue;
    // yes, we do need to change gRemapSmBiosIsRequire here as well
    gRemapSmBiosIsRequire = InputItems[i].BValue;
    gBootChanged = TRUE;
  }
  i++; //62
  if (InputItems[i].Valid) {
    gFwFeatures = (UINT32)StrHexToUint64(InputItems[i].SValue);
    DBG("applied FirmwareFeatures=0x%X\n", gFwFeatures);
  }
  i++; //63
  if (InputItems[i].Valid) {
    gFwFeaturesMask = (UINT32)StrHexToUint64(InputItems[i].SValue);
    DBG("applied FirmwareFeaturesMask=0x%X\n", gFwFeaturesMask);
  }
  i++; //64
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPDebug = InputItems[i].BValue;
 //   gBootChanged = TRUE;
  }

  // CSR
  i = 65;
  if (InputItems[i].Valid) {
    gSettings.BooterConfig = InputItems[i].IValue & 0x7F;
  }
  i++; //66
  if (InputItems[i].Valid) {
    gSettings.CsrActiveConfig = InputItems[i].IValue;
  }

  i++; //67
  if (InputItems[i].Valid) {
    gSettings.FixDsdt = InputItems[i].IValue;
  }
  i++; //68
  if (InputItems[i].Valid) {
    gSettings.OptionsBits = InputItems[i].IValue;
  }
  i++; //69
  if (InputItems[i].Valid) {
    gSettings.FlagsBits = InputItems[i].IValue;
  }


  i++; //70
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
  i++; //71
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
	  snprintf(gSettings.ProductName, 64, "%ls", InputItems[i].SValue);
    // let's fill all other fields based on this ProductName
    // to serve as default
    Model = GetModelFromString(gSettings.ProductName);
    if (Model != MaxMachineType) {
      SetDMISettingsForModel(Model, FALSE);
    }
  }

  i++; //79
  if (InputItems[i].Valid) {
	  snprintf(gSettings.VersionNr, 64, "%ls", InputItems[i].SValue);
  }
  i++; //80
  if (InputItems[i].Valid) {
	  snprintf(gSettings.SerialNr, 64, "%ls", InputItems[i].SValue);
  }
  i++; //81
  if (InputItems[i].Valid) {
	  snprintf(gSettings.BoardNumber, 64, "%ls", InputItems[i].SValue);
  }
  i++; //82
  if (InputItems[i].Valid) {
	  snprintf(gSettings.BoardSerialNumber, 64, "%ls", InputItems[i].SValue);
  }
  i++; //83
  if (InputItems[i].Valid) {
    gSettings.BoardType = (UINT8)(StrDecimalToUintn(InputItems[i].SValue) & 0x0F);
  }
  i++; //84
  if (InputItems[i].Valid) {
	  snprintf(gSettings.BoardVersion, 64, "%ls", InputItems[i].SValue);
  }
  i++; //85
  if (InputItems[i].Valid) {
    gSettings.ChassisType = (UINT8)(StrDecimalToUintn(InputItems[i].SValue) & 0x0F);
  }
  i++; //86
  if (InputItems[i].Valid) {
	  snprintf(gSettings.RomVersion, 64, "%ls", InputItems[i].SValue);
  }
  i++; //87
  if (InputItems[i].Valid) {
	  snprintf(gSettings.ReleaseDate, 64, "%ls", InputItems[i].SValue);
  }

  i++; //88
  if (InputItems[i].Valid) {
    gSettings.DoubleFirstState = InputItems[i].BValue;
  }
  i++; //89
  if (InputItems[i].Valid) {
    gSettings.EnableC7 = InputItems[i].BValue;
  }

  i++; //90
  if (InputItems[i].Valid) {
    Status = LoadUserSettings(SelfRootDir, ConfigsList[OldChosenConfig], &dict);
    if (!EFI_ERROR(Status)) {
      Status = GetUserSettings(SelfRootDir, dict);
      if (gConfigDict[2]) FreeTag(gConfigDict[2]);
      gConfigDict[2] = dict;
		snwprintf(gSettings.ConfigName, 64, "%ls", ConfigsList[OldChosenConfig]);
      gBootChanged = TRUE;
      gThemeChanged = TRUE;
    }
    FillInputs(FALSE);
    NeedSave = FALSE;
  }
  i++; //91
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPKernelLapic = InputItems[i].BValue;
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
    DBG("applied FakeIntel=0x%X\n", gSettings.FakeIntel);
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
//    gSettings.DropOEM_DSM = (UINT16)StrHexToUint64(InputItems[i].SValue);
    gSettings.DropOEM_DSM = (UINT16)InputItems[i].IValue;
    dropDSM = gSettings.DropOEM_DSM; //?
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

  i++; //104
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.FakeCPUID = (UINT32)StrHexToUint64(InputItems[i].SValue);
    DBG("applied FakeCPUID=%06X\n", gSettings.KernelAndKextPatches.FakeCPUID);
    gBootChanged = TRUE;
  }

  i++; //105
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPKernelXCPM = InputItems[i].BValue;
    DBG("applied KernelXCPM\n");
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

  i++; //109
  if (InputItems[i].Valid) {
    gSettings.DualLink = (UINT32)StrHexToUint64(InputItems[i].SValue);
    DBG("applied DualLink=%X\n", gSettings.DualLink);
  }

  i++; //110
  if (InputItems[i].Valid) {
    gSettings.NvidiaNoEFI = InputItems[i].BValue;
  }

  i++; //111
  if (InputItems[i].Valid) {
    gSettings.NvidiaSingle = InputItems[i].BValue;
  }
  i++; //112
  if (InputItems[i].Valid) {
    gSettings.IntelMaxValue = (UINT16)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //113
  if (InputItems[i].Valid) {
    gSettings.AutoMerge = InputItems[i].BValue;
  }
  i++; //114
  if (InputItems[i].Valid) {
    gSettings.DeInit = InputItems[i].BValue;
  }
  i++; //115
  if (InputItems[i].Valid) {
    gSettings.NoCaches = InputItems[i].BValue;
  }
  i++; //116
  if (InputItems[i].Valid) {
    if (OldChosenDsdt == 0xFFFF) {
      snwprintf(gSettings.DsdtName, 64, "BIOS.aml");
    } else {
		snwprintf(gSettings.DsdtName, 64, "%ls", DsdtsList[OldChosenDsdt]);
    }
  }
  i++; //117
  if (InputItems[i].Valid) {
	  snprintf(gSettings.EfiVersion, 64, "%ls", InputItems[i].SValue);
  }
  i++; //118
  if (InputItems[i].Valid) {
	  snprintf(gSettings.BooterCfgStr, 64, "%ls", InputItems[i].SValue);
  }
  i++; //119
  if (InputItems[i].Valid) {
    EFI_DEVICE_PATH_PROTOCOL*  DevicePath = NULL;
    UINT8 TmpIndex = OldChosenAudio & 0xFF;
	  DBG("Chosen output %llu:%ls_%s\n", OldChosenAudio, AudioList[OldChosenAudio].Name, AudioOutputNames[OldChosenAudio]);

    DevicePath = DevicePathFromHandle(AudioList[OldChosenAudio].Handle);
    if (DevicePath != NULL) {
      SetNvramVariable(L"Clover.SoundDevice", &gEfiAppleBootGuid,
                       EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                       GetDevicePathSize(DevicePath), (UINT8 *)DevicePath);
      SetNvramVariable(L"Clover.SoundIndex", &gEfiAppleBootGuid,
                       EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                       1, (UINT8 *)&TmpIndex);

    }
  }
  i++; //120
  if (InputItems[i].Valid) {
    DefaultAudioVolume = (UINT8)StrDecimalToUintn(InputItems[i].SValue);
    if (DefaultAudioVolume > 100) {
        // correct wrong input
        DefaultAudioVolume = 90;
        snwprintf(InputItems[i].SValue, 16, "%04d", DefaultAudioVolume);
    }
    SetNvramVariable(L"Clover.SoundVolume", &gEfiAppleBootGuid,
                     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                     1, &DefaultAudioVolume);
  }
  i++; //121
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPPanicNoKextDump = InputItems[i].BValue;
    gBootChanged = TRUE;
  }

  if (NeedSave) {
    SaveSettings();
  }
}


VOID AboutRefit(VOID)
{
#if USE_XTHEME
  if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    AboutMenu.TitleImage = ThemeX.GetIcon((INTN)BUILTIN_ICON_FUNC_ABOUT);
  } else {
    AboutMenu.TitleImage.setEmpty();
  }
#else
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    AboutMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_ABOUT);
  } else {
    AboutMenu.TitleImage = NULL;
  }
#endif

  if (AboutMenu.Entries.size() == 0) {
//    AboutMenu.AddMenuInfo_f(("Clover Version 5.0"));
#ifdef REVISION_STR
	  AboutMenu.AddMenuInfo_f(" %s ", REVISION_STR);
#else
    AboutMenu.AddMenuInfo_f((L"Clover Revision %s", gFirmwareRevision));
#endif
#ifdef FIRMWARE_BUILDDATE
    AboutMenu.AddMenuInfo_f(" Build: %s", FIRMWARE_BUILDDATE);
#else
    AboutMenu.AddMenuInfo_f(" Build: unknown");
#endif
    AboutMenu.AddMenuInfo_f("");
    AboutMenu.AddMenuInfo_f("Based on rEFIt (c) 2006-2010 Christoph Pfisterer");
    AboutMenu.AddMenuInfo_f("Portions Copyright (c) Intel Corporation");
    AboutMenu.AddMenuInfo_f("Developers:");
    AboutMenu.AddMenuInfo_f("  Slice, dmazar, apianti, JrCs, pene, usrsse2");
    AboutMenu.AddMenuInfo_f("  Kabyl, pcj, jadran, Blackosx, STLVNUB, ycr.ru");
    AboutMenu.AddMenuInfo_f("  FrodoKenny, skoczi, crazybirdy, Oscar09, xsmile");
    AboutMenu.AddMenuInfo_f("  cparm, rehabman, nms42, Sherlocks, Zenith432");
    AboutMenu.AddMenuInfo_f("  stinga11, TheRacerMaster, solstice, SoThOr, DF");
    AboutMenu.AddMenuInfo_f("  cecekpawon, Micky1979, Needy, joevt, ErmaC, vit9696");
    AboutMenu.AddMenuInfo_f("  ath, savvas, syscl, goodwin_c, clovy, jief_machak");
    AboutMenu.AddMenuInfo_f("Credits also:");
    AboutMenu.AddMenuInfo_f("  projectosx.com, applelife.ru, insanelymac.com");
    AboutMenu.AddMenuInfo_f("");
    AboutMenu.AddMenuInfo_f("Running on:");
    AboutMenu.AddMenuInfo_f(" EFI Revision %d.%02d",
                                      gST->Hdr.Revision >> 16, gST->Hdr.Revision & ((1 << 16) - 1));
#if defined(MDE_CPU_IA32)
    AboutMenu.AddMenuInfo_f(" Platform: i386 (32 bit)");
#elif defined(MDE_CPU_X64)
    AboutMenu.AddMenuInfo_f(" Platform: x86_64 (64 bit)");
#elif defined(_MSC_VER)
    AboutMenu.AddMenuInfo_f(" Platform: x86_64 (64 bit) VS");
#else
    AboutMenu.AddMenuInfo_f(" Platform: unknown");
#endif
	  AboutMenu.AddMenuInfo_f(" Firmware: %ls rev %d.%04d", gST->FirmwareVendor, gST->FirmwareRevision >> 16, gST->FirmwareRevision & ((1 << 16) - 1));
	  AboutMenu.AddMenuInfo_f(" Screen Output: %ls", egScreenDescription());
    AboutMenu.AnimeRun = AboutMenu.GetAnime();
    AboutMenu.AddMenuEntry(&MenuEntryReturn, false);
  } else if (AboutMenu.Entries.size() >= 2) {
    /*
      EntryCount instead of InfoLineCount. Lastline == return/back. Is necessary recheck screen res here?
    */
 //   FreePool(AboutMenu.Entries[AboutMenu.Entries.size()-2].Title); //what is FreePool(XStringW)?

    AboutMenu.Entries[AboutMenu.Entries.size()-2].Title.SWPrintf(" Screen Output: %ls", egScreenDescription());
  }

  AboutMenu.RunMenu(NULL);
}

VOID HelpRefit(VOID)
{
#if USE_XTHEME
  if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    HelpMenu.TitleImage = ThemeX.GetIcon(BUILTIN_ICON_FUNC_HELP);
  } else {
    HelpMenu.TitleImage.setEmpty();
  }
#else
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    HelpMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_HELP);
  } else {
    HelpMenu.TitleImage = NULL;
  }
#endif
  if (HelpMenu.Entries.size() == 0) {
    switch (gLanguage)
    {
      case russian:
        HelpMenu.AddMenuInfo_f("ESC - Выход из подменю, обновление главного меню");
        HelpMenu.AddMenuInfo_f("F1  - Помощь по горячим клавишам");
        HelpMenu.AddMenuInfo_f("F2  - Сохранить отчет в preboot.log (только если FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Показать скрытые значки в меню");
        HelpMenu.AddMenuInfo_f("F4  - Родной DSDT сохранить в EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Патченный DSDT сохранить в EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Сохранить ВидеоБиос в EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Проверить звук на выбранном выходе");
        HelpMenu.AddMenuInfo_f("F8  - Сделать дамп звуковых устройств в EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Изменить разрешение экрана на одно из возможных");
        HelpMenu.AddMenuInfo_f("F10 - Снимок экрана в папку EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Извлечь указанный DVD");
        HelpMenu.AddMenuInfo_f("Пробел - Дополнительное меню запуска выбранного тома");
        HelpMenu.AddMenuInfo_f("Цифры 1-9 - Быстрый запуск тома по порядку в меню");
        HelpMenu.AddMenuInfo_f("A (About) - О загрузчике");
        HelpMenu.AddMenuInfo_f("O (Options) - Дополнительные настройки");
        HelpMenu.AddMenuInfo_f("R (Reset) - Теплый перезапуск");
        HelpMenu.AddMenuInfo_f("U (go oUt) - Завершить работу в Кловере");
        HelpMenu.AddMenuInfo_f("S (Shell) - Переход в режим командной строки");
        break;
      case ukrainian:
        HelpMenu.AddMenuInfo_f("ESC - Вийти з меню, оновити головне меню");
        HelpMenu.AddMenuInfo_f("F1  - Ця довідка");
        HelpMenu.AddMenuInfo_f("F2  - Зберегти preboot.log (тiльки FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Відображати приховані розділи");
        HelpMenu.AddMenuInfo_f("F4  - Зберегти OEM DSDT в EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Зберегти патчений DSDT в EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F6  - Зберегти VideoBios в EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F10 - Зберегти знімок екрану в EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Відкрити обраний диск (DVD)");
        HelpMenu.AddMenuInfo_f("Пробіл - докладніше про обраний пункт меню");
        HelpMenu.AddMenuInfo_f("Клавіші 1-9 -  клавіші пунктів меню");
        HelpMenu.AddMenuInfo_f("A - Про систему");
        HelpMenu.AddMenuInfo_f("O - Опції меню");
        HelpMenu.AddMenuInfo_f("R - Перезавантаження");
        HelpMenu.AddMenuInfo_f("U - Відключити ПК");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case spanish:
        HelpMenu.AddMenuInfo_f("ESC - Salir de submenu o actualizar el menu principal");
        HelpMenu.AddMenuInfo_f("F1  - Esta Ayuda");
        HelpMenu.AddMenuInfo_f("F2  - Guardar preboot.log (Solo FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Guardar DSDT oem en EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Guardar DSDT parcheado en EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Guardar VideoBios en EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Guardar Captura de pantalla en EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Expulsar volumen seleccionado (DVD)");
        HelpMenu.AddMenuInfo_f("Espacio - Detalles acerca selected menu entry");
        HelpMenu.AddMenuInfo_f("Digitos 1-9 - Atajo a la entrada del menu");
        HelpMenu.AddMenuInfo_f("A - Menu Acerca de");
        HelpMenu.AddMenuInfo_f("O - Menu Optiones");
        HelpMenu.AddMenuInfo_f("R - Reiniciar Equipo");
        HelpMenu.AddMenuInfo_f("U - Apagar");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case portuguese:
      case brasil:
        HelpMenu.AddMenuInfo_f("ESC - Sai do submenu, atualiza o menu principal");
        HelpMenu.AddMenuInfo_f("F1  - Esta ajuda");
        HelpMenu.AddMenuInfo_f("F2  - Salva preboot.log (somente FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Salva oem DSDT em EFI/CLOVER/ACPI/origin/ (somente FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Salva DSDT corrigido em EFI/CLOVER/ACPI/origin/ (somente FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Salva VideoBios em EFI/CLOVER/misc/ (somente FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Salva screenshot em EFI/CLOVER/misc/ (somente FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Ejeta o volume selecionado (DVD)");
        HelpMenu.AddMenuInfo_f("Espaco - Detalhes sobre a opcao do menu selecionada");
        HelpMenu.AddMenuInfo_f("Tecle 1-9 - Atalho para as entradas do menu");
        HelpMenu.AddMenuInfo_f("A - Sobre o Menu");
        HelpMenu.AddMenuInfo_f("O - Opcoes do Menu");
        HelpMenu.AddMenuInfo_f("R - Reiniciar");
        HelpMenu.AddMenuInfo_f("U - Desligar");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case italian:
        HelpMenu.AddMenuInfo_f("ESC - Esci dal submenu, Aggiorna menu principale");
        HelpMenu.AddMenuInfo_f("F1  - Aiuto");
        HelpMenu.AddMenuInfo_f("F2  - Salva il preboot.log (solo su FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Mostra volumi nascosti");
        HelpMenu.AddMenuInfo_f("F4  - Salva il DSDT oem in EFI/CLOVER/ACPI/origin/ (solo suFAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Salva il patched DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Salva il VideoBios in EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Controlla il suono sull'uscita selezionata");
        HelpMenu.AddMenuInfo_f("F8  - Scarica le uscite audio in EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Cambia la risoluzione dello schermo alla prossima disponibile");
        HelpMenu.AddMenuInfo_f("F10 - Salva screenshot in EFI/CLOVER/misc/ (solo su FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Resetta NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Espelli il volume selezionato (DVD)");
        HelpMenu.AddMenuInfo_f("Spazio - Dettagli sul menu selezionato");
        HelpMenu.AddMenuInfo_f("Digita 1-9 - Abbreviazioni per il menu");
        HelpMenu.AddMenuInfo_f("A - Informazioni");
        HelpMenu.AddMenuInfo_f("O - Menu Opzioni");
        HelpMenu.AddMenuInfo_f("R - Riavvio");
        HelpMenu.AddMenuInfo_f("U - Spegnimento");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case german:
        HelpMenu.AddMenuInfo_f("ESC - Zurueck aus Untermenue, Hauptmenue erneuern");
        HelpMenu.AddMenuInfo_f("F1  - Diese Hilfe");
        HelpMenu.AddMenuInfo_f("F2  - Sichere preboot.log (nur mit FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Sichere OEM DSDT in EFI/CLOVER/ACPI/origin/ (nur mit FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Sichere gepatchtes DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Sichere VideoBios in EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Sichere Bildschirmfoto in EFI/CLOVER/misc/ (nur mit FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Volume auswerfen (DVD)");
        HelpMenu.AddMenuInfo_f("Leertaste - Details über den gewählten Menue Eintrag");
        HelpMenu.AddMenuInfo_f("Zahlen 1-9 - Kurzwahl zum Menue Eintrag");
        HelpMenu.AddMenuInfo_f("A - Menue Informationen");
        HelpMenu.AddMenuInfo_f("O - Menue Optionen");
        HelpMenu.AddMenuInfo_f("R - Neustart");
        HelpMenu.AddMenuInfo_f("U - Ausschalten");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case dutch:
        HelpMenu.AddMenuInfo_f("ESC - Verlaat submenu, Vernieuw hoofdmenu");
        HelpMenu.AddMenuInfo_f("F1  - Onderdeel hulp");
        HelpMenu.AddMenuInfo_f("F2  - preboot.log opslaan (Alleen FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Verborgen opties weergeven");
        HelpMenu.AddMenuInfo_f("F4  - Opslaan oem DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Opslaan gepatchte DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Opslaan VideoBios in EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Controleer geluid op geselecteerde uitgang");
        HelpMenu.AddMenuInfo_f("F8  - Opslaan audio uitgangen in EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Wijzig schermresolutie naar eerstvolgende mogelijke modus");
        HelpMenu.AddMenuInfo_f("F10 - Opslaan schermafdruk in EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Uitwerpen geselecteerd volume (DVD)");
        HelpMenu.AddMenuInfo_f("Spatie - Details over geselecteerd menuoptie");
        HelpMenu.AddMenuInfo_f("Cijfers 1-9 - Snelkoppeling naar menuoptie");
        HelpMenu.AddMenuInfo_f("A - Menu Over");
        HelpMenu.AddMenuInfo_f("O - Menu Opties");
        HelpMenu.AddMenuInfo_f("R - Soft Reset");
        HelpMenu.AddMenuInfo_f("U - Verlaten");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case french:
        HelpMenu.AddMenuInfo_f("ESC - Quitter sous-menu, Retour menu principal");
        HelpMenu.AddMenuInfo_f("F1  - Aide");
        HelpMenu.AddMenuInfo_f("F2  - Enregistrer preboot.log (FAT32 only)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Enregistrer oem DSDT dans EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Enregistrer DSDT modifié dans EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Enregistrer VideoBios dans EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Enregistrer la capture d'écran dans EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Ejecter le volume (DVD)");
        HelpMenu.AddMenuInfo_f("Space - Détails a propos du menu selectionné");
        HelpMenu.AddMenuInfo_f("Digits 1-9 - Raccourci vers entrée menu");
        HelpMenu.AddMenuInfo_f("A - A propos");
        HelpMenu.AddMenuInfo_f("O - Options Menu");
        HelpMenu.AddMenuInfo_f("R - Redémarrer");
        HelpMenu.AddMenuInfo_f("U - Eteindre");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case indonesian:
        HelpMenu.AddMenuInfo_f("ESC - Keluar submenu, Refresh main menu");
        HelpMenu.AddMenuInfo_f("F1  - Help");
        HelpMenu.AddMenuInfo_f("F2  - Simpan preboot.log ke EFI/CLOVER/ACPI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Simpan oem DSDT ke EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Simpan patched DSDT ke EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Simpan VideoBios ke EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Simpan screenshot ke EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Eject volume (DVD)");
        HelpMenu.AddMenuInfo_f("Spasi - Detail dari menu yang dipilih");
        HelpMenu.AddMenuInfo_f("Tombol 1-9 - Shortcut pilihan menu");
        HelpMenu.AddMenuInfo_f("A - About");
        HelpMenu.AddMenuInfo_f("O - Opsi");
        HelpMenu.AddMenuInfo_f("R - Soft Reset");
        HelpMenu.AddMenuInfo_f("U - Shutdown");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case polish:
        HelpMenu.AddMenuInfo_f("ESC - Wyjscie z podmenu, Odswiezenie glownego menu");
        HelpMenu.AddMenuInfo_f("F1  - Pomoc");
        HelpMenu.AddMenuInfo_f("F2  - Zapis preboot.log (tylko FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Zapis DSDT do EFI/CLOVER/ACPI/origin/ (tylko FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Zapis poprawionego DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Zapis BIOSu k. graficznej do EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Zapis zrzutu ekranu do EFI/CLOVER/misc/ (tylko FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Wysuniecie zaznaczonego dysku (tylko dla DVD)");
        HelpMenu.AddMenuInfo_f("Spacja - Informacje nt. dostepnych opcji dla zaznaczonego dysku");
        HelpMenu.AddMenuInfo_f("Znaki 1-9 - Skroty opcji dla wybranego dysku");
        HelpMenu.AddMenuInfo_f("A - Menu Informacyjne");
        HelpMenu.AddMenuInfo_f("O - Menu Opcje");
        HelpMenu.AddMenuInfo_f("R - Restart komputera");
        HelpMenu.AddMenuInfo_f("U - Wylaczenie komputera");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case croatian:
        HelpMenu.AddMenuInfo_f("ESC - izlaz iz podizbornika, Osvježi glavni izbornik");
        HelpMenu.AddMenuInfo_f("F1  - Ovaj izbornik");
        HelpMenu.AddMenuInfo_f("F2  - Spremi preboot.log (samo na FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Spremi oem DSDT u EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Spremi patched DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Spremi VideoBios into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Spremi screenshot into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Izbaci izabrai (DVD)");
        HelpMenu.AddMenuInfo_f("Space - Detalji o odabranom sistemu");
        HelpMenu.AddMenuInfo_f("Brojevi 1 do 9 su prečac do izbora");
        HelpMenu.AddMenuInfo_f("A - Izbornik o meni");
        HelpMenu.AddMenuInfo_f("O - Izbornik opcije");
        HelpMenu.AddMenuInfo_f("R - Restart računala");
        HelpMenu.AddMenuInfo_f("U - Isključivanje računala");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case czech:
        HelpMenu.AddMenuInfo_f("ESC - Vrátit se do hlavní nabídky");
        HelpMenu.AddMenuInfo_f("F1  - Tato Nápověda");
        HelpMenu.AddMenuInfo_f("F2  - Uložit preboot.log (FAT32 only)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Uložit oem DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Uložit patchnuté DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Uložit VideoBios do EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Uložit snímek obrazovky do EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Vysunout vybranou mechaniku (DVD)");
        HelpMenu.AddMenuInfo_f("Mezerník - Podrobnosti o vybraném disku");
        HelpMenu.AddMenuInfo_f("čísla 1-9 - Klávesové zkratky pro disky");
        HelpMenu.AddMenuInfo_f("A - Menu O Programu");
        HelpMenu.AddMenuInfo_f("O - Menu Možnosti");
        HelpMenu.AddMenuInfo_f("R - Částečný restart");
        HelpMenu.AddMenuInfo_f("U - Odejít");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case korean:
        HelpMenu.AddMenuInfo_f("ESC - 하위메뉴에서 나감, 메인메뉴 새로 고침");
        HelpMenu.AddMenuInfo_f("F1  - 이 도움말");
        HelpMenu.AddMenuInfo_f("F2  - preboot.log를 저장합니다. (FAT32방식에만 해당됨)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - oem DSDT를 EFI/CLOVER/ACPI/origin/에 저장합니다. (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - 패치된 DSDT를 EFI/CLOVER/ACPI/origin/에 저장합니다. (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - VideoBios를 EFI/CLOVER/misc/에 저장합니다. (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - 선택한 출력에서 사운드 확인");
        HelpMenu.AddMenuInfo_f("F8  - 오디오 코덱덤프를 EFI/CLOVER/misc/에 저장합니다.");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - 스크린샷을 EFI/CLOVER/misc/에 저장합니다. (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - NVRAM 초기화");
        HelpMenu.AddMenuInfo_f("F12 - 선택한 볼륨을 제거합니다. (DVD)");
        HelpMenu.AddMenuInfo_f("Space - 선택한 메뉴의 상세 설명");
        HelpMenu.AddMenuInfo_f("Digits 1-9 - 메뉴 단축 번호");
        HelpMenu.AddMenuInfo_f("A - 단축키 - 이 부트로더에 관하여");
        HelpMenu.AddMenuInfo_f("O - 단축키 - 부트 옵션");
        HelpMenu.AddMenuInfo_f("R - 단축키 - 리셋");
        HelpMenu.AddMenuInfo_f("U - 단축키 - 시스템 종료");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case romanian:
        HelpMenu.AddMenuInfo_f("ESC - Iesire din sub-meniu, Refresh meniul principal");
        HelpMenu.AddMenuInfo_f("F1  - Ajutor");
        HelpMenu.AddMenuInfo_f("F2  - Salvare preboot.log (doar pentru FAT32)");
        HelpMenu.AddMenuInfo_f("F4  - Salvare oem DSDT in EFI/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Salvare DSDT modificat in EFI/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Salvare VideoBios in EFI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Salvare screenshot in EFI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Scoatere volum selectat (DVD)");
        HelpMenu.AddMenuInfo_f("Space - Detalii despre item-ul selectat");
        HelpMenu.AddMenuInfo_f("Cifre 1-9 - Scurtaturi pentru itemele meniului");
        HelpMenu.AddMenuInfo_f("A - Despre");
        HelpMenu.AddMenuInfo_f("O - Optiuni");
        HelpMenu.AddMenuInfo_f("R - Soft Reset");
        HelpMenu.AddMenuInfo_f("U - Inchidere");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case chinese:
        HelpMenu.AddMenuInfo_f("ESC - 离开子菜单， 刷新主菜单");
        HelpMenu.AddMenuInfo_f("F1  - 帮助");
        HelpMenu.AddMenuInfo_f("F2  - 保存 preboot.log 到 EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - 显示隐藏的启动项");
        HelpMenu.AddMenuInfo_f("F4  - 保存原始的 DSDT 到 EFI/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - 保存修正后的 DSDT 到 EFI/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - 保存 VideoBios 到 EFI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - 检查选中输出设备的声音");
        HelpMenu.AddMenuInfo_f("F8  - 生成声卡输出转储到 EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - 调整屏幕分辨率为下一个可用的模式");
        HelpMenu.AddMenuInfo_f("F10 - 保存截图到 EFI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - 重置 NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - 推出选中的卷 (DVD)");
        HelpMenu.AddMenuInfo_f("空格 - 关于选中项的详情");
        HelpMenu.AddMenuInfo_f("数字 1-9 - 菜单快捷键");
        HelpMenu.AddMenuInfo_f("A - 关于");
        HelpMenu.AddMenuInfo_f("O - 选项");
        HelpMenu.AddMenuInfo_f("R - 软复位");
        HelpMenu.AddMenuInfo_f("U - 退出");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
      case english:
      default:
        HelpMenu.AddMenuInfo_f("ESC - Escape from submenu, Refresh main menu");
        HelpMenu.AddMenuInfo_f("F1  - This help");
        HelpMenu.AddMenuInfo_f("F2  - Save preboot.log into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F3  - Show hidden entries");
        HelpMenu.AddMenuInfo_f("F4  - Save oem DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F5  - Save patched DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F6  - Save VideoBios into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo_f("F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo_f("F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo_f("F10 - Save screenshot into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo_f("F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo_f("F12 - Eject selected volume (DVD)");
        HelpMenu.AddMenuInfo_f("Space - Details about selected menu entry");
        HelpMenu.AddMenuInfo_f("Digits 1-9 - Shortcut to menu entry");
        HelpMenu.AddMenuInfo_f("A - Menu About");
        HelpMenu.AddMenuInfo_f("O - Menu Options");
        HelpMenu.AddMenuInfo_f("R - Soft Reset");
        HelpMenu.AddMenuInfo_f("U - Exit from Clover");
        HelpMenu.AddMenuInfo_f("S - Shell");
        break;
    }
    HelpMenu.AnimeRun = HelpMenu.GetAnime();
    HelpMenu.AddMenuEntry(&MenuEntryReturn, false);
  }

  HelpMenu.RunMenu(NULL);
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
#if !USE_XTHEME
VOID InitSelection(VOID)
{

  if (!AllowGraphicsMode)
    return;
  SelectionBackgroundPixel.r = (GlobalConfig.SelectionColor >> 24) & 0xFF;
  SelectionBackgroundPixel.g = (GlobalConfig.SelectionColor >> 16) & 0xFF;
  SelectionBackgroundPixel.b = (GlobalConfig.SelectionColor >> 8) & 0xFF;
  SelectionBackgroundPixel.a = (GlobalConfig.SelectionColor >> 0) & 0xFF;

  if (SelectionImages[0] != NULL) {
    return;
  }
  // load small selection image
  if (GlobalConfig.SelectionSmallFileName != NULL){
    SelectionImages[2] = egLoadImage(ThemeDir, GlobalConfig.SelectionSmallFileName, FALSE);
  }
  if (SelectionImages[2] == NULL){
    SelectionImages[2] = BuiltinIcon(BUILTIN_SELECTION_SMALL);
    SelectionImages[2]->HasAlpha = FALSE; // support transparensy for selection icons
    CopyMem(&BlueBackgroundPixel, &StdBackgroundPixel, sizeof(EG_PIXEL));
  }
  SelectionImages[2] = egEnsureImageSize(SelectionImages[2],
                                         row1TileSize, row1TileSize, &MenuBackgroundPixel);
  if (SelectionImages[2] == NULL) {
    return;
  }
  // load big selection image
  if (!GlobalConfig.TypeSVG && GlobalConfig.SelectionBigFileName != NULL) {
    SelectionImages[0] = egLoadImage(ThemeDir, GlobalConfig.SelectionBigFileName, FALSE);
    SelectionImages[0] = egEnsureImageSize(SelectionImages[0],
                                           row0TileSize, row0TileSize,
                                           &MenuBackgroundPixel);
  }
  if (SelectionImages[0] == NULL) {
    // calculate big selection image from small one
    SelectionImages[0] = BuiltinIcon(BUILTIN_SELECTION_BIG);
    SelectionImages[0]->HasAlpha = FALSE; // support transparensy for selection icons
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
      SelectionImages[4] = egDecodePNG(ACCESS_EMB_DATA(emb_selection_indicator), ACCESS_EMB_SIZE(emb_selection_indicator), TRUE);

    }
    INTN ScaledIndicatorSize = (INTN)(INDICATOR_SIZE * GlobalConfig.Scale);
    SelectionImages[4] = egEnsureImageSize(SelectionImages[4], ScaledIndicatorSize, ScaledIndicatorSize, &MenuBackgroundPixel);
    if (!SelectionImages[4]) {
      SelectionImages[4] = egCreateFilledImage(ScaledIndicatorSize, ScaledIndicatorSize,
                                               TRUE, &StdBackgroundPixel);

    }
    SelectionImages[5] = egCreateFilledImage(ScaledIndicatorSize, ScaledIndicatorSize,
                                             TRUE, &MenuBackgroundPixel);
  }

  /*
  Button & radio, or any other next icons with builtin icon as fallback should synced to:
   - BUILTIN_ICON_* in lib.h
   - BuiltinIconTable in icns.c
   - Data in egemb_icons.h / scroll_images.h
  */

  // Radio buttons
  //it was a nonsense egLoadImage is just inluded into egLoadIcon.
  // will be corrected with XTheme support
  //the procedure loadIcon should also check embedded icons
  Buttons[0] = egLoadImage(ThemeDir, GetIconsExt(L"radio_button", L"png"), TRUE); //memory leak
  Buttons[1] = egLoadImage(ThemeDir, GetIconsExt(L"radio_button_selected", L"png"), TRUE);
  if (!Buttons[0]) {
    Buttons[0] = egLoadIcon(ThemeDir, L"radio_button.png", 48);
  }
  if (!Buttons[0]) {
    Buttons[0] = egDecodePNG(ACCESS_EMB_DATA(emb_radio_button), ACCESS_EMB_SIZE(emb_radio_button), TRUE);
  }
  if (!Buttons[1]) {
    Buttons[1] = egLoadIcon(ThemeDir, L"radio_button_selected.png", 48);
  }

  if (!Buttons[1]) {
    Buttons[1] = egDecodePNG(ACCESS_EMB_DATA(emb_radio_button_selected), ACCESS_EMB_SIZE(emb_radio_button_selected), TRUE);
  }

  // Checkbox
  Buttons[2] = egLoadImage(ThemeDir, GetIconsExt(L"checkbox", L"png"), TRUE);
  Buttons[3] = egLoadImage(ThemeDir, GetIconsExt(L"checkbox_checked", L"png"), TRUE);
  if (!Buttons[2]) {
//    DBG("egLoadIcon checkbox\n");
    Buttons[2] = egLoadIcon(ThemeDir, L"checkbox.png", 48);
  }
  if (!Buttons[3]) {
//    DBG("egLoadIcon checkbox_checked\n");
    Buttons[3] = egLoadIcon(ThemeDir, L"checkbox_checked.png", 48);
  }

  if (!Buttons[2]) {
//    DBG("embedded checkbox\n");
    Buttons[2] = egDecodePNG(ACCESS_EMB_DATA(emb_checkbox), ACCESS_EMB_SIZE(emb_checkbox), TRUE);
  }

  if (!Buttons[3]) {
//    DBG("embedded checkbox_checked\n");
    Buttons[3] = egDecodePNG(ACCESS_EMB_DATA(emb_checkbox_checked), ACCESS_EMB_SIZE(emb_checkbox_checked), TRUE);
  }
  // non-selected background images
  //totally wrong
  if (GlobalConfig.SelectionBigFileName != NULL) {
    SelectionImages[1] = egCreateFilledImage(row0TileSize, row0TileSize,
                                             TRUE, &MenuBackgroundPixel);
    SelectionImages[3] = egCreateFilledImage(row1TileSize, row1TileSize,
                                             TRUE, &MenuBackgroundPixel);
  } else { // using embedded theme (this is an assumption but a better check is required)
    EG_PIXEL BackgroundPixel;
    if (GlobalConfig.DarkEmbedded || GlobalConfig.TypeSVG) {
      BackgroundPixel = DarkEmbeddedBackgroundPixel;
      BackgroundPixel.a = 0x00;
    } else {
      BackgroundPixel = StdBackgroundPixel;
      BackgroundPixel.a = 0xff;
    }
    if (GlobalConfig.DarkEmbedded) { //nonsense then equal else
      SelectionImages[1] = egCreateFilledImage(row0TileSize, row0TileSize,
                                               TRUE, &BackgroundPixel);
      SelectionImages[3] = egCreateFilledImage(row1TileSize, row1TileSize,
                                               TRUE, &BackgroundPixel);

    } else {
      SelectionImages[1] = egCreateFilledImage(row0TileSize, row0TileSize,
                                               TRUE, &BackgroundPixel); //&StdBackgroundPixel);
      SelectionImages[3] = egCreateFilledImage(row1TileSize, row1TileSize,
                                               TRUE, &BackgroundPixel);
    }
  }
//  DBG("selections inited\n");
}
#endif
//
// Scrolling functions
//
#define CONSTRAIN_MIN(Variable, MinValue) if (Variable < MinValue) Variable = MinValue
#define CONSTRAIN_MAX(Variable, MaxValue) if (Variable > MaxValue) Variable = MaxValue

#if !USE_XTHEME
INTN DrawTextXY(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign)
{
  INTN      TextWidth = 0;
  INTN      XText = 0;
  INTN      Height;
  INTN      TextXYStyle = 1;
  EG_IMAGE  *TextBufferXY = NULL;

  if (!Text) {
    return 0;
  }
  if (!textFace[1].valid) {
    if (textFace[2].valid) {
      TextXYStyle = 2;
    } else {
      TextXYStyle = 0;
    }
  }

  egMeasureText(Text, &TextWidth, NULL);

  if (XAlign == X_IS_LEFT) {
    TextWidth = UGAWidth - XPos - 1;
    XText = XPos;
  }

  if (GlobalConfig.TypeSVG) {
    TextWidth += TextHeight * 2; //give more place for buffer
    if (!textFace[TextXYStyle].valid) {
      DBG("no vaid text face for message!\n");
      Height = TextHeight;
    } else {
      Height = (int)(textFace[TextXYStyle].size * RowHeightFromTextHeight * GlobalConfig.Scale);
    }
  } else {
    Height = TextHeight;
  }

  TextBufferXY = egCreateFilledImage(TextWidth, Height, TRUE, &MenuBackgroundPixel);

  // render the text
  TextWidth = egRenderText(Text, TextBufferXY, 0, 0, 0xFFFF, TextXYStyle);

  if (XAlign != X_IS_LEFT) {
    // shift 64 is prohibited
    XText = XPos - (TextWidth >> XAlign);  //X_IS_CENTER = 1
  }
//  DBG("draw text %ls\n", Text);
//  DBG("pos=%d width=%d xtext=%d Height=%d Y=%d\n", XPos, TextWidth, XText, Height, YPos);
  BltImageAlpha(TextBufferXY, XText, YPos,  &MenuBackgroundPixel, 16);
  egFreeImage(TextBufferXY);

  return TextWidth;
}
#endif
/**
 * Helper function to draw text for Boot Camp Style.
 * @author: Needy
 */

#if USE_XTHEME

#else
VOID DrawMenuText(IN CONST CHAR16 *Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor)
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
    egFillImageArea(TextBuffer, 0, 0, (INTN)SelectedWidth, TextHeight,
                    &SelectionBackgroundPixel);
  }

  // render the text
  if (GlobalConfig.TypeSVG) {
    //clovy - text veltically centred on Height
    egRenderText(Text, TextBuffer, 0,
                 (INTN)((TextHeight - (textFace[TextStyle].size * GlobalConfig.Scale)) / 2),
                 Cursor, TextStyle);
  } else {
    egRenderText(Text, TextBuffer, TEXT_XMARGIN, TEXT_YMARGIN, Cursor, TextStyle);
  }
  BltImageAlpha(TextBuffer, (INTN)XPos, (INTN)YPos, &MenuBackgroundPixel, 16);
}

#endif

#if !USE_XTHEME
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
        ScrollbarBackgroundImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_fill", L"png"), FALSE);
    }
    if (!BarStartImage) {
      BarStartImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_start", L"png"), TRUE);
    }
    if (!BarEndImage) {
      BarEndImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_end", L"png"), TRUE);
    }
    if (!ScrollbarImage) {
      ScrollbarImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_fill", L"png"), FALSE);
    }
    if (!ScrollStartImage) {
      ScrollStartImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_start", L"png"), TRUE);
    }
    if (!ScrollEndImage) {
      ScrollEndImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_end", L"png"), TRUE);
    }
    if (!UpButtonImage) {
      UpButtonImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\up_button", L"png"), TRUE);
    }
    if (!DownButtonImage) {
      DownButtonImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\down_button", L"png"), TRUE);
    }
  }

  if (!BarStartImage  && !GlobalConfig.TypeSVG) {
    BarStartImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_bar_start), ACCESS_EMB_SIZE(emb_scroll_bar_start), TRUE);
  }
  if (!BarEndImage && !GlobalConfig.TypeSVG) {
    BarEndImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_bar_end), ACCESS_EMB_SIZE(emb_scroll_bar_end), TRUE);
  }
  if (!ScrollbarBackgroundImage) {
    if (GlobalConfig.TypeSVG) {
       ScrollbarBackgroundImage = egLoadIcon(ThemeDir, L"scrollbar-background.png", 48);
    }
    if (!ScrollbarBackgroundImage) {
      ScrollbarBackgroundImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_bar_fill), ACCESS_EMB_SIZE(emb_scroll_bar_fill), TRUE);
    }
  }
  if (!ScrollbarImage) {
    if (GlobalConfig.TypeSVG) {
      ScrollbarImage = egLoadIcon(ThemeDir, L"scrollbar-holder.png", 48);
    }
    if (!ScrollbarImage) {
      ScrollbarImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_scroll_fill), ACCESS_EMB_SIZE(emb_scroll_scroll_fill), TRUE);
    }
  }
  if (!ScrollStartImage && !GlobalConfig.TypeSVG) {
    ScrollStartImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_scroll_start), ACCESS_EMB_SIZE(emb_scroll_scroll_start), TRUE);
  }
  if (!ScrollEndImage && !GlobalConfig.TypeSVG) {
    ScrollEndImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_scroll_end), ACCESS_EMB_SIZE(emb_scroll_scroll_end), TRUE);
  }
  if (!UpButtonImage && !GlobalConfig.TypeSVG) {
    UpButtonImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_up_button), ACCESS_EMB_SIZE(emb_scroll_up_button), TRUE);
  }
  if (!DownButtonImage && !GlobalConfig.TypeSVG) {
    DownButtonImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_down_button), ACCESS_EMB_SIZE(emb_scroll_down_button), TRUE);
  }
  if (!GlobalConfig.TypeSVG) {
    UpButton.Width      = ScrollWidth; // 16
    UpButton.Height     = ScrollButtonsHeight; // 20
    DownButton.Width    = UpButton.Width;
    DownButton.Height   = ScrollButtonsHeight;
    BarStart.Height     = ScrollBarDecorationsHeight; // 5
    BarEnd.Height       = ScrollBarDecorationsHeight;
    ScrollStart.Height  = ScrollScrollDecorationsHeight; // 7
    ScrollEnd.Height    = ScrollScrollDecorationsHeight;

  } else {
    UpButton.Width      = ScrollWidth; // 16
    UpButton.Height     = 0; // 20
    DownButton.Width    = UpButton.Width;
    DownButton.Height   = 0;
    BarStart.Height     = ScrollBarDecorationsHeight; // 5
    BarEnd.Height       = ScrollBarDecorationsHeight;
    ScrollStart.Height  = 0; // 7
    ScrollEnd.Height    = 0;

  }
}
#endif

/**
 * Draw entries for GUI.
 */
#if USE_XTHEME

#else


VOID DrawMainMenuEntry(REFIT_ABSTRACT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos)
{
  EG_IMAGE* MainImage = NULL;
  EG_IMAGE* BadgeImage = NULL;
  bool NewImageCreated = false;
  INTN Scale = GlobalConfig.MainEntriesSize >> 3; //usually it is 128>>3 == 16. if 256>>3 == 32

  if (Entry->Row == 0 && Entry->getDriveImage()  &&  !(GlobalConfig.HideBadges & HDBADGES_SWAP)) {
    MainImage = Entry->getDriveImage();
  } else {
    MainImage = Entry->Image;
  }
//this should be inited by the Theme
  if (!MainImage) {
    if (!IsEmbeddedTheme()) {
      MainImage = egLoadIcon(ThemeDir, GetIconsExt(L"icons\\os_mac", L"icns"), Scale << 3);
    }
    if (!MainImage) {
      MainImage = DummyImage(Scale << 3);
    }
    if (MainImage) {
      NewImageCreated = true;
    }
  }
  //  DBG("Entry title=%ls; Width=%d\n", Entry->Title, MainImage->Width);

  if (GlobalConfig.TypeSVG) {
    Scale = 16 * (selected ? 1 : -1);
  } else {
    Scale = ((Entry->Row == 0) ? (Scale * (selected ? 1 : -1)): 16) ;
  }

  if (Entry->Row == 0) {
    BadgeImage = Entry->getBadgeImage();
  } //else null

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
  TextBufferXY.DrawOnBack(XText, YPos, ThemeX.Background);
//  TextBufferXY.DrawWithoutCompose(XText, YPos);
  return TextWidth2;
}

void REFIT_MENU_SCREEN::EraseTextXY()
{
  OldTextBufferImage.Draw(OldTextBufferRect.XPos, OldTextBufferRect.YPos);
}
#else


INTN DrawTextXY(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign)
{
  INTN      TextWidth = 0;
  INTN      XText = 0;
  INTN      Height;
  INTN      TextXYStyle = 1;
  EG_IMAGE  *TextBufferXY = NULL;

  if (!Text) {
    return 0;
  }
  if (!textFace[1].valid) {
    if (textFace[2].valid) {
      TextXYStyle = 2;
    } else {
      TextXYStyle = 0;
    }
  }

  egMeasureText(Text, &TextWidth, NULL);

  if (XAlign == X_IS_LEFT) {
    TextWidth = UGAWidth - XPos - 1;
    XText = XPos;
  }

  if (GlobalConfig.TypeSVG) {
    TextWidth += TextHeight * 2; //give more place for buffer
    if (!textFace[TextXYStyle].valid) {
      DBG("no vaid text face for message!\n");
      Height = TextHeight;
    } else {
      Height = (int)(textFace[TextXYStyle].size * RowHeightFromTextHeight * GlobalConfig.Scale);
    }
  } else {
    Height = TextHeight;
  }

  TextBufferXY = egCreateFilledImage(TextWidth, Height, TRUE, &MenuBackgroundPixel);

  // render the text
  TextWidth = egRenderText(Text, TextBufferXY, 0, 0, 0xFFFF, TextXYStyle);

  if (XAlign != X_IS_LEFT) {
    // shift 64 is prohibited
    XText = XPos - (TextWidth >> XAlign);  //X_IS_CENTER = 1
  }
//  DBG("draw text %ls\n", Text);
//  DBG("pos=%d width=%d xtext=%d Height=%d Y=%d\n", XPos, TextWidth, XText, Height, YPos);
  BltImageAlpha(TextBufferXY, XText, YPos,  &MenuBackgroundPixel, 16);
  egFreeImage(TextBufferXY);

  return TextWidth;
}
#endif
/**
 * Helper function to draw text for Boot Camp Style.
 * @author: Needy
 */
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
#if USE_XTHEME
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
#else
  if (GlobalConfig.TileXSpace >= 25 && GlobalConfig.TileXSpace < 30) {
    MaxTextLen = 14;
  } else if (GlobalConfig.TileXSpace >= 30 && GlobalConfig.TileXSpace < 35) {
    MaxTextLen = 15;
  } else if (GlobalConfig.TileXSpace >= 35 && GlobalConfig.TileXSpace < 40) {
    MaxTextLen = 16;
  } else if (GlobalConfig.TileXSpace >= 40 && GlobalConfig.TileXSpace < 45) {
    MaxTextLen = 17;
  } else if (GlobalConfig.TileXSpace >= 45 && GlobalConfig.TileXSpace < 50) {
    MaxTextLen = 18;
  } else if (GlobalConfig.TileXSpace >= 50) {
    MaxTextLen = 19;
  }
#endif

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
#if USE_XTHEME
    XStringW BCSTextX;
    BCSTextX.takeValueFrom(BCSText);
    DrawTextXY(BCSTextX, XPos, YPos, XAlign);
#else
    DrawTextXY(BCSText, XPos, YPos, XAlign);
#endif

    FreePool(BCSText);
  } else {
		// draw full text
#if USE_XTHEME
    XStringW TextX;
    TextX.takeValueFrom(Text);
    DrawTextXY(TextX, XPos, YPos, XAlign);
#else
    DrawTextXY(Text, XPos, YPos, XAlign);
#endif

  }
}

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
VOID DrawMenuText(IN CONST CHAR16 *Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor)
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
    egFillImageArea(TextBuffer, 0, 0, (INTN)SelectedWidth, TextHeight,
                    &SelectionBackgroundPixel);
  }

  // render the text
  if (GlobalConfig.TypeSVG) {
    //clovy - text veltically centred on Height
    egRenderText(Text, TextBuffer, 0,
                 (INTN)((TextHeight - (textFace[TextStyle].size * GlobalConfig.Scale)) / 2),
                 Cursor, TextStyle);
  } else {
    egRenderText(Text, TextBuffer, TEXT_XMARGIN, TEXT_YMARGIN, Cursor, TextStyle);
  }
  BltImageAlpha(TextBuffer, (INTN)XPos, (INTN)YPos, &MenuBackgroundPixel, 16);
}

#endif

#if !USE_XTHEME
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
        ScrollbarBackgroundImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_fill", L"png"), FALSE);
    }
    if (!BarStartImage) {
      BarStartImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_start", L"png"), TRUE);
    }
    if (!BarEndImage) {
      BarEndImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\bar_end", L"png"), TRUE);
    }
    if (!ScrollbarImage) {
      ScrollbarImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_fill", L"png"), FALSE);
    }
    if (!ScrollStartImage) {
      ScrollStartImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_start", L"png"), TRUE);
    }
    if (!ScrollEndImage) {
      ScrollEndImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\scroll_end", L"png"), TRUE);
    }
    if (!UpButtonImage) {
      UpButtonImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\up_button", L"png"), TRUE);
    }
    if (!DownButtonImage) {
      DownButtonImage = egLoadImage(ThemeDir, GetIconsExt(L"scrollbar\\down_button", L"png"), TRUE);
    }
  }

  if (!BarStartImage  && !GlobalConfig.TypeSVG) {
    BarStartImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_bar_start), ACCESS_EMB_SIZE(emb_scroll_bar_start), TRUE);
  }
  if (!BarEndImage && !GlobalConfig.TypeSVG) {
    BarEndImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_bar_end), ACCESS_EMB_SIZE(emb_scroll_bar_end), TRUE);
  }
  if (!ScrollbarBackgroundImage) {
    if (GlobalConfig.TypeSVG) {
       ScrollbarBackgroundImage = egLoadIcon(ThemeDir, L"scrollbar-background.png", 48);
    }
    if (!ScrollbarBackgroundImage) {
      ScrollbarBackgroundImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_bar_fill), ACCESS_EMB_SIZE(emb_scroll_bar_fill), TRUE);
    }
  }
  if (!ScrollbarImage) {
    if (GlobalConfig.TypeSVG) {
      ScrollbarImage = egLoadIcon(ThemeDir, L"scrollbar-holder.png", 48);
    }
    if (!ScrollbarImage) {
      ScrollbarImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_scroll_fill), ACCESS_EMB_SIZE(emb_scroll_scroll_fill), TRUE);
    }
  }
  if (!ScrollStartImage && !GlobalConfig.TypeSVG) {
    ScrollStartImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_scroll_start), ACCESS_EMB_SIZE(emb_scroll_scroll_start), TRUE);
  }
  if (!ScrollEndImage && !GlobalConfig.TypeSVG) {
    ScrollEndImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_scroll_end), ACCESS_EMB_SIZE(emb_scroll_scroll_end), TRUE);
  }
  if (!UpButtonImage && !GlobalConfig.TypeSVG) {
    UpButtonImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_up_button), ACCESS_EMB_SIZE(emb_scroll_up_button), TRUE);
  }
  if (!DownButtonImage && !GlobalConfig.TypeSVG) {
    DownButtonImage = egDecodePNG(ACCESS_EMB_DATA(emb_scroll_down_button), ACCESS_EMB_SIZE(emb_scroll_down_button), TRUE);
  }
  if (!GlobalConfig.TypeSVG) {
    UpButton.Width      = ScrollWidth; // 16
    UpButton.Height     = ScrollButtonsHeight; // 20
    DownButton.Width    = UpButton.Width;
    DownButton.Height   = ScrollButtonsHeight;
    BarStart.Height     = ScrollBarDecorationsHeight; // 5
    BarEnd.Height       = ScrollBarDecorationsHeight;
    ScrollStart.Height  = ScrollScrollDecorationsHeight; // 7
    ScrollEnd.Height    = ScrollScrollDecorationsHeight;

  } else {
    UpButton.Width      = ScrollWidth; // 16
    UpButton.Height     = 0; // 20
    DownButton.Width    = UpButton.Width;
    DownButton.Height   = 0;
    BarStart.Height     = ScrollBarDecorationsHeight; // 5
    BarEnd.Height       = ScrollBarDecorationsHeight;
    ScrollStart.Height  = 0; // 7
    ScrollEnd.Height    = 0;

  }
}
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
//  INTN iLast;
  INTN Chosen = 0;
  INTN ItemWidth = 0;
  INTN t1, t2;
  INTN VisibleHeight = 0; //assume vertical layout
//  CHAR16 ResultString[TITLE_MAX_LEN]; // assume a title max length of around 128
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

VOID DrawMainMenuEntry(REFIT_ABSTRACT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos)
{
  INTN MainSize = ThemeX.MainEntriesSize;
  XImage MainImage(MainSize, MainSize);
  XImage* BadgeImage;

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
#else


VOID DrawMainMenuEntry(REFIT_ABSTRACT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos)
{
  EG_IMAGE* MainImage = NULL;
  EG_IMAGE* BadgeImage = NULL;
  bool NewImageCreated = false;
  INTN Scale = GlobalConfig.MainEntriesSize >> 3; //usually it is 128>>3 == 16. if 256>>3 == 32

  if (Entry->Row == 0 && Entry->getDriveImage()  &&  !(GlobalConfig.HideBadges & HDBADGES_SWAP)) {
    MainImage = Entry->getDriveImage();
  } else {
    MainImage = Entry->Image;
  }
//this should be inited by the Theme
  if (!MainImage) {
    if (!IsEmbeddedTheme()) {
      MainImage = egLoadIcon(ThemeDir, GetIconsExt(L"icons\\os_mac", L"icns"), Scale << 3);
    }
    if (!MainImage) {
      MainImage = DummyImage(Scale << 3);
    }
    if (MainImage) {
      NewImageCreated = true;
    }
  }
  //  DBG("Entry title=%ls; Width=%d\n", Entry->Title, MainImage->Width);

  if (GlobalConfig.TypeSVG) {
    Scale = 16 * (selected ? 1 : -1);
  } else {
    Scale = ((Entry->Row == 0) ? (Scale * (selected ? 1 : -1)): 16) ;
  }

  if (Entry->Row == 0) {
    BadgeImage = Entry->getBadgeImage();
  } //else null


  if (GlobalConfig.SelectionOnTop) {
    SelectionImages[0]->HasAlpha = TRUE;
    SelectionImages[2]->HasAlpha = TRUE;
    //MainImage->HasAlpha = TRUE;
      BltImageCompositeBadge(MainImage,
                             SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
                             BadgeImage,
                             XPos, YPos, Scale);
  } else {
      BltImageCompositeBadge(SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
                             MainImage,
                             BadgeImage,
                             XPos, YPos, Scale);
  }
  // draw BCS indicator
  // Needy: if Labels (Titles) are hidden there is no point to draw the indicator
  if (GlobalConfig.BootCampStyle && !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
    SelectionImages[4]->HasAlpha = TRUE;

    // indicator is for row 0, main entries, only
    if (Entry->Row == 0) {
      BltImageAlpha(SelectionImages[4 + (selected ? 0 : 1)],
                    XPos + (row0TileSize / 2) - (INTN)(INDICATOR_SIZE * 0.5f * GlobalConfig.Scale),
                    row0PosY + row0TileSize + TextHeight + (INTN)((BCSMargin * 2) * GlobalConfig.Scale),
                    &MenuBackgroundPixel, Scale);
    }
  }

  Entry->Place.XPos = XPos;
  Entry->Place.YPos = YPos;
  Entry->Place.Width = MainImage->Width;
  Entry->Place.Height = MainImage->Height;
  //we can't free MainImage because it may be new image or it may be a link to entry image
  // a workaround
  if (NewImageCreated) {
    egFreeImage(MainImage);
  }
}
#endif
//the purpose of the procedure is restore Background in rect
//XAlign is always centre, Color is the Backgrounf fill
#if USE_XTHEME
VOID XTheme::FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height)
{
  XImage TmpBuffer(Width, Height);
//  TmpBuffer.CopyScaled(Background, 1.f);
  INTN X = XPos - (Width >> 1);  //X_IS_CENTRE
  TmpBuffer.CopyRect(Background, X, YPos); //a part of BackGround image
  TmpBuffer.DrawWithoutCompose(X, YPos);
//  TmpBuffer.Draw(X, YPos, 0, true);
}
#else
VOID FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height, IN EG_PIXEL *Color, IN UINT8 XAlign)
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
  INTN i;
  row0PosX = 0;
  row1PosX = Entries.size();
  // layout
  row0Count = 0; //Nr items in row0
  row1Count = 0;
  for (i = 0; i < (INTN)Entries.size(); i++) {
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
  XString Text;

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
        Text = XString() + gFirmwareRevision + ", booted from " + SelfVolume->VolLabel;
      }
      if (Text.isEmpty()) {
        Text = XString() + gFirmwareRevision + " " + SelfVolume->VolName;
      }
      break;
    case TEXT_CORNER_HELP:
      Text = XString() + "F1:Help";
      break;
    case TEXT_CORNER_OPTIMUS:
      if (gGraphics[0].Vendor != Intel) {
        Text = XString() + "Discrete";
      } else {
        Text = XString() + "Intel";
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
VOID DrawTextCorner(UINTN TextC, UINT8 Align)
{
  INTN    Xpos;
  CHAR16  *Text = NULL;

  if (
      // HIDEUI_ALL - included
      ((TextC == TEXT_CORNER_REVISION) && ((GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION) != 0)) ||
      ((TextC == TEXT_CORNER_HELP) && ((GlobalConfig.HideUIFlags & HIDEUI_FLAG_HELP) != 0)) ||
      ((TextC == TEXT_CORNER_OPTIMUS) && (GlobalConfig.ShowOptimus == FALSE))
      ) {
    return;
  }

  switch (TextC) {
    case TEXT_CORNER_REVISION:
      // Display Clover boot volume
      if (SelfVolume->VolLabel && SelfVolume->VolLabel[0] != L'#') {
        Text = PoolPrint(L"%s, booted from %s", gFirmwareRevision, SelfVolume->VolLabel);
      }
      if ( !Text ) {
        Text = PoolPrint(L"%s", gFirmwareRevision, SelfVolume->VolName);
      }
      break;
    case TEXT_CORNER_HELP:
      Text = PoolPrint(L"F1:Help");
      break;
    case TEXT_CORNER_OPTIMUS:
      if (gGraphics[0].Vendor != Intel) {
        Text = PoolPrint(L"Discrete");
      } else {
        Text = PoolPrint(L"Intel");
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
      Xpos = UGAWidth - (INTN)(TextHeight * 0.7f);//2
      break;
    case X_IS_CENTER:
      Xpos = UGAWidth >> 1;
      break;
    default:
      if ( Text ) FreePool(Text);
      return;
  }
  //  DBG("draw text %ls at (%d, %d)\n", Text, Xpos, UGAHeight - 5 - TextHeight),
  // clovy  DrawTextXY(Text, Xpos, UGAHeight - 5 - TextHeight, Align);
  DrawTextXY(Text, Xpos, UGAHeight - (INTN)(TextHeight * 1.5f), Align);
  if ( Text ) FreePool(Text);
}
#endif



#if USE_XTHEME
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


    // indicator is for row 0, main entries, only
    if (Entry->Row == 0) {
      BltImageAlpha(SelectionImages[4 + (selected ? 0 : 1)],
                    XPos + (row0TileSize / 2) - (INTN)(INDICATOR_SIZE * 0.5f * GlobalConfig.Scale),
                    row0PosY + row0TileSize + TextHeight + (INTN)((BCSMargin * 2) * GlobalConfig.Scale),
                    &MenuBackgroundPixel, Scale);
    }
  }

  Entry->Place.XPos = XPos;
  Entry->Place.YPos = YPos;
  Entry->Place.Width = MainImage->Width;
  Entry->Place.Height = MainImage->Height;
  //we can't free MainImage because it may be new image or it may be a link to entry image
  // a workaround
  if (NewImageCreated) {
    egFreeImage(MainImage);
  }
}
#endif
//the purpose of the procedure is restore Background in rect
//XAlign is always centre, Color is the Backgrounf fill
#if USE_XTHEME
#else
VOID FillRectAreaOfScreen(IN INTN XPos, IN INTN YPos, IN INTN Width, IN INTN Height, IN EG_PIXEL *Color, IN UINT8 XAlign)
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
#endif

#if USE_XTHEME

#else
VOID DrawTextCorner(UINTN TextC, UINT8 Align)
{
  INTN    Xpos;
  CHAR16  *Text = NULL;

  if (
      // HIDEUI_ALL - included
      ((TextC == TEXT_CORNER_REVISION) && ((GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION) != 0)) ||
      ((TextC == TEXT_CORNER_HELP) && ((GlobalConfig.HideUIFlags & HIDEUI_FLAG_HELP) != 0)) ||
      ((TextC == TEXT_CORNER_OPTIMUS) && (GlobalConfig.ShowOptimus == FALSE))
      ) {
    return;
  }

  switch (TextC) {
    case TEXT_CORNER_REVISION:
      // Display Clover boot volume
      if (SelfVolume->VolLabel && SelfVolume->VolLabel[0] != L'#') {
        Text = PoolPrint(L"%s, booted from %s", gFirmwareRevision, SelfVolume->VolLabel);
      }
      if ( !Text ) {
        Text = PoolPrint(L"%s", gFirmwareRevision, SelfVolume->VolName);
      }
      break;
    case TEXT_CORNER_HELP:
      Text = PoolPrint(L"F1:Help");
      break;
    case TEXT_CORNER_OPTIMUS:
      if (gGraphics[0].Vendor != Intel) {
        Text = PoolPrint(L"Discrete");
      } else {
        Text = PoolPrint(L"Intel");
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
      Xpos = UGAWidth - (INTN)(TextHeight * 0.7f);//2
      break;
    case X_IS_CENTER:
      Xpos = UGAWidth >> 1;
      break;
    default:
      if ( Text ) FreePool(Text);
      return;
  }
  //  DBG("draw text %ls at (%d, %d)\n", Text, Xpos, UGAHeight - 5 - TextHeight),
  // clovy  DrawTextXY(Text, Xpos, UGAHeight - 5 - TextHeight, Align);
  DrawTextXY(Text, Xpos, UGAHeight - (INTN)(TextHeight * 1.5f), Align);
  if ( Text ) FreePool(Text);
}
#endif



//
// user-callable dispatcher functions
//

REFIT_ABSTRACT_MENU_ENTRY* NewEntry_(REFIT_ABSTRACT_MENU_ENTRY *Entry, REFIT_MENU_SCREEN **SubScreen, ACTION AtClick, UINTN ID, CONST CHAR8 *Title)
{
    if ( Title ) Entry->Title.takeValueFrom(Title);
    else Entry->Title.setEmpty();
//  if (Title) {
//  } else {
//    Entry->Title = (__typeof__(Entry->Title))AllocateZeroPool(128);
//  }

  Entry->Image =  OptionMenu.TitleImage;
  Entry->AtClick = AtClick;
  // create the submenu
//  *SubScreen = (__typeof_am__(*SubScreen))AllocateZeroPool(sizeof(**SubScreen));
  *SubScreen = new REFIT_MENU_SCREEN();
//  (*SubScreen)->Title = EfiStrDuplicate(Entry->Title);
  (*SubScreen)->Title = Entry->Title;
  (*SubScreen)->TitleImage = Entry->Image;
  (*SubScreen)->ID = ID;
  (*SubScreen)->AnimeRun = (*SubScreen)->GetAnime();
  Entry->SubScreen = *SubScreen;
  return Entry;
}

REFIT_MENU_ITEM_OPTIONS* newREFIT_MENU_ITEM_OPTIONS(REFIT_MENU_SCREEN **SubScreen, ACTION AtClick, UINTN ID, CONST CHAR8 *Title)
{
  //create entry
//  *Entry = (__typeof_am__(*Entry))AllocateZeroPool(sizeof(LOADER_ENTRY)); // carefull, **Entry is not a LOADER_ENTRY. Don't use sizeof.
	REFIT_MENU_ITEM_OPTIONS* Entry = new REFIT_MENU_ITEM_OPTIONS();
	return NewEntry_(Entry, SubScreen, AtClick, ID, Title)->getREFIT_MENU_ITEM_OPTIONS();
//  (*Entry)->Tag = TAG_OPTIONS;
}
//
//VOID NewLoaderEntry(LOADER_ENTRY **Entry, REFIT_MENU_SCREEN **SubScreen, ACTION AtClick, UINTN ID, CONST CHAR8 *Title)
//{
//  //create entry
////  *Entry = (__typeof_am__(*Entry))AllocateZeroPool(sizeof(LOADER_ENTRY)); // carefull, **Entry is not a LOADER_ENTRY. Don't use sizeof.
//  *Entry = new LOADER_ENTRY();
//  NewEntry_(*Entry, SubScreen, AtClick, ID, Title); // cast ok because super class
//}

VOID ModifyTitles(REFIT_ABSTRACT_MENU_ENTRY *ChosenEntry)
{
  if (ChosenEntry->SubScreen->ID == SCREEN_DSDT) {
//    snwprintf((CHAR16*)ChosenEntry->Title, 128, "DSDT fix mask [0x%08X]->", gSettings.FixDsdt); // TODO jief : cast to fix
    ChosenEntry->Title.SWPrintf("DSDT fix mask [0x%08x]->", gSettings.FixDsdt); // TODO jief : cast to fix
    //MsgLog("@ESC: %ls\n", (*ChosenEntry)->Title);
  } else if (ChosenEntry->SubScreen->ID == SCREEN_CSR) {
    // CSR
//    snwprintf((CHAR16*)ChosenEntry->Title, 128, "System Integrity Protection [0x%04X]->", gSettings.CsrActiveConfig); // TODO jief : cast to fix
    ChosenEntry->Title.SWPrintf("System Integrity Protection [0x%04x]->", gSettings.CsrActiveConfig); // TODO jief : cast to fix
    // check for the right booter flag to allow the application
    // of the new System Integrity Protection configuration.
    if (gSettings.CsrActiveConfig != 0 && gSettings.BooterConfig == 0) {
      gSettings.BooterConfig = 0x28;
    }

  } else if (ChosenEntry->SubScreen->ID == SCREEN_BLC) {
//    snwprintf((CHAR16*)ChosenEntry->Title, 128, "boot_args->flags [0x%04X]->", gSettings.BooterConfig); // TODO jief : cast to fix
    ChosenEntry->Title.SWPrintf("boot_args->flags [0x%04x]->", gSettings.BooterConfig); // TODO jief : cast to fix
  } else if (ChosenEntry->SubScreen->ID == SCREEN_DSM) {
//    snwprintf((CHAR16*)ChosenEntry->Title, 128, "Drop OEM _DSM [0x%04X]->", dropDSM); // TODO jief : cast to fix
    ChosenEntry->Title.SWPrintf("Drop OEM _DSM [0x%04x]->", dropDSM); // TODO jief : cast to fix
  }
}

REFIT_ABSTRACT_MENU_ENTRY *SubMenuGraphics()
{
  UINTN  i, N, Ven = 97;
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_GRAPHICS, "Graphics Injector->");
	SubScreen->AddMenuInfoLine_f("Number of VideoCard%s=%llu",((NGFX!=1)?"s":""), NGFX);

  SubScreen->AddMenuItemInput(52, "InjectEDID", FALSE);
  SubScreen->AddMenuItemInput(53, "Fake Vendor EDID:", TRUE);
  SubScreen->AddMenuItemInput(54, "Fake Product EDID:", TRUE);
  SubScreen->AddMenuItemInput(18, "Backlight Level:", TRUE);
  SubScreen->AddMenuItemInput(112, "Intel Max Backlight:", TRUE); //gSettings.IntelMaxValue


  for (i = 0; i < NGFX; i++) {
    SubScreen->AddMenuInfo_f("----------------------");
    SubScreen->AddMenuInfo_f("Card DeviceID=%04x", gGraphics[i].DeviceID);
    N = 20 + i * 6;
    SubScreen->AddMenuItemInput(N, "Model:", TRUE);

    if (gGraphics[i].Vendor == Nvidia) {
      SubScreen->AddMenuItemInput(N+1, "InjectNVidia", FALSE);
    } else if (gGraphics[i].Vendor == Ati) {
      SubScreen->AddMenuItemInput(N+1, "InjectATI", FALSE);
    } else if (gGraphics[i].Vendor == Intel) {
      SubScreen->AddMenuItemInput(N+1, "InjectIntel", FALSE);
    } else {
      SubScreen->AddMenuItemInput(N+1, "InjectX3", FALSE);
    }

    if (gGraphics[i].Vendor == Nvidia) {
      Ven = 95;
    } else if (gGraphics[i].Vendor == Ati) {
      Ven = 94;
    } else /*if (gGraphics[i].Vendor == Intel)*/ {
      Ven = 96;
    }

    if ((gGraphics[i].Vendor == Ati) || (gGraphics[i].Vendor == Intel)) {
      SubScreen->AddMenuItemInput(109, "DualLink:", TRUE);
    }
    if (gGraphics[i].Vendor == Ati) {
      SubScreen->AddMenuItemInput(114, "DeInit:", TRUE);
    }

    SubScreen->AddMenuItemInput(Ven, "FakeID:", TRUE);

    if (gGraphics[i].Vendor == Nvidia) {
      SubScreen->AddMenuItemInput(N+2, "DisplayCFG:", TRUE);
    } else if (gGraphics[i].Vendor == Ati) {
      SubScreen->AddMenuItemInput(N+2, "FBConfig:", TRUE);
    } else /*if (gGraphics[i].Vendor == Intel)*/{
      SubScreen->AddMenuItemInput(N+2, "*-platform-id:", TRUE);
    }

    // ErmaC: NvidiaGeneric entry
    if (gGraphics[i].Vendor == Nvidia) {
      SubScreen->AddMenuItemInput(55, "Generic NVIDIA name", FALSE);
      SubScreen->AddMenuItemInput(110, "NVIDIA No EFI", FALSE);
      SubScreen->AddMenuItemInput(111, "NVIDIA Single", FALSE);
      SubScreen->AddMenuItemInput(56, "Use NVIDIA WEB drivers", FALSE);
    }

    if (gGraphics[i].Vendor == Intel) {
      continue;
    }
    SubScreen->AddMenuItemInput(N+3, "Ports:", TRUE);

    if (gGraphics[i].Vendor == Nvidia) {
      SubScreen->AddMenuItemInput(N+4, "NVCAP:", TRUE);
    } else {
      SubScreen->AddMenuItemInput(N+4, "Connectors:", TRUE);
      SubScreen->AddMenuItemInput(50, "RefCLK:", TRUE);
    }
    SubScreen->AddMenuItemInput(N+5, "Load Video Bios", FALSE);
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

// ErmaC: Audio submenu
REFIT_ABSTRACT_MENU_ENTRY *SubMenuAudio()
{

  UINTN  i;

  // init
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  // create the entry in the main menu
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_AUDIO, "Audio tuning->");

  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose options to tune the HDA devices");
	SubScreen->AddMenuInfoLine_f("Number of Audio Controller%s=%llu", ((NHDA!=1)?"s":""), NHDA);
  for (i = 0; i < NHDA; i++) {
	  SubScreen->AddMenuInfoLine_f("%llu) %ls [%04X][%04X]",
                                           (i+1),
                                           gAudios[i].controller_name,
                                           gAudios[i].controller_vendor_id,
                                           gAudios[i].controller_device_id
                      );
  }

  //SubScreen->AddMenuItemInput(59, "HDAInjection", FALSE);
  if (gSettings.HDAInjection) {
    SubScreen->AddMenuItemInput(60, "HDALayoutId:", TRUE);
  }

  // avaiable configuration
  SubScreen->AddMenuItemInput(57, "ResetHDA", FALSE);
  SubScreen->AddMenuItemInput(58, "AFGLowPowerState", FALSE);

  // return
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

#define nya(x) x/10,x%10

REFIT_ABSTRACT_MENU_ENTRY* SubMenuSpeedStep()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_CPU, "CPU tuning->");
	SubScreen->AddMenuInfoLine_f("%s", gCPUStructure.BrandString);
  SubScreen->AddMenuInfoLine_f("Model: %2X/%2X/%2X",
      gCPUStructure.Family, gCPUStructure.Model, gCPUStructure.Stepping);
  SubScreen->AddMenuInfoLine_f("Cores: %d Threads: %d",
                  gCPUStructure.Cores, gCPUStructure.Threads);
	SubScreen->AddMenuInfoLine_f("FSB speed MHz: %llu",
                  DivU64x32(gCPUStructure.FSBFrequency, Mega));
	SubScreen->AddMenuInfoLine_f("CPU speed MHz: %llu",
                  DivU64x32(gCPUStructure.CPUFrequency, Mega));
  SubScreen->AddMenuInfoLine_f("Ratio: Min=%d.%d Max=%d.%d Turbo=%d.%d/%d.%d/%d.%d/%d.%d",
     nya(gCPUStructure.MinRatio), nya(gCPUStructure.MaxRatio),
     nya(gCPUStructure.Turbo4), nya(gCPUStructure.Turbo3), nya(gCPUStructure.Turbo2), nya(gCPUStructure.Turbo1));


  SubScreen->AddMenuItemInput(76, "Cores enabled:", TRUE);
  SubScreen->AddMenuItemInput(6,  "Halt Enabler", FALSE);
  SubScreen->AddMenuItemInput(7,  "PLimitDict:", TRUE);
  SubScreen->AddMenuItemInput(8,  "UnderVoltStep:", TRUE);
  SubScreen->AddMenuItemInput(88, "DoubleFirstState", FALSE);
  SubScreen->AddMenuItemInput(5,  "GeneratePStates", FALSE);
  SubScreen->AddMenuItemInput(9,  "GenerateCStates", FALSE);
  SubScreen->AddMenuItemInput(10, "EnableC2", FALSE);
  SubScreen->AddMenuItemInput(11, "EnableC4", FALSE);
  SubScreen->AddMenuItemInput(12, "EnableC6", FALSE);
  SubScreen->AddMenuItemInput(89, "EnableC7", FALSE);
  SubScreen->AddMenuItemInput(13, "Use SystemIO", FALSE);
  SubScreen->AddMenuItemInput(75, "C3Latency:", TRUE);
  SubScreen->AddMenuItemInput(19, "BusSpeed [kHz]:", TRUE);
  SubScreen->AddMenuItemInput(14, "QPI [MHz]:", TRUE);
  SubScreen->AddMenuItemInput(77, "Saving Mode:", TRUE);
  SubScreen->AddMenuItemInput(15, "PatchAPIC", FALSE);  //-> move to ACPI?

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuKextPatches()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN    *SubScreen;
  REFIT_INPUT_DIALOG   *InputBootArgs;
  INTN                 NrKexts = gSettings.KernelAndKextPatches.NrKexts;
  KEXT_PATCH  *KextPatchesMenu = gSettings.KernelAndKextPatches.KextPatches; //zzzz
  INTN                 Index;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_KEXTS, "Custom kexts patches->");

  for (Index = 0; Index < NrKexts; Index++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.SWPrintf("%30s", KextPatchesMenu[Index].Label);
//    InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &(KextPatchesMenu[Index].MenuItem);
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuKextBlockInjection(CONST CHAR16* UniSysVer)
{
  REFIT_MENU_ITEM_OPTIONS     *Entry = NULL;
  REFIT_MENU_SCREEN    *SubScreen = NULL;
  REFIT_INPUT_DIALOG   *InputBootArgs;
  UINTN i = 0;
  SIDELOAD_KEXT        *Kext = NULL;
  CHAR8                sysVer[256];

  UnicodeStrToAsciiStrS(UniSysVer, sysVer, sizeof(sysVer));
  for (i = 0; i < sizeof(sysVer)-2; i++) {
    if (sysVer[i] == '\0') {
      sysVer[i+0] = '-';
      sysVer[i+1] = '>';
      sysVer[i+2] = '\0';
      break;
    }
  }

  Kext = InjectKextList;
  while (Kext) {
    if (StrCmp(Kext->KextDirNameUnderOEMPath, UniSysVer) == 0) {
    	if ( SubScreen == NULL ) {
    		Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_KEXT_INJECT, sysVer);
    		SubScreen->AddMenuInfoLine_f("Choose/check kext to disable:");
    	}
//      InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputBootArgs = new REFIT_INPUT_DIALOG;
      InputBootArgs->Title.SWPrintf("%ls, v.%ls", Kext->FileName, Kext->Version);
//      InputBootArgs->Tag = TAG_INPUT;
      InputBootArgs->Row = 0xFFFF; //cursor
      InputBootArgs->Item = &(Kext->MenuItem);
      InputBootArgs->AtClick = ActionEnter;
      InputBootArgs->AtRightClick = ActionDetails;
      SubScreen->AddMenuEntry(InputBootArgs, true);

      SIDELOAD_KEXT *plugInKext = Kext->PlugInList;
      while (plugInKext) {
//        InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
        InputBootArgs = new REFIT_INPUT_DIALOG;
        InputBootArgs->Title.SWPrintf("  |-- %ls, v.%ls", plugInKext->FileName, plugInKext->Version);
//        InputBootArgs->Tag = TAG_INPUT;
        InputBootArgs->Row = 0xFFFF; //cursor
        InputBootArgs->Item = &(plugInKext->MenuItem);
        InputBootArgs->AtClick = ActionEnter;
        InputBootArgs->AtRightClick = ActionDetails;
        SubScreen->AddMenuEntry(InputBootArgs, true);
        plugInKext = plugInKext->Next;
      }
    }
    Kext = Kext->Next;
  }

  if ( SubScreen != NULL ) SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

LOADER_ENTRY *SubMenuKextInjectMgmt(LOADER_ENTRY *Entry)
{
	LOADER_ENTRY       *SubEntry;
	REFIT_MENU_SCREEN  *SubScreen;
	CHAR16             *kextDir = NULL;
	UINTN               i;
	CHAR8               ShortOSVersion[8];
//	CHAR16             *UniSysVer = NULL;
	CHAR8              *ChosenOS = Entry->OSVersion;

	SubEntry = new LOADER_ENTRY();
	NewEntry_(SubEntry, &SubScreen, ActionEnter, SCREEN_SYSTEM, "Block injected kexts->");
	SubEntry->Flags = Entry->Flags;
	if (ChosenOS) {
//    DBG("chosen os %s\n", ChosenOS);
		//shorten os version 10.11.6 -> 10.11
		for (i = 0; i < 8; i++) {
			ShortOSVersion[i] = ChosenOS[i];
			if (ShortOSVersion[i] == '\0') {
				break;
			}
			if (((i > 2) && (ShortOSVersion[i] == '.')) || (i == 5)) {
				ShortOSVersion[i] = '\0';
				break;
			}
		}


		SubScreen->AddMenuInfoLine_f("Block injected kexts for target version of macOS: %s",
		                ShortOSVersion);

		// Add kext from 10
		{
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(L"10"), true);

			CHAR16 DirName[256];
			if (OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {
				snwprintf(DirName, sizeof(DirName), "10_install");
			}
			else {
				if (OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType)) {
					snwprintf(DirName, sizeof(DirName), "10_recovery");
				}
				else {
					snwprintf(DirName, sizeof(DirName), "10_normal");
				}
			}
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(DirName), true);
		}

		// Add kext from 10.{version}
		{
			CHAR16 DirName[256];
			snwprintf(DirName, sizeof(DirName), "%s", ShortOSVersion);
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(DirName), true);

			if (OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {
				snwprintf(DirName, sizeof(DirName), "%s_install", ShortOSVersion);
			}
			else {
				if (OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType)) {
					snwprintf(DirName, sizeof(DirName), "%s_recovery", ShortOSVersion);
				}
				else {
					snwprintf(DirName, sizeof(DirName), "%s_normal", ShortOSVersion);
				}
			}
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(DirName), true);
		}

		// Add kext from :
		// 10.{version}.0 if NO minor version
		// 10.{version}.{minor version} if minor version is > 0
		{
			{
				CHAR16 OSVersionKextsDirName[256];
				if ( AsciiStrCmp(ShortOSVersion, Entry->OSVersion) == 0 ) {
					snwprintf(OSVersionKextsDirName, sizeof(OSVersionKextsDirName), "%s.0", Entry->OSVersion);
				}else{
					snwprintf(OSVersionKextsDirName, sizeof(OSVersionKextsDirName), "%s", Entry->OSVersion);
				}
				SubScreen->AddMenuEntry(SubMenuKextBlockInjection(OSVersionKextsDirName), true);
			}

			CHAR16 DirName[256];
			if (OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {
				snwprintf(DirName, sizeof(DirName), "%s_install",
				        Entry->OSVersion);
			}
			else {
				if (OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType)) {
					snwprintf(DirName, sizeof(DirName), "%s_recovery",
					        Entry->OSVersion);
				}
				else {
					snwprintf(DirName, sizeof(DirName), "%s_normal",
					        Entry->OSVersion);
				}
			}
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(DirName), true);
		}
	}
	else {
		SubScreen->AddMenuInfoLine_f("Block injected kexts for target version of macOS: %s",
		                ChosenOS);
	}
	if ((kextDir = GetOtherKextsDir(TRUE)) != NULL) {
		SubScreen->AddMenuEntry(SubMenuKextBlockInjection(L"Other"), true);
		FreePool(kextDir);
	}
	if ((kextDir = GetOtherKextsDir(FALSE)) != NULL) {
		SubScreen->AddMenuEntry(SubMenuKextBlockInjection(L"Off"), true);
		FreePool(kextDir);
	}

	SubScreen->AddMenuEntry(&MenuEntryReturn, false);
	return SubEntry;
}



REFIT_ABSTRACT_MENU_ENTRY* SubMenuKernelPatches()
{
  REFIT_MENU_ITEM_OPTIONS     *Entry;
  REFIT_MENU_SCREEN    *SubScreen;
  REFIT_INPUT_DIALOG   *InputBootArgs;
  INTN                 NrKernels = gSettings.KernelAndKextPatches.NrKernels;
  KERNEL_PATCH  *KernelPatchesMenu = gSettings.KernelAndKextPatches.KernelPatches; //zzzz
  INTN                 Index;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_KERNELS, "Custom kernel patches->");

  for (Index = 0; Index < NrKernels; Index++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.SWPrintf("%30s", KernelPatchesMenu[Index].Label);
//    InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &(KernelPatchesMenu[Index].MenuItem);
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuBootPatches()
{
  REFIT_MENU_ITEM_OPTIONS     *Entry;
  REFIT_MENU_SCREEN    *SubScreen;
  REFIT_INPUT_DIALOG   *InputBootArgs;
  INTN                 NrBoots = gSettings.KernelAndKextPatches.NrBoots;
  KERNEL_PATCH  *BootPatchesMenu = gSettings.KernelAndKextPatches.BootPatches; //zzzz
  INTN                 Index;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_BOOTER, "Custom booter patches->");

  for (Index = 0; Index < NrBoots; Index++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.SWPrintf("%30s", BootPatchesMenu[Index].Label);
//    InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &(BootPatchesMenu[Index].MenuItem);
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuBinaries()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_BINARIES, "Binaries patching->");

	SubScreen->AddMenuInfoLine_f("%s", gCPUStructure.BrandString);
  SubScreen->AddMenuInfoLine_f("Real CPUID: 0x%06X", gCPUStructure.Signature);


  SubScreen->AddMenuItemInput(64,  "Debug", FALSE);
  SubScreen->AddMenuInfo_f("----------------------");
  SubScreen->AddMenuItemInput(104, "Fake CPUID:", TRUE);
//  SubScreen->AddMenuItemInput(108, "Kernel patching allowed", FALSE);
  SubScreen->AddMenuItemInput(45,  "Kernel Support CPU", FALSE);
  SubScreen->AddMenuItemInput(91,  "Kernel Lapic", FALSE);
  SubScreen->AddMenuItemInput(105, "Kernel XCPM", FALSE);
  SubScreen->AddMenuItemInput(48,  "Kernel PM", FALSE);
  SubScreen->AddMenuItemInput(121,  "Panic No Kext Dump", FALSE);
  SubScreen->AddMenuEntry(SubMenuKernelPatches(), true);
  SubScreen->AddMenuInfo_f("----------------------");
  SubScreen->AddMenuItemInput(46,  "AppleIntelCPUPM Patch", FALSE);
  SubScreen->AddMenuItemInput(47,  "AppleRTC Patch", FALSE);
  SubScreen->AddMenuItemInput(61,  "Dell SMBIOS Patch", FALSE);
//  SubScreen->AddMenuItemInput(115, "No Caches", FALSE);
//  SubScreen->AddMenuItemInput(44,  "Kext patching allowed", FALSE);
  SubScreen->AddMenuEntry(SubMenuKextPatches(), true);
  SubScreen->AddMenuInfo_f("----------------------");
  SubScreen->AddMenuEntry(SubMenuBootPatches(), true);


  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuDropTables()
{
  CHAR8               sign[5];
  CHAR8               OTID[9];
  REFIT_MENU_ITEM_OPTIONS    *Entry;
  REFIT_MENU_SCREEN   *SubScreen;
  REFIT_INPUT_DIALOG  *InputBootArgs;

  sign[4] = 0;
  OTID[8] = 0;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_TABLES, "Tables dropping->");

  if (gSettings.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
    while (DropTable) {
      CopyMem((CHAR8*)&sign, (CHAR8*)&(DropTable->Signature), 4);
      CopyMem((CHAR8*)&OTID, (CHAR8*)&(DropTable->TableId), 8);
      //MsgLog("adding to menu %s (%X) %s (%lx) L=%d(0x%X)\n",
      //       sign, DropTable->Signature,
      //       OTID, DropTable->TableId,
      //       DropTable->Length, DropTable->Length);
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
      InputBootArgs->Title.SWPrintf("Drop \"%4.4s\" \"%8.8s\" %d", sign, OTID, DropTable->Length);
//      InputBootArgs->Tag = TAG_INPUT;
      InputBootArgs->Row = 0xFFFF; //cursor
      InputBootArgs->Item = &(DropTable->MenuItem);
      InputBootArgs->AtClick = ActionEnter;
      InputBootArgs->AtRightClick = ActionDetails;
      SubScreen->AddMenuEntry(InputBootArgs, true);

      DropTable = DropTable->Next;
    }
  }

  SubScreen->AddMenuItemInput(4, "Drop all OEM SSDT", FALSE);
  SubScreen->AddMenuItemInput(113, "Automatic smart merge", FALSE);

  //SubScreen->AddMenuInfoLine_f("PATCHED AML:");
  if (ACPIPatchedAML) {
    ACPI_PATCHED_AML *ACPIPatchedAMLTmp = ACPIPatchedAML;
    while (ACPIPatchedAMLTmp) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
      InputBootArgs->Title.SWPrintf("Drop \"%ls\"", ACPIPatchedAMLTmp->FileName);
//      InputBootArgs->Tag = TAG_INPUT;
      InputBootArgs->Row = 0xFFFF; //cursor
      InputBootArgs->Item = &(ACPIPatchedAMLTmp->MenuItem);
      InputBootArgs->AtClick = ActionEnter;
      InputBootArgs->AtRightClick = ActionDetails;
      SubScreen->AddMenuEntry(InputBootArgs, true);
      ACPIPatchedAMLTmp = ACPIPatchedAMLTmp->Next;
    }
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuSmbios()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_SMBIOS, "SMBIOS->");

	SubScreen->AddMenuInfoLine_f("%s", gCPUStructure.BrandString);
	SubScreen->AddMenuInfoLine_f("%s", gSettings.OEMProduct);
	SubScreen->AddMenuInfoLine_f("with board %s", gSettings.OEMBoard);

  SubScreen->AddMenuItemInput(78,  "Product Name:", TRUE);
  SubScreen->AddMenuItemInput(79,  "Product Version:", TRUE);
  SubScreen->AddMenuItemInput(80,  "Product SN:", TRUE);
  SubScreen->AddMenuItemInput(81,  "Board ID:", TRUE);
  SubScreen->AddMenuItemInput(82,  "Board SN:", TRUE);
  SubScreen->AddMenuItemInput(83,  "Board Type:", TRUE);
  SubScreen->AddMenuItemInput(84,  "Board Version:", TRUE);
  SubScreen->AddMenuItemInput(85,  "Chassis Type:", TRUE);
  SubScreen->AddMenuItemInput(86,  "ROM Version:", TRUE);
  SubScreen->AddMenuItemInput(87,  "ROM Release Date:", TRUE);
  SubScreen->AddMenuItemInput(62,  "FirmwareFeatures:", TRUE);
  SubScreen->AddMenuItemInput(63,  "FirmwareFeaturesMask:", TRUE);
  SubScreen->AddMenuItemInput(17,  "PlatformFeature:", TRUE);
  SubScreen->AddMenuItemInput(117, "EFI Version:", TRUE);

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuDropDSM()
{
  // init
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  // create the entry in the main menu
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DSM, NULL);
  //  Entry->Title.SPrintf("Drop OEM _DSM [0x%04x]->", gSettings.DropOEM_DSM);

  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose devices to drop OEM _DSM methods from DSDT");

  SubScreen->AddMenuCheck("ATI/AMD Graphics",     DEV_ATI, 101);
  SubScreen->AddMenuCheck("Nvidia Graphics",      DEV_NVIDIA, 101);
  SubScreen->AddMenuCheck("Intel Graphics",       DEV_INTEL, 101);
  SubScreen->AddMenuCheck("PCI HDA audio",        DEV_HDA, 101);
  SubScreen->AddMenuCheck("HDMI audio",           DEV_HDMI, 101);
  SubScreen->AddMenuCheck("PCI LAN Adapter",      DEV_LAN, 101);
  SubScreen->AddMenuCheck("PCI WiFi Adapter",     DEV_WIFI, 101);
  SubScreen->AddMenuCheck("IDE HDD",              DEV_IDE, 101);
  SubScreen->AddMenuCheck("SATA HDD",             DEV_SATA, 101);
  SubScreen->AddMenuCheck("USB Controllers",      DEV_USB, 101);
  SubScreen->AddMenuCheck("LPC Controller",       DEV_LPC, 101);
  SubScreen->AddMenuCheck("SMBUS Controller",     DEV_SMBUS, 101);
  SubScreen->AddMenuCheck("IMEI Device",          DEV_IMEI, 101);
  SubScreen->AddMenuCheck("Firewire",             DEV_FIREWIRE, 101);

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  ModifyTitles(Entry);

  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuDsdtFix()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN  *SubScreen;
//  REFIT_INPUT_DIALOG *InputBootArgs;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DSDT, NULL);
  //  Entry->Title.SPrintf("DSDT fix mask [0x%08x]->", gSettings.FixDsdt);

  SubScreen->AddMenuCheck("Add DTGP",     FIX_DTGP, 67);
  SubScreen->AddMenuCheck("Fix Darwin as WinXP",   FIX_WARNING, 67);
  SubScreen->AddMenuCheck("Fix Darwin as Win7",   FIX_DARWIN, 67);
  SubScreen->AddMenuCheck("Fix shutdown", FIX_SHUTDOWN, 67);
  SubScreen->AddMenuCheck("Add MCHC",     FIX_MCHC, 67);
  SubScreen->AddMenuCheck("Fix HPET",     FIX_HPET, 67);
  SubScreen->AddMenuCheck("Fake LPC",     FIX_LPC, 67);
  SubScreen->AddMenuCheck("Fix IPIC",     FIX_IPIC, 67);
  SubScreen->AddMenuCheck("Add SMBUS",    FIX_SBUS, 67);
  SubScreen->AddMenuCheck("Fix display",  FIX_DISPLAY, 67);
  SubScreen->AddMenuCheck("Fix IDE",      FIX_IDE, 67);
  SubScreen->AddMenuCheck("Fix SATA",     FIX_SATA, 67);
  SubScreen->AddMenuCheck("Fix Firewire", FIX_FIREWIRE, 67);
  SubScreen->AddMenuCheck("Fix USB",      FIX_USB, 67);
  SubScreen->AddMenuCheck("Fix LAN",      FIX_LAN, 67);
  SubScreen->AddMenuCheck("Fix Airport",  FIX_WIFI, 67);
  SubScreen->AddMenuCheck("Fix sound",    FIX_HDA, 67);
//  SubScreen->AddMenuCheck("Fix new way",  FIX_NEW_WAY, 67);
  SubScreen->AddMenuCheck("Fix RTC",      FIX_RTC, 67);
  SubScreen->AddMenuCheck("Fix TMR",      FIX_TMR, 67);
  SubScreen->AddMenuCheck("Add IMEI",     FIX_IMEI, 67);
  SubScreen->AddMenuCheck("Fix IntelGFX", FIX_INTELGFX, 67);
  SubScreen->AddMenuCheck("Fix _WAK",     FIX_WAK, 67);
  SubScreen->AddMenuCheck("Del unused",   FIX_UNUSED, 67);
  SubScreen->AddMenuCheck("Fix ADP1",     FIX_ADP1, 67);
  SubScreen->AddMenuCheck("Add PNLF",     FIX_PNLF, 67);
  SubScreen->AddMenuCheck("Fix S3D",      FIX_S3D, 67);
  SubScreen->AddMenuCheck("Rename ACST",  FIX_ACST, 67);
  SubScreen->AddMenuCheck("Add HDMI",     FIX_HDMI, 67);
  SubScreen->AddMenuCheck("Fix Regions",  FIX_REGIONS, 67);
  SubScreen->AddMenuCheck("Fix Headers",  FIX_HEADERS, 67);
  SubScreen->AddMenuCheck("Fix Mutex",    FIX_MUTEX, 67);

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  ModifyTitles(Entry);

  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuDSDTPatches()  //yyyy
{
  REFIT_MENU_ITEM_OPTIONS     *Entry;
  REFIT_MENU_SCREEN    *SubScreen;
  REFIT_INPUT_DIALOG   *InputBootArgs;

  INTN             PatchDsdtNum = gSettings.PatchDsdtNum;
  INPUT_ITEM   *DSDTPatchesMenu = gSettings.PatchDsdtMenuItem;
  INTN                 Index;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DSDT_PATCHES, "Custom DSDT patches->");

  for (Index = 0; Index < PatchDsdtNum; Index++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.takeValueFrom(gSettings.PatchDsdtLabel[Index]);
//    InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &DSDTPatchesMenu[Index];
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuDsdts()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_MENU_SWITCH *InputBootArgs;
  UINTN               i;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_ACPI, "Dsdt name->");

  SubScreen->AddMenuInfoLine_f("Select a DSDT file:");
  SubScreen->AddMenuItemSwitch(116,  "BIOS.aml", FALSE);

  for (i = 0; i < DsdtsNum; i++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title.takeValueFrom(DsdtsList[i]);
//    InputBootArgs->Tag = TAG_SWITCH_OLD;
    InputBootArgs->Row = i + 1;
    InputBootArgs->Item = &InputItems[116];
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}


REFIT_ABSTRACT_MENU_ENTRY* SubMenuACPI()
{
  // init
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  // create the entry in the options menu
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_ACPI, "ACPI patching->");

  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose options to patch ACPI");

  SubScreen->AddMenuItemInput(102, "Debug DSDT", FALSE);

  SubScreen->AddMenuEntry(SubMenuDsdts(), true);
  SubScreen->AddMenuEntry(SubMenuDropTables(), true);
  SubScreen->AddMenuEntry(SubMenuDropDSM(), true);
  SubScreen->AddMenuEntry(SubMenuDsdtFix(), true);
  SubScreen->AddMenuEntry(SubMenuDSDTPatches(), true);
  SubScreen->AddMenuItemInput(49, "Fix MCFG", FALSE);

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuAudioPort()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_MENU_SWITCH *InputBootArgs;
  UINTN               i;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_AUDIOPORTS, "Startup sound output->");

  SubScreen->AddMenuInfoLine_f("Select an audio output, press F7 to test");
  SubScreen->AddMenuItemInput(120, "Volume:", TRUE);

  for (i = 0; i < AudioNum; i++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title.SWPrintf("%ls_%s", AudioList[i].Name, AudioOutputNames[AudioList[i].Device]);
//    InputBootArgs->Tag = TAG_SWITCH_OLD;
    InputBootArgs->Row = i;
    InputBootArgs->Item = &InputItems[119];
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

VOID CreateMenuProps(REFIT_MENU_SCREEN   *SubScreen, DEV_PROPERTY *Prop)
{
	REFIT_INPUT_DIALOG  *InputBootArgs;

//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
	InputBootArgs->Title.SWPrintf("  key: %s", Prop->Key);
//	InputBootArgs->Tag = TAG_INPUT;
	InputBootArgs->Row = 0xFFFF; //cursor
									   //     InputBootArgs->Item = ADDRESS_OF(DEV_PROPERTY, Prop, INPUT_ITEM, MenuItem);
	InputBootArgs->Item = &Prop->MenuItem;
	InputBootArgs->AtClick = ActionEnter;
	InputBootArgs->AtRightClick = ActionDetails;
	SubScreen->AddMenuEntry(InputBootArgs, true);
	switch (Prop->ValueType) {
	case kTagTypeInteger:
			SubScreen->AddMenuInfo_f("     value: 0x%08llx", *(UINT64*)Prop->Value);
		break;
	case kTagTypeString:
			SubScreen->AddMenuInfo_f("     value: %30s", Prop->Value);
		break;
	case   kTagTypeFalse:
		SubScreen->AddMenuInfo_f(("     value: false"));
		break;
	case   kTagTypeTrue:
		SubScreen->AddMenuInfo_f(("     value: true"));
		break;

	default: //type data, print first 24 bytes
			 //CHAR8* Bytes2HexStr(UINT8 *data, UINTN len)
			SubScreen->AddMenuInfo_f("     value[%llu]: %24s", Prop->ValueLen, Bytes2HexStr((UINT8*)Prop->Value, MIN(24, Prop->ValueLen)));
		break;
	}

}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuCustomDevices()
{
  REFIT_MENU_ITEM_OPTIONS    *Entry;
  REFIT_MENU_SCREEN   *SubScreen;

  UINT32              DevAddr, OldDevAddr = 0;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DEVICES, "Custom properties->");

  if (gSettings.ArbProperties) {
    DEV_PROPERTY *Prop = gSettings.ArbProperties;
	if (Prop && (Prop->Device == 0))
	{
		DEV_PROPERTY *Props = NULL;
		while (Prop) {
			SubScreen->AddMenuInfo_f("------------");
			SubScreen->AddMenuInfo_f("%s", Prop->Label);
			Props = Prop->Child;
			while (Props) {
				CreateMenuProps(SubScreen, Props);
				Props = Props->Next;
			}
			Prop = Prop->Next;
		}
	}
    while (Prop) {
      DevAddr = Prop->Device;
      if (DevAddr != 0 && DevAddr != OldDevAddr) {
        OldDevAddr = DevAddr;
        SubScreen->AddMenuInfo_f("------------");
		  SubScreen->AddMenuInfo_f("%s", Prop->Label);
        CreateMenuProps(SubScreen, Prop);
      }
      Prop = Prop->Next;
    }
  }
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  Entry->SubScreen = SubScreen;
  return Entry;
}


REFIT_ABSTRACT_MENU_ENTRY* SubMenuPCI()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_USB, "PCI devices->");

  SubScreen->AddMenuItemInput(74,  "USB Ownership", FALSE);
  SubScreen->AddMenuItemInput(92,  "USB Injection", FALSE);
  SubScreen->AddMenuItemInput(93,  "Inject ClockID", FALSE);
  SubScreen->AddMenuItemInput(106, "Inject EFI Strings", FALSE);
  SubScreen->AddMenuItemInput(107, "No Default Properties", FALSE);
  SubScreen->AddMenuItemInput(97,  "FakeID LAN:", TRUE);
  SubScreen->AddMenuItemInput(98,  "FakeID WIFI:", TRUE);
  SubScreen->AddMenuItemInput(99,  "FakeID SATA:", TRUE);
  SubScreen->AddMenuItemInput(100, "FakeID XHCI:", TRUE);
  SubScreen->AddMenuItemInput(103, "FakeID IMEI:", TRUE);
  SubScreen->AddMenuEntry(SubMenuCustomDevices(), true);

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  Entry->SubScreen = SubScreen;
  return Entry;
}


REFIT_ABSTRACT_MENU_ENTRY* SubMenuThemes()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_MENU_SWITCH *InputBootArgs;
  UINTN               i;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_THEME, "Themes->");

  SubScreen->AddMenuInfoLine_f("Installed themes:");
  //add embedded
  SubScreen->AddMenuItemSwitch(3,  "embedded", FALSE);

  for (i = 0; i < ThemesNum; i++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title.takeValueFrom(ThemesList[i]);
//    InputBootArgs->Tag = TAG_SWITCH_OLD;
    InputBootArgs->Row = i + 1;
    InputBootArgs->Item = &InputItems[3];
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuGUI()
{
  // init
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  // create the entry in the options menu
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_GUI, "GUI tuning->");

  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose options to tune the Interface");

  SubScreen->AddMenuItemInput(70, "Pointer Speed:", TRUE);
  SubScreen->AddMenuItemInput(72, "Mirror Move", FALSE);

  SubScreen->AddMenuEntry(SubMenuThemes(), true);

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}


/*
 * This is a simple and user friendly submenu which makes it possible to modify
 * the System Integrity Protection configuration from the Clover's GUI.
 * Author: Needy.
 * The below function is based on the SubMenuDsdtFix function.
 */
REFIT_ABSTRACT_MENU_ENTRY* SubMenuCSR()
{
  // init
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  // create the entry in the main menu
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_CSR, NULL);

  // submenu description
  SubScreen->AddMenuInfoLine_f("Modify the System Integrity Protection configuration.");
  SubScreen->AddMenuInfoLine_f("All configuration changes apply to the entire machine.");

  // available configurations
  SubScreen->AddMenuCheck("Allow Untrusted Kexts", CSR_ALLOW_UNTRUSTED_KEXTS, 66);
  SubScreen->AddMenuCheck("Allow Unrestricted FS", CSR_ALLOW_UNRESTRICTED_FS, 66);
  SubScreen->AddMenuCheck("Allow Task For PID", CSR_ALLOW_TASK_FOR_PID, 66);
  SubScreen->AddMenuCheck("Allow Kernel Debuger", CSR_ALLOW_KERNEL_DEBUGGER, 66);
  SubScreen->AddMenuCheck("Allow Apple Internal", CSR_ALLOW_APPLE_INTERNAL, 66);
  SubScreen->AddMenuCheck("Allow Unrestricted DTrace", CSR_ALLOW_UNRESTRICTED_DTRACE, 66);
  SubScreen->AddMenuCheck("Allow Unrestricted NVRAM", CSR_ALLOW_UNRESTRICTED_NVRAM, 66);
  SubScreen->AddMenuCheck("Allow Device Configuration", CSR_ALLOW_DEVICE_CONFIGURATION, 66);
  SubScreen->AddMenuCheck("Allow Any Recovery OS", CSR_ALLOW_ANY_RECOVERY_OS, 66);
  SubScreen->AddMenuCheck("Allow Unapproved Kexts", CSR_ALLOW_UNAPPROVED_KEXTS, 66);

  // return
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  ModifyTitles(Entry);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuBLC()
{
  // init
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  // create the entry in the main menu
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_BLC, NULL);
//  Entry->Title.SPrintf("boot_args->flags [0x%02x]->", gSettings.BooterConfig);

  // submenu description
  SubScreen->AddMenuInfoLine_f("Modify flags for boot.efi");

  SubScreen->AddMenuCheck("Reboot On Panic",    kBootArgsFlagRebootOnPanic, 65);
  SubScreen->AddMenuCheck("Hi DPI",             kBootArgsFlagHiDPI, 65);
  SubScreen->AddMenuCheck("Black Screen",       kBootArgsFlagBlack, 65);
  SubScreen->AddMenuCheck("CSR Active Config",  kBootArgsFlagCSRActiveConfig, 65);
  SubScreen->AddMenuCheck("CSR Pending Config", kBootArgsFlagCSRPendingConfig, 65);
  SubScreen->AddMenuCheck("CSR Boot",           kBootArgsFlagCSRBoot, 65);
  SubScreen->AddMenuCheck("Black Background",   kBootArgsFlagBlackBg, 65);
  SubScreen->AddMenuCheck("Login UI",           kBootArgsFlagLoginUI, 65);
  SubScreen->AddMenuCheck("Install UI",         kBootArgsFlagInstallUI, 65);

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  ModifyTitles(Entry);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuSystem()
{
  // init
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  // create the entry in the options menu
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_SYSTEM, "System Parameters->");

  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose options for booted OS");

  SubScreen->AddMenuItemInput(2,  "Block kext:", TRUE);
  SubScreen->AddMenuItemInput(51, "Set OS version if not:", TRUE);
  SubScreen->AddMenuItemInput(118, "Booter Cfg Command:", TRUE);

  SubScreen->AddMenuEntry(SubMenuCSR(), true);
  SubScreen->AddMenuEntry(SubMenuBLC(), true);

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuConfigs()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_MENU_SWITCH *InputBootArgs;
  UINTN               i;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_THEME, "Configs->");

  SubScreen->AddMenuInfoLine_f("Select a config file:");

  for (i = 0; i < ConfigsNum; i++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title.takeValueFrom(ConfigsList[i]);
//    InputBootArgs->Tag = TAG_SWITCH_OLD;
    InputBootArgs->Row = i;
    InputBootArgs->Item = &InputItems[90];
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

VOID  OptionsMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry)
{
  REFIT_ABSTRACT_MENU_ENTRY    *TmpChosenEntry = NULL;
  REFIT_ABSTRACT_MENU_ENTRY    *NextChosenEntry = NULL;
  UINTN               MenuExit = 0;
  UINTN               SubMenuExit;
  UINTN               NextMenuExit;
  //CHAR16*           Flags;

  MENU_STYLE_FUNC     Style = &REFIT_MENU_SCREEN::TextMenuStyle;

  INTN                EntryIndex = 0;
  INTN                SubEntryIndex = -1; //value -1 means old position to remember
  INTN                NextEntryIndex = -1;

  //  REFIT_INPUT_DIALOG* InputBootArgs;
#if USE_XTHEME
  BOOLEAN             OldFontStyle = ThemeX.Proportional;
  ThemeX.Proportional = FALSE; //temporary disable proportional
#else
  BOOLEAN             OldFontStyle = GlobalConfig.Proportional;
  GlobalConfig.Proportional = FALSE; //temporary disable proportional
#endif


  if (AllowGraphicsMode) {
    Style = &REFIT_MENU_SCREEN::GraphicsMenuStyle;
  }

  // remember, if you extended this menu then change procedures
  // FillInputs and ApplyInputs
#if USE_XTHEME
  if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    OptionMenu.TitleImage = ThemeX.GetIcon(BUILTIN_ICON_FUNC_OPTIONS);
  } else {
    OptionMenu.TitleImage.setEmpty();
  }
#else
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    OptionMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_OPTIONS);
  } else {
    OptionMenu.TitleImage = NULL;
  }
#endif

  gThemeOptionsChanged = FALSE;

  if (OptionMenu.Entries.size() == 0) {
    gThemeOptionsChanged = TRUE;
    OptionMenu.ID = SCREEN_OPTIONS;
    OptionMenu.AnimeRun = OptionMenu.GetAnime(); //FALSE;

    OptionMenu.AddMenuItemInput(0, "Boot Args:", TRUE);

//    AddMenuItemInput(&OptionMenu, 90, "Config:", TRUE);
//   InputBootArgs->ShortcutDigit = 0xF1;
    OptionMenu.AddMenuEntry( SubMenuConfigs(), true);

    if (AllowGraphicsMode) {
      OptionMenu.AddMenuEntry( SubMenuGUI(), true);
    }
    OptionMenu.AddMenuEntry( SubMenuACPI(), true);
    OptionMenu.AddMenuEntry( SubMenuSmbios(), true);
    OptionMenu.AddMenuEntry( SubMenuPCI(), true);
    OptionMenu.AddMenuEntry( SubMenuSpeedStep(), true);
    OptionMenu.AddMenuEntry( SubMenuGraphics(), true);
    OptionMenu.AddMenuEntry( SubMenuAudio(), true);
    OptionMenu.AddMenuEntry( SubMenuAudioPort(), true);
    OptionMenu.AddMenuEntry( SubMenuBinaries(), true);
    OptionMenu.AddMenuEntry( SubMenuSystem(), true);
    OptionMenu.AddMenuEntry( &MenuEntryReturn, false);
    //DBG("option menu created entries=%d\n", OptionMenu.Entries.size());
  }

  while (!MenuExit) {
    MenuExit = OptionMenu.RunGenericMenu(Style, &EntryIndex, ChosenEntry);
    //    MenuExit = RunMenu(&OptionMenu, ChosenEntry);
    if (  MenuExit == MENU_EXIT_ESCAPE || (*ChosenEntry)->getREFIT_MENU_ITEM_RETURN()  )
      break;
    if (MenuExit == MENU_EXIT_ENTER || MenuExit == MENU_EXIT_DETAILS) {
      //enter input dialog or subscreen
      if ((*ChosenEntry)->SubScreen != NULL) {
        SubMenuExit = 0;
        while (!SubMenuExit) {

          SubMenuExit = (*ChosenEntry)->SubScreen->RunGenericMenu(Style, &SubEntryIndex, &TmpChosenEntry);
          if (SubMenuExit == MENU_EXIT_ESCAPE || TmpChosenEntry->getREFIT_MENU_ITEM_RETURN()  ){
            ApplyInputs();
            ModifyTitles(*ChosenEntry);
            break;
          }
          if (SubMenuExit == MENU_EXIT_ENTER || MenuExit == MENU_EXIT_DETAILS) {
            if (TmpChosenEntry->SubScreen != NULL) {
              NextMenuExit = 0;
              while (!NextMenuExit) {

                NextMenuExit = TmpChosenEntry->SubScreen->RunGenericMenu(Style, &NextEntryIndex, &NextChosenEntry);
                if (NextMenuExit == MENU_EXIT_ESCAPE || NextChosenEntry->getREFIT_MENU_ITEM_RETURN()  ){
                  ApplyInputs();
                  ModifyTitles(TmpChosenEntry);
                  break;
                }
                if (NextMenuExit == MENU_EXIT_ENTER || MenuExit == MENU_EXIT_DETAILS) {
                  // enter input dialog
                  NextMenuExit = 0;
                  ApplyInputs();
                  ModifyTitles(TmpChosenEntry);
                }
              } //while(!NextMenuExit)
            }
            // enter input dialog
            SubMenuExit = 0;
            ApplyInputs();
            ModifyTitles(TmpChosenEntry);
          }
        } //while(!SubMenuExit)
      }
      MenuExit = 0;
    } // if MENU_EXIT_ENTER
  }
//exit:
#if USE_XTHEME
  ThemeX.Proportional = OldFontStyle;
#else
  GlobalConfig.Proportional = OldFontStyle;
#endif


  ApplyInputs();
}

//UINT32 EncodeOptions(CONST CHAR16 *Options)
//{
//  UINT32 OptionsBits = 0;
//  INTN Index;
//  if (!Options) {
//    return 0;
//  }
//  for (Index = 0; Index < NUM_OPT; Index++) {
//    if (StrStr(Options, ArgOptional[Index])) {
//      OptionsBits |= (1 << Index);
//      if (Index == 1) {
//        OptionsBits &= ~1;
//      }
//    }
//  }
//  return OptionsBits;
//}
//
//VOID DecodeOptions(REFIT_MENU_ITEM_BOOTNUM *Entry)
//{
//  //set checked option
//  INTN Index;
//  if (!Entry) {
//    return;
//  }
//  for (Index = 0; Index < INX_NVWEBON; Index++) { //not including INX_NVWEBON
//    if (gSettings.OptionsBits & (1 << Index)) {
//      Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, ArgOptional[Index]);
//    }
//  }
//  //remove unchecked options
//  for (Index = 0; Index < INX_NVWEBON; Index++) { //not including INX_NVWEBON
//    if ((gSettings.OptionsBits & (1 << Index)) == 0) {
//      Entry->LoadOptions = RemoveLoadOption(Entry->LoadOptions, ArgOptional[Index]);
//    }
//  }
//
//  if (Entry->getLOADER_ENTRY()) {
//    LOADER_ENTRY* loaderEntry = Entry->getLOADER_ENTRY();
//    // Only for non-legacy entries, as LEGACY_ENTRY doesn't have OSVersion
//    if (gSettings.OptionsBits & OPT_NVWEBON) {
//      if (AsciiOSVersionToUint64(loaderEntry->OSVersion) >= AsciiOSVersionToUint64("10.12")) {
//        gSettings.NvidiaWeb = TRUE;
//      } else {
//        Entry->LoadOptions = AddLoadOption(loaderEntry->LoadOptions, ArgOptional[INX_NVWEBON]);
//      }
//    }
//    if ((gSettings.OptionsBits & OPT_NVWEBON) == 0) {
//      if (AsciiOSVersionToUint64(loaderEntry->OSVersion) >= AsciiOSVersionToUint64("10.12")) {
//        gSettings.NvidiaWeb = FALSE;
//      } else {
//        Entry->LoadOptions = RemoveLoadOption(loaderEntry->LoadOptions, ArgOptional[INX_NVWEBON]);
//      }
//    }
//  }
//}
//

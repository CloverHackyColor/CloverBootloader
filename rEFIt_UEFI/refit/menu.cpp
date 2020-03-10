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
//#include "../include/scroll_images.h"

#include "../../Version.h"
//#include "colors.h"

#include "nanosvg.h"
#include "FloatLib.h"
#include "HdaCodecDump.h"
#include "menu.h"
#include "screen.h"

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
CONST CHAR16 *VBIOS_BIN = L"EFI\\CLOVER\\misc\\c0000.bin";

//#define LSTR(s) L##s

// scrolling definitions
static INTN MaxItemOnScreen = -1;
//REFIT_MENU_SCREEN OptionMenu(4, L"Options", NULL, /*InfoLineCount*/0, NULL, 0, NULL, NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL );
REFIT_MENU_SCREEN OptionMenu(4, L"Options", NULL);
extern REFIT_MENU_ITEM_RETURN MenuEntryReturn;
extern UINTN            ThemesNum;
extern CHAR16           *ThemesList[];
extern UINTN            ConfigsNum;
extern CHAR16           *ConfigsList[];
extern UINTN            DsdtsNum;
extern CHAR16           *DsdtsList[];
extern UINTN            AudioNum;
extern HDA_OUTPUTS      AudioList[20];
extern CONST CHAR8            *AudioOutputNames[];
extern CHAR8            NonDetected[];
extern BOOLEAN          GetLegacyLanAddress;
extern UINT8            gLanMac[4][6]; // their MAC addresses
extern EFI_AUDIO_IO_PROTOCOL *AudioIo;

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
#define TEXT_CORNER_OPTIMUS   (3)

#define TITLE_MAX_LEN (SVALUE_MAX_SIZE / sizeof(CHAR16) + 128)

// other menu definitions

#define MENU_FUNCTION_INIT            (0)
#define MENU_FUNCTION_CLEANUP         (1)
#define MENU_FUNCTION_PAINT_ALL       (2)
#define MENU_FUNCTION_PAINT_SELECTION (3)
#define MENU_FUNCTION_PAINT_TIMEOUT   (4)



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

// clovy - set row height based on text size
#define RowHeightFromTextHeight (1.35f)

#define TITLEICON_SPACING (16)

//#define ROW0__TILESIZE (144)
//#define ROW1_TILESIZE (64)
#define TILE1_XSPACING (8)
//#define TILE_YSPACING (24)
#define ROW0_SCROLLSIZE (100)
#define INDICATOR_SIZE (52)

//EG_IMAGE *SelectionImages[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
//EG_IMAGE *Buttons[4] = {NULL, NULL, NULL, NULL};
static EG_IMAGE *TextBuffer = NULL;

//EG_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff }; //non-trasparent

//INTN row0TileSize = 144;
//INTN row1TileSize = 64;

static INTN row0Count, row0PosX, row0PosXRunning;
static INTN row1Count, row1PosX, row1PosXRunning;
static INTN *itemPosX = NULL;
static INTN *itemPosY = NULL;
static INTN row0PosY, row1PosY, textPosY, FunctextPosY;
//static EG_IMAGE* MainImage;
static INTN OldX = 0, OldY = 0;
static INTN OldTextWidth = 0;
static UINTN OldRow = 0;
static INTN OldTimeoutTextWidth = 0;
static INTN MenuWidth, TimeoutPosY;
static INTN EntriesPosX, EntriesPosY;
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
INTN OldChosenConfig;
INTN OldChosenDsdt;
UINTN OldChosenAudio;
UINT8 DefaultAudioVolume = 70;
//INTN NewChosenTheme;
INTN TextStyle;

BOOLEAN mGuiReady = FALSE;



//REFIT_MENU_ITEM_OPTIONS(CONST CHAR16 *Title_, UINTN Row_, CHAR16 ShortcutDigit_, CHAR16 ShortcutLetter_, ACTION AtClick_)
REFIT_MENU_ITEM_OPTIONS  MenuEntryOptions (L"Options",          1, 0, 'O', ActionEnter);
REFIT_MENU_ITEM_ABOUT    MenuEntryAbout   (L"About Clover",     1, 0, 'A', ActionEnter);
REFIT_MENU_ITEM_RESET    MenuEntryReset   (L"Restart Computer", 1, 0, 'R', ActionSelect);
REFIT_MENU_ITEM_SHUTDOWN MenuEntryShutdown(L"Exit Clover",      1, 0, 'U', ActionSelect);
REFIT_MENU_ITEM_RETURN   MenuEntryReturn  (L"Return",           0, 0,  0,  ActionEnter);



REFIT_MENU_SCREEN MainMenu(1, L"Main Menu", L"Automatic boot");
REFIT_MENU_SCREEN AboutMenu(2, L"About", NULL);
REFIT_MENU_SCREEN HelpMenu(3, L"Help", NULL);

CONST CHAR16* ArgOptional[NUM_OPT] = {
  L"arch=i386",       //0
  L"arch=x86_64",     //1
  L"-v ",             //2
  L"-f ",             //3
  L"-s ",             //4
  L"-x ",             //5
  L"nv_disable=1",    //6
  L"slide=0",         //7
  L"darkwake=0",      //8
  L"-xcpm",           //9
  L"-gux_no_idle",    //10
  L"-gux_nosleep",    //11
  L"-gux_nomsi",      //12
  L"-gux_defer_usb2", //13
  L"keepsyms=1",      //14
  L"debug=0x100",     //15
  L"kextlog=0xffff",  //16
  L"-alcoff",         //17
  L"-shikioff",       //18
  L"nvda_drv=1"       //19
};

//UINTN RunGenericMenu(IN REFIT_MENU_SCREEN *Screen, IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry);

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

  InputItemsCount = 0;
  if (New) {
    InputItems = (__typeof__(InputItems))AllocateZeroPool(130 * sizeof(INPUT_ITEM)); //XXX
  }

  InputItems[InputItemsCount].ItemType = ASString;  //0
  //even though Ascii we will keep value as Unicode to convert later
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(SVALUE_MAX_SIZE);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, SVALUE_MAX_SIZE, L"%a ", gSettings.BootArgs);
  InputItems[InputItemsCount].ItemType = UNIString; //1
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(32);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 32, L"%s", gSettings.DsdtName); // 1-> 2
  InputItems[InputItemsCount].ItemType = UNIString; //2
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(63);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 63, L"%s", gSettings.BlockKexts);

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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 8, L"%02d", gSettings.PLimitDict);
  InputItems[InputItemsCount].ItemType = Decimal;  //8
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(8);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 8, L"%02d", gSettings.UnderVoltStep);
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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%06d", gSettings.QPI);
  InputItems[InputItemsCount].ItemType = BoolValue; //15
  InputItems[InputItemsCount++].BValue = gSettings.PatchNMI;
  InputItems[InputItemsCount].ItemType = BoolValue; //16
  InputItems[InputItemsCount++].BValue = gSettings.PatchVBios;
  InputItems[InputItemsCount].ItemType = Decimal;  //17
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(20);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"0x%x", gPlatformFeature);
  InputItems[InputItemsCount].ItemType = Hex;  //18
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(36);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 36, L"0x%X", gSettings.BacklightLevel);
  InputItems[InputItemsCount].ItemType = Decimal;  //19
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  if (gSettings.BusSpeed > 20000) {
    UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%06d", gSettings.BusSpeed);
  } else {
    UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%06d", gCPUStructure.ExternalClock);
  }
  InputItemsCount = 20;
  for (i=0; i<NGFX; i++) {
    InputItems[InputItemsCount].ItemType = ASString;  //20+i*6
    if (New) {
      InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
    }
    UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gGraphics[i].Model);

    if (gGraphics[i].Vendor == Ati) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.InjectATI;
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(20);
      }
      if (StrLen(gSettings.FBName) > 2) { //fool proof: cfg_name is 3 character or more.
        UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%s", gSettings.FBName);
      } else {
        UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%a", gGraphics[i].Config);
      }
    } else if (gGraphics[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.InjectNVidia;
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      for (j=0; j<8; j++) {
        AsciiSPrint((CHAR8*)&tmp[2*j], 3, "%02x", gSettings.Dcfg[j]);
      }
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(40);
      }
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 40, L"%a", tmp);

      //InputItems[InputItemsCount++].SValue = PoolPrint(L"%08x",*(UINT64*)&gSettings.Dcfg[0]);
    } else /*if (gGraphics[i].Vendor == Intel) */ {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.InjectIntel;
      InputItems[InputItemsCount].ItemType = Hex; //22+6i
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(20);
      }
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.IgPlatform);
 //     InputItemsCount += 3;
 //     continue;
    }

    InputItems[InputItemsCount].ItemType = Decimal;  //23+6i
    if (New) {
      InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(8);
    }
    if (gSettings.VideoPorts > 0) {
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 8, L"%02d", gSettings.VideoPorts);
    } else {
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 8, L"%02d", gGraphics[i].Ports);
    }

    if (gGraphics[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = ASString; //24+6i
      for (j=0; j<20; j++) {
        AsciiSPrint((CHAR8*)&tmp[2*j], 3, "%02x", gSettings.NVCAP[j]);
      }
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(84);
      }
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 84, L"%a", tmp);
    } else { //ATI and others there will be connectors
      InputItems[InputItemsCount].ItemType = Hex; //24+6i
      if (New) {
        InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(20);
      }
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%08lx", gGraphics[i].Connectors);
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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%06d", gSettings.RefCLK);

  InputItems[InputItemsCount].ItemType = ASString;  //51 OS version if non-detected
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(SVALUE_MAX_SIZE);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, SVALUE_MAX_SIZE, L"%a ", NonDetected);

  InputItems[InputItemsCount].ItemType = BoolValue; //52
  InputItems[InputItemsCount++].BValue = gSettings.InjectEDID;

  //VendorEDID & ProductEDID 53, 54
  InputItems[InputItemsCount].ItemType = Decimal;  //53
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"0x%04x", gSettings.VendorEDID);
  InputItems[InputItemsCount].ItemType = Decimal;  //54
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"0x%04x", gSettings.ProductEDID);

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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.HDALayoutId);

  // syscl change here
  InputItems[InputItemsCount].ItemType = BoolValue; //61
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPDELLSMBIOS;
  // end of change

  InputItems[InputItemsCount].ItemType = Hex;  //62
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(24);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 24, L"0x%08x", gFwFeatures);

  InputItems[InputItemsCount].ItemType = Hex;  //63
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(24);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 24, L"0x%08x", gFwFeaturesMask);

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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 8, L"%02d", gSettings.PointerSpeed);
  InputItems[InputItemsCount].ItemType = Decimal;  //71
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%04d", gSettings.DoubleClickTime);
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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"0x%04x", gSettings.C3Latency);
  InputItems[InputItemsCount].ItemType = Decimal;  //76
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%02d", gSettings.EnabledCores);
  InputItems[InputItemsCount].ItemType = Decimal;  //77
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%02d", gSettings.SavingMode);

  InputItems[InputItemsCount].ItemType = ASString;  //78
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.ProductName);
  InputItems[InputItemsCount].ItemType = ASString;  //79
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.VersionNr);
  InputItems[InputItemsCount].ItemType = ASString;  //80
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.SerialNr);
  InputItems[InputItemsCount].ItemType = ASString;  //81
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardNumber);
  InputItems[InputItemsCount].ItemType = ASString;  //82
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardSerialNumber);
  InputItems[InputItemsCount].ItemType = Decimal;  //83
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.BoardType);
  InputItems[InputItemsCount].ItemType = ASString;  //84
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardVersion);
  InputItems[InputItemsCount].ItemType = Decimal;  //85
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.ChassisType);
  InputItems[InputItemsCount].ItemType = ASString;  //86
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.RomVersion);
  InputItems[InputItemsCount].ItemType = ASString;  //87
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.ReleaseDate);

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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeATI);
  InputItems[InputItemsCount].ItemType = Hex;  //95
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeNVidia);
  InputItems[InputItemsCount].ItemType = Hex;  //96
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeIntel);

  InputItems[InputItemsCount].ItemType = Hex;  //97
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeLAN);
  InputItems[InputItemsCount].ItemType = Hex;  //98
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeWIFI);
  InputItems[InputItemsCount].ItemType = Hex;  //99
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeSATA);
  InputItems[InputItemsCount].ItemType = Hex;  //100
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeXHCI);
  InputItems[InputItemsCount].ItemType = CheckBit;  //101
  InputItems[InputItemsCount++].IValue = dropDSM;

  InputItems[InputItemsCount].ItemType = BoolValue; //102
  InputItems[InputItemsCount++].BValue = gSettings.DebugDSDT;
  InputItems[InputItemsCount].ItemType = Hex;  //103
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeIMEI);
  InputItems[InputItemsCount].ItemType = Hex;  //104
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(26);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.KernelAndKextPatches.FakeCPUID);


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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%01x", gSettings.DualLink);

  InputItems[InputItemsCount].ItemType = BoolValue; //110
  InputItems[InputItemsCount++].BValue = gSettings.NvidiaNoEFI;
  InputItems[InputItemsCount].ItemType = BoolValue; //111
  InputItems[InputItemsCount++].BValue = gSettings.NvidiaSingle;

  InputItems[InputItemsCount].ItemType = Hex;  //112
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"0x%04x", gSettings.IntelMaxValue);

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
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.EfiVersion);
  InputItems[InputItemsCount].ItemType = ASString;  //118
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(64);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BooterCfgStr);

  InputItems[InputItemsCount].ItemType = RadioSwitch;  //119 - Audio chooser
  InputItems[InputItemsCount++].IValue = 119;
  InputItems[InputItemsCount].ItemType = Decimal;  //120
  if (New) {
    InputItems[InputItemsCount].SValue = (__typeof__(InputItems[InputItemsCount].SValue))AllocateZeroPool(16);
  }
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 16, L"%04d", DefaultAudioVolume);
  
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

    AsciiSPrint(gSettings.BootArgs, 255, "%s ", InputItems[i].SValue);
  }
  i++; //1
  if (InputItems[i].Valid) {
    UnicodeSPrint(gSettings.DsdtName, sizeof(gSettings.DsdtName), L"%s", InputItems[i].SValue);
  }
  i++; //2
  if (InputItems[i].Valid) {
    UnicodeSPrint(gSettings.BlockKexts, sizeof(gSettings.BlockKexts), L"%s", InputItems[i].SValue);
  }
  i++; //3
  if (InputItems[i].Valid) {
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
    DBG("applied PlatformFeature=0x%x\n", gPlatformFeature);
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
        DBG("applied *-platform-id=0x%x\n", gSettings.IgPlatform);
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
          AsciiSPrint(AString, 255, "%s", InputItems[i].SValue);
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
    AsciiSPrint(NonDetected, 64, "%s", InputItems[i].SValue);
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
    DBG("applied FirmwareFeatures=0x%x\n", gFwFeatures);
  }
  i++; //63
  if (InputItems[i].Valid) {
    gFwFeaturesMask = (UINT32)StrHexToUint64(InputItems[i].SValue);
    DBG("applied FirmwareFeaturesMask=0x%x\n", gFwFeaturesMask);
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

  i++; //90
  if (InputItems[i].Valid) {
    Status = LoadUserSettings(SelfRootDir, ConfigsList[OldChosenConfig], &dict);
    if (!EFI_ERROR(Status)) {
      Status = GetUserSettings(SelfRootDir, dict);
      if (gConfigDict[2]) FreeTag(gConfigDict[2]);
      gConfigDict[2] = dict;
      UnicodeSPrint(gSettings.ConfigName, 64, L"%s", ConfigsList[OldChosenConfig]);
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
    DBG("applied FakeCPUID=%06x\n", gSettings.KernelAndKextPatches.FakeCPUID);
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
    DBG("applied DualLink=%x\n", gSettings.DualLink);
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
      UnicodeSPrint(gSettings.DsdtName, 64, L"BIOS.aml");
    } else {
      UnicodeSPrint(gSettings.DsdtName, 64, L"%s", DsdtsList[OldChosenDsdt]);
    }
  }
  i++; //117
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.EfiVersion, 64, "%s", InputItems[i].SValue);
  }
  i++; //118
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.BooterCfgStr, 64, "%s", InputItems[i].SValue);
  }
  i++; //119
  if (InputItems[i].Valid) {
    EFI_DEVICE_PATH_PROTOCOL*  DevicePath = NULL;
    UINT8 TmpIndex = OldChosenAudio & 0xFF;
    DBG("Chosen output %d:%s_%a\n", OldChosenAudio, AudioList[OldChosenAudio].Name, AudioOutputNames[OldChosenAudio]);

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
        UnicodeSPrint(InputItems[i].SValue, 16, L"%04d", DefaultAudioVolume);
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

VOID REFIT_MENU_SCREEN::AddMenuInfo(CONST CHAR16 *Line)
{
  REFIT_INFO_DIALOG *InputBootArgs;

//  InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs = new REFIT_INFO_DIALOG;
  InputBootArgs->Title = PoolPrint(L"%s", Line);
//  InputBootArgs->Tag = TAG_INFO;
//  InputBootArgs->Item = NULL;
  InputBootArgs->AtClick = ActionLight;
  AddMenuEntry(InputBootArgs, true);
}

VOID AboutRefit(VOID)
{
  //  CHAR8* Revision = NULL;
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    AboutMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_ABOUT);
  } else {
    AboutMenu.TitleImage = NULL;
  }
  if (AboutMenu.Entries.size() == 0) {
//    AboutMenu.AddMenuInfo(PoolPrint(L"Clover Version 5.0"));
#ifdef REVISION_STR
    AboutMenu.AddMenuInfo(PoolPrint(L" %a ", REVISION_STR));
#else
    AboutMenu.AddMenuInfo(PoolPrint(L"Clover Revision %a", gFirmwareRevision));
#endif
#ifdef FIRMWARE_BUILDDATE
    AboutMenu.AddMenuInfo(PoolPrint(L" Build: %a", FIRMWARE_BUILDDATE));
#else
    AboutMenu.AddMenuInfo(L" Build: unknown");
#endif
    AboutMenu.AddMenuInfo(L"");
    AboutMenu.AddMenuInfo(L"Based on rEFIt (c) 2006-2010 Christoph Pfisterer");
    AboutMenu.AddMenuInfo(L"Portions Copyright (c) Intel Corporation");
    AboutMenu.AddMenuInfo(L"Developers:");
    AboutMenu.AddMenuInfo(L"  Slice, dmazar, apianti, JrCs, pene, usrsse2");
    AboutMenu.AddMenuInfo(L"  Kabyl, pcj, jadran, Blackosx, STLVNUB, ycr.ru");
    AboutMenu.AddMenuInfo(L"  FrodoKenny, skoczi, crazybirdy, Oscar09, xsmile");
    AboutMenu.AddMenuInfo(L"  cparm, rehabman, nms42, Sherlocks, Zenith432");
    AboutMenu.AddMenuInfo(L"  stinga11, TheRacerMaster, solstice, SoThOr, DF");
    AboutMenu.AddMenuInfo(L"  cecekpawon, Micky1979, Needy, joevt, ErmaC, vit9696");
    AboutMenu.AddMenuInfo(L"  ath, savvas, syscl, goodwin_c, clovy, jief_machak");
    AboutMenu.AddMenuInfo(L"Credits also:");
    AboutMenu.AddMenuInfo(L"  projectosx.com, applelife.ru, insanelymac.com");
    AboutMenu.AddMenuInfo(L"");
    AboutMenu.AddMenuInfo(L"Running on:");
    AboutMenu.AddMenuInfo(PoolPrint(L" EFI Revision %d.%02d",
                                      gST->Hdr.Revision >> 16, gST->Hdr.Revision & ((1 << 16) - 1)));
#if defined(MDE_CPU_IA32)
    AboutMenu.AddMenuInfo(L" Platform: i386 (32 bit)");
#elif defined(MDE_CPU_X64)
    AboutMenu.AddMenuInfo(L" Platform: x86_64 (64 bit)");
#else
    AboutMenu.AddMenuInfo(L" Platform: unknown");
#endif
    AboutMenu.AddMenuInfo(PoolPrint(L" Firmware: %s rev %d.%d", gST->FirmwareVendor, gST->FirmwareRevision >> 16, gST->FirmwareRevision & ((1 << 16) - 1)));
    AboutMenu.AddMenuInfo(PoolPrint(L" Screen Output: %s", egScreenDescription()));
    AboutMenu.AnimeRun = AboutMenu.GetAnime();
    AboutMenu.AddMenuEntry(&MenuEntryReturn, false);
  } else if (AboutMenu.Entries.size() >= 2) {
    /*
      EntryCount instead of InfoLineCount. Lastline == return/back. Is necessary recheck screen res here?
    */
    FreePool(AboutMenu.Entries[AboutMenu.Entries.size()-2].Title);
    AboutMenu.Entries[AboutMenu.Entries.size()-2].Title = PoolPrint(L" Screen Output: %s", egScreenDescription());
  }

  AboutMenu.RunMenu(NULL);
}

VOID HelpRefit(VOID)
{
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    HelpMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_HELP);
  } else {
    HelpMenu.TitleImage = NULL;
  }
  if (HelpMenu.Entries.size() == 0) {
    switch (gLanguage)
    {
      case russian:
        HelpMenu.AddMenuInfo(L"ESC - Выход из подменю, обновление главного меню");
        HelpMenu.AddMenuInfo(L"F1  - Помощь по горячим клавишам");
        HelpMenu.AddMenuInfo(L"F2  - Сохранить отчет в preboot.log (только если FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Показать скрытые значки в меню");
        HelpMenu.AddMenuInfo(L"F4  - Родной DSDT сохранить в EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Патченный DSDT сохранить в EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Сохранить ВидеоБиос в EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Проверить звук на выбранном выходе");
        HelpMenu.AddMenuInfo(L"F8  - Сделать дамп звуковых устройств в EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Изменить разрешение экрана на одно из возможных");
        HelpMenu.AddMenuInfo(L"F10 - Снимок экрана в папку EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Извлечь указанный DVD");
        HelpMenu.AddMenuInfo(L"Пробел - Дополнительное меню запуска выбранного тома");
        HelpMenu.AddMenuInfo(L"Цифры 1-9 - Быстрый запуск тома по порядку в меню");
        HelpMenu.AddMenuInfo(L"A (About) - О загрузчике");
        HelpMenu.AddMenuInfo(L"O (Options) - Дополнительные настройки");
        HelpMenu.AddMenuInfo(L"R (Reset) - Теплый перезапуск");
        HelpMenu.AddMenuInfo(L"U (go oUt) - Завершить работу в Кловере");
        HelpMenu.AddMenuInfo(L"S (Shell) - Переход в режим командной строки");
        break;
      case ukrainian:
        HelpMenu.AddMenuInfo(L"ESC - Вийти з меню, оновити головне меню");
        HelpMenu.AddMenuInfo(L"F1  - Ця довідка");
        HelpMenu.AddMenuInfo(L"F2  - Зберегти preboot.log (тiльки FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Відображати приховані розділи");
        HelpMenu.AddMenuInfo(L"F4  - Зберегти OEM DSDT в EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Зберегти патчений DSDT в EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F6  - Зберегти VideoBios в EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F10 - Зберегти знімок екрану в EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Відкрити обраний диск (DVD)");
        HelpMenu.AddMenuInfo(L"Пробіл - докладніше про обраний пункт меню");
        HelpMenu.AddMenuInfo(L"Клавіші 1-9 -  клавіші пунктів меню");
        HelpMenu.AddMenuInfo(L"A - Про систему");
        HelpMenu.AddMenuInfo(L"O - Опції меню");
        HelpMenu.AddMenuInfo(L"R - Перезавантаження");
        HelpMenu.AddMenuInfo(L"U - Відключити ПК");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case spanish:
        HelpMenu.AddMenuInfo(L"ESC - Salir de submenu o actualizar el menu principal");
        HelpMenu.AddMenuInfo(L"F1  - Esta Ayuda");
        HelpMenu.AddMenuInfo(L"F2  - Guardar preboot.log (Solo FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Guardar DSDT oem en EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Guardar DSDT parcheado en EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Guardar VideoBios en EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Guardar Captura de pantalla en EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Expulsar volumen seleccionado (DVD)");
        HelpMenu.AddMenuInfo(L"Espacio - Detalles acerca selected menu entry");
        HelpMenu.AddMenuInfo(L"Digitos 1-9 - Atajo a la entrada del menu");
        HelpMenu.AddMenuInfo(L"A - Menu Acerca de");
        HelpMenu.AddMenuInfo(L"O - Menu Optiones");
        HelpMenu.AddMenuInfo(L"R - Reiniciar Equipo");
        HelpMenu.AddMenuInfo(L"U - Apagar");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case portuguese:
      case brasil:
        HelpMenu.AddMenuInfo(L"ESC - Sai do submenu, atualiza o menu principal");
        HelpMenu.AddMenuInfo(L"F1  - Esta ajuda");
        HelpMenu.AddMenuInfo(L"F2  - Salva preboot.log (somente FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Salva oem DSDT em EFI/CLOVER/ACPI/origin/ (somente FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Salva DSDT corrigido em EFI/CLOVER/ACPI/origin/ (somente FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Salva VideoBios em EFI/CLOVER/misc/ (somente FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Salva screenshot em EFI/CLOVER/misc/ (somente FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Ejeta o volume selecionado (DVD)");
        HelpMenu.AddMenuInfo(L"Espaco - Detalhes sobre a opcao do menu selecionada");
        HelpMenu.AddMenuInfo(L"Tecle 1-9 - Atalho para as entradas do menu");
        HelpMenu.AddMenuInfo(L"A - Sobre o Menu");
        HelpMenu.AddMenuInfo(L"O - Opcoes do Menu");
        HelpMenu.AddMenuInfo(L"R - Reiniciar");
        HelpMenu.AddMenuInfo(L"U - Desligar");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case italian:
        HelpMenu.AddMenuInfo(L"ESC - Esci dal submenu, Aggiorna menu principale");
        HelpMenu.AddMenuInfo(L"F1  - Aiuto");
        HelpMenu.AddMenuInfo(L"F2  - Salva il preboot.log (solo su FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Mostra volumi nascosti");
        HelpMenu.AddMenuInfo(L"F4  - Salva il DSDT oem in EFI/CLOVER/ACPI/origin/ (solo suFAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Salva il patched DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Salva il VideoBios in EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Controlla il suono sull'uscita selezionata");
        HelpMenu.AddMenuInfo(L"F8  - Scarica le uscite audio in EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Cambia la risoluzione dello schermo alla prossima disponibile");
        HelpMenu.AddMenuInfo(L"F10 - Salva screenshot in EFI/CLOVER/misc/ (solo su FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Resetta NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Espelli il volume selezionato (DVD)");
        HelpMenu.AddMenuInfo(L"Spazio - Dettagli sul menu selezionato");
        HelpMenu.AddMenuInfo(L"Digita 1-9 - Abbreviazioni per il menu");
        HelpMenu.AddMenuInfo(L"A - Informazioni");
        HelpMenu.AddMenuInfo(L"O - Menu Opzioni");
        HelpMenu.AddMenuInfo(L"R - Riavvio");
        HelpMenu.AddMenuInfo(L"U - Spegnimento");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case german:
        HelpMenu.AddMenuInfo(L"ESC - Zurueck aus Untermenue, Hauptmenue erneuern");
        HelpMenu.AddMenuInfo(L"F1  - Diese Hilfe");
        HelpMenu.AddMenuInfo(L"F2  - Sichere preboot.log (nur mit FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Sichere OEM DSDT in EFI/CLOVER/ACPI/origin/ (nur mit FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Sichere gepatchtes DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Sichere VideoBios in EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Sichere Bildschirmfoto in EFI/CLOVER/misc/ (nur mit FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Volume auswerfen (DVD)");
        HelpMenu.AddMenuInfo(L"Leertaste - Details über den gewählten Menue Eintrag");
        HelpMenu.AddMenuInfo(L"Zahlen 1-9 - Kurzwahl zum Menue Eintrag");
        HelpMenu.AddMenuInfo(L"A - Menue Informationen");
        HelpMenu.AddMenuInfo(L"O - Menue Optionen");
        HelpMenu.AddMenuInfo(L"R - Neustart");
        HelpMenu.AddMenuInfo(L"U - Ausschalten");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case dutch:
        HelpMenu.AddMenuInfo(L"ESC - Verlaat submenu, Vernieuw hoofdmenu");
        HelpMenu.AddMenuInfo(L"F1  - Onderdeel hulp");
        HelpMenu.AddMenuInfo(L"F2  - preboot.log opslaan (Alleen FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Verborgen opties weergeven");
        HelpMenu.AddMenuInfo(L"F4  - Opslaan oem DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Opslaan gepatchte DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Opslaan VideoBios in EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Controleer geluid op geselecteerde uitgang");
        HelpMenu.AddMenuInfo(L"F8  - Opslaan audio uitgangen in EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Wijzig schermresolutie naar eerstvolgende mogelijke modus");
        HelpMenu.AddMenuInfo(L"F10 - Opslaan schermafdruk in EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Uitwerpen geselecteerd volume (DVD)");
        HelpMenu.AddMenuInfo(L"Spatie - Details over geselecteerd menuoptie");
        HelpMenu.AddMenuInfo(L"Cijfers 1-9 - Snelkoppeling naar menuoptie");
        HelpMenu.AddMenuInfo(L"A - Menu Over");
        HelpMenu.AddMenuInfo(L"O - Menu Opties");
        HelpMenu.AddMenuInfo(L"R - Soft Reset");
        HelpMenu.AddMenuInfo(L"U - Verlaten");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case french:
        HelpMenu.AddMenuInfo(L"ESC - Quitter sous-menu, Retour menu principal");
        HelpMenu.AddMenuInfo(L"F1  - Aide");
        HelpMenu.AddMenuInfo(L"F2  - Enregistrer preboot.log (FAT32 only)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Enregistrer oem DSDT dans EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Enregistrer DSDT modifié dans EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Enregistrer VideoBios dans EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Enregistrer la capture d'écran dans EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Ejecter le volume (DVD)");
        HelpMenu.AddMenuInfo(L"Space - Détails a propos du menu selectionné");
        HelpMenu.AddMenuInfo(L"Digits 1-9 - Raccourci vers entrée menu");
        HelpMenu.AddMenuInfo(L"A - A propos");
        HelpMenu.AddMenuInfo(L"O - Options Menu");
        HelpMenu.AddMenuInfo(L"R - Redémarrer");
        HelpMenu.AddMenuInfo(L"U - Eteindre");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case indonesian:
        HelpMenu.AddMenuInfo(L"ESC - Keluar submenu, Refresh main menu");
        HelpMenu.AddMenuInfo(L"F1  - Help");
        HelpMenu.AddMenuInfo(L"F2  - Simpan preboot.log ke EFI/CLOVER/ACPI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Simpan oem DSDT ke EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Simpan patched DSDT ke EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Simpan VideoBios ke EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Simpan screenshot ke EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Eject volume (DVD)");
        HelpMenu.AddMenuInfo(L"Spasi - Detail dari menu yang dipilih");
        HelpMenu.AddMenuInfo(L"Tombol 1-9 - Shortcut pilihan menu");
        HelpMenu.AddMenuInfo(L"A - About");
        HelpMenu.AddMenuInfo(L"O - Opsi");
        HelpMenu.AddMenuInfo(L"R - Soft Reset");
        HelpMenu.AddMenuInfo(L"U - Shutdown");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case polish:
        HelpMenu.AddMenuInfo(L"ESC - Wyjscie z podmenu, Odswiezenie glownego menu");
        HelpMenu.AddMenuInfo(L"F1  - Pomoc");
        HelpMenu.AddMenuInfo(L"F2  - Zapis preboot.log (tylko FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Zapis DSDT do EFI/CLOVER/ACPI/origin/ (tylko FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Zapis poprawionego DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Zapis BIOSu k. graficznej do EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Zapis zrzutu ekranu do EFI/CLOVER/misc/ (tylko FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Wysuniecie zaznaczonego dysku (tylko dla DVD)");
        HelpMenu.AddMenuInfo(L"Spacja - Informacje nt. dostepnych opcji dla zaznaczonego dysku");
        HelpMenu.AddMenuInfo(L"Znaki 1-9 - Skroty opcji dla wybranego dysku");
        HelpMenu.AddMenuInfo(L"A - Menu Informacyjne");
        HelpMenu.AddMenuInfo(L"O - Menu Opcje");
        HelpMenu.AddMenuInfo(L"R - Restart komputera");
        HelpMenu.AddMenuInfo(L"U - Wylaczenie komputera");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case croatian:
        HelpMenu.AddMenuInfo(L"ESC - izlaz iz podizbornika, Osvježi glavni izbornik");
        HelpMenu.AddMenuInfo(L"F1  - Ovaj izbornik");
        HelpMenu.AddMenuInfo(L"F2  - Spremi preboot.log (samo na FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Spremi oem DSDT u EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Spremi patched DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Spremi VideoBios into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Spremi screenshot into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Izbaci izabrai (DVD)");
        HelpMenu.AddMenuInfo(L"Space - Detalji o odabranom sistemu");
        HelpMenu.AddMenuInfo(L"Brojevi 1 do 9 su prečac do izbora");
        HelpMenu.AddMenuInfo(L"A - Izbornik o meni");
        HelpMenu.AddMenuInfo(L"O - Izbornik opcije");
        HelpMenu.AddMenuInfo(L"R - Restart računala");
        HelpMenu.AddMenuInfo(L"U - Isključivanje računala");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case czech:
        HelpMenu.AddMenuInfo(L"ESC - Vrátit se do hlavní nabídky");
        HelpMenu.AddMenuInfo(L"F1  - Tato Nápověda");
        HelpMenu.AddMenuInfo(L"F2  - Uložit preboot.log (FAT32 only)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Uložit oem DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Uložit patchnuté DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Uložit VideoBios do EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Uložit snímek obrazovky do EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Vysunout vybranou mechaniku (DVD)");
        HelpMenu.AddMenuInfo(L"Mezerník - Podrobnosti o vybraném disku");
        HelpMenu.AddMenuInfo(L"čísla 1-9 - Klávesové zkratky pro disky");
        HelpMenu.AddMenuInfo(L"A - Menu O Programu");
        HelpMenu.AddMenuInfo(L"O - Menu Možnosti");
        HelpMenu.AddMenuInfo(L"R - Částečný restart");
        HelpMenu.AddMenuInfo(L"U - Odejít");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case korean:
        HelpMenu.AddMenuInfo(L"ESC - 하위메뉴에서 나감, 메인메뉴 새로 고침");
        HelpMenu.AddMenuInfo(L"F1  - 이 도움말");
        HelpMenu.AddMenuInfo(L"F2  - preboot.log를 저장합니다. (FAT32방식에만 해당됨)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - oem DSDT를 EFI/CLOVER/ACPI/origin/에 저장합니다. (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - 패치된 DSDT를 EFI/CLOVER/ACPI/origin/에 저장합니다. (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - VideoBios를 EFI/CLOVER/misc/에 저장합니다. (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - 선택한 출력에서 사운드 확인");
        HelpMenu.AddMenuInfo(L"F8  - 오디오 코덱덤프를 EFI/CLOVER/misc/에 저장합니다.");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - 스크린샷을 EFI/CLOVER/misc/에 저장합니다. (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - NVRAM 초기화");
        HelpMenu.AddMenuInfo(L"F12 - 선택한 볼륨을 제거합니다. (DVD)");
        HelpMenu.AddMenuInfo(L"Space - 선택한 메뉴의 상세 설명");
        HelpMenu.AddMenuInfo(L"Digits 1-9 - 메뉴 단축 번호");
        HelpMenu.AddMenuInfo(L"A - 단축키 - 이 부트로더에 관하여");
        HelpMenu.AddMenuInfo(L"O - 단축키 - 부트 옵션");
        HelpMenu.AddMenuInfo(L"R - 단축키 - 리셋");
        HelpMenu.AddMenuInfo(L"U - 단축키 - 시스템 종료");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case romanian:
        HelpMenu.AddMenuInfo(L"ESC - Iesire din sub-meniu, Refresh meniul principal");
        HelpMenu.AddMenuInfo(L"F1  - Ajutor");
        HelpMenu.AddMenuInfo(L"F2  - Salvare preboot.log (doar pentru FAT32)");
        HelpMenu.AddMenuInfo(L"F4  - Salvare oem DSDT in EFI/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Salvare DSDT modificat in EFI/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Salvare VideoBios in EFI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Salvare screenshot in EFI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Scoatere volum selectat (DVD)");
        HelpMenu.AddMenuInfo(L"Space - Detalii despre item-ul selectat");
        HelpMenu.AddMenuInfo(L"Cifre 1-9 - Scurtaturi pentru itemele meniului");
        HelpMenu.AddMenuInfo(L"A - Despre");
        HelpMenu.AddMenuInfo(L"O - Optiuni");
        HelpMenu.AddMenuInfo(L"R - Soft Reset");
        HelpMenu.AddMenuInfo(L"U - Inchidere");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case chinese:
        HelpMenu.AddMenuInfo(L"ESC - 离开子菜单， 刷新主菜单");
        HelpMenu.AddMenuInfo(L"F1  - 帮助");
        HelpMenu.AddMenuInfo(L"F2  - 保存 preboot.log 到 EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - 显示隐藏的启动项");
        HelpMenu.AddMenuInfo(L"F4  - 保存原始的 DSDT 到 EFI/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - 保存修正后的 DSDT 到 EFI/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - 保存 VideoBios 到 EFI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - 检查选中输出设备的声音");
        HelpMenu.AddMenuInfo(L"F8  - 生成声卡输出转储到 EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - 调整屏幕分辨率为下一个可用的模式");
        HelpMenu.AddMenuInfo(L"F10 - 保存截图到 EFI/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - 重置 NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - 推出选中的卷 (DVD)");
        HelpMenu.AddMenuInfo(L"空格 - 关于选中项的详情");
        HelpMenu.AddMenuInfo(L"数字 1-9 - 菜单快捷键");
        HelpMenu.AddMenuInfo(L"A - 关于");
        HelpMenu.AddMenuInfo(L"O - 选项");
        HelpMenu.AddMenuInfo(L"R - 软复位");
        HelpMenu.AddMenuInfo(L"U - 退出");
        HelpMenu.AddMenuInfo(L"S - Shell");
        break;
      case english:
      default:
        HelpMenu.AddMenuInfo(L"ESC - Escape from submenu, Refresh main menu");
        HelpMenu.AddMenuInfo(L"F1  - This help");
        HelpMenu.AddMenuInfo(L"F2  - Save preboot.log into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F3  - Show hidden entries");
        HelpMenu.AddMenuInfo(L"F4  - Save oem DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F5  - Save patched DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F6  - Save VideoBios into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F7  - Check sound on selected output");
        HelpMenu.AddMenuInfo(L"F8  - Make audio outputs dump into EFI/CLOVER/misc/");
        HelpMenu.AddMenuInfo(L"F9  - Switch screen resoluton to next possible mode");
        HelpMenu.AddMenuInfo(L"F10 - Save screenshot into EFI/CLOVER/misc/ (FAT32)");
        HelpMenu.AddMenuInfo(L"F11 - Reset NVRAM");
        HelpMenu.AddMenuInfo(L"F12 - Eject selected volume (DVD)");
        HelpMenu.AddMenuInfo(L"Space - Details about selected menu entry");
        HelpMenu.AddMenuInfo(L"Digits 1-9 - Shortcut to menu entry");
        HelpMenu.AddMenuInfo(L"A - Menu About");
        HelpMenu.AddMenuInfo(L"O - Menu Options");
        HelpMenu.AddMenuInfo(L"R - Soft Reset");
        HelpMenu.AddMenuInfo(L"U - Exit from Clover");
        HelpMenu.AddMenuInfo(L"S - Shell");
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
    if (GlobalConfig.DarkEmbedded) {
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
}

VOID REFIT_MENU_SCREEN::UpdateScroll(IN UINTN Movement)
{
  INTN Lines;
  UINTN ScrollMovement = SCROLL_SCROLL_DOWN;
  INTN i;
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
      for (i = 0; i < Lines; i++)
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
//
// menu helper functions
//

VOID REFIT_MENU_SCREEN::AddMenuInfoLine(IN CONST CHAR16 *InfoLine)
{
  InfoLines.Add(InfoLine);
//  AddListElement((VOID ***) &(Screen->InfoLines), (UINTN*)&(Screen->InfoLines.size()), (CHAR16*)InfoLine); // TODO jief : cast to fix
}

VOID REFIT_MENU_SCREEN::AddMenuEntry(IN REFIT_ABSTRACT_MENU_ENTRY *Entry, bool freeIt)
{
	if ( !Entry ) return;
	Entries.AddReference(Entry, freeIt);
//  AddListElement((VOID ***) &(Screen->Entries), (UINTN*)&(Screen->Entries.size()), Entry);
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
        if (Tentry->SubScreen->Title) {
          FreePool(Tentry->SubScreen->Title);
          Tentry->SubScreen->Title = NULL;
        }
        // don't free image because of reusing them
     //   FreeMenu(Tentry->SubScreen);
        Tentry->SubScreen->FreeMenu();
        Tentry->SubScreen = NULL;
      }
      if (Tentry->getREFIT_MENU_ITEM_RETURN()) { //can't free constants
        if (Tentry->Title) {
          FreePool(Tentry->Title);
          Tentry->Title = NULL;
        }
      }
      FreePool(Tentry);
    }
    Entries.Empty();
//    FreePool(Screen->Entries);
//    Screen->Entries = NULL;
  }
  if (InfoLines.size() > 0) {
    for (UINTN i = 0; i < InfoLines.size(); i++) {
      // TODO: call a user-provided routine for each element here
      FreePool(InfoLines[i]);
    }
    InfoLines.Empty();
//    Screen->InfoLines.size() = 0;
//    FreePool(Screen->InfoLines);
//    Screen->InfoLines = NULL;
  }

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
        UnicodeSPrint(Item->SValue, SVALUE_MAX_SIZE, L"%s", Backup);
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
    MsgLog("EDITED: %s\n", Item->SValue);
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
  CHAR16        *TimeoutMessage;
  UINTN         MenuExit;

  if (ChosenEntry == NULL) {
    TextStyle = 0;
  } else {
    TextStyle = 2;
  }

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
      TimeoutMessage = PoolPrint(L"%s in %d seconds", TimeoutText, TimeoutCountdown);
      ((*this).*(StyleFunc))(MENU_FUNCTION_PAINT_TIMEOUT, TimeoutMessage);
      FreePool(TimeoutMessage);
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
        DBG("create folder %r\n", Status);
        if (!EFI_ERROR(Status)) {
          Status = egSaveFile(SelfRootDir,  L"EFI\\CLOVER\\new_folder\\new_file.txt", (UINT8*)SomeText, sizeof(*SomeText)+1);
          DBG("create file %r\n", Status);
        }
        break;
*/
      case SCAN_F7:
        if (OldChosenAudio > AudioNum) {
              OldChosenAudio = 0; //security correction
        }
        Status = gBS->HandleProtocol(AudioList[OldChosenAudio].Handle, &gEfiAudioIoProtocolGuid, (VOID**)&AudioIo);
        DBG("open %d audio handle status=%r\n", OldChosenAudio, Status);
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
//  DBG("draw text %s\n", Text);
//  DBG("pos=%d width=%d xtext=%d Height=%d Y=%d\n", XPos, TextWidth, XText, Height, YPos);
  BltImageAlpha(TextBufferXY, XText, YPos,  &MenuBackgroundPixel, 16);
  egFreeImage(TextBufferXY);

  return TextWidth;
}

/**
 * Helper function to draw text for Boot Camp Style.
 * @author: Needy
 */
VOID DrawBCSText(IN CONST CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign)
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

    DrawTextXY(BCSText, XPos, YPos, XAlign);
    FreePool(BCSText);
  } else {
		// draw full text
    DrawTextXY(Text, XPos, YPos, XAlign);
  }
}



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

VOID SetBar(INTN PosX, INTN UpPosY, INTN DownPosY, IN SCROLL_STATE *State)
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

VOID ScrollingBar(IN SCROLL_STATE *State)
{
  EG_IMAGE* Total;
  INTN  i;

  ScrollEnabled = (State->MaxFirstVisible != 0);
  if (ScrollEnabled) {
    Total = egCreateFilledImage(ScrollTotal.Width, ScrollTotal.Height, TRUE, &MenuBackgroundPixel);

    if (ScrollbarBackgroundImage && ScrollbarBackgroundImage->Height) {
      for (i = 0; i < ScrollbarBackground.Height; i+=ScrollbarBackgroundImage->Height) {
        egComposeImage(Total, ScrollbarBackgroundImage, ScrollbarBackground.XPos - ScrollTotal.XPos, ScrollbarBackground.YPos + i - ScrollTotal.YPos);
      }
    }

    egComposeImage(Total, BarStartImage, BarStart.XPos - ScrollTotal.XPos, BarStart.YPos - ScrollTotal.YPos);
    egComposeImage(Total, BarEndImage, BarEnd.XPos - ScrollTotal.XPos, BarEnd.YPos - ScrollTotal.YPos);

    if (ScrollbarImage && ScrollbarImage->Height) {
      for (i = 0; i < Scrollbar.Height; i+=ScrollbarImage->Height) {
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

/**
 * Graphical menu.
 */
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
					//DBG("paint entry %d title=%s\n", i, Entries[i]->Title);
          DrawMenuText(ResultString,
                       (i == ScrollState.CurrentSelection) ? MenuWidth : 0,
                       EntriesPosX, Entry->Place.YPos, 0xFFFF);
        }
      }

      ScrollingBar(&ScrollState);
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
      ScrollingBar(&ScrollState);

      break;
    }

    case MENU_FUNCTION_PAINT_TIMEOUT: //ever be here?
      X = (UGAWidth - StrLen(ParamText) * ScaledWidth) >> 1;
      DrawMenuText(ParamText, 0, X, TimeoutPosY, 0xFFFF);
      break;
  }

  MouseBirth();
}

/**
 * Draw entries for GUI.
 */
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
  //  DBG("Entry title=%s; Width=%d\n", Entry->Title, MainImage->Width);
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

    // inidcator is for row 0, main entries, only
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
  //  DBG("draw text %s at (%d, %d)\n", Text, Xpos, UGAHeight - 5 - TextHeight),
// clovy  DrawTextXY(Text, Xpos, UGAHeight - 5 - TextHeight, Align);
  DrawTextXY(Text, Xpos, UGAHeight - (INTN)(TextHeight * 1.5f), Align);
  if ( Text ) FreePool(Text);
}

VOID REFIT_MENU_SCREEN::MainMenuVerticalStyle(IN UINTN Function, IN CONST CHAR16 *ParamText)
{
  INTN i;
  INTN row0PosYRunning;
  INTN VisibleHeight = 0; //assume vertical layout
  INTN MessageHeight = 20;
  if (GlobalConfig.TypeSVG && textFace[1].valid) {
    MessageHeight = (INTN)(textFace[1].size * RowHeightFromTextHeight * GlobalConfig.Scale);
  }
  else {
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
        CopyMem(&FilmPlace, &BannerPlace, sizeof(BannerPlace));
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

      ScrollingBar(&ScrollState);
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

      ScrollingBar(&ScrollState);
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

/**
 * Main screen text.
 */
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
              // clear the screen
              FillRectAreaOfScreen(textPosX, textPosY, EntriesWidth + GlobalConfig.TileXSpace,
                                   MessageHeight, &MenuBackgroundPixel, X_IS_CENTER);
              // draw the text
              DrawBCSText(Entries[i].Title, textPosX, textPosY, X_IS_CENTER);
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
// clovy                               OldTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
                               OldTextWidth, MessageHeight, &MenuBackgroundPixel, X_IS_CENTER);
      }

      // something is wrong with the DrawMainMenuLabel or Entries[ScrollState.CurrentSelection]
      // and it's required to create the first selection text from here
      // used for the second row entries, when BootCampStyle is used
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
        DBG("can't bear mouse at all! Status=%r\n", Status);
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
        DBG("can't bear mouse at sel! Status=%r\n", Status);
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
        DBG("can't bear mouse at timeout! Status=%r\n", Status);
      }
      break;

  }
}

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

REFIT_ABSTRACT_MENU_ENTRY* NewEntry_(REFIT_ABSTRACT_MENU_ENTRY *Entry, REFIT_MENU_SCREEN **SubScreen, ACTION AtClick, UINTN ID, CONST CHAR8 *Title)
{
  if (Title) {
    Entry->Title = PoolPrint(L"%a", Title);
  } else {
    Entry->Title = (__typeof__(Entry->Title))AllocateZeroPool(128);
  }

  Entry->Image =  OptionMenu.TitleImage;
  Entry->AtClick = AtClick;
  // create the submenu
//  *SubScreen = (__typeof_am__(*SubScreen))AllocateZeroPool(sizeof(**SubScreen));
  *SubScreen = new REFIT_MENU_SCREEN();
  (*SubScreen)->Title = EfiStrDuplicate(Entry->Title);
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

VOID REFIT_MENU_SCREEN::AddMenuCheck(CONST CHAR8 *Text, UINTN Bit, INTN ItemNum)
{
  REFIT_MENU_CHECKBIT *InputBootArgs;

//  InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs = new REFIT_MENU_CHECKBIT;
  InputBootArgs->Title = PoolPrint(L"%a", Text);
//  InputBootArgs->Tag = TAG_CHECKBIT_OLD;
  InputBootArgs->Row = Bit;
  InputBootArgs->Item = &InputItems[ItemNum];
  InputBootArgs->AtClick = ActionEnter;
  InputBootArgs->AtRightClick = ActionDetails;
  AddMenuEntry(InputBootArgs, true);
}

VOID ModifyTitles(REFIT_ABSTRACT_MENU_ENTRY *ChosenEntry)
{
  if (ChosenEntry->SubScreen->ID == SCREEN_DSDT) {
    UnicodeSPrint((CHAR16*)ChosenEntry->Title, 128, L"DSDT fix mask [0x%08x]->", gSettings.FixDsdt); // TODO jief : cast to fix
    //MsgLog("@ESC: %s\n", (*ChosenEntry)->Title);
  } else if (ChosenEntry->SubScreen->ID == SCREEN_CSR) {
    // CSR
    UnicodeSPrint((CHAR16*)ChosenEntry->Title, 128, L"System Integrity Protection [0x%04x]->", gSettings.CsrActiveConfig); // TODO jief : cast to fix
    // check for the right booter flag to allow the application
    // of the new System Integrity Protection configuration.
    if (gSettings.CsrActiveConfig != 0 && gSettings.BooterConfig == 0) {
      gSettings.BooterConfig = 0x28;
    }

  } else if (ChosenEntry->SubScreen->ID == SCREEN_BLC) {
    UnicodeSPrint((CHAR16*)ChosenEntry->Title, 128, L"boot_args->flags [0x%04x]->", gSettings.BooterConfig); // TODO jief : cast to fix
  } else if (ChosenEntry->SubScreen->ID == SCREEN_DSM) {
    UnicodeSPrint((CHAR16*)ChosenEntry->Title, 128, L"Drop OEM _DSM [0x%04x]->", dropDSM); // TODO jief : cast to fix
  }
}

VOID REFIT_MENU_SCREEN::AddMenuItem_(REFIT_MENU_ENTRY_ITEM_ABSTRACT* InputBootArgs, INTN Inx, CONST CHAR8 *Line, BOOLEAN Cursor)
{
  InputBootArgs->Title          = PoolPrint(L"%a", Line);
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

REFIT_ABSTRACT_MENU_ENTRY *SubMenuGraphics()
{
  UINTN  i, N, Ven = 97;
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_GRAPHICS, "Graphics Injector->");
  SubScreen->AddMenuInfoLine(PoolPrint(L"Number of VideoCard%a=%d",((NGFX!=1)?"s":""), NGFX));

  SubScreen->AddMenuItemInput(52, "InjectEDID", FALSE);
  SubScreen->AddMenuItemInput(53, "Fake Vendor EDID:", TRUE);
  SubScreen->AddMenuItemInput(54, "Fake Product EDID:", TRUE);
  SubScreen->AddMenuItemInput(18, "Backlight Level:", TRUE);
  SubScreen->AddMenuItemInput(112, "Intel Max Backlight:", TRUE); //gSettings.IntelMaxValue


  for (i = 0; i < NGFX; i++) {
    SubScreen->AddMenuInfo(L"----------------------");
    SubScreen->AddMenuInfo(PoolPrint(L"Card DeviceID=%04x", gGraphics[i].DeviceID));
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
  SubScreen->AddMenuInfoLine(PoolPrint(L"Choose options to tune the HDA devices"));
  SubScreen->AddMenuInfoLine(PoolPrint(L"Number of Audio Controller%a=%d", ((NHDA!=1)?"s":""), NHDA));
  for (i = 0; i < NHDA; i++) {
      SubScreen->AddMenuInfoLine(PoolPrint(L"%d) %s [%04x][%04x]",
                                           (i+1),
                                           gAudios[i].controller_name,
                                           gAudios[i].controller_vendor_id,
                                           gAudios[i].controller_device_id)
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
  SubScreen->AddMenuInfoLine(PoolPrint(L"%a", gCPUStructure.BrandString));
  SubScreen->AddMenuInfoLine(PoolPrint(L"Model: %2x/%2x/%2x",
      gCPUStructure.Family, gCPUStructure.Model, gCPUStructure.Stepping));
  SubScreen->AddMenuInfoLine(PoolPrint(L"Cores: %d Threads: %d",
                  gCPUStructure.Cores, gCPUStructure.Threads));
  SubScreen->AddMenuInfoLine(PoolPrint(L"FSB speed MHz: %d",
                  DivU64x32(gCPUStructure.FSBFrequency, Mega)));
  SubScreen->AddMenuInfoLine(PoolPrint(L"CPU speed MHz: %d",
                  DivU64x32(gCPUStructure.CPUFrequency, Mega)));
  SubScreen->AddMenuInfoLine(PoolPrint(L"Ratio: Min=%d.%d Max=%d.%d Turbo=%d.%d/%d.%d/%d.%d/%d.%d",
     nya(gCPUStructure.MinRatio), nya(gCPUStructure.MaxRatio),
     nya(gCPUStructure.Turbo4), nya(gCPUStructure.Turbo3), nya(gCPUStructure.Turbo2), nya(gCPUStructure.Turbo1)));


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
    InputBootArgs->Title = PoolPrint(L"%30a", KextPatchesMenu[Index].Label);
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
    		SubScreen->AddMenuInfoLine(PoolPrint(L"Choose/check kext to disable:"));
    	}
//      InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      InputBootArgs = new REFIT_INPUT_DIALOG;
      InputBootArgs->Title = PoolPrint(L"%s, v.%s", Kext->FileName, Kext->Version);
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
        InputBootArgs->Title = PoolPrint(L"  |-- %s, v.%s", plugInKext->FileName, plugInKext->Version);
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
//    DBG("chosen os %a\n", ChosenOS);
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


		SubScreen->AddMenuInfoLine(PoolPrint(
		                L"Block injected kexts for target version of macOS: %a",
		                ShortOSVersion));

		// Add kext from 10
		{
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(L"10"), true);

			CHAR16 DirName[256];
			if (OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {
				UnicodeSPrint(DirName, sizeof(DirName), L"10_install");
			}
			else {
				if (OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType)) {
					UnicodeSPrint(DirName, sizeof(DirName), L"10_recovery");
				}
				else {
					UnicodeSPrint(DirName, sizeof(DirName), L"10_normal");
				}
			}
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(DirName), true);
		}

		// Add kext from 10.{version}
		{
			CHAR16 DirName[256];
			UnicodeSPrint(DirName, sizeof(DirName), L"%a", ShortOSVersion);
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(DirName), true);

			if (OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {
				UnicodeSPrint(DirName, sizeof(DirName), L"%a_install", ShortOSVersion);
			}
			else {
				if (OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType)) {
					UnicodeSPrint(DirName, sizeof(DirName), L"%a_recovery", ShortOSVersion);
				}
				else {
					UnicodeSPrint(DirName, sizeof(DirName), L"%a_normal", ShortOSVersion);
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
					UnicodeSPrint(OSVersionKextsDirName, sizeof(OSVersionKextsDirName), L"%a.0", Entry->OSVersion);
				}else{
					UnicodeSPrint(OSVersionKextsDirName, sizeof(OSVersionKextsDirName), L"%a", Entry->OSVersion);
				}
				SubScreen->AddMenuEntry(SubMenuKextBlockInjection(OSVersionKextsDirName), true);
			}

			CHAR16 DirName[256];
			if (OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {
				UnicodeSPrint(DirName, sizeof(DirName), L"%a_install",
				        Entry->OSVersion);
			}
			else {
				if (OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType)) {
					UnicodeSPrint(DirName, sizeof(DirName), L"%a_recovery",
					        Entry->OSVersion);
				}
				else {
					UnicodeSPrint(DirName, sizeof(DirName), L"%a_normal",
					        Entry->OSVersion);
				}
			}
			SubScreen->AddMenuEntry(SubMenuKextBlockInjection(DirName), true);
		}
	}
	else {
		SubScreen->AddMenuInfoLine(PoolPrint(
		                L"Block injected kexts for target version of macOS: %a",
		                ChosenOS));
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
    InputBootArgs->Title = PoolPrint(L"%30a", KernelPatchesMenu[Index].Label);
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
    InputBootArgs->Title = PoolPrint(L"%30a", BootPatchesMenu[Index].Label);
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

  SubScreen->AddMenuInfoLine(PoolPrint(L"%a", gCPUStructure.BrandString));
  SubScreen->AddMenuInfoLine(PoolPrint(L"Real CPUID: 0x%06x", gCPUStructure.Signature));


  SubScreen->AddMenuItemInput(64,  "Debug", FALSE);
  SubScreen->AddMenuInfo(L"----------------------");
  SubScreen->AddMenuItemInput(104, "Fake CPUID:", TRUE);
//  SubScreen->AddMenuItemInput(108, "Kernel patching allowed", FALSE);
  SubScreen->AddMenuItemInput(45,  "Kernel Support CPU", FALSE);
  SubScreen->AddMenuItemInput(91,  "Kernel Lapic", FALSE);
  SubScreen->AddMenuItemInput(105, "Kernel XCPM", FALSE);
  SubScreen->AddMenuItemInput(48,  "Kernel PM", FALSE);
  SubScreen->AddMenuItemInput(121,  "Panic No Kext Dump", FALSE);
  SubScreen->AddMenuEntry(SubMenuKernelPatches(), true);
  SubScreen->AddMenuInfo(L"----------------------");
  SubScreen->AddMenuItemInput(46,  "AppleIntelCPUPM Patch", FALSE);
  SubScreen->AddMenuItemInput(47,  "AppleRTC Patch", FALSE);
  SubScreen->AddMenuItemInput(61,  "Dell SMBIOS Patch", FALSE);
//  SubScreen->AddMenuItemInput(115, "No Caches", FALSE);
//  SubScreen->AddMenuItemInput(44,  "Kext patching allowed", FALSE);
  SubScreen->AddMenuEntry(SubMenuKextPatches(), true);
  SubScreen->AddMenuInfo(L"----------------------");
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
      //MsgLog("adding to menu %a (%x) %a (%lx) L=%d(0x%x)\n",
      //       sign, DropTable->Signature,
      //       OTID, DropTable->TableId,
      //       DropTable->Length, DropTable->Length);
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
      InputBootArgs->Title = PoolPrint(L"Drop \"%4.4a\" \"%8.8a\" %d", sign, OTID, DropTable->Length);
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

  //SubScreen->AddMenuInfoLine(L"PATCHED AML:");
  if (ACPIPatchedAML) {
    ACPI_PATCHED_AML *ACPIPatchedAMLTmp = ACPIPatchedAML;
    while (ACPIPatchedAMLTmp) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_INPUT_DIALOG;
      InputBootArgs->Title = PoolPrint(L"Drop \"%s\"", ACPIPatchedAMLTmp->FileName);
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

  SubScreen->AddMenuInfoLine(PoolPrint(L"%a", gCPUStructure.BrandString));
  SubScreen->AddMenuInfoLine(PoolPrint(L"%a", gSettings.OEMProduct));
  SubScreen->AddMenuInfoLine(PoolPrint(L"with board %a", gSettings.OEMBoard));

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
  //  Entry->Title = PoolPrint(L"Drop OEM _DSM [0x%04x]->", gSettings.DropOEM_DSM);

  // submenu description
  SubScreen->AddMenuInfoLine(PoolPrint(L"Choose devices to drop OEM _DSM methods from DSDT"));

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
  //  Entry->Title = PoolPrint(L"DSDT fix mask [0x%08x]->", gSettings.FixDsdt);

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
    InputBootArgs->Title = PoolPrint(L"%a", gSettings.PatchDsdtLabel[Index]);
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

  SubScreen->AddMenuInfoLine(L"Select a DSDT file:");
  SubScreen->AddMenuItemSwitch(116,  "BIOS.aml", FALSE);

  for (i = 0; i < DsdtsNum; i++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title = PoolPrint(L"%s", DsdtsList[i]);
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
  SubScreen->AddMenuInfoLine(PoolPrint(L"Choose options to patch ACPI"));

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

  SubScreen->AddMenuInfoLine(L"Select an audio output, press F7 to test");
  SubScreen->AddMenuItemInput(120, "Volume:", TRUE);

  for (i = 0; i < AudioNum; i++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title = PoolPrint(L"%s_%a", AudioList[i].Name, AudioOutputNames[AudioList[i].Device]);
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
	InputBootArgs->Title = PoolPrint(L"  key: %a", Prop->Key);
//	InputBootArgs->Tag = TAG_INPUT;
	InputBootArgs->Row = 0xFFFF; //cursor
									   //     InputBootArgs->Item = ADDRESS_OF(DEV_PROPERTY, Prop, INPUT_ITEM, MenuItem);
	InputBootArgs->Item = &Prop->MenuItem;
	InputBootArgs->AtClick = ActionEnter;
	InputBootArgs->AtRightClick = ActionDetails;
	SubScreen->AddMenuEntry(InputBootArgs, true);
	switch (Prop->ValueType) {
	case kTagTypeInteger:
		SubScreen->AddMenuInfo(PoolPrint(L"     value: 0x%08x", *(UINT64*)Prop->Value));
		break;
	case kTagTypeString:
		SubScreen->AddMenuInfo(PoolPrint(L"     value: %30a", Prop->Value));
		break;
	case   kTagTypeFalse:
		SubScreen->AddMenuInfo(PoolPrint(L"     value: false"));
		break;
	case   kTagTypeTrue:
		SubScreen->AddMenuInfo(PoolPrint(L"     value: true"));
		break;

	default: //type data, print first 24 bytes
			 //CHAR8* Bytes2HexStr(UINT8 *data, UINTN len)
		SubScreen->AddMenuInfo(PoolPrint(L"     value[%d]: %24a", Prop->ValueLen, Bytes2HexStr((UINT8*)Prop->Value, MIN(24, Prop->ValueLen))));
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
			SubScreen->AddMenuInfo(L"------------");
			SubScreen->AddMenuInfo(PoolPrint(L"%a", Prop->Label));
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
        SubScreen->AddMenuInfo(L"------------");
        SubScreen->AddMenuInfo(PoolPrint(L"%a", Prop->Label));
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

  SubScreen->AddMenuInfoLine(L"Installed themes:");
  //add embedded
  SubScreen->AddMenuItemSwitch(3,  "embedded", FALSE);

  for (i = 0; i < ThemesNum; i++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title = PoolPrint(L"%s", ThemesList[i]);
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
  SubScreen->AddMenuInfoLine(PoolPrint(L"Choose options to tune the Interface"));

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
  SubScreen->AddMenuInfoLine(PoolPrint(L"Modify the System Integrity Protection configuration."));
  SubScreen->AddMenuInfoLine(PoolPrint(L"All configuration changes apply to the entire machine."));

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
//  Entry->Title = PoolPrint(L"boot_args->flags [0x%02x]->", gSettings.BooterConfig);

  // submenu description
  SubScreen->AddMenuInfoLine(PoolPrint(L"Modify flags for boot.efi"));

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
  SubScreen->AddMenuInfoLine(PoolPrint(L"Choose options for booted OS"));

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

  SubScreen->AddMenuInfoLine(L"Select a config file:");

  for (i = 0; i < ConfigsNum; i++) {
//    InputBootArgs = (__typeof__(InputBootArgs))AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title = PoolPrint(L"%s", ConfigsList[i]);
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
  BOOLEAN             OldFontStyle = GlobalConfig.Proportional;

  GlobalConfig.Proportional = FALSE; //temporary disable proportional

  if (AllowGraphicsMode) {
    Style = &REFIT_MENU_SCREEN::GraphicsMenuStyle;
  }

  // remember, if you extended this menu then change procedures
  // FillInputs and ApplyInputs

  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    OptionMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_OPTIONS);
  } else {
    OptionMenu.TitleImage = NULL;
  }

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
  GlobalConfig.Proportional = OldFontStyle;
  ApplyInputs();
}

UINT32 EncodeOptions(CONST CHAR16 *Options)
{
  UINT32 OptionsBits = 0;
  INTN Index;
  if (!Options) {
    return 0;
  }
  for (Index = 0; Index < NUM_OPT; Index++) {
    if (StrStr(Options, ArgOptional[Index])) {
      OptionsBits |= (1 << Index);
      if (Index == 1) {
        OptionsBits &= ~1;
      }
    }
  }
  return OptionsBits;
}

VOID DecodeOptions(REFIT_MENU_ITEM_BOOTNUM *Entry)
{
  //set checked option
  INTN Index;
  if (!Entry) {
    return;
  }
  for (Index = 0; Index < INX_NVWEBON; Index++) { //not including INX_NVWEBON
    if (gSettings.OptionsBits & (1 << Index)) {
      Entry->LoadOptions = AddLoadOption(Entry->LoadOptions, ArgOptional[Index]);
    }
  }
  //remove unchecked options
  for (Index = 0; Index < INX_NVWEBON; Index++) { //not including INX_NVWEBON
    if ((gSettings.OptionsBits & (1 << Index)) == 0) {
      Entry->LoadOptions = RemoveLoadOption(Entry->LoadOptions, ArgOptional[Index]);
    }
  }

  if (Entry->getLOADER_ENTRY()) {
    LOADER_ENTRY* loaderEntry = Entry->getLOADER_ENTRY();
    // Only for non-legacy entries, as LEGACY_ENTRY doesn't have OSVersion
    if (gSettings.OptionsBits & OPT_NVWEBON) {
      if (AsciiOSVersionToUint64(loaderEntry->OSVersion) >= AsciiOSVersionToUint64("10.12")) {
        gSettings.NvidiaWeb = TRUE;
      } else {
        Entry->LoadOptions = AddLoadOption(loaderEntry->LoadOptions, ArgOptional[INX_NVWEBON]);
      }
    }
    if ((gSettings.OptionsBits & OPT_NVWEBON) == 0) {
      if (AsciiOSVersionToUint64(loaderEntry->OSVersion) >= AsciiOSVersionToUint64("10.12")) {
        gSettings.NvidiaWeb = FALSE;
      } else {
        Entry->LoadOptions = RemoveLoadOption(loaderEntry->LoadOptions, ArgOptional[INX_NVWEBON]);
      }
    }
  }
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
    if (GlobalConfig.VerticalLayout) {
      MainStyle = &REFIT_MENU_SCREEN::MainMenuVerticalStyle;
    } else {
      MainStyle = &REFIT_MENU_SCREEN::MainMenuStyle;
    }
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
//      DBG("main OptionsBits = 0x%x\n", gSettings.OptionsBits);
      if ( MainChosenEntry->getLOADER_ENTRY() ) gSettings.OptionsBits |= EncodeOptions(MainChosenEntry->getLOADER_ENTRY()->LoadOptions);
//      DBG("add OptionsBits = 0x%x\n", gSettings.OptionsBits);
      if ( MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM() ) DecodeOptions(MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM());
      //      DBG(" enter menu with LoadOptions: %s\n", ((LOADER_ENTRY*)MainChosenEntry)->LoadOptions);
      if (MainChosenEntry->getLOADER_ENTRY()) {
        // Only for non-legacy entries, as LEGACY_ENTRY doesn't have Flags
        gSettings.FlagsBits = MainChosenEntry->getLOADER_ENTRY()->Flags;
      }
 //          DBG(" MainChosenEntry with FlagsBits = 0x%x\n", gSettings.FlagsBits);

      if (TmpArgs) {
        FreePool(TmpArgs);
        TmpArgs = NULL;
      }
      SubMenuExit = 0;
      while (!SubMenuExit) {
        //running details menu
        SubMenuExit = MainChosenEntry->SubScreen->RunGenericMenu(Style, &SubMenuIndex, &TempChosenEntry);
        if ( MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM() ) DecodeOptions(MainChosenEntry->getREFIT_MENU_ITEM_BOOTNUM());
//        DBG("get OptionsBits = 0x%x\n", gSettings.OptionsBits);
//        DBG(" TempChosenEntry FlagsBits = 0x%x\n", ((LOADER_ENTRY*)TempChosenEntry)->Flags);
        if (SubMenuExit == MENU_EXIT_ESCAPE || TempChosenEntry->getREFIT_MENU_ITEM_RETURN() ) {
          SubMenuExit = MENU_EXIT_ENTER;
          MenuExit = 0;
          break;
        }
        if (MainChosenEntry->getREFIT_MENU_ENTRY_CLOVER()) {
          MainChosenEntry->getREFIT_MENU_ENTRY_CLOVER()->LoadOptions = EfiStrDuplicate(((REFIT_MENU_ENTRY_CLOVER*)TempChosenEntry)->LoadOptions);
        }
        //       DBG(" exit menu with LoadOptions: %s\n", ((LOADER_ENTRY*)MainChosenEntry)->LoadOptions);
        if (SubMenuExit == MENU_EXIT_ENTER && MainChosenEntry->getLOADER_ENTRY() && TempChosenEntry->getLOADER_ENTRY()) {
          // Only for non-legacy entries, as LEGACY_ENTRY doesn't have Flags
          MainChosenEntry->getLOADER_ENTRY()->Flags = TempChosenEntry->getLOADER_ENTRY()->Flags;
//           DBG(" get MainChosenEntry FlagsBits = 0x%x\n", ((LOADER_ENTRY*)MainChosenEntry)->Flags);
        }
        if (/*MenuExit == MENU_EXIT_ENTER &&*/ MainChosenEntry->getLOADER_ENTRY()) {
          if (MainChosenEntry->getLOADER_ENTRY()->LoadOptions) {
            AsciiSPrint(gSettings.BootArgs, 255, "%s", MainChosenEntry->getLOADER_ENTRY()->LoadOptions);
          } else {
            ZeroMem(&gSettings.BootArgs, 255);
          }
          DBG(" boot with args: %a\n", gSettings.BootArgs);
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
 //             DBG(" get NextChosenEntry FlagsBits = 0x%x\n", ((LOADER_ENTRY*)NextChosenEntry)->Flags);
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
 //                   DBG(" get DeepChosenEntry FlagsBits = 0x%x\n", ((LOADER_ENTRY*)DeepChosenEntry)->Flags);
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


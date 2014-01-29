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
#include "Version.h"

#include "egemb_back_selected_small.h"

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

#define X_IS_LEFT    64
#define X_IS_RIGHT   0
#define X_IS_CENTER  1
#define BADGE_DIMENSION 64

//#define PREBOOT_LOG L"EFI\\CLOVER\\misc\\preboot.log"
#define VBIOS_BIN L"EFI\\CLOVER\\misc\\c0000.bin"

//#define LSTR(s) L##s

// scrolling definitions
static INTN MaxItemOnScreen = -1;
REFIT_MENU_SCREEN OptionMenu  = {4, L"Options", NULL, 0, NULL, 0, NULL, 0, NULL, FALSE, FALSE, 0, 0, 0, 0,
  FILM_CENTRE, FILM_CENTRE, {0, 0, 0, 0}, NULL };
extern REFIT_MENU_ENTRY MenuEntryReturn;
extern UINTN            ThemesNum;
extern CHAR16            *ThemesList[];

INTN LayoutBannerOffset = 64;
INTN LayoutButtonOffset = 0;
INTN LayoutTextOffset = 0;
INTN LayoutMainMenuHeight = 376;
INTN LayoutAnimMoveForMenuX = 0;

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

INTN DrawTextXY(IN CHAR16 *Text, IN INTN XPos, IN INTN YPos, IN UINT8 XAlign);

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

#define ROW0_TILESIZE (144)
#define ROW1_TILESIZE (64)
#define TILE_XSPACING (8)
#define TILE_YSPACING (24)
#define ROW0_SCROLLSIZE (100)

EG_IMAGE *SelectionImages[4] = { NULL, NULL, NULL, NULL };
static EG_IMAGE *TextBuffer = NULL;

EG_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff }; //non-trasparent

static INTN row0Count, row0PosX, row0PosXRunning;
static INTN row1Count, row1PosX, row1PosXRunning;
static INTN *itemPosX = NULL;
static INTN *itemPosY = NULL;
static INTN row0PosY, row1PosY, textPosY;
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
static EG_RECT UpButton;
static EG_RECT DownButton;
static EG_RECT ScrollbarBackground;
static EG_RECT Scrollbar;
static EG_RECT ScrollbarOldPointerPlace;
static EG_RECT ScrollbarNewPointerPlace;



INPUT_ITEM *InputItems = NULL;
UINTN  InputItemsCount = 0;

BOOLEAN mGuiReady = FALSE;

UINTN RunGenericMenu(IN REFIT_MENU_SCREEN *Screen, IN MENU_STYLE_FUNC StyleFunc, IN OUT INTN *DefaultEntryIndex, OUT REFIT_MENU_ENTRY **ChosenEntry);

VOID RefillInputs(VOID)
{
  UINTN i,j; //for loops
  CHAR8 tmp[41];  
  BOOLEAN bit;
  
  tmp[40] = 0;  //make it null-terminated
  // it's safe to remove type assigning, but there'are numbers in comments
  
  InputItemsCount = 0; 
  InputItems[InputItemsCount].ItemType = ASString;  //0
  //even though Ascii we will keep value as Unicode to convert later
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, SVALUE_MAX_SIZE, L"%a ", gSettings.BootArgs);
  InputItems[InputItemsCount].ItemType = UNIString; //1
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 63, L"%s", gSettings.DsdtName);
//  InputItems[InputItemsCount].ItemType = BoolValue; //2
//  InputItems[InputItemsCount].BValue = gSettings.DropSSDT; 
//  InputItems[InputItemsCount++].SValue = gSettings.DropSSDT?L"[+]":L"[ ]"; 
  //GlobalConfig.Theme  
  InputItemsCount = 3;
  InputItems[InputItemsCount].ItemType = UNIString; //3
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 53, L"%s",
                (GlobalConfig.Theme == NULL)?L"embedded":GlobalConfig.Theme);

  InputItems[InputItemsCount].ItemType = BoolValue; //4 
  InputItems[InputItemsCount].BValue = gSettings.DropSSDT;
  InputItems[InputItemsCount++].SValue = gSettings.DropSSDT?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue;  //5
  InputItems[InputItemsCount].BValue = gSettings.GeneratePStates;
  InputItems[InputItemsCount++].SValue = gSettings.GeneratePStates?L"[+]":L"[ ]";

  InputItems[InputItemsCount].ItemType = BoolValue;  //6
  InputItems[InputItemsCount].BValue = gSettings.SlpSmiEnable;
  InputItems[InputItemsCount++].SValue = gSettings.SlpSmiEnable?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Decimal;  //7
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.PLimitDict);
  InputItems[InputItemsCount].ItemType = Decimal;  //8
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.UnderVoltStep);
  InputItems[InputItemsCount].ItemType = BoolValue; //9
  InputItems[InputItemsCount].BValue = gSettings.GenerateCStates;
  InputItems[InputItemsCount++].SValue = gSettings.GenerateCStates?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //10
  InputItems[InputItemsCount].BValue = gSettings.EnableC2;
  InputItems[InputItemsCount++].SValue = gSettings.EnableC2?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //11
  InputItems[InputItemsCount].BValue = gSettings.EnableC4;
  InputItems[InputItemsCount++].SValue = gSettings.EnableC4?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //12
  InputItems[InputItemsCount].BValue = gSettings.EnableC6;
  InputItems[InputItemsCount++].SValue = gSettings.EnableC6?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //13
  InputItems[InputItemsCount].BValue = gSettings.EnableISS;
  InputItems[InputItemsCount++].SValue = gSettings.EnableISS?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Decimal;  //14
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%d", gSettings.QPI);
  InputItems[InputItemsCount].ItemType = BoolValue; //15
  InputItems[InputItemsCount].BValue = gSettings.PatchNMI;
  InputItems[InputItemsCount++].SValue = gSettings.PatchNMI?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //16
  InputItems[InputItemsCount].BValue = gSettings.PatchVBios;
  InputItems[InputItemsCount++].SValue = gSettings.PatchVBios?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Hex;  //17
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 36, L"0x%X", gSettings.FixDsdt);
  InputItems[InputItemsCount].ItemType = Hex;  //18
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 36, L"0x%X", gSettings.BacklightLevel);
  InputItems[InputItemsCount].ItemType = Decimal;  //19
  if (gSettings.BusSpeed > 20000) {
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gSettings.BusSpeed);
  } else {
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gCPUStructure.ExternalClock);
  }
  InputItemsCount = 20;
//  InputItems[InputItemsCount].ItemType = BoolValue; //20
//  InputItems[InputItemsCount].BValue = gSettings.GraphicsInjector;
//  InputItems[InputItemsCount++].SValue = gSettings.GraphicsInjector?L"[+]":L"[ ]";
  for (i=0; i<NGFX; i++) {
    InputItems[InputItemsCount].ItemType = ASString;  //20+i*6
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%a", gGraphics[i].Model);

    if (gGraphics[i].Vendor == Ati) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount].BValue = gSettings.InjectATI;
      InputItems[InputItemsCount++].SValue = gSettings.InjectATI?L"[+]":L"[ ]";
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
//      InputItems[InputItemsCount].SValue = AllocateZeroPool(20);
      if (StrLen(gSettings.FBName) > 2) { //fool proof: cfg_name is 3 character or more.
        UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%s", gSettings.FBName);
      } else {
        UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%s", gGraphics[i].Config);
      }
    } else if (gGraphics[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount].BValue = gSettings.InjectNVidia;
      InputItems[InputItemsCount++].SValue = gSettings.InjectNVidia?L"[+]":L"[ ]";
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      for (j=0; j<8; j++) {
        AsciiSPrint((CHAR8*)&tmp[2*j], 3, "%02x", gSettings.Dcfg[j]);
      }
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 40, L"%a", tmp);
      
  //    InputItems[InputItemsCount++].SValue = PoolPrint(L"%08x",*(UINT64*)&gSettings.Dcfg[0]);
    } else /*if (gGraphics[i].Vendor == Intel) */ {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount].BValue = gSettings.InjectIntel;
      InputItems[InputItemsCount++].SValue = gSettings.InjectIntel?L"[+]":L"[ ]";
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      InputItems[InputItemsCount++].SValue = L"NA";
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
//    InputItems[InputItemsCount++].SValue = PoolPrint(L"%a", tmp);
    UnicodeSPrint(InputItems[InputItemsCount++].SValue, 84, L"%a", tmp);
    
    InputItems[InputItemsCount].ItemType = BoolValue; //25+6i
    InputItems[InputItemsCount].BValue = gGraphics[i].LoadVBios;
    InputItems[InputItemsCount++].SValue = gGraphics[i].LoadVBios?L"[+]":L"[ ]";
  }
  //and so on 
  InputItemsCount = 44; 
  InputItems[InputItemsCount].ItemType = BoolValue; //44
  InputItems[InputItemsCount].BValue = gSettings.KextPatchesAllowed;
  InputItems[InputItemsCount++].SValue = gSettings.KextPatchesAllowed?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //45
  InputItems[InputItemsCount].BValue = gSettings.KPKernelCpu;
  InputItems[InputItemsCount++].SValue = gSettings.KPKernelCpu?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //46
  InputItems[InputItemsCount].BValue = gSettings.KPAsusAICPUPM;
  InputItems[InputItemsCount++].SValue = gSettings.KPAsusAICPUPM?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //47
  InputItems[InputItemsCount].BValue = gSettings.KPAppleRTC;
  InputItems[InputItemsCount++].SValue = gSettings.KPAppleRTC?L"[+]":L"[ ]";  
  InputItems[InputItemsCount].ItemType = BoolValue; //48
  InputItems[InputItemsCount].BValue = gSettings.KPKernelPm; //KPKernelPm
  InputItems[InputItemsCount++].SValue = gSettings.KPKernelPm?L"[+]":L"[ ]";
  
  InputItems[InputItemsCount].ItemType = BoolValue; //49
  InputItems[InputItemsCount].BValue = gSettings.DropMCFG;
  InputItems[InputItemsCount++].SValue = gSettings.DropMCFG?L"[+]":L"[ ]";
  /*
  InputItems[InputItemsCount].ItemType = BoolValue; //50
  InputItems[InputItemsCount].BValue = gSettings.bDropHPET;
  InputItems[InputItemsCount++].SValue = gSettings.bDropHPET?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //51
  InputItems[InputItemsCount].BValue = gSettings.bDropECDT;
  InputItems[InputItemsCount++].SValue = gSettings.bDropECDT?L"[+]":L"[ ]";
  */
  InputItemsCount = 52;
  InputItems[InputItemsCount].ItemType = BoolValue; //52
  InputItems[InputItemsCount].BValue = gSettings.InjectEDID;
  InputItems[InputItemsCount++].SValue = gSettings.InjectEDID?L"[+]":L"[ ]"; 
  
  for (j=0; j<16; j++) {
    InputItems[InputItemsCount].ItemType = BoolValue; //53+j
    bit = (gSettings.FixDsdt & (1<<j)) != 0;
    InputItems[InputItemsCount].BValue = bit;
    InputItems[InputItemsCount++].SValue = bit?L"[+]":L"[ ]";     
  }
  
  InputItemsCount = 70;
  InputItems[InputItemsCount].ItemType = Decimal;  //70
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%d", gSettings.PointerSpeed);
  InputItems[InputItemsCount].ItemType = Decimal;  //71
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%d", gSettings.DoubleClickTime);
  InputItems[InputItemsCount].ItemType = BoolValue; //72
  InputItems[InputItemsCount].BValue   = gSettings.PointerMirror;
  InputItems[InputItemsCount++].SValue = gSettings.PointerMirror?L"[+]":L"[ ]";
  //reserve for mouse and continue 
  
  InputItemsCount = 74;
  InputItems[InputItemsCount].ItemType = BoolValue; //74
  InputItems[InputItemsCount].BValue   = gSettings.USBFixOwnership;
  InputItems[InputItemsCount++].SValue = gSettings.USBFixOwnership?L"[+]":L"[ ]";

  InputItems[InputItemsCount].ItemType = Hex;  //75
  InputItems[InputItemsCount++].SValue = PoolPrint(L"0x%04x", gSettings.C3Latency);
  InputItems[InputItemsCount].ItemType = Decimal;  //76
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%d", gSettings.EnabledCores);
  /*
  InputItems[InputItemsCount].ItemType = BoolValue; //77
  InputItems[InputItemsCount].BValue   = gSettings.bDropDMAR;
  InputItems[InputItemsCount++].SValue = gSettings.bDropDMAR?L"[+]":L"[ ]";
  */
  InputItemsCount = 78;
  InputItems[InputItemsCount].ItemType = ASString;  //78
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.ProductName);
  InputItems[InputItemsCount].ItemType = ASString;  //79
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.VersionNr);
  InputItems[InputItemsCount].ItemType = ASString;  //80
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.SerialNr);
  InputItems[InputItemsCount].ItemType = ASString;  //81
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardNumber);
  InputItems[InputItemsCount].ItemType = ASString;  //82
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardSerialNumber);
  InputItems[InputItemsCount].ItemType = Decimal;  //83
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.BoardType);
  InputItems[InputItemsCount].ItemType = ASString;  //84
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardVersion);
  InputItems[InputItemsCount].ItemType = Decimal;  //85
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.ChassisType);
  InputItems[InputItemsCount].ItemType = ASString;  //86
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.RomVersion);
  InputItems[InputItemsCount].ItemType = ASString;  //87
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.ReleaseDate);

  InputItems[InputItemsCount].ItemType = BoolValue; //88
  InputItems[InputItemsCount].BValue   = gSettings.DoubleFirstState;
  InputItems[InputItemsCount++].SValue = gSettings.DoubleFirstState?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //89
  InputItems[InputItemsCount].BValue = gSettings.EnableC7;
  InputItems[InputItemsCount++].SValue = gSettings.EnableC7?L"[+]":L"[ ]";
 
  InputItems[InputItemsCount].ItemType = UNIString; //90
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%s", gSettings.ConfigName);
  
  InputItems[InputItemsCount].ItemType = ASString; //91
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a",
                gSettings.LogEveryBoot ? gSettings.LogEveryBoot : "");
  InputItems[InputItemsCount].ItemType = Decimal;  //92
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%d", gSettings.LogLineCount);
  InputItems[InputItemsCount].ItemType = ASString;  //93
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 76, L"%a",
                gSettings.MountEFI ? gSettings.MountEFI : "");
  InputItems[InputItemsCount].ItemType = BoolValue; //94
  InputItems[InputItemsCount].BValue = gSettings.KPLapicPanic;
  InputItems[InputItemsCount++].SValue = gSettings.KPLapicPanic?L"[+]":L"[ ]";

  InputItems[InputItemsCount].ItemType = BoolValue; //95
  InputItems[InputItemsCount].BValue   = gSettings.USBInjection;
  InputItems[InputItemsCount++].SValue = gSettings.USBInjection?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //96
  InputItems[InputItemsCount].BValue   = gSettings.InjectClockID;
  InputItems[InputItemsCount++].SValue = gSettings.InjectClockID?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Hex;  //97
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeATI);
  InputItems[InputItemsCount].ItemType = Hex;  //98
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeNVidia);
  InputItems[InputItemsCount].ItemType = Hex;  //99
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeIntel);
  InputItems[InputItemsCount].ItemType = Hex;  //100
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeLAN);
  InputItems[InputItemsCount].ItemType = Hex;  //101
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeWIFI);
  InputItems[InputItemsCount].ItemType = Hex;  //102
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeSATA);
  InputItems[InputItemsCount].ItemType = Hex;  //103
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeXHCI);
  InputItems[InputItemsCount].ItemType = Hex;  //104
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%04X", gSettings.DropOEM_DSM);
  
/*  InputItems[InputItemsCount].ItemType = BoolValue; //104
  InputItems[InputItemsCount].BValue   = gSettings.DropOEM_DSM;
  InputItems[InputItemsCount++].SValue = gSettings.DropOEM_DSM?L"[+]":L"[ ]"; */
  InputItems[InputItemsCount].ItemType = BoolValue; //105
  InputItems[InputItemsCount].BValue   = gSettings.DebugDSDT;
  InputItems[InputItemsCount++].SValue = gSettings.DebugDSDT?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Hex;  //106
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeIMEI);
  
  InputItemsCount = 110;
  for (j=0; j<16; j++) {
    InputItems[InputItemsCount].ItemType = BoolValue; //110+j
    bit = (gSettings.FixDsdt & (1<<(j+16))) != 0;
    InputItems[InputItemsCount].BValue = bit;
    InputItems[InputItemsCount++].SValue = bit?L"[+]":L"[ ]";     
  }
  
  //menu for drop table
  if (gSettings.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
    while (DropTable) {
      DropTable->MenuItem.ItemType = BoolValue;
      DropTable->MenuItem.SValue = DropTable->MenuItem.BValue?L"[+]":L"[ ]";
      DropTable = DropTable->Next;
    }
  }
}

VOID FillInputs(VOID)
{
  UINTN i,j; //for loops
  CHAR8 tmp[41];
  BOOLEAN bit;
  
  tmp[40] = 0;  //make it null-terminated
  
  InputItemsCount = 0;
  InputItems = AllocateZeroPool(128 * sizeof(INPUT_ITEM)); //XXX
  InputItems[InputItemsCount].ItemType = ASString;  //0
  //even though Ascii we will keep value as Unicode to convert later
  InputItems[InputItemsCount].SValue = AllocateZeroPool(SVALUE_MAX_SIZE);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, SVALUE_MAX_SIZE, L"%a ", gSettings.BootArgs);
  InputItems[InputItemsCount].ItemType = UNIString; //1
  InputItems[InputItemsCount].SValue = AllocateZeroPool(63);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 63, L"%s", gSettings.DsdtName); // 1-> 2
  InputItems[InputItemsCount++].ItemType = BoolValue; 
// 2 - reserved
  InputItemsCount = 3;
  InputItems[InputItemsCount].ItemType = UNIString; //3
  InputItems[InputItemsCount].SValue = AllocateZeroPool(53);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 53, L"%s",
                (GlobalConfig.Theme == NULL)?L"embedded":GlobalConfig.Theme);

  InputItems[InputItemsCount].ItemType = BoolValue; //4
  InputItems[InputItemsCount].BValue = gSettings.DropSSDT;
  InputItems[InputItemsCount++].SValue = gSettings.DropSSDT?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue;  //5
  InputItems[InputItemsCount].BValue = gSettings.GeneratePStates;
  InputItems[InputItemsCount++].SValue = gSettings.GeneratePStates?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue;  //6
  InputItems[InputItemsCount].BValue = gSettings.SlpSmiEnable;
  InputItems[InputItemsCount++].SValue = gSettings.SlpSmiEnable?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Decimal;  //7
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.PLimitDict);
  InputItems[InputItemsCount].ItemType = Decimal;  //8
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.UnderVoltStep);
  InputItems[InputItemsCount].ItemType = BoolValue; //9
  InputItems[InputItemsCount].BValue = gSettings.GenerateCStates;
  InputItems[InputItemsCount++].SValue = gSettings.GenerateCStates?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //10
  InputItems[InputItemsCount].BValue = gSettings.EnableC2;
  InputItems[InputItemsCount++].SValue = gSettings.EnableC2?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //11
  InputItems[InputItemsCount].BValue = gSettings.EnableC4;
  InputItems[InputItemsCount++].SValue = gSettings.EnableC4?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //12
  InputItems[InputItemsCount].BValue = gSettings.EnableC6;
  InputItems[InputItemsCount++].SValue = gSettings.EnableC6?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //13
  InputItems[InputItemsCount].BValue = gSettings.EnableISS;
  InputItems[InputItemsCount++].SValue = gSettings.EnableISS?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Decimal;  //14
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gSettings.QPI);
  InputItems[InputItemsCount].ItemType = BoolValue; //15
  InputItems[InputItemsCount].BValue = gSettings.PatchNMI;
  InputItems[InputItemsCount++].SValue = gSettings.PatchNMI?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //16
  InputItems[InputItemsCount].BValue = gSettings.PatchVBios;
  InputItems[InputItemsCount++].SValue = gSettings.PatchVBios?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Hex;  //17
  InputItems[InputItemsCount].SValue = AllocateZeroPool(36);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 36, L"0x%X", gSettings.FixDsdt);
  InputItems[InputItemsCount].ItemType = Hex;  //18
  InputItems[InputItemsCount].SValue = AllocateZeroPool(36);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 36, L"0x%X", gSettings.BacklightLevel);
  InputItems[InputItemsCount].ItemType = Decimal;  //19
  if (gSettings.BusSpeed > 20000) {
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gSettings.BusSpeed);
  } else {
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gCPUStructure.ExternalClock);
  }
  InputItemsCount = 20;
  //  InputItems[InputItemsCount].ItemType = BoolValue; //20
  //  InputItems[InputItemsCount].BValue = gSettings.GraphicsInjector;
  //  InputItems[InputItemsCount++].SValue = gSettings.GraphicsInjector?L"[+]":L"[ ]";
  for (i=0; i<NGFX; i++) {
    InputItems[InputItemsCount].ItemType = ASString;  //20+i*6
    InputItems[InputItemsCount++].SValue = PoolPrint(L"%a", gGraphics[i].Model);
    
    if (gGraphics[i].Vendor == Ati) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount].BValue = gSettings.InjectATI;
      InputItems[InputItemsCount++].SValue = gSettings.InjectATI?L"[+]":L"[ ]";
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      InputItems[InputItemsCount].SValue = AllocateZeroPool(20);
      if (StrLen(gSettings.FBName) > 2) { //fool proof: cfg_name is 3 character or more.
        UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%s", gSettings.FBName);
      } else {
        UnicodeSPrint(InputItems[InputItemsCount++].SValue, 20, L"%s", gGraphics[i].Config);
      }
    } else if (gGraphics[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount].BValue = gSettings.InjectNVidia;
      InputItems[InputItemsCount++].SValue = gSettings.InjectNVidia?L"[+]":L"[ ]";
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      for (j=0; j<8; j++) {
        AsciiSPrint((CHAR8*)&tmp[2*j], 3, "%02x", gSettings.Dcfg[j]);
      }
      InputItems[InputItemsCount].SValue = AllocateZeroPool(40);
      UnicodeSPrint(InputItems[InputItemsCount++].SValue, 40, L"%a", tmp);
      
      //    InputItems[InputItemsCount++].SValue = PoolPrint(L"%08x",*(UINT64*)&gSettings.Dcfg[0]);
    } else /*if (gGraphics[i].Vendor == Intel) */ {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount].BValue = gSettings.InjectIntel;
      InputItems[InputItemsCount++].SValue = gSettings.InjectIntel?L"[+]":L"[ ]";
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      InputItems[InputItemsCount++].SValue = L"NA";
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
    InputItems[InputItemsCount].SValue = AllocateZeroPool(84);
    UnicodeSPrint(InputItems[InputItemsCount++].SValue, 84, L"%a", tmp);
    
    InputItems[InputItemsCount].ItemType = BoolValue; //25+6i
    InputItems[InputItemsCount].BValue = gGraphics[i].LoadVBios;
    InputItems[InputItemsCount++].SValue = gGraphics[i].LoadVBios?L"[+]":L"[ ]";
  }
  //and so on
  
  InputItemsCount = 44;
  InputItems[InputItemsCount].ItemType = BoolValue; //44
  InputItems[InputItemsCount].BValue = gSettings.KextPatchesAllowed;
  InputItems[InputItemsCount++].SValue = gSettings.KextPatchesAllowed?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //45
  InputItems[InputItemsCount].BValue = gSettings.KPKernelCpu;
  InputItems[InputItemsCount++].SValue = gSettings.KPKernelCpu?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //46
  InputItems[InputItemsCount].BValue = gSettings.KPAsusAICPUPM;
  InputItems[InputItemsCount++].SValue = gSettings.KPAsusAICPUPM?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //47
  InputItems[InputItemsCount].BValue = gSettings.KPAppleRTC;
  InputItems[InputItemsCount++].SValue = gSettings.KPAppleRTC?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //48
  InputItems[InputItemsCount].BValue = gSettings.KPKernelPm;
  InputItems[InputItemsCount++].SValue = gSettings.KPKernelPm?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //49
  InputItems[InputItemsCount].BValue = gSettings.DropMCFG;
  InputItems[InputItemsCount++].SValue = gSettings.DropMCFG?L"[+]":L"[ ]";
  /*
  InputItems[InputItemsCount].ItemType = BoolValue; //50
  InputItems[InputItemsCount].BValue = gSettings.bDropHPET;
  InputItems[InputItemsCount++].SValue = gSettings.bDropHPET?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //51
  InputItems[InputItemsCount].BValue = gSettings.bDropECDT;
  InputItems[InputItemsCount++].SValue = gSettings.bDropECDT?L"[+]":L"[ ]";
  */
  InputItemsCount = 52;
  InputItems[InputItemsCount].ItemType = BoolValue; //52
  InputItems[InputItemsCount].BValue = gSettings.InjectEDID;
  InputItems[InputItemsCount++].SValue = gSettings.InjectEDID?L"[+]":L"[ ]";
  
  for (j=0; j<16; j++) {
    InputItems[InputItemsCount].ItemType = BoolValue; //53+j
    bit = (gSettings.FixDsdt & (1<<j)) != 0;
    InputItems[InputItemsCount].BValue = bit;
    InputItems[InputItemsCount++].SValue = bit?L"[+]":L"[ ]";
  }
  
  InputItemsCount = 70;
  InputItems[InputItemsCount].ItemType = Decimal;  //70
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.PointerSpeed);
  InputItems[InputItemsCount].ItemType = Decimal;  //71
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%04d", gSettings.DoubleClickTime);
  InputItems[InputItemsCount].ItemType = BoolValue; //72
  InputItems[InputItemsCount].BValue   = gSettings.PointerMirror;
  InputItems[InputItemsCount++].SValue = gSettings.PointerMirror?L"[+]":L"[ ]";
  //reserve for mouse and continue
  
  InputItemsCount = 74;
  InputItems[InputItemsCount].ItemType = BoolValue; //74
  InputItems[InputItemsCount].BValue   = gSettings.USBFixOwnership;
  InputItems[InputItemsCount++].SValue = gSettings.USBFixOwnership?L"[+]":L"[ ]";

  InputItems[InputItemsCount].ItemType = Hex;  //75
  InputItems[InputItemsCount++].SValue = PoolPrint(L"0x%04x", gSettings.C3Latency);
  InputItems[InputItemsCount].ItemType = Decimal;  //76
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%02d", gSettings.EnabledCores);
  /*
  InputItems[InputItemsCount].ItemType = BoolValue; //77
  InputItems[InputItemsCount].BValue   = gSettings.bDropDMAR;
  InputItems[InputItemsCount++].SValue = gSettings.bDropDMAR?L"[+]":L"[ ]";
  */
  InputItemsCount = 78;
  InputItems[InputItemsCount].ItemType = ASString;  //78
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.ProductName);
  InputItems[InputItemsCount].ItemType = ASString;  //79
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.VersionNr);
  InputItems[InputItemsCount].ItemType = ASString;  //80
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.SerialNr);
  InputItems[InputItemsCount].ItemType = ASString;  //81
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardNumber);
  InputItems[InputItemsCount].ItemType = ASString;  //82
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardSerialNumber);
  InputItems[InputItemsCount].ItemType = Decimal;  //83
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.BoardType);
  InputItems[InputItemsCount].ItemType = ASString;  //84
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.BoardVersion);
  InputItems[InputItemsCount].ItemType = Decimal;  //85
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%d", gSettings.ChassisType);
  InputItems[InputItemsCount].ItemType = ASString;  //86
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.RomVersion);
  InputItems[InputItemsCount].ItemType = ASString;  //87
  InputItems[InputItemsCount].SValue = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a", gSettings.ReleaseDate);
  
  InputItems[InputItemsCount].ItemType = BoolValue; //88
  InputItems[InputItemsCount].BValue   = gSettings.DoubleFirstState;
  InputItems[InputItemsCount++].SValue = gSettings.DoubleFirstState?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //89
  InputItems[InputItemsCount].BValue = gSettings.EnableC7;
  InputItems[InputItemsCount++].SValue = gSettings.EnableC7?L"[+]":L"[ ]";

  InputItems[InputItemsCount].ItemType = UNIString; //90
  InputItems[InputItemsCount].SValue   = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%s", gSettings.ConfigName);

  InputItems[InputItemsCount].ItemType = ASString; //91
  InputItems[InputItemsCount].SValue   = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 64, L"%a",
                gSettings.LogEveryBoot ? gSettings.LogEveryBoot : "");
  InputItems[InputItemsCount].ItemType = Decimal;  //92
  InputItems[InputItemsCount++].SValue = PoolPrint(L"%06d", gSettings.LogLineCount);
  InputItems[InputItemsCount].ItemType = ASString;  //93
  InputItems[InputItemsCount].SValue   = AllocateZeroPool(64);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 76, L"%a",
                gSettings.MountEFI ? gSettings.MountEFI : "");
  InputItems[InputItemsCount].ItemType = BoolValue; //94
  InputItems[InputItemsCount].BValue = gSettings.KPLapicPanic;
  InputItems[InputItemsCount++].SValue = gSettings.KPLapicPanic?L"[+]":L"[ ]";

  InputItems[InputItemsCount].ItemType = BoolValue; //95
  InputItems[InputItemsCount].BValue   = gSettings.USBInjection;
  InputItems[InputItemsCount++].SValue = gSettings.USBInjection?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = BoolValue; //96
  InputItems[InputItemsCount].BValue   = gSettings.InjectClockID;
  InputItems[InputItemsCount++].SValue = gSettings.InjectClockID?L"[+]":L"[ ]";
  
  InputItems[InputItemsCount].ItemType = Hex;  //97
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeATI);
  InputItems[InputItemsCount].ItemType = Hex;  //98
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeNVidia);
  InputItems[InputItemsCount].ItemType = Hex;  //99
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeIntel);
  
  InputItems[InputItemsCount].ItemType = Hex;  //100
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeLAN);
  InputItems[InputItemsCount].ItemType = Hex;  //101
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeWIFI);
  InputItems[InputItemsCount].ItemType = Hex;  //102
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeSATA);
  InputItems[InputItemsCount].ItemType = Hex;  //103
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeXHCI);
  InputItems[InputItemsCount].ItemType = Hex;  //104
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%04X", gSettings.DropOEM_DSM);
/*  InputItems[InputItemsCount].ItemType = BoolValue; //104
  InputItems[InputItemsCount].BValue   = gSettings.DropOEM_DSM;
  InputItems[InputItemsCount++].SValue = gSettings.DropOEM_DSM?L"[+]":L"[ ]";*/
  InputItems[InputItemsCount].ItemType = BoolValue; //105
  InputItems[InputItemsCount].BValue   = gSettings.DebugDSDT;
  InputItems[InputItemsCount++].SValue = gSettings.DebugDSDT?L"[+]":L"[ ]";
  InputItems[InputItemsCount].ItemType = Hex;  //106
  InputItems[InputItemsCount].SValue = AllocateZeroPool(26);
  UnicodeSPrint(InputItems[InputItemsCount++].SValue, 26, L"0x%08X", gSettings.FakeIMEI);
  
  InputItemsCount = 110;
  for (j=0; j<16; j++) {
    InputItems[InputItemsCount].ItemType = BoolValue; //110+j
    bit = (gSettings.FixDsdt & (1<<(j+16))) != 0;
    InputItems[InputItemsCount].BValue = bit;
    InputItems[InputItemsCount++].SValue = bit?L"[+]":L"[ ]";     
  }
  
  //menu for drop table
  if (gSettings.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
    while (DropTable) {
      DropTable->MenuItem.ItemType = BoolValue;
      DropTable->MenuItem.SValue = DropTable->MenuItem.BValue?L"[+]":L"[ ]";
      DropTable = DropTable->Next;
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
  CHAR8  AString[256];
    TagPtr dict;
//  DBG("ApplyInputs\n");
  if (InputItems[i].Valid) {
    gBootArgsChanged = TRUE;
    AsciiSPrint(gSettings.BootArgs, 255, "%s ", InputItems[i].SValue);
  }
  i++; //1
  if (InputItems[i].Valid) {
    UnicodeSPrint(gSettings.DsdtName, 120, L"%s", InputItems[i].SValue);    
  }
  i++; //2
  if (InputItems[i].Valid) {
//    gSettings.iCloudFix = InputItems[i].BValue;
  }
  i++; //3
  if (InputItems[i].Valid) {
//    gSettings.StringInjector = InputItems[i].BValue;
    if (GlobalConfig.Theme) {
      FreePool(GlobalConfig.Theme);
    }
    GlobalConfig.Theme = PoolPrint(L"%s", InputItems[i].SValue);
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
 //   DBG("Apply ProcessorInterconnectSpeed=%d\n", gSettings.QPI);
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
/*  if (InputItems[i].Valid) {
    gSettings.FixDsdt = (UINT32)StrHexToUint64(InputItems[i].SValue);
  } */
  i++; //18
  if (InputItems[i].Valid) {
    gSettings.BacklightLevel = (UINT16)StrHexToUint64(InputItems[i].SValue);
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
        ZeroMem(AString, 255);
        AsciiSPrint(AString, 255, "%s", InputItems[i].SValue);
        hex2bin(AString, (UINT8*)&gSettings.Dcfg[0], 8);
      } /* else if (gGraphics[j].Vendor == Intel) {
        //nothing to do
      } */
    }
    i++; //23
    if (InputItems[i].Valid) {
      gGraphics[j].Ports = (UINT8)(StrDecimalToUintn(InputItems[i].SValue) & 0x0F);
    }    
    i++; //24
    if (InputItems[i].Valid) {
      ZeroMem(AString, 255);
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
  i = 44;
  if (InputItems[i].Valid) {
    gSettings.KextPatchesAllowed = InputItems[i].BValue;
  }  
  i++; //45
  if (InputItems[i].Valid) {
    gSettings.KPKernelCpu = InputItems[i].BValue;
  }
  i++; //46
  if (InputItems[i].Valid) {
    gSettings.KPAsusAICPUPM = InputItems[i].BValue;
  }
  i++; //47
  if (InputItems[i].Valid) {
    gSettings.KPAppleRTC = InputItems[i].BValue;
  }
  if (gSettings.KPAsusAICPUPM || gSettings.KPAppleRTC || (gSettings.KPATIConnectorsPatch != NULL)) {
    gSettings.KPKextPatchesNeeded = TRUE;
  }
  i++; //48
  if (InputItems[i].Valid) {
    gSettings.KPKernelPm = InputItems[i].BValue;
  }
  i++; //49
  if (InputItems[i].Valid) {
    gSettings.DropMCFG = InputItems[i].BValue;
  }
  /*
  i++; //50
  if (InputItems[i].Valid) {
    gSettings.bDropHPET = InputItems[i].BValue;
  }
  i++; //51
  if (InputItems[i].Valid) {
    gSettings.bDropECDT = InputItems[i].BValue;
  }
  */
  i=52; //52
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
  /*
  i++; //77
  if (InputItems[i].Valid) {
    gSettings.bDropDMAR = InputItems[i].BValue;
  }
  */
  i=78; //78
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.ProductName, 64, "%s", InputItems[i].SValue);
    // let's fill all other fields based on this ProductName
    // to serve as default
    Model = GetModelFromString(gSettings.ProductName);
    if (Model != MaxMachineType) {
      SetDMISettingsForModel(Model);
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
      if (StrCmp(InputItems[i].SValue, gSettings.ConfigName) != 0) {
          if ((StrLen(InputItems[i].SValue) == 0) ||
              (StrCmp(InputItems[i].SValue, gSettings.MainConfigName) == 0)) {
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
              LoadUserSettings(SelfRootDir, InputItems[i].SValue, &dict);
              Status = GetUserSettings(SelfRootDir, dict);
              if (!EFI_ERROR(Status)) {
                  if (gConfigDict[2]) FreeTag(gConfigDict[2]);
                  gConfigDict[2] = dict;
                  if (gSettings.ConfigName) FreePool(gSettings.ConfigName);
                  gSettings.ConfigName = EfiStrDuplicate(InputItems[i].SValue);
              }
              DBG("Main settings3 from menu: %r\n", Status);
          }
    //    if (!EFI_ERROR(Status)) {
      RefillInputs();
      NeedSave = FALSE;
    }
//    return; //do not double SaveSettings() as it done by GetUserSettings()
  }
  i++; //91
  if (InputItems[i].Valid) {
    AsciiSPrint(gSettings.LogEveryBoot, 64, "%s", InputItems[i].SValue);
  }
  i++; //92
  if (InputItems[i].Valid) {
    gSettings.LogLineCount = (UINT32)StrDecimalToUintn(InputItems[i].SValue);
  }    
  i++; //93
  if (InputItems[i].Valid) {
    //we must reallocate MountEFI
    if (gSettings.MountEFI) {
      FreePool(gSettings.MountEFI);
    }
    gSettings.MountEFI = AllocateZeroPool(38); // make the room for at least a UUID
    AsciiSPrint(gSettings.MountEFI, 38, "%s", InputItems[i].SValue);
  }    
  i++; //94
  if (InputItems[i].Valid) {
    gSettings.KPLapicPanic = InputItems[i].BValue;
  }
  i++; //95
  if (InputItems[i].Valid) {
    gSettings.USBInjection = InputItems[i].BValue;
  }
  i++; //96
  if (InputItems[i].Valid) {
    gSettings.InjectClockID = InputItems[i].BValue;
  }
  i++; //97
  if (InputItems[i].Valid) {
    gSettings.FakeATI = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //98
  if (InputItems[i].Valid) {
    gSettings.FakeNVidia = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //99
  if (InputItems[i].Valid) {
    gSettings.FakeIntel = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //99
  if (InputItems[i].Valid) {
    gSettings.FakeIntel = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //100
  if (InputItems[i].Valid) {
    gSettings.FakeLAN = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //101
  if (InputItems[i].Valid) {
    gSettings.FakeWIFI = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //102
  if (InputItems[i].Valid) {
    gSettings.FakeSATA = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }
  i++; //103
  if (InputItems[i].Valid) {
    gSettings.FakeXHCI = (UINT32)StrHexToUint64(InputItems[i].SValue);
  }

  i++; //104
  if (InputItems[i].Valid) {
//    gSettings.DropOEM_DSM = InputItems[i].BValue;
    gSettings.DropOEM_DSM = (UINT16)StrHexToUint64(InputItems[i].SValue);
    defDSM = TRUE;
  }
  i++; //105
  if (InputItems[i].Valid) {
    gSettings.DebugDSDT = InputItems[i].BValue;
  }
  i++; //106
  if (InputItems[i].Valid) {
    gSettings.FakeIMEI = (UINT32)StrHexToUint64(InputItems[i].SValue);
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

//
// Graphics helper functions
//

static VOID InitSelection(VOID)
{
  UINTN       x, y, src_x, src_y;
  EG_PIXEL    *DestPtr, *SrcPtr;
  
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
    SelectionImages[2] = egPrepareEmbeddedImage(&egemb_back_selected_small, FALSE);
  }
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
//    // calculate big selection image from small one
    SelectionImages[0] = egCreateImage(ROW0_TILESIZE, ROW0_TILESIZE, FALSE);
    if (SelectionImages[0] == NULL) {
      egFreeImage(SelectionImages[2]);
      SelectionImages[2] = NULL;
      return;
    }
    if (GlobalConfig.SelectionOnTop) {
      SelectionImages[0]->HasAlpha = TRUE;
      SelectionImages[2]->HasAlpha = TRUE;
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
  //TODO FALSE -> TRUE
  SelectionImages[1] = egCreateFilledImage(ROW0_TILESIZE, ROW0_TILESIZE, TRUE, &MenuBackgroundPixel);
  SelectionImages[3] = egCreateFilledImage(ROW1_TILESIZE, ROW1_TILESIZE, TRUE, &MenuBackgroundPixel);
}

//
// Scrolling functions
//
#define CONSTRAIN_MIN(Variable, MinValue) if (Variable < MinValue) Variable = MinValue
#define CONSTRAIN_MAX(Variable, MaxValue) if (Variable > MaxValue) Variable = MaxValue

static VOID InitScroll(OUT SCROLL_STATE *State, IN INTN ItemCount, IN UINTN MaxCount, IN UINTN VisibleSpace)
{
  State->LastSelection = State->CurrentSelection = 0;
  State->MaxIndex = (INTN)MaxCount - 1;
  State->MaxScroll = ItemCount - 1;
  State->FirstVisible = 0;
  
  if (VisibleSpace == 0)
    State->MaxVisible = State->MaxScroll;
  else
    State->MaxVisible = (INTN)VisibleSpace - 1;

  if (State->MaxVisible >= (INTN)ItemCount)
      State->MaxVisible = (INTN)ItemCount - 1;
  
  State->MaxFirstVisible = State->MaxScroll - State->MaxVisible;
  CONSTRAIN_MIN(State->MaxFirstVisible, 0);
  
  State->IsScrolling = (State->MaxFirstVisible > 0);
  State->PaintAll = TRUE;
  State->PaintSelection = FALSE;
  
  State->LastVisible = State->FirstVisible + State->MaxVisible;
 // DBG("InitScroll: MaxIndex=%d, FirstVisible=%d, MaxVisible=%d, MaxFirstVisible=%d\n",
  //    State->MaxIndex, State->FirstVisible, State->MaxVisible, State->MaxFirstVisible);
     //4 0 11 0
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
    if (Screen->Entries)
        FreePool(Screen->Entries);
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
  CHAR16        *Buffer;
  SCROLL_STATE  StateLine;

  UINTN         LineSize = 38;


  if (Item->ItemType != BoolValue) {
    // Grow Item->SValue to SVALUE_MAX_SIZE if we want to edit a text field
    Item->SValue = EfiReallocatePool(Item->SValue, StrSize(Item->SValue), SVALUE_MAX_SIZE);
  }
  
  Buffer = Item->SValue;

  InitScroll(&StateLine, 128, 128, StrLen(Item->SValue));
  //  MsgLog("initial SValue: %s\n", Item->SValue);

  do {

    if (Item->ItemType == BoolValue) {
      Item->BValue = !Item->BValue;
      Item->SValue = Item->BValue?L"[+] ":L"[ ] ";
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
          Status = SaveBooterLog(SelfRootDir, PREBOOT_LOG);
          if (EFI_ERROR(Status)) {
            Status = SaveBooterLog(NULL, PREBOOT_LOG);
          }
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
      }
      
      switch (key.UnicodeChar) {
        case CHAR_BACKSPACE:  
          if (Buffer[0] != CHAR_NULL && Pos != 0) {
            for (i = Pos + Item->LineShift; i < StrSize(Buffer); i++) {
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
            if (StrSize(Buffer) < SVALUE_MAX_SIZE - 1) {
              for (i = StrSize(Buffer); i >  Pos + Item->LineShift; i--) {
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
      UnicodeSPrint(Item->SValue, SVALUE_MAX_SIZE, L"%s", Backup);
      StyleFunc(Screen, State, MENU_FUNCTION_PAINT_SELECTION, NULL);
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
  //UINTN         LogSize;

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
        if ((Screen->Entries[gItemID])->Tag == TAG_INPUT){
          MenuExit = InputDialog(Screen, StyleFunc, &State);
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
        if ((Screen->Entries[gItemID])->Tag == TAG_INPUT){
          MenuExit = InputDialog(Screen, StyleFunc, &State);
        } else {
          MenuExit = MENU_EXIT_DETAILS;
        }
        //  State.CurrentSelection = Index;
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
        //      UpdateScroll(&State, SCROLL_PAGE_UP);
        SetNextScreenMode(1);
        StyleFunc(Screen, &State, MENU_FUNCTION_INIT, NULL);
        break;
      case SCAN_PAGE_DOWN:
        //        UpdateScroll(&State, SCROLL_PAGE_DOWN);
        SetNextScreenMode(-1);
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
        if ((Screen->Entries[State.CurrentSelection])->Tag == TAG_INPUT){
          MenuExit = InputDialog(Screen, StyleFunc, &State);
        } else if ((Screen->Entries[State.CurrentSelection])->Tag == TAG_CLOVER){
          MenuExit = MENU_EXIT_DETAILS;
        } else {
          MenuExit = MENU_EXIT_ENTER;
        }
        break;
      case ' ':
        if ((Screen->Entries[State.CurrentSelection])->Tag == TAG_INPUT){
          MenuExit = InputDialog(Screen, StyleFunc, &State);
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
  static UINTN MenuWidth = 0, ItemWidth = 0, MenuHeight = 0;
  static UINTN MenuPosY = 0;
  //static CHAR16 **DisplayStrings;
  CHAR16 *TimeoutMessage;
	CHAR16 ResultString[256];

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
      MenuWidth = 50;  // minimum
      for (i = 0; i <= State->MaxIndex; i++) {
        ItemWidth = StrLen(Screen->Entries[i]->Title);
        if (MenuWidth < ItemWidth)
          MenuWidth = ItemWidth;
      }
      if (MenuWidth > ConWidth - 6)
        MenuWidth = ConWidth - 6;

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
				if (Screen->Entries[i]->Tag == TAG_INPUT)
					StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->SValue);
				for (j = StrLen(ResultString); j < (INTN)MenuWidth; j++)
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
      if (Screen->Entries[State->LastSelection]->Tag == TAG_INPUT)
				StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->SValue);
			for (j = StrLen(ResultString); j < (INTN)MenuWidth; j++)
				ResultString[j] = L' ';
			ResultString[j] = 0;
			gST->ConOut->OutputString (gST->ConOut, ResultString);



			gST->ConOut->SetCursorPosition (gST->ConOut, 2, MenuPosY + (State->CurrentSelection - State->FirstVisible));
      gST->ConOut->SetAttribute (gST->ConOut, ATTR_CHOICE_CURRENT);
			StrCpy(ResultString, Screen->Entries[State->CurrentSelection]->Title);
			if (Screen->Entries[State->CurrentSelection]->Tag == TAG_INPUT)
				StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->SValue);
			for (j = StrLen(ResultString); j < (INTN)MenuWidth; j++)
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
  INTN TextWidth;
  EG_IMAGE *TextBufferXY = NULL;

  if (!Text) return 0;

  egMeasureText(Text, &TextWidth, NULL);
  TextBufferXY = egCreateImage(TextWidth, TextHeight, TRUE);

  egFillImage(TextBufferXY, &MenuBackgroundPixel);

  // render the text
  egRenderText(Text, TextBufferXY, 0, 0, 0xFFFF); //input only
  BltImageAlpha(TextBufferXY, (XPos - (TextWidth >> XAlign)), YPos,  &MenuBackgroundPixel, 16);
  egFreeImage(TextBufferXY);
  return TextWidth;
}

VOID DrawMenuText(IN CHAR16 *Text, IN INTN SelectedWidth, IN INTN XPos, IN INTN YPos, IN INTN Cursor)
{
  if (TextBuffer && (TextBuffer->Height != TextHeight)) {
    egFreeImage(TextBuffer);
    TextBuffer = NULL;
  }

  if (TextBuffer == NULL)
    TextBuffer = egCreateImage(LAYOUT_TEXT_WIDTH, TextHeight, TRUE);
  
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
  egRenderText(Text, TextBuffer, TEXT_XMARGIN, TEXT_YMARGIN, (INTN)Cursor);
  BltImageAlpha(TextBuffer, (INTN)XPos, (INTN)YPos, &MenuBackgroundPixel, 16);
}


VOID InitBar(VOID)
{
  if (ThemeDir) {
    if (!ScrollbarBackgroundImage) {
      ScrollbarBackgroundImage = egLoadImage(ThemeDir, L"scrollbar\\bar_fill.png", FALSE);
    }
    if (!BarStartImage) {
      BarStartImage = egLoadImage(ThemeDir, L"scrollbar\\bar_start.png", TRUE);
    }
    if (!BarEndImage) {
      BarEndImage = egLoadImage(ThemeDir, L"scrollbar\\bar_end.png", TRUE);
    }
    if (!ScrollbarImage) {
      ScrollbarImage = egLoadImage(ThemeDir, L"scrollbar\\scroll_fill.png", FALSE);
    }
    if (!ScrollStartImage) {
      ScrollStartImage = egLoadImage(ThemeDir, L"scrollbar\\scroll_start.png", TRUE);
    }
    if (!ScrollEndImage) {
      ScrollEndImage = egLoadImage(ThemeDir, L"scrollbar\\scroll_end.png", TRUE);
    }
    if (!UpButtonImage) {
      UpButtonImage = egLoadImage(ThemeDir, L"scrollbar\\up_button.png", TRUE);
    }
    if (!DownButtonImage) {
      DownButtonImage = egLoadImage(ThemeDir, L"scrollbar\\down_button.png", TRUE);
    }
  }
  
  if (!BarStartImage) {
    BarStartImage = egCreateFilledImage(ScrollWidth, 5, TRUE, &StdBackgroundPixel);
  }
  if (!BarEndImage) {
    BarEndImage = egCreateFilledImage(ScrollWidth, 5, TRUE, &StdBackgroundPixel);
  }
  if (!ScrollbarBackgroundImage) {
    ScrollbarBackgroundImage = egCreateFilledImage(ScrollWidth, 1, TRUE, &DarkBackgroundPixel);
  }
  if (!ScrollbarImage) {
    ScrollbarImage = egCreateFilledImage(ScrollWidth, 1, TRUE, &StdBackgroundPixel);
  }
  if (!ScrollStartImage) {
    ScrollStartImage = egCreateFilledImage(ScrollWidth, 7, TRUE, &StdBackgroundPixel);
  }
  if (!ScrollEndImage) {
    ScrollEndImage = egCreateFilledImage(ScrollWidth, 7, TRUE, &StdBackgroundPixel);
  }
  if (!UpButtonImage) {
    UpButtonImage = egCreateFilledImage(ScrollWidth, 20, TRUE, &StdBackgroundPixel);
  }
  if (!DownButtonImage) {
    DownButtonImage = egCreateFilledImage(ScrollWidth, 20, TRUE, &StdBackgroundPixel);
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
    //VOID egComposeImage(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *TopImage, IN INTN PosX, IN INTN PosY)
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
  INTN j;
  INTN ItemWidth = 0;
  INTN X;
  INTN VisibleHeight = 0; //assume vertical layout
  CHAR16 ResultString[256];
  
  switch (Function) {
      
    case MENU_FUNCTION_INIT:
      
      egGetScreenSize(&UGAWidth, &UGAHeight);
      SwitchToGraphicsAndClear();
      
 //     EntriesPosY = ((UGAHeight - LAYOUT_TOTAL_HEIGHT) >> 1) + LAYOUT_BANNER_YOFFSET + (TextHeight << 1);
      EntriesPosY = ((UGAHeight - LAYOUT_TOTAL_HEIGHT) >> 1) + LayoutBannerOffset + (TextHeight << 1);
      
      VisibleHeight = (UGAHeight - EntriesPosY) / TextHeight - Screen->InfoLineCount - 1;
      //DBG("MENU_FUNCTION_INIT 1 EntriesPosY=%d VisibleHeight=%d\n", EntriesPosY, VisibleHeight);
      InitScroll(State, Screen->EntryCount, Screen->EntryCount, VisibleHeight);
      // determine width of the menu
      MenuWidth = 50;  // minimum
      /* for (i = 0; i < (INTN)Screen->InfoLineCount; i++) {
       ItemWidth = StrLen(Screen->InfoLines[i]);
       if (MenuWidth < ItemWidth)
       MenuWidth = ItemWidth;
       }
      // DBG("MENU_FUNCTION_INIT 2\n");
      for (i = 0; i <= State->MaxIndex; i++) {
       ItemWidth = StrLen(Screen->Entries[i]->Title) + 
       StrLen(((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->SValue);
       if (MenuWidth < ItemWidth)
       MenuWidth = ItemWidth;
       } */
      
//      DBG("MENU_FUNCTION_INIT 3\n");
      MenuWidth = TEXT_XMARGIN * 2 + (MenuWidth * GlobalConfig.CharWidth); // FontWidth;
      if (MenuWidth > LAYOUT_TEXT_WIDTH)
        MenuWidth = LAYOUT_TEXT_WIDTH;
      
      if (Screen->TitleImage) {
        if (MenuWidth > (INTN)(UGAWidth - TITLEICON_SPACING - Screen->TitleImage->Width)) {
          MenuWidth = UGAWidth - TITLEICON_SPACING - Screen->TitleImage->Width - 2;
        }        
        EntriesPosX = (UGAWidth - (Screen->TitleImage->Width + TITLEICON_SPACING + MenuWidth)) >> 1;
      }
      else {
        EntriesPosX = (UGAWidth - MenuWidth) >> 1;
      }
 //     TimeoutPosY = EntriesPosY + MultU64x64((Screen->EntryCount + 1), TextHeight);
      TimeoutPosY = EntriesPosY + (Screen->EntryCount + 1) * TextHeight;
         
      // initial painting
      egMeasureText(Screen->Title, &ItemWidth, NULL);
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE)) {
        DrawMenuText(Screen->Title, 0, ((UGAWidth - ItemWidth) >> 1) - TEXT_XMARGIN, EntriesPosY - TextHeight * 2, 0xFFFF);
      }
      
      if (Screen->TitleImage) {
        Screen->FilmPlace.XPos = (INTN)(EntriesPosX - (Screen->TitleImage->Width + TITLEICON_SPACING));
        Screen->FilmPlace.YPos = (INTN)EntriesPosY;
        Screen->FilmPlace.Width = Screen->TitleImage->Width;
        Screen->FilmPlace.Height = Screen->TitleImage->Height;
        BltImageAlpha(Screen->TitleImage, Screen->FilmPlace.XPos, Screen->FilmPlace.YPos, &MenuBackgroundPixel, 16);
      }
      
      if (Screen->InfoLineCount > 0) {
        for (i = 0; i < (INTN)Screen->InfoLineCount; i++) {
          DrawMenuText(Screen->InfoLines[i], 0, EntriesPosX, EntriesPosY, 0xFFFF);
          EntriesPosY += TextHeight;
        }
        EntriesPosY += TextHeight;  // also add a blank line
      }
      InitBar();
      InitAnime(Screen);
      
      break;
      
    case MENU_FUNCTION_CLEANUP:
      HidePointer();
      break;
      
    case MENU_FUNCTION_PAINT_ALL:
      
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
          StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[i]))->Item->SValue);
          StrCat(ResultString, L" ");
          Screen->Entries[i]->Place.Width = StrLen(ResultString) * GlobalConfig.CharWidth;
          //Slice - suppose to use Row as Cursor in text
          DrawMenuText(ResultString,
                       (i == State->CurrentSelection)?(MenuWidth /* Screen->Entries[i]->Place.Width */):0,
                       EntriesPosX, Screen->Entries[i]->Place.YPos,
                       TitleLen + Screen->Entries[i]->Row);
        }
        else {
//          DBG("paint entry %d title=%s\n", i, Screen->Entries[i]->Title);
          DrawMenuText(Screen->Entries[i]->Title,
                       (i == State->CurrentSelection) ? MenuWidth : 0,
                       EntriesPosX, Screen->Entries[i]->Place.YPos, 0xFFFF);
        }
      }
      
      ScrollingBar(State);
      MouseBirth();
      break;
      
    case MENU_FUNCTION_PAINT_SELECTION:
      HidePointer();
      
      // blackosx swapped this around so drawing of selection comes before drawing scrollbar.
      
      // redraw selection cursor
      //usr-sse2
      if (Screen->Entries[State->LastSelection]->Tag == TAG_INPUT) {
        UINTN  TitleLen = StrLen(Screen->Entries[State->LastSelection]->Title);
        StrCpy(ResultString, Screen->Entries[State->LastSelection]->Title);
        StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->SValue + ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->LastSelection]))->Item->LineShift);
        StrCat(ResultString, L" ");
        DrawMenuText(ResultString, 0,
                     EntriesPosX, EntriesPosY + (State->LastSelection - State->FirstVisible) * TextHeight,
                     TitleLen + Screen->Entries[State->LastSelection]->Row);
      }
      else {
        DrawMenuText(Screen->Entries[State->LastSelection]->Title, 0,
                     EntriesPosX, EntriesPosY + (State->LastSelection - State->FirstVisible) * TextHeight, 0xFFFF);
      }
            //Current selection
      if (Screen->Entries[State->CurrentSelection]->Tag == TAG_INPUT) {
        UINTN  TitleLen = StrLen(Screen->Entries[State->CurrentSelection]->Title);
        StrCpy(ResultString, Screen->Entries[State->CurrentSelection]->Title);
        StrCat(ResultString, ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->SValue + ((REFIT_INPUT_DIALOG*)(Screen->Entries[State->CurrentSelection]))->Item->LineShift);
        StrCat(ResultString, L" ");
        DrawMenuText(ResultString, MenuWidth /* StrLen(ResultString) * GlobalConfig.CharWidth */,
                     EntriesPosX, EntriesPosY + (State->CurrentSelection - State->FirstVisible) * TextHeight,
                     TitleLen + Screen->Entries[State->CurrentSelection]->Row);
      }
      else {
        DrawMenuText(Screen->Entries[State->CurrentSelection]->Title, MenuWidth,
                     EntriesPosX, EntriesPosY + (State->CurrentSelection - State->FirstVisible) * TextHeight, 0xFFFF);
      }
      
      
      ScrollStart.YPos = ScrollbarBackground.YPos + ScrollbarBackground.Height * State->FirstVisible / (State->MaxIndex + 1);
      Scrollbar.YPos = ScrollStart.YPos + ScrollStart.Height;
      ScrollEnd.YPos = Scrollbar.YPos + Scrollbar.Height; // ScrollEnd.Height is already subtracted      
      ScrollingBar(State);
      
      MouseBirth();
      break;
      
    case MENU_FUNCTION_PAINT_TIMEOUT:
      X = (UGAWidth - StrLen(ParamText) * GlobalConfig.CharWidth) >> 1;
      DrawMenuText(ParamText, 0, X, TimeoutPosY, 0xFFFF);
      break;      
  }
}

//
// graphical main menu style
//

static VOID DrawMainMenuEntry(REFIT_MENU_ENTRY *Entry, BOOLEAN selected, INTN XPos, INTN YPos)
{
   EG_IMAGE *TmpBuffer = NULL;

 if (((Entry->Tag == TAG_LOADER) || (Entry->Tag == TAG_LEGACY)) &&
        !(GlobalConfig.HideBadges & HDBADGES_SWAP) &&
      (Entry->Row == 0)) {
    MainImage = Entry->DriveImage;
  } else {
    MainImage = Entry->Image;
  }

  if (!MainImage) {
    if (ThemeDir) {
      MainImage = egLoadIcon(ThemeDir, L"icons\\osx.icns", 128);
    } 
    if (!MainImage) {
      MainImage = DummyImage(128);
    }
  }
/*  if (!MainImage) {  //looks to be impossible, else fatal bug
    Entry->Place.XPos = XPos;
    Entry->Place.YPos = YPos;
    Entry->Place.Width = 48;
    Entry->Place.Height = 48;
    return;
  } */
//  DBG("Entry title=%s; Width=%d\n", Entry->Title, MainImage->Width);
//  egComposeImage();
  if (GlobalConfig.SelectionOnTop) {
    SelectionImages[0]->HasAlpha = TRUE;
    SelectionImages[2]->HasAlpha = TRUE;
//    MainImage->HasAlpha = TRUE;
    BltImageCompositeBadge(MainImage, 
                           SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
                           (Entry->Row == 0) ? Entry->BadgeImage:NULL, XPos, YPos);

  } else {
    BltImageCompositeBadge(SelectionImages[((Entry->Row == 0) ? 0 : 2) + (selected ? 0 : 1)],
                         MainImage, (Entry->Row == 0) ? Entry->BadgeImage:NULL, XPos, YPos);
  }
  Entry->Place.XPos = XPos;
  Entry->Place.YPos = YPos;
  Entry->Place.Width = MainImage->Width;
  Entry->Place.Height = MainImage->Height;
  egFreeImage(TmpBuffer);
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
  if (OldTextWidth > TextWidth) {
    //Clear old text
    FillRectAreaOfScreen(OldX, OldY,
                         OldTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
  }
  if ((GlobalConfig.HideBadges & HDBADGES_INLINE) && 
      (!OldRow) && (OldTextWidth) && (OldTextWidth != TextWidth)) {
    //Clear badge
    BltImageAlpha(NULL, (OldX - (OldTextWidth >> 1) - (BADGE_DIMENSION + 16)),
                  (OldY - ((BADGE_DIMENSION - TextHeight) >> 1)), &MenuBackgroundPixel, BADGE_DIMENSION >> 3);
  }
  DrawTextXY(Text, XPos, YPos, X_IS_CENTER);
  
  //show inline badge
   if ((GlobalConfig.HideBadges & HDBADGES_INLINE) &&
      (Screen->Entries[State->CurrentSelection]->Row == 0))
  {
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
  if (row0PosX > row1PosX) { //9<10
    MsgLog("BUG! (index_row0 > index_row1) Needed sorting\n");
  }
}

VOID MainMenuVerticalStyle(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText)
{
  INTN i;
  INTN row0PosYRunning;
  INTN VisibleHeight = 0; //assume vertical layout
  
  switch (Function) {
      
    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      SwitchToGraphicsAndClear();
      //adjustable by theme.plist?
      EntriesPosY = LAYOUT_Y_EDGE;
      EntriesGap = LAYOUT_Y_EDGE;
      EntriesWidth = ROW0_TILESIZE;
      EntriesHeight = ROW0_TILESIZE;
      //
      VisibleHeight = (UGAHeight - EntriesPosY - LAYOUT_Y_EDGE + EntriesGap) / (EntriesHeight + EntriesGap);
      EntriesPosX = UGAWidth - EntriesWidth - BAR_WIDTH - LAYOUT_X_EDGE;
      TimeoutPosY = UGAHeight - LAYOUT_Y_EDGE - TextHeight;
      
      CountItems(Screen);
      InitScroll(State, row0Count, Screen->EntryCount, VisibleHeight);
      row0PosX = EntriesPosX;
      row0PosY = EntriesPosY;
      row1PosX = (UGAWidth + EntriesGap - (ROW1_TILESIZE + TILE_XSPACING) * row1Count) >> 1;
      textPosY = TimeoutPosY - TILE_YSPACING - TextHeight;
      row1PosY = textPosY - ROW1_TILESIZE - TILE_YSPACING - LayoutTextOffset;
      if (!itemPosX) {
        itemPosX = AllocatePool(sizeof(UINT64) * Screen->EntryCount);
        itemPosY = AllocatePool(sizeof(UINT64) * Screen->EntryCount);
      }      
  //    row0PosXRunning = row0PosX;
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
          row1PosXRunning += ROW1_TILESIZE + TILE_XSPACING;
          //         DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
      InitSelection();
      CopyMem(&Screen->FilmPlace, &BannerPlace, sizeof(BannerPlace)); 
    
      InitBar();
      InitAnime(Screen);
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
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION)){
#ifdef FIRMWARE_REVISION
        DrawTextXY(FIRMWARE_REVISION, 5, UGAHeight - 5 - TextHeight, X_IS_LEFT);
#else
        DrawTextXY(gST->FirmwareRevision, 5, UGAHeight - 5 - TextHeight, X_IS_LEFT);
#endif
      }      
      
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
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION)){
#ifdef FIRMWARE_REVISION
        DrawTextXY(FIRMWARE_REVISION, 5, UGAHeight - 5 - TextHeight, X_IS_LEFT);
#else
        DrawTextXY(gST->FirmwareRevision, 5, UGAHeight - 5 - TextHeight, X_IS_LEFT);
#endif
      }      
      
      MouseBirth();
      break;
      
    case MENU_FUNCTION_PAINT_TIMEOUT:
//      X = (EntriesPosX - StrLen(ParamText) * GlobalConfig.CharWidth) >> 1;
//      DrawMenuText(ParamText, 0, X, TimeoutPosY, 0xFFFF);
      i = (GlobalConfig.HideBadges & HDBADGES_INLINE)?3:1;
      HidePointer();
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        FillRectAreaOfScreen((UGAWidth >> 1), textPosY + TextHeight * i,
                             OldTimeoutTextWidth, TextHeight, &MenuBackgroundPixel, X_IS_CENTER);
        OldTimeoutTextWidth = DrawTextXY(ParamText, (UGAWidth >> 1), textPosY + TextHeight * i, X_IS_CENTER);
      }
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION)){
#ifdef FIRMWARE_REVISION
        DrawTextXY(FIRMWARE_REVISION, 5, UGAHeight - 5 - TextHeight, X_IS_LEFT);
#else
        DrawTextXY(gST->FirmwareRevision, 5, UGAHeight - 5 - TextHeight, X_IS_LEFT);
#endif
      }      
      
      break;
  }
}
#endif


//static 
VOID MainMenuStyle(IN REFIT_MENU_SCREEN *Screen, IN SCROLL_STATE *State, IN UINTN Function, IN CHAR16 *ParamText)
{
  INTN i; 
  
  switch (Function) {
      
    case MENU_FUNCTION_INIT:
      egGetScreenSize(&UGAWidth, &UGAHeight);
      SwitchToGraphicsAndClear();
      
      EntriesGap = TILE_XSPACING;
      EntriesWidth = ROW0_TILESIZE;
      EntriesHeight = ROW0_TILESIZE;
      
      MaxItemOnScreen = (UGAWidth - ROW0_SCROLLSIZE * 2) / (EntriesWidth + EntriesGap); //8
      CountItems(Screen);
      InitScroll(State, row0Count, Screen->EntryCount, MaxItemOnScreen);
      row0PosX = (UGAWidth + EntriesGap - (EntriesWidth + EntriesGap) *
                  ((MaxItemOnScreen < row0Count)?MaxItemOnScreen:row0Count)) >> 1;
      row0PosY = ((UGAHeight - LayoutMainMenuHeight) >> 1) + LayoutBannerOffset; //LAYOUT_BANNER_YOFFSET; 
      
      row1PosX = (UGAWidth + EntriesGap - (ROW1_TILESIZE + TILE_XSPACING) * row1Count) >> 1;
      row1PosY = row0PosY + EntriesHeight + TILE_YSPACING + LayoutButtonOffset;
      if (row1Count > 0)
        textPosY = row1PosY + ROW1_TILESIZE + TILE_YSPACING + LayoutTextOffset;
      else
        textPosY = row1PosY;
      
      if (!itemPosX) {
        itemPosX = AllocatePool(sizeof(UINT64) * Screen->EntryCount);
      }
      
      row0PosXRunning = row0PosX;
      row1PosXRunning = row1PosX;
 //     DBG("EntryCount =%d\n", Screen->EntryCount);
      for (i = 0; i < (INTN)Screen->EntryCount; i++) {
        if (Screen->Entries[i]->Row == 0) {
          itemPosX[i] = row0PosXRunning;
          row0PosXRunning += EntriesWidth + EntriesGap;
        } else {
          itemPosX[i] = row1PosXRunning;
          row1PosXRunning += ROW1_TILESIZE + TILE_XSPACING;
 //         DBG("next item in row1 at x=%d\n", row1PosXRunning);
        }
      }
      // initial painting
      InitSelection();
      CopyMem(&Screen->FilmPlace, &BannerPlace, sizeof(BannerPlace)); 
      
      InitAnime(Screen);
      
 //     DBG("main menu inited\n");
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
          }
        } else {
          DrawMainMenuEntry(Screen->Entries[i], (i == State->CurrentSelection)?1:0,
                            itemPosX[i], row1PosY);
        }
      }
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)){
        DrawMainMenuLabel(Screen->Entries[State->CurrentSelection]->Title,
                          (UGAWidth >> 1), textPosY, Screen, State);
      }
      //      DBG("DrawRevision\n");
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION)){
#ifdef FIRMWARE_REVISION
        DrawTextXY(FIRMWARE_REVISION, (UGAWidth - 2), UGAHeight - 5 - TextHeight, X_IS_RIGHT);
#else
        DrawTextXY(gST->FirmwareRevision, (UGAWidth - 2), UGAHeight - 5 - TextHeight, X_IS_RIGHT);
#endif
      }      
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
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_LABEL)) {
          DrawMainMenuLabel(Screen->Entries[State->CurrentSelection]->Title,
                            (UGAWidth >> 1), textPosY, Screen, State);
      }
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION)){
#ifdef FIRMWARE_REVISION
          DrawTextXY(FIRMWARE_REVISION, (UGAWidth - 2), UGAHeight - 5 - TextHeight, X_IS_RIGHT);
#else
          DrawTextXY(gST->FirmwareRevision, (UGAWidth - 2), UGAHeight - 5 - TextHeight, X_IS_RIGHT);
#endif
      }
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
      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_REVISION)){
#ifdef FIRMWARE_REVISION
          DrawTextXY(FIRMWARE_REVISION, (UGAWidth - 2), UGAHeight - 5 - TextHeight, X_IS_RIGHT);
#else
          DrawTextXY(gST->FirmwareRevision, (UGAWidth - 2), UGAHeight - 5 - TextHeight, X_IS_RIGHT);
#endif
      }
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
  Entry->Title = PoolPrint(L"Graphics Injector menu ->");
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
  InputBootArgs->Entry.Title = PoolPrint(L"InjectEDID:");
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
      InputBootArgs->Entry.Title = PoolPrint(L"InjectNVidia:");
    } else if (gGraphics[i].Vendor == Ati) {
      InputBootArgs->Entry.Title = PoolPrint(L"InjectATI:");
    } else if (gGraphics[i].Vendor == Intel) {
      InputBootArgs->Entry.Title = PoolPrint(L"InjectIntel:");
    } else {
      InputBootArgs->Entry.Title = PoolPrint(L"InjectX3:");
    }
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = 0xFFFF; //cursor
    InputBootArgs->Item = &InputItems[N+1];
    InputBootArgs->Entry.AtClick = ActionEnter;
    InputBootArgs->Entry.AtRightClick = ActionDetails;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
    
    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    if (gGraphics[i].Vendor == Nvidia) {
      Ven = 98;
    } else if (gGraphics[i].Vendor == Ati) {
      Ven = 97;
    } else /*if (gGraphics[i].Vendor == Intel)*/ {
      Ven = 99;
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
    } else {
      InputBootArgs->Entry.Title = PoolPrint(L"FBConfig:");
    }
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[N+2].SValue); //cursor
    InputBootArgs->Item = &InputItems[N+2];    
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
    
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
//    if (gGraphics[i].Vendor == Nvidia) {
      InputBootArgs->Entry.Title = PoolPrint(L"NVCAP:");
//    } else {
//      InputBootArgs->Entry.Title = PoolPrint(L"Connectors:");
//    }
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[N+4].SValue); //cursor
    InputBootArgs->Item = &InputItems[N+4];    
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
    }
    
    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    InputBootArgs->Entry.Title = PoolPrint(L"LoadVideoBios:");
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = 0xFFFF; //cursor
    InputBootArgs->Item = &InputItems[N+5];    
    InputBootArgs->Entry.AtClick = ActionEnter;
    InputBootArgs->Entry.AtRightClick = ActionDetails;
    AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
    
  }
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
  Entry->Title = PoolPrint(L"CPU tuning menu ->");
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
  InputBootArgs->Entry.Title = PoolPrint(L"GeneratePStates:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = 'G';
  InputBootArgs->Item = &InputItems[5];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Halt Enabler:");
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
  InputBootArgs->Entry.Title = PoolPrint(L"DoubleFirstState:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = 'D';
  InputBootArgs->Item = &InputItems[88];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtDoubleClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"GenerateCStates:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = 'C';
  InputBootArgs->Item = &InputItems[9];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"EnableC2:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = '2';
  InputBootArgs->Item = &InputItems[10];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"EnableC4:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = '4';
  InputBootArgs->Item = &InputItems[11];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"EnableC6:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = '6';
  InputBootArgs->Item = &InputItems[12];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"EnableC7:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Entry.ShortcutLetter = '7';
  InputBootArgs->Item = &InputItems[89];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Use SystemIO:");
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
  
  //15   
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"PatchAPIC:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  //    InputBootArgs->Entry.ShortcutDigit = 0;
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
  Entry->Title = PoolPrint(L"Binaries patching menu ->");
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

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kext patching allowed:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[44];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kernel Support CPU:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[45];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kernel Lapic Patch:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[94];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Kernel PM Patch:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[48];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"AppleIntelCPUPM patch:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[46];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"AppleRTC patch:");
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
  CHAR16*             Flags;
  REFIT_MENU_ENTRY    *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN   *SubScreen;
  REFIT_INPUT_DIALOG  *InputBootArgs;

  sign[4] = 0;
  OTID[8] = 0;
  Flags = AllocateZeroPool(255);

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"Tables dropping menu ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;
  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_TABLES;
  SubScreen->AnimeRun = GetAnime(SubScreen);

  if (gSettings.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
    while (DropTable) {
      //      DBG("Attempting to drop \"%4.4a\" (%8.8X) \"%8.8a\" (%16.16lX)\n", &(DropTable->Signature), DropTable->Signature, &(DropTable->TableId), DropTable->TableId);
/*      AddMenuInfoLine(SubScreen, PoolPrint(L"To drop \"%4.4a\": \"%8.8a\"",
                                           &(DropTable->Signature),
                                           &(DropTable->TableId)));
 */
      CopyMem((CHAR8*)&sign, (CHAR8*)&(DropTable->Signature), 4);
      CopyMem((CHAR8*)&OTID, (CHAR8*)&(DropTable->TableId), 8);

      MsgLog("adding to menu %a (%x) %a (%lx) L=%d(0x%x)\n",
             sign, DropTable->Signature,
             OTID, DropTable->TableId,
             DropTable->Length, DropTable->Length);
      InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
      UnicodeSPrint(Flags, 255, L"Drop \"%4.4a\"  \"%8.8a\" %d:", sign, OTID, DropTable->Length);
      InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
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
  InputBootArgs->Entry.Title = PoolPrint(L"Drop all OEM SSDT:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  //    InputBootArgs->Entry.ShortcutDigit = 0;
  InputBootArgs->Entry.ShortcutLetter = 'S';
  InputBootArgs->Item = &InputItems[4];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  /*
   InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
   InputBootArgs->Entry.Title = PoolPrint(L"Drop OEM APIC:");
   InputBootArgs->Entry.Tag = TAG_INPUT;
   InputBootArgs->Entry.Row = 0xFFFF; //cursor
   InputBootArgs->Item = &InputItems[48];
   InputBootArgs->Entry.AtClick = ActionEnter;
   InputBootArgs->Entry.AtRightClick = ActionDetails;
   AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

   InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
   InputBootArgs->Entry.Title = PoolPrint(L"Drop MCFG:");
   InputBootArgs->Entry.Tag = TAG_INPUT;
   InputBootArgs->Entry.Row = 0xFFFF; //cursor
   InputBootArgs->Item = &InputItems[49];
   InputBootArgs->Entry.AtClick = ActionEnter;
   InputBootArgs->Entry.AtRightClick = ActionDetails;
   AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

   InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
   InputBootArgs->Entry.Title = PoolPrint(L"Drop OEM HPET:");
   InputBootArgs->Entry.Tag = TAG_INPUT;
   InputBootArgs->Entry.Row = 0xFFFF; //cursor
   InputBootArgs->Item = &InputItems[50];
   InputBootArgs->Entry.AtClick = ActionEnter;
   InputBootArgs->Entry.AtRightClick = ActionDetails;
   AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

   InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
   InputBootArgs->Entry.Title = PoolPrint(L"Drop OEM ECDT:");
   InputBootArgs->Entry.Tag = TAG_INPUT;
   InputBootArgs->Entry.Row = 0xFFFF; //cursor
   InputBootArgs->Item = &InputItems[51];
   InputBootArgs->Entry.AtClick = ActionEnter;
   InputBootArgs->Entry.AtRightClick = ActionDetails;
   AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

   InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
   InputBootArgs->Entry.Title = PoolPrint(L"Drop OEM DMAR:");
   InputBootArgs->Entry.Tag = TAG_INPUT;
   InputBootArgs->Entry.Row = 0xFFFF; //cursor
   InputBootArgs->Item = &InputItems[77];
   InputBootArgs->Entry.AtClick = ActionEnter;
   InputBootArgs->Entry.AtRightClick = ActionDetails;
   AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
   //bDropBGRT
   InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
   InputBootArgs->Entry.Title = PoolPrint(L"Drop OEM BGRT:");
   InputBootArgs->Entry.Tag = TAG_INPUT;
   InputBootArgs->Entry.Row = 0xFFFF; //cursor
   InputBootArgs->Item = &InputItems[89];
   InputBootArgs->Entry.AtClick = ActionEnter;
   InputBootArgs->Entry.AtRightClick = ActionDetails;
   AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
   */

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  FreePool(Flags);
  return Entry;
}


REFIT_MENU_ENTRY  *SubMenuSmbios()
{
  REFIT_MENU_ENTRY   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;
  CHAR16*           Flags;
  Flags = AllocateZeroPool(255);
    
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
  UnicodeSPrint(Flags, 255, L"Product name:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[78].SValue);
  InputBootArgs->Entry.ShortcutDigit = 0xF1;
  InputBootArgs->Item = &InputItems[78];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"Product version:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[79].SValue);
  InputBootArgs->Item = &InputItems[79];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"Product sn:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[80].SValue);
  InputBootArgs->Item = &InputItems[80];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"Board ID:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[81].SValue);
  InputBootArgs->Item = &InputItems[81];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"Board sn:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[82].SValue);
  InputBootArgs->Item = &InputItems[82];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"Board type:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[83].SValue);
  InputBootArgs->Item = &InputItems[83];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"Board version:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[84].SValue);
  InputBootArgs->Item = &InputItems[84];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"Chassis type:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[85].SValue);
  InputBootArgs->Item = &InputItems[85];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"ROM version:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[86].SValue);
  InputBootArgs->Item = &InputItems[86];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"ROM release date:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[87].SValue);
  InputBootArgs->Item = &InputItems[87];    
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
    
  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  FreePool(Flags);
  return Entry;  
}

REFIT_MENU_ENTRY  *SubMenuDsdtFix()
{
  REFIT_MENU_ENTRY   *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;
  CHAR16*           Flags;
  Flags = AllocateZeroPool(255);
  
  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = AllocateZeroPool(255);
  UnicodeSPrint(Entry->Title, 255, L"DSDT fix mask [0x%08x]->", gSettings.FixDsdt);
//  Entry->Title = PoolPrint(L"DSDT fix mask [0x%04x]->", gSettings.FixDsdt);
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
  InputBootArgs->Entry.Title = PoolPrint(L"Debug DSDT  :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[105];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);  
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"DSDT name:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[1].SValue);
//  InputBootArgs->Entry.ShortcutDigit = 0;
  InputBootArgs->Entry.ShortcutLetter = 'D';
  InputBootArgs->Entry.Image = NULL;
  InputBootArgs->Entry.BadgeImage = NULL;
  InputBootArgs->Entry.SubScreen = NULL;
  InputBootArgs->Item = &InputItems[1];    //1
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Drop _DSM   :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[104].SValue);; //cursor
  InputBootArgs->Item = &InputItems[104];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);  
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add DTGP    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[53];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix Darwin  :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[54];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix shutdown:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[55];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add MCHC    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[56];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix HPET    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[57];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fake LPC    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[58];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix IPIC    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[59];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add SMBUS   :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[60];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix display :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[61];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix IDE     :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[62];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix SATA    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[63];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix Firewire:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[64];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix USB     :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[65];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix LAN     :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[66];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix Airport :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[67];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix sound   :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[68];    
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix new way :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[125];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
/*
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix Darwin  :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[110];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
*/
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix RTC     :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[111];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix TMR     :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[112];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add IMEI    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[113];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix IntelGFX:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[114];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix _WAK    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[115];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Del unused  :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[116];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix ADP1    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[117];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Add PNLF    :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[118];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Fix S3D     :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[119];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Rename ACST :");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF; //cursor
  InputBootArgs->Item = &InputItems[120];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  FreePool(Flags);
  return Entry;
} 


REFIT_MENU_ENTRY  *SubMenuRcScripts()
{
  REFIT_MENU_ENTRY   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;
  CHAR16*           Flags;
  Flags = AllocateZeroPool(255);

  Entry = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
  Entry->Title = PoolPrint(L"RC Scripts Variables ->");
  Entry->Image =  OptionMenu.TitleImage;
  Entry->Tag = TAG_OPTIONS;
  Entry->AtClick = ActionEnter;

  // create the submenu
  SubScreen = AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen->Title = Entry->Title;
  SubScreen->TitleImage = Entry->Image;
  SubScreen->ID = SCREEN_RC_SCRIPTS;
  SubScreen->AnimeRun = GetAnime(SubScreen);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Mount EFI:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[93].SValue);
  InputBootArgs->Entry.ShortcutLetter = 'E';
  InputBootArgs->Item = &InputItems[93];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Log Line Count:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[92].SValue);
  InputBootArgs->Entry.ShortcutLetter = 'C';
  InputBootArgs->Item = &InputItems[92];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtRightClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Log Every Boot:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[91].SValue);
  InputBootArgs->Item = &InputItems[91];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  FreePool(Flags);
  return Entry;
}

REFIT_MENU_ENTRY  *SubMenuPCI()
{
  REFIT_MENU_ENTRY   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;
  CHAR16*           Flags;
  Flags = AllocateZeroPool(255);

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
  InputBootArgs->Entry.Title = PoolPrint(L"USB Ownership:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  InputBootArgs->Item = &InputItems[74];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"USB Injection:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  InputBootArgs->Item = &InputItems[95];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"Inject ClockID:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = 0xFFFF;
  InputBootArgs->Item = &InputItems[96];
  InputBootArgs->Entry.AtClick = ActionEnter;
  InputBootArgs->Entry.AtRightClick = ActionDetails;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID LAN:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[100].SValue); //cursor
  InputBootArgs->Item = &InputItems[100];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID WIFI:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[101].SValue); //cursor
  InputBootArgs->Item = &InputItems[101];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID SATA:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[102].SValue); //cursor
  InputBootArgs->Item = &InputItems[102];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);

  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID XHCI:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[103].SValue); //cursor
  InputBootArgs->Item = &InputItems[103];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  InputBootArgs->Entry.Title = PoolPrint(L"FakeID IMEI:");
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[106].SValue); //cursor
  InputBootArgs->Item = &InputItems[106];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtDoubleClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);
  
  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  FreePool(Flags);
  return Entry;
}


REFIT_MENU_ENTRY  *SubMenuThemes()
{
  REFIT_MENU_ENTRY   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  REFIT_INPUT_DIALOG *InputBootArgs;
  UINTN               i;
  CHAR16*             Flags = AllocateZeroPool(255);

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
    AddMenuInfoLine(SubScreen, PoolPrint(L"     %s", ThemesList[i]));
  }
  
  InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
  UnicodeSPrint(Flags, 255, L"Theme:");
  InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
  InputBootArgs->Entry.Tag = TAG_INPUT;
  InputBootArgs->Entry.Row = StrLen(InputItems[3].SValue);
  InputBootArgs->Item = &InputItems[3];
  InputBootArgs->Entry.AtClick = ActionSelect;
  InputBootArgs->Entry.AtRightClick = ActionEnter;
  AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY*)InputBootArgs);  

  AddMenuEntry(SubScreen, &MenuEntryReturn);
  Entry->SubScreen = SubScreen;
  FreePool(Flags);
  return Entry;
}


VOID  OptionsMenu(OUT REFIT_MENU_ENTRY **ChosenEntry)
{
  REFIT_MENU_ENTRY  *TmpChosenEntry = NULL;
  UINTN             MenuExit = 0;
  UINTN             SubMenuExit;
  CHAR16*           Flags;
  MENU_STYLE_FUNC   Style = TextMenuStyle;
  MENU_STYLE_FUNC   SubStyle;
  INTN              EntryIndex = 0;
  INTN              SubMenuIndex;
//  INTN              DFIndex = 9;
  REFIT_INPUT_DIALOG* InputBootArgs;
  
  if (AllowGraphicsMode)
    Style = GraphicsMenuStyle;
  
  //remember, if you extended this menu then change procedures
  // FillInputs and ApplyInputs
  
  if (OptionMenu.EntryCount == 0) {
    if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
      OptionMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_OPTIONS);
    } else {
      OptionMenu.TitleImage = NULL;
    }
    OptionMenu.ID = SCREEN_OPTIONS;
    OptionMenu.AnimeRun = GetAnime(&OptionMenu); //FALSE;
    Flags = AllocateZeroPool(255);
    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    *ChosenEntry = (REFIT_MENU_ENTRY*)InputBootArgs;   

    UnicodeSPrint(Flags, 255, L"Config:");
    InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[90].SValue);
    InputBootArgs->Entry.ShortcutDigit = 0xF1;
    InputBootArgs->Item = &InputItems[90];    //0
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);
    
    InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
    UnicodeSPrint(Flags, 255, L"Boot Args:");
    InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
    InputBootArgs->Entry.Tag = TAG_INPUT;
    InputBootArgs->Entry.Row = StrLen(InputItems[0].SValue);
    InputBootArgs->Item = &InputItems[0];    
    InputBootArgs->Entry.AtClick = ActionSelect;
    InputBootArgs->Entry.AtDoubleClick = ActionEnter;
    AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);
    //3
	if (AllowGraphicsMode) {
/*		InputBootArgs = AllocateZeroPool(sizeof(REFIT_INPUT_DIALOG));
		UnicodeSPrint(Flags, 255, L"Theme:");
		InputBootArgs->Entry.Title = EfiStrDuplicate(Flags);
		InputBootArgs->Entry.Tag = TAG_INPUT;
		InputBootArgs->Entry.Row = StrLen(InputItems[3].SValue);
		InputBootArgs->Item = &InputItems[3];  
		InputBootArgs->Entry.AtClick = ActionSelect;
		InputBootArgs->Entry.AtRightClick = ActionEnter;
		AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);
*/
        
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
		InputBootArgs->Entry.Title = PoolPrint(L"Mirror move:");
		InputBootArgs->Entry.Tag = TAG_INPUT;
		InputBootArgs->Entry.Row = 0xFFFF;
		InputBootArgs->Item = &InputItems[72];
		InputBootArgs->Entry.AtClick = ActionEnter;
		InputBootArgs->Entry.AtRightClick = ActionDetails;
		AddMenuEntry(&OptionMenu, (REFIT_MENU_ENTRY*)InputBootArgs);

    AddMenuEntry(&OptionMenu, SubMenuThemes());
	}

 //   DFIndex = OptionMenu.EntryCount;
    AddMenuEntry(&OptionMenu, SubMenuDropTables());
    AddMenuEntry(&OptionMenu, SubMenuDsdtFix());
    AddMenuEntry(&OptionMenu, SubMenuSmbios());
    AddMenuEntry(&OptionMenu, SubMenuPCI());
    AddMenuEntry(&OptionMenu, SubMenuSpeedStep());
    AddMenuEntry(&OptionMenu, SubMenuGraphics());
    AddMenuEntry(&OptionMenu, SubMenuBinaries());
    AddMenuEntry(&OptionMenu, SubMenuRcScripts());
    AddMenuEntry(&OptionMenu, &MenuEntryReturn);
    FreePool(Flags);
    //    DBG("option menu created entries=%d\n", OptionMenu.EntryCount);
  }
  //  StyleFunc(OptionMenu, &State, MENU_FUNCTION_INIT, NULL);
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
              MsgLog("@ESC: %s\n", (*ChosenEntry)->Title);
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
              MsgLog("@ENTER: tmp=%s\n", TmpTitle);
              while (*TmpTitle) {
                *(*ChosenEntry)->Title++ = *TmpTitle++;
              }
              MsgLog("@ENTER: chosen=%s\n", (*ChosenEntry)->Title);
            }
            if (TmpChosenEntry->ShortcutDigit == 0xF1) {
              MenuExit = MENU_EXIT_ENTER;
         //     DBG("Escape menu from input dialog\n");
              ApplyInputs();
              return;
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

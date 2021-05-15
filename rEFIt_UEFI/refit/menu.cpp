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
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../libeg/libegint.h"
//#include "../include/scroll_images.h"

#include "../Platform/Settings.h"
//#include "colors.h"

#include "../libeg/nanosvg.h"
#include "../libeg/FloatLib.h"
#include "../Platform/HdaCodecDump.h"
#include "menu.h"
#include "screen.h"
#include "../cpp_foundation/XString.h"
#include "../libeg/XTheme.h"
#include "../libeg/VectorGraphics.h" // for testSVG
#include "../gui/shared_with_menu.h"
#include "../Platform/platformdata.h"
#include "../Platform/cpu.h"
#include "../Platform/Nvram.h"
#include "../include/DsdtFixList.h"
#include "../include/Devices.h"
#include "../include/QuirksCodes.h"
#include "../Platform/boot.h"
#include "../Platform/Injectors.h"
#include "../Platform/KextList.h"
#include "../gui/REFIT_MENU_SCREEN.h"
#include "../gui/REFIT_MAINMENU_SCREEN.h"
#include "../Settings/Self.h"
#include "../Platform/VersionString.h"
#include "../Settings/ConfigManager.h"
#include "../Platform/CloverVersion.h"


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


INTN LayoutMainMenuHeight = 376;
INTN LayoutAnimMoveForMenuX = 0;
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
REFIT_MENU_ITEM_OPTIONS  MenuEntryOptions (L"Options"_XSW,          1, 0, 'O', ActionEnter);
REFIT_MENU_ITEM_ABOUT    MenuEntryAbout   (L"About Clover"_XSW,     1, 0, 'A', ActionEnter);
REFIT_MENU_ITEM_RESET    MenuEntryReset   (L"Restart Computer"_XSW, 1, 0, 'R', ActionSelect);
REFIT_MENU_ITEM_SHUTDOWN MenuEntryShutdown(L"Exit Clover"_XSW,      1, 0, 'U', ActionSelect);
REFIT_MENU_ITEM_RETURN   MenuEntryReturn  (L"Return"_XSW,           0, 0,  0,  ActionEnter);

REFIT_MAINMENU_SCREEN MainMenu(1, L"Main Menu"_XSW, L"Automatic boot"_XSW);
REFIT_MENU_SCREEN AboutMenu(2, L"About"_XSW, L""_XSW);
REFIT_MENU_SCREEN HelpMenu(3, L"Help"_XSW, L""_XSW);
REFIT_MENU_SCREEN OptionMenu(4, L"Options"_XSW, L""_XSW);


void FillInputs(BOOLEAN New)
{
  UINTN i,j; //for loops
  CHAR8 tmp[41];
//  BOOLEAN bit;

  tmp[40] = 0;  //make it null-terminated

  UINTN InputItemsCount = 0;
  if (New) {
//    InputItems = (__typeof__(InputItems))A_llocateZeroPool(130 * sizeof(INPUT_ITEM)); //XXX
    InputItems = new INPUT_ITEM[130];
  }

  InputItems[InputItemsCount].ItemType = ASString;  //0
  //even though Ascii we will keep value as Unicode to convert later
  // no need for extra space here, it is added by ApplyInputs()
  InputItems[InputItemsCount++].SValue.takeValueFrom(gSettings.Boot.BootArgs);
  InputItems[InputItemsCount].ItemType = UNIString; //1
  InputItems[InputItemsCount++].SValue.takeValueFrom(gSettings.ACPI.DSDT.DsdtName); // 1-> 2
  InputItems[InputItemsCount].ItemType = UNIString; //2
  InputItems[InputItemsCount++].SValue = GlobalConfig.BlockKexts;

  InputItems[InputItemsCount].ItemType = RadioSwitch;  //3 - Themes chooser
  InputItems[InputItemsCount++].IValue = 3;

  InputItems[InputItemsCount].ItemType = BoolValue; //4
  InputItems[InputItemsCount++].BValue = GlobalConfig.DropSSDT;
  InputItems[InputItemsCount].ItemType = BoolValue;  //5
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.SSDT.Generate.GeneratePStates;
  InputItems[InputItemsCount].ItemType = BoolValue;  //6
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.SlpSmiEnable;
  InputItems[InputItemsCount].ItemType = Decimal;  //7
  InputItems[InputItemsCount++].SValue.SWPrintf("%02d", gSettings.ACPI.SSDT.PLimitDict);
  InputItems[InputItemsCount].ItemType = Decimal;  //8
  InputItems[InputItemsCount++].SValue.SWPrintf("%02d", gSettings.ACPI.SSDT.UnderVoltStep);
  InputItems[InputItemsCount].ItemType = BoolValue; //9
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.SSDT.Generate.GenerateCStates;
  InputItems[InputItemsCount].ItemType = BoolValue; //10
  InputItems[InputItemsCount++].BValue = GlobalConfig.EnableC2;
  InputItems[InputItemsCount].ItemType = BoolValue; //11
  InputItems[InputItemsCount++].BValue = GlobalConfig.EnableC4;
  InputItems[InputItemsCount].ItemType = BoolValue; //12
  InputItems[InputItemsCount++].BValue = GlobalConfig.EnableC6;
  InputItems[InputItemsCount].ItemType = BoolValue; //13
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.SSDT.EnableISS;
  InputItems[InputItemsCount].ItemType = Decimal;  //14
  InputItems[InputItemsCount++].SValue.SWPrintf("%06d", gSettings.CPU.QPI);
  InputItems[InputItemsCount].ItemType = BoolValue; //15
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.PatchNMI;
  InputItems[InputItemsCount].ItemType = BoolValue; //16
  InputItems[InputItemsCount++].BValue = gSettings.Graphics.PatchVBios;
  InputItems[InputItemsCount].ItemType = Decimal;  //17
	InputItems[InputItemsCount++].SValue.SWPrintf("0x%llX", gSettings.Smbios.gPlatformFeature);
  InputItems[InputItemsCount].ItemType = Hex;  //18
	InputItems[InputItemsCount++].SValue.SWPrintf("0x%hX", gSettings.SystemParameters.BacklightLevel);
  InputItems[InputItemsCount].ItemType = Decimal;  //19
  if (gSettings.CPU.BusSpeed > 20000) {
    InputItems[InputItemsCount++].SValue.SWPrintf("%06d", gSettings.CPU.BusSpeed);
  } else {
	  InputItems[InputItemsCount++].SValue.SWPrintf("%06llu", gCPUStructure.ExternalClock);
  }
  InputItemsCount = 20;
  for (i=0; i<gConf.GfxPropertiesArray.size(); i++) {
    InputItems[InputItemsCount].ItemType = ASString;  //20+i*6
	  InputItems[InputItemsCount++].SValue.SWPrintf("%s", gConf.GfxPropertiesArray[i].Model.c_str());

    if (gConf.GfxPropertiesArray[i].Vendor == Ati) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.Graphics.InjectAsDict.InjectATI;
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      if ( gSettings.Graphics.FBName.length() > 2 ) { //fool proof: cfg_name is 3 character or more.
		    InputItems[InputItemsCount++].SValue.SWPrintf("%ls", gSettings.Graphics.FBName.wc_str());
      } else {
		    InputItems[InputItemsCount++].SValue.SWPrintf("%s", gConf.GfxPropertiesArray[i].Config.c_str());
      }
    } else if (gConf.GfxPropertiesArray[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.Graphics.InjectAsDict.InjectNVidia;
      InputItems[InputItemsCount].ItemType = ASString; //22+6i
      for (j=0; j<8; j++) {
        snprintf((CHAR8*)&tmp[2*j], 3, "%02hhX", gSettings.Graphics.Dcfg[j]);
      }
		  InputItems[InputItemsCount++].SValue.SWPrintf("%s", tmp);

      //InputItems[InputItemsCount++].SValue = P__oolPrint(L"%08x",*(UINT64*)&gSettings.Graphics.Dcfg[0]);
    } else /*if (gGraphics[i].Vendor == Intel) */ {
      InputItems[InputItemsCount].ItemType = BoolValue; //21+i*6
      InputItems[InputItemsCount++].BValue = gSettings.Graphics.InjectAsDict.InjectIntel;
      InputItems[InputItemsCount].ItemType = Hex; //22+6i
		  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", GlobalConfig.IgPlatform);
 //     InputItemsCount += 3;
 //     continue;
    }

    InputItems[InputItemsCount].ItemType = Decimal;  //23+6i
    if (gSettings.Graphics.VideoPorts > 0) {
      InputItems[InputItemsCount++].SValue.SWPrintf("%02d", gSettings.Graphics.VideoPorts);
    } else {
      InputItems[InputItemsCount++].SValue.SWPrintf("%02d", gConf.GfxPropertiesArray[i].Ports);
    }

    if (gConf.GfxPropertiesArray[i].Vendor == Nvidia) {
      InputItems[InputItemsCount].ItemType = ASString; //24+6i
      for (j=0; j<gSettings.Graphics.NVCAP.size(); j++) {
        snprintf((CHAR8*)&tmp[2*j], 3, "%02hhX", gSettings.Graphics.NVCAP[j]);
      }
	    InputItems[InputItemsCount++].SValue.SWPrintf("%s", tmp);
    } else { //ATI and others there will be connectors
      InputItems[InputItemsCount].ItemType = Hex; //24+6i
	    InputItems[InputItemsCount++].SValue.SWPrintf("%08x", gConf.GfxPropertiesArray[i].Connectors);
    }

    InputItems[InputItemsCount].ItemType = BoolValue; //25+6i
    InputItems[InputItemsCount++].BValue = gConf.GfxPropertiesArray[i].LoadVBios;
  }
  //and so on

  InputItemsCount = 44;
  InputItems[InputItemsCount].ItemType = BoolValue; //44
  InputItems[InputItemsCount++].BValue = GlobalConfig.KextPatchesAllowed;
  InputItems[InputItemsCount].ItemType = BoolValue; //45
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.EightApple;
  InputItems[InputItemsCount].ItemType = BoolValue; //46
  InputItems[InputItemsCount++].BValue = GlobalConfig.KPAppleIntelCPUPM;
  InputItems[InputItemsCount].ItemType = BoolValue; //47
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPAppleRTC;
  InputItems[InputItemsCount].ItemType = BoolValue; //48
  InputItems[InputItemsCount++].BValue = GlobalConfig.KPKernelPm;
  InputItems[InputItemsCount].ItemType = BoolValue; //49
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.FixMCFG;

  InputItems[InputItemsCount].ItemType = Decimal;  //50
  InputItems[InputItemsCount++].SValue.SWPrintf("%06d", gSettings.Graphics.RefCLK);

  InputItems[InputItemsCount].ItemType = ASString;  //51 OS version if non-detected
	InputItems[InputItemsCount++].SValue.SWPrintf("%s ", NonDetected.c_str());

  InputItems[InputItemsCount].ItemType = BoolValue; //52
  InputItems[InputItemsCount++].BValue = gSettings.Graphics.EDID.InjectEDID;

  //VendorEDID & ProductEDID 53, 54
  InputItems[InputItemsCount].ItemType = Decimal;  //53
	InputItems[InputItemsCount++].SValue.SWPrintf("0x%04hX", gSettings.Graphics.EDID.VendorEDID);
  InputItems[InputItemsCount].ItemType = Decimal;  //54
	InputItems[InputItemsCount++].SValue.SWPrintf("0x%04hX", gSettings.Graphics.EDID.ProductEDID);

  // ErmaC: NvidiaGeneric menu selector y/n
  InputItems[InputItemsCount].ItemType = BoolValue; //55
  InputItems[InputItemsCount++].BValue = gSettings.Graphics.NvidiaGeneric;
  InputItems[InputItemsCount].ItemType = BoolValue; //56
  InputItems[InputItemsCount++].BValue = gSettings.SystemParameters.NvidiaWeb;
  InputItems[InputItemsCount].ItemType = BoolValue; //57
  InputItems[InputItemsCount++].BValue = gSettings.Devices.Audio.ResetHDA;
  InputItems[InputItemsCount].ItemType = BoolValue; //58
  InputItems[InputItemsCount++].BValue = gSettings.Devices.Audio.AFGLowPowerState;
  InputItems[InputItemsCount].ItemType = BoolValue; //59
  InputItems[InputItemsCount++].BValue = gSettings.Devices.Audio.HDAInjection;
  InputItems[InputItemsCount].ItemType = Decimal;  // 60
  InputItems[InputItemsCount++].SValue.SWPrintf("%d", gSettings.Devices.Audio.HDALayoutId);

  // syscl change here
  InputItems[InputItemsCount].ItemType = BoolValue; //61
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPDELLSMBIOS;
  // end of change

  InputItems[InputItemsCount].ItemType = Hex;  //62
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Smbios.FirmwareFeatures);

  InputItems[InputItemsCount].ItemType = Hex;  //63
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Smbios.FirmwareFeaturesMask);

  // Debug for KernelAndKextPatches
  InputItems[InputItemsCount].ItemType = BoolValue; //64
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPDebug;


  // CSR - aka System Integrity Protection configuration
  InputItems[InputItemsCount].ItemType = CheckBit; //65
  InputItems[InputItemsCount++].IValue = gSettings.RtVariables.BooterConfig;
  InputItems[InputItemsCount].ItemType = CheckBit; //66
  InputItems[InputItemsCount++].IValue = gSettings.RtVariables.CsrActiveConfig;


  InputItems[InputItemsCount].ItemType = CheckBit; //67
  InputItems[InputItemsCount++].IValue = gSettings.ACPI.DSDT.FixDsdt;
  InputItems[InputItemsCount].ItemType = CheckBit; //68
  InputItems[InputItemsCount++].IValue = GlobalConfig.OptionsBits;
  InputItems[InputItemsCount].ItemType = CheckBit; //69
  InputItems[InputItemsCount++].IValue = GlobalConfig.FlagsBits;

  InputItems[InputItemsCount].ItemType = Decimal;  //70
	InputItems[InputItemsCount++].SValue.SWPrintf("%02lld", gSettings.GUI.Mouse.PointerSpeed);
  InputItems[InputItemsCount].ItemType = Decimal;  //71
	InputItems[InputItemsCount++].SValue.SWPrintf("%04llu", gSettings.GUI.Mouse.DoubleClickTime);
  InputItems[InputItemsCount].ItemType = BoolValue; //72
  InputItems[InputItemsCount++].BValue = gSettings.GUI.Mouse.PointerMirror;

  //reserve for mouse and continue

  InputItemsCount = 74;
  InputItems[InputItemsCount].ItemType = BoolValue; //74
  InputItems[InputItemsCount++].BValue = gSettings.Devices.USB.USBFixOwnership;

  InputItems[InputItemsCount].ItemType = Hex;  //75
	InputItems[InputItemsCount++].SValue.SWPrintf("0x%04hX", GlobalConfig.C3Latency);
  InputItems[InputItemsCount].ItemType = Decimal;  //76
  InputItems[InputItemsCount++].SValue.SWPrintf("%02d", GlobalConfig.EnabledCores);
  InputItems[InputItemsCount].ItemType = Decimal;  //77
  InputItems[InputItemsCount++].SValue.SWPrintf("%02d", gSettings.CPU.SavingMode);

  InputItems[InputItemsCount].ItemType = ASString;  //78
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", gSettings.Smbios.ProductName.c_str());
  InputItems[InputItemsCount].ItemType = ASString;  //79
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", gSettings.Smbios.SystemVersion.c_str());
  InputItems[InputItemsCount].ItemType = ASString;  //80
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", gSettings.Smbios.SerialNr.c_str());
  InputItems[InputItemsCount].ItemType = ASString;  //81
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", gSettings.Smbios.BoardNumber.c_str());
  InputItems[InputItemsCount].ItemType = ASString;  //82
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", gSettings.Smbios.BoardSerialNumber.c_str());
  InputItems[InputItemsCount].ItemType = Decimal;  //83
  InputItems[InputItemsCount++].SValue.SWPrintf("%d", gSettings.Smbios.BoardType);
  InputItems[InputItemsCount].ItemType = ASString;  //84
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", gSettings.Smbios.BoardVersion.c_str());
  InputItems[InputItemsCount].ItemType = Decimal;  //85
  InputItems[InputItemsCount++].SValue.SWPrintf("%d", gSettings.Smbios.ChassisType);
  InputItems[InputItemsCount].ItemType = ASString;  //86
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", GlobalConfig.BiosVersionUsed.c_str());
  InputItems[InputItemsCount].ItemType = ASString;  //87
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", GlobalConfig.ReleaseDateUsed.c_str());

  InputItems[InputItemsCount].ItemType = BoolValue; //88
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.SSDT.DoubleFirstState;
  InputItems[InputItemsCount].ItemType = BoolValue; //89
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.SSDT.EnableC7;
  InputItems[InputItemsCount].ItemType = RadioSwitch; //90
  InputItems[InputItemsCount++].IValue = 90;

  InputItems[InputItemsCount].ItemType = BoolValue; //91
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPKernelLapic;

  InputItems[InputItemsCount].ItemType = BoolValue; //92
  InputItems[InputItemsCount++].BValue = gSettings.Devices.USB.USBInjection;
  InputItems[InputItemsCount].ItemType = BoolValue; //93
  InputItems[InputItemsCount++].BValue = gSettings.Devices.USB.InjectClockID;

  InputItems[InputItemsCount].ItemType = Hex;  //94
	InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Devices.FakeID.FakeATI);
  InputItems[InputItemsCount].ItemType = Hex;  //95
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Devices.FakeID.FakeNVidia);
  InputItems[InputItemsCount].ItemType = Hex;  //96
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Devices.FakeID.FakeIntel);

  InputItems[InputItemsCount].ItemType = Hex;  //97
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Devices.FakeID.FakeLAN);
  InputItems[InputItemsCount].ItemType = Hex;  //98
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Devices.FakeID.FakeWIFI);
  InputItems[InputItemsCount].ItemType = Hex;  //99
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Devices.FakeID.FakeSATA);
  InputItems[InputItemsCount].ItemType = Hex;  //100
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Devices.FakeID.FakeXHCI);
  InputItems[InputItemsCount].ItemType = CheckBit;  //101 - Quirks
  InputItems[InputItemsCount++].IValue = gSettings.Quirks.QuirksMask; //

  InputItems[InputItemsCount].ItemType = BoolValue; //102
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.DSDT.DebugDSDT;
  InputItems[InputItemsCount].ItemType = Hex;  //103
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.Devices.FakeID.FakeIMEI);
  InputItems[InputItemsCount].ItemType = Hex;  //104
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%08X", gSettings.KernelAndKextPatches.FakeCPUID);


  InputItems[InputItemsCount].ItemType = BoolValue; //105
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPKernelXCPM;

  InputItems[InputItemsCount].ItemType = BoolValue; //106
  InputItems[InputItemsCount++].BValue = gSettings.Devices.StringInjector;
  InputItems[InputItemsCount].ItemType = BoolValue; //107
  InputItems[InputItemsCount++].BValue = gSettings.Devices.NoDefaultProperties;
  InputItems[InputItemsCount].ItemType = BoolValue; //108
  InputItems[InputItemsCount++].BValue = GlobalConfig.KernelPatchesAllowed;

  InputItems[InputItemsCount].ItemType = Hex; //109
  InputItems[InputItemsCount++].SValue.SWPrintf("%01X", gSettings.Graphics.DualLink);

  InputItems[InputItemsCount].ItemType = BoolValue; //110
  InputItems[InputItemsCount++].BValue = gSettings.Graphics.NvidiaNoEFI;
  InputItems[InputItemsCount].ItemType = BoolValue; //111
  InputItems[InputItemsCount++].BValue = gSettings.Graphics.NvidiaSingle;

  InputItems[InputItemsCount].ItemType = Hex;  //112
  InputItems[InputItemsCount++].SValue.SWPrintf("0x%04X", gSettings.Devices.IntelMaxValue);

  InputItems[InputItemsCount].ItemType = BoolValue; //113
  InputItems[InputItemsCount++].BValue = gSettings.ACPI.AutoMerge;
  InputItems[InputItemsCount].ItemType = BoolValue; //114
  InputItems[InputItemsCount++].BValue = gSettings.Graphics.RadeonDeInit;
  InputItems[InputItemsCount].ItemType = BoolValue; //115
  InputItems[InputItemsCount++].BValue = gSettings.SystemParameters.NoCaches;
  InputItems[InputItemsCount].ItemType = RadioSwitch;  //116 - DSDT chooser
  InputItems[InputItemsCount++].IValue = 116;

  InputItems[InputItemsCount].ItemType = ASString;  //117
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", GlobalConfig.EfiVersionUsed.c_str());
  InputItems[InputItemsCount].ItemType = ASString;  //118
	InputItems[InputItemsCount++].SValue.SWPrintf("%s", gSettings.RtVariables.BooterCfgStr.c_str());

  InputItems[InputItemsCount].ItemType = RadioSwitch;  //119 - Audio chooser
  InputItems[InputItemsCount++].IValue = 119;
  InputItems[InputItemsCount].ItemType = Decimal;  //120
  InputItems[InputItemsCount++].SValue.SWPrintf("%04d", DefaultAudioVolume);
  
  InputItems[InputItemsCount].ItemType = BoolValue; //121
  InputItems[InputItemsCount++].BValue = gSettings.KernelAndKextPatches.KPPanicNoKextDump;
  InputItems[InputItemsCount].ItemType = Decimal;  //122
  InputItems[InputItemsCount++].SValue.SWPrintf("%04hhu", gSettings.Quirks.OcBooterQuirks.ProvideMaxSlide);
  InputItems[InputItemsCount].ItemType = BoolValue; //123
  InputItems[InputItemsCount++].BValue = gSettings.GUI.ProvideConsoleGop;



  //menu for drop table
  if (GlobalConfig.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = GlobalConfig.ACPIDropTables;
    while (DropTable) {
      DropTable->MenuItem.ItemType = BoolValue;
      DropTable = DropTable->Next;
    }
  }

  for ( size_t idx = 0 ; idx < ACPIPatchedAML.size() ; ++idx) {
    ACPI_PATCHED_AML& ACPIPatchedAMLTmp = ACPIPatchedAML[idx];
    ACPIPatchedAMLTmp.MenuItem.ItemType = BoolValue;
  }
}


void ApplyInputs(void)
{
//  EFI_STATUS Status = EFI_NOT_FOUND;
  MACHINE_TYPES Model;
  BOOLEAN NeedSave = TRUE;
  INTN i = 0;
  UINTN j;

//  DBG("ApplyInputs\n");
  if (InputItems[i].Valid) {
	  gSettings.Boot.BootArgs = InputItems[i].SValue;
	  gSettings.Boot.BootArgs.replaceAll('\\', '_');
    GlobalConfig.gBootChanged = TRUE;
  }
  i++; //1
  if (InputItems[i].Valid) {
	  gSettings.ACPI.DSDT.DsdtName = InputItems[i].SValue;
  }
  i++; //2
  if (InputItems[i].Valid) {
	  GlobalConfig.BlockKexts = InputItems[i].SValue;
  }
  i++; //3
  if (InputItems[i].Valid) {
    if (OldChosenTheme == 0xFFFF) {
      gSettings.GUI.Theme = L"embedded"_XSW;
    } else {
      gSettings.GUI.Theme.takeValueFrom(ThemeNameArray[OldChosenTheme]);
    }

    //will change theme after ESC
    GlobalConfig.gThemeChanged = TRUE;
  }
  i++; //4
  if (InputItems[i].Valid) {
    GlobalConfig.DropSSDT = InputItems[i].BValue;
  }
  i++; //5
  if (InputItems[i].Valid) {
    gSettings.ACPI.SSDT.Generate.GeneratePStates = InputItems[i].BValue;
  }
  i++; //6
  if (InputItems[i].Valid) {
    gSettings.ACPI.SlpSmiEnable = InputItems[i].BValue;
  }
  i++; //7
  if (InputItems[i].Valid) {
//    DBG("InputItems[i]: %ls\n", InputItems[i].SValue);
    gSettings.ACPI.SSDT.PLimitDict = (UINT8)(StrDecimalToUintn(InputItems[i].SValue.wc_str()) & 0x7F);
//    DBG("Item 7=PLimitDict %d\n", gSettings.ACPI.SSDT.PLimitDict);
 }
  i++; //8
  if (InputItems[i].Valid) {
    gSettings.ACPI.SSDT.UnderVoltStep = (UINT8)(StrDecimalToUintn(InputItems[i].SValue.wc_str()) & 0x3F);
//    DBG("Item 8=UnderVoltStep %d\n", gSettings.ACPI.SSDT.UnderVoltStep);
  }
  i++; //9
  if (InputItems[i].Valid) {
    gSettings.ACPI.SSDT.Generate.GenerateCStates = InputItems[i].BValue;
  }
  i++; //10
  if (InputItems[i].Valid) {
    GlobalConfig.EnableC2 = InputItems[i].BValue;
  }
  i++; //11
  if (InputItems[i].Valid) {
    GlobalConfig.EnableC4 = InputItems[i].BValue;
  }
  i++; //12
  if (InputItems[i].Valid) {
    GlobalConfig.EnableC6 = InputItems[i].BValue;
  }
  i++; //13
  if (InputItems[i].Valid) {
    gSettings.ACPI.SSDT.EnableISS = InputItems[i].BValue;
  }
  i++; //14
  if (InputItems[i].Valid) {
    gSettings.CPU.QPI = (UINT16)StrDecimalToUintn(InputItems[i].SValue.wc_str());
    DBG("applied QPI=%d\n", gSettings.CPU.QPI);
  }
  i++; //15
  if (InputItems[i].Valid) {
    gSettings.ACPI.PatchNMI = InputItems[i].BValue;
  }
  i++; //16
  if (InputItems[i].Valid) {
    gSettings.Graphics.PatchVBios = InputItems[i].BValue;
  }
  i++; //17
  if (InputItems[i].Valid) {
    gSettings.Smbios.gPlatformFeature = (UINT64)StrHexToUint64(InputItems[i].SValue.wc_str());
	  DBG("applied PlatformFeature=0x%llX\n", gSettings.Smbios.gPlatformFeature);
  }
  i++; //18 | Download-Fritz: There is no GUI element for BacklightLevel; please revise
  if (InputItems[i].Valid) {
    gSettings.SystemParameters.BacklightLevel = (UINT16)StrHexToUint64(InputItems[i].SValue.wc_str());
    gSettings.SystemParameters.BacklightLevelConfig = TRUE;
  }
  i++; //19
  if (InputItems[i].Valid) {
    gSettings.CPU.BusSpeed = (UINT32)StrDecimalToUintn(InputItems[i].SValue.wc_str());
    DBG("applied BusSpeed=%d\n", gSettings.CPU.BusSpeed);
  }

  i = 19;
  for (j = 0; j < gConf.GfxPropertiesArrayNonConst.size(); j++) {
    i++; //20
    if (InputItems[i].Valid) {
      gConf.GfxPropertiesArrayNonConst[j].Model = InputItems[i].SValue;
    }
    i++; //21
    if (InputItems[i].Valid) {
      if (gConf.GfxPropertiesArrayNonConst[j].Vendor == Ati) {
        gSettings.Graphics.InjectAsDict.InjectATI = InputItems[i].BValue;
      } else if (gConf.GfxPropertiesArrayNonConst[j].Vendor == Nvidia) {
        gSettings.Graphics.InjectAsDict.InjectNVidia = InputItems[i].BValue;
      } else if (gConf.GfxPropertiesArrayNonConst[j].Vendor == Intel) {
        gSettings.Graphics.InjectAsDict.InjectIntel = InputItems[i].BValue;
      }
    }
    i++; //22
    if (InputItems[i].Valid) {
      if (gConf.GfxPropertiesArrayNonConst[j].Vendor == Ati) {
		  gSettings.Graphics.FBName = InputItems[i].SValue;
      } else if (gConf.GfxPropertiesArrayNonConst[j].Vendor == Nvidia) {
        hex2bin(InputItems[i].SValue, (UINT8*)&gSettings.Graphics.Dcfg[0], sizeof(gSettings.Graphics.Dcfg));
      } else if (gConf.GfxPropertiesArrayNonConst[j].Vendor == Intel) {
        //ig-platform-id for Ivy+ and snb-platform-id for Sandy
        GlobalConfig.IgPlatform = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
        DBG("applied *-platform-id=0x%X\n", GlobalConfig.IgPlatform);
      }
    }

    if (gConf.GfxPropertiesArrayNonConst[j].Vendor == Intel) {
      i += 3;
      continue;
    }

    i++; //23
    if (InputItems[i].Valid) {
      gConf.GfxPropertiesArrayNonConst[j].Ports = (UINT8)(StrDecimalToUintn(InputItems[i].SValue.wc_str()) & 0x0F);
    }
    i++; //24
    if (InputItems[i].Valid) {
      if (gConf.GfxPropertiesArray[j].Vendor == Nvidia) {
        if ( InputItems[i].SValue.notEmpty() ) {
          hex2bin(InputItems[i].SValue, gSettings.Graphics.NVCAP.data(), gSettings.Graphics.NVCAP.size());
        }
      } else {
        gConf.GfxPropertiesArrayNonConst[j].Connectors = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
        gConf.GfxPropertiesArrayNonConst[j].ConnChanged = TRUE;
      }
    }
    i++; //25
    if (InputItems[i].Valid) {
      gConf.GfxPropertiesArrayNonConst[j].LoadVBios = InputItems[i].BValue;
    }
  }  //end of Graphics Cards
  // next number == 42

  i = 44;
  if (InputItems[i].Valid) {
    GlobalConfig.KextPatchesAllowed = InputItems[i].BValue;
    GlobalConfig.gBootChanged = TRUE;
  }
  i++; //45
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.EightApple = InputItems[i].BValue;
    GlobalConfig.gBootChanged = TRUE;
  }
  i++; //46
  if (InputItems[i].Valid) {
    GlobalConfig.KPAppleIntelCPUPM = InputItems[i].BValue;
    GlobalConfig.gBootChanged = TRUE;
  }
  i++; //47
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPAppleRTC = InputItems[i].BValue;
    GlobalConfig.gBootChanged = TRUE;
  }
  i++; //48
  if (InputItems[i].Valid) {
     GlobalConfig.KPKernelPm = InputItems[i].BValue;
     GlobalConfig.gBootChanged = TRUE;
  }
  i++; //49
  if (InputItems[i].Valid) {
    gSettings.ACPI.FixMCFG = InputItems[i].BValue;
  }

  i++; //50
  if (InputItems[i].Valid) {
    gSettings.Graphics.RefCLK = (UINT32)StrDecimalToUintn(InputItems[i].SValue.wc_str());
  }

  i++; //51
  if (InputItems[i].Valid) {
	  NonDetected = InputItems[i].SValue;
  }

  i++; //52
  if (InputItems[i].Valid) {
    gSettings.Graphics.EDID.InjectEDID = InputItems[i].BValue;
  }
  i++; //53
  if (InputItems[i].Valid) {
    gSettings.Graphics.EDID.VendorEDID = (UINT16)StrHexToUint64(InputItems[i].SValue.wc_str());
  }
  i++; //54
  if (InputItems[i].Valid) {
    gSettings.Graphics.EDID.ProductEDID = (UINT16)StrHexToUint64(InputItems[i].SValue.wc_str());
  }
  i++; //55
  // ErmaC: NvidiaGeneric bool(Y/N)
  if (InputItems[i].Valid) {
    gSettings.Graphics.NvidiaGeneric = InputItems[i].BValue;
  }
  i++; //56
  if (InputItems[i].Valid) {
    gSettings.SystemParameters.NvidiaWeb = InputItems[i].BValue;
  }
  i++; //57
  if (InputItems[i].Valid) {
    gSettings.Devices.Audio.ResetHDA = InputItems[i].BValue;
  }
  i++; //58
  if (InputItems[i].Valid) {
    gSettings.Devices.Audio.AFGLowPowerState = InputItems[i].BValue;
  }
  i++; //59
  if (InputItems[i].Valid) {
    gSettings.Devices.Audio.HDAInjection = InputItems[i].BValue;
  }
  i++; //60
  if (InputItems[i].Valid) {
    gSettings.Devices.Audio.HDALayoutId = (UINT32)(StrDecimalToUintn(InputItems[i].SValue.wc_str()));
  }
  i++; //61
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPDELLSMBIOS = InputItems[i].BValue;
//    // yes, we do need to change gRemapSmBiosIsRequire here as well
//    gRemapSmBiosIsRequire = InputItems[i].BValue;
    GlobalConfig.gBootChanged = TRUE;
  }
  i++; //62
  if (InputItems[i].Valid) {
    gSettings.Smbios.FirmwareFeatures = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
    DBG("applied FirmwareFeatures=0x%X\n", gSettings.Smbios.FirmwareFeatures);
  }
  i++; //63
  if (InputItems[i].Valid) {
    gSettings.Smbios.FirmwareFeaturesMask = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
    DBG("applied FirmwareFeaturesMask=0x%X\n", gSettings.Smbios.FirmwareFeaturesMask);
  }
  i++; //64
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPDebug = InputItems[i].BValue;
 //   GlobalConfig.gBootChanged = TRUE;
  }

  // CSR
  i = 65;
  if (InputItems[i].Valid) {
    gSettings.RtVariables.BooterConfig = InputItems[i].IValue & 0x7F;
  }
  i++; //66
  if (InputItems[i].Valid) {
    gSettings.RtVariables.CsrActiveConfig = InputItems[i].IValue;
  }

  i++; //67
  if (InputItems[i].Valid) {
    gSettings.ACPI.DSDT.FixDsdt = InputItems[i].IValue;
  }
  i++; //68
  if (InputItems[i].Valid) {
    GlobalConfig.OptionsBits = InputItems[i].IValue;
  }
  i++; //69
  if (InputItems[i].Valid) {
    GlobalConfig.FlagsBits = InputItems[i].IValue;
  }


  i++; //70
  if (InputItems[i].Valid) {
    INTN Minus = 0;
    if (InputItems[i].SValue[0] == '-') {
      Minus = 1;
    }
    gSettings.GUI.Mouse.PointerSpeed = StrDecimalToUintn(InputItems[i].SValue.data(Minus));
    if (Minus) {
      gSettings.GUI.Mouse.PointerSpeed = -gSettings.GUI.Mouse.PointerSpeed;
    }
//    DBG("Pointer Speed=%d\n", gSettings.PointerSpeed);
  }
  i++; //71
  if (InputItems[i].Valid) {
    gSettings.GUI.Mouse.DoubleClickTime = StrDecimalToUintn(InputItems[i].SValue.wc_str());
//    DBG("DoubleClickTime=%d ms\n", gSettings.DoubleClickTime);
  }
  i++; //72
  if (InputItems[i].Valid) {
    gSettings.GUI.Mouse.PointerMirror = InputItems[i].BValue;
  }


  i = 74;
  if (InputItems[i].Valid) {
    gSettings.Devices.USB.USBFixOwnership = InputItems[i].BValue;
  }
  i++; //75
  if (InputItems[i].Valid) {
    GlobalConfig.C3Latency = (UINT16)StrHexToUint64(InputItems[i].SValue.wc_str());
  }

  i++; //76
  if (InputItems[i].Valid) {
    GlobalConfig.EnabledCores = (UINT8)StrDecimalToUintn(InputItems[i].SValue.wc_str());
  }
  i++; //77
  if (InputItems[i].Valid) {
    gSettings.CPU.SavingMode = (UINT8)StrDecimalToUintn(InputItems[i].SValue.wc_str());
  }

  i++; //78
  if (InputItems[i].Valid) {
	  gSettings.Smbios.ProductName = InputItems[i].SValue;
    // let's fill all other fields based on this ProductName
    // to serve as default
    Model = GetModelFromString(gSettings.Smbios.ProductName);
    if (Model != MaxMachineType) {
      SetDMISettingsForModel(Model, &gSettings, &GlobalConfig);
    }
  }

  i++; //79
  if (InputItems[i].Valid) {
	  gSettings.Smbios.SystemVersion = InputItems[i].SValue;
  }
  i++; //80
  if (InputItems[i].Valid) {
	  gSettings.Smbios.SerialNr = InputItems[i].SValue;
  }
  i++; //81
  if (InputItems[i].Valid) {
	  gSettings.Smbios.BoardNumber = InputItems[i].SValue;
  }
  i++; //82
  if (InputItems[i].Valid) {
	  gSettings.Smbios.BoardSerialNumber = InputItems[i].SValue;
  }
  i++; //83
  if (InputItems[i].Valid) {
    gSettings.Smbios.BoardType = (UINT8)(StrDecimalToUintn(InputItems[i].SValue.wc_str()) & 0x0F);
  }
  i++; //84
  if (InputItems[i].Valid) {
	  gSettings.Smbios.BoardVersion = InputItems[i].SValue;
  }
  i++; //85
  if (InputItems[i].Valid) {
    gSettings.Smbios.ChassisType = (UINT8)(StrDecimalToUintn(InputItems[i].SValue.wc_str()) & 0x0F);
  }
  i++; //86
  if (InputItems[i].Valid) {
	  GlobalConfig.BiosVersionUsed = InputItems[i].SValue;
  }
  i++; //87
  if (InputItems[i].Valid) {
	  GlobalConfig.ReleaseDateUsed = InputItems[i].SValue;
  }

  i++; //88
  if (InputItems[i].Valid) {
    gSettings.ACPI.SSDT.DoubleFirstState = InputItems[i].BValue;
  }
  i++; //89
  if (InputItems[i].Valid) {
    gSettings.ACPI.SSDT.EnableC7 = InputItems[i].BValue;
  }

  i++; //90
  if (InputItems[i].Valid) {
//    TagDict* dict;
    /*Status = */gConf.ReLoadConfig(XStringW(ConfigsList[OldChosenConfig])/*, &dict*/); // TODO: make a ReloadConfig, because in case of a reload, there are probably slightly different things to do.
//    if (!EFI_ERROR(Status)) {
      GlobalConfig.gBootChanged = TRUE;
      GlobalConfig.gThemeChanged = TRUE;
//      Status = GetUserSettings(dict, gSettings);
//      if (gConfigDict[2]) gConfigDict[2]->FreeTag();
//      gConfigDict[2] = dict;
//      GlobalConfig.ConfigName.takeValueFrom(ConfigsList[OldChosenConfig]);
//    }
    FillInputs(FALSE);
    NeedSave = FALSE;
  }
  i++; //91
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPKernelLapic = InputItems[i].BValue;
    GlobalConfig.gBootChanged = TRUE;
  }
  i++; //92
  if (InputItems[i].Valid) {
    gSettings.Devices.USB.USBInjection = InputItems[i].BValue;
  }
  i++; //93
  if (InputItems[i].Valid) {
    gSettings.Devices.USB.InjectClockID = InputItems[i].BValue;
  }
  i++; //94
  if (InputItems[i].Valid) {
    gSettings.Devices.FakeID.FakeATI = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
  }
  i++; //95
  if (InputItems[i].Valid) {
    gSettings.Devices.FakeID.FakeNVidia = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
  }
  i++; //96
  if (InputItems[i].Valid) {
    gSettings.Devices.FakeID.FakeIntel = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
    DBG("applied FakeIntel=0x%X\n", gSettings.Devices.FakeID.FakeIntel);
  }
  i++; //97
  if (InputItems[i].Valid) {
    gSettings.Devices.FakeID.FakeLAN = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
  }
  i++; //98
  if (InputItems[i].Valid) {
    gSettings.Devices.FakeID.FakeWIFI = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
  }
  i++; //99
  if (InputItems[i].Valid) {
    gSettings.Devices.FakeID.FakeSATA = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
  }
  i++; //100
  if (InputItems[i].Valid) {
    gSettings.Devices.FakeID.FakeXHCI = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
  }

  i++; //101  - Quirks
  if (InputItems[i].Valid) {
    gSettings.Quirks.QuirksMask = InputItems[i].IValue;
    gSettings.Quirks.OcBooterQuirks.AvoidRuntimeDefrag     = ((gSettings.Quirks.QuirksMask & QUIRK_DEFRAG) != 0); //1
    gSettings.Quirks.OcBooterQuirks.DevirtualiseMmio       = ((gSettings.Quirks.QuirksMask & QUIRK_MMIO) != 0);   //0
    gSettings.Quirks.OcBooterQuirks.DisableSingleUser      = ((gSettings.Quirks.QuirksMask & QUIRK_SU) != 0);     //0
    gSettings.Quirks.OcBooterQuirks.DisableVariableWrite   = ((gSettings.Quirks.QuirksMask & QUIRK_VAR) != 0);    //0
    gSettings.Quirks.OcBooterQuirks.DiscardHibernateMap    = ((gSettings.Quirks.QuirksMask & QUIRK_HIBER) != 0);  //0
    gSettings.Quirks.OcBooterQuirks.EnableSafeModeSlide    = ((gSettings.Quirks.QuirksMask & QUIRK_SAFE) != 0);   //1
    gSettings.Quirks.OcBooterQuirks.EnableWriteUnprotector = ((gSettings.Quirks.QuirksMask & QUIRK_UNPROT) != 0); //1
    gSettings.Quirks.OcBooterQuirks.ForceExitBootServices  = ((gSettings.Quirks.QuirksMask & QUIRK_EXIT) != 0);   //0
    gSettings.Quirks.OcBooterQuirks.ProtectMemoryRegions   = ((gSettings.Quirks.QuirksMask & QUIRK_REGION) != 0); //0
    gSettings.Quirks.OcBooterQuirks.ProtectSecureBoot      = ((gSettings.Quirks.QuirksMask & QUIRK_SECURE) != 0); //0
    gSettings.Quirks.OcBooterQuirks.ProtectUefiServices    = ((gSettings.Quirks.QuirksMask & QUIRK_UEFI) != 0);   //0
    gSettings.Quirks.OcBooterQuirks.ProvideCustomSlide     = ((gSettings.Quirks.QuirksMask & QUIRK_CUSTOM) != 0); //1
    gSettings.Quirks.OcBooterQuirks.RebuildAppleMemoryMap  = ((gSettings.Quirks.QuirksMask & QUIRK_MAP) != 0);    //0
    gSettings.Quirks.OcBooterQuirks.SetupVirtualMap        = ((gSettings.Quirks.QuirksMask & QUIRK_VIRT) != 0);   //1
    gSettings.Quirks.OcBooterQuirks.SignalAppleOS          = ((gSettings.Quirks.QuirksMask & QUIRK_OS) != 0);     //0
    gSettings.Quirks.OcBooterQuirks.SyncRuntimePermissions = ((gSettings.Quirks.QuirksMask & QUIRK_PERM) != 0);   //1
	  DBG("applied Quirks mask:%x\n", gSettings.Quirks.QuirksMask); //default is 0xA861
  }
  i++; //102
  if (InputItems[i].Valid) {
    gSettings.ACPI.DSDT.DebugDSDT = InputItems[i].BValue;
  }
  i++; //103
  if (InputItems[i].Valid) {
    gSettings.Devices.FakeID.FakeIMEI = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
  }

  i++; //104
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.FakeCPUID = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
    DBG("applied FakeCPUID=%06X\n", gSettings.KernelAndKextPatches.FakeCPUID);
    GlobalConfig.gBootChanged = TRUE;
  }

  i++; //105
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPKernelXCPM = InputItems[i].BValue;
    DBG("applied KernelXCPM\n");
    GlobalConfig.gBootChanged = TRUE;
  }

  i++; //106
  if (InputItems[i].Valid) {
    gSettings.Devices.StringInjector = InputItems[i].BValue;
  }

  i++; //107
  if (InputItems[i].Valid) {
    gSettings.Devices.NoDefaultProperties = InputItems[i].BValue;
  }

  i++; //108
  if (InputItems[i].Valid) {
    GlobalConfig.KernelPatchesAllowed = InputItems[i].BValue;
    GlobalConfig.gBootChanged = TRUE;
  }

  i++; //109
  if (InputItems[i].Valid) {
    gSettings.Graphics.DualLink = (UINT32)StrHexToUint64(InputItems[i].SValue.wc_str());
    DBG("applied DualLink=%X\n", gSettings.Graphics.DualLink);
  }

  i++; //110
  if (InputItems[i].Valid) {
    gSettings.Graphics.NvidiaNoEFI = InputItems[i].BValue;
  }

  i++; //111
  if (InputItems[i].Valid) {
    gSettings.Graphics.NvidiaSingle = InputItems[i].BValue;
  }
  i++; //112
  if (InputItems[i].Valid) {
    gSettings.Devices.IntelMaxValue = (UINT16)StrHexToUint64(InputItems[i].SValue.wc_str());
  }
  i++; //113
  if (InputItems[i].Valid) {
    gSettings.ACPI.AutoMerge = InputItems[i].BValue;
  }
  i++; //114
  if (InputItems[i].Valid) {
    gSettings.Graphics.RadeonDeInit = InputItems[i].BValue;
  }
  i++; //115
  if (InputItems[i].Valid) {
    gSettings.SystemParameters.NoCaches = InputItems[i].BValue;
  }
  i++; //116
  if (InputItems[i].Valid) {
    if (OldChosenDsdt == 0xFFFF) {
      gSettings.ACPI.DSDT.DsdtName = L"BIOS.aml"_XSW;
    } else {
      gSettings.ACPI.DSDT.DsdtName.SWPrintf("%ls", DsdtsList[OldChosenDsdt]);
    }
  }
  i++; //117
  if (InputItems[i].Valid) {
	  GlobalConfig.EfiVersionUsed = InputItems[i].SValue;
  }
  i++; //118
  if (InputItems[i].Valid) {
	  gSettings.RtVariables.BooterCfgStr = InputItems[i].SValue;
  }
  i++; //119
  if (InputItems[i].Valid) {
    EFI_DEVICE_PATH_PROTOCOL*  DevicePath = NULL;
    int TmpIndex;
    if (OldChosenAudio >= AudioList.size()) {
//      DBG("crasy OldChosenAudio = %lld\n", OldChosenAudio);
      OldChosenAudio = 0;
    }
    TmpIndex = OldChosenAudio & 0x2F;
//	  DBG("Chosen output %u:%ls_%s\n", TmpIndex, AudioList[TmpIndex].Name,
//        AudioOutputNames[AudioList[TmpIndex].Device]);

    DevicePath = DevicePathFromHandle(AudioList[TmpIndex].Handle);
//    DBG("choosen sound devicepath=%ls\n", DevicePathToStr(DevicePath));
    if (DevicePath != NULL) {
      SetNvramVariable(L"Clover.SoundDevice", &gEfiAppleBootGuid,
                       EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                       GetDevicePathSize(DevicePath), (UINT8 *)DevicePath);
      SetNvramVariable(L"Clover.SoundIndex", &gEfiAppleBootGuid,
                       EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                       1, (UINT8 *)&TmpIndex);
//      DBG(" sound written to nvram variables\n");
    }
  }
  i++; //120
  if (InputItems[i].Valid) {
    DefaultAudioVolume = (UINT8)StrDecimalToUintn(InputItems[i].SValue.wc_str());
//    DBG(" set output volume to %d\n", DefaultAudioVolume);
    if (DefaultAudioVolume > 100) {
        // correct wrong input
        DefaultAudioVolume = 90;
        InputItems[i].SValue.SWPrintf("%04d", DefaultAudioVolume);
    }
    SetNvramVariable(L"Clover.SoundVolume", &gEfiAppleBootGuid,
                     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                     1, &DefaultAudioVolume);
  }
  i++; //121
  if (InputItems[i].Valid) {
    gSettings.KernelAndKextPatches.KPPanicNoKextDump = InputItems[i].BValue;
    GlobalConfig.gBootChanged = TRUE;
  }
  i++; //122
  if (InputItems[i].Valid) {
    gSettings.Quirks.OcBooterQuirks.ProvideMaxSlide = (uint8_t)StrDecimalToUintn(InputItems[i].SValue.wc_str());
    DBG(" set MaxSlide = %hhu\n", gSettings.Quirks.OcBooterQuirks.ProvideMaxSlide);
  }
  i++; //123
  if (InputItems[i].Valid) {
    gSettings.GUI.ProvideConsoleGop = InputItems[i].BValue;
    DBG("applied ConsoleGopEnable=%s\n", gSettings.GUI.ProvideConsoleGop ? "Y" : "N" );
  }


  if (NeedSave) {
    ApplySettings();
  }
}


void AboutRefit(void)
{
  if (AboutMenu.Entries.size() == 0) {
    AboutMenu.Daylight = ThemeX.Daylight;
    if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
      AboutMenu.TitleImage.ImageNight.setEmpty();
      AboutMenu.TitleImage = ThemeX.GetIcon(BUILTIN_ICON_FUNC_ABOUT);
    }
//    else {
//      AboutMenu.TitleImage.setEmpty(); //done in the constructor
//    }
//    AboutMenu.AddMenuInfo_f(("Clover Version 5.0"));
    if ( "unknown"_XS8 != LString8(gRevisionStr) ) AboutMenu.AddMenuInfo_f("%s", gRevisionStr);
    if ( "unknown"_XS8 != LString8(gFirmwareBuildDate) ) AboutMenu.AddMenuInfo_f(" Build: %s", gFirmwareBuildDate);
    if ( "unknown"_XS8 != gBuildId ) AboutMenu.AddMenuInfo_f(" Build id: %s", gBuildId.c_str());
    AboutMenu.AddMenuInfo_f("%s", path_independant.c_str()); // trick to let the compiler think path_independant is used and must be kept in binary.
    AboutMenu.Entries[AboutMenu.Entries.size()-1].Title.SWPrintf("%s", gBuildIdGrepTag.c_str());
    AboutMenu.Entries[AboutMenu.Entries.size()-1].Title.SWPrintf(" ");
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
    AboutMenu.AddMenuInfo_f(" ");
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
	  AboutMenu.AddMenuInfo_f(" Screen Output: %s", egScreenDescription().c_str());
    AboutMenu.GetAnime();
    AboutMenu.AddMenuEntry(&MenuEntryReturn, false);
  } else if (AboutMenu.Entries.size() >= 2) {
    /*
      EntryCount instead of InfoLineCount. Lastline == return/back. Is necessary recheck screen res here?
    */
 //   FreePool(AboutMenu.Entries[AboutMenu.Entries.size()-2].Title); //what is FreePool(XStringW)?

    AboutMenu.Entries[AboutMenu.Entries.size()-2].Title.SWPrintf(" Screen Output: %s", egScreenDescription().c_str());
  }

  AboutMenu.RunMenu(NULL);
}

void HelpRefit(void)
{
  if (HelpMenu.Entries.size() == 0) {
    HelpMenu.Daylight = ThemeX.Daylight;
    if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
      HelpMenu.TitleImage.ImageNight.setEmpty();
      HelpMenu.TitleImage = ThemeX.GetIcon(BUILTIN_ICON_FUNC_HELP);
    }
    //else {
    //  HelpMenu.TitleImage.setEmpty();
    //}
    switch (gSettings.GUI.languageCode)
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
    HelpMenu.GetAnime();
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


//
// user-callable dispatcher functions
//

REFIT_ABSTRACT_MENU_ENTRY* NewEntry_(REFIT_ABSTRACT_MENU_ENTRY *Entry, REFIT_MENU_SCREEN **SubScreen, ACTION AtClick, UINTN ID, const XString8& Title)
{
  Entry->Title = Title;
  Entry->Image =  OptionMenu.TitleImage;
  Entry->AtClick = AtClick;
  // create the submenu
  *SubScreen = new REFIT_MENU_SCREEN;
  (*SubScreen)->Title = Entry->Title;
  (*SubScreen)->TitleImage = Entry->Image;
  (*SubScreen)->ID = ID;
  (*SubScreen)->GetAnime();
  Entry->SubScreen = *SubScreen;
  return Entry;
}

REFIT_MENU_ITEM_OPTIONS* newREFIT_MENU_ITEM_OPTIONS(REFIT_MENU_SCREEN **SubScreen, ACTION AtClick, UINTN ID, const XString8& Title)
{
	REFIT_MENU_ITEM_OPTIONS* Entry = new REFIT_MENU_ITEM_OPTIONS;
	return NewEntry_(Entry, SubScreen, AtClick, ID, Title)->getREFIT_MENU_ITEM_OPTIONS();
}

void ModifyTitles(REFIT_ABSTRACT_MENU_ENTRY *ChosenEntry)
{
  if (ChosenEntry->SubScreen->ID == SCREEN_DSDT) {
    ChosenEntry->Title.SWPrintf("DSDT fix mask [0x%08x]->", gSettings.ACPI.DSDT.FixDsdt);
    //MsgLog("@ESC: %ls\n", (*ChosenEntry)->Title);
  } else if (ChosenEntry->SubScreen->ID == SCREEN_CSR) {
    // CSR
    ChosenEntry->Title.SWPrintf("System Integrity Protection [0x%04x]->", gSettings.RtVariables.CsrActiveConfig);
    // check for the right booter flag to allow the application
    // of the new System Integrity Protection configuration.
    if (gSettings.RtVariables.CsrActiveConfig != 0 && gSettings.RtVariables.BooterConfig == 0) {
      gSettings.RtVariables.BooterConfig = 0x28;
    }
  } else if (ChosenEntry->SubScreen->ID == SCREEN_BLC) {
	  ChosenEntry->Title.SWPrintf("boot_args->flags [0x%04hx]->", gSettings.RtVariables.BooterConfig);
  }
}

REFIT_ABSTRACT_MENU_ENTRY *SubMenuGraphics()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_GRAPHICS, "Graphics Injector->"_XS8);
	SubScreen->AddMenuInfoLine_f("Number of VideoCard%s=%zu",((gConf.GfxPropertiesArray.size()!=1)?"s":""), gConf.GfxPropertiesArray.size());
  SubScreen->AddMenuItemInput(52, "InjectEDID", FALSE);
  SubScreen->AddMenuItemInput(53, "Fake Vendor EDID:", TRUE);
  SubScreen->AddMenuItemInput(54, "Fake Product EDID:", TRUE);
  SubScreen->AddMenuItemInput(18, "Backlight Level:", TRUE);
  SubScreen->AddMenuItemInput(112, "Intel Max Backlight:", TRUE); //gSettings.Devices.IntelMaxValue


  for (UINTN i = 0; i < gConf.GfxPropertiesArray.size(); i++) {
    SubScreen->AddMenuInfo_f("----------------------");
	  SubScreen->AddMenuInfo_f("Card DeviceID=%04hx", gConf.GfxPropertiesArray[i].DeviceID);
    UINTN N = 20 + i * 6;
    SubScreen->AddMenuItemInput(N, "Model:", TRUE);

    if (gConf.GfxPropertiesArray[i].Vendor == Nvidia) {
      SubScreen->AddMenuItemInput(N+1, "InjectNVidia", FALSE);
    } else if (gConf.GfxPropertiesArray[i].Vendor == Ati) {
      SubScreen->AddMenuItemInput(N+1, "InjectATI", FALSE);
    } else if (gConf.GfxPropertiesArray[i].Vendor == Intel) {
      SubScreen->AddMenuItemInput(N+1, "InjectIntel", FALSE);
    } else {
      SubScreen->AddMenuItemInput(N+1, "InjectX3", FALSE);
    }

    UINTN  Ven = 97; //it can be used for non Ati, Nvidia, Intel in QEMU for example
    if (gConf.GfxPropertiesArray[i].Vendor == Nvidia) {
      Ven = 95;
    } else if (gConf.GfxPropertiesArray[i].Vendor == Ati) {
      Ven = 94;
    } else if (gConf.GfxPropertiesArray[i].Vendor == Intel) {
      Ven = 96;
    }

    if ((gConf.GfxPropertiesArray[i].Vendor == Ati) || (gConf.GfxPropertiesArray[i].Vendor == Intel)) {
      SubScreen->AddMenuItemInput(109, "DualLink:", TRUE);
    }
    if (gConf.GfxPropertiesArray[i].Vendor == Ati) {
      SubScreen->AddMenuItemInput(114, "DeInit:", TRUE);
    }

    SubScreen->AddMenuItemInput(Ven, "FakeID:", TRUE);

    if (gConf.GfxPropertiesArray[i].Vendor == Nvidia) {
      SubScreen->AddMenuItemInput(N+2, "DisplayCFG:", TRUE);
    } else if (gConf.GfxPropertiesArray[i].Vendor == Ati) {
      SubScreen->AddMenuItemInput(N+2, "FBConfig:", TRUE);
    } else /*if (gGraphics[i].Vendor == Intel)*/{
      SubScreen->AddMenuItemInput(N+2, "*-platform-id:", TRUE);
    }

    // ErmaC: NvidiaGeneric entry
    if (gConf.GfxPropertiesArray[i].Vendor == Nvidia) {
      SubScreen->AddMenuItemInput(55, "Generic NVIDIA name", FALSE);
      SubScreen->AddMenuItemInput(110, "NVIDIA No EFI", FALSE);
      SubScreen->AddMenuItemInput(111, "NVIDIA Single", FALSE);
      SubScreen->AddMenuItemInput(56, "Use NVIDIA WEB drivers", FALSE);
    }

    if (gConf.GfxPropertiesArray[i].Vendor == Intel) {
      continue;
    }
    SubScreen->AddMenuItemInput(N+3, "Ports:", TRUE);

    if (gConf.GfxPropertiesArray[i].Vendor == Nvidia) {
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
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_AUDIO, "Audio tuning->"_XS8);

  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose options to tune the HDA devices");
	SubScreen->AddMenuInfoLine_f("Number of Audio Controller%s=%zu", ((gConf.HdaPropertiesArray.size()!=1)?"s":""), gConf.HdaPropertiesArray.size());
  for (i = 0 ; i < gConf.HdaPropertiesArray.size() ; i++) {
	  SubScreen->AddMenuInfoLine_f("%llu) %ls [%04hX][%04hX]",
                                           (i+1),
                                           gConf.HdaPropertiesArray[i].controller_name,
                                           gConf.HdaPropertiesArray[i].controller_vendor_id,
                                           gConf.HdaPropertiesArray[i].controller_device_id
                      );
  }

  //SubScreen->AddMenuItemInput(59, "HDAInjection", FALSE);
  if (gSettings.Devices.Audio.HDAInjection) {
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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_CPU, "CPU tuning->"_XS8);
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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_KEXTS, "Custom kexts patches->"_XS8);

  for ( size_t Index = 0; Index < gSettings.KernelAndKextPatches.KextPatches.size(); Index++) {
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.SWPrintf("%90s", gSettings.KernelAndKextPatches.KextPatches[Index].Label.c_str());
//    InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &(gSettings.KernelAndKextPatches.KextPatches[Index].MenuItem);
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuKextBlockInjection(const XString8& UniSysVer)
{
  REFIT_MENU_ITEM_OPTIONS     *Entry = NULL;
  REFIT_MENU_SCREEN    *SubScreen = NULL;
  REFIT_INPUT_DIALOG   *InputBootArgs;

  for ( size_t idx = 0 ; idx < InjectKextList.size() ; idx ++ ) {
    SIDELOAD_KEXT& Kext = InjectKextList[idx];
    if ( Kext.KextDirNameUnderOEMPath == UniSysVer ) {
    	if ( SubScreen == NULL ) {
          Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_KEXT_INJECT, S8Printf("%s->", UniSysVer.c_str()));
          SubScreen->AddMenuInfoLine_f("Choose/check kext to disable:");
    	}
      InputBootArgs = new REFIT_INPUT_DIALOG;
      InputBootArgs->Title.SWPrintf("%ls, v.%ls", Kext.FileName.wc_str(), Kext.Version.wc_str());
//      InputBootArgs->Tag = TAG_INPUT;
      InputBootArgs->Row = 0xFFFF; //cursor
      InputBootArgs->Item = &(Kext.MenuItem);
      InputBootArgs->AtClick = ActionEnter;
      InputBootArgs->AtRightClick = ActionDetails;
      SubScreen->AddMenuEntry(InputBootArgs, true);

      for ( size_t idxPlugin = 0 ; idxPlugin < Kext.PlugInList.size() ; idxPlugin ++ ) {
        SIDELOAD_KEXT& plugInKext = Kext.PlugInList[idxPlugin];
        InputBootArgs = new REFIT_INPUT_DIALOG;
        InputBootArgs->Title.SWPrintf("  |-- %ls, v.%ls", plugInKext.FileName.wc_str(), plugInKext.Version.wc_str());
//        InputBootArgs->Tag = TAG_INPUT;
        InputBootArgs->Row = 0xFFFF; //cursor
        InputBootArgs->Item = &(plugInKext.MenuItem);
        InputBootArgs->AtClick = ActionEnter;
        InputBootArgs->AtRightClick = ActionDetails;
        SubScreen->AddMenuEntry(InputBootArgs, true);
      }
    }
  }

  if ( SubScreen != NULL ) SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

LOADER_ENTRY* LOADER_ENTRY::SubMenuKextInjectMgmt()
{
	LOADER_ENTRY       *SubEntry;
	REFIT_MENU_SCREEN  *SubSubScreen;

	SubEntry = new LOADER_ENTRY;
	NewEntry_(SubEntry, &SubSubScreen, ActionEnter, SCREEN_SYSTEM, "Block injected kexts->"_XS8);
	SubEntry->Flags = Flags;
	if (macOSVersion.notEmpty()) {

    XString8 OSVersionKextsDirName; // declare here to avoid multiple allocation

	  {
	    XString8 ShortOSVersion = macOSVersion.nbElement() == 1 ? macOSVersion.asString(1) : macOSVersion.asString(macOSVersion.nbElement()-1);
	    SubSubScreen->AddMenuInfoLine_f("Block injected kexts for target version of macOS: %s", ShortOSVersion.c_str());
	  }

    // Add kext from 10 or 11
		{
      OSVersionKextsDirName = macOSVersion.asString(1);
			SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection(OSVersionKextsDirName), true);
      OSVersionKextsDirName.S8Catf("_%s", getSuffixForMacOsVersion(LoaderType).c_str());
			SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection(OSVersionKextsDirName), true);
		}

		// Add kext from 10(or 11).{version}
		{
      OSVersionKextsDirName = macOSVersion.asString(2);
      if ( macOSVersion.elementAt(1) == -1 ) OSVersionKextsDirName.S8Catf(".0");
			SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection(OSVersionKextsDirName), true);
      OSVersionKextsDirName.S8Catf("_%s", getSuffixForMacOsVersion(LoaderType).c_str());
			SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection(OSVersionKextsDirName), true);
		}

		// Add kext from :
		// 10(or 11).{version}.0 if NO minor version
		// 10(or 11).{version}.{minor version} if minor version is > 0
		if ( macOSVersion.nbElement() >= 2 )
    {
      OSVersionKextsDirName = macOSVersion.asString(3);
      if ( macOSVersion.elementAt(1) == -1 ) OSVersionKextsDirName.S8Catf(".0");
      if ( macOSVersion.elementAt(2) == -1 ) OSVersionKextsDirName.S8Catf(".0");
      SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection(OSVersionKextsDirName), true);
      OSVersionKextsDirName.S8Catf("_%s", getSuffixForMacOsVersion(LoaderType).c_str());
			SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection(OSVersionKextsDirName), true);
		}
	}
	else {
		SubSubScreen->AddMenuInfoLine_f("Block injected kexts for unknown macOS version");
    SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection("Unknown"_XS8), true);
	}

	XStringW kextDir;
	kextDir = GetOtherKextsDir(TRUE);
	if ( kextDir.notEmpty() ) {
		SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection("Other"_XS8), true);
	}
	kextDir = GetOtherKextsDir(FALSE);
	if ( kextDir.notEmpty() ) {
		SubSubScreen->AddMenuEntry(SubMenuKextBlockInjection("Off"_XS8), true);
	}

	SubSubScreen->AddMenuEntry(&MenuEntryReturn, false);
	return SubEntry;
}



REFIT_ABSTRACT_MENU_ENTRY* SubMenuKernelPatches()
{
  REFIT_MENU_ITEM_OPTIONS     *Entry;
  REFIT_MENU_SCREEN    *SubScreen;
  REFIT_INPUT_DIALOG   *InputBootArgs;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_KERNELS, "Custom kernel patches->"_XS8);

  for (size_t Index = 0; Index < gSettings.KernelAndKextPatches.KernelPatches.size(); Index++) {
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.SWPrintf("%90s", gSettings.KernelAndKextPatches.KernelPatches[Index].Label.c_str());
//    InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &(gSettings.KernelAndKextPatches.KernelPatches[Index].MenuItem);
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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_BOOTER, "Custom booter patches->"_XS8);

  for (size_t Index = 0; Index < gSettings.KernelAndKextPatches.BootPatches.size(); Index++) {
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.SWPrintf("%90s", gSettings.KernelAndKextPatches.BootPatches[Index].Label.c_str());
//    InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &(gSettings.KernelAndKextPatches.BootPatches[Index].MenuItem);
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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_BINARIES, "Binaries patching->"_XS8);

  SubScreen->AddMenuInfoLine_f("%s", gCPUStructure.BrandString);
  SubScreen->AddMenuInfoLine_f("Real CPUID: 0x%06X", gCPUStructure.Signature);

  SubScreen->AddMenuItemInput(64,  "Debug", FALSE);
  SubScreen->AddMenuInfo_f("----------------------");
  SubScreen->AddMenuItemInput(104, "Fake CPUID:", TRUE);
  SubScreen->AddMenuItemInput(91,  "Kernel Lapic", FALSE);
  SubScreen->AddMenuItemInput(105, "Kernel XCPM", FALSE);
  SubScreen->AddMenuItemInput(48,  "Kernel PM", FALSE);
  SubScreen->AddMenuItemInput(121, "Panic No Kext Dump", FALSE);
  SubScreen->AddMenuEntry(SubMenuKernelPatches(), true);
  SubScreen->AddMenuInfo_f("----------------------");
  SubScreen->AddMenuItemInput(46,  "AppleIntelCPUPM Patch", FALSE);
  SubScreen->AddMenuItemInput(47,  "AppleRTC Patch", FALSE);
  SubScreen->AddMenuItemInput(45,  "No 8 Apples Patch", FALSE);
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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_TABLES, "Tables dropping->"_XS8);

  if (GlobalConfig.ACPIDropTables) {
    ACPI_DROP_TABLE *DropTable = GlobalConfig.ACPIDropTables;
    while (DropTable) {
      CopyMem((CHAR8*)&sign, (CHAR8*)&(DropTable->Signature), 4);
      CopyMem((CHAR8*)&OTID, (CHAR8*)&(DropTable->TableId), 8);
      //MsgLog("adding to menu %s (%X) %s (%lx) L=%d(0x%X)\n",
      //       sign, DropTable->Signature,
      //       OTID, DropTable->TableId,
      //       DropTable->Length, DropTable->Length);
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

  for ( size_t idx = 0 ; idx < ACPIPatchedAML.size() ; ++idx) {
    ACPI_PATCHED_AML& ACPIPatchedAMLTmp = ACPIPatchedAML[idx];
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.SWPrintf("Drop \"%s\"", ACPIPatchedAMLTmp.FileName.c_str());
//      InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &(ACPIPatchedAMLTmp.MenuItem);
    InputBootArgs->AtClick = ActionEnter;
    InputBootArgs->AtRightClick = ActionDetails;
    SubScreen->AddMenuEntry(InputBootArgs, true);
  }

  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuSmbios()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_SMBIOS, "SMBIOS->"_XS8);

	SubScreen->AddMenuInfoLine_f("%s", gCPUStructure.BrandString);
	SubScreen->AddMenuInfoLine_f("%s", GlobalConfig.OEMProductFromSmbios.c_str());
	SubScreen->AddMenuInfoLine_f("with board %s", GlobalConfig.OEMBoardFromSmbios.c_str());

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

REFIT_ABSTRACT_MENU_ENTRY* SubMenuDsdtFix()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry; //, *SubEntry;
  REFIT_MENU_SCREEN  *SubScreen;
//  REFIT_INPUT_DIALOG *InputBootArgs;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DSDT, NullXString8);
  //  Entry->Title.SPrintf("DSDT fix mask [0x%08hhx]->", gSettings.ACPI.DSDT.FixDsdt);

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

REFIT_ABSTRACT_MENU_ENTRY* SubMenuDSDTPatches()
{
  REFIT_MENU_ITEM_OPTIONS     *Entry;
  REFIT_MENU_SCREEN    *SubScreen;
  REFIT_INPUT_DIALOG   *InputBootArgs;

  size_t        PatchDsdtNum = gSettings.ACPI.DSDT.DSDTPatchArray.size();
//  INPUT_ITEM*   DSDTPatchesMenu = gSettings.PatchDsdtMenuItem;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DSDT_PATCHES, "Custom DSDT patches->"_XS8);

  for (size_t Index = 0; Index < PatchDsdtNum; Index++) {
    InputBootArgs = new REFIT_INPUT_DIALOG;
    InputBootArgs->Title.SWPrintf("%90s", gSettings.ACPI.DSDT.DSDTPatchArray[Index].PatchDsdtLabel.c_str());
//    InputBootArgs->Tag = TAG_INPUT;
    InputBootArgs->Row = 0xFFFF; //cursor
    InputBootArgs->Item = &gSettings.ACPI.DSDT.DSDTPatchArray[Index].PatchDsdtMenuItem;
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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_ACPI, "Dsdt name->"_XS8);

  SubScreen->AddMenuInfoLine_f("Select a DSDT file:");
  SubScreen->AddMenuItemSwitch(116,  "BIOS.aml", FALSE);

  for (i = 0; i < DsdtsNum; i++) {
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
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_ACPI, "ACPI patching->"_XS8);

  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose options to patch ACPI");

  SubScreen->AddMenuItemInput(102, "Debug DSDT", FALSE);

  SubScreen->AddMenuEntry(SubMenuDsdts(), true);
  SubScreen->AddMenuEntry(SubMenuDropTables(), true);
//  SubScreen->AddMenuEntry(SubMenuDropDSM(), true);
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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_AUDIOPORTS, "Startup sound output->"_XS8);

  SubScreen->AddMenuInfoLine_f("Select an audio output, press F7 to test");
  SubScreen->AddMenuItemInput(120, "Volume:", TRUE);

  for (i = 0; i < AudioList.size(); i++) {
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title.SWPrintf("%ls_%s", AudioList[i].Name.wc_str(), AudioOutputNames[AudioList[i].Device]);
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

void CreateMenuProps(REFIT_MENU_SCREEN* SubScreen, SETTINGS_DATA::DevicesClass::SimplePropertyClass* Prop)
{
	REFIT_INPUT_DIALOG  *InputBootArgs;

  InputBootArgs = new REFIT_INPUT_DIALOG;
	InputBootArgs->Title.SWPrintf("  key: %s", Prop->Key.c_str());
	InputBootArgs->Row = 0xFFFF; //cursor
	InputBootArgs->Item = &Prop->MenuItem;
	InputBootArgs->AtClick = ActionEnter;
	InputBootArgs->AtRightClick = ActionDetails;
	SubScreen->AddMenuEntry(InputBootArgs, true);
	switch (Prop->ValueType) {
	case kTagTypeInteger:
			SubScreen->AddMenuInfo_f("     value: 0x%08llx", *(UINT64*)Prop->Value.data());
		break;
	case kTagTypeString:
			SubScreen->AddMenuInfo_f("     value: %90s", Prop->Value.data());
		break;
	case   kTagTypeFalse:
		SubScreen->AddMenuInfo_f(("     value: false"));
		break;
	case   kTagTypeTrue:
		SubScreen->AddMenuInfo_f(("     value: true"));
		break;
  case   kTagTypeFloat:
    SubScreen->AddMenuInfo_f("     value: %f", *(float*)Prop->Value.data());
    break;
	default: //type data, print first 24 bytes
			 //CHAR8* Bytes2HexStr(UINT8 *data, UINTN len)
			SubScreen->AddMenuInfo_f("     value[%zu]: %24s", Prop->Value.size(), Bytes2HexStr((UINT8*)Prop->Value.data(), MIN(24, Prop->Value.size())).c_str());
		break;
	}
}

void CreateMenuAddProp(REFIT_MENU_SCREEN* SubScreen, SETTINGS_DATA::DevicesClass::AddPropertyClass* Prop)
{
  REFIT_INPUT_DIALOG  *InputBootArgs;

  InputBootArgs = new REFIT_INPUT_DIALOG;
  InputBootArgs->Title.SWPrintf("  key: %s", Prop->Key.c_str());
  InputBootArgs->Row = 0xFFFF; //cursor
  InputBootArgs->Item = &Prop->MenuItem;
  InputBootArgs->AtClick = ActionEnter;
  InputBootArgs->AtRightClick = ActionDetails;
  SubScreen->AddMenuEntry(InputBootArgs, true);
  switch (Prop->ValueType) {
  case kTagTypeInteger:
      SubScreen->AddMenuInfo_f("     value: 0x%08llx", *(UINT64*)Prop->Value.data());
    break;
  case kTagTypeString:
      SubScreen->AddMenuInfo_f("     value: %90s", Prop->Value.data());
    break;
  case   kTagTypeFalse:
    SubScreen->AddMenuInfo_f(("     value: false"));
    break;
  case   kTagTypeTrue:
    SubScreen->AddMenuInfo_f(("     value: true"));
    break;
  case   kTagTypeFloat:
    SubScreen->AddMenuInfo_f("     value: %f", *(float*)Prop->Value.data());
    break;
  default: //type data, print first 24 bytes
       //CHAR8* Bytes2HexStr(UINT8 *data, UINTN len)
      SubScreen->AddMenuInfo_f("     value[%zu]: %24s", Prop->Value.size(), Bytes2HexStr((UINT8*)Prop->Value.data(), MIN(24, Prop->Value.size())).c_str());
    break;
  }
}
//
//REFIT_ABSTRACT_MENU_ENTRY* SubMenuCustomDevices()
//{
//  REFIT_MENU_ITEM_OPTIONS    *Entry;
//  REFIT_MENU_SCREEN   *SubScreen;
//
//  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DEVICES, "Old Custom properties->"_XS8);
//
//  for ( size_t idx = 0 ; idx < gSettings.Devices.newProperties.array.size(); ++idx)
//  {
//    SETTINGS_DATA::DevicesClass::PropertiesClass::PropertyDict& Prop = gSettings.Devices.newProperties.array[idx];
//
//    SubScreen->AddMenuInfo_f("------------");
//    SubScreen->AddMenuInfo_f("%ls", Prop.DevicePathAsString.wc_str());
//    for ( size_t idxChild = 0 ; idxChild < Prop.propertiesArray.size(); ++idxChild) {
//      SETTINGS_DATA::DevicesClass::SimplePropertyClass& Props = Prop.propertiesArray[idxChild];
//      CreateMenuProps(SubScreen, &Props);
//    }
//  }
//  for ( size_t idx = 0 ; idx < gSettings.Devices.newArbitrary.size()  ; ++idx) {
//    SETTINGS_DATA::DevicesClass::ArbitraryProperty& Prop = gSettings.Devices.newArbitrary[idx];
//    SubScreen->AddMenuInfo_f("------------");
//    for ( size_t idxChild = 0 ; idxChild < Prop.CustomPropertyArray.size(); ++idxChild) {
//      SETTINGS_DATA::DevicesClass::SimplePropertyClass& Props = Prop.CustomPropertyArray[idxChild];
//      SubScreen->AddMenuInfo_f("%s", Prop.Label.c_str());
//      CreateMenuProps(SubScreen, &Props);
//    }
//  }
//  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
//  Entry->SubScreen = SubScreen;
//  return Entry;
//}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuProperties()
{
  REFIT_MENU_ITEM_OPTIONS    *Entry;
  REFIT_MENU_SCREEN   *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DEVICES, "Properties->"_XS8);

  for ( size_t idx = 0 ; idx < gSettings.Devices.Properties.PropertyArray.size(); ++idx)
  {
    SETTINGS_DATA::DevicesClass::PropertiesClass::PropertyClass& Prop = gSettings.Devices.Properties.PropertyArray[idx];

    if ( idx > 0 ) SubScreen->AddMenuInfo_f("------------");
    SubScreen->AddMenuInfo_f("%ls", Prop.DevicePathAsString.wc_str());
    for ( size_t idxChild = 0 ; idxChild < Prop.propertiesArray.size(); ++idxChild) {
      SETTINGS_DATA::DevicesClass::SimplePropertyClass& Props = Prop.propertiesArray[idxChild];
      CreateMenuProps(SubScreen, &Props);
    }
  }
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  Entry->SubScreen = SubScreen;
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuArbProperties()
{
  REFIT_MENU_ITEM_OPTIONS    *Entry;
  REFIT_MENU_SCREEN   *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DEVICES, "Arbitrary properties->"_XS8);

  for ( size_t idx = 0 ; idx < gSettings.Devices.ArbitraryArray.size()  ; ++idx) {
    SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass& Prop = gSettings.Devices.ArbitraryArray[idx];
    if ( idx > 0 ) SubScreen->AddMenuInfo_f("------------");
    for ( size_t idxChild = 0 ; idxChild < Prop.CustomPropertyArray.size(); ++idxChild) {
      SETTINGS_DATA::DevicesClass::SimplePropertyClass& Props = Prop.CustomPropertyArray[idxChild];
      SubScreen->AddMenuInfo_f("%s", Prop.Label.c_str());
      CreateMenuProps(SubScreen, &Props);
    }
  }
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  Entry->SubScreen = SubScreen;
  return Entry;
}

REFIT_ABSTRACT_MENU_ENTRY* SubMenuAddProperties()
{
  REFIT_MENU_ITEM_OPTIONS    *Entry;
  REFIT_MENU_SCREEN   *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_DEVICES, "Add properties->"_XS8);

  for ( size_t idx = 0 ; idx < gSettings.Devices.AddPropertyArray.size()  ; ++idx) {
    SETTINGS_DATA::DevicesClass::AddPropertyClass& Prop = gSettings.Devices.AddPropertyArray[idx];
//    SubScreen->AddMenuInfo_f("%s", Prop.Label.c_str());
    CreateMenuAddProp(SubScreen, &Prop);
  }
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  Entry->SubScreen = SubScreen;
  return Entry;
}



REFIT_ABSTRACT_MENU_ENTRY* SubMenuPCI()
{
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_USB, "PCI devices->"_XS8);

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
//  SubScreen->AddMenuEntry(SubMenuCustomDevices(), true);
  SubScreen->AddMenuEntry(SubMenuProperties(), true);
  SubScreen->AddMenuEntry(SubMenuArbProperties(), true);
  SubScreen->AddMenuEntry(SubMenuAddProperties(), true);

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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_THEME, "Themes->"_XS8);

  SubScreen->AddMenuInfoLine_f("Installed themes:");
  //add embedded
  SubScreen->AddMenuItemSwitch(3,  "embedded", FALSE);

  for (i = 0; i < ThemeNameArray.size(); i++) {
    InputBootArgs = new REFIT_MENU_SWITCH;
    InputBootArgs->Title.takeValueFrom(ThemeNameArray[i]);
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
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_GUI, "GUI tuning->"_XS8);

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
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_CSR, NullXString8);

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
  SubScreen->AddMenuCheck("Allow Executable Policy Override", CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE, 66);
  SubScreen->AddMenuCheck("Allow Non-authenticated Root", CSR_ALLOW_NON_AUTHENTICATED_ROOT, 66);

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
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_BLC, NullXString8);
//  Entry->Title.SPrintf("boot_args->flags [0x%02hhx]->", gSettings.RtVariables.BooterConfig);

  // submenu description
  SubScreen->AddMenuInfoLine_f("Modify flags for boot.efi");

  SubScreen->AddMenuCheck("Reboot On Panic",    kBootArgsFlagRebootOnPanic, 65);
  SubScreen->AddMenuCheck("Hi DPI",             kBootArgsFlagHiDPI, 65);
  SubScreen->AddMenuCheck("Black Screen",       kBootArgsFlagBlack, 65);
  SubScreen->AddMenuCheck("CSR Active Config",  kBootArgsFlagCSRActiveConfig, 65);
  SubScreen->AddMenuCheck("CSR Pending Config", kBootArgsFlagCSRConfigMode, 65);
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
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_SYSTEM, "System Parameters->"_XS8);

  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose options for booted OS");

  SubScreen->AddMenuItemInput(2,  "Block kext:", TRUE);
  SubScreen->AddMenuItemInput(51, "Set OS version if not detected:", TRUE);
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

  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_THEME, "Configs->"_XS8);

  SubScreen->AddMenuInfoLine_f("Select a config file:");

  for (i = 0; i < ConfigsNum; i++) {
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

REFIT_ABSTRACT_MENU_ENTRY* SubMenuQuirks()
{
  // init
  REFIT_MENU_ITEM_OPTIONS   *Entry;
  REFIT_MENU_SCREEN  *SubScreen;
  
  // create the entry in the main menu
  Entry = newREFIT_MENU_ITEM_OPTIONS(&SubScreen, ActionEnter, SCREEN_QUIRKS, NullXString8);
  Entry->Title.SWPrintf("Quirks mask [0x%04x]->", gSettings.Quirks.QuirksMask);
  
  // submenu description
  SubScreen->AddMenuInfoLine_f("Choose options to fix memory");
  
  SubScreen->AddMenuCheck("AvoidRuntimeDefrag",     QUIRK_DEFRAG, 101);
  SubScreen->AddMenuCheck("DevirtualiseMmio",       QUIRK_MMIO, 101);
  SubScreen->AddMenuCheck("DisableSingleUser",      QUIRK_SU, 101);
  SubScreen->AddMenuCheck("DisableVariableWrite",   QUIRK_VAR, 101);
  SubScreen->AddMenuCheck("DiscardHibernateMap",    QUIRK_HIBER, 101);
  SubScreen->AddMenuCheck("EnableSafeModeSlide",    QUIRK_SAFE, 101);
  SubScreen->AddMenuCheck("EnableWriteUnprotector", QUIRK_UNPROT, 101);
  SubScreen->AddMenuCheck("ForceExitBootServices",  QUIRK_EXIT, 101);
  SubScreen->AddMenuCheck("ProtectMemoryRegions",   QUIRK_REGION, 101);
  SubScreen->AddMenuCheck("ProtectSecureBoot",      QUIRK_SECURE, 101);
  SubScreen->AddMenuCheck("ProtectUefiServices",    QUIRK_UEFI, 101);
  SubScreen->AddMenuItemInput(123, "ProvideConsoleGopEnable", FALSE);
  SubScreen->AddMenuCheck("ProvideCustomSlide",     QUIRK_CUSTOM, 101);
//decimal
  SubScreen->AddMenuItemInput(122, "ProvideMaxSlide:", TRUE);
  SubScreen->AddMenuCheck("RebuildAppleMemoryMap",  QUIRK_MAP, 101);
  SubScreen->AddMenuCheck("SetupVirtualMap",        QUIRK_VIRT, 101);
  SubScreen->AddMenuCheck("SignalAppleOS",          QUIRK_OS, 101);
  SubScreen->AddMenuCheck("SyncRuntimePermissions", QUIRK_PERM, 101);
  
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  ModifyTitles(Entry);
  
  return Entry;
}


void  OptionsMenu(OUT REFIT_ABSTRACT_MENU_ENTRY **ChosenEntry)
{
  REFIT_ABSTRACT_MENU_ENTRY    *TmpChosenEntry = NULL;
  REFIT_ABSTRACT_MENU_ENTRY    *NextChosenEntry = NULL;
  UINTN               MenuExit = 0;
  UINTN               SubMenuExit;
  UINTN               NextMenuExit;
  //CHAR16*           Flags;

//  MENU_STYLE_FUNC     Style = &REFIT_MENU_SCREEN::TextMenuStyle;

  INTN                EntryIndex = 0;
  INTN                SubEntryIndex = -1; //value -1 means old position to remember
  INTN                NextEntryIndex = -1;

  BOOLEAN             OldFontStyle = ThemeX.Proportional;
  ThemeX.Proportional = FALSE; //temporary disable proportional

//  if (AllowGraphicsMode) {
//    Style = &REFIT_MENU_SCREEN::GraphicsMenuStyle;
//  }

  // remember, if you extended this menu then change procedures
  // FillInputs and ApplyInputs
  gThemeOptionsChanged = FALSE;

  if (OptionMenu.Entries.size() == 0) {
    OptionMenu.Daylight = ThemeX.Daylight;
    if (!(ThemeX.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
      OptionMenu.TitleImage.ImageNight.setEmpty();
      OptionMenu.TitleImage = ThemeX.GetIcon(BUILTIN_ICON_FUNC_OPTIONS);
    }
    gThemeOptionsChanged = TRUE;
    OptionMenu.ID = SCREEN_OPTIONS;
    OptionMenu.GetAnime(); //FALSE;

    OptionMenu.AddMenuItemInput(0, "Boot Args:", TRUE);

//    AddMenuItemInput(&OptionMenu, 90, "Config:", TRUE);
//   InputBootArgs->ShortcutDigit = 0xF1;
    OptionMenu.AddMenuEntry( SubMenuConfigs(), true);

    if (AllowGraphicsMode) {
      OptionMenu.AddMenuEntry( SubMenuGUI(), true);
    }
    OptionMenu.AddMenuEntry( SubMenuACPI(), true);
    OptionMenu.AddMenuEntry( SubMenuSmbios(), true);
    OptionMenu.AddMenuEntry( SubMenuBinaries(), true);
    OptionMenu.AddMenuEntry( SubMenuQuirks(), true);
    OptionMenu.AddMenuEntry( SubMenuGraphics(), true);
    OptionMenu.AddMenuEntry( SubMenuPCI(), true);
    OptionMenu.AddMenuEntry( SubMenuSpeedStep(), true);
    OptionMenu.AddMenuEntry( SubMenuAudio(), true);
    OptionMenu.AddMenuEntry( SubMenuAudioPort(), true);
    OptionMenu.AddMenuEntry( SubMenuSystem(), true);
    OptionMenu.AddMenuEntry( &MenuEntryReturn, false);
    //DBG("option menu created entries=%d\n", OptionMenu.Entries.size());
  }

  while (!MenuExit) {
    MenuExit = OptionMenu.RunGenericMenu(&EntryIndex, ChosenEntry);
    if (MenuExit == MENU_EXIT_ESCAPE || (*ChosenEntry)->getREFIT_MENU_ITEM_RETURN())
      break;
    if (MenuExit == MENU_EXIT_ENTER || MenuExit == MENU_EXIT_DETAILS) {
      //enter input dialog or subscreen
      if ((*ChosenEntry)->SubScreen != NULL) {
        SubMenuExit = 0;
        while (!SubMenuExit) {
          SubMenuExit = (*ChosenEntry)->SubScreen->RunGenericMenu(&SubEntryIndex, &TmpChosenEntry);
          if (SubMenuExit == MENU_EXIT_ESCAPE || TmpChosenEntry->getREFIT_MENU_ITEM_RETURN()  ){
            ApplyInputs();
            ModifyTitles(*ChosenEntry);
            break;
          }
          if (SubMenuExit == MENU_EXIT_ENTER || MenuExit == MENU_EXIT_DETAILS) {
            if (TmpChosenEntry->SubScreen != NULL) {
              NextMenuExit = 0;
              while (!NextMenuExit) {
                NextMenuExit = TmpChosenEntry->SubScreen->RunGenericMenu(&NextEntryIndex, &NextChosenEntry);
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
  ThemeX.Proportional = OldFontStyle;

  ApplyInputs();
}

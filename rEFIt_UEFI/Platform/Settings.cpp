/*
 Slice 2012
 */

#include "../entry_scan/entry_scan.h"
#include "../entry_scan/loader.h"
#include "kernel_patcher.h"
#include "ati.h"
#include "../libeg/nanosvg.h"
#include "nvidia.h"
#include "../refit/screen.h"
#include "../refit/menu.h"
#include "gma.h"
#include "../libeg/VectorGraphics.h"
#include "Nvram.h"
#include "BootOptions.h"
#include "StartupSound.h"
#include "Edid.h"
#include "platformdata.h"
#include "smbios.h"
#include "guid.h"
#include "card_vlist.h"
#include "Injectors.h"
#include "cpu.h"
#include "APFS.h"
#include "hda.h"
#include "FixBiosDsdt.h"
#include "../entry_scan/secureboot.h"
#include "../include/Pci.h"
#include "../include/Devices.h"
#include "ati_reg.h"
#include "../../Version.h"
#include "../Platform/Settings.h"

#include <Protocol/OcQuirksProtocol.h>

#ifndef DEBUG_ALL
#define DEBUG_SET 1
#else
#define DEBUG_SET DEBUG_ALL
#endif

#if DEBUG_SET == 0
#define DBG(...)
#else
#define DBG(...) DebugLog (DEBUG_SET, __VA_ARGS__)
#endif

//#define DUMP_KERNEL_KEXT_PATCHES 1

//#define SHORT_LOCATE 1

//#define kXMLTagArray      "array"

//EFI_GUID gRandomUUID = {0x0A0B0C0D, 0x0000, 0x1010, {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}};

#define NUM_OF_CONFIGS 3
#define GEN_PMCON_1                 0xA0


INTN OldChosenTheme;
INTN OldChosenConfig;
INTN OldChosenDsdt;
UINTN OldChosenAudio;
BOOLEAN                        SavePreBootLog;
UINT8                            DefaultAudioVolume;
INTN LayoutBannerOffset = 64;
INTN LayoutTextOffset = 0;
INTN LayoutButtonOffset = 0;

ACPI_PATCHED_AML                *ACPIPatchedAML = NULL;
SIDELOAD_KEXT                   *InjectKextList = NULL;
//SYSVARIABLES                    *SysVariables;
CHAR16                          *IconFormat = NULL;

TagDict*                          gConfigDict[NUM_OF_CONFIGS] = {NULL, NULL, NULL};

SETTINGS_DATA                   gSettings;
LANGUAGES                       gLanguage;
GFX_PROPERTIES                  gGraphics[4]; //no more then 4 graphics cards
HDA_PROPERTIES                  gAudios[4]; //no more then 4 Audio Controllers
//SLOT_DEVICE                     Arpt;
SLOT_DEVICE                     SlotDevices[16]; //assume DEV_XXX, Arpt=6
EFI_EDID_DISCOVERED_PROTOCOL    *EdidDiscovered;
//UINT8                           *gEDID = NULL;
//EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;
//UINT16                          gCPUtype;
UINTN                           NGFX                        = 0; // number of GFX
UINTN                           NHDA                        = 0; // number of HDA Devices


UINTN                           nLanCards;        // number of LAN cards
UINT16                          gLanVendor[4];    // their vendors
UINT8                           *gLanMmio[4];     // their MMIO regions
UINT8                           gLanMac[4][6];    // their MAC addresses
UINTN                           nLanPaths;        // number of LAN pathes

UINTN                           ThemesNum                   = 0;
CONST CHAR16                          *ThemesList[100]; //no more then 100 themes?
UINTN                           ConfigsNum;
CHAR16                          *ConfigsList[20];
UINTN                           DsdtsNum = 0;
CHAR16                          *DsdtsList[20];
UINTN                           AudioNum;
HDA_OUTPUTS                     AudioList[20];
UINTN                           RtVariablesNum;
RT_VARIABLES                    *RtVariables;

// firmware
BOOLEAN                         gFirmwareClover             = FALSE;
UINTN                           gEvent;
UINT16                          gBacklightLevel;
//BOOLEAN                         defDSM;
//UINT16                          dropDSM;

BOOLEAN                         GetLegacyLanAddress;
BOOLEAN                         ResumeFromCoreStorage;
BOOLEAN                         gRemapSmBiosIsRequire;

// QPI
BOOLEAN                         SetTable132                 = FALSE;

//EG_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff }; //define in lib.h
const INTN BCSMargin = 11;

//
DRIVERS_FLAGS gDriversFlags;  //the initializer is not needed for global variables

#ifdef FIRMWARE_REVISION
CONST CHAR16 *gFirmwareRevision = FIRMWARE_REVISION;
CONST CHAR8* gRevisionStr = REVISION_STR;
CONST CHAR8* gFirmwareBuildDate = FIRMWARE_BUILDDATE;
CONST CHAR8* gBuildInfo = BUILDINFOS_STR;
#else
CONST CHAR16 *gFirmwareRevision = "unknown";
CONST CHAR8* gRevisionStr = "unknown";
CONST CHAR8* gFirmwareBuildDate = "unknown";
CONST CHAR8* gBuildInfo = NULL;
#endif

EFI_GUID            gUuid;

EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl = NULL;

extern BOOLEAN                NeedPMfix;
OC_ABC_SETTINGS               gQuirks;
BOOLEAN                       gProvideConsoleGopEnable;

//extern INTN                     OldChosenAudio;

// global configuration with default values
REFIT_CONFIG   GlobalConfig;

static struct FIX_CONFIG { const CHAR8* oldName; const CHAR8* newName; UINT32 bitData; } FixesConfig[] =
{
  { "AddDTGP_0001", "AddDTGP", FIX_DTGP },
  { "FixDarwin_0002", "FixDarwin", FIX_WARNING },
  { "FixShutdown_0004", "FixShutdown", FIX_SHUTDOWN },
  { "AddMCHC_0008", "AddMCHC", FIX_MCHC },
  { "FixHPET_0010", "FixHPET", FIX_HPET },
  { "FakeLPC_0020", "FakeLPC", FIX_LPC },
  { "FixIPIC_0040", "FixIPIC", FIX_IPIC },
  { "FixSBUS_0080", "FixSBUS", FIX_SBUS },
  { "FixDisplay_0100", "FixDisplay", FIX_DISPLAY },
  { "FixIDE_0200", "FixIDE", FIX_IDE },
  { "FixSATA_0400", "FixSATA", FIX_SATA },
  { "FixFirewire_0800", "FixFirewire", FIX_FIREWIRE },
  { "FixUSB_1000", "FixUSB", FIX_USB },
  { "FixLAN_2000", "FixLAN", FIX_LAN },
  { "FixAirport_4000", "FixAirport", FIX_WIFI },
  { "FixHDA_8000", "FixHDA", FIX_HDA },
  { "FixDarwin7_10000", "FixDarwin7", FIX_DARWIN },
  { "FIX_RTC_20000", "FixRTC", FIX_RTC },
  { "FIX_TMR_40000", "FixTMR", FIX_TMR },
  { "AddIMEI_80000", "AddIMEI", FIX_IMEI },
  { "FIX_INTELGFX_100000", "FixIntelGfx", FIX_INTELGFX },
  { "FIX_WAK_200000", "FixWAK", FIX_WAK },
  { "DeleteUnused_400000", "DeleteUnused", FIX_UNUSED },
  { "FIX_ADP1_800000", "FixADP1", FIX_ADP1 },
  { "AddPNLF_1000000", "AddPNLF", FIX_PNLF },
  { "FIX_S3D_2000000", "FixS3D", FIX_S3D },
  { "FIX_ACST_4000000", "FixACST", FIX_ACST },
  { "AddHDMI_8000000", "AddHDMI", FIX_HDMI },
  { "FixRegions_10000000", "FixRegions", FIX_REGIONS },
  { "FixHeaders_20000000", "FixHeaders", FIX_HEADERS },
  { NULL, "FixMutex", FIX_MUTEX }
};

UINT32
GetCrc32 (
          UINT8 *Buffer,
          UINTN Size
          )
{
  UINT32 x = 0;
  UINT32 *Fake = (UINT32*)Buffer;
  if (!Fake) return 0;
  Size >>= 2;
  while (Size--) x+= *Fake++;
  return x;
}

ACPI_NAME_LIST *
ParseACPIName(const XString8& String)
{
  ACPI_NAME_LIST* List = NULL;
  ACPI_NAME_LIST* Next = NULL;
  INTN i, j, Len, pos0, pos1;
  Len = String.length();
  //  DBG("parse ACPI name: %s\n", String);
  if (Len > 0)   {
    //Parse forward but put in stack LIFO "_SB.PCI0.RP02.PXSX"  -1,3,8,13,18
    pos0 = -1;
    while (pos0 < Len) {
      List = (__typeof__(List))AllocateZeroPool(sizeof(ACPI_NAME_LIST));
      List->Next = Next;
      List->Name = (__typeof__(List->Name))AllocateZeroPool(5);
      pos1 = pos0 + 1;
      while ((pos1 < Len) && String[pos1] != '.') pos1++; // 3,8,13,18
      //    if ((pos1 == Len) || (String[pos1] == ',')) { //always
      for (i = pos0 + 1, j = 0; i < pos1; i++) {
        List->Name[j++] = String[i];
      }
      // extend by '_' up to 4 symbols
      if (j < 4) {
        SetMem(List->Name + j, 4 - j, '_');
      }
      List->Name[4] = '\0';
      //    }
      //      DBG("string between [%d,%d]: %s\n", pos0, pos1, List->Name);
      pos0 = pos1; //comma or zero@end
      Next = List;
    }
  }
  return List;
}

VOID
ParseLoadOptions (
                  OUT  XStringW* ConfNamePtr,
                  OUT  TagDict** Dict
                  )
{
  CHAR8 *End;
  CHAR8 *Start;
  UINTN TailSize;
  UINTN i;
  CONST CHAR8 *PlistStrings[]  =
  {
    "<?xml",
    "<!DOCTYPE plist",
    "<plist",
    "<dict>",
    "\0"
  };

  UINTN PlistStringsLen;
  CHAR8 *AsciiConf;

  AsciiConf              = NULL;
  *Dict                  = NULL;

  XStringW& ConfName = *ConfNamePtr;

  Start = (CHAR8*)SelfLoadedImage->LoadOptions;
  End   = (CHAR8*)((CHAR8*)SelfLoadedImage->LoadOptions + SelfLoadedImage->LoadOptionsSize);
  while ((Start < End) && ((*Start == ' ') || (*Start == '\\') || (*Start == '/')))
  {
    ++Start;
  }

  TailSize = End - Start;
  //DBG("TailSize = %d\n", TailSize);

  if ((TailSize) <= 0) {
    return;
  }

  for (i = 0; PlistStrings[i][0] != '\0'; i++) {
    PlistStringsLen = AsciiStrLen(PlistStrings[i]);
    //DBG("PlistStrings[%d] = %s\n", i, PlistStrings[i]);
    if (PlistStringsLen < TailSize) {
      if (AsciiStriNCmp(PlistStrings[i], Start, PlistStringsLen)) {
        DBG(" - found plist string = %s, parse XML in LoadOptions\n", PlistStrings[i]);
        if (ParseXML(Start, Dict, TailSize) != EFI_SUCCESS) {
          *Dict = NULL;
          DBG("  - [!] xml in load options is bad\n");
          return;
        }
        return;
      }
    }
  }

  while ((End > Start) && ((*End == ' ') || (*End == '\\') || (*End == '/'))) {
    --End;
  }

  TailSize = End - Start;
  //  DBG("TailSize2 = %d\n", TailSize);

  if (TailSize > 6) {
    if (AsciiStriNCmp(".plist", End - 6, 6)) {
      End      -= 6;
      TailSize -= 6;
      //      DBG("TailSize3 = %d\n", TailSize);
    }
  } else if (TailSize <= 0) {
    return;
  }

  ConfName.strncpy(Start, TailSize + 1);
}

//
// analyze SelfLoadedImage->LoadOptions to extract Default Volume and Default Loader
// input and output data are global
//
VOID
GetBootFromOption(VOID)
{
  UINT8  *Data = (UINT8*)SelfLoadedImage->LoadOptions;
  UINTN  Len = SelfLoadedImage->LoadOptionsSize;
  UINTN  NameSize, Name2Size;

  Data += 4; //skip signature as we already here
  NameSize = *(UINT16*)Data;

  Data += 2; // pointer to Volume name
  gSettings.DefaultVolume.strncpy((__typeof__(gSettings.DefaultVolume.wc_str()))Data, NameSize);

  Data += NameSize;
  Name2Size = Len - NameSize;
  if (Name2Size != 0) {
    gSettings.DefaultLoader.strncpy((__typeof__(gSettings.DefaultVolume.wc_str()))Data, NameSize);
  }

  DBG("Clover started with option to boot %ls from %ls\n",
      gSettings.DefaultLoader.notEmpty() ? gSettings.DefaultLoader.wc_str() : L"legacy",
      gSettings.DefaultVolume.wc_str());
}

//
// check if this entry corresponds to Boot# variable and then set BootCurrent
//
VOID
SetBootCurrent(REFIT_MENU_ITEM_BOOTNUM *Entry)
{
  EFI_STATUS      Status;
  BO_BOOT_OPTION  BootOption;
  XStringW        VarName;
  UINTN           VarSize = 0;
  UINT8           *BootVariable;
  UINTN           NameSize;
  UINT8           *Data;
  UINT16          *BootOrder;
  UINT16          *BootOrderNew;
  UINT16          *Ptr;
  UINTN           BootOrderSize;
  INTN            BootIndex = 0, Index;


  VarName = SWPrintf("Boot%04llX", Entry->BootNum);
  BootVariable = (UINT8*)GetNvramVariable(VarName.wc_str(), &gEfiGlobalVariableGuid, NULL, &VarSize);
  if ((BootVariable == NULL) || (VarSize == 0)) {
    DBG("Boot option %ls not found\n", VarName.wc_str());
    return;
  }

  //decode the variable
  BootOption.Variable = BootVariable;
  BootOption.VariableSize = VarSize;
  ParseBootOption (&BootOption);

  if ((BootOption.OptionalDataSize == 0) ||
      (BootOption.OptionalData == NULL) ||
      (*(UINT32*)BootOption.OptionalData != CLOVER_SIGN)) {
    DBG("BootVariable of the entry is empty\n");
    FreePool(BootVariable);
    return;
  }

  Data = BootOption.OptionalData + 4;
  NameSize = *(UINT16*)Data;
  Data += 2;
  if (StriCmp((CHAR16*)Data, Entry->Volume->VolName.wc_str()) != 0) {
	  DBG("Boot option %llu has other volume name %ls\n", Entry->BootNum, (CHAR16*)Data);
    FreePool(BootVariable);
    return;
  }

  if (VarSize > NameSize + 6) {
    Data += NameSize;
    if (StriCmp((CHAR16*)Data, Basename(Entry->LoaderPath.wc_str())) != 0) {
		DBG("Boot option %llu has other loader name %ls\n", Entry->BootNum, (CHAR16*)Data);
      FreePool(BootVariable);
      return;
    }
  }

  FreePool(BootVariable);
  //all check passed, save the number
  Status = SetNvramVariable (L"BootCurrent",
                             &gEfiGlobalVariableGuid,
                             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                             sizeof(UINT16),
                             &Entry->BootNum);
  if (EFI_ERROR(Status)) {
    DBG("Can't save BootCurrent, status=%s\n", efiStrError(Status));
  }
  //Next step is rotate BootOrder to set BootNum to first place
  BootOrder = (__typeof__(BootOrder))GetNvramVariable(L"BootOrder", &gEfiGlobalVariableGuid, NULL, &BootOrderSize);
  if (BootOrder == NULL) {
    return;
  }
  VarSize = (INTN)BootOrderSize / sizeof(UINT16); //reuse variable
  for (Index = 0; Index < (INTN)VarSize; Index++) {
    if (BootOrder[Index] == Entry->BootNum) {
      BootIndex = Index;
      break;
    }
  }
  if (BootIndex != 0) {
    BootOrderNew = (__typeof__(BootOrderNew))AllocatePool(BootOrderSize);
    Ptr = BootOrderNew;
    for (Index = 0; Index < (INTN)VarSize - BootIndex; Index++) {
      *Ptr++ = BootOrder[Index + BootIndex];
    }
    for (Index = 0; Index < BootIndex; Index++) {
      *Ptr++ = BootOrder[Index];
    }
    Status = gRT->SetVariable (L"BootOrder",
                               &gEfiGlobalVariableGuid,
                               EFI_VARIABLE_NON_VOLATILE
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS
                               | EFI_VARIABLE_RUNTIME_ACCESS,
                               BootOrderSize,
                               BootOrderNew
                               );
    if (EFI_ERROR(Status)) {
      DBG("Can't save BootOrder, status=%s\n", efiStrError(Status));
    }
    DBG("Set new BootOrder\n");
    PrintBootOrder(BootOrderNew, VarSize);
    FreePool(BootOrderNew);
  }
  FreePool(BootOrder);

}

//
// returns binary setting in a new allocated buffer and data length in dataLen.
// data can be specified in <data></data> base64 encoded
// or in <string></string> hex encoded
//
UINT8
*GetDataSetting (
                 IN      const TagDict* Dict,
                 IN      CONST CHAR8  *PropName,
                 OUT     UINTN  *DataLen
                 )
{
  const TagStruct* Prop;
  UINT8  *Data = NULL;

  Prop = Dict->propertyForKey(PropName);
  if (Prop != NULL) {
    if (Prop->isData() /*&& Prop->dataLen > 0*/) { //rehabman: allow zero length data
      // data property
      Data = (__typeof__(Data))AllocateZeroPool(Prop->getData()->dataLenValue());
      CopyMem(Data, Prop->getData()->dataValue(), Prop->getData()->dataLenValue());

      if (DataLen != NULL) *DataLen = Prop->getData()->dataLenValue();
      /*
       DBG("Data: %p, Len: %d = ", Data, Prop->dataLen);
       for (i = 0; i < Prop->dataLen; i++) {
       DBG("%02hhX ", Data[i]);
       }
       DBG("\n");
       */
    } else if ( Prop->isString() ) {
      // assume data in hex encoded string property
      UINT32 Len = (UINT32)Prop->getString()->stringValue().length() >> 1; // number of hex digits
      Data = (__typeof__(Data))AllocateZeroPool(Len); // 2 chars per byte, one more byte for odd number
      Len  = hex2bin(Prop->getString()->stringValue().c_str(), Data, Len);

      if (DataLen != NULL) *DataLen = Len;
      /*
       DBG("Data(str): %p, Len: %d = ", data, len);
       for (i = 0; i < Len; i++) {
       DBG("%02hhX ", data[i]);
       }
       DBG("\n");
       */
    } else {
      MsgLog("ATTENTION : PropName '%s' is not data or string. Ignored", PropName);
      if (DataLen != NULL) *DataLen = 0;
    }
  }else{
    if (DataLen != NULL) *DataLen = 0;
  }
  return Data;
}

EFI_STATUS
LoadUserSettings (
                  IN EFI_FILE *RootDir,
                  IN const XStringW& ConfName,
                  TagDict** Dict)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  UINTN      Size = 0;
  CHAR8*     ConfigPtr = NULL;
  XStringW   ConfigPlistPath;
  XStringW   ConfigOemPath;

  //  DbgHeader("LoadUserSettings");

  // load config
  if ( ConfName.isEmpty() || Dict == NULL ) {
    return EFI_NOT_FOUND;
  }

  ConfigPlistPath = SWPrintf("EFI\\CLOVER\\%ls.plist", ConfName.wc_str());
  ConfigOemPath   = SWPrintf("%ls\\%ls.plist", OEMPath.wc_str(), ConfName.wc_str());
  if (FileExists (SelfRootDir, ConfigOemPath)) {
    Status = egLoadFile(SelfRootDir, ConfigOemPath.wc_str(), (UINT8**)&ConfigPtr, &Size);
  }
  if (EFI_ERROR(Status)) {
    if ((RootDir != NULL) && FileExists (RootDir, ConfigPlistPath)) {
      Status = egLoadFile(RootDir, ConfigPlistPath.wc_str(), (UINT8**)&ConfigPtr, &Size);
    }
    if (!EFI_ERROR(Status)) {
      DBG("Using %ls.plist at RootDir at path: %ls\n", ConfName.wc_str(), ConfigPlistPath.wc_str());
    } else {
      Status = egLoadFile(SelfRootDir, ConfigPlistPath.wc_str(), (UINT8**)&ConfigPtr, &Size);
      if (!EFI_ERROR(Status)) {
        DBG("Using %ls.plist at SelfRootDir at path: %ls\n", ConfName.wc_str(), ConfigPlistPath.wc_str());
      }else{
        DBG("Cannot find %ls.plist at path: '%ls' or '%ls'\n", ConfName.wc_str(), ConfigPlistPath.wc_str(), ConfigOemPath.wc_str());
      }
    }
  }else{
    DBG("Using %ls.plist at SelfRootDir at path: %ls\n", ConfName.wc_str(), ConfigOemPath.wc_str());
  }

  if (!EFI_ERROR(Status) && ConfigPtr != NULL) {
    Status = ParseXML((const CHAR8*)ConfigPtr, Dict, Size);
    if (EFI_ERROR(Status)) {
      //  Dict = NULL;
      DBG("config.plist parse error Status=%s\n", efiStrError(Status));
      return Status;
    }
  }
  // free configPtr ?
  return Status;
}

STATIC BOOLEAN AddCustomLoaderEntry(IN CUSTOM_LOADER_ENTRY *Entry)
{
  if (Entry == NULL) {
    return FALSE;
  }
  if (gSettings.CustomEntries) {
    CUSTOM_LOADER_ENTRY *Entries = gSettings.CustomEntries;

    while (Entries->Next != NULL) {
      Entries = Entries->Next;
    }
    Entries->Next = Entry;

  } else {
    gSettings.CustomEntries = Entry;
  }

  return TRUE;
}

STATIC BOOLEAN AddCustomLegacyEntry (IN CUSTOM_LEGACY_ENTRY *Entry)
{
  if (Entry == NULL) {
    return FALSE;
  }

  if (gSettings.CustomLegacy != NULL) {
    CUSTOM_LEGACY_ENTRY *Entries = gSettings.CustomLegacy;

    while (Entries->Next != NULL) {
      Entries = Entries->Next;
    }

    Entries->Next = Entry;
  } else {
    gSettings.CustomLegacy = Entry;
  }
  return TRUE;
}
STATIC
BOOLEAN
AddCustomToolEntry (
                    IN CUSTOM_TOOL_ENTRY *Entry
                    )
{
  if (Entry == NULL) {
    return FALSE;
  }

  if (gSettings.CustomTool) {
    CUSTOM_TOOL_ENTRY *Entries = gSettings.CustomTool;

    while (Entries->Next != NULL) {
      Entries = Entries->Next;
    }

    Entries->Next = Entry;
  } else {
    gSettings.CustomTool = Entry;
  }
  return TRUE;
}

STATIC
BOOLEAN
AddCustomSubEntry (
                   IN OUT  CUSTOM_LOADER_ENTRY *Entry,
                   IN      CUSTOM_LOADER_ENTRY *SubEntry)
{
  if ((Entry == NULL) || (SubEntry == NULL)) {
    return FALSE;
  }

  if (Entry->SubEntries != NULL) {
    CUSTOM_LOADER_ENTRY *Entries = Entry->SubEntries;

    while (Entries->Next != NULL) {
      Entries                    = Entries->Next;
    }

    Entries->Next                = SubEntry;
  } else {
    Entry->SubEntries            = SubEntry;
  }

  return TRUE;
}

//BOOLEAN
//CopyKernelAndKextPatches (IN OUT  KERNEL_AND_KEXT_PATCHES *Dst,
//                          IN      CONST KERNEL_AND_KEXT_PATCHES *Src)
//{
//  if (Dst == NULL || Src == NULL) return FALSE;
//
//  Dst->KPDebug           = Src->KPDebug;
////  Dst->KPKernelCpu       = Src->KPKernelCpu;
//  Dst->KPKernelLapic     = Src->KPKernelLapic;
//  Dst->KPKernelXCPM      = Src->KPKernelXCPM;
//  Dst->KPKernelPm        = Src->KPKernelPm;
//  Dst->KPAppleIntelCPUPM = Src->KPAppleIntelCPUPM;
//  Dst->KPAppleRTC        = Src->KPAppleRTC;
//  Dst->EightApple        = Src->EightApple;
//  Dst->KPDELLSMBIOS      = Src->KPDELLSMBIOS;
//  Dst->FakeCPUID         = Src->FakeCPUID;
//  Dst->KPPanicNoKextDump = Src->KPPanicNoKextDump;
//
//  if (Src->KPATIConnectorsController != NULL) {
//    Dst->KPATIConnectorsController = (__typeof__(Dst->KPATIConnectorsController))AllocateCopyPool(AsciiStrSize(Src->KPATIConnectorsController) ,Src->KPATIConnectorsController);
//  }
//
//  if ((Src->KPATIConnectorsDataLen > 0) &&
//      (Src->KPATIConnectorsData != NULL) &&
//      (Src->KPATIConnectorsPatch != NULL)) {
//    Dst->KPATIConnectorsDataLen = Src->KPATIConnectorsDataLen;
//    Dst->KPATIConnectorsData = (__typeof__(Dst->KPATIConnectorsData))AllocateCopyPool(Src->KPATIConnectorsDataLen, Src->KPATIConnectorsData);
//    Dst->KPATIConnectorsPatch = (__typeof__(Dst->KPATIConnectorsPatch))AllocateCopyPool(Src->KPATIConnectorsDataLen, Src->KPATIConnectorsPatch);
//  }
//
//  if ((Src->ForceKexts.size() > 0) && (Src->ForceKexts != NULL)) {
//    INTN i;
//    Dst->ForceKexts = (__typeof__(Dst->ForceKexts))AllocatePool (Src->ForceKexts.size() * sizeof(CHAR16 *));
//    Dst->ForceKexts.size() = 0;
//
//    for (i = 0; i < Src->ForceKexts.size(); i++) {
//      Dst->ForceKexts[Dst->ForceKexts.size()++] = EfiStrDuplicate (Src->ForceKexts[i]);
//    }
//  }
//
//  if ((Src->NrKexts > 0) && (Src->KextPatches != NULL)) {
//    INTN i;
//    Dst->KextPatches = (__typeof__(Dst->KextPatches))AllocatePool (Src->NrKexts * sizeof(KEXT_PATCH));
//    Dst->NrKexts = 0;
//    for (i = 0; i < Src->NrKexts; i++)
//    {
//      if ((Src->KextPatches[i].DataLen <= 0) ||
//          (Src->KextPatches[i].Data == NULL) ||
//          (Src->KextPatches[i].Patch == NULL)) {
//        continue;
//      }
//
//      if (Src->KextPatches[i].Name) {
//        Dst->KextPatches[Dst->NrKexts].Name       = (CHAR8 *)AllocateCopyPool(AsciiStrSize (Src->KextPatches[i].Name), Src->KextPatches[i].Name);
//      }
//
//      if (Src->KextPatches[i].Label) {
//        Dst->KextPatches[Dst->NrKexts].Label      = (CHAR8 *)AllocateCopyPool(AsciiStrSize (Src->KextPatches[i].Label), Src->KextPatches[i].Label);
//      }
//
//      Dst->KextPatches[Dst->NrKexts].MenuItem.BValue     = Src->KextPatches[i].MenuItem.BValue;
//      Dst->KextPatches[Dst->NrKexts].IsPlistPatch = Src->KextPatches[i].IsPlistPatch;
//      Dst->KextPatches[Dst->NrKexts].DataLen      = Src->KextPatches[i].DataLen;
//      Dst->KextPatches[Dst->NrKexts].Data = (__typeof__(Dst->KextPatches[Dst->NrKexts].Data))AllocateCopyPool(Src->KextPatches[i].DataLen, Src->KextPatches[i].Data);
//      Dst->KextPatches[Dst->NrKexts].Patch = (__typeof__(Dst->KextPatches[Dst->NrKexts].Patch))AllocateCopyPool(Src->KextPatches[i].DataLen, Src->KextPatches[i].Patch);
//      Dst->KextPatches[Dst->NrKexts].MatchOS = (__typeof__(Dst->KextPatches[Dst->NrKexts].MatchOS))AllocateCopyPool(AsciiStrSize(Src->KextPatches[i].MatchOS), Src->KextPatches[i].MatchOS);
//      Dst->KextPatches[Dst->NrKexts].MatchBuild = (__typeof__(Dst->KextPatches[Dst->NrKexts].MatchBuild))AllocateCopyPool(AsciiStrSize(Src->KextPatches[i].MatchBuild), Src->KextPatches[i].MatchBuild);
//      if (Src->KextPatches[i].MaskFind != NULL) {
//        Dst->KextPatches[Dst->NrKexts].MaskFind = (__typeof__(Dst->KextPatches[Dst->NrKexts].MaskFind))AllocateCopyPool(Src->KextPatches[i].DataLen, Src->KextPatches[i].MaskFind);
//      } else {
//        Dst->KextPatches[Dst->NrKexts].MaskFind = NULL;
//      }
//      if (Src->KextPatches[i].MaskReplace != NULL) {
//        Dst->KextPatches[Dst->NrKexts].MaskReplace = (__typeof__(Dst->KextPatches[Dst->NrKexts].MaskReplace))AllocateCopyPool(Src->KextPatches[i].DataLen, Src->KextPatches[i].MaskReplace);
//      } else {
//        Dst->KextPatches[Dst->NrKexts].MaskReplace = NULL;
//      }
//      if (Src->KextPatches[i].StartPattern != NULL) {
//        Dst->KextPatches[Dst->NrKexts].StartPattern = (__typeof__(Dst->KextPatches[Dst->NrKexts].StartPattern))AllocateCopyPool(Src->KextPatches[i].StartPatternLen, Src->KextPatches[i].StartPattern);
//        Dst->KextPatches[Dst->NrKexts].StartMask = (__typeof__(Dst->KextPatches[Dst->NrKexts].StartMask))AllocateCopyPool(Src->KextPatches[i].StartPatternLen, Src->KextPatches[i].StartMask);
//      } else {
//        Dst->KextPatches[Dst->NrKexts].StartPattern = NULL;
//        Dst->KextPatches[Dst->NrKexts].StartMask = NULL;
//      }
//      //ProcedureName
//      if (Src->KextPatches[i].ProcedureName != NULL) {
//        INTN len = strlen(Src->KextPatches[i].ProcedureName);
//        Dst->KextPatches[Dst->NrKexts].ProcedureName = (__typeof__(Dst->KextPatches[Dst->NrKexts].ProcedureName))AllocateCopyPool(len+1, Src->KextPatches[i].ProcedureName);
//      } else {
//        Dst->KextPatches[Dst->NrKexts].ProcedureName = NULL;
//      }
//      Dst->KextPatches[Dst->NrKexts].StartPatternLen = Src->KextPatches[i].StartPatternLen;
//      Dst->KextPatches[Dst->NrKexts].SearchLen = Src->KextPatches[i].SearchLen;
//      ++(Dst->NrKexts);
//    }
//  }
//
//  if ((Src->NrKernels > 0) && (Src->KernelPatches != NULL)) {
//    INTN i;
//    Dst->KernelPatches = (__typeof__(Dst->KernelPatches))AllocatePool (Src->NrKernels * sizeof(KEXT_PATCH));
//    Dst->NrKernels = 0;
//
//    for (i = 0; i < Src->NrKernels; i++)
//    {
//      if ((Src->KernelPatches[i].DataLen <= 0) ||
//          (Src->KernelPatches[i].Data == NULL) ||
//          (Src->KernelPatches[i].Patch == NULL)) {
//        continue;
//      }
//
//      if (Src->KernelPatches[i].Label) {
//        Dst->KernelPatches[Dst->NrKernels].Label      = (CHAR8 *)AllocateCopyPool(AsciiStrSize (Src->KernelPatches[i].Label), Src->KernelPatches[i].Label);
//      }
//
//      Dst->KernelPatches[Dst->NrKernels].MenuItem.BValue     = Src->KernelPatches[i].MenuItem.BValue;
//      Dst->KernelPatches[Dst->NrKernels].DataLen      = Src->KernelPatches[i].DataLen;
//      Dst->KernelPatches[Dst->NrKernels].Data = (__typeof__(Dst->KernelPatches[Dst->NrKernels].Data))AllocateCopyPool(Src->KernelPatches[i].DataLen, Src->KernelPatches[i].Data);
//      Dst->KernelPatches[Dst->NrKernels].Patch = (__typeof__(Dst->KernelPatches[Dst->NrKernels].Patch))AllocateCopyPool(Src->KernelPatches[i].DataLen, Src->KernelPatches[i].Patch);
//      Dst->KernelPatches[Dst->NrKernels].Count        = Src->KernelPatches[i].Count;
//      Dst->KernelPatches[Dst->NrKernels].MatchOS = (__typeof__(Dst->KernelPatches[Dst->NrKernels].MatchOS))AllocateCopyPool(AsciiStrSize(Src->KernelPatches[i].MatchOS), Src->KernelPatches[i].MatchOS);
//      Dst->KernelPatches[Dst->NrKernels].MatchBuild = (__typeof__(Dst->KernelPatches[Dst->NrKernels].MatchBuild))AllocateCopyPool(AsciiStrSize(Src->KernelPatches[i].MatchBuild), Src->KernelPatches[i].MatchBuild);
//      if (Src->KernelPatches[i].MaskFind != NULL) {
//        Dst->KernelPatches[Dst->NrKernels].MaskFind = (__typeof__(Dst->KernelPatches[Dst->NrKernels].MaskFind))AllocateCopyPool(Src->KernelPatches[i].DataLen, Src->KernelPatches[i].MaskFind);
//      } else {
//        Dst->KernelPatches[Dst->NrKernels].MaskFind = NULL;
//      }
//      if (Src->KernelPatches[i].MaskReplace != NULL) {
//        Dst->KernelPatches[Dst->NrKernels].MaskReplace = (__typeof__(Dst->KernelPatches[Dst->NrKernels].MaskReplace))AllocateCopyPool(Src->KernelPatches[i].DataLen, Src->KernelPatches[i].MaskReplace);
//      } else {
//        Dst->KernelPatches[Dst->NrKernels].MaskReplace = NULL;
//      }
//      if (Src->KernelPatches[i].StartPattern != NULL) {
//        Dst->KernelPatches[Dst->NrKernels].StartPattern = (__typeof__(Dst->KernelPatches[Dst->NrKernels].StartPattern))AllocateCopyPool(Src->KernelPatches[i].StartPatternLen, Src->KernelPatches[i].StartPattern);
//        Dst->KernelPatches[Dst->NrKernels].StartMask = (__typeof__(Dst->KernelPatches[Dst->NrKernels].StartMask))AllocateCopyPool(Src->KernelPatches[i].StartPatternLen, Src->KernelPatches[i].StartMask);
//      } else {
//        Dst->KernelPatches[Dst->NrKernels].StartPattern = NULL;
//        Dst->KernelPatches[Dst->NrKernels].StartMask    = NULL;
//      }
//      Dst->KernelPatches[Dst->NrKernels].StartPatternLen = Src->KernelPatches[i].StartPatternLen;
//      Dst->KernelPatches[Dst->NrKernels].SearchLen = Src->KernelPatches[i].SearchLen;
//      if (Src->KernelPatches[i].ProcedureName != NULL) {
//        INTN len = strlen(Src->KernelPatches[i].ProcedureName);
//        Dst->KernelPatches[Dst->NrKernels].ProcedureName = (__typeof__(Dst->KernelPatches[Dst->NrKernels].ProcedureName))AllocateCopyPool(len+1, Src->KernelPatches[i].ProcedureName);
//      } else {
//        Dst->KernelPatches[Dst->NrKernels].ProcedureName = NULL;
//      }
//
//      ++(Dst->NrKernels);
//    }
//  }
//
//  return TRUE;
//}

STATIC
CUSTOM_LOADER_ENTRY
*DuplicateCustomEntry (
                       IN CUSTOM_LOADER_ENTRY *Entry
                       )
{
  if (Entry == NULL) {
    return NULL;
  }

  CUSTOM_LOADER_ENTRY* DuplicateEntry = new CUSTOM_LOADER_ENTRY;
  if (DuplicateEntry != NULL) {
    DuplicateEntry->Volume           = Entry->Volume;
    DuplicateEntry->Path             = Entry->Path;
    DuplicateEntry->LoadOptions      = Entry->LoadOptions;
    DuplicateEntry->FullTitle        = Entry->FullTitle;
    DuplicateEntry->Title            = Entry->Title;
    DuplicateEntry->ImagePath        = Entry->ImagePath;
    DuplicateEntry->DriveImagePath   = Entry->DriveImagePath;
    DuplicateEntry->BootBgColor      = Entry->BootBgColor;
    DuplicateEntry->Image            = Entry->Image;
    DuplicateEntry->DriveImage       = Entry->DriveImage;
    DuplicateEntry->Hotkey           = Entry->Hotkey;
    DuplicateEntry->Flags            = Entry->Flags;
    DuplicateEntry->Type             = Entry->Type;
    DuplicateEntry->VolumeType       = Entry->VolumeType;
    DuplicateEntry->KernelScan       = Entry->KernelScan;
    DuplicateEntry->CustomBoot       = Entry->CustomBoot;
    DuplicateEntry->CustomLogo       = Entry->CustomLogo;
//    CopyKernelAndKextPatches (&DuplicateEntry->KernelAndKextPatches, &Entry->KernelAndKextPatches);
    DuplicateEntry->KernelAndKextPatches = Entry->KernelAndKextPatches;
  }

  return DuplicateEntry;
}

STATIC
BOOLEAN
FillinKextPatches (IN OUT KERNEL_AND_KEXT_PATCHES *Patches,
                   const TagDict* DictPointer)
{
  const TagStruct* Prop;
  const TagArray* arrayProp;
  // UINTN  i;

  if (Patches == NULL || DictPointer == NULL) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Debug");
  if (Prop != NULL || gBootChanged) {
    Patches->KPDebug = IsPropertyNotNullAndTrue(Prop);
  }
/*
  Prop = GetProperty(DictPointer, "KernelCpu");
  if (Prop != NULL || gBootChanged) {
    Patches->KPKernelCpu = IsPropertyTrue(Prop);
  }
*/
  Prop = DictPointer->propertyForKey("KernelLapic");
  if (Prop != NULL || gBootChanged) {
    Patches->KPKernelLapic = IsPropertyNotNullAndTrue(Prop);
  }

  Prop = DictPointer->propertyForKey("KernelXCPM");
  if (Prop != NULL || gBootChanged) {
    Patches->KPKernelXCPM = IsPropertyNotNullAndTrue(Prop);
    if (IsPropertyNotNullAndTrue(Prop)) {
      DBG("KernelXCPM: enabled\n");
    }
  }

  Prop = DictPointer->propertyForKey("KernelPm");
  if (Prop != NULL || gBootChanged) {
    Patches->KPKernelPm = IsPropertyNotNullAndTrue(Prop);
  }
  
  Prop = DictPointer->propertyForKey("PanicNoKextDump");
  if (Prop != NULL || gBootChanged) {
    Patches->KPPanicNoKextDump = IsPropertyNotNullAndTrue(Prop);
  }

  Prop = DictPointer->propertyForKey("AppleIntelCPUPM");
  if (Prop != NULL || gBootChanged) {
    Patches->KPAppleIntelCPUPM = IsPropertyNotNullAndTrue(Prop);
  }
 //anyway
  if (NeedPMfix) {
    Patches->KPKernelPm = TRUE;
    Patches->KPAppleIntelCPUPM = TRUE;
  }

  Prop = DictPointer->propertyForKey("AppleRTC");
  if (Prop != NULL || gBootChanged) {
    Patches->KPAppleRTC = !IsPropertyNotNullAndFalse(Prop);  //default = TRUE
  }

  Prop = DictPointer->propertyForKey("EightApple");
  if (Prop != NULL || gBootChanged) {
    Patches->EightApple = IsPropertyNotNullAndTrue(Prop);
  }

  //
  // Dell SMBIOS Patch
  //
  // syscl: we do not need gBootChanged and Prop is empty condition
  // this change will boost Dell SMBIOS Patch a bit
  // but the major target is to make code clean
  Prop = DictPointer->propertyForKey("DellSMBIOSPatch");
  Patches->KPDELLSMBIOS = IsPropertyNotNullAndTrue(Prop); // default == FALSE
  gRemapSmBiosIsRequire = Patches->KPDELLSMBIOS;

  Prop = DictPointer->propertyForKey("FakeCPUID");
  if (Prop != NULL || gBootChanged) {
    Patches->FakeCPUID = (UINT32)GetPropertyAsInteger(Prop, 0);
    DBG("FakeCPUID: %X\n", Patches->FakeCPUID);
  }

  Prop = DictPointer->propertyForKey("ATIConnectorsController");
  if ( Prop != NULL && Prop->isString() ) {
    UINTN len = 0, i=0;

    // ATIConnectors patch
    Patches->KPATIConnectorsController = Prop->getString()->stringValue();

    UINT8* p = GetDataSetting (DictPointer, "ATIConnectorsData", &len);
    Patches->KPATIConnectorsData.stealValueFrom(p, len);
    p = GetDataSetting (DictPointer, "ATIConnectorsPatch", &i);
    Patches->KPATIConnectorsPatch.stealValueFrom(p, i);

    if (Patches->KPATIConnectorsData == NULL
        || Patches->KPATIConnectorsPatch == NULL
        || Patches->KPATIConnectorsData.size() == 0
        || Patches->KPATIConnectorsData.size() != i) {
      // invalid params - no patching
      DBG("ATIConnectors patch: invalid parameters!\n");

      Patches->KPATIConnectorsController.setEmpty();
      Patches->KPATIConnectorsData.setEmpty();
      Patches->KPATIConnectorsPatch.setEmpty();
      Patches->KPATIConnectorsController.setEmpty();
    }
  }

  /*
   * ForceKextsToLoad is an array of string
   */
  arrayProp = DictPointer->arrayPropertyForKey("ForceKextsToLoad");
  if ( arrayProp != NULL ) {
    INTN i;
    INTN Count = arrayProp->arrayContent().size();
    if (Count > 0) {
      const TagStruct* Prop2 = NULL;

      DBG("ForceKextsToLoad: %lld requested\n", Count);

      for (i = 0; i < Count; i++) {
        Prop2 = &arrayProp->arrayContent()[i];
        if ( !Prop2->isString() ) {
          MsgLog("ATTENTION : property not string in ForceKextsToLoad\n");
          continue;
        }

        if (Prop2->getString()->stringValue().notEmpty()) {
          const CHAR8* p = Prop2->getString()->stringValue().c_str();
          if (*p == '\\') {
            ++p;
          }

          if (AsciiStrSize(p) > 1) {
            Patches->ForceKexts.Add(p);
            DBG(" - [%zu]: %ls\n", Patches->ForceKexts.size(), Patches->ForceKexts[Patches->ForceKexts.size()-1].wc_str());
          }
        }
      }
    }
  }

  // KextsToPatch is an array of dict
  arrayProp = DictPointer->arrayPropertyForKey("KextsToPatch");
  if (arrayProp != NULL) {
    INTN i;
    INTN Count = arrayProp->arrayContent().size();
    
    Patches->KextPatches.setEmpty();
    
    if (Count > 0) {
      const TagDict* Prop2 = NULL;
      const TagStruct* Dict = NULL;

      DBG("KextsToPatch: %lld requested\n", Count);
      for (i = 0; i < Count; i++) {
        UINTN FindLen = 0, ReplaceLen = 0, MaskLen = 0;
        Prop2 = arrayProp->dictElementAt(i);
        if ( !Prop2->isDict() ) {
        	MsgLog("ATTENTION : property not dict in KextsToPatch\n");
          continue;
        }
        DBG(" - [%02lld]:", i);

        Dict = Prop2->propertyForKey("Name");
        if (Dict == NULL) {
          DBG(" patch without Name, skipped\n");
          continue;
        }
        if ( !Dict->isString() ) {
        	MsgLog("ATTENTION : Name property not string in KextsToPatch\n");
          continue;
        }

        KEXT_PATCH* newPatchPtr = new KEXT_PATCH();
        KEXT_PATCH& newPatch = *newPatchPtr;
        
        newPatch.Name = Dict->getString()->stringValue();
        newPatch.Label.takeValueFrom(newPatch.Name);

        Dict = Prop2->propertyForKey("Comment");
        if (Dict != NULL) {
          newPatch.Label += " (";
          newPatch.Label += Dict->getString()->stringValue();
          newPatch.Label += ")";

        } else {
          newPatch.Label += " (NoLabel)";
        }
        DBG(" %s", newPatch.Label.c_str());

        newPatch.MenuItem.BValue     = TRUE;
        Dict = Prop2->propertyForKey("Disabled");
        if ((Dict != NULL) && IsPropertyNotNullAndTrue(Dict)) {
          newPatch.MenuItem.BValue     = FALSE;
        }
        
        Dict = Prop2->propertyForKey("RangeFind");
        newPatch.SearchLen = GetPropertyAsInteger(Dict, 0); //default 0 will be calculated later
        
        UINT8* TmpData = GetDataSetting(Prop2, "StartPattern", &FindLen);
        if (TmpData != NULL) {
          newPatch.StartPattern.stealValueFrom(TmpData, FindLen);
        }
        
        TmpData    = GetDataSetting (Prop2, "MaskStart", &ReplaceLen);
        ReplaceLen = MIN(ReplaceLen, FindLen);
        if (FindLen != 0) {
          newPatch.StartMask.memset(0xFF, FindLen);
          if (TmpData != NULL) {
            newPatch.StartMask.ncpy(TmpData, ReplaceLen);
          }
        }
        if (TmpData != NULL) {
          FreePool(TmpData);
        }

        TmpData    = GetDataSetting (Prop2, "Find", &FindLen);
        UINT8* TmpPatch   = GetDataSetting (Prop2, "Replace", &ReplaceLen);

        if (!FindLen || !ReplaceLen) {
          DBG(" - invalid Find/Replace data - skipping!\n");
          continue;
        }
        
        Dict = Prop2->propertyForKey("Procedure");
        if ( Dict != NULL ) {
        	if ( Dict->isString() ) {
	          newPatch.ProcedureName = Dict->getString()->stringValue();
	        }else{
	        	MsgLog("ATTENTION : Procedure property not string in KextsToPatch\n");
	        }
        }


        newPatch.Data.stealValueFrom(TmpData, FindLen);
        
        TmpData    = GetDataSetting (Prop2, "MaskFind", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;

        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newPatch.MaskFind.memset(FindLen, 0xFF);
          newPatch.MaskFind.ncpy(TmpData, MaskLen);
        }
        FreePool(TmpData);
        // take into account a possibility to set ReplaceLen < FindLen. In this case assumes MaskReplace = 0 for the rest of bytes 
        newPatch.Patch.memset(0, FindLen);
        ReplaceLen = MIN(ReplaceLen, FindLen);
        newPatch.Patch.ncpy(TmpPatch, ReplaceLen);
        FreePool(TmpPatch);
        
        MaskLen = 0;
        TmpData    = GetDataSetting (Prop2, "MaskReplace", &MaskLen);
        MaskLen = MIN(ReplaceLen, MaskLen);
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newPatch.MaskReplace.memset(0, FindLen);
          newPatch.MaskReplace.ncpy(TmpData, MaskLen); //other bytes are zeros, means no replace
        }
        FreePool(TmpData);

        // check enable/disabled patch (OS based) by Micky1979
        Dict = Prop2->propertyForKey("MatchOS");
        if ((Dict != NULL) && (Dict->isString())) {
          newPatch.MatchOS = Dict->getString()->stringValue();
          DBG(" :: MatchOS: %s", newPatch.MatchOS.c_str());
        }

        Dict = Prop2->propertyForKey("MatchBuild");
        if ((Dict != NULL) && (Dict->isString())) {
          newPatch.MatchBuild = Dict->getString()->stringValue();
          DBG(" :: MatchBuild: %s", newPatch.MatchBuild.c_str());
        }

        // check if this is Info.plist patch or kext binary patch
        Dict = Prop2->propertyForKey("InfoPlistPatch");
        newPatch.IsPlistPatch = IsPropertyNotNullAndTrue(Dict);

        if (newPatch.IsPlistPatch) {
          DBG(" :: PlistPatch");
        } else {
          DBG(" :: BinPatch");
        }

        DBG(" :: data len: %zu\n", newPatch.Data.size());
        if (!newPatch.MenuItem.BValue) {
          DBG(" - patch disabled at config\n");
        }
        Patches->KextPatches.AddReference(newPatchPtr, true);
      }
    }

    //gSettings.NrKexts = (INT32)i;
    //there is one moment. This data is allocated in BS memory but will be used
    // after OnExitBootServices. This is wrong and these arrays should be reallocated
    // but I am not sure
  }

  /*
   * KernelToPatch is an array of dict
   */
  arrayProp = DictPointer->arrayPropertyForKey("KernelToPatch");
  if (Prop != NULL) {
    INTN   i;
    INTN   Count = arrayProp->arrayContent().size();
    //delete old and create new
    Patches->KernelPatches.setEmpty();
    if (Count > 0) {
      const TagDict* Prop2 = NULL;
      const TagStruct* prop3 = NULL;
      DBG("KernelToPatch: %lld requested\n", Count);
      for (i = 0; i < Count; i++) {
        UINTN FindLen = 0, ReplaceLen = 0, MaskLen = 0;
        UINT8 *TmpData, *TmpPatch;

        Prop2 = arrayProp->dictElementAt(i, "KernelToPatch"_XS8);

        DBG(" - [%02lld]:", i);

        KEXT_PATCH* newKernelPatchPtr = new KEXT_PATCH;
        KEXT_PATCH& newKernelPatch = *newKernelPatchPtr;

        newKernelPatch.Label = "NoLabel"_XS8;
        prop3 = Prop2->propertyForKey("Comment");
        if (prop3 != NULL) {
        	if ( prop3->isString() ) {
	          newKernelPatch.Label = prop3->getString()->stringValue();
	        }else{
	        	MsgLog("ATTENTION : Comment property not string in KernelToPatch\n");
	        }
        }
        DBG(" %s", newKernelPatch.Label.c_str());

        prop3 = Prop2->propertyForKey("Disabled");
        newKernelPatch.MenuItem.BValue   = !IsPropertyNotNullAndTrue(prop3);
        
        prop3 = Prop2->propertyForKey("RangeFind");
        newKernelPatch.SearchLen = GetPropertyAsInteger(prop3, 0); //default 0 will be calculated later
        
        TmpData    = GetDataSetting (Prop2, "StartPattern", &FindLen);
        if (TmpData != NULL) {
          newKernelPatch.StartPattern.stealValueFrom(TmpData, FindLen);
        }
        
        TmpData    = GetDataSetting (Prop2, "MaskStart", &ReplaceLen);
        ReplaceLen = MIN(ReplaceLen, FindLen);
        if (FindLen != 0) {
          newKernelPatch.StartMask.memset(0xFF, FindLen);
          if (TmpData != NULL) {
            newKernelPatch.StartMask.ncpy(TmpData, ReplaceLen);
          }
        }
        if (TmpData != NULL) {
          FreePool(TmpData);
        }


        TmpData    = GetDataSetting (Prop2, "Find", &FindLen);
        TmpPatch   = GetDataSetting (Prop2, "Replace", &ReplaceLen);
//replace len can be smaller if mask using
        if (!FindLen || !ReplaceLen /*|| (FindLen != ReplaceLen)*/) {
          DBG(" :: invalid Find/Replace data - skipping!\n");
          continue;
        }
        
        prop3 = Prop2->propertyForKey("Procedure");
        if (prop3 != NULL) {
        	if ( prop3->isString() ) {
	          newKernelPatch.ProcedureName = prop3->getString()->stringValue();
	        }else{
	        	MsgLog("ATTENTION : Procedure property not string in KernelToPatch\n");
	        }
        }


        newKernelPatch.Data.stealValueFrom(TmpData, FindLen);

        TmpData    = GetDataSetting (Prop2, "MaskFind", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newKernelPatch.MaskFind.memset(0xFF, FindLen);
          newKernelPatch.MaskFind.ncpy(TmpData, MaskLen);
        }
        FreePool(TmpData);
        // this is "Replace" string len of ReplaceLen
        ReplaceLen = MIN(ReplaceLen, FindLen);
        newKernelPatch.Patch.memset(0, FindLen);
        newKernelPatch.Patch.ncpy(TmpPatch, ReplaceLen);
        FreePool(TmpPatch);
        MaskLen = 0;
        TmpData    = GetDataSetting (Prop2, "MaskReplace", &MaskLen); //reuse MaskLen
        MaskLen = MIN(ReplaceLen, MaskLen);
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newKernelPatch.MaskReplace.memset(0, FindLen);
          newKernelPatch.MaskReplace.ncpy(TmpData, MaskLen);
        }
        FreePool(TmpData);
        newKernelPatch.Count        = 0;

        prop3 = Prop2->propertyForKey("Count");
        if (prop3 != NULL) {
          newKernelPatch.Count = GetPropertyAsInteger(prop3, 0);
        }

        // check enable/disabled patch (OS based) by Micky1979
        prop3 = Prop2->propertyForKey("MatchOS");
        if ((prop3 != NULL) && (prop3->isString())) {
          newKernelPatch.MatchOS = prop3->getString()->stringValue();
          DBG(" :: MatchOS: %s", newKernelPatch.MatchOS.c_str());
        }

        prop3 = Prop2->propertyForKey("MatchBuild");
        if ((prop3 != NULL) && (prop3->isString())) {
          newKernelPatch.MatchBuild = prop3->getString()->stringValue();
          DBG(" :: MatchBuild: %s", newKernelPatch.MatchBuild.c_str());
        }
        DBG(" :: data len: %zu\n", newKernelPatch.Data.size());
        Patches->KernelPatches.AddReference(newKernelPatchPtr, true);
      }
    }
  }

  /*
   * BootPatches is an array of dict
   */
  arrayProp = DictPointer->arrayPropertyForKey("BootPatches");
  if (Prop != NULL) {
    INTN   i;
    INTN   Count = arrayProp->arrayContent().size();
    //delete old and create new
    Patches->BootPatches.setEmpty();
    if (Count > 0) {
      const TagDict* Prop2 = NULL;
      const TagStruct* prop3 = NULL;

      DBG("BootPatches: %lld requested\n", Count);
      for (i = 0; i < Count; i++) {
        UINTN FindLen = 0, ReplaceLen = 0, MaskLen = 0;
        UINT8 *TmpData, *TmpPatch;

        Prop2 = arrayProp->dictElementAt(i, "BootPatches"_XS8);

        DBG(" - [%02lld]:", i);

        KEXT_PATCH* newBootPatchPtr = new KEXT_PATCH;
        KEXT_PATCH& newBootPatch = *newBootPatchPtr;

        newBootPatch.Label = "NoLabel"_XS8;
        prop3 = Prop2->propertyForKey("Comment");
        if (prop3 != NULL) {
        	if ( prop3->isString() ) {
	          newBootPatch.Label = prop3->getString()->stringValue();
	        }else{
	        	MsgLog("ATTENTION : Comment property not string in KernelToPatch\n");
	        }
        }

        DBG(" %s", newBootPatch.Label.c_str());

        prop3 = Prop2->propertyForKey("Disabled");
        newBootPatch.MenuItem.BValue   = !IsPropertyNotNullAndTrue(prop3);
        newBootPatch.MenuItem.ItemType = BoolValue;
        
        prop3 = Prop2->propertyForKey("RangeFind");
        newBootPatch.SearchLen = GetPropertyAsInteger(prop3, 0); //default 0 will be calculated later
        
        TmpData    = GetDataSetting (Prop2, "StartPattern", &FindLen);
        if (TmpData != NULL) {
          newBootPatch.StartPattern.stealValueFrom(TmpData, FindLen);
        }
        
        TmpData    = GetDataSetting (Prop2, "MaskStart", &ReplaceLen);
        ReplaceLen = MIN(ReplaceLen, FindLen);
        if (FindLen != 0) {
          newBootPatch.StartMask.memset(0xFF, FindLen);
          if (TmpData != NULL) {
            newBootPatch.StartMask.ncpy(TmpData, ReplaceLen);
          }
        }
        if (TmpData != NULL) {
          FreePool(TmpData);
        }
        

        TmpData    = GetDataSetting (Prop2, "Find", &FindLen);
        TmpPatch   = GetDataSetting (Prop2, "Replace", &ReplaceLen);
        if (!FindLen || !ReplaceLen) {
          DBG(" :: invalid Find/Replace data - skipping!\n");
          continue;
        }
        ReplaceLen = MIN(ReplaceLen, FindLen);
        newBootPatch.Data.stealValueFrom(TmpData, FindLen);

        MaskLen = 0;
        TmpData    = GetDataSetting(Prop2, "MaskFind", &MaskLen);
        MaskLen = MIN(FindLen, MaskLen);
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newBootPatch.MaskFind.memset(0xFF, FindLen);
          newBootPatch.MaskFind.ncpy(TmpData, MaskLen);
        }
        FreePool(TmpData);
        newBootPatch.Patch.memset(0, FindLen);
        newBootPatch.Patch.ncpy(TmpPatch, ReplaceLen);
        FreePool(TmpPatch);
        MaskLen = 0;
        TmpData    = GetDataSetting(Prop2, "MaskReplace", &MaskLen);
        MaskLen = MIN(ReplaceLen, MaskLen);
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newBootPatch.MaskReplace.memset(0, FindLen);
          newBootPatch.MaskReplace.ncpy(TmpData, MaskLen);
        }
        FreePool(TmpData);
        newBootPatch.Count        = 0;

        prop3 = Prop2->propertyForKey("Count");
        if (prop3 != NULL) {
          newBootPatch.Count = GetPropertyAsInteger(prop3, 0);
        }

        prop3 = Prop2->propertyForKey("MatchOS");
        if ((prop3 != NULL) && (prop3->isString())) {
          newBootPatch.MatchOS = prop3->getString()->stringValue();
          DBG(" :: MatchOS: %s", newBootPatch.MatchOS.c_str());
        }

        prop3 = Prop2->propertyForKey("MatchBuild");
        if ((prop3 != NULL) && (prop3->isString())) {
          newBootPatch.MatchBuild = prop3->getString()->stringValue();
          DBG(" :: MatchBuild: %s", newBootPatch.MatchBuild.c_str());
        }

        DBG(" :: data len: %zu\n", newBootPatch.Data.size());
        Patches->BootPatches.AddReference(newBootPatchPtr, true);
      }
    }
  }


  return TRUE;
}

BOOLEAN
IsPatchEnabled (const XString8& MatchOSEntry, const XString8& CurrOS)
{
  BOOLEAN ret = FALSE;

  if (MatchOSEntry.isEmpty() || CurrOS.isEmpty()) {
    return TRUE; //undefined matched corresponds to old behavior
  }

  XString8Array mos = Split<XString8Array>(MatchOSEntry, ","_XS8).trimEachString();
  
  if ( mos[0] == "All"_XS8) {
    return TRUE;
  }

  for (size_t i = 0; i < mos.size(); ++i) {
    // dot represent MatchOS
    if (
        ( mos[i].contains("."_XS8) && IsOSValid(mos[i], CurrOS)) || // MatchOS
        ( mos[i].contains(CurrOS) ) // MatchBuild
        ) {
      //DBG("\nthis patch will activated for OS %ls!\n", mos->array[i]);
      ret =  TRUE;
      break;
    }
  }
  return ret;
}


BOOLEAN IsOSValid(const XString8& MatchOS, const XString8& CurrOS)
{
  /* example for valid matches are:
   10.7, only 10.7 (10.7.1 will be skipped)
   10.10.2 only 10.10.2 (10.10.1 or 10.10.5 will be skipped)
   10.10.x (or 10.10.X), in this case is valid for all minor version of 10.10 (10.10.(0-9))
   */

  BOOLEAN ret = FALSE;

  if (MatchOS.isEmpty() || CurrOS.isEmpty()) {
    return TRUE; //undefined matched corresponds to old behavior
  }

//  osToc = GetStrArraySeparatedByChar(MatchOS, '.');
  XString8Array osToc = Split<XString8Array>(MatchOS, "."_XS8).trimEachString();
  XString8Array currOStoc = Split<XString8Array>(CurrOS, "."_XS8).trimEachString();

  if (osToc.size() == 2) {
    if (currOStoc.size() == 2) {
      if ( osToc[0] == currOStoc[0] && osToc[1] == currOStoc[1]) {
        ret = TRUE;
      }
    }
  } else if (osToc.size() == 3) {
    if (currOStoc.size() == 3) {
      if ( osToc[0] == currOStoc[0]
          && osToc[1] == currOStoc[1]
          && osToc[2] == currOStoc[2]) {
        ret = TRUE;
      } else if ( osToc[0] == currOStoc[0]
                 && osToc[1] == currOStoc[1]
                 && osToc[2].equalIC("x") ) {
        ret = TRUE;
      }
    } else if (currOStoc.size() == 2) {
      if ( osToc[0] == currOStoc[0]
          && osToc[1] ==  currOStoc[1] ) {
        ret = TRUE;
      } else if ( osToc[0] == currOStoc[0]
                 && osToc[1] ==  currOStoc[1]
                 && osToc[2].equalIC("x") == 0 ) {
        ret = TRUE;
      }
    }
  }
  return ret;
}

UINT8 CheckVolumeType(UINT8 VolumeType, const TagStruct* Prop)
{
 	if ( !Prop->isString() ) {
   	MsgLog("ATTENTION : Prop property not string in CheckVolumeType\n");
   	return 0;
  }
  UINT8 VolumeTypeTmp = VolumeType;
  if (Prop->getString()->stringValue().equalIC("Internal")) {
    VolumeTypeTmp |= VOLTYPE_INTERNAL;
  } else if (Prop->getString()->stringValue().equalIC("External")) {
    VolumeTypeTmp |= VOLTYPE_EXTERNAL;
  } else if (Prop->getString()->stringValue().equalIC("Optical")) {
    VolumeTypeTmp |= VOLTYPE_OPTICAL;
  } else if (Prop->getString()->stringValue().equalIC("FireWire")) {
    VolumeTypeTmp |= VOLTYPE_FIREWIRE;
  }
  return VolumeTypeTmp;
}

UINT8 GetVolumeType(const TagDict* DictPointer)
{
  const TagStruct* Prop;
  UINT8 VolumeType = 0;

  Prop = DictPointer->propertyForKey("VolumeType");
  if (Prop != NULL) {
    if (Prop->isString()) {
      VolumeType = CheckVolumeType(0, Prop);
    } else if (Prop->isArray()) {
      INTN   i;
      INTN   Count = Prop->getArray()->arrayContent().size();
      if (Count > 0) {
        for (i = 0; i < Count; i++) {
          const TagStruct* Prop2 = &Prop->getArray()->arrayContent()[i];
          if ( !Prop2->isString() || Prop2->getString()->stringValue().isEmpty() ) {
            continue;
          }
          VolumeType = CheckVolumeType(VolumeType, Prop2);
        }
      }
    }
  }
  return VolumeType;
}


STATIC
BOOLEAN
FillinCustomEntry (
                   IN OUT  CUSTOM_LOADER_ENTRY *Entry,
                   const TagDict* DictPointer,
                   IN      BOOLEAN SubEntry
                   )
{
  const TagStruct* Prop;

  if ((Entry == NULL) || (DictPointer == NULL)) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Disabled");
  if (IsPropertyNotNullAndTrue(Prop)) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Volume");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Volume = Prop->getString()->stringValue();
  }

  Prop = DictPointer->propertyForKey("Path");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Path = Prop->getString()->stringValue();
  }

  Prop = DictPointer->propertyForKey("Settings");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Settings = Prop->getString()->stringValue();
  }

  Prop = DictPointer->propertyForKey("CommonSettings");
  Entry->CommonSettings = IsPropertyNotNullAndTrue(Prop);


  Prop = DictPointer->propertyForKey("AddArguments");
  if (Prop != NULL && (Prop->isString())) {
//    if (Entry->LoadOptions.notEmpty()) {
//      Entry->Options.SPrintf("%s %s", Entry->Options.c_str(), Prop->getString()->stringValue());
//    } else {
//      Entry->Options.SPrintf("%s", Prop->getString()->stringValue());
//    }
	Entry->LoadOptions.import(Split<XString8Array>(Prop->getString()->stringValue(), " "));
  } else {
    Prop = DictPointer->propertyForKey("Arguments");
    if (Prop != NULL && (Prop->isString())) {
//      Entry->Options.SPrintf("%s", Prop->getString()->stringValue());
      Entry->LoadOptions = Split<XString8Array>(Prop->getString()->stringValue(), " ");
      Entry->Flags       = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);
    }
  }
  Prop = DictPointer->propertyForKey("Title");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Title = Prop->getString()->stringValue();
  }
  Prop = DictPointer->propertyForKey("FullTitle");
  if (Prop != NULL && (Prop->isString())) {
    Entry->FullTitle = Prop->getString()->stringValue();
  }

  Prop = DictPointer->propertyForKey("Image");
  if (Prop != NULL) {
    Entry->ImagePath.setEmpty();
    Entry->Image.setEmpty();
    if (Prop->isString()) {
      Entry->ImagePath = SWPrintf("%s", Prop->getString()->stringValue().c_str());
    }
    // we can't load the file yet, as ThemeDir is not initialized
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "ImageData", &DataLen);
    if (TmpData) {
      if (!EFI_ERROR(Entry->Image.Image.FromPNG(TmpData, DataLen))) {
        Entry->Image.setFilled();
      }
      FreePool(TmpData);
    }
  }

  Prop = DictPointer->propertyForKey("DriveImage");
  if (Prop != NULL) {
    Entry->DriveImagePath.setEmpty();
    Entry->DriveImage.setEmpty();
    if (Prop->isString()) {
      Entry->DriveImagePath = SWPrintf("%s", Prop->getString()->stringValue().c_str());
    }
    // we can't load the file yet, as ThemeDir is not initialized
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "ImageData", &DataLen);
    if (TmpData) {
      if (!EFI_ERROR(Entry->DriveImage.Image.FromPNG(TmpData, DataLen))) {
        Entry->DriveImage.setFilled();
      }
      FreePool(TmpData);
    }
  }

  Prop = DictPointer->propertyForKey("Hotkey");
  if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
    Entry->Hotkey = Prop->getString()->stringValue()[0];
  }

  // Whether or not to draw boot screen
  Prop = DictPointer->propertyForKey("CustomLogo");
  if (Prop != NULL) {
    if (IsPropertyNotNullAndTrue(Prop)) {
      Entry->CustomBoot    = CUSTOM_BOOT_APPLE;
    } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
      if (Prop->getString()->stringValue().equalIC("Apple")) {
        Entry->CustomBoot  = CUSTOM_BOOT_APPLE;
      } else if (Prop->getString()->stringValue().equalIC("Alternate")) {
        Entry->CustomBoot  = CUSTOM_BOOT_ALT_APPLE;
      } else if (Prop->getString()->stringValue().equalIC("Theme")) {
        Entry->CustomBoot  = CUSTOM_BOOT_THEME;
      } else {
        XStringW customLogo = XStringW() = Prop->getString()->stringValue();
        Entry->CustomBoot  = CUSTOM_BOOT_USER;
        Entry->CustomLogo.LoadXImage(SelfRootDir, customLogo);
        if (Entry->CustomLogo.isEmpty()) {
          DBG("Custom boot logo not found at path `%ls`!\n", customLogo.wc_str());
          Entry->CustomBoot = CUSTOM_BOOT_DISABLED;
        }
      }
    } else if ( Prop->isData() && Prop->getData()->dataLenValue() > 0 ) {
      Entry->CustomBoot = CUSTOM_BOOT_USER;
      Entry->CustomLogo.FromPNG(Prop->getData()->dataValue(), Prop->getData()->dataLenValue());
      if (Entry->CustomLogo.isEmpty()) {
        DBG("Custom boot logo not decoded from data!\n"/*, Prop->getString()->stringValue().c_str()*/);
        Entry->CustomBoot = CUSTOM_BOOT_DISABLED;
      }
    } else {
      Entry->CustomBoot = CUSTOM_BOOT_USER_DISABLED;
    }
    DBG("Custom entry boot %s LogoWidth = (0x%lld)\n", CustomBootModeToStr(Entry->CustomBoot), Entry->CustomLogo.GetWidth());
  } else {
    Entry->CustomBoot = CUSTOM_BOOT_DISABLED;
  }

  Prop = DictPointer->propertyForKey("BootBgColor");
  if (Prop != NULL && Prop->isString()) {
    UINTN   Color;
    Color = AsciiStrHexToUintn(Prop->getString()->stringValue());

    Entry->BootBgColor.Red = (Color >> 24) & 0xFF;
    Entry->BootBgColor.Green = (Color >> 16) & 0xFF;
    Entry->BootBgColor.Blue = (Color >> 8) & 0xFF;
    Entry->BootBgColor.Reserved = (Color >> 0) & 0xFF;
  }

  // Hidden Property, Values:
  // - No (show the entry)
  // - Yes (hide the entry but can be show with F3)
  // - Always (always hide the entry)
  Prop = DictPointer->propertyForKey("Hidden");
  if (Prop != NULL) {
    if ((Prop->isString()) &&
        (Prop->getString()->stringValue().equalIC("Always"))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else if (IsPropertyNotNullAndTrue(Prop)) {
      Entry->Hidden = true;
    } else {
      Entry->Hidden = false;
    }
  }

  Prop = DictPointer->propertyForKey("Type");
  if (Prop != NULL && (Prop->isString())) {
    if ((Prop->getString()->stringValue().equalIC("OSX")) ||
        (Prop->getString()->stringValue().equalIC("macOS"))) {
      Entry->Type = OSTYPE_OSX;
    } else if (Prop->getString()->stringValue().equalIC("OSXInstaller")) {
      Entry->Type = OSTYPE_OSX_INSTALLER;
    } else if (Prop->getString()->stringValue().equalIC("OSXRecovery")) {
      Entry->Type = OSTYPE_RECOVERY;
    } else if (Prop->getString()->stringValue().equalIC("Windows")) {
      Entry->Type = OSTYPE_WINEFI;
    } else if (Prop->getString()->stringValue().equalIC("Linux")) {
      Entry->Type = OSTYPE_LIN;
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);
    } else if (Prop->getString()->stringValue().equalIC("LinuxKernel")) {
      Entry->Type = OSTYPE_LINEFI;
    } else {
      DBG("** Warning: unknown custom entry Type '%s'\n", Prop->getString()->stringValue().c_str());
      Entry->Type = OSTYPE_OTHER;
    }
  } else {
    if (Entry->Type == 0 && Entry->Path.notEmpty()) {
      // Try to set Entry->type from Entry->Path
      Entry->Type = GetOSTypeFromPath(Entry->Path);
    }
  }

  Entry->VolumeType = GetVolumeType(DictPointer);

  if (Entry->LoadOptions.isEmpty() && OSTYPE_IS_WINDOWS(Entry->Type)) {
    Entry->LoadOptions.Add("-s");
    Entry->LoadOptions.Add("-h");
  }
  if (Entry->Title.isEmpty()) {
    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
      Entry->Title = L"Recovery"_XSW;
    } else if (OSTYPE_IS_OSX_INSTALLER(Entry->Type)) {
      Entry->Title = L"Install macOS"_XSW;
    }
  }
  if (Entry->Image.isEmpty() && (Entry->ImagePath.isEmpty())) {
    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
      Entry->ImagePath = L"mac"_XSW;
    }
  }
  if (Entry->DriveImage.isEmpty() && (Entry->DriveImagePath.isEmpty())) {
    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
      Entry->DriveImagePath = L"recovery"_XSW;
    }
  }
  // OS Specific flags
  if (OSTYPE_IS_OSX(Entry->Type) || OSTYPE_IS_OSX_RECOVERY(Entry->Type) || OSTYPE_IS_OSX_INSTALLER(Entry->Type)) {

    // InjectKexts default values
    Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_CHECKFAKESMC);
    //  Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_WITHKEXTS);

    Prop = DictPointer->propertyForKey("InjectKexts");
    if (Prop != NULL) {
      if ( Prop->isTrueOrYes() ) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      } else if ( Prop->isString() && Prop->getString()->stringValue().equalIC("Detect") ) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      } else {
        DBG("** Warning: unknown custom entry InjectKexts value '%s'\n", Prop->getString()->stringValue().c_str());
      }
    } else {
      // Use global settings
      if (gSettings.WithKexts) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
      if (gSettings.WithKextsIfNoFakeSMC) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
    }

    // NoCaches default value
    Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_NOCACHES);

    Prop = DictPointer->propertyForKey("NoCaches");
    if (Prop != NULL) {
      if (IsPropertyNotNullAndTrue(Prop)) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
      } else {
        // Use global settings
        if (gSettings.NoCaches) {
          Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
        }
      }
    }

    // KernelAndKextPatches
    if (!SubEntry) { // CopyKernelAndKextPatches already in: DuplicateCustomEntry if SubEntry == TRUE
      //DBG("Copying global patch settings\n");
//      CopyKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)),
 //                               (KERNEL_AND_KEXT_PATCHES *)(((UINTN)&gSettings) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)));

//      CopyKernelAndKextPatches(&Entry->KernelAndKextPatches, &gSettings.KernelAndKextPatches);
      Entry->KernelAndKextPatches = gSettings.KernelAndKextPatches;

      //#ifdef DUMP_KERNEL_KEXT_PATCHES
      //    DumpKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)));
      //#endif

    }

  }

  if (Entry->Type == OSTYPE_LINEFI) {
    Prop = DictPointer->propertyForKey("Kernel");
    if (Prop != NULL) {
      if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        if ((Prop->getString()->stringValue()[0] == 'N') || (Prop->getString()->stringValue()[0] == 'n')) {
          Entry->KernelScan = KERNEL_SCAN_NEWEST;
        } else if ((Prop->getString()->stringValue()[0] == 'O') || (Prop->getString()->stringValue()[0] == 'o')) {
          Entry->KernelScan = KERNEL_SCAN_OLDEST;
        } else if ((Prop->getString()->stringValue()[0] == 'F') || (Prop->getString()->stringValue()[0] == 'f')) {
          Entry->KernelScan = KERNEL_SCAN_FIRST;
        } else if ((Prop->getString()->stringValue()[0] == 'L') || (Prop->getString()->stringValue()[0] == 'l')) {
          Entry->KernelScan = KERNEL_SCAN_LAST;
        } else if ((Prop->getString()->stringValue()[0] == 'M') || (Prop->getString()->stringValue()[0] == 'm')) {
          Entry->KernelScan = KERNEL_SCAN_MOSTRECENT;
        } else if ((Prop->getString()->stringValue()[0] == 'E') || (Prop->getString()->stringValue()[0] == 'e')) {
          Entry->KernelScan = KERNEL_SCAN_EARLIEST;
        }
      }
    }
  }

  /*
   * Sub entries
   * an array of dict OR a bool
  */
  Prop = DictPointer->propertyForKey("SubEntries");
  if (Prop != NULL) {
    if ( Prop->isBool() && Prop->getBool()->boolValue() ) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
    } else if ( Prop->isArray() ) {
      CUSTOM_LOADER_ENTRY *CustomSubEntry;
      INTN   i;
      INTN   Count = Prop->getArray()->arrayContent().size();
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
      for (i = 0; i < Count; i++) {
        const TagDict* Dict = Prop->getArray()->dictElementAt(i, "SubEntries"_XS8);
        // Allocate a sub entry
        CustomSubEntry = DuplicateCustomEntry (Entry);
        if (CustomSubEntry) {
          if (!FillinCustomEntry (CustomSubEntry, Dict, TRUE) || !AddCustomSubEntry (Entry, CustomSubEntry)) {
            if (CustomSubEntry) {
              FreePool(CustomSubEntry);
            }
          }
        }
      }
    }else{
      MsgLog("MALFORMED PLIST : SubEntries must be a bool OR an array of dict");
    }
  }
  return TRUE;
}

BOOLEAN
FillingCustomLegacy (
                    IN OUT CUSTOM_LEGACY_ENTRY *Entry,
                    const TagDict* DictPointer
                    )
{
  const TagStruct* Prop;
  if ((Entry == NULL) || (DictPointer == NULL)) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Disabled");
  if (IsPropertyNotNullAndTrue(Prop)) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Volume");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Volume = Prop->getString()->stringValue();
  }

  Prop = DictPointer->propertyForKey("FullTitle");
  if (Prop != NULL && (Prop->isString())) {
    Entry->FullTitle = Prop->getString()->stringValue();
  }

  Prop = DictPointer->propertyForKey("Title");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Title = Prop->getString()->stringValue();
  }
  Prop = DictPointer->propertyForKey("Image");
  if (Prop != NULL) {
    if (Prop->isString()) {
      Entry->Image.LoadXImage(ThemeX.ThemeDir, Prop->getString()->stringValue());
    }
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "ImageData", &DataLen);
    if (TmpData) {
      if (!EFI_ERROR(Entry->Image.Image.FromPNG(TmpData, DataLen))) {
        Entry->Image.setFilled();
      }
      FreePool(TmpData);
    }
  }

  Prop = DictPointer->propertyForKey("DriveImage");
  if (Prop != NULL) {
    if (Prop->isString()) {
      Entry->Image.LoadXImage(ThemeX.ThemeDir, Prop->getString()->stringValue());
    }
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "DriveImageData", &DataLen);
    if (TmpData) {
      if (!EFI_ERROR(Entry->DriveImage.Image.FromPNG(TmpData, DataLen))) {
        Entry->DriveImage.setFilled();
      }
      FreePool(TmpData);
    }
  }

  Prop = DictPointer->propertyForKey("Hotkey");
  if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
    Entry->Hotkey = Prop->getString()->stringValue()[0];
  }

  // Hidden Property, Values:
  // - No (show the entry)
  // - Yes (hide the entry but can be show with F3)
  // - Always (always hide the entry)
  Prop = DictPointer->propertyForKey("Hidden");
  if (Prop != NULL) {
    if ((Prop->isString()) &&
        (Prop->getString()->stringValue().equalIC("Always"))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else if (IsPropertyNotNullAndTrue(Prop)) {
      Entry->Hidden = true;
    } else {
      Entry->Hidden = false;
    }
  }

  Prop = DictPointer->propertyForKey("Type");
  if (Prop != NULL && (Prop->isString())) {
    if (Prop->getString()->stringValue().equalIC("Windows")) {
      Entry->Type = OSTYPE_WIN;
    } else if (Prop->getString()->stringValue().equalIC("Linux")) {
      Entry->Type = OSTYPE_LIN;
    } else {
      Entry->Type = OSTYPE_OTHER;
    }
  }

  Entry->VolumeType = GetVolumeType(DictPointer);
  return TRUE;
}

BOOLEAN
FillingCustomTool (IN OUT CUSTOM_TOOL_ENTRY *Entry, const TagDict* DictPointer)
{
  const TagStruct* Prop;
  if ((Entry == NULL) || (DictPointer == NULL)) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Disabled");
  if (IsPropertyNotNullAndTrue(Prop)) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Volume");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Volume.takeValueFrom(Prop->getString()->stringValue());
  }

  Prop = DictPointer->propertyForKey("Path");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Path.takeValueFrom(Prop->getString()->stringValue());
  }

  Prop = DictPointer->propertyForKey("Arguments");
  if (Prop != NULL && (Prop->isString())) {
//    if (!Entry->Options.isEmpty()) {
//      Entry->Options.setEmpty();
//    } else {
//      Entry->Options.SPrintf("%s", Prop->getString()->stringValue());
//    }
      Entry->LoadOptions = Split<XString8Array>(Prop->getString()->stringValue(), " ");
  }

  Prop = DictPointer->propertyForKey("FullTitle");
  if (Prop != NULL && (Prop->isString())) {
    Entry->FullTitle = Prop->getString()->stringValue();
  }

  Prop = DictPointer->propertyForKey("Title");
  if (Prop != NULL && (Prop->isString())) {
    Entry->Title = Prop->getString()->stringValue();
  }

  Prop = DictPointer->propertyForKey("Image");
  if (Prop != NULL) {
    Entry->ImagePath.setEmpty();
    if (Prop->isString()) {
      Entry->ImagePath = Prop->getString()->stringValue();
    }
    Entry->Image.LoadXImage(ThemeX.ThemeDir, Entry->ImagePath);
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "ImageData", &DataLen);
    if (TmpData) {
      if (!EFI_ERROR(Entry->Image.Image.FromPNG(TmpData, DataLen))) {
        Entry->Image.setFilled();
      }
      FreePool(TmpData);
    }
  }
  Prop = DictPointer->propertyForKey("Hotkey");
  if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
    Entry->Hotkey = Prop->getString()->stringValue()[0];
  }

  // Hidden Property, Values:
  // - No (show the entry)
  // - Yes (hide the entry but can be show with F3)
  // - Always (always hide the entry)
  Prop = DictPointer->propertyForKey("Hidden");
  if (Prop != NULL) {
    if ((Prop->isString()) &&
        (Prop->getString()->stringValue().equalIC("Always"))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else if (IsPropertyNotNullAndTrue(Prop)) {
      Entry->Hidden = true;
    } else {
      Entry->Hidden = false;
    }
  }

  Entry->VolumeType = GetVolumeType(DictPointer);

  return TRUE;
}

// EDID reworked by Sherlocks
VOID
GetEDIDSettings(const TagDict* DictPointer)
{
  const TagStruct* Prop;
  const TagDict* Dict;
  UINTN  j = 128;

  Dict = DictPointer->dictPropertyForKey("EDID");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Inject");
    gSettings.InjectEDID = IsPropertyNotNullAndTrue(Prop); // default = false!

    if (gSettings.InjectEDID){
      //DBG("Inject EDID\n");
      Prop = Dict->propertyForKey("Custom");
      if (Prop != NULL) {
        gSettings.CustomEDID   = GetDataSetting(Dict, "Custom", &j);
        if ((j % 128) != 0) {
          DBG(" Custom EDID has wrong length=%llu\n", j);
        } else {
          DBG(" Custom EDID is ok\n");
          gSettings.CustomEDIDsize = (UINT16)j;
          InitializeEdidOverride();
        }
      }

      Prop = Dict->propertyForKey("VendorID");
      if (Prop) {
        gSettings.VendorEDID = (UINT16)GetPropertyAsInteger(Prop, gSettings.VendorEDID);
        //DBG("  VendorID = 0x%04lx\n", gSettings.VendorEDID);
      }

      Prop = Dict->propertyForKey("ProductID");
      if (Prop) {
        gSettings.ProductEDID = (UINT16)GetPropertyAsInteger(Prop, gSettings.ProductEDID);
        //DBG("  ProductID = 0x%04lx\n", gSettings.ProductEDID);
      }

      Prop = Dict->propertyForKey("HorizontalSyncPulseWidth");
      if (Prop) {
        gSettings.EdidFixHorizontalSyncPulseWidth = (UINT16)GetPropertyAsInteger(Prop, gSettings.EdidFixHorizontalSyncPulseWidth);
        //DBG("  EdidFixHorizontalSyncPulseWidth = 0x%02lx\n", gSettings.EdidFixHorizontalSyncPulseWidth);
      }

      Prop = Dict->propertyForKey("VideoInputSignal");
      if (Prop) {
        gSettings.EdidFixVideoInputSignal = (UINT8)GetPropertyAsInteger(Prop, gSettings.EdidFixVideoInputSignal);
        //DBG("  EdidFixVideoInputSignal = 0x%02lx\n", gSettings.EdidFixVideoInputSignal);
      }
    } else {
      //DBG("Not Inject EDID\n");
    }
  }
}

EFI_STATUS
GetEarlyUserSettings (
                      IN EFI_FILE *RootDir,
                      const TagDict* CfgDict
                      )
{
  EFI_STATUS  Status = EFI_SUCCESS;
//  const TagDict*      Dict;
//  const TagDict*      Dict2;
//  const TagDict*      DictPointer;
//  const TagStruct*    Prop;
//  const TagArray*     arrayProp;
  VOID        *Value = NULL;
  BOOLEAN     SpecialBootMode = FALSE;

  {
    UINTN       Size = 0;
    //read aptiofixflag from nvram for special boot
    Status = GetVariable2(L"aptiofixflag", &gEfiAppleBootGuid, &Value, &Size);
    if (!EFI_ERROR(Status)) {
      SpecialBootMode = TRUE;
      FreePool(Value);
    }
  }

  gSettings.KextPatchesAllowed              = TRUE;
  gSettings.KernelAndKextPatches.KPAppleRTC = TRUE;
  gSettings.KernelAndKextPatches.KPDELLSMBIOS = FALSE; // default is false
  gSettings.KernelPatchesAllowed            = TRUE;

  if (CfgDict != NULL) {
    //DBG("Loading early settings\n");
    DbgHeader("GetEarlyUserSettings");

    const TagDict* BootDict = CfgDict->dictPropertyForKey("Boot");
    if (BootDict != NULL) {
      const TagStruct* Prop = BootDict->propertyForKey("Timeout");
      if (Prop != NULL) {
        GlobalConfig.Timeout = (INT32)GetPropertyAsInteger(Prop, GlobalConfig.Timeout);
		  DBG("timeout set to %lld\n", GlobalConfig.Timeout);
      }

      Prop = BootDict->propertyForKey("SkipHibernateTimeout");
      gSettings.SkipHibernateTimeout = IsPropertyNotNullAndTrue(Prop);

      //DisableCloverHotkeys
      Prop = BootDict->propertyForKey("DisableCloverHotkeys");
      gSettings.DisableCloverHotkeys = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("Arguments");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        gSettings.BootArgs = Prop->getString()->stringValue();
      }

      // defaults if "DefaultVolume" is not present or is empty
      gSettings.LastBootedVolume = FALSE;
      //     gSettings.DefaultVolume    = NULL;

      Prop = BootDict->propertyForKey("DefaultVolume");
      if (Prop != NULL) {
        if ( Prop->isString()  &&  Prop->getString()->stringValue().notEmpty() ) {
          gSettings.DefaultVolume.setEmpty();
          // check for special value for remembering boot volume
          if (Prop->getString()->stringValue().equalIC("LastBootedVolume")) {
            gSettings.LastBootedVolume = TRUE;
          } else {
            gSettings.DefaultVolume = Prop->getString()->stringValue();
          }
        }
      }

      Prop = BootDict->propertyForKey("DefaultLoader");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in DefaultLoader\n");
        }else{
	        gSettings.DefaultLoader = Prop->getString()->stringValue();
	      }
      }

      Prop = BootDict->propertyForKey("Debug");
      GlobalConfig.DebugLog       = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("Fast");
      GlobalConfig.FastBoot       = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("NoEarlyProgress");
      GlobalConfig.NoEarlyProgress = IsPropertyNotNullAndTrue(Prop);

      if (SpecialBootMode) {
        GlobalConfig.FastBoot       = TRUE;
        DBG("Fast option enabled\n");
      }

      Prop = BootDict->propertyForKey("NeverHibernate");
      GlobalConfig.NeverHibernate = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("StrictHibernate");
      GlobalConfig.StrictHibernate = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("RtcHibernateAware");
      GlobalConfig.RtcHibernateAware = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("HibernationFixup");
      if (Prop) {
        GlobalConfig.HibernationFixup = IsPropertyNotNullAndTrue(Prop); //it will be set automatically
      }

      Prop = BootDict->propertyForKey("SignatureFixup");
      GlobalConfig.SignatureFixup = IsPropertyNotNullAndTrue(Prop);

      //      Prop = GetProperty(DictPointer, "GetLegacyLanAddress");
      //      GetLegacyLanAddress = IsPropertyTrue(Prop);

      // Secure boot
      Prop = BootDict->propertyForKey("Secure");
      if (Prop != NULL) {
        if ( Prop->isFalse() ) {
          // Only disable setup mode, we want always secure boot
          gSettings.SecureBootSetupMode = 0;
        } else if ( Prop->isTrue()  &&  !gSettings.SecureBoot ) {
          // This mode will force boot policy even when no secure boot or it is disabled
          gSettings.SecureBootSetupMode = 1;
          gSettings.SecureBoot          = 1;
        }
      }
      // Secure boot policy
      Prop = BootDict->propertyForKey("Policy");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        if ((Prop->getString()->stringValue()[0] == 'D') || (Prop->getString()->stringValue()[0] == 'd')) {
          // Deny all images
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_DENY;
        } else if ((Prop->getString()->stringValue()[0] == 'A') || (Prop->getString()->stringValue()[0] == 'a')) {
          // Allow all images
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_ALLOW;
        } else if ((Prop->getString()->stringValue()[0] == 'Q') || (Prop->getString()->stringValue()[0] == 'q')) {
          // Query user
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_QUERY;
        } else if ((Prop->getString()->stringValue()[0] == 'I') || (Prop->getString()->stringValue()[0] == 'i')) {
          // Insert
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_INSERT;
        } else if ((Prop->getString()->stringValue()[0] == 'W') || (Prop->getString()->stringValue()[0] == 'w')) {
          // White list
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_WHITELIST;
        } else if ((Prop->getString()->stringValue()[0] == 'B') || (Prop->getString()->stringValue()[0] == 'b')) {
          // Black list
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_BLACKLIST;
        } else if ((Prop->getString()->stringValue()[0] == 'U') || (Prop->getString()->stringValue()[0] == 'u')) {
          // User policy
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_USER;
        }
      }
      // Secure boot white list
      const TagArray* arrayProp = BootDict->arrayPropertyForKey("WhiteList");
      if (Prop != NULL) {
        INTN   i;
        INTN   Count = arrayProp->arrayContent().size();
        if (Count > 0) {
          gSettings.SecureBootWhiteListCount = 0;
          gSettings.SecureBootWhiteList = (__typeof__(gSettings.SecureBootWhiteList))AllocateZeroPool(Count * sizeof(CHAR16 *));
          if (gSettings.SecureBootWhiteList) {
            for (i = 0; i < Count; i++) {
              const TagStruct* prop2 = &arrayProp->arrayContent()[i];
              if ( !prop2->isString() ) {
                MsgLog("MALFORMED PLIST : WhiteList must be an array of string");
                continue;
              }
              if ( prop2->getString()->stringValue().notEmpty() ) {
                gSettings.SecureBootWhiteList[gSettings.SecureBootWhiteListCount++] = SWPrintf("%s", prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
              }
            }
          }
        }
      }
      // Secure boot black list
      arrayProp = BootDict->arrayPropertyForKey("BlackList");
      if (arrayProp != NULL && arrayProp->isArray()) {
        INTN   i;
        INTN   Count = arrayProp->arrayContent().size();
        if (Count > 0) {
          gSettings.SecureBootBlackListCount = 0;
          gSettings.SecureBootBlackList = (__typeof__(gSettings.SecureBootBlackList))AllocateZeroPool(Count * sizeof(CHAR16 *));
          if (gSettings.SecureBootBlackList) {
            for (i = 0; i < Count; i++) {
              const TagStruct* prop2 = &arrayProp->arrayContent()[i];
              if ( !prop2->isString() ) {
                MsgLog("MALFORMED PLIST : BlackList must be an array of string");
                continue;
              }
              if ( prop2->getString()->stringValue().notEmpty() ) {
                gSettings.SecureBootBlackList[gSettings.SecureBootBlackListCount++] = SWPrintf("%s", prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
              }
            }
          }
        }
      }

      // XMP memory profiles
      Prop = BootDict->propertyForKey("XMPDetection");
      if (Prop != NULL) {
        gSettings.XMPDetection = 0;
        if ( Prop->isFalse() ) {
          gSettings.XMPDetection = -1;
        } else if ( Prop->isString() ) {
          if ((Prop->getString()->stringValue()[0] == 'n') ||
              (Prop->getString()->stringValue()[0] == 'N') ||
              (Prop->getString()->stringValue()[0] == '-')) {
            gSettings.XMPDetection = -1;
          } else {
            gSettings.XMPDetection = (INT8)AsciiStrDecimalToUintn(Prop->getString()->stringValue().c_str());
          }
        } else if (Prop->isInt64()) {
          gSettings.XMPDetection   = Prop->getInt64()->intValue();
        }
        // Check that the setting value is sane
        if ((gSettings.XMPDetection < -1) || (gSettings.XMPDetection > 2)) {
          gSettings.XMPDetection   = -1;
        }
      }

      // Legacy bios protocol
      Prop = BootDict->propertyForKey("Legacy");
      if (Prop != NULL)  {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : Prop property not string in Legacy\n");
        }else{
          gSettings.LegacyBoot = Prop->getString()->stringValue();
        }
      } else if (gFirmwareClover) {
        // default for CLOVER EFI boot
        gSettings.LegacyBoot = "PBR"_XS8;
      } else {
        // default for UEFI boot
        gSettings.LegacyBoot = "LegacyBiosDefault"_XS8;
      }

      // Entry for LegacyBiosDefault
      Prop = BootDict->propertyForKey("LegacyBiosDefaultEntry");
      if (Prop != NULL) {
        gSettings.LegacyBiosDefaultEntry = (UINT16)GetPropertyAsInteger(Prop, 0); // disabled by default
      }

      // Whether or not to draw boot screen
      Prop = BootDict->propertyForKey("CustomLogo");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          gSettings.CustomBoot   = CUSTOM_BOOT_APPLE;
        } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          if (Prop->getString()->stringValue().equalIC("Apple")) {
            gSettings.CustomBoot = CUSTOM_BOOT_APPLE;
          } else if (Prop->getString()->stringValue().equalIC("Alternate")) {
            gSettings.CustomBoot = CUSTOM_BOOT_ALT_APPLE;
          } else if (Prop->getString()->stringValue().equalIC("Theme")) {
            gSettings.CustomBoot = CUSTOM_BOOT_THEME;
          } else {
            XStringW customLogo = XStringW() = Prop->getString()->stringValue();
            gSettings.CustomBoot = CUSTOM_BOOT_USER;
            if (gSettings.CustomLogo != NULL) {
              delete gSettings.CustomLogo;
            }
            gSettings.CustomLogo = new XImage;
            gSettings.CustomLogo->LoadXImage(RootDir, customLogo);
            if (gSettings.CustomLogo->isEmpty()) {
              DBG("Custom boot logo not found at path `%ls`!\n", customLogo.wc_str());
              gSettings.CustomBoot = CUSTOM_BOOT_DISABLED;
            }
          }
        } else if ( Prop->isData()  && Prop->getData()->dataLenValue() > 0 ) {
          gSettings.CustomBoot = CUSTOM_BOOT_USER;
          if (gSettings.CustomLogo != NULL) {
            delete gSettings.CustomLogo;
          }
          gSettings.CustomLogo = new XImage;
          gSettings.CustomLogo->FromPNG(Prop->getData()->dataValue(), Prop->getData()->dataLenValue());
          if (gSettings.CustomLogo->isEmpty()) {
            DBG("Custom boot logo not decoded from data!\n"/*, Prop->getString()->stringValue().c_str()*/);
            gSettings.CustomBoot = CUSTOM_BOOT_DISABLED;
          }
        } else {
          gSettings.CustomBoot = CUSTOM_BOOT_USER_DISABLED;
        }
      } else {
        gSettings.CustomBoot   = CUSTOM_BOOT_DISABLED;
      }
      DBG("Custom boot %s (0x%llX)\n", CustomBootModeToStr(gSettings.CustomBoot), (uintptr_t)gSettings.CustomLogo);
    }

    //*** SYSTEM ***

    const TagDict* SystemParametersDict = CfgDict->dictPropertyForKey("SystemParameters");
    if (SystemParametersDict != NULL) {
      // Inject kexts
      const TagStruct* Prop = SystemParametersDict->propertyForKey("InjectKexts");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          gSettings.WithKexts            = TRUE;
        } else if ((Prop->isString()) &&
                   (Prop->getString()->stringValue().equalIC("Detect"))) {
          //   gSettings.WithKexts            = TRUE;
          gSettings.WithKextsIfNoFakeSMC = TRUE;
        }
      } else {
        gSettings.WithKexts            = TRUE;  //default
      }

      // No caches - obsolete
      Prop = SystemParametersDict->propertyForKey("NoCaches");
      if (IsPropertyNotNullAndTrue(Prop)) {
        gSettings.NoCaches = TRUE;
      }
      //test float
      Prop = SystemParametersDict->propertyForKey("BlueValue");
      float tmpF = GetPropertyFloat(Prop, 1.2f);
      DBG(" get BlueValue=%f\n", tmpF);
      
    }

    // KernelAndKextPatches
    const TagDict* KernelAndKextPatchesDict = CfgDict->dictPropertyForKey("KernelAndKextPatches");
    if (KernelAndKextPatchesDict != NULL) {
      FillinKextPatches(&gSettings.KernelAndKextPatches, KernelAndKextPatchesDict);
    }

    const TagDict* GUIDict = CfgDict->dictPropertyForKey("GUI");
    if (GUIDict != NULL) {
      const TagStruct* Prop = GUIDict->propertyForKey("Timezone");
      GlobalConfig.Timezone = (INT32)GetPropertyAsInteger(Prop, GlobalConfig.Timezone);
      //initialize Daylight when we know timezone
      EFI_TIME          Now;
      gRT->GetTime(&Now, NULL);
      INT32 NowHour = Now.Hour + GlobalConfig.Timezone;
      if (NowHour <  0 ) NowHour += 24;
      if (NowHour >= 24 ) NowHour -= 24;
      ThemeX.Daylight = (NowHour > 8) && (NowHour < 20);

      Prop = GUIDict->propertyForKey("Theme");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        ThemeX.Theme.takeValueFrom(Prop->getString()->stringValue());
        GlobalConfig.Theme.takeValueFrom(Prop->getString()->stringValue());
        DBG("Default theme: %ls\n", GlobalConfig.Theme.wc_str());
        OldChosenTheme = 0xFFFF; //default for embedded
        for (UINTN i = 0; i < ThemesNum; i++) {
          //now comparison is case sensitive
          if (StriCmp(GlobalConfig.Theme.wc_str(), ThemesList[i]) == 0) {
            OldChosenTheme = i;
            break;
          }
        }
      }
      // get embedded theme property even when starting with other themes, as they may be changed later
      Prop = GUIDict->propertyForKey("EmbeddedThemeType");
      if (Prop && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        if (Prop->getString()->stringValue().equalIC("Dark")) {
          ThemeX.DarkEmbedded = TRUE;
          //ThemeX.Font = FONT_GRAY;
        } else if (Prop->getString()->stringValue().equalIC("Light")) {
          ThemeX.DarkEmbedded = FALSE;
          //ThemeX.Font = FONT_ALFA;
        } else if (Prop->getString()->stringValue().equalIC("Daytime")) {
          ThemeX.DarkEmbedded = !ThemeX.Daylight;
          //ThemeX.Font = ThemeX.Daylight?FONT_ALFA:FONT_GRAY;
        }
      }
      Prop = GUIDict->propertyForKey("PlayAsync"); //PlayAsync
      gSettings.PlayAsync = IsPropertyNotNullAndTrue(Prop);

      // CustomIcons
      Prop = GUIDict->propertyForKey("CustomIcons");
      GlobalConfig.CustomIcons = IsPropertyNotNullAndTrue(Prop);
      Prop = GUIDict->propertyForKey("TextOnly");
      GlobalConfig.TextOnly = IsPropertyNotNullAndTrue(Prop);
      Prop = GUIDict->propertyForKey("ShowOptimus");
      GlobalConfig.ShowOptimus = IsPropertyNotNullAndTrue(Prop);

      Prop = GUIDict->propertyForKey("ScreenResolution");
      if (Prop != NULL) {
        if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          GlobalConfig.ScreenResolution.takeValueFrom(Prop->getString()->stringValue());
        }
      }

      Prop = GUIDict->propertyForKey("ConsoleMode");
      if (Prop != NULL) {
        if (Prop->isInt64()) {
          GlobalConfig.ConsoleMode = Prop->getInt64()->intValue();
        } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          if ( Prop->getString()->stringValue().contains("Max") ) {
            GlobalConfig.ConsoleMode = -1;
            DBG("ConsoleMode will be set to highest mode\n");
          } else if ( Prop->getString()->stringValue().contains("Min") ) {
            GlobalConfig.ConsoleMode = -2;
            DBG("ConsoleMode will be set to lowest mode\n");
          } else {
            GlobalConfig.ConsoleMode = (INT32)AsciiStrDecimalToUintn(Prop->getString()->stringValue());
          }
        }
        if (GlobalConfig.ConsoleMode > 0) {
          DBG("ConsoleMode will be set to mode #%lld\n", GlobalConfig.ConsoleMode);
        }
      }

      Prop = GUIDict->propertyForKey("Language");
      if (Prop != NULL) {
        gSettings.Language = Prop->getString()->stringValue();
        if ( Prop->getString()->stringValue().contains("en") ) {
          gLanguage = english;
          GlobalConfig.Codepage = 0xC0;
          GlobalConfig.CodepageSize = 0;
        } else if ( Prop->getString()->stringValue().contains("ru")) {
          gLanguage = russian;
          GlobalConfig.Codepage = 0x410;
          GlobalConfig.CodepageSize = 0x40;
        } else if ( Prop->getString()->stringValue().contains("ua")) {
          gLanguage = ukrainian;
          GlobalConfig.Codepage = 0x400;
          GlobalConfig.CodepageSize = 0x60;
        } else if ( Prop->getString()->stringValue().contains("fr")) {
          gLanguage = french; //default is extended latin
        } else if ( Prop->getString()->stringValue().contains("it")) {
          gLanguage = italian;
        } else if ( Prop->getString()->stringValue().contains("es")) {
          gLanguage = spanish;
        } else if ( Prop->getString()->stringValue().contains("pt")) {
          gLanguage = portuguese;
        } else if ( Prop->getString()->stringValue().contains("br")) {
          gLanguage = brasil;
        } else if ( Prop->getString()->stringValue().contains("de")) {
          gLanguage = german;
        } else if ( Prop->getString()->stringValue().contains("nl")) {
          gLanguage = dutch;
        } else if ( Prop->getString()->stringValue().contains("pl")) {
          gLanguage = polish;
        } else if ( Prop->getString()->stringValue().contains("cz")) {
          gLanguage = czech;
        } else if ( Prop->getString()->stringValue().contains("hr")) {
          gLanguage = croatian;
        } else if ( Prop->getString()->stringValue().contains("id")) {
          gLanguage = indonesian;
        } else if ( Prop->getString()->stringValue().contains("zh_CN")) {
          gLanguage = chinese;
          GlobalConfig.Codepage = 0x3400;
          GlobalConfig.CodepageSize = 0x19C0;
        } else if ( Prop->getString()->stringValue().contains("ro")) {
          gLanguage = romanian;
        } else if ( Prop->getString()->stringValue().contains("ko")) {
          gLanguage = korean;
          GlobalConfig.Codepage = 0x1100;
          GlobalConfig.CodepageSize = 0x100;
        }
      }

//      if (gSettings.Language != NULL) { // gSettings.Language != NULL cannot be false because gSettings.Language is dclared as CHAR8 Language[16]; Must we replace by gSettings.Language[0] != NULL
      Prop = GUIDict->propertyForKey("KbdPrevLang");
      if (Prop != NULL) {
        gSettings.KbdPrevLang = IsPropertyNotNullAndTrue(Prop);
      }
//      }

      const TagDict* MouseDict = GUIDict->dictPropertyForKey("Mouse");
      if (MouseDict != NULL) {
        const TagStruct* prop = MouseDict->propertyForKey("Speed");
        if (prop != NULL) {
          gSettings.PointerSpeed = (INT32)GetPropertyAsInteger(prop, 0);
          gSettings.PointerEnabled = (gSettings.PointerSpeed != 0);
        }
        //but we can disable mouse even if there was positive speed
        prop = MouseDict->propertyForKey("Enabled");
        if (IsPropertyNotNullAndFalse(prop)) {
          gSettings.PointerEnabled = FALSE;
        }

        prop = MouseDict->propertyForKey("Mirror");
        if (IsPropertyNotNullAndTrue(prop)) {
          gSettings.PointerMirror = TRUE;
        }

        prop = MouseDict->propertyForKey("DoubleClickTime");
        if (prop != NULL) {
          gSettings.DoubleClickTime = (UINT64)GetPropertyAsInteger(prop, 500);
        }
      }
      // hide by name/uuid. Array of string
      const TagArray* HideArray = GUIDict->arrayPropertyForKey("Hide");
      if (HideArray != NULL) {
        INTN   i;
        INTN   Count = HideArray->arrayContent().size();
        if (Count > 0) {
          gSettings.HVHideStrings.setEmpty();
          for (i = 0; i < Count; i++) {
            const TagStruct* prop2 = &HideArray->arrayContent()[i];
            if ( !prop2->isString()) {
              MsgLog("MALFORMED PLIST : Hide must be an array of string");
              continue;
            }
            if ( prop2->getString()->stringValue().notEmpty() ) {
              gSettings.HVHideStrings.Add(prop2->getString()->stringValue());
              DBG("Hiding entries with string %s\n", prop2->getString()->stringValue().c_str());
            }
          }
        }
      }
      gSettings.LinuxScan = TRUE;
      // Disable loader scan
      Prop = GUIDict->propertyForKey("Scan");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndFalse(Prop)) {
          gSettings.DisableEntryScan = TRUE;
          gSettings.DisableToolScan  = TRUE;
          GlobalConfig.NoLegacy      = TRUE;
        } else if (Prop->isDict()) {
          const TagStruct* prop2 = Prop->getDict()->propertyForKey("Entries");
          if (IsPropertyNotNullAndFalse(prop2)) {
            gSettings.DisableEntryScan = TRUE;
          }

          prop2 = Prop->getDict()->propertyForKey("Tool");
          if (IsPropertyNotNullAndFalse(prop2)) {
            gSettings.DisableToolScan = TRUE;
          }

          prop2 = Prop->getDict()->propertyForKey("Linux");
          gSettings.LinuxScan = !IsPropertyNotNullAndFalse(prop2);

          prop2 = Prop->getDict()->propertyForKey("Legacy");
          if (prop2 != NULL) {
            if (prop2->isFalse()) {
              GlobalConfig.NoLegacy = TRUE;
            } else if ((prop2->isString()) && prop2->getString()->stringValue().notEmpty() ) {
              if ((prop2->getString()->stringValue()[0] == 'N') || (prop2->getString()->stringValue()[0] == 'n')) {
                GlobalConfig.NoLegacy = TRUE;
              } else if ((prop2->getString()->stringValue()[0] == 'F') || (prop2->getString()->stringValue()[0] == 'f')) {
                GlobalConfig.LegacyFirst = TRUE;
              }
            }
          }

          prop2 = Prop->getDict()->propertyForKey("Kernel");
          if (prop2 != NULL) {
            if (prop2->isFalse()) {
              gSettings.KernelScan = KERNEL_SCAN_NONE;
            } else if ((prop2->isString()) && prop2->getString()->stringValue().notEmpty() ) {
              if ((prop2->getString()->stringValue()[0] == 'N') || (prop2->getString()->stringValue()[0] == 'n')) {
                gSettings.KernelScan = ( prop2->getString()->stringValue().length() > 1  &&  (prop2->getString()->stringValue()[1] == 'E' || prop2->getString()->stringValue()[1] == 'e') ) ? KERNEL_SCAN_NEWEST : KERNEL_SCAN_NONE;
              } else if ((prop2->getString()->stringValue()[0] == 'O') || (prop2->getString()->stringValue()[0] == 'o')) {
                gSettings.KernelScan = KERNEL_SCAN_OLDEST;
              } else if ((prop2->getString()->stringValue()[0] == 'F') || (prop2->getString()->stringValue()[0] == 'f')) {
                gSettings.KernelScan = KERNEL_SCAN_FIRST;
              } else if ((prop2->getString()->stringValue()[0] == 'L') || (prop2->getString()->stringValue()[0] == 'l')) {
                gSettings.KernelScan = KERNEL_SCAN_LAST;
              } else if ((prop2->getString()->stringValue()[0] == 'M') || (prop2->getString()->stringValue()[0] == 'm')) {
                gSettings.KernelScan = KERNEL_SCAN_MOSTRECENT;
              } else if ((prop2->getString()->stringValue()[0] == 'E') || (prop2->getString()->stringValue()[0] == 'e')) {
                gSettings.KernelScan = KERNEL_SCAN_EARLIEST;
              }
            }
          }
        }
      }
      // Custom entries
      const TagDict* CustomDict2 = GUIDict->dictPropertyForKey("Custom");
      if (CustomDict2 != NULL) {
        const TagArray* arrayProp = CustomDict2->arrayPropertyForKey("Entries"); // Entries is an array of dict
        if (Prop != NULL) {
          INTN   i;
          INTN   Count = arrayProp->arrayContent().size();
          if (Count > 0) {
            for (i = 0; i < Count; i++) {
              const TagDict* Dict3 = arrayProp->dictElementAt(i, "Custom/Entries"_XS8);
              // Allocate an entry
              CUSTOM_LOADER_ENTRY* Entry = new CUSTOM_LOADER_ENTRY;
              // Fill it in
              if (Entry != NULL && (!FillinCustomEntry(Entry, Dict3, FALSE) || !AddCustomLoaderEntry(Entry))) {
                delete Entry;
              }
            }
          }
        }

        const TagArray* LegacyArray = CustomDict2->arrayPropertyForKey("Legacy"); // is an array of dict
        if (LegacyArray != NULL) {
          CUSTOM_LEGACY_ENTRY *Entry;
          INTN   i;
          INTN   Count = LegacyArray->arrayContent().size();
          if (Count > 0) {
            for (i = 0; i < Count; i++) {
              const TagDict* Dict3 = LegacyArray->dictElementAt(i, "Legacy"_XS8);
              // Allocate an entry
              Entry = new CUSTOM_LEGACY_ENTRY;
              if (Entry) {
                // Fill it in
                if (!FillingCustomLegacy(Entry, Dict3) || !AddCustomLegacyEntry(Entry)) {
                  FreePool(Entry);
                }
              }
            }
          }
        }

        const TagArray* ToolArray = CustomDict2->arrayPropertyForKey("Tool"); // is an array of dict
        if (ToolArray != NULL) {
          CUSTOM_TOOL_ENTRY *Entry;
          INTN   i;
          INTN   Count = ToolArray->arrayContent().size();
          if (Count > 0) {
            for (i = 0; i < Count; i++) {
              const TagDict* Dict3 = ToolArray->dictElementAt(i, "Tool"_XS8);
              if ( !Dict3->isDict() ) {
                MsgLog("MALFORMED PLIST : Entries must be an array of dict");
                continue;
              }
              // Allocate an entry
              Entry = new CUSTOM_TOOL_ENTRY;
              if (Entry) {
                // Fill it in
                if (!FillingCustomTool(Entry, Dict3) || !AddCustomToolEntry(Entry)) {
                  FreePool(Entry);
                }
              }
            }
          }
        }
      }
    }

    const TagDict* GraphicsDict = CfgDict->dictPropertyForKey("Graphics");
    if (GraphicsDict != NULL) {

      const TagStruct* Prop             = GraphicsDict->propertyForKey("PatchVBios");
      gSettings.PatchVBios              = IsPropertyNotNullAndTrue(Prop);

      gSettings.PatchVBiosBytesCount    = 0;

      const TagArray* Dict2 = GraphicsDict->arrayPropertyForKey("PatchVBiosBytes"); // array of dict
      if (Dict2 != NULL) {
        INTN   i;
        INTN   Count = Dict2->arrayContent().size();
        if (Count > 0) {
          VBIOS_PATCH_BYTES *VBiosPatch;
          UINTN             FindSize    = 0;
          UINTN             ReplaceSize = 0;
          BOOLEAN           Valid;

          // alloc space for up to 16 entries
          gSettings.PatchVBiosBytes = (__typeof__(gSettings.PatchVBiosBytes))AllocateZeroPool(Count * sizeof(VBIOS_PATCH_BYTES));

          // get all entries
          for (i = 0; i < Count; i++) {
            const TagDict* dict3 = Dict2->dictElementAt(i, "Graphics/PatchVBiosBytes"_XS8);
            Valid = TRUE;
            // read entry
            VBiosPatch          = &gSettings.PatchVBiosBytes[gSettings.PatchVBiosBytesCount];
            VBiosPatch->Find    = GetDataSetting (dict3, "Find",    &FindSize);
            VBiosPatch->Replace = GetDataSetting (dict3, "Replace", &ReplaceSize);

            if (VBiosPatch->Find == NULL || FindSize == 0) {
              Valid = FALSE;
				DBG("PatchVBiosBytes[%lld]: missing Find data\n", i);
            }

            if (VBiosPatch->Replace == NULL || ReplaceSize == 0) {
              Valid = FALSE;
				DBG("PatchVBiosBytes[%lld]: missing Replace data\n", i);
            }

            if (FindSize != ReplaceSize) {
              Valid = FALSE;
				DBG("PatchVBiosBytes[%lld]: Find and Replace data are not the same size\n", i);
            }

            if (Valid) {
              VBiosPatch->NumberOfBytes = FindSize;
              // go to next entry
              ++gSettings.PatchVBiosBytesCount;
            } else {
              // error - release mem
              if (VBiosPatch->Find != NULL) {
                FreePool(VBiosPatch->Find);
                VBiosPatch->Find = NULL;
              }

              if (VBiosPatch->Replace != NULL) {
                FreePool(VBiosPatch->Replace);
                VBiosPatch->Replace = NULL;
              }
            }
          }

          if (gSettings.PatchVBiosBytesCount == 0) {
            FreePool(gSettings.PatchVBiosBytes);
            gSettings.PatchVBiosBytes = NULL;
          }
        }
      }

      GetEDIDSettings(GraphicsDict);
    }

    const TagArray* DisableDriversArray = CfgDict->arrayPropertyForKey("DisableDrivers"); // array of string
    if (DisableDriversArray != NULL) {
      INTN   i;
      INTN   Count = DisableDriversArray->arrayContent().size();
      if (Count > 0) {
        gSettings.BlackListCount = 0;
        gSettings.BlackList = (__typeof__(gSettings.BlackList))AllocateZeroPool(Count * sizeof(CHAR16 *));

        for (i = 0; i < Count; i++) {
          const TagStruct* Prop = &DisableDriversArray->arrayContent()[i];
          if ( !Prop->isString()) {
            MsgLog("MALFORMED PLIST : DisableDrivers must be an array of string");
            continue;
          }
          gSettings.BlackList[gSettings.BlackListCount++] = SWPrintf("%s", Prop->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
        }
      }
    }

    const TagDict* DevicesDict = CfgDict->dictPropertyForKey("Devices");
    if (DevicesDict != NULL) {
      const TagDict* Dict2 = DevicesDict->dictPropertyForKey("Audio");
      if (Dict2 != NULL) {
        // HDA
        const TagStruct* Prop = Dict2->propertyForKey("ResetHDA");
        gSettings.ResetHDA = IsPropertyNotNullAndTrue(Prop);
      }
    }

    const TagDict* RtVariablesDict = CfgDict->dictPropertyForKey("RtVariables");
    if (RtVariablesDict != NULL) {
      const TagStruct* Prop = RtVariablesDict->propertyForKey("ROM");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          // that's ok. Property can be data, but not when the value is 'UseMacAddr0' or 'UseMacAddr1';
        }else{
          if ((Prop->getString()->stringValue().equalIC("UseMacAddr0")) ||
              (Prop->getString()->stringValue().equalIC("UseMacAddr1"))) {
            GetLegacyLanAddress = TRUE;
          }
        }
      }
    }

    const TagDict* DictPointer = CfgDict->dictPropertyForKey("Quirks");
    if (DictPointer != NULL) {
      const TagStruct* Prop;
      Prop               = DictPointer->propertyForKey( "AvoidRuntimeDefrag");
      gQuirks.AvoidRuntimeDefrag = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.AvoidRuntimeDefrag? QUIRK_DEFRAG:0;
      Prop               = DictPointer->propertyForKey( "DevirtualiseMmio");
      gQuirks.DevirtualiseMmio   = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.DevirtualiseMmio? QUIRK_MMIO:0;
      Prop               = DictPointer->propertyForKey( "DisableSingleUser");
      gQuirks.DisableSingleUser  = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.DisableSingleUser? QUIRK_SU:0;
      Prop               = DictPointer->propertyForKey( "DisableVariableWrite");
      gQuirks.DisableVariableWrite = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.DisableVariableWrite? QUIRK_VAR:0;
      Prop               = DictPointer->propertyForKey( "DiscardHibernateMap");
      gQuirks.DiscardHibernateMap = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.DiscardHibernateMap? QUIRK_HIBER:0;
      Prop               = DictPointer->propertyForKey( "EnableSafeModeSlide");
      gQuirks.EnableSafeModeSlide = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.EnableSafeModeSlide? QUIRK_SAFE:0;
      Prop               = DictPointer->propertyForKey( "EnableWriteUnprotector");
      gQuirks.EnableWriteUnprotector = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.EnableWriteUnprotector? QUIRK_UNPROT:0;
      Prop               = DictPointer->propertyForKey( "ForceExitBootServices");
      gQuirks.ForceExitBootServices = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.ForceExitBootServices? QUIRK_EXIT:0;
      Prop               = DictPointer->propertyForKey( "ProtectMemoryRegions");
      gQuirks.ProtectMemoryRegions = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.ProtectMemoryRegions? QUIRK_REGION:0;
      Prop               = DictPointer->propertyForKey( "ProtectSecureBoot");
      gQuirks.ProtectSecureBoot = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.ProtectMemoryRegions? QUIRK_SECURE:0;
      Prop               = DictPointer->propertyForKey( "ProtectUefiServices");
      gQuirks.ProtectUefiServices = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.ProtectUefiServices? QUIRK_UEFI:0;
      Prop               = DictPointer->propertyForKey( "ProvideConsoleGopEnable");
      gProvideConsoleGopEnable = IsPropertyNotNullAndTrue(Prop);
      Prop               = DictPointer->propertyForKey( "ProvideCustomSlide");
      gQuirks.ProvideCustomSlide = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.ProvideCustomSlide? QUIRK_CUSTOM:0;
      Prop               = DictPointer->propertyForKey( "ProvideMaxSlide");
      gQuirks.ProvideMaxSlide = GetPropertyAsInteger(Prop, 0);
      Prop               = DictPointer->propertyForKey( "RebuildAppleMemoryMap");
      gQuirks.RebuildAppleMemoryMap = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.RebuildAppleMemoryMap? QUIRK_MAP:0;
      Prop               = DictPointer->propertyForKey( "SetupVirtualMap");
      gQuirks.SetupVirtualMap = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.SetupVirtualMap? QUIRK_VIRT:0;
      Prop               = DictPointer->propertyForKey( "SignalAppleOS");
      gQuirks.SignalAppleOS = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.SignalAppleOS? QUIRK_OS:0;
      Prop               = DictPointer->propertyForKey( "SyncRuntimePermissions");
      gQuirks.SyncRuntimePermissions = IsPropertyNotNullAndTrue(Prop);
      gSettings.QuirksMask  |= gQuirks.SyncRuntimePermissions? QUIRK_PERM:0;
      const TagArray* Dict2 = DictPointer->arrayPropertyForKey("MmioWhitelist"); // array of dict
      if (Dict2 != NULL) {
        INTN   Count = Dict2->arrayContent().size();
        //OC_SCHEMA_INTEGER_IN  ("Address", OC_MMIO_WL_STRUCT, Address),
        //OC_SCHEMA_STRING_IN   ("Comment", OC_MMIO_WL_STRUCT, Comment),
        //OC_SCHEMA_BOOLEAN_IN  ("Enabled", OC_MMIO_WL_STRUCT, Enabled),
        if (Count > 0) {
          gQuirks.MmioWhitelistLabels = (__typeof__(gQuirks.MmioWhitelistLabels))AllocatePool(sizeof(char*) * Count);
          gQuirks.MmioWhitelist = (__typeof__(gQuirks.MmioWhitelist))AllocatePool(sizeof(*gQuirks.MmioWhitelist) * Count);
          gQuirks.MmioWhitelistEnabled = (__typeof__(gQuirks.MmioWhitelistEnabled))AllocatePool(sizeof(BOOLEAN) * Count);
          gQuirks.MmioWhitelistSize = 0;
          for (INTN i = 0; i < Count; i++) {
            const TagDict* Dict3 = Dict2->dictElementAt(i, "MmioWhitelist"_XS8);

            gQuirks.MmioWhitelistLabels[gQuirks.MmioWhitelistSize] = (__typeof__(char *))AllocateZeroPool(256);
            
            const TagStruct* Prop2 = Dict3->propertyForKey("Comment");
            if (Prop2 != NULL && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
              snprintf(gQuirks.MmioWhitelistLabels[gQuirks.MmioWhitelistSize], 255, "%s", Prop2->getString()->stringValue().c_str());
            } else {
              snprintf(gQuirks.MmioWhitelistLabels[gQuirks.MmioWhitelistSize], 255, " (NoLabel)");
            }
            
            Prop2 = Dict3->propertyForKey("Address");
            if (Prop2 != 0) {
              gQuirks.MmioWhitelist[gQuirks.MmioWhitelistSize] = GetPropertyAsInteger(Prop2, 0);
              Prop2 = Dict3->propertyForKey("Enabled");
              gQuirks.MmioWhitelistEnabled[gQuirks.MmioWhitelistSize] = IsPropertyNotNullAndTrue(Prop2);
            }
            gQuirks.MmioWhitelistSize++;
          }
        }
      }
    }
  }

  return Status;
}

VOID
GetListOfConfigs ()
{
  REFIT_DIR_ITER    DirIter;
  EFI_FILE_INFO     *DirEntry;
  INTN              NameLen;

  ConfigsNum = 0;
  OldChosenConfig = 0;

  DirIterOpen(SelfRootDir, OEMPath.wc_str(), &DirIter);
  DbgHeader("Found config plists");
  while (DirIterNext(&DirIter, 2, L"config*.plist", &DirEntry)) {
    CHAR16  FullName[256];
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }

	  snwprintf(FullName, 512, "%ls\\%ls", OEMPath.wc_str(), DirEntry->FileName);
    if (FileExists(SelfRootDir, FullName)) {
      if (StriCmp(DirEntry->FileName, L"config.plist") == 0) {
        OldChosenConfig = ConfigsNum;
      }
      NameLen = StrLen(DirEntry->FileName) - 6; //without ".plist"
      ConfigsList[ConfigsNum] = (CHAR16*)AllocateCopyPool(NameLen * sizeof(CHAR16) + 2, DirEntry->FileName);
      ConfigsList[ConfigsNum++][NameLen] = L'\0';
      DBG("- %ls\n", DirEntry->FileName);
    }
  }
  DirIterClose(&DirIter);
}

VOID
GetListOfDsdts ()
{
  REFIT_DIR_ITER    DirIter;
  EFI_FILE_INFO     *DirEntry;
  INTN              NameLen;
  XStringW          AcpiPath = SWPrintf("%ls\\ACPI\\patched", OEMPath.wc_str());

  if (DsdtsNum > 0) {
    for (UINTN i = 0; i < DsdtsNum; i++) {
      if (DsdtsList[DsdtsNum] != NULL) {
        FreePool(DsdtsList[DsdtsNum]);
      }
    }
  }   
  DsdtsNum = 0;
  OldChosenDsdt = 0xFFFF;

  DirIterOpen(SelfRootDir, AcpiPath.wc_str(), &DirIter);
  DbgHeader("Found DSDT tables");
  while (DirIterNext(&DirIter, 2, L"DSDT*.aml", &DirEntry)) {
    CHAR16  FullName[256];
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }

    snwprintf(FullName, 512, "%ls\\%ls", AcpiPath.wc_str(), DirEntry->FileName);
    if (FileExists(SelfRootDir, FullName)) {
      if ( gSettings.DsdtName.equalIC(DirEntry->FileName) ) {
        OldChosenDsdt = DsdtsNum;
      }
      NameLen = StrLen(DirEntry->FileName); //with ".aml"
      DsdtsList[DsdtsNum] = (CHAR16*)AllocateCopyPool(NameLen * sizeof(CHAR16) + 2, DirEntry->FileName); // if changing, notice freepool above
      DsdtsList[DsdtsNum++][NameLen] = L'\0';
      DBG("- %ls\n", DirEntry->FileName);
    }
  }
  DirIterClose(&DirIter);
}


VOID
GetListOfACPI ()
{
  REFIT_DIR_ITER    DirIter;
  EFI_FILE_INFO     *DirEntry;
  ACPI_PATCHED_AML  *ACPIPatchedAMLTmp;
  INTN               Count = gSettings.DisabledAMLCount;
  XStringW           AcpiPath = SWPrintf("%ls\\ACPI\\patched", OEMPath.wc_str());

  while (ACPIPatchedAML != NULL) {
    if (ACPIPatchedAML->FileName) {
      FreePool(ACPIPatchedAML->FileName);
    }
    ACPIPatchedAMLTmp = ACPIPatchedAML;
    ACPIPatchedAML = ACPIPatchedAML->Next;
    FreePool(ACPIPatchedAMLTmp);
  }
  ACPIPatchedAML = NULL;

  DirIterOpen(SelfRootDir, AcpiPath.wc_str(), &DirIter);

  while (DirIterNext(&DirIter, 2, L"*.aml", &DirEntry)) {
    CHAR16  FullName[256];
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }
    if (StriStr(DirEntry->FileName, L"DSDT")) {
      continue;
    }

    snwprintf(FullName, 512, "%ls\\%ls", AcpiPath.wc_str(), DirEntry->FileName);
    if (FileExists(SelfRootDir, FullName)) {
      BOOLEAN ACPIDisabled = FALSE;
      ACPIPatchedAMLTmp = new ACPI_PATCHED_AML; // if changing, notice freepool above
      ACPIPatchedAMLTmp->FileName = SWPrintf("%ls", DirEntry->FileName).forgetDataWithoutFreeing(); // if changing, notice freepool above

      for (INTN i = 0; i < Count; i++) {
        if ((gSettings.DisabledAML[i] != NULL) &&
            (StriCmp(ACPIPatchedAMLTmp->FileName, gSettings.DisabledAML[i]) == 0)
            ) {
          ACPIDisabled = TRUE;
          break;
        }
      }
      ACPIPatchedAMLTmp->MenuItem.BValue = ACPIDisabled;
      ACPIPatchedAMLTmp->Next = ACPIPatchedAML;
      ACPIPatchedAML = ACPIPatchedAMLTmp;
    }
  }

  DirIterClose(&DirIter);
}

XStringW GetBundleVersion(const XStringW& FullName)
{
  EFI_STATUS      Status;
  XStringW        CFBundleVersion;
  XStringW        InfoPlistPath;
  CHAR8*          InfoPlistPtr = NULL;
  TagDict*      InfoPlistDict = NULL;
  const TagStruct*      Prop = NULL;
  UINTN           Size;

  InfoPlistPath = SWPrintf("%ls\\%ls", FullName.wc_str(), L"Contents\\Info.plist");
  Status = egLoadFile(SelfRootDir, InfoPlistPath.wc_str(), (UINT8**)&InfoPlistPtr, &Size);
  if (EFI_ERROR(Status)) {
//    InfoPlistPath = SWPrintf("%ls", FullName, L"Info.plist"); // Jief : there was this line. Seems that L"Info.plist" parameter was not used
    Status = egLoadFile(SelfRootDir, FullName.wc_str(), (UINT8**)&InfoPlistPtr, &Size);
  }
  if(!EFI_ERROR(Status)) {
    Status = ParseXML(InfoPlistPtr, &InfoPlistDict, Size);
    if(!EFI_ERROR(Status)) {
      Prop = InfoPlistDict->propertyForKey("CFBundleVersion");
      if (Prop != NULL && Prop->isString() && Prop->getString()->stringValue().notEmpty()) {
        CFBundleVersion = SWPrintf("%s", Prop->getString()->stringValue().c_str());
      }
    }
  }
  if (InfoPlistPtr) {
    FreePool(InfoPlistPtr);
  }
  if ( InfoPlistDict ) InfoPlistDict->FreeTag();
  return CFBundleVersion;
}

VOID GetListOfInjectKext(CHAR16 *KextDirNameUnderOEMPath)
{

  REFIT_DIR_ITER  DirIter;
  EFI_FILE_INFO*  DirEntry;
  SIDELOAD_KEXT*  mKext;
  SIDELOAD_KEXT*  mPlugInKext;
  XStringW        FullName;
  XStringW        FullPath = SWPrintf("%ls\\KEXTS\\%ls", OEMPath.wc_str(), KextDirNameUnderOEMPath);
  REFIT_DIR_ITER  PlugInsIter;
  EFI_FILE_INFO   *PlugInEntry;
  XStringW        PlugInsPath;
  XStringW         PlugInsName;
  BOOLEAN         Blocked = FALSE;
  if (StrCmp(KextDirNameUnderOEMPath, L"Off") == 0) {
    Blocked = TRUE;
  }

  DirIterOpen(SelfRootDir, FullPath.wc_str(), &DirIter);
  while (DirIterNext(&DirIter, 1, L"*.kext", &DirEntry)) {
    if (DirEntry->FileName[0] == L'.' || StrStr(DirEntry->FileName, L".kext") == NULL) {
      continue;
    }
    /*
     <key>CFBundleVersion</key>
     <string>8.8.8</string>
     */
    FullName = SWPrintf("%ls\\%ls", FullPath.wc_str(), DirEntry->FileName);

    mKext = new SIDELOAD_KEXT;
    mKext->FileName = SWPrintf("%ls", DirEntry->FileName);
    mKext->MenuItem.BValue = Blocked;
    mKext->KextDirNameUnderOEMPath = SWPrintf("%ls", KextDirNameUnderOEMPath);
    mKext->Next = InjectKextList;
    mKext->Version = GetBundleVersion(FullName);
    InjectKextList = mKext;
    //   DBG("Added mKext=%ls, MatchOS=%ls\n", mKext->FileName, mKext->MatchOS);

    // Obtain PlugInList
    // Iterate over PlugIns directory
    PlugInsPath = SWPrintf("%ls\\%ls", FullName.wc_str(), L"Contents\\PlugIns");

    DirIterOpen(SelfRootDir, PlugInsPath.wc_str(), &PlugInsIter);
    while (DirIterNext(&PlugInsIter, 1, L"*.kext", &PlugInEntry)) {
      if (PlugInEntry->FileName[0] == L'.' || StrStr(PlugInEntry->FileName, L".kext") == NULL) {
        continue;
      }
      PlugInsName = SWPrintf("%ls\\%ls", PlugInsPath.wc_str(), PlugInEntry->FileName);
      mPlugInKext = new SIDELOAD_KEXT;
      mPlugInKext->FileName = SWPrintf("%ls", PlugInEntry->FileName);
      mPlugInKext->MenuItem.BValue = Blocked;
      mPlugInKext->KextDirNameUnderOEMPath = SWPrintf("%ls", KextDirNameUnderOEMPath);
      mPlugInKext->Next    = mKext->PlugInList;
      mPlugInKext->Version = GetBundleVersion(PlugInsName);
      mKext->PlugInList    = mPlugInKext;
      //      DBG("---| added plugin=%ls, MatchOS=%ls\n", mPlugInKext->FileName, mPlugInKext->MatchOS);
    }
    DirIterClose(&PlugInsIter);
  }
  DirIterClose(&DirIter);
}

VOID InitKextList()
{
  REFIT_DIR_ITER  KextsIter;
  EFI_FILE_INFO   *FolderEntry = NULL;
  XStringW        KextsPath;

  if (InjectKextList) {
    return;  //don't scan again
  }
  KextsPath = SWPrintf("%ls\\kexts", OEMPath.wc_str());

  // Iterate over kexts directory

  DirIterOpen(SelfRootDir, KextsPath.wc_str(), &KextsIter);
  while (DirIterNext(&KextsIter, 1, L"*", &FolderEntry)) {
    if (FolderEntry->FileName[0] == L'.') {
      continue;
    }
    GetListOfInjectKext(FolderEntry->FileName);
  }
  DirIterClose(&KextsIter);
}

#define CONFIG_THEME_FILENAME L"theme.plist"
#define CONFIG_THEME_SVG L"theme.svg"

VOID
GetListOfThemes ()
{
  EFI_STATUS     Status          = EFI_NOT_FOUND;
  REFIT_DIR_ITER DirIter;
  EFI_FILE_INFO  *DirEntry;
  XStringW        ThemeTestPath;
  EFI_FILE       *ThemeTestDir   = NULL;
  CHAR8          *ThemePtr       = NULL;
  UINTN          Size = 0;

  DbgHeader("GetListOfThemes");

  ThemesNum = 0;
  DirIterOpen(SelfRootDir, L"\\EFI\\CLOVER\\themes", &DirIter);
  while (DirIterNext(&DirIter, 1, L"*", &DirEntry)) {
    if (DirEntry->FileName[0] == '.') {
      //DBG("Skip theme: %ls\n", DirEntry->FileName);
      continue;
    }
    //DBG("Found theme directory: %ls", DirEntry->FileName);
	  DBG("- [%02llu]: %ls", ThemesNum, DirEntry->FileName);
    ThemeTestPath = SWPrintf("EFI\\CLOVER\\themes\\%ls", DirEntry->FileName);
    Status = SelfRootDir->Open(SelfRootDir, &ThemeTestDir, ThemeTestPath.wc_str(), EFI_FILE_MODE_READ, 0);
    if (!EFI_ERROR(Status)) {
      Status = egLoadFile(ThemeTestDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
      if (EFI_ERROR(Status) || (ThemePtr == NULL) || (Size == 0)) {
        Status = egLoadFile(ThemeTestDir, CONFIG_THEME_SVG, (UINT8**)&ThemePtr, &Size);
        if (EFI_ERROR(Status)) {
          Status = EFI_NOT_FOUND;
          DBG(" - bad theme because %ls nor %ls can't be load", CONFIG_THEME_FILENAME, CONFIG_THEME_SVG);
        }
      }
      if (!EFI_ERROR(Status)) {
        //we found a theme
        if ((StriCmp(DirEntry->FileName, L"embedded") == 0) ||
            (StriCmp(DirEntry->FileName, L"random") == 0)) {
          ThemePtr = NULL;
        } else {
          ThemesList[ThemesNum++] = (CHAR16*)AllocateCopyPool(StrSize(DirEntry->FileName), DirEntry->FileName);
        }
      }
    }
    DBG("\n");
    if (ThemePtr) {
      FreePool(ThemePtr);
    }
  }
  DirIterClose(&DirIter);
}

EFI_STATUS
XTheme::GetThemeTagSettings(const TagDict* DictPointer)
{
  const TagDict* Dict;
  const TagDict* Dict3;
  const TagStruct* Prop;
  const TagStruct* Prop2;

  //fill default to have an ability change theme
  //assume Xtheme is already inited by embedded values
//theme variables
  ScrollWidth                           = 16;
  ScrollButtonsHeight                   = 20;
  ScrollBarDecorationsHeight            = 5;
  ScrollScrollDecorationsHeight         = 7;
  Font                     = FONT_LOAD; //not default

  // if NULL parameter, quit after setting default values, this is embedded theme
  if (DictPointer == NULL) {
    return EFI_SUCCESS;
  }

  Prop    = DictPointer->propertyForKey("BootCampStyle");
  BootCampStyle = IsPropertyNotNullAndTrue(Prop);

  Dict    = DictPointer->dictPropertyForKey("Background");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Type");
    if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
      if ((Prop->getString()->stringValue()[0] == 'S') || (Prop->getString()->stringValue()[0] == 's')) {
        BackgroundScale = imScale;
      } else if ((Prop->getString()->stringValue()[0] == 'T') || (Prop->getString()->stringValue()[0] == 't')) {
        BackgroundScale = imTile;
      }
    }

    Prop = Dict->propertyForKey("Path");
    if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
      BackgroundName = Prop->getString()->stringValue();
    }

    Prop = Dict->propertyForKey("Sharp");
    BackgroundSharp  = GetPropertyAsInteger(Prop, BackgroundSharp);

    Prop = Dict->propertyForKey("Dark");
    BackgroundDark   = IsPropertyNotNullAndTrue(Prop);
  }

  Prop = DictPointer->propertyForKey("Banner");
  if (Prop != NULL) {
    // retain for legacy themes.
    if ( Prop->isString() && Prop->getString()->stringValue().notEmpty() ) {
      BannerFileName = Prop->getString()->stringValue();
    } else {
      // for new placement settings
      Dict = Prop->getDict();
      Prop2 = Dict->propertyForKey("Path");
      if (Prop2 != NULL) {
        if ( Prop2->isString() && Prop2->getString()->stringValue().notEmpty() ) {
          BannerFileName = Prop2->getString()->stringValue();
        }
      }

      Prop2 = Dict->propertyForKey("ScreenEdgeX");
      if (Prop2 != NULL && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty() ) {
        if (Prop2->getString()->stringValue().equal("left")) {
          BannerEdgeHorizontal = SCREEN_EDGE_LEFT;
        } else if (Prop2->getString()->stringValue().equal("right")) {
          BannerEdgeHorizontal = SCREEN_EDGE_RIGHT;
        }
      }

      Prop2 = Dict->propertyForKey("ScreenEdgeY");
      if (Prop2 != NULL && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty() ) {
        if (Prop2->getString()->stringValue().equal("top")) {
          BannerEdgeVertical = SCREEN_EDGE_TOP;
        } else if (Prop2->getString()->stringValue().equal("bottom")) {
          BannerEdgeVertical = SCREEN_EDGE_BOTTOM;
        }
      }

      Prop2 = Dict->propertyForKey("DistanceFromScreenEdgeX%");
      BannerPosX   = GetPropertyAsInteger(Prop2, 0);

      Prop2 = Dict->propertyForKey("DistanceFromScreenEdgeY%");
      BannerPosY   = GetPropertyAsInteger(Prop2, 0);

      Prop2 = Dict->propertyForKey("NudgeX");
      BannerNudgeX = GetPropertyAsInteger(Prop2, 0);

      Prop2 = Dict->propertyForKey("NudgeY");
      BannerNudgeY = GetPropertyAsInteger(Prop2, 0);
    }
  }

  Dict = DictPointer->dictPropertyForKey("Badges");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Swap");
    if (Prop != NULL && Prop->isTrue()) {
      HideBadges |= HDBADGES_SWAP;
      DBG("OS main and drive as badge\n");
    }

    Prop = Dict->propertyForKey("Show");
    if (Prop != NULL && Prop->isTrue()) {
      HideBadges |= HDBADGES_SHOW;
    }

    Prop = Dict->propertyForKey("Inline");
    if (Prop != NULL && Prop->isTrue()) {
      HideBadges |= HDBADGES_INLINE;
    }

    // blackosx added X and Y position for badge offset.
    Prop = Dict->propertyForKey("OffsetX");
    BadgeOffsetX = GetPropertyAsInteger(Prop, BadgeOffsetX);

    Prop = Dict->propertyForKey("OffsetY");
    BadgeOffsetY = GetPropertyAsInteger(Prop, BadgeOffsetY);

    Prop = Dict->propertyForKey("Scale");
    ThemeX.BadgeScale = GetPropertyAsInteger(Prop, BadgeScale);
  }

  Dict = DictPointer->dictPropertyForKey("Origination");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("DesignWidth");
    ThemeDesignWidth = GetPropertyAsInteger(Prop, ThemeDesignWidth);

    Prop = Dict->propertyForKey("DesignHeight");
    ThemeDesignHeight = GetPropertyAsInteger(Prop, ThemeDesignHeight);
  }

  Dict = DictPointer->dictPropertyForKey("Layout");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("BannerOffset");
    LayoutBannerOffset = GetPropertyAsInteger(Prop, LayoutBannerOffset);

    Prop = Dict->propertyForKey("ButtonOffset");
    LayoutButtonOffset = GetPropertyAsInteger(Prop, LayoutButtonOffset);

    Prop = Dict->propertyForKey("TextOffset");
    LayoutTextOffset = GetPropertyAsInteger(Prop, LayoutTextOffset);

    Prop = Dict->propertyForKey("AnimAdjustForMenuX");
    LayoutAnimMoveForMenuX = GetPropertyAsInteger(Prop, LayoutAnimMoveForMenuX);

    Prop = Dict->propertyForKey("Vertical");
    VerticalLayout = IsPropertyNotNullAndTrue(Prop);

    // GlobalConfig.MainEntriesSize
    Prop = Dict->propertyForKey("MainEntriesSize");
    MainEntriesSize = GetPropertyAsInteger(Prop, MainEntriesSize);

    Prop = Dict->propertyForKey("TileXSpace");
    TileXSpace = GetPropertyAsInteger(Prop, TileXSpace);

    Prop = Dict->propertyForKey("TileYSpace");
    TileYSpace = GetPropertyAsInteger(Prop, TileYSpace);

    Prop = Dict->propertyForKey("SelectionBigWidth");
    row0TileSize = GetPropertyAsInteger(Prop, row0TileSize);

    Prop = Dict->propertyForKey("SelectionSmallWidth");
    row1TileSize = (INTN)GetPropertyAsInteger(Prop, row1TileSize);

  }

  Dict = DictPointer->dictPropertyForKey("Components");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Banner");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_BANNER;
    }

    Prop = Dict->propertyForKey("Functions");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_FUNCS;
    }

    Prop = Dict->propertyForKey("Tools");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_TOOLS;
    }

    Prop = Dict->propertyForKey("Label");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_LABEL;
    }

    Prop = Dict->propertyForKey("Revision");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_REVISION;
    }

    Prop = Dict->propertyForKey("Help");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_HELP;
    }

    Prop = Dict->propertyForKey("MenuTitle");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_MENU_TITLE;
    }

    Prop = Dict->propertyForKey("MenuTitleImage");
    if (Prop && Prop->isFalse()) {
      HideUIFlags |= HIDEUI_FLAG_MENU_TITLE_IMAGE;
    }
  }

  Dict = DictPointer->dictPropertyForKey("Selection");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Color");
    SelectionColor = (UINTN)GetPropertyAsInteger(Prop, SelectionColor);

    Prop = Dict->propertyForKey("Small");
    if ( Prop && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
      SelectionSmallFileName = Prop->getString()->stringValue();
    }

    Prop = Dict->propertyForKey("Big");
    if ( Prop && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
      SelectionBigFileName = Prop->getString()->stringValue();
    }

    Prop = Dict->propertyForKey("Indicator");
    if ( Prop && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
      SelectionIndicatorName = Prop->getString()->stringValue();
    }

    Prop = Dict->propertyForKey("OnTop");
    SelectionOnTop = IsPropertyNotNullAndTrue(Prop);

    Prop = Dict->propertyForKey("ChangeNonSelectedGrey");
    NonSelectedGrey = IsPropertyNotNullAndTrue(Prop);
  }

  Dict = DictPointer->dictPropertyForKey("Scroll");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Width");
    ScrollWidth = (UINTN)GetPropertyAsInteger(Prop, ScrollWidth);

    Prop = Dict->propertyForKey("Height");
    ScrollButtonsHeight = (UINTN)GetPropertyAsInteger(Prop, ScrollButtonsHeight);

    Prop = Dict->propertyForKey("BarHeight");
    ScrollBarDecorationsHeight = (UINTN)GetPropertyAsInteger(Prop, ScrollBarDecorationsHeight);

    Prop = Dict->propertyForKey("ScrollHeight");
    ScrollScrollDecorationsHeight = (UINTN)GetPropertyAsInteger(Prop,ScrollScrollDecorationsHeight);
  }

  Dict = DictPointer->dictPropertyForKey("Font");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Type");
    if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
      if ((Prop->getString()->stringValue()[0] == 'A') || (Prop->getString()->stringValue()[0] == 'B')) {
        Font = FONT_ALFA;
      } else if ((Prop->getString()->stringValue()[0] == 'G') || (Prop->getString()->stringValue()[0] == 'W')) {
        Font = FONT_GRAY;
      } else if ((Prop->getString()->stringValue()[0] == 'L') || (Prop->getString()->stringValue()[0] == 'l')) {
        Font = FONT_LOAD;
      }
    }
    if (Font == FONT_LOAD) {
      Prop = Dict->propertyForKey("Path");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        FontFileName = Prop->getString()->stringValue();
      }
    }
    Prop = Dict->propertyForKey("CharWidth");
    CharWidth = (UINTN)GetPropertyAsInteger(Prop, CharWidth);
    if (CharWidth & 1) {
      MsgLog("Warning! Character width %lld should be even!\n", CharWidth);
    }

    Prop = Dict->propertyForKey("Proportional");
    Proportional = IsPropertyNotNullAndTrue(Prop);
  }
  
  const TagArray* AnimeArray = DictPointer->arrayPropertyForKey("Anime"); // array of dict
  if (AnimeArray != NULL) {
    INTN   Count = AnimeArray->arrayContent().size();
    for (INTN i = 0; i < Count; i++) {
      Dict3 = AnimeArray->dictElementAt(i, "Anime"_XS8);
      if ( !Dict3->isDict() ) {
        MsgLog("MALFORMED PLIST : Anime must be an array of dict");
        continue;
      }

      FILM *NewFilm = new FILM();

      Prop = Dict3->propertyForKey("ID");
      NewFilm->SetIndex((UINTN)GetPropertyAsInteger(Prop, 1)); //default=main screen

      Prop = Dict3->propertyForKey("Path");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        NewFilm->Path = Prop->getString()->stringValue();
      }

      Prop = Dict3->propertyForKey("Frames");
      NewFilm->NumFrames = (UINTN)GetPropertyAsInteger(Prop, 0);

      Prop = Dict3->propertyForKey("FrameTime");
      NewFilm->FrameTime = (UINTN)GetPropertyAsInteger(Prop, 50); //default will be 50ms

      Prop = Dict3->propertyForKey("ScreenEdgeX");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
        if (Prop->getString()->stringValue().equal("left")) {
          NewFilm->ScreenEdgeHorizontal = SCREEN_EDGE_LEFT;
        } else if (Prop->getString()->stringValue().equal("right")) {
          NewFilm->ScreenEdgeHorizontal = SCREEN_EDGE_RIGHT;
        }
      }

      Prop = Dict3->propertyForKey("ScreenEdgeY");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty() ) {
        if (Prop->getString()->stringValue().equal("top")) {
          NewFilm->ScreenEdgeVertical = SCREEN_EDGE_TOP;
        } else if (Prop->getString()->stringValue().equal("bottom")) {
          NewFilm->ScreenEdgeVertical = SCREEN_EDGE_BOTTOM;
        }
      }

      //default values are centre

      Prop = Dict3->propertyForKey("DistanceFromScreenEdgeX%");
      NewFilm->FilmX = GetPropertyAsInteger(Prop, INITVALUE);

      Prop = Dict3->propertyForKey("DistanceFromScreenEdgeY%");
      NewFilm->FilmY = GetPropertyAsInteger(Prop, INITVALUE);

      Prop = Dict3->propertyForKey("NudgeX");
      NewFilm->NudgeX = GetPropertyAsInteger(Prop, INITVALUE);

      Prop = Dict3->propertyForKey("NudgeY");
      NewFilm->NudgeY = GetPropertyAsInteger(Prop, INITVALUE);

      Prop = Dict3->propertyForKey("Once");
      NewFilm->RunOnce = IsPropertyNotNullAndTrue(Prop);

      NewFilm->GetFrames(ThemeX); //used properties: ID, Path, NumFrames
      ThemeX.Cinema.AddFilm(NewFilm);
 //     delete NewFilm; //looks like already deleted
    }
  }

//not sure if it needed
  if (BackgroundName.isEmpty()) {
    BackgroundName.takeValueFrom("background");
  }
  if (BannerFileName.isEmpty()) {
    BannerFileName.takeValueFrom("logo");
  }
  if (SelectionSmallFileName.isEmpty()) {
    SelectionSmallFileName.takeValueFrom("selection_small");
  }
  if (SelectionBigFileName.isEmpty()) {
    SelectionBigFileName.takeValueFrom("selection_big");
  }
  if (SelectionIndicatorName.isEmpty()) {
    SelectionIndicatorName.takeValueFrom("selection_indicator");
  }
  if (FontFileName.isEmpty()) {
    FontFileName.takeValueFrom("font");
  }

  return EFI_SUCCESS;
}

TagDict* XTheme::LoadTheme(const XStringW& TestTheme)

{
  EFI_STATUS Status    = EFI_UNSUPPORTED;
  TagDict*     ThemeDict = NULL;
  CHAR8      *ThemePtr = NULL;
  UINTN      Size      = 0;

  if (TestTheme.isEmpty()) {
    return NULL;
  }
  if (UGAHeight > HEIGHT_2K) {
    ThemePath = SWPrintf("EFI\\CLOVER\\themes\\%ls@2x", TestTheme.wc_str());
  } else {
    ThemePath = SWPrintf("EFI\\CLOVER\\themes\\%ls", TestTheme.wc_str());
  }
  Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath.wc_str(), EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    if (ThemeDir != NULL) {
      ThemeDir->Close (ThemeDir);
      ThemeDir = NULL;
    }
    ThemePath = SWPrintf("EFI\\CLOVER\\themes\\%ls", TestTheme.wc_str());
    Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath.wc_str(), EFI_FILE_MODE_READ, 0);
  }

  if (!EFI_ERROR(Status)) {
    Status = egLoadFile(ThemeDir, CONFIG_THEME_SVG, (UINT8**)&ThemePtr, &Size);
    if (!EFI_ERROR(Status) && (ThemePtr != NULL) && (Size != 0)) {
      Status = ParseSVGXTheme(ThemePtr);
      if (EFI_ERROR(Status)) {
        ThemeDict = NULL;
      } else {
        ThemeDict = TagDict::getEmptyTag();
      }
      if (ThemeDict == NULL) {
        DBG("svg file %ls not parsed\n", CONFIG_THEME_SVG);
      } else {
        DBG("Using vector theme '%ls' (%ls)\n", TestTheme.wc_str(), ThemePath.wc_str());
      }
    } else {
      Status = egLoadFile(ThemeDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
      if (!EFI_ERROR(Status) && (ThemePtr != NULL) && (Size != 0)) {
        Status = ParseXML(ThemePtr, &ThemeDict, 0);
        if (EFI_ERROR(Status)) {
          ThemeDict = NULL;
        }
        if (ThemeDict == NULL) {
          DBG("xml file %ls not parsed\n", CONFIG_THEME_FILENAME);
        } else {
          DBG("Using theme '%ls' (%ls)\n", TestTheme.wc_str(), ThemePath.wc_str());
        }
      }
    }
  }
  if (ThemePtr != NULL) {
    FreePool(ThemePtr);
  }
  return ThemeDict;
}

EFI_STATUS
InitTheme(BOOLEAN UseThemeDefinedInNVRam)
{
  EFI_STATUS Status       = EFI_NOT_FOUND;
  UINTN      Size         = 0;
  UINTN      i;
  TagDict*     ThemeDict    = NULL;
  CHAR8      *ChosenTheme = NULL;
  UINTN      Rnd;
  EFI_TIME   Now;
	
  gRT->GetTime(&Now, NULL);
  DbgHeader("InitXTheme");
  ThemeX.Init();
  
  //initialize Daylight when we know timezone
  if (GlobalConfig.Timezone != 0xFF) { // 0xFF:default=timezone not set
    INT32 NowHour = Now.Hour + GlobalConfig.Timezone;
    if (NowHour <  0 ) NowHour += 24;
    if (NowHour >= 24 ) NowHour -= 24;
    ThemeX.Daylight = (NowHour > 8) && (NowHour < 20);
  } else {
    ThemeX.Daylight = TRUE; // when timezone is not set
  }
  if (ThemeX.Daylight) {
    DBG("use Daylight theme\n");
  } else {
    DBG("use night theme\n");
  }

  for (i = 0; i < 3; i++) {
    //    DBG("validate %d face\n", i);
    textFace[i].valid = FALSE;
  }

  NSVGfontChain *fontChain = fontsDB;
  while (fontChain) {
    NSVGfont *font = fontChain->font;
    //    DBG("free font %s\n", font->fontFamily);
    NSVGfontChain *nextChain = fontChain->next;
    if (font) {
      nsvg__deleteFont(font);
      fontChain->font = NULL;
    }
    FreePool(fontChain);
    fontChain = nextChain;
  }
  //as all font freed then free the chain
  fontsDB = NULL;

  /*
   if (mainParser) {
     nsvg__deleteParser(mainParser);
     DBG("parser deleted\n");
     mainParser = NULL;
   }
   */
  ThemeX.FontImage.setEmpty();

  Rnd = (ThemesNum != 0) ? Now.Second % ThemesNum : 0;

  //  DBG("...done\n");
  ThemeX.GetThemeTagSettings(NULL);

  if (ThemesNum > 0  &&
      (GlobalConfig.Theme.isEmpty() || StriCmp(GlobalConfig.Theme.wc_str(), L"embedded") != 0)) {
    // Try special theme first
      XStringW TestTheme;
 //   if (Time != NULL) {
      if ((Now.Month == 12) && ((Now.Day >= 25) && (Now.Day <= 31))) {
        TestTheme = L"christmas"_XSW;
      } else if ((Now.Month == 1) && ((Now.Day >= 1) && (Now.Day <= 3))) {
        TestTheme = L"newyear"_XSW;
      }

      if (TestTheme.notEmpty()) {
        ThemeDict = ThemeX.LoadTheme(TestTheme);
        if (ThemeDict != NULL) {
          DBG("special theme %ls found and %ls parsed\n", TestTheme.wc_str(), CONFIG_THEME_FILENAME);
//          ThemeX.Theme.takeValueFrom(TestTheme);
          GlobalConfig.Theme = TestTheme;

        } else { // special theme not loaded
          DBG("special theme %ls not found, skipping\n", TestTheme.wc_str()/*, CONFIG_THEME_FILENAME*/);
        }
        TestTheme.setEmpty();
      }
//    }
    // Try theme from nvram
    if (ThemeDict == NULL && UseThemeDefinedInNVRam) {
      ChosenTheme = (__typeof__(ChosenTheme))GetNvramVariable(L"Clover.Theme", &gEfiAppleBootGuid, NULL, &Size);
      if (ChosenTheme != NULL) {
        if (AsciiStrCmp(ChosenTheme, "embedded") == 0) {
          goto finish;
        }
        if (AsciiStrCmp(ChosenTheme, "random") == 0) {
          ThemeDict = ThemeX.LoadTheme(XStringW(ThemesList[Rnd]));
          goto finish;
        }

        TestTheme.takeValueFrom(ChosenTheme);
        if (TestTheme.notEmpty()) {
          ThemeDict = ThemeX.LoadTheme (TestTheme);
          if (ThemeDict != NULL) {
            DBG("theme %s defined in NVRAM found and %ls parsed\n", ChosenTheme, CONFIG_THEME_FILENAME);
//            ThemeX.Theme.takeValueFrom(TestTheme);
            GlobalConfig.Theme = TestTheme;
          } else { // theme from nvram not loaded
            if (GlobalConfig.Theme.notEmpty()) {
              DBG("theme %s chosen from nvram is absent, using theme defined in config: %ls\n", ChosenTheme, GlobalConfig.Theme.wc_str());
            } else {
              DBG("theme %s chosen from nvram is absent, get first theme\n", ChosenTheme);
            }
          }
          TestTheme.setEmpty();
        }
        FreePool(ChosenTheme);
        ChosenTheme = NULL;
      }
    }
    // Try to get theme from settings
    if (ThemeDict == NULL) {
      if (GlobalConfig.Theme.isEmpty()) {
        DBG("no default theme, get random theme %ls\n", ThemesList[Rnd]);
        ThemeDict = ThemeX.LoadTheme(XStringW(ThemesList[Rnd]));
      } else {
        if (StriCmp(GlobalConfig.Theme.wc_str(), L"random") == 0) {
          ThemeDict = ThemeX.LoadTheme(XStringW(ThemesList[Rnd]));
        } else {
          ThemeDict = ThemeX.LoadTheme(GlobalConfig.Theme);
          if (ThemeDict == NULL) {
            DBG("GlobalConfig: %ls not found, get embedded theme\n", GlobalConfig.Theme.wc_str());
          } else {
            DBG("chosen theme %ls\n", GlobalConfig.Theme.wc_str());
          }
        }
      }
    }
  } // ThemesNum>0

finish:
  if (!ThemeDict) {  // No theme could be loaded, use embedded
    DBG(" using embedded theme\n");
    if (ThemeX.DarkEmbedded) { // when using embedded, set Daylight according to darkembedded
      ThemeX.Daylight = FALSE;
    } else {
      ThemeX.Daylight = TRUE;
    }

    ThemeX.FillByEmbedded();
    OldChosenTheme = 0xFFFF;

    if (ThemeX.ThemeDir != NULL) {
      ThemeX.ThemeDir->Close(ThemeX.ThemeDir);
      ThemeX.ThemeDir = NULL;
    }

 //   ThemeX.GetThemeTagSettings(NULL); already done
    //fill some fields
    //ThemeX.Font = FONT_ALFA; //to be inverted later. At start we have FONT_GRAY
    ThemeX.embedded = true;
    Status = StartupSoundPlay(ThemeX.ThemeDir, NULL);
  } else { // theme loaded successfully
    ThemeX.embedded = false;
    ThemeX.Theme.takeValueFrom(GlobalConfig.Theme); //XStringW from CHAR16*)
    // read theme settings
    if (!ThemeX.TypeSVG) {
      const TagDict* DictPointer = ThemeDict->dictPropertyForKey("Theme");
      if (DictPointer != NULL) {
        Status = ThemeX.GetThemeTagSettings(DictPointer);
        if (EFI_ERROR(Status)) {
          DBG("Config theme error: %s\n", efiStrError(Status));
        } else {
          ThemeX.FillByDir();
        }
      }
    }
    ThemeDict->FreeTag();

    if (!ThemeX.Daylight) {
      Status = StartupSoundPlay(ThemeX.ThemeDir, L"sound_night.wav");
      if (EFI_ERROR(Status)) {
        Status = StartupSoundPlay(ThemeX.ThemeDir, L"sound.wav");
      }
    } else {
      Status = StartupSoundPlay(ThemeX.ThemeDir, L"sound.wav");
    }

  }
  for (i = 0; i < ThemesNum; i++) {
    if ( ThemeX.Theme.equalIC(ThemesList[i]) ) {
      OldChosenTheme = i;
      break;
    }
  }
  if (ChosenTheme != NULL) {
    FreePool(ChosenTheme);
  }
  if (!ThemeX.TypeSVG) {
    ThemeX.PrepareFont();
  }

  //ThemeX.ClearScreen();
  return Status;
}

VOID
ParseSMBIOSSettings(
                    const TagDict* DictPointer
                    )
{
  const TagStruct* Prop;
  const TagStruct* Prop1;
  BOOLEAN Default = FALSE;


  Prop = DictPointer->propertyForKey("ProductName");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in ProductName\n");
    }else{
      MACHINE_TYPES Model;
      gSettings.ProductName = Prop->getString()->stringValue();
      // let's fill all other fields based on this ProductName
      // to serve as default
      Model = GetModelFromString(gSettings.ProductName);
      if (Model != MaxMachineType) {
        SetDMISettingsForModel (Model, FALSE);
        Default = TRUE;
      } else {
        //if new model then fill at least as iMac13,2, except custom ProductName
        // something else?
        SetDMISettingsForModel (iMac132, FALSE);
      }
    }
  }
  DBG("Using ProductName from config: %s\n", gSettings.ProductName.c_str());

  Prop = DictPointer->propertyForKey("SmbiosVersion");
  gSettings.SmbiosVersion = (UINT16)GetPropertyAsInteger(Prop, 0x204);

  // Check for BiosVersion and BiosReleaseDate by Sherlocks
  Prop = DictPointer->propertyForKey("BiosVersion");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      DBG("BiosVersion: not set, Using BiosVersion from clover\n");
    }else{
      const CHAR8* i = gSettings.RomVersion.c_str();
      const CHAR8* j = Prop->getString()->stringValue().c_str();

      i += AsciiStrLen(i);
      while (*i != '.') {
        i--;
      }

      j += AsciiStrLen(j);
      while (*j != '.') {
        j--;
      }

      if (((i[1] > '0') && (j[1] == '0')) || ((i[1] >= j[1]) && (i[2] > j[2]))) {
        DBG("Using latest BiosVersion from clover\n");
      } else if ((i[1] == j[1]) && (i[2] == j[2])) {
        if (((i[3] > '0') && (j[3] == '0')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
          DBG("Using latest BiosVersion from clover\n");
        } else if ((i[3] == j[3]) && (i[4] == j[4])) {
          if (((i[5] > '0') && (j[5] == '0')) || ((i[5] > '1') && (j[5] == '1')) ||
              ((i[5] > '2') && (j[5] == '2')) || ((i[5] >= j[5]) && (i[6] > j[6]))) {
            DBG("Using latest BiosVersion from clover\n");
          } else if ((i[5] == j[5]) && (i[6] == j[6])) {
            DBG("Found same BiosVersion in clover and config\n");
          } else {
            gSettings.RomVersion = Prop->getString()->stringValue();
            DBG("Using latest BiosVersion from config\n");
          }
        } else {
          gSettings.RomVersion = Prop->getString()->stringValue();
          DBG("Using latest BiosVersion from config\n");
        }
      } else {
        gSettings.RomVersion = Prop->getString()->stringValue();
        DBG("Using latest BiosVersion from config\n");
      }
    }
  } else {
    DBG("BiosVersion: not set, Using BiosVersion from clover\n");
  }
  DBG("BiosVersion: %s\n", gSettings.RomVersion.c_str());

  Prop1 = DictPointer->propertyForKey("BiosReleaseDate");
  if (Prop1 != NULL) {
    if ( !Prop1->isString() ) {
      MsgLog("ATTENTION : property not string in BiosReleaseDate\n");
    }else{
      if (Prop != NULL) {
        const CHAR8* i = gSettings.ReleaseDate.c_str();
        const CHAR8* j = Prop1->getString()->stringValue().c_str();

        if ((AsciiStrLen(i) == 8) && (AsciiStrLen(j) == 8)) {
          if (((i[6] > '0') && (j[6] == '0')) || ((i[6] >= j[6]) && (i[7] > j[7]))) {
            //DBG("Found old BiosReleaseDate from config\n");
            //DBG("Using latest BiosReleaseDate from clover\n");
          } else if ((i[6] == j[6]) && (i[7] == j[7])) {
            if (((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1]))) {
              //DBG("Found old BiosReleaseDate from config\n");
              //DBG("Using latest BiosReleaseDate from clover\n");
            } else if ((i[0] == j[0]) && (i[1] == j[1])) {
              if (((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) ||
                  ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
                //DBG("Found old BiosReleaseDate from config\n");
                //DBG("Using latest BiosReleaseDate from clover\n");
              } else if ((i[3] == j[3]) && (i[4] == j[4])) {
                //DBG("Found same BiosReleaseDate in clover and config\n");
              } else {
                gSettings.ReleaseDate = Prop1->getString()->stringValue();
                //DBG("Using latest BiosReleaseDate from config\n");
              }
            } else {
              gSettings.ReleaseDate = Prop1->getString()->stringValue();
              //DBG("Using latest BiosReleaseDate from config\n");
            }
          } else {
            gSettings.ReleaseDate = Prop1->getString()->stringValue();
            //DBG("Using latest BiosReleaseDate from config\n");
          }
        } else if ((AsciiStrLen(i) == 8) && (AsciiStrLen(j) == 10)) {
          if (((i[6] > '0') && (j[8] == '0')) || ((i[6] >= j[8]) && (i[7] > j[9]))) {
            //DBG("Found old BiosReleaseDate from config\n");
            //DBG("Using latest BiosReleaseDate from clover\n");
          } else if ((i[6] == j[8]) && (i[7] == j[9])) {
            if (((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1]))) {
              //DBG("Found old BiosReleaseDate from config\n");
              //DBG("Using latest BiosReleaseDate from clover\n");
            } else if ((i[0] == j[0]) && (i[1] == j[1])) {
              if (((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) ||
                  ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
                //DBG("Found old BiosReleaseDate from config\n");
                //DBG("Using latest BiosReleaseDate from clover\n");
              } else if ((i[3] == j[3]) && (i[4] == j[4])) {
                //DBG("Found same BiosReleaseDate in clover and config\n");
              } else {
                gSettings.ReleaseDate.S8Printf("%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
                //DBG("Using latest BiosReleaseDate from config\n");
              }
            } else {
              gSettings.ReleaseDate.S8Printf("%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
              //DBG("Using latest BiosReleaseDate from config\n");
            }
          } else {
            gSettings.ReleaseDate.S8Printf("%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
            //DBG("Using latest BiosReleaseDate from config\n");
          }
        } else if ((AsciiStrLen(i) == 10) && (AsciiStrLen(j) == 10)) {
          if (((i[8] > '0') && (j[8] == '0')) || ((i[8] >= j[8]) && (i[9] > j[9]))) {
            //DBG("Found old BiosReleaseDate from config\n");
            //DBG("Using latest BiosReleaseDate from clover\n");
          } else if ((i[8] == j[8]) && (i[9] == j[9])) {
            if (((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1]))) {
              //DBG("Found old BiosReleaseDate from config\n");
              //DBG("Using latest BiosReleaseDate from clover\n");
            } else if ((i[0] == j[0]) && (i[1] == j[1])) {
              if (((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) ||
                ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
                //DBG("Found old BiosReleaseDate from config\n");
                //DBG("Using latest BiosReleaseDate from clover\n");
              } else if ((i[3] == j[3]) && (i[4] == j[4])) {
                //DBG("Found same BiosReleaseDate in clover and config\n");
              } else {
                gSettings.ReleaseDate = Prop1->getString()->stringValue();
                //DBG("Using latest BiosReleaseDate from config\n");
              }
            } else {
              gSettings.ReleaseDate = Prop1->getString()->stringValue();
              //DBG("Using latest BiosReleaseDate from config\n");
            }
          } else {
            gSettings.ReleaseDate = Prop1->getString()->stringValue();
            //DBG("Using latest BiosReleaseDate from config\n");
          }
        } else if ((AsciiStrLen(i) == 10) && (AsciiStrLen(j) == 8)) {
          if (((i[8] > '0') && (j[6] == '0')) || ((i[8] >= j[6]) && (i[9] > j[7]))) {
            //DBG("Found old BiosReleaseDate from config\n");
            //DBG("Using latest BiosReleaseDate from clover\n");
          } else if ((i[8] == j[6]) && (i[9] == j[7])) {
            if (((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1]))) {
              //DBG("Found old BiosReleaseDate from config\n");
              //DBG("Using latest BiosReleaseDate from clover\n");
            } else if ((i[0] == j[0]) && (i[1] == j[1])) {
              if (((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) ||
                  ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
                //DBG("Found old BiosReleaseDate from config\n");
                //DBG("Using latest BiosReleaseDate from clover\n");
              } else if ((i[3] == j[3]) && (i[4] == j[4])) {
                //DBG("Found same BiosReleaseDate in clover and config\n");
              } else {
                gSettings.ReleaseDate.S8Printf("%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
                //DBG("Using latest BiosReleaseDate from config\n");
              }
            } else {
              gSettings.ReleaseDate.S8Printf("%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
              //DBG("Using latest BiosReleaseDate from config\n");
            }
          } else {
            gSettings.ReleaseDate.S8Printf("%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
            //DBG("Using latest BiosReleaseDate from config\n");
          }
        } else {
          //DBG("Found unknown date format from config\n");
          if (Prop != NULL) {
            i = gSettings.ReleaseDate.c_str();
            j = gSettings.RomVersion.c_str();

            j += AsciiStrLen(j);
            while (*j != '.') {
              j--;
            }

            if ((AsciiStrLen(i) == 8)) {
              gSettings.ReleaseDate.S8Printf("%c%c/%c%c/%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
              //DBG("Using the date of used BiosVersion\n");
            } else if ((AsciiStrLen(i) == 10)) {
              gSettings.ReleaseDate.S8Printf("%c%c/%c%c/20%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
              //DBG("Using the date of used BiosVersion\n");
            }
          } else {
            //DBG("Using BiosReleaseDate from clover\n");
          }
        }
      } else {
        //DBG("BiosReleaseDate: set to %s from config, Ignore BiosReleaseDate\n", Prop1->getString()->stringValue().c_str());
        //DBG("Using BiosReleaseDate from clover\n");
      }
    }
  } else {
    if (Prop != NULL) {
      const CHAR8* i = gSettings.ReleaseDate.c_str();
      const CHAR8* j = gSettings.RomVersion.c_str();

      j += AsciiStrLen(j);
      while (*j != '.') {
        j--;
      }

      if ((AsciiStrLen(i) == 8)) {
        gSettings.ReleaseDate.S8Printf("%c%c/%c%c/%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
        //DBG("BiosReleaseDate: not set, Using the date of used BiosVersion\n");
      } else if ((AsciiStrLen(i) == 10)) {
        gSettings.ReleaseDate.S8Printf("%c%c/%c%c/20%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
        //DBG("BiosReleaseDate: not set, Using the date of used BiosVersion\n");
      }
    } else {
      //DBG("BiosReleaseDate: not set, Using BiosReleaseDate from clover\n");
    }
  }
  DBG("BiosReleaseDate: %s\n", gSettings.ReleaseDate.c_str());

  Prop = DictPointer->propertyForKey("EfiVersion");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in EfiVersion\n");
      if ( gSettings.EfiVersion.notEmpty() ) {
        DBG("Using EfiVersion from clover: %s\n", gSettings.EfiVersion.c_str());
      }
    }else{
      if (AsciiStrVersionToUint64(gSettings.EfiVersion, 4, 5) > AsciiStrVersionToUint64(Prop->getString()->stringValue(), 4, 5)) {
        DBG("Using latest EfiVersion from clover: %s\n", gSettings.EfiVersion.c_str());
      } else if (AsciiStrVersionToUint64(gSettings.EfiVersion, 4, 5) < AsciiStrVersionToUint64(Prop->getString()->stringValue(), 4, 5)) {
        gSettings.EfiVersion = Prop->getString()->stringValue();
        gSettings.EfiVersion.trim();
        DBG("Using latest EfiVersion from config: %s\n", gSettings.EfiVersion.c_str());
      } else {
        DBG("Using EfiVersion from clover: %s\n", gSettings.EfiVersion.c_str());
      }
    }
  } else if ( gSettings.EfiVersion.notEmpty() ) {
    DBG("Using EfiVersion from clover: %s\n", gSettings.EfiVersion.c_str());
  }

  Prop = DictPointer->propertyForKey("FirmwareFeatures");
  if (Prop != NULL) {
    gFwFeatures = (UINT32)GetPropertyAsInteger(Prop, gFwFeatures);
    DBG("Using FirmwareFeatures from config: 0x%08X\n", gFwFeatures);
  } else {
    DBG("Using FirmwareFeatures from clover: 0x%08X\n", gFwFeatures);
  }

  Prop = DictPointer->propertyForKey("FirmwareFeaturesMask");
  if (Prop != NULL) {
    gFwFeaturesMask = (UINT32)GetPropertyAsInteger(Prop, gFwFeaturesMask);
    DBG("Using FirmwareFeaturesMask from config: 0x%08X\n", gFwFeaturesMask);
  } else {
    DBG("Using FirmwareFeaturesMask from clover: 0x%08X\n", gFwFeaturesMask);
  }

  Prop = DictPointer->propertyForKey("PlatformFeature");
  if (Prop != NULL) {
    gPlatformFeature = (UINT64)GetPropertyAsInteger(Prop, (INTN)gPlatformFeature);
  } else {
    if (gPlatformFeature == 0xFFFF) {
      DBG("PlatformFeature will not set in SMBIOS\n");
    } else {
		DBG("Using PlatformFeature from clover: 0x%llX\n", gPlatformFeature);
    }
  }

  Prop = DictPointer->propertyForKey("BiosVendor");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in BiosVendor\n");
    }else{
      gSettings.VendorName = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("Manufacturer");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in Manufacturer\n");
    }else{
      gSettings.ManufactureName = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("Version");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in Version\n");
    }else{
      gSettings.VersionNr = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("Family");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in Family\n");
    }else{
      gSettings.FamilyName = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("SerialNumber");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in SerialNumber\n");
    }else{
      gSettings.SerialNr = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("SmUUID");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in SmUUID\n");
    }else{
      if (IsValidGuidAsciiString(Prop->getString()->stringValue())) {
        StrToGuidLE(Prop->getString()->stringValue(), &gSettings.SmUUID);
        gSettings.SmUUIDConfig = TRUE;
      } else {
        DBG("Error: invalid SmUUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->getString()->stringValue().c_str());
      }
    }
  }

  Prop = DictPointer->propertyForKey("BoardManufacturer");
  if ( Prop != NULL ) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in BoardManufacturer\n");
    }else{
      if( Prop->getString()->stringValue().notEmpty() ) {
        gSettings.BoardManufactureName = Prop->getString()->stringValue();
      }
    }
  }

  Prop = DictPointer->propertyForKey("BoardSerialNumber");
  if ( Prop != NULL ) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in BoardSerialNumber\n");
    }else{
      if( Prop->getString()->stringValue().notEmpty() ) {
        gSettings.BoardSerialNumber = Prop->getString()->stringValue();
      }
    }
  }

  Prop = DictPointer->propertyForKey("Board-ID");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in Board-ID\n");
    }else{
      gSettings.BoardNumber = Prop->getString()->stringValue();
      DBG("Board-ID set from config as %s\n", gSettings.BoardNumber.c_str());
    }
  }

  if (!Default) {
    gSettings.BoardVersion = gSettings.ProductName;
  }
  Prop = DictPointer->propertyForKey("BoardVersion");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in BoardVersion\n");
    }else{
      gSettings.BoardVersion = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("BoardType");
  if (Prop != NULL) {
    gSettings.BoardType = (UINT8)GetPropertyAsInteger(Prop, gSettings.BoardType);
    DBG("BoardType: 0x%hhX\n", gSettings.BoardType);
  }

  Prop = DictPointer->propertyForKey("Mobile");
  if (Prop != NULL) {
    if (IsPropertyNotNullAndFalse(Prop))
      gSettings.Mobile = FALSE;
    else if (IsPropertyNotNullAndTrue(Prop))
      gSettings.Mobile = TRUE;
  } else if (!Default) {
    gSettings.Mobile = gSettings.ProductName.contains("MacBook");
  }

  Prop = DictPointer->propertyForKey("LocationInChassis");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in LocationInChassis\n");
    }else{
      gSettings.LocationInChassis = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("ChassisManufacturer");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in ChassisManufacturer\n");
    }else{
      gSettings.ChassisManufacturer = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("ChassisAssetTag");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in ChassisAssetTag\n");
    }else{
      gSettings.ChassisAssetTag = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("ChassisType");
  if (Prop != NULL) {
    gSettings.ChassisType = (UINT8)GetPropertyAsInteger(Prop, gSettings.ChassisType);
    DBG("ChassisType: 0x%hhX\n", gSettings.ChassisType);
  }

  Prop = DictPointer->propertyForKey("NoRomInfo");
  if (Prop != NULL) {
    gSettings.NoRomInfo = IsPropertyNotNullAndTrue(Prop);
  }
}

static void getACPISettings(const TagDict *CfgDict)
{
  const TagDict* ACPIDict = CfgDict->dictPropertyForKey("ACPI");
  if (ACPIDict) {
    const TagArray* DropTablesArray = ACPIDict->arrayPropertyForKey("DropTables"); // array of dict
    if (DropTablesArray) {
      INTN   i;
      INTN Count = DropTablesArray->arrayContent().size();
      BOOLEAN Dropped;
      
      if (Count > 0) {
        DBG("Dropping %lld tables:\n", Count);
        
        for (i = 0; i < Count; i++) {
          UINT32 Signature = 0;
          UINT32 TabLength = 0;
          UINT64 TableId = 0;
          BOOLEAN OtherOS = FALSE;
          
          const TagDict* Dict2 = DropTablesArray->dictElementAt(i, "ACPI/DropTables"_XS8);
          
          DBG(" - [%02lld]: Drop table ", i);
          // Get the table signatures to drop
          const TagStruct* Prop2 = Dict2->propertyForKey("Signature");
          if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
            CHAR8  s1 = 0, s2 = 0, s3 = 0, s4 = 0;
            const CHAR8 *str = Prop2->getString()->stringValue().c_str();
            if (*str) {
              s1 = *str++;
            }
            if (*str) {
              s2 = *str++;
            }
            if (*str) {
              s3 = *str++;
            }
            if (*str) {
              s4 = *str++;
            }
            Signature = SIGNATURE_32(s1, s2, s3, s4);
            DBG(" signature=\"%c%c%c%c\" (%8.8X)\n", s1, s2, s3, s4, Signature);
          }
          // Get the table ids to drop
          Prop2 = Dict2->propertyForKey("TableId");
          if (Prop2 != NULL) {
            if ( !Prop2->isString() ) {
              MsgLog("ATTENTION : property not string in TableId\n");
            }else{
              UINTN  IdIndex = 0;
              CHAR8  Id[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
              const CHAR8 *Str = Prop2->getString()->stringValue().c_str();
              if (Str) {
                while (*Str && (IdIndex < 8)) {
                  //      DBG("%c", *Str);
                  Id[IdIndex++] = *Str++;
                }
              }
              
              CopyMem(&TableId, (CHAR8*)&Id[0], 8);
              DBG(" table-id=\"%s\" (%16.16llX)\n", Id, TableId);
            }
          }
          // Get the table len to drop
          Prop2 = Dict2->propertyForKey("Length");
          if (Prop2 != NULL) {
            TabLength = (UINT32)GetPropertyAsInteger(Prop2, 0);
            DBG(" length=%d(0x%X)", TabLength, TabLength);
          }
          // Check if to drop for other OS as well
          Prop2 = Dict2->propertyForKey("DropForAllOS");
          if (Prop2 != NULL) {
            OtherOS = IsPropertyNotNullAndTrue(Prop2);
          }
          
          DBG("----\n");
          //set to drop
          if (gSettings.ACPIDropTables) {
            ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
            DBG("         - set table: %08X, %16llx to drop:", Signature, TableId);
            Dropped = FALSE;
            while (DropTable) {
              if (((Signature == DropTable->Signature) &&
                   (!TableId || (DropTable->TableId == TableId)) &&
                   (!TabLength || (DropTable->Length == TabLength))) ||
                  (!Signature && (DropTable->TableId == TableId))) {
                DropTable->MenuItem.BValue = TRUE;
                DropTable->OtherOS = OtherOS;
                gSettings.DropSSDT         = FALSE; //if one item=true then dropAll=false by default
                //DBG(" true");
                Dropped = TRUE;
              }
              DropTable = DropTable->Next;
            }
            DBG(" %s\n", Dropped ? "yes" : "no");
          }
        }
      }
    }
    
    const TagDict* DSDTDict = ACPIDict->dictPropertyForKey("DSDT");
    if (DSDTDict) {
      //gSettings.DsdtName by default is "DSDT.aml", but name "BIOS" will mean autopatch
      const TagStruct* Prop = DSDTDict->propertyForKey("Name");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in DSDT/Name\n");
        }else{
          gSettings.DsdtName = Prop->getString()->stringValue();
        }
      }
      
      Prop = DSDTDict->propertyForKey("Debug");
      gSettings.DebugDSDT = IsPropertyNotNullAndTrue(Prop);
      
      Prop = DSDTDict->propertyForKey("Rtc8Allowed");
      gSettings.Rtc8Allowed = IsPropertyNotNullAndTrue(Prop);
      
      Prop = DSDTDict->propertyForKey("PNLF_UID");
      gSettings.PNLF_UID = (UINT8)GetPropertyAsInteger(Prop, 0x0A);
      
      Prop = DSDTDict->propertyForKey("FixMask");
      gSettings.FixDsdt = (UINT32)GetPropertyAsInteger(Prop, gSettings.FixDsdt);
      
      const TagDict* FixesDict = DSDTDict->dictPropertyForKey("Fixes");
      if (FixesDict != NULL) {
        UINTN Index;
        //         DBG("Fixes will override DSDT fix mask %08X!\n", gSettings.FixDsdt);
        gSettings.FixDsdt = 0;
        for (Index = 0; Index < sizeof(FixesConfig)/sizeof(FixesConfig[0]); Index++) {
          const TagStruct* Prop2 = FixesDict->propertyForKey(FixesConfig[Index].newName);
          if (!Prop2 && FixesConfig[Index].oldName) {
            Prop2 = FixesDict->propertyForKey(FixesConfig[Index].oldName);
          }
          if (IsPropertyNotNullAndTrue(Prop2)) {
            gSettings.FixDsdt |= FixesConfig[Index].bitData;
          }
        }
      }
      DBG(" - final DSDT Fix mask=%08X\n", gSettings.FixDsdt);
      
      const TagArray* PatchesArray = DSDTDict->arrayPropertyForKey("Patches"); // array of dict
      if (PatchesArray != NULL) {
        INTN   i;
        INTN Count = PatchesArray->arrayContent().size();
        if (Count > 0) {
          gSettings.PatchDsdtNum      = (UINT32)Count;
          gSettings.PatchDsdtFind = (__typeof__(gSettings.PatchDsdtFind))AllocateZeroPool(Count * sizeof(UINT8*));
          gSettings.PatchDsdtReplace = (__typeof__(gSettings.PatchDsdtReplace))AllocateZeroPool(Count * sizeof(UINT8*));
          gSettings.PatchDsdtTgt = (__typeof__(gSettings.PatchDsdtTgt))AllocateZeroPool(Count * sizeof(UINT8*));
          gSettings.LenToFind = (__typeof__(gSettings.LenToFind))AllocateZeroPool(Count * sizeof(UINT32));
          gSettings.LenToReplace = (__typeof__(gSettings.LenToReplace))AllocateZeroPool(Count * sizeof(UINT32));
          gSettings.PatchDsdtLabel = (__typeof__(gSettings.PatchDsdtLabel))AllocateZeroPool(Count * sizeof(UINT8*));
          gSettings.PatchDsdtMenuItem = new INPUT_ITEM[Count];
          DBG("PatchesDSDT: %lld requested\n", Count);
          
          for (i = 0; i < Count; i++) {
            UINTN Size = 0;
            const TagDict* Prop2 = PatchesArray->dictElementAt(i,"DSDT/Patches"_XS8);
            DBG(" - [%02lld]:", i);
            XString8 DSDTPatchesLabel;
            
            const TagStruct* Prop3 = Prop2->propertyForKey("Comment");
            if (Prop3 != NULL && (Prop3->isString()) && Prop3->getString()->stringValue().notEmpty()) {
              DSDTPatchesLabel = Prop3->getString()->stringValue();
            } else {
              DSDTPatchesLabel = " (NoLabel)"_XS8;
            }
            gSettings.PatchDsdtLabel[i] = (__typeof_am__(gSettings.PatchDsdtLabel[i]))AllocateZeroPool(256);
            snprintf(gSettings.PatchDsdtLabel[i], 255, "%s", DSDTPatchesLabel.c_str());
            DBG(" (%s)", gSettings.PatchDsdtLabel[i]);
            
            Prop3 = Prop2->propertyForKey("Disabled");
            gSettings.PatchDsdtMenuItem[i].BValue = !IsPropertyNotNullAndTrue(Prop3);
            
            //DBG(" DSDT bin patch #%d ", i);
            gSettings.PatchDsdtFind[i]    = GetDataSetting (Prop2, "Find",     &Size);
            DBG(" lenToFind: %llu", Size);
            gSettings.LenToFind[i]        = (UINT32)Size;
            gSettings.PatchDsdtReplace[i] = GetDataSetting (Prop2, "Replace",  &Size);
            DBG(", lenToReplace: %llu", Size);
            gSettings.LenToReplace[i]     = (UINT32)Size;
            gSettings.PatchDsdtTgt[i]     = (CHAR8*)GetDataSetting (Prop2, "TgtBridge", &Size);
            DBG(", Target Bridge: %s\n", gSettings.PatchDsdtTgt[i]);
            if (!gSettings.PatchDsdtMenuItem[i].BValue) {
              DBG("  patch disabled at config\n");
            }
          }
        } //if count > 0
      } //if prop PatchesDSDT
      
      Prop = DSDTDict->propertyForKey("ReuseFFFF");
      if (IsPropertyNotNullAndTrue(Prop)) {
        gSettings.ReuseFFFF = TRUE;
      }
      
      Prop = DSDTDict->propertyForKey("SuspendOverride");
      if (IsPropertyNotNullAndTrue(Prop)) {
        gSettings.SuspendOverride = TRUE;
      }
      /*
       Prop   = GetProperty(Dict2, "DropOEM_DSM");
       defDSM = FALSE;
       
       if (Prop != NULL) {
       defDSM = TRUE; //set by user
       if (IsPropertyTrue(Prop)) {
       gSettings.DropOEM_DSM = 0xFFFF;
       } else if (IsPropertyFalse(Prop)) {
       gSettings.DropOEM_DSM = 0;
       } else if (Prop->isInt()) {
       gSettings.DropOEM_DSM = (UINT16)(UINTN)Prop->??;
       } else if (Prop->type == kTagTypeDict) {
       Prop2 = GetProperty(Prop, "ATI");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_ATI;
       }
       
       Prop2 = GetProperty(Prop, "NVidia");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_NVIDIA;
       }
       
       Prop2 = GetProperty(Prop, "IntelGFX");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_INTEL;
       }
       
       Prop2 = GetProperty(Prop, "HDA");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_HDA;
       }
       
       Prop2 = GetProperty(Prop, "HDMI");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_HDMI;
       }
       
       Prop2 = GetProperty(Prop, "SATA");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_SATA;
       }
       
       Prop2 = GetProperty(Prop, "LAN");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_LAN;
       }
       
       Prop2 = GetProperty(Prop, "WIFI");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_WIFI;
       }
       
       Prop2 = GetProperty(Prop, "USB");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_USB;
       }
       
       Prop2 = GetProperty(Prop, "LPC");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_LPC;
       }
       
       Prop2 = GetProperty(Prop, "SmBUS");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_SMBUS;
       }
       
       Prop2 = GetProperty(Prop, "Firewire");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_FIREWIRE;
       }
       
       Prop2 = GetProperty(Prop, "IDE");
       if (IsPropertyTrue(Prop2)) {
       gSettings.DropOEM_DSM |= DEV_IDE;
       }
       }
       }
       */
    }
    
    const TagDict* SSDTDict = ACPIDict->dictPropertyForKey("SSDT");
    if (SSDTDict) {
      const TagStruct* Prop2 = SSDTDict->propertyForKey("Generate");
      if (Prop2 != NULL) {
        if (IsPropertyNotNullAndTrue(Prop2)) {
          gSettings.GeneratePStates = TRUE;
          gSettings.GenerateCStates = TRUE;
          gSettings.GenerateAPSN = TRUE;
          gSettings.GenerateAPLF = TRUE;
          gSettings.GeneratePluginType = TRUE;
          
        } else if (IsPropertyNotNullAndFalse(Prop2)) {
          gSettings.GeneratePStates = FALSE;
          gSettings.GenerateCStates = FALSE;
          gSettings.GenerateAPSN = FALSE;
          gSettings.GenerateAPLF = FALSE;
          gSettings.GeneratePluginType = FALSE;
          
        } else if (Prop2->isDict()) {
          const TagStruct* Prop = Prop2->getDict()->propertyForKey("PStates");
          gSettings.GeneratePStates = IsPropertyNotNullAndTrue(Prop);
          gSettings.GenerateAPSN = gSettings.GeneratePStates;
          gSettings.GenerateAPLF = gSettings.GeneratePStates;
          gSettings.GeneratePluginType = gSettings.GeneratePStates;
          Prop = Prop2->getDict()->propertyForKey("CStates");
          gSettings.GenerateCStates = IsPropertyNotNullAndTrue(Prop);
          Prop = Prop2->getDict()->propertyForKey("APSN");
          if (Prop) {
            gSettings.GenerateAPSN = IsPropertyNotNullAndTrue(Prop);
          }
          Prop = Prop2->getDict()->propertyForKey("APLF");
          if (Prop) {
            gSettings.GenerateAPLF = IsPropertyNotNullAndTrue(Prop);
          }
          Prop = Prop2->getDict()->propertyForKey("PluginType");
          if (Prop) {
            gSettings.GeneratePluginType = IsPropertyNotNullAndTrue(Prop);
          }
        }
      }
      
      const TagStruct* Prop = SSDTDict->propertyForKey("DropOem");
      gSettings.DropSSDT  = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("NoOemTableId"); // to disable OEM table ID on ACPI/orgin/SSDT file names
      gSettings.NoOemTableId = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("NoDynamicExtract"); // to disable extracting child SSDTs
      gSettings.NoDynamicExtract = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("UseSystemIO");
      gSettings.EnableISS = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("EnableC7");
      if (Prop != NULL) {
        gSettings.EnableC7 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC7: %s\n", gSettings.EnableC7 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC6");
      if (Prop != NULL) {
        gSettings.EnableC6 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC6: %s\n", gSettings.EnableC6 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC4");
      if (Prop != NULL) {
        gSettings.EnableC4 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC4: %s\n", gSettings.EnableC4 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC2");
      if (Prop != NULL) {
        gSettings.EnableC2 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC2: %s\n", gSettings.EnableC2 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("C3Latency");
      if (Prop != NULL) {
        gSettings.C3Latency = (UINT16)GetPropertyAsInteger(Prop, gSettings.C3Latency);
        DBG("C3Latency: %d\n", gSettings.C3Latency);
      }
      
      Prop                       = SSDTDict->propertyForKey("PLimitDict");
      gSettings.PLimitDict       = (UINT8)GetPropertyAsInteger(Prop, 0);
      
      Prop                       = SSDTDict->propertyForKey("UnderVoltStep");
      gSettings.UnderVoltStep    = (UINT8)GetPropertyAsInteger(Prop, 0);
      
      Prop                       = SSDTDict->propertyForKey("DoubleFirstState");
      gSettings.DoubleFirstState = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("MinMultiplier");
      if (Prop != NULL) {
        gSettings.MinMultiplier  = (UINT8)GetPropertyAsInteger(Prop, gSettings.MinMultiplier);
        DBG("MinMultiplier: %d\n", gSettings.MinMultiplier);
      }
      
      Prop = SSDTDict->propertyForKey("MaxMultiplier");
      if (Prop != NULL) {
        gSettings.MaxMultiplier = (UINT8)GetPropertyAsInteger(Prop, gSettings.MaxMultiplier);
        DBG("MaxMultiplier: %d\n", gSettings.MaxMultiplier);
      }
      
      Prop = SSDTDict->propertyForKey("PluginType");
      if (Prop != NULL) {
        gSettings.PluginType = (UINT8)GetPropertyAsInteger(Prop, gSettings.PluginType);
        DBG("PluginType: %d\n", gSettings.PluginType);
      }
    }
    
    //     Prop               = GetProperty(DictPointer, "DropMCFG");
    //     gSettings.DropMCFG = IsPropertyTrue(Prop);
    
    const TagStruct* Prop = ACPIDict->propertyForKey("ResetAddress");
    if (Prop) {
      gSettings.ResetAddr = (UINT32)GetPropertyAsInteger(Prop, 0x64);
      DBG("ResetAddr: 0x%llX\n", gSettings.ResetAddr);
      
      if (gSettings.ResetAddr  == 0x64) {
        gSettings.ResetVal = 0xFE;
      } else if  (gSettings.ResetAddr  == 0xCF9) {
        gSettings.ResetVal = 0x06;
      }
      
      DBG("Calc ResetVal: 0x%hhX\n", gSettings.ResetVal);
    }
    
    Prop = ACPIDict->propertyForKey("ResetValue");
    if (Prop) {
      gSettings.ResetVal = (UINT8)GetPropertyAsInteger(Prop, gSettings.ResetVal);
      DBG("ResetVal: 0x%hhX\n", gSettings.ResetVal);
    }
    //other known pair is 0x0CF9/0x06. What about 0x92/0x01 ?
    
    Prop = ACPIDict->propertyForKey("HaltEnabler");
    gSettings.SlpSmiEnable = IsPropertyNotNullAndTrue(Prop);
    
    //
    Prop = ACPIDict->propertyForKey("FixHeaders");
    gSettings.FixHeaders = IsPropertyNotNullAndTrue(Prop);
    
    Prop = ACPIDict->propertyForKey("FixMCFG");
    gSettings.FixMCFG = IsPropertyNotNullAndTrue(Prop);
    
    
    Prop = ACPIDict->propertyForKey("DisableASPM");
    gSettings.NoASPM = IsPropertyNotNullAndTrue(Prop);
    
    Prop = ACPIDict->propertyForKey("smartUPS");
    if (Prop) {
      gSettings.smartUPS   = IsPropertyNotNullAndTrue(Prop);
      DBG("smartUPS: present\n");
    }
    
    Prop               = ACPIDict->propertyForKey("PatchAPIC");
    gSettings.PatchNMI = IsPropertyNotNullAndTrue(Prop);
    
    const TagArray* SortedOrderArray = ACPIDict->arrayPropertyForKey("SortedOrder"); // array of string
    if (Prop) {
      INTN   i;
      INTN Count = SortedOrderArray->arrayContent().size();
      const TagStruct* Prop2 = NULL;
      if (Count > 0) {
        gSettings.SortedACPICount = 0;
        gSettings.SortedACPI = (__typeof__(gSettings.SortedACPI))AllocateZeroPool(Count * sizeof(CHAR16 *));
        
        for (i = 0; i < Count; i++) {
          Prop2 = &SortedOrderArray->arrayContent()[i];
          if ( !Prop2->isString()) {
            MsgLog("MALFORMED PLIST : SortedOrder must be an array of string");
            continue;
          }
          gSettings.SortedACPI[gSettings.SortedACPICount++] = SWPrintf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
        }
      }
    }
    
    Prop = ACPIDict->propertyForKey("AutoMerge");
    gSettings.AutoMerge  = IsPropertyNotNullAndTrue(Prop);
    
    const TagArray* DisabledAMLArray = ACPIDict->arrayPropertyForKey("DisabledAML"); // array of string
    if (DisabledAMLArray) {
      INTN   i;
      INTN Count = DisabledAMLArray->arrayContent().size();
      const TagStruct* Prop2 = NULL;
      if (Count > 0) {
        gSettings.DisabledAMLCount = 0;
        gSettings.DisabledAML = (__typeof__(gSettings.DisabledAML))AllocateZeroPool(Count * sizeof(CHAR16 *));
        
        if (gSettings.DisabledAML) {
          for (i = 0; i < Count; i++) {
            Prop2 = &DisabledAMLArray->arrayContent()[i];
            if ( !Prop2->isString()) {
              MsgLog("MALFORMED PLIST : DisabledAML must be an array of string");
              continue;
            }
            gSettings.DisabledAML[gSettings.DisabledAMLCount++] = SWPrintf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
          }
        }
      }
    }
    
    const TagDict* RenameDevicesDict = ACPIDict->dictPropertyForKey("RenameDevices"); // dict of key/string
    if ( RenameDevicesDict ) {
      INTN   i;
      INTN Count = RenameDevicesDict->dictKeyCount();
      if (Count > 0) {
        gSettings.DeviceRenameCount = 0;
        gSettings.DeviceRename = (__typeof__(gSettings.DeviceRename))AllocateZeroPool(Count * sizeof(ACPI_NAME_LIST));
        DBG("Devices to rename %lld\n", Count);
        for (i = 0; i < Count; i++) {
          const TagKey* key;
          const TagStruct* value;
          if ( !EFI_ERROR(RenameDevicesDict->getKeyAndValueAtIndex(i, &key, &value)) ) {
            ACPI_NAME_LIST *List = ParseACPIName(key->keyStringValue());
            gSettings.DeviceRename[gSettings.DeviceRenameCount].Next = List;
            while (List) {
              DBG("%s:", List->Name);
              List = List->Next;
            }
            if (value->isString()) {
              gSettings.DeviceRename[gSettings.DeviceRenameCount++].Name = S8Printf("%s", value->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
              DBG("->will be renamed to %s\n", value->getString()->stringValue().c_str());
            }
          }
        }
      }
    }
  }
}

EFI_STATUS
GetUserSettings(const TagDict* CfgDict)
{
  EFI_STATUS Status = EFI_NOT_FOUND;

  if (CfgDict != NULL) {
    DbgHeader ("GetUserSettings");

    // Boot settings.
    // Discussion. Why Arguments is here? It should be SystemParameters property!
    // we will read them again because of change in GUI menu. It is not only EarlySettings
    //
    const TagDict* BootDict = CfgDict->dictPropertyForKey("Boot");
    if (BootDict != NULL) {

      const TagStruct* Prop = BootDict->propertyForKey("Arguments");
      if ( Prop != NULL  &&  Prop->isString()  &&  Prop->getString()->stringValue().notEmpty()  &&  !gSettings.BootArgs.contains(Prop->getString()->stringValue()) ) {
        gSettings.BootArgs = Prop->getString()->stringValue();
        //gBootArgsChanged = TRUE;
        //gBootChanged = TRUE;
      }

      Prop                     = BootDict->propertyForKey("NeverDoRecovery");
      gSettings.NeverDoRecovery  = IsPropertyNotNullAndTrue(Prop);
    }


    //Graphics

    const TagDict* GraphicsDict = CfgDict->dictPropertyForKey("Graphics");
    if (GraphicsDict != NULL) {
      INTN i;
      const TagStruct* Prop     = GraphicsDict->propertyForKey("Inject");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          gSettings.GraphicsInjector = TRUE;
          gSettings.InjectIntel      = TRUE;
          gSettings.InjectATI        = TRUE;
          gSettings.InjectNVidia     = TRUE;
        } else if (Prop->isDict()) {
          const TagDict* Dict2 = Prop->getDict();
          const TagStruct* Prop2 = Dict2->propertyForKey("Intel");
          if (Prop2 != NULL) {
            gSettings.InjectIntel = IsPropertyNotNullAndTrue(Prop2);
          }

          Prop2 = Dict2->propertyForKey("ATI");
          if (Prop2 != NULL) {
            gSettings.InjectATI = IsPropertyNotNullAndTrue(Prop2);
          }

          Prop2 = Dict2->propertyForKey("NVidia");
          if (Prop2 != NULL) {
            gSettings.InjectNVidia = IsPropertyNotNullAndTrue(Prop2);
          }
        } else {
          gSettings.GraphicsInjector = FALSE;
          gSettings.InjectIntel      = FALSE;
          gSettings.InjectATI        = FALSE;
          gSettings.InjectNVidia     = FALSE;
        }
      }

      Prop = GraphicsDict->propertyForKey("RadeonDeInit");
      gSettings.DeInit = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("VRAM");
      gSettings.VRAM = (UINTN)GetPropertyAsInteger(Prop, (INTN)gSettings.VRAM); //Mb
      //
      Prop = GraphicsDict->propertyForKey("RefCLK");
      gSettings.RefCLK = (UINT16)GetPropertyAsInteger(Prop, 0);

      Prop = GraphicsDict->propertyForKey("LoadVBios");
      gSettings.LoadVBios = IsPropertyNotNullAndTrue(Prop);

      for (i = 0; i < (INTN)NGFX; i++) {
        gGraphics[i].LoadVBios = gSettings.LoadVBios; //default
      }

      Prop = GraphicsDict->propertyForKey("VideoPorts");
      gSettings.VideoPorts   = (UINT16)GetPropertyAsInteger(Prop, gSettings.VideoPorts);

      Prop = GraphicsDict->propertyForKey("BootDisplay");
      gSettings.BootDisplay = (INT8)GetPropertyAsInteger(Prop, -1);

      Prop = GraphicsDict->propertyForKey("FBName");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in FBName\n");
        }else{
          gSettings.FBName = Prop->getString()->stringValue();
        }
      }

      Prop = GraphicsDict->propertyForKey("NVCAP");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in NVCAP\n");
        }else{
          hex2bin (Prop->getString()->stringValue().c_str(), (UINT8*)&gSettings.NVCAP[0], 20);
          DBG("Read NVCAP:");

          for (i = 0; i<20; i++) {
            DBG("%02hhX", gSettings.NVCAP[i]);
          }

          DBG("\n");
          //thus confirmed this procedure is working
        }
      }

      Prop = GraphicsDict->propertyForKey("display-cfg");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in display-cfg\n");
        }else{
          hex2bin (Prop->getString()->stringValue().c_str(), (UINT8*)&gSettings.Dcfg[0], 8);
        }
      }

      Prop = GraphicsDict->propertyForKey("DualLink");
      gSettings.DualLink = (UINT32)GetPropertyAsInteger(Prop, gSettings.DualLink);

      //InjectEDID - already done in earlysettings
      //No! Take again
      GetEDIDSettings(GraphicsDict);

      // ErmaC: NvidiaGeneric
      Prop = GraphicsDict->propertyForKey("NvidiaGeneric");
      gSettings.NvidiaGeneric = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("NvidiaNoEFI");
      gSettings.NvidiaNoEFI = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("NvidiaSingle");
      gSettings.NvidiaSingle = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("ig-platform-id");
      gSettings.IgPlatform = (UINT32)GetPropertyAsInteger(Prop, gSettings.IgPlatform);

      Prop = GraphicsDict->propertyForKey("snb-platform-id");
      gSettings.IgPlatform = (UINT32)GetPropertyAsInteger(Prop, gSettings.IgPlatform);

      FillCardList(GraphicsDict); //#@ Getcardslist
    }

    const TagDict* DevicesDict = CfgDict->dictPropertyForKey("Devices");
    if (DevicesDict != NULL) {
      const TagStruct* Prop = DevicesDict->propertyForKey("Inject");
      gSettings.StringInjector = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("SetIntelBacklight");
      gSettings.IntelBacklight = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("SetIntelMaxBacklight");
      gSettings.IntelMaxBacklight = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("IntelMaxValue");
      gSettings.IntelMaxValue = (UINT16)GetPropertyAsInteger(Prop, gSettings.IntelMaxValue);

      /*
       * Properties is a single string, or a dict
       */
      Prop = DevicesDict->propertyForKey("Properties");
      if (Prop != NULL) {
        if (Prop->isString()) {

          EFI_PHYSICAL_ADDRESS  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
          cDeviceProperties = Prop->getString()->stringValue();
          //-------
          Status = gBS->AllocatePages (
                                       AllocateMaxAddress,
                                       EfiACPIReclaimMemory,
                                       EFI_SIZE_TO_PAGES (cDeviceProperties.sizeInBytes()) + 1,
                                       &BufferPtr
                                       );

          if (!EFI_ERROR(Status)) {
            cProperties = (UINT8*)(UINTN)BufferPtr;
            cPropSize   = (UINT32)(cDeviceProperties.length() >> 1);
            cPropSize = hex2bin(cDeviceProperties.c_str(), cProperties, cPropSize);
            DBG("Injected EFIString of length %d\n", cPropSize);
          }
          //---------
        }
        else if ( Prop->isDict() ) {
          INTN i;
          const TagDict* PropertiesDict = Prop->getDict();
          INTN Count = PropertiesDict->dictKeyCount(); //ok
          gSettings.AddProperties = new DEV_PROPERTY[Count];
          DEV_PROPERTY *DevPropDevice;
          DEV_PROPERTY *DevProps;
          DEV_PROPERTY **Child;

          if (Count > 0) {
			      DBG("Add %lld devices (kTagTypeDict):\n", Count);

            for (i = 0; i < Count; i++) {
              const TagKey* key;
              const TagStruct* value;
              EFI_DEVICE_PATH_PROTOCOL* DevicePath = NULL;

              if ( !EFI_ERROR(PropertiesDict->getKeyAndValueAtIndex(i, &key, &value)) ) {  //take a <key> with DevicePath. If GetKeyValueAtIndex return success, key and value != NULL
                XStringW DevicePathStr = key->keyStringValue();
                //         DBG("Device: %ls\n", DevicePathStr);

                // when key in Devices/Properties is one of the strings "PrimaryGPU" / "SecondaryGPU", use device path of first / second gpu accordingly
                if ( DevicePathStr.equalIC("PrimaryGPU") ) {
                  DevicePath = DevicePathFromHandle(gGraphics[0].Handle); // first gpu
                } else if ( DevicePathStr.equalIC("SecondaryGPU") && NGFX > 1) {
                  DevicePath = DevicePathFromHandle(gGraphics[1].Handle); // second gpu
                } else {
                  DevicePath = ConvertTextToDevicePath(DevicePathStr.wc_str()); //TODO
                }
                if (DevicePath == NULL) {
                  continue;
                }
                //Create Device node
                DevPropDevice = gSettings.ArbProperties;
                gSettings.ArbProperties = new DEV_PROPERTY;
                gSettings.ArbProperties->Next = DevPropDevice; //next device
                gSettings.ArbProperties->Child = NULL;
                gSettings.ArbProperties->Device = 0; //to differ from arbitrary
                gSettings.ArbProperties->DevicePath = DevicePath; //this is pointer
                gSettings.ArbProperties->Label = S8Printf("%s", key->keyStringValue().c_str()).forgetDataWithoutFreeing();
                Child = &(gSettings.ArbProperties->Child);

                if ((value != NULL) && (value->isDict())) {
                  INTN PropCount = 0;
                  const TagDict* valueDict = value->getDict();
                  PropCount = valueDict->dictKeyCount();
                  //         DBG("Add %d properties:\n", PropCount);
                  for (INTN j = 0; j < PropCount; j++) {
                    DevProps = *Child;
                    *Child = new DEV_PROPERTY;
                  //  *Child = new (__typeof_am__(**Child))();
                    (*Child)->Next = DevProps;

                    const TagKey* key2;
                    const TagStruct* value2;
                    if (EFI_ERROR(valueDict->getKeyAndValueAtIndex(j, &key2, &value2))) {
                      continue;
                    }
                    if (key2->keyStringValue()[0] != '#') {
                      (*Child)->MenuItem.BValue = TRUE;
                      (*Child)->Key = S8Printf("%s", key2->keyStringValue().c_str()).forgetDataWithoutFreeing();
                    }
                    else {
                      (*Child)->MenuItem.BValue = FALSE;
                      (*Child)->Key = S8Printf("%s", key2->keyStringValue().c_str() - 1).forgetDataWithoutFreeing();
                    }

                    //    DBG("<key>%s\n  <value> type %d\n", (*Child)->Key, Prop3->type);
                    if (value2 && (value2->isString()) && value2->getString()->stringValue().notEmpty()) {
                      //first suppose it is Ascii string
                      (*Child)->Value = (UINT8*)S8Printf("%s", value2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
                      (*Child)->ValueLen = value2->getString()->stringValue().sizeInBytesIncludingTerminator();
                      (*Child)->ValueType = kTagTypeString;
                    }
                    else if (value2 && (value2->isInt64())) {
                      if ( value2->getInt64()->intValue() < MIN_INT32  ||  value2->getInt64()->intValue() > MAX_INT32 ) {
                        MsgLog("Invalid int value for key %s\n", key2->keyStringValue().c_str());
                      }else{
                        INT32 intValue = (INT32)value2->getInt64()->intValue();
                        (*Child)->Value = (__typeof__((*Child)->Value))AllocatePool(sizeof(intValue));
                        *(INT32*)((*Child)->Value) = intValue;
                        (*Child)->ValueLen = sizeof(intValue);
                        (*Child)->ValueType = kTagTypeInteger;
                      }
                    }
                    else if (value2 && value2->isTrue() ) {
                      (*Child)->Value = (__typeof__((*Child)->Value))AllocateZeroPool(4);
                      (*Child)->Value[0] = TRUE;
                      (*Child)->ValueLen = 1;
                      (*Child)->ValueType = kTagTypeTrue;
                    }
                    else if ( value2 && value2->isFalse() ) {
                      (*Child)->Value = (__typeof__((*Child)->Value))AllocateZeroPool(4);
                      //(*Child)->Value[0] = FALSE;
                      (*Child)->ValueLen = 1;
                      (*Child)->ValueType = kTagTypeFalse;
                    }
                    else if (value2 && (value2->isData())) {
                      UINTN Size = value2->getData()->dataLenValue();
                      //     (*Child)->Value = GetDataSetting(value2, "Value", &Size);  //TODO
                      UINT8* Data = (__typeof__(Data))AllocateZeroPool(Size);
                      CopyMem(Data, value2->getData()->dataValue(), Size);
                      (*Child)->Value = Data;
                      (*Child)->ValueLen = Size;
                      (*Child)->ValueType = kTagTypeData;
                    }
                  }
                }
              }
            }
          }
        }
      }

      Prop  = DevicesDict->propertyForKey("LANInjection");
      gSettings.LANInjection = !IsPropertyNotNullAndFalse(Prop);  //default = TRUE

      Prop  = DevicesDict->propertyForKey("HDMIInjection");
      gSettings.HDMIInjection = IsPropertyNotNullAndTrue(Prop);

      Prop  = DevicesDict->propertyForKey("NoDefaultProperties");
      gSettings.NoDefaultProperties = !IsPropertyNotNullAndFalse(Prop);

      const TagArray* ArbitraryArray = DevicesDict->arrayPropertyForKey("Arbitrary"); // array of dict
      if (ArbitraryArray != NULL) {
        INTN Index;
        INTN   Count = ArbitraryArray->arrayContent().size();
        DEV_PROPERTY *DevProp;

        if (Count > 0) {
          DBG("Add %lld devices (Arbitrary):\n", Count);
          for (Index = 0; Index < Count; Index++) {
            UINTN DeviceAddr = 0U;
            XString8 Label;
            DBG(" - [%02lld]:", Index);
            const TagDict* Dict2 = ArbitraryArray->dictElementAt(Index, "Arbitrary"_XS8);
            const TagStruct*     Prop3;
            Prop3 = Dict2->propertyForKey("PciAddr");
            if (Prop3 != NULL) {
              UINT8 Bus, Dev, Func;

              if ( !Prop3->isString() ) {
                MsgLog("ATTENTION : property not string in PciAddr\n");
                continue;
              }
              if ( Prop3->getString()->stringValue().length() < 2  ||  Prop3->getString()->stringValue()[2] != ':') {
                DBG(" wrong PciAddr string: %s\n", Prop3->getString()->stringValue().c_str());
                continue;
              }
              CONST CHAR8* Str = Prop3->getString()->stringValue().c_str();
              Bus   = hexstrtouint8(Str);
              Dev   = hexstrtouint8(&Str[3]);
              Func  = hexstrtouint8(&Str[6]);
              DeviceAddr = PCIADDR(Bus, Dev, Func);
              Label.S8Printf("[%02hhX:%02hhX.%02hhX] ", Bus, Dev, Func);
              DBG(" %s", Label.c_str());
            } else {
              DBG(" no PciAddr\n");
              continue;
            }

            Prop3 = Dict2->propertyForKey("Comment");
            if (Prop3 != NULL) {
              if ( !Prop3->isString() ) {
                MsgLog("ATTENTION : property not string in Comment\n");
              }else{
                Label += Prop3->getString()->stringValue();
                DBG(" (%s)", Prop3->getString()->stringValue().c_str());
              }
            }
            DBG("\n");
            const TagArray* CustomPropertiesArray = Dict2->arrayPropertyForKey("CustomProperties"); // array of dict
            if (CustomPropertiesArray != NULL) {
              const TagDict* Dict3;
              INTN PropIndex;
              INTN PropCount = CustomPropertiesArray->arrayContent().size();

              for (PropIndex = 0; PropIndex < PropCount; PropIndex++) {
                Dict3 = CustomPropertiesArray->dictElementAt(PropIndex, "CustomProperties"_XS8);
                DevProp = gSettings.ArbProperties;
                gSettings.ArbProperties = new DEV_PROPERTY;
                gSettings.ArbProperties->Next = DevProp;

                gSettings.ArbProperties->Device = (UINT32)DeviceAddr;
                gSettings.ArbProperties->Label = (__typeof__(gSettings.ArbProperties->Label))AllocateCopyPool(Label.sizeInBytesIncludingTerminator(), Label.c_str());

                const TagStruct* DisabledProp = Dict3->propertyForKey("Disabled");
                gSettings.ArbProperties->MenuItem.BValue = !IsPropertyNotNullAndTrue(DisabledProp);

                DisabledProp = Dict3->propertyForKey("Key");
                if (DisabledProp && (DisabledProp->isString()) && DisabledProp->getString()->stringValue().notEmpty()) {
                  gSettings.ArbProperties->Key = S8Printf("%s", DisabledProp->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
                }

                DisabledProp = Dict3->propertyForKey("Value");
                if (DisabledProp && (DisabledProp->isString()) && DisabledProp->getString()->stringValue().notEmpty()) {
                  //first suppose it is Ascii string
                  gSettings.ArbProperties->Value = (UINT8*)S8Printf("%s", DisabledProp->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
                  gSettings.ArbProperties->ValueLen = DisabledProp->getString()->stringValue().sizeInBytesIncludingTerminator();
                  gSettings.ArbProperties->ValueType = kTagTypeString;
                } else if (DisabledProp && (DisabledProp->isInt64())) {
                  if ( DisabledProp->getInt64()->intValue() < MIN_INT32  ||  DisabledProp->getInt64()->intValue() > MAX_INT32 ) {
                    MsgLog("Invalid int value for key 'Value'\n");
                  }else{
                    INT32 intValue = (INT32)DisabledProp->getInt64()->intValue();
                    gSettings.ArbProperties->Value = (__typeof__(gSettings.ArbProperties->Value))AllocatePool(sizeof(intValue));
  //                    CopyMem(gSettings.ArbProperties->Value, &Prop3->intValue, 4);
                    *(INT32*)(gSettings.ArbProperties->Value) = intValue;
                    gSettings.ArbProperties->ValueLen = sizeof(intValue);
                    gSettings.ArbProperties->ValueType = kTagTypeInteger;
                  }
                } else if ( DisabledProp && DisabledProp->isTrue() ) {
                  gSettings.ArbProperties->Value = (__typeof__(gSettings.ArbProperties->Value))AllocateZeroPool(4);
                  gSettings.ArbProperties->Value[0] = TRUE;
                  gSettings.ArbProperties->ValueLen = 1;
                  gSettings.ArbProperties->ValueType = kTagTypeTrue;
                } else if ( DisabledProp && DisabledProp->isFalse() ) {
                  gSettings.ArbProperties->Value = (__typeof__(gSettings.ArbProperties->Value))AllocateZeroPool(4);
                  //gSettings.ArbProperties->Value[0] = FALSE;
                  gSettings.ArbProperties->ValueLen = 1;
                  gSettings.ArbProperties->ValueType = kTagTypeFalse;
                } else {
                  //else  data
                  UINTN Size = 0;
                  gSettings.ArbProperties->Value = GetDataSetting (Dict3, "Value", &Size);
                  gSettings.ArbProperties->ValueLen = Size;
                  gSettings.ArbProperties->ValueType = kTagTypeData;
                }

                //Special case. In future there must be more such cases
                if ((AsciiStrStr(gSettings.ArbProperties->Key, "-platform-id") != NULL)) {
                  CopyMem((CHAR8*)&gSettings.IgPlatform, gSettings.ArbProperties->Value, 4);
                }
              }   //for() device properties
            }
          } //for() devices
        }
        //        gSettings.NrAddProperties = 0xFFFE;
      }
      //can use AddProperties with ArbProperties
      const TagArray* AddPropertiesArray = DevicesDict->arrayPropertyForKey("AddProperties"); // array of dict
      if (Prop != NULL) {
        INTN i;
        INTN Count = AddPropertiesArray->arrayContent().size();
        INTN Index = 0;  //begin from 0 if second enter

        if (Count > 0) {
			DBG("Add %lld properties:\n", Count);
          gSettings.AddProperties = new DEV_PROPERTY[Count];

          for (i = 0; i < Count; i++) {
            UINTN Size = 0;
			  DBG(" - [%02lld]:", i);
            const TagDict* Dict2 = AddPropertiesArray->dictElementAt(i, "AddProperties"_XS8);
            const TagStruct* DeviceProp = Dict2->propertyForKey("Device");
            if (DeviceProp && (DeviceProp->isString()) && DeviceProp->getString()->stringValue().notEmpty()) {
              DEV_PROPERTY *Property = &gSettings.AddProperties[Index];

              if (DeviceProp->getString()->stringValue().equalIC("ATI")) {
                Property->Device = (UINT32)DEV_ATI;
              } else if (DeviceProp->getString()->stringValue().equalIC("NVidia")) {
                Property->Device = (UINT32)DEV_NVIDIA;
              } else if (DeviceProp->getString()->stringValue().equalIC("IntelGFX")) {
                Property->Device = (UINT32)DEV_INTEL;
              } else if (DeviceProp->getString()->stringValue().equalIC("LAN")) {
                Property->Device = (UINT32)DEV_LAN;
              } else if (DeviceProp->getString()->stringValue().equalIC("WIFI")) {
                Property->Device = (UINT32)DEV_WIFI;
              } else if (DeviceProp->getString()->stringValue().equalIC("Firewire")) {
                Property->Device = (UINT32)DEV_FIREWIRE;
              } else if (DeviceProp->getString()->stringValue().equalIC("SATA")) {
                Property->Device = (UINT32)DEV_SATA;
              } else if (DeviceProp->getString()->stringValue().equalIC("IDE")) {
                Property->Device = (UINT32)DEV_IDE;
              } else if (DeviceProp->getString()->stringValue().equalIC("HDA")) {
                Property->Device = (UINT32)DEV_HDA;
              } else if (DeviceProp->getString()->stringValue().equalIC("HDMI")) {
                Property->Device = (UINT32)DEV_HDMI;
              } else if (DeviceProp->getString()->stringValue().equalIC("LPC")) {
                Property->Device = (UINT32)DEV_LPC;
              } else if (DeviceProp->getString()->stringValue().equalIC("SmBUS")) {
                Property->Device = (UINT32)DEV_SMBUS;
              } else if (DeviceProp->getString()->stringValue().equalIC("USB")) {
                Property->Device = (UINT32)DEV_USB;
              } else {
                DBG(" unknown device, ignored\n"/*, i*/);
                continue;
              }
            }

            if ( DeviceProp->isString() ) DBG(" %s ", DeviceProp->getString()->stringValue().c_str());

            const TagStruct* Prop2 = Dict2->propertyForKey("Disabled");
            gSettings.AddProperties[Index].MenuItem.BValue = !IsPropertyNotNullAndTrue(Prop2);

            Prop2 = Dict2->propertyForKey("Key");
            if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
              gSettings.AddProperties[Index].Key = S8Printf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
            }

            Prop2 = Dict2->propertyForKey("Value");
            if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
              //first suppose it is Ascii string
              gSettings.AddProperties[Index].Value = (UINT8*)S8Printf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
              gSettings.AddProperties[Index].ValueLen = Prop2->getString()->stringValue().sizeInBytesIncludingTerminator();
            } else if (Prop2 && (Prop2->isInt64())) {
              if ( Prop2->getInt64()->intValue() < MIN_INT32  ||  Prop2->getInt64()->intValue() > MAX_INT32 ) {
                MsgLog("Invalid int value for key 'Value'\n");
              }else{
                INT32 intValue = (INT32)Prop2->getInt64()->intValue();
                gSettings.AddProperties[Index].Value = (__typeof__(gSettings.AddProperties[Index].Value))AllocatePool (sizeof(intValue));
  //              CopyMem(gSettings.AddProperties[Index].Value, &Prop2->intValue, 4);
                *(INT32*)(gSettings.AddProperties[Index].Value) = intValue;
                gSettings.AddProperties[Index].ValueLen = sizeof(intValue);
              }
            } else {
              //else  data
              gSettings.AddProperties[Index].Value = GetDataSetting (Dict2, "Value", &Size);
              gSettings.AddProperties[Index].ValueLen = Size;
            }

			  DBG("Key: %s, len: %llu\n", gSettings.AddProperties[Index].Key, gSettings.AddProperties[Index].ValueLen);

            if (!gSettings.AddProperties[Index].MenuItem.BValue) {
              DBG("  property disabled at config\n");
            }

            ++Index;
          }

          gSettings.NrAddProperties = Index;
        }
      }
      //end AddProperties

      const TagDict* FakeIDDict = DevicesDict->dictPropertyForKey("FakeID");
      if (FakeIDDict != NULL) {
        const TagStruct* Prop2 = FakeIDDict->propertyForKey("ATI");
        if (Prop2 && (Prop2->isString())) {
          gSettings.FakeATI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("NVidia");
        if (Prop2 && (Prop2->isString())) {
          gSettings.FakeNVidia  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("IntelGFX");
        if (Prop2 && (Prop2->isString())) {
          gSettings.FakeIntel  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("LAN");
        if (Prop2 && (Prop2->isString())) {
          gSettings.FakeLAN  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("WIFI");
        if (Prop2 && (Prop2->isString())) {
          gSettings.FakeWIFI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("SATA");
        if (Prop2 && (Prop2->isString())) {
          gSettings.FakeSATA  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("XHCI");
        if (Prop2 && (Prop2->isString())) {
          gSettings.FakeXHCI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("IMEI");
        if (Prop2 && (Prop2->isString())) {
          gSettings.FakeIMEI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }
      }

      Prop                   = DevicesDict->propertyForKey("UseIntelHDMI");
      gSettings.UseIntelHDMI = IsPropertyNotNullAndTrue(Prop);

      Prop                = DevicesDict->propertyForKey("ForceHPET");
      gSettings.ForceHPET = IsPropertyNotNullAndTrue(Prop);

      Prop                = DevicesDict->propertyForKey("DisableFunctions");
      if (Prop && (Prop->isString())) {
        gSettings.DisableFunctions  = (UINT32)AsciiStrHexToUint64(Prop->getString()->stringValue());
      }

      Prop                = DevicesDict->propertyForKey("AirportBridgeDeviceName");
      if (Prop && (Prop->isString())) {
        gSettings.AirportBridgeDeviceName = Prop->getString()->stringValue();
      }

      const TagDict* AudioDict = DevicesDict->dictPropertyForKey("Audio");
      if (AudioDict != NULL) {
        // HDA
        //       Prop = GetProperty(Prop2, "ResetHDA");
        //       gSettings.ResetHDA = IsPropertyTrue(Prop);
        Prop = AudioDict->propertyForKey("Inject");
        if (Prop != NULL) {
          // enabled by default
          // syntax:
          // - HDAInjection=No or 0 - disables injection
          // - HDAInjection=887 - injects layout-id 887 decimal (0x00000377)
          // - HDAInjection=0x377 - injects layout-id 887 decimal (0x00000377)
          // - HDAInjection=Detect - reads codec device id (eg. 0x0887)
          //   converts it to decimal 887 and injects this as layout-id.
          //   if hex device is cannot be converted to decimal, injects legacy value 12 decimal
          // - all other values are equal to HDAInjection=Detect
          if (Prop->isInt64()) {
            gSettings.HDALayoutId = (INT32)Prop->getInt64()->intValue(); //must be signed
            gSettings.HDAInjection = (gSettings.HDALayoutId > 0);
          } else if (Prop->isString()){
            if ( Prop->getString()->stringValue().notEmpty()  &&  (Prop->getString()->stringValue()[0] == 'n' || Prop->getString()->stringValue()[0] == 'N') ) {
              // if starts with n or N, then no HDA injection
              gSettings.HDAInjection = FALSE;
            } else if ( Prop->getString()->stringValue().length() > 1  &&
                        Prop->getString()->stringValue()[0] == '0'  &&
                        ( Prop->getString()->stringValue()[1] == 'x' || Prop->getString()->stringValue()[1] == 'X' ) ) {
              // assume it's a hex layout id
              gSettings.HDALayoutId = (INT32)AsciiStrHexToUintn(Prop->getString()->stringValue());
              gSettings.HDAInjection = TRUE;
            } else {
              // assume it's a decimal layout id
              gSettings.HDALayoutId = (INT32)AsciiStrDecimalToUintn(Prop->getString()->stringValue());
              gSettings.HDAInjection = TRUE;
            }
          }
        }

        Prop = AudioDict->propertyForKey("AFGLowPowerState");
        gSettings.AFGLowPowerState = IsPropertyNotNullAndTrue(Prop);
      }

      const TagDict* USBDict = DevicesDict->dictPropertyForKey("USB");
      if (USBDict != NULL) {
        // USB
        Prop = USBDict->propertyForKey("Inject");
        gSettings.USBInjection = !IsPropertyNotNullAndFalse(Prop); // enabled by default

        Prop = USBDict->propertyForKey("AddClockID");
        gSettings.InjectClockID = IsPropertyNotNullAndTrue(Prop); // disabled by default
        // enabled by default for CloverEFI
        // disabled for others
        gSettings.USBFixOwnership = gFirmwareClover;
        Prop = USBDict->propertyForKey("FixOwnership");
        if (Prop != NULL) {
          gSettings.USBFixOwnership = IsPropertyNotNullAndTrue(Prop);
        }
        DBG("USB FixOwnership: %s\n", gSettings.USBFixOwnership?"yes":"no");

        Prop = USBDict->propertyForKey("HighCurrent");
        gSettings.HighCurrent = IsPropertyNotNullAndTrue(Prop);

        Prop = USBDict->propertyForKey("NameEH00");
        gSettings.NameEH00 = IsPropertyNotNullAndTrue(Prop);
      }
    }

    //*** ACPI ***//

    getACPISettings(CfgDict);

    //*** SMBIOS ***//
    const TagDict* SMBIOSDict = CfgDict->dictPropertyForKey("SMBIOS");
    if (SMBIOSDict != NULL) {
      ParseSMBIOSSettings(SMBIOSDict);
      const TagStruct* Prop = SMBIOSDict->propertyForKey("Trust");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndFalse(Prop)) {
          gSettings.TrustSMBIOS = FALSE;
        } else if (IsPropertyNotNullAndTrue(Prop)) {
          gSettings.TrustSMBIOS = TRUE;
        }
      }
      Prop = SMBIOSDict->propertyForKey("MemoryRank");
      gSettings.Attribute = (INT8)GetPropertyAsInteger(Prop, -1); //1==Single Rank, 2 == Dual Rank, 0==undefined -1 == keep as is

      // Delete the user memory when a new config is selected
      INTN i = 0;
      for (i = 0; i < gRAM.UserInUse && i < MAX_RAM_SLOTS; i++) {
        gRAM.User[i].ModuleSize = 0;
        gRAM.User[i].InUse = 0;
      }
      gRAM.UserInUse = 0;
      gRAM.UserChannels = 0;
      gSettings.InjectMemoryTables = FALSE;

      // Inject memory tables into SMBIOS
      const TagDict* MemoryDict = SMBIOSDict->dictPropertyForKey("Memory");
      if (MemoryDict != NULL){
        // Get memory table count
        const TagStruct* Prop2   = MemoryDict->propertyForKey("SlotCount");
        gRAM.UserInUse = (UINT8)GetPropertyAsInteger(Prop2, 0);
        // Get memory channels
        Prop2             = MemoryDict->propertyForKey("Channels");
        gRAM.UserChannels = (UINT8)GetPropertyAsInteger(Prop2, 0);
        // Get memory tables
        const TagArray* ModulesArray = MemoryDict->arrayPropertyForKey("Modules"); // array of dict
        if (ModulesArray != NULL) {
          INTN Count = ModulesArray->arrayContent().size();
          for (i = 0; i < Count; i++) {
            const TagDict* Prop3 = ModulesArray->dictElementAt(i, "SMBIOS/Memory/Modules"_XS8);
            UINT8 Slot = MAX_RAM_SLOTS;
            RAM_SLOT_INFO *SlotPtr;
            // Get memory slot
            Prop2 = Prop3->propertyForKey("Slot");
            if (Prop2 == NULL) {
              continue;
            }

            if (Prop2->isString() && Prop2->getString()->stringValue().notEmpty() ) {
              Slot = (UINT8)AsciiStrDecimalToUintn(Prop2->getString()->stringValue());
            } else if (Prop2->isInt64()) {
              Slot = Prop2->getInt64()->intValue();
            } else {
              continue;
            }

            if (Slot >= MAX_RAM_SLOTS) {
              continue;
            }

            SlotPtr = &gRAM.User[Slot];

            // Get memory size
            Prop2 = Prop3->propertyForKey("Size");
            SlotPtr->ModuleSize = (UINT32)GetPropertyAsInteger(Prop2, SlotPtr->ModuleSize);
            // Get memory frequency
            Prop2 = Prop3->propertyForKey("Frequency");
            SlotPtr->Frequency  = (UINT32)GetPropertyAsInteger(Prop2, SlotPtr->Frequency);
            // Get memory vendor
            Prop2 = Prop3->propertyForKey("Vendor");
            if (Prop2 && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              SlotPtr->Vendor   = S8Printf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
            }
            // Get memory part number
            Prop2 = Prop3->propertyForKey("Part");
            if (Prop2 && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              SlotPtr->PartNo   = S8Printf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
            }
            // Get memory serial number
            Prop2 = Prop3->propertyForKey("Serial");
            if (Prop2 && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              SlotPtr->SerialNo = S8Printf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
            }
            // Get memory type
            SlotPtr->Type = MemoryTypeDdr3;
            Prop2 = Prop3->propertyForKey("Type");
            if (Prop2 && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              if (Prop2->getString()->stringValue().equalIC("DDR2")) {
                SlotPtr->Type = MemoryTypeDdr2;
              } else if (Prop2->getString()->stringValue().equalIC("DDR3")) {
                SlotPtr->Type = MemoryTypeDdr3;
              } else if (Prop2->getString()->stringValue().equalIC("DDR4")) {
                SlotPtr->Type = MemoryTypeDdr4;
              } else if (Prop2->getString()->stringValue().equalIC("DDR")) {
                SlotPtr->Type = MemoryTypeDdr;
              }
            }

            SlotPtr->InUse = (SlotPtr->ModuleSize > 0);
            if (SlotPtr->InUse) {
              if (gRAM.UserInUse <= Slot) {
                gRAM.UserInUse = Slot + 1;
              }
            }
          }

          if (gRAM.UserInUse > 0) {
            gSettings.InjectMemoryTables = TRUE;
          }
        }
      }

      const TagArray* SlotsArray = SMBIOSDict->arrayPropertyForKey("Slots"); // array of dict
      if (SlotsArray != NULL) {
        INTN   DeviceN;
        INTN Count = SlotsArray->arrayContent().size();

        for (INTN Index = 0; Index < Count; ++Index) {
          const TagDict* SlotsDict = SlotsArray->dictElementAt(Index, "SMBIOS/Slots"_XS8);
          if (!Index) {
            DBG("Slots->Devices:\n");
          }

          const TagStruct* Prop2 = SlotsDict->propertyForKey("Device");
          DeviceN = -1;
          if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
            if (Prop2->getString()->stringValue().equalIC("ATI")) {
              DeviceN = 0;
            } else if (Prop2->getString()->stringValue().equalIC("NVidia")) {
              DeviceN = 1;
            } else if (Prop2->getString()->stringValue().equalIC("IntelGFX")) {
              DeviceN = 2;
            } else if (Prop2->getString()->stringValue().equalIC("LAN")) {
              DeviceN = 5;
            } else if (Prop2->getString()->stringValue().equalIC("WIFI")) {
              DeviceN = 6;
            } else if (Prop2->getString()->stringValue().equalIC("Firewire")) {
              DeviceN = 12;
            } else if (Prop2->getString()->stringValue().equalIC("HDMI")) {
              DeviceN = 4;
            } else if (Prop2->getString()->stringValue().equalIC("USB")) {
              DeviceN = 11;
            } else if (Prop2->getString()->stringValue().equalIC("NVME")) {
              DeviceN = 13;
            } else {
              DBG(" - add properties to unknown device %s, ignored\n", Prop2->getString()->stringValue().c_str());
              continue;
            }
          } else {
            DBG(" - no device  property for slot\n");
            continue;
          }

          if (DeviceN >= 0) {
            SLOT_DEVICE *SlotDevice = &SlotDevices[DeviceN];
            Prop2                   = SlotsDict->propertyForKey("ID");
            SlotDevice->SlotID      = (UINT8)GetPropertyAsInteger(Prop2, DeviceN);
            SlotDevice->SlotType    = SlotTypePci;

            Prop2                   = SlotsDict->propertyForKey("Type");
            if (Prop2 != NULL) {
              switch ((UINT8)GetPropertyAsInteger(Prop2, 0)) {
                case 0:
                  SlotDevice->SlotType = SlotTypePci;
                  break;

                case 1:
                  SlotDevice->SlotType = SlotTypePciExpressX1;
                  break;

                case 2:
                  SlotDevice->SlotType = SlotTypePciExpressX2;
                  break;

                case 4:
                  SlotDevice->SlotType = SlotTypePciExpressX4;
                  break;

                case 8:
                  SlotDevice->SlotType = SlotTypePciExpressX8;
                  break;

                case 16:
                  SlotDevice->SlotType = SlotTypePciExpressX16;
                  break;

                default:
                  SlotDevice->SlotType = SlotTypePciExpress;
                  break;
              }
            }
            Prop2 = SlotsDict->propertyForKey("Name");
            if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
				snprintf (SlotDevice->SlotName, 31, "%s", Prop2->getString()->stringValue().c_str());
            } else {
				snprintf (SlotDevice->SlotName, 31, "PCI Slot %lld", DeviceN);
            }

            DBG(" - %s\n", SlotDevice->SlotName);
          }
        }
      }
    }

    //CPU
    const TagDict* CPUDict = CfgDict->dictPropertyForKey("CPU");
    if (CPUDict != NULL) {
      const TagStruct* Prop = CPUDict->propertyForKey("QPI");
      if (Prop != NULL) {
        gSettings.QPI = (UINT16)GetPropertyAsInteger(Prop, gSettings.QPI);
        DBG("QPI: %dMHz\n", gSettings.QPI);
      }

      Prop = CPUDict->propertyForKey("FrequencyMHz");
      if (Prop != NULL) {
        gSettings.CpuFreqMHz = (UINT32)GetPropertyAsInteger(Prop, gSettings.CpuFreqMHz);
        DBG("CpuFreq: %dMHz\n", gSettings.CpuFreqMHz);
      }

      Prop = CPUDict->propertyForKey("Type");
      gSettings.CpuType = GetAdvancedCpuType();
      if (Prop != NULL) {
        gSettings.CpuType = (UINT16)GetPropertyAsInteger(Prop, gSettings.CpuType);
		  DBG("CpuType: %hX\n", gSettings.CpuType);
      }

      Prop = CPUDict->propertyForKey("QEMU");
      gSettings.QEMU = IsPropertyNotNullAndTrue(Prop);
      if (gSettings.QEMU) {
        DBG("QEMU: true\n");
      }

      Prop = CPUDict->propertyForKey("UseARTFrequency");
      gSettings.UseARTFreq = IsPropertyNotNullAndTrue(Prop);

      gSettings.UserChange = FALSE;
      Prop = CPUDict->propertyForKey("BusSpeedkHz");
      if (Prop != NULL) {
        gSettings.BusSpeed = (UINT32)GetPropertyAsInteger(Prop, gSettings.BusSpeed);
        DBG("BusSpeed: %dkHz\n", gSettings.BusSpeed);
        gSettings.UserChange = TRUE;
      }

      Prop = CPUDict->propertyForKey("C6");
      if (Prop != NULL) {
        gSettings.EnableC6 = IsPropertyNotNullAndTrue(Prop);
      }

      Prop = CPUDict->propertyForKey("C4");
      if (Prop != NULL) {
        gSettings.EnableC4 = IsPropertyNotNullAndTrue(Prop);
      }

      Prop = CPUDict->propertyForKey("C2");
      if (Prop != NULL) {
        gSettings.EnableC2 = IsPropertyNotNullAndTrue(Prop);
      }

      //Usually it is 0x03e9, but if you want Turbo, you may set 0x00FA
      Prop                 = CPUDict->propertyForKey("Latency");
      gSettings.C3Latency  = (UINT16)GetPropertyAsInteger(Prop, gSettings.C3Latency);

      Prop                 = CPUDict->propertyForKey("SavingMode");
      gSettings.SavingMode = (UINT8)GetPropertyAsInteger(Prop, 0xFF); //the default value means not set

      Prop                 = CPUDict->propertyForKey("HWPEnable");
      if (Prop && IsPropertyNotNullAndTrue(Prop) && (gCPUStructure.Model >= CPU_MODEL_SKYLAKE_U)) {
        gSettings.HWP = TRUE;
        AsmWriteMsr64 (MSR_IA32_PM_ENABLE, 1);
      }
      Prop                 = CPUDict->propertyForKey("HWPValue");
      if (Prop && gSettings.HWP) {
        gSettings.HWPValue = (UINT32)GetPropertyAsInteger(Prop, 0);
        AsmWriteMsr64 (MSR_IA32_HWP_REQUEST, gSettings.HWPValue);
      }

      Prop                 = CPUDict->propertyForKey("TDP");
      gSettings.TDP  = (UINT8)GetPropertyAsInteger(Prop, 0);

      Prop                 = CPUDict->propertyForKey("TurboDisable");
      if (Prop && IsPropertyNotNullAndTrue(Prop)) {
        UINT64 msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
        gSettings.Turbo = 0;
        msr &= ~(1ULL<<38);
        AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, msr);
      }
    }

    // RtVariables
    const TagDict* RtVariablesDict = CfgDict->dictPropertyForKey("RtVariables");
    if (RtVariablesDict != NULL) {
      // ROM: <data>bin data</data> or <string>base 64 encoded bin data</string>
      const TagStruct* Prop = RtVariablesDict->propertyForKey("ROM");
      if (Prop != NULL) {
        if ( Prop->isString()  &&  Prop->getString()->stringValue().equalIC("UseMacAddr0") ) {
          gSettings.RtROM         = &gLanMac[0][0];
          gSettings.RtROMLen      = 6;
        } else if ( Prop->isString()  &&  Prop->getString()->stringValue().equalIC("UseMacAddr1") ) {
          gSettings.RtROM         = &gLanMac[1][0];
          gSettings.RtROMLen      = 6;
        } else if ( Prop->isString()  ||  Prop->isData() ) { // GetDataSetting accept both
          UINTN ROMLength         = 0;
          gSettings.RtROM         = GetDataSetting(RtVariablesDict, "ROM", &ROMLength);
          gSettings.RtROMLen      = ROMLength;
        } else {
          MsgLog("MALFORMED PLIST : property not string or data in RtVariables/ROM\n");
        }

        if (gSettings.RtROM == NULL || gSettings.RtROMLen == 0) {
          gSettings.RtROM       = NULL;
          gSettings.RtROMLen    = 0;
        }
      }

      // MLB: <string>some value</string>
      Prop = RtVariablesDict->propertyForKey("MLB");
      if ( Prop != NULL ) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in RtVariables/MLB\n");
        }else{
          if( Prop->getString()->stringValue().notEmpty() ) {
            gSettings.RtMLB = Prop->getString()->stringValue();
          }
        }
      }
      // CsrActiveConfig
      Prop = RtVariablesDict->propertyForKey("CsrActiveConfig");
      gSettings.CsrActiveConfig = (UINT32)GetPropertyAsInteger(Prop, 0x267); //the value 0xFFFF means not set

      //BooterConfig
      Prop = RtVariablesDict->propertyForKey("BooterConfig");
      gSettings.BooterConfig = (UINT16)GetPropertyAsInteger(Prop, 0); //the value 0 means not set
      //let it be string like "log=0"
      Prop = RtVariablesDict->propertyForKey("BooterCfg");
      if ( Prop != NULL ) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in RtVariables/BooterCfg\n");
        }else{
          if( Prop->getString()->stringValue().notEmpty() ) {
            gSettings.BooterCfgStr = Prop->getString()->stringValue();
          }
        }
      }
      //Block external variables
      const TagArray* BlockArray = RtVariablesDict->arrayPropertyForKey("Block"); // array of dict
      if (BlockArray != NULL) {
        INTN   i;
        INTN Count = BlockArray->arrayContent().size();
        RtVariablesNum = 0;
        RtVariables = (__typeof__(RtVariables))AllocateZeroPool(Count * sizeof(RT_VARIABLES));
        for (i = 0; i < Count; i++) {
          CfgDict = BlockArray->dictElementAt(i, "Block"_XS8);
          const TagStruct* Prop2 = CfgDict->propertyForKey("Comment");
          if ( Prop2 != NULL ) {
            if ( !Prop2->isString() ) {
              MsgLog("ATTENTION : property not string in Block/Comment\n");
            }else{
              if( Prop2->getString()->stringValue().notEmpty() ) {
                DBG(" %s\n", Prop2->getString()->stringValue().c_str());
              }
            }
          }
          Prop2 = CfgDict->propertyForKey("Disabled");
          if (IsPropertyNotNullAndFalse(Prop2)) {
            continue;
          }
          Prop2 = CfgDict->propertyForKey("Guid");
          if ( Prop2 != NULL ) {
            if ( !Prop2->isString() ) {
              MsgLog("ATTENTION : property not string in Block/Guid\n");
            }else{
              if( Prop2->getString()->stringValue().notEmpty() ) {
                if (IsValidGuidAsciiString(Prop2->getString()->stringValue())) {
                  StrToGuidLE(Prop2->getString()->stringValue(), &RtVariables[RtVariablesNum].VarGuid);
                }else{
                 DBG("Error: invalid GUID for RT var '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->getString()->stringValue().c_str());
                }
              }
            }
          }

          Prop2 = CfgDict->propertyForKey("Name");
          RtVariables[RtVariablesNum].Name = NULL;
          if ( Prop2 != NULL ) {
            if ( !Prop2->isString() ) {
              MsgLog("ATTENTION : property not string in Block/Name\n");
            }else{
              if( Prop2->getString()->stringValue().notEmpty() ) {
                snwprintf(RtVariables[RtVariablesNum].Name, 64, "%s", Prop2->getString()->stringValue().c_str());
              }
            }
          }
          RtVariablesNum++;
        }
      }
    }

    if (gSettings.RtROM == NULL) {
      gSettings.RtROM    = (UINT8*)&gSettings.SmUUID.Data4[2];
      gSettings.RtROMLen = 6;
    }

    if (gSettings.RtMLB.isEmpty()) {
      gSettings.RtMLB       = gSettings.BoardSerialNumber;
    }

    // if CustomUUID and InjectSystemID are not specified
    // then use InjectSystemID=TRUE and SMBIOS UUID
    // to get Chameleon's default behaviour (to make user's life easier)
    CopyMem((VOID*)&gUuid, (VOID*)&gSettings.SmUUID, sizeof(EFI_GUID));
    gSettings.InjectSystemID = TRUE;

    // SystemParameters again - values that can depend on previous params
    const TagDict* SystemParametersDict = CfgDict->dictPropertyForKey("SystemParameters");
    if (SystemParametersDict != NULL) {
      //BacklightLevel
      const TagStruct* Prop = SystemParametersDict->propertyForKey("BacklightLevel");
      if (Prop != NULL) {
        gSettings.BacklightLevel       = (UINT16)GetPropertyAsInteger(Prop, gSettings.BacklightLevel);
        gSettings.BacklightLevelConfig = TRUE;
      }

      Prop = SystemParametersDict->propertyForKey("CustomUUID");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in SystemParameters/CustomUUID\n");
        }else{
          BOOLEAN IsValidCustomUUID = FALSE;
          if (IsValidGuidAsciiString(Prop->getString()->stringValue())) {
            gSettings.CustomUuid = Prop->getString()->stringValue();
            DBG("Converted CustomUUID %ls\n", gSettings.CustomUuid.wc_str());
            Status = StrToGuidLE(gSettings.CustomUuid, &gUuid);
            if (!EFI_ERROR(Status)) {
              IsValidCustomUUID = TRUE;
              // if CustomUUID specified, then default for InjectSystemID=FALSE
              // to stay compatibile with previous Clover behaviour
              gSettings.InjectSystemID = FALSE;
              DBG("The UUID is valid\n");
            }
          }

          if (!IsValidCustomUUID) {
            DBG("Error: invalid CustomUUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->getString()->stringValue().c_str());
          }
        }
      }
      //else gUuid value from SMBIOS
      //     DBG("Finally use %s\n", strguid(&gUuid));

      Prop                     = SystemParametersDict->propertyForKey("InjectSystemID");
      gSettings.InjectSystemID = gSettings.InjectSystemID ? !IsPropertyNotNullAndFalse(Prop) : IsPropertyNotNullAndTrue(Prop);

      Prop                     = SystemParametersDict->propertyForKey("NvidiaWeb");
      gSettings.NvidiaWeb      = IsPropertyNotNullAndTrue(Prop);

    }


    const TagDict* BootGraphicsDict = CfgDict->dictPropertyForKey("BootGraphics");
    if (BootGraphicsDict != NULL) {
      const TagStruct* Prop = BootGraphicsDict->propertyForKey("DefaultBackgroundColor");
      gSettings.DefaultBackgroundColor = (UINT32)GetPropertyAsInteger(Prop, 0x80000000); //the value 0x80000000 means not set

      Prop = BootGraphicsDict->propertyForKey("UIScale");
      gSettings.UIScale = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

      Prop = BootGraphicsDict->propertyForKey("EFILoginHiDPI");
      gSettings.EFILoginHiDPI = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

      Prop = BootGraphicsDict->propertyForKey("flagstate");
      *(UINT32*)&gSettings.flagstate[0] = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

    }
    /*
     //Example
     <key>RMde</key>
     <array>
     <string>char</string>
     <data>
     QQ==
     </data>
     </array>

     DictPointer = GetProperty(Dict, "SMCKeys");
     if (DictPointer != NULL) {   //sss
     TagStruct*     Key, ValArray;
     for (Key = DictPointer->tag; Key != NULL; Key = Key->tagNext) {
     ValArray = Prop->tag;
     if (Key->type != kTagTypeKey || ValArray == NULL) {
     DBG(" ERROR: Tag is not <key>, type = %d\n", Key->type);
     continue;
     }
     //....
     }
     }
     */
    /*
     {
     EFI_GUID AppleGuid;

     CopyMem((VOID*)&AppleGuid, (VOID*)&gUuid, sizeof(EFI_GUID));
     AppleGuid.Data1 = SwapBytes32 (AppleGuid.Data1);
     AppleGuid.Data2 = SwapBytes16 (AppleGuid.Data2);
     AppleGuid.Data3 = SwapBytes16 (AppleGuid.Data3);
     DBG("Platform Uuid: %s, InjectSystemID: %s\n", strguid(&AppleGuid), gSettings.InjectSystemID ? "Yes" : "No");
     }
     */

    if (gBootChanged) {
      const TagDict* KernelAndKextPatchesDict = CfgDict->dictPropertyForKey("KernelAndKextPatches");
      if (KernelAndKextPatchesDict != NULL) {
        DBG("refill kernel patches bcoz gBootChanged\n");
        FillinKextPatches(&gSettings.KernelAndKextPatches, KernelAndKextPatchesDict);
      }
    } else {
      //DBG("\n ConfigName: %ls n", gSettings.ConfigName);
    }
    if (gThemeChanged) {
      GlobalConfig.Theme.setEmpty();
      const TagDict* GUIDict = CfgDict->dictPropertyForKey("GUI");
      if (GUIDict != NULL) {
        const TagStruct* Prop = GUIDict->propertyForKey("Theme");
        if ((Prop != NULL) && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          GlobalConfig.Theme.takeValueFrom(Prop->getString()->stringValue());
          DBG("Theme from new config: %ls\n", GlobalConfig.Theme.wc_str());
        }
      }
    }
    SaveSettings();
  }
  //DBG("config.plist read and return %s\n", efiStrError(Status));
  return EFI_SUCCESS;
}
/*
static CONST CHAR8 *SearchString(
                            IN  CONST CHAR8       *Source,
                            IN  UINT64      SourceSize,
                            IN  CONST CHAR8       *Search,
                            IN  UINTN       SearchSize
                            )
{
  CONST CHAR8 *End = Source + SourceSize;

  while (Source < End) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
      return Source;
    } else {
      Source++;
    }
  }
  return NULL;
}
*/
XString8 GetOSVersion(IN LOADER_ENTRY *Entry)
{
  XString8   OSVersion;
  EFI_STATUS Status      = EFI_NOT_FOUND;
  CHAR8*     PlistBuffer = NULL;
  UINTN      PlistLen;
  TagDict*     Dict        = NULL;
  const TagDict*     DictPointer = NULL;
  const TagStruct*     Prop        = NULL;

  if (!Entry || !Entry->Volume) {
    return NullXString8;
  }

  if (OSTYPE_IS_OSX(Entry->LoaderType))
  {
  	XString8 uuidPrefix;
    if ( Entry->APFSTargetUUID.notEmpty() ) uuidPrefix = S8Printf("\\%ls", Entry->APFSTargetUUID.wc_str());

  	XStringW plist = SWPrintf("%s\\System\\Library\\CoreServices\\SystemVersion.plist", uuidPrefix.c_str());
		if ( !FileExists(Entry->Volume->RootDir, plist) ) {
			plist = SWPrintf("%s\\System\\Library\\CoreServices\\ServerVersion.plist", uuidPrefix.c_str());
			if ( !FileExists(Entry->Volume->RootDir, plist) ) {
				plist.setEmpty();
    	}
    }

    if ( plist.notEmpty() ) { // found macOS System
      Status = egLoadFile(Entry->Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = Dict->propertyForKey("ProductVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductVersion\n");
          }else{
            if( Prop->getString()->stringValue().notEmpty() ) {
              OSVersion = Prop->getString()->stringValue();
            }
          }
        }
        Prop = Dict->propertyForKey("ProductBuildVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
          }else{
            if( Prop->getString()->stringValue().notEmpty() ) {
              Entry->BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }
  }

  if (OSTYPE_IS_OSX_INSTALLER (Entry->LoaderType)) {
    // Detect exact version for 2nd stage Installer (thanks to dmazar for this idea)
    // This should work for most installer cases. Rest cases will be read from boot.efi before booting.
    // Reworked by Sherlocks. 2018.04.12

    // 1st stage - 1
    // Check for plist - createinstallmedia/BaseSystem/InstallDVD/InstallESD
    CONST CHAR16 *InstallerPlist = L"\\.IABootFilesSystemVersion.plist"; // 10.9 - 10.13.3
    if (!FileExists (Entry->Volume->RootDir, InstallerPlist) && FileExists (Entry->Volume->RootDir, L"\\System\\Library\\CoreServices\\boot.efi") &&
        ((FileExists (Entry->Volume->RootDir, L"\\BaseSystem.dmg") && FileExists (Entry->Volume->RootDir, L"\\mach_kernel")) || // 10.7/10.8
         FileExists (Entry->Volume->RootDir, L"\\System\\Installation\\CDIS\\Mac OS X Installer.app") || // 10.6/10.7
         FileExists (Entry->Volume->RootDir, L"\\System\\Installation\\CDIS\\OS X Installer.app") || // 10.8 - 10.11
         FileExists (Entry->Volume->RootDir, L"\\System\\Installation\\CDIS\\macOS Installer.app") || // 10.12+
         FileExists (Entry->Volume->RootDir, L"\\.IAPhysicalMedia"))) { // 10.13.4+
      InstallerPlist = L"\\System\\Library\\CoreServices\\SystemVersion.plist";
    }
    if (FileExists (Entry->Volume->RootDir, InstallerPlist)) {
      Status = egLoadFile(Entry->Volume->RootDir, InstallerPlist, (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = Dict->propertyForKey("ProductVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductVersion\n");
          }else{
            if( Prop->getString()->stringValue().notEmpty() ) {
              OSVersion = Prop->getString()->stringValue();
            }
          }
        }
        Prop = Dict->propertyForKey("ProductBuildVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
          }else{
            if( Prop->getString()->stringValue().notEmpty() ) {
              Entry->BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }

    // 1st stage - 2
    // Check for plist - createinstallmedia/NetInstall
    if (OSVersion.isEmpty()) {
      InstallerPlist = L"\\.IABootFiles\\com.apple.Boot.plist"; // 10.9 - ...
      if (FileExists (Entry->Volume->RootDir, InstallerPlist)) {
        Status = egLoadFile(Entry->Volume->RootDir, InstallerPlist, (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = Dict->propertyForKey("Kernel Flags");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in Kernel Flags\n");
            }else{
              if ( Prop->getString()->stringValue().contains("Install%20macOS%20BigSur") || Prop->getString()->stringValue().contains("Install%20macOS%2011.0")) {
                OSVersion = "11.0"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%2010.16")) {
                OSVersion = "10.16"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20Catalina") || Prop->getString()->stringValue().contains("Install%20macOS%2010.15")) {
                OSVersion = "10.15"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20Mojave") || Prop->getString()->stringValue().contains("Install%20macOS%2010.14")) {
                OSVersion = "10.14"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20High%20Sierra") || Prop->getString()->stringValue().contains("Install%20macOS%2010.13")) {
                OSVersion = "10.13"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20macOS%20Sierra") || Prop->getString()->stringValue().contains("Install%20OS%20hhX%2010.12")) {
                OSVersion = "10.12"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20OS%20hhX%20El%20Capitan") || Prop->getString()->stringValue().contains("Install%20OS%20hhX%2010.11")) {
                OSVersion = "10.11"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20OS%20hhX%20Yosemite") || Prop->getString()->stringValue().contains("Install%20OS%20hhX%2010.10")) {
                OSVersion = "10.10"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20OS%20hhX%20Mavericks.app")) {
                OSVersion = "10.9"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20OS%20hhX%20Mountain%20Lion")) {
                OSVersion = "10.8"_XS8;
              } else if ( Prop->getString()->stringValue().contains("Install%20Mac%20OS%20hhX%20Lion")) {
                OSVersion = "10.7"_XS8;
              }
            }
          }
        }
      }
    }

    // 2nd stage - 1
    // Check for plist - AppStore/createinstallmedia/startosinstall/Fusion Drive
    if (OSVersion.isEmpty()) {
      InstallerPlist = L"\\macOS Install Data\\Locked Files\\Boot Files\\SystemVersion.plist"; // 10.12.4+
      if (!FileExists (Entry->Volume->RootDir, InstallerPlist)) {
        InstallerPlist = L"\\macOS Install Data\\InstallInfo.plist"; // 10.12+
        if (!FileExists (Entry->Volume->RootDir, InstallerPlist)) {
          InstallerPlist = L"\\com.apple.boot.R\\SystemVersion.plist"; // 10.12+
          if (!FileExists (Entry->Volume->RootDir, InstallerPlist)) {
            InstallerPlist = L"\\com.apple.boot.P\\SystemVersion.plist"; // 10.12+
            if (!FileExists (Entry->Volume->RootDir, InstallerPlist)) {
              InstallerPlist = L"\\com.apple.boot.S\\SystemVersion.plist"; // 10.12+
              if (!FileExists (Entry->Volume->RootDir, InstallerPlist) &&
                  (FileExists (Entry->Volume->RootDir, L"\\com.apple.boot.R\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                   FileExists (Entry->Volume->RootDir, L"\\com.apple.boot.P\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                   FileExists (Entry->Volume->RootDir, L"\\com.apple.boot.S\\System\\Library\\PrelinkedKernels\\prelinkedkernel"))) {
                InstallerPlist = L"\\System\\Library\\CoreServices\\SystemVersion.plist"; // 10.11
              }
            }
          }
        }
      }
      if (FileExists (Entry->Volume->RootDir, InstallerPlist)) {
        Status = egLoadFile(Entry->Volume->RootDir, InstallerPlist, (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = Dict->propertyForKey("ProductVersion");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in ProductVersion\n");
            }else{
              if( Prop->getString()->stringValue().notEmpty() ) {
                OSVersion = Prop->getString()->stringValue();
              }
            }
          }
          Prop = Dict->propertyForKey("ProductBuildVersion");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
            }else{
              if( Prop->getString()->stringValue().notEmpty() ) {
                Entry->BuildVersion = Prop->getString()->stringValue();
              }
            }
          }
          // In InstallInfo.plist, there is no a version key only when updating from AppStore in 10.13+
          // If use the startosinstall in 10.13+, this version key exists in InstallInfo.plist
          DictPointer = Dict->dictPropertyForKey("System Image Info"); // 10.12+
          if (DictPointer != NULL) {
            Prop = DictPointer->propertyForKey("version");
            if ( Prop != NULL ) {
              if ( !Prop->isString() ) {
                MsgLog("ATTENTION : property not string in version\n");
              }else{
                OSVersion = Prop->getString()->stringValue();
              }
            }
          }
          Dict->FreeTag();
        }
      }
    }

    // 2nd stage - 2
    // Check for ia.log - InstallESD/createinstallmedia/startosinstall
    // Implemented by Sherlocks
    if (OSVersion.isEmpty()) {
      CONST CHAR8  *s, *fileBuffer;
//      CHAR8  *Res5 = (__typeof__(Res5))AllocateZeroPool(5);
//      CHAR8  *Res6 = (__typeof__(Res6))AllocateZeroPool(6);
//      CHAR8  *Res7 = (__typeof__(Res7))AllocateZeroPool(7);
//      CHAR8  *Res8 = (__typeof__(Res8))AllocateZeroPool(8);
      UINTN  fileLen = 0;
      XStringW InstallerLog;
      InstallerLog = L"\\Mac OS X Install Data\\ia.log"_XSW; // 10.7
      if (!FileExists (Entry->Volume->RootDir, InstallerLog)) {
        InstallerLog = L"\\OS X Install Data\\ia.log"_XSW; // 10.8 - 10.11
        if (!FileExists (Entry->Volume->RootDir, InstallerLog)) {
          InstallerLog = L"\\macOS Install Data\\ia.log"_XSW; // 10.12+
        }
      }
      if (FileExists (Entry->Volume->RootDir, InstallerLog)) {
        Status = egLoadFile(Entry->Volume->RootDir, InstallerLog.wc_str(), (UINT8 **)&fileBuffer, &fileLen);
        if (!EFI_ERROR(Status)) {
          XString8 targetString;
          targetString.strncpy(fileBuffer, fileLen);
      //    s = SearchString(targetString, fileLen, "Running OS Build: Mac OS X ", 27);
          s = AsciiStrStr(targetString.c_str(), "Running OS Build: Mac OS X ");
          if (s[31] == ' ') {
            OSVersion.S8Printf("%c%c.%c\n", s[27], s[28], s[30]);
            if (s[38] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c\n", s[33], s[34], s[35], s[36], s[37]);
            } else if (s[39] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[33], s[34], s[35], s[36], s[37], s[38]);
            }
          } else if (s[31] == '.') {
            OSVersion.S8Printf("%c%c.%c.%c\n", s[27], s[28], s[30], s[32]);
            if (s[40] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c\n", s[35], s[36], s[37], s[38], s[39]);
            } else if (s[41] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[35], s[36], s[37], s[38], s[39], s[40]);
            }
          } else if (s[32] == ' ') {
            OSVersion.S8Printf("%c%c.%c%c\n", s[27], s[28], s[30], s[31]);
            if (s[39] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38]);
            } else if (s[40] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38], s[39]);
            } else if (s[41] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38], s[39], s[40]);
            }
          } else if (s[32] == '.') {
            OSVersion.S8Printf("%c%c.%c%c.%c\n", s[27], s[28], s[30], s[31], s[33]);
            if (s[41] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40]);
            } else if (s[42] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40], s[41]);
            } else if (s[43] == ')') {
              Entry->BuildVersion.S8Printf("%c%c%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40], s[41], s[42]);
            }
          }
          FreePool(fileBuffer);
        }
      }
    }

    // 2nd stage - 3
    // Check for plist - Preboot of APFS
    if ( OSVersion.isEmpty() )
    {
			XStringW plist = L"\\macOS Install Data\\Locked Files\\Boot Files\\SystemVersion.plist"_XSW;
			if ( !FileExists(Entry->Volume->RootDir, plist) ) {
				plist.setEmpty();
			}

      if ( plist.notEmpty() ) { // found macOS System

        Status = egLoadFile(Entry->Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = Dict->propertyForKey("ProductVersion");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in ProductVersion\n");
            }else{
              OSVersion = Prop->getString()->stringValue();
            }
          }
          Prop = Dict->propertyForKey("ProductBuildVersion");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
            }else{
              Entry->BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }
  }

  if (OSTYPE_IS_OSX_RECOVERY (Entry->LoaderType)) {

  	XString8 uuidPrefix;
    if ( Entry->APFSTargetUUID.notEmpty() ) uuidPrefix = S8Printf("\\%ls", Entry->APFSTargetUUID.wc_str());

  	XStringW plist = SWPrintf("%s\\SystemVersion.plist", uuidPrefix.c_str());
		if ( !FileExists(Entry->Volume->RootDir, plist) ) {
			plist = SWPrintf("%s\\ServerVersion.plist", uuidPrefix.c_str());
			if ( !FileExists(Entry->Volume->RootDir, plist) ) {
        plist = L"\\com.apple.recovery.boot\\SystemVersion.plist"_XSW;
        if ( !FileExists(Entry->Volume->RootDir, plist) ) {
          plist = L"\\com.apple.recovery.boot\\ServerVersion.plist"_XSW;
          if ( !FileExists(Entry->Volume->RootDir, plist) ) {
					  plist.setEmpty();
					}
				}
    	}
    }

    // Detect exact version for OS X Recovery
    if ( plist.notEmpty() ) { // found macOS System
      Status = egLoadFile(Entry->Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = Dict->propertyForKey("ProductVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductVersion\n");
          }else{
            OSVersion = Prop->getString()->stringValue();
          }
        }
        Prop = Dict->propertyForKey("ProductBuildVersion");
        if ( Prop != NULL ) {
          if ( !Prop->isString() ) {
            MsgLog("ATTENTION : property not string in ProductBuildVersion\n");
          }else{
            Entry->BuildVersion = Prop->getString()->stringValue();
          }
        }
      }
      Dict->FreeTag();
    } else if (FileExists (Entry->Volume->RootDir, L"\\com.apple.recovery.boot\\boot.efi")) {
      // Special case - com.apple.recovery.boot/boot.efi exists but SystemVersion.plist doesn't --> 10.9 recovery
      OSVersion = "10.9"_XS8;
    }
  }

  if (PlistBuffer != NULL) {
    FreePool(PlistBuffer);
  }

  return OSVersion;
}

//constexpr XStringW iconMac = L"mac"_XSW;
CONST XStringW
GetOSIconName (const XString8& OSVersion)
{
  XStringW OSIconName;
  if (OSVersion.isEmpty()) {
    OSIconName = L"mac"_XSW;
  } else if (OSVersion.contains("10.16") ||
             (OSVersion.contains("11.0")  != 0)) {
    // Big Sur
    OSIconName = L"bigsur,mac"_XSW;
  } else if (OSVersion.contains("10.15") != 0) {
    // Catalina
    OSIconName = L"cata,mac"_XSW;
  } else if (OSVersion.contains("10.14") != 0) {
    // Mojave
    OSIconName = L"moja,mac"_XSW;
  } else if (OSVersion.contains("10.13") != 0) {
    // High Sierra
    OSIconName = L"hsierra,mac"_XSW;
  } else if (OSVersion.contains("10.12") != 0) {
    // Sierra
    OSIconName = L"sierra,mac"_XSW;
  } else if (OSVersion.contains("10.11") != 0) {
    // El Capitan
    OSIconName = L"cap,mac"_XSW;
  } else if (OSVersion.contains("10.10") != 0) {
    // Yosemite
    OSIconName = L"yos,mac"_XSW;
  } else if (OSVersion.contains("10.9") != 0) {
    // Mavericks
    OSIconName = L"mav,mac"_XSW;
  } else if (OSVersion.contains("10.8") != 0) {
    // Mountain Lion
    OSIconName = L"cougar,mac"_XSW;
  } else if (OSVersion.contains("10.7") != 0) {
    // Lion
    OSIconName = L"lion,mac"_XSW;
  } else if (OSVersion.contains("10.6") != 0) {
    // Snow Leopard
    OSIconName = L"snow,mac"_XSW;
  } else if (OSVersion.contains("10.5") != 0) {
    // Leopard
    OSIconName = L"leo,mac"_XSW;
  } else if (OSVersion.contains("10.4") != 0) {
    // Tiger
    OSIconName = L"tiger,mac"_XSW;
  } else {
    OSIconName = L"mac"_XSW;
  }

  return OSIconName;
}

//Get the UUID of the AppleRaid or CoreStorage volume from the boot helper partition
EFI_STATUS
GetRootUUID (IN  REFIT_VOLUME *Volume)
{
  EFI_STATUS Status;
  CHAR8      *PlistBuffer;
  UINTN      PlistLen;
  TagDict*     Dict;
  const TagStruct*     Prop;

  CONST CHAR16*    SystemPlistR;
  CONST CHAR16*    SystemPlistP;
  CONST CHAR16*    SystemPlistS;

  BOOLEAN    HasRock;
  BOOLEAN    HasPaper;
  BOOLEAN    HasScissors;

  Status = EFI_NOT_FOUND;
  if (Volume == NULL) {
    return EFI_NOT_FOUND;
  }

  SystemPlistR = L"\\com.apple.boot.R\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist";
  if (FileExists (Volume->RootDir, SystemPlistR)) {
    HasRock      = FileExists (Volume->RootDir,     SystemPlistR);
  } else {
    SystemPlistR = L"\\com.apple.boot.R\\com.apple.Boot.plist";
    HasRock      = FileExists (Volume->RootDir,     SystemPlistR);
  }

  SystemPlistP = L"\\com.apple.boot.P\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist";
  if (FileExists (Volume->RootDir, SystemPlistP)) {
    HasPaper     = FileExists (Volume->RootDir,     SystemPlistP);
  } else {
    SystemPlistP = L"\\com.apple.boot.P\\com.apple.Boot.plist";
    HasPaper     = FileExists (Volume->RootDir,     SystemPlistP);
  }

  SystemPlistS = L"\\com.apple.boot.S\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist";
  if (FileExists (Volume->RootDir, SystemPlistS)) {
    HasScissors  = FileExists (Volume->RootDir,     SystemPlistS);
  } else {
    SystemPlistS = L"\\com.apple.boot.S\\com.apple.Boot.plist";
    HasScissors  = FileExists (Volume->RootDir,     SystemPlistS);
  }

  PlistBuffer = NULL;
  // Playing Rock, Paper, Scissors to chose which settings to load.
  if (HasRock && HasPaper && HasScissors) {
    // Rock wins when all three are around
    Status = egLoadFile(Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasRock && HasPaper) {
    // Paper beats rock
    Status = egLoadFile(Volume->RootDir, SystemPlistP, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasRock && HasScissors) {
    // Rock beats scissors
    Status = egLoadFile(Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasPaper && HasScissors) {
    // Scissors beat paper
    Status = egLoadFile(Volume->RootDir, SystemPlistS, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasPaper) {
    // No match
    Status = egLoadFile(Volume->RootDir, SystemPlistP, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasScissors) {
    // No match
    Status = egLoadFile(Volume->RootDir, SystemPlistS, (UINT8 **)&PlistBuffer, &PlistLen);
  } else {
    // Rock wins by default
    Status = egLoadFile(Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  }

  if (!EFI_ERROR(Status)) {
    Dict = NULL;
    if (ParseXML(PlistBuffer, &Dict, 0) != EFI_SUCCESS) {
      FreePool(PlistBuffer);
      return EFI_NOT_FOUND;
    }

    Prop = Dict->propertyForKey("Root UUID");
    if ( Prop != NULL ) {
      if ( !Prop->isString() ) {
        MsgLog("ATTENTION : property not string in Root UUID\n");
      }else{
        Status = StrToGuidLE(Prop->getString()->stringValue(), &Volume->RootUUID);
      }
    }

    Dict->FreeTag();
    FreePool(PlistBuffer);
  }

  return Status;
}


VOID
GetDevices ()
{
  EFI_STATUS          Status;
  UINTN               HandleCount  = 0;
  EFI_HANDLE          *HandleArray = NULL;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;
  UINTN               Index;
  UINTN               Segment      = 0;
  UINTN               Bus          = 0;
  UINTN               Device       = 0;
  UINTN               Function     = 0;
  UINTN               i;
  UINT32              Bar0;
  //  UINT8               *Mmio        = NULL;
  radeon_card_info_t  *info;
  SLOT_DEVICE         *SlotDevice;

  NGFX = 0;
  NHDA = 0;
  AudioNum = 0;
  //Arpt.Valid = FALSE; //global variables initialized by 0 - c-language
  XStringW GopDevicePathStr;
  XStringW DevicePathStr;

  DbgHeader("GetDevices");

  // Get GOP handle, in order to check to which GPU the monitor is currently connected
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleArray);
  if (!EFI_ERROR(Status)) {
    GopDevicePathStr = DevicePathToXStringW(DevicePathFromHandle(HandleArray[0]));
    DBG("GOP found at: %ls\n", GopDevicePathStr.wc_str());
  }

  // Scan PCI handles
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleArray
                                    );

  if (!EFI_ERROR(Status)) {
    for (Index = 0; Index < HandleCount; ++Index) {
      Status = gBS->HandleProtocol(HandleArray[Index], &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
      if (!EFI_ERROR(Status)) {
        // Read PCI BUS
        PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        Status = PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  sizeof (Pci) / sizeof (UINT32),
                                  &Pci
                                  );

		  DBG("PCI (%02llX|%02llX:%02llX.%02llX) : %04hX %04hX class=%02hhX%02hhX%02hhX\n",
             Segment,
             Bus,
             Device,
             Function,
             Pci.Hdr.VendorId,
             Pci.Hdr.DeviceId,
             Pci.Hdr.ClassCode[2],
             Pci.Hdr.ClassCode[1],
             Pci.Hdr.ClassCode[0]
             );

        // GFX
        //if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
        //    (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA) &&
        //    (NGFX < 4)) {

        if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
            ((Pci.Hdr.ClassCode[1] == (PCI_CLASS_DISPLAY_VGA)) ||
             (Pci.Hdr.ClassCode[1] == (PCI_CLASS_DISPLAY_OTHER))) &&
            (NGFX < 4)) {
          CONST CHAR8 *CardFamily = "";
          UINT16 UFamily;
          GFX_PROPERTIES *gfx = &gGraphics[NGFX];

          // GOP device path should contain the device path of the GPU to which the monitor is connected
          DevicePathStr = DevicePathToXStringW(DevicePathFromHandle(HandleArray[Index]));
          if (StrStr(GopDevicePathStr.wc_str(), DevicePathStr.wc_str())) {
            DBG(" - GOP: Provided by device\n");
            if (NGFX != 0) {
               // we found GOP on a GPU scanned later, make space for this GPU at first position
               for (i=NGFX; i>0; i--) {
                 CopyMem(&gGraphics[i], &gGraphics[i-1], sizeof(GFX_PROPERTIES));
               }
               ZeroMem(&gGraphics[0], sizeof(GFX_PROPERTIES));
               gfx = &gGraphics[0]; // GPU with active GOP will be added at the first position
            }
          }

          gfx->DeviceID       = Pci.Hdr.DeviceId;
          gfx->Segment        = Segment;
          gfx->Bus            = Bus;
          gfx->Device         = Device;
          gfx->Function       = Function;
          gfx->Handle         = HandleArray[Index];

          switch (Pci.Hdr.VendorId) {
            case 0x1002:
              info        = NULL;
              gfx->Vendor = Ati;

              i = 0;
              do {
                info      = &radeon_cards[i];
                if (info->device_id == Pci.Hdr.DeviceId) {
                  break;
                }
              } while (radeon_cards[i++].device_id != 0);

				  snprintf (gfx->Model,  64, "%s", info->model_name);
				  snprintf (gfx->Config, 64, "%s", card_configs[info->cfg_name].name);
              gfx->Ports                  = card_configs[info->cfg_name].ports;
              DBG(" - GFX: Model=%s (ATI/AMD)\n", gfx->Model);

              //get mmio
              if (info->chip_family < CHIP_FAMILY_HAINAN) {
                gfx->Mmio = (UINT8 *)(UINTN)(Pci.Device.Bar[2] & ~0x0f);
              } else {
                gfx->Mmio = (UINT8 *)(UINTN)(Pci.Device.Bar[5] & ~0x0f);
              }
              gfx->Connectors = *(UINT32*)(gfx->Mmio + RADEON_BIOS_0_SCRATCH);
              //           DBG(" - RADEON_BIOS_0_SCRATCH = 0x%08X\n", gfx->Connectors);
              gfx->ConnChanged = FALSE;

              SlotDevice                  = &SlotDevices[0];
              SlotDevice->SegmentGroupNum = (UINT16)Segment;
              SlotDevice->BusNum          = (UINT8)Bus;
              SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
              SlotDevice->Valid           = TRUE;
              snprintf (SlotDevice->SlotName, 31, "PCI Slot 0");
              SlotDevice->SlotID          = 1;
              SlotDevice->SlotType        = SlotTypePciExpressX16;
              break;

            case 0x8086:
              gfx->Vendor                 = Intel;
				  snprintf (gfx->Model, 64, "%s", get_gma_model (Pci.Hdr.DeviceId));
              DBG(" - GFX: Model=%s (Intel)\n", gfx->Model);
              gfx->Ports = 1;
              gfx->Connectors = (1 << NGFX);
              gfx->ConnChanged = FALSE;
              break;

            case 0x10de:
              gfx->Vendor = Nvidia;
              Bar0        = Pci.Device.Bar[0];
              gfx->Mmio   = (UINT8*)(UINTN)(Bar0 & ~0x0f);
              //DBG("BAR: 0x%p\n", Mmio);
              // get card type
              gfx->Family = (REG32(gfx->Mmio, 0) >> 20) & 0x1ff;
              UFamily = gfx->Family & 0x1F0;
              if ((UFamily == NV_ARCH_KEPLER1) ||
                  (UFamily == NV_ARCH_KEPLER2) ||
                  (UFamily == NV_ARCH_KEPLER3)) {
                CardFamily = "Kepler";
              }
              else if ((UFamily == NV_ARCH_FERMI1) ||
                       (UFamily == NV_ARCH_FERMI2)) {
                CardFamily = "Fermi";
              }
              else if ((UFamily == NV_ARCH_MAXWELL1) ||
                       (UFamily == NV_ARCH_MAXWELL2)) {
                CardFamily = "Maxwell";
              }
              else if (UFamily == NV_ARCH_PASCAL) {
                CardFamily = "Pascal";
              }
              else if (UFamily == NV_ARCH_VOLTA) {
                CardFamily = "Volta";
              }
              else if (UFamily == NV_ARCH_TURING) {
                CardFamily = "Turing";
              }
              else if ((UFamily >= NV_ARCH_TESLA) && (UFamily < 0xB0)) { //not sure if 0xB0 is Tesla or Fermi
                CardFamily = "Tesla";
              } else {
                CardFamily = "NVidia unknown";
              }

              snprintf (
                          gfx->Model,
                          64,
                          "%s",
                          get_nvidia_model (((Pci.Hdr.VendorId << 16) | Pci.Hdr.DeviceId),
                                            ((Pci.Device.SubsystemVendorID << 16) | Pci.Device.SubsystemID),
                                             NULL) //NULL: get from generic lists
                          );

				  DBG(" - GFX: Model=%s family %hX (%s)\n", gfx->Model, gfx->Family, CardFamily);
              gfx->Ports                  = 0;

              SlotDevice                  = &SlotDevices[1];
              SlotDevice->SegmentGroupNum = (UINT16)Segment;
              SlotDevice->BusNum          = (UINT8)Bus;
              SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
              SlotDevice->Valid           = TRUE;
              snprintf (SlotDevice->SlotName, 31, "PCI Slot 0");
              SlotDevice->SlotID          = 1;
              SlotDevice->SlotType        = SlotTypePciExpressX16;
              break;

            default:
              gfx->Vendor = Unknown;
				  snprintf (gfx->Model, 64, "pci%hx,%hx", Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
              gfx->Ports  = 1;
              gfx->Connectors = (1 << NGFX);
              gfx->ConnChanged = FALSE;

              break;
          }

          NGFX++;
        }   //if gfx

        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_OTHER)) {
          SlotDevice                  = &SlotDevices[6];
          SlotDevice->SegmentGroupNum = (UINT16)Segment;
          SlotDevice->BusNum          = (UINT8)Bus;
          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
          SlotDevice->Valid           = TRUE;
          snprintf (SlotDevice->SlotName, 31, "AirPort");
          SlotDevice->SlotID          = 0;
          SlotDevice->SlotType        = SlotTypePciExpressX1;
          DBG(" - WIFI: Vendor= ");
          switch (Pci.Hdr.VendorId) {
            case 0x11ab:
              DBG("Marvell\n");
              break;
            case 0x10ec:
              DBG("Realtek\n");
              break;
            case 0x14e4:
              DBG("Broadcom\n");
              break;
            case 0x1969:
            case 0x168C:
              DBG("Atheros\n");
              break;
            case 0x1814:
              DBG("Ralink\n");
              break;
            case 0x8086:
              DBG("Intel\n");
              break;

            default:
              DBG(" 0x%04X\n", Pci.Hdr.VendorId);
              break;
          }
        }

        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_ETHERNET)) {
          SlotDevice                  = &SlotDevices[5];
          SlotDevice->SegmentGroupNum = (UINT16)Segment;
          SlotDevice->BusNum          = (UINT8)Bus;
          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
          SlotDevice->Valid           = TRUE;
          snprintf (SlotDevice->SlotName, 31, "Ethernet");
          SlotDevice->SlotID          = 2;
          SlotDevice->SlotType        = SlotTypePciExpressX1;
          gLanVendor[nLanCards]       = Pci.Hdr.VendorId;
          Bar0                        = Pci.Device.Bar[0];
          gLanMmio[nLanCards++]       = (UINT8*)(UINTN)(Bar0 & ~0x0f);
          if (nLanCards >= 4) {
            DBG(" - [!] too many LAN card in the system (upto 4 limit exceeded), overriding the last one\n");
            nLanCards = 3; // last one will be rewritten
          }
          DBG(" - LAN: %llu Vendor=", nLanCards-1);
          switch (Pci.Hdr.VendorId) {
            case 0x11ab:
              DBG("Marvell\n");
              break;
            case 0x10ec:
              DBG("Realtek\n");
              break;
            case 0x14e4:
              DBG("Broadcom\n");
              break;
            case 0x1969:
            case 0x168C:
              DBG("Atheros\n");
              break;
            case 0x8086:
              DBG("Intel\n");
              break;
            case 0x10de:
              DBG("Nforce\n");
              break;

            default:
              DBG("Unknown\n");
              break;
          }
        }

        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_FIREWIRE)) {
          SlotDevice = &SlotDevices[12];
          SlotDevice->SegmentGroupNum = (UINT16)Segment;
          SlotDevice->BusNum          = (UINT8)Bus;
          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
          SlotDevice->Valid           = TRUE;
          snprintf (SlotDevice->SlotName, 31, "FireWire");
          SlotDevice->SlotID          = 3;
          SlotDevice->SlotType        = SlotTypePciExpressX4;
        }

        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
                 ((Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA) ||
                  (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_AUDIO)) &&
                 (NHDA < 4)) {
          HDA_PROPERTIES *hda = &gAudios[NHDA];

          // Populate Controllers IDs
          hda->controller_vendor_id       = Pci.Hdr.VendorId;
          hda->controller_device_id       = Pci.Hdr.DeviceId;

          // HDA Controller Info
          HdaControllerGetName(((hda->controller_device_id << 16) | hda->controller_vendor_id), &hda->controller_name);
 

          if (IsHDMIAudio(HandleArray[Index])) {
            DBG(" - HDMI Audio: \n");

            SlotDevice = &SlotDevices[4];
            SlotDevice->SegmentGroupNum = (UINT16)Segment;
            SlotDevice->BusNum          = (UINT8)Bus;
            SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
            SlotDevice->Valid           = TRUE;
            snprintf (SlotDevice->SlotName, 31, "HDMI port");
            SlotDevice->SlotID          = 5;
            SlotDevice->SlotType        = SlotTypePciExpressX4;
          }
          if (gSettings.ResetHDA) {
            //Slice method from VoodooHDA
            //PCI_HDA_TCSEL_OFFSET = 0x44
            UINT8 Value = 0;
            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);

            if (EFI_ERROR(Status)) {
              continue;
            }

            Value &= 0xf8;
            PciIo->Pci.Write (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);
            //ResetControllerHDA();
          }
          NHDA++;
        } // if Audio device
      }
    }
  }
}


VOID
SetDevices (LOADER_ENTRY *Entry)
{
  //  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *modeInfo;
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;
  UINTN               HandleCount;
  UINTN               i, j;
  EFI_HANDLE          *HandleBuffer;
  pci_dt_t            PCIdevice;
  UINTN               Segment;
  UINTN               Bus;
  UINTN               Device;
  UINTN               Function;
  BOOLEAN             StringDirty = FALSE;
  BOOLEAN             TmpDirty    = FALSE;
  UINT16              PmCon;
  UINT32              Rcba;
  UINT32              Hptc;
  DEV_PROPERTY *Prop = NULL;
  DEV_PROPERTY *Prop2 = NULL;
  DevPropDevice *device = NULL;

  GetEdidDiscovered ();

  //First make string from Device->Properties
  Prop = gSettings.ArbProperties;
  device = NULL;
  if (!device_inject_string) {
    device_inject_string = devprop_create_string();
  }
  while (Prop) {
    if (Prop->Device != 0) {
      Prop = Prop->Next; //skip CustomProperties
      continue;
    }
    device = devprop_add_device_pci(device_inject_string, NULL, Prop->DevicePath);
    DBG("add device: %ls\n", DevicePathToXStringW(Prop->DevicePath).wc_str());
    Prop2 = Prop->Child;
    while (Prop2) {
      if (Prop2->MenuItem.BValue) {
        if (AsciiStrStr(Prop2->Key, "-platform-id") != NULL) {
          if (gSettings.IgPlatform == 0 && Prop2->Value) {
            CopyMem((UINT8*)&gSettings.IgPlatform, (UINT8*)Prop2->Value, Prop2->ValueLen);
          }
          devprop_add_value(device, Prop2->Key, (UINT8*)&gSettings.IgPlatform, 4);
          DBG("   Add key=%s valuelen=%llu\n", Prop2->Key, Prop2->ValueLen);
        } else if ((AsciiStrStr(Prop2->Key, "override-no-edid") || AsciiStrStr(Prop2->Key, "override-no-connect"))
          && gSettings.InjectEDID && gSettings.CustomEDID) {
          // special case for EDID properties
          devprop_add_value(device, Prop2->Key, gSettings.CustomEDID, 128);
          DBG("   Add key=%s from custom EDID\n", Prop2->Key);
        } else {
          devprop_add_value(device, Prop2->Key, (UINT8*)Prop2->Value, Prop2->ValueLen);
          DBG("   Add key=%s valuelen=%llu\n", Prop2->Key, Prop2->ValueLen);
        }
      }

      StringDirty = TRUE;
      Prop2 = Prop2->Next;

    }
    Prop = Prop->Next;
  }

  devices_number = 1; //should initialize for reentering GUI
  // Scan PCI handles
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer
                                    );

  if (!EFI_ERROR(Status)) {
    for (i = 0; i < HandleCount; i++) {
      Status = gBS->HandleProtocol (HandleBuffer[i], &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
      if (!EFI_ERROR(Status)) {
        // Read PCI BUS
        Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
        if (EFI_ERROR(Status)) {
          continue;
        }

        Status                               = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        PCIdevice.DeviceHandle               = HandleBuffer[i];
        PCIdevice.dev.addr                   = (UINT32)PCIADDR(Bus, Device, Function);
        PCIdevice.vendor_id                  = Pci.Hdr.VendorId;
        PCIdevice.device_id                  = Pci.Hdr.DeviceId;
        PCIdevice.revision                   = Pci.Hdr.RevisionID;
        PCIdevice.subclass                   = Pci.Hdr.ClassCode[0];
        PCIdevice.class_id                   = *((UINT16*)(Pci.Hdr.ClassCode+1));
        PCIdevice.subsys_id.subsys.vendor_id = Pci.Device.SubsystemVendorID;
        PCIdevice.subsys_id.subsys.device_id = Pci.Device.SubsystemID;
        PCIdevice.used                       = FALSE;

        //if (gSettings.NrAddProperties == 0xFFFE) {  //yyyy it means Arbitrary
        //------------------
        Prop = gSettings.ArbProperties;  //check for additional properties
        device = NULL;
        /*       if (!string) {
         string = devprop_create_string();
         } */
        while (Prop) {
          if (Prop->Device != PCIdevice.dev.addr) {
            Prop = Prop->Next;
            continue;
          }
          if (!PCIdevice.used) {
            device = devprop_add_device_pci(device_inject_string, &PCIdevice, NULL);
            PCIdevice.used = TRUE;
          }
          //special corrections
          if (Prop->MenuItem.BValue) {
            if (AsciiStrStr(Prop->Key, "-platform-id") != NULL) {
              devprop_add_value(device, Prop->Key, (UINT8*)&gSettings.IgPlatform, 4);
            } else {
              devprop_add_value(device, Prop->Key, (UINT8*)Prop->Value, Prop->ValueLen);
            }
          }

          StringDirty = TRUE;
          Prop = Prop->Next;
        }
        //------------------
        if (PCIdevice.used) {
          DBG("custom properties for device %02llX:%02llX.%02llX injected\n", Bus, Device, Function);
          //continue;
        }
        //}

        // GFX
        if (/* gSettings.GraphicsInjector && */
            (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
            ((Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA) ||
             (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_OTHER))) {
          //gGraphics.DeviceID = Pci.Hdr.DeviceId;

          switch (Pci.Hdr.VendorId) {
            case 0x1002:
              if (gSettings.InjectATI) {
                //can't do this in one step because of C-conventions
                TmpDirty    = setup_ati_devprop(Entry, &PCIdevice);
                StringDirty |=  TmpDirty;
              } else {
                MsgLog ("ATI injection not set\n");
              }

              for (j = 0; j < 4; j++) {
                if (gGraphics[j].Handle == PCIdevice.DeviceHandle) {
                  if (gGraphics[j].ConnChanged) {
                    *(UINT32*)(gGraphics[j].Mmio + RADEON_BIOS_0_SCRATCH) = gGraphics[j].Connectors;
                  }
                  break;
                }
              }

              if (gSettings.DeInit) {
                for (j = 0; j < 4; j++) {
                  if (gGraphics[j].Handle == PCIdevice.DeviceHandle) {
                    *(UINT32*)(gGraphics[j].Mmio + 0x6848) = 0; //EVERGREEN_GRPH_FLIP_CONTROL, 1<<0 SURFACE_UPDATE_H_RETRACE_EN
                    *(UINT32*)(gGraphics[j].Mmio + 0x681C) = 0; //EVERGREEN_GRPH_PRIMARY_SURFACE_ADDRESS_HIGH
                    *(UINT32*)(gGraphics[j].Mmio + 0x6820) = 0; //EVERGREEN_GRPH_SECONDARY_SURFACE_ADDRESS_HIGH
                    *(UINT32*)(gGraphics[j].Mmio + 0x6808) = 0; //EVERGREEN_GRPH_LUT_10BIT_BYPASS_CONTROL, EVERGREEN_LUT_10BIT_BYPASS_EN  (1 << 8)
                    *(UINT32*)(gGraphics[j].Mmio + 0x6800) = 1; //EVERGREEN_GRPH_ENABLE
                    *(UINT32*)(gGraphics[j].Mmio + 0x6EF8) = 0; //EVERGREEN_MASTER_UPDATE_MODE
                    //*(UINT32*)(gGraphics[j].Mmio + R600_BIOS_0_SCRATCH) = 0x00810000;
                    DBG("Device %llu deinited\n", j);
                  }
                }
              }
              break;

            case 0x8086:
              if (gSettings.InjectIntel) {
                TmpDirty    = setup_gma_devprop(Entry, &PCIdevice);
                StringDirty |=  TmpDirty;
                MsgLog ("Intel GFX revision  = 0x%hhX\n", PCIdevice.revision);
              } else {
                MsgLog ("Intel GFX injection not set\n");
              }

              // IntelBacklight reworked by Sherlocks. 2018.10.07
              if (gSettings.IntelBacklight || gSettings.IntelMaxBacklight) {
                UINT32 LEV2 = 0, LEVL = 0, P0BL = 0, GRAN = 0;
                UINT32 LEVW = 0, LEVX = 0, LEVD = 0, PCHL = 0;
                UINT32 ShiftLEVX = 0, FBLEVX = 0;
                UINT32 SYSLEVW = 0x80000000;
                UINT32 MACLEVW = 0xC0000000;

                MsgLog ("Intel GFX IntelBacklight\n");
                // Read LEV2
                /*Status = */PciIo->Mem.Read(
                                             PciIo,
                                             EfiPciIoWidthUint32,
                                             0,
                                             0x48250,
                                             1,
                                             &LEV2
                                             );
                // Read LEVL
                /*Status = */PciIo->Mem.Read(
                                             PciIo,
                                             EfiPciIoWidthUint32,
                                             0,
                                             0x48254,
                                             1,
                                             &LEVL
                                             );
                // Read P0BL -- what is the sense to read if not used?
                /*Status = */PciIo->Mem.Read(
                                             PciIo,
                                             EfiPciIoWidthUint32,
                                             0,
                                             0x70040,
                                             1,
                                             &P0BL
                                             );
                // Read GRAN
                /*Status = */PciIo->Mem.Read(
                                             PciIo,
                                             EfiPciIoWidthUint32,
                                             0,
                                             0xC2000,
                                             1,
                                             &GRAN
                                             );
                // Read LEVW
                /*Status = */PciIo->Mem.Read(
                                             PciIo,
                                             EfiPciIoWidthUint32,
                                             0,
                                             0xC8250,
                                             1,
                                             &LEVW
                                             );
                // Read LEVX
                /*Status = */PciIo->Mem.Read(
                                             PciIo,
                                             EfiPciIoWidthUint32,
                                             0,
                                             0xC8254,
                                             1,
                                             &LEVX
                                             );
                ShiftLEVX = LEVX >> 16;
                // Read LEVD
                /*Status = */PciIo->Mem.Read(
                                             PciIo,
                                             EfiPciIoWidthUint32,
                                             0,
                                             0xC8258,
                                             1,
                                             &LEVD
                                             );
                // Read PCHL
                /*Status = */PciIo->Mem.Read(
                                             PciIo,
                                             EfiPciIoWidthUint32,
                                             0,
                                             0xE1180,
                                             1,
                                             &PCHL
                                             );
                MsgLog ("  LEV2 = 0x%X, LEVL = 0x%X, P0BL = 0x%X, GRAN = 0x%X\n", LEV2, LEVL, P0BL, GRAN);
                MsgLog ("  LEVW = 0x%X, LEVX = 0x%X, LEVD = 0x%X, PCHL = 0x%X\n", LEVW, LEVX, LEVD, PCHL);

                // Maximum brightness level of each framebuffers
                //  Sandy Bridge/Ivy Bridge: 0x0710
                //  Haswell/Broadwell: 0x056C/0x07A1/0x0AD9/0x1499
                //  Skylake/KabyLake: 0x056C
                //  Coffee Lake: 0xFFFF
                switch (Pci.Hdr.DeviceId) {
                  case 0x0102: // "Intel HD Graphics 2000"
                  case 0x0106: // "Intel HD Graphics 2000"
                  case 0x010A: // "Intel HD Graphics P3000"
                  case 0x0112: // "Intel HD Graphics 3000"
                  case 0x0116: // "Intel HD Graphics 3000"
                  case 0x0122: // "Intel HD Graphics 3000"
                  case 0x0126: // "Intel HD Graphics 3000"
                    if (gSettings.IgPlatform) {
                      switch (gSettings.IgPlatform) {
                        case (UINT32)0x00030010:
                        case (UINT32)0x00050000:
                          FBLEVX = 0xFFFF;
                          break;
                        default:
                          FBLEVX = 0x0710;
                          break;
                      }
                    } else {
                      FBLEVX = 0x0710;
                    }
                    break;

                  case 0x0152: // "Intel HD Graphics 2500"
                  case 0x0156: // "Intel HD Graphics 2500"
                  case 0x015A: // "Intel HD Graphics 2500"
                  case 0x0162: // "Intel HD Graphics 4000"
                  case 0x0166: // "Intel HD Graphics 4000"
                  case 0x016A: // "Intel HD Graphics P4000"
                    FBLEVX = 0x0710;
                    break;

                  case 0x0412: // "Intel HD Graphics 4600"
                  case 0x0416: // "Intel HD Graphics 4600"
                  case 0x041A: // "Intel HD Graphics P4600"
                  case 0x041E: // "Intel HD Graphics 4400"
                  case 0x0422: // "Intel HD Graphics 5000"
                  case 0x0426: // "Intel HD Graphics 5000"
                  case 0x042A: // "Intel HD Graphics 5000"
                  case 0x0A06: // "Intel HD Graphics"
                  case 0x0A16: // "Intel HD Graphics 4400"
                  case 0x0A1E: // "Intel HD Graphics 4200"
                  case 0x0A22: // "Intel Iris Graphics 5100"
                  case 0x0A26: // "Intel HD Graphics 5000"
                  case 0x0A2A: // "Intel Iris Graphics 5100"
                  case 0x0A2B: // "Intel Iris Graphics 5100"
                  case 0x0A2E: // "Intel Iris Graphics 5100"
                  case 0x0D12: // "Intel HD Graphics 4600"
                  case 0x0D16: // "Intel HD Graphics 4600"
                  case 0x0D22: // "Intel Iris Pro Graphics 5200"
                  case 0x0D26: // "Intel Iris Pro Graphics 5200"
                  case 0x0D2A: // "Intel Iris Pro Graphics 5200"
                  case 0x0D2B: // "Intel Iris Pro Graphics 5200"
                  case 0x0D2E: // "Intel Iris Pro Graphics 5200"
                    if (gSettings.IgPlatform) {
                      switch (gSettings.IgPlatform) {
                        case (UINT32)0x04060000:
                        case (UINT32)0x0c060000:
                        case (UINT32)0x04160000:
                        case (UINT32)0x0c160000:
                        case (UINT32)0x04260000:
                        case (UINT32)0x0c260000:
                        case (UINT32)0x0d260000:
                        case (UINT32)0x0d220003:
                          FBLEVX = 0x1499;
                          break;
                        case (UINT32)0x0a160000:
                        case (UINT32)0x0a260000:
                        case (UINT32)0x0a260005:
                        case (UINT32)0x0a260006:
                          FBLEVX = 0x0AD9;
                          break;
                        case (UINT32)0x0d260007:
                          FBLEVX = 0x07A1;
                          break;
                        case (UINT32)0x04120004:
                        case (UINT32)0x0412000b:
                          break;
                        default:
                          FBLEVX = 0x056C;
                          break;
                      }
                    } else {
                      switch (Pci.Hdr.DeviceId) {
                        case 0x0406:
                        case 0x0C06:
                        case 0x0416:
                        case 0x0C16:
                        case 0x0426:
                        case 0x0C26:
                        case 0x0D22:
                          FBLEVX = 0x1499;
                          break;
                        case 0x0A16:
                        case 0x0A26:
                          FBLEVX = 0x0AD9;
                          break;
                        case 0x0D26:
                          FBLEVX = 0x07A1;
                          break;
                        default:
                          FBLEVX = 0x056C;
                          break;
                      }
                    }
                    break;

                  case 0x1612: // "Intel HD Graphics 5600"
                  case 0x1616: // "Intel HD Graphics 5500"
                  case 0x161E: // "Intel HD Graphics 5300"
                  case 0x1626: // "Intel HD Graphics 6000"
                  case 0x162B: // "Intel Iris Graphics 6100"
                  case 0x162D: // "Intel Iris Pro Graphics P6300"
                  case 0x1622: // "Intel Iris Pro Graphics 6200"
                  case 0x162A: // "Intel Iris Pro Graphics P6300"
                    if (gSettings.IgPlatform) {
                      switch (gSettings.IgPlatform) {
                        case (UINT32)0x16060000:
                        case (UINT32)0x160e0000:
                        case (UINT32)0x16160000:
                        case (UINT32)0x161e0000:
                        case (UINT32)0x16220000:
                        case (UINT32)0x16260000:
                        case (UINT32)0x162b0000:
                        case (UINT32)0x16260004:
                        case (UINT32)0x162b0004:
                        case (UINT32)0x16220007:
                        case (UINT32)0x16260008:
                        case (UINT32)0x162b0008:
                          FBLEVX = 0x1499;
                          break;
                        case (UINT32)0x16260005:
                        case (UINT32)0x16260006:
                          FBLEVX = 0x0AD9;
                          break;
                        case (UINT32)0x16120003:
                          FBLEVX = 0x07A1;
                          break;
                        default:
                          FBLEVX = 0x056C;
                          break;
                      }
                    } else {
                      switch (Pci.Hdr.DeviceId) {
                        case 0x1606:
                        case 0x160E:
                        case 0x1616:
                        case 0x161E:
                        case 0x1622:
                          FBLEVX = 0x1499;
                          break;
                        case 0x1626:
                          FBLEVX = 0x0AD9;
                          break;
                        case 0x1612:
                          FBLEVX = 0x07A1;
                          break;
                        default:
                          FBLEVX = 0x056C;
                          break;
                      }
                    }
                    break;

                  case 0x1902: // "Intel HD Graphics 510"
                  case 0x1906: // "Intel HD Graphics 510"
                  case 0x190B: // "Intel HD Graphics 510"
                  case 0x1912: // "Intel HD Graphics 530"
                  case 0x1916: // "Intel HD Graphics 520"
                  case 0x191B: // "Intel HD Graphics 530"
                  case 0x191D: // "Intel HD Graphics P530"
                  case 0x191E: // "Intel HD Graphics 515"
                  case 0x1921: // "Intel HD Graphics 520"
                  case 0x1923: // "Intel HD Graphics 535"
                  case 0x1926: // "Intel Iris Graphics 540"
                  case 0x1927: // "Intel Iris Graphics 550"
                  case 0x192B: // "Intel Iris Graphics 555"
                  case 0x192D: // "Intel Iris Graphics P555"
                  case 0x1932: // "Intel Iris Pro Graphics 580"
                  case 0x193A: // "Intel Iris Pro Graphics P580"
                  case 0x193B: // "Intel Iris Pro Graphics 580"
                  case 0x193D: // "Intel Iris Pro Graphics P580"
                    if (gSettings.IgPlatform) {
                      switch (gSettings.IgPlatform) {
                        case (UINT32)0x19120001:
                        FBLEVX = 0xFFFF;
                        break;
                      default:
                        FBLEVX = 0x056C;
                        break;
                      }
                    } else {
                      FBLEVX = 0x056C;
                    }
                    break;

                  case 0x5902: // "Intel HD Graphics 610"
                  case 0x5906: // "Intel HD Graphics 610"
                  case 0x5912: // "Intel HD Graphics 630"
                  case 0x5916: // "Intel HD Graphics 620"
                  case 0x591A: // "Intel HD Graphics P630"
                  case 0x591B: // "Intel HD Graphics 630"
                  case 0x591D: // "Intel HD Graphics P630"
                  case 0x591E: // "Intel HD Graphics 615"
                  case 0x5923: // "Intel HD Graphics 635"
                  case 0x5926: // "Intel Iris Plus Graphics 640"
                  case 0x5927: // "Intel Iris Plus Graphics 650"
                  case 0x5917: // "Intel UHD Graphics 620"
                  case 0x591C: // "Intel UHD Graphics 615"
                  case 0x87C0: // "Intel UHD Graphics 617"
                  case 0x87CA: // "Intel UHD Graphics 615"
                    FBLEVX = 0x056C;
                    break;

                  case 0x3E90: // "Intel UHD Graphics 610"
                  case 0x3E93: // "Intel UHD Graphics 610"
                  case 0x3E91: // "Intel UHD Graphics 630"
                  case 0x3E92: // "Intel UHD Graphics 630"
                  case 0x3E98: // "Intel UHD Graphics 630"
                  case 0x3E9B: // "Intel UHD Graphics 630"
                  case 0x3EA5: // "Intel Iris Plus Graphics 655"
                  case 0x3EA0: // "Intel UHD Graphics 620"
                  case 0x9B41: // "Intel UHD Graphics 620"
                  case 0x9BCA: // "Intel UHD Graphics 620"
                    FBLEVX = 0xFFFF;
                    break;

                  default:
                    FBLEVX = 0xFFFF;
                    break;
                }

                // Write LEVW
                if (LEVW != SYSLEVW) {
                  MsgLog ("  Found invalid LEVW, set System LEVW: 0x%X\n", SYSLEVW);
                  /*Status = */PciIo->Mem.Write(
                                                PciIo,
                                                EfiPciIoWidthUint32,
                                                0,
                                                0xC8250,
                                                1,
                                                &SYSLEVW
                                                );
                }

                switch (gCPUStructure.Model) {
                  case CPU_MODEL_SANDY_BRIDGE:
                  case CPU_MODEL_IVY_BRIDGE:
                  case CPU_MODEL_IVY_BRIDGE_E5:
                    // if change SYS LEVW to macOS LEVW, the brightness of the pop-up may decrease or increase.
                    // but the brightness of the monitor will not actually change. so we should not use this.
                    MsgLog ("  Skip writing macOS LEVW: 0x%X\n", MACLEVW);
                    break;

                  case CPU_MODEL_HASWELL:
                  case CPU_MODEL_HASWELL_ULT:
                  case CPU_MODEL_HASWELL_U5:    // Broadwell
                  case CPU_MODEL_BROADWELL_HQ:
                  case CPU_MODEL_BROADWELL_E5:
                  case CPU_MODEL_BROADWELL_DE:
                    // if not change SYS LEVW to macOS LEVW, backlight will be dark and don't work keys for backlight.
                    // so we should use this.
                    MsgLog ("  Write macOS LEVW: 0x%X\n", MACLEVW);

                    /*Status = */PciIo->Mem.Write(
                                                  PciIo,
                                                  EfiPciIoWidthUint32,
                                                  0,
                                                  0xC8250,
                                                  1,
                                                  &MACLEVW
                                                  );
                    break;

                  default:
                    if (gSettings.IntelBacklight) {
                      MsgLog ("  Write macOS LEVW: 0x%X\n", MACLEVW);

                      /*Status = */PciIo->Mem.Write(
                                                    PciIo,
                                                    EfiPciIoWidthUint32,
                                                    0,
                                                    0xC8250,
                                                    1,
                                                    &MACLEVW
                                                    );
                    }
                    break;
                }

                switch (Pci.Hdr.DeviceId) {
                  case 0x0042: // "Intel HD Graphics"
                  case 0x0046: // "Intel HD Graphics"
                  case 0x0102: // "Intel HD Graphics 2000"
                  case 0x0106: // "Intel HD Graphics 2000"
                  case 0x010A: // "Intel HD Graphics P3000"
                  case 0x0112: // "Intel HD Graphics 3000"
                  case 0x0116: // "Intel HD Graphics 3000"
                  case 0x0122: // "Intel HD Graphics 3000"
                  case 0x0126: // "Intel HD Graphics 3000"
                  case 0x0152: // "Intel HD Graphics 2500"
                  case 0x0156: // "Intel HD Graphics 2500"
                  case 0x015A: // "Intel HD Graphics 2500"
                  case 0x0162: // "Intel HD Graphics 4000"
                  case 0x0166: // "Intel HD Graphics 4000"
                  case 0x016A: // "Intel HD Graphics P4000"
                    // Write LEVL/LEVX
                    if (gSettings.IntelMaxBacklight) {
                      if (!LEVL) {
                        LEVL = FBLEVX;
                        MsgLog ("  Found invalid LEVL, set LEVL: 0x%X\n", LEVL);
                      }

                      if (!LEVX) {
                        ShiftLEVX = FBLEVX;
                        MsgLog ("  Found invalid LEVX, set LEVX: 0x%X\n", ShiftLEVX);
                      }

                      if (gSettings.IntelMaxValue) {
                        FBLEVX = gSettings.IntelMaxValue;
                        MsgLog ("  Read IntelMaxValue: 0x%X\n", FBLEVX);
                      } else {
                        MsgLog ("  Read default Framebuffer LEVX: 0x%X\n", FBLEVX);
                      }

                      LEVL = (LEVL * FBLEVX) / ShiftLEVX;
                      MsgLog ("  Write new LEVL: 0x%X\n", LEVL);

                      /*Status = */PciIo->Mem.Write(
                                                    PciIo,
                                                    EfiPciIoWidthUint32,
                                                    0,
                                                    0x48254,
                                                    1,
                                                    &LEVL
                                                    );

                      LEVX = FBLEVX | FBLEVX << 16;
                      MsgLog ("  Write new LEVX: 0x%X\n", LEVX);

                      /*Status = */PciIo->Mem.Write(
                                                    PciIo,
                                                    EfiPciIoWidthUint32,
                                                    0,
                                                    0xC8254,
                                                    1,
                                                    &LEVX
                                                    );
                    }
                    break;

                  case 0x3E90: // "Intel UHD Graphics 610"
                  case 0x3E93: // "Intel UHD Graphics 610"
                  case 0x3E91: // "Intel UHD Graphics 630"
                  case 0x3E92: // "Intel UHD Graphics 630"
                  case 0x3E98: // "Intel UHD Graphics 630"
                  case 0x3E9B: // "Intel UHD Graphics 630"
                  case 0x3EA5: // "Intel Iris Plus Graphics 655"
                  case 0x3EA0: // "Intel UHD Graphics 620"
                  case 0x9B41: // "Intel UHD Graphics 620"
                  case 0x9BCA: // "Intel UHD Graphics 620"
                    // Write LEVD
                    if (gSettings.IntelMaxBacklight) {
                      if (gSettings.IntelMaxValue) {
                        FBLEVX = gSettings.IntelMaxValue;
                        MsgLog ("  Read IntelMaxValue: 0x%X\n", FBLEVX);
                      } else {
                        MsgLog ("  Read default Framebuffer LEVX: 0x%X\n", FBLEVX);
                      }

                      LEVD = (UINT32)DivU64x32(MultU64x32(FBLEVX, LEVX), 0xFFFF);
                      MsgLog ("  Write new LEVD: 0x%X\n", LEVD);

                      /*Status = */PciIo->Mem.Write(
                                                    PciIo,
                                                    EfiPciIoWidthUint32,
                                                    0,
                                                    0xC8258,
                                                    1,
                                                    &LEVD
                                                    );
                    }
                    break;

                  default:
                    // Write LEVX
                    if (gSettings.IntelMaxBacklight) {
                      if (gSettings.IntelMaxValue) {
                        FBLEVX = gSettings.IntelMaxValue;
                        MsgLog ("  Read IntelMaxValue: 0x%X\n", FBLEVX);
                        LEVX = FBLEVX | FBLEVX << 16;
                      } else if (!LEVX) {
                        MsgLog ("  Found invalid LEVX, set LEVX: 0x%X\n", FBLEVX);
                        LEVX = FBLEVX | FBLEVX << 16;
                      } else if (ShiftLEVX != FBLEVX) {
                        MsgLog ("  Read default Framebuffer LEVX: 0x%X\n", FBLEVX);
                        LEVX = (((LEVX & 0xFFFF) * FBLEVX / ShiftLEVX) | FBLEVX << 16);
                      }

                      MsgLog ("  Write new LEVX: 0x%X\n", LEVX);

                      /*Status = */PciIo->Mem.Write(
                                                    PciIo,
                                                    EfiPciIoWidthUint32,
                                                    0,
                                                    0xC8254,
                                                    1,
                                                    &LEVX
                                                    );
                    }
                    break;
                }

                if (gSettings.FakeIntel == 0x00008086) {
                  UINT32 IntelDisable = 0x03;
                  PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, 0x50, 1, &IntelDisable);
                }
              }
              break;

            case 0x10de:
              if (gSettings.InjectNVidia) {
                TmpDirty    = setup_nvidia_devprop(&PCIdevice);
                StringDirty |=  TmpDirty;
              } else {
                MsgLog ("NVidia GFX injection not set\n");
              }
              break;

            default:
              break;
          }
        }

        //LAN
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_ETHERNET)) {
          //MsgLog ("Ethernet device found\n");
            TmpDirty = set_eth_props (&PCIdevice);
            StringDirty |=  TmpDirty;
        }

        //USB
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_USB)) {
          if (gSettings.USBInjection) {
            TmpDirty = set_usb_props (&PCIdevice);
            StringDirty |=  TmpDirty;
          }
        }

        // HDA
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
                 ((Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA) ||
                  (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_AUDIO))) {
                   // HDMI injection inside
          if (gSettings.HDAInjection ) {
            TmpDirty    = setup_hda_devprop (PciIo, &PCIdevice, Entry->OSVersion);
            StringDirty |= TmpDirty;
          }
          if (gSettings.ResetHDA) {
            
            //PCI_HDA_TCSEL_OFFSET = 0x44
            UINT8 Value = 0;
            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);
            
            if (EFI_ERROR(Status)) {
              continue;
            }
            
            Value &= 0xf8;
            PciIo->Pci.Write (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);
          }
        }

        //LPC
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_BRIDGE) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_BRIDGE_ISA))
        {
          if (gSettings.LpcTune) {
            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, GEN_PMCON_1, 1, &PmCon);
            MsgLog ("Initial PmCon value=%hX\n", PmCon);

            if (gSettings.EnableC6) {
              PmCon |= 1 << 11;
              DBG("C6 enabled\n");
            } else {
              PmCon &= ~(1 << 11);
              DBG("C6 disabled\n");
            }
            /*
             if (gSettings.EnableC2) {
             PmCon |= 1 << 10;
             DBG("BIOS_PCIE enabled\n");
             } else {
             PmCon &= ~(1 << 10);
             DBG("BIOS_PCIE disabled\n");
             }
             */
            if (gSettings.EnableC4) {
              PmCon |= 1 << 7;
              DBG("C4 enabled\n");
            } else {
              PmCon &= ~(1 << 7);
              DBG("C4 disabled\n");
            }

            if (gSettings.EnableISS) {
              PmCon |= 1 << 3;
              DBG("SpeedStep enabled\n");
            } else {
              PmCon &= ~(1 << 3);
              DBG("SpeedStep disabled\n");
            }

            PciIo->Pci.Write (PciIo, EfiPciIoWidthUint16, GEN_PMCON_1, 1, &PmCon);

            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16,GEN_PMCON_1, 1, &PmCon);
            MsgLog ("Set PmCon value=%hX\n", PmCon);

          }
          Rcba   = 0;
          /* Scan Port */
          Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0xF0, 1, &Rcba);
          if (EFI_ERROR(Status)) continue;
          //        Rcba &= 0xFFFFC000;
          if ((Rcba & 0xFFFFC000) == 0) {
            MsgLog (" RCBA disabled; cannot use it\n");
            continue;
          }
          if ((Rcba & 1) == 0) {
            MsgLog (" RCBA access disabled; trying to enable\n");
            Rcba |= 1;

            PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, 0xF0, 1, &Rcba);
          }

          Rcba &= 0xFFFFC000;
          if (gSettings.ForceHPET) {
            Hptc = REG32 ((UINTN)Rcba, 0x3404);
            if ((Hptc & 0x80) != 0) {
              DBG("HPET is already enabled\n");
            } else {
              DBG("HPET is disabled, trying to enable...\n");
              REG32 ((UINTN)Rcba, 0x3404) = Hptc | 0x80;
            }
            // Re-Check if HPET is enabled.
            Hptc = REG32 ((UINTN)Rcba, 0x3404);
            if ((Hptc & 0x80) == 0) {
              DBG("HPET is disabled in HPTC. Cannot enable!\n");
            } else {
              DBG("HPET is enabled\n");
            }
          }

          if (gSettings.DisableFunctions){
            UINT32 FD = REG32 ((UINTN)Rcba, 0x3418);
            DBG("Initial value of FD register 0x%X\n", FD);
            FD |= gSettings.DisableFunctions;
            REG32 ((UINTN)Rcba, 0x3418) = FD;
            FD = REG32 ((UINTN)Rcba, 0x3418);
            DBG(" recheck value after patch 0x%X\n", FD);
          }
        }
      }
    }
  }

  if (StringDirty) {
    EFI_PHYSICAL_ADDRESS BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
    device_inject_stringlength                   = device_inject_string->length * 2;
    DBG("stringlength = %d\n", device_inject_stringlength);
    // gDeviceProperties = (__typeof__(gDeviceProperties))AllocateAlignedPages EFI_SIZE_TO_PAGES (device_inject_stringlength + 1), 64);

    Status = gBS->AllocatePages (
                                 AllocateMaxAddress,
                                 EfiACPIReclaimMemory,
                                 EFI_SIZE_TO_PAGES ((UINTN)device_inject_stringlength + 1),
                                 &BufferPtr
                                 );

    if (!EFI_ERROR(Status)) {
      mProperties       = (UINT8*)(UINTN)BufferPtr;
      gDeviceProperties = devprop_generate_string (device_inject_string);
      gDeviceProperties[device_inject_stringlength] = 0;
      //     DBG(gDeviceProperties);
      //     DBG("\n");
      //     StringDirty = FALSE;
      //-------
      mPropSize = (UINT32)AsciiStrLen(gDeviceProperties) / 2;
      //     DBG("Preliminary size of mProperties=%d\n", mPropSize);
      mPropSize = hex2bin (gDeviceProperties, mProperties, mPropSize);
      //     DBG("Final size of mProperties=%d\n", mPropSize);
      //---------
      //      Status = egSaveFile(SelfRootDir,  L"EFI\\CLOVER\\misc\\devprop.bin", (UINT8*)mProperties, mPropSize);
      //and now we can free memory?
      if (gSettings.AddProperties) {
        FreePool(gSettings.AddProperties);
      }
      if (gSettings.ArbProperties) {
        DEV_PROPERTY *Props;
        DEV_PROPERTY *Next;
        Prop = gSettings.ArbProperties;
        while (Prop) {
          Props = Prop->Child;
          if (Prop->Label) {
            FreePool(Prop->Label);
          }
          if (Prop->Key) {
            FreePool(Prop->Key);
          }
          if (Prop->Value) {
            FreePool(Prop->Value);
          }
          if (Prop->DevicePath) {
            FreePool(Prop->DevicePath);
          }
          while (Props) {
            if (Props->Label) {
              FreePool(Props->Label);
            }
            if (Props->Key) {
              FreePool(Props->Key);
            }
            if (Props->Value) {
              FreePool(Props->Value);
            }
            if (Props->DevicePath) {
              FreePool(Props->DevicePath);
            }
            Next = Props->Next;
            FreePool(Props);
            //delete Props;
            Props = Next;
          }
          Next = Prop->Next;
          FreePool(Prop);
          Prop = Next;
        }
      }
    }
  }

	MsgLog ("CurrentMode: Width=%lld Height=%lld\n", UGAWidth, UGAHeight);
}

EFI_STATUS
SaveSettings ()
{
  // TODO: SetVariable()..
  // here we can apply user settings instead of default one
  gMobile                       = gSettings.Mobile;

  if ((gSettings.BusSpeed != 0) && (gSettings.BusSpeed > 10 * kilo) && (gSettings.BusSpeed < 500 * kilo)) {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_PENTIUM_M:
      case CPU_MODEL_ATOM://  Atom
      case CPU_MODEL_DOTHAN:// Pentium M, Dothan, 90nm
      case CPU_MODEL_YONAH:// Core Duo/Solo, Pentium M DC
      case CPU_MODEL_MEROM:// Core Xeon, Core 2 Duo, 65nm, Mobile
        //case CPU_MODEL_CONROE:// Core Xeon, Core 2 Duo, 65nm, Desktop like Merom but not mobile
      case CPU_MODEL_CELERON:
      case CPU_MODEL_PENRYN:// Core 2 Duo/Extreme, Xeon, 45nm , Mobile
      case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
      case CPU_MODEL_FIELDS:// Core i7, i5 LGA1156, "Clarksfield", "Lynnfield", "Jasper", 45nm
      case CPU_MODEL_DALES:// Core i7, i5, Nehalem
      case CPU_MODEL_CLARKDALE:// Core i7, i5, i3 LGA1156, "Westmere", "Clarkdale", , 32nm
      case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
      case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
      case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
        gCPUStructure.ExternalClock = gSettings.BusSpeed;
        //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, kilo)));
        break;
      default:
        //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gSettings.BusSpeed, kilo)));

        // for sandy bridge or newer
        // to match ExternalClock 25 MHz like real mac, divide BusSpeed by 4
        gCPUStructure.ExternalClock = (gSettings.BusSpeed + 3) / 4;
        //DBG("Corrected ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, kilo)));
        break;
    }

    gCPUStructure.FSBFrequency  = MultU64x64 (gSettings.BusSpeed, kilo); //kHz -> Hz
    gCPUStructure.MaxSpeed      = (UINT32)(DivU64x32 ((UINT64)gSettings.BusSpeed * gCPUStructure.MaxRatio, 10000)); //kHz->MHz
  }

  if ((gSettings.CpuFreqMHz > 100) && (gSettings.CpuFreqMHz < 20000)) {
    gCPUStructure.MaxSpeed      = gSettings.CpuFreqMHz;
  }

  // to determine the use of Table 132
  if (gSettings.QPI) {
    gSettings.SetTable132 = TRUE;
    //DBG("QPI: use Table 132\n");
  }
  else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
      case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
      case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
      case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
        gSettings.SetTable132 = TRUE;
        DBG("QPI: use Table 132\n");
        break;
      default:
        //DBG("QPI: disable Table 132\n");
        break;
    }
  }

  gCPUStructure.CPUFrequency    = MultU64x64 (gCPUStructure.MaxSpeed, Mega);

  return EFI_SUCCESS;
}

XStringW GetOtherKextsDir (BOOLEAN On)
{
  XStringW SrcDir;

  SrcDir = SWPrintf("%ls\\kexts\\%s", OEMPath.wc_str(), On?"Other":"Off");
  if (!FileExists (SelfVolume->RootDir, SrcDir)) {
    SrcDir = SWPrintf("\\EFI\\CLOVER\\kexts\\%s", On?"Other":"Off");
    if (!FileExists (SelfVolume->RootDir, SrcDir)) {
      SrcDir.setEmpty();
    }
  }

  return SrcDir;
}

//dmazar
// caller is responsible for FreePool the result
XStringW
GetOSVersionKextsDir (
                       const XString8& OSVersion
                       )
{
  XString8 FixedVersion;
  CHAR8  *DotPtr;

  if (OSVersion.notEmpty()) {
    FixedVersion.strncpy(OSVersion.c_str(), 5);
    //    DBG("%s\n", FixedVersion);
    // OSVersion may contain minor version too (can be 10.x or 10.x.y)
    if ((DotPtr = AsciiStrStr (FixedVersion.c_str(), ".")) != NULL) {
      DotPtr = AsciiStrStr (DotPtr+1, "."); // second dot
    }

    if (DotPtr != NULL) {
      *DotPtr = 0;
    }
  }

  //MsgLog ("OS=%ls\n", OSTypeStr);

  // find source injection folder with kexts
  // note: we are just checking for existance of particular folder, not checking if it is empty or not
  // check OEM subfolders: version specific or default to Other
  XStringW SrcDir = SWPrintf("%ls\\kexts\\%s", OEMPath.wc_str(), FixedVersion.c_str());
  if (!FileExists (SelfVolume->RootDir, SrcDir)) {
    SrcDir = SWPrintf("\\EFI\\CLOVER\\kexts\\%s", FixedVersion.c_str());
    if (!FileExists (SelfVolume->RootDir, SrcDir)) {
      SrcDir.setEmpty();
    }
  }
  return SrcDir;
}

EFI_STATUS
InjectKextsFromDir (
                    EFI_STATUS Status,
                    CONST CHAR16 *SrcDir
                    )
{

  if (EFI_ERROR(Status)) {
    MsgLog (" - ERROR: Kext injection failed!\n");
    return EFI_NOT_STARTED;
  }

  return Status;
}

EFI_STATUS LOADER_ENTRY::SetFSInjection ()
{
  EFI_STATUS           Status;
  FSINJECTION_PROTOCOL *FSInject;
  XStringW              SrcDir;
  //BOOLEAN              InjectionNeeded = FALSE;
  //BOOLEAN              BlockCaches     = FALSE;
  FSI_STRING_LIST      *Blacklist      = 0;
  FSI_STRING_LIST      *ForceLoadKexts = NULL;

  MsgLog ("Beginning FSInjection\n");

  // get FSINJECTION_PROTOCOL
  Status = gBS->LocateProtocol(&gFSInjectProtocolGuid, NULL, (void **)&FSInject);
  if (EFI_ERROR(Status)) {
    //Print (L"- No FSINJECTION_PROTOCOL, Status = %s\n", efiStrError(Status));
    MsgLog (" - ERROR: gFSInjectProtocolGuid not found!\n");
    return EFI_NOT_STARTED;
  }

  // check if blocking of caches is needed
  if (  OSFLAG_ISSET(Flags, OSFLAG_NOCACHES) || LoadOptions.contains("-f")  ) {
    MsgLog ("Blocking kext caches\n");
    //  BlockCaches = TRUE;
    // add caches to blacklist
    Blacklist = FSInject->CreateStringList();
    if (Blacklist == NULL) {
      MsgLog (" - ERROR: Not enough memory!\n");
      return EFI_NOT_STARTED;
    }

    /*
     From 10.7 to 10.9, status of directly restoring ESD files or update from Appstore cannot block kernel cache. because there are boot.efi and kernelcache file without kernel file.
     After macOS installed, boot.efi can call kernel file from S/L/Kernels.
     For this reason, long time ago, chameleon's user restored Base System.dmg to made USB installer and added kernel file in root and custom kexts in S/L/E. then used "-f" option.
     From 10.10+, boot.efi call only prelinkedkernel file without kernel file. we can never block only kernelcache.
     The use of these block caches is meaningless in modern macOS. Unlike the old days, we do not have to do the tedious task of putting the files needed for booting into the S/L/E.
     Caution! Do not add this list. If add this list, will see "Kernel cache load error (0xe)". This is just a guideline.
     by Sherlocks, 2017.11
     */

    // Installed/createinstallmedia
    //FSInject->AddStringToList(Blacklist, L"\\System\\Library\\PrelinkedKernels\\prelinkedkernel"); // 10.10+/10.13.4+

    // Recovery
    //FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\kernelcache"); // 10.7 - 10.10
    //FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\prelinkedkernel"); // 10.11+

    // BaseSytem/InstallESD
    //FSInject->AddStringToList(Blacklist, L"\\kernelcache"); // 10.7 - 10.9/(10.7/10.8)

    // 1st stage - createinstallmedia
    //FSInject->AddStringToList(Blacklist, L"\\.IABootFiles\\kernelcache"); // 10.9/10.10
    //FSInject->AddStringToList(Blacklist, L"\\.IABootFiles\\prelinkedkernel"); // 10.11 - 10.13.3

    // 2nd stage - InstallESD/AppStore/startosinstall
    //FSInject->AddStringToList(Blacklist, L"\\Mac OS X Install Data\\kernelcache"); // 10.7
    //FSInject->AddStringToList(Blacklist, L"\\OS X Install Data\\kernelcache"); // 10.8 - 10.10
    //FSInject->AddStringToList(Blacklist, L"\\OS X Install Data\\prelinkedkernel"); // 10.11
    //FSInject->AddStringToList(Blacklist, L"\\macOS Install Data\\prelinkedkernel"); // 10.12 - 10.12.3
    //FSInject->AddStringToList(Blacklist, L"\\macOS Install Data\\Locked Files\\Boot Files\\prelinkedkernel");// 10.12.4+

    // 2nd stage - Fusion Drive
    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.R\\System\\Library\\PrelinkedKernels\\prelinkedkernel"); // 10.11
    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.P\\System\\Library\\PrelinkedKernels\\prelinkedkernel"); // 10.11
    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.S\\System\\Library\\PrelinkedKernels\\prelinkedkernel"); // 10.11
    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.R\\prelinkedkernel"); // 10.12+
    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.P\\prelinkedkernel"); // 10.12+
    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.S\\prelinkedkernel"); // 10.12+

    // NetInstall
    //FSInject->AddStringToList(Blacklist, L"\\NetInstall macOS High Sierra.nbi\\i386\\x86_64\\kernelcache");


    // Block Caches list
    // InstallDVD/Installed
    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\Extensions.mkext"); // 10.6
    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Extensions.mkext"); // 10.6
    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\kernelcache"); // 10.6/10.6 - 10.9

    if (gSettings.BlockKexts[0] != L'\0') {
      FSInject->AddStringToList(Blacklist, SWPrintf("\\System\\Library\\Extensions\\%ls", gSettings.BlockKexts).wc_str());
    }
  }

  // check if kext injection is needed
  // (will be done only if caches are blocked or if boot.efi refuses to load kernelcache)
  //SrcDir = NULL;
  if (OSFLAG_ISSET(Flags, OSFLAG_WITHKEXTS)) {
    SrcDir = GetOtherKextsDir(TRUE);
    Status = FSInject->Install(
                                Volume->DeviceHandle,
                                L"\\System\\Library\\Extensions",
                                SelfVolume->DeviceHandle,
                                //GetOtherKextsDir (),
                                SrcDir.wc_str(),
                                Blacklist,
                                ForceLoadKexts
                                );
    //InjectKextsFromDir(Status, GetOtherKextsDir());
    InjectKextsFromDir(Status, SrcDir.wc_str());

    SrcDir = GetOSVersionKextsDir(OSVersion);
    Status = FSInject->Install(
                                Volume->DeviceHandle,
                                L"\\System\\Library\\Extensions",
                                SelfVolume->DeviceHandle,
                                //GetOSVersionKextsDir(OSVersion),
                                SrcDir.wc_str(),
                                Blacklist,
                                ForceLoadKexts
                                );
    //InjectKextsFromDir(Status, GetOSVersionKextsDir(OSVersion));
    InjectKextsFromDir(Status, SrcDir.wc_str());
  } else {
    MsgLog("skipping kext injection (not requested)\n");
  }

  // prepare list of kext that will be forced to load
  ForceLoadKexts = FSInject->CreateStringList();
  if (ForceLoadKexts == NULL) {
    MsgLog(" - Error: not enough memory!\n");
    return EFI_NOT_STARTED;
  }

  KextPatcherRegisterKexts(FSInject, ForceLoadKexts);

  // reinit Volume->RootDir? it seems it's not needed.

  return Status;
}


namespace old {
#include "../../CloverApp/Clover/Clover-Bridging-Header.h"
}

static void breakpoint()
{
  DBG("Bug\n");
}

#define WriteOldFixLengthString(str, strSize) xb.ncat(str.s(), MIN(str.sizeInBytes(), strSize*sizeof(*str.s()))); xb.memsetAtPos(xb.size(), 0, strSize*sizeof(*str.s())-MIN(str.sizeInBytes(), strSize*sizeof(*str.s())));
#define checkOffset(w) \
  { \
    size_t offset = OFFSET_OF(old::SETTINGS_DATA, w) ; \
    if ( offset != xb.size() ) { \
      breakpoint(); \
    } \
  }

XBuffer<UINT8> SETTINGS_DATA::serialize() const
{
  XBuffer<UINT8> xb;

  // SMBIOS TYPE0
  WriteOldFixLengthString(VendorName, 64);
checkOffset(RomVersion);
  WriteOldFixLengthString(RomVersion, 64);
  WriteOldFixLengthString(EfiVersion, 64);
  WriteOldFixLengthString(ReleaseDate, 64);
  // SMBIOS TYPE1
  WriteOldFixLengthString(ManufactureName, 64);
  WriteOldFixLengthString(ProductName, 64);
  WriteOldFixLengthString(VersionNr, 64);
  WriteOldFixLengthString(SerialNr, 64);
  xb.ncat(&SmUUID, sizeof(SmUUID));
  xb.cat(SmUUIDConfig);
  xb.ncat(&pad0, sizeof(pad0));
//CHAR8                    Uuid[64]);
//CHAR8                    SKUNumber[64]);
  WriteOldFixLengthString(FamilyName, 64);
  WriteOldFixLengthString(OEMProduct, 64);
  WriteOldFixLengthString(OEMVendor, 64);
  // SMBIOS TYPE2
  WriteOldFixLengthString(BoardManufactureName, 64);
  WriteOldFixLengthString(BoardSerialNumber, 64);
  WriteOldFixLengthString(BoardNumber, 64); //Board-ID
  WriteOldFixLengthString(LocationInChassis, 64);
  WriteOldFixLengthString(BoardVersion, 64);
  WriteOldFixLengthString(OEMBoard, 64);
checkOffset(BoardType);
  xb.cat(BoardType);
  xb.cat(pad1);
  // SMBIOS TYPE3
  xb.cat(Mobile);
  xb.cat(ChassisType);
  WriteOldFixLengthString(ChassisManufacturer, 64);
  WriteOldFixLengthString(ChassisAssetTag, 64);
  // SMBIOS TYPE4
  xb.cat(CpuFreqMHz);
  xb.cat(BusSpeed); //in kHz
  xb.cat(Turbo);
  xb.cat(EnabledCores);
  xb.cat(UserChange);
  xb.cat(QEMU);
  // SMBIOS TYPE17
  xb.cat(SmbiosVersion);
  xb.cat(Attribute);
  xb.ncat(&pad17, sizeof(pad17));
  WriteOldFixLengthString(MemoryManufacturer, 64);
  WriteOldFixLengthString(MemorySerialNumber, 64);
  WriteOldFixLengthString(MemoryPartNumber, 64);
  WriteOldFixLengthString(MemorySpeed, 64);
  // SMBIOS TYPE131
checkOffset(CpuType);
  xb.cat(CpuType);
  // SMBIOS TYPE132
  xb.cat(QPI);
  xb.cat(SetTable132);
  xb.cat(TrustSMBIOS);
  xb.cat(InjectMemoryTables);
  xb.cat(XMPDetection);
  xb.cat(UseARTFreq);
  // SMBIOS TYPE133
  xb.ncat(&pad18, sizeof(pad18));
  xb.cat(PlatformFeature);

  // PatchTableType11
  xb.cat(NoRomInfo);

  // OS parameters
  WriteOldFixLengthString(Language, 16);
checkOffset(BootArgs);
  WriteOldFixLengthString(BootArgs, 256);
  xb.memsetAtPos(xb.size(), 0, 1);
checkOffset(CustomUuid);
  WriteOldFixLengthString(CustomUuid, 40);
  xb.ncat(&pad20, sizeof(pad20));
checkOffset(DefaultVolume);
  xb.cat(uintptr_t(0)); //DefaultVolume was CHAR16*
  xb.cat(uintptr_t(0)); //DefaultLoader was CHAR16*
//Boot
checkOffset(LastBootedVolume);
  xb.cat(LastBootedVolume);
  xb.cat(SkipHibernateTimeout);
//Monitor
  xb.cat(IntelMaxBacklight);
  xb.ncat(&pad21, sizeof(pad21));
  xb.cat(VendorEDID);
  xb.cat(ProductEDID);
  xb.cat(BacklightLevel);
  xb.cat(BacklightLevelConfig);
  xb.cat(IntelBacklight);
//Boot options
  xb.cat(MemoryFix);
  xb.cat(WithKexts);
  xb.cat(WithKextsIfNoFakeSMC);
  xb.cat(FakeSMCFound);
  xb.cat(NoCaches);

  // GUI parameters
  xb.cat(Debug);
//  BOOLEAN                 Proportional); //never used
  xb.ncat(&pad22, sizeof(pad22));
  xb.cat(DefaultBackgroundColor);

  //ACPI
checkOffset(ResetAddr);
  xb.cat(ResetAddr);
  xb.cat(ResetVal);
  xb.cat(NoASPM);
  xb.cat(DropSSDT);
  xb.cat(NoOemTableId);
  xb.cat(NoDynamicExtract);
  xb.cat(AutoMerge);
  xb.cat(GeneratePStates);
  xb.cat(GenerateCStates);
  xb.cat(GenerateAPSN);
  xb.cat(GenerateAPLF);
  xb.cat(GeneratePluginType);
  xb.cat(PLimitDict);
  xb.cat(UnderVoltStep);
  xb.cat(DoubleFirstState);
  xb.cat(SuspendOverride);
  xb.cat(EnableC2);
  xb.cat(EnableC4);
  xb.cat(EnableC6);
  xb.cat(EnableISS);
  xb.cat(SlpSmiEnable);
  xb.cat(FixHeaders);
  xb.ncat(&pad23, sizeof(pad23));
  xb.cat(C3Latency);
  xb.cat(smartUPS);
  xb.cat(PatchNMI);
  xb.cat(EnableC7);
  xb.cat(SavingMode);
  WriteOldFixLengthString(DsdtName, 28);
  xb.cat(FixDsdt);
  xb.cat(MinMultiplier);
  xb.cat(MaxMultiplier);
  xb.cat(PluginType);
//  BOOLEAN                 DropMCFG);
checkOffset(FixMCFG);
  xb.cat(FixMCFG);
  xb.cat(DeviceRenameCount);
  xb.cat(DeviceRename);
  //Injections
  xb.cat(StringInjector);
  xb.cat(InjectSystemID);
  xb.cat(NoDefaultProperties);
  xb.cat(ReuseFFFF);

  //PCI devices
  xb.cat(FakeATI);    //97
  xb.cat(FakeNVidia);
  xb.cat(FakeIntel);
  xb.cat(FakeLAN);   //100
  xb.cat(FakeWIFI);
  xb.cat(FakeSATA);
  xb.cat(FakeXHCI);  //103
  xb.cat(FakeIMEI);  //106

  //Graphics
//  UINT16                  PCIRootUID);
checkOffset(GraphicsInjector);
  xb.cat(GraphicsInjector);
  xb.cat(InjectIntel);
  xb.cat(InjectATI);
  xb.cat(InjectNVidia);
  xb.cat(DeInit);
  xb.cat(LoadVBios);
  xb.cat(PatchVBios);
  xb.ncat(&pad24, sizeof(pad24));
  xb.cat(PatchVBiosBytes);
  xb.cat(PatchVBiosBytesCount);
  xb.cat(InjectEDID);
  xb.cat(LpcTune);
  xb.cat(DropOEM_DSM); //vacant
  xb.ncat(&pad25, sizeof(pad25));
  xb.cat(CustomEDID);
  xb.cat(CustomEDIDsize);
  xb.cat(EdidFixHorizontalSyncPulseWidth);
  xb.cat(EdidFixVideoInputSignal);
  xb.ncat(&pad26, sizeof(pad26));
  WriteOldFixLengthString(FBName, 16);
  xb.cat(VideoPorts);
  xb.cat(NvidiaGeneric);
  xb.cat(NvidiaNoEFI);
  xb.cat(NvidiaSingle);
  xb.ncat(&pad27, sizeof(pad27));
  xb.cat(VRAM);
  xb.ncat(&Dcfg, sizeof(Dcfg));
  xb.ncat(&NVCAP, sizeof(NVCAP));
  xb.cat(BootDisplay);
  xb.cat(NvidiaWeb);
  xb.ncat(&pad41, sizeof(pad41));
  xb.cat(DualLink);
  xb.cat(IgPlatform);

  // Secure boot white/black list
checkOffset(SecureBootWhiteListCount);
  xb.cat(SecureBootWhiteListCount);
  xb.cat(SecureBootBlackListCount);
  xb.cat(SecureBootWhiteList);
  xb.cat(SecureBootBlackList);

  // Secure boot
  xb.cat(SecureBoot);
  xb.cat(SecureBootSetupMode);
  xb.cat(SecureBootPolicy);

  // HDA
  xb.cat(HDAInjection);
  xb.cat(HDALayoutId);

  // USB DeviceTree injection
  xb.cat(USBInjection);
  xb.cat(USBFixOwnership);
  xb.cat(InjectClockID);
  xb.cat(HighCurrent);
  xb.cat(NameEH00);
  xb.cat(NameXH00);
  xb.cat(LANInjection);
  xb.cat(HDMIInjection);

 // UINT8                   pad61[2]);

  // LegacyBoot
checkOffset(LegacyBoot);
  WriteOldFixLengthString(LegacyBoot, 32);
  xb.cat(LegacyBiosDefaultEntry);

  //SkyLake
  xb.cat(HWP);
  xb.cat(TDP);
  xb.cat(HWPValue);

  //Volumes hiding
  xb.cat(uintptr_t(0)); // HVHideStrings was **
  xb.cat((INTN)0);

  // KernelAndKextPatches
  xb.memsetAtPos(xb.size(), 0, 112);  //KernelAndKextPatches was 112 bytes
  xb.cat(KextPatchesAllowed);
  xb.cat(KernelPatchesAllowed); //From GUI: Only for user patches, not internal Clover
  WriteOldFixLengthString(AirportBridgeDeviceName, 5);

  // Pre-language
  xb.cat(KbdPrevLang);

  //Pointer
  xb.cat(PointerEnabled);
  xb.ncat(&pad28, sizeof(pad28));
  xb.cat(PointerSpeed);
  xb.cat(DoubleClickTime);
  xb.cat(PointerMirror);

//  UINT8                   pad7[6]);
checkOffset(CustomBoot);
  xb.cat(CustomBoot);
  xb.ncat(&pad29, sizeof(pad29));
  xb.cat(CustomLogo);
  xb.cat(RefCLK);

  // SysVariables
  xb.ncat(&pad30, sizeof(pad30));
  xb.cat(uintptr_t(0)); // RtMLB was CHAR8*
  xb.cat(RtROM);
  xb.cat(RtROMLen);
  xb.cat(CsrActiveConfig);
  xb.cat(BooterConfig);
  WriteOldFixLengthString(BooterCfgStr, 64);
  xb.cat(DisableCloverHotkeys);
  xb.cat(NeverDoRecovery);

  // Multi-config
  xb.ncat(&ConfigName, sizeof(ConfigName));
  xb.ncat(&pad31, sizeof(pad31));
  xb.cat(uintptr_t(0)); // MainConfigName was a CHAR16*

  //Drivers
  xb.cat(BlackListCount);
  xb.cat(BlackList);

  //SMC keys
  xb.ncat(&RPlt, sizeof(RPlt));
  xb.ncat(&RBr, sizeof(RBr));
  xb.ncat(&EPCI, sizeof(EPCI));
  xb.ncat(&REV, sizeof(REV));

  //other devices
checkOffset(Rtc8Allowed);
  xb.cat(Rtc8Allowed);
  xb.cat(ForceHPET);
  xb.cat(ResetHDA);
  xb.cat(PlayAsync);
  xb.ncat(&pad32, sizeof(pad32));
  xb.cat(DisableFunctions);

  //Patch DSDT arbitrary
  xb.cat(PatchDsdtNum);
  xb.cat(PatchDsdtFind);
  xb.cat(LenToFind);
  xb.cat(PatchDsdtReplace);
  xb.cat(LenToReplace);
  xb.cat(DebugDSDT);
  xb.cat(SlpWak);
  xb.cat(UseIntelHDMI);
  xb.cat(AFGLowPowerState);
  xb.cat(PNLF_UID);
//  UINT8                   pad83[4]);

  // Table dropping
  xb.ncat(&pad34, sizeof(pad34));
  xb.cat(ACPIDropTables);

  // Custom entries
  xb.cat(DisableEntryScan);
  xb.cat(DisableToolScan);
  xb.cat((BOOLEAN)0); // was ShowHiddenEntries (BOOLEAN)
  xb.cat(KernelScan);
  xb.cat(LinuxScan);
//  UINT8                   pad84[3]);
  xb.ncat(&pad35, sizeof(pad35));
checkOffset(CustomEntries);
  xb.cat(CustomEntries);
  xb.cat(CustomLegacy);
  xb.cat(CustomTool);

  //Add custom properties
  xb.cat(NrAddProperties);
  xb.cat(AddProperties);

  //BlackListed kexts
  xb.ncat(&BlockKexts, sizeof(BlockKexts));

  //ACPI tables
  xb.cat(SortedACPICount);
  xb.cat(SortedACPI);

  // ACPI/PATCHED/AML
  xb.cat(DisabledAMLCount);
  xb.ncat(&pad36, sizeof(pad36));
  xb.cat(DisabledAML);
  xb.cat(PatchDsdtLabel);
  xb.cat(PatchDsdtTgt);
  xb.cat(PatchDsdtMenuItem);

  //other
  xb.cat(IntelMaxValue);
//  UINT32                  AudioVolume);

  // boot.efi
checkOffset(OptionsBits);
  xb.cat(OptionsBits);
  xb.cat(FlagsBits);
  xb.cat(UIScale);
  xb.cat(EFILoginHiDPI);
  xb.ncat(&flagstate, sizeof(flagstate));
  xb.ncat(&pad37, sizeof(pad37));
  xb.cat(ArbProperties);
//  xb.cat(QuirksMask);
//  xb.ncat(&pad38, sizeof(pad38));
//  xb.cat(MaxSlide);

//  if ( xb.size() != sizeof(old::SETTINGS_DATA) ) {
  if ( xb.size() != 3088 ) {
    panic("SETTINGS_DATA::serialize wrong size\n");
  }
  return xb;
}

//TagDict* regenerateConfigPlist_addDictToDict(TagDict* dict)
//{
//  TagDict* tagDict = TagDict::getEmptyTag();
//  dict->dictContent().AddReference(tagDict, true);
//  return tagDict;
//}
//TagKey* regenerateConfigPlist_addKeyToDict(TagDict* dict, const XString8& key)
//{
//  TagKey* tagKey = TagKey::getEmptyTag();
//  tagKey->setKeyValue(key);
//  dict->dictContent().AddReference(tagKey, true);
//  return tagKey;
//}
//TagBool* regenerateConfigPlist_addBoolToDict(TagDict* dict, const bool value)
//{
//  TagBool* tagBool = TagBool::getEmptyTag();
//  tagBool->setBoolValue(value);
//  dict->dictContent().AddReference(tagBool, true);
//  return tagBool;
//}
//void regenerateConfigPlist_addKeyAndBoolToDict(TagDict* dict, const XString8& key, const bool value)
//{
//  regenerateConfigPlist_addKeyToDict(dict, key);
//  regenerateConfigPlist_addBoolToDict(dict, value);
//}
//
//void testConfigPlist()
//{
//  TagDict* plist = TagDict::getEmptyTag();
//  regenerateConfigPlist_addKeyToDict(plist, "ACPI"_XS8);
//  TagDict* ACPIDict = regenerateConfigPlist_addDictToDict(plist);
//  regenerateConfigPlist_addKeyAndBoolToDict(ACPIDict, "AutoMerge"_XS8, gSettings.AutoMerge);
//  regenerateConfigPlist_addKeyToDict(ACPIDict, "DSDT"_XS8);
//  TagDict* DSDTDict = regenerateConfigPlist_addDictToDict(ACPIDict);
//  regenerateConfigPlist_addKeyToDict(DSDTDict, "Debug"_XS8);
//  regenerateConfigPlist_addBoolToDict(DSDTDict, gSettings.KernelAndKextPatches.KPDebug);
////  regenerateConfigPlist_addKeyToDict(DSDTDict, "DropOEM_DSM"_XS8);
////  TagDict* DropOEM_DSMDict = regenerateConfigPlist_addDictToDict(DSDTDict);
//  regenerateConfigPlist_addKeyToDict(DSDTDict, "Fixes"_XS8);
//  TagDict* FixesDict = regenerateConfigPlist_addDictToDict(DSDTDict);
//  for (size_t Index = 0; Index < sizeof(FixesConfig)/sizeof(FixesConfig[0]); Index++) {
//    if ( gSettings.FixDsdt & FixesConfig[Index].bitData ) {
//      regenerateConfigPlist_addKeyToDict(FixesDict, LString8(FixesConfig[Index].newName));
//      regenerateConfigPlist_addBoolToDict(FixesDict, true);
//    }
//  }
//  XString8 s;
//  plist->sprintf(0, &s);
//  MsgLog("%s\n", s.c_str());
//}
//

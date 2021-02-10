/*
 Slice 2012
 */

#include <Platform.h>
#include "Settings.h"
#include "FixBiosDsdt.h"
#include "../include/VolumeTypes.h"
#include "../include/OSFlags.h"
#include "../include/OSTypes.h"
#include "../include/BootTypes.h"
#include "../entry_scan/loader.h"
#include "../Platform/BootLog.h"
#include "../entry_scan/secureboot.h"
#include "../libeg/XTheme.h"
#include "cpu.h"
#include "VersionString.h"
#include "card_vlist.h"
#include "Injectors.h"
#include "../include/Pci.h"
#include "../include/Devices.h"
#include "smbios.h"
#include "Net.h"
#include "Nvram.h"
#include "BootOptions.h"
#include "SelfOem.h"
#include "ati_reg.h"
#include "ati.h"
#include "nvidia.h"
#include "gma.h"
#include "Edid.h"
#include "hda.h"
#include "../../Version.h"

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


XStringWArray                   ThemeNameArray;
UINTN                           ConfigsNum;
CHAR16                          *ConfigsList[20];
UINTN                           DsdtsNum = 0;
CHAR16                          *DsdtsList[20];
XObjArray<HDA_OUTPUTS>          AudioList;
XObjArray<RT_VARIABLES>         BlockRtVariableArray;

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
CONST CHAR16 *gFirmwareRevision = L"unknown";
CONST CHAR8* gRevisionStr = "unknown";
CONST CHAR8* gFirmwareBuildDate = "unknown";
CONST CHAR8* gBuildInfo = NULL;
#endif
#ifdef BUILD_ID
const LString8 gBuildId __attribute__((used)) = BUILD_ID;
const LString8 gBuildIdGrepTag __attribute__((used)) = "CloverBuildIdGrepTag: " BUILD_ID;
#else
const LString8 gBuildId __attribute__((used)) = "unknown";
const LString8 gBuildIdGrepTag __attribute__((used)) = "CloverBuildIdGrepTag: " "unknown";
#endif

// __attribute__((used)) seems to not always work. So, in AboutRefit(), there is a trick to let the compiler thinks it's used.
const LString8 path_independant __attribute__((used)) = "path_independant";

EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl = NULL;

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


CONST CHAR8* AudioOutputNames[] = {
  "LineOut",
  "Speaker",
  "Headphones",
  "SPDIF",
  "Garniture",
  "HDMI",
  "Other"
};

EFI_STATUS
SaveSettings ();

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
        List->Name[j++] = String.data()[i]; // String[i] return a char32_t. what if there is an utf8 char ?
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

void
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
  *Dict                  = NULL;

  XStringW& ConfName = *ConfNamePtr;

  Start = (CHAR8*)self.getSelfLoadedImage().LoadOptions;
  End   = (CHAR8*)((CHAR8*)self.getSelfLoadedImage().LoadOptions + self.getSelfLoadedImage().LoadOptionsSize);
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
// analyze self.getSelfLoadedImage().LoadOptions to extract Default Volume and Default Loader
// input and output data are global
//
void
GetBootFromOption(void)
{
  UINT8  *Data = (UINT8*)self.getSelfLoadedImage().LoadOptions;
  UINTN  Len = self.getSelfLoadedImage().LoadOptionsSize;
  UINTN  NameSize, Name2Size;

  Data += 4; //skip signature as we already here
  NameSize = *(UINT16*)Data;

  Data += 2; // pointer to Volume name
  gSettings.Boot.DefaultVolume.strncpy((__typeof__(gSettings.Boot.DefaultVolume.wc_str()))Data, NameSize);

  Data += NameSize;
  Name2Size = Len - NameSize;
  if (Name2Size != 0) {
    gSettings.Boot.DefaultLoader.strncpy((__typeof__(gSettings.Boot.DefaultVolume.wc_str()))Data, NameSize);
  }

  DBG("Clover started with option to boot %ls from %ls\n",
      gSettings.Boot.DefaultLoader.notEmpty() ? gSettings.Boot.DefaultLoader.wc_str() : L"legacy",
      gSettings.Boot.DefaultVolume.wc_str());
}

//
// check if this entry corresponds to Boot# variable and then set BootCurrent
//
void
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
static UINT8
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
                  IN const XStringW& ConfName,
                  TagDict** Dict)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  UINTN      Size = 0;
  CHAR8*     ConfigPtr = NULL;
//  XStringW   ConfigPlistPath;
//  XStringW   ConfigOemPath;

  //  DbgHeader("LoadUserSettings");

  // load config
  if ( ConfName.isEmpty() || Dict == NULL ) {
    return EFI_NOT_FOUND;
  }

//  ConfigPlistPath = SWPrintf("%ls.plist", ConfName.wc_str());
//  ConfigOemPath   = SWPrintf("%ls\\%ls.plist", selfOem.getOOEMPath.wc_str(), ConfName.wc_str());
  Status = EFI_NOT_FOUND;
  XStringW configFilename = SWPrintf("%ls.plist", ConfName.wc_str());
  if ( selfOem.oemDirExists() ) {
    if (FileExists (&selfOem.getOemDir(), configFilename)) {
      Status = egLoadFile(&selfOem.getOemDir(), configFilename.wc_str(), (UINT8**)&ConfigPtr, &Size);
      if (EFI_ERROR(Status)) {
        DBG("Cannot find %ls at path (%s): '%ls', trying '%ls'\n", configFilename.wc_str(), efiStrError(Status), selfOem.getOemFullPath().wc_str(), self.getCloverDirFullPath().wc_str());
      }else{
        DBG("Using %ls at path: %ls\n", configFilename.wc_str(), selfOem.getOemFullPath().wc_str());
      }
    }
  }
  if (EFI_ERROR(Status)) {
    if ( FileExists(&self.getCloverDir(), configFilename.wc_str())) {
      Status = egLoadFile(&self.getCloverDir(), configFilename.wc_str(), (UINT8**)&ConfigPtr, &Size);
    }
    if (EFI_ERROR(Status)) {
      DBG("Cannot find %ls at path '%ls' : %s\n", configFilename.wc_str(), self.getCloverDirFullPath().wc_str(), efiStrError(Status));
    } else {
      DBG("Using %ls at path: %ls\n", configFilename.wc_str(), self.getCloverDirFullPath().wc_str());
    }
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

//  Prop = DictPointer->propertyForKey("OcFuzzyMatch");
//if ( Prop ) panic("config.plist/KernelAndKextPatches/OcFuzzyMatch has been moved in section config.plist/Quirks. Update your config.plist");
//  if (Prop != NULL || GlobalConfig.gBootChanged) {
//    Patches->FuzzyMatch = IsPropertyNotNullAndTrue(Prop);
//  }
//
//  Prop = DictPointer->propertyForKey("OcKernelCache");
//if ( Prop ) panic("config.plist/KernelAndKextPatches/OcKernelCache has been moved in section config.plist/Quirks. Update your config.plist");
//  if (Prop != NULL || GlobalConfig.gBootChanged) {
//    if ( Prop->isString() ) {
//      if ( Prop->getString()->stringValue().notEmpty() ) {
//        Patches->OcKernelCache = Prop->getString()->stringValue();
//      }else{
//        Patches->OcKernelCache = "Auto"_XS8;
//      }
//    }else{
//      MsgLog("MALFORMED PLIST : KernelAndKextPatches/KernelCache must be a string");
//      Patches->OcKernelCache = "Auto"_XS8;
//    }
//  }

  {
//    const TagDict* OcQuirksDict = DictPointer->dictPropertyForKey("OcQuirks");
//if ( OcQuirksDict ) panic("config.plist/KernelAndKextPatches/OcQuirks has been merged in the config.plist/Quirks section. Update your config.plist");
//    if ( OcQuirksDict )
//    {
//      Prop = OcQuirksDict->propertyForKey("AppleCpuPmCfgLock");
//if ( !Prop ) panic("Cannot find AppleCpuPmCfgLock in OcQuirks under KernelAndKextPatches (OC kernel quirks)");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.AppleCpuPmCfgLock = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("AppleXcpmCfgLock");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.AppleXcpmCfgLock = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("AppleXcpmExtraMsrs");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.AppleXcpmExtraMsrs = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("AppleXcpmForceBoost");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.AppleXcpmForceBoost = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("CustomSMBIOSGuid");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.CustomSmbiosGuid = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("DisableIoMapper");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.DisableIoMapper = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("DisableLinkeditJettison");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.DisableLinkeditJettison = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("DisableRtcChecksum");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.DisableRtcChecksum = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("DummyPowerManagement");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.DummyPowerManagement = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("ExternalDiskIcons");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.ExternalDiskIcons = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("IncreasePciBarSize");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.IncreasePciBarSize = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("LapicKernelPanic");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.LapicKernelPanic = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("PanicNoKextDump");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.PanicNoKextDump = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("PowerTimeoutKernelPanic");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.PowerTimeoutKernelPanic = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("ThirdPartyDrives");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.ThirdPartyDrives = IsPropertyNotNullAndTrue(Prop);
//      }
//
//      Prop = OcQuirksDict->propertyForKey("XhciPortLimit");
//      if (Prop != NULL || GlobalConfig.gBootChanged) {
//        Patches->OcKernelQuirks.XhciPortLimit = IsPropertyNotNullAndTrue(Prop);
//      }
//    }
  }

  Prop = DictPointer->propertyForKey("Debug");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPDebug = IsPropertyNotNullAndTrue(Prop);
  }
/*
  Prop = GetProperty(DictPointer, "KernelCpu");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPKernelCpu = IsPropertyTrue(Prop);
  }
*/
  Prop = DictPointer->propertyForKey("KernelLapic");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPKernelLapic = IsPropertyNotNullAndTrue(Prop);
  }

  Prop = DictPointer->propertyForKey("KernelXCPM");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPKernelXCPM = IsPropertyNotNullAndTrue(Prop);
    if (IsPropertyNotNullAndTrue(Prop)) {
      DBG("KernelXCPM: enabled\n");
    }
  }

  Prop = DictPointer->propertyForKey("KernelPm");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPKernelPm = IsPropertyNotNullAndTrue(Prop);
  }
  
  Prop = DictPointer->propertyForKey("PanicNoKextDump");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPPanicNoKextDump = IsPropertyNotNullAndTrue(Prop);
  }

  Prop = DictPointer->propertyForKey("AppleIntelCPUPM");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPAppleIntelCPUPM = IsPropertyNotNullAndTrue(Prop);
  }
 //anyway
  if (GlobalConfig.NeedPMfix) {
    Patches->KPKernelPm = TRUE;
    Patches->KPAppleIntelCPUPM = TRUE;
  }

  Prop = DictPointer->propertyForKey("AppleRTC");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPAppleRTC = !IsPropertyNotNullAndFalse(Prop);  //default = TRUE
  }

  Prop = DictPointer->propertyForKey("EightApple");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->EightApple = IsPropertyNotNullAndTrue(Prop);
  }

  //
  // Dell SMBIOS Patch
  //
  // syscl: we do not need GlobalConfig.gBootChanged and Prop is empty condition
  // this change will boost Dell SMBIOS Patch a bit
  // but the major target is to make code clean
  Prop = DictPointer->propertyForKey("DellSMBIOSPatch");
  Patches->KPDELLSMBIOS = IsPropertyNotNullAndTrue(Prop); // default == FALSE
  gRemapSmBiosIsRequire = Patches->KPDELLSMBIOS;

  Prop = DictPointer->propertyForKey("FakeCPUID");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
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

    if (Patches->KPATIConnectorsData.isEmpty()
        || Patches->KPATIConnectorsPatch.isEmpty()
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

          if (strlen(p) > 1) {
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

     //   newPatch.MenuItem.BValue     = TRUE;
        Dict = Prop2->propertyForKey("Disabled");
        newPatch.MenuItem.BValue = !IsPropertyNotNullAndTrue(Dict); //if absent then false, BValue = !Disabled
        
     //   if ((Dict != NULL) && IsPropertyNotNullAndTrue(Dict)) {
     //     newPatch.MenuItem.BValue     = FALSE;
     //   }
        
        
        Dict = Prop2->propertyForKey("RangeFind");
        newPatch.SearchLen = GetPropertyAsInteger(Dict, 0); //default 0 will be calculated later

        Dict = Prop2->propertyForKey("Skip");
        newPatch.Skip = GetPropertyAsInteger(Dict, 0); //default 0 will be calculated later

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
          newPatch.MaskFind.memset(0xFF, FindLen);
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
        
        newPatch.Count = 1;
        Dict = Prop2->propertyForKey("Count");
        if (Dict != NULL) {
          newPatch.Count = GetPropertyAsInteger(Dict, 1);
        }

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
  if (arrayProp != NULL) {
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
        
        newKernelPatch.Name = "kernel"_XS8;

        prop3 = Prop2->propertyForKey("Disabled");
        newKernelPatch.MenuItem.BValue   = !IsPropertyNotNullAndTrue(prop3);
        
        prop3 = Prop2->propertyForKey("RangeFind");
        newKernelPatch.SearchLen = GetPropertyAsInteger(prop3, 0); //default 0 will be calculated later

        prop3 = Prop2->propertyForKey("Skip");
        newKernelPatch.Skip = GetPropertyAsInteger(prop3, 0); //default 0 will be calculated later
        
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
  if (arrayProp != NULL) {
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
        
        newBootPatch.Name = "boot.efi"_XS8;

        prop3 = Prop2->propertyForKey("Disabled");
        newBootPatch.MenuItem.BValue   = !IsPropertyNotNullAndTrue(prop3);
        newBootPatch.MenuItem.ItemType = BoolValue;
        
        prop3 = Prop2->propertyForKey("RangeFind");
        newBootPatch.SearchLen = GetPropertyAsInteger(prop3, 0); //default 0 will be calculated later
 
        prop3 = Prop2->propertyForKey("Skip");
        newBootPatch.Skip = GetPropertyAsInteger(prop3, 0); //default 0 will be calculated later

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




//BOOLEAN IsOSValid(const XString8& MatchOS, const MacOsVersion& CurrOS)
//{
//  /* example for valid matches are:
//   10.7, only 10.7 (10.7.1 will be skipped)
//   10.10.2 only 10.10.2 (10.10.1 or 10.10.5 will be skipped)
//   10.10.x (or 10.10.X), in this case is valid for all minor version of 10.10 (10.10.(0-9))
//   */
//
//  BOOLEAN ret = FALSE;
//
//  if (MatchOS.isEmpty() || CurrOS.isEmpty()) {
//    return TRUE; //undefined matched corresponds to old behavior
//  }
//
////  osToc = GetStrArraySeparatedByChar(MatchOS, '.');
//  XString8Array osToc = Split<XString8Array>(MatchOS, "."_XS8).trimEachString();
//  XString8Array currOStoc = Split<XString8Array>(CurrOS, "."_XS8).trimEachString();
//
//  if ( osToc.size() > 0 && currOStoc.size() > 0 && osToc[0] == "11"_XS8 && currOStoc[0] == "11"_XS8 ) {
//    if (osToc.size() == 1 ) return true;
//    if (osToc.size() == 2 ) {
//      if ( osToc[1].equalIC("x") ) return true;
//      if ( currOStoc.size() == 2 && osToc[1] == currOStoc[1] ) return true;
//    }
//  }
//  if (osToc.size() == 2) {
//    if (currOStoc.size() == 2) {
//      if ( osToc[0] == currOStoc[0] && osToc[1] == currOStoc[1]) {
//        ret = TRUE;
//      }
//    }
//  } else if (osToc.size() == 3) {
//    if (currOStoc.size() == 3) {
//      if ( osToc[0] == currOStoc[0]
//          && osToc[1] == currOStoc[1]
//          && osToc[2] == currOStoc[2]) {
//        ret = TRUE;
//      } else if ( osToc[0] == currOStoc[0]
//                 && osToc[1] == currOStoc[1]
//                 && osToc[2].equalIC("x") ) {
//        ret = TRUE;
//      }
//    } else if (currOStoc.size() == 2) {
//      if ( osToc[0] == currOStoc[0]
//          && osToc[1] ==  currOStoc[1] ) {
//        ret = TRUE;
//      } else if ( osToc[0] == currOStoc[0]
//                 && osToc[1] ==  currOStoc[1]
//                 && osToc[2].equalIC("x") == 0 ) {
//        ret = TRUE;
//      }
//    }
//  }
//  return ret;
//}

static UINT8 CheckVolumeType(UINT8 VolumeType, const TagStruct* Prop)
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

static UINT8 GetVolumeType(const TagDict* DictPointer)
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


static CONST CHAR8 *CustomBootModeStr[] = {
   "CUSTOM_BOOT_DISABLED",
   "CUSTOM_BOOT_USER_DISABLED",
   "CUSTOM_BOOT_NONE",
   "CUSTOM_BOOT_APPLE",
   "CUSTOM_BOOT_ALT_APPLE",
   "CUSTOM_BOOT_THEME",
   "CUSTOM_BOOT_USER",
};
static CONST CHAR8 *CustomBootModeToStr(IN UINT8 Mode)
{
  if (Mode >= (sizeof(CustomBootModeStr) / sizeof(CustomBootModeStr[0]))) {
    return CustomBootModeStr[0];
  }
  return CustomBootModeStr[Mode];
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
    if ( Prop->getString()->stringValue()[0] < __WCHAR_MAX__ ) {
      Entry->Hotkey = (wchar_t)(Prop->getString()->stringValue()[0]);
    }
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
        Entry->CustomLogo.LoadXImage(&self.getSelfVolumeRootDir(), customLogo);
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
      DBG("     hiding entry because Hidden flag is YES\n");
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
      } else if ( Prop->isFalseOrNn() ) {
        // nothing to do
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

static BOOLEAN
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
      Entry->Image.LoadXImage(&ThemeX.getThemeDir(), Prop->getString()->stringValue());
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
      Entry->Image.LoadXImage(&ThemeX.getThemeDir(), Prop->getString()->stringValue());
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
    if ( Prop->getString()->stringValue()[0] < __WCHAR_MAX__ ) {
      Entry->Hotkey = (wchar_t)(Prop->getString()->stringValue()[0]);
    }
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
      DBG("     hiding entry because Hidden flag is YES\n");
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

static BOOLEAN
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
    if (Prop->isString()) {
      Entry->ImagePath = Prop->getString()->stringValue();
    }
    Entry->Image.LoadXImage(&ThemeX.getThemeDir(), Entry->ImagePath);
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
    if ( Prop->getString()->stringValue()[0] < __WCHAR_MAX__ ) {
      Entry->Hotkey = (wchar_t)(Prop->getString()->stringValue()[0]);
    }
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
      DBG("     hiding entry because Hidden flag is YES\n");
      Entry->Hidden = true;
    } else {
      Entry->Hidden = false;
    }
  }

  Entry->VolumeType = GetVolumeType(DictPointer);

  return TRUE;
}

// EDID reworked by Sherlocks
static void
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
                      const TagDict* CfgDict,
                      SETTINGS_DATA& settingsData
                      )
{
  EFI_STATUS  Status = EFI_SUCCESS;
//  const TagDict*      Dict;
//  const TagDict*      Dict2;
//  const TagDict*      DictPointer;
//  const TagStruct*    Prop;
//  const TagArray*     arrayProp;
//  void        *Value = NULL;
//  BOOLEAN     SpecialBootMode = FALSE;

//  {
//    UINTN       Size = 0;
//    //read aptiofixflag from nvram for special boot
//    Status = GetVariable2(L"aptiofixflag", &gEfiAppleBootGuid, &Value, &Size);
//    if (!EFI_ERROR(Status)) {
//      SpecialBootMode = TRUE;
//      FreePool(Value);
//    }
//  }

  settingsData.KextPatchesAllowed              = TRUE;
  settingsData.KernelAndKextPatches.KPAppleRTC = TRUE;
  settingsData.KernelAndKextPatches.KPDELLSMBIOS = FALSE; // default is false
  settingsData.KernelPatchesAllowed            = TRUE;

  if (CfgDict != NULL) {
    //DBG("Loading early settings\n");
    DbgHeader("GetEarlyUserSettings");

    const TagDict* BootDict = CfgDict->dictPropertyForKey("Boot");
    if (BootDict != NULL) {
      const TagStruct* Prop = BootDict->propertyForKey("Timeout");
      if (Prop != NULL) {
        settingsData.Boot.Timeout = (INT32)GetPropertyAsInteger(Prop, settingsData.Boot.Timeout);
      DBG("timeout set to %lld\n", settingsData.Boot.Timeout);
      }

      Prop = BootDict->propertyForKey("SkipHibernateTimeout");
      settingsData.Boot.SkipHibernateTimeout = IsPropertyNotNullAndTrue(Prop);

      //DisableCloverHotkeys
      Prop = BootDict->propertyForKey("DisableCloverHotkeys");
      settingsData.Boot.DisableCloverHotkeys = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("Arguments");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        settingsData.Boot.BootArgs = Prop->getString()->stringValue();
      }

      // defaults if "DefaultVolume" is not present or is empty
      settingsData.Boot.LastBootedVolume = FALSE;
      //     settingsData.Boot.DefaultVolume    = NULL;

      Prop = BootDict->propertyForKey("DefaultVolume");
      if (Prop != NULL) {
        if ( Prop->isString()  &&  Prop->getString()->stringValue().notEmpty() ) {
          settingsData.Boot.DefaultVolume.setEmpty();
          // check for special value for remembering boot volume
          if (Prop->getString()->stringValue().equalIC("LastBootedVolume")) {
            settingsData.Boot.LastBootedVolume = TRUE;
          } else {
            settingsData.Boot.DefaultVolume = Prop->getString()->stringValue();
          }
        }
      }

      Prop = BootDict->propertyForKey("DefaultLoader");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in DefaultLoader\n");
        }else{
          settingsData.Boot.DefaultLoader = Prop->getString()->stringValue();
        }
      }

      Prop = BootDict->propertyForKey("Debug");
      if ( Prop ) {
        if ( Prop->isString() ) {
          if ( Prop->getString()->stringValue().equalIC("true") ) settingsData.Boot.DebugLog = true;
          else if ( Prop->getString()->stringValue().equalIC("false") ) settingsData.Boot.DebugLog = false;
          else MsgLog("MALFORMED config.plist : property Boot/Debug must be true, false, or scratch\n");
        }else if ( Prop->isBool() ) {
          settingsData.Boot.DebugLog = Prop->getBool()->boolValue();
        }else{
          MsgLog("MALFORMED config.plist : property Boot/Debug must be a string (true, false) or <true/> or <false/>\n");
        }
      }

      Prop = BootDict->propertyForKey("Fast");
      settingsData.Boot.FastBoot       = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("NoEarlyProgress");
      settingsData.Boot.NoEarlyProgress = IsPropertyNotNullAndTrue(Prop);

//      if (SpecialBootMode) {
//        GlobalConfig.isFastBoot()       = TRUE;
//        DBG("Fast option enabled\n");
//      }

      Prop = BootDict->propertyForKey("NeverHibernate");
      settingsData.Boot.NeverHibernate = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("StrictHibernate");
      settingsData.Boot.StrictHibernate = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("RtcHibernateAware");
      settingsData.Boot.RtcHibernateAware = IsPropertyNotNullAndTrue(Prop);

      Prop = BootDict->propertyForKey("HibernationFixup");
      if (Prop) {
        settingsData.Boot.HibernationFixup = IsPropertyNotNullAndTrue(Prop); //it will be set automatically
      }

      Prop = BootDict->propertyForKey("SignatureFixup");
      settingsData.Boot.SignatureFixup = IsPropertyNotNullAndTrue(Prop);

      //      Prop = GetProperty(DictPointer, "GetLegacyLanAddress");
      //      GetLegacyLanAddress = IsPropertyTrue(Prop);

      // Secure boot
      /* this parameter, which should be called SecureBootSetupMode, is ignored if :
       *   it is true
       *   SecureBoot is already true.
       */
      Prop = BootDict->propertyForKey("Secure");
      if (Prop != NULL) {
        if ( Prop->isFalse() ) {
          // Only disable setup mode, we want always secure boot
          settingsData.Boot.SecureBootSetupMode = 0;
        } else if ( Prop->isTrue()  &&  !settingsData.Boot.SecureBoot ) {
          // This mode will force boot policy even when no secure boot or it is disabled
          settingsData.Boot.SecureBootSetupMode = 1;
          settingsData.Boot.SecureBoot          = 1;
        }
      }
      // Secure boot policy
      Prop = BootDict->propertyForKey("Policy");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        if ((Prop->getString()->stringValue()[0] == 'D') || (Prop->getString()->stringValue()[0] == 'd')) {
          // Deny all images
          settingsData.Boot.SecureBootPolicy = SECURE_BOOT_POLICY_DENY;
        } else if ((Prop->getString()->stringValue()[0] == 'A') || (Prop->getString()->stringValue()[0] == 'a')) {
          // Allow all images
          settingsData.Boot.SecureBootPolicy = SECURE_BOOT_POLICY_ALLOW;
        } else if ((Prop->getString()->stringValue()[0] == 'Q') || (Prop->getString()->stringValue()[0] == 'q')) {
          // Query user
          settingsData.Boot.SecureBootPolicy = SECURE_BOOT_POLICY_QUERY;
        } else if ((Prop->getString()->stringValue()[0] == 'I') || (Prop->getString()->stringValue()[0] == 'i')) {
          // Insert
          settingsData.Boot.SecureBootPolicy = SECURE_BOOT_POLICY_INSERT;
        } else if ((Prop->getString()->stringValue()[0] == 'W') || (Prop->getString()->stringValue()[0] == 'w')) {
          // White list
          settingsData.Boot.SecureBootPolicy = SECURE_BOOT_POLICY_WHITELIST;
        } else if ((Prop->getString()->stringValue()[0] == 'B') || (Prop->getString()->stringValue()[0] == 'b')) {
          // Black list
          settingsData.Boot.SecureBootPolicy = SECURE_BOOT_POLICY_BLACKLIST;
        } else if ((Prop->getString()->stringValue()[0] == 'U') || (Prop->getString()->stringValue()[0] == 'u')) {
          // User policy
          settingsData.Boot.SecureBootPolicy = SECURE_BOOT_POLICY_USER;
        }
      }
      // Secure boot white list
      const TagArray* arrayProp = BootDict->arrayPropertyForKey("WhiteList");
      if (arrayProp != NULL) {
        INTN   i;
        INTN   Count = arrayProp->arrayContent().size();
        if (Count > 0) {
          settingsData.Boot.SecureBootWhiteListCount = 0;
          settingsData.Boot.SecureBootWhiteList = (__typeof__(settingsData.Boot.SecureBootWhiteList))AllocateZeroPool(Count * sizeof(CHAR16 *));
          if (settingsData.Boot.SecureBootWhiteList) {
            for (i = 0; i < Count; i++) {
              const TagStruct* prop2 = &arrayProp->arrayContent()[i];
              if ( !prop2->isString() ) {
                MsgLog("MALFORMED PLIST : WhiteList must be an array of string");
                continue;
              }
              if ( prop2->getString()->stringValue().notEmpty() ) {
                settingsData.Boot.SecureBootWhiteList[settingsData.Boot.SecureBootWhiteListCount++] = SWPrintf("%s", prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
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
          settingsData.Boot.SecureBootBlackListCount = 0;
          settingsData.Boot.SecureBootBlackList = (__typeof__(settingsData.Boot.SecureBootBlackList))AllocateZeroPool(Count * sizeof(CHAR16 *));
          if (settingsData.Boot.SecureBootBlackList) {
            for (i = 0; i < Count; i++) {
              const TagStruct* prop2 = &arrayProp->arrayContent()[i];
              if ( !prop2->isString() ) {
                MsgLog("MALFORMED PLIST : BlackList must be an array of string");
                continue;
              }
              if ( prop2->getString()->stringValue().notEmpty() ) {
                settingsData.Boot.SecureBootBlackList[settingsData.Boot.SecureBootBlackListCount++] = SWPrintf("%s", prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
              }
            }
          }
        }
      }

      // XMP memory profiles
      // -1 = do not detect
      // 0 = Detect the better XMP profile
      // 1 = Use first profile if present
      // 2 = Use second profile
      Prop = BootDict->propertyForKey("XMPDetection");
      if (Prop != NULL) {
        settingsData.Boot.XMPDetection = 0;
        if ( Prop->isFalse() ) {
          settingsData.Boot.XMPDetection = -1;
        } else if ( Prop->isString() ) {
          if ((Prop->getString()->stringValue()[0] == 'n') ||
              (Prop->getString()->stringValue()[0] == 'N') ||
              (Prop->getString()->stringValue()[0] == '-')) {
            settingsData.Boot.XMPDetection = -1;
          } else {
            settingsData.Boot.XMPDetection = (INT8)AsciiStrDecimalToUintn(Prop->getString()->stringValue().c_str());
          }
        } else if (Prop->isInt64()) {
          settingsData.Boot.XMPDetection = (INT8)Prop->getInt64()->intValue();
        }
        // Check that the setting value is sane
        if ((settingsData.Boot.XMPDetection < -1) || (settingsData.Boot.XMPDetection > 2)) {
          settingsData.Boot.XMPDetection   = -1;
        }
      }

      // Legacy bios protocol
      Prop = BootDict->propertyForKey("Legacy");
      if (Prop != NULL)  {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : Prop property not string in Legacy\n");
        }else{
          settingsData.Boot.LegacyBoot = Prop->getString()->stringValue();
        }
      } else if (gFirmwareClover) {
        // default for CLOVER EFI boot
        settingsData.Boot.LegacyBoot = "PBR"_XS8;
      } else {
        // default for UEFI boot
        settingsData.Boot.LegacyBoot = "LegacyBiosDefault"_XS8;
      }

      // Entry for LegacyBiosDefault
      Prop = BootDict->propertyForKey("LegacyBiosDefaultEntry");
      if (Prop != NULL) {
        settingsData.Boot.LegacyBiosDefaultEntry = (UINT16)GetPropertyAsInteger(Prop, 0); // disabled by default
      }

      // Whether or not to draw boot screen
      Prop = BootDict->propertyForKey("CustomLogo");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          settingsData.Boot.CustomBoot   = CUSTOM_BOOT_APPLE;
        } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          if (Prop->getString()->stringValue().equalIC("Apple")) {
            settingsData.Boot.CustomBoot = CUSTOM_BOOT_APPLE;
          } else if (Prop->getString()->stringValue().equalIC("Alternate")) {
            settingsData.Boot.CustomBoot = CUSTOM_BOOT_ALT_APPLE;
          } else if (Prop->getString()->stringValue().equalIC("Theme")) {
            settingsData.Boot.CustomBoot = CUSTOM_BOOT_THEME;
          } else {
            XStringW customLogo = XStringW() = Prop->getString()->stringValue();
            settingsData.Boot.CustomBoot = CUSTOM_BOOT_USER;
            if (settingsData.Boot.CustomLogo != NULL) {
              delete settingsData.Boot.CustomLogo;
            }
            settingsData.Boot.CustomLogo = new XImage;
            settingsData.Boot.CustomLogo->LoadXImage(&self.getSelfVolumeRootDir(), customLogo);
            if (settingsData.Boot.CustomLogo->isEmpty()) {
              DBG("Custom boot logo not found at path `%ls`!\n", customLogo.wc_str());
              settingsData.Boot.CustomBoot = CUSTOM_BOOT_DISABLED;
            }
          }
        } else if ( Prop->isData()  && Prop->getData()->dataLenValue() > 0 ) {
          settingsData.Boot.CustomBoot = CUSTOM_BOOT_USER;
          if (settingsData.Boot.CustomLogo != NULL) {
            delete settingsData.Boot.CustomLogo;
          }
          settingsData.Boot.CustomLogo = new XImage;
          settingsData.Boot.CustomLogo->FromPNG(Prop->getData()->dataValue(), Prop->getData()->dataLenValue());
          if (settingsData.Boot.CustomLogo->isEmpty()) {
            DBG("Custom boot logo not decoded from data!\n"/*, Prop->getString()->stringValue().c_str()*/);
            settingsData.Boot.CustomBoot = CUSTOM_BOOT_DISABLED;
          }
        } else {
          settingsData.Boot.CustomBoot = CUSTOM_BOOT_USER_DISABLED;
        }
      } else {
        settingsData.Boot.CustomBoot   = CUSTOM_BOOT_DISABLED;
      }
      DBG("Custom boot %s (0x%llX)\n", CustomBootModeToStr(settingsData.Boot.CustomBoot), (uintptr_t)settingsData.Boot.CustomLogo);
    }

    //*** SYSTEM ***
    settingsData.WithKexts            = TRUE;  //default
    const TagDict* SystemParametersDict = CfgDict->dictPropertyForKey("SystemParameters");
    if (SystemParametersDict != NULL) {
      // Inject kexts
      const TagStruct* Prop = SystemParametersDict->propertyForKey("InjectKexts");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          settingsData.WithKexts            = TRUE;
        } else if ((Prop->isString()) &&
                   (Prop->getString()->stringValue().equalIC("Detect"))) {
          //   settingsData.WithKexts            = TRUE;
          settingsData.WithKextsIfNoFakeSMC = TRUE;
        }
      }

      // No caches - obsolete
      Prop = SystemParametersDict->propertyForKey("NoCaches");
      if (IsPropertyNotNullAndTrue(Prop)) {
        settingsData.NoCaches = TRUE;
      }
      //test float - success
//      Prop = SystemParametersDict->propertyForKey("BlueValue");
//      float tmpF = GetPropertyFloat(Prop, 1.2f);
//      DBG(" get BlueValue=%f\n", tmpF);
      
    }

    // KernelAndKextPatches
    const TagDict* KernelAndKextPatchesDict = CfgDict->dictPropertyForKey("KernelAndKextPatches");
    if (KernelAndKextPatchesDict != NULL) {
      FillinKextPatches(&settingsData.KernelAndKextPatches, KernelAndKextPatchesDict);
    }

    const TagDict* GUIDict = CfgDict->dictPropertyForKey("GUI");
    if (GUIDict != NULL) {
      const TagStruct* Prop = GUIDict->propertyForKey("Timezone");
      settingsData.GUI.Timezone = (INT32)GetPropertyAsInteger(Prop, settingsData.GUI.Timezone);
      //initialize Daylight when we know timezone
#ifdef CLOVER_BUILD
      EFI_TIME          Now;
      gRT->GetTime(&Now, NULL);
      INT32 NowHour = Now.Hour + settingsData.GUI.Timezone;
      if (NowHour <  0 ) NowHour += 24;
      if (NowHour >= 24 ) NowHour -= 24;
      ThemeX.Daylight = (NowHour > 8) && (NowHour < 20);
#endif

      Prop = GUIDict->propertyForKey("Theme");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        ThemeX.Theme.takeValueFrom(Prop->getString()->stringValue());
        settingsData.GUI.Theme.takeValueFrom(Prop->getString()->stringValue());
        DBG("Default theme: %ls\n", settingsData.GUI.Theme.wc_str());
        OldChosenTheme = 0xFFFF; //default for embedded
        for (UINTN i = 0; i < ThemeNameArray.size(); i++) {
          //now comparison is case sensitive
          if ( settingsData.GUI.Theme.equalIC(ThemeNameArray[i]) ) {
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
      settingsData.PlayAsync = IsPropertyNotNullAndTrue(Prop);

      // CustomIcons
      Prop = GUIDict->propertyForKey("CustomIcons");
      settingsData.GUI.CustomIcons = IsPropertyNotNullAndTrue(Prop);
      Prop = GUIDict->propertyForKey("TextOnly");
      settingsData.GUI.TextOnly = IsPropertyNotNullAndTrue(Prop);
      Prop = GUIDict->propertyForKey("ShowOptimus");
      settingsData.GUI.ShowOptimus = IsPropertyNotNullAndTrue(Prop);

      Prop = GUIDict->propertyForKey("ScreenResolution");
      if (Prop != NULL) {
        if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          settingsData.GUI.ScreenResolution.takeValueFrom(Prop->getString()->stringValue());
        }
      }

      Prop = GUIDict->propertyForKey("ProvideConsoleGop");
      settingsData.ProvideConsoleGop = !IsPropertyNotNullAndFalse(Prop); //default is true

      Prop = GUIDict->propertyForKey("ConsoleMode");
      if (Prop != NULL) {
        if (Prop->isInt64()) {
          settingsData.GUI.ConsoleMode = Prop->getInt64()->intValue();
        } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          if ( Prop->getString()->stringValue().contains("Max") ) {
            settingsData.GUI.ConsoleMode = -1;
            DBG("ConsoleMode will be set to highest mode\n");
          } else if ( Prop->getString()->stringValue().contains("Min") ) {
            settingsData.GUI.ConsoleMode = -2;
            DBG("ConsoleMode will be set to lowest mode\n");
          } else {
            settingsData.GUI.ConsoleMode = (INT32)AsciiStrDecimalToUintn(Prop->getString()->stringValue());
          }
        }
        if (settingsData.GUI.ConsoleMode > 0) {
          DBG("ConsoleMode will be set to mode #%lld\n", settingsData.GUI.ConsoleMode);
        }
      }

      Prop = GUIDict->propertyForKey("Language");
      if (Prop != NULL) {
        settingsData.Language = Prop->getString()->stringValue();
        if ( Prop->getString()->stringValue().contains("en") ) {
          gLanguage = english;
          settingsData.GUI.Codepage = 0xC0;
          settingsData.GUI.CodepageSize = 0;
        } else if ( Prop->getString()->stringValue().contains("ru")) {
          gLanguage = russian;
          settingsData.GUI.Codepage = 0x410;
          settingsData.GUI.CodepageSize = 0x40;
        } else if ( Prop->getString()->stringValue().contains("ua")) {
          gLanguage = ukrainian;
          settingsData.GUI.Codepage = 0x400;
          settingsData.GUI.CodepageSize = 0x60;
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
          settingsData.GUI.Codepage = 0x3400;
          settingsData.GUI.CodepageSize = 0x19C0;
        } else if ( Prop->getString()->stringValue().contains("ro")) {
          gLanguage = romanian;
        } else if ( Prop->getString()->stringValue().contains("ko")) {
          gLanguage = korean;
          settingsData.GUI.Codepage = 0x1100;
          settingsData.GUI.CodepageSize = 0x100;
        }
      }

//      if (settingsData.Language != NULL) { // settingsData.Language != NULL cannot be false because settingsData.Language is dclared as CHAR8 Language[16]; Must we replace by settingsData.Language[0] != NULL
      Prop = GUIDict->propertyForKey("KbdPrevLang");
      if (Prop != NULL) {
        settingsData.KbdPrevLang = IsPropertyNotNullAndTrue(Prop);
      }
//      }

      const TagDict* MouseDict = GUIDict->dictPropertyForKey("Mouse");
      if (MouseDict != NULL) {
        const TagStruct* prop = MouseDict->propertyForKey("Speed");
        if (prop != NULL) {
          settingsData.PointerSpeed = (INT32)GetPropertyAsInteger(prop, 0);
          settingsData.PointerEnabled = (settingsData.PointerSpeed != 0);
        }
        //but we can disable mouse even if there was positive speed
        prop = MouseDict->propertyForKey("Enabled");
        if (IsPropertyNotNullAndFalse(prop)) {
          settingsData.PointerEnabled = FALSE;
        }

        prop = MouseDict->propertyForKey("Mirror");
        if (IsPropertyNotNullAndTrue(prop)) {
          settingsData.PointerMirror = TRUE;
        }

        prop = MouseDict->propertyForKey("DoubleClickTime");
        if (prop != NULL) {
          settingsData.DoubleClickTime = (UINT64)GetPropertyAsInteger(prop, 500);
        }
      }
      // hide by name/uuid. Array of string
      const TagArray* HideArray = GUIDict->arrayPropertyForKey("Hide");
      if (HideArray != NULL) {
        INTN   i;
        INTN   Count = HideArray->arrayContent().size();
        if (Count > 0) {
          settingsData.HVHideStrings.setEmpty();
          for (i = 0; i < Count; i++) {
            const TagStruct* prop2 = &HideArray->arrayContent()[i];
            if ( !prop2->isString()) {
              MsgLog("MALFORMED PLIST : Hide must be an array of string");
              continue;
            }
            if ( prop2->getString()->stringValue().notEmpty() ) {
              settingsData.HVHideStrings.Add(prop2->getString()->stringValue());
              DBG("Hiding entries with string %s\n", prop2->getString()->stringValue().c_str());
            }
          }
        }
      }
      settingsData.LinuxScan = TRUE;
      // Disable loader scan
      Prop = GUIDict->propertyForKey("Scan");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndFalse(Prop)) {
          settingsData.DisableEntryScan = TRUE;
          settingsData.DisableToolScan  = TRUE;
          settingsData.GUI.NoLegacy      = TRUE;
        } else if (Prop->isDict()) {
          const TagStruct* prop2 = Prop->getDict()->propertyForKey("Entries");
          if (IsPropertyNotNullAndFalse(prop2)) {
            settingsData.DisableEntryScan = TRUE;
          }
          prop2 = Prop->getDict()->propertyForKey("Tool");
          if (IsPropertyNotNullAndFalse(prop2)) {
            settingsData.DisableToolScan = TRUE;
          }
          prop2 = Prop->getDict()->propertyForKey("Linux");
          settingsData.LinuxScan = !IsPropertyNotNullAndFalse(prop2);
          prop2 = Prop->getDict()->propertyForKey("Legacy");
          if (prop2 != NULL) {
            if (prop2->isFalse()) {
              settingsData.GUI.NoLegacy = TRUE;
            } else if ((prop2->isString()) && prop2->getString()->stringValue().notEmpty() ) {
              if ((prop2->getString()->stringValue()[0] == 'N') || (prop2->getString()->stringValue()[0] == 'n')) {
                settingsData.GUI.NoLegacy = TRUE;
              } else if ((prop2->getString()->stringValue()[0] == 'F') || (prop2->getString()->stringValue()[0] == 'f')) {
                settingsData.GUI.LegacyFirst = TRUE;
               }
            }
          }
          prop2 = Prop->getDict()->propertyForKey("Kernel");
          if (prop2 != NULL) {
            if (prop2->isFalse()) {
              settingsData.KernelScan = KERNEL_SCAN_NONE;
            } else if ((prop2->isString()) && prop2->getString()->stringValue().notEmpty() ) {
              if ((prop2->getString()->stringValue()[0] == 'N') || (prop2->getString()->stringValue()[0] == 'n')) {
                settingsData.KernelScan = ( prop2->getString()->stringValue().length() > 1  &&  (prop2->getString()->stringValue()[1] == 'E' || prop2->getString()->stringValue()[1] == 'e') ) ? KERNEL_SCAN_NEWEST : KERNEL_SCAN_NONE;
              } else if ((prop2->getString()->stringValue()[0] == 'O') || (prop2->getString()->stringValue()[0] == 'o')) {
                settingsData.KernelScan = KERNEL_SCAN_OLDEST;
              } else if ((prop2->getString()->stringValue()[0] == 'F') || (prop2->getString()->stringValue()[0] == 'f')) {
                settingsData.KernelScan = KERNEL_SCAN_FIRST;
              } else if ((prop2->getString()->stringValue()[0] == 'L') || (prop2->getString()->stringValue()[0] == 'l')) {
                settingsData.KernelScan = KERNEL_SCAN_LAST;
              } else if ((prop2->getString()->stringValue()[0] == 'M') || (prop2->getString()->stringValue()[0] == 'm')) {
                settingsData.KernelScan = KERNEL_SCAN_MOSTRECENT;
              } else if ((prop2->getString()->stringValue()[0] == 'E') || (prop2->getString()->stringValue()[0] == 'e')) {
                settingsData.KernelScan = KERNEL_SCAN_EARLIEST;
              }
            }
          }
        }
      }
      // Custom entries
      const TagDict* CustomDict2 = GUIDict->dictPropertyForKey("Custom");
      if (CustomDict2 != NULL) {
        const TagArray* arrayProp = CustomDict2->arrayPropertyForKey("Entries"); // Entries is an array of dict
        if (arrayProp != NULL) {
          INTN   Count = arrayProp->arrayContent().size();
          if (Count > 0) {
            for (INTN i = 0; i < Count; i++) {
              const TagDict* Dict3 = arrayProp->dictElementAt(i, "Custom/Entries"_XS8);
              // Allocate an entry
              CUSTOM_LOADER_ENTRY* Entry = new CUSTOM_LOADER_ENTRY;
              // Fill it in
              if ( !FillinCustomEntry(Entry, Dict3, FALSE) || !AddCustomLoaderEntry(Entry) ) {
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
              // Fill it in
              if (!FillingCustomLegacy(Entry, Dict3) || !AddCustomLegacyEntry(Entry)) {
                delete Entry;
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
              // Allocate an entry
              Entry = new CUSTOM_TOOL_ENTRY;
              // Fill it in
              if (!FillingCustomTool(Entry, Dict3) || !AddCustomToolEntry(Entry)) {
                delete Entry;
              }
            }
          }
        }
      }
    }
    const TagDict* GraphicsDict = CfgDict->dictPropertyForKey("Graphics");
    if (GraphicsDict != NULL) {

      const TagStruct* Prop             = GraphicsDict->propertyForKey("PatchVBios");
      settingsData.PatchVBios              = IsPropertyNotNullAndTrue(Prop);

      settingsData.PatchVBiosBytesCount    = 0;

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
          settingsData.PatchVBiosBytes = (__typeof__(settingsData.PatchVBiosBytes))AllocateZeroPool(Count * sizeof(VBIOS_PATCH_BYTES));

          // get all entries
          for (i = 0; i < Count; i++) {
            const TagDict* dict3 = Dict2->dictElementAt(i, "Graphics/PatchVBiosBytes"_XS8);
            Valid = TRUE;
            // read entry
            VBiosPatch          = &settingsData.PatchVBiosBytes[settingsData.PatchVBiosBytesCount];
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
              ++settingsData.PatchVBiosBytesCount;
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

          if (settingsData.PatchVBiosBytesCount == 0) {
            FreePool(settingsData.PatchVBiosBytes);
            settingsData.PatchVBiosBytes = NULL;
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
        settingsData.DisabledDriverArray.setEmpty();

        for (i = 0; i < Count; i++) {
          const TagStruct* Prop = &DisableDriversArray->arrayContent()[i];
          if ( !Prop->isString()) {
            MsgLog("MALFORMED PLIST : DisableDrivers must be an array of string");
            continue;
          }
          settingsData.DisabledDriverArray.Add(Prop->getString()->stringValue());
        }
      }
    }

    const TagDict* DevicesDict = CfgDict->dictPropertyForKey("Devices");
    if (DevicesDict != NULL) {
      const TagDict* Dict2 = DevicesDict->dictPropertyForKey("Audio");
      if (Dict2 != NULL) {
        // HDA
        const TagStruct* Prop = Dict2->propertyForKey("ResetHDA");
        settingsData.ResetHDA = IsPropertyNotNullAndTrue(Prop);
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

    settingsData.mmioWhiteListArray.setEmpty();
 //   const TagDict* OcQuirksDict = CfgDict->dictPropertyForKey("OcQuirks");
//if ( OcQuirksDict ) panic("config.plist/OcQuirks has been renamed Quirks. Update your config.plist");

    const TagDict* OcQuirksDict = CfgDict->dictPropertyForKey("Quirks");
//if ( !OcQuirksDict ) panic("Cannot find config.plist/Quirks");
    if (OcQuirksDict != NULL) {
      const TagStruct* Prop;
      Prop               = OcQuirksDict->propertyForKey("AvoidRuntimeDefrag");
//if ( !Prop ) panic("Cannot find AvoidRuntimeDefrag in OcQuirks under root (OC booter quirks)");
      settingsData.ocBooterQuirks.AvoidRuntimeDefrag = !IsPropertyNotNullAndFalse(Prop); //true if absent so no panic
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.AvoidRuntimeDefrag? QUIRK_DEFRAG:0;
      Prop               = OcQuirksDict->propertyForKey( "DevirtualiseMmio");
      settingsData.ocBooterQuirks.DevirtualiseMmio   = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.DevirtualiseMmio? QUIRK_MMIO:0;
      Prop               = OcQuirksDict->propertyForKey( "DisableSingleUser");
      settingsData.ocBooterQuirks.DisableSingleUser  = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.DisableSingleUser? QUIRK_SU:0;
      Prop               = OcQuirksDict->propertyForKey( "DisableVariableWrite");
      settingsData.ocBooterQuirks.DisableVariableWrite = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.DisableVariableWrite? QUIRK_VAR:0;
      Prop               = OcQuirksDict->propertyForKey( "DiscardHibernateMap");
      settingsData.ocBooterQuirks.DiscardHibernateMap = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.DiscardHibernateMap? QUIRK_HIBER:0;
      Prop               = OcQuirksDict->propertyForKey( "EnableSafeModeSlide");
      settingsData.ocBooterQuirks.EnableSafeModeSlide = !IsPropertyNotNullAndFalse(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.EnableSafeModeSlide? QUIRK_SAFE:0;
      Prop               = OcQuirksDict->propertyForKey( "EnableWriteUnprotector");
      settingsData.ocBooterQuirks.EnableWriteUnprotector = !IsPropertyNotNullAndFalse(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.EnableWriteUnprotector? QUIRK_UNPROT:0;
      Prop               = OcQuirksDict->propertyForKey( "ForceExitBootServices");
      settingsData.ocBooterQuirks.ForceExitBootServices = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.ForceExitBootServices? QUIRK_EXIT:0;
      Prop               = OcQuirksDict->propertyForKey( "ProtectMemoryRegions");
      settingsData.ocBooterQuirks.ProtectMemoryRegions = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.ProtectMemoryRegions? QUIRK_REGION:0;
      Prop               = OcQuirksDict->propertyForKey( "ProtectSecureBoot");
      settingsData.ocBooterQuirks.ProtectSecureBoot = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.ProtectSecureBoot? QUIRK_SECURE:0;
      Prop               = OcQuirksDict->propertyForKey( "ProtectUefiServices");
      settingsData.ocBooterQuirks.ProtectUefiServices = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.ProtectUefiServices? QUIRK_UEFI:0;
      //it is in GUI section
//      Prop               = OcQuirksDict->propertyForKey( "ProvideConsoleGopEnable");
//      settingsData.ProvideConsoleGop = !IsPropertyNotNullAndFalse(Prop);
      Prop               = OcQuirksDict->propertyForKey( "ProvideCustomSlide");
      settingsData.ocBooterQuirks.ProvideCustomSlide = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.ProvideCustomSlide? QUIRK_CUSTOM:0;
      Prop               = OcQuirksDict->propertyForKey( "ProvideMaxSlide");
      settingsData.ocBooterQuirks.ProvideMaxSlide = (UINT8)GetPropertyAsInteger(Prop, 0); // cast will be safe when the new parser will ensure that the value is UINT8
      Prop               = OcQuirksDict->propertyForKey( "RebuildAppleMemoryMap");
      settingsData.ocBooterQuirks.RebuildAppleMemoryMap = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.RebuildAppleMemoryMap? QUIRK_MAP:0;
      Prop               = OcQuirksDict->propertyForKey( "SetupVirtualMap");
      settingsData.ocBooterQuirks.SetupVirtualMap = !IsPropertyNotNullAndFalse(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.SetupVirtualMap? QUIRK_VIRT:0;
      Prop               = OcQuirksDict->propertyForKey( "SignalAppleOS");
      settingsData.ocBooterQuirks.SignalAppleOS = IsPropertyNotNullAndTrue(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.SignalAppleOS? QUIRK_OS:0;
      Prop               = OcQuirksDict->propertyForKey( "SyncRuntimePermissions");
      settingsData.ocBooterQuirks.SyncRuntimePermissions = !IsPropertyNotNullAndFalse(Prop);
      settingsData.QuirksMask  |= settingsData.ocBooterQuirks.SyncRuntimePermissions? QUIRK_PERM:0;
      settingsData.mmioWhiteListArray.setEmpty();

      const TagArray* Dict2 = OcQuirksDict->arrayPropertyForKey("MmioWhitelist"); // array of dict
      if (Dict2 != NULL) {
        INTN   Count = Dict2->arrayContent().size();
        //OC_SCHEMA_INTEGER_IN  ("Address", OC_MMIO_WL_STRUCT, Address),
        //OC_SCHEMA_STRING_IN   ("Comment", OC_MMIO_WL_STRUCT, Comment),
        //OC_SCHEMA_BOOLEAN_IN  ("Enabled", OC_MMIO_WL_STRUCT, Enabled),
        if (Count > 0) {
          for (INTN i = 0; i < Count; i++)
          {
            const TagDict* Dict3 = Dict2->dictElementAt(i, "MmioWhitelist"_XS8);
            MMIOWhiteList* mmioWhiteListPtr = new MMIOWhiteList();
            MMIOWhiteList& mmioWhiteList = *mmioWhiteListPtr;

            const TagStruct* Prop2 = Dict3->propertyForKey("Comment");
            if (Prop2 != NULL && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              mmioWhiteList.comment = Prop2->getString()->stringValue();
            } else {
              mmioWhiteList.comment = " (NoLabel)"_XS8;
            }
            
            Prop2 = Dict3->propertyForKey("Address");
            if (Prop2 != 0) {
              mmioWhiteList.address = GetPropertyAsInteger(Prop2, 0);
              Prop2 = Dict3->propertyForKey("Enabled");
              mmioWhiteList.enabled = IsPropertyNotNullAndTrue(Prop2);
            }
            settingsData.mmioWhiteListArray.AddReference(mmioWhiteListPtr, true);
          }
        }
      }

      Prop = OcQuirksDict->propertyForKey("FuzzyMatch");
      if (Prop != NULL || GlobalConfig.gBootChanged) {
        settingsData.KernelAndKextPatches.FuzzyMatch = !IsPropertyNotNullAndFalse(Prop);
      }

      Prop = OcQuirksDict->propertyForKey("KernelCache");
      if (Prop != NULL || GlobalConfig.gBootChanged) {
        if ( Prop->isString() ) {
          if ( Prop->getString()->stringValue().notEmpty() ) {
            settingsData.KernelAndKextPatches.OcKernelCache = Prop->getString()->stringValue();
          }else{
            settingsData.KernelAndKextPatches.OcKernelCache = "Auto"_XS8;
          }
        }else{
          MsgLog("MALFORMED PLIST : Quirks/KernelCache must be a string");
          settingsData.KernelAndKextPatches.OcKernelCache = "Auto"_XS8;
        }
      }


      // Booter Quirks
//      Prop = OcQuirksDict->propertyForKey("AppleCpuPmCfgLock");
//      settingsData.KernelAndKextPatches.OcKernelQuirks.AppleCpuPmCfgLock = IsPropertyNotNullAndTrue(Prop);
      settingsData.KernelAndKextPatches.OcKernelQuirks.AppleCpuPmCfgLock = settingsData.KernelAndKextPatches.KPAppleIntelCPUPM;

//      Prop = OcQuirksDict->propertyForKey("AppleXcpmCfgLock"); //
//      settingsData.KernelAndKextPatches.OcKernelQuirks.AppleXcpmCfgLock = IsPropertyNotNullAndTrue(Prop);
      settingsData.KernelAndKextPatches.OcKernelQuirks.AppleXcpmCfgLock = settingsData.KernelAndKextPatches.KPKernelPm;

      Prop = OcQuirksDict->propertyForKey("AppleXcpmExtraMsrs");
      settingsData.KernelAndKextPatches.OcKernelQuirks.AppleXcpmExtraMsrs = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("AppleXcpmForceBoost");
      settingsData.KernelAndKextPatches.OcKernelQuirks.AppleXcpmForceBoost = IsPropertyNotNullAndTrue(Prop);

// We can't use that Quirks because we don't delegate SMBios to OC.
//      Prop = OcQuirksDict->propertyForKey("CustomSMBIOSGuid");
//      settingsData.KernelAndKextPatches.OcKernelQuirks.CustomSmbiosGuid = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("DisableIoMapper");
//if ( !Prop ) panic("Cannot find DisableIoMapper in config.plist/Quirks. You forgot to merge your quirks into one section. Update your config.plist");
      settingsData.KernelAndKextPatches.OcKernelQuirks.DisableIoMapper = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("DisableLinkeditJettison");
      settingsData.KernelAndKextPatches.OcKernelQuirks.DisableLinkeditJettison = IsPropertyNotNullAndTrue(Prop);

 //     Prop = OcQuirksDict->propertyForKey("DisableRtcChecksum");
 //     settingsData.KernelAndKextPatches.OcKernelQuirks.DisableRtcChecksum = IsPropertyNotNullAndTrue(Prop);
      settingsData.KernelAndKextPatches.OcKernelQuirks.DisableRtcChecksum = settingsData.KernelAndKextPatches.KPAppleRTC;

      Prop = OcQuirksDict->propertyForKey("DummyPowerManagement");
      settingsData.KernelAndKextPatches.OcKernelQuirks.DummyPowerManagement = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("ExternalDiskIcons");
      settingsData.KernelAndKextPatches.OcKernelQuirks.ExternalDiskIcons = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("IncreasePciBarSize");
      settingsData.KernelAndKextPatches.OcKernelQuirks.IncreasePciBarSize = IsPropertyNotNullAndTrue(Prop);

 //     Prop = OcQuirksDict->propertyForKey("LapicKernelPanic");
 //     settingsData.KernelAndKextPatches.OcKernelQuirks.LapicKernelPanic = IsPropertyNotNullAndTrue(Prop);
      settingsData.KernelAndKextPatches.OcKernelQuirks.LapicKernelPanic = settingsData.KernelAndKextPatches.KPKernelLapic;

//      Prop = OcQuirksDict->propertyForKey("PanicNoKextDump");
//      settingsData.KernelAndKextPatches.OcKernelQuirks.PanicNoKextDump = IsPropertyNotNullAndTrue(Prop); //KPPanicNoKextDump
      settingsData.KernelAndKextPatches.OcKernelQuirks.PanicNoKextDump = settingsData.KernelAndKextPatches.KPPanicNoKextDump;

      Prop = OcQuirksDict->propertyForKey("PowerTimeoutKernelPanic");
      settingsData.KernelAndKextPatches.OcKernelQuirks.PowerTimeoutKernelPanic = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("ThirdPartyDrives");
      settingsData.KernelAndKextPatches.OcKernelQuirks.ThirdPartyDrives = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("XhciPortLimit");
      settingsData.KernelAndKextPatches.OcKernelQuirks.XhciPortLimit = IsPropertyNotNullAndTrue(Prop);
    }
  }

  return Status;
}


void
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
      if (IsValidGuidString(Prop->getString()->stringValue())) {
        gSettings.SmUUID = Prop->getString()->stringValue();
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
//          gSettings.DSDTPatchArray.size()      = (UINT32)Count;
//          gSettings.PatchDsdtFind = (__typeof__(gSettings.PatchDsdtFind))AllocateZeroPool(Count * sizeof(UINT8*));
//          gSettings.PatchDsdtReplace = (__typeof__(gSettings.PatchDsdtReplace))AllocateZeroPool(Count * sizeof(UINT8*));
//          gSettings.PatchDsdtTgt = (__typeof__(gSettings.PatchDsdtTgt))AllocateZeroPool(Count * sizeof(UINT8*));
//          gSettings.LenToFind = (__typeof__(gSettings.LenToFind))AllocateZeroPool(Count * sizeof(UINT32));
//          gSettings.LenToReplace = (__typeof__(gSettings.LenToReplace))AllocateZeroPool(Count * sizeof(UINT32));
//          gSettings.PatchDsdtLabel = (__typeof__(gSettings.PatchDsdtLabel))AllocateZeroPool(Count * sizeof(UINT8*));
//          gSettings.PatchDsdtMenuItem = new INPUT_ITEM[Count];
          DBG("PatchesDSDT: %lld requested\n", Count);
          gSettings.DSDTPatchArray.setEmpty();
          for (i = 0; i < Count; i++)
          {
            DSDT_Patch* dsdtPatchPtr = new DSDT_Patch();
            DSDT_Patch& dsdtPatch = *dsdtPatchPtr;
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
            dsdtPatch.PatchDsdtLabel = DSDTPatchesLabel;
            DBG(" (%s)", dsdtPatch.PatchDsdtLabel.c_str());
            
            Prop3 = Prop2->propertyForKey("Disabled");
            dsdtPatch.PatchDsdtMenuItem.BValue = !IsPropertyNotNullAndTrue(Prop3);
            
            //DBG(" DSDT bin patch #%d ", i);
            UINT8* data = GetDataSetting (Prop2, "Find", &Size);
            dsdtPatch.PatchDsdtFind.stealValueFrom(data, Size);
            DBG(" lenToFind: %zu", dsdtPatch.PatchDsdtFind.size());
            data = GetDataSetting (Prop2, "Replace",  &Size);
            dsdtPatch.PatchDsdtReplace.stealValueFrom(data, Size);
            DBG(", lenToReplace: %zu", dsdtPatch.PatchDsdtReplace.size());
            data = GetDataSetting (Prop2, "TgtBridge", &Size);
            if ( Size != 0 ) {
              if ( Size != 4 ) {
                DBG("\n          TgtBridge must 4 bytes. It's %llu byte(s). Ignored\n", Size);
              }else{
                dsdtPatch.PatchDsdtTgt.stealValueFrom(data, Size);
                DBG(", Target Bridge: %c%c%c%c", dsdtPatch.PatchDsdtTgt[0], dsdtPatch.PatchDsdtTgt[1], dsdtPatch.PatchDsdtTgt[2], dsdtPatch.PatchDsdtTgt[3]);
              }
            }
            DBG("\n");
            if (!dsdtPatch.PatchDsdtMenuItem.BValue) {
              DBG("  patch disabled at config\n");
            }
            gSettings.DSDTPatchArray.AddReference(dsdtPatchPtr, true);
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
      if (Prop) {
        gSettings.EnableC7 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC7: %s\n", gSettings.EnableC7 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC6");
      if (Prop) {
        gSettings.EnableC6 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC6: %s\n", gSettings.EnableC6 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC4");
      if (Prop) {
        gSettings.EnableC4 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC4: %s\n", gSettings.EnableC4 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC2");
      if (Prop) {
        gSettings.EnableC2 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC2: %s\n", gSettings.EnableC2 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("C3Latency");
        gSettings.C3Latency = (UINT16)GetPropertyAsInteger(Prop, gSettings.C3Latency);
        DBG("C3Latency: %d\n", gSettings.C3Latency);
      
      Prop                       = SSDTDict->propertyForKey("PLimitDict");
      gSettings.PLimitDict       = (UINT8)GetPropertyAsInteger(Prop, 0);
      
      Prop                       = SSDTDict->propertyForKey("UnderVoltStep");
      gSettings.UnderVoltStep    = (UINT8)GetPropertyAsInteger(Prop, 0);
      
      Prop                       = SSDTDict->propertyForKey("DoubleFirstState");
      gSettings.DoubleFirstState = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("MinMultiplier");
        gSettings.MinMultiplier  = (UINT8)GetPropertyAsInteger(Prop, gSettings.MinMultiplier);
        DBG("MinMultiplier: %d\n", gSettings.MinMultiplier);
      
      Prop = SSDTDict->propertyForKey("MaxMultiplier");
        gSettings.MaxMultiplier = (UINT8)GetPropertyAsInteger(Prop, gSettings.MaxMultiplier);
        DBG("MaxMultiplier: %d\n", gSettings.MaxMultiplier);
      
      Prop = SSDTDict->propertyForKey("PluginType");
        gSettings.PluginType = (UINT8)GetPropertyAsInteger(Prop, gSettings.PluginType);
        DBG("PluginType: %d\n", gSettings.PluginType);
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
      gSettings.ResetVal = (UINT8)GetPropertyAsInteger(Prop, gSettings.ResetVal);
      DBG("ResetVal: 0x%hhX\n", gSettings.ResetVal);
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
    if (SortedOrderArray) {
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
GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& settingsData)
{
//  EFI_STATUS Status = EFI_NOT_FOUND;

  if (CfgDict != NULL) {
    DbgHeader ("GetUserSettings");

    // Boot settings.
    // Discussion. Why Arguments is here? It should be SystemParameters property!
    // we will read them again because of change in GUI menu. It is not only EarlySettings
    //
    const TagDict* BootDict = CfgDict->dictPropertyForKey("Boot");
    if (BootDict != NULL) {

      const TagStruct* Prop = BootDict->propertyForKey("Arguments");
      if ( Prop != NULL  &&  Prop->isString()  &&  Prop->getString()->stringValue().notEmpty()  &&  !settingsData.Boot.BootArgs.contains(Prop->getString()->stringValue()) ) {
        settingsData.Boot.BootArgs = Prop->getString()->stringValue();
        //gBootArgsChanged = TRUE;
        //GlobalConfig.gBootChanged = TRUE;
      }

      Prop                     = BootDict->propertyForKey("NeverDoRecovery");
      settingsData.NeverDoRecovery  = IsPropertyNotNullAndTrue(Prop);
    }


    //Graphics

    const TagDict* GraphicsDict = CfgDict->dictPropertyForKey("Graphics");
    if (GraphicsDict != NULL) {
      INTN i;
      const TagStruct* Prop     = GraphicsDict->propertyForKey("Inject");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          settingsData.GraphicsInjector = TRUE;
          settingsData.InjectIntel      = TRUE;
          settingsData.InjectATI        = TRUE;
          settingsData.InjectNVidia     = TRUE;
        } else if (Prop->isDict()) {
          const TagDict* Dict2 = Prop->getDict();
          const TagStruct* Prop2 = Dict2->propertyForKey("Intel");
          if (Prop2 != NULL) {
            settingsData.InjectIntel = IsPropertyNotNullAndTrue(Prop2);
          }

          Prop2 = Dict2->propertyForKey("ATI");
          if (Prop2 != NULL) {
            settingsData.InjectATI = IsPropertyNotNullAndTrue(Prop2);
          }

          Prop2 = Dict2->propertyForKey("NVidia");
          if (Prop2 != NULL) {
            settingsData.InjectNVidia = IsPropertyNotNullAndTrue(Prop2);
          }
        } else {
          settingsData.GraphicsInjector = FALSE;
          settingsData.InjectIntel      = FALSE;
          settingsData.InjectATI        = FALSE;
          settingsData.InjectNVidia     = FALSE;
        }
      }

      Prop = GraphicsDict->propertyForKey("RadeonDeInit");
      settingsData.DeInit = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("VRAM");
      settingsData.VRAM = (UINTN)GetPropertyAsInteger(Prop, (INTN)settingsData.VRAM); //Mb
      //
      Prop = GraphicsDict->propertyForKey("RefCLK");
      settingsData.RefCLK = (UINT16)GetPropertyAsInteger(Prop, 0);

      Prop = GraphicsDict->propertyForKey("LoadVBios");
      settingsData.LoadVBios = IsPropertyNotNullAndTrue(Prop);

      for (i = 0; i < (INTN)NGFX; i++) {
        gGraphics[i].LoadVBios = settingsData.LoadVBios; //default
      }

      Prop = GraphicsDict->propertyForKey("VideoPorts");
      settingsData.VideoPorts   = (UINT16)GetPropertyAsInteger(Prop, settingsData.VideoPorts);

      Prop = GraphicsDict->propertyForKey("BootDisplay");
      settingsData.BootDisplay = (INT8)GetPropertyAsInteger(Prop, -1);

      Prop = GraphicsDict->propertyForKey("FBName");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in FBName\n");
        }else{
          settingsData.FBName = Prop->getString()->stringValue();
        }
      }

      Prop = GraphicsDict->propertyForKey("NVCAP");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in NVCAP\n");
        }else{
          hex2bin (Prop->getString()->stringValue().c_str(), (UINT8*)&settingsData.NVCAP[0], 20);
          DBG("Read NVCAP:");

          for (i = 0; i<20; i++) {
            DBG("%02hhX", settingsData.NVCAP[i]);
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
          hex2bin (Prop->getString()->stringValue().c_str(), (UINT8*)&settingsData.Dcfg[0], 8);
        }
      }

      Prop = GraphicsDict->propertyForKey("DualLink");
      settingsData.DualLink = (UINT32)GetPropertyAsInteger(Prop, settingsData.DualLink);

      //InjectEDID - already done in earlysettings
      //No! Take again
      GetEDIDSettings(GraphicsDict);

      // ErmaC: NvidiaGeneric
      Prop = GraphicsDict->propertyForKey("NvidiaGeneric");
      settingsData.NvidiaGeneric = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("NvidiaNoEFI");
      settingsData.NvidiaNoEFI = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("NvidiaSingle");
      settingsData.NvidiaSingle = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("ig-platform-id");
      settingsData.IgPlatform = (UINT32)GetPropertyAsInteger(Prop, settingsData.IgPlatform);

      Prop = GraphicsDict->propertyForKey("snb-platform-id");
      settingsData.IgPlatform = (UINT32)GetPropertyAsInteger(Prop, settingsData.IgPlatform);

      FillCardList(GraphicsDict); //#@ Getcardslist
    }

    const TagDict* DevicesDict = CfgDict->dictPropertyForKey("Devices");
    if (DevicesDict != NULL) {
      const TagStruct* Prop = DevicesDict->propertyForKey("Inject");
      settingsData.StringInjector = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("SetIntelBacklight");
      settingsData.IntelBacklight = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("SetIntelMaxBacklight");
      settingsData.IntelMaxBacklight = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("IntelMaxValue");
      settingsData.IntelMaxValue = (UINT16)GetPropertyAsInteger(Prop, settingsData.IntelMaxValue);

      /*
       * Properties is a single string, or a dict
       */
      Prop = DevicesDict->propertyForKey("Properties");
      if (Prop != NULL) {
        if (Prop->isString()) {

          cDeviceProperties = Prop->getString()->stringValue();
          //-------
#ifdef CLOVER_BUILD
          EFI_PHYSICAL_ADDRESS  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
          EFI_STATUS Status = gBS->AllocatePages (
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
#endif
        }
        else if ( Prop->isDict() ) {
          INTN i;
          const TagDict* PropertiesDict = Prop->getDict();
          INTN Count = PropertiesDict->dictKeyCount(); //ok
          settingsData.AddProperties = new DEV_PROPERTY[Count];
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
#ifdef CLOVER_BUILD
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
#endif
                //Create Device node
                DevPropDevice = settingsData.ArbProperties;
                settingsData.ArbProperties = new DEV_PROPERTY;
                settingsData.ArbProperties->Next = DevPropDevice; //next device
                settingsData.ArbProperties->Child = NULL;
                settingsData.ArbProperties->Device = 0; //to differ from arbitrary
                settingsData.ArbProperties->DevicePath = DevicePath; //this is pointer
                settingsData.ArbProperties->Label = S8Printf("%s", key->keyStringValue().c_str()).forgetDataWithoutFreeing();
                Child = &(settingsData.ArbProperties->Child);

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
      settingsData.LANInjection = !IsPropertyNotNullAndFalse(Prop);  //default = TRUE

      Prop  = DevicesDict->propertyForKey("HDMIInjection");
      settingsData.HDMIInjection = IsPropertyNotNullAndTrue(Prop);

      Prop  = DevicesDict->propertyForKey("NoDefaultProperties");
      settingsData.NoDefaultProperties = !IsPropertyNotNullAndFalse(Prop);

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
                DevProp = settingsData.ArbProperties;
                settingsData.ArbProperties = new DEV_PROPERTY;
                settingsData.ArbProperties->Next = DevProp;

                settingsData.ArbProperties->Device = (UINT32)DeviceAddr;
                settingsData.ArbProperties->Label = (__typeof__(settingsData.ArbProperties->Label))AllocateCopyPool(Label.sizeInBytesIncludingTerminator(), Label.c_str());

                const TagStruct* DisabledProp = Dict3->propertyForKey("Disabled");
                settingsData.ArbProperties->MenuItem.BValue = !IsPropertyNotNullAndTrue(DisabledProp);

                DisabledProp = Dict3->propertyForKey("Key");
                if (DisabledProp && (DisabledProp->isString()) && DisabledProp->getString()->stringValue().notEmpty()) {
                  settingsData.ArbProperties->Key = S8Printf("%s", DisabledProp->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
                }

                DisabledProp = Dict3->propertyForKey("Value");
                if (DisabledProp && (DisabledProp->isString()) && DisabledProp->getString()->stringValue().notEmpty()) {
                  //first suppose it is Ascii string
                  settingsData.ArbProperties->Value = (UINT8*)S8Printf("%s", DisabledProp->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
                  settingsData.ArbProperties->ValueLen = DisabledProp->getString()->stringValue().sizeInBytesIncludingTerminator();
                  settingsData.ArbProperties->ValueType = kTagTypeString;
                } else if (DisabledProp && (DisabledProp->isInt64())) {
                  if ( DisabledProp->getInt64()->intValue() < MIN_INT32  ||  DisabledProp->getInt64()->intValue() > MAX_INT32 ) {
                    MsgLog("Invalid int value for key 'Value'\n");
                  }else{
                    INT32 intValue = (INT32)DisabledProp->getInt64()->intValue();
                    settingsData.ArbProperties->Value = (__typeof__(settingsData.ArbProperties->Value))AllocatePool(sizeof(intValue));
  //                    CopyMem(settingsData.ArbProperties->Value, &Prop3->intValue, 4);
                    *(INT32*)(settingsData.ArbProperties->Value) = intValue;
                    settingsData.ArbProperties->ValueLen = sizeof(intValue);
                    settingsData.ArbProperties->ValueType = kTagTypeInteger;
                  }
                } else if ( DisabledProp && DisabledProp->isTrue() ) {
                  settingsData.ArbProperties->Value = (__typeof__(settingsData.ArbProperties->Value))AllocateZeroPool(4);
                  settingsData.ArbProperties->Value[0] = TRUE;
                  settingsData.ArbProperties->ValueLen = 1;
                  settingsData.ArbProperties->ValueType = kTagTypeTrue;
                } else if ( DisabledProp && DisabledProp->isFalse() ) {
                  settingsData.ArbProperties->Value = (__typeof__(settingsData.ArbProperties->Value))AllocateZeroPool(4);
                  //settingsData.ArbProperties->Value[0] = FALSE;
                  settingsData.ArbProperties->ValueLen = 1;
                  settingsData.ArbProperties->ValueType = kTagTypeFalse;
                } else {
                  //else  data
                  UINTN Size = 0;
                  settingsData.ArbProperties->Value = GetDataSetting (Dict3, "Value", &Size);
                  settingsData.ArbProperties->ValueLen = Size;
                  settingsData.ArbProperties->ValueType = kTagTypeData;
                }

                //Special case. In future there must be more such cases
                if ((AsciiStrStr(settingsData.ArbProperties->Key, "-platform-id") != NULL)) {
                  CopyMem((CHAR8*)&settingsData.IgPlatform, settingsData.ArbProperties->Value, 4);
                }
              }   //for() device properties
            }
          } //for() devices
        }
        //        settingsData.NrAddProperties = 0xFFFE;
      }
      //can use AddProperties with ArbProperties
      const TagArray* AddPropertiesArray = DevicesDict->arrayPropertyForKey("AddProperties"); // array of dict
      if (AddPropertiesArray != NULL) {
        INTN i;
        INTN Count = AddPropertiesArray->arrayContent().size();
        INTN Index = 0;  //begin from 0 if second enter
//count = 0x1F1E1D1C1B1A1918
        if (Count > 0) {
      DBG("Add %lld properties:\n", Count);
          settingsData.AddProperties = new DEV_PROPERTY[Count];

          for (i = 0; i < Count; i++) {
            UINTN Size = 0;
        DBG(" - [%02lld]:", i);
            const TagDict* Dict2 = AddPropertiesArray->dictElementAt(i, "AddProperties"_XS8);
            const TagStruct* DeviceProp = Dict2->propertyForKey("Device");
            if (DeviceProp && (DeviceProp->isString()) && DeviceProp->getString()->stringValue().notEmpty()) {
              DEV_PROPERTY *Property = &settingsData.AddProperties[Index];

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
            settingsData.AddProperties[Index].MenuItem.BValue = !IsPropertyNotNullAndTrue(Prop2);

            Prop2 = Dict2->propertyForKey("Key");
            if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
              settingsData.AddProperties[Index].Key = S8Printf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
            }

            Prop2 = Dict2->propertyForKey("Value");
            if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
              //first suppose it is Ascii string
              settingsData.AddProperties[Index].Value = (UINT8*)S8Printf("%s", Prop2->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
              settingsData.AddProperties[Index].ValueLen = Prop2->getString()->stringValue().sizeInBytesIncludingTerminator();
            } else if (Prop2 && (Prop2->isInt64())) {
              if ( Prop2->getInt64()->intValue() < MIN_INT32  ||  Prop2->getInt64()->intValue() > MAX_INT32 ) {
                MsgLog("Invalid int value for key 'Value'\n");
              }else{
                INT32 intValue = (INT32)Prop2->getInt64()->intValue();
                settingsData.AddProperties[Index].Value = (__typeof__(settingsData.AddProperties[Index].Value))AllocatePool (sizeof(intValue));
  //              CopyMem(settingsData.AddProperties[Index].Value, &Prop2->intValue, 4);
                *(INT32*)(settingsData.AddProperties[Index].Value) = intValue;
                settingsData.AddProperties[Index].ValueLen = sizeof(intValue);
              }
            } else {
              //else  data
              settingsData.AddProperties[Index].Value = GetDataSetting (Dict2, "Value", &Size);
              settingsData.AddProperties[Index].ValueLen = Size;
            }

        DBG("Key: %s, len: %llu\n", settingsData.AddProperties[Index].Key, settingsData.AddProperties[Index].ValueLen);

            if (!settingsData.AddProperties[Index].MenuItem.BValue) {
              DBG("  property disabled at config\n");
            }

            ++Index;
          }

          settingsData.NrAddProperties = Index;
        }
      }
      //end AddProperties

      const TagDict* FakeIDDict = DevicesDict->dictPropertyForKey("FakeID");
      if (FakeIDDict != NULL) {
        const TagStruct* Prop2 = FakeIDDict->propertyForKey("ATI");
        if (Prop2 && (Prop2->isString())) {
          settingsData.FakeATI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("NVidia");
        if (Prop2 && (Prop2->isString())) {
          settingsData.FakeNVidia  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("IntelGFX");
        if (Prop2 && (Prop2->isString())) {
          settingsData.FakeIntel  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("LAN");
        if (Prop2 && (Prop2->isString())) {
          settingsData.FakeLAN  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("WIFI");
        if (Prop2 && (Prop2->isString())) {
          settingsData.FakeWIFI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("SATA");
        if (Prop2 && (Prop2->isString())) {
          settingsData.FakeSATA  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("XHCI");
        if (Prop2 && (Prop2->isString())) {
          settingsData.FakeXHCI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("IMEI");
        if (Prop2 && (Prop2->isString())) {
          settingsData.FakeIMEI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }
      }

      Prop                   = DevicesDict->propertyForKey("UseIntelHDMI");
      settingsData.UseIntelHDMI = IsPropertyNotNullAndTrue(Prop);

      Prop                = DevicesDict->propertyForKey("ForceHPET");
      settingsData.ForceHPET = IsPropertyNotNullAndTrue(Prop);

      Prop                = DevicesDict->propertyForKey("DisableFunctions");
      if (Prop && (Prop->isString())) {
        settingsData.DisableFunctions  = (UINT32)AsciiStrHexToUint64(Prop->getString()->stringValue());
      }

      Prop                = DevicesDict->propertyForKey("AirportBridgeDeviceName");
      if (Prop && (Prop->isString())) {
        if ( Prop->getString()->stringValue().length() != 4 ) {
           MsgLog("ERROR IN PLIST : AirportBridgeDeviceName must 4 chars long");
        }else{
          settingsData.AirportBridgeDeviceName = Prop->getString()->stringValue();
        }
      }

      const TagDict* AudioDict = DevicesDict->dictPropertyForKey("Audio");
      if (AudioDict != NULL) {
        // HDA
        //       Prop = GetProperty(Prop2, "ResetHDA");
        //       settingsData.ResetHDA = IsPropertyTrue(Prop);
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
            settingsData.HDALayoutId = (INT32)Prop->getInt64()->intValue(); //must be signed
            settingsData.HDAInjection = (settingsData.HDALayoutId > 0);
          } else if (Prop->isString()){
            if ( Prop->getString()->stringValue().notEmpty()  &&  (Prop->getString()->stringValue()[0] == 'n' || Prop->getString()->stringValue()[0] == 'N') ) {
              // if starts with n or N, then no HDA injection
              settingsData.HDAInjection = FALSE;
            } else if ( Prop->getString()->stringValue().length() > 1  &&
                        Prop->getString()->stringValue()[0] == '0'  &&
                        ( Prop->getString()->stringValue()[1] == 'x' || Prop->getString()->stringValue()[1] == 'X' ) ) {
              // assume it's a hex layout id
              settingsData.HDALayoutId = (INT32)AsciiStrHexToUintn(Prop->getString()->stringValue());
              settingsData.HDAInjection = TRUE;
            } else {
              // assume it's a decimal layout id
              settingsData.HDALayoutId = (INT32)AsciiStrDecimalToUintn(Prop->getString()->stringValue());
              settingsData.HDAInjection = TRUE;
            }
          }
        }

        Prop = AudioDict->propertyForKey("AFGLowPowerState");
        settingsData.AFGLowPowerState = IsPropertyNotNullAndTrue(Prop);
      }

      const TagDict* USBDict = DevicesDict->dictPropertyForKey("USB");
      if (USBDict != NULL) {
        // USB
        Prop = USBDict->propertyForKey("Inject");
        settingsData.USBInjection = !IsPropertyNotNullAndFalse(Prop); // enabled by default

        Prop = USBDict->propertyForKey("AddClockID");
        settingsData.InjectClockID = IsPropertyNotNullAndTrue(Prop); // disabled by default
        // enabled by default for CloverEFI
        // disabled for others
        settingsData.USBFixOwnership = gFirmwareClover;
        Prop = USBDict->propertyForKey("FixOwnership");
        if (Prop != NULL) {
          settingsData.USBFixOwnership = IsPropertyNotNullAndTrue(Prop);
        }
        DBG("USB FixOwnership: %s\n", settingsData.USBFixOwnership?"yes":"no");

        Prop = USBDict->propertyForKey("HighCurrent");
        settingsData.HighCurrent = IsPropertyNotNullAndTrue(Prop);

        Prop = USBDict->propertyForKey("NameEH00");
        settingsData.NameEH00 = IsPropertyNotNullAndTrue(Prop);
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
          settingsData.TrustSMBIOS = FALSE;
        } else if (IsPropertyNotNullAndTrue(Prop)) {
          settingsData.TrustSMBIOS = TRUE;
        }
      }
      Prop = SMBIOSDict->propertyForKey("MemoryRank");
      settingsData.Attribute = (INT8)GetPropertyAsInteger(Prop, -1); //1==Single Rank, 2 == Dual Rank, 0==undefined -1 == keep as is

      // Delete the user memory when a new config is selected
      INTN i = 0;
      for (i = 0; i < gRAM.UserInUse && i < MAX_RAM_SLOTS; i++) {
        gRAM.User[i].ModuleSize = 0;
        gRAM.User[i].InUse = 0;
      }
      gRAM.UserInUse = 0;
      gRAM.UserChannels = 0;
      settingsData.InjectMemoryTables = FALSE;

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
              Slot = (UINT8)Prop2->getInt64()->intValue();
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
            settingsData.InjectMemoryTables = TRUE;
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
    settingsData.CpuType = GetAdvancedCpuType(); //let it be default
    settingsData.SavingMode = 0xFF; //default
    const TagDict* CPUDict = CfgDict->dictPropertyForKey("CPU");
    if (CPUDict != NULL) {
      const TagStruct* Prop = CPUDict->propertyForKey("QPI");
      if (Prop != NULL) {
        settingsData.QPI = (UINT16)GetPropertyAsInteger(Prop, settingsData.QPI);
        DBG("QPI: %dMHz\n", settingsData.QPI);
      }

      Prop = CPUDict->propertyForKey("FrequencyMHz");
      if (Prop != NULL) {
        settingsData.CpuFreqMHz = (UINT32)GetPropertyAsInteger(Prop, settingsData.CpuFreqMHz);
        DBG("CpuFreq: %dMHz\n", settingsData.CpuFreqMHz);
      }

      Prop = CPUDict->propertyForKey("Type");
      if (Prop != NULL) {
        settingsData.CpuType = (UINT16)GetPropertyAsInteger(Prop, settingsData.CpuType);
        DBG("CpuType: %hX\n", settingsData.CpuType);
      }

      Prop = CPUDict->propertyForKey("QEMU");
      settingsData.QEMU = IsPropertyNotNullAndTrue(Prop);
      if (settingsData.QEMU) {
        DBG("QEMU: true\n");
      }

      Prop = CPUDict->propertyForKey("UseARTFrequency");
      if (Prop != NULL) {
        settingsData.UseARTFreq = IsPropertyNotNullAndTrue(Prop);
      }

      settingsData.UserChange = FALSE;
      Prop = CPUDict->propertyForKey("BusSpeedkHz");
      if (Prop != NULL) {
        settingsData.BusSpeed = (UINT32)GetPropertyAsInteger(Prop, settingsData.BusSpeed);
        DBG("BusSpeed: %dkHz\n", settingsData.BusSpeed);
        settingsData.UserChange = TRUE;
      }

      Prop = CPUDict->propertyForKey("C6");
      if (Prop != NULL) {
        settingsData.EnableC6 = IsPropertyNotNullAndTrue(Prop);
      }

      Prop = CPUDict->propertyForKey("C4");
      if (Prop != NULL) {
        settingsData.EnableC4 = IsPropertyNotNullAndTrue(Prop);
      }

      Prop = CPUDict->propertyForKey("C2");
      if (Prop != NULL) {
        settingsData.EnableC2 = IsPropertyNotNullAndTrue(Prop);
      }

      //Usually it is 0x03e9, but if you want Turbo, you may set 0x00FA
      Prop                 = CPUDict->propertyForKey("Latency");
      settingsData.C3Latency  = (UINT16)GetPropertyAsInteger(Prop, settingsData.C3Latency);

      Prop                 = CPUDict->propertyForKey("SavingMode");
      settingsData.SavingMode = (UINT8)GetPropertyAsInteger(Prop, 0xFF); //the default value means not set

      Prop                 = CPUDict->propertyForKey("HWPEnable");
      if (Prop && IsPropertyNotNullAndTrue(Prop) && (gCPUStructure.Model >= CPU_MODEL_SKYLAKE_U)) {
        settingsData.HWP = TRUE;
#ifdef CLOVER_BUILD
        AsmWriteMsr64 (MSR_IA32_PM_ENABLE, 1);
#endif
      }
      Prop                 = CPUDict->propertyForKey("HWPValue");
      if (Prop && settingsData.HWP) {
        settingsData.HWPValue = (UINT32)GetPropertyAsInteger(Prop, 0);
#ifdef CLOVER_BUILD
        AsmWriteMsr64 (MSR_IA32_HWP_REQUEST, settingsData.HWPValue);
#endif
      }

      Prop                 = CPUDict->propertyForKey("TDP");
      settingsData.TDP  = (UINT8)GetPropertyAsInteger(Prop, 0);

      Prop                 = CPUDict->propertyForKey("TurboDisable");
      if (Prop && IsPropertyNotNullAndTrue(Prop)) {
#ifdef CLOVER_BUILD
        UINT64 msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
#endif
        settingsData.Turbo = 0;
#ifdef CLOVER_BUILD
        msr &= ~(1ULL<<38);
        AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, msr);
#endif
      }
    }

    // RtVariables
    settingsData.RtROM.setEmpty();
    const TagDict* RtVariablesDict = CfgDict->dictPropertyForKey("RtVariables");
    if (RtVariablesDict != NULL) {
      // ROM: <data>bin data</data> or <string>base 64 encoded bin data</string>
      const TagStruct* Prop = RtVariablesDict->propertyForKey("ROM");
      if (Prop != NULL) {
        if ( Prop->isString()  &&  Prop->getString()->stringValue().equalIC("UseMacAddr0") ) {
          settingsData.RtROM.ncpy(&gLanMac[0][0], 6);
        } else if ( Prop->isString()  &&  Prop->getString()->stringValue().equalIC("UseMacAddr1") ) {
          settingsData.RtROM.ncpy(&gLanMac[1][0], 6);
        } else if ( Prop->isString()  ||  Prop->isData() ) { // GetDataSetting accept both
          UINTN ROMLength         = 0;
          void* ROM = GetDataSetting(RtVariablesDict, "ROM", &ROMLength);
          settingsData.RtROM.ncpy(ROM, ROMLength);
        } else {
          MsgLog("MALFORMED PLIST : property not string or data in RtVariables/ROM\n");
        }
      }

      // MLB: <string>some value</string>
      Prop = RtVariablesDict->propertyForKey("MLB");
      if ( Prop != NULL ) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in RtVariables/MLB\n");
        }else{
          if( Prop->getString()->stringValue().notEmpty() ) {
            settingsData.RtMLB = Prop->getString()->stringValue();
          }
        }
      }
      // CsrActiveConfig
      Prop = RtVariablesDict->propertyForKey("CsrActiveConfig");
      settingsData.CsrActiveConfig = (UINT32)GetPropertyAsInteger(Prop, 0x2E7); //the value 0xFFFF means not set

      //BooterConfig
      Prop = RtVariablesDict->propertyForKey("BooterConfig");
      settingsData.BooterConfig = (UINT16)GetPropertyAsInteger(Prop, 0); //the value 0 means not set
      //let it be string like "log=0"
      Prop = RtVariablesDict->propertyForKey("BooterCfg");
      if ( Prop != NULL ) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in RtVariables/BooterCfg\n");
        }else{
          if( Prop->getString()->stringValue().notEmpty() ) {
            settingsData.BooterCfgStr = Prop->getString()->stringValue();
          }
        }
      }
      //Block external variables
      const TagArray* BlockArray = RtVariablesDict->arrayPropertyForKey("Block"); // array of dict
      if (BlockArray != NULL) {
        INTN   i;
        INTN Count = BlockArray->arrayContent().size();
        BlockRtVariableArray.setEmpty();
        RT_VARIABLES* RtVariablePtr = new RT_VARIABLES();
        RT_VARIABLES& RtVariable = *RtVariablePtr;
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
                if (IsValidGuidString(Prop2->getString()->stringValue())) {
                  StrToGuidLE(Prop2->getString()->stringValue(), &RtVariable.VarGuid);
                }else{
                 DBG("Error: invalid GUID for RT var '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->getString()->stringValue().c_str());
                }
              }
            }
          }

          Prop2 = CfgDict->propertyForKey("Name");
          RtVariable.Name.setEmpty();
          if ( Prop2 != NULL ) {
            if ( !Prop2->isString() ) {
              MsgLog("ATTENTION : property not string in Block/Name\n");
            }else{
              if( Prop2->getString()->stringValue().notEmpty() ) {
                RtVariable.Name = Prop2->getString()->stringValue();
              }
            }
          }
          BlockRtVariableArray.AddReference(RtVariablePtr, true);
        }
      }
    }

    if (settingsData.RtROM.isEmpty()) {
      EFI_GUID uuid;
      StrToGuidLE(settingsData.SmUUID, &uuid);
      settingsData.RtROM.ncpy(&uuid.Data4[2], 6);
    }

    if (settingsData.RtMLB.isEmpty()) {
      settingsData.RtMLB       = settingsData.BoardSerialNumber;
    }

    // if CustomUUID and InjectSystemID are not specified
    // then use InjectSystemID=TRUE and SMBIOS UUID
    // to get Chameleon's default behaviour (to make user's life easier)
//    CopyMem((void*)&gUuid, (void*)&settingsData.SmUUID, sizeof(EFI_GUID));

    // SystemParameters again - values that can depend on previous params
    const TagDict* SystemParametersDict = CfgDict->dictPropertyForKey("SystemParameters");
    if (SystemParametersDict != NULL) {
      //BacklightLevel
      const TagStruct* Prop = SystemParametersDict->propertyForKey("BacklightLevel");
      if (Prop != NULL) {
        settingsData.BacklightLevel       = (UINT16)GetPropertyAsInteger(Prop, settingsData.BacklightLevel);
        settingsData.BacklightLevelConfig = TRUE;
      }

      Prop = SystemParametersDict->propertyForKey("CustomUUID");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in SystemParameters/CustomUUID\n");
        }else{
          if (IsValidGuidString(Prop->getString()->stringValue())) {
          settingsData.CustomUuid = Prop->getString()->stringValue();
            // if CustomUUID specified, then default for InjectSystemID=FALSE
            // to stay compatibile with previous Clover behaviour
            DBG("The UUID is valid\n");
          }else{
            DBG("Error: invalid CustomUUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->getString()->stringValue().c_str());
            settingsData.CustomUuid = {0};
          }
        }
      }
      //else gUuid value from SMBIOS
      //     DBG("Finally use %s\n", strguid(&gUuid));

      settingsData.InjectSystemID_ = 2;
      Prop                     = SystemParametersDict->propertyForKey("InjectSystemID");
      if ( Prop ) {
        if ( Prop->isBool() ) settingsData.InjectSystemID_ = Prop->getBool()->boolValue();
        else if (  Prop->isString() ) {
          // TODO a function that takes a string and return if it's true or false
          if ( Prop->getString()->stringValue().equalIC("true") ) settingsData.InjectSystemID_ = 1;
          else if ( Prop->getString()->stringValue()[0] == 'y' ) settingsData.InjectSystemID_ = 1;
          else if ( Prop->getString()->stringValue()[0] == 'Y' ) settingsData.InjectSystemID_ = 1;
          else if ( Prop->getString()->stringValue().equalIC("false") ) settingsData.InjectSystemID_ = 0;
          else if ( Prop->getString()->stringValue().equalIC("n") ) settingsData.InjectSystemID_ = 0;
          else if ( Prop->getString()->stringValue().equalIC("N") ) settingsData.InjectSystemID_ = 0;
          else {
            DBG("MALFORMED PLIST : SMBIOS/InjectSystemID must be true, yes, false, no, or non existant");
          }
        }else{
          DBG("MALFORMED PLIST : SMBIOS/InjectSystemID must be <true/>, <false/> or non existant");
        }
      }

      Prop                     = SystemParametersDict->propertyForKey("NvidiaWeb");
      settingsData.NvidiaWeb      = IsPropertyNotNullAndTrue(Prop);

    }


    const TagDict* BootGraphicsDict = CfgDict->dictPropertyForKey("BootGraphics");
    if (BootGraphicsDict != NULL) {
      const TagStruct* Prop = BootGraphicsDict->propertyForKey("DefaultBackgroundColor");
      settingsData.DefaultBackgroundColor = (UINT32)GetPropertyAsInteger(Prop, 0x80000000); //the value 0x80000000 means not set

      Prop = BootGraphicsDict->propertyForKey("UIScale");
      settingsData.UIScale = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

      Prop = BootGraphicsDict->propertyForKey("EFILoginHiDPI");
      settingsData.EFILoginHiDPI = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

      Prop = BootGraphicsDict->propertyForKey("flagstate");
      *(UINT32*)&settingsData.flagstate[0] = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

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

     CopyMem((void*)&AppleGuid, (void*)&gUuid, sizeof(EFI_GUID));
     AppleGuid.Data1 = SwapBytes32 (AppleGuid.Data1);
     AppleGuid.Data2 = SwapBytes16 (AppleGuid.Data2);
     AppleGuid.Data3 = SwapBytes16 (AppleGuid.Data3);
     DBG("Platform Uuid: %s, InjectSystemID: %s\n", strguid(&AppleGuid), settingsData.InjectSystemID ? "Yes" : "No");
     }
     */

    if (GlobalConfig.gBootChanged) {
      const TagDict* KernelAndKextPatchesDict = CfgDict->dictPropertyForKey("KernelAndKextPatches");
      if (KernelAndKextPatchesDict != NULL) {
        DBG("refill kernel patches bcoz GlobalConfig.gBootChanged\n");
        FillinKextPatches(&settingsData.KernelAndKextPatches, KernelAndKextPatchesDict);
      }
    } else {
      //DBG("\n ConfigName: %ls n", settingsData.ConfigName);
    }
    if (GlobalConfig.gThemeChanged) {
      settingsData.GUI.Theme.setEmpty();
      const TagDict* GUIDict = CfgDict->dictPropertyForKey("GUI");
      if (GUIDict != NULL) {
        const TagStruct* Prop = GUIDict->propertyForKey("Theme");
        if ((Prop != NULL) && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          settingsData.GUI.Theme.takeValueFrom(Prop->getString()->stringValue());
          DBG("Theme from new config: %ls\n", settingsData.GUI.Theme.wc_str());
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

XString8 GetAuthRootDmg(const EFI_FILE& dir, const XStringW& path)
{
  XString8 returnValue;

  XStringW plist = SWPrintf("%ls\\com.apple.Boot.plist", path.wc_str());
  if ( !FileExists(dir, plist) ) return NullXString8;

  CHAR8*     PlistBuffer = NULL;
  UINTN      PlistLen;
  TagDict*     Dict        = NULL;
  const TagStruct*     Prop        = NULL;

  EFI_STATUS Status = egLoadFile(&dir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
  if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS)
  {
    Prop = Dict->propertyForKey("Kernel Flags");
    if ( Prop != NULL ) {
      if ( !Prop->isString() ) {
        MsgLog("ATTENTION : Kernel Flags not string in ProductVersion\n");
      }else{
        if( Prop->getString()->stringValue().notEmpty() ) {
          const XString8& kernelFlags = Prop->getString()->stringValue();
          size_t idx = kernelFlags.indexOf("auth-root-dmg");
          if ( idx == MAX_XSIZE ) return NullXString8;
          idx += strlen("auth-root-dmg");
          while ( idx < kernelFlags.length()  &&  IS_BLANK(kernelFlags[idx]) ) ++idx;
          if ( kernelFlags[idx] == '=' ) ++idx;
          else return NullXString8;
          while ( idx < kernelFlags.length()  &&  IS_BLANK(kernelFlags[idx]) ) ++idx;
          if ( kernelFlags.equalAtIC(idx, "file://"_XS8) ) idx += strlen("file://");
          size_t idxEnd = idx;
          while ( idxEnd < kernelFlags.length()  &&  !IS_BLANK(kernelFlags[idxEnd]) ) ++idxEnd;
          returnValue = kernelFlags.subString(idx, idxEnd - idx);
        }
      }
    }
  }
  if ( PlistBuffer ) FreePool(PlistBuffer);
  return returnValue;
}

MacOsVersion GetMacOSVersionFromFolder(const EFI_FILE& dir, const XStringW& path)
{
  MacOsVersion macOSVersion;

  XStringW plist = SWPrintf("%ls\\SystemVersion.plist", path.wc_str());
  if ( !FileExists(dir, plist) ) {
    plist = SWPrintf("%ls\\ServerVersion.plist", path.wc_str());
    if ( !FileExists(dir, plist) ) {
      plist.setEmpty();
    }
  }

  if ( plist.notEmpty() ) { // found macOS System
    CHAR8*     PlistBuffer = NULL;
    UINTN      PlistLen;
    TagDict*     Dict        = NULL;
    const TagStruct*     Prop        = NULL;

    EFI_STATUS Status = egLoadFile(&dir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
    if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
      Prop = Dict->propertyForKey("ProductVersion");
      if ( Prop != NULL ) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in ProductVersion\n");
        }else{
          if( Prop->getString()->stringValue().notEmpty() ) {
            macOSVersion = Prop->getString()->stringValue();
          }
        }
      }
    }
    if ( PlistBuffer ) FreePool(PlistBuffer);
  }
  return macOSVersion;
}

MacOsVersion GetOSVersion(int LoaderType, const XStringW& APFSTargetUUID, const REFIT_VOLUME* Volume, XString8* BuildVersionPtr)
{
  XString8   OSVersion;
  XString8   BuildVersion;
  EFI_STATUS Status      = EFI_NOT_FOUND;
  CHAR8*     PlistBuffer = NULL;
  UINTN      PlistLen;
  TagDict*     Dict        = NULL;
  const TagDict*     DictPointer = NULL;
  const TagStruct*     Prop        = NULL;

  if ( !Volume ) {
    return NullXString8;
  }

  if (OSTYPE_IS_OSX(LoaderType))
  {
    XString8 uuidPrefix;
    if ( APFSTargetUUID.notEmpty() ) uuidPrefix = S8Printf("\\%ls", APFSTargetUUID.wc_str());

    XStringW plist = SWPrintf("%s\\System\\Library\\CoreServices\\SystemVersion.plist", uuidPrefix.c_str());
    if ( !FileExists(Volume->RootDir, plist) ) {
      plist = SWPrintf("%s\\System\\Library\\CoreServices\\ServerVersion.plist", uuidPrefix.c_str());
      if ( !FileExists(Volume->RootDir, plist) ) {
        plist.setEmpty();
      }
    }

    if ( plist.notEmpty() ) { // found macOS System
      Status = egLoadFile(Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
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
              BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }
  }

  if (OSTYPE_IS_OSX_INSTALLER (LoaderType)) {
    // Detect exact version for 2nd stage Installer (thanks to dmazar for this idea)
    // This should work for most installer cases. Rest cases will be read from boot.efi before booting.
    // Reworked by Sherlocks. 2018.04.12

    // 1st stage - 1
    // Check for plist - createinstallmedia/BaseSystem/InstallDVD/InstallESD

    XStringW InstallerPlist;

    if ( APFSTargetUUID.notEmpty() ) {
      InstallerPlist = SWPrintf("%ls\\System\\Library\\CoreServices\\SystemVersion.plist", APFSTargetUUID.wc_str());
      if ( !FileExists(Volume->RootDir, InstallerPlist) ) InstallerPlist.setEmpty();
    }

    if ( InstallerPlist.isEmpty() ) {
      InstallerPlist = SWPrintf("\\.IABootFilesSystemVersion.plist"); // 10.9 - 10.13.3
      if (!FileExists(Volume->RootDir, InstallerPlist) && FileExists (Volume->RootDir, L"\\System\\Library\\CoreServices\\boot.efi") &&
          ((FileExists(Volume->RootDir, L"\\BaseSystem.dmg") && FileExists (Volume->RootDir, L"\\mach_kernel")) || // 10.7/10.8
           FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\Mac OS X Installer.app") || // 10.6/10.7
           FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\OS X Installer.app") || // 10.8 - 10.11
           FileExists(Volume->RootDir, L"\\System\\Installation\\CDIS\\macOS Installer.app") || // 10.12+
           FileExists(Volume->RootDir, L"\\.IAPhysicalMedia"))) { // 10.13.4+
        InstallerPlist = SWPrintf("\\System\\Library\\CoreServices\\SystemVersion.plist");
      }
    }
    if (FileExists (Volume->RootDir, InstallerPlist)) {
      Status = egLoadFile(Volume->RootDir, InstallerPlist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
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
              BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }

//    if ( OSVersion.isEmpty() )
//    {
//      if ( FileExists(Volume->RootDir, SWPrintf("\\%ls\\com.apple.installer\\BridgeVersion.plist", APFSTargetUUID.wc_str()).wc_str()) ) {
//        OSVersion = "11.0"_XS8;
//        // TODO so far, is there is a BridgeVersion.plist, it's version 11.0. Has to be improved with next releases.
//      }
//    }

    // 1st stage - 2
    // Check for plist - createinstallmedia/NetInstall
    if (OSVersion.isEmpty()) {
      InstallerPlist = SWPrintf("\\.IABootFiles\\com.apple.Boot.plist"); // 10.9 - ...
      if (FileExists (Volume->RootDir, InstallerPlist)) {
        Status = egLoadFile(Volume->RootDir, InstallerPlist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR(Status) && PlistBuffer != NULL && ParseXML(PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = Dict->propertyForKey("Kernel Flags");
          if ( Prop != NULL ) {
            if ( !Prop->isString() ) {
              MsgLog("ATTENTION : property not string in Kernel Flags\n");
            }else{
              if ( Prop->getString()->stringValue().contains("Install%20macOS%20BigSur") || Prop->getString()->stringValue().contains("Install%20macOS%2011.0")) {
                OSVersion = "11"_XS8;
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
      InstallerPlist = SWPrintf("\\macOS Install Data\\Locked Files\\Boot Files\\SystemVersion.plist"); // 10.12.4+
      if (!FileExists (Volume->RootDir, InstallerPlist)) {
        InstallerPlist = SWPrintf("\\macOS Install Data\\InstallInfo.plist"); // 10.12+
        if (!FileExists (Volume->RootDir, InstallerPlist)) {
          InstallerPlist = SWPrintf("\\com.apple.boot.R\\SystemVersion.plist)"); // 10.12+
          if (!FileExists (Volume->RootDir, InstallerPlist)) {
            InstallerPlist = SWPrintf("\\com.apple.boot.P\\SystemVersion.plist"); // 10.12+
            if (!FileExists (Volume->RootDir, InstallerPlist)) {
              InstallerPlist = SWPrintf("\\com.apple.boot.S\\SystemVersion.plist"); // 10.12+
              if (!FileExists (Volume->RootDir, InstallerPlist) &&
                  (FileExists (Volume->RootDir, L"\\com.apple.boot.R\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                   FileExists (Volume->RootDir, L"\\com.apple.boot.P\\System\\Library\\PrelinkedKernels\\prelinkedkernel") ||
                   FileExists (Volume->RootDir, L"\\com.apple.boot.S\\System\\Library\\PrelinkedKernels\\prelinkedkernel"))) {
                InstallerPlist = SWPrintf("\\System\\Library\\CoreServices\\SystemVersion.plist"); // 10.11
              }
            }
          }
        }
      }
      if (FileExists (Volume->RootDir, InstallerPlist)) {
        Status = egLoadFile(Volume->RootDir, InstallerPlist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
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
                BuildVersion = Prop->getString()->stringValue();
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
      if (!FileExists (Volume->RootDir, InstallerLog)) {
        InstallerLog = L"\\OS X Install Data\\ia.log"_XSW; // 10.8 - 10.11
        if (!FileExists (Volume->RootDir, InstallerLog)) {
          InstallerLog = L"\\macOS Install Data\\ia.log"_XSW; // 10.12+
        }
      }
      if (FileExists (Volume->RootDir, InstallerLog)) {
        Status = egLoadFile(Volume->RootDir, InstallerLog.wc_str(), (UINT8 **)&fileBuffer, &fileLen);
        if (!EFI_ERROR(Status)) {
          XString8 targetString;
          targetString.strncpy(fileBuffer, fileLen);
      //    s = SearchString(targetString, fileLen, "Running OS Build: Mac OS X ", 27);
          s = AsciiStrStr(targetString.c_str(), "Running OS Build: Mac OS X ");
          if (s[31] == ' ') {
            OSVersion.S8Printf("%c%c.%c\n", s[27], s[28], s[30]);
            if (s[38] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c\n", s[33], s[34], s[35], s[36], s[37]);
            } else if (s[39] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[33], s[34], s[35], s[36], s[37], s[38]);
            }
          } else if (s[31] == '.') {
            OSVersion.S8Printf("%c%c.%c.%c\n", s[27], s[28], s[30], s[32]);
            if (s[40] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c\n", s[35], s[36], s[37], s[38], s[39]);
            } else if (s[41] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[35], s[36], s[37], s[38], s[39], s[40]);
            }
          } else if (s[32] == ' ') {
            OSVersion.S8Printf("%c%c.%c%c\n", s[27], s[28], s[30], s[31]);
            if (s[39] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38]);
            } else if (s[40] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38], s[39]);
            } else if (s[41] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38], s[39], s[40]);
            }
          } else if (s[32] == '.') {
            OSVersion.S8Printf("%c%c.%c%c.%c\n", s[27], s[28], s[30], s[31], s[33]);
            if (s[41] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40]);
            } else if (s[42] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40], s[41]);
            } else if (s[43] == ')') {
              BuildVersion.S8Printf("%c%c%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40], s[41], s[42]);
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
      if ( !FileExists(Volume->RootDir, plist) ) {
        plist.setEmpty();
      }

      if ( plist.notEmpty() ) { // found macOS System

        Status = egLoadFile(Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
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
              BuildVersion = Prop->getString()->stringValue();
            }
          }
        }
        Dict->FreeTag();
      }
    }
  }

  if (OSTYPE_IS_OSX_RECOVERY (LoaderType)) {

    XString8 uuidPrefix;
    if ( APFSTargetUUID.notEmpty() ) uuidPrefix = S8Printf("\\%ls", APFSTargetUUID.wc_str());

    XStringW plist = SWPrintf("%s\\SystemVersion.plist", uuidPrefix.c_str());
    if ( !FileExists(Volume->RootDir, plist) ) {
      plist = SWPrintf("%s\\ServerVersion.plist", uuidPrefix.c_str());
      if ( !FileExists(Volume->RootDir, plist) ) {
        plist = L"\\com.apple.recovery.boot\\SystemVersion.plist"_XSW;
        if ( !FileExists(Volume->RootDir, plist) ) {
          plist = L"\\com.apple.recovery.boot\\ServerVersion.plist"_XSW;
          if ( !FileExists(Volume->RootDir, plist) ) {
            plist.setEmpty();
          }
        }
      }
    }

    // Detect exact version for OS X Recovery
    if ( plist.notEmpty() ) { // found macOS System
      Status = egLoadFile(Volume->RootDir, plist.wc_str(), (UINT8 **)&PlistBuffer, &PlistLen);
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
            BuildVersion = Prop->getString()->stringValue();
          }
        }
      }
      Dict->FreeTag();
    } else if (FileExists (Volume->RootDir, L"\\com.apple.recovery.boot\\boot.efi")) {
      // Special case - com.apple.recovery.boot/boot.efi exists but SystemVersion.plist doesn't --> 10.9 recovery
      OSVersion = "10.9"_XS8;
    }
  }

  if (PlistBuffer != NULL) {
    FreePool(PlistBuffer);
  }
  (*BuildVersionPtr).stealValueFrom(&BuildVersion);
  return OSVersion;
}

//constexpr XStringW iconMac = L"mac"_XSW;
CONST XStringW
GetOSIconName (const MacOsVersion& OSVersion)
{
  XStringW OSIconName;
  if (OSVersion.isEmpty()) {
    OSIconName = L"mac"_XSW;
  } else if ( (OSVersion.elementAt(0) == 10 && OSVersion.elementAt(1) == 16 ) ||
              (OSVersion.elementAt(0) == 11 /*&& OSVersion.elementAt(1) == 0*/ )
            ) {
    // Big Sur
    OSIconName = L"bigsur,mac"_XSW;
  }else if ( OSVersion.elementAt(0) == 10 ) {
    if ( OSVersion.elementAt(1) == 15 ) {
      // Catalina
      OSIconName = L"cata,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 14 ) {
      // Mojave
      OSIconName = L"moja,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 13 ) {
      // High Sierra
      OSIconName = L"hsierra,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 12 ) {
      // Sierra
      OSIconName = L"sierra,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 11 ) {
      // El Capitan
      OSIconName = L"cap,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 10 ) {
      // Yosemite
      OSIconName = L"yos,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 9 ) {
      // Mavericks
      OSIconName = L"mav,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 8 ) {
      // Mountain Lion
      OSIconName = L"cougar,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 7 ) {
      // Lion
      OSIconName = L"lion,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 6 ) {
      // Snow Leopard
      OSIconName = L"snow,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 5 ) {
      // Leopard
      OSIconName = L"leo,mac"_XSW;
    } else if ( OSVersion.elementAt(1) == 4 ) {
      // Tiger
      OSIconName = L"tiger,mac"_XSW;
    } else {
      OSIconName = L"mac"_XSW;
    }
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

void
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
  AudioList.setEmpty();
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
      Status = gBS->HandleProtocol(HandleArray[Index], &gEfiPciIoProtocolGuid, (void **)&PciIo);
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

void
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
      Status = gBS->HandleProtocol (HandleBuffer[i], &gEfiPciIoProtocolGuid, (void **)&PciIo);
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
            TmpDirty    = setup_hda_devprop (PciIo, &PCIdevice, Entry->macOSVersion);
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
      //      Status = egSaveFile(&self.getSelfRootDir(),  SWPrintf("%ls\\misc\\devprop.bin", self.getCloverDirFullPath().wc_str()).wc_str()    , (UINT8*)mProperties, mPropSize);
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

  if ((gSettings.BusSpeed != 0) && (gSettings.BusSpeed > 10 * Kilo) && (gSettings.BusSpeed < 500 * Kilo)) {
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
        //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
        break;
      default:
        //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gSettings.BusSpeed, Kilo)));

        // for sandy bridge or newer
        // to match ExternalClock 25 MHz like real mac, divide BusSpeed by 4
        gCPUStructure.ExternalClock = (gSettings.BusSpeed + 3) / 4;
        //DBG("Corrected ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
        break;
    }

    gCPUStructure.FSBFrequency  = MultU64x64 (gSettings.BusSpeed, Kilo); //kHz -> Hz
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
  if ( !selfOem.isKextsDirFound() ) return NullXStringW;
  if ( FileExists(&selfOem.getKextsDir(), On ? L"Other" : L"Off") ) {
    return On ? L"Other"_XSW : L"Off"_XSW;
  }
  return NullXStringW;
}


//dmazar
// Jief 2020-10: this is only called by SetFSInjection(). SetFSInjection() doesn't check for return value emptiness.
XStringW GetOSVersionKextsDir(const MacOsVersion& OSVersion)
{
//  XString8 FixedVersion;
//  CHAR8  *DotPtr;

  if ( !selfOem.isKextsDirFound() ) return NullXStringW;

//  if (OSVersion.notEmpty()) {
//    FixedVersion.strncpy(OSVersion.c_str(), 5);
//    //    DBG("%s\n", FixedVersion);
//    // OSVersion may contain minor version too (can be 10.x or 10.x.y)
//    if ((DotPtr = AsciiStrStr (FixedVersion.c_str(), ".")) != NULL) {
//      DotPtr = AsciiStrStr (DotPtr+1, "."); // second dot
//    }
//
//    if (DotPtr != NULL) {
//      *DotPtr = 0;
//    }
//  }

  //MsgLog ("OS=%ls\n", OSTypeStr);

  // find source injection folder with kexts
  // note: we are just checking for existance of particular folder, not checking if it is empty or not
  // check OEM subfolders: version specific or default to Other
  // Jief : NOTE selfOem.getKextsFullPath() return a path under OEM if exists, or in Clover if not.
  XStringW SrcDir = SWPrintf("%ls\\%s", selfOem.getKextsFullPath().wc_str(), OSVersion.asString(2).c_str());
  if (FileExists (&self.getSelfVolumeRootDir(), SrcDir)) return SrcDir;
  return NullXStringW;
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

EFI_STATUS LOADER_ENTRY::SetFSInjection()
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

    SrcDir = GetOSVersionKextsDir(macOSVersion);
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
#include "../../CloverApp/Clover/CloverOldHeaders.h"
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
  xb.cat((BOOLEAN)SmUUID.notEmpty());
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
  xb.cat(Boot.XMPDetection);
  xb.cat(UseARTFreq);
  // SMBIOS TYPE133
  xb.ncat(&pad18, sizeof(pad18));
  xb.cat(PlatformFeature);

  // PatchTableType11
  xb.cat(NoRomInfo);

  // OS parameters
  WriteOldFixLengthString(Language, 16);
checkOffset(BootArgs);
  WriteOldFixLengthString(Boot.BootArgs, 256);
  xb.memsetAtPos(xb.size(), 0, 1);
checkOffset(CustomUuid);
  WriteOldFixLengthString(XStringW(CustomUuid), 40);
  xb.ncat(&pad20, sizeof(pad20));
checkOffset(DefaultVolume);
  xb.cat(uintptr_t(0)); //DefaultVolume was CHAR16*
  xb.cat(uintptr_t(0)); //DefaultLoader was CHAR16*
//Boot
checkOffset(LastBootedVolume);
  xb.cat(Boot.LastBootedVolume);
  xb.cat(Boot.SkipHibernateTimeout);
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
  xb.cat(InjectSystemID_);
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
  xb.cat(Boot.SecureBootWhiteListCount);
  xb.cat(Boot.SecureBootBlackListCount);
  xb.cat(Boot.SecureBootWhiteList);
  xb.cat(Boot.SecureBootBlackList);

  // Secure boot
  xb.cat(Boot.SecureBoot);
  xb.cat(Boot.SecureBootSetupMode);
  xb.cat(Boot.SecureBootPolicy);

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
  WriteOldFixLengthString(Boot.LegacyBoot, 32);
  xb.cat(Boot.LegacyBiosDefaultEntry);

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
  xb.cat(Boot.CustomBoot);
  xb.ncat(&pad29, sizeof(pad29));
  xb.cat(Boot.CustomLogo);
  xb.cat(RefCLK);

  // SysVariables
  xb.ncat(&pad30, sizeof(pad30));
checkOffset(RtMLB);
  xb.cat(uintptr_t(0)); // RtMLB was CHAR8*
  xb.cat(uintptr_t(0)); // RtROM was UINT8*
checkOffset(RtROMLen);
  xb.cat(RtROM.size());
checkOffset(CsrActiveConfig);
  xb.cat(CsrActiveConfig);
  xb.cat(BooterConfig);
  WriteOldFixLengthString(BooterCfgStr, 64);
  xb.cat(Boot.DisableCloverHotkeys);
  xb.cat(NeverDoRecovery);

  // Multi-config
  xb.ncat(&ConfigName, sizeof(ConfigName));
  xb.ncat(&pad31, sizeof(pad31));
  xb.cat(uintptr_t(0)); // MainConfigName was a CHAR16*

  //Drivers
  xb.cat(DisabledDriverArray.size()); // BlackListCount
  xb.cat(uintptr_t(0));  // BlackList was a pointer

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
  xb.cat((UINT32)DSDTPatchArray.size()); // PatchDsdtNum
  xb.cat(uintptr_t(0)); // PatchDsdtFind
  xb.cat(uintptr_t(0)); // LenToFind
  xb.cat(uintptr_t(0)); // PatchDsdtReplace
  xb.cat(uintptr_t(0)); // LenToReplace
checkOffset(DebugDSDT);
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
  xb.cat(uintptr_t(0)); // PatchDsdtLabel
  xb.cat(uintptr_t(0)); // PatchDsdtTgt
  xb.cat(uintptr_t(0)); // PatchDsdtMenuItem

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

EFI_GUID nullUUID = {0,0,0,{0}};

const XString8& SETTINGS_DATA::getUUID()
{
  if ( CustomUuid.notEmpty() ) return CustomUuid;
  return SmUUID;
}

const XString8& SETTINGS_DATA::getUUID(EFI_GUID *uuid)
{
  if ( CustomUuid.notEmpty() ) {
    EFI_STATUS Status = StrToGuidLE(CustomUuid, uuid);
    if ( EFI_ERROR(Status) ) panic("CustomUuid(%s) is not valid", CustomUuid.c_str()); // we panic, because it's a bug. Validity is checked when imported from settings
    return CustomUuid;
  }
  EFI_STATUS Status = StrToGuidLE(SmUUID, uuid);
  if ( EFI_ERROR(Status) ) panic("CustomUuid(%s) is not valid", CustomUuid.c_str()); // same as before
  return SmUUID;
}



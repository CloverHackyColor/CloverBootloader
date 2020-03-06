/*
 Slice 2012
 */

#include "entry_scan.h"
#include "kernel_patcher.h"
#include "ati.h"
#include "nanosvg.h"
#include "nvidia.h"
#include "../refit/screen.h"
#include "../refit/menu.h"
#include "gma.h"
//#include "XPointer.h"

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


ACPI_PATCHED_AML                *ACPIPatchedAML;
SIDELOAD_KEXT                   *InjectKextList = NULL;
//SYSVARIABLES                    *SysVariables;
CHAR16                          *IconFormat = NULL;

TagPtr                          gConfigDict[NUM_OF_CONFIGS] = {NULL, NULL, NULL};

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
CHAR16                          *ThemesList[50]; //no more then 50 themes?
UINTN                           ConfigsNum;
CHAR16                          *ConfigsList[20];
UINTN                           DsdtsNum;
CHAR16                          *DsdtsList[20];
UINTN                           AudioNum;
HDA_OUTPUTS                     AudioList[20];
UINTN                           RtVariablesNum;
RT_VARIABLES                    *RtVariables;

// firmware
BOOLEAN                         gFirmwareClover             = FALSE;
UINTN                           gEvent;
UINT16                          gBacklightLevel;
BOOLEAN                         defDSM;
UINT16                          dropDSM;

BOOLEAN                         GetLegacyLanAddress;
BOOLEAN                         ResumeFromCoreStorage;
BOOLEAN                         gRemapSmBiosIsRequire;
CONST CHAR16                        **SystemPlists                = NULL;
CONST CHAR16                        **InstallPlists               = NULL;
CONST CHAR16                        **RecoveryPlists              = NULL;

// QPI
BOOLEAN                         SetTable132                 = FALSE;

GUI_ANIME                       *GuiAnime                   = NULL;
EG_IMAGE *SelectionImages[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
EG_IMAGE *Buttons[4] = {NULL, NULL, NULL, NULL};
EG_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff }; //non-trasparent

INTN row0TileSize = 144;
INTN row1TileSize = 64;
INTN BCSMargin = 11;
BOOLEAN DayLight;



extern MEM_STRUCTURE            gRAM;
extern BOOLEAN                  NeedPMfix;

extern INTN                     ScrollWidth;
extern INTN                     ScrollButtonsHeight;
extern INTN                     ScrollBarDecorationsHeight;
extern INTN                     ScrollScrollDecorationsHeight;
extern EFI_AUDIO_IO_PROTOCOL    *AudioIo;
//extern INTN                     OldChosenAudio;


// global configuration with default values
REFIT_CONFIG   GlobalConfig = {
  -1,             // INTN        Timeout;
  0,              // UINTN       DisableFlags;
  0,              // UINTN       HideBadges;
  0,              // UINTN       HideUIFlags;
  FALSE,          // BOOLEAN     TextOnly;
  TRUE,           // BOOLEAN     Quiet;
  FALSE,          // BOOLEAN     LegacyFirst;
  FALSE,          // BOOLEAN     NoLegacy;
  FALSE,          // BOOLEAN     DebugLog;
  FALSE,          // BOOLEAN     FastBoot;
  FALSE,          // BOOLEAN     NeverHibernate;
  FALSE,          // BOOLEAN     StrictHibernate;
  FALSE,          // BOOLEAN     RtcHibernateAware;
  FONT_GRAY,      // FONT_TYPE   Font; //Welcome should be white
  9,              // INTN        CharWidth;
  0xFFFFFF80,     // UINTN       SelectionColor;
  NULL,           // CHAR16      *FontFileName;
  NULL,           // CHAR16      *Theme;
  NULL,           // CHAR16      *BannerFileName;
  NULL,           // CHAR16      *SelectionSmallFileName;
  NULL,           // CHAR16      *SelectionBigFileName;
  NULL,           // CHAR16      *SelectionIndicatorName;
  NULL,           // CHAR16      *DefaultSelection;
  NULL,           // CHAR16      *ScreenResolution;
  0,              // INTN        ConsoleMode;
  NULL,           // CHAR16      *BackgroundName;
  imNone,         // SCALING     BackgroundScale;
  0,              // UINTN       BackgroundSharp;
  FALSE,          // BOOLEAN     BackgroundDark;
  FALSE,          // BOOLEAN     CustomIcons;
  FALSE,          // BOOLEAN     SelectionOnTop;
  FALSE,          // BOOLEAN     BootCampStyle;
  0,              // INTN        BadgeOffsetX;
  0,              // INTN        BadgeOffsetY;
  4,              // INTN        BadgeScale;
  0xFFFF,         // INTN        ThemeDesignWidth;
  0xFFFF,         // INTN        ThemeDesignHeight;
  0xFFFF,         // INTN        BannerPosX;
  0xFFFF,         // INTN        BannerPosY;
  0,              // INTN        BannerEdgeHorizontal;
  0,              // INTN        BannerEdgeVertical;
  0,              // INTN        BannerNudgeX;
  0,              // INTN        BannerNudgeY;
  FALSE,          // BOOLEAN     VerticalLayout;
  FALSE,          // BOOLEAN     NonSelectedGrey;
  128,            // INTN        MainEntriesSize;
  8,              // INTN        TileXSpace;
  24,             // INTN        TileYSpace;
  ICON_FORMAT_DEF, // INTN       IconFormat;
  FALSE,          // BOOLEAN     Proportional;
  FALSE,          // BOOLEAN     NoEarlyProgress;
  FALSE,          // BOOLEAN     ShowOptimus;
  FALSE,          // BOOLEAN     HibernationFixup;
  FALSE,          // BOOLEAN     SignatureFixup;
  FALSE,          // BOOLEAN     DarkEmbedded;
  FALSE,          // BOOLEAN     TypeSVG;
  0,              // INT32       Timezone;
  0xC0,           // INTN        Codepage;
  0xC0,           // INTN        CodepageSize; //extended latin
  1.0f,           // float       Scale;
  0.0f,           // float       CentreShift;
};

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


/*
 VOID __inline WaitForSts(VOID) {
 UINT32 inline_timeout = 100000;
 while (AsmReadMsr64(MSR_IA32_PERF_STATUS) & (1 << 21)) { if (!inline_timeout--) break; }
 }
 */
#if 0
UINT32
GetCrc32 (
          UINT8 *Buffer,
          UINTN Size
          )
{
  UINTN  i;
  UINTN  Len;
  UINT32 x;
  UINT32 *Fake;

  Fake = (UINT32*)Buffer;
  if (Fake == NULL) {
    DBG ("Buffer=NULL\n");
    return 0;
  }

  x = 0;
  Len = Size >> 2;
  for (i = 0; i < Len; i++) {
    x += Fake[i];
  }

  return x;
}
#else //nice programming
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
#endif

/*
 return TRUE if the property present && value = TRUE
 else return FALSE
 */
BOOLEAN
IsPropertyTrue (
                TagPtr Prop
                )
{
  return Prop != NULL &&
  ((Prop->type == kTagTypeTrue) ||
   ((Prop->type == kTagTypeString) && Prop->string &&
    ((Prop->string[0] == 'y') || (Prop->string[0] == 'Y'))));
}

/*
 return TRUE if the property present && value = FALSE
 else return FALSE
 */
BOOLEAN
IsPropertyFalse (
                 TagPtr Prop
                 )
{
  return Prop != NULL &&
  ((Prop->type == kTagTypeFalse) ||
   ((Prop->type == kTagTypeString) && Prop->string &&
    ((Prop->string[0] == 'N') || (Prop->string[0] == 'n'))));
}

/*
 Possible values
 <integer>1234</integer>
 <integer>+1234</integer>
 <integer>-1234</integer>
 <string>0x12abd</string>
 */
INTN
GetPropertyInteger (
                    TagPtr Prop,
                    INTN Default
                    )
{
  if (Prop == NULL) {
    return Default;
  }

  if (Prop->type == kTagTypeInteger) {
    return (INTN)Prop->string; //this is union char* or size_t
  } else if ((Prop->type == kTagTypeString) && Prop->string) {
    if ((Prop->string[1] == 'x') || (Prop->string[1] == 'X')) {
      return (INTN)AsciiStrHexToUintn (Prop->string);
    }

    if (Prop->string[0] == '-') {
      return -(INTN)AsciiStrDecimalToUintn (Prop->string + 1);
    }

//    return (INTN)AsciiStrDecimalToUintn (Prop->string);
    return (INTN)AsciiStrDecimalToUintn((Prop->string[0] == '+') ? (Prop->string + 1) : Prop->string);
  }
  return Default;
}

ACPI_NAME_LIST *
ParseACPIName(CHAR8 *String)
{
  ACPI_NAME_LIST* List = NULL;
  ACPI_NAME_LIST* Next = NULL;
  INTN i, j, Len, pos0, pos1;
  Len = AsciiStrLen(String);
  //  DBG("parse ACPI name: %a\n", String);
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
      //      DBG("string between [%d,%d]: %a\n", pos0, pos1, List->Name);
      pos0 = pos1; //comma or zero@end
      Next = List;
    }
  }
  return List;
}

VOID
ParseLoadOptions (
                  OUT  CHAR16** Conf,
                  OUT  TagPtr* Dict
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
  *Conf                  = NULL;
  *Dict                  = NULL;

  Start = (CHAR8*)SelfLoadedImage->LoadOptions;
  End   = (CHAR8*)((CHAR8*)SelfLoadedImage->LoadOptions + SelfLoadedImage->LoadOptionsSize);
  while ((Start < End) && ((*Start == ' ') || (*Start == '\\') || (*Start == '/')))
  {
    ++Start;
  }

  TailSize = End - Start;
  //DBG ("TailSize = %d\n", TailSize);

  if ((TailSize) <= 0) {
    return;
  }

  for (i = 0; PlistStrings[i][0] != '\0'; i++) {
    PlistStringsLen = AsciiStrLen (PlistStrings[i]);
    //DBG ("PlistStrings[%d] = %a\n", i, PlistStrings[i]);
    if (PlistStringsLen < TailSize) {
      if (AsciiStriNCmp (PlistStrings[i], Start, PlistStringsLen)) {
        DBG (" - found plist string = %a, parse XML in LoadOptions\n", PlistStrings[i]);
        if (ParseXML (Start, Dict, (UINT32)TailSize) != EFI_SUCCESS) {
          *Dict = NULL;
          DBG ("  - [!] xml in load options is bad\n");
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
  //  DBG ("TailSize2 = %d\n", TailSize);

  if (TailSize > 6) {
    if (AsciiStriNCmp (".plist", End - 6, 6)) {
      End      -= 6;
      TailSize -= 6;
      //      DBG ("TailSize3 = %d\n", TailSize);
    }
  } else if (TailSize <= 0) {
    return;
  }

  AsciiConf = (__typeof__(AsciiConf))AllocateCopyPool (TailSize + 1, Start);
  if (AsciiConf != NULL) {
    *(AsciiConf + TailSize) = '\0';
    *Conf = (__typeof_am__(*Conf))AllocateZeroPool ((TailSize + 1) * sizeof(**Conf));
    AsciiStrToUnicodeStrS (AsciiConf, *Conf, TailSize);
    FreePool (AsciiConf);
  }
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
  gSettings.DefaultVolume = (__typeof__(gSettings.DefaultVolume))AllocateCopyPool(NameSize, Data);

  Data += NameSize;
  Name2Size = Len - NameSize;
  if (Name2Size != 0) {
    gSettings.DefaultLoader = (__typeof__(gSettings.DefaultLoader))AllocateCopyPool(Name2Size, Data);
  }

  DBG("Clover started with option to boot %s from %s\n",
      (gSettings.DefaultLoader != NULL)?gSettings.DefaultLoader:L"legacy",
      gSettings.DefaultVolume);
}

//
// check if this entry corresponds to Boot# variable and then set BootCurrent
//
VOID
SetBootCurrent(REFIT_MENU_ITEM_ABSTRACT_ENTRY_LOADER *Entry)
{
  EFI_STATUS      Status;
  BO_BOOT_OPTION  BootOption;
  CHAR16          *VarName;
  UINTN           VarSize = 0;
  UINT8           *BootVariable;
  UINTN           NameSize;
  UINT8           *Data;
  UINT16          *BootOrder;
  UINT16          *BootOrderNew;
  UINT16          *Ptr;
  UINTN           BootOrderSize;
  INTN            BootIndex = 0, Index;


  VarName = PoolPrint(L"Boot%04x", Entry->BootNum);
  BootVariable = (UINT8*)GetNvramVariable (VarName, &gEfiGlobalVariableGuid, NULL, &VarSize);
  if ((BootVariable == NULL) || (VarSize == 0)) {
    DBG("Boot option %s not found\n", VarName);
    FreePool(VarName);
    return;
  }
  FreePool(VarName);

  //decode the variable
  BootOption.Variable = BootVariable;
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
  if (StriCmp((CHAR16*)Data, Entry->Volume->VolName) != 0) {
    DBG("Boot option %d has other volume name %s\n", Entry->BootNum, (CHAR16*)Data);
    FreePool(BootVariable);
    return;
  }

  if (VarSize > NameSize + 6) {
    Data += NameSize;
    if (StriCmp((CHAR16*)Data, Basename(Entry->LoaderPath)) != 0) {
      DBG("Boot option %d has other loader name %s\n", Entry->BootNum, (CHAR16*)Data);
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
    DBG("Can't save BootCurrent, status=%r\n", Status);
  }
  //Next step is rotate BootOrder to set BootNum to first place
  BootOrder = (__typeof__(BootOrder))GetNvramVariable (L"BootOrder", &gEfiGlobalVariableGuid, NULL, &BootOrderSize);
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
      DBG("Can't save BootOrder, status=%r\n", Status);
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
                 IN      TagPtr Dict,
                 IN      CONST CHAR8  *PropName,
                 OUT  UINTN  *DataLen
                 )
{
  TagPtr Prop;
  UINT8  *Data = NULL;
  UINT32 Len;
  //UINTN   i;

  Prop = GetProperty (Dict, PropName);
  if (Prop != NULL) {
    if (Prop->data != NULL /*&& Prop->dataLen > 0*/) { //rehabman: allow zero length data
      // data property
      Data = (__typeof__(Data))AllocateZeroPool (Prop->dataLen);
      CopyMem (Data, Prop->data, Prop->dataLen);

      if (DataLen != NULL) {
        *DataLen = Prop->dataLen;
      }
      /*
       DBG ("Data: %p, Len: %d = ", Data, Prop->dataLen);
       for (i = 0; i < Prop->dataLen; i++) {
       DBG ("%02x ", Data[i]);
       }
       DBG ("\n");
       */
    } else {
      // assume data in hex encoded string property
      Len = (UINT32)AsciiStrLen (Prop->string) >> 1; // number of hex digits
      Data = (__typeof__(Data))AllocateZeroPool(Len); // 2 chars per byte, one more byte for odd number
      Len  = hex2bin (Prop->string, Data, Len);

      if (DataLen != NULL) {
        *DataLen = Len;
      }
      /*
       DBG ("Data(str): %p, Len: %d = ", data, len);
       for (i = 0; i < Len; i++) {
       DBG ("%02x ", data[i]);
       }
       DBG ("\n");
       */
    }
  }

  return Data;
}

EFI_STATUS
LoadUserSettings (
                  IN EFI_FILE *RootDir,
                  IN CONST CHAR16   *ConfName,
                  TagPtr   *Dict)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  UINTN      Size = 0;
  CHAR8*     gConfigPtr = NULL;
  CHAR16*    ConfigPlistPath;
  CHAR16*    ConfigOemPath;

  //  DbgHeader("LoadUserSettings");

  // load config
  if ((ConfName == NULL) || (Dict == NULL)) {
    return EFI_NOT_FOUND;
  }

  ConfigPlistPath = PoolPrint (L"EFI\\CLOVER\\%s.plist", ConfName);
  ConfigOemPath   = PoolPrint (L"%s\\%s.plist", OEMPath, ConfName);
  if (FileExists (SelfRootDir, ConfigOemPath)) {
    Status = egLoadFile (SelfRootDir, ConfigOemPath, (UINT8**)&gConfigPtr, &Size);
  }
  if (EFI_ERROR (Status)) {
    if ((RootDir != NULL) && FileExists (RootDir, ConfigPlistPath)) {
      Status = egLoadFile (RootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &Size);
    }
    if (!EFI_ERROR (Status)) {
      DBG ("Using %s.plist at RootDir at path: %s\n", ConfName, ConfigPlistPath);
    } else {
      Status = egLoadFile (SelfRootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &Size);
      if (!EFI_ERROR (Status)) {
        DBG ("Using %s.plist at SelfRootDir at path: %s\n", ConfName, ConfigPlistPath);
      }
    }
  }

  if (!EFI_ERROR (Status) && gConfigPtr != NULL) {
    Status = ParseXML ((const CHAR8*)gConfigPtr, Dict, (UINT32)Size);
    if (EFI_ERROR (Status)) {
      //  Dict = NULL;
      DBG ("config.plist parse error Status=%r\n", Status);
      return Status;
    }
  }
  return Status;
}

STATIC BOOLEAN AddCustomEntry (IN CUSTOM_LOADER_ENTRY *Entry)
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

BOOLEAN
CopyKernelAndKextPatches (IN OUT  KERNEL_AND_KEXT_PATCHES *Dst,
                          IN      KERNEL_AND_KEXT_PATCHES *Src)
{
  if (Dst == NULL || Src == NULL) return FALSE;

  Dst->KPDebug           = Src->KPDebug;
  Dst->KPKernelCpu       = Src->KPKernelCpu;
  Dst->KPKernelLapic     = Src->KPKernelLapic;
  Dst->KPKernelXCPM      = Src->KPKernelXCPM;
  Dst->KPKernelPm        = Src->KPKernelPm;
  Dst->KPAppleIntelCPUPM = Src->KPAppleIntelCPUPM;
  Dst->KPAppleRTC        = Src->KPAppleRTC;
  Dst->KPDELLSMBIOS      = Src->KPDELLSMBIOS;
  Dst->FakeCPUID         = Src->FakeCPUID;
  Dst->KPPanicNoKextDump = Src->KPPanicNoKextDump;

  if (Src->KPATIConnectorsController != NULL) {
    Dst->KPATIConnectorsController = EfiStrDuplicate (Src->KPATIConnectorsController);
  }

  if ((Src->KPATIConnectorsDataLen > 0) &&
      (Src->KPATIConnectorsData != NULL) &&
      (Src->KPATIConnectorsPatch != NULL)) {
    Dst->KPATIConnectorsDataLen = Src->KPATIConnectorsDataLen;
    Dst->KPATIConnectorsData = (__typeof__(Dst->KPATIConnectorsData))AllocateCopyPool (Src->KPATIConnectorsDataLen, Src->KPATIConnectorsData);
    Dst->KPATIConnectorsPatch = (__typeof__(Dst->KPATIConnectorsPatch))AllocateCopyPool (Src->KPATIConnectorsDataLen, Src->KPATIConnectorsPatch);
  }

  if ((Src->NrForceKexts > 0) && (Src->ForceKexts != NULL)) {
    INTN i;
    Dst->ForceKexts = (__typeof__(Dst->ForceKexts))AllocatePool (Src->NrForceKexts * sizeof(CHAR16 *));

    for (i = 0; i < Src->NrForceKexts; i++) {
      Dst->ForceKexts[Dst->NrForceKexts++] = EfiStrDuplicate (Src->ForceKexts[i]);
    }
  }

  if ((Src->NrKexts > 0) && (Src->KextPatches != NULL)) {
    INTN i;
    Dst->KextPatches = (__typeof__(Dst->KextPatches))AllocatePool (Src->NrKexts * sizeof(KEXT_PATCH));

    for (i = 0; i < Src->NrKexts; i++)
    {
      if ((Src->KextPatches[i].DataLen <= 0) ||
          (Src->KextPatches[i].Data == NULL) ||
          (Src->KextPatches[i].Patch == NULL)) {
        continue;
      }

      if (Src->KextPatches[i].Name) {
        Dst->KextPatches[Dst->NrKexts].Name       = (CHAR8 *)AllocateCopyPool (AsciiStrSize (Src->KextPatches[i].Name), Src->KextPatches[i].Name);
      }

      if (Src->KextPatches[i].Label) {
        Dst->KextPatches[Dst->NrKexts].Label      = (CHAR8 *)AllocateCopyPool (AsciiStrSize (Src->KextPatches[i].Label), Src->KextPatches[i].Label);
      }

      Dst->KextPatches[Dst->NrKexts].MenuItem.BValue     = Src->KextPatches[i].MenuItem.BValue;
      Dst->KextPatches[Dst->NrKexts].IsPlistPatch = Src->KextPatches[i].IsPlistPatch;
      Dst->KextPatches[Dst->NrKexts].DataLen      = Src->KextPatches[i].DataLen;
      Dst->KextPatches[Dst->NrKexts].Data = (__typeof__(Dst->KextPatches[Dst->NrKexts].Data))AllocateCopyPool (Src->KextPatches[i].DataLen, Src->KextPatches[i].Data);
      Dst->KextPatches[Dst->NrKexts].Patch = (__typeof__(Dst->KextPatches[Dst->NrKexts].Patch))AllocateCopyPool (Src->KextPatches[i].DataLen, Src->KextPatches[i].Patch);
      Dst->KextPatches[Dst->NrKexts].MatchOS = (__typeof__(Dst->KextPatches[Dst->NrKexts].MatchOS))AllocateCopyPool (AsciiStrSize(Src->KextPatches[i].MatchOS), Src->KextPatches[i].MatchOS);
      Dst->KextPatches[Dst->NrKexts].MatchBuild = (__typeof__(Dst->KextPatches[Dst->NrKexts].MatchBuild))AllocateCopyPool (AsciiStrSize(Src->KextPatches[i].MatchBuild), Src->KextPatches[i].MatchBuild);
      ++(Dst->NrKexts);
    }
  }

  if ((Src->NrKernels > 0) && (Src->KernelPatches != NULL)) {
    INTN i;
    Dst->KernelPatches = (__typeof__(Dst->KernelPatches))AllocatePool (Src->NrKernels * sizeof(KERNEL_PATCH));

    for (i = 0; i < Src->NrKernels; i++)
    {
      if ((Src->KernelPatches[i].DataLen <= 0) ||
          (Src->KernelPatches[i].Data == NULL) ||
          (Src->KernelPatches[i].Patch == NULL)) {
        continue;
      }

      if (Src->KernelPatches[i].Label) {
        Dst->KernelPatches[Dst->NrKernels].Label      = (CHAR8 *)AllocateCopyPool (AsciiStrSize (Src->KernelPatches[i].Label), Src->KernelPatches[i].Label);
      }

      Dst->KernelPatches[Dst->NrKernels].MenuItem.BValue     = Src->KernelPatches[i].MenuItem.BValue;
      Dst->KernelPatches[Dst->NrKernels].DataLen      = Src->KernelPatches[i].DataLen;
      Dst->KernelPatches[Dst->NrKernels].Data = (__typeof__(Dst->KernelPatches[Dst->NrKernels].Data))AllocateCopyPool (Src->KernelPatches[i].DataLen, Src->KernelPatches[i].Data);
      Dst->KernelPatches[Dst->NrKernels].Patch = (__typeof__(Dst->KernelPatches[Dst->NrKernels].Patch))AllocateCopyPool (Src->KernelPatches[i].DataLen, Src->KernelPatches[i].Patch);
      Dst->KernelPatches[Dst->NrKernels].Count        = Src->KernelPatches[i].Count;
      Dst->KernelPatches[Dst->NrKernels].MatchOS = (__typeof__(Dst->KernelPatches[Dst->NrKernels].MatchOS))AllocateCopyPool (AsciiStrSize(Src->KernelPatches[i].MatchOS), Src->KernelPatches[i].MatchOS);
      Dst->KernelPatches[Dst->NrKernels].MatchBuild = (__typeof__(Dst->KernelPatches[Dst->NrKernels].MatchBuild))AllocateCopyPool (AsciiStrSize(Src->KernelPatches[i].MatchBuild), Src->KernelPatches[i].MatchBuild);
      if (Src->KernelPatches[i].MaskFind != NULL) {
        Dst->KernelPatches[Dst->NrKernels].MaskFind = (__typeof__(Dst->KernelPatches[Dst->NrKernels].MaskFind))AllocateCopyPool (Src->KernelPatches[i].DataLen, Src->KernelPatches[i].MaskFind);
      } else {
        Dst->KernelPatches[Dst->NrKernels].MaskFind        = NULL;
      }
      if (Src->KernelPatches[i].MaskReplace != NULL) {
        Dst->KernelPatches[Dst->NrKernels].MaskReplace = (__typeof__(Dst->KernelPatches[Dst->NrKernels].MaskReplace))AllocateCopyPool (Src->KernelPatches[i].DataLen, Src->KernelPatches[i].MaskReplace);
      } else {
        Dst->KernelPatches[Dst->NrKernels].MaskReplace        = NULL;
      }
      ++(Dst->NrKernels);
    }
  }

  return TRUE;
}

STATIC
CUSTOM_LOADER_ENTRY
*DuplicateCustomEntry (
                       IN CUSTOM_LOADER_ENTRY *Entry
                       )
{
  CUSTOM_LOADER_ENTRY *DuplicateEntry;

  if (Entry == NULL) {
    return NULL;
  }

  DuplicateEntry = (CUSTOM_LOADER_ENTRY *)AllocateZeroPool (sizeof(CUSTOM_LOADER_ENTRY));
  if (DuplicateEntry != NULL) {
    if (Entry->Volume != NULL) {
      DuplicateEntry->Volume         = EfiStrDuplicate (Entry->Volume);
    }

    if (Entry->Path != NULL) {
      DuplicateEntry->Path           = EfiStrDuplicate (Entry->Path);
    }

    if (Entry->Options != NULL) {
      DuplicateEntry->Options        = EfiStrDuplicate (Entry->Options);
    }

    if (Entry->FullTitle != NULL) {
      DuplicateEntry->FullTitle      = EfiStrDuplicate (Entry->FullTitle);
    }

    if (Entry->Title != NULL) {
      DuplicateEntry->Title          = EfiStrDuplicate (Entry->Title);
    }

    if (Entry->ImagePath != NULL) {
      DuplicateEntry->ImagePath      = EfiStrDuplicate (Entry->ImagePath);
    }

    if (Entry->DriveImagePath) {
      DuplicateEntry->DriveImagePath = EfiStrDuplicate (Entry->DriveImagePath);
    }

    if (Entry->BootBgColor) {
      DuplicateEntry->BootBgColor = (__typeof__(DuplicateEntry->BootBgColor))AllocateCopyPool (sizeof(EG_PIXEL), Entry->BootBgColor);
    }

    DuplicateEntry->Image            = Entry->Image;
    DuplicateEntry->DriveImage       = Entry->DriveImage;
    DuplicateEntry->Hotkey           = Entry->Hotkey;
    DuplicateEntry->Flags            = Entry->Flags;
    DuplicateEntry->Type             = Entry->Type;
    DuplicateEntry->VolumeType       = Entry->VolumeType;
    DuplicateEntry->KernelScan       = Entry->KernelScan;
    DuplicateEntry->CustomBoot       = Entry->CustomBoot;
    DuplicateEntry->CustomLogo       = Entry->CustomLogo;

    CopyKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)DuplicateEntry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)),
                              (KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)));
  }

  return DuplicateEntry;
}

STATIC
BOOLEAN
FillinKextPatches (IN OUT KERNEL_AND_KEXT_PATCHES *Patches,
                   TagPtr DictPointer)
{
  TagPtr Prop;
  // UINTN  i;

  if (Patches == NULL || DictPointer == NULL) {
    return FALSE;
  }

  if (NeedPMfix) {
    Patches->KPKernelPm = TRUE;
    Patches->KPAppleIntelCPUPM = TRUE;
  }

  Prop = GetProperty (DictPointer, "Debug");
  if (Prop != NULL || gBootChanged) {
    Patches->KPDebug = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "KernelCpu");
  if (Prop != NULL || gBootChanged) {
    Patches->KPKernelCpu = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "KernelLapic");
  if (Prop != NULL || gBootChanged) {
    Patches->KPKernelLapic = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "KernelXCPM");
  if (Prop != NULL || gBootChanged) {
    Patches->KPKernelXCPM = IsPropertyTrue (Prop);
    if (IsPropertyTrue(Prop)) {
      DBG("KernelXCPM: enabled\n");
    }
  }

  Prop = GetProperty (DictPointer, "KernelPm");
  if (Prop != NULL || gBootChanged) {
    Patches->KPKernelPm = IsPropertyTrue (Prop);
  }
  
  Prop = GetProperty (DictPointer, "PanicNoKextDump");
  if (Prop != NULL || gBootChanged) {
    Patches->KPPanicNoKextDump = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "AppleIntelCPUPM");
  if (Prop != NULL || gBootChanged) {
    Patches->KPAppleIntelCPUPM = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "AppleRTC");
  if (Prop != NULL || gBootChanged) {
    Patches->KPAppleRTC = !IsPropertyFalse (Prop);  //default = TRUE
  }

  //
  // Dell SMBIOS Patch
  //
  // syscl: we do not need gBootChanged and Prop is empty condition
  // this change will boost Dell SMBIOS Patch a bit
  // but the major target is to make code clean
  Prop = GetProperty(DictPointer, "DellSMBIOSPatch");
  Patches->KPDELLSMBIOS = IsPropertyTrue (Prop); // default == FALSE
  gRemapSmBiosIsRequire = Patches->KPDELLSMBIOS;

  Prop = GetProperty (DictPointer, "FakeCPUID");
  if (Prop != NULL || gBootChanged) {
    Patches->FakeCPUID = (UINT32)GetPropertyInteger (Prop, 0);
    DBG ("FakeCPUID: %x\n", Patches->FakeCPUID);
  }

  Prop = GetProperty (DictPointer, "ATIConnectorsController");
  if (Prop != NULL) {
    UINTN len = 0, i=0;

    // ATIConnectors patch
    Patches->KPATIConnectorsController = (__typeof__(Patches->KPATIConnectorsController))AllocateZeroPool (AsciiStrSize(Prop->string) * sizeof(CHAR16));
    AsciiStrToUnicodeStrS (Prop->string, Patches->KPATIConnectorsController, AsciiStrSize(Prop->string));

    Patches->KPATIConnectorsData = GetDataSetting (DictPointer, "ATIConnectorsData", &len);
    Patches->KPATIConnectorsDataLen = len;
    Patches->KPATIConnectorsPatch = GetDataSetting (DictPointer, "ATIConnectorsPatch", &i);

    if (Patches->KPATIConnectorsData == NULL
        || Patches->KPATIConnectorsPatch == NULL
        || Patches->KPATIConnectorsDataLen == 0
        || Patches->KPATIConnectorsDataLen != i) {
      // invalid params - no patching
      DBG ("ATIConnectors patch: invalid parameters!\n");

      if (Patches->KPATIConnectorsController != NULL) {
        FreePool (Patches->KPATIConnectorsController);
      }

      if (Patches->KPATIConnectorsData != NULL) {
        FreePool (Patches->KPATIConnectorsData);
      }

      if (Patches->KPATIConnectorsPatch != NULL) {
        FreePool (Patches->KPATIConnectorsPatch);
      }

      Patches->KPATIConnectorsController = NULL;
      Patches->KPATIConnectorsData       = NULL;
      Patches->KPATIConnectorsPatch      = NULL;
      Patches->KPATIConnectorsDataLen    = 0;
    }
  }

  Prop = GetProperty (DictPointer, "ForceKextsToLoad");
  if (Prop != NULL) {
    INTN   i, Count = GetTagCount (Prop);
    if (Count > 0) {
      TagPtr Prop2 = NULL;
      CHAR16 **newForceKexts = (__typeof__(newForceKexts))AllocateZeroPool ((Patches->NrForceKexts + Count) * sizeof(CHAR16 *));

      if (Patches->ForceKexts != NULL) {
        CopyMem (newForceKexts, Patches->ForceKexts, (Patches->NrForceKexts * sizeof(CHAR16 *)));
        FreePool (Patches->ForceKexts);
      }

      Patches->ForceKexts = newForceKexts;
      DBG ("ForceKextsToLoad: %d requested\n", Count);

      for (i = 0; i < Count; i++) {
        EFI_STATUS Status = GetElement (Prop, i, &Prop2);
        if (EFI_ERROR (Status)) {
          DBG (" - [%02d]: ForceKexts error %r getting next element\n", i, Status);
          continue;
        }

        if (Prop2 == NULL) {
          break;
        }

        if (Prop2->string != NULL) {
          if (*(Prop2->string) == '\\') {
            ++Prop2->string;
          }

          if (AsciiStrSize(Prop2->string) > 1) {
            Patches->ForceKexts[Patches->NrForceKexts] = (__typeof_am__(Patches->ForceKexts[Patches->NrForceKexts]))AllocateZeroPool (AsciiStrSize(Prop2->string) * sizeof(CHAR16));
            AsciiStrToUnicodeStrS(Prop2->string, Patches->ForceKexts[Patches->NrForceKexts], 255);
            DBG (" - [%d]: %s\n", Patches->NrForceKexts, Patches->ForceKexts[Patches->NrForceKexts]);
            ++Patches->NrForceKexts;
          }
        }
      }
    }
  }

  Prop = GetProperty (DictPointer, "KextsToPatch"); //zzzz
  if (Prop != NULL) {
    INTN   i, Count = GetTagCount (Prop);
    //delete old and create new
    if (Patches->KextPatches) {
      for (i = 0; i < Patches->NrKexts; i++) {
        if (Patches->KextPatches[i].Name) {
          FreePool(Patches->KextPatches[i].Name);
        }
        if (Patches->KextPatches[i].Label) {
          FreePool(Patches->KextPatches[i].Label);
        }
        if (Patches->KextPatches[i].Data) {
          FreePool(Patches->KextPatches[i].Data);
        }
        if (Patches->KextPatches[i].Patch) {
          FreePool(Patches->KextPatches[i].Patch);
        }
        if (Patches->KextPatches[i].MaskFind) {
          FreePool(Patches->KextPatches[i].MaskFind);
        }
        if (Patches->KextPatches[i].MaskReplace) {
          FreePool(Patches->KextPatches[i].MaskReplace);
        }
        if (Patches->KextPatches[i].MatchOS) {
          FreePool(Patches->KextPatches[i].MatchOS);
        }
        if (Patches->KextPatches[i].MatchBuild) {
          FreePool(Patches->KextPatches[i].MatchBuild);
        }
      }
      Patches->NrKexts = 0;
      FreePool (Patches->KextPatches);
      Patches->KextPatches = NULL;
    }
    if (Count > 0) {
      TagPtr     Prop2 = NULL, Dict = NULL;
      KEXT_PATCH *newPatches = (__typeof__(newPatches))AllocateZeroPool (Count * sizeof(KEXT_PATCH));

      Patches->KextPatches = newPatches;
      DBG ("KextsToPatch: %d requested\n", Count);
      for (i = 0; i < Count; i++) {
        CHAR8 *KextPatchesName, *KextPatchesLabel;
        UINTN FindLen = 0, ReplaceLen = 0, MaskLen = 0;
        UINT8 *TmpData, *TmpPatch;
        EFI_STATUS Status = GetElement (Prop, i, &Prop2);
        if (EFI_ERROR (Status)) {
          DBG (" - [%02d]: Patches error %r getting next element\n", i, Status);
          continue;
        }

        if (Prop2 == NULL) {
          break;
        }
        DBG (" - [%02d]:", i);

        Dict = GetProperty (Prop2, "Name");
        if (Dict == NULL) {
          DBG(" patch without Name, skipped\n");
          continue;
        }

        KextPatchesName = (__typeof__(KextPatchesName))AllocateCopyPool (255, Dict->string);
        KextPatchesLabel = (__typeof__(KextPatchesLabel))AllocateCopyPool (255, KextPatchesName);

        Dict = GetProperty (Prop2, "Comment");
        if (Dict != NULL) {
          //this is impossible because UnicodeStrToAsciiStr not extend output size
          //         UnicodeStrToAsciiStr(PoolPrint(L"%a (%a)", KextPatchesLabel, Dict->string), KextPatchesLabel);

          AsciiStrCatS(KextPatchesLabel, 255, " (");
          AsciiStrCatS(KextPatchesLabel, 255, Dict->string);
          AsciiStrCatS(KextPatchesLabel, 255, ")");

        } else {
          AsciiStrCatS(KextPatchesLabel, 255, " (NoLabel)");
        }
        DBG (" %a", KextPatchesLabel);

        Patches->KextPatches[Patches->NrKexts].MenuItem.BValue     = TRUE;
        Dict = GetProperty (Prop2, "Disabled");
        if ((Dict != NULL) && IsPropertyTrue (Dict)) {
          Patches->KextPatches[Patches->NrKexts].MenuItem.BValue     = FALSE;
        }

        TmpData    = GetDataSetting (Prop2, "Find", &FindLen);
        TmpPatch   = GetDataSetting (Prop2, "Replace", &ReplaceLen);

        if (!FindLen || !ReplaceLen || (FindLen != ReplaceLen)) {
          DBG (" - invalid Find/Replace data - skipping!\n");
          continue;
        }

        Patches->KextPatches[Patches->NrKexts].Data = (__typeof__(Patches->KextPatches[Patches->NrKexts].Data))AllocateCopyPool (FindLen, TmpData);
        Patches->KextPatches[Patches->NrKexts].DataLen      = FindLen;
        FreePool(TmpData);
        TmpData    = GetDataSetting (Prop2, "MaskFind", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;

        if (TmpData == NULL || MaskLen == 0) {
          Patches->KextPatches[Patches->NrKexts].MaskFind = NULL;
        } else {
          Patches->KextPatches[Patches->NrKexts].MaskFind = (__typeof__(Patches->KextPatches[Patches->NrKexts].MaskFind))AllocatePool (FindLen);
          SetMem(Patches->KextPatches[Patches->NrKexts].MaskFind, FindLen, 0xFF);
          CopyMem(Patches->KextPatches[Patches->NrKexts].MaskFind, TmpData, MaskLen);
        }
        FreePool(TmpData);
        Patches->KextPatches[Patches->NrKexts].Patch = (__typeof__(Patches->KextPatches[Patches->NrKexts].Patch))AllocateCopyPool (FindLen, TmpPatch);
        FreePool(TmpPatch);
        MaskLen = 0;
        TmpData    = GetDataSetting (Prop2, "MaskReplace", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;

        if (TmpData == NULL || MaskLen == 0) {
          Patches->KextPatches[Patches->NrKexts].MaskReplace = NULL;
        } else {
          Patches->KextPatches[Patches->NrKexts].MaskReplace = (__typeof__(Patches->KextPatches[Patches->NrKexts].MaskReplace))AllocateZeroPool (FindLen);
          CopyMem(Patches->KextPatches[Patches->NrKexts].MaskReplace, TmpData, MaskLen);
        }
        FreePool(TmpData);
        Patches->KextPatches[Patches->NrKexts].MatchOS      = NULL;
        Patches->KextPatches[Patches->NrKexts].MatchBuild   = NULL;
        Patches->KextPatches[Patches->NrKexts].Name = (__typeof__(Patches->KextPatches[Patches->NrKexts].Name))AllocateCopyPool (AsciiStrSize(KextPatchesName), KextPatchesName);
        FreePool(KextPatchesName);
        Patches->KextPatches[Patches->NrKexts].Label = (__typeof__(Patches->KextPatches[Patches->NrKexts].Label))AllocateCopyPool (AsciiStrSize(KextPatchesLabel), KextPatchesLabel);
        FreePool(KextPatchesLabel);

        // check enable/disabled patch (OS based) by Micky1979
        Dict = GetProperty (Prop2, "MatchOS");
        if ((Dict != NULL) && (Dict->type == kTagTypeString)) {
          Patches->KextPatches[Patches->NrKexts].MatchOS = (__typeof__(Patches->KextPatches[Patches->NrKexts].MatchOS))AllocateCopyPool (AsciiStrSize(Dict->string), Dict->string);
          DBG(" :: MatchOS: %a", Patches->KextPatches[Patches->NrKexts].MatchOS);
        }

        Dict = GetProperty (Prop2, "MatchBuild");
        if ((Dict != NULL) && (Dict->type == kTagTypeString)) {
          Patches->KextPatches[Patches->NrKexts].MatchBuild = (__typeof__(Patches->KextPatches[Patches->NrKexts].MatchBuild))AllocateCopyPool (AsciiStrSize(Dict->string), Dict->string);
          DBG(" :: MatchBuild: %a", Patches->KextPatches[Patches->NrKexts].MatchBuild);
        }

        // check if this is Info.plist patch or kext binary patch
        Dict = GetProperty (Prop2, "InfoPlistPatch");
        Patches->KextPatches[Patches->NrKexts].IsPlistPatch = IsPropertyTrue (Dict);

        if (Patches->KextPatches[Patches->NrKexts].IsPlistPatch) {
          DBG (" :: PlistPatch");
        } else {
          DBG (" :: BinPatch");
        }

        DBG (" :: data len: %d\n", Patches->KextPatches[Patches->NrKexts].DataLen);
        if (!Patches->KextPatches[Patches->NrKexts++].MenuItem.BValue) {
          DBG(" - patch disabled at config\n");
        }
      }
    }

    //gSettings.NrKexts = (INT32)i;
    //there is one moment. This data is allocated in BS memory but will be used
    // after OnExitBootServices. This is wrong and these arrays should be reallocated
    // but I am not sure
  }


  Prop = GetProperty (DictPointer, "KernelToPatch");
  if (Prop != NULL) {
    INTN   i, Count = GetTagCount (Prop);
    //delete old and create new
    if (Patches->KernelPatches) {
      // free all subarrays
      for (i = 0; i < Patches->NrKernels; i++) {
        if (Patches->KernelPatches[i].Label) {
          FreePool(Patches->KernelPatches[i].Label);
        }
        if (Patches->KernelPatches[i].Data) {
          FreePool(Patches->KernelPatches[i].Data);
        }
        if (Patches->KernelPatches[i].Patch) {
          FreePool(Patches->KernelPatches[i].Patch);
        }
        if (Patches->KernelPatches[i].MaskFind) {
          FreePool(Patches->KernelPatches[i].MaskFind);
        }
        if (Patches->KernelPatches[i].MaskReplace) {
          FreePool(Patches->KernelPatches[i].MaskReplace);
        }
        if (Patches->KernelPatches[i].MatchOS) {
          FreePool(Patches->KernelPatches[i].MatchOS);
        }
        if (Patches->KernelPatches[i].MatchBuild) {
          FreePool(Patches->KernelPatches[i].MatchBuild);
        }
      }
      Patches->NrKernels = 0;
      FreePool (Patches->KernelPatches);
      Patches->KernelPatches = NULL;
    }
    if (Count > 0) {
      TagPtr        Prop2 = NULL, Dict = NULL;
      KERNEL_PATCH  *newPatches = (__typeof__(newPatches))AllocateZeroPool (Count * sizeof(KERNEL_PATCH));

      Patches->KernelPatches = newPatches;
      DBG ("KernelToPatch: %d requested\n", Count);
      for (i = 0; i < Count; i++) {
        CHAR8 *KernelPatchesLabel;
        UINTN FindLen = 0, ReplaceLen = 0, MaskLen = 0;
        UINT8 *TmpData, *TmpPatch;
        EFI_STATUS Status = GetElement (Prop, i, &Prop2);
        if (EFI_ERROR (Status)) {
          DBG (" - [%02d]: Patches error %r getting next element\n", i, Status);
          continue;
        }

        if (Prop2 == NULL) {
          break;
        }
        DBG (" - [%02d]:", i);

        Dict = GetProperty (Prop2, "Comment");
        if (Dict != NULL) {
          KernelPatchesLabel = (__typeof__(KernelPatchesLabel))AllocateCopyPool (AsciiStrSize (Dict->string), Dict->string);
        } else {
          KernelPatchesLabel = (__typeof__(KernelPatchesLabel))AllocateCopyPool (8, "NoLabel");
        }
        DBG (" %a", KernelPatchesLabel);

        Dict = GetProperty (Prop2, "Disabled");
        Patches->KernelPatches[Patches->NrKernels].MenuItem.BValue   = !IsPropertyTrue (Dict);

        TmpData    = GetDataSetting (Prop2, "Find", &FindLen);
        TmpPatch   = GetDataSetting (Prop2, "Replace", &ReplaceLen);

        if (!FindLen || !ReplaceLen || (FindLen != ReplaceLen)) {
          DBG (" :: invalid Find/Replace data - skipping!\n");
          continue;
        }

        Patches->KernelPatches[Patches->NrKernels].Data = (__typeof__(Patches->KernelPatches[Patches->NrKernels].Data))AllocateCopyPool (FindLen, TmpData);
        Patches->KernelPatches[Patches->NrKernels].DataLen      = FindLen;
        FreePool(TmpData);
        TmpData    = GetDataSetting (Prop2, "MaskFind", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;
        if (TmpData == NULL || MaskLen == 0) {
          Patches->KernelPatches[Patches->NrKexts].MaskFind = NULL;
        } else {
          Patches->KernelPatches[Patches->NrKexts].MaskFind = (__typeof__(Patches->KernelPatches[Patches->NrKexts].MaskFind))AllocatePool (FindLen);
          SetMem(Patches->KernelPatches[Patches->NrKexts].MaskFind, FindLen, 0xFF);
          CopyMem(Patches->KernelPatches[Patches->NrKexts].MaskFind, TmpData, MaskLen);
        }
        FreePool(TmpData);
        Patches->KernelPatches[Patches->NrKernels].Patch = (__typeof__(Patches->KernelPatches[Patches->NrKernels].Patch))AllocateCopyPool (FindLen, TmpPatch);
        FreePool(TmpPatch);
        TmpData    = GetDataSetting (Prop2, "MaskReplace", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;
        if (TmpData == NULL || MaskLen == 0) {
          Patches->KernelPatches[Patches->NrKexts].MaskReplace = NULL;
        } else {
          Patches->KernelPatches[Patches->NrKexts].MaskReplace = (__typeof__(Patches->KernelPatches[Patches->NrKexts].MaskReplace))AllocateZeroPool (FindLen);
          CopyMem(Patches->KernelPatches[Patches->NrKexts].MaskReplace, TmpData, MaskLen);
        }
        FreePool(TmpData);
        Patches->KernelPatches[Patches->NrKernels].Count        = 0;
        Patches->KernelPatches[Patches->NrKernels].MatchOS      = NULL;
        Patches->KernelPatches[Patches->NrKernels].MatchBuild   = NULL;
        Patches->KernelPatches[Patches->NrKernels].Label = (__typeof__(Patches->KernelPatches[Patches->NrKernels].Label))AllocateCopyPool (AsciiStrSize (KernelPatchesLabel), KernelPatchesLabel);

        Dict = GetProperty (Prop2, "Count");
        if (Dict != NULL) {
          Patches->KernelPatches[Patches->NrKernels].Count = GetPropertyInteger (Dict, 0);
        }
        FreePool(KernelPatchesLabel);

        // check enable/disabled patch (OS based) by Micky1979
        Dict = GetProperty (Prop2, "MatchOS");
        if ((Dict != NULL) && (Dict->type == kTagTypeString)) {
          Patches->KernelPatches[Patches->NrKernels].MatchOS = (__typeof__(Patches->KernelPatches[Patches->NrKernels].MatchOS))AllocateCopyPool (AsciiStrSize (Dict->string), Dict->string);
          DBG(" :: MatchOS: %a", Patches->KernelPatches[Patches->NrKernels].MatchOS);
        }

        Dict = GetProperty (Prop2, "MatchBuild");
        if ((Dict != NULL) && (Dict->type == kTagTypeString)) {
          Patches->KernelPatches[Patches->NrKernels].MatchBuild = (__typeof__(Patches->KernelPatches[Patches->NrKernels].MatchBuild))AllocateCopyPool (AsciiStrSize (Dict->string), Dict->string);
          DBG(" :: MatchBuild: %a", Patches->KernelPatches[Patches->NrKernels].MatchBuild);
        }
        DBG (" :: data len: %d\n", Patches->KernelPatches[Patches->NrKernels].DataLen);
        Patches->NrKernels++;
      }
    }
  }

  Prop = GetProperty (DictPointer, "BootPatches");
  if (Prop != NULL) {
    INTN   i, Count = GetTagCount (Prop);
    //delete old and create new
    if (Patches->BootPatches) {
      // free all subtree
      for (i = 0; i < Patches->NrBoots; i++) {
        if (Patches->BootPatches[i].Label) {
          FreePool(Patches->BootPatches[i].Label);
        }
        if (Patches->BootPatches[i].Data) {
          FreePool(Patches->BootPatches[i].Data);
        }
        if (Patches->BootPatches[i].Patch) {
          FreePool(Patches->BootPatches[i].Patch);
        }
        if (Patches->BootPatches[i].MaskFind) {
          FreePool(Patches->BootPatches[i].MaskFind);
        }
        if (Patches->BootPatches[i].MaskReplace) {
          FreePool(Patches->BootPatches[i].MaskReplace);
        }
        if (Patches->BootPatches[i].MatchOS) {
          FreePool(Patches->BootPatches[i].MatchOS);
        }
        if (Patches->BootPatches[i].MatchBuild) {
          FreePool(Patches->BootPatches[i].MatchBuild);
        }
      }
      Patches->NrBoots = 0;
      FreePool (Patches->BootPatches);
    }
    if (Count > 0) {
      TagPtr        Prop2 = NULL, Dict = NULL;
      KERNEL_PATCH  *newPatches = (__typeof__(newPatches))AllocateZeroPool (Count * sizeof(KERNEL_PATCH));

      Patches->BootPatches = newPatches;
      DBG ("BootPatches: %d requested\n", Count);
      for (i = 0; i < Count; i++) {
        CHAR8 *BootPatchesLabel;
        UINTN FindLen = 0, ReplaceLen = 0, MaskLen = 0;
        UINT8 *TmpData, *TmpPatch;
        EFI_STATUS Status = GetElement (Prop, i, &Prop2);
        if (EFI_ERROR (Status)) {
          DBG (" - [%02d]: error %r getting next element\n", i, Status);
          continue;
        }
        if (Prop2 == NULL) {
          break;
        }
        DBG (" - [%02d]:", i);

        Dict = GetProperty (Prop2, "Comment");
        if (Dict != NULL) {
          BootPatchesLabel = (__typeof__(BootPatchesLabel))AllocateCopyPool (AsciiStrSize (Dict->string), Dict->string);
        } else {
          BootPatchesLabel = (__typeof__(BootPatchesLabel))AllocateCopyPool (8, "NoLabel");
        }

        DBG (" %a", BootPatchesLabel);

        Dict = GetProperty (Prop2, "Disabled");
        Patches->BootPatches[Patches->NrBoots].MenuItem.BValue   = !IsPropertyTrue (Dict);
        Patches->BootPatches[Patches->NrBoots].MenuItem.ItemType = BoolValue;

        TmpData    = GetDataSetting (Prop2, "Find", &FindLen);
        TmpPatch   = GetDataSetting (Prop2, "Replace", &ReplaceLen);
        if (!FindLen || !ReplaceLen || (FindLen != ReplaceLen)) {
          DBG (" :: invalid Find/Replace data - skipping!\n");
          continue;
        }

        Patches->BootPatches[Patches->NrBoots].Data = (__typeof__(Patches->BootPatches[Patches->NrBoots].Data))AllocateCopyPool (FindLen, TmpData);
        Patches->BootPatches[Patches->NrBoots].DataLen      = FindLen;
        FreePool(TmpData);
        TmpData    = GetDataSetting (Prop2, "MaskFind", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;
        if (TmpData == NULL || MaskLen == 0) {
          Patches->BootPatches[Patches->NrKexts].MaskFind = NULL;
        } else {
          Patches->BootPatches[Patches->NrKexts].MaskFind = (__typeof__(Patches->BootPatches[Patches->NrKexts].MaskFind))AllocatePool (FindLen);
          SetMem(Patches->BootPatches[Patches->NrKexts].MaskFind, FindLen, 0xFF);
          CopyMem(Patches->BootPatches[Patches->NrKexts].MaskFind, TmpData, MaskLen);
        }
        FreePool(TmpData);
        Patches->BootPatches[Patches->NrBoots].Patch = (__typeof__(Patches->BootPatches[Patches->NrBoots].Patch))AllocateCopyPool (FindLen, TmpPatch);
        FreePool(TmpPatch);
        TmpData    = GetDataSetting (Prop2, "MaskReplace", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;
        if (TmpData == NULL || MaskLen == 0) {
          Patches->BootPatches[Patches->NrKexts].MaskReplace = NULL;
        } else {
          Patches->BootPatches[Patches->NrKexts].MaskReplace = (__typeof__(Patches->BootPatches[Patches->NrKexts].MaskReplace))AllocateZeroPool (FindLen);
          CopyMem(Patches->BootPatches[Patches->NrKexts].MaskReplace, TmpData, MaskLen);
        }
        FreePool(TmpData);
        Patches->BootPatches[Patches->NrBoots].Count        = 0;
        Patches->BootPatches[Patches->NrBoots].MatchOS      = NULL;
        Patches->BootPatches[Patches->NrBoots].MatchBuild   = NULL;
        Patches->BootPatches[Patches->NrBoots].Label = (__typeof__(Patches->BootPatches[Patches->NrBoots].Label))AllocateCopyPool (AsciiStrSize (BootPatchesLabel), BootPatchesLabel);

        Dict = GetProperty (Prop2, "Count");
        if (Dict != NULL) {
          Patches->BootPatches[Patches->NrBoots].Count = GetPropertyInteger (Dict, 0);
        }
        FreePool(BootPatchesLabel);

        Dict = GetProperty (Prop2, "MatchOS");
        if ((Dict != NULL) && (Dict->type == kTagTypeString)) {
          Patches->BootPatches[Patches->NrBoots].MatchOS = (__typeof__(Patches->BootPatches[Patches->NrBoots].MatchOS))AllocateCopyPool (AsciiStrSize (Dict->string), Dict->string);
          DBG(" :: MatchOS: %a", Patches->BootPatches[Patches->NrBoots].MatchOS);
        }

        Dict = GetProperty (Prop2, "MatchBuild");
        if ((Dict != NULL) && (Dict->type == kTagTypeString)) {
          Patches->BootPatches[Patches->NrBoots].MatchBuild = (__typeof__(Patches->BootPatches[Patches->NrBoots].MatchBuild))AllocateCopyPool (AsciiStrSize (Dict->string), Dict->string);
          DBG(" :: MatchBuild: %a", Patches->BootPatches[Patches->NrBoots].MatchBuild);
        }

        DBG (" :: data len: %d\n", Patches->BootPatches[Patches->NrBoots].DataLen);
        Patches->NrBoots++;
      }
    }
  }


  return TRUE;
}

BOOLEAN
IsPatchEnabled (CHAR8 *MatchOSEntry, CHAR8 *CurrOS)
{
  INTN i;
  BOOLEAN ret = FALSE;
  struct MatchOSes *mos; // = (__typeof__(mos))AllocatePool(sizeof(struct MatchOSes));

  if (!MatchOSEntry || !CurrOS) {
    return TRUE; //undefined matched corresponds to old behavior
  }

  mos = GetStrArraySeparatedByChar(MatchOSEntry, ',');
  if (!mos) {
    return TRUE; //memory fails -> anyway the patch enabled
  }
  
  TrimMatchOSArray(mos);
  
  if (AsciiStrStr(mos->array[0], "All") != NULL) {
    return TRUE;
  }

  for (i = 0; i < mos->count; ++i) {
    // dot represent MatchOS
    if (
        ((AsciiStrStr(mos->array[i], ".") != NULL) && IsOSValid(mos->array[i], CurrOS)) || // MatchOS
        (AsciiStrStr(mos->array[i], CurrOS) != NULL) // MatchBuild
        ) {
      //DBG ("\nthis patch will activated for OS %s!\n", mos->array[i]);
      ret =  TRUE;
      break;
    }
  }
  deallocMatchOSes(mos);
  return ret;
}

struct
MatchOSes *GetStrArraySeparatedByChar(CHAR8 *str, CHAR8 sep)
{
  struct MatchOSes *mo;
  INTN len = 0, i = 0, inc = 1, newLen = 0;
  //  CHAR8 *comp = NULL; //unused
  CHAR8 doubleSep[2];

  mo = (__typeof__(mo))AllocatePool(sizeof(struct MatchOSes));
  if (!mo) {
    return NULL;
  }
  mo->count = countOccurrences( str, sep ) + 1;
  //  DBG("found %d %c in %s\n", mo->count, sep, str);
  len = (INTN)AsciiStrLen(str);
  doubleSep[0] = sep; doubleSep[1] = sep;

  if(AsciiStrStr(str, doubleSep) || !len || str[0] == sep || str[len -1] == sep) {
    mo->count = 0;
    mo->array[0] = NULL;
    //    DBG("emtpy string\n");
    return mo;
  }

  if (mo->count > 1) {
    //INTN indexes[mo->count + 1];
    INTN *indexes = (INTN *) AllocatePool(mo->count + 1);

    for (i = 0; i < len; ++i) {
      CHAR8 c = str[i];
      if (c == sep) {
        indexes[inc]=i;
        //        DBG("index %d = %d\n", inc, i);
        inc++;
      }
    }
    // manually add first index
    indexes[0] = 0;
    // manually add last index
    indexes[mo->count] = len;

    for (i = 0; i < mo->count; ++i) {
      INTN startLocation, endLocation;
      mo->array[i] = 0;

      if (i == 0) {
        startLocation = indexes[0];
        endLocation = indexes[1] - 1;
      } else if (i == mo->count - 1) { // never reach the end of the array
        startLocation = indexes[i] + 1;
        endLocation = len;
      } else {
        startLocation = indexes[i] + 1;
        endLocation = indexes[i + 1] - 1;
      }
      //      DBG("start %d, end %d\n", startLocation, endLocation);
      newLen = (endLocation - startLocation) + 2;
      /*     comp = (CHAR8 *) AllocatePool(newLen);
       AsciiStrnCpy(comp, str + startLocation, newLen);
       comp[newLen] = '\0'; */
      mo->array[i] = (__typeof_am__(mo->array[i]))AllocateCopyPool(newLen, str + startLocation);
      mo->array[i][newLen - 1] = '\0';
    }

    FreePool(indexes);
  }
  else {
    //    DBG("str contains only one component and it is our string %s!\n", str);
    mo->array[0] = (__typeof_am__(mo->array[0]))AllocateCopyPool(AsciiStrLen(str)+1, str);
  }
  return mo;
}

CHAR8*
TrimString(CHAR8* String)
{
  CHAR8 *TempString, *End, *TrimmedString;
  
  if (!String) {
    return NULL;
  }
  
  TempString = String;
  for ( ; *TempString == ' '; TempString++) {
  }
  
  End = TempString + AsciiStrLen(TempString) - 1;
  
  for ( ; (End > TempString) && (*End == ' '); End--) {
  }
  *(End + 1) = '\0';
  
  TrimmedString = (__typeof__(TrimmedString))AllocateCopyPool(AsciiStrSize(TempString), TempString);
  FreePool(String);
  return TrimmedString;
}

VOID
TrimMatchOSArray(struct MatchOSes *s)
{
  INTN i;
  if (!s) {
    return;
  }
  
  for (i = 0; i < s->count; i++) {
    s->array[i] = TrimString(s->array[i]);
  }
}

BOOLEAN IsOSValid(CHAR8 *MatchOS, CHAR8 *CurrOS)
{
  /* example for valid matches are:
   10.7, only 10.7 (10.7.1 will be skipped)
   10.10.2 only 10.10.2 (10.10.1 or 10.10.5 will be skipped)
   10.10.x (or 10.10.X), in this case is valid for all minor version of 10.10 (10.10.(0-9))
   */

  BOOLEAN ret = FALSE;
  struct MatchOSes *osToc;
  struct MatchOSes *currOStoc;

  if (!MatchOS || !CurrOS) {
    return TRUE; //undefined matched corresponds to old behavior
  }

  osToc = GetStrArraySeparatedByChar(MatchOS, '.');
  currOStoc = GetStrArraySeparatedByChar(CurrOS,  '.');

  if (osToc->count == 2) {
    if (currOStoc->count == 2) {
      if (AsciiStrCmp(osToc->array[0], currOStoc->array[0]) == 0
          && AsciiStrCmp(osToc->array[1], currOStoc->array[1]) == 0) {
        ret = TRUE;
      }
    }
  } else if (osToc->count == 3) {
    if (currOStoc->count == 3) {
      if (AsciiStrCmp(osToc->array[0], currOStoc->array[0]) == 0
          && AsciiStrCmp(osToc->array[1], currOStoc->array[1]) == 0
          && AsciiStrCmp(osToc->array[2], currOStoc->array[2]) == 0) {
        ret = TRUE;
      } else if (AsciiStrCmp(osToc->array[0], currOStoc->array[0]) == 0
                 && AsciiStrCmp(osToc->array[1], currOStoc->array[1]) == 0
                 && (AsciiStrCmp(osToc->array[2], "x") == 0 || AsciiStrCmp(osToc->array[2], "X") == 0)) {
        ret = TRUE;
      }
    } else if (currOStoc->count == 2) {
      if (AsciiStrCmp(osToc->array[0], currOStoc->array[0]) == 0
          && AsciiStrCmp(osToc->array[1], currOStoc->array[1]) == 0) {
        ret = TRUE;
      } else if (AsciiStrCmp(osToc->array[0], currOStoc->array[0]) == 0
                 && AsciiStrCmp(osToc->array[1], currOStoc->array[1]) == 0
                 && (AsciiStrCmp(osToc->array[2], "x") == 0 || AsciiStrCmp(osToc->array[2], "X") == 0)) {
        ret = TRUE;
      }
    }
  }

  deallocMatchOSes(osToc);
  deallocMatchOSes(currOStoc);
  return ret;
}

INTN countOccurrences( CHAR8 *s, CHAR8 c )
{
  return *s == '\0'
  ? 0
  : countOccurrences( s + 1, c ) + (*s == c);
}

VOID deallocMatchOSes(struct MatchOSes *s)
{
  INTN i;

  if (!s) {
    return;
  }

  for (i = 0; i < s->count; i++) {
    if (s->array[i]) {
      FreePool(s->array[i]);
    }
  }

  FreePool(s);
}
// End of MatchOS

UINT8 CheckVolumeType(UINT8 VolumeType, TagPtr Prop)
{
  UINT8 VolumeTypeTmp = VolumeType;
  if (AsciiStriCmp (Prop->string, "Internal") == 0) {
    VolumeTypeTmp |= VOLTYPE_INTERNAL;
  } else if (AsciiStriCmp (Prop->string, "External") == 0) {
    VolumeTypeTmp |= VOLTYPE_EXTERNAL;
  } else if (AsciiStriCmp (Prop->string, "Optical") == 0) {
    VolumeTypeTmp |= VOLTYPE_OPTICAL;
  } else if (AsciiStriCmp (Prop->string, "FireWire") == 0) {
    VolumeTypeTmp |= VOLTYPE_FIREWIRE;
  }
  return VolumeTypeTmp;
}

UINT8 GetVolumeType(TagPtr DictPointer)
{
  TagPtr Prop, Prop2;
  UINT8 VolumeType = 0;

  Prop = GetProperty (DictPointer, "VolumeType");
  if (Prop != NULL) {
    if (Prop->type == kTagTypeString) {
      VolumeType = CheckVolumeType(0, Prop);
    } else if (Prop->type == kTagTypeArray) {
      INTN   i, Count = GetTagCount(Prop);
      if (Count > 0) {
        Prop2 = NULL;
        for (i = 0; i < Count; i++) {
          if (EFI_ERROR (GetElement(Prop, i, &Prop2))) {
            continue;
          }

          if (Prop2 == NULL) {
            break;
          }

          if ((Prop2->type != kTagTypeString) || (Prop2->string == NULL)) {
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
                   TagPtr DictPointer,
                   IN      BOOLEAN SubEntry
                   )
{
  TagPtr Prop;

  if ((Entry == NULL) || (DictPointer == NULL)) {
    return FALSE;
  }

  Prop = GetProperty (DictPointer, "Disabled");
  if (IsPropertyTrue (Prop)) {
    return FALSE;
  }

  Prop = GetProperty (DictPointer, "Volume");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Volume) {
      FreePool (Entry->Volume);
    }

    Entry->Volume = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "Path");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Path) {
      FreePool (Entry->Path);
    }

    Entry->Path = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "Settings");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Settings) {
      FreePool (Entry->Settings);
    }

    Entry->Settings = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "CommonSettings");
  Entry->CommonSettings = IsPropertyTrue (Prop);


  Prop = GetProperty (DictPointer, "AddArguments");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Options != NULL) {
      CONST CHAR16 *OldOptions = Entry->Options;
      Entry->Options     = PoolPrint (L"%s %a", OldOptions, Prop->string);
      FreePool (OldOptions);
    } else {
      Entry->Options     = PoolPrint (L"%a", Prop->string);
    }
  } else {
    Prop = GetProperty (DictPointer, "Arguments");
    if (Prop != NULL && (Prop->type == kTagTypeString)) {
      if (Entry->Options != NULL) {
        FreePool (Entry->Options);
      }

      Entry->Options     = PoolPrint (L"%a", Prop->string);
      Entry->Flags       = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);
    }
  }

  Prop = GetProperty (DictPointer, "Title");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->FullTitle != NULL) {
      FreePool (Entry->FullTitle);
      Entry->FullTitle   = NULL;
    }

    if (Entry->Title != NULL) {
      FreePool (Entry->Title);
    }

    Entry->Title = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "FullTitle");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->FullTitle) {
      FreePool (Entry->FullTitle);
    }

    Entry->FullTitle = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "Image");
  if (Prop != NULL) {
    if (Entry->ImagePath) {
      FreePool (Entry->ImagePath);
      Entry->ImagePath = NULL;
    }

    if (Entry->Image) {
      egFreeImage (Entry->Image);
      Entry->Image     = NULL;
    }

    if (Prop->type == kTagTypeString) {
      Entry->ImagePath = PoolPrint (L"%a", Prop->string);
    }
  } else {
    Prop = GetProperty (DictPointer, "ImageData");
    if (Prop != NULL) {
      if (Entry->Image) {
        egFreeImage (Entry->Image);
        Entry->Image     = NULL;
      }
      if (Entry->ImagePath) {
        FreePool (Entry->ImagePath);
        Entry->ImagePath = NULL;
      }
      if (Prop->type == kTagTypeString) {
        UINT32 len       = (UINT32)(AsciiStrLen (Prop->string) >> 1);
        if (len > 0) {
          UINT8 *data    = (UINT8 *)AllocateZeroPool (len);
          if (data) {
            Entry->Image = egDecodePNG(data, hex2bin (Prop->string, data, len), TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->Image     = egDecodePNG (Prop->data, Prop->dataLen, TRUE);
      }
    }
  }

  Prop = GetProperty (DictPointer, "DriveImage");
  if (Prop != NULL) {
    if (Entry->DriveImagePath != NULL) {
      FreePool (Entry->DriveImagePath);
      Entry->DriveImagePath = NULL;
    }

    if (Entry->DriveImage != NULL) {
      egFreeImage (Entry->DriveImage);
      Entry->DriveImage     = NULL;
    }

    if (Prop->type == kTagTypeString) {
      Entry->DriveImagePath = PoolPrint (L"%a", Prop->string);
    }
  } else {
    Prop = GetProperty (DictPointer, "DriveImageData");
    if (Prop != NULL) {
      if (Entry->DriveImage) {
        egFreeImage (Entry->DriveImage);
        Entry->Image          = NULL;
      }

      if (Entry->DriveImagePath != NULL) {
        FreePool (Entry->DriveImagePath);
        Entry->DriveImagePath = NULL;
      }

      if (Prop->type == kTagTypeString) {
        UINT32 len = (UINT32)(AsciiStrLen (Prop->string) >> 1);
        if (len > 0) {
          UINT8 *data = (UINT8 *)AllocateZeroPool (len);
          if (data) {
            Entry->DriveImage = egDecodePNG (data, hex2bin (Prop->string, data, len), TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->DriveImage    = egDecodePNG (Prop->data, Prop->dataLen, TRUE);
      }
    }
  }

  Prop = GetProperty (DictPointer, "Hotkey");
  if (Prop != NULL && (Prop->type == kTagTypeString) && Prop->string) {
    Entry->Hotkey = *(Prop->string);
  }

  // Whether or not to draw boot screen
  Prop = GetProperty (DictPointer, "CustomLogo");
  if (Prop != NULL) {
    if (IsPropertyTrue (Prop)) {
      Entry->CustomBoot    = CUSTOM_BOOT_APPLE;
    } else if ((Prop->type == kTagTypeString) && Prop->string) {
      if (AsciiStriCmp (Prop->string, "Apple") == 0) {
        Entry->CustomBoot  = CUSTOM_BOOT_APPLE;
      } else if (AsciiStriCmp (Prop->string, "Alternate") == 0) {
        Entry->CustomBoot  = CUSTOM_BOOT_ALT_APPLE;
      } else if (AsciiStriCmp (Prop->string, "Theme") == 0) {
        Entry->CustomBoot  = CUSTOM_BOOT_THEME;
      } else {
        CHAR16 *customLogo = PoolPrint (L"%a", Prop->string);
        Entry->CustomBoot  = CUSTOM_BOOT_USER;
        if (Entry->CustomLogo != NULL) {
          egFreeImage (Entry->CustomLogo);
        }
        Entry->CustomLogo  = egLoadImage (SelfRootDir, customLogo, TRUE);
        if (Entry->CustomLogo == NULL) {
          DBG ("Custom boot logo not found at path `%s`!\n", customLogo);
        }
        if (customLogo != NULL) {
          FreePool (customLogo);
        }
      }
    } else if ((Prop->type == kTagTypeData) &&
               (Prop->data != NULL) && (Prop->dataLen > 0)) {
      Entry->CustomBoot = CUSTOM_BOOT_USER;

      if (Entry->CustomLogo != NULL) {
        egFreeImage (Entry->CustomLogo);
      }

      Entry->CustomLogo = egDecodePNG (Prop->data, Prop->dataLen, TRUE);
      if (Entry->CustomLogo == NULL) {
        DBG ("Custom boot logo not decoded from data!\n", Prop->string);
      }
    } else {
      Entry->CustomBoot = CUSTOM_BOOT_USER_DISABLED;
    }
    DBG ("Custom entry boot %s (0x%X)\n", CustomBootModeToStr (Entry->CustomBoot), Entry->CustomLogo);
  }

  Prop = GetProperty (DictPointer, "BootBgColor");
  if (Prop != NULL && Prop->type == kTagTypeString) {
    UINTN   Color;
    Color = AsciiStrHexToUintn (Prop->string);
    Entry->BootBgColor = (__typeof__(Entry->BootBgColor))AllocateZeroPool (sizeof(EG_PIXEL));
    Entry->BootBgColor->r = (Color >> 24) & 0xFF;
    Entry->BootBgColor->g = (Color >> 16) & 0xFF;
    Entry->BootBgColor->b = (Color >> 8) & 0xFF;
    Entry->BootBgColor->a = (Color >> 0) & 0xFF;
  }

  // Hidden Property, Values:
  // - No (show the entry)
  // - Yes (hide the entry but can be show with F3)
  // - Always (always hide the entry)
  Prop = GetProperty (DictPointer, "Hidden");
  if (Prop != NULL) {
    if ((Prop->type == kTagTypeString) &&
        (AsciiStriCmp (Prop->string, "Always") == 0)) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else if (IsPropertyTrue (Prop)) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_HIDDEN);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_HIDDEN);
    }
  }

  Prop = GetProperty (DictPointer, "Type");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if ((AsciiStriCmp (Prop->string, "OSX") == 0) ||
        (AsciiStriCmp (Prop->string, "macOS") == 0)) {
      Entry->Type = OSTYPE_OSX;
    } else if (AsciiStriCmp (Prop->string, "OSXInstaller") == 0) {
      Entry->Type = OSTYPE_OSX_INSTALLER;
    } else if (AsciiStriCmp (Prop->string, "OSXRecovery") == 0) {
      Entry->Type = OSTYPE_RECOVERY;
    } else if (AsciiStriCmp (Prop->string, "Windows") == 0) {
      Entry->Type = OSTYPE_WINEFI;
    } else if (AsciiStriCmp (Prop->string, "Linux") == 0) {
      Entry->Type = OSTYPE_LIN;
    } else if (AsciiStriCmp (Prop->string, "LinuxKernel") == 0) {
      Entry->Type = OSTYPE_LINEFI;
    } else {
      DBG ("** Warning: unknown custom entry Type '%a'\n", Prop->string);
      Entry->Type = OSTYPE_OTHER;
    }
  } else {
    if (Entry->Type == 0 && Entry->Path) {
      // Try to set Entry->type from Entry->Path
      Entry->Type = GetOSTypeFromPath (Entry->Path);
    }
  }

  Entry->VolumeType = GetVolumeType(DictPointer);

  if (Entry->Options == NULL && OSTYPE_IS_WINDOWS(Entry->Type)) {
    Entry->Options = L"-s -h";
  }

  if (Entry->Title == NULL) {
    if (OSTYPE_IS_OSX_RECOVERY (Entry->Type)) {
      Entry->Title = PoolPrint (L"Recovery");
    } else if (OSTYPE_IS_OSX_INSTALLER (Entry->Type)) {
      Entry->Title = PoolPrint (L"Install macOS");
    }
  }
  if ((Entry->Image == NULL) && (Entry->ImagePath == NULL)) {
    if (OSTYPE_IS_OSX_RECOVERY (Entry->Type)) {
      Entry->ImagePath = L"mac";
    }
  }
  if ((Entry->DriveImage == NULL) && (Entry->DriveImagePath == NULL)) {
    if (OSTYPE_IS_OSX_RECOVERY (Entry->Type)) {
      Entry->DriveImagePath = L"recovery";
    }
  }

  // OS Specific flags
  if (OSTYPE_IS_OSX(Entry->Type) || OSTYPE_IS_OSX_RECOVERY (Entry->Type) || OSTYPE_IS_OSX_INSTALLER (Entry->Type)) {

    // InjectKexts default values
    Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_CHECKFAKESMC);
    //  Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_WITHKEXTS);

    Prop = GetProperty (DictPointer, "InjectKexts");
    if (Prop != NULL) {
      if (Prop->type == kTagTypeTrue) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      } else if ((Prop->type == kTagTypeString) &&
                 (AsciiStriCmp (Prop->string, "Yes") == 0)) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      } else if ((Prop->type == kTagTypeString) &&
                 (AsciiStriCmp (Prop->string, "Detect") == 0)) {
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      } else {
        DBG ("** Warning: unknown custom entry InjectKexts value '%a'\n", Prop->string);
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

    Prop = GetProperty (DictPointer, "NoCaches");
    if (Prop != NULL) {
      if (IsPropertyTrue (Prop)) {
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
      //DBG ("Copying global patch settings\n");
      CopyKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)),
                                (KERNEL_AND_KEXT_PATCHES *)(((UINTN)&gSettings) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)));

      //#ifdef DUMP_KERNEL_KEXT_PATCHES
      //    DumpKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)));
      //#endif

    }

  }

  if (Entry->Type == OSTYPE_LINEFI) {
    Prop = GetProperty (DictPointer, "Kernel");
    if (Prop != NULL) {
      if ((Prop->type == kTagTypeString) && Prop->string) {
        if ((Prop->string[0] == 'N') || (Prop->string[0] == 'n')) {
          Entry->KernelScan = KERNEL_SCAN_NEWEST;
        } else if ((Prop->string[0] == 'O') || (Prop->string[0] == 'o')) {
          Entry->KernelScan = KERNEL_SCAN_OLDEST;
        } else if ((Prop->string[0] == 'F') || (Prop->string[0] == 'f')) {
          Entry->KernelScan = KERNEL_SCAN_FIRST;
        } else if ((Prop->string[0] == 'L') || (Prop->string[0] == 'l')) {
          Entry->KernelScan = KERNEL_SCAN_LAST;
        } else if ((Prop->string[0] == 'M') || (Prop->string[0] == 'm')) {
          Entry->KernelScan = KERNEL_SCAN_MOSTRECENT;
        } else if ((Prop->string[0] == 'E') || (Prop->string[0] == 'e')) {
          Entry->KernelScan = KERNEL_SCAN_EARLIEST;
        }
      }
    }
  }

  // Sub entries
  Prop = GetProperty (DictPointer, "SubEntries");
  if (Prop != NULL) {
    if (Prop->type == kTagTypeFalse) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
    } else if (Prop->type != kTagTypeTrue) {
      CUSTOM_LOADER_ENTRY *CustomSubEntry;
      INTN   i, Count = GetTagCount (Prop);
      TagPtr Dict = NULL;
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
      if (Count > 0) {
        for (i = 0; i < Count; i++) {
          if (EFI_ERROR (GetElement (Prop, i, &Dict))) {
            continue;
          }
          if (Dict == NULL) {
            break;
          }
          // Allocate a sub entry
          CustomSubEntry = DuplicateCustomEntry (Entry);
          if (CustomSubEntry) {
            if (!FillinCustomEntry (CustomSubEntry, Dict, TRUE) || !AddCustomSubEntry (Entry, CustomSubEntry)) {
              if (CustomSubEntry) {
                FreePool (CustomSubEntry);
              }
            }
          }
        }
      }
    }
  }
  return TRUE;
}

STATIC
BOOLEAN
FillinCustomLegacy (
                    IN OUT CUSTOM_LEGACY_ENTRY *Entry,
                    TagPtr DictPointer
                    )
{
  TagPtr Prop;
  if ((Entry == NULL) || (DictPointer == NULL)) {
    return FALSE;
  }

  Prop = GetProperty (DictPointer, "Disabled");
  if (IsPropertyTrue (Prop)) {
    return FALSE;
  }

  Prop = GetProperty (DictPointer, "Volume");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Volume != NULL) {
      FreePool (Entry->Volume);
    }

    Entry->Volume = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "FullTitle");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->FullTitle) {
      FreePool (Entry->FullTitle);
    }

    Entry->FullTitle = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "Title");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Title != NULL) {
      FreePool (Entry->Title);
    }

    Entry->Title = PoolPrint (L"%a", Prop->string);
  }
  Prop = GetProperty (DictPointer, "Image");
  if (Prop != NULL) {
    if (Entry->ImagePath != NULL) {
      FreePool (Entry->ImagePath);
      Entry->ImagePath = NULL;
    }

    if (Entry->Image != NULL) {
      egFreeImage (Entry->Image);
      Entry->Image = NULL;
    }

    if (Prop->type == kTagTypeString) {
      Entry->ImagePath = PoolPrint (L"%a", Prop->string);
    }
  } else {
    Prop = GetProperty (DictPointer, "ImageData");
    if (Prop != NULL) {
      if (Entry->Image != NULL) {
        egFreeImage (Entry->Image);
        Entry->Image = NULL;
      }

      if (Prop->type == kTagTypeString) {
        UINT32 Len       = (UINT32)(AsciiStrLen (Prop->string) >> 1);
        if (Len > 0) {
          UINT8 *Data    = (UINT8 *)AllocateZeroPool (Len);
          if (Data != NULL) {
            Entry->Image = egDecodePNG (Data, hex2bin (Prop->string, Data, Len), TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->Image     = egDecodePNG (Prop->data, Prop->dataLen, TRUE);
      }
    }
  }

  Prop = GetProperty (DictPointer, "DriveImage");
  if (Prop != NULL) {
    if (Entry->DriveImagePath != NULL) {
      FreePool (Entry->DriveImagePath);
      Entry->DriveImagePath = NULL;
    }

    if (Entry->DriveImage != NULL) {
      egFreeImage (Entry->DriveImage);
      Entry->DriveImage = NULL;
    }

    if (Prop->type == kTagTypeString) {
      Entry->DriveImagePath = PoolPrint (L"%a", Prop->string);
    }
  } else {
    Prop = GetProperty (DictPointer, "DriveImageData");
    if (Prop != NULL) {
      if (Entry->DriveImage != NULL) {
        egFreeImage (Entry->DriveImage);
        Entry->Image = NULL;
      }

      if (Entry->DriveImagePath != NULL) {
        FreePool (Entry->DriveImagePath);
        Entry->DriveImagePath = NULL;
      }

      if (Prop->type == kTagTypeString) {
        UINT32 len = (UINT32)(AsciiStrLen (Prop->string) >> 1);
        if (len > 0) {
          UINT8 *data = (UINT8 *)AllocateZeroPool (len);
          if (data) {
            Entry->DriveImage = egDecodePNG (data, hex2bin (Prop->string, data, len), TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->DriveImage = egDecodePNG (Prop->data, Prop->dataLen, TRUE);
      }
    }
  }

  Prop = GetProperty (DictPointer, "Hotkey");
  if (Prop != NULL && (Prop->type == kTagTypeString) && Prop->string) {
    Entry->Hotkey = *(Prop->string);
  }

  // Hidden Property, Values:
  // - No (show the entry)
  // - Yes (hide the entry but can be show with F3)
  // - Always (always hide the entry)
  Prop = GetProperty (DictPointer, "Hidden");
  if (Prop != NULL) {
    if ((Prop->type == kTagTypeString) &&
        (AsciiStriCmp (Prop->string, "Always") == 0)) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else if (IsPropertyTrue (Prop)) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_HIDDEN);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_HIDDEN);
    }
  }

  Prop = GetProperty (DictPointer, "Type");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (AsciiStriCmp (Prop->string, "Windows") == 0) {
      Entry->Type = OSTYPE_WIN;
    } else if (AsciiStriCmp (Prop->string, "Linux") == 0) {
      Entry->Type = OSTYPE_LIN;
    } else {
      Entry->Type = OSTYPE_OTHER;
    }
  }

  Entry->VolumeType = GetVolumeType(DictPointer);
  return TRUE;
}

STATIC
BOOLEAN
FillinCustomTool (
                  IN OUT CUSTOM_TOOL_ENTRY *Entry,
                  TagPtr            DictPointer
                  )
{
  TagPtr Prop;
  if ((Entry == NULL) || (DictPointer == NULL)) {
    return FALSE;
  }

  Prop = GetProperty (DictPointer, "Disabled");
  if (IsPropertyTrue (Prop)) {
    return FALSE;
  }

  Prop = GetProperty (DictPointer, "Volume");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Volume) {
      FreePool (Entry->Volume);
    }
    Entry->Volume = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "Path");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Path != NULL) {
      FreePool (Entry->Path);
    }

    Entry->Path = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "Arguments");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Options != NULL) {
      FreePool (Entry->Options);
    } else {
      Entry->Options = PoolPrint (L"%a", Prop->string);
    }
  }

  Prop = GetProperty (DictPointer, "FullTitle");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->FullTitle != NULL) {
      FreePool (Entry->FullTitle);
    }

    Entry->FullTitle = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "Title");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Title != NULL) {
      FreePool (Entry->Title);
    }
    Entry->Title = PoolPrint (L"%a", Prop->string);
  }

  Prop = GetProperty (DictPointer, "Image");
  if (Prop != NULL) {
    if (Entry->ImagePath != NULL) {
      FreePool (Entry->ImagePath);
      Entry->ImagePath = NULL;
    }

    if (Entry->Image != NULL) {
      egFreeImage (Entry->Image);
      Entry->Image = NULL;
    }

    if (Prop->type == kTagTypeString) {
      Entry->ImagePath = PoolPrint (L"%a", Prop->string);
    }
  } else {
    Prop = GetProperty (DictPointer, "ImageData");
    if (Prop != NULL) {
      if (Entry->Image != NULL) {
        egFreeImage (Entry->Image);
        Entry->Image = NULL;
      }

      if (Prop->type == kTagTypeString) {
        UINT32 Len = (UINT32)(AsciiStrLen (Prop->string) >> 1);
        if (Len > 0) {
          UINT8 *data = (UINT8 *)AllocateZeroPool (Len);
          if (data != NULL) {
            Entry->Image = egDecodePNG (data, hex2bin (Prop->string, data, Len), TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->Image = egDecodePNG (Prop->data, Prop->dataLen, TRUE);
      }
    }
  }
  Prop = GetProperty (DictPointer, "Hotkey");
  if (Prop != NULL && (Prop->type == kTagTypeString) && Prop->string) {
    Entry->Hotkey = *(Prop->string);
  }

  // Hidden Property, Values:
  // - No (show the entry)
  // - Yes (hide the entry but can be show with F3)
  // - Always (always hide the entry)
  Prop = GetProperty (DictPointer, "Hidden");
  if (Prop != NULL) {
    if ((Prop->type == kTagTypeString) &&
        (AsciiStriCmp (Prop->string, "Always") == 0)) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else if (IsPropertyTrue (Prop)) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_HIDDEN);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_HIDDEN);
    }
  }

  Entry->VolumeType = GetVolumeType(DictPointer);

  return TRUE;
}

// EDID reworked by Sherlocks
VOID
GetEDIDSettings(TagPtr DictPointer)
{
  TagPtr Prop, Dict;
  UINTN  j = 128;

  Dict = GetProperty (DictPointer, "EDID");
  if (Dict != NULL) {
    Prop = GetProperty (Dict, "Inject");
    gSettings.InjectEDID = IsPropertyTrue(Prop); // default = false!

    if (gSettings.InjectEDID){
      //DBG ("Inject EDID\n");
      Prop = GetProperty (Dict, "Custom");
      if (Prop != NULL) {
        gSettings.CustomEDID   = GetDataSetting(Dict, "Custom", &j);
        if ((j % 128) != 0) {
          DBG (" Custom EDID has wrong length=%d\n", j);
        } else {
          DBG (" Custom EDID is ok\n");
          gSettings.CustomEDIDsize = (UINT16)j;
          InitializeEdidOverride();
        }
      }

      Prop = GetProperty (Dict, "VendorID");
      if (Prop) {
        gSettings.VendorEDID = (UINT16)GetPropertyInteger(Prop, gSettings.VendorEDID);
        //DBG("  VendorID = 0x%04lx\n", gSettings.VendorEDID);
      }

      Prop = GetProperty (Dict, "ProductID");
      if (Prop) {
        gSettings.ProductEDID = (UINT16)GetPropertyInteger(Prop, gSettings.ProductEDID);
        //DBG("  ProductID = 0x%04lx\n", gSettings.ProductEDID);
      }

      Prop = GetProperty (Dict, "HorizontalSyncPulseWidth");
      if (Prop) {
        gSettings.EdidFixHorizontalSyncPulseWidth = (UINT16)GetPropertyInteger(Prop, gSettings.EdidFixHorizontalSyncPulseWidth);
        //DBG("  EdidFixHorizontalSyncPulseWidth = 0x%02lx\n", gSettings.EdidFixHorizontalSyncPulseWidth);
      }

      Prop = GetProperty (Dict, "VideoInputSignal");
      if (Prop) {
        gSettings.EdidFixVideoInputSignal = (UINT8)GetPropertyInteger(Prop, gSettings.EdidFixVideoInputSignal);
        //DBG("  EdidFixVideoInputSignal = 0x%02lx\n", gSettings.EdidFixVideoInputSignal);
      }
    } else {
      //DBG ("Not Inject EDID\n");
    }
  }
}

EFI_STATUS
GetEarlyUserSettings (
                      IN EFI_FILE *RootDir,
                      TagPtr CfgDict
                      )
{
  EFI_STATUS  Status = EFI_SUCCESS;
  TagPtr      Dict;
  TagPtr      Dict2;
  TagPtr      DictPointer;
  TagPtr      Prop;
  VOID        *Value = NULL;
  UINTN       Size = 0;
  BOOLEAN     SpecialBootMode = FALSE;

  //read aptiofixflag from nvram for special boot
  Status = GetVariable2 (L"aptiofixflag", &gEfiAppleBootGuid, &Value, &Size);
  if (!EFI_ERROR(Status)) {
    SpecialBootMode = TRUE;
    FreePool(Value);
  }



  gSettings.KextPatchesAllowed              = TRUE;
  gSettings.KernelAndKextPatches.KPAppleRTC = TRUE;
  gSettings.KernelAndKextPatches.KPDELLSMBIOS = FALSE; // default is false
  gSettings.KernelPatchesAllowed            = TRUE;

  Dict = CfgDict;
  if (Dict != NULL) {
    //DBG ("Loading early settings\n");
    DbgHeader("GetEarlyUserSettings");

    DictPointer = GetProperty (Dict, "Boot");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "Timeout");
      if (Prop != NULL) {
        GlobalConfig.Timeout = (INT32)GetPropertyInteger (Prop, GlobalConfig.Timeout);
        DBG ("timeout set to %d\n", GlobalConfig.Timeout);
      }

      Prop = GetProperty (DictPointer, "SkipHibernateTimeout");
      gSettings.SkipHibernateTimeout = IsPropertyTrue(Prop);

      //DisableCloverHotkeys
      Prop = GetProperty (DictPointer, "DisableCloverHotkeys");
      gSettings.DisableCloverHotkeys = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "Arguments");
      if (Prop != NULL && (Prop->type == kTagTypeString) && Prop->string != NULL) {
        AsciiStrnCpyS(gSettings.BootArgs, 256, Prop->string, 255);
      }

      // defaults if "DefaultVolume" is not present or is empty
      gSettings.LastBootedVolume = FALSE;
      //     gSettings.DefaultVolume    = NULL;

      Prop = GetProperty (DictPointer, "DefaultVolume");
      if (Prop != NULL) {
        Size = AsciiStrSize (Prop->string);
        if (Size > 0) {
          if (gSettings.DefaultVolume  != NULL) { //override value from Boot Option
            FreePool(gSettings.DefaultVolume);
            gSettings.DefaultVolume = NULL;
          }

          // check for special value for remembering boot volume
          if (AsciiStriCmp (Prop->string, "LastBootedVolume") == 0) {
            gSettings.LastBootedVolume = TRUE;
          } else {
            gSettings.DefaultVolume = (__typeof__(gSettings.DefaultVolume))AllocateZeroPool (Size * sizeof(CHAR16));
            AsciiStrToUnicodeStrS(Prop->string, gSettings.DefaultVolume, Size);
          }
        }
      }

      Prop = GetProperty (DictPointer, "DefaultLoader");
      if (Prop != NULL) {
        gSettings.DefaultLoader = (__typeof__(gSettings.DefaultLoader))AllocateZeroPool (AsciiStrSize (Prop->string) * sizeof(CHAR16));
        AsciiStrToUnicodeStrS (Prop->string, gSettings.DefaultLoader, AsciiStrSize (Prop->string));
      }

      Prop = GetProperty (DictPointer, "Debug");
      GlobalConfig.DebugLog       = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "Fast");
      GlobalConfig.FastBoot       = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "NoEarlyProgress");
      GlobalConfig.NoEarlyProgress = IsPropertyTrue (Prop);

      if (SpecialBootMode) {
        GlobalConfig.FastBoot       = TRUE;
        DBG ("Fast option enabled\n");
      }

      Prop = GetProperty (DictPointer, "NeverHibernate");
      GlobalConfig.NeverHibernate = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "StrictHibernate");
      GlobalConfig.StrictHibernate = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "RtcHibernateAware");
      GlobalConfig.RtcHibernateAware = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "HibernationFixup");
      if (Prop) {
        GlobalConfig.HibernationFixup = IsPropertyTrue (Prop); //it will be set automatically
      }

      Prop = GetProperty (DictPointer, "SignatureFixup");
      GlobalConfig.SignatureFixup = IsPropertyTrue (Prop);

      //      Prop = GetProperty (DictPointer, "GetLegacyLanAddress");
      //      GetLegacyLanAddress = IsPropertyTrue (Prop);

      // Secure boot
      Prop = GetProperty (DictPointer, "Secure");
      if (Prop != NULL) {
        if (Prop->type == kTagTypeFalse) {
          // Only disable setup mode, we want always secure boot
          gSettings.SecureBootSetupMode = 0;
        } else if ((Prop->type == kTagTypeTrue) && !gSettings.SecureBoot) {
          // This mode will force boot policy even when no secure boot or it is disabled
          gSettings.SecureBootSetupMode = 1;
          gSettings.SecureBoot          = 1;
        }
      }
      // Secure boot policy
      Prop = GetProperty (DictPointer, "Policy");
      if (Prop != NULL && (Prop->type == kTagTypeString) && Prop->string) {
        if ((Prop->string[0] == 'D') || (Prop->string[0] == 'd')) {
          // Deny all images
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_DENY;
        } else if ((Prop->string[0] == 'A') || (Prop->string[0] == 'a')) {
          // Allow all images
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_ALLOW;
        } else if ((Prop->string[0] == 'Q') || (Prop->string[0] == 'q')) {
          // Query user
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_QUERY;
        } else if ((Prop->string[0] == 'I') || (Prop->string[0] == 'i')) {
          // Insert
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_INSERT;
        } else if ((Prop->string[0] == 'W') || (Prop->string[0] == 'w')) {
          // White list
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_WHITELIST;
        } else if ((Prop->string[0] == 'B') || (Prop->string[0] == 'b')) {
          // Black list
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_BLACKLIST;
        } else if ((Prop->string[0] == 'U') || (Prop->string[0] == 'u')) {
          // User policy
          gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_USER;
        }
      }
      // Secure boot white list
      Prop = GetProperty (DictPointer, "WhiteList");
      if (Prop != NULL && (Prop->type == kTagTypeArray)) {
        INTN   i, Count = GetTagCount (Prop);
        if (Count > 0) {
          gSettings.SecureBootWhiteListCount = 0;
          gSettings.SecureBootWhiteList = (__typeof__(gSettings.SecureBootWhiteList))AllocateZeroPool (Count * sizeof(CHAR16 *));
          if (gSettings.SecureBootWhiteList) {
            for (i = 0; i < Count; i++) {
              if (EFI_ERROR (GetElement (Prop, i, &Dict2))) {
                continue;
              }

              if (Dict2 == NULL) {
                break;
              }

              if ((Dict2->type == kTagTypeString) && Dict2->string) {
                gSettings.SecureBootWhiteList[gSettings.SecureBootWhiteListCount++] = PoolPrint (L"%a", Dict2->string);
              }
            }
          }
        }
      }
      // Secure boot black list
      Prop = GetProperty (DictPointer, "BlackList");
      if (Prop != NULL && (Prop->type == kTagTypeArray)) {
        INTN   i, Count = GetTagCount (Prop);
        if (Count > 0) {
          gSettings.SecureBootBlackListCount = 0;
          gSettings.SecureBootBlackList = (__typeof__(gSettings.SecureBootBlackList))AllocateZeroPool (Count * sizeof(CHAR16 *));
          if (gSettings.SecureBootBlackList) {
            for (i = 0; i < Count; i++) {
              if (EFI_ERROR (GetElement (Prop, i, &Dict2))) {
                continue;
              }

              if (Dict2 == NULL) {
                break;
              }

              if ((Dict2->type == kTagTypeString) && Dict2->string) {
                gSettings.SecureBootBlackList[gSettings.SecureBootBlackListCount++] = PoolPrint (L"%a", Dict2->string);
              }
            }
          }
        }
      }

      // XMP memory profiles
      Prop = GetProperty (DictPointer, "XMPDetection");
      if (Prop != NULL) {
        gSettings.XMPDetection = 0;
        if (Prop->type == kTagTypeFalse) {
          gSettings.XMPDetection = -1;
        } else if (Prop->type == kTagTypeString) {
          if ((Prop->string[0] == 'n') ||
              (Prop->string[0] == 'N') ||
              (Prop->string[0] == '-')) {
            gSettings.XMPDetection = -1;
          } else {
            gSettings.XMPDetection = (INT8)AsciiStrDecimalToUintn (Prop->string);
          }
        } else if (Prop->type == kTagTypeInteger) {
          gSettings.XMPDetection   = (INT8)(UINTN)Prop->string;
        }
        // Check that the setting value is sane
        if ((gSettings.XMPDetection < -1) || (gSettings.XMPDetection > 2)) {
          gSettings.XMPDetection   = -1;
        }
      }

      // Legacy bios protocol
      Prop = GetProperty (DictPointer, "Legacy");
      if (Prop != NULL)  {
        AsciiStrToUnicodeStrS (Prop->string, gSettings.LegacyBoot, 32);
      } else if (gFirmwareClover) {
        // default for CLOVER EFI boot
        UnicodeSPrint (gSettings.LegacyBoot, sizeof(gSettings.LegacyBoot), L"PBR");
      } else {
        // default for UEFI boot
        UnicodeSPrint (gSettings.LegacyBoot, sizeof(gSettings.LegacyBoot), L"LegacyBiosDefault");
      }

      // Entry for LegacyBiosDefault
      Prop = GetProperty (DictPointer, "LegacyBiosDefaultEntry");
      if (Prop != NULL) {
        gSettings.LegacyBiosDefaultEntry = (UINT16)GetPropertyInteger (Prop, 0); // disabled by default
      }

      // Whether or not to draw boot screen
      Prop = GetProperty (DictPointer, "CustomLogo");
      if (Prop != NULL) {
        if (IsPropertyTrue (Prop)) {
          gSettings.CustomBoot   = CUSTOM_BOOT_APPLE;
        } else if ((Prop->type == kTagTypeString) && Prop->string) {
          if (AsciiStriCmp (Prop->string, "Apple") == 0) {
            gSettings.CustomBoot = CUSTOM_BOOT_APPLE;
          } else if (AsciiStriCmp (Prop->string, "Alternate") == 0) {
            gSettings.CustomBoot = CUSTOM_BOOT_ALT_APPLE;
          } else if (AsciiStriCmp (Prop->string, "Theme") == 0) {
            gSettings.CustomBoot = CUSTOM_BOOT_THEME;
          } else {
            CHAR16 *customLogo   = PoolPrint (L"%a", Prop->string);
            gSettings.CustomBoot = CUSTOM_BOOT_USER;
            if (gSettings.CustomLogo != NULL) {
              egFreeImage (gSettings.CustomLogo);
            }

            gSettings.CustomLogo = egLoadImage (RootDir, customLogo, TRUE);
            if (gSettings.CustomLogo == NULL) {
              DBG ("Custom boot logo not found at path `%s`!\n", customLogo);
            }

            if (customLogo != NULL) {
              FreePool (customLogo);
            }
          }
        } else if ((Prop->type == kTagTypeData) &&
                   (Prop->data != NULL) && (Prop->dataLen > 0)) {
          gSettings.CustomBoot = CUSTOM_BOOT_USER;
          if (gSettings.CustomLogo != NULL) {
            egFreeImage (gSettings.CustomLogo);
          }

          gSettings.CustomLogo = egDecodePNG (Prop->data, Prop->dataLen, TRUE);
          if (gSettings.CustomLogo == NULL) {
            DBG ("Custom boot logo not decoded from data!\n", Prop->string);
          }
        } else {
          gSettings.CustomBoot = CUSTOM_BOOT_USER_DISABLED;
        }
      } else {
        gSettings.CustomBoot   = CUSTOM_BOOT_DISABLED;
      }

      DBG ("Custom boot %s (0x%X)\n", CustomBootModeToStr (gSettings.CustomBoot), gSettings.CustomLogo);
    }

    //*** SYSTEM ***

    DictPointer = GetProperty (Dict, "SystemParameters");
    if (DictPointer != NULL) {
      // Inject kexts
      Prop = GetProperty (DictPointer, "InjectKexts");
      if (Prop != NULL) {
        if (IsPropertyTrue (Prop)) {
          gSettings.WithKexts            = TRUE;
        } else if ((Prop->type == kTagTypeString) &&
                   (AsciiStriCmp (Prop->string, "Detect") == 0)) {
          //   gSettings.WithKexts            = TRUE;
          gSettings.WithKextsIfNoFakeSMC = TRUE;
        }
      } else {
        gSettings.WithKexts            = TRUE;  //default
      }

      // No caches - obsolete
      Prop = GetProperty (DictPointer, "NoCaches");
      if (IsPropertyTrue (Prop)) {
        gSettings.NoCaches = TRUE;
      }
    }

    // KernelAndKextPatches
    DictPointer = GetProperty (Dict, "KernelAndKextPatches");
    if (DictPointer != NULL) {
      FillinKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)&gSettings) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)), DictPointer);
    }

    DictPointer = GetProperty (Dict, "GUI");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "Timezone");
      GlobalConfig.Timezone = (INT32)GetPropertyInteger (Prop, GlobalConfig.Timezone);
      //initialize Daylight when we know timezone
      EFI_TIME          Now;
      gRT->GetTime(&Now, NULL);
      INT32 NowHour = Now.Hour + GlobalConfig.Timezone;
      if (NowHour <  0 ) NowHour += 24;
      if (NowHour >= 24 ) NowHour -= 24;
      DayLight = (NowHour > 8) && (NowHour < 20);

      Prop = GetProperty (DictPointer, "Theme");
      if (Prop != NULL) {
        if ((Prop->type == kTagTypeString) && Prop->string) {
          UINTN i;
          GlobalConfig.Theme = PoolPrint (L"%a", Prop->string);
          DBG ("Default theme: %s\n", GlobalConfig.Theme);
          OldChosenTheme = 0xFFFF; //default for embedded
          for (i = 0; i < ThemesNum; i++) {
            if (StriCmp(GlobalConfig.Theme, ThemesList[i]) == 0) {
              OldChosenTheme = i;
              break;
            }
          }
          if ((AsciiStriCmp (Prop->string, "embedded") == 0) || (AsciiStriCmp (Prop->string, "") == 0)) {
            Prop = GetProperty (DictPointer, "EmbeddedThemeType");
            if (Prop && (Prop->type == kTagTypeString) && Prop->string) {
              if (AsciiStriCmp (Prop->string, "Dark") == 0) {
                GlobalConfig.DarkEmbedded = TRUE;
                GlobalConfig.Font = FONT_GRAY;
              } else if (AsciiStriCmp (Prop->string, "Light") == 0) {
                GlobalConfig.DarkEmbedded = FALSE;
                GlobalConfig.Font = FONT_ALFA;
              } else if (AsciiStriCmp (Prop->string, "DayTime") == 0) {
                GlobalConfig.DarkEmbedded = !DayLight;
                GlobalConfig.Font = DayLight?FONT_ALFA:FONT_GRAY;
              }
            }
          }
        }
      } else if (Prop == NULL) {
        Prop = GetProperty (DictPointer, "EmbeddedThemeType");
        if (Prop && (Prop->type == kTagTypeString) && Prop->string) {
          if (AsciiStriCmp (Prop->string, "Dark") == 0) {
            GlobalConfig.DarkEmbedded = TRUE;
            GlobalConfig.Font = FONT_GRAY;
          } else if (AsciiStriCmp (Prop->string, "Light") == 0) {
            GlobalConfig.DarkEmbedded = FALSE;
            GlobalConfig.Font = FONT_ALFA;
          } else if (AsciiStriCmp (Prop->string, "Daytime") == 0) {
            GlobalConfig.DarkEmbedded = !DayLight;
            GlobalConfig.Font = DayLight?FONT_ALFA:FONT_GRAY;
          }
        }
      }

      Prop = GetProperty (DictPointer, "PlayAsync"); //PlayAsync
      gSettings.PlayAsync = IsPropertyTrue (Prop);

      // CustomIcons
      Prop = GetProperty (DictPointer, "CustomIcons");
      if (IsPropertyTrue (Prop)) {
        GlobalConfig.CustomIcons = TRUE;
      }

      Prop = GetProperty (DictPointer, "ShowOptimus");
      GlobalConfig.ShowOptimus = IsPropertyTrue (Prop);
      //DBG("ShowOptimus set to %d\n", GlobalConfig.ShowOptimus);

      Prop = GetProperty (DictPointer, "TextOnly");
      if (IsPropertyTrue (Prop)) {
        GlobalConfig.TextOnly = TRUE;
        //DBG ("TextOnly option enabled\n");
      }

      Prop = GetProperty (DictPointer, "ScreenResolution");
      if (Prop != NULL) {
        if ((Prop->type == kTagTypeString) && Prop->string) {
          GlobalConfig.ScreenResolution = PoolPrint (L"%a", Prop->string);
        }
      }

      Prop = GetProperty (DictPointer, "ConsoleMode");
      if (Prop != NULL) {
        if (Prop->type == kTagTypeInteger) {
          GlobalConfig.ConsoleMode = (INT32)(UINTN)Prop->string;
        } else if ((Prop->type == kTagTypeString) && Prop->string) {
          if (AsciiStrStr (Prop->string, "Max") != NULL) {
            GlobalConfig.ConsoleMode = -1;
            DBG ("ConsoleMode will be set to highest mode\n");
          } else if (AsciiStrStr (Prop->string, "Min") != NULL) {
            GlobalConfig.ConsoleMode = -2;
            DBG ("ConsoleMode will be set to lowest mode\n");
          } else {
            GlobalConfig.ConsoleMode = (INT32)AsciiStrDecimalToUintn (Prop->string);
          }
        }
        if (GlobalConfig.ConsoleMode > 0) {
          DBG ("ConsoleMode will be set to mode #%d\n", GlobalConfig.ConsoleMode);
        }
      }

      Prop = GetProperty (DictPointer, "Language");
      if (Prop != NULL) {
        AsciiStrCpyS (gSettings.Language, 16, Prop->string);
        if (AsciiStrStr (Prop->string, "en")) {
          gLanguage = english;
          GlobalConfig.Codepage = 0xC0;
          GlobalConfig.CodepageSize = 0;
        } else if (AsciiStrStr (Prop->string, "ru")) {
          gLanguage = russian;
          GlobalConfig.Codepage = 0x410;
          GlobalConfig.CodepageSize = 0x40;
        } else if (AsciiStrStr (Prop->string, "ua")) {
          gLanguage = ukrainian;
          GlobalConfig.Codepage = 0x400;
          GlobalConfig.CodepageSize = 0x60;
        } else if (AsciiStrStr (Prop->string, "fr")) {
          gLanguage = french; //default is extended latin
        } else if (AsciiStrStr (Prop->string, "it")) {
          gLanguage = italian;
        } else if (AsciiStrStr (Prop->string, "es")) {
          gLanguage = spanish;
        } else if (AsciiStrStr (Prop->string, "pt")) {
          gLanguage = portuguese;
        } else if (AsciiStrStr (Prop->string, "br")) {
          gLanguage = brasil;
        } else if (AsciiStrStr (Prop->string, "de")) {
          gLanguage = german;
        } else if (AsciiStrStr (Prop->string, "nl")) {
          gLanguage = dutch;
        } else if (AsciiStrStr (Prop->string, "pl")) {
          gLanguage = polish;
        } else if (AsciiStrStr (Prop->string, "cz")) {
          gLanguage = czech;
        } else if (AsciiStrStr (Prop->string, "hr")) {
          gLanguage = croatian;
        } else if (AsciiStrStr (Prop->string, "id")) {
          gLanguage = indonesian;
        } else if (AsciiStrStr (Prop->string, "zh_CN")) {
          gLanguage = chinese;
          GlobalConfig.Codepage = 0x3400;
          GlobalConfig.CodepageSize = 0x19C0;
        } else if (AsciiStrStr (Prop->string, "ro")) {
          gLanguage = romanian;
        } else if (AsciiStrStr (Prop->string, "ko")) {
          gLanguage = korean;
          GlobalConfig.Codepage = 0x1100;
          GlobalConfig.CodepageSize = 0x100;
        }
      }

      if (gSettings.Language != NULL) {
        Prop = GetProperty (DictPointer, "KbdPrevLang");
        if (Prop != NULL) {
          gSettings.KbdPrevLang = IsPropertyTrue (Prop);
        }
      }

      Prop = GetProperty (DictPointer, "Mouse");
      if (Prop != NULL) {
        Dict2 = GetProperty (Prop, "Speed");
        if (Dict2 != NULL) {
          gSettings.PointerSpeed = (INT32)GetPropertyInteger (Dict2, 0);
          gSettings.PointerEnabled = (gSettings.PointerSpeed != 0);
        }
        //but we can disable mouse even if there was positive speed
        Dict2 = GetProperty (Prop, "Enabled");
        if (IsPropertyFalse (Dict2)) {
          gSettings.PointerEnabled = FALSE;
        }

        Dict2 = GetProperty (Prop, "Mirror");
        if (IsPropertyTrue (Dict2)) {
          gSettings.PointerMirror = TRUE;
        }

        Dict2 = GetProperty (Prop, "DoubleClickTime");
        if (Dict2 != NULL) {
          gSettings.DoubleClickTime = (UINTN)GetPropertyInteger (Dict2, 0);
        }
      }
      // hide by name/uuid
      Prop = GetProperty (DictPointer, "Hide");
      if (Prop != NULL) {
        INTN   i, Count = GetTagCount (Prop);
        if (Count > 0) {
          gSettings.HVCount = 0;
          gSettings.HVHideStrings = (__typeof__(gSettings.HVHideStrings))AllocateZeroPool (Count * sizeof(CHAR16 *));
          if (gSettings.HVHideStrings) {
            for (i = 0; i < Count; i++) {
              if (EFI_ERROR (GetElement (Prop, i, &Dict2))) {
                continue;
              }

              if (Dict2 == NULL) {
                break;
              }

              if ((Dict2->type == kTagTypeString) && Dict2->string) {
                gSettings.HVHideStrings[gSettings.HVCount] = PoolPrint (L"%a", Dict2->string);
                if (gSettings.HVHideStrings[gSettings.HVCount]) {
                  DBG ("Hiding entries with string %s\n", gSettings.HVHideStrings[gSettings.HVCount]);
                  gSettings.HVCount++;
                }
              }
            }
          }
        }
      }
      gSettings.LinuxScan = TRUE;
      // Disable loader scan
      Prop = GetProperty (DictPointer, "Scan");
      if (Prop != NULL) {
        if (IsPropertyFalse (Prop)) {
          gSettings.DisableEntryScan = TRUE;
          gSettings.DisableToolScan  = TRUE;
          GlobalConfig.NoLegacy      = TRUE;
        } else if (Prop->type == kTagTypeDict) {
          Dict2 = GetProperty (Prop, "Entries");
          if (IsPropertyFalse (Dict2)) {
            gSettings.DisableEntryScan = TRUE;
          }

          Dict2 = GetProperty (Prop, "Tool");
          if (IsPropertyFalse (Dict2)) {
            gSettings.DisableToolScan = TRUE;
          }

          Dict2 = GetProperty (Prop, "Linux");
          gSettings.LinuxScan = !IsPropertyFalse (Dict2);

          Dict2 = GetProperty (Prop, "Legacy");
          if (Dict2 != NULL) {
            if (Dict2->type == kTagTypeFalse) {
              GlobalConfig.NoLegacy = TRUE;
            } else if ((Dict2->type == kTagTypeString) && Dict2->string) {
              if ((Dict2->string[0] == 'N') || (Dict2->string[0] == 'n')) {
                GlobalConfig.NoLegacy = TRUE;
              } else if ((Dict2->string[0] == 'F') || (Dict2->string[0] == 'f')) {
                GlobalConfig.LegacyFirst = TRUE;
              }
            }
          }

          Dict2 = GetProperty (Prop, "Kernel");
          if (Dict2 != NULL) {
            if (Dict2->type == kTagTypeFalse) {
              gSettings.KernelScan = KERNEL_SCAN_NONE;
            } else if ((Dict2->type == kTagTypeString) && Dict2->string) {
              if ((Dict2->string[0] == 'N') || (Dict2->string[0] == 'n')) {
                gSettings.KernelScan = ((Dict2->string[1] == 'E') || (Dict2->string[1] == 'e')) ? KERNEL_SCAN_NEWEST : KERNEL_SCAN_NONE;
              } else if ((Dict2->string[0] == 'O') || (Dict2->string[0] == 'o')) {
                gSettings.KernelScan = KERNEL_SCAN_OLDEST;
              } else if ((Dict2->string[0] == 'F') || (Dict2->string[0] == 'f')) {
                gSettings.KernelScan = KERNEL_SCAN_FIRST;
              } else if ((Dict2->string[0] == 'L') || (Dict2->string[0] == 'l')) {
                gSettings.KernelScan = KERNEL_SCAN_LAST;
              } else if ((Dict2->string[0] == 'M') || (Dict2->string[0] == 'm')) {
                gSettings.KernelScan = KERNEL_SCAN_MOSTRECENT;
              } else if ((Dict2->string[0] == 'E') || (Dict2->string[0] == 'e')) {
                gSettings.KernelScan = KERNEL_SCAN_EARLIEST;
              }
            }
          }
        }
      }
      // Custom entries
      Dict2 = GetProperty (DictPointer, "Custom");
      if (Dict2 != NULL) {
        Prop = GetProperty (Dict2, "Entries");
        if (Prop != NULL) {
          CUSTOM_LOADER_ENTRY *Entry;
          INTN   i, Count = GetTagCount (Prop);
          TagPtr Dict3;

          if (Count > 0) {
            for (i = 0; i < Count; i++) {
              if (EFI_ERROR (GetElement (Prop, i, &Dict3))) {
                continue;
              }

              if (Dict3 == NULL) {
                break;
              }
              // Allocate an entry
              Entry = (CUSTOM_LOADER_ENTRY *)AllocateZeroPool (sizeof(CUSTOM_LOADER_ENTRY));
              // Fill it in
              if (Entry != NULL && (!FillinCustomEntry (Entry, Dict3, FALSE) || !AddCustomEntry (Entry))) {
                FreePool (Entry);
              }
            }
          }
        }

        Prop = GetProperty (Dict2, "Legacy");
        if (Prop != NULL) {
          CUSTOM_LEGACY_ENTRY *Entry;
          INTN   i, Count = GetTagCount (Prop);
          TagPtr Dict3;

          if (Count > 0) {
            for (i = 0; i < Count; i++) {
              if (EFI_ERROR (GetElement (Prop, i, &Dict3))) {
                continue;
              }

              if (Dict3 == NULL) {
                break;
              }
              // Allocate an entry
              Entry = (CUSTOM_LEGACY_ENTRY *)AllocateZeroPool (sizeof(CUSTOM_LEGACY_ENTRY));
              if (Entry) {
                // Fill it in
                if (!FillinCustomLegacy(Entry, Dict3) || !AddCustomLegacyEntry (Entry)) {
                  FreePool (Entry);
                }
              }
            }
          }
        }

        Prop = GetProperty (Dict2, "Tool");
        if (Prop != NULL) {
          CUSTOM_TOOL_ENTRY *Entry;
          INTN   i, Count = GetTagCount (Prop);
          TagPtr Dict3;
          if (Count > 0) {
            for (i = 0; i < Count; i++) {
              if (EFI_ERROR (GetElement (Prop, i, &Dict3))) {
                continue;
              }

              if (Dict3 == NULL) {
                break;
              }

              // Allocate an entry
              Entry = (CUSTOM_TOOL_ENTRY *)AllocateZeroPool (sizeof(CUSTOM_TOOL_ENTRY));
              if (Entry) {
                // Fill it in
                if (!FillinCustomTool(Entry, Dict3) || !AddCustomToolEntry (Entry)) {
                  FreePool (Entry);
                }
              }
            }
          }
        }
      }
    }

    DictPointer = GetProperty (Dict, "Graphics");
    if (DictPointer != NULL) {

      Prop                              = GetProperty (DictPointer, "PatchVBios");
      gSettings.PatchVBios              = IsPropertyTrue (Prop);

      gSettings.PatchVBiosBytesCount    = 0;

      Dict2 = GetProperty (DictPointer, "PatchVBiosBytes");
      if (Dict2 != NULL) {
        INTN   i, Count = GetTagCount (Dict2);
        if (Count > 0) {
          VBIOS_PATCH_BYTES *VBiosPatch;
          UINTN             FindSize    = 0;
          UINTN             ReplaceSize = 0;
          BOOLEAN           Valid;

          // alloc space for up to 16 entries
          gSettings.PatchVBiosBytes = (__typeof__(gSettings.PatchVBiosBytes))AllocateZeroPool (Count * sizeof(VBIOS_PATCH_BYTES));

          // get all entries
          for (i = 0; i < Count; i++) {
            // Get the next entry
            if (EFI_ERROR (GetElement (Dict2, i, &Prop))) {
              continue;
            }

            if (Prop == NULL) {
              break;
            }

            Valid = TRUE;
            // read entry
            VBiosPatch          = &gSettings.PatchVBiosBytes[gSettings.PatchVBiosBytesCount];
            VBiosPatch->Find    = GetDataSetting (Prop, "Find",    &FindSize);
            VBiosPatch->Replace = GetDataSetting (Prop, "Replace", &ReplaceSize);

            if (VBiosPatch->Find == NULL || FindSize == 0) {
              Valid = FALSE;
              DBG ("PatchVBiosBytes[%d]: missing Find data\n", i);
            }

            if (VBiosPatch->Replace == NULL || ReplaceSize == 0) {
              Valid = FALSE;
              DBG ("PatchVBiosBytes[%d]: missing Replace data\n", i);
            }

            if (FindSize != ReplaceSize) {
              Valid = FALSE;
              DBG ("PatchVBiosBytes[%d]: Find and Replace data are not the same size\n", i);
            }

            if (Valid) {
              VBiosPatch->NumberOfBytes = FindSize;
              // go to next entry
              ++gSettings.PatchVBiosBytesCount;
            } else {
              // error - release mem
              if (VBiosPatch->Find != NULL) {
                FreePool (VBiosPatch->Find);
                VBiosPatch->Find = NULL;
              }

              if (VBiosPatch->Replace != NULL) {
                FreePool (VBiosPatch->Replace);
                VBiosPatch->Replace = NULL;
              }
            }
          }

          if (gSettings.PatchVBiosBytesCount == 0) {
            FreePool (gSettings.PatchVBiosBytes);
            gSettings.PatchVBiosBytes = NULL;
          }
        }
      }

      GetEDIDSettings(DictPointer);
    }

    DictPointer = GetProperty (Dict, "DisableDrivers");
    if (DictPointer != NULL) {
      INTN   i, Count = GetTagCount (DictPointer);
      if (Count > 0) {
        gSettings.BlackListCount = 0;
        gSettings.BlackList = (__typeof__(gSettings.BlackList))AllocateZeroPool (Count * sizeof(CHAR16 *));

        for (i = 0; i < Count; i++) {
          if (!EFI_ERROR (GetElement (DictPointer, i, &Prop)) &&
              Prop != NULL && (Prop->type == kTagTypeString)) {
            gSettings.BlackList[gSettings.BlackListCount++] = PoolPrint (L"%a", Prop->string);
          }
        }
      }
    }

    DictPointer            = GetProperty (Dict,        "Devices");
    if (DictPointer != NULL) {
      Dict2                = GetProperty (DictPointer, "Audio");
      if (Dict2 != NULL) {
        // HDA
        Prop               = GetProperty (Dict2,       "ResetHDA");
        gSettings.ResetHDA = IsPropertyTrue (Prop);
      }
    }

    DictPointer = GetProperty (Dict, "RtVariables");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "ROM");
      if (Prop != NULL) {
        if ((AsciiStriCmp (Prop->string, "UseMacAddr0") == 0) ||
            (AsciiStriCmp (Prop->string, "UseMacAddr1") == 0)) {
          GetLegacyLanAddress = TRUE;
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

  DirIterOpen(SelfRootDir, OEMPath, &DirIter);
  DbgHeader("Found config plists");
  while (DirIterNext(&DirIter, 2, L"config*.plist", &DirEntry)) {
    CHAR16  FullName[256];
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }

    UnicodeSPrint(FullName, 512, L"%s\\%s", OEMPath, DirEntry->FileName);
    if (FileExists(SelfRootDir, FullName)) {
      if (StriCmp(DirEntry->FileName, L"config.plist") == 0) {
        OldChosenConfig = ConfigsNum;
      }
      NameLen = StrLen(DirEntry->FileName) - 6; //without ".plist"
      ConfigsList[ConfigsNum] = (CHAR16*)AllocateCopyPool (NameLen * sizeof(CHAR16) + 2, DirEntry->FileName);
      ConfigsList[ConfigsNum++][NameLen] = L'\0';
      DBG("- %s\n", DirEntry->FileName);
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
  CHAR16*     AcpiPath = PoolPrint(L"%s\\ACPI\\patched", OEMPath);

  DsdtsNum = 0;
  OldChosenDsdt = 0xFFFF;

  DirIterOpen(SelfRootDir, AcpiPath, &DirIter);
  DbgHeader("Found DSDT tables");
  while (DirIterNext(&DirIter, 2, L"DSDT*.aml", &DirEntry)) {
    CHAR16  FullName[256];
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }

    UnicodeSPrint(FullName, 512, L"%s\\%s", AcpiPath, DirEntry->FileName);
    if (FileExists(SelfRootDir, FullName)) {
      if (StriCmp(DirEntry->FileName, gSettings.DsdtName) == 0) {
        OldChosenDsdt = DsdtsNum;
      }
      NameLen = StrLen(DirEntry->FileName); //with ".aml"
      DsdtsList[DsdtsNum] = (CHAR16*)AllocateCopyPool (NameLen * sizeof(CHAR16) + 2, DirEntry->FileName);
      DsdtsList[DsdtsNum++][NameLen] = L'\0';
      DBG("- %s\n", DirEntry->FileName);
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
  INTN i, Count = gSettings.DisabledAMLCount;
  CHAR16*     AcpiPath = PoolPrint(L"%s\\ACPI\\patched", OEMPath);

  ACPIPatchedAML = NULL;

  DirIterOpen(SelfRootDir, AcpiPath, &DirIter);

  while (DirIterNext(&DirIter, 2, L"*.aml", &DirEntry)) {
    CHAR16  FullName[256];
    if (DirEntry->FileName[0] == L'.') {
      continue;
    }
    if (StriStr(DirEntry->FileName, L"DSDT")) {
      continue;
    }

    UnicodeSPrint(FullName, 512, L"%s\\%s", AcpiPath, DirEntry->FileName);
    if (FileExists(SelfRootDir, FullName)) {
      BOOLEAN ACPIDisabled = FALSE;
      ACPIPatchedAMLTmp = (__typeof__(ACPIPatchedAMLTmp))AllocateZeroPool (sizeof(ACPI_PATCHED_AML));
      ACPIPatchedAMLTmp->FileName = PoolPrint(L"%s", DirEntry->FileName);

      for (i = 0; i < Count; i++) {
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
  FreePool(AcpiPath);
}

CHAR16* GetBundleVersion(CHAR16 *FullName)
{
  EFI_STATUS      Status;
  CHAR16*         CFBundleVersion = NULL;
  CHAR16*         InfoPlistPath;
  CHAR8*          InfoPlistPtr = NULL;
  TagPtr          InfoPlistDict = NULL;
  TagPtr          Prop = NULL;
  UINTN           Size;

  InfoPlistPath = PoolPrint(L"%s\\%s", FullName, L"Contents\\Info.plist");
  Status = egLoadFile (SelfRootDir, InfoPlistPath, (UINT8**)&InfoPlistPtr, &Size);
  if (EFI_ERROR(Status)) {
    FreePool(InfoPlistPath);
    InfoPlistPath = PoolPrint(L"%s", FullName, L"Info.plist"); //special kexts like IOGraphics
    Status = egLoadFile (SelfRootDir, InfoPlistPath, (UINT8**)&InfoPlistPtr, &Size);
  }
  if(!EFI_ERROR(Status)) {
    Status = ParseXML ((const CHAR8*)InfoPlistPtr, &InfoPlistDict, (UINT32)Size);
    if(!EFI_ERROR(Status)) {
      Prop = GetProperty(InfoPlistDict, "CFBundleVersion");
      if (Prop != NULL && Prop->string != NULL) {
        CFBundleVersion = PoolPrint (L"%a", Prop->string);
      }
    }
  }
  if (InfoPlistPtr) {
    FreePool(InfoPlistPtr);
  }
  FreePool(InfoPlistPath);
  return CFBundleVersion;
}

VOID GetListOfInjectKext(CHAR16 *KextDirNameUnderOEMPath)
{

  REFIT_DIR_ITER  DirIter;
  EFI_FILE_INFO*  DirEntry;
  SIDELOAD_KEXT*  mKext;
  SIDELOAD_KEXT*  mPlugInKext;
  CHAR16*         FullName;
  CHAR16*         FullPath = PoolPrint(L"%s\\KEXTS\\%s", OEMPath, KextDirNameUnderOEMPath);
  REFIT_DIR_ITER  PlugInsIter;
  EFI_FILE_INFO   *PlugInEntry;
  CHAR16*         PlugInsPath;
  CHAR16*         PlugInsName;
  BOOLEAN         Blocked = FALSE;
  if (StrCmp(KextDirNameUnderOEMPath, L"Off") == 0) {
    Blocked = TRUE;
  }

  DirIterOpen(SelfRootDir, FullPath, &DirIter);
  while (DirIterNext(&DirIter, 1, L"*.kext", &DirEntry)) {
    if (DirEntry->FileName[0] == L'.' || StrStr(DirEntry->FileName, L".kext") == NULL) {
      continue;
    }
    /*
     <key>CFBundleVersion</key>
     <string>8.8.8</string>
     */
    FullName = PoolPrint(L"%s\\%s", FullPath, DirEntry->FileName);

    mKext = (__typeof__(mKext))AllocateZeroPool (sizeof(SIDELOAD_KEXT));
    mKext->FileName = PoolPrint(L"%s", DirEntry->FileName);
    mKext->MenuItem.BValue = Blocked;
    mKext->KextDirNameUnderOEMPath = PoolPrint(L"%s", KextDirNameUnderOEMPath);
    mKext->Next = InjectKextList;
    mKext->Version = GetBundleVersion(FullName);
    InjectKextList = mKext;
    //   DBG("Added mKext=%s, MatchOS=%s\n", mKext->FileName, mKext->MatchOS);

    // Obtain PlugInList
    // Iterate over PlugIns directory
    PlugInsPath = PoolPrint(L"%s\\%s", FullName, L"Contents\\PlugIns");

    DirIterOpen(SelfRootDir, PlugInsPath, &PlugInsIter);
    while (DirIterNext(&PlugInsIter, 1, L"*.kext", &PlugInEntry)) {
      if (PlugInEntry->FileName[0] == L'.' || StrStr(PlugInEntry->FileName, L".kext") == NULL) {
        continue;
      }
      PlugInsName = PoolPrint(L"%s\\%s", PlugInsPath, PlugInEntry->FileName);
      mPlugInKext = (__typeof__(mPlugInKext))AllocateZeroPool(sizeof(SIDELOAD_KEXT));
      mPlugInKext->FileName = PoolPrint(L"%s", PlugInEntry->FileName);
      mPlugInKext->MenuItem.BValue = Blocked;
      mPlugInKext->KextDirNameUnderOEMPath = PoolPrint(L"%s", KextDirNameUnderOEMPath);
      mPlugInKext->Next    = mKext->PlugInList;
      mPlugInKext->Version = GetBundleVersion(PlugInsName);
      mKext->PlugInList    = mPlugInKext;
      //      DBG("---| added plugin=%s, MatchOS=%s\n", mPlugInKext->FileName, mPlugInKext->MatchOS);
      FreePool(PlugInsName);
    }
    FreePool(PlugInsPath);
    FreePool(FullName);
    DirIterClose(&PlugInsIter);
  }
  DirIterClose(&DirIter);
  FreePool(FullPath);
}

VOID InitKextList()
{
  REFIT_DIR_ITER  KextsIter;
  EFI_FILE_INFO   *FolderEntry = NULL;
  CHAR16          *KextsPath;

  if (InjectKextList) {
    return;  //don't scan again
  }
  KextsPath = PoolPrint(L"%s\\kexts", OEMPath);

  // Iterate over kexts directory

  DirIterOpen(SelfRootDir, KextsPath, &KextsIter);
  while (DirIterNext(&KextsIter, 1, L"*", &FolderEntry)) {
    if (FolderEntry->FileName[0] == L'.') {
      continue;
    }
    GetListOfInjectKext(FolderEntry->FileName);
  }
  DirIterClose(&KextsIter);
  FreePool(KextsPath);
}

#define CONFIG_THEME_FILENAME L"theme.plist"
#define CONFIG_THEME_SVG L"theme.svg"

VOID
GetListOfThemes ()
{
  EFI_STATUS     Status          = EFI_NOT_FOUND;
  REFIT_DIR_ITER DirIter;
  EFI_FILE_INFO  *DirEntry;
  CHAR16         *ThemeTestPath;
  EFI_FILE       *ThemeTestDir   = NULL;
  CHAR8          *ThemePtr       = NULL;
  UINTN          Size = 0;

  DbgHeader("GetListOfThemes");

  ThemesNum = 0;
  DirIterOpen (SelfRootDir, L"\\EFI\\CLOVER\\themes", &DirIter);
  while (DirIterNext(&DirIter, 1, L"*", &DirEntry)) {
    if (DirEntry->FileName[0] == '.') {
      //DBG("Skip theme: %s\n", DirEntry->FileName);
      continue;
    }
    //DBG ("Found theme directory: %s", DirEntry->FileName);
    DBG ("- [%02d]: %s", ThemesNum, DirEntry->FileName);
    ThemeTestPath = PoolPrint (L"EFI\\CLOVER\\themes\\%s", DirEntry->FileName);
    if (ThemeTestPath != NULL) {
      Status = SelfRootDir->Open (SelfRootDir, &ThemeTestDir, ThemeTestPath, EFI_FILE_MODE_READ, 0);
      if (!EFI_ERROR (Status)) {
        Status = egLoadFile (ThemeTestDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
        if (EFI_ERROR (Status) || (ThemePtr == NULL) || (Size == 0)) {
          Status = egLoadFile (ThemeTestDir, CONFIG_THEME_SVG, (UINT8**)&ThemePtr, &Size);
          if (EFI_ERROR (Status)) {
            Status = EFI_NOT_FOUND;
            DBG (" - bad theme because %s nor %s can't be load", CONFIG_THEME_FILENAME, CONFIG_THEME_SVG);
          }
        }
        if (!EFI_ERROR (Status)) {
          //we found a theme
          if ((StriCmp(DirEntry->FileName, L"embedded") == 0) ||
              (StriCmp(DirEntry->FileName, L"random") == 0)) {
            ThemePtr = NULL;
          } else {
            ThemesList[ThemesNum++] = (CHAR16*)AllocateCopyPool (StrSize (DirEntry->FileName), DirEntry->FileName);
          }
        }
      }
      FreePool (ThemeTestPath);
    }
    DBG ("\n");
    if (ThemePtr) {
      FreePool(ThemePtr);
    }
  }
  DirIterClose (&DirIter);
}

STATIC
EFI_STATUS
GetThemeTagSettings (
                     TagPtr DictPointer
                     )
{
  TagPtr Dict, Dict2, Dict3;

  //fill default to have an ability change theme
  GlobalConfig.BackgroundScale = imCrop;

  if (GlobalConfig.BackgroundName != NULL) {
    FreePool (GlobalConfig.BackgroundName);
    GlobalConfig.BackgroundName = NULL;
  }

  GlobalConfig.BackgroundSharp = 0;
  GlobalConfig.BackgroundDark = 0;

  if (GlobalConfig.BannerFileName != NULL) {
    FreePool (GlobalConfig.BannerFileName);
    GlobalConfig.BannerFileName = NULL;
  }

  GlobalConfig.HideBadges               = 0;
  GlobalConfig.BadgeOffsetX             = 0xFFFF;
  GlobalConfig.BadgeOffsetY             = 0xFFFF;
  GlobalConfig.BadgeScale               = 8; //default
  GlobalConfig.ThemeDesignWidth         = 0xFFFF;
  GlobalConfig.ThemeDesignHeight        = 0xFFFF;
  GlobalConfig.BannerEdgeHorizontal     = SCREEN_EDGE_LEFT;
  GlobalConfig.BannerEdgeVertical       = SCREEN_EDGE_TOP;
  GlobalConfig.BannerPosX               = 0xFFFF;
  GlobalConfig.BannerPosY               = 0xFFFF;
  GlobalConfig.BannerNudgeX             = 0;
  GlobalConfig.BannerNudgeY             = 0;
  GlobalConfig.VerticalLayout           = FALSE;
  GlobalConfig.MainEntriesSize          = 128;
  GlobalConfig.TileXSpace               = 8;
  GlobalConfig.TileYSpace               = 24;
  row0TileSize                          = 144;
  row1TileSize                          = 64;
  LayoutBannerOffset                    = 64; //default value if not set
  LayoutButtonOffset                    = 0; //default value if not set
  LayoutTextOffset                      = 0; //default value if not set
  LayoutAnimMoveForMenuX                = 0; //default value if not set
  GlobalConfig.HideUIFlags              = 0;
  GlobalConfig.SelectionColor           = 0x80808080;

  if (GlobalConfig.SelectionSmallFileName != NULL) {
    FreePool (GlobalConfig.SelectionSmallFileName);
    GlobalConfig.SelectionSmallFileName = NULL;
  }

  if (GlobalConfig.SelectionBigFileName != NULL) {
    FreePool (GlobalConfig.SelectionBigFileName);
    GlobalConfig.SelectionBigFileName   = NULL;
  }

  if (GlobalConfig.SelectionIndicatorName != NULL) {
    FreePool (GlobalConfig.SelectionIndicatorName);
    GlobalConfig.SelectionIndicatorName = NULL;
  }

  GlobalConfig.SelectionOnTop           = FALSE;
  GlobalConfig.BootCampStyle            = FALSE;
  ScrollWidth                           = 16;
  ScrollButtonsHeight                   = 20;
  ScrollBarDecorationsHeight            = 5;
  ScrollScrollDecorationsHeight         = 7;
  GlobalConfig.Font                     = FONT_LOAD;
  if (GlobalConfig.FontFileName != NULL) {
    FreePool (GlobalConfig.FontFileName);
    GlobalConfig.FontFileName          = NULL;
  }
  GlobalConfig.CharWidth               = 9;
  //  GlobalConfig.PruneScrollRows         = 0;
  GuiAnime = NULL;

  if (BigBack != NULL) {
    egFreeImage (BigBack);
    BigBack = NULL;
  }

  if (BackgroundImage != NULL) {
    egFreeImage (BackgroundImage);
    BackgroundImage = NULL;
  }

  if (FontImage != NULL) {
    egFreeImage (FontImage);
    FontImage = NULL;
  }
  FreeScrollBar();

  if (IconFormat != NULL) {
    FreePool (IconFormat);
    IconFormat = NULL;
  }

  GlobalConfig.IconFormat = ICON_FORMAT_DEF;

  // if NULL parameter, quit after setting default values, this is embedded theme
  if (DictPointer == NULL) {
    return EFI_SUCCESS;
  }

  Dict    = GetProperty (DictPointer, "BootCampStyle");
  GlobalConfig.BootCampStyle = IsPropertyTrue(Dict);

  Dict    = GetProperty (DictPointer, "Background");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Type");
    if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
      if ((Dict2->string[0] == 'S') || (Dict2->string[0] == 's')) {
        GlobalConfig.BackgroundScale = imScale;
      } else if ((Dict2->string[0] == 'T') || (Dict2->string[0] == 't')) {
        GlobalConfig.BackgroundScale = imTile;
      }
    }
    //  }

    Dict2 = GetProperty (Dict, "Path");
    if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
      GlobalConfig.BackgroundName = PoolPrint (L"%a", Dict2->string);
    }

    Dict2 = GetProperty (Dict, "Sharp");
    GlobalConfig.BackgroundSharp  = (INT32)GetPropertyInteger (Dict2, GlobalConfig.BackgroundSharp);

    Dict2 = GetProperty (Dict, "Dark");
    GlobalConfig.BackgroundDark   = IsPropertyTrue(Dict2);
  }

  Dict = GetProperty (DictPointer, "Banner");
  if (Dict != NULL) {
    // retain for legacy themes.
    if ((Dict->type == kTagTypeString) && Dict->string) {
      GlobalConfig.BannerFileName = PoolPrint (L"%a", Dict->string);
    } else {
      // for new placement settings
      Dict2 = GetProperty (Dict, "Path");
      if (Dict2 != NULL) {
        if ((Dict2->type == kTagTypeString) && Dict2->string) {
          GlobalConfig.BannerFileName = PoolPrint (L"%a", Dict2->string);
        }
      }

      Dict2 = GetProperty (Dict, "ScreenEdgeX");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        if (AsciiStrCmp (Dict2->string, "left") == 0) {
          GlobalConfig.BannerEdgeHorizontal = SCREEN_EDGE_LEFT;
        } else if (AsciiStrCmp (Dict2->string, "right") == 0) {
          GlobalConfig.BannerEdgeHorizontal = SCREEN_EDGE_RIGHT;
        }
      }

      Dict2 = GetProperty (Dict, "ScreenEdgeY");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        if (AsciiStrCmp (Dict2->string, "top") == 0) {
          GlobalConfig.BannerEdgeVertical = SCREEN_EDGE_TOP;
        } else if (AsciiStrCmp (Dict2->string, "bottom") == 0) {
          GlobalConfig.BannerEdgeVertical = SCREEN_EDGE_BOTTOM;
        }
      }

      Dict2 = GetProperty (Dict, "DistanceFromScreenEdgeX%");
      GlobalConfig.BannerPosX   = (INT32)GetPropertyInteger (Dict2, 0);

      Dict2 = GetProperty (Dict, "DistanceFromScreenEdgeY%");
      GlobalConfig.BannerPosY   = (INT32)GetPropertyInteger (Dict2, 0);

      Dict2 = GetProperty (Dict, "NudgeX");
      GlobalConfig.BannerNudgeX = (INT32)GetPropertyInteger (Dict2, 0);

      Dict2 = GetProperty (Dict, "NudgeY");
      GlobalConfig.BannerNudgeY = (INT32)GetPropertyInteger (Dict2, 0);
    }
  }

  Dict = GetProperty (DictPointer, "Badges");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Swap");
    if (Dict2 != NULL && Dict2->type == kTagTypeTrue) {
      GlobalConfig.HideBadges |= HDBADGES_SWAP;
      DBG ("OS main and drive as badge\n");
    }

    Dict2 = GetProperty (Dict, "Show");
    if (Dict2 != NULL && Dict2->type == kTagTypeTrue) {
      GlobalConfig.HideBadges |= HDBADGES_SHOW;
    }

    Dict2 = GetProperty (Dict, "Inline");
    if (Dict2 != NULL && Dict2->type == kTagTypeTrue) {
      GlobalConfig.HideBadges |= HDBADGES_INLINE;
    }

    // blackosx added X and Y position for badge offset.
    Dict2 = GetProperty (Dict, "OffsetX");
    GlobalConfig.BadgeOffsetX = (INTN)GetPropertyInteger (Dict2, GlobalConfig.BadgeOffsetX);

    Dict2 = GetProperty (Dict, "OffsetY");
    GlobalConfig.BadgeOffsetY = (INTN)GetPropertyInteger (Dict2, GlobalConfig.BadgeOffsetY);

    Dict2 = GetProperty (Dict, "Scale");
    GlobalConfig.BadgeScale = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.BadgeScale);
  }

  Dict = GetProperty (DictPointer, "Origination");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "DesignWidth");
    GlobalConfig.ThemeDesignWidth = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.ThemeDesignWidth);

    Dict2 = GetProperty (Dict, "DesignHeight");
    GlobalConfig.ThemeDesignHeight = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.ThemeDesignHeight);
  }

  Dict = GetProperty (DictPointer, "Layout");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "BannerOffset");
    LayoutBannerOffset = (UINTN)GetPropertyInteger (Dict2, LayoutBannerOffset);

    Dict2 = GetProperty (Dict, "ButtonOffset");
    LayoutButtonOffset = (UINTN)GetPropertyInteger (Dict2, LayoutButtonOffset);

    Dict2 = GetProperty (Dict, "TextOffset");
    LayoutTextOffset = (UINTN)GetPropertyInteger (Dict2, LayoutTextOffset);

    Dict2 = GetProperty (Dict, "AnimAdjustForMenuX");
    LayoutAnimMoveForMenuX = (UINTN)GetPropertyInteger (Dict2, LayoutAnimMoveForMenuX);

    Dict2 = GetProperty (Dict, "Vertical");
    if (Dict2 && Dict2->type == kTagTypeTrue) {
      GlobalConfig.VerticalLayout = TRUE;
    }

    // GlobalConfig.MainEntriesSize
    Dict2 = GetProperty (Dict, "MainEntriesSize");
    GlobalConfig.MainEntriesSize = (INT32)GetPropertyInteger (Dict2, GlobalConfig.MainEntriesSize);

    Dict2 = GetProperty (Dict, "TileXSpace");
    GlobalConfig.TileXSpace = (INT32)GetPropertyInteger (Dict2, GlobalConfig.TileXSpace);

    Dict2 = GetProperty (Dict, "TileYSpace");
    GlobalConfig.TileYSpace = (INT32)GetPropertyInteger (Dict2, GlobalConfig.TileYSpace);

    Dict2 = GetProperty (Dict, "SelectionBigWidth");
    row0TileSize = (INTN)GetPropertyInteger (Dict2, row0TileSize);

    Dict2 = GetProperty (Dict, "SelectionSmallWidth");
    row1TileSize = (INTN)GetPropertyInteger (Dict2, row1TileSize);

  }

  Dict = GetProperty (DictPointer, "Components");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Banner");
    if (Dict2 && Dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_BANNER;
    }

    Dict2 = GetProperty (Dict, "Functions");
    if (Dict2 && Dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_FUNCS;
    }

    Dict2 = GetProperty (Dict, "Tools");
    if (Dict2 && Dict2->type == kTagTypeFalse) {
      GlobalConfig.DisableFlags |= HIDEUI_FLAG_TOOLS;
    }

    Dict2 = GetProperty (Dict, "Label");
    if (Dict2 && Dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_LABEL;
    }

    Dict2 = GetProperty (Dict, "Revision");
    if (Dict2 && Dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_REVISION;
    }

    Dict2 = GetProperty (Dict, "Help");
    if (Dict2 && Dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_HELP;
    }

    Dict2 = GetProperty (Dict, "MenuTitle");
    if (Dict2 && Dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_MENU_TITLE;
    }

    Dict2 = GetProperty (Dict, "MenuTitleImage");
    if (Dict2 && Dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_MENU_TITLE_IMAGE;
    }
  }

  Dict = GetProperty (DictPointer, "Selection");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Color");
    GlobalConfig.SelectionColor = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.SelectionColor);

    Dict2 = GetProperty (Dict, "Small");
    if ( Dict2 && (Dict2->type == kTagTypeString) && Dict2->string) {
      GlobalConfig.SelectionSmallFileName = PoolPrint (L"%a", Dict2->string);
    }

    Dict2 = GetProperty (Dict, "Big");
    if ( Dict2 && (Dict2->type == kTagTypeString) && Dict2->string) {
      GlobalConfig.SelectionBigFileName = PoolPrint (L"%a", Dict2->string);
    }

    Dict2 = GetProperty (Dict, "Indicator");
    if ( Dict2 && (Dict2->type == kTagTypeString) && Dict2->string) {
      GlobalConfig.SelectionIndicatorName = PoolPrint (L"%a", Dict2->string);
    }

    Dict2 = GetProperty (Dict, "OnTop");
    GlobalConfig.SelectionOnTop = IsPropertyTrue (Dict2);

    Dict2 = GetProperty (Dict, "ChangeNonSelectedGrey");
    GlobalConfig.NonSelectedGrey = IsPropertyTrue (Dict2);
  }

  Dict = GetProperty (DictPointer, "Scroll");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Width");
    ScrollWidth = (UINTN)GetPropertyInteger (Dict2, ScrollWidth);

    Dict2 = GetProperty (Dict, "Height");
    ScrollButtonsHeight = (UINTN)GetPropertyInteger (Dict2, ScrollButtonsHeight);

    Dict2 = GetProperty (Dict, "BarHeight");
    ScrollBarDecorationsHeight = (UINTN)GetPropertyInteger (Dict2, ScrollBarDecorationsHeight);

    Dict2 = GetProperty (Dict, "ScrollHeight");
    ScrollScrollDecorationsHeight = (UINTN)GetPropertyInteger (Dict2,ScrollScrollDecorationsHeight);
  }

  Dict = GetProperty (DictPointer, "Font");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Type");
    if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
      if ((Dict2->string[0] == 'A') || (Dict2->string[0] == 'B')) {
        GlobalConfig.Font = FONT_ALFA;
      } else if ((Dict2->string[0] == 'G') || (Dict2->string[0] == 'W')) {
        GlobalConfig.Font = FONT_GRAY;
      } else if ((Dict2->string[0] == 'L') || (Dict2->string[0] == 'l')) {
        GlobalConfig.Font = FONT_LOAD;
      }
    }
    if (GlobalConfig.Font == FONT_LOAD) {
      Dict2 = GetProperty (Dict, "Path");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        GlobalConfig.FontFileName = PoolPrint (L"%a", Dict2->string);
      }
    }

    Dict2 = GetProperty (Dict, "CharWidth");
    GlobalConfig.CharWidth = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.CharWidth);
    if (GlobalConfig.CharWidth & 1) {
      MsgLog("Warning! Character width %d should be even!\n", GlobalConfig.CharWidth);
    }

    Dict2 = GetProperty (Dict, "Proportional");
    GlobalConfig.Proportional = IsPropertyTrue (Dict2);
  }

  Dict = GetProperty (DictPointer, "Anime");
  if (Dict != NULL) {
    INTN   i, Count = GetTagCount (Dict);
    for (i = 0; i < Count; i++) {
      GUI_ANIME *Anime;
      if (EFI_ERROR (GetElement (Dict, i, &Dict3))) {
        continue;
      }

      if (Dict3 == NULL) {
        break;
      }

      Anime = (__typeof__(Anime))AllocateZeroPool (sizeof(GUI_ANIME));
      if (Anime == NULL) {
        break;
      }

      Dict2 = GetProperty (Dict3, "ID");
      Anime->ID = (UINTN)GetPropertyInteger (Dict2, 1); //default=main screen

      Dict2 = GetProperty (Dict3, "Path");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        Anime->Path = PoolPrint (L"%a", Dict2->string);
      }

      Dict2 = GetProperty (Dict3, "Frames");
      Anime->Frames = (UINTN)GetPropertyInteger (Dict2, Anime->Frames);

      Dict2 = GetProperty (Dict3, "FrameTime");
      Anime->FrameTime = (UINTN)GetPropertyInteger (Dict2, Anime->FrameTime);

      Dict2 = GetProperty (Dict3, "ScreenEdgeX");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        if (AsciiStrCmp (Dict2->string, "left") == 0) {
          Anime->ScreenEdgeHorizontal = SCREEN_EDGE_LEFT;
        } else if (AsciiStrCmp (Dict2->string, "right") == 0) {
          Anime->ScreenEdgeHorizontal = SCREEN_EDGE_RIGHT;
        }
      }

      Dict2 = GetProperty (Dict3, "ScreenEdgeY");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        if (AsciiStrCmp (Dict2->string, "top") == 0) {
          Anime->ScreenEdgeVertical = SCREEN_EDGE_TOP;
        } else if (AsciiStrCmp (Dict2->string, "bottom") == 0) {
          Anime->ScreenEdgeVertical = SCREEN_EDGE_BOTTOM;
        }
      }

      //default values are centre

      Dict2 = GetProperty (Dict3, "DistanceFromScreenEdgeX%");
      Anime->FilmX = (INT32)GetPropertyInteger (Dict2, INITVALUE);

      Dict2 = GetProperty (Dict3, "DistanceFromScreenEdgeY%");
      Anime->FilmY = (INT32)GetPropertyInteger (Dict2, INITVALUE);

      Dict2 = GetProperty (Dict3, "NudgeX");
      Anime->NudgeX = (INT32)GetPropertyInteger (Dict2, INITVALUE);

      Dict2 = GetProperty (Dict3, "NudgeY");
      Anime->NudgeY = (INT32)GetPropertyInteger (Dict2, INITVALUE);

      Dict2 = GetProperty (Dict3, "Once");
      Anime->Once = IsPropertyTrue (Dict2);

      // Add the anime to the list
      if ((Anime->ID == 0) || (Anime->Path == NULL)) {
        FreePool (Anime);
      } else if (GuiAnime != NULL) { //second anime or further
        if (GuiAnime->ID == Anime->ID) { //why the same anime here?
          Anime->Next = GuiAnime->Next;
          FreeAnime (GuiAnime); //free double
        } else {
          GUI_ANIME *Ptr = GuiAnime;
          while (Ptr->Next) {
            if (Ptr->Next->ID == Anime->ID) { //delete double from list
              GUI_ANIME *Next = Ptr->Next;
              Ptr->Next = Next->Next;
              FreeAnime (Next);
              break;
            }
            Ptr       = Ptr->Next;
          }
          Anime->Next = GuiAnime;
        }
        GuiAnime      = Anime;
      } else {
        GuiAnime      = Anime; //first anime
      }
    }
  }

  // set file defaults in case they were not set
  Dict = GetProperty (DictPointer, "Icon");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Format");
    if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
      if (AsciiStriCmp (Dict2->string, "ICNS") == 0) {
        GlobalConfig.IconFormat = ICON_FORMAT_ICNS;
        IconFormat = PoolPrint (L"%s", L"icns");
      } else if (AsciiStriCmp (Dict2->string, "PNG") == 0) {
        GlobalConfig.IconFormat = ICON_FORMAT_PNG;
        IconFormat = PoolPrint (L"%s", L"png");
      } else if (AsciiStriCmp (Dict2->string, "BMP") == 0) {
        GlobalConfig.IconFormat = ICON_FORMAT_BMP;
        IconFormat = PoolPrint (L"%s", L"bmp");
      }/* else {
        GlobalConfig.IconFormat = ICON_FORMAT_DEF;
        }*/
    }
  }

  if (GlobalConfig.BackgroundName == NULL) {
    GlobalConfig.BackgroundName = GetIconsExt(L"background", L"png");
  }
  if (GlobalConfig.BannerFileName == NULL) {
    GlobalConfig.BannerFileName = GetIconsExt(L"logo", L"png");
  }
  if (GlobalConfig.SelectionSmallFileName == NULL) {
    GlobalConfig.SelectionSmallFileName = GetIconsExt(L"selection_small", L"png");
  }
  if (GlobalConfig.SelectionBigFileName == NULL) {
    GlobalConfig.SelectionBigFileName = GetIconsExt(L"selection_big", L"png");
  }
  if (GlobalConfig.SelectionIndicatorName == NULL) {
    GlobalConfig.SelectionIndicatorName = GetIconsExt(L"selection_indicator", L"png");
  }
  if (GlobalConfig.FontFileName == NULL) {
    GlobalConfig.FontFileName = GetIconsExt(L"font", L"png");
  }

  return EFI_SUCCESS;
}

TagPtr
LoadTheme (CHAR16 *TestTheme)
{
  EFI_STATUS Status    = EFI_UNSUPPORTED;
  TagPtr     ThemeDict = NULL;
  CHAR8      *ThemePtr = NULL;
  UINTN      Size      = 0;

  if (TestTheme != NULL) {
    if (ThemePath != NULL) {
      FreePool (ThemePath);
    }
    if (UGAHeight > HEIGHT_2K) {
      ThemePath = PoolPrint(L"EFI\\CLOVER\\themes\\%s@2x", TestTheme);
    } else {
      ThemePath = PoolPrint(L"EFI\\CLOVER\\themes\\%s", TestTheme);
    }
    if (ThemePath != NULL) {
      if (ThemeDir != NULL) {
        ThemeDir->Close (ThemeDir);
        ThemeDir = NULL;
      }
      Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath, EFI_FILE_MODE_READ, 0);
      if (EFI_ERROR (Status)) {
        FreePool (ThemePath);
        ThemePath = PoolPrint(L"EFI\\CLOVER\\themes\\%s", TestTheme);
        Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath, EFI_FILE_MODE_READ, 0);
      }
      if (!EFI_ERROR (Status)) {
        Status = egLoadFile(ThemeDir, CONFIG_THEME_SVG, (UINT8**)&ThemePtr, &Size);
        if (!EFI_ERROR(Status) && (ThemePtr != NULL) && (Size != 0)) {
          Status = ParseSVGTheme((const CHAR8*)ThemePtr, &ThemeDict, 0);
          if (EFI_ERROR(Status)) {
            ThemeDict = NULL;
          }
          if (ThemeDict == NULL) {
            DBG("svg file %s not parsed\n", CONFIG_THEME_SVG);
          } else {
            DBG("Using vector theme '%s' (%s)\n", TestTheme, ThemePath);
          }
        } else {
          Status = egLoadFile(ThemeDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
          if (!EFI_ERROR (Status) && (ThemePtr != NULL) && (Size != 0)) {
            Status = ParseXML((const CHAR8*)ThemePtr, &ThemeDict, 0);
            if (EFI_ERROR (Status)) {
              ThemeDict = NULL;
            }
            if (ThemeDict == NULL) {
              DBG ("xml file %s not parsed\n", CONFIG_THEME_FILENAME);
            } else {
              DBG ("Using theme '%s' (%s)\n", TestTheme, ThemePath);
            }
          }
        }
      }
    }
  }
  if (ThemePtr != NULL) {
    FreePool (ThemePtr);
  }
  return ThemeDict;
}

EFI_STATUS
InitTheme(
          BOOLEAN UseThemeDefinedInNVRam,
          EFI_TIME *Time
          )
{
  EFI_STATUS Status       = EFI_NOT_FOUND;
  UINTN      Size         = 0;
  UINTN      i;
  TagPtr     ThemeDict    = NULL;
  CHAR8      *ChosenTheme = NULL;
  CHAR16     *TestTheme   = NULL;
  UINTN      Rnd;

  DbgHeader("InitTheme");
  GlobalConfig.TypeSVG = FALSE;
  GlobalConfig.BootCampStyle = FALSE;
  GlobalConfig.Scale = 1.0f;

  if (DayLight) {
    DBG("use daylight theme\n");
  } else {
    DBG("use night theme\n");
  }

  for (i = 0; i < 3; i++) {
    textFace[i].valid = FALSE;
  }

  NSVGfont *nextFont, *font = fontsDB;
  while (font) {
    nextFont = font->next;
    nsvg__deleteFont(font);
    font = nextFont;
  }
  fontsDB = NULL;
  if (mainParser) {
    nsvg__deleteParser(mainParser);
    mainParser = NULL;
  }

  row0TileSize = 144;
  row1TileSize = 64;
  if (FontImage != NULL) {
    egFreeImage (FontImage);
    FontImage = NULL;
  }

  Rnd = ((Time != NULL) && (ThemesNum != 0)) ? Time->Second % ThemesNum : 0;

  // Free selection images which are not builtin icons
  for (i = 0; i < 6; i++) {
    if (SelectionImages[i] != NULL) {
      if ((SelectionImages[i] != BuiltinIconTable[BUILTIN_SELECTION_SMALL].Image) &&
          (SelectionImages[i] != BuiltinIconTable[BUILTIN_SELECTION_BIG].Image)) {
        egFreeImage (SelectionImages[i]);
      }
      SelectionImages[i] = NULL;
    }
  }

  // Free banner which is not builtin icon
  if (Banner != NULL) {
    if (Banner != BuiltinIconTable[BUILTIN_ICON_BANNER].Image) {
      egFreeImage (Banner);
    }
    Banner  = NULL;
  }

  //Free buttons images
  for (i = 0; i < 4; i++) {
    if (Buttons[i] != NULL) {
      egFreeImage(Buttons[i]);
      Buttons[i] = NULL;
    }
  }

  // Kill mouse before we invalidate builtin pointer image
 // KillMouse();
  //here we have no access to Mouse

  // Invalidate BuiltinIcons
//    DBG ("Invalidating BuiltinIcons...\n");
  for (i = 0; i < BUILTIN_ICON_COUNT; i++) {
    if (BuiltinIconTable[i].Image != NULL) {
      egFreeImage (BuiltinIconTable[i].Image);
      BuiltinIconTable[i].Image = NULL;
    }
  }

  while (GuiAnime != NULL) {
    GUI_ANIME *NextAnime = GuiAnime->Next;
    FreeAnime (GuiAnime);
    GuiAnime             = NextAnime;
  }

  GetThemeTagSettings(NULL);

  if (ThemesNum > 0 &&
      (!GlobalConfig.Theme || StriCmp(GlobalConfig.Theme, L"embedded") != 0)) {
    // Try special theme first
    if (Time != NULL) {
      if ((Time->Month == 12) && ((Time->Day >= 25) && (Time->Day <= 31))) {
        TestTheme = PoolPrint (L"christmas");
      } else if ((Time->Month == 1) && ((Time->Day >= 1) && (Time->Day <= 3))) {
        TestTheme = PoolPrint (L"newyear");
      }

      if (TestTheme != NULL) {
        ThemeDict = LoadTheme (TestTheme);
        if (ThemeDict != NULL) {
//                  DBG ("special theme %s found and %s parsed\n", TestTheme, CONFIG_THEME_FILENAME);
          if (GlobalConfig.Theme) {
            FreePool (GlobalConfig.Theme);
          }
          GlobalConfig.Theme = TestTheme;
        } else { // special theme not loaded
 //                  DBG ("special theme %s not found, skipping\n", TestTheme, CONFIG_THEME_FILENAME);
          FreePool (TestTheme);
        }
        TestTheme = NULL;
      }
    }
    // Try theme from nvram
    if (ThemeDict == NULL && UseThemeDefinedInNVRam) {
      ChosenTheme = (__typeof__(ChosenTheme))GetNvramVariable(L"Clover.Theme", &gEfiAppleBootGuid, NULL, &Size);
      if (ChosenTheme != NULL) {
        if (AsciiStrCmp (ChosenTheme, "embedded") == 0) {
          goto finish;
        }
        if (AsciiStrCmp (ChosenTheme, "random") == 0) {
          ThemeDict = LoadTheme (ThemesList[Rnd]);
          goto finish;
        }

        TestTheme   = PoolPrint (L"%a", ChosenTheme);
        if (TestTheme != NULL) {
          ThemeDict = LoadTheme (TestTheme);
          if (ThemeDict != NULL) {
            DBG ("theme %a defined in NVRAM found and %s parsed\n", ChosenTheme, CONFIG_THEME_FILENAME);
            if (GlobalConfig.Theme != NULL) {
              FreePool (GlobalConfig.Theme);
            }
            GlobalConfig.Theme = TestTheme;
          } else { // theme from nvram not loaded
            if (GlobalConfig.Theme != NULL) {
              DBG ("theme %a chosen from nvram is absent, using theme defined in config: %s\n", ChosenTheme, GlobalConfig.Theme);
            } else {
              DBG ("theme %a chosen from nvram is absent, get first theme\n", ChosenTheme);
            }
            FreePool (TestTheme);
          }
          TestTheme = NULL;
        }
        FreePool (ChosenTheme);
        ChosenTheme = NULL;
      }
    }
    // Try to get theme from settings
    if (ThemeDict == NULL) {
      if (GlobalConfig.Theme == NULL) {
        if (Time != NULL) {
          DBG ("no default theme, get random theme %s\n", ThemesList[Rnd]);
        } else {
          DBG ("no default theme, get first theme %s\n", ThemesList[0]);
        }
      } else {
        if (StriCmp(GlobalConfig.Theme, L"random") == 0) {
          ThemeDict = LoadTheme (ThemesList[Rnd]);
        } else {
          ThemeDict = LoadTheme (GlobalConfig.Theme);
          if (ThemeDict == NULL) {
            DBG ("GlobalConfig: %s not found, get embedded theme\n", GlobalConfig.Theme);
            FreePool (GlobalConfig.Theme);
            GlobalConfig.Theme = NULL;
          }
        }
      }
    }
  } // ThemesNum>0

finish:
  if (!ThemeDict) {  // No theme could be loaded, use embedded
    DBG (" using embedded theme\n");
    GlobalConfig.Theme = NULL;
    OldChosenTheme = 0xFFFF;
    if (ThemePath != NULL) {
      FreePool (ThemePath);
      ThemePath = NULL;
    }

    if (ThemeDir != NULL) {
      ThemeDir->Close (ThemeDir);
      ThemeDir = NULL;
    }

    GetThemeTagSettings(NULL);
    //fill some fields
    //GlobalConfig.Timeout = -1;
    GlobalConfig.SelectionColor = 0xA0A0A080;
    GlobalConfig.Font = FONT_ALFA; //to be inverted later
    GlobalConfig.CharWidth = 9;
    GlobalConfig.HideBadges = HDBADGES_SHOW;
    GlobalConfig.BadgeScale = 16;
      Status = StartupSoundPlay(ThemeDir, NULL);
  } else { // theme loaded successfully
    // read theme settings
    if (!GlobalConfig.TypeSVG) {
      TagPtr DictPointer = GetProperty(ThemeDict, "Theme");
      if (DictPointer != NULL) {
        Status = GetThemeTagSettings(DictPointer);
        if (EFI_ERROR (Status)) {
          DBG ("Config theme error: %r\n", Status);
        }
      }
    }
    FreeTag(ThemeDict);

    if (!DayLight) {
      Status = StartupSoundPlay(ThemeDir, L"sound_night.wav");
      if (EFI_ERROR(Status)) {
        Status = StartupSoundPlay(ThemeDir, L"sound.wav");
      }
    } else {
      Status = StartupSoundPlay(ThemeDir, L"sound.wav");
    }

  }
  for (i = 0; i < ThemesNum; i++) {
    if (GlobalConfig.Theme && StriCmp(GlobalConfig.Theme, ThemesList[i]) == 0) {
      OldChosenTheme = i;
      break;
    }
  }
  if (ChosenTheme != NULL) {
    FreePool (ChosenTheme);
  }
  PrepareFont();
  return Status;
}

VOID
ParseSMBIOSSettings(
                    TagPtr DictPointer
                    )
{
  CHAR8  *i, *j;
  CHAR8  *Res1 = (__typeof__(Res1))AllocateZeroPool(9);
  CHAR8  *Res2 = (__typeof__(Res2))AllocateZeroPool(11);
  CHAR16 UStr[64];
  TagPtr Prop, Prop1;
  BOOLEAN Default = FALSE;


  Prop = GetProperty (DictPointer, "ProductName");
  if (Prop != NULL) {
    MACHINE_TYPES Model;
    AsciiStrCpyS (gSettings.ProductName, 64, Prop->string);
    // let's fill all other fields based on this ProductName
    // to serve as default
    Model = GetModelFromString (gSettings.ProductName);
    if (Model != MaxMachineType) {
      SetDMISettingsForModel (Model, FALSE);
      Default = TRUE;
    } else {
      //if new model then fill at least as iMac13,2, except custom ProductName
      // something else?
      SetDMISettingsForModel (iMac132, FALSE);
    }
    DBG ("Using ProductName from config: %a\n", gSettings.ProductName);
  } else {
    DBG ("Using ProductName from clover: %a\n", gSettings.ProductName);
  }

  Prop = GetProperty(DictPointer, "SmbiosVersion");
  gSettings.SmbiosVersion = (UINT16)GetPropertyInteger(Prop, 0x204);

  // Check for BiosVersion and BiosReleaseDate by Sherlocks
  Prop = GetProperty (DictPointer, "BiosVersion");
  if (Prop != NULL) {
    i = gSettings.RomVersion;
    j = Prop->string;

    i += AsciiStrLen (i);
    while (*i != '.') {
      i--;
    }

    j += AsciiStrLen (j);
    while (*j != '.') {
      j--;
    }

    if (((i[1] > '0') && (j[1] == '0')) || ((i[1] >= j[1]) && (i[2] > j[2]))) {
      DBG ("Using latest BiosVersion from clover\n");
    } else if ((i[1] == j[1]) && (i[2] == j[2])) {
      if (((i[3] > '0') && (j[3] == '0')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
        DBG ("Using latest BiosVersion from clover\n");
      } else if ((i[3] == j[3]) && (i[4] == j[4])) {
        if (((i[5] > '0') && (j[5] == '0')) || ((i[5] > '1') && (j[5] == '1')) ||
            ((i[5] > '2') && (j[5] == '2')) || ((i[5] >= j[5]) && (i[6] > j[6]))) {
          DBG ("Using latest BiosVersion from clover\n");
        } else if ((i[5] == j[5]) && (i[6] == j[6])) {
          DBG ("Found same BiosVersion in clover and config\n");
        } else {
          AsciiStrCpyS (gSettings.RomVersion, 64, Prop->string);
          DBG ("Using latest BiosVersion from config\n");
        }
      } else {
        AsciiStrCpyS (gSettings.RomVersion, 64, Prop->string);
        DBG ("Using latest BiosVersion from config\n");
      }
    } else {
      AsciiStrCpyS (gSettings.RomVersion, 64, Prop->string);
      DBG ("Using latest BiosVersion from config\n");
    }
  } else {
    DBG ("BiosVersion: not set, Using BiosVersion from clover\n");
  }
  DBG ("BiosVersion: %a\n", gSettings.RomVersion);

  Prop1 = GetProperty (DictPointer, "BiosReleaseDate");
  if (Prop1 != NULL) {
    if (Prop != NULL) {
      i = gSettings.ReleaseDate;
      j = Prop1->string;

      if ((AsciiStrLen(i) == 8) && (AsciiStrLen(j) == 8)) {
        if (((i[6] > '0') && (j[6] == '0')) || ((i[6] >= j[6]) && (i[7] > j[7]))) {
          //DBG ("Found old BiosReleaseDate from config\n");
          //DBG ("Using latest BiosReleaseDate from clover\n");
        } else if ((i[6] == j[6]) && (i[7] == j[7])) {
          if (((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1]))) {
            //DBG ("Found old BiosReleaseDate from config\n");
            //DBG ("Using latest BiosReleaseDate from clover\n");
          } else if ((i[0] == j[0]) && (i[1] == j[1])) {
            if (((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) ||
                ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
              //DBG ("Found old BiosReleaseDate from config\n");
              //DBG ("Using latest BiosReleaseDate from clover\n");
            } else if ((i[3] == j[3]) && (i[4] == j[4])) {
              //DBG ("Found same BiosReleaseDate in clover and config\n");
            } else {
              AsciiStrCpyS (gSettings.ReleaseDate, 64, Prop1->string);
              //DBG ("Using latest BiosReleaseDate from config\n");
            }
          } else {
            AsciiStrCpyS (gSettings.ReleaseDate, 64, Prop1->string);
            //DBG ("Using latest BiosReleaseDate from config\n");
          }
        } else {
          AsciiStrCpyS (gSettings.ReleaseDate, 64, Prop1->string);
          //DBG ("Using latest BiosReleaseDate from config\n");
        }
      } else if ((AsciiStrLen(i) == 8) && (AsciiStrLen(j) == 10)) {
        if (((i[6] > '0') && (j[8] == '0')) || ((i[6] >= j[8]) && (i[7] > j[9]))) {
          //DBG ("Found old BiosReleaseDate from config\n");
          //DBG ("Using latest BiosReleaseDate from clover\n");
        } else if ((i[6] == j[8]) && (i[7] == j[9])) {
          if (((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1]))) {
            //DBG ("Found old BiosReleaseDate from config\n");
            //DBG ("Using latest BiosReleaseDate from clover\n");
          } else if ((i[0] == j[0]) && (i[1] == j[1])) {
            if (((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) ||
                ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
              //DBG ("Found old BiosReleaseDate from config\n");
              //DBG ("Using latest BiosReleaseDate from clover\n");
            } else if ((i[3] == j[3]) && (i[4] == j[4])) {
              //DBG ("Found same BiosReleaseDate in clover and config\n");
            } else {
              AsciiSPrint (Res1, 9, "%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
              AsciiStrCpyS (gSettings.ReleaseDate, 64, Res1);
              //DBG ("Using latest BiosReleaseDate from config\n");
            }
          } else {
            AsciiSPrint (Res1, 9, "%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
            AsciiStrCpyS (gSettings.ReleaseDate, 64, Res1);
            //DBG ("Using latest BiosReleaseDate from config\n");
          }
        } else {
          AsciiSPrint (Res1, 9, "%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
          AsciiStrCpyS (gSettings.ReleaseDate, 64, Res1);
          //DBG ("Using latest BiosReleaseDate from config\n");
        }
      } else if ((AsciiStrLen(i) == 10) && (AsciiStrLen(j) == 10)) {
        if (((i[8] > '0') && (j[8] == '0')) || ((i[8] >= j[8]) && (i[9] > j[9]))) {
          //DBG ("Found old BiosReleaseDate from config\n");
          //DBG ("Using latest BiosReleaseDate from clover\n");
        } else if ((i[8] == j[8]) && (i[9] == j[9])) {
          if (((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1]))) {
            //DBG ("Found old BiosReleaseDate from config\n");
            //DBG ("Using latest BiosReleaseDate from clover\n");
          } else if ((i[0] == j[0]) && (i[1] == j[1])) {
            if (((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) ||
              ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
              //DBG ("Found old BiosReleaseDate from config\n");
              //DBG ("Using latest BiosReleaseDate from clover\n");
            } else if ((i[3] == j[3]) && (i[4] == j[4])) {
              //DBG ("Found same BiosReleaseDate in clover and config\n");
            } else {
              AsciiStrCpyS (gSettings.ReleaseDate, 64, Prop1->string);
              //DBG ("Using latest BiosReleaseDate from config\n");
            }
          } else {
            AsciiStrCpyS (gSettings.ReleaseDate, 64, Prop1->string);
            //DBG ("Using latest BiosReleaseDate from config\n");
          }
        } else {
          AsciiStrCpyS (gSettings.ReleaseDate, 64, Prop1->string);
          //DBG ("Using latest BiosReleaseDate from config\n");
        }
      } else if ((AsciiStrLen(i) == 10) && (AsciiStrLen(j) == 8)) {
        if (((i[8] > '0') && (j[6] == '0')) || ((i[8] >= j[6]) && (i[9] > j[7]))) {
          //DBG ("Found old BiosReleaseDate from config\n");
          //DBG ("Using latest BiosReleaseDate from clover\n");
        } else if ((i[8] == j[6]) && (i[9] == j[7])) {
          if (((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1]))) {
            //DBG ("Found old BiosReleaseDate from config\n");
            //DBG ("Using latest BiosReleaseDate from clover\n");
          } else if ((i[0] == j[0]) && (i[1] == j[1])) {
            if (((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) ||
                ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4]))) {
              //DBG ("Found old BiosReleaseDate from config\n");
              //DBG ("Using latest BiosReleaseDate from clover\n");
            } else if ((i[3] == j[3]) && (i[4] == j[4])) {
              //DBG ("Found same BiosReleaseDate in clover and config\n");
            } else {
              AsciiSPrint (Res2, 11, "%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
              AsciiStrCpyS (gSettings.ReleaseDate, 64, Res2);
              //DBG ("Using latest BiosReleaseDate from config\n");
            }
          } else {
            AsciiSPrint (Res2, 11, "%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
            AsciiStrCpyS (gSettings.ReleaseDate, 64, Res2);
            //DBG ("Using latest BiosReleaseDate from config\n");
          }
        } else {
          AsciiSPrint (Res2, 11, "%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
          AsciiStrCpyS (gSettings.ReleaseDate, 64, Res2);
          //DBG ("Using latest BiosReleaseDate from config\n");
        }
      } else {
        //DBG ("Found unknown date format from config\n");
        if (Prop != NULL) {
          i = gSettings.ReleaseDate;
          j = gSettings.RomVersion;

          j += AsciiStrLen (j);
          while (*j != '.') {
            j--;
          }

          if ((AsciiStrLen(i) == 8)) {
            AsciiSPrint (Res1, 9, "%c%c/%c%c/%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
            AsciiStrCpyS (gSettings.ReleaseDate, 64, Res1);
            //DBG ("Using the date of used BiosVersion\n");
          } else if ((AsciiStrLen(i) == 10)) {
            AsciiSPrint (Res2, 11, "%c%c/%c%c/20%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
            AsciiStrCpyS (gSettings.ReleaseDate, 64, Res2);
            //DBG ("Using the date of used BiosVersion\n");
          }
        } else {
          //DBG ("Using BiosReleaseDate from clover\n");
        }
      }
    } else {
      //DBG ("BiosReleaseDate: set to %a from config, Ignore BiosReleaseDate\n", Prop1->string);
      //DBG ("Using BiosReleaseDate from clover\n");
    }
  } else {
    if (Prop != NULL) {
      i = gSettings.ReleaseDate;
      j = gSettings.RomVersion;

      j += AsciiStrLen (j);
      while (*j != '.') {
        j--;
      }

      if ((AsciiStrLen(i) == 8)) {
        AsciiSPrint (Res1, 9, "%c%c/%c%c/%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
        AsciiStrCpyS (gSettings.ReleaseDate, 64, Res1);
        //DBG ("BiosReleaseDate: not set, Using the date of used BiosVersion\n");
      } else if ((AsciiStrLen(i) == 10)) {
        AsciiSPrint (Res2, 11, "%c%c/%c%c/20%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
        AsciiStrCpyS (gSettings.ReleaseDate, 64, Res2);
        //DBG ("BiosReleaseDate: not set, Using the date of used BiosVersion\n");
      }
    } else {
      //DBG ("BiosReleaseDate: not set, Using BiosReleaseDate from clover\n");
    }
  }
  DBG ("BiosReleaseDate: %a\n", gSettings.ReleaseDate);

  Prop = GetProperty (DictPointer, "EfiVersion");
  if (Prop != NULL) {
    if (AsciiStrVersionToUint64(gSettings.EfiVersion, 4, 5) > AsciiStrVersionToUint64(Prop->string, 4, 5)) {
      DBG ("Using latest EfiVersion from clover: %a\n", gSettings.EfiVersion);
    } else if (AsciiStrVersionToUint64(gSettings.EfiVersion, 4, 5) < AsciiStrVersionToUint64(Prop->string, 4, 5)) {
      AsciiStrCpyS (gSettings.EfiVersion, 64, Prop->string);
      DBG ("Using latest EfiVersion from config: %a\n", gSettings.EfiVersion);
    } else {
      DBG ("Using EfiVersion from clover: %a\n", gSettings.EfiVersion);
    }
  } else if (iStrLen(gSettings.EfiVersion, 64) > 0) {
    DBG ("Using EfiVersion from clover: %a\n", gSettings.EfiVersion);
  }

  Prop = GetProperty (DictPointer, "FirmwareFeatures");
  if (Prop != NULL) {
    gFwFeatures = (UINT32)GetPropertyInteger (Prop, gFwFeatures);
    DBG ("Using FirmwareFeatures from config: 0x%08x\n", gFwFeatures);
  } else {
    DBG ("Using FirmwareFeatures from clover: 0x%08x\n", gFwFeatures);
  }

  Prop = GetProperty (DictPointer, "FirmwareFeaturesMask");
  if (Prop != NULL) {
    gFwFeaturesMask = (UINT32)GetPropertyInteger (Prop, gFwFeaturesMask);
    DBG ("Using FirmwareFeaturesMask from config: 0x%08x\n", gFwFeaturesMask);
  } else {
    DBG ("Using FirmwareFeaturesMask from clover: 0x%08x\n", gFwFeaturesMask);
  }

  Prop = GetProperty (DictPointer, "PlatformFeature");
  if (Prop != NULL) {
    gPlatformFeature = (UINT64)GetPropertyInteger(Prop, (INTN)gPlatformFeature);
  } else {
    if (gPlatformFeature == 0xFFFF) {
      DBG ("PlatformFeature will not set in SMBIOS\n");
    } else {
      DBG ("Using PlatformFeature from clover: 0x%x\n", gPlatformFeature);
    }
  }

  Prop = GetProperty (DictPointer, "BiosVendor");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.VendorName, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "Manufacturer");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.ManufactureName, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "Version");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.VersionNr, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "Family");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.FamilyName, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "SerialNumber");
  if (Prop != NULL) {
    ZeroMem(gSettings.SerialNr, 64);
    AsciiStrCpyS (gSettings.SerialNr, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "SmUUID");
  if (Prop != NULL) {
    if (IsValidGuidAsciiString (Prop->string)) {
      AsciiStrToUnicodeStrS (Prop->string, (CHAR16*)&UStr[0], 64);
      StrToGuidLE ((CHAR16*)&UStr[0], &gSettings.SmUUID);
      gSettings.SmUUIDConfig = TRUE;
    } else {
      DBG ("Error: invalid SmUUID '%a' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->string);
    }
  }

  Prop = GetProperty (DictPointer, "BoardManufacturer");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.BoardManufactureName, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "BoardSerialNumber");
  if (Prop != NULL && AsciiStrLen (Prop->string) > 0) {
    AsciiStrCpyS (gSettings.BoardSerialNumber, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "Board-ID");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.BoardNumber, 64, Prop->string);
    DBG ("Board-ID set from config as %a\n", gSettings.BoardNumber);
  }

  Prop = GetProperty (DictPointer, "BoardVersion");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.BoardVersion, 64, Prop->string);
  } else if (!Default) {
    AsciiStrCpyS (gSettings.BoardVersion, 64, gSettings.ProductName);
  }

  Prop = GetProperty (DictPointer, "BoardType");
  if (Prop != NULL) {
    gSettings.BoardType = (UINT8)GetPropertyInteger (Prop, gSettings.BoardType);
    DBG ("BoardType: 0x%x\n", gSettings.BoardType);
  }

  Prop = GetProperty (DictPointer, "Mobile");
  if (Prop != NULL) {
    if (IsPropertyFalse(Prop))
      gSettings.Mobile = FALSE;
    else if (IsPropertyTrue(Prop))
      gSettings.Mobile = TRUE;
  } else if (!Default) {
    gSettings.Mobile = (AsciiStrStr(gSettings.ProductName, "MacBook") != NULL);
  }

  Prop = GetProperty (DictPointer, "LocationInChassis");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.LocationInChassis, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "ChassisManufacturer");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.ChassisManufacturer, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "ChassisAssetTag");
  if (Prop != NULL) {
    AsciiStrCpyS (gSettings.ChassisAssetTag, 64, Prop->string);
  }

  Prop = GetProperty (DictPointer, "ChassisType");
  if (Prop != NULL) {
    gSettings.ChassisType = (UINT8)GetPropertyInteger (Prop, gSettings.ChassisType);
    DBG ("ChassisType: 0x%x\n", gSettings.ChassisType);
  }

  Prop = GetProperty (DictPointer, "NoRomInfo");
  if (Prop != NULL) {
    gSettings.NoRomInfo = IsPropertyTrue (Prop);
  }
}

EFI_STATUS
GetUserSettings(
                IN  EFI_FILE *RootDir,
                TagPtr CfgDict
                )
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  TagPtr     Dict;
  TagPtr     Dict2;
  TagPtr     Prop;
  TagPtr     Prop2;
  TagPtr     Prop3;
  TagPtr     DictPointer;
  BOOLEAN    IsValidCustomUUID = FALSE;
  //UINTN      i;

  Dict              = CfgDict;
  if (Dict != NULL) {
    //    DBG ("Loading main settings\n");
    DbgHeader ("GetUserSettings");

    // Boot settings.
    // Discussion. Why Arguments is here? It should be SystemParameters property!
    // we will read them again because of change in GUI menu. It is not only EarlySettings
    //
    DictPointer = GetProperty (Dict, "Boot");
    if (DictPointer != NULL) {

      Prop = GetProperty (DictPointer, "Arguments");
      //if (Prop != NULL && (Prop->type == kTagTypeString) && Prop->string != NULL) {
      if ((Prop != NULL) && (Prop->type == kTagTypeString) && (Prop->string != NULL) && (AsciiStrStr(gSettings.BootArgs, Prop->string) == NULL)) {
        AsciiStrnCpyS(gSettings.BootArgs, 256, Prop->string, 255);
        //gBootArgsChanged = TRUE;
        //gBootChanged = TRUE;
      }

      Prop                     = GetProperty (DictPointer, "NeverDoRecovery");
      gSettings.NeverDoRecovery  = IsPropertyTrue (Prop);
    }


    //Graphics

    DictPointer = GetProperty (Dict, "Graphics");
    if (DictPointer != NULL) {
      INTN i;
      Dict2     = GetProperty (DictPointer, "Inject");
      if (Dict2 != NULL) {
        if (IsPropertyTrue (Dict2)) {
          gSettings.GraphicsInjector = TRUE;
          gSettings.InjectIntel      = TRUE;
          gSettings.InjectATI        = TRUE;
          gSettings.InjectNVidia     = TRUE;
        } else if (Dict2->type == kTagTypeDict) {
          Prop = GetProperty (Dict2, "Intel");
          if (Prop != NULL) {
            gSettings.InjectIntel = IsPropertyTrue (Prop);
          }

          Prop = GetProperty (Dict2, "ATI");
          if (Prop != NULL) {
            gSettings.InjectATI = IsPropertyTrue (Prop);
          }

          Prop = GetProperty (Dict2, "NVidia");
          if (Prop != NULL) {
            gSettings.InjectNVidia = IsPropertyTrue (Prop);
          }
        } else {
          gSettings.GraphicsInjector = FALSE;
          gSettings.InjectIntel      = FALSE;
          gSettings.InjectATI        = FALSE;
          gSettings.InjectNVidia     = FALSE;
        }
      }

      Prop = GetProperty (DictPointer, "RadeonDeInit");
      gSettings.DeInit = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "VRAM");
      gSettings.VRAM = (UINTN)GetPropertyInteger(Prop, (INTN)gSettings.VRAM); //Mb
      //
      Prop = GetProperty (DictPointer, "RefCLK");
      gSettings.RefCLK = (UINT16)GetPropertyInteger (Prop, 0);

      Prop = GetProperty (DictPointer, "LoadVBios");
      gSettings.LoadVBios = IsPropertyTrue (Prop);

      for (i = 0; i < (INTN)NGFX; i++) {
        gGraphics[i].LoadVBios = gSettings.LoadVBios; //default
      }

      Prop = GetProperty (DictPointer, "VideoPorts");
      gSettings.VideoPorts   = (UINT16)GetPropertyInteger (Prop, gSettings.VideoPorts);

      Prop = GetProperty (DictPointer, "BootDisplay");
      gSettings.BootDisplay = (INT8)GetPropertyInteger (Prop, -1);

      Prop = GetProperty (DictPointer, "FBName");
      if (Prop != NULL) {
        AsciiStrToUnicodeStrS(Prop->string, gSettings.FBName, 16);
      }

      Prop = GetProperty (DictPointer, "NVCAP");
      if (Prop != NULL) {
        hex2bin (Prop->string, (UINT8*)&gSettings.NVCAP[0], 20);
        DBG ("Read NVCAP:");

        for (i = 0; i<20; i++) {
          DBG ("%02x", gSettings.NVCAP[i]);
        }

        DBG ("\n");
        //thus confirmed this procedure is working
      }

      Prop = GetProperty (DictPointer, "display-cfg");
      if (Prop != NULL) {
        hex2bin (Prop->string, (UINT8*)&gSettings.Dcfg[0], 8);
      }

      Prop = GetProperty (DictPointer, "DualLink");
      gSettings.DualLink = (UINT32)GetPropertyInteger (Prop, gSettings.DualLink);

      //InjectEDID - already done in earlysettings
      //No! Take again
      GetEDIDSettings(DictPointer);

      // ErmaC: NvidiaGeneric
      Prop = GetProperty (DictPointer, "NvidiaGeneric");
      gSettings.NvidiaGeneric = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "NvidiaNoEFI");
      gSettings.NvidiaNoEFI = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "NvidiaSingle");
      gSettings.NvidiaSingle = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "ig-platform-id");
      gSettings.IgPlatform = (UINT32)GetPropertyInteger (Prop, gSettings.IgPlatform);

      Prop = GetProperty (DictPointer, "snb-platform-id");
      gSettings.IgPlatform = (UINT32)GetPropertyInteger (Prop, gSettings.IgPlatform);

      FillCardList(DictPointer); //#@ Getcardslist
    }

    DictPointer = GetProperty (Dict, "Devices");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "Inject");
      gSettings.StringInjector = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "SetIntelBacklight");
      gSettings.IntelBacklight = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "SetIntelMaxBacklight");
      gSettings.IntelMaxBacklight = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "IntelMaxValue");
      gSettings.IntelMaxValue = (UINT16)GetPropertyInteger (Prop, gSettings.IntelMaxValue);

      Prop = GetProperty (DictPointer, "Properties");
      if (Prop != NULL) {
        if (Prop->type == kTagTypeString) {

          EFI_PHYSICAL_ADDRESS  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
          UINTN strlength  = AsciiStrLen(Prop->string);
          cDeviceProperties = (__typeof__(cDeviceProperties))AllocateZeroPool(strlength + 1);
          AsciiStrCpyS(cDeviceProperties, strlength + 1, Prop->string);
          //-------
          Status = gBS->AllocatePages (
                                       AllocateMaxAddress,
                                       EfiACPIReclaimMemory,
                                       EFI_SIZE_TO_PAGES (strlength) + 1,
                                       &BufferPtr
                                       );

          if (!EFI_ERROR(Status)) {
            cProperties = (UINT8*)(UINTN)BufferPtr;
            cPropSize   = (UINT32)(strlength >> 1);
            cPropSize = hex2bin(cDeviceProperties, cProperties, cPropSize);
            DBG("Injected EFIString of length %d\n", cPropSize);
          }
          //---------
        }
        else if (Prop->type == kTagTypeDict) {
          //analyze dict-array
          INTN   i, Count = GetTagCount(Prop);
          gSettings.AddProperties = (__typeof__(gSettings.AddProperties))AllocateZeroPool(Count * sizeof(DEV_PROPERTY));
          DEV_PROPERTY *DevPropDevice;
          DEV_PROPERTY *DevProps;
          DEV_PROPERTY **Child;

          if (Count > 0) {
            DBG("Add %d devices (kTagTypeDict):\n", Count);

            for (i = 0; i < Count; i++) {
              Prop2 = NULL;
              EFI_DEVICE_PATH_PROTOCOL* DevicePath = NULL;
              if (!EFI_ERROR(GetElement(Prop, i, &Prop2))) {  //take a <key> with DevicePath
                if ((Prop2 != NULL) && (Prop2->type == kTagTypeKey)) {
                  CHAR16* DevicePathStr = PoolPrint(L"%a", Prop2->string);
                  //         DBG("Device: %s\n", DevicePathStr);

                  // when key in Devices/Properties is one of the strings "PrimaryGPU" / "SecondaryGPU", use device path of first / second gpu accordingly
                  if (StriCmp(DevicePathStr, L"PrimaryGPU") == 0) {
                    DevicePath = DevicePathFromHandle(gGraphics[0].Handle); // first gpu
                  } else if (StriCmp(DevicePathStr, L"SecondaryGPU") == 0 && NGFX > 1) {
                    DevicePath = DevicePathFromHandle(gGraphics[1].Handle); // second gpu
                  } else {
                    DevicePath = ConvertTextToDevicePath(DevicePathStr); //TODO
                  }
                  FreePool(DevicePathStr);
                  if (DevicePath == NULL) {
                    continue;
                  }
                }
                else continue;
                //Create Device node
                DevPropDevice = gSettings.ArbProperties;
                gSettings.ArbProperties = (__typeof__(gSettings.ArbProperties))AllocateZeroPool(sizeof(DEV_PROPERTY));
                gSettings.ArbProperties->Next = DevPropDevice; //next device
                gSettings.ArbProperties->Child = NULL;
                gSettings.ArbProperties->Device = 0; //to differ from arbitrary
                gSettings.ArbProperties->DevicePath = DevicePath; //this is pointer
                gSettings.ArbProperties->Label = (__typeof__(gSettings.ArbProperties->Label))AllocateCopyPool(AsciiStrSize(Prop2->string), Prop2->string);
                Child = &(gSettings.ArbProperties->Child);

                Prop2 = Prop2->tag; //take a <dict> for this device
                if ((Prop2 != NULL) && (Prop2->type == kTagTypeDict)) {
                  INTN PropCount = 0;
                  PropCount = GetTagCount(Prop2);  //properties count for this device
                  //         DBG("Add %d properties:\n", PropCount);
                  for (INTN j = 0; j < PropCount; j++) {
                    Prop3 = NULL;
                    DevProps = *Child;
                    *Child = (__typeof_am__(*Child))AllocateZeroPool(sizeof(**Child));
                  //  *Child = new (__typeof_am__(**Child))();
                    (*Child)->Next = DevProps;

                    if (EFI_ERROR(GetElement(Prop2, j, &Prop3))) {  // Prop3 -> <key>
                      continue;
                    }
                    if ((Prop3 != NULL) && (Prop3->type == kTagTypeKey) &&
                        (Prop3->string != NULL)
                        ) {
                      if (Prop3->string[0] != '#') {
                        (*Child)->MenuItem.BValue = TRUE;
                        (*Child)->Key = (__typeof__((*Child)->Key))AllocateCopyPool(AsciiStrSize(Prop3->string), Prop3->string);
                      }
                      else {
                        (*Child)->MenuItem.BValue = FALSE;
                        (*Child)->Key = (__typeof__((*Child)->Key))AllocateCopyPool(AsciiStrSize(Prop3->string) - 1, Prop3->string + 1);
                      }

                      Prop3 = Prop3->tag; //expected value
                      //    DBG("<key>%a\n  <value> type %d\n", (*Child)->Key, Prop3->type);
                      if (Prop3 && (Prop3->type == kTagTypeString) && Prop3->string) {
                        //first suppose it is Ascii string
                        (*Child)->Value = (__typeof__((*Child)->Value))AllocateCopyPool(AsciiStrSize(Prop3->string), Prop3->string);
                        (*Child)->ValueLen = AsciiStrLen(Prop3->string) + 1;
                        (*Child)->ValueType = kTagTypeString;
                      }
                      else if (Prop3 && (Prop3->type == kTagTypeInteger)) {
                        (*Child)->Value = (__typeof__((*Child)->Value))AllocatePool(4);
                        CopyMem((*Child)->Value, &(Prop3->string), 4);
                        (*Child)->ValueLen = 4;
                        (*Child)->ValueType = kTagTypeInteger;
                      }
                      else if (Prop3 && (Prop3->type == kTagTypeTrue)) {
                        (*Child)->Value = (__typeof__((*Child)->Value))AllocateZeroPool(4);
                        (*Child)->Value[0] = TRUE;
                        (*Child)->ValueLen = 1;
                        (*Child)->ValueType = kTagTypeTrue;
                      }
                      else if (Prop3 && (Prop3->type == kTagTypeFalse)) {
                        (*Child)->Value = (__typeof__((*Child)->Value))AllocateZeroPool(4);
                        //(*Child)->Value[0] = FALSE;
                        (*Child)->ValueLen = 1;
                        (*Child)->ValueType = kTagTypeFalse;
                      }
                      else if (Prop3 && (Prop3->type == kTagTypeData)) {
                        UINTN Size = Prop3->dataLen;
                        //     (*Child)->Value = GetDataSetting(Prop3, "Value", &Size);  //TODO
                        UINT8* Data = (__typeof__(Data))AllocateZeroPool(Size);
                        CopyMem(Data, Prop3->data, Size);
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
      }

      Prop  = GetProperty (DictPointer, "LANInjection");
      gSettings.LANInjection = !IsPropertyFalse (Prop);  //default = TRUE

      Prop  = GetProperty (DictPointer, "HDMIInjection");
      gSettings.HDMIInjection = IsPropertyTrue (Prop);

      Prop  = GetProperty (DictPointer, "NoDefaultProperties");
      gSettings.NoDefaultProperties = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "Arbitrary"); //yyyy
      if (Prop != NULL) {
        INTN Index, Count = GetTagCount (Prop);
        DEV_PROPERTY *DevProp;

        if (Count > 0) {
          DBG ("Add %d devices (Arbitrary):\n", Count);
          for (Index = 0; Index < Count; Index++) {
            UINTN DeviceAddr = 0U;
            CHAR8 *Label;
            DBG (" - [%02d]:", Index);
            if (EFI_ERROR (GetElement (Prop, Index, &Prop2))) {
              DBG (" continue\n", Index);
              continue;
            }
            Dict2 = GetProperty (Prop2, "PciAddr");
            if (Dict2 != NULL) {
              INTN Bus, Dev, Func;
              CHAR8 *Str = Dict2->string;

              if (Str[2] != ':') {
                DBG(" wrong PciAddr string: %a\n", Str);
                continue;
              }
              Label = (__typeof__(Label))AllocatePool(64);
              Bus   = hexstrtouint8(Str);
              Dev   = hexstrtouint8(&Str[3]);
              Func  = hexstrtouint8(&Str[6]);
              DeviceAddr = PCIADDR(Bus, Dev, Func);
              AsciiSPrint(Label, 64, "[%02x:%02x.%02x] ", Bus, Dev, Func);
              DBG(" %a", Label);
            } else {
              DBG (" no PciAddr\n");
              continue;
            }

            Dict2 = GetProperty (Prop2, "Comment");
            if (Dict2 != NULL) {
              AsciiStrCatS(Label, 64, Dict2->string);
              DBG (" (%a)", Dict2->string);
            }
            DBG ("\n");
            Dict2 = GetProperty (Prop2, "CustomProperties");
            if (Dict2 != NULL) {
              TagPtr Dict3;
              INTN PropIndex, PropCount = GetTagCount (Dict2);

              for (PropIndex = 0; PropIndex < PropCount; PropIndex++) {
                UINTN Size = 0;
                if (!EFI_ERROR(GetElement(Dict2, PropIndex, &Dict3))) {

                  DevProp = gSettings.ArbProperties;
                  gSettings.ArbProperties = (__typeof__(gSettings.ArbProperties))AllocateZeroPool(sizeof(DEV_PROPERTY));
                  gSettings.ArbProperties->Next = DevProp;

                  gSettings.ArbProperties->Device = (UINT32)DeviceAddr;
                  gSettings.ArbProperties->Label = (__typeof__(gSettings.ArbProperties->Label))AllocateCopyPool(AsciiStrSize(Label), Label);

                  Prop3 = GetProperty (Dict3, "Disabled");
                  gSettings.ArbProperties->MenuItem.BValue = !IsPropertyTrue(Prop3);

                  Prop3 = GetProperty (Dict3, "Key");
                  if (Prop3 && (Prop3->type == kTagTypeString) && Prop3->string) {
                    gSettings.ArbProperties->Key = (__typeof__(gSettings.ArbProperties->Key))AllocateCopyPool(AsciiStrSize(Prop3->string), Prop3->string);
                  }

                  Prop3 = GetProperty (Dict3, "Value");
                  if (Prop3 && (Prop3->type == kTagTypeString) && Prop3->string) {
                    //first suppose it is Ascii string
                    gSettings.ArbProperties->Value = (__typeof__(gSettings.ArbProperties->Value))AllocateCopyPool(AsciiStrSize(Prop3->string), Prop3->string);
                    gSettings.ArbProperties->ValueLen = AsciiStrLen(Prop3->string) + 1;
                    gSettings.ArbProperties->ValueType = kTagTypeString;
                  } else if (Prop3 && (Prop3->type == kTagTypeInteger)) {
                    gSettings.ArbProperties->Value = (__typeof__(gSettings.ArbProperties->Value))AllocatePool(4);
                    CopyMem (gSettings.ArbProperties->Value, &(Prop3->string), 4);
                    gSettings.ArbProperties->ValueLen = 4;
                    gSettings.ArbProperties->ValueType = kTagTypeInteger;
                  } else if (Prop3 && (Prop3->type == kTagTypeTrue)) {
                    gSettings.ArbProperties->Value = (__typeof__(gSettings.ArbProperties->Value))AllocateZeroPool(4);
                    gSettings.ArbProperties->Value[0] = TRUE;
                    gSettings.ArbProperties->ValueLen = 1;
                    gSettings.ArbProperties->ValueType = kTagTypeTrue;
                  } else if (Prop3 && (Prop3->type == kTagTypeFalse)) {
                    gSettings.ArbProperties->Value = (__typeof__(gSettings.ArbProperties->Value))AllocateZeroPool(4);
                    //gSettings.ArbProperties->Value[0] = FALSE;
                    gSettings.ArbProperties->ValueLen = 1;
                    gSettings.ArbProperties->ValueType = kTagTypeFalse;
                  } else {
                    //else  data
                    gSettings.ArbProperties->Value = GetDataSetting (Dict3, "Value", &Size);
                    gSettings.ArbProperties->ValueLen = Size;
                    gSettings.ArbProperties->ValueType = kTagTypeData;
                  }

                  //Special case. In future there must be more such cases
                  if ((AsciiStrStr(gSettings.ArbProperties->Key, "-platform-id") != NULL)) {
                    CopyMem ((CHAR8*)&gSettings.IgPlatform, gSettings.ArbProperties->Value, 4);
                  }
                }
              }   //for() device properties
            }
            FreePool(Label);
          } //for() devices
        }
        //        gSettings.NrAddProperties = 0xFFFE;
      }
      //can use AddProperties with ArbProperties
      Prop = GetProperty (DictPointer, "AddProperties");
      if (Prop != NULL) {
        INTN i, Count = GetTagCount (Prop);
        INTN Index = 0;  //begin from 0 if second enter

        if (Count > 0) {
          DBG ("Add %d properties:\n", Count);
          gSettings.AddProperties = (__typeof__(gSettings.AddProperties))AllocateZeroPool (Count * sizeof(DEV_PROPERTY));

          for (i = 0; i < Count; i++) {
            UINTN Size = 0;
            DBG (" - [%02d]:", i);
            if (EFI_ERROR (GetElement (Prop, i, &Dict2))) {
              DBG (" continue\n");
              continue;
            }

            if (Dict2 == NULL) {
              DBG (" break\n", i);
              break;
            }

            Prop2 = GetProperty (Dict2, "Device");
            if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
              DEV_PROPERTY *Property = &gSettings.AddProperties[Index];

              if (AsciiStriCmp (Prop2->string,        "ATI") == 0) {
                Property->Device = (UINT32)DEV_ATI;
              } else if (AsciiStriCmp (Prop2->string, "NVidia") == 0) {
                Property->Device = (UINT32)DEV_NVIDIA;
              } else if (AsciiStriCmp (Prop2->string, "IntelGFX") == 0) {
                Property->Device = (UINT32)DEV_INTEL;
              } else if (AsciiStriCmp (Prop2->string, "LAN") == 0) {
                Property->Device = (UINT32)DEV_LAN;
              } else if (AsciiStriCmp (Prop2->string, "WIFI") == 0) {
                Property->Device = (UINT32)DEV_WIFI;
              } else if (AsciiStriCmp (Prop2->string, "Firewire") == 0) {
                Property->Device = (UINT32)DEV_FIREWIRE;
              } else if (AsciiStriCmp (Prop2->string, "SATA") == 0) {
                Property->Device = (UINT32)DEV_SATA;
              } else if (AsciiStriCmp (Prop2->string, "IDE") == 0) {
                Property->Device = (UINT32)DEV_IDE;
              } else if (AsciiStriCmp (Prop2->string, "HDA") == 0) {
                Property->Device = (UINT32)DEV_HDA;
              } else if (AsciiStriCmp (Prop2->string, "HDMI") == 0) {
                Property->Device = (UINT32)DEV_HDMI;
              } else if (AsciiStriCmp (Prop2->string, "LPC") == 0) {
                Property->Device = (UINT32)DEV_LPC;
              } else if (AsciiStriCmp (Prop2->string, "SmBUS") == 0) {
                Property->Device = (UINT32)DEV_SMBUS;
              } else if (AsciiStriCmp (Prop2->string, "USB") == 0) {
                Property->Device = (UINT32)DEV_USB;
              } else {
                DBG (" unknown device, ignored\n", i);
                continue;
              }
            }

            DBG (" %a ", Prop2->string);

            Prop2 = GetProperty (Dict2, "Disabled");
            gSettings.AddProperties[Index].MenuItem.BValue = !IsPropertyTrue (Prop2);

            Prop2 = GetProperty (Dict2, "Key");
            if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
              gSettings.AddProperties[Index].Key = (__typeof__(gSettings.AddProperties[Index].Key))AllocateCopyPool (AsciiStrSize (Prop2->string), Prop2->string);
            }

            Prop2 = GetProperty (Dict2, "Value");
            if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
              //first suppose it is Ascii string
              gSettings.AddProperties[Index].Value = (__typeof__(gSettings.AddProperties[Index].Value))AllocateCopyPool (AsciiStrSize (Prop2->string), Prop2->string);
              gSettings.AddProperties[Index].ValueLen = AsciiStrLen (Prop2->string) + 1;
            } else if (Prop2 && (Prop2->type == kTagTypeInteger)) {
              gSettings.AddProperties[Index].Value = (__typeof__(gSettings.AddProperties[Index].Value))AllocatePool (4);
              CopyMem (gSettings.AddProperties[Index].Value, &(Prop2->string), 4);
              gSettings.AddProperties[Index].ValueLen = 4;
            } else {
              //else  data
              gSettings.AddProperties[Index].Value = GetDataSetting (Dict2, "Value", &Size);
              gSettings.AddProperties[Index].ValueLen = Size;
            }

            DBG ("Key: %a, len: %d\n", gSettings.AddProperties[Index].Key, gSettings.AddProperties[Index].ValueLen);

            if (!gSettings.AddProperties[Index].MenuItem.BValue) {
              DBG ("  property disabled at config\n");
            }

            ++Index;
          }

          gSettings.NrAddProperties = Index;
        }
      }
      //end AddProperties

      Prop = GetProperty (DictPointer, "FakeID");
      if (Prop != NULL) {
        Prop2 = GetProperty (Prop, "ATI");
        if (Prop2 && (Prop2->type == kTagTypeString)) {
          gSettings.FakeATI  = (UINT32)AsciiStrHexToUint64(Prop2->string);
        }

        Prop2 = GetProperty (Prop, "NVidia");
        if (Prop2 && (Prop2->type == kTagTypeString)) {
          gSettings.FakeNVidia  = (UINT32)AsciiStrHexToUint64(Prop2->string);
        }

        Prop2 = GetProperty (Prop, "IntelGFX");
        if (Prop2 && (Prop2->type == kTagTypeString)) {
          gSettings.FakeIntel  = (UINT32)AsciiStrHexToUint64(Prop2->string);
        }

        Prop2 = GetProperty (Prop, "LAN");
        if (Prop2 && (Prop2->type == kTagTypeString)) {
          gSettings.FakeLAN  = (UINT32)AsciiStrHexToUint64(Prop2->string);
        }

        Prop2 = GetProperty (Prop, "WIFI");
        if (Prop2 && (Prop2->type == kTagTypeString)) {
          gSettings.FakeWIFI  = (UINT32)AsciiStrHexToUint64(Prop2->string);
        }

        Prop2 = GetProperty (Prop, "SATA");
        if (Prop2 && (Prop2->type == kTagTypeString)) {
          gSettings.FakeSATA  = (UINT32)AsciiStrHexToUint64(Prop2->string);
        }

        Prop2 = GetProperty (Prop, "XHCI");
        if (Prop2 && (Prop2->type == kTagTypeString)) {
          gSettings.FakeXHCI  = (UINT32)AsciiStrHexToUint64(Prop2->string);
        }

        Prop2 = GetProperty (Prop, "IMEI");
        if (Prop2 && (Prop2->type == kTagTypeString)) {
          gSettings.FakeIMEI  = (UINT32)AsciiStrHexToUint64(Prop2->string);
        }
      }

      Prop                   = GetProperty (DictPointer, "UseIntelHDMI");
      gSettings.UseIntelHDMI = IsPropertyTrue (Prop);

      Prop                = GetProperty (DictPointer, "ForceHPET");
      gSettings.ForceHPET = IsPropertyTrue (Prop);

      Prop                = GetProperty (DictPointer, "DisableFunctions");
      if (Prop && (Prop->type == kTagTypeString)) {
        gSettings.DisableFunctions  = (UINT32)AsciiStrHexToUint64(Prop->string);
      }

      Prop                = GetProperty (DictPointer, "AirportBridgeDeviceName");
      if (Prop && (Prop->type == kTagTypeString)) {
        AsciiStrCpyS (gSettings.AirportBridgeDeviceName, sizeof(gSettings.AirportBridgeDeviceName), Prop->string);
      }

      Prop2 = GetProperty (DictPointer, "Audio");
      if (Prop2 != NULL) {
        // HDA
        //       Prop = GetProperty (Prop2, "ResetHDA");
        //       gSettings.ResetHDA = IsPropertyTrue (Prop);
        Prop = GetProperty (Prop2, "Inject");
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
          if (Prop->type == kTagTypeInteger) {
            gSettings.HDALayoutId = (INT32)(UINTN)Prop->string; //must be signed
            gSettings.HDAInjection = (gSettings.HDALayoutId > 0);
          } else if (Prop->type == kTagTypeString){
            if ((Prop->string[0] == 'n') || (Prop->string[0] == 'N')) {
              // if starts with n or N, then no HDA injection
              gSettings.HDAInjection = FALSE;
            } else if ((Prop->string[0] == '0')  &&
                       (Prop->string[1] == 'x' || Prop->string[1] == 'X')) {
              // assume it's a hex layout id
              gSettings.HDALayoutId = (INT32)AsciiStrHexToUintn (Prop->string);
              gSettings.HDAInjection = TRUE;
            } else {
              // assume it's a decimal layout id
              gSettings.HDALayoutId = (INT32)AsciiStrDecimalToUintn (Prop->string);
              gSettings.HDAInjection = TRUE;
            }
          }
        }

        Prop = GetProperty (Prop2, "AFGLowPowerState");
        gSettings.AFGLowPowerState = IsPropertyTrue (Prop);
      }

      Prop2 = GetProperty (DictPointer, "USB");
      if (Prop2 != NULL) {
        // USB
        Prop = GetProperty (Prop2, "Inject");
        gSettings.USBInjection = !IsPropertyFalse (Prop); // enabled by default

        Prop = GetProperty (Prop2, "AddClockID");
        gSettings.InjectClockID = IsPropertyTrue (Prop); // disabled by default
        // enabled by default for CloverEFI
        // disabled for others
        gSettings.USBFixOwnership = gFirmwareClover;
        Prop = GetProperty (Prop2, "FixOwnership");
        if (Prop != NULL) {
          gSettings.USBFixOwnership = IsPropertyTrue (Prop);
        }
        DBG ("USB FixOwnership: %a\n", gSettings.USBFixOwnership?"yes":"no");

        Prop = GetProperty (Prop2, "HighCurrent");
        gSettings.HighCurrent = IsPropertyTrue (Prop);

        Prop = GetProperty (Prop2, "NameEH00");
        gSettings.NameEH00 = IsPropertyTrue (Prop);
      }
    }

    //*** ACPI ***//

    DictPointer = GetProperty (Dict, "ACPI");
    if (DictPointer) {
      Prop = GetProperty (DictPointer, "DropTables");
      if (Prop) {
        INTN   i, Count = GetTagCount (Prop);
        BOOLEAN Dropped;

        if (Count > 0) {
          DBG ("Dropping %d tables:\n", Count);

          for (i = 0; i < Count; i++) {
            UINT32 Signature = 0;
            UINT32 TabLength = 0;
            UINT64 TableId = 0;

            if (EFI_ERROR (GetElement (Prop, i, &Dict2))) {
              DBG (" - [%02d]: Drop table continue\n", i);
              continue;
            }

            if (Dict2 == NULL) {
              DBG (" - [%02d]: Drop table break\n", i);
              break;
            }

            DBG (" - [%02d]: Drop table ", i);
            // Get the table signatures to drop
            Prop2 = GetProperty (Dict2, "Signature");
            if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
              CHAR8  s1 = 0, s2 = 0, s3 = 0, s4 = 0;
              CHAR8 *str = Prop2->string;
              DBG (" signature=\"");
              if (str) {
                if (*str) {
                  s1 = *str++;
                  DBG ("%c", s1);
                }
                if (*str) {
                  s2 = *str++;
                  DBG ("%c", s2);
                }
                if (*str) {
                  s3 = *str++;
                  DBG ("%c", s3);
                }
                if (*str) {
                  s4 = *str++;
                  DBG ("%c", s4);
                }
              }
              Signature = SIGNATURE_32(s1, s2, s3, s4);
              DBG ("\" (%8.8X)", Signature);
            }
            // Get the table ids to drop
            Prop2 = GetProperty (Dict2, "TableId");
            if (Prop2 != NULL) {
              UINTN  IdIndex = 0;
              CHAR8  Id[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
              CHAR8 *Str = Prop2->string;
              DBG (" table-id=\"");
              if (Str) {
                while (*Str && (IdIndex < 8)) {
                  DBG ("%c", *Str);
                  Id[IdIndex++] = *Str++;
                }
              }

              CopyMem (&TableId, (CHAR8*)&Id[0], 8);
              DBG ("\" (%16.16lX)", TableId);
            }
            // Get the table len to drop
            Prop2 = GetProperty (Dict2, "Length");
            if (Prop2 != NULL) {
              TabLength = (UINT32)GetPropertyInteger (Prop2, 0);
              DBG (" length=%d(0x%x)", TabLength);
            }

            DBG ("\n");
            //set to drop
            if (gSettings.ACPIDropTables) {
              ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
              DBG ("         - set table: %08x, %16lx to drop:", Signature, TableId);
              Dropped = FALSE;
              while (DropTable) {
                if (((Signature == DropTable->Signature) &&
                     (!TableId || (DropTable->TableId == TableId)) &&
                     (!TabLength || (DropTable->Length == TabLength))) ||
                    (!Signature && (DropTable->TableId == TableId))) {
                  DropTable->MenuItem.BValue = TRUE;
                  gSettings.DropSSDT         = FALSE; //if one item=true then dropAll=false by default
                  //DBG (" true");
                  Dropped = TRUE;
                }
                DropTable = DropTable->Next;
              }
              DBG (" %a\n", Dropped ? "yes" : "no");
            }
          }
        }
      }

      Dict2 = GetProperty (DictPointer, "DSDT");
      if (Dict2) {
        //gSettings.DsdtName by default is "DSDT.aml", but name "BIOS" will mean autopatch
        Prop = GetProperty (Dict2, "Name");
        if (Prop != NULL) {
          AsciiStrToUnicodeStrS (Prop->string, gSettings.DsdtName, 28);
        }

        Prop = GetProperty (Dict2, "Debug");
        gSettings.DebugDSDT = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "Rtc8Allowed");
        gSettings.Rtc8Allowed = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "PNLF_UID");
        gSettings.PNLF_UID = (UINT8)GetPropertyInteger (Prop, 0x0A);

        Prop = GetProperty (Dict2, "FixMask");
        gSettings.FixDsdt = (UINT32)GetPropertyInteger (Prop, gSettings.FixDsdt);

        Prop = GetProperty (Dict2, "Fixes");
        if (Prop != NULL) {
          UINTN Index;
          //         DBG ("Fixes will override DSDT fix mask %08x!\n", gSettings.FixDsdt);
          if (Prop->type == kTagTypeDict) {
            gSettings.FixDsdt = 0;
            for (Index = 0; Index < sizeof(FixesConfig)/sizeof(FixesConfig[0]); Index++) {
              Prop2 = GetProperty(Prop, FixesConfig[Index].newName);
              if (!Prop2 && FixesConfig[Index].oldName) {
                Prop2 = GetProperty(Prop, FixesConfig[Index].oldName);
              }
              if (IsPropertyTrue(Prop2)) {
                gSettings.FixDsdt |= FixesConfig[Index].bitData;
              }
            }
          }
        }
        DBG (" - final DSDT Fix mask=%08x\n", gSettings.FixDsdt);

        Prop = GetProperty (Dict2, "Patches"); //yyyy
        if (Prop != NULL) {
          INTN   i, Count = GetTagCount (Prop);
          if (Count > 0) {
            gSettings.PatchDsdtNum      = (UINT32)Count;
            gSettings.PatchDsdtFind = (__typeof__(gSettings.PatchDsdtFind))AllocateZeroPool (Count * sizeof(UINT8*));
            gSettings.PatchDsdtReplace = (__typeof__(gSettings.PatchDsdtReplace))AllocateZeroPool (Count * sizeof(UINT8*));
            gSettings.PatchDsdtTgt = (__typeof__(gSettings.PatchDsdtTgt))AllocateZeroPool (Count * sizeof(UINT8*));
            gSettings.LenToFind = (__typeof__(gSettings.LenToFind))AllocateZeroPool (Count * sizeof(UINT32));
            gSettings.LenToReplace = (__typeof__(gSettings.LenToReplace))AllocateZeroPool (Count * sizeof(UINT32));
            gSettings.PatchDsdtLabel = (__typeof__(gSettings.PatchDsdtLabel))AllocateZeroPool (Count * sizeof(UINT8*));
            gSettings.PatchDsdtMenuItem = (__typeof__(gSettings.PatchDsdtMenuItem))AllocateZeroPool (Count * sizeof(INPUT_ITEM));
            DBG ("PatchesDSDT: %d requested\n", Count);

            for (i = 0; i < Count; i++) {
              UINTN Size = 0;
              CHAR8 *DSDTPatchesLabel;
              Status     = GetElement (Prop, i, &Prop2);
              if (EFI_ERROR (Status)) {
                DBG ("error %r getting next element of PatchesDSDT at index %d\n", Status, i);
                continue;
              }

              if (Prop2 == NULL) {
                break;
              }

              DBG(" - [%02d]:", i);
              DSDTPatchesLabel = (__typeof__(DSDTPatchesLabel))AllocateZeroPool(256);

              Prop3 = GetProperty (Prop2, "Comment");
              if (Prop3 != NULL && (Prop3->type == kTagTypeString) && Prop3->string) {
                AsciiSPrint(DSDTPatchesLabel, 255, "%a", Prop3->string);
              } else {
                AsciiSPrint(DSDTPatchesLabel, 255, " (NoLabel)");
              }
              gSettings.PatchDsdtLabel[i] = (__typeof_am__(gSettings.PatchDsdtLabel[i]))AllocateZeroPool(256);
              AsciiSPrint(gSettings.PatchDsdtLabel[i], 255, "%a", DSDTPatchesLabel);
              DBG(" (%a)", gSettings.PatchDsdtLabel[i]);

              FreePool(DSDTPatchesLabel);

              Prop3 = GetProperty (Prop2, "Disabled");
              gSettings.PatchDsdtMenuItem[i].BValue = !IsPropertyTrue (Prop3);

              //DBG (" DSDT bin patch #%d ", i);
              gSettings.PatchDsdtFind[i]    = GetDataSetting (Prop2, "Find",     &Size);
              DBG (" lenToFind: %d", Size);
              gSettings.LenToFind[i]        = (UINT32)Size;
              gSettings.PatchDsdtReplace[i] = GetDataSetting (Prop2, "Replace",  &Size);
              DBG (", lenToReplace: %d", Size);
              gSettings.LenToReplace[i]     = (UINT32)Size;
              gSettings.PatchDsdtTgt[i]     = (CHAR8*)GetDataSetting (Prop2, "TgtBridge", &Size);
              DBG (", Target Bridge: %a\n", gSettings.PatchDsdtTgt[i]);
              if (!gSettings.PatchDsdtMenuItem[i].BValue) {
                DBG("  patch disabled at config\n");
              }
            }
          } //if count > 0
        } //if prop PatchesDSDT

        Prop = GetProperty (Dict2, "ReuseFFFF");
        if (IsPropertyTrue (Prop)) {
          gSettings.ReuseFFFF = TRUE;
        }

        Prop = GetProperty (Dict2, "SuspendOverride");
        if (IsPropertyTrue (Prop)) {
          gSettings.SuspendOverride = TRUE;
        }

        Prop   = GetProperty (Dict2, "DropOEM_DSM");
        defDSM = FALSE;

        if (Prop != NULL) {
          defDSM = TRUE; //set by user
          if (IsPropertyTrue (Prop)) {
            gSettings.DropOEM_DSM = 0xFFFF;
          } else if (IsPropertyFalse (Prop)) {
            gSettings.DropOEM_DSM = 0;
          } else if (Prop->type == kTagTypeInteger) {
            gSettings.DropOEM_DSM = (UINT16)(UINTN)Prop->string;
          } else if (Prop->type == kTagTypeDict) {
            Prop2 = GetProperty (Prop, "ATI");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_ATI;
            }

            Prop2 = GetProperty (Prop, "NVidia");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_NVIDIA;
            }

            Prop2 = GetProperty (Prop, "IntelGFX");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_INTEL;
            }

            Prop2 = GetProperty (Prop, "HDA");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_HDA;
            }

            Prop2 = GetProperty (Prop, "HDMI");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_HDMI;
            }

            Prop2 = GetProperty (Prop, "SATA");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_SATA;
            }

            Prop2 = GetProperty (Prop, "LAN");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_LAN;
            }

            Prop2 = GetProperty (Prop, "WIFI");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_WIFI;
            }

            Prop2 = GetProperty (Prop, "USB");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_USB;
            }

            Prop2 = GetProperty (Prop, "LPC");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_LPC;
            }

            Prop2 = GetProperty (Prop, "SmBUS");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_SMBUS;
            }

            Prop2 = GetProperty (Prop, "Firewire");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_FIREWIRE;
            }

            Prop2 = GetProperty (Prop, "IDE");
            if (IsPropertyTrue (Prop2)) {
              gSettings.DropOEM_DSM |= DEV_IDE;
            }
          }
        }
      }

      Dict2 = GetProperty (DictPointer, "SSDT");
      if (Dict2) {
        Prop2 = GetProperty (Dict2, "Generate");
        if (Prop2 != NULL) {
          if (IsPropertyTrue (Prop2)) {
            gSettings.GeneratePStates = TRUE;
            gSettings.GenerateCStates = TRUE;
            gSettings.GenerateAPSN = TRUE;
            gSettings.GenerateAPLF = TRUE;
            gSettings.GeneratePluginType = TRUE;

          } else if (IsPropertyFalse (Prop2)) {
            gSettings.GeneratePStates = FALSE;
            gSettings.GenerateCStates = FALSE;
            gSettings.GenerateAPSN = FALSE;
            gSettings.GenerateAPLF = FALSE;
            gSettings.GeneratePluginType = FALSE;

          } else if (Prop2->type == kTagTypeDict) {
            Prop = GetProperty (Prop2, "PStates");
            gSettings.GeneratePStates = IsPropertyTrue (Prop);
            gSettings.GenerateAPSN = gSettings.GeneratePStates;
            gSettings.GenerateAPLF = gSettings.GeneratePStates;
            gSettings.GeneratePluginType = gSettings.GeneratePStates;
            Prop = GetProperty (Prop2, "CStates");
            gSettings.GenerateCStates = IsPropertyTrue (Prop);
            Prop = GetProperty (Prop2, "APSN");
            if (Prop) {
              gSettings.GenerateAPSN = IsPropertyTrue (Prop);
            }
            Prop = GetProperty (Prop2, "APLF");
            if (Prop) {
              gSettings.GenerateAPLF = IsPropertyTrue (Prop);
            }
            Prop = GetProperty (Prop2, "PluginType");
            if (Prop) {
              gSettings.GeneratePluginType = IsPropertyTrue (Prop);
            }
          }
        }

        Prop = GetProperty (Dict2, "DropOem");
        gSettings.DropSSDT  = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "NoOemTableId"); // to disable OEM table ID on ACPI/orgin/SSDT file names
        gSettings.NoOemTableId = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "NoDynamicExtract"); // to disable extracting child SSDTs
        gSettings.NoDynamicExtract = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "UseSystemIO");
        gSettings.EnableISS = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "EnableC7");
        if (Prop != NULL) {
          gSettings.EnableC7 = IsPropertyTrue (Prop);
          DBG ("EnableC7: %a\n", gSettings.EnableC7 ? "yes" : "no");
        }

        Prop = GetProperty (Dict2, "EnableC6");
        if (Prop != NULL) {
          gSettings.EnableC6 = IsPropertyTrue (Prop);
          DBG ("EnableC6: %a\n", gSettings.EnableC6 ? "yes" : "no");
        }

        Prop = GetProperty (Dict2, "EnableC4");
        if (Prop != NULL) {
          gSettings.EnableC4 = IsPropertyTrue (Prop);
          DBG ("EnableC4: %a\n", gSettings.EnableC4 ? "yes" : "no");
        }

        Prop = GetProperty (Dict2, "EnableC2");
        if (Prop != NULL) {
          gSettings.EnableC2 = IsPropertyTrue (Prop);
          DBG ("EnableC2: %a\n", gSettings.EnableC2 ? "yes" : "no");
        }

        Prop = GetProperty (Dict2, "C3Latency");
        if (Prop != NULL) {
          gSettings.C3Latency = (UINT16)GetPropertyInteger (Prop, gSettings.C3Latency);
          DBG ("C3Latency: %d\n", gSettings.C3Latency);
        }

        Prop                       = GetProperty (Dict2, "PLimitDict");
        gSettings.PLimitDict       = (UINT8)GetPropertyInteger (Prop, 0);

        Prop                       = GetProperty (Dict2, "UnderVoltStep");
        gSettings.UnderVoltStep    = (UINT8)GetPropertyInteger (Prop, 0);

        Prop                       = GetProperty (Dict2, "DoubleFirstState");
        gSettings.DoubleFirstState = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "MinMultiplier");
        if (Prop != NULL) {
          gSettings.MinMultiplier  = (UINT8)GetPropertyInteger (Prop, gSettings.MinMultiplier);
          DBG ("MinMultiplier: %d\n", gSettings.MinMultiplier);
        }

        Prop = GetProperty (Dict2, "MaxMultiplier");
        if (Prop != NULL) {
          gSettings.MaxMultiplier = (UINT8)GetPropertyInteger (Prop, gSettings.MaxMultiplier);
          DBG ("MaxMultiplier: %d\n", gSettings.MaxMultiplier);
        }

        Prop = GetProperty (Dict2, "PluginType");
        if (Prop != NULL) {
          gSettings.PluginType = (UINT8)GetPropertyInteger (Prop, gSettings.PluginType);
          DBG ("PluginType: %d\n", gSettings.PluginType);
        }
      }

      //     Prop               = GetProperty (DictPointer, "DropMCFG");
      //     gSettings.DropMCFG = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "ResetAddress");
      if (Prop) {
        gSettings.ResetAddr = (UINT32)GetPropertyInteger (Prop, 0x64);
        DBG ("ResetAddr: 0x%x\n", gSettings.ResetAddr);

        if (gSettings.ResetAddr  == 0x64) {
          gSettings.ResetVal = 0xFE;
        } else if  (gSettings.ResetAddr  == 0xCF9) {
          gSettings.ResetVal = 0x06;
        }

        DBG ("Calc ResetVal: 0x%x\n", gSettings.ResetVal);
      }

      Prop = GetProperty (DictPointer, "ResetValue");
      if (Prop) {
        gSettings.ResetVal = (UINT8)GetPropertyInteger (Prop, gSettings.ResetVal);
        DBG ("ResetVal: 0x%x\n", gSettings.ResetVal);
      }
      //other known pair is 0x0CF9/0x06. What about 0x92/0x01 ?

      Prop = GetProperty (DictPointer, "HaltEnabler");
      gSettings.SlpSmiEnable = IsPropertyTrue (Prop);

      //
      Prop = GetProperty (DictPointer, "FixHeaders");
      gSettings.FixHeaders = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "FixMCFG");
      gSettings.FixMCFG = IsPropertyTrue (Prop);


      Prop = GetProperty (DictPointer, "DisableASPM");
      gSettings.NoASPM = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "smartUPS");
      if (Prop) {
        gSettings.smartUPS   = IsPropertyTrue (Prop);
        DBG ("smartUPS: present\n");
      }

      Prop               = GetProperty (DictPointer, "PatchAPIC");
      gSettings.PatchNMI = IsPropertyTrue (Prop);

      Prop               = GetProperty (DictPointer, "SortedOrder");
      if (Prop) {
        INTN   i, Count = GetTagCount (Prop);
        Prop2 = NULL;
        if (Count > 0) {
          gSettings.SortedACPICount = 0;
          gSettings.SortedACPI = (__typeof__(gSettings.SortedACPI))AllocateZeroPool (Count * sizeof(CHAR16 *));

          for (i = 0; i < Count; i++) {
            if (!EFI_ERROR (GetElement (Prop, i, &Prop2)) &&
                (Prop2 != NULL) && (Prop2->type == kTagTypeString)) {
              gSettings.SortedACPI[gSettings.SortedACPICount++] = PoolPrint (L"%a", Prop2->string);
            }
          }
        }
      }

      Prop = GetProperty (DictPointer, "AutoMerge");
      gSettings.AutoMerge  = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "DisabledAML");
      if (Prop) {
        INTN   i, Count = GetTagCount (Prop);
        Prop2 = NULL;
        if (Count > 0) {
          gSettings.DisabledAMLCount = 0;
          gSettings.DisabledAML = (__typeof__(gSettings.DisabledAML))AllocateZeroPool (Count * sizeof(CHAR16 *));

          if (gSettings.DisabledAML) {
            for (i = 0; i < Count; i++) {
              if (!EFI_ERROR (GetElement (Prop, i, &Prop2)) &&
                  (Prop2 != NULL) &&
                  (Prop2->type == kTagTypeString)
                  ) {
                gSettings.DisabledAML[gSettings.DisabledAMLCount++] = PoolPrint (L"%a", Prop2->string);
              }
            }
          }
        }
      }

      Prop = GetProperty(DictPointer, "RenameDevices");
      if (Prop && Prop->type == kTagTypeDict) {
        INTN   i, Count = GetTagCount(Prop);
        if (Count > 0) {
          gSettings.DeviceRenameCount = 0;
          gSettings.DeviceRename = (__typeof__(gSettings.DeviceRename))AllocateZeroPool(Count * sizeof(ACPI_NAME_LIST));
          DBG("Devices to rename %d\n", Count);
          for (i = 0; i < Count; i++) {
            Prop2 = NULL;
            if (!EFI_ERROR(GetElement(Prop, i, &Prop2)) &&
                (Prop2 != NULL) &&
                (Prop2->type == kTagTypeKey)) {
              ACPI_NAME_LIST *List = ParseACPIName(Prop2->string);
              gSettings.DeviceRename[gSettings.DeviceRenameCount].Next = List;
              while (List) {
                DBG("%a:", List->Name);
                List = List->Next;
              }
              Prop2 = Prop2->tag;
              if (Prop2->type == kTagTypeString) {
                gSettings.DeviceRename[gSettings.DeviceRenameCount++].Name = (__typeof__(gSettings.DeviceRename[gSettings.DeviceRenameCount++].Name))AllocateCopyPool(5, Prop2->string);
                DBG("->will be renamed to %a\n", Prop2->string);
              }
            }
          }
        }
      }
    }

    //*** SMBIOS ***//
    DictPointer = GetProperty (Dict, "SMBIOS");
    if (DictPointer != NULL) {
      ParseSMBIOSSettings(DictPointer);
      Prop = GetProperty (DictPointer, "Trust");
      if (Prop != NULL) {
        if (IsPropertyFalse (Prop)) {
          gSettings.TrustSMBIOS = FALSE;
        } else if (IsPropertyTrue (Prop)) {
          gSettings.TrustSMBIOS = TRUE;
        }
      }
      Prop = GetProperty(DictPointer, "MemoryRank");
      gSettings.Attribute = (INT8)GetPropertyInteger(Prop, -1); //1==Single Rank, 2 == Dual Rank, 0==undefined -1 == keep as is

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
      Prop = GetProperty (DictPointer, "Memory");
      if (Prop != NULL){
        // Get memory table count
        Prop2   = GetProperty (Prop, "SlotCount");
        gRAM.UserInUse = (UINT8)GetPropertyInteger (Prop2, 0);
        // Get memory channels
        Prop2             = GetProperty (Prop, "Channels");
        gRAM.UserChannels = (UINT8)GetPropertyInteger (Prop2, 0);
        // Get memory tables
        Prop2 = GetProperty (Prop, "Modules");
        if (Prop2 != NULL) {
          INTN   Count = GetTagCount (Prop2);
          Prop3 = NULL;

          for (i = 0; i < Count; i++) {
            UINT8 Slot = MAX_RAM_SLOTS;
            RAM_SLOT_INFO *SlotPtr;
            if (EFI_ERROR (GetElement (Prop2, i, &Prop3))) {
              continue;
            }

            if (Prop3 == NULL) {
              break;
            }
            // Get memory slot
            Dict2 = GetProperty (Prop3, "Slot");
            if (Dict2 == NULL) {
              continue;
            }

            if (Dict2->type == kTagTypeString && Dict2->string) {
              Slot = (UINT8)AsciiStrDecimalToUintn (Dict2->string);
            } else if (Dict2->type == kTagTypeInteger) {
              Slot = (UINT8)(UINTN)Dict2->string;
            } else {
              continue;
            }

            if (Slot >= MAX_RAM_SLOTS) {
              continue;
            }

            SlotPtr = &gRAM.User[Slot];

            // Get memory size
            Dict2 = GetProperty (Prop3, "Size");
            SlotPtr->ModuleSize = (UINT32)GetPropertyInteger (Dict2, SlotPtr->ModuleSize);
            // Get memory frequency
            Dict2 = GetProperty (Prop3, "Frequency");
            SlotPtr->Frequency  = (UINT32)GetPropertyInteger (Dict2, SlotPtr->Frequency);
            // Get memory vendor
            Dict2 = GetProperty (Prop3, "Vendor");
            if (Dict2 && Dict2->type == kTagTypeString && Dict2->string != NULL) {
              SlotPtr->Vendor   = Dict2->string;
            }
            // Get memory part number
            Dict2 = GetProperty (Prop3, "Part");
            if (Dict2 && Dict2->type == kTagTypeString && Dict2->string != NULL) {
              SlotPtr->PartNo   = Dict2->string;
            }
            // Get memory serial number
            Dict2 = GetProperty (Prop3, "Serial");
            if (Dict2 && Dict2->type == kTagTypeString && Dict2->string != NULL) {
              SlotPtr->SerialNo = Dict2->string;
            }
            // Get memory type
            SlotPtr->Type = MemoryTypeDdr3;
            Dict2 = GetProperty (Prop3, "Type");
            if (Dict2 && Dict2->type == kTagTypeString && Dict2->string != NULL) {
              if (AsciiStriCmp (Dict2->string, "DDR2") == 0) {
                SlotPtr->Type = MemoryTypeDdr2;
              } else if (AsciiStriCmp (Dict2->string, "DDR3") == 0) {
                SlotPtr->Type = MemoryTypeDdr3;
              } else if (AsciiStriCmp (Dict2->string, "DDR4") == 0) {
                SlotPtr->Type = MemoryTypeDdr4;
              } else if (AsciiStriCmp (Dict2->string, "DDR") == 0) {
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

      Prop = GetProperty (DictPointer, "Slots");
      if (Prop != NULL) {
        INTN   DeviceN;
        INTN   Count = GetTagCount (Prop);
        Prop3 = NULL;

        for (INTN Index = 0; Index < Count; ++Index) {
          if (EFI_ERROR (GetElement (Prop, Index, &Prop3))) {
            continue;
          }
          if (Prop3 == NULL) {
            break;
          }

          if (!Index) {
            DBG ("Slots->Devices:\n");
          }

          Prop2 = GetProperty (Prop3, "Device");
          DeviceN = -1;
          if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
            if (AsciiStriCmp (Prop2->string,        "ATI") == 0) {
              DeviceN = 0;
            } else if (AsciiStriCmp (Prop2->string, "NVidia") == 0) {
              DeviceN = 1;
            } else if (AsciiStriCmp (Prop2->string, "IntelGFX") == 0) {
              DeviceN = 2;
            } else if (AsciiStriCmp (Prop2->string, "LAN") == 0) {
              DeviceN = 5;
            } else if (AsciiStriCmp (Prop2->string, "WIFI") == 0) {
              DeviceN = 6;
            } else if (AsciiStriCmp (Prop2->string, "Firewire") == 0) {
              DeviceN = 12;
            } else if (AsciiStriCmp (Prop2->string, "HDMI") == 0) {
              DeviceN = 4;
            } else if (AsciiStriCmp (Prop2->string, "USB") == 0) {
              DeviceN = 11;
            } else if (AsciiStriCmp (Prop2->string, "NVME") == 0) {
              DeviceN = 13;
            } else {
              DBG (" - add properties to unknown device %a, ignored\n", Prop2->string);
              continue;
            }
          } else {
            DBG (" - no device  property for slot\n");
            continue;
          }

          if (DeviceN >= 0) {
            SLOT_DEVICE *SlotDevice = &SlotDevices[DeviceN];
            Prop2                   = GetProperty (Prop3, "ID");
            SlotDevice->SlotID      = (UINT8)GetPropertyInteger (Prop2, DeviceN);
            SlotDevice->SlotType    = SlotTypePci;

            Prop2                   = GetProperty (Prop3, "Type");
            if (Prop2 != NULL) {
              switch ((UINT8)GetPropertyInteger(Prop2, 0)) {
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
            Prop2 = GetProperty (Prop3, "Name");
            if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
              AsciiSPrint (SlotDevice->SlotName, 31, "%a", Prop2->string);
            } else {
              AsciiSPrint (SlotDevice->SlotName, 31, "PCI Slot %d", DeviceN);
            }

            DBG (" - %a\n", SlotDevice->SlotName);
          }
        }
      }
    }

    //CPU
    DictPointer = GetProperty (Dict, "CPU");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "QPI");
      if (Prop != NULL) {
        gSettings.QPI = (UINT16)GetPropertyInteger (Prop, gSettings.QPI);
        DBG ("QPI: %dMHz\n", gSettings.QPI);
      }

      Prop = GetProperty (DictPointer, "FrequencyMHz");
      if (Prop != NULL) {
        gSettings.CpuFreqMHz = (UINT32)GetPropertyInteger (Prop, gSettings.CpuFreqMHz);
        DBG ("CpuFreq: %dMHz\n", gSettings.CpuFreqMHz);
      }

      Prop = GetProperty (DictPointer, "Type");
      gSettings.CpuType = GetAdvancedCpuType();
      if (Prop != NULL) {
        gSettings.CpuType = (UINT16)GetPropertyInteger (Prop, gSettings.CpuType);
        DBG ("CpuType: %x\n", gSettings.CpuType);
      }

      Prop = GetProperty (DictPointer, "QEMU");
      gSettings.QEMU = IsPropertyTrue (Prop);
      if (gSettings.QEMU) {
        DBG ("QEMU: true\n");
      }

      Prop = GetProperty (DictPointer, "UseARTFrequency");
      gSettings.UseARTFreq = IsPropertyTrue (Prop);

      gSettings.UserChange = FALSE;
      Prop = GetProperty (DictPointer, "BusSpeedkHz");
      if (Prop != NULL) {
        gSettings.BusSpeed = (UINT32)GetPropertyInteger (Prop, gSettings.BusSpeed);
        DBG ("BusSpeed: %dkHz\n", gSettings.BusSpeed);
        gSettings.UserChange = TRUE;
      }

      Prop = GetProperty (DictPointer, "C6");
      if (Prop != NULL) {
        gSettings.EnableC6 = IsPropertyTrue (Prop);
      }

      Prop = GetProperty (DictPointer, "C4");
      if (Prop != NULL) {
        gSettings.EnableC4 = IsPropertyTrue (Prop);
      }

      Prop = GetProperty (DictPointer, "C2");
      if (Prop != NULL) {
        gSettings.EnableC2 = IsPropertyTrue (Prop);
      }

      //Usually it is 0x03e9, but if you want Turbo, you may set 0x00FA
      Prop                 = GetProperty (DictPointer, "Latency");
      gSettings.C3Latency  = (UINT16)GetPropertyInteger (Prop, gSettings.C3Latency);

      Prop                 = GetProperty (DictPointer, "SavingMode");
      gSettings.SavingMode = (UINT8)GetPropertyInteger (Prop, 0xFF); //the default value means not set

      Prop                 = GetProperty (DictPointer, "HWPEnable");
      if (Prop && IsPropertyTrue (Prop) && (gCPUStructure.Model >= CPU_MODEL_SKYLAKE_U)) {
        gSettings.HWP = TRUE;
        AsmWriteMsr64 (MSR_IA32_PM_ENABLE, 1);
      }
      Prop                 = GetProperty (DictPointer, "HWPValue");
      if (Prop && gSettings.HWP) {
        gSettings.HWPValue = (UINT32)GetPropertyInteger(Prop, 0);
        AsmWriteMsr64 (MSR_IA32_HWP_REQUEST, gSettings.HWPValue);
      }

      Prop                 = GetProperty (DictPointer, "TDP");
      gSettings.TDP  = (UINT8)GetPropertyInteger (Prop, 0);

      Prop                 = GetProperty (DictPointer, "TurboDisable");
      if (Prop && IsPropertyTrue (Prop)) {
        UINT64 msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
        gSettings.Turbo = 0;
        msr &= ~(1ULL<<38);
        AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, msr);
      }
    }

    // RtVariables
    DictPointer = GetProperty (Dict, "RtVariables");
    if (DictPointer != NULL) {
      // ROM: <data>bin data</data> or <string>base 64 encoded bin data</string>
      Prop = GetProperty (DictPointer, "ROM");
      if (Prop != NULL) {
        if (AsciiStriCmp (Prop->string, "UseMacAddr0") == 0) {
          gSettings.RtROM         = &gLanMac[0][0];
          gSettings.RtROMLen      = 6;
        } else if (AsciiStriCmp (Prop->string, "UseMacAddr1") == 0) {
          gSettings.RtROM         = &gLanMac[1][0];
          gSettings.RtROMLen      = 6;
        } else {
          UINTN ROMLength         = 0;
          gSettings.RtROM         = GetDataSetting (DictPointer, "ROM", &ROMLength);
          gSettings.RtROMLen      = ROMLength;
        }

        if (gSettings.RtROM == NULL || gSettings.RtROMLen == 0) {
          gSettings.RtROM       = NULL;
          gSettings.RtROMLen    = 0;
        }
      }

      // MLB: <string>some value</string>
      Prop = GetProperty (DictPointer, "MLB");
      if (Prop != NULL && AsciiStrLen (Prop->string) > 0) {
        gSettings.RtMLB = (__typeof__(gSettings.RtMLB))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
      }
      // CsrActiveConfig
      Prop = GetProperty (DictPointer, "CsrActiveConfig");
      gSettings.CsrActiveConfig = (UINT32)GetPropertyInteger (Prop, 0x267); //the value 0xFFFF means not set

      //BooterConfig
      Prop = GetProperty (DictPointer, "BooterConfig");
      gSettings.BooterConfig = (UINT16)GetPropertyInteger (Prop, 0); //the value 0 means not set
      //let it be string like "log=0"
      Prop = GetProperty (DictPointer, "BooterCfg");
      if (Prop != NULL && AsciiStrLen (Prop->string) > 0) {
        AsciiStrCpyS (gSettings.BooterCfgStr, 64, Prop->string);
      }
      //Block external variables
      Prop = GetProperty (DictPointer, "Block");
      if (Prop != NULL) {
        INTN   i, Count = GetTagCount (Prop);
        CHAR16 UStr[64];
        RtVariablesNum = 0;
        RtVariables = (__typeof__(RtVariables))AllocateZeroPool(Count * sizeof(RT_VARIABLES));
        for (i = 0; i < Count; i++) {
          Status = GetElement (Prop, i, &Dict);
          if (!EFI_ERROR(Status)) {
            Prop2 = GetProperty (Dict, "Comment");
            if (Prop2 && Prop2->string) {
              DBG(" %a\n", Prop2->string);
            }
            Prop2 = GetProperty(Dict, "Disabled");
            if (IsPropertyFalse(Prop2)) {
              continue;
            }
            Prop2 = GetProperty(Dict, "Guid");
            if (Prop2 != NULL && AsciiStrLen(Prop2->string) > 0) {             
              ZeroMem(UStr, 64 * sizeof(CHAR16));
              if (IsValidGuidAsciiString(Prop2->string)) {
                AsciiStrToUnicodeStrS(Prop2->string, (CHAR16*)&UStr[0], 64);
                StrToGuidLE((CHAR16*)&UStr[0], &RtVariables[RtVariablesNum].VarGuid);
              }
              else {
                DBG("Error: invalid GUID for RT var '%a' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->string);
              }
            }

            Prop2 = GetProperty(Dict, "Name");
            RtVariables[RtVariablesNum].Name = NULL;
            if (Prop2 != NULL && AsciiStrLen(Prop2->string) > 0) {
              AsciiStrToUnicodeStrS(Prop2->string, (CHAR16*)&UStr[0], 64);
        //      RtVariables[RtVariablesNum].Name = (__typeof__(RtVariables[RtVariablesNum].Name))AllocateCopyPool(128, UStr);
              StrCpyS(RtVariables[RtVariablesNum].Name, 64, UStr);
            }
            RtVariablesNum++;
          }
        }
      }

    }

    if (gSettings.RtROM == NULL) {
      gSettings.RtROM    = (UINT8*)&gSettings.SmUUID.Data4[2];
      gSettings.RtROMLen = 6;
    }

    if (gSettings.RtMLB == NULL) {
      gSettings.RtMLB       = &gSettings.BoardSerialNumber[0];
    }

    // if CustomUUID and InjectSystemID are not specified
    // then use InjectSystemID=TRUE and SMBIOS UUID
    // to get Chameleon's default behaviour (to make user's life easier)
    CopyMem ((VOID*)&gUuid, (VOID*)&gSettings.SmUUID, sizeof(EFI_GUID));
    gSettings.InjectSystemID = TRUE;

    // SystemParameters again - values that can depend on previous params
    DictPointer = GetProperty (Dict, "SystemParameters");
    if (DictPointer != NULL) {
      //BacklightLevel
      Prop = GetProperty (DictPointer, "BacklightLevel");
      if (Prop != NULL) {
        gSettings.BacklightLevel       = (UINT16)GetPropertyInteger (Prop, gSettings.BacklightLevel);
        gSettings.BacklightLevelConfig = TRUE;
      }

      Prop = GetProperty (DictPointer, "CustomUUID");
      if (Prop != NULL) {
        if (IsValidGuidAsciiString (Prop->string)) {
          AsciiStrToUnicodeStrS(Prop->string, gSettings.CustomUuid, 40);
          DBG("Converted CustomUUID %s\n", gSettings.CustomUuid);
          Status = StrToGuidLE (gSettings.CustomUuid, &gUuid);
          if (!EFI_ERROR (Status)) {
            IsValidCustomUUID = TRUE;
            // if CustomUUID specified, then default for InjectSystemID=FALSE
            // to stay compatibile with previous Clover behaviour
            gSettings.InjectSystemID = FALSE;
            //            DBG("The UUID is valid\n");
          }
        }

        if (!IsValidCustomUUID) {
          DBG ("Error: invalid CustomUUID '%a' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->string);
        }
      }
      //else gUuid value from SMBIOS
      //     DBG("Finally use %g\n", &gUuid);

      Prop                     = GetProperty (DictPointer, "InjectSystemID");
      gSettings.InjectSystemID = gSettings.InjectSystemID ? !IsPropertyFalse(Prop) : IsPropertyTrue (Prop);

      Prop                     = GetProperty (DictPointer, "NvidiaWeb");
      gSettings.NvidiaWeb      = IsPropertyTrue (Prop);

    }


    DictPointer = GetProperty (Dict, "BootGraphics");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "DefaultBackgroundColor");
      gSettings.DefaultBackgroundColor = (UINT32)GetPropertyInteger (Prop, 0x80000000); //the value 0x80000000 means not set

      Prop = GetProperty (DictPointer, "UIScale");
      gSettings.UIScale = (UINT32)GetPropertyInteger (Prop, 0x80000000);

      Prop = GetProperty (DictPointer, "EFILoginHiDPI");
      gSettings.EFILoginHiDPI = (UINT32)GetPropertyInteger (Prop, 0x80000000);

      Prop = GetProperty (DictPointer, "flagstate");
      *(UINT32*)&gSettings.flagstate[0] = (UINT32)GetPropertyInteger (Prop, 0x80000000);

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

     DictPointer = GetProperty (Dict, "SMCKeys");
     if (DictPointer != NULL) {   //sss
     TagPtr     Key, ValArray;
     for (Key = DictPointer->tag; Key != NULL; Key = Key->tagNext) {
     ValArray = Prop->tag;
     if (Key->type != kTagTypeKey || ValArray == NULL) {
     DBG (" ERROR: Tag is not <key>, type = %d\n", Key->type);
     continue;
     }
     //....
     }
     }
     */
    /*
     {
     EFI_GUID AppleGuid;

     CopyMem ((VOID*)&AppleGuid, (VOID*)&gUuid, sizeof(EFI_GUID));
     AppleGuid.Data1 = SwapBytes32 (AppleGuid.Data1);
     AppleGuid.Data2 = SwapBytes16 (AppleGuid.Data2);
     AppleGuid.Data3 = SwapBytes16 (AppleGuid.Data3);
     DBG ("Platform Uuid: %g, InjectSystemID: %a\n", &AppleGuid, gSettings.InjectSystemID ? "Yes" : "No");
     }
     */

    if (gBootChanged) {
      DictPointer = GetProperty (Dict, "KernelAndKextPatches");
      if (DictPointer != NULL) {
        DBG("refill kernel patches bcoz gBootChanged\n");
        FillinKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)&gSettings) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)), DictPointer);
      }
    } else {
      //DBG("\n ConfigName: %s n", gSettings.ConfigName);
    }

    if (gThemeChanged && GlobalConfig.Theme) {
      DictPointer = GetProperty (Dict, "GUI");
      if (DictPointer != NULL) {
        Prop = GetProperty (DictPointer, "Theme");
        if ((Prop != NULL) && (Prop->type == kTagTypeString) && Prop->string) {
          FreePool(GlobalConfig.Theme);
          GlobalConfig.Theme = PoolPrint (L"%a", Prop->string);
          DBG ("Theme from new config: %s\n", GlobalConfig.Theme);
        }
      }
    }

    SaveSettings();
  }
  //DBG ("config.plist read and return %r\n", Status);
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
CHAR8 *GetOSVersion(IN LOADER_ENTRY *Entry)
{
  CHAR8      *OSVersion  = NULL;
  EFI_STATUS Status      = EFI_NOT_FOUND;
  CHAR8*     PlistBuffer = NULL;
  UINTN      PlistLen;
  TagPtr     DictPointer = NULL;
  TagPtr     Dict        = NULL;
  TagPtr     Prop        = NULL;
  UINTN      i           = 0;
  UINTN      j           = 0;

  if (!Entry || !Entry->Volume) {
    return OSVersion;
  }

  if (OSTYPE_IS_OSX(Entry->LoaderType)) {
    // Detect exact version for Mac OS X Regular/Server
    i = 0;
    while (SystemPlists[i] != NULL && !FileExists(Entry->Volume->RootDir, SystemPlists[i])) {
      i++;
    }

    if (SystemPlists[i] != NULL) { // found macOS System
      Status = egLoadFile (Entry->Volume->RootDir, SystemPlists[i], (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = GetProperty (Dict, "ProductVersion");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        }
        Prop = GetProperty (Dict, "ProductBuildVersion");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        }
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
      Status = egLoadFile (Entry->Volume->RootDir, InstallerPlist, (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = GetProperty (Dict, "ProductVersion");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        }
        Prop = GetProperty (Dict, "ProductBuildVersion");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        }
      }
    }

    // 1st stage - 2
    // Check for plist - createinstallmedia/NetInstall
    if (OSVersion == NULL) {
      InstallerPlist = L"\\.IABootFiles\\com.apple.Boot.plist"; // 10.9 - 10.13.3
      if (FileExists (Entry->Volume->RootDir, InstallerPlist)) {
        Status = egLoadFile (Entry->Volume->RootDir, InstallerPlist, (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = GetProperty (Dict, "Kernel Flags");
          if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
            if (AsciiStrStr (Prop->string, "Install%20OS%20X%20Mavericks.app")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (5, "10.9");
            } else if (AsciiStrStr (Prop->string, "Install%20macOS%20Catalina") || AsciiStrStr (Prop->string, "Install%20macOS%2010.15")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (6, "10.15");
            } else if (AsciiStrStr (Prop->string, "Install%20macOS%20Mojave") || AsciiStrStr (Prop->string, "Install%20macOS%2010.14")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (6, "10.14");
            } else if (AsciiStrStr (Prop->string, "Install%20macOS%20High%20Sierra") || AsciiStrStr (Prop->string, "Install%20macOS%2010.13")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (6, "10.13");
            } else if (AsciiStrStr (Prop->string, "Install%20macOS%20Sierra") || AsciiStrStr (Prop->string, "Install%20OS%20X%2010.12")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (6, "10.12");
            } else if (AsciiStrStr (Prop->string, "Install%20OS%20X%20El%20Capitan") || AsciiStrStr (Prop->string, "Install%20OS%20X%2010.11")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (6, "10.11");
            } else if (AsciiStrStr (Prop->string, "Install%20OS%20X%20Yosemite") || AsciiStrStr (Prop->string, "Install%20OS%20X%2010.10")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (6, "10.10");
            } else if (AsciiStrStr (Prop->string, "Install%20OS%20X%20Mountain%20Lion")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (5, "10.8");
            } else if (AsciiStrStr (Prop->string, "Install%20Mac%20OS%20X%20Lion")) {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (5, "10.7");
            }
          }
        }
      }
    }

    // 2nd stage - 1
    // Check for plist - AppStore/createinstallmedia/startosinstall/Fusion Drive
    if (OSVersion == NULL) {
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
        Status = egLoadFile (Entry->Volume->RootDir, InstallerPlist, (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = GetProperty (Dict, "ProductVersion");
          if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
            OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
          }
          Prop = GetProperty (Dict, "ProductBuildVersion");
          if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
            Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
          }
          // In InstallInfo.plist, there is no a version key only when updating from AppStore in 10.13+
          // If use the startosinstall in 10.13+, this version key exists in InstallInfo.plist
          DictPointer = GetProperty (Dict, "System Image Info"); // 10.12+
          if (DictPointer != NULL) {
            Prop = GetProperty (DictPointer, "version");
            if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
              OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
            }
          }
        }
      }
    }

    // 2nd stage - 2
    // Check for ia.log - InstallESD/createinstallmedia/startosinstall
    // Implemented by Sherlocks
    if (OSVersion == NULL) {
      CONST CHAR8  *s, *fileBuffer, *targetString;
      CHAR8  *Res5 = (__typeof__(Res5))AllocateZeroPool(5);
      CHAR8  *Res6 = (__typeof__(Res6))AllocateZeroPool(6);
      CHAR8  *Res7 = (__typeof__(Res7))AllocateZeroPool(7);
      CHAR8  *Res8 = (__typeof__(Res8))AllocateZeroPool(8);
      UINTN  fileLen = 0;
      CONST CHAR16 *InstallerLog = L"\\Mac OS X Install Data\\ia.log"; // 10.7
      if (!FileExists (Entry->Volume->RootDir, InstallerLog)) {
        InstallerLog = L"\\OS X Install Data\\ia.log"; // 10.8 - 10.11
        if (!FileExists (Entry->Volume->RootDir, InstallerLog)) {
          InstallerLog = L"\\macOS Install Data\\ia.log"; // 10.12+
        }
      }
      if (FileExists (Entry->Volume->RootDir, InstallerLog)) {
        Status = egLoadFile(Entry->Volume->RootDir, InstallerLog, (UINT8 **)&fileBuffer, &fileLen);
        if (!EFI_ERROR (Status)) {
          targetString = (CHAR8*) AllocateZeroPool(fileLen+1);
          CopyMem((VOID*)targetString, (VOID*)fileBuffer, fileLen);
      //    s = SearchString(targetString, fileLen, "Running OS Build: Mac OS X ", 27);
          s = AsciiStrStr(targetString, "Running OS Build: Mac OS X ");
          if (s[31] == ' ') {
            AsciiSPrint (Res5, 5, "%c%c.%c\n", s[27], s[28], s[30]);
            OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Res5), Res5);
            if (s[38] == ')') {
              AsciiSPrint (Res6, 6, "%c%c%c%c%c\n", s[33], s[34], s[35], s[36], s[37]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res6), Res6);
            } else if (s[39] == ')') {
              AsciiSPrint (Res7, 7, "%c%c%c%c%c%c\n", s[33], s[34], s[35], s[36], s[37], s[38]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res7), Res7);
            }
          } else if (s[31] == '.') {
            AsciiSPrint (Res7, 7, "%c%c.%c.%c\n", s[27], s[28], s[30], s[32]);
            OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Res7), Res7);
            if (s[40] == ')') {
              AsciiSPrint (Res6, 6, "%c%c%c%c%c\n", s[35], s[36], s[37], s[38], s[39]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res6), Res6);
            } else if (s[41] == ')') {
              AsciiSPrint (Res7, 7, "%c%c%c%c%c%c\n", s[35], s[36], s[37], s[38], s[39], s[40]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res7), Res7);
            }
          } else if (s[32] == ' ') {
            AsciiSPrint (Res6, 6, "%c%c.%c%c\n", s[27], s[28], s[30], s[31]);
            OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Res6), Res6);
            if (s[39] == ')') {
              AsciiSPrint (Res6, 6, "%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res6), Res6);
            } else if (s[40] == ')') {
              AsciiSPrint (Res7, 7, "%c%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38], s[39]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res7), Res7);
            } else if (s[41] == ')') {
              AsciiSPrint (Res8, 8, "%c%c%c%c%c%c%c\n", s[34], s[35], s[36], s[37], s[38], s[39], s[40]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res8), Res8);
            }
          } else if (s[32] == '.') {
            AsciiSPrint (Res8, 8, "%c%c.%c%c.%c\n", s[27], s[28], s[30], s[31], s[33]);
            OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Res8), Res8);
            if (s[41] == ')') {
              AsciiSPrint (Res6, 6, "%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res6), Res6);
            } else if (s[42] == ')') {
              AsciiSPrint (Res7, 7, "%c%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40], s[41]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res7), Res7);
            } else if (s[43] == ')') {
              AsciiSPrint (Res8, 8, "%c%c%c%c%c%c%c\n", s[36], s[37], s[38], s[39], s[40], s[41], s[42]);
              Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Res8), Res8);
            }
          }
          FreePool(fileBuffer);
          FreePool(targetString);
        }
      }
    }

    // 2nd stage - 3
    // Check for plist - Preboot of APFS
    if (OSVersion == NULL && APFSSupport == TRUE) {
      i = 0;
      while (InstallPlists[i] != NULL && !FileExists(Entry->Volume->RootDir, InstallPlists[i])) {
        i++;
      }

      if (InstallPlists[i] != NULL) {
        Status = egLoadFile (Entry->Volume->RootDir, InstallPlists[i], (UINT8 **)&PlistBuffer, &PlistLen);
        if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
          Prop = GetProperty (Dict, "ProductVersion");
          if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
            OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
          }
          Prop = GetProperty (Dict, "ProductBuildVersion");
          if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
            Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
          }
        }
      }
    }
  }

  if (OSTYPE_IS_OSX_RECOVERY (Entry->LoaderType)) {
    j = 0;
    while (RecoveryPlists[j] != NULL && !FileExists(Entry->Volume->RootDir, RecoveryPlists[j])) {
      j++;
    }
    // Detect exact version for OS X Recovery

    if (RecoveryPlists[j] != NULL) {
      Status = egLoadFile (Entry->Volume->RootDir, RecoveryPlists[j], (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = GetProperty (Dict, "ProductVersion");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          OSVersion = (__typeof__(OSVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        }
        Prop = GetProperty (Dict, "ProductBuildVersion");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          Entry->BuildVersion = (__typeof__(Entry->BuildVersion))AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        }
      }
    } else if (FileExists (Entry->Volume->RootDir, L"\\com.apple.recovery.boot\\boot.efi")) {
      // Special case - com.apple.recovery.boot/boot.efi exists but SystemVersion.plist doesn't --> 10.9 recovery
      OSVersion = (__typeof__(OSVersion))AllocateCopyPool (5, "10.9");
    }
  }

  if (PlistBuffer != NULL) {
    FreePool (PlistBuffer);
  }

  return OSVersion;
}

CONST CHAR16
*GetOSIconName (
                IN  CONST CHAR8 *OSVersion
                )
{
  CONST CHAR16 *OSIconName;
  if (OSVersion == NULL) {
    OSIconName = L"mac";
  } else if (AsciiStrStr (OSVersion, "10.15") != 0) {
    // Catalina
    OSIconName = L"cata,mac";
  } else if (AsciiStrStr (OSVersion, "10.14") != 0) {
    // Mojave
    OSIconName = L"moja,mac";
  } else if (AsciiStrStr (OSVersion, "10.13") != 0) {
    // High Sierra
    OSIconName = L"hsierra,mac";
  } else if (AsciiStrStr (OSVersion, "10.12") != 0) {
    // Sierra
    OSIconName = L"sierra,mac";
  } else if (AsciiStrStr (OSVersion, "10.11") != 0) {
    // El Capitan
    OSIconName = L"cap,mac";
  } else if (AsciiStrStr (OSVersion, "10.10") != 0) {
    // Yosemite
    OSIconName = L"yos,mac";
  } else if (AsciiStrStr (OSVersion, "10.9") != 0) {
    // Mavericks
    OSIconName = L"mav,mac";
  } else if (AsciiStrStr (OSVersion, "10.8") != 0) {
    // Mountain Lion
    OSIconName = L"cougar,mac";
  } else if (AsciiStrStr (OSVersion, "10.7") != 0) {
    // Lion
    OSIconName = L"lion,mac";
  } else if (AsciiStrStr (OSVersion, "10.6") != 0) {
    // Snow Leopard
    OSIconName = L"snow,mac";
  } else if (AsciiStrStr (OSVersion, "10.5") != 0) {
    // Leopard
    OSIconName = L"leo,mac";
  } else if (AsciiStrStr (OSVersion, "10.4") != 0) {
    // Tiger
    OSIconName = L"tiger,mac";
  } else {
    OSIconName = L"mac";
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
  TagPtr     Dict;
  TagPtr     Prop;
  CHAR16     Uuid[40];

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
    Status = egLoadFile (Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasRock && HasPaper) {
    // Paper beats rock
    Status = egLoadFile (Volume->RootDir, SystemPlistP, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasRock && HasScissors) {
    // Rock beats scissors
    Status = egLoadFile (Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasPaper && HasScissors) {
    // Scissors beat paper
    Status = egLoadFile (Volume->RootDir, SystemPlistS, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasPaper) {
    // No match
    Status = egLoadFile (Volume->RootDir, SystemPlistP, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasScissors) {
    // No match
    Status = egLoadFile (Volume->RootDir, SystemPlistS, (UINT8 **)&PlistBuffer, &PlistLen);
  } else {
    // Rock wins by default
    Status = egLoadFile (Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  }

  if (!EFI_ERROR (Status)) {
    Dict = NULL;
    if (ParseXML (PlistBuffer, &Dict, 0) != EFI_SUCCESS) {
      FreePool (PlistBuffer);
      return EFI_NOT_FOUND;
    }

    Prop = GetProperty (Dict, "Root UUID");
    if (Prop != NULL) {
      AsciiStrToUnicodeStrS(Prop->string, Uuid, 40);
      Status = StrToGuidLE (Uuid, &Volume->RootUUID);
    }

    FreePool (PlistBuffer);
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
  CHAR16 *GopDevicePathStr = NULL;
  CHAR16 *DevicePathStr = NULL;

  DbgHeader("GetDevices");

  // Get GOP handle, in order to check to which GPU the monitor is currently connected
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleArray);
  if (!EFI_ERROR(Status)) {
    GopDevicePathStr = DevicePathToStr(DevicePathFromHandle(HandleArray[0]));
    DBG("GOP found at: %s\n", GopDevicePathStr);
  }

  // Scan PCI handles
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleArray
                                    );

  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < HandleCount; ++Index) {
      Status = gBS->HandleProtocol(HandleArray[Index], &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
      if (!EFI_ERROR (Status)) {
        // Read PCI BUS
        PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        Status = PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  sizeof (Pci) / sizeof (UINT32),
                                  &Pci
                                  );

        DBG ("PCI (%02x|%02x:%02x.%02x) : %04x %04x class=%02x%02x%02x\n",
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
          DevicePathStr = DevicePathToStr(DevicePathFromHandle(HandleArray[Index]));
          if (StrStr(GopDevicePathStr, DevicePathStr)) {
            DBG (" - GOP: Provided by device\n");            
            if (NGFX != 0) {
               // we found GOP on a GPU scanned later, make space for this GPU at first position
               for (i=NGFX; i>0; i--) {
                 CopyMem (&gGraphics[i], &gGraphics[i-1], sizeof(GFX_PROPERTIES));
               }
               ZeroMem(&gGraphics[0], sizeof(GFX_PROPERTIES));
               gfx = &gGraphics[0]; // GPU with active GOP will be added at the first position
            }
          }
          if (DevicePathStr != NULL) {
            FreePool(DevicePathStr);
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

              AsciiSPrint (gfx->Model,  64, "%a", info->model_name);
              AsciiSPrint (gfx->Config, 64, "%a", card_configs[info->cfg_name].name);
              gfx->Ports                  = card_configs[info->cfg_name].ports;
              DBG (" - GFX: Model=%a (ATI/AMD)\n", gfx->Model);

              //get mmio
              if (info->chip_family < CHIP_FAMILY_HAINAN) {
                gfx->Mmio = (UINT8 *)(UINTN)(Pci.Device.Bar[2] & ~0x0f);
              } else {
                gfx->Mmio = (UINT8 *)(UINTN)(Pci.Device.Bar[5] & ~0x0f);
              }
              gfx->Connectors = *(UINT32*)(gfx->Mmio + RADEON_BIOS_0_SCRATCH);
              //           DBG(" - RADEON_BIOS_0_SCRATCH = 0x%08x\n", gfx->Connectors);
              gfx->ConnChanged = FALSE;

              SlotDevice                  = &SlotDevices[0];
              SlotDevice->SegmentGroupNum = (UINT16)Segment;
              SlotDevice->BusNum          = (UINT8)Bus;
              SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
              SlotDevice->Valid           = TRUE;
              AsciiSPrint (SlotDevice->SlotName, 31, "PCI Slot 0");
              SlotDevice->SlotID          = 1;
              SlotDevice->SlotType        = SlotTypePciExpressX16;
              break;

            case 0x8086:
              gfx->Vendor                 = Intel;
              AsciiSPrint (gfx->Model, 64, "%a", get_gma_model (Pci.Hdr.DeviceId));
              DBG (" - GFX: Model=%a (Intel)\n", gfx->Model);
              gfx->Ports = 1;
              gfx->Connectors = (1 << NGFX);
              gfx->ConnChanged = FALSE;
              break;

            case 0x10de:
              gfx->Vendor = Nvidia;
              Bar0        = Pci.Device.Bar[0];
              gfx->Mmio   = (UINT8*)(UINTN)(Bar0 & ~0x0f);
              //DBG ("BAR: 0x%p\n", Mmio);
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

              AsciiSPrint (
                           gfx->Model,
                           64,
                           "%a",
                           get_nvidia_model (((Pci.Hdr.VendorId << 16) | Pci.Hdr.DeviceId),
                                             ((Pci.Device.SubsystemVendorID << 16) | Pci.Device.SubsystemID),
                                             NULL) //NULL: get from generic lists
                           );

              DBG(" - GFX: Model=%a family %x (%a)\n", gfx->Model, gfx->Family, CardFamily);
              gfx->Ports                  = 0;

              SlotDevice                  = &SlotDevices[1];
              SlotDevice->SegmentGroupNum = (UINT16)Segment;
              SlotDevice->BusNum          = (UINT8)Bus;
              SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
              SlotDevice->Valid           = TRUE;
              AsciiSPrint (SlotDevice->SlotName, 31, "PCI Slot 0");
              SlotDevice->SlotID          = 1;
              SlotDevice->SlotType        = SlotTypePciExpressX16;
              break;

            default:
              gfx->Vendor = Unknown;
              AsciiSPrint (gfx->Model, 64, "pci%x,%x", Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
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
          AsciiSPrint (SlotDevice->SlotName, 31, "AirPort");
          SlotDevice->SlotID          = 0;
          SlotDevice->SlotType        = SlotTypePciExpressX1;
          DBG(" - WIFI: Vendor=", Pci.Hdr.VendorId);
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
              DBG("Unknown\n");
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
          AsciiSPrint (SlotDevice->SlotName, 31, "Ethernet");
          SlotDevice->SlotID          = 2;
          SlotDevice->SlotType        = SlotTypePciExpressX1;
          gLanVendor[nLanCards]       = Pci.Hdr.VendorId;
          Bar0                        = Pci.Device.Bar[0];
          gLanMmio[nLanCards++]       = (UINT8*)(UINTN)(Bar0 & ~0x0f);
          if (nLanCards >= 4) {
            DBG(" - [!] too many LAN card in the system (upto 4 limit exceeded), overriding the last one\n");
            nLanCards = 3; // last one will be rewritten
          }
          DBG(" - LAN: %d Vendor=", nLanCards-1);
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
          AsciiSPrint (SlotDevice->SlotName, 31, "FireWire");
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
            AsciiSPrint (SlotDevice->SlotName, 31, "HDMI port");
            SlotDevice->SlotID          = 5;
            SlotDevice->SlotType        = SlotTypePciExpressX4;
          }
          if (gSettings.ResetHDA) {
            //Slice method from VoodooHDA
            UINT8 Value = 0;
            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);

            if (EFI_ERROR (Status)) {
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
  if (GopDevicePathStr != NULL) {
    FreePool(GopDevicePathStr);
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
    DBG("add device: %s\n", DevicePathToStr(Prop->DevicePath));
    Prop2 = Prop->Child;
    while (Prop2) {
      if (Prop2->MenuItem.BValue) {
        if (AsciiStrStr(Prop2->Key, "-platform-id") != NULL) {
          if (gSettings.IgPlatform == 0) {
            CopyMem((UINT8*)&gSettings.IgPlatform, (UINT8*)Prop2->Value, Prop2->ValueLen);
          }
          devprop_add_value(device, Prop2->Key, (UINT8*)&gSettings.IgPlatform, 4);
          DBG("   Add key=%a valuelen=%d\n", Prop2->Key, Prop2->ValueLen);
        } else if ((AsciiStrStr(Prop2->Key, "override-no-edid") || AsciiStrStr(Prop2->Key, "override-no-connect"))
          && gSettings.InjectEDID && gSettings.CustomEDID) {
          // special case for EDID properties
          devprop_add_value(device, Prop2->Key, gSettings.CustomEDID, 128);
          DBG("   Add key=%a from custom EDID\n", Prop2->Key);
        } else {
          devprop_add_value(device, Prop2->Key, (UINT8*)Prop2->Value, Prop2->ValueLen);
          DBG("   Add key=%a valuelen=%d\n", Prop2->Key, Prop2->ValueLen);
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

  if (!EFI_ERROR (Status)) {
    for (i = 0; i < HandleCount; i++) {
      Status = gBS->HandleProtocol (HandleBuffer[i], &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
      if (!EFI_ERROR (Status)) {
        // Read PCI BUS
        Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
        if (EFI_ERROR (Status)) {
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
          DBG("custom properties for device %02x:%02x.%02x injected\n", Bus, Device, Function);
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
                    DBG("Device %d deinited\n", j);
                  }
                }
              }
              break;

            case 0x8086:
              if (gSettings.InjectIntel) {
                TmpDirty    = setup_gma_devprop(Entry, &PCIdevice);
                StringDirty |=  TmpDirty;
                MsgLog ("Intel GFX revision  = 0x%x\n", PCIdevice.revision);
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
                MsgLog ("  LEV2 = 0x%x, LEVL = 0x%x, P0BL = 0x%x, GRAN = 0x%x\n", LEV2, LEVL, P0BL, GRAN);
                MsgLog ("  LEVW = 0x%x, LEVX = 0x%x, LEVD = 0x%x, PCHL = 0x%x\n", LEVW, LEVX, LEVD, PCHL);

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
                  MsgLog ("  Found invalid LEVW, set System LEVW: 0x%x\n", SYSLEVW);
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
                    MsgLog ("  Skip writing macOS LEVW: 0x%x\n", MACLEVW);
                    break;

                  case CPU_MODEL_HASWELL:
                  case CPU_MODEL_HASWELL_ULT:
                  case CPU_MODEL_HASWELL_U5:    // Broadwell
                  case CPU_MODEL_BROADWELL_HQ:
                  case CPU_MODEL_BROADWELL_E5:
                  case CPU_MODEL_BROADWELL_DE:
                    // if not change SYS LEVW to macOS LEVW, backlight will be dark and don't work keys for backlight.
                    // so we should use this.
                    MsgLog ("  Write macOS LEVW: 0x%x\n", MACLEVW);

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
                      MsgLog ("  Write macOS LEVW: 0x%x\n", MACLEVW);

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
                        MsgLog ("  Found invalid LEVL, set LEVL: 0x%x\n", LEVL);
                      }

                      if (!LEVX) {
                        ShiftLEVX = FBLEVX;
                        MsgLog ("  Found invalid LEVX, set LEVX: 0x%x\n", ShiftLEVX);
                      }

                      if (gSettings.IntelMaxValue) {
                        FBLEVX = gSettings.IntelMaxValue;
                        MsgLog ("  Read IntelMaxValue: 0x%x\n", FBLEVX);
                      } else {
                        MsgLog ("  Read default Framebuffer LEVX: 0x%x\n", FBLEVX);
                      }

                      LEVL = (LEVL * FBLEVX) / ShiftLEVX;
                      MsgLog ("  Write new LEVL: 0x%x\n", LEVL);

                      /*Status = */PciIo->Mem.Write(
                                                    PciIo,
                                                    EfiPciIoWidthUint32,
                                                    0,
                                                    0x48254,
                                                    1,
                                                    &LEVL
                                                    );

                      LEVX = FBLEVX | FBLEVX << 16;
                      MsgLog ("  Write new LEVX: 0x%x\n", LEVX);

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
                        MsgLog ("  Read IntelMaxValue: 0x%x\n", FBLEVX);
                      } else {
                        MsgLog ("  Read default Framebuffer LEVX: 0x%x\n", FBLEVX);
                      }

                      LEVD = (UINT32)DivU64x32(MultU64x32(FBLEVX, LEVX), 0xFFFF);
                      MsgLog ("  Write new LEVD: 0x%x\n", LEVD);

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
                        MsgLog ("  Read IntelMaxValue: 0x%x\n", FBLEVX);
                        LEVX = FBLEVX | FBLEVX << 16;
                      } else if (!LEVX) {
                        MsgLog ("  Found invalid LEVX, set LEVX: 0x%x\n", FBLEVX);
                        LEVX = FBLEVX | FBLEVX << 16;
                      } else if (ShiftLEVX != FBLEVX) {
                        MsgLog ("  Read default Framebuffer LEVX: 0x%x\n", FBLEVX);
                        LEVX = (((LEVX & 0xFFFF) * FBLEVX / ShiftLEVX) | FBLEVX << 16);
                      }

                      MsgLog ("  Write new LEVX: 0x%x\n", LEVX);

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
        else if (gSettings.HDAInjection &&
                 (Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
                 ((Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA) ||
                  (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_AUDIO))) {
                   // HDMI injection inside
                   TmpDirty    = setup_hda_devprop (PciIo, &PCIdevice, Entry->OSVersion);
                   StringDirty |= TmpDirty;
                 }

        //LPC
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_BRIDGE) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_BRIDGE_ISA))
        {
          if (gSettings.LpcTune) {
            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, GEN_PMCON_1, 1, &PmCon);
            MsgLog ("Initial PmCon value=%x\n", PmCon);

            if (gSettings.EnableC6) {
              PmCon |= 1 << 11;
              DBG ("C6 enabled\n");
            } else {
              PmCon &= ~(1 << 11);
              DBG ("C6 disabled\n");
            }
            /*
             if (gSettings.EnableC2) {
             PmCon |= 1 << 10;
             DBG ("BIOS_PCIE enabled\n");
             } else {
             PmCon &= ~(1 << 10);
             DBG ("BIOS_PCIE disabled\n");
             }
             */
            if (gSettings.EnableC4) {
              PmCon |= 1 << 7;
              DBG ("C4 enabled\n");
            } else {
              PmCon &= ~(1 << 7);
              DBG ("C4 disabled\n");
            }

            if (gSettings.EnableISS) {
              PmCon |= 1 << 3;
              DBG ("SpeedStep enabled\n");
            } else {
              PmCon &= ~(1 << 3);
              DBG ("SpeedStep disabled\n");
            }

            PciIo->Pci.Write (PciIo, EfiPciIoWidthUint16, GEN_PMCON_1, 1, &PmCon);

            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16,GEN_PMCON_1, 1, &PmCon);
            MsgLog ("Set PmCon value=%x\n", PmCon);

          }
          Rcba   = 0;
          /* Scan Port */
          Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0xF0, 1, &Rcba);
          if (EFI_ERROR (Status)) continue;
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
              DBG ("HPET is already enabled\n");
            } else {
              DBG ("HPET is disabled, trying to enable...\n");
              REG32 ((UINTN)Rcba, 0x3404) = Hptc | 0x80;
            }
            // Re-Check if HPET is enabled.
            Hptc = REG32 ((UINTN)Rcba, 0x3404);
            if ((Hptc & 0x80) == 0) {
              DBG ("HPET is disabled in HPTC. Cannot enable!\n");
            } else {
              DBG ("HPET is enabled\n");
            }
          }

          if (gSettings.DisableFunctions){
            UINT32 FD = REG32 ((UINTN)Rcba, 0x3418);
            DBG ("Initial value of FD register 0x%x\n", FD);
            FD |= gSettings.DisableFunctions;
            REG32 ((UINTN)Rcba, 0x3418) = FD;
            FD = REG32 ((UINTN)Rcba, 0x3418);
            DBG (" recheck value after patch 0x%x\n", FD);
          }
        }
      }
    }
  }

  if (StringDirty) {
    EFI_PHYSICAL_ADDRESS BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
    device_inject_stringlength                   = device_inject_string->length * 2;
    DBG ("stringlength = %d\n", device_inject_stringlength);
    // gDeviceProperties = (__typeof__(gDeviceProperties))AllocateAlignedPages EFI_SIZE_TO_PAGES (device_inject_stringlength + 1), 64);

    Status = gBS->AllocatePages (
                                 AllocateMaxAddress,
                                 EfiACPIReclaimMemory,
                                 EFI_SIZE_TO_PAGES ((UINTN)device_inject_stringlength + 1),
                                 &BufferPtr
                                 );

    if (!EFI_ERROR (Status)) {
      mProperties       = (UINT8*)(UINTN)BufferPtr;
      gDeviceProperties = devprop_generate_string (device_inject_string);
      gDeviceProperties[device_inject_stringlength] = 0;
      //     DBG (gDeviceProperties);
      //     DBG ("\n");
      //     StringDirty = FALSE;
      //-------
      mPropSize = (UINT32)AsciiStrLen (gDeviceProperties) / 2;
      //     DBG ("Preliminary size of mProperties=%d\n", mPropSize);
      mPropSize = hex2bin (gDeviceProperties, mProperties, mPropSize);
      //     DBG ("Final size of mProperties=%d\n", mPropSize);
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

  MsgLog ("CurrentMode: Width=%d Height=%d\n", UGAWidth, UGAHeight);
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
    //DBG ("QPI: use Table 132\n");
  }
  else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
      case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
      case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
      case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
        gSettings.SetTable132 = TRUE;
        DBG ("QPI: use Table 132\n");
        break;
      default:
        //DBG ("QPI: disable Table 132\n");
        break;
    }
  }

  gCPUStructure.CPUFrequency    = MultU64x64 (gCPUStructure.MaxSpeed, Mega);

  return EFI_SUCCESS;
}

CHAR16
*GetOtherKextsDir (BOOLEAN On)
{
  CHAR16 *SrcDir         = NULL;

  SrcDir     = PoolPrint (L"%s\\kexts\\%a", OEMPath, On?"Other":"Off");
  if (!FileExists (SelfVolume->RootDir, SrcDir)) {
    FreePool (SrcDir);
    SrcDir = PoolPrint (L"\\EFI\\CLOVER\\kexts\\%a", On?"Other":"Off");
    if (!FileExists (SelfVolume->RootDir, SrcDir)) {
      FreePool (SrcDir);
      SrcDir = NULL;
    }
  }

  return SrcDir;
}

//dmazar
// caller is responsible for FreePool the result
CHAR16
*GetOSVersionKextsDir (
                       CHAR8 *OSVersion
                       )
{
  CHAR16 *SrcDir         = NULL;
  CHAR8  FixedVersion[16];
  CHAR8  *DotPtr;

  if (OSVersion != NULL) {
    AsciiStrnCpyS(FixedVersion, 16, OSVersion, 5);
    //    DBG("%a\n", FixedVersion);
    // OSVersion may contain minor version too (can be 10.x or 10.x.y)
    if ((DotPtr = AsciiStrStr (FixedVersion, ".")) != NULL) {
      DotPtr = AsciiStrStr (DotPtr+1, "."); // second dot
    }

    if (DotPtr != NULL) {
      *DotPtr = 0;
    }
  }

  //MsgLog ("OS=%s\n", OSTypeStr);

  // find source injection folder with kexts
  // note: we are just checking for existance of particular folder, not checking if it is empty or not
  // check OEM subfolders: version specific or default to Other
  SrcDir     = PoolPrint (L"%s\\kexts\\%a", OEMPath, FixedVersion);
  if (!FileExists (SelfVolume->RootDir, SrcDir)) {
    FreePool (SrcDir);
    SrcDir = PoolPrint (L"\\EFI\\CLOVER\\kexts\\%a", FixedVersion);
    if (!FileExists (SelfVolume->RootDir, SrcDir)) {
      FreePool (SrcDir);
      SrcDir = NULL;
    }
  }
  return SrcDir;
}

EFI_STATUS
InjectKextsFromDir (
                    EFI_STATUS Status,
                    CHAR16 *SrcDir
                    )
{

  if (EFI_ERROR (Status)) {
    MsgLog (" - ERROR: Kext injection failed!\n");
    return EFI_NOT_STARTED;
  }

  return Status;
}

EFI_STATUS
SetFSInjection (
                IN LOADER_ENTRY *Entry
                )
{
  EFI_STATUS           Status;
  REFIT_VOLUME         *Volume;
  FSINJECTION_PROTOCOL *FSInject;
  CHAR16               *SrcDir         = NULL;
  //BOOLEAN              InjectionNeeded = FALSE;
  //BOOLEAN              BlockCaches     = FALSE;
  FSI_STRING_LIST      *Blacklist      = 0;
  FSI_STRING_LIST      *ForceLoadKexts = NULL;

  MsgLog ("Beginning FSInjection\n");

  Volume = Entry->Volume;

  // some checks?
  /*
   // apianti - this seems to not work sometimes or ever, so just always start
   if ((Volume->BootType == BOOTING_BY_PBR) ||
   (Volume->BootType == BOOTING_BY_MBR) ||
   (Volume->BootType == BOOTING_BY_CD)) {
   MsgLog ("not started - not an EFI boot\n");
   return EFI_UNSUPPORTED;
   }
   */

  // get FSINJECTION_PROTOCOL
  Status = gBS->LocateProtocol(&gFSInjectProtocolGuid, NULL, (void **)&FSInject);
  if (EFI_ERROR (Status)) {
    //Print (L"- No FSINJECTION_PROTOCOL, Status = %r\n", Status);
    MsgLog (" - ERROR: gFSInjectProtocolGuid not found!\n");
    return EFI_NOT_STARTED;
  }

  // check if blocking of caches is needed
  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_NOCACHES) || ((StrStr(Entry->LoadOptions, L"-f") != NULL))) {
    MsgLog ("Blocking kext caches\n");
    //  BlockCaches = TRUE;
    // add caches to blacklist
    Blacklist = FSInject->CreateStringList ();
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
      FSInject->AddStringToList(Blacklist, PoolPrint (L"\\System\\Library\\Extensions\\%s", gSettings.BlockKexts));
    }
  }

  // check if kext injection is needed
  // (will be done only if caches are blocked or if boot.efi refuses to load kernelcache)
  //SrcDir = NULL;
  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_WITHKEXTS)) {
    SrcDir = GetOtherKextsDir(TRUE);
    Status = FSInject->Install (
                                Volume->DeviceHandle,
                                L"\\System\\Library\\Extensions",
                                SelfVolume->DeviceHandle,
                                //GetOtherKextsDir (),
                                SrcDir,
                                Blacklist,
                                ForceLoadKexts
                                );
    //InjectKextsFromDir(Status, GetOtherKextsDir());
    InjectKextsFromDir(Status, SrcDir);
    FreePool (SrcDir);

    SrcDir = GetOSVersionKextsDir (Entry->OSVersion);
    Status = FSInject->Install (
                                Volume->DeviceHandle,
                                L"\\System\\Library\\Extensions",
                                SelfVolume->DeviceHandle,
                                //GetOSVersionKextsDir (Entry->OSVersion),
                                SrcDir,
                                Blacklist,
                                ForceLoadKexts
                                );
    //InjectKextsFromDir(Status, GetOSVersionKextsDir(Entry->OSVersion));
    InjectKextsFromDir(Status, SrcDir);
    FreePool(SrcDir);
  } else {
    MsgLog ("skipping kext injection (not requested)\n");
  }

  // prepare list of kext that will be forced to load
  ForceLoadKexts = FSInject->CreateStringList ();
  if (ForceLoadKexts == NULL) {
    MsgLog (" - Error: not enough memory!\n");
    return EFI_NOT_STARTED;
  }

  KextPatcherRegisterKexts (FSInject, ForceLoadKexts, Entry);

  // reinit Volume->RootDir? it seems it's not needed.

  return Status;
}


/*
 Slice 2012
*/

#include "entry_scan.h"
#include "kernel_patcher.h"
#include "ati.h"

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

//#define kXMLTagArray   		"array"

//EFI_GUID gRandomUUID = {0x0A0B0C0D, 0x0000, 0x1010, {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}};

TagPtr                          gConfigDict[NUM_OF_CONFIGS] = {NULL, NULL, NULL};

SETTINGS_DATA                   gSettings;
LANGUAGES                       gLanguage;
GFX_PROPERTIES                  gGraphics[4]; //no more then 4 graphics cards
//SLOT_DEVICE                     Arpt;
SLOT_DEVICE                     SlotDevices[16]; //assume DEV_XXX, Arpt=6
EFI_EDID_DISCOVERED_PROTOCOL    *EdidDiscovered;
UINT8                           *gEDID = NULL;
//EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;
//UINT16                          gCPUtype;
UINTN                           NGFX                        = 0; // number of GFX

UINTN                           ThemesNum                   = 0;
CHAR16                          *ThemesList[50]; //no more then 50 themes?

// firmware
BOOLEAN                         gFirmwareClover             = FALSE;
UINTN                           gEvent;
UINT16                          gBacklightLevel;
BOOLEAN                         defDSM;
UINT16                          dropDSM;

extern MEM_STRUCTURE            gRAM;
extern BOOLEAN                  NeedPMfix;


GUI_ANIME                       *GuiAnime                   = NULL;

extern INTN                     ScrollWidth;
extern INTN                     ScrollButtonsHeight;
extern INTN                     ScrollBarDecorationsHeight;
extern INTN                     ScrollScrollDecorationsHeight;

extern UINT8 GetOSTypeFromPath (
  IN  CHAR16 *Path
  );

// global configuration with default values
REFIT_CONFIG   GlobalConfig = { FALSE, -1, 0, 0, 0, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE,
  //Font
  FONT_ALFA, 7, 0xFFFFFF80, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, None, 0,
  //BackgroundDark
  FALSE, FALSE, FALSE, 0, 0, 4, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  //BannerEdgeHorizontal
  0, 0, 0, 0,
  //VerticalLayout
  FALSE, FALSE, 128, 8, 24
};
/*
VOID __inline WaitForSts(VOID) {
  UINT32 inline_timeout = 100000;
  while (AsmReadMsr64(MSR_IA32_PERF_STATUS) & (1 << 21)) { if (!inline_timeout--) break; }
}
*/
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
  for (i = 0; i < Len; ++i) {
    x += Fake[i];
  }

  return x;
}

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
    return (INTN)Prop->string;    
  } else if ((Prop->type == kTagTypeString) && Prop->string) {
    if ((Prop->string[1] == 'x') || (Prop->string[1] == 'X')) {
      return (INTN)AsciiStrHexToUintn (Prop->string);
    }

    if (Prop->string[0] == '-') {
      return -(INTN)AsciiStrDecimalToUintn (Prop->string + 1);
    }

    return (INTN)AsciiStrDecimalToUintn (Prop->string);
  }
  return Default;
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
    CHAR8 *PlistStrings[]  =
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
//  DBG ("TailSize = %d\n", TailSize);

    if ((TailSize) <= 0) {
      return;
    }

    for (i = 0; PlistStrings[i] != '\0'; ++i) {
      PlistStringsLen = AsciiStrLen (PlistStrings[i]);
//    DBG ("PlistStrings[%d] = %a\n", i, PlistStrings[i]);
      if (PlistStringsLen < TailSize) {
        if (AsciiStriNCmp (PlistStrings[i], Start, PlistStringsLen)) {
          DBG ("Found Plist String = %a, parse XML in LoadOptions\n", PlistStrings[i]);
          if (ParseXML (Start, Dict, (UINT32)TailSize) != EFI_SUCCESS) {
            *Dict = NULL;
            DBG ("Xml in load options is bad\n");

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

    AsciiConf = AllocateCopyPool (TailSize + 1, Start);
    if (AsciiConf != NULL) {
      *(AsciiConf + TailSize) = '\0';
      *Conf = AllocateZeroPool ((TailSize + 1) * sizeof (CHAR16));
      AsciiStrToUnicodeStr (AsciiConf, *Conf);
      FreePool (AsciiConf);
      return;
    }
}

//
// returns binary setting in a new allocated buffer and data length in dataLen.
// data can be specified in <data></data> base64 encoded
// or in <string></string> hex encoded
//
VOID
*GetDataSetting (
  IN      TagPtr Dict,
  IN      CHAR8  *PropName,
     OUT  UINTN  *DataLen
  )
{
  TagPtr Prop;
  UINT8  *Data;
  UINT32 Len;
  //UINTN   i;
    
  Prop = GetProperty (Dict, PropName);
  if (Prop != NULL) {
    if (Prop->data != NULL && Prop->dataLen > 0) {
      // data property
      Data = AllocateZeroPool (Prop->dataLen);
      CopyMem (Data, Prop->data, Prop->dataLen);

      if (DataLen != NULL) {
        *DataLen = Prop->dataLen;
      }
/*
      DBG ("Data: %p, Len: %d = ", Data, Prop->dataLen);
      for (i = 0; i < Prop->dataLen; ++i) {
         DBG ("%02x ", Data[i]);
      }
       DBG ("\n");
*/
    } else {
      // assume data in hex encoded string property
      Data = AllocateZeroPool((UINT32)(AsciiStrLen (Prop->string) >> 1)); // 2 chars per byte
      Len  = hex2bin (Prop->string, Data, Len);

      if (DataLen != NULL) {
         *DataLen = Len;
      }
/*
      DBG ("Data(str): %p, Len: %d = ", data, len);
      for (i = 0; i < Len; ++i) {
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
  IN CHAR16   *ConfName,
     TagPtr   *Dict)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  UINTN      Size = 0;
  CHAR8*     gConfigPtr = NULL;
  CHAR16*    ConfigPlistPath;
  CHAR16*    ConfigOemPath;
  
  // load config
  if ((ConfName == NULL) || (Dict == NULL)) {
    DBG ("Can't load plist in LoadUserSettings: NULL\n");
    return EFI_NOT_FOUND;
  }
  
  ConfigPlistPath = PoolPrint (L"EFI\\CLOVER\\%s.plist", ConfName);
  ConfigOemPath   = PoolPrint (L"%s\\%s.plist", OEMPath, ConfName);
 // DBG ("PlistPath: %s\n", ConfigPlistPath);
  if (FileExists (SelfRootDir, ConfigOemPath)) {
    Status = egLoadFile (SelfRootDir, ConfigOemPath, (UINT8**)&gConfigPtr, &Size);
  } else {
    DBG ("Oem %s.plist not found at path: %s\n", ConfName, ConfigOemPath);
  }
  
  if (EFI_ERROR (Status)) {
    if ((RootDir != NULL) && FileExists (RootDir, ConfigPlistPath)) {
      Status = egLoadFile (RootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &Size);
    }

    if (EFI_ERROR (Status)) {
      Status = egLoadFile (SelfRootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &Size);
    }
  } else {
    DBG ("Using OEM %s.plist at path: %s\n", ConfName, ConfigOemPath);
  }
  
  if (EFI_ERROR (Status) || gConfigPtr == NULL) {
//  DBG ("Error loading %s.plist! Status=%r\n", ConfName, Status);
    return Status;
  }
  
  Status = ParseXML ((const CHAR8*)gConfigPtr, Dict, (UINT32)Size);
  if (EFI_ERROR (Status)) {
//  Dict = NULL;
    DBG (" plist parse error Status=%r\n", Status);
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
CopyKernelAndKextPatches (
  IN OUT  KERNEL_AND_KEXT_PATCHES *Dst,
  IN      KERNEL_AND_KEXT_PATCHES *Src)
{
  if (Dst == NULL || Src == NULL) return FALSE;

  Dst->KPDebug       = Src->KPDebug;
  Dst->KPKernelCpu   = Src->KPKernelCpu;
  Dst->KPLapicPanic  = Src->KPLapicPanic;
  Dst->KPAsusAICPUPM = Src->KPAsusAICPUPM;
  Dst->KPAppleRTC    = Src->KPAppleRTC;
  Dst->KPKernelPm    = Src->KPKernelPm;
  Dst->FakeCPUID     = Src->FakeCPUID;

  if (Src->KPATIConnectorsController != NULL) {
    Dst->KPATIConnectorsController = EfiStrDuplicate (Src->KPATIConnectorsController);
  }

  if ((Src->KPATIConnectorsDataLen > 0) &&
      (Src->KPATIConnectorsData != NULL) &&
      (Src->KPATIConnectorsPatch != NULL)) {
    Dst->KPATIConnectorsDataLen = Src->KPATIConnectorsDataLen;
    Dst->KPATIConnectorsData    = AllocateCopyPool (Src->KPATIConnectorsDataLen, Src->KPATIConnectorsData);
    Dst->KPATIConnectorsPatch   = AllocateCopyPool (Src->KPATIConnectorsDataLen, Src->KPATIConnectorsPatch);
  }

  if ((Src->NrForceKexts > 0) && (Src->ForceKexts != NULL)) {
    INTN i;
    Dst->ForceKexts = AllocatePool (Src->NrForceKexts * sizeof(CHAR16 *));

    for (i = 0; i < Src->NrForceKexts; ++i) {
      Dst->ForceKexts[Dst->NrForceKexts++] = EfiStrDuplicate (Src->ForceKexts[i]);
    }
  }

  if ((Src->NrKexts > 0) && (Src->KextPatches != NULL)) {
    INTN i;
    Dst->KextPatches = AllocatePool (Src->NrKexts * sizeof(KEXT_PATCH));

    for (i = 0; i < Src->NrKexts; ++i)
    {
      if ((Src->KextPatches[i].DataLen <= 0) ||
          (Src->KextPatches[i].Data == NULL) ||
          (Src->KextPatches[i].Patch == NULL)) {
        continue;
      }

      if (Src->KextPatches[i].Name) {
        Dst->KextPatches[Dst->NrKexts].Name       = (CHAR8 *)AllocateCopyPool (AsciiStrSize (Src->KextPatches[i].Name), Src->KextPatches[i].Name);
      }

      Dst->KextPatches[Dst->NrKexts].IsPlistPatch = Src->KextPatches[i].IsPlistPatch;
      Dst->KextPatches[Dst->NrKexts].DataLen      = Src->KextPatches[i].DataLen;
      Dst->KextPatches[Dst->NrKexts].Data         = AllocateCopyPool (Src->KextPatches[i].DataLen, Src->KextPatches[i].Data);
      Dst->KextPatches[Dst->NrKexts].Patch        = AllocateCopyPool (Src->KextPatches[i].DataLen, Src->KextPatches[i].Patch);
      ++(Dst->NrKexts);
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
      DuplicateEntry->BootBgColor    = AllocateCopyPool (sizeof(EG_PIXEL), Entry->BootBgColor);
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
FillinKextPatches (
  IN OUT KERNEL_AND_KEXT_PATCHES *Patches,
  TagPtr DictPointer
  )
{
  TagPtr Prop;
  UINTN  i;

  if (Patches == NULL || DictPointer == NULL) {
    return FALSE;
  }

  if (NeedPMfix) {
    Patches->KPKernelPm = TRUE;
    Patches->KPAsusAICPUPM = TRUE;
  }

  Prop = GetProperty (DictPointer, "Debug");
  if (Prop != NULL) {
    Patches->KPDebug = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "KernelCpu");
  if (Prop != NULL) {
    Patches->KPKernelCpu = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "FakeCPUID");
  if (Prop != NULL) {
    Patches->FakeCPUID = (UINT32)GetPropertyInteger (Prop, 0);
    DBG ("Config set FakeCPUID=%x\n", Patches->FakeCPUID);
  }

  Prop = GetProperty (DictPointer, "AsusAICPUPM");
  if (Prop != NULL) {
    Patches->KPAsusAICPUPM = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "KernelPm");
  if (Prop != NULL) {
    Patches->KPKernelPm = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "KernelLapic");
  if (Prop != NULL) {
    Patches->KPLapicPanic = IsPropertyTrue (Prop);
  }

  Prop = GetProperty (DictPointer, "ATIConnectorsController");
  if (Prop != NULL) {
    UINTN len = 0;

    // ATIConnectors patch
    Patches->KPATIConnectorsController = AllocateZeroPool (AsciiStrSize (Prop->string) * sizeof(CHAR16));
    AsciiStrToUnicodeStr (Prop->string, Patches->KPATIConnectorsController);

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

  Prop = GetProperty (DictPointer, "AppleRTC");
  if (Prop != NULL) {
    Patches->KPAppleRTC = !IsPropertyFalse (Prop);  //default = TRUE
  }

  Prop = GetProperty (DictPointer, "ForceKextsToLoad");
  if (Prop != NULL) {
    UINTN Count = GetTagCount (Prop);
    if (Count > 0) {
      TagPtr Prop2;
      CHAR16 **newForceKexts = AllocateZeroPool ((Patches->NrForceKexts + Count) * sizeof(CHAR16 *));

      if (Patches->ForceKexts != NULL) {
        CopyMem (newForceKexts, Patches->ForceKexts, (Patches->NrForceKexts * sizeof(CHAR16 *)));
        FreePool (Patches->ForceKexts);
      }

      Patches->ForceKexts = newForceKexts;
      DBG ("ForceKextsToLoad: %d requested\n", Count);

      for (i = 0; i < Count; ++i) {
        EFI_STATUS Status = GetElement (Prop, i, &Prop);
        if (EFI_ERROR (Status)) {
           DBG ("error %r getting next element at index %d\n", Status, i);
           continue;
        }

        if (Prop2 == NULL) {
          break;
        }

        if (Prop2->string != NULL) {
          if (*(Prop2->string) == '\\') {
            ++Prop2->string;
          }

          if (AsciiStrLen (Prop2->string) > 0) {
             Patches->ForceKexts[Patches->NrForceKexts] = AllocateZeroPool (AsciiStrSize (Prop2->string) * sizeof(CHAR16));
             AsciiStrToUnicodeStr (Prop2->string, Patches->ForceKexts[Patches->NrForceKexts]);
             DBG ("ForceKextsToLoad %d: %s\n", Patches->NrForceKexts, Patches->ForceKexts[Patches->NrForceKexts]);
             ++Patches->NrForceKexts;
          }
        }
      }
    }
  }

  Prop = GetProperty (DictPointer, "KextsToPatch");
  if (Prop != NULL) {
    UINTN Count = GetTagCount (Prop);
    if (Count > 0) {
      UINTN      j           = 0;
      TagPtr     Prop2;
      TagPtr     Dict;
      KEXT_PATCH *newPatches = AllocateZeroPool ((Patches->NrKexts + Count) * sizeof(KEXT_PATCH));

      // Patches->NrKexts = 0;
      if (Patches->KextPatches != NULL) {
         CopyMem (newPatches, Patches->KextPatches, (Patches->NrKexts * sizeof(KEXT_PATCH)));
         FreePool (Patches->KextPatches);
      }

      Patches->KextPatches = newPatches;
      DBG ("KextsToPatch: %d requested\n", Count);
      for (i = 0; i < Count; ++i) {
        EFI_STATUS Status = GetElement (Prop, i, &Prop);
        if (EFI_ERROR (Status)) {
          DBG ("error %r getting next element at index %d\n", Status, i);
          continue;
        }

        if (Prop2 == NULL) {
          break;
        }

        Patches->KextPatches[Patches->NrKexts].Name  = NULL;
        Patches->KextPatches[Patches->NrKexts].Data  = NULL;
        Patches->KextPatches[Patches->NrKexts].Patch = NULL;
        DBG ("KextToPatch %d:", i);

        Dict = GetProperty (Prop2, "Name");
        if (Dict == NULL) {
           continue;
        }

        Patches->KextPatches[Patches->NrKexts].Name = AllocateCopyPool (AsciiStrSize (Dict->string), Dict->string);
        Dict = GetProperty (Prop2, "Comment");
        if (Dict != NULL) {
           DBG (" %a (%a)", Patches->KextPatches[Patches->NrKexts].Name, Dict->string);
        } else {
           DBG (" %a", Patches->KextPatches[Patches->NrKexts].Name);
        }

        // check if this is Info.plist patch or kext binary patch
        Dict = GetProperty (Prop2, "InfoPlistPatch");
        Patches->KextPatches[Patches->NrKexts].IsPlistPatch = IsPropertyTrue (Dict);

        if (Patches->KextPatches[Patches->NrKexts].IsPlistPatch) {
          // Info.plist
          // Find and Replace should be in <string>...</string>
          DBG (" Info.plist patch");
          Dict = GetProperty (Prop2, "Find");
          Patches->KextPatches[Patches->NrKexts].DataLen = 0;
          if (Dict && Dict->string) {
            Patches->KextPatches[Patches->NrKexts].DataLen = AsciiStrLen (Dict->string);
            Patches->KextPatches[Patches->NrKexts].Data = (UINT8*)AllocateCopyPool (Patches->KextPatches[Patches->NrKexts].DataLen + 1, Dict->string);
          }

          Dict = GetProperty (Prop2, "Replace");
          j = 0;
          if (Dict && Dict->string) {
            j = AsciiStrLen (Dict->string);
            Patches->KextPatches[Patches->NrKexts].Patch = (UINT8*)AllocateCopyPool (j + 1, Dict->string);
          }
        } else {
          // kext binary patch
          // Find and Replace should be in <data>...</data> or <string>...</string>
          DBG (" Kext bin patch");
          Patches->KextPatches[Patches->NrKexts].Data    = GetDataSetting (Prop2, "Find", &j);
          Patches->KextPatches[Patches->NrKexts].DataLen = j;
          Patches->KextPatches[Patches->NrKexts].Patch   = GetDataSetting (Prop2, "Replace", &j);
        }

        if ((Patches->KextPatches[Patches->NrKexts].DataLen != (INTN)j) || (j == 0)) {
          DBG (" - invalid Find/Replace data - skipping!\n");
          if (Patches->KextPatches[Patches->NrKexts].Name != NULL) {
            FreePool (Patches->KextPatches[Patches->NrKexts].Name); //just erase name
            Patches->KextPatches[Patches->NrKexts].Name  = NULL;
          }

          if (Patches->KextPatches[Patches->NrKexts].Data != NULL) {
            FreePool (Patches->KextPatches[Patches->NrKexts].Data); //just erase data
            Patches->KextPatches[Patches->NrKexts].Data  = NULL;
          }

          if (Patches->KextPatches[Patches->NrKexts].Patch != NULL) {
            FreePool (Patches->KextPatches[Patches->NrKexts].Patch); //just erase patch
            Patches->KextPatches[Patches->NrKexts].Patch = NULL;
          }

          continue; //same i
        }

        DBG (", data len: %d\n", Patches->KextPatches[Patches->NrKexts].DataLen);
        Patches->NrKexts++; //must be out of DBG because it may be empty compiled
      }
    }

    //gSettings.NrKexts = (INT32)i;
    //there is one moment. This data is allocated in BS memory but will be used
    // after OnExitBootServices. This is wrong and these arrays should be reallocated
    // but I am not sure
   }

   return TRUE;
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

  Prop = GetProperty (DictPointer, "AddArguments");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (Entry->Options != NULL) {
      CHAR16 *OldOptions = Entry->Options;
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
            Entry->Image = egDecodeImage (data, hex2bin (Prop->string, data, len), NULL, TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->Image     = egDecodeImage (Prop->data, Prop->dataLen, NULL, TRUE);
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
            Entry->DriveImage = egDecodeImage (data, hex2bin (Prop->string, data, len), NULL, TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->DriveImage    = egDecodeImage (Prop->data, Prop->dataLen, NULL, TRUE);
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

      Entry->CustomLogo = egDecodeImage (Prop->data, Prop->dataLen, NULL, TRUE);
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
    Entry->BootBgColor = AllocateZeroPool (sizeof(EG_PIXEL));
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
    if (AsciiStriCmp (Prop->string, "OSX") == 0) {
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

  Prop = GetProperty (DictPointer, "VolumeType");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (AsciiStriCmp (Prop->string, "Internal") == 0) {
      Entry->VolumeType = VOLTYPE_INTERNAL;
    } else if (AsciiStriCmp (Prop->string, "External") == 0) {
      Entry->VolumeType = VOLTYPE_EXTERNAL;
    } else if (AsciiStriCmp (Prop->string, "Optical") == 0) {
      Entry->VolumeType = VOLTYPE_OPTICAL;
    } else if (AsciiStriCmp (Prop->string, "FireWire") == 0) {
      Entry->VolumeType = VOLTYPE_FIREWIRE;
    }
  } else {
    INTN i;
    INTN Count = GetTagCount (Prop);
    if (Count > 0) {
      TagPtr Prop2;
      for (i = 0; i < Count; ++i) {
        if (EFI_ERROR (GetElement (Prop, i, &Prop))) {
          continue;
        }

        if (Prop2 == NULL) {
          break;
        }

        if ((Prop2->type != kTagTypeString) || (Prop2->string == NULL)) {
          continue;
        }

        if (AsciiStriCmp (Prop2->string, "Internal") == 0) {
          Entry->VolumeType |= VOLTYPE_INTERNAL;
        } else if (AsciiStriCmp (Prop2->string, "External") == 0) {
          Entry->VolumeType |= VOLTYPE_EXTERNAL;
        } else if (AsciiStriCmp (Prop2->string, "Optical") == 0) {
          Entry->VolumeType |= VOLTYPE_OPTICAL;
        } else if (AsciiStriCmp (Prop2->string, "FireWire") == 0) {
          Entry->VolumeType |= VOLTYPE_FIREWIRE;
        }
      }
    }
  }

  if (Entry->Options == NULL && OSTYPE_IS_WINDOWS(Entry->Type)) {
    Entry->Options = L"-s -h";
  }

  if (Entry->Title == NULL) {
    if (OSTYPE_IS_OSX_RECOVERY (Entry->Type)) {
      Entry->Title = PoolPrint (L"Recovery");
    } else if (OSTYPE_IS_OSX_INSTALLER (Entry->Type)) {
      Entry->Title = PoolPrint (L"Install OSX");
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
    Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_WITHKEXTS);

    Prop = GetProperty (DictPointer, "InjectKexts");
    if (Prop != NULL) {
      if (IsPropertyTrue (Prop)) {
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
    DBG ("Copying global patch settings\n");
    CopyKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)),
                             (KERNEL_AND_KEXT_PATCHES *)(((UINTN)&gSettings) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)));
    Prop = GetProperty (DictPointer, "KernelAndKextPatches");
    if (Prop != NULL) {
      FillinKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)), Prop);
      DBG ("Filled in patch settings\n");
    }
#ifdef DUMP_KERNEL_KEXT_PATCHES
    DumpKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)));
#endif
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
      CUSTOM_LOADER_ENTRY *SubEntry;
      INTN   i, Count = GetTagCount (Prop);
      TagPtr Dict = NULL;
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
      if (Count > 0) {
        for (i = 0; i < Count; ++i) {
          if (EFI_ERROR (GetElement (Prop, i, &Dict))) {
            continue;
          }
          if (Dict == NULL) {
            break;
          }
          // Allocate a sub entry
          SubEntry = DuplicateCustomEntry (Entry);
          if (SubEntry) {
            if (!FillinCustomEntry (SubEntry, Dict, TRUE) || !AddCustomSubEntry (Entry, SubEntry)) {
              FreePool (SubEntry);
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
            Entry->Image = egDecodeImage (Data, hex2bin (Prop->string, Data, Len), NULL, TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->Image     = egDecodeImage (Prop->data, Prop->dataLen, NULL, TRUE);
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
            Entry->DriveImage = egDecodeImage (data, hex2bin (Prop->string, data, len), NULL, TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->DriveImage = egDecodeImage (Prop->data, Prop->dataLen, NULL, TRUE);
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
  Prop = GetProperty (DictPointer, "VolumeType");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (AsciiStriCmp (Prop->string, "Internal") == 0) {
      Entry->VolumeType = VOLTYPE_INTERNAL;
    } else if (AsciiStriCmp (Prop->string, "External") == 0) {
      Entry->VolumeType = VOLTYPE_EXTERNAL;
    } else if (AsciiStriCmp (Prop->string, "Optical") == 0) {
      Entry->VolumeType = VOLTYPE_OPTICAL;
    } else if (AsciiStriCmp (Prop->string, "FireWire") == 0) {
      Entry->VolumeType = VOLTYPE_FIREWIRE;
    }
  } else {
    INTN i;
    INTN Count = GetTagCount (Prop);
    if (Count > 0)
    {
      TagPtr Prop2;
      for (i = 0; i < Count; ++i) {
        if (EFI_ERROR (GetElement (Prop, i, &Prop))) {
          continue;
        }
        if (Prop2 == NULL) {
          break;
        }
        if ((Prop2->type != kTagTypeString) || (Prop2->string == NULL)) {
          continue;
        }
        if (AsciiStriCmp (Prop2->string, "Internal") == 0) {
          Entry->VolumeType |= VOLTYPE_INTERNAL;
        } else if (AsciiStriCmp (Prop2->string, "External") == 0) {
          Entry->VolumeType |= VOLTYPE_EXTERNAL;
        } else if (AsciiStriCmp (Prop2->string, "Optical") == 0) {
          Entry->VolumeType |= VOLTYPE_OPTICAL;
        } else if (AsciiStriCmp (Prop2->string, "FireWire") == 0) {
          Entry->VolumeType |= VOLTYPE_FIREWIRE;
        }
      }
    }
  }
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
            Entry->Image = egDecodeImage (data, hex2bin (Prop->string, data, Len), NULL, TRUE);
          }
        }
      } else if (Prop->type == kTagTypeData) {
        Entry->Image = egDecodeImage (Prop->data, Prop->dataLen, NULL, TRUE);
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

  Prop = GetProperty (DictPointer, "VolumeType");
  if (Prop != NULL && (Prop->type == kTagTypeString)) {
    if (AsciiStriCmp (Prop->string, "Internal") == 0) {
      Entry->VolumeType = VOLTYPE_INTERNAL;
    } else if (AsciiStriCmp (Prop->string, "External") == 0) {
      Entry->VolumeType = VOLTYPE_EXTERNAL;
    } else if (AsciiStriCmp (Prop->string, "Optical") == 0) {
      Entry->VolumeType = VOLTYPE_OPTICAL;
    } else if (AsciiStriCmp (Prop->string, "FireWire") == 0) {
      Entry->VolumeType = VOLTYPE_FIREWIRE;
    }
  } else {
    INTN i;
    INTN Count = GetTagCount (Prop);
    if (Count > 0)
    {
      TagPtr Prop2;
      for (i = 0; i < Count; ++i) {
        if (EFI_ERROR (GetElement (Prop, i, &Prop))) {
          continue;
        }

        if (Prop2 == NULL) {
          break;
        }

        if ((Prop2->type != kTagTypeString) || (Prop2->string == NULL)) {
          continue;
        }

        if (AsciiStriCmp (Prop2->string, "Internal") == 0) {
          Entry->VolumeType |= VOLTYPE_INTERNAL;
        } else if (AsciiStriCmp (Prop2->string, "External") == 0) {
          Entry->VolumeType |= VOLTYPE_EXTERNAL;
        } else if (AsciiStriCmp (Prop2->string, "Optical") == 0) {
          Entry->VolumeType |= VOLTYPE_OPTICAL;
        } else if (AsciiStriCmp (Prop2->string, "FireWire") == 0) {
          Entry->VolumeType |= VOLTYPE_FIREWIRE;
        }
      }
    }
  }
  return TRUE;
}

EFI_STATUS
GetEarlyUserSettings (
  IN EFI_FILE *RootDir,
  TagPtr CfgDict
  )
{
  EFI_STATUS	Status = EFI_SUCCESS;
  TagPtr      Dict;
  TagPtr      Dict2;
  TagPtr      DictPointer;
  TagPtr      Prop;
//CHAR8       ANum[4];
//UINTN       i      = 0;

  gSettings.KextPatchesAllowed              = TRUE;
  gSettings.KernelAndKextPatches.KPAppleRTC = TRUE;
  
  Dict = CfgDict;
  if (Dict != NULL) {
    DBG ("Loading early settings\n");
    
    DictPointer = GetProperty (Dict, "Boot");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "Timeout");
      if (Prop != NULL) {
        GlobalConfig.Timeout = (INT32)GetPropertyInteger (Prop, GlobalConfig.Timeout);
        DBG ("timeout set to %d\n", GlobalConfig.Timeout);
 /*
        if (Prop->type == kTagTypeInteger) {
          GlobalConfig.Timeout = (INT32)(UINTN)Prop->string;
          DBG ("timeout set to %d\n", GlobalConfig.Timeout);
        } else if ((Prop->type == kTagTypeString) && Prop->string) {
          if (Prop->string[0] == '-') {
            GlobalConfig.Timeout = -1;
          } else {
            GlobalConfig.Timeout = (INTN)AsciiStrDecimalToUintn (Prop->string);
          }
        }
*/
      }

      Prop = GetProperty (DictPointer, "Arguments");
      if (Prop != NULL && (Prop->type == kTagTypeString) && Prop->string != NULL) {
        AsciiStrnCpy(gSettings.BootArgs, Prop->string, 255);
      }
      
      // defaults if "DefaultVolume" is not present or is empty
      gSettings.LastBootedVolume = FALSE;
      gSettings.DefaultVolume    = NULL;

      Prop = GetProperty (DictPointer, "DefaultVolume");
      if (Prop != NULL) {
        UINTN Size = AsciiStrSize (Prop->string);
        if (Size > 0) {
          // check for special value for remembering boot volume
          if (AsciiStriCmp (Prop->string, "LastBootedVolume") == 0) {
            gSettings.LastBootedVolume = TRUE;
          } else {
            gSettings.DefaultVolume    = AllocateZeroPool (Size * sizeof(CHAR16));
            AsciiStrToUnicodeStr (Prop->string, gSettings.DefaultVolume);
          }
        }
      }
      
      Prop = GetProperty (DictPointer, "DefaultLoader");
      if (Prop != NULL) {
        gSettings.DefaultLoader = AllocateZeroPool (AsciiStrSize (Prop->string) * sizeof(CHAR16));
        AsciiStrToUnicodeStr (Prop->string, gSettings.DefaultLoader);
      }
      
      Prop = GetProperty (DictPointer, "Log");
      if (IsPropertyTrue (Prop)) {
        GlobalConfig.DebugLog       = TRUE;
      }
      
      Prop = GetProperty (DictPointer, "Fast");
      if (IsPropertyTrue (Prop)) {
        GlobalConfig.FastBoot       = TRUE;
      }

      Prop = GetProperty (DictPointer, "NeverHibernate");
      if (IsPropertyTrue (Prop)) {
        GlobalConfig.NeverHibernate = TRUE;
      }
      
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
        INTN i;
        INTN Count = GetTagCount (Prop);
        if (Count > 0) {
          gSettings.SecureBootWhiteListCount = 0;
          gSettings.SecureBootWhiteList = AllocateZeroPool (Count * sizeof(CHAR16 *));
          if (gSettings.SecureBootWhiteList) {
            for (i = 0; i < Count; ++i) {
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
        INTN i;
        INTN Count = GetTagCount (Prop);
        if (Count > 0) {
          gSettings.SecureBootBlackListCount = 0;
          gSettings.SecureBootBlackList      = AllocateZeroPool (Count * sizeof(CHAR16 *));
          if (gSettings.SecureBootBlackList) {
            for (i = 0; i < Count; ++i) {
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
        AsciiStrToUnicodeStr (Prop->string, gSettings.LegacyBoot);
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

          gSettings.CustomLogo = egDecodeImage (Prop->data, Prop->dataLen, NULL, TRUE);
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
          gSettings.WithKexts            = TRUE;
          gSettings.WithKextsIfNoFakeSMC = TRUE;
        }
      }
      
      // No caches
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
      Prop = GetProperty (DictPointer, "Theme");
      if (Prop != NULL) {
        if ((Prop->type == kTagTypeString) && Prop->string) {
          GlobalConfig.Theme = PoolPrint (L"%a", Prop->string);
          DBG ("Default theme: %s\n", GlobalConfig.Theme);
        }
      }
      //CustomIcons
      Prop = GetProperty (DictPointer, "CustomIcons");
      if (IsPropertyTrue (Prop)) {
        GlobalConfig.CustomIcons = TRUE;
      }
      
      Prop = GetProperty (DictPointer, "TextOnly");
      if (IsPropertyTrue (Prop)) {
        GlobalConfig.TextOnly = TRUE;
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
//      AsciiStrToUnicodeStr (Prop->string, gSettings.Language);
        AsciiStrCpy (gSettings.Language, Prop->string);
        if (AsciiStrStr (Prop->string, "en")) {
          gLanguage = english;
        } else if (AsciiStrStr (Prop->string, "ru")) {
          gLanguage = russian;
        } else if (AsciiStrStr (Prop->string, "fr")) {
          gLanguage = french;
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
        } else if (AsciiStrStr (Prop->string, "ua")) {
          gLanguage = ukrainian;
        } else if (AsciiStrStr (Prop->string, "cz")) {
          gLanguage = czech;
        } else if (AsciiStrStr (Prop->string, "hr")) {
          gLanguage = croatian;
        } else if (AsciiStrStr (Prop->string, "id")) {
          gLanguage = indonesian;
        } else if (AsciiStrStr (Prop->string, "ko")) {
          gLanguage = korean;
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
        INTN i;
        INTN Count = GetTagCount (Prop);
        if (Count > 0) {
          gSettings.HVCount = 0;
          gSettings.HVHideStrings = AllocateZeroPool (Count * sizeof(CHAR16 *));
          if (gSettings.HVHideStrings) {
            for (i = 0; i < Count; ++i) {
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
            for (i = 0; i < Count; ++i) {
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
            for (i = 0; i < Count; ++i) {
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
            for (i = 0; i < Count; ++i) {
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
        UINTN Count = GetTagCount (Dict2);
        if (Count > 0) {
          UINTN             i           = 0;
          VBIOS_PATCH_BYTES *VBiosPatch;
          UINTN             FindSize    = 0;
          UINTN             ReplaceSize = 0;
          BOOLEAN           Valid;
          
          // alloc space for up to 16 entries
          gSettings.PatchVBiosBytes = AllocateZeroPool (Count * sizeof(VBIOS_PATCH_BYTES));
          
          // get all entries
          for (i = 0; i < Count; ++i) {
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
      
      //InjectEDID
      Prop = GetProperty (DictPointer, "InjectEDID");
      gSettings.InjectEDID            = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "CustomEDID");
      if (Prop != NULL) {
        UINTN j = 128;
        gSettings.CustomEDID   = GetDataSetting (DictPointer, "CustomEDID", &j);
        if ((j % 128) != 0) {
          DBG ("CustomEDID has wrong length=%d\n", j);
        } else {
          DBG ("CustomEDID ok\n");
          InitializeEdidOverride ();
        }
      }
    }

    DictPointer = GetProperty (Dict, "DisableDrivers");
    if (DictPointer != NULL) {
      INTN i;
      INTN Count = GetTagCount (DictPointer);
      if (Count > 0) {
        gSettings.BlackListCount = 0;
        gSettings.BlackList = AllocateZeroPool (Count * sizeof(CHAR16 *));

        for (i = 0; i < Count; ++i) {
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
  }

  return Status;
}
#define CONFIG_THEME_FILENAME L"theme.plist"

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

  ThemesNum = 0;
  DirIterOpen (SelfRootDir, L"\\EFI\\CLOVER\\themes", &DirIter);
  while (DirIterNext(&DirIter, 1, L"*.EFI", &DirEntry)) {
    if (DirEntry->FileName[0] == '.'){
      continue;
    }

    DBG ("Found theme %s", DirEntry->FileName);
    ThemeTestPath = PoolPrint (L"EFI\\CLOVER\\themes\\%s", DirEntry->FileName);
    if (ThemeTestPath != NULL) {
      Status = SelfRootDir->Open (SelfRootDir, &ThemeTestDir, ThemeTestPath, EFI_FILE_MODE_READ, 0);
      if (!EFI_ERROR (Status)) {
        Status = egLoadFile (ThemeTestDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
        if (EFI_ERROR (Status) || (ThemePtr == NULL) || (Size == 0)) {
          Status = EFI_NOT_FOUND;
          DBG (" - no theme.plist");
        } else {
          //we found a theme
          ThemesList[ThemesNum++] = (CHAR16*)AllocateCopyPool (StrSize (DirEntry->FileName), DirEntry->FileName);
        }
      }

      FreePool (ThemeTestPath);
    }

    DBG ("\n");
  }

  DirIterClose (&DirIter);
}

STATIC
EFI_STATUS
GetThemeTagSettings (
  TagPtr DictPointer
  )
{
  TagPtr Dict;
  TagPtr Dict2;
  
  //fill default to have an ability change theme
  GlobalConfig.BackgroundScale = Crop;

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
  row0TileSize = 144;
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

  GlobalConfig.SelectionOnTop           = FALSE;
  ScrollWidth                           = 16;
  ScrollButtonsHeight                   = 20;
  ScrollBarDecorationsHeight            = 5;
  ScrollScrollDecorationsHeight         = 7;
  GlobalConfig.Font                     = FONT_LOAD;
  if (GlobalConfig.FontFileName != NULL) {
    FreePool (GlobalConfig.FontFileName);
    GlobalConfig.FontFileName          = NULL;
  }
  GlobalConfig.CharWidth               = 7;
  GuiAnime = NULL;

  if (BigBack != NULL) {
    egFreeImage (BigBack);
    BigBack = NULL;
  }

  if (BackgroundImage != NULL) {
    egFreeImage (BackgroundImage);
    BackgroundImage = NULL;
  }

  if (Banner != NULL) {
    egFreeImage (Banner);
    Banner  = NULL;
  }

  if (FontImage != NULL) {
    egFreeImage (FontImage);
    FontImage = NULL;
  }
  FreeScrollBar();
  
  // if NULL parameter, quit after setting default values
  if (DictPointer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Dict    = GetProperty (DictPointer, "Background");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Type");
    if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
      if ((Dict2->string[0] == 'S') || (Dict2->string[0] == 's')) {
        GlobalConfig.BackgroundScale = Scale;
      } else if ((Dict2->string[0] == 'T') || (Dict2->string[0] == 't')) {
        GlobalConfig.BackgroundScale = Tile;
      }
    }
  }

  Dict2 = GetProperty (Dict, "Path");
  if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
    GlobalConfig.BackgroundName = PoolPrint (L"%a", Dict2->string);
  }

  Dict2 = GetProperty (Dict, "Sharp");
  if (Dict2 != NULL) {
    GlobalConfig.BackgroundSharp  = (INT32)GetPropertyInteger (Dict2, GlobalConfig.BackgroundSharp);
  }

  Dict2 = GetProperty (Dict, "Dark");
  if (Dict2 != NULL) {
    GlobalConfig.BackgroundDark   = (Dict2->type == kTagTypeTrue);
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
      if (Dict2 != NULL) {
        GlobalConfig.BannerPosX   = (INT32)GetPropertyInteger (Dict2, 0);
      }

      Dict2 = GetProperty (Dict, "DistanceFromScreenEdgeY%");
      if (Dict2 != NULL) {
        GlobalConfig.BannerPosY   = (INT32)GetPropertyInteger (Dict2, 0);
      }

      Dict2 = GetProperty (Dict, "NudgeX");
      if (Dict2 != NULL) {
        GlobalConfig.BannerNudgeX = (INT32)GetPropertyInteger (Dict2, 0);
      }

      Dict2 = GetProperty (Dict, "NudgeY");
      if (Dict2 != NULL) {
        GlobalConfig.BannerNudgeY = (INT32)GetPropertyInteger (Dict2, 0);
      }
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
    if (Dict2 != NULL) {
      GlobalConfig.BadgeOffsetX = (INTN)GetPropertyInteger (Dict2, GlobalConfig.BadgeOffsetX);
    }

    Dict2 = GetProperty (Dict, "OffsetY");
    if (Dict2 != NULL) {
      GlobalConfig.BadgeOffsetY = (INTN)GetPropertyInteger (Dict2, GlobalConfig.BadgeOffsetY);
    }

    Dict2 = GetProperty (Dict, "Scale");
    if (Dict2 != NULL) {
      GlobalConfig.BadgeScale = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.BadgeScale);
    }
  }
  
  Dict = GetProperty (DictPointer, "Origination");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "DesignWidth");
    if (Dict2 != NULL) {
      GlobalConfig.ThemeDesignWidth = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.ThemeDesignWidth);
    }

    Dict2 = GetProperty (Dict, "DesignHeight");
    if (Dict2 != NULL) {
      GlobalConfig.ThemeDesignHeight = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.ThemeDesignHeight);
    }
  }
  
  Dict = GetProperty (DictPointer, "Layout");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "BannerOffset");
    if (Dict2 != NULL) {
      LayoutBannerOffset = (UINTN)GetPropertyInteger (Dict2, LayoutBannerOffset);
    }

    Dict2 = GetProperty (Dict, "ButtonOffset");
    if (Dict2 != NULL) {
      LayoutButtonOffset = (UINTN)GetPropertyInteger (Dict2, LayoutButtonOffset);
    }

    Dict2 = GetProperty (Dict, "TextOffset");
    if (Dict2 != NULL) {
      LayoutTextOffset = (UINTN)GetPropertyInteger (Dict2, LayoutTextOffset);
    }

    Dict2 = GetProperty (Dict, "AnimAdjustForMenuX");
    if (Dict2 != NULL) {
       LayoutAnimMoveForMenuX = (UINTN)GetPropertyInteger (Dict2, LayoutAnimMoveForMenuX);
    }

    Dict2 = GetProperty (Dict, "Vertical");
    if (Dict2 && Dict2->type == kTagTypeTrue) {
      GlobalConfig.VerticalLayout = TRUE;
    }

    // GlobalConfig.MainEntriesSize
    Dict2 = GetProperty (Dict, "MainEntriesSize");
    if (Dict2 != NULL) {
      GlobalConfig.MainEntriesSize = (INT32)GetPropertyInteger (Dict2, GlobalConfig.MainEntriesSize);
    }

    Dict2 = GetProperty (Dict, "TileXSpace");
    if (Dict2 != NULL) {
      GlobalConfig.TileXSpace = (INT32)GetPropertyInteger (Dict2, GlobalConfig.TileXSpace);
    }

    Dict2 = GetProperty (Dict, "TileYSpace");
    if (Dict2 != NULL) {
      GlobalConfig.TileYSpace = (INT32)GetPropertyInteger (Dict2, GlobalConfig.TileYSpace);
    }

    Dict2 = GetProperty (Dict, "SelectionBigWidth");
    if (Dict2 != NULL) {
      row0TileSize = (INT32)GetPropertyInteger (Dict2, row0TileSize);
    }
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
    if ((Dict2->type == kTagTypeString) && Dict2->string) {
      GlobalConfig.SelectionSmallFileName = PoolPrint (L"%a", Dict2->string);
    }

    Dict2 = GetProperty (Dict, "Big");
    if ((Dict2->type == kTagTypeString) && Dict2->string) {
      GlobalConfig.SelectionBigFileName = PoolPrint (L"%a", Dict2->string);
    }

    Dict2 = GetProperty (Dict, "OnTop");
    if (IsPropertyTrue (Dict2)) {
      GlobalConfig.SelectionOnTop = TRUE;
    }

    Dict2 = GetProperty (Dict, "ChangeNonSelectedGrey");
    if (IsPropertyTrue (Dict2)) {
      GlobalConfig.NonSelectedGrey = TRUE;
    }
  }
  
  Dict = GetProperty (DictPointer, "Scroll");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Width");
    ScrollWidth = (UINTN)GetPropertyInteger (Dict2,                   ScrollWidth);
    Dict2 = GetProperty (Dict, "Height");
    ScrollButtonsHeight = (UINTN)GetPropertyInteger (Dict2,           ScrollButtonsHeight);
    Dict2 = GetProperty (Dict, "BarHeight");
    ScrollBarDecorationsHeight = (UINTN)GetPropertyInteger (Dict2,    ScrollBarDecorationsHeight);
    Dict2 = GetProperty (Dict, "ScrollHeight");
    ScrollScrollDecorationsHeight = (UINTN)GetPropertyInteger (Dict2, ScrollScrollDecorationsHeight);
  }
  
  Dict = GetProperty (DictPointer, "Font");
  if (Dict != NULL) {
    Dict2 = GetProperty (Dict, "Type");
    if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
      if ((Dict2->string[0] == 'A') || (Dict2->string[0] == 'a')) {
        GlobalConfig.Font = FONT_ALFA;
      } else if ((Dict2->string[0] == 'G') || (Dict2->string[0] == 'g')) {
        GlobalConfig.Font = FONT_GRAY;
      } else if ((Dict2->string[0] == 'L') || (Dict2->string[0] == 'l')) {
        GlobalConfig.Font = FONT_LOAD;
      }
    }
    Dict2 = GetProperty (Dict, "Path");
    if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
      GlobalConfig.FontFileName = PoolPrint (L"%a", Dict2->string);
    }

    Dict2 = GetProperty (Dict, "CharWidth");
    if (Dict2 != NULL) {
      GlobalConfig.CharWidth = (UINTN)GetPropertyInteger (Dict2, GlobalConfig.CharWidth);
    }
  }
  
  Dict = GetProperty (DictPointer, "Anime");
  if (Dict != NULL) {
    INTN i;
    INTN Count = GetTagCount (Dict);
    for (i = 0; i < Count; ++i) {
      GUI_ANIME *Anime;
      if (EFI_ERROR (GetElement (Dict, i, &DictPointer))) {
        continue;
      }

      if (DictPointer == NULL) {
        break;
      }

      Anime = AllocateZeroPool (sizeof(GUI_ANIME));
      if (Anime == NULL) {
        break;
      }

      Dict2 = GetProperty (DictPointer, "ID");
      if (Dict2 != NULL) {
        Anime->ID = (UINTN)GetPropertyInteger (Dict2, Anime->ID);
      }

      Dict2 = GetProperty (DictPointer, "Path");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        Anime->Path = PoolPrint (L"%a", Dict2->string);
      }

      Dict2 = GetProperty (DictPointer, "Frames");
      if (Dict2 != NULL) {
        Anime->Frames = (UINTN)GetPropertyInteger (Dict2, Anime->Frames);
      }

      Dict2 = GetProperty (DictPointer, "FrameTime");
      if (Dict2 != NULL) {
        Anime->FrameTime = (UINTN)GetPropertyInteger (Dict2, Anime->FrameTime);
      }
      
      Dict2 = GetProperty (DictPointer, "ScreenEdgeX");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        if (AsciiStrCmp (Dict2->string, "left") == 0) {
          Anime->ScreenEdgeHorizontal = SCREEN_EDGE_LEFT;
        } else if (AsciiStrCmp (Dict2->string, "right") == 0) {
          Anime->ScreenEdgeHorizontal = SCREEN_EDGE_RIGHT;
        }
      }

      Dict2 = GetProperty (DictPointer, "ScreenEdgeY");
      if (Dict2 != NULL && (Dict2->type == kTagTypeString) && Dict2->string) {
        if (AsciiStrCmp (Dict2->string, "top") == 0) {
          Anime->ScreenEdgeVertical = SCREEN_EDGE_TOP;
        } else if (AsciiStrCmp (Dict2->string, "bottom") == 0) {
          Anime->ScreenEdgeVertical = SCREEN_EDGE_BOTTOM;
        }
      }
      
      //default value is centre
      Anime->FilmX  = INITVALUE;
      Anime->FilmY  = INITVALUE;
      Anime->NudgeX = INITVALUE;
      Anime->NudgeY = INITVALUE;
      
      Dict2 = GetProperty (DictPointer, "DistanceFromScreenEdgeX%");
      if (Dict2 != NULL) {
        Anime->FilmX = (INT32)GetPropertyInteger (Dict2, Anime->FilmX);
      }

      Dict2 = GetProperty (DictPointer, "DistanceFromScreenEdgeY%");
      if (Dict2 != NULL) {
        Anime->FilmY = (INT32)GetPropertyInteger (Dict2, Anime->FilmY);
      }
      
      Dict2 = GetProperty (DictPointer, "NudgeX");
      if (Dict2 != NULL) {
        Anime->NudgeX = (INT32)GetPropertyInteger (Dict2, Anime->NudgeX);
      }

      Dict2 = GetProperty (DictPointer, "NudgeY");
      if (Dict2 != NULL) {
        Anime->NudgeY = (INT32)GetPropertyInteger (Dict2, Anime->NudgeY);
      }
      
      Dict2 = GetProperty (DictPointer, "Once");
      if (Dict2 != NULL) {
        if (IsPropertyTrue (Dict2)) {
          Anime->Once = TRUE;
        }
      }

      // Add the anime to the list
      if (Anime != NULL) {
        if ((Anime->ID == 0) || (Anime->Path == NULL)) {
          FreePool (Anime);
        } else if (GuiAnime != NULL) {
          if (GuiAnime->ID == Anime->ID) {
            Anime->Next = GuiAnime->Next;
            FreeAnime (GuiAnime);
          } else {
            GUI_ANIME *Ptr = GuiAnime;
            while (Ptr->Next) {
              if (Ptr->Next->ID == Anime->ID) {
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
          GuiAnime      = Anime;
        }
      }
    }
  }
  
  // set file defaults in case they were not set
  if (GlobalConfig.BackgroundName == NULL) {
    GlobalConfig.BackgroundName         = PoolPrint (L"background.png");
  }
  if (GlobalConfig.BannerFileName == NULL) {
    GlobalConfig.BannerFileName         = PoolPrint (L"logo.png");
  }
  if (GlobalConfig.SelectionSmallFileName == NULL) {
    GlobalConfig.SelectionSmallFileName = PoolPrint (L"selection_small.png");
  }
  if (GlobalConfig.SelectionBigFileName == NULL) {
    GlobalConfig.SelectionBigFileName   = PoolPrint (L"selection_big.png");
  }
  
  return EFI_SUCCESS;
}

TagPtr
LoadTheme (
  CHAR16 *TestTheme
  )
{
  EFI_STATUS Status    = EFI_UNSUPPORTED;
  TagPtr     ThemeDict = NULL;
  CHAR8      *ThemePtr = NULL;
  UINTN      Size      = 0;

  if (TestTheme != NULL) {
    if (ThemePath != NULL) {
      FreePool (ThemePath);
    }

    ThemePath = PoolPrint (L"EFI\\CLOVER\\themes\\%s", TestTheme);
    if (ThemePath != NULL) {
      if (ThemeDir != NULL) {
        ThemeDir->Close (ThemeDir);
        ThemeDir = NULL;
      }

      Status = SelfRootDir->Open (SelfRootDir, &ThemeDir, ThemePath, EFI_FILE_MODE_READ, 0);
      if (!EFI_ERROR (Status)) {
        Status = egLoadFile (ThemeDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
        if (!EFI_ERROR (Status) && (ThemePtr != NULL) && (Size != 0)) {
          Status = ParseXML ((const CHAR8*)ThemePtr, &ThemeDict, 0);

          if (EFI_ERROR (Status)) {
            ThemeDict = NULL;
          }

          if (ThemeDict == NULL) {
            DBG ("xml file %s not parsed\n", CONFIG_THEME_FILENAME);
          } else {
            DBG ("Using theme '%s' (%s)\n", TestTheme, ThemePath);
          }
        }

        if (ThemePtr != NULL) {
          FreePool (ThemePtr);
        }
      }
    }
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
  
  Rnd = Time != NULL && ThemesNum != 0 ? Time->Second % ThemesNum : 0;
  
  // Invalidated BuiltinIcons
  DBG ("Invalidating BuiltinIcons...\n");

  for (i = 0; i < BUILTIN_ICON_COUNT; ++i) {
    if (BuiltinIconTable[i].Image != NULL) {
      egFreeImage (BuiltinIconTable[i].Image);
      BuiltinIconTable[i].Image = NULL;
    }    
  }

  for (i = 0; i < 4; ++i) {
    if (SelectionImages[i] != NULL) {
      egFreeImage (SelectionImages[i]);
      SelectionImages[i] = NULL;
    }
  } 
  
  KillMouse();
  
  while (GuiAnime != NULL) {
    GUI_ANIME *NextAnime = GuiAnime->Next;
    FreeAnime (GuiAnime);
    GuiAnime             = NextAnime;
  }
  
  if (ThemesNum > 0 &&
      (!GlobalConfig.Theme ||
       StrCmp (GlobalConfig.Theme, L"embedded") != 0)) {
    
    // Try special theme first
    if (Time != NULL) {
      if ((Time->Month == 12) && ((Time->Day >= 25) && (Time->Day <= 31))) {
         TestTheme = PoolPrint (L"christmas");
      } else if ((Time->Month == 1) && ((Time->Day >= 1) && (Time->Day <= 7))) {
         TestTheme = PoolPrint (L"newyear");
      }

      if (TestTheme != NULL) {
        ThemeDict = LoadTheme (TestTheme);
        if (ThemeDict != NULL) {
          DBG ("special theme %s found and %s parsed\n", TestTheme, CONFIG_THEME_FILENAME);
          if (GlobalConfig.Theme) {
            FreePool (GlobalConfig.Theme);
          }

          GlobalConfig.Theme = TestTheme;
        } else { // special theme not loaded
          DBG ("special theme %s not found, skipping\n", TestTheme, CONFIG_THEME_FILENAME);
          FreePool (TestTheme);
        }

        TestTheme = NULL;
      } else {
        //shuffle
        if (StrCmp (GlobalConfig.Theme, L"random") == 0) {
          ThemeDict = LoadTheme (ThemesList[Rnd]);
        }
      }
    }
    
    // Try theme from nvram  
    if (ThemeDict == NULL && UseThemeDefinedInNVRam) {
      ChosenTheme   = GetNvramVariable(L"Clover.Theme", &gEfiAppleBootGuid, NULL, &Size);
      if (ChosenTheme != NULL) {
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
          DBG ("no default theme, get random theme %s\n", CONFIG_THEME_FILENAME, ThemesList[Rnd]);
        } else {
          DBG ("no default theme, get first theme %s\n",  CONFIG_THEME_FILENAME, ThemesList[0]);
        }
      } else {
        ThemeDict = LoadTheme (GlobalConfig.Theme);
        if (ThemeDict == NULL) {
          DBG ("GlobalConfig: %s not found, get random theme %s\n", CONFIG_THEME_FILENAME, ThemesList[Rnd]);
          FreePool (GlobalConfig.Theme);
          GlobalConfig.Theme = NULL; 
        }
      }
    }
  
    // Try to get a theme
    if (ThemeDict == NULL) {
      ThemeDict = LoadTheme (ThemesList[Rnd]);
      if (ThemeDict != NULL) {
        GlobalConfig.Theme = AllocateCopyPool (StrSize (ThemesList[Rnd]), ThemesList[Rnd]);
      }
    }

  } // ThemesNum>0
  
  // No theme could be loaded, use embedded
  if (!ThemeDict) {
    DBG ("no themes available, using embedded\n");
    GlobalConfig.Theme = NULL;
    if (ThemePath != NULL) {
      FreePool (ThemePath);
      ThemePath = NULL;
    }

    if (ThemeDir != NULL) {
      ThemeDir->Close (ThemeDir);
      ThemeDir = NULL;
    }

    // set default values
    GetThemeTagSettings(NULL);
    //fill some fields
    GlobalConfig.SelectionColor = 0xA0A0A080;
    GlobalConfig.Font = FONT_ALFA; //to be inverted later
    GlobalConfig.HideBadges |= HDBADGES_SHOW;
    GlobalConfig.BadgeScale = 16;
  } else { // theme loaded successfully
    // read theme settings
    TagPtr DictPointer = GetProperty (ThemeDict, "Theme");
    if (DictPointer != NULL) {
      Status = GetThemeTagSettings(DictPointer);
      if (EFI_ERROR (Status)) {
         DBG ("Config theme error: %r\n", Status);
      }
    }

    FreeTag(ThemeDict);
  }
  
  PrepareFont();
  
  return Status;
}

VOID
ParseSMBIOSSettings(
  TagPtr DictPointer
  )
{
  CHAR16 UStr[64];
  TagPtr Prop = GetProperty (DictPointer, "ProductName");
  if (Prop != NULL) {
    MACHINE_TYPES Model;
    AsciiStrCpy (gSettings.ProductName, Prop->string);
    // let's fill all other fields based on this ProductName
    // to serve as default
    Model = GetModelFromString (gSettings.ProductName);
    if (Model != MaxMachineType) {
      SetDMISettingsForModel (Model);
    }
  }

  Prop = GetProperty (DictPointer, "BiosVendor");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.VendorName, Prop->string);
  }

  Prop = GetProperty (DictPointer, "BiosVersion");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.RomVersion, Prop->string);
  }

  Prop = GetProperty (DictPointer, "BiosReleaseDate");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.ReleaseDate, Prop->string);
  }

  Prop = GetProperty (DictPointer, "Manufacturer");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.ManufactureName, Prop->string);
  }

  Prop = GetProperty (DictPointer, "Version");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.VersionNr, Prop->string);
  }

  Prop = GetProperty (DictPointer, "Family");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.FamilyName, Prop->string);
  }

  Prop = GetProperty (DictPointer, "SerialNumber");
  if (Prop != NULL) {
    ZeroMem(gSettings.SerialNr, 64);
    AsciiStrCpy (gSettings.SerialNr, Prop->string);
  }

  Prop = GetProperty (DictPointer, "SmUUID");
  if (Prop != NULL) {
    if (IsValidGuidAsciiString (Prop->string)) {
      AsciiStrToUnicodeStr (Prop->string, (CHAR16*)&UStr[0]);
      StrToGuidLE ((CHAR16*)&UStr[0], &gSettings.SmUUID);
      gSettings.SmUUIDConfig = TRUE;
    } else {
      DBG ("Error: invalid SmUUID '%a' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->string);
    }
  }

  Prop = GetProperty (DictPointer, "BoardManufacturer");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.BoardManufactureName, Prop->string);
  }

  Prop = GetProperty (DictPointer, "BoardSerialNumber");
  if (Prop != NULL && AsciiStrLen (Prop->string) > 0) {
    AsciiStrCpy (gSettings.BoardSerialNumber, Prop->string);
    gSettings.BoardSNConfig = TRUE;
  }

  Prop = GetProperty (DictPointer, "Board-ID");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.BoardNumber, Prop->string);
  }

  Prop = GetProperty (DictPointer, "BoardVersion");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.BoardVersion, Prop->string);
  }

  Prop = GetProperty (DictPointer, "BoardType");
  if (Prop != NULL) {
    gSettings.BoardType = (UINT8)GetPropertyInteger (Prop, gSettings.BoardType);
  }

  Prop = GetProperty (DictPointer, "Mobile");
  if (Prop != NULL) {
      gSettings.Mobile = IsPropertyFalse(Prop);
  }

  Prop = GetProperty (DictPointer, "LocationInChassis");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.LocationInChassis, Prop->string);
  }

  Prop = GetProperty (DictPointer, "ChassisManufacturer");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.ChassisManufacturer, Prop->string);
  }

  Prop = GetProperty (DictPointer, "ChassisAssetTag");
  if (Prop != NULL) {
    AsciiStrCpy (gSettings.ChassisAssetTag, Prop->string);
  }

  Prop = GetProperty (DictPointer, "ChassisType");
  if (Prop != NULL) {
    gSettings.ChassisType = (UINT8)GetPropertyInteger (Prop, gSettings.ChassisType);
    DBG ("Config set ChassisType=0x%x\n", gSettings.ChassisType);
  }
  //gFwFeatures = 0xC0001403 - by default
  Prop = GetProperty (DictPointer, "FirmwareFeatures");
  if (Prop != NULL) {
    gFwFeatures       = (UINT32)GetPropertyInteger (Prop, gFwFeatures);
    gFwFeaturesConfig = TRUE;
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
  TagPtr     DictPointer;
  UINTN      i;
  
  Dict              = CfgDict;
  if (Dict != NULL) {
    DBG ("Loading main settings\n");
    //Graphics
    
    DictPointer = GetProperty (Dict, "Graphics");
    if (DictPointer != NULL) {
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
      
      Prop = GetProperty (DictPointer, "VRAM");
      if (Prop != NULL) {
        gSettings.VRAM = LShiftU64((UINTN)GetPropertyInteger (Prop, 0), 20); //Mb -> bytes
      }
      //
      gSettings.RefCLK = 0;
      Prop = GetProperty (DictPointer, "RefCLK");

      if (Prop != NULL) {
        gSettings.RefCLK = (UINT16)GetPropertyInteger (Prop, 0);
      }
      
      Prop = GetProperty (DictPointer, "LoadVBios");
      gSettings.LoadVBios      = FALSE;

      if (Prop != NULL && IsPropertyTrue (Prop)) {
        gSettings.LoadVBios    = TRUE;
      }

      for (i = 0; i < NGFX; ++i) {
        gGraphics[i].LoadVBios = gSettings.LoadVBios; //default
      }

      Prop = GetProperty (DictPointer, "VideoPorts");
      if (Prop != NULL) {
        gSettings.VideoPorts   = (UINT16)GetPropertyInteger (Prop, 0);
      }

      Prop = GetProperty (DictPointer, "FBName");
      if (Prop != NULL) {
        AsciiStrToUnicodeStr (Prop->string, gSettings.FBName);
      }

      Prop = GetProperty (DictPointer, "NVCAP");
      if (Prop != NULL) {
        hex2bin (Prop->string, (UINT8*)&gSettings.NVCAP[0], 20);
        DBG ("Read NVCAP:");

        for (i = 0; i<20; ++i) {
          DBG ("%02x", gSettings.NVCAP[i]);
        }

        DBG ("\n");
        //thus confirmed this procedure is working
      }

      Prop = GetProperty (DictPointer, "display-cfg");
      if (Prop != NULL) {
        hex2bin (Prop->string, (UINT8*)&gSettings.Dcfg[0], 8);
      }
      //
      Prop = GetProperty (DictPointer, "DualLink");
      if (Prop != NULL) {
        gSettings.DualLink = (UINT32)GetPropertyInteger (Prop, 0);
      }
      //InjectEDID - already done in earlysettings
      
      Prop = GetProperty (DictPointer, "ig-platform-id");
      if (Prop != NULL) {
        gSettings.IgPlatform = (UINT32)GetPropertyInteger (Prop, 0);
      }
    }
    
    DictPointer = GetProperty (Dict, "Devices");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "Inject");
      gSettings.StringInjector = IsPropertyTrue (Prop);

      Prop = GetProperty (DictPointer, "Properties");
      if (Prop != NULL) {
        EFI_PHYSICAL_ADDRESS  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
        UINTN strlength   = AsciiStrLen (Prop->string);
        cDeviceProperties = AllocateZeroPool (strlength + 1);
        AsciiStrCpy (cDeviceProperties, Prop->string);
        //-------
        Status = gBS->AllocatePages (
                        AllocateMaxAddress,
                        EfiACPIReclaimMemory,
                        EFI_SIZE_TO_PAGES (strlength) + 1,
                        &BufferPtr
                        );

        if (!EFI_ERROR (Status)) {
          cProperties = (UINT8*)(UINTN)BufferPtr;
          cPropSize   = (UINT32)(strlength >> 1);
          cPropSize   = hex2bin (cDeviceProperties, cProperties, cPropSize);
          DBG ("Injected EFIString of length %d\n", cPropSize);
        }
        //---------
      }

      Prop                          = GetProperty (DictPointer, "NoDefaultProperties");
      gSettings.NoDefaultProperties = IsPropertyTrue (Prop);
      
      Prop = GetProperty (DictPointer, "AddProperties");
      if (Prop != NULL) {
        INTN i;
        INTN Index;
        INTN Count = GetTagCount (Prop);

        Index = 0;  //begin from 0 if second enter
        if (Count > 0) {
          DBG ("Add %d properties\n", Count);
          gSettings.AddProperties = AllocateZeroPool (Count * sizeof(DEV_PROPERTY));
          
          for (i = 0; i < Count; ++i) {
            UINTN Size = 0;
            if (EFI_ERROR (GetElement (Prop, i, &Dict2))) {
              DBG ("AddProperties continue at %d\n", i);
              continue;
            }

            if (Dict2 == NULL) {
              DBG ("AddProperties break at %d\n", i);
              break;
            }

            Prop2 = GetProperty (Dict2, "Device");
            if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
              DEV_PROPERTY *Property = &gSettings.AddProperties[Index];

              if (AsciiStriCmp (Prop2->string,        "ATI") == 0) {
                Property->Device = DEV_ATI;
              } else if (AsciiStriCmp (Prop2->string, "NVidia") == 0) {
                Property->Device = DEV_NVIDIA;
              } else if (AsciiStriCmp (Prop2->string, "IntelGFX") == 0) {
                Property->Device = DEV_INTEL;
              } else if (AsciiStriCmp (Prop2->string, "LAN") == 0) {
                Property->Device = DEV_LAN;
              } else if (AsciiStriCmp (Prop2->string, "WIFI") == 0) {
                Property->Device = DEV_WIFI;
              } else if (AsciiStriCmp (Prop2->string, "Firewire") == 0) {
                Property->Device = DEV_FIREWIRE;
              } else if (AsciiStriCmp (Prop2->string, "SATA") == 0) {
                Property->Device = DEV_SATA;
              } else if (AsciiStriCmp (Prop2->string, "IDE") == 0) {
                Property->Device = DEV_IDE;
              } else if (AsciiStriCmp (Prop2->string, "HDA") == 0) {
                Property->Device = DEV_HDA;
              } else if (AsciiStriCmp (Prop2->string, "HDMI") == 0) {
                Property->Device = DEV_HDMI;
              } else if (AsciiStriCmp (Prop2->string, "LPC") == 0) {
                Property->Device = DEV_LPC;
              } else if (AsciiStriCmp (Prop2->string, "SmBUS") == 0) {
                Property->Device = DEV_SMBUS;
              } else if (AsciiStriCmp (Prop2->string, "USB") == 0) {
                Property->Device = DEV_USB;
              } else {
                DBG (" add properties to unknown device, ignored\n");
                continue;
              }
            }
            
            Prop2 = GetProperty (Dict2, "Key");
            if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
              gSettings.AddProperties[Index].Key = AllocateCopyPool (AsciiStrSize (Prop2->string), Prop2->string);
            }

            Prop2 = GetProperty (Dict2, "Value");
            if (Prop2 && (Prop2->type == kTagTypeString) && Prop2->string) {
              //first suppose it is Ascii string
              gSettings.AddProperties[Index].Value = AllocateCopyPool (AsciiStrSize (Prop2->string), Prop2->string);
              gSettings.AddProperties[Index].ValueLen = AsciiStrLen (Prop2->string) + 1;
            } else if (Prop2 && (Prop2->type == kTagTypeInteger)) {
              gSettings.AddProperties[Index].Value = AllocatePool (4);
              CopyMem (gSettings.AddProperties[Index].Value, &(Prop2->string), 4);
              gSettings.AddProperties[Index].ValueLen = 4;
            } else {
              //else  data
              gSettings.AddProperties[Index].Value = GetDataSetting (Dict2, "Value", &Size);
              gSettings.AddProperties[Index].ValueLen = Size;
            }

            ++Index;
          }

          gSettings.NrAddProperties = Index;
        }
      }
      
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
            } else {
              // assume it's a decimal layout id
              gSettings.HDALayoutId = (INT32)AsciiStrDecimalToUintn (Prop->string);
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
        if (Prop != NULL) {
          // enabled by default
          if (IsPropertyFalse (Prop)) {
            gSettings.USBInjection = FALSE;
          }
        }
        Prop = GetProperty (Prop2, "AddClockID");
        if (Prop != NULL) {
          // disabled by default
          if (IsPropertyFalse (Prop))
            gSettings.InjectClockID = FALSE;
          else if (IsPropertyTrue (Prop))
            gSettings.InjectClockID = TRUE;
        }
        // enabled by default for CloverEFI
        // disabled for others
        gSettings.USBFixOwnership = gFirmwareClover;
        Prop = GetProperty (Prop2, "FixOwnership");
        if (Prop != NULL) {
          if (IsPropertyFalse (Prop))
            gSettings.USBFixOwnership = FALSE;
          else if (IsPropertyTrue (Prop)) {
            gSettings.USBFixOwnership = TRUE;
            DBG ("USB FixOwnership: true\n");
          }
        }
        Prop = GetProperty (Prop2, "HighCurrent");
        if (Prop != NULL) {
          // disabled by default
          if (IsPropertyFalse (Prop))
            gSettings.HighCurrent = FALSE;
          else if (IsPropertyTrue (Prop))
            gSettings.HighCurrent = TRUE;
        }
      }
    }
    
    //*** ACPI ***//
    
    DictPointer = GetProperty (Dict, "ACPI");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "DropTables");
      if (Prop != NULL) {
        INTN i;
        INTN Count = GetTagCount (Prop);

        if (Count > 0) {
          DBG ("Dropping %d tables\n", Count);

          for (i = 0; i < Count; ++i) {
            UINT32 Signature = 0;
            UINT32 TabLength = 0;
            UINT64 TableId = 0;

            if (EFI_ERROR (GetElement (Prop, i, &Dict2))) {
              DBG ("Drop table continue at %d\n", i);
              continue;
            }

            if (Dict2 == NULL) {
              DBG ("Drop table break at %d\n", i);
              break;
            }

            DBG ("Drop table %d", i);
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
              DBG ("set table: %08x, %16lx to drop:", Signature, TableId);
              while (DropTable) {
                if (((Signature == DropTable->Signature) &&
                     (!TableId || (DropTable->TableId == TableId)) &&
                     (!TabLength || (DropTable->Length == TabLength))) ||
                    (!Signature && (DropTable->TableId == TableId))) {
                  DropTable->MenuItem.BValue = TRUE;
                  gSettings.DropSSDT         = FALSE; //if one item=true then dropAll=false by default
                  DBG ("  true\n");
                }

                DropTable = DropTable->Next;
              }
              DBG ("\n");
            }
          }
        }
      }
      
      Dict2 = GetProperty (DictPointer, "DSDT");
      if (Dict2 != NULL) {
        //gSettings.DsdtName by default is "DSDT.aml", but name "BIOS" will mean autopatch
        Prop = GetProperty (Dict2, "Name");
        if (Prop != NULL) {
          AsciiStrToUnicodeStr (Prop->string, gSettings.DsdtName);
        }

        Prop = GetProperty (Dict2, "Debug");
        if (Prop != NULL && IsPropertyTrue (Prop)) {
          gSettings.DebugDSDT = TRUE;
        }

        Prop = GetProperty (Dict2, "Rtc8Allowed");
        if (Prop != NULL && IsPropertyTrue (Prop)) {
          gSettings.Rtc8Allowed = TRUE;
        }
        
        Prop = GetProperty (Dict2, "FixMask");
        if (Prop != NULL) {
          gSettings.FixDsdt = (UINT32)GetPropertyInteger (Prop, gSettings.FixDsdt);
          DBG ("Config set Fix DSDT mask=%08x\n", gSettings.FixDsdt);
        }

        Prop = GetProperty (Dict2, "Fixes");
        if (Prop != NULL) {
          DBG ("Config set Fixes will override FixMask mask!\n");

          if (Prop->type == kTagTypeDict) {
            gSettings.FixDsdt = 0;

            Prop2 = GetProperty (Prop, "AddDTGP_0001");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_DTGP;
            }

            Prop2 = GetProperty (Prop, "FixDarwin_0002");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_WARNING;
            }

            Prop2 = GetProperty (Prop, "FixShutdown_0004");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_SHUTDOWN;
            }

            Prop2 = GetProperty (Prop, "AddMCHC_0008");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_MCHC;
            }

            Prop2 = GetProperty (Prop, "FixHPET_0010");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_HPET;
            }

            Prop2 = GetProperty (Prop, "FakeLPC_0020");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_LPC;
            }

            Prop2 = GetProperty (Prop, "FixIPIC_0040");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_IPIC;
            }

            Prop2 = GetProperty (Prop, "FixSBUS_0080");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_SBUS;
            }

            Prop2 = GetProperty (Prop, "FixDisplay_0100");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_DISPLAY;
            }

            Prop2 = GetProperty (Prop, "FixIDE_0200");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_IDE;
            }

            Prop2 = GetProperty (Prop, "FixSATA_0400");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_SATA;
            }

            Prop2 = GetProperty (Prop, "FixFirewire_0800");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_FIREWIRE;
            }

            Prop2 = GetProperty (Prop, "FixUSB_1000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_USB;
            }

            Prop2 = GetProperty (Prop, "FixLAN_2000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_LAN;
            }

            Prop2 = GetProperty (Prop, "FixAirport_4000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_WIFI;
            }

            Prop2 = GetProperty (Prop, "FixHDA_8000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_HDA;
            }

            Prop2 = GetProperty (Prop, "FIX_DARWIN_10000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_WARNING;
            }

            Prop2 = GetProperty (Prop, "FIX_RTC_20000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_RTC;
            }

            Prop2 = GetProperty (Prop, "FIX_TMR_40000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_TMR;
            }

            Prop2 = GetProperty (Prop, "AddIMEI_80000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_IMEI;
            }

            Prop2 = GetProperty (Prop, "FIX_INTELGFX_100000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_INTELGFX;
            }

            Prop2 = GetProperty (Prop, "FIX_WAK_200000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_WAK;
            }

            Prop2 = GetProperty (Prop, "DeleteUnused_400000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_UNUSED;
            }

            Prop2 = GetProperty (Prop, "FIX_ADP1_800000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_ADP1;
            }

            Prop2 = GetProperty (Prop, "AddPNLF_1000000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_PNLF;
            }

            Prop2 = GetProperty (Prop, "FIX_S3D_2000000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_S3D;
            }

            Prop2 = GetProperty (Prop, "FIX_ACST_4000000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
                gSettings.FixDsdt |= FIX_ACST;
            }

            Prop2 = GetProperty (Prop, "AddHDMI_8000000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_HDMI;
            }

            Prop2 = GetProperty (Prop, "FixRegions_10000000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_REGIONS;
            }

            Prop2 = GetProperty (Prop, "NewWay_80000000");
            if (Prop2 != NULL && IsPropertyTrue (Prop2)) {
              gSettings.FixDsdt |= FIX_NEW_WAY;
            }
          }

          DBG ("   final mask=%08x\n", gSettings.FixDsdt);
        }
        
        Prop = GetProperty (Dict2, "Patches");
        if (Prop != NULL) {
          UINTN Count = GetTagCount (Prop);
          if (Count > 0) {
            gSettings.PatchDsdtNum     = (UINT32)Count;
            gSettings.PatchDsdtFind    = AllocateZeroPool (Count * sizeof(UINT8*));
            gSettings.PatchDsdtReplace = AllocateZeroPool (Count * sizeof(UINT8*));
            gSettings.LenToFind        = AllocateZeroPool (Count * sizeof(UINT32));
            gSettings.LenToReplace     = AllocateZeroPool (Count * sizeof(UINT32));
            DBG ("PatchesDSDT: %d requested\n", Count);

            for (i = 0; i < Count; ++i) {
              UINTN Size = 0;
              Status     = GetElement (Prop, i, &Prop);
              if (EFI_ERROR (Status)) {
                DBG ("error %r getting next element of PatchesDSDT at index %d\n", Status, i);
                continue;
              }

              if (Prop2 == NULL) {
                break;
              }

              DBG (" DSDT bin patch #%d ", i);
              gSettings.PatchDsdtFind[i]    = GetDataSetting (Prop2, "Find",    &Size);
              DBG (" lenToFind=%d ", Size);
              gSettings.LenToFind[i]        = (UINT32)Size;
              gSettings.PatchDsdtReplace[i] = GetDataSetting (Prop2, "Replace", &Size);
              DBG (" lenToReplace=%d\n", Size);
              gSettings.LenToReplace[i]     = (UINT32)Size;
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
/*
         Prop = GetProperty (Dict2, "SlpSmiAtWake");
         if (Prop != NULL) {
           if (IsPropertyTrue (Prop)) {
             gSettings.SlpWak = TRUE;
           }
         }
*/
        Prop   = GetProperty (Dict2, "DropOEM_DSM");
        defDSM = FALSE;

        if (Prop != NULL) {
          defDSM = TRUE;
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
      if (Dict2 != NULL) {
        Prop2 = GetProperty (Dict2, "Generate");
        if (Prop2 != NULL) {
          if (IsPropertyTrue (Prop2)) {
            gSettings.GeneratePStates = TRUE;
            gSettings.GenerateCStates = TRUE;
          } else if (IsPropertyFalse (Prop2)) {
            gSettings.GeneratePStates = FALSE;
            gSettings.GenerateCStates = FALSE;
          } else if (Prop2->type == kTagTypeDict) {
            Prop                      = GetProperty (Prop2, "PStates");
            gSettings.GeneratePStates = IsPropertyTrue (Prop);

            Prop                      = GetProperty (Prop2, "CStates");
            gSettings.GenerateCStates = IsPropertyTrue (Prop);
          }
        }

        Prop = GetProperty (Dict2, "DropOem");
        gSettings.DropSSDT  = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "UseSystemIO");
        gSettings.EnableISS = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "EnableC7");
        if (Prop != NULL) {
          gSettings.EnableC7 = IsPropertyTrue (Prop);
          DBG ("Config set EnableC7: %a\n", gSettings.EnableC7 ? "+" : "-");
        }

        Prop = GetProperty (Dict2, "EnableC6");
        if (Prop != NULL) {
          gSettings.EnableC6 = IsPropertyTrue (Prop);
          DBG ("Config set EnableC6: %a\n", gSettings.EnableC6 ? "+" : "-");
        }

        Prop = GetProperty (Dict2, "EnableC4");
        if (Prop != NULL) {
          gSettings.EnableC4 = IsPropertyTrue (Prop);
          DBG ("Config set EnableC4: %a\n", gSettings.EnableC4 ? "+" : "-");
        }

        Prop = GetProperty (Dict2, "EnableC2");
        if (Prop != NULL) {
          gSettings.EnableC2 = IsPropertyTrue (Prop);
          DBG ("Config set EnableC2: %a\n", gSettings.EnableC2 ? "+" : "-");
        }

        Prop = GetProperty (Dict2, "C3Latency");
        if (Prop != NULL) {
          gSettings.C3Latency = (UINT16)GetPropertyInteger (Prop, gSettings.C3Latency);
          DBG ("Config set C3Latency: %d\n", gSettings.C3Latency);
        }
        
        Prop                       = GetProperty (Dict2, "PLimitDict");
        gSettings.PLimitDict       = (UINT8)GetPropertyInteger (Prop, 0);

        Prop                    = GetProperty (Dict2, "UnderVoltStep");
        gSettings.UnderVoltStep = (UINT8)GetPropertyInteger (Prop, 0);

        Prop                       = GetProperty (Dict2, "DoubleFirstState");
        gSettings.DoubleFirstState = IsPropertyTrue (Prop);

        Prop = GetProperty (Dict2, "MinMultiplier");
        if (Prop != NULL) {
          gSettings.MinMultiplier  = (UINT8)GetPropertyInteger (Prop, gSettings.MinMultiplier);
          DBG ("Config set MinMultiplier=%d\n", gSettings.MinMultiplier);
        }

        Prop = GetProperty (Dict2, "MaxMultiplier");
        if (Prop != NULL) {
          gSettings.MaxMultiplier = (UINT8)GetPropertyInteger (Prop, gSettings.MaxMultiplier);
          DBG ("Config set MaxMultiplier=%d\n", gSettings.MaxMultiplier);
        }

        Prop = GetProperty (Dict2, "PluginType");
        if (Prop != NULL) {
          gSettings.PluginType = (UINT8)GetPropertyInteger (Prop, gSettings.PluginType);
          DBG ("Config set PluginType=%d\n", gSettings.PluginType);
        }
      }

      Prop               = GetProperty (DictPointer, "DropMCFG");
      gSettings.DropMCFG = IsPropertyTrue (Prop);
      
      Prop = GetProperty (DictPointer, "ResetAddress");
      if (Prop != NULL) {
        gSettings.ResetAddr = (UINT32)GetPropertyInteger (Prop, 0x64);
        DBG ("Config set ResetAddr=0x%x\n", gSettings.ResetAddr);

        if (gSettings.ResetAddr  == 0x64) {
          gSettings.ResetVal = 0xFE;
        } else if  (gSettings.ResetAddr  == 0xCF9) {
          gSettings.ResetVal = 0x06;
        }

        DBG ("Config calc ResetVal=0x%x\n", gSettings.ResetVal);
      }

      Prop = GetProperty (DictPointer, "ResetValue");
      if (Prop != NULL) {
        gSettings.ResetVal = (UINT8)GetPropertyInteger (Prop, gSettings.ResetVal);
        DBG ("Config set ResetVal=0x%x\n", gSettings.ResetVal);
      }
      //other known pair is 0x0CF9/0x06. What about 0x92/0x01 ?
            
      Prop = GetProperty (DictPointer, "HaltEnabler");
      gSettings.SlpSmiEnable = IsPropertyTrue (Prop);
       
      Prop = GetProperty (DictPointer, "smartUPS");
      if (Prop != NULL) {
        gSettings.smartUPS   = IsPropertyTrue (Prop);
        DBG ("Config set smartUPS present\n");
      }

      Prop               = GetProperty (DictPointer, "PatchAPIC");
      gSettings.PatchNMI = IsPropertyTrue (Prop);
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
      // Inject memory tables into SMBIOS
      Prop = GetProperty (DictPointer, "Memory");
      if (Prop != NULL){
        // Get memory table count
        TagPtr Prop2   = GetProperty (Prop, "SlotCount");
        gRAM.UserInUse = (UINT8)GetPropertyInteger (Prop2, 0);
        // Get memory channels
        Prop2             = GetProperty (Prop, "Channels");
        gRAM.UserChannels = (UINT8)GetPropertyInteger (Prop2, 0);
        // Get memory tables
        Prop2 = GetProperty (Prop, "Modules");
        if (Prop2 != NULL) {
          INTN   i;
          INTN   Count = GetTagCount (Prop2);
          TagPtr Prop3 = NULL;

          for (i = 0; i < Count; ++i) {
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

            UINTN Slot = MAX_RAM_SLOTS;

            if (Dict2->type == kTagTypeString && Dict2->string) {
              Slot = AsciiStrDecimalToUintn (Dict2->string);
            } else if (Dict2->type == kTagTypeInteger) {
              Slot = (UINTN)Dict2->string;
            } else {
              continue;
            }

            if (Slot >= MAX_RAM_SLOTS) {
              continue;
            }

            RAM_SLOT_INFO *SlotPtr = &gRAM.User[Slot];

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
              } else if (AsciiStriCmp (Dict2->string, "DDR") == 0) {
                SlotPtr->Type = MemoryTypeDdr;
              }
            }

            SlotPtr->InUse = (SlotPtr->ModuleSize > 0);
          }

          gSettings.InjectMemoryTables = TRUE;
        }
      }

      Prop = GetProperty (DictPointer, "Slots");
      if (Prop != NULL) {
        INTN   Index;
        INTN   DeviceN;
        INTN   Count = GetTagCount (Prop);
        TagPtr Prop3 = NULL;

        for (Index = 0; Index < Count; ++Index) {
          if (EFI_ERROR (GetElement (Prop, Index, &Prop3))) {
            continue;
          }
          if (Prop3 == NULL) {
            break;
          }
          
          Prop2 = GetProperty (Prop3, "Device");
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
            } else {
              DBG (" add properties to unknown device %a, ignored\n", Prop2->string);
              continue;
            }
          } else {
            DBG (" no device  property for slot\n");
            continue;
          }

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
        }
      }
    }
    
    //CPU
    DictPointer = GetProperty (Dict, "CPU");
    if (DictPointer != NULL) {
      Prop = GetProperty (DictPointer, "QPI");
      if (Prop != NULL) {
        gSettings.QPI = (UINT16)GetPropertyInteger (Prop, (INTN)gCPUStructure.ProcessorInterconnectSpeed);
        if (gSettings.QPI == 0) { //this is not default, this is zero!
          gSettings.QPI = 0xFFFF;
          DBG ("Config set QPi = 0 disable table132\n");
        } else {
          DBG ("Config set QPI=%dMHz\n", gSettings.QPI);
        }
      }

      Prop = GetProperty (DictPointer, "FrequencyMHz");      
      if (Prop != NULL) {
        gSettings.CpuFreqMHz = (UINT32)GetPropertyInteger (Prop, gSettings.CpuFreqMHz);
        DBG ("Config set CpuFreq=%dMHz\n", gSettings.CpuFreqMHz);
      }

      Prop = GetProperty (DictPointer, "Type");
      gSettings.CpuType = GetAdvancedCpuType();
      if (Prop != NULL) {
        gSettings.CpuType = (UINT16)GetPropertyInteger (Prop, gSettings.CpuType);
        DBG ("Config set CpuType=%x\n", gSettings.CpuType);
      }
      
      Prop = GetProperty (DictPointer, "BusSpeedkHz");
      if (Prop != NULL) {
        gSettings.BusSpeed = (UINT32)GetPropertyInteger (Prop, gSettings.BusSpeed);
        DBG ("Config set BusSpeed=%dkHz\n", gSettings.BusSpeed);
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

    }
    
    // RtVariables
    DictPointer = GetProperty (Dict, "RtVariables");
    if (DictPointer != NULL) {
      // ROM: <data>bin data</data> or <string>base 64 encoded bin data</string>
      Prop = GetProperty (DictPointer, "ROM");
      if (Prop != NULL) {
        UINTN ROMLength         = 0;
        gSettings.RtROM         = GetDataSetting (DictPointer, "ROM", &ROMLength);
        gSettings.RtROMLen      = ROMLength;

        if (gSettings.RtROM == NULL || gSettings.RtROMLen == 0) {
          gSettings.RtROM       = NULL;
          gSettings.RtROMLen    = 0;
        } else {
          gSettings.RtROMConfig = TRUE;
        }
      }
      
      // MLB: <string>some value</string>
      Prop = GetProperty (DictPointer, "MLB");
      if (Prop != NULL && AsciiStrLen (Prop->string) > 0) {
        gSettings.RtMLB         = AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        gSettings.RtMLBConfig   = TRUE;
      }
      
      // Setting Clover Variables for RC Scripts in config.plist is now deprecated (r2889+)
      Prop = GetProperty (DictPointer, "MountEFI");
      if (Prop != NULL) {
        DBG ("** Warning: ignoring RtVariable MountEFI set in config.plist: deprecated !\n");
      }

      Prop = GetProperty (DictPointer, "LogLineCount");
      if (Prop != NULL) {
        DBG ("** Warning: ignoring RtVariable LogLineCount set in config.plist: deprecated !\n");
      }

      Prop = GetProperty (DictPointer, "LogEveryBoot");
      if (Prop != NULL) {
        DBG ("** Warning: ignoring RtVariable LogEveryBoot set in config.plist: deprecated !\n");
      }
    }

    if (gSettings.RtROM == NULL) {
      UINT8 *Variable;
      gRT->GetVariable (L"ROM", &gEfiAppleNvramGuid, NULL, &gSettings.RtROMLen, Variable);

      if (Variable != NULL) {
        CopyMem (gSettings.RtROM, Variable, &gSettings.RtROMLen);
      }

      if (gSettings.RtROM == NULL) {
        gSettings.RtROM    = (UINT8*)&gSettings.SmUUID.Data4[2];
        gSettings.RtROMLen = 6;
      }
    }

    if (gSettings.RtMLB == NULL) {
      if (!gSettings.BoardSNConfig) {
        CHAR8 *Variable = NULL;
        gRT->GetVariable (L"MLB", &gEfiAppleNvramGuid, NULL, NULL, Variable);

        if (Variable != NULL) {
          AsciiStrCpy (gSettings.RtMLB, Variable);
        }
      }

      if (gSettings.RtMLB == NULL) {
        gSettings.RtMLB       = &gSettings.BoardSerialNumber[0];
        gSettings.RtMLBConfig = gSettings.BoardSNConfig;
      }
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
          AsciiStrToUnicodeStr (Prop->string, gSettings.CustomUuid);
          Status = StrToGuidLE (gSettings.CustomUuid, &gUuid);
          // if CustomUUID specified, then default for InjectSystemID=FALSE
          // to stay compatibile with previous Clover behaviour
          gSettings.InjectSystemID = FALSE;
        } else {
          DBG ("Error: invalid CustomUUID '%a' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->string);
        }
      }
      //else gUuid value from SMBIOS
      
      Prop                     = GetProperty (DictPointer, "InjectSystemID");
      gSettings.InjectSystemID = gSettings.InjectSystemID ? !IsPropertyFalse(Prop) : IsPropertyTrue (Prop);
    }
    
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
    SaveSettings();
  }
//DBG ("config.plist read and return %r\n", Status);
  return Status;
}

CHAR8 *GetOSVersion(IN LOADER_ENTRY *Entry)
{
  CHAR8      *OSVersion  = NULL;
  EFI_STATUS Status      = EFI_NOT_FOUND;
  CHAR8*     PlistBuffer = NULL;
  UINTN      PlistLen;
  TagPtr     Dict        = NULL;
  TagPtr     Prop        = NULL;
  UINTN      i;
  
  if (!Entry || !Entry->Volume) {
    return OSVersion;
  }

  if (OSTYPE_IS_OSX(Entry->LoaderType)) {
    // Detect exact version for Mac OS X Regular/Server
    CHAR16* SystemPlists[] = { L"\\System\\Library\\CoreServices\\SystemVersion.plist", // OS X Regular
      L"\\System\\Library\\CoreServices\\ServerVersion.plist", // OS X Server
      NULL };

    i = 0;
    while (SystemPlists[i] != NULL && !FileExists(Entry->Volume->RootDir, SystemPlists[i])) {
      ++i;
    }

    if (SystemPlists[i] != NULL) { // found OSX System
      Status = egLoadFile (Entry->Volume->RootDir, SystemPlists[i], (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = GetProperty (Dict, "ProductVersion");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          OSVersion = AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        }
      }
    }
  }

  if (OSTYPE_IS_OSX_INSTALLER (Entry->LoaderType)) {
    // Detect exact version for 2nd stage Installer (thanks to dmazar for this idea)
    // This should work for most installer cases. Rest cases will be read from boot.efi before booting.
    CHAR16 *InstallerPlist = L"\\.IABootFiles\\com.apple.Boot.plist";
    if (FileExists (Entry->Volume->RootDir, InstallerPlist)) {
      Status = egLoadFile (Entry->Volume->RootDir, InstallerPlist, (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop = GetProperty (Dict, "Kernel Flags");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          if (AsciiStrStr (Prop->string, "Install%20OS%20X%20Mavericks.app")) {
            OSVersion = AllocateZeroPool (5);
            UnicodeStrToAsciiStr (L"10.9", OSVersion);
          } else if (AsciiStrStr (Prop->string, "Install%20OS%20X%20Yosemite") || AsciiStrStr (Prop->string, "Install%20OS%20X%2010.10")) {
            OSVersion = AllocateZeroPool (6);
            UnicodeStrToAsciiStr (L"10.10", OSVersion);
          } else if (AsciiStrStr (Prop->string, "Install%20OS%20X%20Mountain%20Lion")) {
            OSVersion = AllocateZeroPool (5);
            UnicodeStrToAsciiStr (L"10.8", OSVersion);
          } else if (AsciiStrStr (Prop->string, "Install%20Mac%20OS%20X%20Lion")) {
            OSVersion = AllocateZeroPool (5);
            UnicodeStrToAsciiStr (L"10.7", OSVersion);
          }
        }
      }
    }
  }

  if (OSTYPE_IS_OSX_RECOVERY (Entry->LoaderType)) {
    // Detect exact version for OS X Recovery
    CHAR16 *RecoveryPlist = L"\\com.apple.recovery.boot\\SystemVersion.plist";
    if (FileExists (Entry->Volume->RootDir, RecoveryPlist)) {
      Status = egLoadFile (Entry->Volume->RootDir, RecoveryPlist, (UINT8 **)&PlistBuffer, &PlistLen);
      if (!EFI_ERROR (Status) && PlistBuffer != NULL && ParseXML (PlistBuffer, &Dict, 0) == EFI_SUCCESS) {
        Prop       = GetProperty (Dict, "ProductVersion");
        if (Prop != NULL && Prop->string != NULL && Prop->string[0] != '\0') {
          OSVersion = AllocateCopyPool (AsciiStrSize (Prop->string), Prop->string);
        }
      }
    } else if (FileExists (Entry->Volume->RootDir, L"\\com.apple.recovery.boot\\boot.efi")) {
      // Special case - com.apple.recovery.boot/boot.efi exists but SystemVersion.plist doesn't --> 10.9 recovery
      OSVersion    = AllocateZeroPool (5);
      UnicodeStrToAsciiStr (L"10.9", OSVersion);
    }
  }
  
  if (PlistBuffer != NULL) {
    FreePool (PlistBuffer);
  }
  
  return OSVersion;
}

CHAR16
*GetOSIconName (
  IN  CHAR8 *OSVersion
  )
{
  CHAR16 *OSIconName;
  if (OSVersion == NULL) {
    OSIconName = L"mac";
  } else if (AsciiStrStr (OSVersion, "10.4") != 0) {
    // Tiger
    OSIconName = L"tiger,mac";
  } else if (AsciiStrStr (OSVersion, "10.5") != 0) {
    // Leopard
    OSIconName = L"leo,mac";
  } else if (AsciiStrStr (OSVersion, "10.6") != 0) {
    // Snow Leopard
    OSIconName = L"snow,mac";
  } else if (AsciiStrStr (OSVersion, "10.7") != 0) {
    // Lion
    OSIconName = L"lion,mac";
  } else if (AsciiStrStr (OSVersion, "10.8") != 0) {
    // Mountain Lion
    OSIconName = L"cougar,mac";
  } else if (AsciiStrStr (OSVersion, "10.9") != 0) {
    // Mavericks
    OSIconName = L"mav,mac";
  } else if (AsciiStrStr (OSVersion, "10.10") != 0) {
    // Yosemite
    OSIconName = L"yos,mac";
  } else {
    OSIconName = L"mac";
  }

  return OSIconName;
}

//Get the UUID of the AppleRaid or CoreStorage volume from the boot helper partition
EFI_STATUS
GetRootUUID (
  IN  REFIT_VOLUME *Volume
  )
{
  EFI_STATUS Status       = EFI_NOT_FOUND;
  CHAR8      *PlistBuffer = 0;
  UINTN      PlistLen;
  TagPtr     Dict         = NULL;
  TagPtr     Prop         = NULL;
  CHAR16	   Uuid[40];
    
  CHAR16*    SystemPlistP = L"\\com.apple.boot.P\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist";
  CHAR16*    SystemPlistR = L"\\com.apple.boot.R\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist"; //untested, found in chameleon
  CHAR16*    SystemPlistS = L"\\com.apple.boot.S\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist"; //untested, found in chameleon
    
  if (Volume == NULL) {
    return EFI_NOT_FOUND;
  }
    
  if (FileExists (Volume->RootDir, SystemPlistP)) {
    Status = egLoadFile (Volume->RootDir, SystemPlistP, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (FileExists (Volume->RootDir, SystemPlistR)) {
    Status = egLoadFile (Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (FileExists (Volume->RootDir, SystemPlistS)) {
    Status = egLoadFile (Volume->RootDir, SystemPlistS, (UINT8 **)&PlistBuffer, &PlistLen);
  }
   
  if (!EFI_ERROR (Status)) {
    if (ParseXML (PlistBuffer, &Dict, 0) != EFI_SUCCESS) {
      FreePool (PlistBuffer);
      return EFI_NOT_FOUND;
    }
        
    Prop = GetProperty (Dict, "Root UUID");
    if (Prop != NULL) {
       AsciiStrToUnicodeStr (Prop->string, Uuid);
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
  EFI_HANDLE			    *HandleArray = NULL;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;
  UINTN               Index;
  UINTN               Segment      = 0;
  UINTN               Bus          = 0;
  UINTN               Device       = 0;
  UINTN               Function     = 0;
  UINTN               i;
  UINT32              Bar0;
  UINT8               *Mmio        = NULL;
  radeon_card_info_t  *info;
  SLOT_DEVICE         *SlotDevice;

  NGFX = 0;
//Arpt.Valid = FALSE; //global variables initialized by 0 - c-language
  
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
        if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
            (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA) &&
            (NGFX < 4)) {
          GFX_PROPERTIES *gfx = &gGraphics[NGFX];
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

              for (i = 0; radeon_cards[i].device_id; ++i) {
                info      = &radeon_cards[i];

                if (info->device_id == Pci.Hdr.DeviceId) {
                   break;
                }
              }

              AsciiSPrint (gfx->Model,  64, "%a", info->model_name);
              AsciiSPrint (gfx->Config, 64, "%a", card_configs[info->cfg_name].name);
              gfx->Ports                  = card_configs[info->cfg_name].ports;
              DBG ("Found Radeon model=%a\n", gfx->Model);

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
              DBG ("Found GFX model=%a\n", gfx->Model);
              gfx->Ports = 1;
/*          
              SlotDevice                  = &SlotDevices[2];
              SlotDevice->SegmentGroupNum = (UINT16)Segment;
              SlotDevice->BusNum          = (UINT8)Bus;
              SlotDevice->DevFuncNum      = (UINT8)((Device << 4) | (Function & 0x0F));
              SlotDevice->Valid           = TRUE;
              AsciiSPrint (SlotDevice->SlotName, 31, "PCI Slot 0");
              SlotDevice->SlotID          = 0;
              SlotDevice->SlotType        = SlotTypePciExpressX16;
*/
              break;

            case 0x10de:
              gfx->Vendor = Nvidia;
              Bar0        = Pci.Device.Bar[0];
              Mmio        = (UINT8 *)(UINTN)(Bar0 & ~0x0f);
              //	DBG ("BAR: 0x%p\n", Mmio);
              // get card type
              gfx->Family = (REG32 (Mmio, 0) >> 20) & 0x1ff;
              

              AsciiSPrint (
                gfx->Model,
                64,
                "%a",
                get_nvidia_model (((Pci.Hdr.VendorId <<16) | Pci.Hdr.DeviceId),
                ((Pci.Device.SubsystemVendorID << 16) | Pci.Device.SubsystemID))
                );

              DBG ("Found NVidia model=%a\n", gfx->Model);
              gfx->Ports                  = 2;

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
              break;
          }                

          NGFX++;
        }   //if gfx  

        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_OTHER)) {
          SlotDevice                  = &SlotDevices[6];
 //       DBG ("Found AirPort. Landing enabled...\n");
          SlotDevice->SegmentGroupNum = (UINT16)Segment;
          SlotDevice->BusNum          = (UINT8)Bus;
          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
          SlotDevice->Valid           = TRUE;
          AsciiSPrint (SlotDevice->SlotName, 31, "Airport");
          SlotDevice->SlotID          = 0;
          SlotDevice->SlotType        = SlotTypePciExpressX1;
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
        }

        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_FIREWIRE)) {
          SlotDevice = &SlotDevices[12];
          SlotDevice->SegmentGroupNum = (UINT16)Segment;
          SlotDevice->BusNum          = (UINT8)Bus;
          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
          SlotDevice->Valid           = TRUE;
          AsciiSPrint (SlotDevice->SlotName, 31, "Firewire");
          SlotDevice->SlotID          = 3;
          SlotDevice->SlotType        = SlotTypePciExpressX4;
        }
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA)) {
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
        }
      }
    }
  }
}


VOID
SetDevices (
  LOADER_ENTRY *Entry
  )
{
  //	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *modeInfo;
  EFI_STATUS					Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;
  UINTN               HandleCount;
  UINTN               i;
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
  
  GetEdidDiscovered ();
  // Scan PCI handles 
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (!EFI_ERROR (Status)) {
    for (i = 0; i < HandleCount; ++i) {
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
        // GFX
        if (/* gSettings.GraphicsInjector && */
            (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
            (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA)) {
          
//        gGraphics.DeviceID = Pci.Hdr.DeviceId;
          
          switch (Pci.Hdr.VendorId) {
            case 0x1002:
              if (gSettings.InjectATI) {
                //can't do in one step because of C-conventions
                TmpDirty    = setup_ati_devprop(Entry, &PCIdevice);
                StringDirty |=  TmpDirty;
              } else {
                MsgLog ("ATI injection not set\n");
              }

              break;

            case 0x8086:
              if (gSettings.InjectIntel) {
                TmpDirty    = setup_gma_devprop(&PCIdevice);
                StringDirty |=  TmpDirty;
                MsgLog ("Intel GFX revision  =0x%x\n", PCIdevice.revision);
              } else {
                MsgLog ("Intel GFX injection not set\n");
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
          //           MsgLog ("Ethernet device found\n");
          if (!(gSettings.FixDsdt & FIX_LAN)) {
            TmpDirty = set_eth_props (&PCIdevice);
            StringDirty |=  TmpDirty;
          }
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
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA)) {
          //no HDMI injection
          if ((Pci.Hdr.VendorId != 0x1002) &&
              (Pci.Hdr.VendorId != 0x10de)) {
            TmpDirty    = set_hda_props (PciIo, &PCIdevice, Entry->OSVersion);
            StringDirty |= TmpDirty;
          }
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
          if (gSettings.ForceHPET) {
            Rcba   = 0;
            /* Scan Port */
            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0xF0, 1, &Rcba);
            
            if (EFI_ERROR (Status)) continue;
            //Rcba &= 0xFFFFC000;
            if ((Rcba & 0xFFFFC000) == 0) {
              MsgLog (" RCBA disabled; cannot force enable HPET\n");
            } else {
              if ((Rcba & 1) == 0) {
                MsgLog (" RCBA access disabled; trying to enable\n");
                Rcba |= 1;

                PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, 0xF0, 1, &Rcba);
              }

 /*           Hptc = 0;
              PciIo->Mem.Read (
                       PciIo,
                       EfiPciIoWidthUint32,
                       EFI_PCI_IO_PASS_THROUGH_BAR,
                       0x3404ULL,
                       1,
                       &Hptc
                       );
*/
              Rcba &= 0xFFFFC000;
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
          }
        }
      }
    }
  }
  
  if (StringDirty) {
    EFI_PHYSICAL_ADDRESS BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
    stringlength                   = string->length * 2;
    DBG ("stringlength = %d\n", stringlength);
   // gDeviceProperties            = AllocateAlignedPages EFI_SIZE_TO_PAGES (stringlength + 1), 64);
    
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiACPIReclaimMemory,
                    EFI_SIZE_TO_PAGES (stringlength + 1),
                    &BufferPtr
                    );

    if (!EFI_ERROR (Status)) {
      mProperties       = (UINT8*)(UINTN)BufferPtr; 
      gDeviceProperties = (VOID*)devprop_generate_string (string);
      gDeviceProperties[stringlength] = 0;
//          DBG (gDeviceProperties);
//          DBG ("\n");
//      StringDirty = FALSE;
      //-------
      mPropSize = (UINT32)AsciiStrLen (gDeviceProperties) / 2;
 //     DBG ("Preliminary size of mProperties=%d\n", mPropSize);
      mPropSize = hex2bin (gDeviceProperties, mProperties, mPropSize);
 //     DBG ("Final size of mProperties=%d\n", mPropSize);
      //---------      
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
    gCPUStructure.ExternalClock = gSettings.BusSpeed;
    gCPUStructure.FSBFrequency  = MultU64x64 (gSettings.BusSpeed, kilo); //kHz -> Hz
    gCPUStructure.MaxSpeed      = (UINT32)(DivU64x32 ((UINT64)gSettings.BusSpeed * gCPUStructure.MaxRatio, 10000)); //kHz->MHz
  }

  if ((gSettings.CpuFreqMHz > 100) && (gSettings.CpuFreqMHz < 20000)) {
    gCPUStructure.MaxSpeed      = gSettings.CpuFreqMHz;
  }
  
  gCPUStructure.CPUFrequency    = MultU64x64 (gCPUStructure.MaxSpeed, Mega);

  return EFI_SUCCESS;
}

//dmazar
CHAR16
*GetExtraKextsDir (
  CHAR8 *OSVersion
  )
{
  CHAR16 *SrcDir         = NULL;
  CHAR8  FixedVersion[6] = "Other";
  CHAR8  *DotPtr;

  if (OSVersion != NULL) {
    AsciiStrnCpy(FixedVersion, OSVersion, 5);
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
    SrcDir   = PoolPrint (L"%s\\kexts\\Other", OEMPath);

    if (!FileExists (SelfVolume->RootDir, SrcDir)) {
      FreePool (SrcDir);
      SrcDir = NULL;
    }
  }

  if (SrcDir == NULL) {
    // if not found, check EFI\CLOVER\kexts\...
    SrcDir = PoolPrint (L"\\EFI\\CLOVER\\kexts\\%a", FixedVersion);
    if (!FileExists (SelfVolume->RootDir, SrcDir)) {
      FreePool (SrcDir);
 //     SrcDir = PoolPrint (L"\\EFI\\CLOVER\\kexts\\Other", gSettings.OEMProduct);
      SrcDir   = PoolPrint (L"\\EFI\\CLOVER\\kexts\\Other");
      if (!FileExists (SelfVolume->RootDir, SrcDir)) {
        FreePool (SrcDir);
        SrcDir = NULL;
      }
    }
  }
  
  return SrcDir;
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
  FSI_STRING_LIST      *ForceLoadKexts;

  MsgLog ("FSInjection: ");
    
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
    MsgLog ("not started - gFSInjectProtocolGuid not found\n");
    return EFI_NOT_STARTED;
  }

  // check if blocking of caches is needed
  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_NOCACHES)) {
    MsgLog ("blocking caches");
//  BlockCaches = TRUE;
    // add caches to blacklist
    Blacklist = FSInject->CreateStringList ();
    if (Blacklist == NULL) {
      MsgLog (": Error not enough memory!\n");
      return EFI_NOT_STARTED;
    }

    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\kernelcache");
    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\Extensions.mkext");
    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Extensions.mkext");
    FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\kernelcache");
    FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\Extensions.mkext");

    if (gSettings.BlockKexts[0] != L'\0') {
      FSInject->AddStringToList(Blacklist, PoolPrint (L"\\System\\Library\\Extensions\\%s", gSettings.BlockKexts));
    }

    MsgLog (", ");
  }
    
  // check if kext injection is needed
  // (will be done only if caches are blocked or if boot.efi refuses to load kernelcache)
  SrcDir = NULL;
  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_WITHKEXTS)) {
    SrcDir = GetExtraKextsDir(Entry->OSVersion);
    if (SrcDir != NULL) {
      // we have found it - injection will be done
      MsgLog ("using kexts path: '%s'", SrcDir);
//    InjectionNeeded = TRUE;
    } else {
      MsgLog ("skipping kext injection (kexts folder not found)");
    }
  } else {
    MsgLog ("skipping kext injection (not requested)");
  }
    
  // prepare list of kext that will be forced to load
  ForceLoadKexts = FSInject->CreateStringList ();
  if (ForceLoadKexts == NULL) {
    MsgLog (" - Error: not enough memory!\n");
    return EFI_NOT_STARTED;
  }

  KextPatcherRegisterKexts (FSInject, ForceLoadKexts, Entry);

  Status = FSInject->Install (
                       Volume->DeviceHandle,
                       L"\\System\\Library\\Extensions",
                       SelfVolume->DeviceHandle,
                       SrcDir,
                       Blacklist,
                       ForceLoadKexts
                       );
    
  if (SrcDir != NULL) {
    FreePool (SrcDir);
  }
    
  if (EFI_ERROR (Status)) {
    MsgLog (" - Error: could not install injection!\n");
    return EFI_NOT_STARTED;
  }
    
  // reinit Volume->RootDir? it seems it's not needed.
    
  MsgLog ("\n");
  return Status;
}

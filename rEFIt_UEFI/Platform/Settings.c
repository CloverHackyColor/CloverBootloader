/*
 Slice 2012
*/

#include "Platform.h"
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
#define DBG(...) DebugLog(DEBUG_SET, __VA_ARGS__)
#endif

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
UINTN                           NGFX = 0; // number of GFX

UINTN                           ThemesNum = 0;
CHAR16                          *ThemesList[50]; //no more then 50 themes?

// firmware
BOOLEAN                         gFirmwareClover = FALSE;
UINTN                           gEvent;
UINT16                          gBacklightLevel;
BOOLEAN                         defDSM;
UINT16                          dropDSM;

extern MEM_STRUCTURE            gRAM;
extern BOOLEAN                  NeedPMfix;

GUI_ANIME *GuiAnime = NULL;

extern INTN ScrollWidth;
extern INTN ScrollButtonsHeight;
extern INTN ScrollBarDecorationsHeight;
extern INTN ScrollScrollDecorationsHeight;

extern UINT8 GetOSTypeFromPath(IN CHAR16 *Path);

// global configuration with default values
REFIT_CONFIG   GlobalConfig = { FALSE, -1, 0, 0, 0, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE,
  //Font
  FONT_ALFA, 7, 0xFFFFFF80, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, None, 0,
  //BackgroundDark
  FALSE, FALSE, FALSE, FALSE, 0, 0, 4, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  //BannerEdgeHorizontal
  0, 0, 0, 0,
  //VerticalLayout
  FALSE, 128, 8, 24
};
/*
VOID __inline WaitForSts(VOID) {
	UINT32 inline_timeout = 100000;
	while (AsmReadMsr64(MSR_IA32_PERF_STATUS) & (1 << 21)) { if (!inline_timeout--) break; }
} */

UINT32 GetCrc32(UINT8 *Buffer, UINTN Size)
{
  UINTN i, len;
  UINT32 x;
  UINT32 *fake = (UINT32*)Buffer;
  x = 0;
  if (!fake) {
    DBG("Buffer=NULL\n");
    return 0;
  }
  len = Size >> 2;
//  DBG("Buffer len=%d fake[]=\n", len);
  for (i=0; i<len; i++) {
//    DBG("%X ", fake[i]);
    x += fake[i];
  }
//  DBG("\n");
  return x;
}

VOID ParseLoadOptions(OUT CHAR16** conf, OUT TagPtr* dict)
{
    CHAR8* start = (CHAR8*)SelfLoadedImage->LoadOptions;
    CHAR8* end   = (CHAR8*)((CHAR8*)SelfLoadedImage->LoadOptions+SelfLoadedImage->LoadOptionsSize);
    UINTN   TailSize;
    UINTN  i = 0;
    CHAR8* PlistStrings[] = {"<?xml",
                            "<!DOCTYPE plist",
                            "<plist",
                            "<dict>",
                            '\0'};
    UINTN  PlistStringsLen;
    CHAR8*  AsciiConf = NULL;
    
    *conf = NULL;
    *dict = NULL;
    for (; (start < end) && ((*start == ' ') || (*start == '\\') || (*start == '/')); start++) {}
    TailSize = end-start;
//    DBG("TailSize = %d\n", TailSize);
    if ((TailSize) <= 0) return;
    for (i=0; PlistStrings[i] != '\0'; i++) {
        PlistStringsLen = AsciiStrLen(PlistStrings[i]);
//        DBG("PlistStrings[%d] = %a\n", i, PlistStrings[i]);
        if (PlistStringsLen < TailSize)
            if (AsciiStriNCmp(PlistStrings[i],start, PlistStringsLen)) {
                DBG("Found Plist String = %a, parse XML in LoadOptions\n", PlistStrings[i]);
                if (ParseXML(start, dict, (UINT32)TailSize) != EFI_SUCCESS) {
                    *dict = NULL;
                    DBG("Xml in load options is bad\n");
                    return;
                }
                return;
            }
    }
    for (; (end > start) && ((*end == ' ') || (*end == '\\') || (*end == '/')); end--) {}
    TailSize = end-start;
//    DBG("TailSize2 = %d\n", TailSize);
    if (TailSize <= 0) return;
    if (TailSize > 6)
        if (AsciiStriNCmp(".plist",end-6, 6)) {
            end-=6;
            TailSize-=6;
//            DBG("TailSize3 = %d\n", TailSize);
        }
    AsciiConf = AllocateCopyPool(TailSize + 1, start);
    if (!AsciiConf) return;
    *(AsciiConf+TailSize) = '\0';
    *conf = AllocateZeroPool((TailSize + 1) * sizeof (CHAR16));
    AsciiStrToUnicodeStr(AsciiConf, *conf);
    FreePool(AsciiConf);
    return;
}

//
// returns binary setting in a new allocated buffer and data length in dataLen.
// data can be specified in <data></data> base64 encoded
// or in <string></string> hex encoded
//
VOID *GetDataSetting(IN TagPtr dict, IN CHAR8 *propName, OUT UINTN *dataLen)
{
    TagPtr  prop;
    UINT8   *data = NULL;
    UINT32   len;
    //UINTN   i;
    
    prop = GetProperty(dict, propName);
    if (prop) {
        if (prop->data != NULL && prop->dataLen > 0) {
            // data property
            data = AllocateZeroPool(prop->dataLen);
            CopyMem(data, prop->data, prop->dataLen);
            if (dataLen != NULL) {
                *dataLen = prop->dataLen;
            }
            //DBG("Data: %p, Len: %d = ", data, prop->dataLen);
            //for (i = 0; i < prop->dataLen; i++) DBG("%02x ", data[i]);
            //DBG("\n");
        } else {
            // assume data in hex encoded string property
            len = (UINT32)(AsciiStrLen(prop->string) >> 1); // 2 chars per byte
            data = AllocateZeroPool(len);
            len = hex2bin(prop->string, data, len);
            if (dataLen != NULL) {
                *dataLen = len;
            }
            //DBG("Data(str): %p, Len: %d = ", data, len);
            //for (i = 0; i < len; i++) DBG("%02x ", data[i]);
            //DBG("\n");
        }
    }
    return data;
}

EFI_STATUS LoadUserSettings(IN EFI_FILE *RootDir, IN CHAR16 *ConfName, TagPtr * dict)
{
  EFI_STATUS	Status = EFI_NOT_FOUND;
  UINTN       size = 0;
  CHAR8*      gConfigPtr = NULL;
  CHAR16*     ConfigPlistPath;
  CHAR16*     ConfigOemPath;
  
  // load config
  if ((ConfName == NULL) || (dict == NULL)) {
    DBG("Can't load plist in LoadUserSettings: NULL\n");
    return EFI_NOT_FOUND;
  }
  
  ConfigPlistPath = PoolPrint(L"EFI\\CLOVER\\%s.plist", ConfName);
  ConfigOemPath = PoolPrint(L"%s\\%s.plist", OEMPath, ConfName);
 // DBG("PlistPath: %s\n", ConfigPlistPath);
  if (FileExists(SelfRootDir, ConfigOemPath)) {
    Status = egLoadFile(SelfRootDir, ConfigOemPath, (UINT8**)&gConfigPtr, &size);
  } else {
    DBG("Oem %s.plist not found at path: %s\n", ConfName, ConfigOemPath);
  }
  
  if (EFI_ERROR(Status)) {
    if ((RootDir != NULL) && FileExists(RootDir, ConfigPlistPath)) {
      Status = egLoadFile(RootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &size);
    }
    if (EFI_ERROR(Status)) {
      Status = egLoadFile(SelfRootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &size);
    }
  } else {
    DBG("Using OEM %s.plist at path: %s\n", ConfName, ConfigOemPath);
  }
  
  if(EFI_ERROR(Status) || gConfigPtr == NULL) {
//    DBG("Error loading %s.plist! Status=%r\n", ConfName, Status);
    return Status;
  }
  
  Status = ParseXML((const CHAR8*)gConfigPtr, dict, (UINT32)size);
  if (EFI_ERROR(Status)) {
    dict = NULL;
    DBG(" plist parse error Status=%r\n", Status);
    return Status;
  }
  
  return EFI_SUCCESS;
}

static BOOLEAN AddCustomEntry(IN CUSTOM_LOADER_ENTRY *Entry)
{
  if (Entry == NULL) {
    return FALSE;
  }
  if (gSettings.CustomEntries) {
    CUSTOM_LOADER_ENTRY *Entries = gSettings.CustomEntries;
    while (Entries->Next) {
      Entries = Entries->Next;
    }
    Entries->Next = Entry;
  } else {
    gSettings.CustomEntries = Entry;
  }
  return TRUE;
}
static BOOLEAN AddCustomLegacyEntry(IN CUSTOM_LEGACY_ENTRY *Entry)
{
  if (Entry == NULL) {
    return FALSE;
  }
  if (gSettings.CustomLegacy) {
    CUSTOM_LEGACY_ENTRY *Entries = gSettings.CustomLegacy;
    while (Entries->Next) {
      Entries = Entries->Next;
    }
    Entries->Next = Entry;
  } else {
    gSettings.CustomLegacy = Entry;
  }
  return TRUE;
}
static BOOLEAN AddCustomToolEntry(IN CUSTOM_TOOL_ENTRY *Entry)
{
  if (Entry == NULL) {
    return FALSE;
  }
  if (gSettings.CustomTool) {
    CUSTOM_TOOL_ENTRY *Entries = gSettings.CustomTool;
    while (Entries->Next) {
      Entries = Entries->Next;
    }
    Entries->Next = Entry;
  } else {
    gSettings.CustomTool = Entry;
  }
  return TRUE;
}
static BOOLEAN AddCustomSubEntry(IN OUT CUSTOM_LOADER_ENTRY *Entry, IN CUSTOM_LOADER_ENTRY *SubEntry)
{
   if ((Entry == NULL) || (SubEntry == NULL)) {
    return FALSE;
  }
  if (Entry->SubEntries) {
    CUSTOM_LOADER_ENTRY *Entries = Entry->SubEntries;
    while (Entries->Next) {
      Entries = Entries->Next;
    }
    Entries->Next = SubEntry;
  } else {
    Entry->SubEntries = SubEntry;
  }
  return TRUE;
}
static CUSTOM_LOADER_ENTRY *DuplicateCustomEntry(IN CUSTOM_LOADER_ENTRY *Entry)
{
  CUSTOM_LOADER_ENTRY *DuplicateEntry;
  if (Entry == NULL) {
    return NULL;
  }
  DuplicateEntry = (CUSTOM_LOADER_ENTRY *)AllocateZeroPool(sizeof(CUSTOM_LOADER_ENTRY));
  if (DuplicateEntry) {
    if (Entry->Volume) {
      DuplicateEntry->Volume = EfiStrDuplicate(Entry->Volume);
    }
    if (Entry->Path) {
      DuplicateEntry->Path = EfiStrDuplicate(Entry->Path);
    }
    if (Entry->Options) {
      DuplicateEntry->Options = EfiStrDuplicate(Entry->Options);
    }
    if (Entry->FullTitle) {
      DuplicateEntry->FullTitle = EfiStrDuplicate(Entry->FullTitle);
    }
    if (Entry->Title) {
      DuplicateEntry->Title = EfiStrDuplicate(Entry->Title);
    }
    if (Entry->ImagePath) {
      DuplicateEntry->ImagePath = EfiStrDuplicate(Entry->ImagePath);
    }
    if (Entry->DriveImagePath) {
      DuplicateEntry->DriveImagePath = EfiStrDuplicate(Entry->DriveImagePath);
    }
    if (Entry->BootBgColor) {
      DuplicateEntry->BootBgColor = AllocateCopyPool(sizeof(EG_PIXEL), Entry->BootBgColor);
    }
    DuplicateEntry->Image = Entry->Image;
    DuplicateEntry->DriveImage = Entry->DriveImage;
    DuplicateEntry->Hotkey = Entry->Hotkey;
    DuplicateEntry->Flags = Entry->Flags;
    DuplicateEntry->Type = Entry->Type;
    DuplicateEntry->VolumeType = Entry->VolumeType;
  }
  return DuplicateEntry;
}
static BOOLEAN FillinCustomEntry(IN OUT CUSTOM_LOADER_ENTRY *Entry, TagPtr dictPointer, IN BOOLEAN SubEntry)
{
  TagPtr prop;
  UINT8  OSType;
  if ((Entry == NULL) || (dictPointer == NULL)) {
    return FALSE;
  }
  prop = GetProperty(dictPointer, "Volume");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Volume) {
      FreePool(Entry->Volume);
    }
    Entry->Volume = PoolPrint(L"%a", prop->string);
  }
  prop = GetProperty(dictPointer, "Path");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Path) {
      FreePool(Entry->Path);
    }
    Entry->Path = PoolPrint(L"%a", prop->string);
  }
  prop = GetProperty(dictPointer, "AddArguments");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Options) {
      CHAR16 *OldOptions = Entry->Options;
      Entry->Options = PoolPrint(L"%s %a", OldOptions, prop->string);
      FreePool(OldOptions);
    } else {
      Entry->Options = PoolPrint(L"%a", prop->string);
    }
  } else {
    prop = GetProperty(dictPointer, "Arguments");
    if (prop && (prop->type == kTagTypeString)) {
      if (Entry->Options) {
        FreePool(Entry->Options);
      }
      Entry->Options = PoolPrint(L"%a", prop->string);
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);
    }
  }
  prop = GetProperty(dictPointer, "Title");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->FullTitle) {
      FreePool(Entry->FullTitle);
      Entry->FullTitle = NULL;
    }
    if (Entry->Title) {
      FreePool(Entry->Title);
    }
    Entry->Title = PoolPrint(L"%a", prop->string);
  }
  prop = GetProperty(dictPointer, "FullTitle");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->FullTitle) {
      FreePool(Entry->FullTitle);
    }
    Entry->FullTitle = PoolPrint(L"%a", prop->string);
  } 
  prop = GetProperty(dictPointer, "Image");
  if (prop) {
    if (Entry->ImagePath) {
      FreePool(Entry->ImagePath);
      Entry->ImagePath = NULL;
    }
    if (Entry->Image) {
      egFreeImage(Entry->Image);
      Entry->Image = NULL;
    }
    if (prop->type == kTagTypeString) {
      Entry->ImagePath = PoolPrint(L"%a", prop->string);
    }
  } else {
    prop = GetProperty(dictPointer, "ImageData");
    if (prop) {
      if (Entry->Image) {
        egFreeImage(Entry->Image);
        Entry->Image = NULL;
      }
      if (Entry->ImagePath) {
        FreePool(Entry->ImagePath);
        Entry->ImagePath = NULL;
      }
      if (prop->type == kTagTypeString) {
        UINT32 len = (UINT32)(AsciiStrLen(prop->string) >> 1);
        if (len > 0) {
          UINT8 *data = (UINT8 *)AllocateZeroPool(len);
          if (data) {
            Entry->Image = egDecodeImage(data, hex2bin(prop->string, data, len), NULL, TRUE);
          }
        }
      } else if (prop->type == kTagTypeData) {
        Entry->Image = egDecodeImage(prop->data, prop->dataLen, NULL, TRUE);
      }
    }
  }
  prop = GetProperty(dictPointer, "DriveImage");
  if (prop) {
    if (Entry->DriveImagePath) {
      FreePool(Entry->DriveImagePath);
      Entry->DriveImagePath = NULL;
    }
    if (Entry->DriveImage) {
      egFreeImage(Entry->DriveImage);
      Entry->DriveImage = NULL;
    }
    if (prop->type == kTagTypeString) {
      Entry->DriveImagePath = PoolPrint(L"%a", prop->string);
    }
  } else {
    prop = GetProperty(dictPointer, "DriveImageData");
    if (prop) {
      if (Entry->DriveImage) {
        egFreeImage(Entry->DriveImage);
        Entry->Image = NULL;
      }
      if (Entry->DriveImagePath) {
        FreePool(Entry->DriveImagePath);
        Entry->DriveImagePath = NULL;
      }
      if (prop->type == kTagTypeString) {
        UINT32 len = (UINT32)(AsciiStrLen(prop->string) >> 1);
        if (len > 0) {
          UINT8 *data = (UINT8 *)AllocateZeroPool(len);
          if (data) {
            Entry->DriveImage = egDecodeImage(data, hex2bin(prop->string, data, len), NULL, TRUE);
          }
        }
      } else if (prop->type == kTagTypeData) {
        Entry->DriveImage = egDecodeImage(prop->data, prop->dataLen, NULL, TRUE);
      }
    }
  }
  prop = GetProperty(dictPointer, "Hotkey");
  if (prop && (prop->type == kTagTypeString) && prop->string) {
    Entry->Hotkey = *(prop->string);
  }
  prop = GetProperty(dictPointer, "BootBgColor");
  if (prop && prop->type == kTagTypeString) {
    UINTN   Color;
    Color = AsciiStrHexToUintn(prop->string);
    Entry->BootBgColor = AllocateZeroPool(sizeof(EG_PIXEL));
    Entry->BootBgColor->r = (Color >> 24) & 0xFF;
    Entry->BootBgColor->g = (Color >> 16) & 0xFF;
    Entry->BootBgColor->b = (Color >> 8) & 0xFF;
    Entry->BootBgColor->a = (Color >> 0) & 0xFF;
  }
  prop = GetProperty(dictPointer, "Hidden");
  if (prop) {
    if ((prop->type == kTagTypeTrue) ||
        ((prop->type == kTagTypeString) && prop->string &&
         ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_HIDDEN);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_HIDDEN);
    }
  }
  prop = GetProperty(dictPointer, "Disabled");
  if (prop) {
    if ((prop->type == kTagTypeTrue) ||
        ((prop->type == kTagTypeString) && prop->string &&
         ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_DISABLED);
    }
  }
  prop = GetProperty(dictPointer, "Type");
  if (prop && (prop->type == kTagTypeString)) {
    if (AsciiStriCmp(prop->string, "OSX") == 0) {
      Entry->Type = OSTYPE_OSX;
    } else if (AsciiStriCmp(prop->string, "OSXInstaller") == 0) {
      Entry->Type = OSTYPE_OSX_INSTALLER;
    } else if (AsciiStriCmp(prop->string, "OSXRecovery") == 0) {
      Entry->Type = OSTYPE_RECOVERY;
    } else if (AsciiStriCmp(prop->string, "Windows") == 0) {
      Entry->Type = OSTYPE_WINEFI;
    } else if (AsciiStriCmp(prop->string, "Linux") == 0) {
      Entry->Type = OSTYPE_LIN;
    } else if (AsciiStriCmp(prop->string, "LinuxKernel") == 0) {
      Entry->Type = OSTYPE_LINEFI;
    } else {
      Entry->Type = OSTYPE_OTHER;
    }
  }
  // fix the type if it's forgotten or incorrect
  OSType = GetOSTypeFromPath(Entry->Path);
  if ((Entry->Type != OSType) && ((OSType != OSTYPE_OTHER) || (Entry->Type == 0))) {
    Entry->Type = OSType;
  }
  prop = GetProperty(dictPointer, "VolumeType");
  if (prop && (prop->type == kTagTypeString)) {
    if (AsciiStriCmp(prop->string, "Internal") == 0) {
      Entry->VolumeType = VOLTYPE_INTERNAL;
    } else if (AsciiStriCmp(prop->string, "External") == 0) {
      Entry->VolumeType = VOLTYPE_EXTERNAL;
    } else if (AsciiStriCmp(prop->string, "Optical") == 0) {
      Entry->VolumeType = VOLTYPE_OPTICAL;
    } else if (AsciiStriCmp(prop->string, "FireWire") == 0) {
      Entry->VolumeType = VOLTYPE_FIREWIRE;
    }
  } else {
    INTN i, count = GetTagCount(prop);
    if (count > 0)
    {
      TagPtr prop2;
      for (i = 0; i < count; ++i) {
        if (EFI_ERROR(GetElement(prop, i, &prop2))) {
          continue;
        }
        if (prop2 == NULL) {
          break;
        }
        if ((prop2->type != kTagTypeString) || (prop2->string == NULL)) {
          continue;
        }
        if (AsciiStriCmp(prop2->string, "Internal") == 0) {
          Entry->VolumeType |= VOLTYPE_INTERNAL;
        } else if (AsciiStriCmp(prop2->string, "External") == 0) {
          Entry->VolumeType |= VOLTYPE_EXTERNAL;
        } else if (AsciiStriCmp(prop2->string, "Optical") == 0) {
          Entry->VolumeType |= VOLTYPE_OPTICAL;
        } else if (AsciiStriCmp(prop2->string, "FireWire") == 0) {
          Entry->VolumeType |= VOLTYPE_FIREWIRE;
        }
      }
    }
  }
  if (Entry->Options == NULL) {
    if (OSTYPE_IS_WINDOWS(Entry->Type)) {
      Entry->Options = L"-s -h";
    }
  }
  if (Entry->Title == NULL) {
    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
      Entry->Title = PoolPrint(L"Recovery");
    } else if (OSTYPE_IS_OSX_INSTALLER(Entry->Type)) {
      Entry->Title = PoolPrint(L"Install OSX");
    }
  }
  if ((Entry->Image == NULL) && (Entry->ImagePath == NULL)) {
    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
      Entry->ImagePath = L"mac";
    }
  }
  if ((Entry->DriveImage == NULL) && (Entry->DriveImagePath == NULL)) {
    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
      Entry->DriveImagePath = L"recovery";
    }
  }

  // OS Specific flags
  if (OSTYPE_IS_OSX(Entry->Type) || OSTYPE_IS_OSX_RECOVERY(Entry->Type) || OSTYPE_IS_OSX_INSTALLER(Entry->Type)) {
     prop = GetProperty(dictPointer, "InjectKexts");
     if (prop) {
       if ((prop->type == kTagTypeTrue) ||
           ((prop->type == kTagTypeString) && prop->string &&
            ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
         Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
       } else {
         Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_WITHKEXTS);
       }
     }
     prop = GetProperty(dictPointer, "NoCaches");
     if (prop) {
       if ((prop->type == kTagTypeTrue) ||
           ((prop->type == kTagTypeString) && prop->string &&
            ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
         Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
       } else {
         Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_NOCACHES);
       }
     }
  }
  if (Entry->Type == OSTYPE_LINEFI) {
    prop = GetProperty(dictPointer, "Kernel");
    if (prop) {
      if ((prop->type == kTagTypeString) && prop->string) {
        if ((prop->string[0] == 'N') || (prop->string[0] == 'n')) {
          Entry->KernelScan = KERNEL_SCAN_NEWEST;
        } else if ((prop->string[0] == 'O') || (prop->string[0] == 'o')) {
          Entry->KernelScan = KERNEL_SCAN_OLDEST;
        } else if ((prop->string[0] == 'F') || (prop->string[0] == 'f')) {
          Entry->KernelScan = KERNEL_SCAN_FIRST;
        } else if ((prop->string[0] == 'L') || (prop->string[0] == 'l')) {
          Entry->KernelScan = KERNEL_SCAN_LAST;
        } else if ((prop->string[0] == 'M') || (prop->string[0] == 'm')) {
          Entry->KernelScan = KERNEL_SCAN_MOSTRECENT;
        } else if ((prop->string[0] == 'E') || (prop->string[0] == 'e')) {
          Entry->KernelScan = KERNEL_SCAN_EARLIEST;
        }
      }
    }
  }

  // Sub entries
  prop = GetProperty(dictPointer, "SubEntries");
  if (prop) {
    if (prop->type == kTagTypeFalse) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
    } else if (prop->type != kTagTypeTrue) {
      CUSTOM_LOADER_ENTRY *SubEntry;
      INTN   i, Count = GetTagCount(prop);
      TagPtr dict = NULL;
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
      if (Count > 0) {
        for (i = 0; i < Count; ++i) {
          if (EFI_ERROR(GetElement(prop, i, &dict))) {
            continue;
          }
          if (dict == NULL) {
            break;
          }
          // Allocate a sub entry
          SubEntry = DuplicateCustomEntry(Entry);
          if (SubEntry) {
            if (!FillinCustomEntry(SubEntry, dict, TRUE) || !AddCustomSubEntry(Entry, SubEntry)) {
              FreePool(SubEntry);
            }
          }
        }
      }
    }
  }
  return TRUE;
}
static BOOLEAN FillinCustomLegacy(IN OUT CUSTOM_LEGACY_ENTRY *Entry, TagPtr dictPointer)
{
  TagPtr prop;
  if ((Entry == NULL) || (dictPointer == NULL)) {
    return FALSE;
  }
  prop = GetProperty(dictPointer, "Volume");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Volume) {
      FreePool(Entry->Volume);
    }
    Entry->Volume = PoolPrint(L"%a", prop->string);
  }
  prop = GetProperty(dictPointer, "FullTitle");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->FullTitle) {
      FreePool(Entry->FullTitle);
    }
    Entry->FullTitle = PoolPrint(L"%a", prop->string);
  }
  prop = GetProperty(dictPointer, "Title");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Title) {
      FreePool(Entry->Title);
    }
    Entry->Title = PoolPrint(L"%a", prop->string);
  }
    prop = GetProperty(dictPointer, "Image");
  if (prop) {
    if (Entry->ImagePath) {
      FreePool(Entry->ImagePath);
      Entry->ImagePath = NULL;
    }
    if (Entry->Image) {
      egFreeImage(Entry->Image);
      Entry->Image = NULL;
    }
    if (prop->type == kTagTypeString) {
      Entry->ImagePath = PoolPrint(L"%a", prop->string);
    }
  } else {
    prop = GetProperty(dictPointer, "ImageData");
    if (prop) {
      if (Entry->Image) {
        egFreeImage(Entry->Image);
        Entry->Image = NULL;
      }
      if (prop->type == kTagTypeString) {
        UINT32 len = (UINT32)(AsciiStrLen(prop->string) >> 1);
        if (len > 0) {
          UINT8 *data = (UINT8 *)AllocateZeroPool(len);
          if (data) {
            Entry->Image = egDecodeImage(data, hex2bin(prop->string, data, len), NULL, TRUE);
          }
        }
      } else if (prop->type == kTagTypeData) {
        Entry->Image = egDecodeImage(prop->data, prop->dataLen, NULL, TRUE);
      }
    }
  }
  prop = GetProperty(dictPointer, "DriveImage");
  if (prop) {
    if (Entry->DriveImagePath) {
      FreePool(Entry->DriveImagePath);
      Entry->DriveImagePath = NULL;
    }
    if (Entry->DriveImage) {
      egFreeImage(Entry->DriveImage);
      Entry->DriveImage = NULL;
    }
    if (prop->type == kTagTypeString) {
      Entry->DriveImagePath = PoolPrint(L"%a", prop->string);
    }
  } else {
    prop = GetProperty(dictPointer, "DriveImageData");
    if (prop) {
      if (Entry->DriveImage) {
        egFreeImage(Entry->DriveImage);
        Entry->Image = NULL;
      }
      if (Entry->DriveImagePath) {
        FreePool(Entry->DriveImagePath);
        Entry->DriveImagePath = NULL;
      }
      if (prop->type == kTagTypeString) {
        UINT32 len = (UINT32)(AsciiStrLen(prop->string) >> 1);
        if (len > 0) {
          UINT8 *data = (UINT8 *)AllocateZeroPool(len);
          if (data) {
            Entry->DriveImage = egDecodeImage(data, hex2bin(prop->string, data, len), NULL, TRUE);
          }
        }
      } else if (prop->type == kTagTypeData) {
        Entry->DriveImage = egDecodeImage(prop->data, prop->dataLen, NULL, TRUE);
      }
    }
  }
  prop = GetProperty(dictPointer, "Hotkey");
  if (prop && (prop->type == kTagTypeString) && prop->string) {
    Entry->Hotkey = *(prop->string);
  }
  prop = GetProperty(dictPointer, "Hidden");
  if (prop) {
    if ((prop->type == kTagTypeTrue) ||
        ((prop->type == kTagTypeString) && prop->string &&
         ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_HIDDEN);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_HIDDEN);
    }
  }
  prop = GetProperty(dictPointer, "Disabled");
  if (prop) {
    if ((prop->type == kTagTypeTrue) ||
        ((prop->type == kTagTypeString) && prop->string &&
         ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_DISABLED);
    }
  }
  prop = GetProperty(dictPointer, "Type");
  if (prop && (prop->type == kTagTypeString)) {
    if (AsciiStriCmp(prop->string, "Windows") == 0) {
      Entry->Type = OSTYPE_WIN;
    } else if (AsciiStriCmp(prop->string, "Linux") == 0) {
      Entry->Type = OSTYPE_LIN;
    } else {
      Entry->Type = OSTYPE_OTHER;
    }
  }
  prop = GetProperty(dictPointer, "VolumeType");
  if (prop && (prop->type == kTagTypeString)) {
    if (AsciiStriCmp(prop->string, "Internal") == 0) {
      Entry->VolumeType = VOLTYPE_INTERNAL;
    } else if (AsciiStriCmp(prop->string, "External") == 0) {
      Entry->VolumeType = VOLTYPE_EXTERNAL;
    } else if (AsciiStriCmp(prop->string, "Optical") == 0) {
      Entry->VolumeType = VOLTYPE_OPTICAL;
    } else if (AsciiStriCmp(prop->string, "FireWire") == 0) {
      Entry->VolumeType = VOLTYPE_FIREWIRE;
    }
  } else {
    INTN i, count = GetTagCount(prop);
    if (count > 0)
    {
      TagPtr prop2;
      for (i = 0; i < count; ++i) {
        if (EFI_ERROR(GetElement(prop, i, &prop2))) {
          continue;
        }
        if (prop2 == NULL) {
          break;
        }
        if ((prop2->type != kTagTypeString) || (prop2->string == NULL)) {
          continue;
        }
        if (AsciiStriCmp(prop2->string, "Internal") == 0) {
          Entry->VolumeType |= VOLTYPE_INTERNAL;
        } else if (AsciiStriCmp(prop2->string, "External") == 0) {
          Entry->VolumeType |= VOLTYPE_EXTERNAL;
        } else if (AsciiStriCmp(prop2->string, "Optical") == 0) {
          Entry->VolumeType |= VOLTYPE_OPTICAL;
        } else if (AsciiStriCmp(prop2->string, "FireWire") == 0) {
          Entry->VolumeType |= VOLTYPE_FIREWIRE;
        }
      }
    }
  }
  return TRUE;
}
static BOOLEAN FillinCustomTool(IN OUT CUSTOM_TOOL_ENTRY *Entry, TagPtr dictPointer)
{
  TagPtr prop;
  if ((Entry == NULL) || (dictPointer == NULL)) {
    return FALSE;
  }
  prop = GetProperty(dictPointer, "Volume");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Volume) {
      FreePool(Entry->Volume);
    }
    Entry->Volume = PoolPrint(L"%a", prop->string);
  }
  prop = GetProperty(dictPointer, "Path");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Path) {
      FreePool(Entry->Path);
    }
    Entry->Path = PoolPrint(L"%a", prop->string);
  }
  prop = GetProperty(dictPointer, "Arguments");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Options) {
      FreePool(Entry->Options);
    } else {
      Entry->Options = PoolPrint(L"%a", prop->string);
    }
  }
  prop = GetProperty(dictPointer, "FullTitle");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->FullTitle) {
      FreePool(Entry->FullTitle);
    }
    Entry->FullTitle = PoolPrint(L"%a", prop->string);
  }
  prop = GetProperty(dictPointer, "Title");
  if (prop && (prop->type == kTagTypeString)) {
    if (Entry->Title) {
      FreePool(Entry->Title);
    }
    Entry->Title = PoolPrint(L"%a", prop->string);
  }
    prop = GetProperty(dictPointer, "Image");
  if (prop) {
    if (Entry->ImagePath) {
      FreePool(Entry->ImagePath);
      Entry->ImagePath = NULL;
    }
    if (Entry->Image) {
      egFreeImage(Entry->Image);
      Entry->Image = NULL;
    }
    if (prop->type == kTagTypeString) {
      Entry->ImagePath = PoolPrint(L"%a", prop->string);
    }
  } else {
    prop = GetProperty(dictPointer, "ImageData");
    if (prop) {
      if (Entry->Image) {
        egFreeImage(Entry->Image);
        Entry->Image = NULL;
      }
      if (prop->type == kTagTypeString) {
        UINT32 len = (UINT32)(AsciiStrLen(prop->string) >> 1);
        if (len > 0) {
          UINT8 *data = (UINT8 *)AllocateZeroPool(len);
          if (data) {
            Entry->Image = egDecodeImage(data, hex2bin(prop->string, data, len), NULL, TRUE);
          }
        }
      } else if (prop->type == kTagTypeData) {
        Entry->Image = egDecodeImage(prop->data, prop->dataLen, NULL, TRUE);
      }
    }
  }
  prop = GetProperty(dictPointer, "Hotkey");
  if (prop && (prop->type == kTagTypeString) && prop->string) {
    Entry->Hotkey = *(prop->string);
  }
  prop = GetProperty(dictPointer, "Hidden");
  if (prop) {
    if ((prop->type == kTagTypeTrue) ||
        ((prop->type == kTagTypeString) && prop->string &&
         ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_HIDDEN);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_HIDDEN);
    }
  }
  prop = GetProperty(dictPointer, "Disabled");
  if (prop) {
    if ((prop->type == kTagTypeTrue) ||
        ((prop->type == kTagTypeString) && prop->string &&
         ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
    } else {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_DISABLED);
    }
  }
  prop = GetProperty(dictPointer, "VolumeType");
  if (prop && (prop->type == kTagTypeString)) {
    if (AsciiStriCmp(prop->string, "Internal") == 0) {
      Entry->VolumeType = VOLTYPE_INTERNAL;
    } else if (AsciiStriCmp(prop->string, "External") == 0) {
      Entry->VolumeType = VOLTYPE_EXTERNAL;
    } else if (AsciiStriCmp(prop->string, "Optical") == 0) {
      Entry->VolumeType = VOLTYPE_OPTICAL;
    } else if (AsciiStriCmp(prop->string, "FireWire") == 0) {
      Entry->VolumeType = VOLTYPE_FIREWIRE;
    }
  } else {
    INTN i, count = GetTagCount(prop);
    if (count > 0)
    {
      TagPtr prop2;
      for (i = 0; i < count; ++i) {
        if (EFI_ERROR(GetElement(prop, i, &prop2))) {
          continue;
        }
        if (prop2 == NULL) {
          break;
        }
        if ((prop2->type != kTagTypeString) || (prop2->string == NULL)) {
          continue;
        }
        if (AsciiStriCmp(prop2->string, "Internal") == 0) {
          Entry->VolumeType |= VOLTYPE_INTERNAL;
        } else if (AsciiStriCmp(prop2->string, "External") == 0) {
          Entry->VolumeType |= VOLTYPE_EXTERNAL;
        } else if (AsciiStriCmp(prop2->string, "Optical") == 0) {
          Entry->VolumeType |= VOLTYPE_OPTICAL;
        } else if (AsciiStriCmp(prop2->string, "FireWire") == 0) {
          Entry->VolumeType |= VOLTYPE_FIREWIRE;
        }
      }
    }
  }
  return TRUE;
}

EFI_STATUS GetEarlyUserSettings(IN EFI_FILE *RootDir, TagPtr CfgDict)
{
  EFI_STATUS	Status = EFI_SUCCESS;
  TagPtr      dict;
  TagPtr      dict2;
  TagPtr      dictPointer;
  TagPtr      prop;
 // CHAR8       ANum[4];
 // UINTN       i = 0;

    dict = CfgDict;
 if(dict != NULL) {
  DBG("Loading early settings\n");
  
  dictPointer = GetProperty(dict, "Boot");
  if (dictPointer) {
    prop = GetProperty(dictPointer, "Timeout");
    if (prop) {
      if (prop->type == kTagTypeInteger) {
        GlobalConfig.Timeout = (INT32)(UINTN)prop->string;
        DBG("timeout set to %d\n", GlobalConfig.Timeout);
      } else if ((prop->type == kTagTypeString) && prop->string) {
        if (prop->string[0] == '-') {
          GlobalConfig.Timeout = -1;
        } else {
          GlobalConfig.Timeout = (INTN)AsciiStrDecimalToUintn(prop->string);
        }
      }
    }
    prop = GetProperty(dictPointer, "Arguments");
    if (prop && (prop->type == kTagTypeString) && prop->string) {
      AsciiStrnCpy(gSettings.BootArgs, prop->string, 255);
    }
    
    prop = GetProperty(dictPointer, "DefaultVolume");
    if(prop) {
      gSettings.DefaultVolume = AllocateZeroPool(AsciiStrSize(prop->string) * sizeof(CHAR16));
      AsciiStrToUnicodeStr(prop->string, gSettings.DefaultVolume);
    }
    
    prop = GetProperty(dictPointer, "DefaultLoader");
    if(prop) {
      gSettings.DefaultLoader = AllocateZeroPool(AsciiStrSize(prop->string) * sizeof(CHAR16));
      AsciiStrToUnicodeStr(prop->string, gSettings.DefaultLoader);
    }

    prop = GetProperty(dictPointer, "Log");
    if (prop) {
      if ((prop->type == kTagTypeTrue) ||
          ((prop->type == kTagTypeString) && prop->string &&
           ((prop->string[0] == 'Y') || (prop->string[0] == 'y')))) {
        GlobalConfig.DebugLog = TRUE;
      }
    }

    prop = GetProperty(dictPointer, "Fast");
    if (prop) {
      if ((prop->type == kTagTypeTrue) ||
          ((prop->type == kTagTypeString) && prop->string &&
           ((prop->string[0] == 'Y') || (prop->string[0] == 'y')))) {
            GlobalConfig.FastBoot = TRUE;
          }
    } 
    prop = GetProperty(dictPointer, "NeverHibernate");
    if (prop) {
      if ((prop->type == kTagTypeTrue) ||
          ((prop->type == kTagTypeString) && prop->string &&
           ((prop->string[0] == 'Y') || (prop->string[0] == 'y')))) {
            GlobalConfig.NeverHibernate = TRUE;
          }
    }

    prop = GetProperty(dictPointer, "IgnoreNVRAMBoot");
    if (prop) {
      if ((prop->type == kTagTypeTrue) ||
          ((prop->type == kTagTypeString) && prop->string &&
           ((prop->string[0] == 'Y') || (prop->string[0] == 'y')))) {
            GlobalConfig.IgnoreNVRAMBoot = TRUE;
          }
    }
    
    // Secure boot
    prop = GetProperty(dictPointer, "Secure");
    if (prop) {
      if (prop->type == kTagTypeFalse) {
        // Only disable setup mode, we want always secure boot
        gSettings.SecureBootSetupMode = 0;
      } else if ((prop->type == kTagTypeTrue) && !gSettings.SecureBoot) {
        // This mode will force boot policy even when no secure boot or it is disabled
        gSettings.SecureBootSetupMode = 1;
        gSettings.SecureBoot = 1;
      }
    }
    // Secure boot policy
    prop = GetProperty(dictPointer, "Policy");
    if (prop && (prop->type == kTagTypeString) && prop->string) {
      if ((prop->string[0] == 'D') || (prop->string[0] == 'd')) {
        // Deny all images
        gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_DENY;
      } else if ((prop->string[0] == 'A') || (prop->string[0] == 'a')) {
        // Allow all images
        gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_ALLOW;
      } else if ((prop->string[0] == 'Q') || (prop->string[0] == 'q')) {
        // Query user
        gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_QUERY;
      } else if ((prop->string[0] == 'I') || (prop->string[0] == 'i')) {
        // Insert
        gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_INSERT;
      } else if ((prop->string[0] == 'W') || (prop->string[0] == 'w')) {
        // White list
        gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_WHITELIST;
      } else if ((prop->string[0] == 'B') || (prop->string[0] == 'b')) {
        // Black list
        gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_BLACKLIST;
      } else if ((prop->string[0] == 'U') || (prop->string[0] == 'u')) {
        // User policy
        gSettings.SecureBootPolicy = SECURE_BOOT_POLICY_USER;
      }
    }
    // Secure boot white list
    prop = GetProperty(dictPointer, "WhiteList");
    if (prop && (prop->type == kTagTypeArray)) {
      INTN i, Count = GetTagCount(prop);
      if (Count > 0) {
        gSettings.SecureBootWhiteListCount = 0;
        gSettings.SecureBootWhiteList = AllocateZeroPool(Count * sizeof(CHAR16 *));
        if (gSettings.SecureBootWhiteList) {
          for (i = 0; i < Count; ++i) {
            if (EFI_ERROR(GetElement(prop, i, &dict2))) {
              continue;
            }
            if (dict2 == NULL) {
              break;
            }
            if ((dict2->type == kTagTypeString) && dict2->string) {
              gSettings.SecureBootWhiteList[gSettings.SecureBootWhiteListCount++] = PoolPrint(L"%a", dict2->string);
            }
          }
        }
      }
    }
    // Secure boot black list
    prop = GetProperty(dictPointer, "BlackList");
    if (prop && (prop->type == kTagTypeArray)) {
      INTN i, Count = GetTagCount(prop);
      if (Count > 0) {
        gSettings.SecureBootBlackListCount = 0;
        gSettings.SecureBootBlackList = AllocateZeroPool(Count * sizeof(CHAR16 *));
        if (gSettings.SecureBootBlackList) {
          for (i = 0; i < Count; ++i) {
            if (EFI_ERROR(GetElement(prop, i, &dict2))) {
              continue;
            }
            if (dict2 == NULL) {
              break;
            }
            if ((dict2->type == kTagTypeString) && dict2->string) {
              gSettings.SecureBootBlackList[gSettings.SecureBootBlackListCount++] = PoolPrint(L"%a", dict2->string);
            }
          }
        }
      }
    }

    // XMP memory profiles
    prop = GetProperty(dictPointer, "XMPDetection");
    if (prop) {
      gSettings.XMPDetection = 0;
      if (prop->type == kTagTypeFalse) {
        gSettings.XMPDetection = -1;
      } else if (prop->type == kTagTypeString) {
        if ((prop->string[0] == 'n') ||
            (prop->string[0] == 'N') ||
            (prop->string[0] == '-')) {
          gSettings.XMPDetection = -1;
        } else {
          gSettings.XMPDetection = (INT8)AsciiStrDecimalToUintn(prop->string);
        }
      } else if (prop->type == kTagTypeInteger) {
        gSettings.XMPDetection = (INT8)(UINTN)prop->string;
      }
      // Check that the setting value is sane
      if ((gSettings.XMPDetection < -1) || (gSettings.XMPDetection > 2)) {
        gSettings.XMPDetection = -1;
      }
    }

    // Legacy bios protocol
    prop = GetProperty(dictPointer, "Legacy");
    if(prop)  {
      AsciiStrToUnicodeStr(prop->string, gSettings.LegacyBoot);
    } else if (gFirmwareClover) {
      // default for CLOVER EFI boot
      UnicodeSPrint(gSettings.LegacyBoot, sizeof(gSettings.LegacyBoot), L"PBR");
    } else {
      // default for UEFI boot
      UnicodeSPrint(gSettings.LegacyBoot, sizeof(gSettings.LegacyBoot), L"LegacyBiosDefault");
    }

    // Entry for LegacyBiosDefault
    prop = GetProperty(dictPointer, "LegacyBiosDefaultEntry");
    if (prop) {
      if (prop->type == kTagTypeInteger) {
        gSettings.LegacyBiosDefaultEntry = (UINT16)(UINTN)prop->string;
      } else if ((prop->type == kTagTypeString) && prop->string) {
        gSettings.LegacyBiosDefaultEntry = (UINT16)AsciiStrDecimalToUintn(prop->string);
      }
    } else {
	gSettings.LegacyBiosDefaultEntry = 0; // disabled by default
    }
  }

  dictPointer = GetProperty(dict, "GUI");
  if (dictPointer) {
    prop = GetProperty(dictPointer, "Theme");
    if (prop) {
      if ((prop->type == kTagTypeString) && prop->string) {
        GlobalConfig.Theme = PoolPrint(L"%a", prop->string);
        DBG("Default theme: %s\n", GlobalConfig.Theme);
      }
    }
    //CustomIcons
    prop = GetProperty(dictPointer, "CustomIcons");
    if (prop) {
      if ((prop->type == kTagTypeTrue) ||
          ((prop->type == kTagTypeString) && prop->string &&
           ((prop->string[0] == 'Y') || (prop->string[0] == 'y')))) {
        GlobalConfig.CustomIcons = TRUE;
      }
    }

    prop = GetProperty(dictPointer, "TextOnly");
    if (prop) {
      if ((prop->type == kTagTypeTrue) ||
          ((prop->type == kTagTypeString) && prop->string &&
           ((prop->string[0] == 'Y') || (prop->string[0] == 'y')))) {
        GlobalConfig.TextOnly = TRUE;
      }
    }

    prop = GetProperty(dictPointer, "ScreenResolution");
    if (prop) {
      if ((prop->type == kTagTypeString) && prop->string) {
        GlobalConfig.ScreenResolution = PoolPrint(L"%a", prop->string);
      }
    }

    prop = GetProperty(dictPointer, "ConsoleMode");
    if (prop) {
      if (prop->type == kTagTypeInteger) {
        GlobalConfig.ConsoleMode = (INT32)(UINTN)prop->string;
      } else if ((prop->type == kTagTypeString) && prop->string) {
	if (AsciiStrStr(prop->string, "Max") != NULL) {
          GlobalConfig.ConsoleMode = -1;
          DBG("ConsoleMode will be set to highest mode\n");
	} else if (AsciiStrStr(prop->string, "Min") != NULL) {
          GlobalConfig.ConsoleMode = -2;
          DBG("ConsoleMode will be set to lowest mode\n");
        } else {
          GlobalConfig.ConsoleMode = (INT32)AsciiStrDecimalToUintn(prop->string);
        }
      }
      if (GlobalConfig.ConsoleMode > 0) {
        DBG("ConsoleMode will be set to mode #%d\n", GlobalConfig.ConsoleMode);
      }
    }

    prop = GetProperty(dictPointer, "Language");
    if(prop) {
      //       AsciiStrToUnicodeStr(prop->string, gSettings.Language);
      AsciiStrCpy(gSettings.Language,  prop->string);
      if (AsciiStrStr(prop->string, "en")) {
        gLanguage = english;
      } else if (AsciiStrStr(prop->string, "ru")) {
        gLanguage = russian;
      } else if (AsciiStrStr(prop->string, "fr")) {
        gLanguage = french;
      } else if (AsciiStrStr(prop->string, "it")) {
        gLanguage = italian;
      } else if (AsciiStrStr(prop->string, "es")) {
        gLanguage = spanish;
      } else if (AsciiStrStr(prop->string, "pt")) {
        gLanguage = portuguese;
      } else if (AsciiStrStr(prop->string, "br")) {
        gLanguage = brasil; 
      } else if (AsciiStrStr(prop->string, "de")) {
        gLanguage = german;
      } else if (AsciiStrStr(prop->string, "nl")) {
        gLanguage = dutch;
      } else if (AsciiStrStr(prop->string, "pl")) {
        gLanguage = polish; 
      } else if (AsciiStrStr(prop->string, "ua")) {
        gLanguage = ukrainian;
      } else if (AsciiStrStr(prop->string, "cz")) {
        gLanguage = czech;
      } else if (AsciiStrStr(prop->string, "hr")) {
        gLanguage = croatian;
      } else if (AsciiStrStr(prop->string, "id")) {
        gLanguage = indonesian; 
      } else if (AsciiStrStr(prop->string, "ko")) {
        gLanguage = korean;
      }
    }

    prop = GetProperty(dictPointer, "Mouse");
    if (prop) {
      dict2 = GetProperty(prop, "Speed");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          gSettings.PointerSpeed = (INT32)(UINTN)dict2->string;
          if (gSettings.PointerSpeed < 0) {
            gSettings.PointerSpeed = -gSettings.PointerSpeed;
          }
        } else if ((dict2->type == kTagTypeString) && dict2->string) {
          gSettings.PointerSpeed = (UINT16)AsciiStrDecimalToUintn(dict2->string + ((dict2->string[0] == '-') ? 1 : 0));
        }
        gSettings.PointerEnabled = (gSettings.PointerSpeed != 0);
      }
      //but we can disable mouse even if there was positive speed
      dict2 = GetProperty(prop, "Enabled");
      if (dict2) {
        if ((dict2->type == kTagTypeFalse) ||
            ((dict2->type == kTagTypeString) && dict2->string &&
             ((dict2->string[0] == 'N') || (dict2->string[0] == 'n')))) {
              gSettings.PointerEnabled = FALSE;
            }
      }
      dict2 = GetProperty(prop, "Mirror");
      if (dict2) {
        if ((dict2->type == kTagTypeTrue) ||
            ((dict2->type == kTagTypeString) && dict2->string &&
            ((dict2->string[0] == 'Y') || (dict2->string[0] == 'y')))) {
          gSettings.PointerMirror = TRUE;
        }
      }
      dict2 = GetProperty(prop, "DoubleClickTime");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          gSettings.DoubleClickTime = (UINTN)dict2->string;
        } else if (dict2->type == kTagTypeString) {
          gSettings.DoubleClickTime = (UINT16)AsciiStrDecimalToUintn(dict2->string + ((dict2->string[0] == '-') ? 1 : 0));
        }
      }
    }
    // hide by name/uuid
    prop = GetProperty(dictPointer, "Hide");
    if (prop) {
      INTN i, Count = GetTagCount(prop);
      if (Count > 0) {
        gSettings.HVCount = 0;
        gSettings.HVHideStrings = AllocateZeroPool(Count * sizeof(CHAR16 *));
        if (gSettings.HVHideStrings) {
          for (i = 0; i < Count; ++i) {
            if (EFI_ERROR(GetElement(prop, i, &dict2))) {
              continue;
            }
            if (dict2 == NULL) {
              break;
            }
            if ((dict2->type == kTagTypeString) && dict2->string) {
              gSettings.HVHideStrings[gSettings.HVCount] = PoolPrint(L"%a", dict2->string);
              if (gSettings.HVHideStrings[gSettings.HVCount]) {
                DBG("Hiding entries with string %s\n", gSettings.HVHideStrings[gSettings.HVCount]);
                gSettings.HVCount++;
              }
            }
          }
        }
      }
    }
    // Disable loader scan
    prop = GetProperty(dictPointer, "Scan");
    if (prop) {
      if ((prop->type == kTagTypeFalse) ||
          ((prop->type == kTagTypeString) && prop->string &&
           ((prop->string[0] == 'N') || (prop->string[0] == 'n')))) {
        gSettings.DisableEntryScan = TRUE;
        gSettings.DisableToolScan = TRUE;
        GlobalConfig.NoLegacy = TRUE;
      } else if (prop->type == kTagTypeDict) {
        dict2 = GetProperty(prop, "Entries");
        if (dict2) {
          if ((dict2->type == kTagTypeFalse) ||
              ((dict2->type == kTagTypeString) && dict2->string &&
              ((dict2->string[0] == 'N') || (dict2->string[0] == 'n')))) {
            gSettings.DisableEntryScan = TRUE;
          }
        }
        dict2 = GetProperty(prop, "Tool");
        if (dict2) {
          if ((dict2->type == kTagTypeFalse) ||
              ((dict2->type == kTagTypeString) && dict2->string &&
              ((dict2->string[0] == 'N') || (dict2->string[0] == 'n')))) {
            gSettings.DisableToolScan = TRUE;
          }
        }
        dict2 = GetProperty(prop, "Legacy");
        if (dict2) {
          if (dict2->type == kTagTypeFalse) {
            GlobalConfig.NoLegacy = TRUE;
          }
          else if ((dict2->type == kTagTypeString) && dict2->string) {
            if ((dict2->string[0] == 'N') || (dict2->string[0] == 'n')) {
              GlobalConfig.NoLegacy = TRUE;
            } else if ((dict2->string[0] == 'F') || (dict2->string[0] == 'f')) {
              GlobalConfig.LegacyFirst = TRUE;
            }
          }
        }
        dict2 = GetProperty(prop, "Kernel");
        if (dict2) {
          if (dict2->type == kTagTypeFalse) {
            gSettings.KernelScan = KERNEL_SCAN_NONE;
          } else if ((dict2->type == kTagTypeString) && dict2->string) {
            if ((dict2->string[0] == 'N') || (dict2->string[0] == 'n')) {
              gSettings.KernelScan = ((dict2->string[1] == 'E') || (dict2->string[1] == 'e')) ? KERNEL_SCAN_NEWEST : KERNEL_SCAN_NONE;
            } else if ((dict2->string[0] == 'O') || (dict2->string[0] == 'o')) {
              gSettings.KernelScan = KERNEL_SCAN_OLDEST;
            } else if ((dict2->string[0] == 'F') || (dict2->string[0] == 'f')) {
              gSettings.KernelScan = KERNEL_SCAN_FIRST;
            } else if ((dict2->string[0] == 'L') || (dict2->string[0] == 'l')) {
              gSettings.KernelScan = KERNEL_SCAN_LAST;
            } else if ((dict2->string[0] == 'M') || (dict2->string[0] == 'm')) {
              gSettings.KernelScan = KERNEL_SCAN_MOSTRECENT;
            } else if ((dict2->string[0] == 'E') || (dict2->string[0] == 'e')) {
              gSettings.KernelScan = KERNEL_SCAN_EARLIEST;
            }
          }
        }
      }
    }
    // Custom entries
    dict2 = GetProperty(dictPointer, "Custom");
    if (dict2) {
      prop = GetProperty(dict2, "Entries");
      if (prop) {
        CUSTOM_LOADER_ENTRY *Entry;
        INTN   i, Count = GetTagCount(prop);
        TagPtr dict3;
        if (Count > 0) {
          for (i = 0; i < Count; ++i) {
            if (EFI_ERROR(GetElement(prop, i, &dict3))) {
              continue;
            }
            if (dict3 == NULL) {
              break;
            }
            // Allocate an entry
            Entry = (CUSTOM_LOADER_ENTRY *)AllocateZeroPool(sizeof(CUSTOM_LOADER_ENTRY));
            if (Entry) {
              // Fill it in
              if (!FillinCustomEntry(Entry, dict3, FALSE) || !AddCustomEntry(Entry)) {
                FreePool(Entry);
              }
            }
          }
        }
      }
      prop = GetProperty(dict2, "Legacy");
      if (prop) {
        CUSTOM_LEGACY_ENTRY *Entry;
        INTN   i, Count = GetTagCount(prop);
        TagPtr dict3;
        if (Count > 0) {
          for (i = 0; i < Count; ++i) {
            if (EFI_ERROR(GetElement(prop, i, &dict3))) {
              continue;
            }
            if (dict3 == NULL) {
              break;
            }
            // Allocate an entry
            Entry = (CUSTOM_LEGACY_ENTRY *)AllocateZeroPool(sizeof(CUSTOM_LEGACY_ENTRY));
            if (Entry) {
              // Fill it in
              if (!FillinCustomLegacy(Entry, dict3) || !AddCustomLegacyEntry(Entry)) {
                FreePool(Entry);
              }
            }
          }
        }
      }
      prop = GetProperty(dict2, "Tool");
      if (prop) {
        CUSTOM_TOOL_ENTRY *Entry;
        INTN   i, Count = GetTagCount(prop);
        TagPtr dict3;
        if (Count > 0) {
          for (i = 0; i < Count; ++i) {
            if (EFI_ERROR(GetElement(prop, i, &dict3))) {
              continue;
            }
            if (dict3 == NULL) {
              break;
            }
            // Allocate an entry
            Entry = (CUSTOM_TOOL_ENTRY *)AllocateZeroPool(sizeof(CUSTOM_TOOL_ENTRY));
            if (Entry) {
              // Fill it in
              if (!FillinCustomTool(Entry, dict3) || !AddCustomToolEntry(Entry)) {
                FreePool(Entry);
              }
            }
          }
        }
      }
    }
  }
  dictPointer = GetProperty(dict, "Graphics");
  if (dictPointer) {
    
    prop = GetProperty(dictPointer, "PatchVBios");
    gSettings.PatchVBios = FALSE;
    if(prop) {
      if ((prop->type == kTagTypeTrue) ||
          (prop->string[0] == 'y') || (prop->string[0] == 'Y'))
        gSettings.PatchVBios = TRUE;
    }
    
    gSettings.PatchVBiosBytesCount = 0;
    dict2 = GetProperty(dictPointer,"PatchVBiosBytes");
    if (dict2) {
      UINTN Count = GetTagCount(dict2);
      if (Count > 0) {
        UINTN     Index = 0;
        VBIOS_PATCH_BYTES *VBiosPatch;
        UINTN     FindSize = 0;
        UINTN     ReplaceSize = 0;
        BOOLEAN   Valid;
      
        // alloc space for up to 16 entries
        gSettings.PatchVBiosBytes = AllocateZeroPool(Count * sizeof(VBIOS_PATCH_BYTES));
      
        // get all entries
        Index = 0;
        for (Index = 0; Index < Count; Index++) {
          // Get the next entry
          if (EFI_ERROR(GetElement(dict2, Index, &prop))) {
             continue;
          }
          if (prop == NULL) {
            break;
          }
          Valid = TRUE;
          // read entry
          VBiosPatch = &gSettings.PatchVBiosBytes[gSettings.PatchVBiosBytesCount];
          VBiosPatch->Find = GetDataSetting(prop, "Find", &FindSize);
          VBiosPatch->Replace = GetDataSetting(prop, "Replace", &ReplaceSize);
          if (VBiosPatch->Find == NULL || FindSize == 0) {
            Valid = FALSE;
            DBG("PatchVBiosBytes[%d]: missing Find data\n", Index);
          }
          if (VBiosPatch->Replace == NULL || ReplaceSize == 0) {
            Valid = FALSE;
            DBG("PatchVBiosBytes[%d]: missing Replace data\n", Index);
          }
          if (FindSize != ReplaceSize) {
            Valid = FALSE;
            DBG("PatchVBiosBytes[%d]: Find and Replace data are not the same size\n", Index);
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
    
    //InjectEDID
    prop = GetProperty(dictPointer, "InjectEDID");
    gSettings.InjectEDID = FALSE;
    if(prop) {
      if ((prop->type == kTagTypeTrue) ||
          (prop->string[0] == 'y') || (prop->string[0] == 'Y'))
        gSettings.InjectEDID = TRUE;
    }
    prop = GetProperty(dictPointer, "CustomEDID");
    if(prop) {
      UINTN j = 128;
      gSettings.CustomEDID = GetDataSetting(dictPointer, "CustomEDID", &j);
      if ((j % 128) != 0) {
        DBG("CustomEDID has wrong length=%d\n", j);
      } else {
        DBG("CustomEDID ok\n");
        InitializeEdidOverride();
      }
    }
    
  }
  dictPointer = GetProperty(dict,"DisableDrivers");
  if(dictPointer) {
    INTN i, Count = GetTagCount(dictPointer);
    if (Count > 0) {
      gSettings.BlackListCount = 0;
      gSettings.BlackList = AllocateZeroPool(Count * sizeof(CHAR16 *));
      for (i = 0; i < Count; ++i) {
        if (!EFI_ERROR(GetElement(dictPointer, i, &prop)) &&
            prop && (prop->type == kTagTypeString)) {
          gSettings.BlackList[gSettings.BlackListCount++] = PoolPrint(L"%a", prop->string);
        }
      }
    }
  }
 }
  return Status;
}
#define CONFIG_THEME_FILENAME L"theme.plist"

VOID GetListOfThemes()
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  REFIT_DIR_ITER          DirIter;
	EFI_FILE_INFO           *DirEntry;
  CHAR16                  *ThemeTestPath;
  EFI_FILE                *ThemeTestDir = NULL;
  CHAR8                   *ThemePtr = NULL;
  UINTN                   Size = 0;

  ThemesNum = 0;
  DirIterOpen(SelfRootDir, L"\\EFI\\CLOVER\\themes", &DirIter);
  while (DirIterNext(&DirIter, 1, L"*.EFI", &DirEntry)) {
    if (DirEntry->FileName[0] == '.'){
      continue;
    }
    DBG("Found theme %s", DirEntry->FileName);
    ThemeTestPath = PoolPrint(L"EFI\\CLOVER\\themes\\%s", DirEntry->FileName);
    if (ThemeTestPath) {
      Status = SelfRootDir->Open(SelfRootDir, &ThemeTestDir, ThemeTestPath, EFI_FILE_MODE_READ, 0);
      if (!EFI_ERROR(Status)) {
        Status = egLoadFile(ThemeTestDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
        if (EFI_ERROR(Status) || (ThemePtr == NULL) || (Size == 0)) {
          Status = EFI_NOT_FOUND;
          DBG(" - no theme.plist");
        } else {
          //we found a theme
          ThemesList[ThemesNum++] = (CHAR16*)AllocateCopyPool(StrSize(DirEntry->FileName), DirEntry->FileName);
          
        }
      }
      FreePool(ThemeTestPath);
    }
    DBG("\n");
  }
  DirIterClose(&DirIter);
}

STATIC EFI_STATUS GetThemeTagSettings(TagPtr dictPointer)
{
  TagPtr dict, dict2;

  //fill default to have an ability change theme
  GlobalConfig.BackgroundScale = Crop;
  if (GlobalConfig.BackgroundName) {
    FreePool(GlobalConfig.BackgroundName);
    GlobalConfig.BackgroundName = NULL;
  }
  GlobalConfig.BackgroundSharp = 0;
  GlobalConfig.BackgroundDark = 0;
  if (GlobalConfig.BannerFileName) {
    FreePool(GlobalConfig.BannerFileName);
    GlobalConfig.BannerFileName = NULL;
  }
  GlobalConfig.HideBadges = 0;
  GlobalConfig.BadgeOffsetX = 0xFFFF;
  GlobalConfig.BadgeOffsetY = 0xFFFF;
  GlobalConfig.BadgeScale = 8; //default
  GlobalConfig.ThemeDesignWidth = 0xFFFF;
  GlobalConfig.ThemeDesignHeight = 0xFFFF;
  GlobalConfig.BannerEdgeHorizontal = SCREEN_EDGE_LEFT;
  GlobalConfig.BannerEdgeVertical = SCREEN_EDGE_TOP;
  GlobalConfig.BannerPosX = 0xFFFF;
  GlobalConfig.BannerPosY = 0xFFFF;
  GlobalConfig.BannerNudgeX = 0;
  GlobalConfig.BannerNudgeY = 0;
  GlobalConfig.VerticalLayout = FALSE;
  GlobalConfig.MainEntriesSize = 128;
  GlobalConfig.TileXSpace = 8;
  GlobalConfig.TileYSpace = 24;
  LayoutBannerOffset = 64; //default value if not set
  LayoutButtonOffset = 0; //default value if not set
  LayoutTextOffset = 0; //default value if not set
  LayoutAnimMoveForMenuX = 0; //default value if not set
  GlobalConfig.HideUIFlags = 0;
  GlobalConfig.SelectionColor = 0x80808080; 
  if (GlobalConfig.SelectionSmallFileName) {
    FreePool(GlobalConfig.SelectionSmallFileName);
    GlobalConfig.SelectionSmallFileName = NULL;
  }
  if (GlobalConfig.SelectionBigFileName) {
    FreePool(GlobalConfig.SelectionBigFileName);
    GlobalConfig.SelectionBigFileName = NULL;
  }
  GlobalConfig.SelectionOnTop = FALSE;
  ScrollWidth = 16;
  ScrollButtonsHeight = 20;
  ScrollBarDecorationsHeight = 5;
  ScrollScrollDecorationsHeight = 7;
  GlobalConfig.Font = FONT_LOAD;
  if (GlobalConfig.FontFileName) {
    FreePool(GlobalConfig.FontFileName);
    GlobalConfig.FontFileName = NULL;
  }
  GlobalConfig.CharWidth = 7;
  GuiAnime = NULL;
  if (BigBack) {
    egFreeImage(BigBack);
    BigBack = NULL;
  }
  if (BackgroundImage) {
    egFreeImage(BackgroundImage);
    BackgroundImage = NULL;
  }
  if (Banner) {
    egFreeImage(Banner);
    Banner  = NULL;
  }
  if (FontImage) {
    egFreeImage(FontImage);
    FontImage = NULL;
  }
  FreeScrollBar();

  // if NULL parameter, quit after setting default values
  if (dictPointer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  dict = GetProperty(dictPointer, "Background");
  if (dict) {
    dict2 = GetProperty(dict, "Type");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        if ((dict2->string[0] == 'S') || (dict2->string[0] == 's')) {
          GlobalConfig.BackgroundScale = Scale;
        } else if ((dict2->string[0] == 'T') || (dict2->string[0] == 't')) {
          GlobalConfig.BackgroundScale = Tile;
        }
      }
    }
    dict2 = GetProperty(dict, "Path");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        GlobalConfig.BackgroundName = PoolPrint(L"%a", dict2->string);
      }
    }
    dict2 = GetProperty(dict, "Sharp");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        GlobalConfig.BackgroundSharp = (UINTN)dict2->string;
      } else if ((dict2->type == kTagTypeString) && dict2->string) {
        GlobalConfig.BackgroundSharp = AsciiStrHexToUintn(dict2->string);
      }
    }
    dict2 = GetProperty(dict, "Dark");
    if (dict2) {
      GlobalConfig.BackgroundDark = (dict2->type == kTagTypeTrue);
    }
  }

  dict = GetProperty(dictPointer, "Banner");
  if (dict) {
    // retain for legacy themes.
    if ((dict->type == kTagTypeString) && dict->string) {
      GlobalConfig.BannerFileName = PoolPrint(L"%a", dict->string);
    } else {
      // for new placement settings
      dict2 = GetProperty(dict, "Path");
      if (dict2) {
        if ((dict2->type == kTagTypeString) && dict2->string) {
          GlobalConfig.BannerFileName = PoolPrint(L"%a", dict2->string);
        }
      }
      dict2 = GetProperty(dict, "ScreenEdgeX");
      if (dict2) {
        if ((dict2->type == kTagTypeString) && dict2->string) {
          if (AsciiStrCmp(dict2->string, "left") == 0) {
            GlobalConfig.BannerEdgeHorizontal = SCREEN_EDGE_LEFT;
          } else if (AsciiStrCmp(dict2->string, "right") == 0) {
            GlobalConfig.BannerEdgeHorizontal = SCREEN_EDGE_RIGHT;
          }
        }
      }
      dict2 = GetProperty(dict, "ScreenEdgeY");
      if (dict2) {
        if ((dict2->type == kTagTypeString) && dict2->string) {
          if (AsciiStrCmp(dict2->string, "top") == 0) {
            GlobalConfig.BannerEdgeVertical = SCREEN_EDGE_TOP;
          } else if (AsciiStrCmp(dict2->string, "bottom") == 0) {
            GlobalConfig.BannerEdgeVertical = SCREEN_EDGE_BOTTOM;
          }
        }
      }
      dict2 = GetProperty(dict, "DistanceFromScreenEdgeX%");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          GlobalConfig.BannerPosX = (INT32)(UINTN)dict2->string;
        }
      }
      dict2 = GetProperty(dict, "DistanceFromScreenEdgeY%");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          GlobalConfig.BannerPosY = (INT32)(UINTN)dict2->string;
        }
      }
      dict2 = GetProperty(dict, "NudgeX");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          GlobalConfig.BannerNudgeX = (INT32)(UINTN)dict2->string;
        }
      }
      dict2 = GetProperty(dict, "NudgeY");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          GlobalConfig.BannerNudgeY = (INT32)(UINTN)dict2->string;
        }
      }
    } 
  }

  dict = GetProperty(dictPointer, "Badges");
  if (dict) {
    dict2 = GetProperty(dict, "Swap");
    if (dict2) {
      if (dict2->type == kTagTypeTrue) {
        GlobalConfig.HideBadges |= HDBADGES_SWAP;
        DBG("OS main and drive as badge\n");
      }
    }
    dict2 = GetProperty(dict, "Show");
    if (dict2) {
      if (dict2->type == kTagTypeTrue) {
        GlobalConfig.HideBadges |= HDBADGES_SHOW;
      }
    }
    dict2 = GetProperty(dict, "Inline");
    if (dict2) {
      if (dict2->type == kTagTypeTrue) {
        GlobalConfig.HideBadges |= HDBADGES_INLINE;
      }
    }
    // blackosx added X and Y position for badge offset.
    dict2 = GetProperty(dict, "OffsetX");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        GlobalConfig.BadgeOffsetX = (UINTN)dict2->string;
      }
    }
    dict2 = GetProperty(dict, "OffsetY");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        GlobalConfig.BadgeOffsetY = (UINTN)dict2->string;
      }
    }
    dict2 = GetProperty(dict, "Scale");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        GlobalConfig.BadgeScale = (UINTN)dict2->string;
      }
    }
  }

  dict = GetProperty(dictPointer, "Origination");
  if (dict) {
    dict2 = GetProperty(dict, "DesignWidth");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        GlobalConfig.ThemeDesignWidth = (UINTN)dict2->string;
      }
    }
    dict2 = GetProperty(dict, "DesignHeight");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        GlobalConfig.ThemeDesignHeight = (UINTN)dict2->string;
      }
    }
  }

  dict = GetProperty(dictPointer, "Layout");
  if (dict) {
    dict2 = GetProperty(dict, "BannerOffset");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        LayoutBannerOffset = (UINTN)dict2->string;
      }
    }    
    dict2 = GetProperty(dict, "ButtonOffset");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        LayoutButtonOffset = (UINTN)dict2->string;
      }
    }  
    dict2 = GetProperty(dict, "TextOffset");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        LayoutTextOffset = (UINTN)dict2->string;
      }
    }
    dict2 = GetProperty(dict, "AnimAdjustForMenuX");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        LayoutAnimMoveForMenuX = (UINTN)dict2->string;
      }
    }
    dict2 = GetProperty(dict, "Vertical");
    if (dict2 && dict2->type == kTagTypeTrue) {
      GlobalConfig.VerticalLayout = TRUE;
    }
    // GlobalConfig.MainEntriesSize
    dict2 = GetProperty(dict, "MainEntriesSize");
    if (dict2 && dict2->type == kTagTypeInteger) {
      GlobalConfig.MainEntriesSize = (INT32)(UINTN)dict2->string;
    }
    dict2 = GetProperty(dict, "TileXSpace");
    if (dict2 && dict2->type == kTagTypeInteger) {
      GlobalConfig.TileXSpace = (INT32)(UINTN)dict2->string;
    }
    dict2 = GetProperty(dict, "TileYSpace");
    if (dict2 && dict2->type == kTagTypeInteger) {
      GlobalConfig.TileYSpace = (INT32)(UINTN)dict2->string;
    }

  }  

  dict = GetProperty(dictPointer, "Components");
  if (dict) {
    dict2 = GetProperty(dict, "Banner");
    if (dict2 && dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_BANNER;
    }
    dict2 = GetProperty(dict, "Functions");
    if (dict2 && dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_FUNCS;
    }
    dict2 = GetProperty(dict, "Tools");
    if (dict2 && dict2->type == kTagTypeFalse) {
      GlobalConfig.DisableFlags |= HIDEUI_FLAG_TOOLS;
    }
    dict2 = GetProperty(dict, "Label");
    if (dict2 && dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_LABEL;
    }
    dict2 = GetProperty(dict, "Revision");
    if (dict2 && dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_REVISION;
    }
    dict2 = GetProperty(dict, "MenuTitle");
    if (dict2 && dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_MENU_TITLE;
    }
    dict2 = GetProperty(dict, "MenuTitleImage");
    if (dict2 && dict2->type == kTagTypeFalse) {
      GlobalConfig.HideUIFlags |= HIDEUI_FLAG_MENU_TITLE_IMAGE;
    }
  }

  dict = GetProperty(dictPointer, "Selection");
  if (dict) {
    dict2 = GetProperty(dict, "Color");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        GlobalConfig.SelectionColor = (UINTN)dict2->string;
      } else if ((dict2->type == kTagTypeString) && dict2->string) {
        GlobalConfig.SelectionColor = AsciiStrHexToUintn(dict2->string);
      }
    }
    dict2 = GetProperty(dict, "Small");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        GlobalConfig.SelectionSmallFileName = PoolPrint(L"%a", dict2->string);
      }
    }
    dict2 = GetProperty(dict, "Big");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        GlobalConfig.SelectionBigFileName = PoolPrint(L"%a", dict2->string);
      }
    }
    dict2 = GetProperty(dict, "OnTop");
    if (dict2) {
      if ((dict2->type == kTagTypeTrue) ||
          ((dict2->type == kTagTypeString) && dict2->string &&
           ((dict2->string[0] == 'Y') || (dict2->string[0] == 'y')))) {
            GlobalConfig.SelectionOnTop = TRUE;
          }
    }
  }

  dict = GetProperty(dictPointer, "Scroll");
  if (dict) {
    dict2 = GetProperty(dict, "Width");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        ScrollWidth = (UINTN)dict2->string;
      } else if ((dict2->type == kTagTypeString) && dict2->string) {
        ScrollWidth = AsciiStrDecimalToUintn(dict2->string);
      }
    }
    dict2 = GetProperty(dict, "Height");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        ScrollButtonsHeight = (UINTN)dict2->string;
      } else if ((dict2->type == kTagTypeString) && dict2->string) {
        ScrollButtonsHeight = AsciiStrDecimalToUintn(dict2->string);
      }
    }
    dict2 = GetProperty(dict, "BarHeight");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        ScrollBarDecorationsHeight = (UINTN)dict2->string;
      } else if ((dict2->type == kTagTypeString) && dict2->string) {
        ScrollBarDecorationsHeight = AsciiStrDecimalToUintn(dict2->string);
      }
    }
    dict2 = GetProperty(dict, "ScrollHeight");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        ScrollScrollDecorationsHeight = (UINTN)dict2->string;
      } else if ((dict2->type == kTagTypeString) && dict2->string) {
        ScrollScrollDecorationsHeight = AsciiStrDecimalToUintn(dict2->string);
      }
    }
  }

  dict = GetProperty(dictPointer, "Font");
  if (dict) {
    dict2 = GetProperty(dict, "Type");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        if ((dict2->string[0] == 'A') || (dict2->string[0] == 'a')) {
          GlobalConfig.Font = FONT_ALFA;
        } else if ((dict2->string[0] == 'G') || (dict2->string[0] == 'g')) {
          GlobalConfig.Font = FONT_GRAY;
        } else if ((dict2->string[0] == 'L') || (dict2->string[0] == 'l')) {
          GlobalConfig.Font = FONT_LOAD;
        }
      }
    }
    dict2 = GetProperty(dict, "Path");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        GlobalConfig.FontFileName = PoolPrint(L"%a", dict2->string);
      }
    }
    dict2 = GetProperty(dict, "CharWidth");
    if (dict2) {
      if (dict2->type == kTagTypeInteger) {
        GlobalConfig.CharWidth = (UINTN)dict2->string;
      } else if ((dict2->type == kTagTypeString) && dict2->string) {
        GlobalConfig.CharWidth = AsciiStrDecimalToUintn(dict2->string);
      }
    }
  }

  dict = GetProperty(dictPointer, "Anime");
  if (dict) {
    INTN i, Count = GetTagCount(dict);
    for (i = 0; i < Count; ++i) {
      GUI_ANIME *Anime;
      if (EFI_ERROR(GetElement(dict, i, &dictPointer))) {
        continue;
      }
      if (dictPointer == NULL) {
        break;
      }
      Anime = AllocateZeroPool(sizeof(GUI_ANIME));
      if (Anime == NULL) {
        break;
      }
      dict2 = GetProperty(dictPointer, "ID");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          Anime->ID = (UINTN)dict2->string;
        } else if ((dict2->type == kTagTypeString) && dict2->string) {
          Anime->ID = AsciiStrDecimalToUintn(dict2->string);
        }
      }
      dict2 = GetProperty(dictPointer, "Path");
      if (dict2) {
        if ((dict2->type == kTagTypeString) && dict2->string) {
          Anime->Path = PoolPrint(L"%a", dict2->string);
        }
      }
      dict2 = GetProperty(dictPointer, "Frames");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          Anime->Frames = (UINTN)dict2->string;
        } else if ((dict2->type == kTagTypeString) && dict2->string) {
          Anime->Frames = AsciiStrDecimalToUintn(dict2->string);
        }
      }
      dict2 = GetProperty(dictPointer, "FrameTime");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          Anime->FrameTime = (UINTN)dict2->string;
        } else if ((dict2->type == kTagTypeString) && dict2->string) {
          Anime->FrameTime = AsciiStrDecimalToUintn(dict2->string);
        }
      }

      dict2 = GetProperty(dictPointer, "ScreenEdgeX");
      if (dict2) {
        if ((dict2->type == kTagTypeString) && dict2->string) {
          if (AsciiStrCmp(dict2->string, "left") == 0) {
            Anime->ScreenEdgeHorizontal = SCREEN_EDGE_LEFT;
          } else if (AsciiStrCmp(dict2->string, "right") == 0) {
            Anime->ScreenEdgeHorizontal = SCREEN_EDGE_RIGHT;
          }
        }
      }
      dict2 = GetProperty(dictPointer, "ScreenEdgeY");
      if (dict2) {
        if ((dict2->type == kTagTypeString) && dict2->string) {
          if (AsciiStrCmp(dict2->string, "top") == 0) {
            Anime->ScreenEdgeVertical = SCREEN_EDGE_TOP;
          } else if (AsciiStrCmp(dict2->string, "bottom") == 0) {
            Anime->ScreenEdgeVertical = SCREEN_EDGE_BOTTOM;
          }
        }
      }
      
      //default value is centre
      Anime->FilmX = INITVALUE;
      Anime->FilmY = INITVALUE;
      Anime->NudgeX = INITVALUE;
      Anime->NudgeY = INITVALUE;
       
      dict2 = GetProperty(dictPointer, "DistanceFromScreenEdgeX%");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          Anime->FilmX = (INT32)(UINTN)dict2->string;
        }
      }
      dict2 = GetProperty(dictPointer, "DistanceFromScreenEdgeY%");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          Anime->FilmY = (INT32)(UINTN)dict2->string;
        }
      }
      
      dict2 = GetProperty(dictPointer, "NudgeX");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          Anime->NudgeX = (INT32)(UINTN)dict2->string;
        }
      }
      dict2 = GetProperty(dictPointer, "NudgeY");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          Anime->NudgeY = (INT32)(UINTN)dict2->string;
        }
      }
      
      dict2 = GetProperty(dictPointer, "Once");
      if (dict2) {
        if ((dict2->type == kTagTypeTrue) ||
            ((dict2->type == kTagTypeString) && dict2->string &&
             ((dict2->string[0] == 'Y') || (dict2->string[0] == 'y')))) {
              Anime->Once = TRUE;
            }
      }
      // Add the anime to the list
      if (Anime) {
        if ((Anime->ID == 0) || (Anime->Path == NULL)) {
          FreePool(Anime);
        } else if (GuiAnime) {
          if (GuiAnime->ID == Anime->ID) {
            Anime->Next = GuiAnime->Next;
            FreeAnime(GuiAnime);
            GuiAnime = Anime;
          } else {
            GUI_ANIME *Ptr = GuiAnime;
            while (Ptr->Next) {
              if (Ptr->Next->ID == Anime->ID) {
                GUI_ANIME *Next = Ptr->Next;
                Ptr->Next = Next->Next;
                FreeAnime(Next);
                break;
              }
              Ptr = Ptr->Next;
            }
            Anime->Next = GuiAnime;
            GuiAnime = Anime;
          }
        } else {
          GuiAnime = Anime;
        }
      }
    }
  }

  // set file defaults in case they were not set
  if (!GlobalConfig.BackgroundName) {
    GlobalConfig.BackgroundName = PoolPrint(L"background.png");
  }
  if (!GlobalConfig.BannerFileName) {
    GlobalConfig.BannerFileName = PoolPrint(L"logo.png");
  }
  if (!GlobalConfig.SelectionSmallFileName) {
    GlobalConfig.SelectionSmallFileName = PoolPrint(L"selection_small.png");
  }
  if (!GlobalConfig.SelectionBigFileName) {
    GlobalConfig.SelectionBigFileName = PoolPrint(L"selection_big.png");
  }
  
  return EFI_SUCCESS;
}

TagPtr LoadTheme(CHAR16 *testTheme) {
  EFI_STATUS Status = EFI_UNSUPPORTED;
  TagPtr     ThemeDict = NULL;
  CHAR8      *ThemePtr = NULL;
  UINTN      Size = 0;

  if (testTheme) {
    if (ThemePath) {
      FreePool(ThemePath);
    }
    ThemePath = PoolPrint(L"EFI\\CLOVER\\themes\\%s", testTheme);
    if (ThemePath) {
      if (ThemeDir) {
        ThemeDir->Close(ThemeDir);
        ThemeDir = NULL;
      }
      Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath, EFI_FILE_MODE_READ, 0);
      if (!EFI_ERROR(Status)) {
        Status = egLoadFile(ThemeDir, CONFIG_THEME_FILENAME, (UINT8**)&ThemePtr, &Size);
        if (!EFI_ERROR(Status) && (ThemePtr != NULL) && (Size != 0)) {
          Status = ParseXML((const CHAR8*)ThemePtr, &ThemeDict, 0);
          if (EFI_ERROR(Status)) {
            ThemeDict = NULL;
          }
          if (!ThemeDict) {
            DBG("xml file %s not parsed\n", CONFIG_THEME_FILENAME);
          } else {
            DBG("Using theme '%s' (%s)\n", testTheme, ThemePath);
          }
        }
        if (ThemePtr) {
          FreePool(ThemePtr);
        }
      }
    }
  }
  return ThemeDict;
}

EFI_STATUS InitTheme(BOOLEAN useThemeDefinedInNVRam, EFI_TIME *time)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  UINTN   Size = 0;
  UINTN   Index;
  TagPtr  ThemeDict = NULL;
  CHAR8   *chosenTheme = NULL;
  CHAR16  *testTheme = NULL;
  
  // Invalidated BuiltinIcons
  DBG("Invalidating BuiltinIcons...\n");
  for (Index = 0; Index < BUILTIN_ICON_COUNT; Index++) {
    if (BuiltinIconTable[Index].Image) {
      egFreeImage(BuiltinIconTable[Index].Image);
      BuiltinIconTable[Index].Image = NULL;
    }    
  }
  for (Index = 0; Index < 4; Index++) {
    if (SelectionImages[Index]) {
      egFreeImage(SelectionImages[Index]);
      SelectionImages[Index] = NULL;
    }
  } 
  
  KillMouse();
  
  while (GuiAnime) {
    GUI_ANIME *NextAnime=GuiAnime->Next;
    FreeAnime(GuiAnime);
    GuiAnime=NextAnime;
  }
  
  if (ThemesNum > 0 && (!GlobalConfig.Theme || StrCmp(GlobalConfig.Theme,L"embedded") != 0)) {
    
    // Try special theme first
    if (time!=NULL) {
      if ((time->Month == 12) && ((time->Day >= 25) && (time->Day <= 31))) {
         testTheme = PoolPrint(L"christmas");
      } else if ((time->Month == 1) && ((time->Day >= 1) && (time->Day <= 7))) {
         testTheme = PoolPrint(L"newyear");
      }
      if (testTheme) {
        ThemeDict = LoadTheme(testTheme);
        if (ThemeDict) {
          DBG("special theme %s found and %s parsed\n", testTheme, CONFIG_THEME_FILENAME);
          if (GlobalConfig.Theme) {
            FreePool(GlobalConfig.Theme);
          }
          GlobalConfig.Theme = testTheme;
        } else { // special theme not loaded
          DBG("special theme %s not found, skipping\n", testTheme, CONFIG_THEME_FILENAME);
          FreePool(testTheme);
        }
        testTheme = NULL;
      }
    }
    
    // Try theme from nvram  
    if (!ThemeDict && useThemeDefinedInNVRam) {
      chosenTheme = GetNvramVariable(L"Clover.Theme", &gEfiAppleBootGuid, NULL, &Size);
      if (chosenTheme) {
        testTheme = PoolPrint(L"%a", chosenTheme);
        if (testTheme) {
          ThemeDict = LoadTheme(testTheme);
          if (ThemeDict) {
             DBG("theme %a defined in NVRAM found and %s parsed\n", chosenTheme, CONFIG_THEME_FILENAME);
             if (GlobalConfig.Theme) {
               FreePool(GlobalConfig.Theme);
             }
             GlobalConfig.Theme = testTheme;
          } else { // theme from nvram not loaded
            if (GlobalConfig.Theme) {
              DBG("theme %a chosen from nvram is absent, using theme defined in config: %s\n", chosenTheme, GlobalConfig.Theme);
            } else {
              DBG("theme %a chosen from nvram is absent, get first theme\n", chosenTheme);
            }
            FreePool(testTheme);
          }
          testTheme = NULL;
        }
        FreePool(chosenTheme);
        chosenTheme = NULL;
      }
    }
    
    // Try to get theme from settings
    if (!ThemeDict) {
      if (!GlobalConfig.Theme) {
        DBG("no default theme, get first theme %s\n", CONFIG_THEME_FILENAME, ThemesList[0]);
      } else {
        ThemeDict = LoadTheme(GlobalConfig.Theme);
        if (!ThemeDict) {
          DBG("GlobalConfig: %s not found, get first theme %s\n", CONFIG_THEME_FILENAME, ThemesList[0]);
          FreePool(GlobalConfig.Theme);
          GlobalConfig.Theme = NULL; 
        }
      }
    }
  
    // Try to get first theme
    if (!ThemeDict) {
      ThemeDict = LoadTheme(ThemesList[0]);
      if (ThemeDict) {
        GlobalConfig.Theme = AllocateCopyPool(StrSize(ThemesList[0]),ThemesList[0]);
      }
    }

  } // ThemesNum>0
  
  // No theme could be loaded, use embedded
  if (!ThemeDict) {
    DBG("no themes available, using embedded\n");
    GlobalConfig.Theme = NULL;
    if (ThemePath) {
      FreePool(ThemePath);
    }
    ThemePath = NULL;
    if (ThemeDir) {
      ThemeDir->Close(ThemeDir);
    }
    ThemeDir = NULL;
    // set default values
    GetThemeTagSettings(NULL);
    //fill some fields
    GlobalConfig.SelectionColor = 0xA0A0A080;
    GlobalConfig.Font = FONT_ALFA; //to be inverted later
    GlobalConfig.HideBadges |= HDBADGES_SHOW;
    GlobalConfig.BadgeScale = 16;
  } else { // theme loaded successfully
    // read theme settings
    TagPtr dictPointer = GetProperty(ThemeDict, "Theme");
    if (dictPointer) {
      Status = GetThemeTagSettings(dictPointer);
      if (EFI_ERROR(Status)) {
         DBG("Config theme error: %r\n", Status);
      }
    }
    FreeTag(ThemeDict);
  }
  
  PrepareFont();
  
  return Status;
}

VOID ParseSMBIOSSettings(TagPtr dictPointer)
{
  CHAR16 UStr[64];
  TagPtr prop = GetProperty(dictPointer,"ProductName");
  if(prop) {
   MACHINE_TYPES Model;
   AsciiStrCpy(gSettings.ProductName, prop->string);
   // let's fill all other fields based on this ProductName
   // to serve as default
   Model = GetModelFromString(gSettings.ProductName);
   if (Model != MaxMachineType) {
      SetDMISettingsForModel(Model);
   }
  }
  prop = GetProperty(dictPointer,"BiosVendor");
  if(prop) {
    AsciiStrCpy(gSettings.VendorName, prop->string);
  }
  prop = GetProperty(dictPointer,"BiosVersion");
  if(prop) {
    AsciiStrCpy(gSettings.RomVersion, prop->string);
  }
  prop = GetProperty(dictPointer,"BiosReleaseDate");
  if(prop) {
    AsciiStrCpy(gSettings.ReleaseDate, prop->string);
  }
  prop = GetProperty(dictPointer,"Manufacturer");
  if(prop) {
    AsciiStrCpy(gSettings.ManufactureName, prop->string);
  }
  prop = GetProperty(dictPointer,"Version");
  if(prop) {
    AsciiStrCpy(gSettings.VersionNr, prop->string);
  }
  prop = GetProperty(dictPointer,"Family");
  if(prop) {
    AsciiStrCpy(gSettings.FamilyName, prop->string);
  }
  prop = GetProperty(dictPointer,"SerialNumber");
  if(prop) {
    ZeroMem(gSettings.SerialNr, 64);
    AsciiStrCpy(gSettings.SerialNr, prop->string);
  }
  prop = GetProperty(dictPointer,"SmUUID");
  if(prop) {
    if (IsValidGuidAsciiString(prop->string)) {
      AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
      StrToGuidLE((CHAR16*)&UStr[0], &gSettings.SmUUID);
    } else {
      DBG("Error: invalid SmUUID '%a' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", prop->string);
    }
  }
  prop = GetProperty(dictPointer,"BoardManufacturer");
  if(prop) {
    AsciiStrCpy(gSettings.BoardManufactureName, prop->string);
  }
  prop = GetProperty(dictPointer,"BoardSerialNumber");
  if(prop) {
    AsciiStrCpy(gSettings.BoardSerialNumber, prop->string);
  }
  prop = GetProperty(dictPointer,"Board-ID");
  if(prop) {
    AsciiStrCpy(gSettings.BoardNumber, prop->string);
  }
  prop = GetProperty(dictPointer,"BoardVersion");
  if(prop) {
    AsciiStrCpy(gSettings.BoardVersion, prop->string);
  }
  prop = GetProperty(dictPointer,"BoardType");
  if(prop) {
    if (prop->type == kTagTypeInteger) {
      gSettings.BoardType = (UINT8)(UINTN)prop->string;
    } else if (prop->type == kTagTypeString){
      gSettings.BoardType = (UINT8)AsciiStrDecimalToUintn(prop->string);
    }
  }
  prop = GetProperty(dictPointer,"Mobile");
  if(prop) {
    if ((prop->type == kTagTypeFalse) ||
        ((prop->type == kTagTypeString) &&
         ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
      gSettings.Mobile = FALSE;
    else if ((prop->type == kTagTypeTrue) ||
             ((prop->type == kTagTypeString) &&
              ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
      gSettings.Mobile = TRUE;
  }
  prop = GetProperty(dictPointer,"LocationInChassis");
  if(prop) {
    AsciiStrCpy(gSettings.LocationInChassis, prop->string);
  }
  prop = GetProperty(dictPointer,"ChassisManufacturer");
  if(prop) {
    AsciiStrCpy(gSettings.ChassisManufacturer, prop->string);
  }
  prop = GetProperty(dictPointer,"ChassisAssetTag");
  if(prop) {
    AsciiStrCpy(gSettings.ChassisAssetTag, prop->string);
  }
  prop = GetProperty(dictPointer,"ChassisType");
  if(prop) {
    if (prop->type == kTagTypeInteger) {
      gSettings.ChassisType = (UINT8)(UINTN)prop->string;
    } else if (prop->type == kTagTypeString){
      AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
      gSettings.ChassisType = (UINT8)StrHexToUint64((CHAR16*)&UStr[0]);
    }
    DBG("Config set ChassisType=0x%x\n", gSettings.ChassisType);
  }
  //gFwFeatures = 0xC0001403 - by default
  prop = GetProperty(dictPointer, "FirmwareFeatures");
  if(prop) {
    if (prop->type == kTagTypeInteger) {
      gFwFeatures = (UINT32)(UINTN)prop->string;
    } else if (prop->type == kTagTypeString){
      AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
      gFwFeatures = (UINT32)StrHexToUint64((CHAR16*)&UStr[0]);
    }
  }
}

EFI_STATUS GetUserSettings(IN EFI_FILE *RootDir, TagPtr CfgDict)
{
  EFI_STATUS	Status = EFI_NOT_FOUND;
  TagPtr      dict, dict2;
  TagPtr      prop, prop2;
  TagPtr      dictPointer;
  UINTN       i;
  //CHAR8       ANum[4];
  CHAR16      UStr[64];
  
    dict = CfgDict;
  if(dict != NULL) {
    
    DBG("Loading main settings\n");
    
    //*** SYSTEM ***
    
    dictPointer = GetProperty(dict, "SystemParameters");
    if (dictPointer) {
      // Inject kexts
      prop = GetProperty(dictPointer, "InjectKexts");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
        {
          gSettings.WithKexts = TRUE;
        } else if ((prop->type == kTagTypeString) &&
                   ((AsciiStrStr(prop->string, "IfNoFakeSMC") != NULL) ||
                    (AsciiStrStr(prop->string, "Automatic") != NULL) ||
                    (AsciiStrStr(prop->string, "Detect") != NULL))
                   )
        {
          gSettings.WithKexts = TRUE;
          gSettings.WithKextsIfNoFakeSMC = TRUE;
        }
      }

      // No caches
      prop = GetProperty(dictPointer, "NoCaches");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.NoCaches = TRUE;
        }
      }

    }
    

    //Graphics
    
    dictPointer = GetProperty(dict, "Graphics");
    if (dictPointer) {
      dict2 = GetProperty(dictPointer, "Inject");
      if(dict2) {
        if ((dict2->type == kTagTypeTrue) ||
            ((dict2->type == kTagTypeString) &&
             ((dict2->string[0] == 'y') || (dict2->string[0] == 'Y')))) {
          gSettings.GraphicsInjector = TRUE;
          gSettings.InjectIntel = TRUE;
          gSettings.InjectATI = TRUE;
          gSettings.InjectNVidia = TRUE;
        } else if (dict2->type == kTagTypeDict) {
          prop = GetProperty(dict2, "Intel");
          if(prop) {
            if ((prop->type == kTagTypeTrue) ||
                ((prop->type == kTagTypeString) &&
                 ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
              gSettings.InjectIntel = TRUE;
            } else {
              gSettings.InjectIntel = FALSE;
            }
          }
          prop = GetProperty(dict2, "ATI");
          if(prop) {
            if ((prop->type == kTagTypeTrue) ||
                ((prop->type == kTagTypeString) &&
                 ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
              gSettings.InjectATI = TRUE;
            } else {
              gSettings.InjectATI = FALSE;
            }
          }
          prop = GetProperty(dict2, "NVidia");
          if(prop) {
            if ((prop->type == kTagTypeTrue) ||
                ((prop->type == kTagTypeString) &&
                 ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
              gSettings.InjectNVidia = TRUE;
            } else {
              gSettings.InjectNVidia = FALSE;
            }
          }
        } else {
          gSettings.GraphicsInjector = FALSE;
          gSettings.InjectIntel = FALSE;
          gSettings.InjectATI = FALSE;
          gSettings.InjectNVidia = FALSE;
        }
      }

      prop = GetProperty(dictPointer, "VRAM");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.VRAM = LShiftU64((UINTN)prop->string, 20);
        } else if (prop->type == kTagTypeString){
          gSettings.VRAM = LShiftU64(AsciiStrDecimalToUintn(prop->string), 20);  //Mb -> bytes
        }
      }
      //
      gSettings.RefCLK = 0;
      prop = GetProperty(dictPointer, "RefCLK");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.RefCLK = LShiftU64((UINTN)prop->string, 20);
        } else if (prop->type == kTagTypeString){
          gSettings.RefCLK = (UINT32)AsciiStrDecimalToUintn(prop->string);
        }
      }
      
      prop = GetProperty(dictPointer, "LoadVBios");
      gSettings.LoadVBios = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.LoadVBios = TRUE;
      }
      for (i=0; i<NGFX; i++) {
        gGraphics[i].LoadVBios = gSettings.LoadVBios; //default
      }
      prop = GetProperty(dictPointer, "VideoPorts");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.VideoPorts = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.VideoPorts = (UINT16)AsciiStrDecimalToUintn(prop->string);
        }
      }
      prop = GetProperty(dictPointer, "FBName");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, gSettings.FBName);
      }
      prop = GetProperty(dictPointer, "NVCAP");
      if(prop) {     
        hex2bin(prop->string, (UINT8*)&gSettings.NVCAP[0], 20);
        DBG("Read NVCAP:");
        for (i=0; i<20; i++) {
          DBG("%02x", gSettings.NVCAP[i]);
        }
        DBG("\n");
        //thus confirmed this procedure is working
      } 
      prop = GetProperty(dictPointer, "display-cfg");
      if(prop) {      
        hex2bin(prop->string, (UINT8*)&gSettings.Dcfg[0], 8);
      } 
      //
      prop = GetProperty(dictPointer, "DualLink");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.DualLink = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.DualLink = (UINT32)AsciiStrDecimalToUintn(prop->string);
        }
      }
      //InjectEDID - already done in earlysettings

      prop = GetProperty(dictPointer, "ig-platform-id");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.IgPlatform = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.IgPlatform = (UINT32)AsciiStrHexToUint64(prop->string);
        }
      }
    }    
    
    dictPointer = GetProperty(dict, "Devices");
    if (dictPointer) {

      prop = GetProperty(dictPointer, "Inject");
      gSettings.StringInjector = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.StringInjector = TRUE;
      }
      prop = GetProperty(dictPointer, "Properties");
      if(prop) {
        EFI_PHYSICAL_ADDRESS  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
        UINTN strlength = AsciiStrLen(prop->string);
        cDeviceProperties = AllocateZeroPool(strlength + 1);
        AsciiStrCpy(cDeviceProperties, prop->string);
        //-------
        Status = gBS->AllocatePages (
                                     AllocateMaxAddress,
                                     EfiACPIReclaimMemory,
                                     EFI_SIZE_TO_PAGES(strlength) + 1,
                                     &BufferPtr
                                     );
        if (!EFI_ERROR(Status)) {
          cProperties = (UINT8*)(UINTN)BufferPtr;
          cPropSize = (UINT32)(strlength >> 1);
          cPropSize = hex2bin(cDeviceProperties, cProperties, cPropSize);
          DBG("Injected EFIString of length %d\n", cPropSize);
        }
        //---------
      }
      prop = GetProperty(dictPointer, "NoDefaultProperties");
      gSettings.NoDefaultProperties = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.NoDefaultProperties = TRUE;
      }
      
      prop = GetProperty(dictPointer, "AddProperties");
      if(prop) {
        INTN i, Index, Count = GetTagCount(prop);
        Index = 0;  //begin from 0 if second enter
        if (Count > 0) {
          DBG("Add %d properties\n", Count);
          gSettings.AddProperties = AllocateZeroPool(Count * sizeof(DEV_PROPERTY));
          
          for (i = 0; i < Count; i++) {
            UINTN Size = 0;
            if (EFI_ERROR(GetElement(prop, i, &dict2))) {
              DBG("AddProperties continue at %d\n", i);
              continue;
            }
            if (dict2 == NULL) {
              DBG("AddProperties break at %d\n", i);
              break;
            }
            prop2 = GetProperty(dict2, "Device");
            if (prop2 && (prop2->type == kTagTypeString) && prop2->string) {
              if (AsciiStriCmp(prop2->string,        "ATI") == 0) {
                gSettings.AddProperties[Index].Device = DEV_ATI;
              } else if (AsciiStriCmp(prop2->string, "NVidia") == 0) {
                gSettings.AddProperties[Index].Device = DEV_NVIDIA;
              } else if (AsciiStriCmp(prop2->string, "IntelGFX") == 0) {
                gSettings.AddProperties[Index].Device = DEV_INTEL;
              } else if (AsciiStriCmp(prop2->string, "LAN") == 0) {
                gSettings.AddProperties[Index].Device = DEV_LAN;
              } else if (AsciiStriCmp(prop2->string, "WIFI") == 0) {
                gSettings.AddProperties[Index].Device = DEV_WIFI;
              } else if (AsciiStriCmp(prop2->string, "Firewire") == 0) {
                gSettings.AddProperties[Index].Device = DEV_FIREWIRE;
              } else if (AsciiStriCmp(prop2->string, "SATA") == 0) {
                gSettings.AddProperties[Index].Device = DEV_SATA;
              } else if (AsciiStriCmp(prop2->string, "IDE") == 0) {
                gSettings.AddProperties[Index].Device = DEV_IDE;
              } else if (AsciiStriCmp(prop2->string, "HDA") == 0) {
                gSettings.AddProperties[Index].Device = DEV_HDA;
              } else if (AsciiStriCmp(prop2->string, "HDMI") == 0) {
                gSettings.AddProperties[Index].Device = DEV_HDMI;
              } else if (AsciiStriCmp(prop2->string, "LPC") == 0) {
                gSettings.AddProperties[Index].Device = DEV_LPC;
              } else if (AsciiStriCmp(prop2->string, "SmBUS") == 0) {
                gSettings.AddProperties[Index].Device = DEV_SMBUS;
              } else if (AsciiStriCmp(prop2->string, "USB") == 0) {
                gSettings.AddProperties[Index].Device = DEV_USB;
              } else {
                DBG(" add properties to unknown device, ignored\n");
                continue; 
              }              
            }

            prop2 = GetProperty(dict2, "Key");
            if (prop2 && (prop2->type == kTagTypeString) && prop2->string) {
              gSettings.AddProperties[Index].Key = AllocateCopyPool(AsciiStrSize(prop2->string), prop2->string);
            }
            prop2 = GetProperty(dict2, "Value");
            if (prop2 && (prop2->type == kTagTypeString) && prop2->string) {
              //first suppose it is Ascii string
              gSettings.AddProperties[Index].Value = AllocateCopyPool(AsciiStrSize(prop2->string), prop2->string);
              gSettings.AddProperties[Index].ValueLen = AsciiStrLen(prop2->string) + 1;
            } else if (prop2 && (prop2->type == kTagTypeInteger)) {
              gSettings.AddProperties[Index].Value = AllocatePool(4);
              CopyMem(gSettings.AddProperties[Index].Value, &(prop2->string), 4);
              gSettings.AddProperties[Index].ValueLen = 4;
            } else {
              //else  data
              gSettings.AddProperties[Index].Value = GetDataSetting(dict2, "Value", &Size);
              gSettings.AddProperties[Index].ValueLen = Size;
            }
            Index++;
          }
          gSettings.NrAddProperties = Index;
        }
      }
      
      prop = GetProperty(dictPointer, "FakeID");
      if(prop) {
        prop2 = GetProperty(prop, "ATI");
        if (prop2 && (prop2->type == kTagTypeString)) {
          AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
          gSettings.FakeATI  = (UINT32)StrHexToUint64(UStr);
        }
        prop2 = GetProperty(prop, "NVidia");
        if (prop2 && (prop2->type == kTagTypeString)) {
          AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
          gSettings.FakeNVidia  = (UINT32)StrHexToUint64(UStr);
        }
        prop2 = GetProperty(prop, "IntelGFX");
        if (prop2 && (prop2->type == kTagTypeString)) {
          AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
          gSettings.FakeIntel  = (UINT32)StrHexToUint64(UStr);
        }
        prop2 = GetProperty(prop, "LAN");
        if (prop2 && (prop2->type == kTagTypeString)) {
          AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
          gSettings.FakeLAN  = (UINT32)StrHexToUint64(UStr);
        }
        prop2 = GetProperty(prop, "WIFI");
        if (prop2 && (prop2->type == kTagTypeString)) {
          AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
          gSettings.FakeWIFI  = (UINT32)StrHexToUint64(UStr);
        }
        prop2 = GetProperty(prop, "SATA");
        if (prop2 && (prop2->type == kTagTypeString)) {
          AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
          gSettings.FakeSATA  = (UINT32)StrHexToUint64(UStr);
        }
        prop2 = GetProperty(prop, "XHCI");
        if (prop2 && (prop2->type == kTagTypeString)) {
          AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
          gSettings.FakeXHCI  = (UINT32)StrHexToUint64(UStr);
        }
        prop2 = GetProperty(prop, "IMEI");
        if (prop2 && (prop2->type == kTagTypeString)) {
          AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
          gSettings.FakeIMEI  = (UINT32)StrHexToUint64(UStr);
        }
      }
      prop = GetProperty(dictPointer, "UseIntelHDMI");
      if(prop) {
        gSettings.UseIntelHDMI = FALSE;
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
            gSettings.UseIntelHDMI = TRUE;
        }
      }

      prop2 = GetProperty(dictPointer, "Audio");
      if (prop2) {
        // HDA
        prop = GetProperty(prop2, "Inject");
        if(prop) {
          // enabled by default
          // syntax:
          // - HDAInjection=No or 0 - disables injection
          // - HDAInjection=887 - injects layout-id 887 decimal (0x00000377)
          // - HDAInjection=0x377 - injects layout-id 887 decimal (0x00000377)
          // - HDAInjection=Detect - reads codec device id (eg. 0x0887)
          //   converts it to decimal 887 and injects this as layout-id.
          //   if hex device is cannot be converted to decimal, injects legacy value 12 decimal
          // - all other values are equal to HDAInjection=Detect
          if (prop->type == kTagTypeInteger) {
            gSettings.HDALayoutId = (UINTN)prop->string;
            gSettings.HDAInjection = (gSettings.HDALayoutId > 0);
          } else if (prop->type == kTagTypeString){
            if ((prop->string[0] == 'n') || (prop->string[0] == 'N')) {
              // if starts with n or N, then no HDA injection
              gSettings.HDAInjection = FALSE;
            } else if ((prop->string[0] == '0')  &&
                       (prop->string[1] == 'x' || prop->string[1] == 'X')) {
              // assume it's a hex layout id
              gSettings.HDALayoutId = AsciiStrHexToUintn(prop->string);
            } else {
              // assume it's a decimal layout id
              gSettings.HDALayoutId = AsciiStrDecimalToUintn(prop->string);
            }
          }
        }
      }

      prop2 = GetProperty(dictPointer, "USB");
      if (prop2) {
        // USB
        prop = GetProperty(prop2, "Inject");
        if(prop) {
          // enabled by default
          // syntax: USBInjection=Yes/No
          if ((prop->type == kTagTypeFalse) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'n') || (prop->string[0] == 'N')))) {
                gSettings.USBInjection = FALSE;
              }
        }
        prop = GetProperty(prop2, "AddClockID");
        if(prop) {
          // disabled by default
          // syntax: InjectClockID=Yes/No
          if ((prop->type == kTagTypeFalse) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
            gSettings.InjectClockID = FALSE;
          else if ((prop->type == kTagTypeTrue) ||
                   ((prop->type == kTagTypeString) &&
                    ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
            gSettings.InjectClockID = TRUE;
        }
        // enabled by default for CloverEFI or Duet
        // disabled for others
        gSettings.USBFixOwnership = gFirmwareClover || (StrCmp(gST->FirmwareVendor, L"EDK II") == 0);
        prop = GetProperty(prop2, "FixOwnership");
        if(prop) {
          if ((prop->type == kTagTypeFalse) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
            gSettings.USBFixOwnership = FALSE;
          else if ((prop->type == kTagTypeTrue) ||
                   ((prop->type == kTagTypeString) &&
                    ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
            gSettings.USBFixOwnership = TRUE;
            DBG("USB FixOwnership: true\n");
          }
        }
        prop = GetProperty(prop2, "HighCurrent");
        if(prop) {
          // disabled by default
          if ((prop->type == kTagTypeFalse) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
            gSettings.HighCurrent = FALSE;
          else if ((prop->type == kTagTypeTrue) ||
                   ((prop->type == kTagTypeString) &&
                    ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
            gSettings.HighCurrent = TRUE;
        }
      }
    }

    //*** ACPI ***//
    
    dictPointer = GetProperty(dict, "ACPI");
    if (dictPointer) {
      prop = GetProperty(dictPointer, "DropTables");
      if (prop) {
        INTN i, Count = GetTagCount(prop);
        if (Count > 0) {
          DBG("Dropping %d tables\n", Count);
          for (i = 0; i < Count; ++i) {
            UINT32 Signature = 0;
            UINT32 TabLength = 0;
            UINT64 TableId = 0;
            if (EFI_ERROR(GetElement(prop, i, &dict2))) {
              DBG("Drop table continue at %d\n", i);
              continue;
            }
            if (dict2 == NULL) {
              DBG("Drop table break at %d\n", i);
              break;
            }
            DBG("Drop table %d", i);
            // Get the table signatures to drop
            prop2 = GetProperty(dict2, "Signature");
            if (prop2 && (prop2->type == kTagTypeString) && prop2->string) {
              CHAR8  s1 = 0, s2 = 0, s3 = 0, s4 = 0;
              CHAR8 *str = prop2->string;
              DBG(" signature=\"");
              if (str) {
                if (*str) {
                  s1 = *str++;
                  DBG("%c", s1);
                }
                if (*str) {
                  s2 = *str++;
                  DBG("%c", s2);
                }
                if (*str) {
                  s3 = *str++;
                  DBG("%c", s3);
                }
                if (*str) {
                  s4 = *str++;
                  DBG("%c", s4);
                }
              }
              Signature = SIGNATURE_32(s1, s2, s3, s4);
              DBG("\" (%8.8X)", Signature);
            }
            // Get the table ids to drop
            prop2 = GetProperty(dict2, "TableId");
            if (prop2) {
              UINTN  idi = 0;
              CHAR8  id[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
              CHAR8 *str = prop2->string;
              DBG(" table-id=\"");
              if (str) {
                while (*str && (idi < 8)) {
                  DBG("%c", *str);
                  id[idi++] = *str++;
                }
              }
              CopyMem(&TableId, (CHAR8*)&id[0], 8);
              DBG("\" (%16.16lX)", TableId);
            }
            // Get the table len to drop
            prop2 = GetProperty(dict2, "Length");
            if (prop2) {
              if (prop2->type == kTagTypeInteger) {
                TabLength  = (UINT32)(UINTN)prop2->string;
              } else if (prop2->type == kTagTypeString){
                AsciiStrToUnicodeStr(prop2->string, (CHAR16*)&UStr[0]);
                TabLength  = (UINT32)StrHexToUint64(UStr);
              }
              DBG(" length=%d(0x%x)", TabLength);

            }
            DBG("\n");
            //set to drop
            if (gSettings.ACPIDropTables) {
              ACPI_DROP_TABLE *DropTable = gSettings.ACPIDropTables;
              DBG("set table: %08x, %16lx to drop:", Signature, TableId);
              while (DropTable) {
                if (((Signature == DropTable->Signature) &&
                     (!TableId || (DropTable->TableId == TableId)) &&
                     (!TabLength || (DropTable->Length == TabLength))) ||
                     (!Signature && (DropTable->TableId == TableId))) {
                  DropTable->MenuItem.BValue = TRUE;
                  gSettings.DropSSDT = FALSE; //if one item=true then dropAll=false by default
                  DBG("  true\n");
                }                
                DropTable = DropTable->Next;
              }
              DBG("\n");
            }

          }
        }
      }
      
      dict2 = GetProperty(dictPointer, "DSDT");
      if (dict2) {
        //gSettings.DsdtName by default is "DSDT.aml", but name "BIOS" will mean autopatch
        prop = GetProperty(dict2, "Name");
        if(prop) {
          AsciiStrToUnicodeStr(prop->string, gSettings.DsdtName);
        }
        prop = GetProperty(dict2, "Debug");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
            gSettings.DebugDSDT = TRUE;
          }
        }
        prop = GetProperty(dict2, "Rtc8Allowed");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.Rtc8Allowed = TRUE;
              }
        }
        
        prop = GetProperty(dict2, "FixMask");
        if(prop) {
          if (prop->type == kTagTypeInteger) {
            gSettings.FixDsdt  = (UINT32)(UINTN)prop->string;
          } else if (prop->type == kTagTypeString){
            AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
            gSettings.FixDsdt  = (UINT32)StrHexToUint64(UStr); 
          }
          DBG("Config set Fix DSDT mask=%08x\n", gSettings.FixDsdt);
        }
        prop = GetProperty(dict2, "Fixes");
        if (prop) {
          DBG("Config set Fixes will override FixMask mask!\n");
          if (prop->type == kTagTypeDict) {
            gSettings.FixDsdt = 0;
            prop2 = GetProperty(prop, "AddDTGP_0001");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_DTGP;
                  }
            }
            prop2 = GetProperty(prop, "FixDarwin_0002");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_WARNING;
                  }
            }
            prop2 = GetProperty(prop, "FixShutdown_0004");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_SHUTDOWN;
                  }
            }
            prop2 = GetProperty(prop, "AddMCHC_0008");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_MCHC;
                  }
            }
            prop2 = GetProperty(prop, "FixHPET_0010");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_HPET;
                  }
            }
            prop2 = GetProperty(prop, "FakeLPC_0020");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_LPC;
                  }
            }
            prop2 = GetProperty(prop, "FixIPIC_0040");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_IPIC;
                  }
            }
            prop2 = GetProperty(prop, "FixSBUS_0080");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_SBUS;
                  }
            }
            prop2 = GetProperty(prop, "FixDisplay_0100");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_DISPLAY;
                  }
            }
            prop2 = GetProperty(prop, "FixIDE_0200");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_IDE;
                  }
            }
            prop2 = GetProperty(prop, "FixSATA_0400");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_SATA;
                  }
            }
            prop2 = GetProperty(prop, "FixFirewire_0800");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_FIREWIRE;
                  }
            }
            prop2 = GetProperty(prop, "FixUSB_1000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_USB;
                  }
            }
            prop2 = GetProperty(prop, "FixLAN_2000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_LAN;
                  }
            }
            prop2 = GetProperty(prop, "FixAirport_4000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_WIFI;
                  }
            }
            prop2 = GetProperty(prop, "FixHDA_8000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_HDA;
                  }
            }
            prop2 = GetProperty(prop, "FIX_DARWIN_10000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_WARNING;
                  }
            }
            prop2 = GetProperty(prop, "FIX_RTC_20000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_RTC;
                  }
            }
            prop2 = GetProperty(prop, "FIX_TMR_40000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_TMR;
                  }
            }
            prop2 = GetProperty(prop, "AddIMEI_80000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_IMEI;
                  }
            }
            prop2 = GetProperty(prop, "FIX_INTELGFX_100000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_INTELGFX;
                  }
            }
            prop2 = GetProperty(prop, "FIX_WAK_200000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_WAK;
                  }
            }
            prop2 = GetProperty(prop, "DeleteUnused_400000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_UNUSED;
                  }
            }
            prop2 = GetProperty(prop, "FIX_ADP1_800000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_ADP1;
                  }
            }
            prop2 = GetProperty(prop, "AddPNLF_1000000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_PNLF;
                  }
            }
            prop2 = GetProperty(prop, "FIX_S3D_2000000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_S3D;
                  }
            }
            prop2 = GetProperty(prop, "FIX_ACST_4000000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_ACST;
                  }
            }
            prop2 = GetProperty(prop, "AddHDMI_8000000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_HDMI;
                  }
            }
            prop2 = GetProperty(prop, "FixRegions_10000000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_REGIONS;
                  }
            }
            prop2 = GetProperty(prop, "NewWay_80000000");
            if(prop2) {
              if ((prop2->type == kTagTypeTrue) ||
                  ((prop2->type == kTagTypeString) &&
                   ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                    gSettings.FixDsdt |= FIX_NEW_WAY;
                  }
            }

          }
          DBG("   final mask=%08x\n", gSettings.FixDsdt);
        }

        prop = GetProperty(dict2,"Patches");
        if(prop) {
          UINTN Count = GetTagCount(prop);
          if (Count > 0) {
            gSettings.PatchDsdtNum = (UINT32)Count;
            gSettings.PatchDsdtFind = AllocateZeroPool(Count * sizeof(UINT8*));
            gSettings.PatchDsdtReplace = AllocateZeroPool(Count * sizeof(UINT8*));
            gSettings.LenToFind = AllocateZeroPool(Count * sizeof(UINT32));
            gSettings.LenToReplace = AllocateZeroPool(Count * sizeof(UINT32));
            DBG("PatchesDSDT: %d requested\n", Count);
            for (i = 0; i < Count; ++i) {
              UINTN Size = 0;
              Status = GetElement(prop, i, &prop2);
              if (EFI_ERROR(Status)) {
                DBG("error %r getting next element of PatchesDSDT at index %d\n", Status, i);
                continue;
              }
              if (!prop2) {
                break;
              }
              DBG(" DSDT bin patch #%d ", i);
              gSettings.PatchDsdtFind[i] = GetDataSetting(prop2,"Find", &Size);
              DBG(" lenToFind=%d ", Size);
              gSettings.LenToFind[i] = (UINT32)Size;
              gSettings.PatchDsdtReplace[i] = GetDataSetting(prop2,"Replace", &Size);
              DBG(" lenToReplace=%d\n", Size);
              gSettings.LenToReplace[i] = (UINT32)Size;
            }
          } //if count > 0
        } //if prop PatchesDSDT
        prop = GetProperty(dict2, "ReuseFFFF");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.ReuseFFFF = TRUE;
              }
        }
        prop = GetProperty(dict2, "SuspendOverride");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
            gSettings.SuspendOverride = TRUE;
          }
        }
/*
        prop = GetProperty(dict2, "SlpSmiAtWake");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.SlpWak = TRUE;
              }
        }
*/
        prop = GetProperty(dict2, "DropOEM_DSM"); 
        defDSM = FALSE;
        if(prop) {
          defDSM = TRUE;
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.DropOEM_DSM = 0xFFFF;          
              } else if ((prop->type == kTagTypeFalse) ||
                         ((prop->type == kTagTypeString) &&
                          ((prop->string[0] == 'n') || (prop->string[0] == 'N')))) {
                 gSettings.DropOEM_DSM = 0;
               } else if (prop->type == kTagTypeInteger) {
                 gSettings.DropOEM_DSM = (UINT16)(UINTN)prop->string;
               } else if (prop->type == kTagTypeDict) {
                 prop2 = GetProperty(prop, "ATI");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_ATI;
                       }
                 }
                 prop2 = GetProperty(prop, "NVidia");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_NVIDIA;
                       }
                 }
                 prop2 = GetProperty(prop, "IntelGFX");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_INTEL;
                       }
                 }
                 prop2 = GetProperty(prop, "HDA");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_HDA;
                       }
                 }
                 prop2 = GetProperty(prop, "HDMI");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_HDMI;
                       }
                 }
                 prop2 = GetProperty(prop, "SATA");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_SATA;
                       }
                 }
                 prop2 = GetProperty(prop, "LAN");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_LAN;
                       }
                 }
                 prop2 = GetProperty(prop, "WIFI");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_WIFI;
                       }
                 }
                 prop2 = GetProperty(prop, "USB");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_USB;
                       }
                 }          
                 prop2 = GetProperty(prop, "LPC");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_LPC;
                       }
                 }          
                 prop2 = GetProperty(prop, "SmBUS");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_SMBUS;
                       }
                 }          
                 prop2 = GetProperty(prop, "Firewire");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_FIREWIRE;
                       }
                 }          
                 prop2 = GetProperty(prop, "IDE");
                 if(prop2) {
                   if ((prop2->type == kTagTypeTrue) ||
                       ((prop2->type == kTagTypeString) &&
                        ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
                         gSettings.DropOEM_DSM |= DEV_IDE;
                       }
                 }          
               }
        }
      }

      dict2 = GetProperty(dictPointer, "SSDT");
      if (dict2) {
        prop2 = GetProperty(dict2, "Generate");
        if (prop2) {
          if ((prop2->type == kTagTypeTrue) ||
              ((prop2->type == kTagTypeString) &&
               ((prop2->string[0] == 'y') || (prop2->string[0] == 'Y')))) {
            gSettings.GeneratePStates = TRUE;
            gSettings.GenerateCStates = TRUE;
          } else if ((prop2->type == kTagTypeFalse) ||
                     ((prop2->type == kTagTypeString) &&
                     ((prop2->string[0] == 'n') || (prop2->string[0] == 'N')))) {
            gSettings.GeneratePStates = FALSE;
            gSettings.GenerateCStates = FALSE;
         } else if (prop2->type == kTagTypeDict) {
            prop = GetProperty(prop2, "PStates");
            if(prop) {
              if ((prop->type == kTagTypeTrue) ||
                  ((prop->type == kTagTypeString) &&
                   ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.GeneratePStates = TRUE;
              } else {
                gSettings.GeneratePStates = FALSE;
              }
            }
            prop = GetProperty(prop2, "CStates");
            if(prop) {
              if ((prop->type == kTagTypeTrue) ||
                  ((prop->type == kTagTypeString) &&
                   ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.GenerateCStates = TRUE;
              } else {
                gSettings.GenerateCStates = FALSE;
              }
            }
          }
        }
        prop = GetProperty(dict2, "DropOem");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
            gSettings.DropSSDT = TRUE;
          } else {
            gSettings.DropSSDT = FALSE;
          }
        }
        prop = GetProperty(dict2, "UseSystemIO");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.EnableISS = TRUE;
              } else {
                gSettings.EnableISS = FALSE;
              }
        }
        prop = GetProperty(dict2, "EnableC7");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.EnableC7 = TRUE;
              } else {
                gSettings.EnableC7 = FALSE;
              }
          DBG("Config set EnableC7: %a\n", gSettings.EnableC7?"+":"-");
        }
        prop = GetProperty(dict2, "EnableC6");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.EnableC6 = TRUE;
              } else {
                gSettings.EnableC6 = FALSE;
              }
          DBG("Config set EnableC6: %a\n", gSettings.EnableC6?"+":"-");
        }
        prop = GetProperty(dict2, "EnableC4");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.EnableC4 = TRUE;
              } else {
                gSettings.EnableC4 = FALSE;
              }
          DBG("Config set EnableC4: %a\n", gSettings.EnableC4?"+":"-");
        }
        prop = GetProperty(dict2, "EnableC2");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                gSettings.EnableC2 = TRUE;
              } else {
                gSettings.EnableC2 = FALSE;
              }
          DBG("Config set EnableC2: %a\n", gSettings.EnableC2?"+":"-");
        }
        prop = GetProperty(dict2, "C3Latency");
        if(prop) {
          if (prop->type == kTagTypeInteger) {
            gSettings.C3Latency = (UINT16)(UINTN)prop->string;
          } else if (prop->type == kTagTypeString){
    //        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
            gSettings.C3Latency = (UINT16)AsciiStrHexToUint64(prop->string);
          }
        }

        prop = GetProperty(dict2, "PLimitDict");
        if(prop) {
          if (prop->type == kTagTypeInteger) {
            gSettings.PLimitDict = (UINT8)(UINTN)prop->string;
          } else if (prop->type == kTagTypeString){
            gSettings.PLimitDict = (UINT8)AsciiStrDecimalToUintn(prop->string);
          }
        }
        prop = GetProperty(dict2, "UnderVoltStep");
        if(prop) {
          if (prop->type == kTagTypeInteger) {
            gSettings.UnderVoltStep = (UINT8)(UINTN)prop->string;
          } else if (prop->type == kTagTypeString){
            gSettings.UnderVoltStep = (UINT8)AsciiStrDecimalToUintn(prop->string);
          }
        }
        prop = GetProperty(dict2, "DoubleFirstState");
        if(prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
            gSettings.DoubleFirstState = TRUE;
          } else if ((prop->type == kTagTypeFalse) ||
                     ((prop->type == kTagTypeString) &&
                      ((prop->string[0] == 'n') || (prop->string[0] == 'N')))) {
            gSettings.DoubleFirstState = FALSE;
          }
        }
        prop = GetProperty(dict2,"MinMultiplier");
        if(prop) {
          if (prop->type == kTagTypeInteger) {
            gSettings.MinMultiplier = (UINT8)(UINTN)prop->string;
          } else if (prop->type == kTagTypeString){
            gSettings.MinMultiplier = (UINT8)AsciiStrDecimalToUintn(prop->string);
          }
          DBG("Config set MinMultiplier=%d\n", gSettings.MinMultiplier);
        }
        prop = GetProperty(dict2, "MaxMultiplier");
        if(prop) {
          if (prop->type == kTagTypeInteger) {
            gSettings.MaxMultiplier = (UINT8)(UINTN)prop->string;
          } else if (prop->type == kTagTypeString){
            gSettings.MaxMultiplier = (UINT8)AsciiStrDecimalToUintn(prop->string);
          }
          DBG("Config set MaxMultiplier=%d\n", gSettings.MaxMultiplier);
        }
        prop = GetProperty(dict2, "PluginType");
        if(prop) {
          if (prop->type == kTagTypeInteger) {
            gSettings.PluginType = (UINT8)(UINTN)prop->string;
          } else if (prop->type == kTagTypeString){
            gSettings.PluginType = (UINT8)AsciiStrDecimalToUintn(prop->string);
          }
          DBG("Config set PluginType=%d\n", gSettings.PluginType);
        }
      }
      prop = GetProperty(dictPointer, "DropMCFG");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
              gSettings.DropMCFG = TRUE;
            } else {
              gSettings.DropMCFG = FALSE;
            }
      }

      prop = GetProperty(dictPointer, "ResetAddress");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.ResetAddr = (UINT64)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
 //         AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.ResetAddr  = AsciiStrHexToUint64(prop->string);
        }
        DBG("Config set ResetAddr=0x%x\n", gSettings.ResetAddr);
        if (gSettings.ResetAddr  == 0x64) {
          gSettings.ResetVal = 0xFE;
        } else if  (gSettings.ResetAddr  == 0xCF9) {
          gSettings.ResetVal = 0x06;
        }
        DBG("Config calc ResetVal=0x%x\n", gSettings.ResetVal);
      }
      prop = GetProperty(dictPointer, "ResetValue");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.ResetVal = (UINT8)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
 //         AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.ResetVal = (UINT8)AsciiStrHexToUint64(prop->string);
        }
        DBG("Config set ResetVal=0x%x\n", gSettings.ResetVal);
      }
      //other known pair is 0x0CF9/0x06. What about 0x92/0x01 ?


      prop = GetProperty(dictPointer, "HaltEnabler");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.SlpSmiEnable = TRUE;
        }
      }
      
      prop = GetProperty(dictPointer, "smartUPS");
      gSettings.smartUPS = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.smartUPS = TRUE;
          DBG("Config set smartUPS present\n");
        }
      }
      prop = GetProperty(dictPointer, "PatchAPIC");
 //     gSettings.PatchNMI = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.PatchNMI = TRUE;
      }
    }
    
    //*** SMBIOS ***//
    dictPointer = GetProperty(dict,"SMBIOS");
    if (dictPointer) {
      ParseSMBIOSSettings(dictPointer);
      prop = GetProperty(dictPointer, "Trust");
      if (prop) {
        if ((prop->type == kTagTypeFalse) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
            gSettings.TrustSMBIOS = FALSE;
        else if ((prop->type == kTagTypeTrue) ||
                 ((prop->type == kTagTypeString) &&
                  ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
                   gSettings.TrustSMBIOS = TRUE;
                 }
      }
      // Inject memory tables into SMBIOS
      prop = GetProperty(dictPointer, "Memory");
      if (prop){
        // Get memory table count
        TagPtr prop2 = GetProperty(prop, "SlotCount");
        gRAM.UserInUse = MAX_RAM_SLOTS;
        if (prop2) {
          if (prop2->type == kTagTypeString) {
            gRAM.UserInUse = (UINT8)AsciiStrDecimalToUintn(prop2->string);
          } else if (prop2->type == kTagTypeInteger) {
            gRAM.UserInUse = (UINT8)(UINTN)prop2->string;
          }
        }
        // Get memory channels
        prop2 = GetProperty(prop, "Channels");
        if (prop2) {
          if (prop2->type == kTagTypeString) {
            gRAM.UserChannels = (UINT8)AsciiStrDecimalToUintn(prop2->string);
          } else if (prop2->type == kTagTypeInteger) {
            gRAM.UserChannels = (UINT8)(UINTN)prop2->string;
          }
        }
        // Get memory tables
        prop2 = GetProperty(prop, "Modules");
        if (prop2) {
          INTN   i, Count = GetTagCount(prop2);
          TagPtr prop3 = NULL;
          for (i = 0; i < Count; ++i) {
            UINTN Slot = MAX_RAM_SLOTS;
            if (EFI_ERROR(GetElement(prop2, i, &prop3))) {
              continue;
            }
            if (prop3 == NULL) {
              break;
            }
            // Get memory slot
            dict2 = GetProperty(prop3, "Slot");
            if (dict2 == NULL) {
              continue;
            }
            if (dict2->type == kTagTypeString) {
              Slot = AsciiStrDecimalToUintn(dict2->string);
            } else if (dict2->type == kTagTypeInteger) {
              Slot = (UINTN)dict2->string;
            } else {
              continue;
            }
            if (Slot >= MAX_RAM_SLOTS) {
              continue;
            }
            // Get memory size
            dict2 = GetProperty(prop3, "Size");
            if (dict2) {
              if (dict2->type == kTagTypeString) {
                gRAM.User[Slot].ModuleSize = (UINT32)AsciiStrDecimalToUintn(dict2->string);
              } else if (dict2->type == kTagTypeInteger) {
                gRAM.User[Slot].ModuleSize = (UINT32)(UINTN)dict2->string;
              }
            }
            // Get memory frequency
            dict2 = GetProperty(prop3, "Frequency");
            if (dict2) {
               if (dict2->type == kTagTypeString) {
                  gRAM.User[Slot].Frequency = (UINT32)AsciiStrDecimalToUintn(dict2->string);
               } else if (dict2->type == kTagTypeInteger) {
                  gRAM.User[Slot].Frequency = (UINT32)(UINTN)dict2->string;
               }
            }
            // Get memory vendor
            dict2 = GetProperty(prop3, "Vendor");
            if (dict2) {
              if (dict2->type == kTagTypeString) {
                gRAM.User[Slot].Vendor = dict2->string;
              }
            }
            // Get memory part number
            dict2 = GetProperty(prop3, "Part");
            if (dict2) {
              if (dict2->type == kTagTypeString) {
                gRAM.User[Slot].PartNo = dict2->string;
              }
            }
            // Get memory serial number
            dict2 = GetProperty(prop3, "Serial");
            if (dict2) {
              if (dict2->type == kTagTypeString) {
                gRAM.User[Slot].SerialNo = dict2->string;
              }
            }
            // Get memory type
            gRAM.User[Slot].Type = MemoryTypeDdr3;
            dict2 = GetProperty(prop3, "Type");
            if (dict2) {
              if (dict2->type == kTagTypeString) {
                if (AsciiStriCmp(dict2->string, "DDR2") == 0) {
                  gRAM.User[Slot].Type = MemoryTypeDdr2;
                } else if (AsciiStriCmp(dict2->string, "DDR") == 0) {
                  gRAM.User[Slot].Type = MemoryTypeDdr;
                }
              }
            }
            gRAM.User[Slot].InUse = (gRAM.User[Slot].ModuleSize > 0);
          }
        }
        gSettings.InjectMemoryTables = TRUE;
      }
      prop = GetProperty(dictPointer, "Slots");
      if (prop) {
        INTN   Index, DeviceN, Count = GetTagCount(prop);
        TagPtr prop3 = NULL;
        for (Index = 0; Index < Count; ++Index) {
          if (EFI_ERROR(GetElement(prop, Index, &prop3))) {
            continue;
          }
          if (prop3 == NULL) {
            break;
          }
          
          prop2 = GetProperty(prop3, "Device");
          if (prop2 && (prop2->type == kTagTypeString) && prop2->string) {
            if (AsciiStriCmp(prop2->string,        "ATI") == 0) {
              DeviceN = 0;
            } else if (AsciiStriCmp(prop2->string, "NVidia") == 0) {
              DeviceN = 1;
            } else if (AsciiStriCmp(prop2->string, "IntelGFX") == 0) {
              DeviceN = 2;
            } else if (AsciiStriCmp(prop2->string, "LAN") == 0) {
              DeviceN = 5;
            } else if (AsciiStriCmp(prop2->string, "WIFI") == 0) {
              DeviceN = 6;
            } else if (AsciiStriCmp(prop2->string, "Firewire") == 0) {
              DeviceN = 12;
            } else if (AsciiStriCmp(prop2->string, "HDMI") == 0) {
              DeviceN = 4;
            } else if (AsciiStriCmp(prop2->string, "USB") == 0) {
              DeviceN = 11;
            } else {
              DBG(" add properties to unknown device %a, ignored\n", prop2->string);
              continue;
            }
          } else {
            DBG(" no device  property for slot\n");
            continue;
          }

          SlotDevices[DeviceN].SlotID = DeviceN;
          prop2 = GetProperty(prop3, "ID");
          if (prop2) {
            if (prop2->type == kTagTypeInteger) {
              SlotDevices[DeviceN].SlotID = (UINT8)((UINTN)prop2->string & 0xFF);
            } else if (prop2->type == kTagTypeString){
              SlotDevices[DeviceN].SlotID = (UINT8)AsciiStrDecimalToUintn(prop2->string);
            }            
          }

          SlotDevices[DeviceN].SlotType = SlotTypePci;
          prop2 = GetProperty(prop3, "Type");
          if (prop2) {
            UINT8 Code = 0;
            if (prop2->type == kTagTypeInteger) {
              Code = (UINT8)((UINTN)prop2->string & 0xFF);
            } else if (prop2->type == kTagTypeString){
              Code = (UINT8)AsciiStrDecimalToUintn(prop2->string);
            }
            switch (Code) {
              case 0:
                SlotDevices[DeviceN].SlotType = SlotTypePci;
                break;
              case 1:
                SlotDevices[DeviceN].SlotType = SlotTypePciExpressX1;
                break;
              case 2:
                SlotDevices[DeviceN].SlotType = SlotTypePciExpressX2;
                break;
              case 4:
                SlotDevices[DeviceN].SlotType = SlotTypePciExpressX4;
                break;
              case 8:
                SlotDevices[DeviceN].SlotType = SlotTypePciExpressX8;
                break;
              case 16:
                SlotDevices[DeviceN].SlotType = SlotTypePciExpressX16;
                break;
                
              default:
                SlotDevices[DeviceN].SlotType = SlotTypePciExpress;
                break;
            }
          }
          prop2 = GetProperty(prop3, "Name");
          if (prop2 && (prop2->type == kTagTypeString) && prop2->string) {
            AsciiSPrint(SlotDevices[DeviceN].SlotName, 31, "%a", prop2->string);
          } else {
            AsciiSPrint(SlotDevices[DeviceN].SlotName, 31, "PCI Slot %d", DeviceN);
          }
        }
      }
    }
    
    //CPU
    dictPointer = GetProperty(dict,"CPU");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"QPI");
      gSettings.QPI = (UINT16)gCPUStructure.ProcessorInterconnectSpeed; //MHz
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.QPI = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.QPI = (UINT16)AsciiStrDecimalToUintn(prop->string);
        }
        if (!gSettings.QPI) {
          gSettings.QPI = 0xFFFF;
          DBG("Config set QPI=0 disable table132\n");
        } else {
          DBG("Config set QPI=%dMHz\n", gSettings.QPI);
        }
      }
      prop = GetProperty(dictPointer,"FrequencyMHz");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.CpuFreqMHz = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.CpuFreqMHz = (UINT32)AsciiStrDecimalToUintn(prop->string);
        }
        DBG("Config set CpuFreq=%dMHz\n", gSettings.CpuFreqMHz);
      }
      prop = GetProperty(dictPointer,"Type");
      gSettings.CpuType = GetAdvancedCpuType();
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.CpuType = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.CpuType = (UINT16)AsciiStrDecimalToUintn(prop->string);
        }
        DBG("Config set CpuType=%x\n", gSettings.CpuType);
      }
      
      prop = GetProperty(dictPointer,"BusSpeedkHz");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.BusSpeed = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.BusSpeed = (UINT32)AsciiStrDecimalToUintn(prop->string);
        }
        DBG("Config set BusSpeed=%dkHz\n", gSettings.BusSpeed);
      }

      prop = GetProperty(dictPointer, "C6");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.EnableC6 = TRUE;
        } else {
          gSettings.EnableC6 = FALSE;
        }
      }
      
      prop = GetProperty(dictPointer, "C4");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.EnableC4 = TRUE;
        } else {
          gSettings.EnableC4 = FALSE;
        }
      }
      
      prop = GetProperty(dictPointer, "C2");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.EnableC2 = TRUE;
          DBG(" C2 enabled\n");
        } else {
          gSettings.EnableC2 = FALSE;
        }
      }
      //Usually it is 0x03e9, but if you want Turbo, you may set 0x00FA
      prop = GetProperty(dictPointer, "Latency");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.C3Latency = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.C3Latency = (UINT16)AsciiStrDecimalToUintn(prop->string);
        }
      }
    }

    // KernelAndKextPatches
//xxx    gSettings.KPKernelCpu = TRUE; // enabled by default
    gSettings.KPKextPatchesNeeded = FALSE;
    gSettings.KPLapicPanic = FALSE; // disabled by default
    if (NeedPMfix) {
      gSettings.KPKernelPm = TRUE;
      gSettings.KPAsusAICPUPM = TRUE;
    }
    dictPointer = GetProperty(dict,"KernelAndKextPatches");
    if (dictPointer) {
      gSettings.KPDebug = FALSE;
      prop = GetProperty(dictPointer,"Debug");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))){
          gSettings.KPDebug = TRUE;
        }
      }
      prop = GetProperty(dictPointer,"KernelCpu");
      if(prop) {
        gSettings.KPKernelCpu = FALSE; // disabled here because user set false and enabled by default
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))){
          gSettings.KPKernelCpu = TRUE;
        }
      }
      prop = GetProperty(dictPointer,"KernelPm");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))){
          gSettings.KPKernelPm = TRUE;
        }
      }
      prop = GetProperty(dictPointer,"KernelLapic");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))){
              gSettings.KPLapicPanic = TRUE;
            }
      }
      prop = GetProperty(dictPointer,"ATIConnectorsController");
      if(prop) {
        UINTN len = 0;
        // ATIConnectors patch
        gSettings.KPATIConnectorsController = AllocateZeroPool(AsciiStrSize(prop->string) * sizeof(CHAR16));
        AsciiStrToUnicodeStr(prop->string, gSettings.KPATIConnectorsController);
        
        gSettings.KPATIConnectorsData = GetDataSetting(dictPointer, "ATIConnectorsData", &len);
        gSettings.KPATIConnectorsDataLen = len;
        gSettings.KPATIConnectorsPatch = GetDataSetting(dictPointer, "ATIConnectorsPatch", &i);
        
        if (gSettings.KPATIConnectorsData == NULL
            || gSettings.KPATIConnectorsPatch == NULL
            || gSettings.KPATIConnectorsDataLen == 0
            || gSettings.KPATIConnectorsDataLen != i) {
          // invalid params - no patching
          DBG("ATIConnectors patch: invalid parameters!\n");
          if (gSettings.KPATIConnectorsController != NULL) FreePool(gSettings.KPATIConnectorsController);
          if (gSettings.KPATIConnectorsData != NULL) FreePool(gSettings.KPATIConnectorsData);
          if (gSettings.KPATIConnectorsPatch != NULL) FreePool(gSettings.KPATIConnectorsPatch);
          gSettings.KPATIConnectorsController = NULL;
          gSettings.KPATIConnectorsData = NULL;
          gSettings.KPATIConnectorsPatch = NULL;
          gSettings.KPATIConnectorsDataLen = 0;
        } else {
          // ok
          gSettings.KPKextPatchesNeeded = TRUE;
        }
      }
      prop = GetProperty(dictPointer,"AsusAICPUPM");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))){
          gSettings.KPAsusAICPUPM = TRUE;
        }
      }
      gSettings.KPKextPatchesNeeded |= gSettings.KPAsusAICPUPM;
      
      prop = GetProperty(dictPointer,"AppleRTC");
      gSettings.KPAppleRTC = TRUE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.KPAppleRTC = TRUE;
        } else if ((prop->type == kTagTypeFalse) ||
                   ((prop->type == kTagTypeString) &&
                    ((prop->string[0] == 'n') || (prop->string[0] == 'N')))){
          gSettings.KPAppleRTC = FALSE;
        }
      }
      gSettings.KPKextPatchesNeeded |= gSettings.KPAppleRTC;
      
      prop = GetProperty(dictPointer,"KextsToPatch");
      if(prop) {
        UINTN Count = GetTagCount(prop);
        if (Count > 0) {
          UINTN j = 0;
          gSettings.NrKexts = 0;
          gSettings.KextPatches = AllocateZeroPool(Count * sizeof(KEXT_PATCH));
          DBG("KextsToPatch: %d requested\n", Count);
          for (i = 0; i < Count; ++i) {
            Status = GetElement(prop, i, &dictPointer);
            if (EFI_ERROR(Status)) {
            DBG("error %r getting next element at index %d\n", Status, i);
               continue;
            }
            if (!dictPointer) {
              break;
            }
            gSettings.KextPatches[gSettings.NrKexts].Name = NULL;
            gSettings.KextPatches[gSettings.NrKexts].Data = NULL;
            gSettings.KextPatches[gSettings.NrKexts].Patch = NULL;
            DBG("KextToPatch %d:", i);
            dict2 = GetProperty(dictPointer,"Name");
            if (!dict2) {
               continue;
            }
            gSettings.KextPatches[gSettings.NrKexts].Name = AllocateCopyPool(AsciiStrSize(dict2->string), dict2->string);
            dict2 = GetProperty(dictPointer,"Comment");
            if (dict2) {
              DBG(" %a (%a)", gSettings.KextPatches[gSettings.NrKexts].Name, dict2->string);
            } else {
              DBG(" %a", gSettings.KextPatches[gSettings.NrKexts].Name);
            }
            gSettings.KPKextPatchesNeeded = TRUE;
        
            // check if this is Info.plist patch or kext binary patch
            gSettings.KextPatches[gSettings.NrKexts].IsPlistPatch = FALSE;
            dict2 = GetProperty(dictPointer, "InfoPlistPatch");
            if(dict2) {
              if ((dict2->type == kTagTypeTrue) ||
                  (dict2->string[0] == 'y') ||
                  (dict2->string[0] == 'Y'))
                gSettings.KextPatches[gSettings.NrKexts].IsPlistPatch = TRUE;
            }
            
            if (gSettings.KextPatches[gSettings.NrKexts].IsPlistPatch) {
              // Info.plist
              // Find and Replace should be in <string>...</string>
              DBG(" Info.plist patch");
              dict2 = GetProperty(dictPointer, "Find");
              gSettings.KextPatches[gSettings.NrKexts].DataLen = 0;
              if(dict2 && dict2->string) {
                gSettings.KextPatches[gSettings.NrKexts].DataLen = AsciiStrLen(dict2->string);
                gSettings.KextPatches[gSettings.NrKexts].Data = (UINT8*) AllocateCopyPool(gSettings.KextPatches[gSettings.NrKexts].DataLen + 1, dict2->string);
              }
              dict2 = GetProperty(dictPointer, "Replace");
              j = 0;
              if(dict2 && dict2->string) {
                j = AsciiStrLen(dict2->string);
                gSettings.KextPatches[gSettings.NrKexts].Patch = (UINT8*) AllocateCopyPool(j + 1, dict2->string);
              }
            } else {
              // kext binary patch
              // Find and Replace should be in <data>...</data> or <string>...</string>
              DBG(" Kext bin patch");
              gSettings.KextPatches[gSettings.NrKexts].Data = GetDataSetting(dictPointer,"Find", &j);
              gSettings.KextPatches[gSettings.NrKexts].DataLen = j;
              gSettings.KextPatches[gSettings.NrKexts].Patch = GetDataSetting(dictPointer,"Replace", &j);
            }
        
            if ((gSettings.KextPatches[gSettings.NrKexts].DataLen != (INTN)j) || (j == 0)) {
              DBG(" - invalid Find/Replace data - skipping!\n");
              if (gSettings.KextPatches[gSettings.NrKexts].Name != NULL) {
                FreePool(gSettings.KextPatches[gSettings.NrKexts].Name); //just erase name
                gSettings.KextPatches[gSettings.NrKexts].Name = NULL;
              }
              if (gSettings.KextPatches[gSettings.NrKexts].Data != NULL) {
                 FreePool(gSettings.KextPatches[gSettings.NrKexts].Data); //just erase data
                 gSettings.KextPatches[gSettings.NrKexts].Data = NULL;
              }
              if (gSettings.KextPatches[gSettings.NrKexts].Patch != NULL) {
                 FreePool(gSettings.KextPatches[gSettings.NrKexts].Patch); //just erase patch
                 gSettings.KextPatches[gSettings.NrKexts].Patch = NULL;
              }
              continue; //same i
            }
            
            DBG(", data len: %d\n", gSettings.KextPatches[gSettings.NrKexts].DataLen);
            gSettings.NrKexts++; //must be out of DBG because it may be empty compiled
          }
        }
        //gSettings.NrKexts = (INT32)i;
        //there is one moment. This data is allocated in BS memory but will be used 
        // after OnExitBootServices. This is wrong and these arrays should be reallocated
        // but I am not sure
      }
    }
 
    
    // RtVariables
    dictPointer = GetProperty(dict, "RtVariables");
    if (dictPointer) {
      
      // ROM: <data>bin data</data> or <string>base 64 encoded bin data</string>
      prop = GetProperty(dictPointer, "ROM");
      if(prop) {
        UINTN       ROMLength = 0;
        gSettings.RtROM = GetDataSetting(dictPointer, "ROM", &ROMLength);
        gSettings.RtROMLen = ROMLength;
        if (gSettings.RtROM == NULL || gSettings.RtROMLen == 0) {
          gSettings.RtROM = NULL;
          gSettings.RtROMLen = 0;
        }
      }
      
      // MLB: <string>some value</string>
      prop = GetProperty(dictPointer, "MLB");
      if(prop && AsciiStrLen(prop->string) > 0) {
		    gSettings.RtMLB = AllocateCopyPool(AsciiStrSize(prop->string), prop->string);
      }
      
      //a set of variables needed to rc.local
      prop = GetProperty(dictPointer, "MountEFI");
      if (prop) {
        if (prop->type == kTagTypeTrue) {
          gSettings.MountEFI = "Yes";
        } else if (prop->type == kTagTypeFalse) {
          gSettings.MountEFI = "No";
        } else if((prop->type ==  kTagTypeString)  && AsciiStrLen(prop->string) > 0) {
          gSettings.MountEFI = AllocateCopyPool(AsciiStrSize(prop->string), prop->string);
        }
      }
      prop = GetProperty(dictPointer, "LogLineCount");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.LogLineCount = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString && AsciiStrLen(prop->string) > 0){
          gSettings.LogLineCount = (UINT32)AsciiStrDecimalToUintn(prop->string);
        }
      }
      prop = GetProperty(dictPointer, "LogEveryBoot");
      if(prop) {
        if (prop->type == kTagTypeTrue) {
          gSettings.LogEveryBoot = "Yes";
        } else if (prop->type == kTagTypeFalse) {
          gSettings.LogEveryBoot = "No";
        } else if (prop->type == kTagTypeInteger) {
          gSettings.LogEveryBoot = AllocateZeroPool(10); //ten digits will be enough :)
          AsciiSPrint(gSettings.LogEveryBoot, 10, "%d", (UINTN)prop->string);
        } else if ((prop->type ==  kTagTypeString) && AsciiStrLen(prop->string) > 0) {
          gSettings.LogEveryBoot = AllocateCopyPool(AsciiStrSize(prop->string), prop->string);
        }
      }
    }
    if (!gSettings.RtMLB) {
      gSettings.RtMLB = &gSettings.BoardSerialNumber[0];
    }
    if(!gSettings.RtROMLen) {
      gSettings.RtROM = (UINT8*)&gSettings.SmUUID.Data4[2];
      gSettings.RtROMLen = 6;
    }

    if (AsciiStrLen(gSettings.RtMLB) != 17) {
      DBG("Warning! Your MLB is not suitable for iMessage (must be 17 chars long) !\n");
    }
    
    // if CustomUUID and InjectSystemID are not specified
    // then use InjectSystemID=TRUE and SMBIOS UUID
    // to get Chameleon's default behaviour (to make user's life easier)
    CopyMem((VOID*)&gUuid, (VOID*)&gSettings.SmUUID, sizeof(EFI_GUID));
    gSettings.InjectSystemID = TRUE;
    
    // SystemParameters again - values that can depend on previous params
    dictPointer = GetProperty(dict, "SystemParameters");
    if (dictPointer) {

      //BacklightLevel
      prop = GetProperty(dictPointer, "BacklightLevel");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.BacklightLevel = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          gSettings.BacklightLevel = (UINT16)AsciiStrHexToUint64(prop->string);
        }
      }

      prop = GetProperty(dictPointer, "CustomUUID");
      if(prop) {
        if (IsValidGuidAsciiString(prop->string)) {
          AsciiStrToUnicodeStr(prop->string, gSettings.CustomUuid);
          Status = StrToGuidLE(gSettings.CustomUuid, &gUuid);
          // if CustomUUID specified, then default for InjectSystemID=FALSE
          // to stay compatibile with previous Clover behaviour
          gSettings.InjectSystemID = FALSE;
        } else {
          DBG("Error: invalid CustomUUID '%a' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", prop->string);
        }
      }
      //else gUuid value from SMBIOS
      
      prop = GetProperty(dictPointer, "InjectSystemID");
      if(prop) {
        if ((prop->type == kTagTypeFalse) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
          gSettings.InjectSystemID = FALSE;
        else if ((prop->type == kTagTypeTrue) ||
                 ((prop->type == kTagTypeString) &&
                  ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.InjectSystemID = TRUE;
      }
      
    }
    
    /*
    {
      EFI_GUID AppleGuid;
      
      CopyMem((VOID*)&AppleGuid, (VOID*)&gUuid, sizeof(EFI_GUID));
      AppleGuid.Data1 = SwapBytes32(AppleGuid.Data1);
      AppleGuid.Data2 = SwapBytes16(AppleGuid.Data2);
      AppleGuid.Data3 = SwapBytes16(AppleGuid.Data3);
      DBG("Platform Uuid: %g, InjectSystemID: %a\n", &AppleGuid, gSettings.InjectSystemID ? "Yes" : "No");
    }
     */
    
    SaveSettings();
  }	
  //  DBG("config.plist read and return %r\n", Status);
  return Status;
}

CHAR8 *GetOSVersion(IN LOADER_ENTRY *Entry)
{
  CHAR8        *OSVersion = NULL;
  EFI_STATUS    Status = EFI_NOT_FOUND;
  CHAR8*        plistBuffer = NULL;
  UINTN         plistLen;
  TagPtr        dict  = NULL;
  TagPtr        prop  = NULL;
  UINTN         Index = 0;
  
  if (!Entry || !Entry->Volume) {
    return OSVersion;
  }
  if (OSTYPE_IS_OSX(Entry->LoaderType)) {
    // Detect exact version for Mac OS X Regular/Server
    CHAR16* SystemPlists[] = { L"\\System\\Library\\CoreServices\\SystemVersion.plist", // OS X Regular
      L"\\System\\Library\\CoreServices\\ServerVersion.plist", // OS X Server
      NULL };
    for (Index = 0; SystemPlists[Index] != NULL && !FileExists(Entry->Volume->RootDir, SystemPlists[Index]); Index++);
    if (SystemPlists[Index] != NULL) { // found OSX System
      Status = egLoadFile(Entry->Volume->RootDir, SystemPlists[Index], (UINT8 **)&plistBuffer, &plistLen);
      if (!EFI_ERROR(Status) && plistBuffer != NULL && ParseXML(plistBuffer, &dict, 0) == EFI_SUCCESS) {
        prop = GetProperty(dict, "ProductVersion");
        if (prop != NULL && prop->string != NULL && prop->string[0] != '\0') {
          OSVersion = AllocateCopyPool(AsciiStrSize(prop->string), prop->string);
        }
      }
    }
  }
  if (OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {
    // Detect exact version for 2nd stage Installer (thanks to dmazar for this idea)
    // This should work for most installer cases. Rest cases will be read from boot.efi before booting.
    CHAR16 *InstallerPlist = L"\\.IABootFiles\\com.apple.Boot.plist";
    if (FileExists(Entry->Volume->RootDir, InstallerPlist)) {
      Status = egLoadFile(Entry->Volume->RootDir, InstallerPlist, (UINT8 **)&plistBuffer, &plistLen);
      if (!EFI_ERROR(Status) && plistBuffer != NULL && ParseXML(plistBuffer, &dict, 0) == EFI_SUCCESS) {
        prop = GetProperty(dict, "Kernel Flags");
        if(prop != NULL && prop->string != NULL && prop->string[0] != '\0') {
          if (AsciiStrStr(prop->string, "Install%20OS%20X%20Mavericks.app")) {
            OSVersion = AllocateZeroPool(5);
            UnicodeStrToAsciiStr(L"10.9", OSVersion);
          } else if (AsciiStrStr(prop->string, "Install%20OS%20X%20Mountain%20Lion.app")) {
            OSVersion = AllocateZeroPool(5);
            UnicodeStrToAsciiStr(L"10.8", OSVersion);
          } else if (AsciiStrStr(prop->string, "Install%20Mac%20OS%20X%20Lion.app")) {
            OSVersion = AllocateZeroPool(5);
            UnicodeStrToAsciiStr(L"10.7", OSVersion);
          }
        }
      }
    }
  }
  if (OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType)) {
    // Detect exact version for OS X Recovery
    CHAR16 *RecoveryPlist = L"\\com.apple.recovery.boot\\SystemVersion.plist";
    if (FileExists(Entry->Volume->RootDir, RecoveryPlist)) {
      Status = egLoadFile(Entry->Volume->RootDir, RecoveryPlist, (UINT8 **)&plistBuffer, &plistLen);
      if (!EFI_ERROR(Status) && plistBuffer != NULL && ParseXML(plistBuffer, &dict, 0) == EFI_SUCCESS) {
        prop = GetProperty(dict, "ProductVersion");
        if (prop != NULL && prop->string != NULL && prop->string[0] != '\0') {
          OSVersion = AllocateCopyPool(AsciiStrSize(prop->string), prop->string);
        }
      }
    } else if (FileExists(Entry->Volume->RootDir, L"\\com.apple.recovery.boot\\boot.efi")) {
      // Special case - com.apple.recovery.boot/boot.efi exists but SystemVersion.plist doesn't --> 10.9 recovery
      OSVersion = AllocateZeroPool(5);
      UnicodeStrToAsciiStr(L"10.9", OSVersion);
    }
  }
  
  if (plistBuffer != NULL) {
    FreePool(plistBuffer);
  }
  
  return OSVersion;
}

CHAR16 *GetOSIconName(IN CHAR8 *OSVersion)
{
  CHAR16 *OSIconName;
  if (OSVersion == NULL) {
    OSIconName = L"mac";
  } else if (AsciiStrStr(OSVersion, "10.4") != 0) {
    // Tiger
    OSIconName = L"tiger,mac";
  } else if (AsciiStrStr(OSVersion, "10.5") != 0) {
    // Leopard
    OSIconName = L"leo,mac";
  } else if (AsciiStrStr(OSVersion, "10.6") != 0) {
    // Snow Leopard
    OSIconName = L"snow,mac";
  } else if (AsciiStrStr(OSVersion, "10.7") != 0) {
    // Lion
    OSIconName = L"lion,mac";
  } else if (AsciiStrStr(OSVersion, "10.8") != 0) {
    // Mountain Lion
    OSIconName = L"cougar,mac";
  } else if (AsciiStrStr(OSVersion, "10.9") != 0) {
    // Mavericks
    OSIconName = L"mav,mac";
  } else {
    OSIconName = L"mac";
  }
  return OSIconName;
}

//Get the UUID of the AppleRaid or CoreStorage volume from the boot helper partition
EFI_STATUS GetRootUUID(IN REFIT_VOLUME *Volume)
{
	EFI_STATUS				Status = EFI_NOT_FOUND;
	CHAR8*					plistBuffer = 0;
	UINTN                   plistLen;
	TagPtr					dict  = NULL;
	TagPtr					prop  = NULL;
    CHAR16	                Uuid[40];
    
    CHAR16*     SystemPlistP = L"\\com.apple.boot.P\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist";
    CHAR16*     SystemPlistR = L"\\com.apple.boot.R\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist"; //untested, found in chameleon
    CHAR16*     SystemPlistS = L"\\com.apple.boot.S\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist"; //untested, found in chameleon
    
    if (!Volume) {
        return EFI_NOT_FOUND;
    }
    
	if(FileExists(Volume->RootDir, SystemPlistP))
	{
		Status = egLoadFile(Volume->RootDir, SystemPlistP, (UINT8 **)&plistBuffer, &plistLen);
	}
	else if(FileExists(Volume->RootDir, SystemPlistR))
	{
		Status = egLoadFile(Volume->RootDir, SystemPlistR, (UINT8 **)&plistBuffer, &plistLen);
	}
	else if(FileExists(Volume->RootDir, SystemPlistS))
	{
		Status = egLoadFile(Volume->RootDir, SystemPlistS, (UINT8 **)&plistBuffer, &plistLen);
	}
   
	if(!EFI_ERROR(Status))
	{
		if(ParseXML(plistBuffer, &dict, 0) != EFI_SUCCESS)
		{
			FreePool(plistBuffer);
			return EFI_NOT_FOUND;
		}
        
		prop = GetProperty(dict, "Root UUID");
		if(prop != NULL)
		{
       AsciiStrToUnicodeStr(prop->string, Uuid);
       Status = StrToGuidLE(Uuid, &Volume->RootUUID);            
    }
    FreePool(plistBuffer);
	}
	
	return Status;
}


VOID GetDevices(VOID)
{
  EFI_STATUS			Status;
	UINTN           HandleCount = 0;
	EFI_HANDLE			*HandleArray = NULL;
	EFI_PCI_IO_PROTOCOL *PciIo;
	PCI_TYPE00          Pci;
	UINTN         Index;
	UINTN         Segment = 0;
	UINTN         Bus = 0;
	UINTN         Device = 0;
	UINTN         Function = 0;
  UINTN         i;
  UINT32        Bar0;
  UINT8         *Mmio = NULL;
  radeon_card_info_t *info;

  NGFX = 0;
//  Arpt.Valid = FALSE; //global variables initialized by 0 - c-language
  
  // Scan PCI handles 
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleArray
                                    );
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < HandleCount; Index++) {
      Status = gBS->HandleProtocol (
                                    HandleArray[Index],
                                    &gEfiPciIoProtocolGuid,
                                    (VOID **)&PciIo
                                    );
      if (!EFI_ERROR (Status)) {
        // Read PCI BUS 
        Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        Status = PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  sizeof (Pci) / sizeof (UINT32),
                                  &Pci
                                  );
        DBG("PCI (%02x|%02x:%02x.%02x) : %04x %04x class=%02x%02x%02x\n",
            Segment, Bus, Device, Function,
            Pci.Hdr.VendorId, Pci.Hdr.DeviceId,
            Pci.Hdr.ClassCode[2], Pci.Hdr.ClassCode[1], Pci.Hdr.ClassCode[0]);
        // GFX
        if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
            (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA) &&
            (NGFX < 4)) {
          gGraphics[NGFX].DeviceID = Pci.Hdr.DeviceId;
          gGraphics[NGFX].Segment = Segment;
          gGraphics[NGFX].Bus = Bus;
          gGraphics[NGFX].Device = Device;
          gGraphics[NGFX].Function = Function;
          gGraphics[NGFX].Handle = HandleArray[Index];
          switch (Pci.Hdr.VendorId) {
            case 0x1002:
              info = NULL;
              gGraphics[NGFX].Vendor = Ati;
              for (i = 0; radeon_cards[i].device_id ; i++) {
                info = &radeon_cards[i];
                if (radeon_cards[i].device_id == Pci.Hdr.DeviceId) {
                   break;
                }
              }
              AsciiSPrint(gGraphics[NGFX].Model, 64, "%a", info->model_name);
              AsciiSPrint(gGraphics[NGFX].Config, 64, "%a", card_configs[info->cfg_name].name);
              gGraphics[NGFX].Ports = card_configs[info->cfg_name].ports;
              DBG("Found Radeon model=%a\n", gGraphics[NGFX].Model);
              SlotDevices[0].SegmentGroupNum = (UINT16)Segment;
              SlotDevices[0].BusNum = (UINT8)Bus;
              SlotDevices[0].DevFuncNum = (UINT8)((Device << 4) | (Function & 0x0F));
              SlotDevices[0].Valid = TRUE;
              AsciiSPrint(SlotDevices[0].SlotName, 31, "PCI Slot 0");
              SlotDevices[0].SlotID = 1;
              SlotDevices[0].SlotType = SlotTypePciExpressX16;
              break;
            case 0x8086:
              gGraphics[NGFX].Vendor = Intel;
              AsciiSPrint(gGraphics[NGFX].Model, 64, "%a", get_gma_model(Pci.Hdr.DeviceId));
              DBG("Found GFX model=%a\n", gGraphics[NGFX].Model);
              gGraphics[NGFX].Ports = 1;
  /*            SlotDevices[2].SegmentGroupNum = (UINT16)Segment;
              SlotDevices[2].BusNum = (UINT8)Bus;
              SlotDevices[2].DevFuncNum = (UINT8)((Device << 4) | (Function & 0x0F));
              SlotDevices[2].Valid = TRUE;
              AsciiSPrint(SlotDevices[2].SlotName, 31, "PCI Slot 0");
              SlotDevices[2].SlotID = 0;
              SlotDevices[2].SlotType = SlotTypePciExpressX16; */
              break;
            case 0x10de:
              gGraphics[NGFX].Vendor = Nvidia;
              Bar0 = Pci.Device.Bar[0];
              Mmio = (UINT8 *)(UINTN)(Bar0 & ~0x0f);
              //	DBG("BAR: 0x%p\n", Mmio);
              // get card type
              gGraphics[NGFX].Family = (REG32(Mmio, 0) >> 20) & 0x1ff;
              

              AsciiSPrint(gGraphics[NGFX].Model, 64, "%a",
                          get_nvidia_model(((Pci.Hdr.VendorId <<16) | Pci.Hdr.DeviceId),
                                           ((Pci.Device.SubsystemVendorID << 16) | Pci.Device.SubsystemID)));
              DBG("Found NVidia model=%a\n", gGraphics[NGFX].Model);
              gGraphics[NGFX].Ports = 2;
              SlotDevices[1].SegmentGroupNum = (UINT16)Segment;
              SlotDevices[1].BusNum = (UINT8)Bus;
              SlotDevices[1].DevFuncNum = (UINT8)((Device << 4) | (Function & 0x0F));
              SlotDevices[1].Valid = TRUE;
              AsciiSPrint(SlotDevices[1].SlotName, 31, "PCI Slot 0");
              SlotDevices[1].SlotID = 1;
              SlotDevices[1].SlotType = SlotTypePciExpressX16;
              break;
            default:
              gGraphics[NGFX].Vendor = Unknown;
              AsciiSPrint(gGraphics[NGFX].Model, 64, "pci%x,%x", Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
              gGraphics[NGFX].Ports = 1;
              break;
          }                

          NGFX++;
        }   //if gfx    
        else if((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_OTHER)) {
 //         DBG("Found AirPort. Landing enabled...\n");
          SlotDevices[6].SegmentGroupNum = (UINT16)Segment;
          SlotDevices[6].BusNum = (UINT8)Bus;
          SlotDevices[6].DevFuncNum = (UINT8)((Device << 4) | (Function & 0x0F));
          SlotDevices[6].Valid = TRUE;
          AsciiSPrint(SlotDevices[6].SlotName, 31, "Airport");
          SlotDevices[6].SlotID = 0;
          SlotDevices[6].SlotType = SlotTypePciExpressX1;
        }
        else if((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_ETHERNET)) {
          SlotDevices[5].SegmentGroupNum = (UINT16)Segment;
          SlotDevices[5].BusNum = (UINT8)Bus;
          SlotDevices[5].DevFuncNum = (UINT8)((Device << 4) | (Function & 0x0F));
          SlotDevices[5].Valid = TRUE;
          AsciiSPrint(SlotDevices[5].SlotName, 31, "Ethernet");
          SlotDevices[5].SlotID = 2;
          SlotDevices[5].SlotType = SlotTypePciExpressX1;
        }
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_FIREWIRE)) {
          SlotDevices[12].SegmentGroupNum = (UINT16)Segment;
          SlotDevices[12].BusNum = (UINT8)Bus;
          SlotDevices[12].DevFuncNum = (UINT8)((Device << 4) | (Function & 0x0F));
          SlotDevices[12].Valid = TRUE;
          AsciiSPrint(SlotDevices[12].SlotName, 31, "Firewire");
          SlotDevices[12].SlotID = 3;
          SlotDevices[12].SlotType = SlotTypePciExpressX4;
        }

      }
    }
  }
}  

VOID SetDevices(CHAR8 *OSVersion)
{
  //	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *modeInfo;
  EFI_STATUS						Status;
  EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00            Pci;
	UINTN                 HandleCount;
	UINTN                 Index;
	EFI_HANDLE            *HandleBuffer;
	pci_dt_t              PCIdevice;
  UINTN         Segment;
	UINTN         Bus;
	UINTN         Device;
	UINTN         Function;
  BOOLEAN       StringDirty = FALSE;
  BOOLEAN       TmpDirty = FALSE;
  UINT16        PmCon;
  
  GetEdidDiscovered();
  // Scan PCI handles 
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer
                                    );
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < HandleCount; Index++) {
      Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
      if (!EFI_ERROR (Status)) {
        // Read PCI BUS 
        Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
        if (EFI_ERROR (Status)) {
          continue;
        }
        Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        PCIdevice.DeviceHandle = HandleBuffer[Index];
        PCIdevice.dev.addr = (UINT32)PCIADDR(Bus, Device, Function);
        PCIdevice.vendor_id = Pci.Hdr.VendorId;
        PCIdevice.device_id = Pci.Hdr.DeviceId;
        PCIdevice.revision = Pci.Hdr.RevisionID;
        PCIdevice.subclass = Pci.Hdr.ClassCode[0];
        PCIdevice.class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
        PCIdevice.subsys_id.subsys.vendor_id = Pci.Device.SubsystemVendorID;
        PCIdevice.subsys_id.subsys.device_id = Pci.Device.SubsystemID;
        // GFX
        if (/* gSettings.GraphicsInjector && */
            (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
            (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA)) {
          
          //          gGraphics.DeviceID = Pci.Hdr.DeviceId;
          
          switch (Pci.Hdr.VendorId) {
            case 0x1002:
              if (gSettings.InjectATI) {
                //can't do in one step because of C-conventions
                TmpDirty = setup_ati_devprop(&PCIdevice);
                StringDirty |=  TmpDirty;
              } else {
                MsgLog("ATI injection not set\n");
              }
              break;
            case 0x8086:
              if (gSettings.InjectIntel) {
                TmpDirty = setup_gma_devprop(&PCIdevice);
                StringDirty |=  TmpDirty;
                MsgLog("Intel GFX revision  =0x%x\n", PCIdevice.revision);
              } else {
                MsgLog("Intel GFX injection not set\n");
              }
              break;
            case 0x10de:
              if (gSettings.InjectNVidia) {
                TmpDirty = setup_nvidia_devprop(&PCIdevice);
                StringDirty |=  TmpDirty;
              } else {
                MsgLog("NVidia GFX injection not set\n");
              }
              break;
            default:
              break;
          }
        }
        
        //LAN
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_ETHERNET)) {
          //           MsgLog("Ethernet device found\n");
          if (!(gSettings.FixDsdt & FIX_LAN)) {
            TmpDirty = set_eth_props(&PCIdevice);
            StringDirty |=  TmpDirty;
          }
        }
        
        //USB
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_USB)) {
            if (gSettings.USBInjection) {
              TmpDirty = set_usb_props(&PCIdevice);
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
            TmpDirty = set_hda_props(PciIo, &PCIdevice, OSVersion);
            StringDirty |=  TmpDirty;
          }
        }
        
        //LPC
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_BRIDGE) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_BRIDGE_ISA))
        {
          if (gSettings.LpcTune) {
            Status = PciIo->Pci.Read (
                                      PciIo, 
                                      EfiPciIoWidthUint16, 
                                      GEN_PMCON_1, 
                                      1, 
                                      &PmCon
                                      );
            MsgLog("Initial PmCon value=%x\n", PmCon);  
            if (gSettings.EnableC6) {
              PmCon |= 1 << 11;	
              DBG("C6 enabled\n");
            } else {
              PmCon &= ~(1 << 11);
              DBG("C6 disabled\n");
            }
            /*           if (gSettings.EnableC2) {
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
            
            Status = PciIo->Pci.Write (
                                       PciIo, 
                                       EfiPciIoWidthUint16, 
                                       GEN_PMCON_1, 
                                       1, 
                                       &PmCon
                                       );
            
            Status = PciIo->Pci.Read (
                                      PciIo, 
                                      EfiPciIoWidthUint16, 
                                      GEN_PMCON_1, 
                                      1, 
                                      &PmCon
                                      );
            MsgLog("Set PmCon value=%x\n", PmCon);                   
            
          } 
        }
      }
    }
  }
	
  if (StringDirty) {
    EFI_PHYSICAL_ADDRESS  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
    stringlength = string->length * 2;
    DBG("stringlength = %d\n", stringlength);
   // gDeviceProperties = AllocateAlignedPages(EFI_SIZE_TO_PAGES(stringlength + 1), 64);
    
    Status = gBS->AllocatePages (
                                 AllocateMaxAddress,
                                 EfiACPIReclaimMemory,
                                 EFI_SIZE_TO_PAGES(stringlength+1),
                                 &BufferPtr
                                 );
    if (!EFI_ERROR(Status)) {
      mProperties = (UINT8*)(UINTN)BufferPtr; 
      gDeviceProperties = (VOID*)devprop_generate_string(string);
      gDeviceProperties[stringlength] = 0;
//          DBG(gDeviceProperties);
//          DBG("\n");
      StringDirty = FALSE;
      //-------
      mPropSize = (UINT32)AsciiStrLen(gDeviceProperties) / 2;
 //     DBG("Preliminary size of mProperties=%d\n", mPropSize);
      mPropSize = hex2bin(gDeviceProperties, mProperties, mPropSize);
 //     DBG("Final size of mProperties=%d\n", mPropSize);
      //---------      
    }
	}
  
	MsgLog("CurrentMode: Width=%d Height=%d\n", UGAWidth, UGAHeight);  
}

EFI_STATUS SaveSettings()
{
  // TODO: SetVariable()..
  //here we can apply user settings instead of default one
  gMobile = gSettings.Mobile;
  
  if ((gSettings.BusSpeed != 0) &&
      (gSettings.BusSpeed > 10 * kilo) &&
      (gSettings.BusSpeed < 500 * kilo)){
    gCPUStructure.ExternalClock = gSettings.BusSpeed;
    gCPUStructure.FSBFrequency = MultU64x64(gSettings.BusSpeed, kilo); //kHz -> Hz
    gCPUStructure.MaxSpeed = (UINT32)(DivU64x32((UINT64)gSettings.BusSpeed  * gCPUStructure.MaxRatio, 10000)); //kHz->MHz
  }

  if ((gSettings.CpuFreqMHz > 100) &&
      (gSettings.CpuFreqMHz < 20000)){
    gCPUStructure.MaxSpeed = gSettings.CpuFreqMHz;
  }
  
  gCPUStructure.CPUFrequency = MultU64x64(gCPUStructure.MaxSpeed, Mega);
  return EFI_SUCCESS;
}

//dmazar
CHAR16* GetExtraKextsDir(CHAR8 *OSVersion)
{
  CHAR16 *SrcDir = NULL;
  CHAR8   FixedVersion[6] = "Other";

  if (OSVersion != NULL) {
    AsciiStrnCpy(FixedVersion, OSVersion, 4);
    FixedVersion[4] = 0;
  }
  
  //MsgLog("OS=%s\n", OSTypeStr);
  
  // find source injection folder with kexts
  // note: we are just checking for existance of particular folder, not checking if it is empty or not
  // check OEM subfolders: version specific or default to Other
  SrcDir = PoolPrint(L"%s\\kexts\\%a", OEMPath, FixedVersion);
  if (!FileExists(SelfVolume->RootDir, SrcDir)) {
    FreePool(SrcDir);
    SrcDir = PoolPrint(L"%s\\kexts\\Other", OEMPath);
    if (!FileExists(SelfVolume->RootDir, SrcDir)) {
      FreePool(SrcDir);
      SrcDir = NULL;
    }
  }
  if (SrcDir == NULL) {
    // if not found, check EFI\CLOVER\kexts\...
    SrcDir = PoolPrint(L"\\EFI\\CLOVER\\kexts\\%a", FixedVersion);
    if (!FileExists(SelfVolume->RootDir, SrcDir)) {
      FreePool(SrcDir);
 //     SrcDir = PoolPrint(L"\\EFI\\CLOVER\\kexts\\Other", gSettings.OEMProduct);
      SrcDir = PoolPrint(L"\\EFI\\CLOVER\\kexts\\Other");
      if (!FileExists(SelfVolume->RootDir, SrcDir)) {
        FreePool(SrcDir);
        SrcDir = NULL;
      }
    }
  }
  
  return SrcDir;
}

EFI_STATUS SetFSInjection(IN LOADER_ENTRY *Entry)
{
    EFI_STATUS                  Status;
    REFIT_VOLUME                *Volume;
    FSINJECTION_PROTOCOL        *FSInject;
    CHAR16                      *SrcDir = NULL;
    BOOLEAN                     InjectionNeeded = FALSE;
    BOOLEAN                     BlockCaches = FALSE;
    FSI_STRING_LIST             *Blacklist = 0;
    FSI_STRING_LIST             *ForceLoadKexts;

    MsgLog("FSInjection: ");
    
    Volume = Entry->Volume;
    
    // some checks?
    /*
    // apianti - this seems to not work sometimes or ever, so just always start
    if ((Volume->BootType == BOOTING_BY_PBR) ||
        (Volume->BootType == BOOTING_BY_MBR) ||
        (Volume->BootType == BOOTING_BY_CD)) {
        MsgLog("not started - not an EFI boot\n");
        return EFI_UNSUPPORTED;
    }
    //*/
    
    // get FSINJECTION_PROTOCOL
    Status = gBS->LocateProtocol(&gFSInjectProtocolGuid, NULL, (void **)&FSInject);
    if (EFI_ERROR(Status)) {
      //Print(L"- No FSINJECTION_PROTOCOL, Status = %r\n", Status);
      MsgLog("not started - gFSInjectProtocolGuid not found\n");
      return EFI_NOT_STARTED;
    }

    // check if blocking of caches is needed
    if (OSFLAG_ISSET(Entry->Flags, OSFLAG_NOCACHES)) {
        MsgLog("blocking caches");
        BlockCaches = TRUE;
        // add caches to blacklist
        Blacklist = FSInject->CreateStringList();
        if (Blacklist == NULL) {
          MsgLog(": Error not enough memory!\n");
          return EFI_NOT_STARTED;
        }
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\kernelcache");
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\Extensions.mkext");
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Extensions.mkext");
        FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\kernelcache");
        FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\Extensions.mkext");
      if (gSettings.BlockKexts[0] != L'\0') {
        FSInject->AddStringToList(Blacklist, PoolPrint(L"\\System\\Library\\Extensions\\%s", gSettings.BlockKexts));
      }
        MsgLog(", ");
    }
    
    // check if kext injection is needed
    // (will be done only if caches are blocked or if boot.efi refuses to load kernelcache)
    SrcDir = NULL;
    if (OSFLAG_ISSET(Entry->Flags, OSFLAG_WITHKEXTS)) {
        SrcDir = GetExtraKextsDir(Entry->OSVersion);
        if (SrcDir != NULL) {
          // we have found it - injection will be done
          MsgLog("using kexts path: '%s'", SrcDir);
          InjectionNeeded = TRUE;
        } else {
          MsgLog("skipping kext injection (kexts folder not found)");
        }
    } else {
        MsgLog("skipping kext injection (not requested)");
    }
    
    // prepare list of kext that will be forced to load
    ForceLoadKexts = FSInject->CreateStringList();
    if (ForceLoadKexts == NULL) {
        MsgLog(" - Error: not enough memory!\n");
        return EFI_NOT_STARTED;
    }
    KextPatcherRegisterKexts(FSInject, ForceLoadKexts);

    Status = FSInject->Install(Volume->DeviceHandle, L"\\System\\Library\\Extensions",
                               SelfVolume->DeviceHandle, SrcDir,
                               Blacklist, ForceLoadKexts);
    
    if (SrcDir != NULL) FreePool(SrcDir);
    
    if (EFI_ERROR(Status)) {
        MsgLog(" - Error: could not install injection!\n");
        return EFI_NOT_STARTED;
    }
    
    // reinit Volume->RootDir? it seems it's not needed.
    
    MsgLog("\n");
	return Status;
}

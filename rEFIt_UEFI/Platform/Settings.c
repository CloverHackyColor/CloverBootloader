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

TagPtr                          gConfigDict = NULL;
SETTINGS_DATA                   gSettings;
LANGUAGES                       gLanguage;
GFX_PROPERTIES                  gGraphics[4]; //no more then 4 graphics cards
SLOT_DEVICE                     Arpt;
EFI_EDID_DISCOVERED_PROTOCOL    *EdidDiscovered;
UINT8                           *gEDID = NULL;
//EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;
//UINT16                          gCPUtype;
UINTN                           NGFX = 0; // number of GFX

// firmware
BOOLEAN                         gFirmwareClover = FALSE;
UINTN                           gEvent;
UINT16                          gBacklightLevel;

GUI_ANIME *GuiAnime = NULL;

extern INTN ScrollWidth;
extern INTN ScrollButtonsHeight;
extern INTN ScrollBarDecorationsHeight;
extern INTN ScrollScrollDecorationsHeight;

// global configuration with default values
REFIT_CONFIG   GlobalConfig = { FALSE, -1, 0, 0, 0, TRUE, FALSE, FALSE, FALSE, FONT_ALFA, 7, 0xFFFFFF80, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, None };

VOID __inline WaitForSts(VOID) {
	UINT32 inline_timeout = 100000;
	while (AsmReadMsr64(MSR_IA32_PERF_STATUS) & (1 << 21)) { if (!inline_timeout--) break; }
}

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

EFI_STATUS LoadUserSettings(IN EFI_FILE *RootDir)
{
  EFI_STATUS	Status = EFI_NOT_FOUND;
  UINTN       size;
  CHAR8*      gConfigPtr = NULL;
  CHAR16*     ConfigPlistPath = PoolPrint(L"EFI\\CLOVER\\%s.plist", gSettings.ConfigName);
  CHAR16*     ConfigOemPath = PoolPrint(L"%s\\%s.plist", OEMPath, gSettings.ConfigName);
  
  DBG("ConfigPlistPath: %s\n", ConfigPlistPath);
  
  // load config
  if (FileExists(SelfRootDir, ConfigOemPath)) {
    Status = egLoadFile(SelfRootDir, ConfigOemPath, (UINT8**)&gConfigPtr, &size);
  } else {
    DBG("Oem %s.plist not found at path: %s\n", gSettings.ConfigName, ConfigOemPath);
  }
  
  if (EFI_ERROR(Status)) {
    if ((RootDir != NULL) && FileExists(RootDir, ConfigPlistPath)) {
      Status = egLoadFile(RootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &size);
    }
    if (EFI_ERROR(Status)) {
      Status = egLoadFile(SelfRootDir, ConfigPlistPath, (UINT8**)&gConfigPtr, &size);
    }
  }	else {
    DBG("Using OEM config.plist at path: %s\n", ConfigOemPath);
  }
  
  
  if(EFI_ERROR(Status) || gConfigPtr == NULL) {
    DBG("Error loading config.plist! Status=%r\n", Status);
    return Status;
  }
  
  if(ParseXML((const CHAR8*)gConfigPtr, &gConfigDict) != EFI_SUCCESS) {
    gConfigDict = NULL;
    DBG(" config parse error\n");
    return EFI_UNSUPPORTED;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS GetEarlyUserSettings(IN EFI_FILE *RootDir)
{
  EFI_STATUS	Status = EFI_NOT_FOUND;
  TagPtr      dict;
  TagPtr      dict2;
  TagPtr      dictPointer;
  TagPtr      prop;
 // CHAR8       ANum[4];
 // UINTN       i = 0;
  if (gConfigDict == NULL) {
    Status = LoadUserSettings(RootDir);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  DBG("Loading early settings\n");
  //Graphics
  // это не переносится в refit.conf, потому что оно загружается только один раз
  
  dict = gConfigDict;
  dictPointer = GetProperty(dict, "GUI");
  if (dictPointer) {
    prop = GetProperty(dictPointer, "Timeout");
    if (prop) {
      if (prop->type == kTagTypeInteger) {
        GlobalConfig.Timeout = (INTN)prop->string;
        DBG("timeout set to %d\n", GlobalConfig.Timeout);
      } else if ((prop->type == kTagTypeString) && prop->string) {
        if (prop->string[0] == '-') {
          GlobalConfig.Timeout = -1;
        } else {
          GlobalConfig.Timeout = (INTN)AsciiStrDecimalToUintn(prop->string);
        }
      }
    }
    prop = GetProperty(dictPointer, "Theme");
    if (prop) {
      if ((prop->type == kTagTypeString) && prop->string) {
        GlobalConfig.Theme = PoolPrint(L"%a", prop->string);
        DBG("Default theme: %s\n", GlobalConfig.Theme);
      }
    }
    prop = GetProperty(dictPointer, "SystemLog");
    if (prop) {
      if ((prop->type == kTagTypeTrue) ||
          ((prop->type == kTagTypeString) && prop->string &&
           ((prop->string[0] == 'Y') || (prop->string[0] == 'y')))) {
        GlobalConfig.SystemLog = TRUE;
      }
    }
    prop = GetProperty(dictPointer, "ScreenResolution");
    if (prop) {
      if ((prop->type == kTagTypeString) && prop->string) {
        GlobalConfig.ScreenResolution = PoolPrint(L"%a", prop->string);
      }
    }
    prop = GetProperty(dictPointer, "Mouse");
    if (prop) {
      dict2 = GetProperty(prop, "Enabled");
      if (dict2) {
        if ((dict2->type == kTagTypeFalse) ||
            ((dict2->type == kTagTypeString) && dict2->string &&
             ((dict2->string[0] == 'N') || (dict2->string[0] == 'n')))) {
          gSettings.PointerEnabled = FALSE;
        }
      }
      dict2 = GetProperty(prop, "Speed");
      if (dict2) {
        if (dict2->type == kTagTypeInteger) {
          gSettings.PointerSpeed = (INTN)dict2->string;
          if (gSettings.PointerSpeed < 0) {
            gSettings.PointerSpeed = -gSettings.PointerSpeed;
          }
        } else if ((dict2->type == kTagTypeString) && dict2->string) {
          gSettings.DoubleClickTime = (UINT16)AsciiStrDecimalToUintn(prop->string + ((dict2->string[0] == '-') ? 1 : 0));
        }
        gSettings.PointerEnabled = (gSettings.PointerSpeed > 0);
      }
      dict2 = GetProperty(prop, "Mirror");
      if (dict2) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) && prop->string &&
            ((prop->string[0] == 'Y') || (prop->string[0] == 'y')))) {
          gSettings.PointerMirror = TRUE;
        }
      }
      dict2 = GetProperty(prop, "DoubleClickTime");
      if (dict2) {
        if (prop->type == kTagTypeInteger) {
          gSettings.DoubleClickTime = (UINTN)prop->string;
        } else if (prop->type == kTagTypeString) {
          gSettings.DoubleClickTime = (UINT16)AsciiStrDecimalToUintn(prop->string + ((dict2->string[0] == '-') ? 1 : 0));
        }
      }
    }
            // Hide entries
    prop = GetProperty(dictPointer, "HideEntries");
    if (prop) {
      DBG("configure Hide entries\n");
      dict2 = GetProperty(prop, "OSXInstall");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideAllOSXInstall = TRUE;
        }
      }
      dict2 = GetProperty(prop, "Recovery");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideAllRecovery = TRUE;
        }
      }
      dict2 = GetProperty(prop, "Duplicate");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideDuplicatedBootTarget = TRUE;
        }
      }
      dict2 = GetProperty(prop, "WindowsEFI");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideAllWindowsEFI = TRUE;
        }
      }
      dict2 = GetProperty(prop, "Grub");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideAllGrub = TRUE;
        }
      }
      dict2 = GetProperty(prop, "Gentoo");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideAllGentoo = TRUE;
        }
      }
      dict2 = GetProperty(prop, "Ubuntu");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideAllUbuntu = TRUE;
        }
      }
      dict2 = GetProperty(prop, "OpticalUEFI");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideOpticalUEFI = TRUE;
        }
      }
      dict2 = GetProperty(prop, "InternalUEFI");
      if (dict2) {
        DBG("hide InternalUEFI\n");
        if (dict2->type == kTagTypeTrue) {
          DBG("....true\n");
          gSettings.HVHideInternalUEFI = TRUE;
        } else {
          DBG("....false\n");
        }
      }
      dict2 = GetProperty(prop, "ExternalUEFI");
      if (dict2) {
        if (dict2->type == kTagTypeTrue) {
          gSettings.HVHideExternalUEFI = TRUE;
        }
      }
    }
    // Hide volumes
    prop = GetProperty(dictPointer, "Volumes");
    if (prop) {
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
      // hide by name/uuid
      dict2 = GetProperty(prop, "Hide");
      if (dict2) {
        INTN i, Count = GetTagCount(dict2);
        if (Count > 0) {
          gSettings.HVCount = 0;
          gSettings.HVHideStrings = AllocateZeroPool(Count * sizeof(CHAR16 *));
          if (gSettings.HVHideStrings) {
            for (i = 0; i < Count; ++i) {
              if (EFI_ERROR(GetElement(dict2, i, &dictPointer))) {
                continue;
              }
              if (dictPointer == NULL) {
                break;
              }
              if ((dictPointer->type == kTagTypeString) && dictPointer->string) {
                gSettings.HVHideStrings[gSettings.HVCount] = PoolPrint(L"%a", dictPointer->string);
                if (gSettings.HVHideStrings[gSettings.HVCount]) {
                  DBG("Hiding volume with string %s\n", gSettings.HVHideStrings[gSettings.HVCount]);
                  gSettings.HVCount++;
                }
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
      if (j != 128) {
        DBG("CustomEDID has wrong length=%d\n", j);
      } else {
        DBG("CustomEDID ok\n");
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

  return Status;
}
#define CONFIG_THEME_PATH L"theme.plist"

STATIC EFI_STATUS GetThemeTagSettings(TagPtr dictPointer)
{
  TagPtr dict, dict2;
  if (dictPointer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  dict = GetProperty(dictPointer, "Background");
  if (dict) {
    dict2 = GetProperty(dict, "Type");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        if ((dict2->string[0] == 'C') || (dict2->string[0] == 'c')) {
          GlobalConfig.BackgroundScale = Crop;
        } else if ((dict2->string[0] == 'S') || (dict2->string[0] == 's')) {
          GlobalConfig.BackgroundScale = Scale;
        } else if ((dict2->string[0] == 'T') || (dict2->string[0] == 't')) {
          GlobalConfig.BackgroundScale = Tile;
        }
      }
    }
    dict2 = GetProperty(dict, "Path");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        if (GlobalConfig.BackgroundName) {
          FreePool(GlobalConfig.BackgroundName);
        }
        GlobalConfig.BackgroundName = PoolPrint(L"%a", dict2->string);
      }
    }
  }
  dict = GetProperty(dictPointer, "Banner");
  if (dict) {
    if ((dict->type == kTagTypeString) && dict->string) {
      if (GlobalConfig.BannerFileName) {
        FreePool(GlobalConfig.BannerFileName);
      }
      GlobalConfig.BannerFileName = PoolPrint(L"%a", dict->string);
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
        if (GlobalConfig.SelectionSmallFileName) {
          FreePool(GlobalConfig.SelectionSmallFileName);
        }
        GlobalConfig.SelectionSmallFileName = PoolPrint(L"%a", dict2->string);
      }
    }
    dict2 = GetProperty(dict, "Big");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        if (GlobalConfig.SelectionBigFileName) {
          FreePool(GlobalConfig.SelectionBigFileName);
        }
        GlobalConfig.SelectionBigFileName = PoolPrint(L"%a", dict2->string);
      }
    }
/*    dict2 = GetProperty(dict, "Default");
    if (dict2) {
      if ((dict2->type == kTagTypeString) && dict2->string) {
        if (GlobalConfig.DefaultSelection) {
          FreePool(GlobalConfig.DefaultSelection);
        }
        GlobalConfig.DefaultSelection = PoolPrint(L"%a", dict2->string);
      }
    } */
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
        if (GlobalConfig.FontFileName) {
          FreePool(GlobalConfig.FontFileName);
        }
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
          if (Anime->Path) {
            FreePool(Anime->Path);
          }
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
  return EFI_SUCCESS;
}

EFI_STATUS GetThemeSettings(VOID)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  //get theme from NVRAM in the case of UEFI boot
  UINTN   Size = 0;
  TagPtr  ThemeDict = NULL;
  CHAR8  *ThemePtr = NULL;
  CHAR8  *chosenTheme = GetNvramVariable(L"Clover.Theme", &gEfiAppleBootGuid, NULL, &Size);
  if (chosenTheme) {
    ThemePath = PoolPrint(L"EFI\\CLOVER\\themes\\%a", chosenTheme);
    if (ThemePath) {
      Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath, EFI_FILE_MODE_READ, 0);
      if (!EFI_ERROR(Status)) {
        Status = egLoadFile(ThemeDir, CONFIG_THEME_PATH, (UINT8**)&ThemePtr, &Size);
        if (!EFI_ERROR(Status) &&
            ((ThemePtr == NULL) || (Size == 0))) {
          Status = EFI_NOT_FOUND;
        } else {
          Status = ParseXML((const CHAR8*)ThemePtr, &ThemeDict);
          if (!EFI_ERROR(Status) && (ThemeDict == NULL)) {
            Status = EFI_UNSUPPORTED;
          }
        }
      }
    }
  }
  // Try to get theme from settings
  if (EFI_ERROR(Status)) {
    if (GlobalConfig.Theme) {
      if (chosenTheme) {
        DBG("theme %s chosen from nvram is absent, using default\n", chosenTheme);
        chosenTheme = NULL;
      }
      if (ThemePath) {
        FreePool(ThemePath);
      }
      ThemePath = PoolPrint(L"EFI\\CLOVER\\themes\\%s", GlobalConfig.Theme);
      if (ThemePath) {
        if (ThemeDir) {
          ThemeDir->Close(ThemeDir);
          ThemeDir = NULL;
        }
        Status = SelfRootDir->Open(SelfRootDir, &ThemeDir, ThemePath, EFI_FILE_MODE_READ, 0);
        if (!EFI_ERROR(Status)) {
           Status = egLoadFile(ThemeDir, CONFIG_THEME_PATH, (UINT8**)&ThemePtr, &Size);
           if (!EFI_ERROR(Status) &&
              ((ThemePtr == NULL) || (Size == 0))) {
                 Status = EFI_NOT_FOUND;
           } else {
              Status = ParseXML((const CHAR8*)ThemePtr, &ThemeDict);
              if (!EFI_ERROR(Status) && (ThemeDict == NULL)) {
                 Status = EFI_UNSUPPORTED;
              }
           }
        }
      }
      if (EFI_ERROR(Status)) {
        DBG("default theme %s is absent, using embedded\n", GlobalConfig.Theme);
      }
    } else if (chosenTheme) {
      DBG("theme %s chosen from nvram is absent, using embedded\n", chosenTheme);
      chosenTheme = NULL;
    } else {
      DBG("no default theme, using embedded\n");
    }
  }
  // Read defaults from config
  if (gConfigDict) {
    TagPtr dictPointer = GetProperty(gConfigDict, "Theme");
    if (dictPointer) {
      EFI_STATUS S = GetThemeTagSettings(dictPointer);
      if (EFI_ERROR(S)) {
         DBG("Config theme error: %r\n", S);
      }
    }
  }
  // Check for fall back to embedded theme
  if (EFI_ERROR(Status) || (ThemeDict == NULL)) {
    if (GlobalConfig.Theme) {
      FreePool(GlobalConfig.Theme);
    }
    GlobalConfig.Theme = NULL;
    if (ThemePath) {
      FreePool(ThemePath);
    }
    ThemePath = NULL;
    if (ThemeDir) {
      ThemeDir->Close(ThemeDir);
    }
    ThemeDir = NULL;
  } else {
    TagPtr dictPointer = GetProperty(ThemeDict, "Theme");
    if (chosenTheme) {
      DBG("Theme: %a Path: %s\n", chosenTheme, ThemePath);
    } else {
      DBG("Theme: %s Path: %s\n", GlobalConfig.Theme, ThemePath);
    }
    // read theme settings
    if (dictPointer) {
      Status = GetThemeTagSettings(dictPointer);
    }
  }
  if (ThemeDict) {
    FreeTag(ThemeDict);
  }
  return Status;
}

EFI_STATUS GetUserSettings(IN EFI_FILE *RootDir)
{
  EFI_STATUS	Status = EFI_NOT_FOUND;
  TagPtr      dict, dict2;
  TagPtr      prop;
  TagPtr      dictPointer;
  UINTN       i;
  //CHAR8       ANum[4];
  CHAR16      UStr[64];
  
  if (gConfigDict == NULL) {
    Status = LoadUserSettings(RootDir);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  dict = gConfigDict;
  if(dict != NULL) {
    
    DBG("Loading main settings\n");
    
    //*** SYSTEM ***
    dictPointer = GetProperty(dict, "SystemParameters");
    if (dictPointer) {
      prop = GetProperty(dictPointer, "prev-lang:kbd");
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
      
      prop = GetProperty(dictPointer, "boot-args");
      if(prop) {
        AsciiStrCpy(gSettings.BootArgs, prop->string);
      }
      
      prop = GetProperty(dictPointer, "DefaultBootVolume");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, gSettings.DefaultBoot);
      }
      
      prop = GetProperty(dictPointer, "LegacyBoot");
      if(prop)  {
        AsciiStrToUnicodeStr(prop->string, gSettings.LegacyBoot);
      }
      //BacklightLevel
      prop = GetProperty(dictPointer, "BacklightLevel");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.BacklightLevel = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.BacklightLevel = (UINT16)StrHexToUint64((CHAR16*)&UStr[0]);
        }
      }
    }

    //Graphics
    
    dictPointer = GetProperty(dict, "Graphics");
    if (dictPointer) {
      prop = GetProperty(dictPointer, "GraphicsInjector");
      if(prop) {
        if ((prop->type == kTagTypeFalse) ||
            ((prop->type == kTagTypeString) &&
            ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
          gSettings.GraphicsInjector = FALSE;
        else if ((prop->type == kTagTypeTrue) ||
                 ((prop->type == kTagTypeString) &&
                 ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.GraphicsInjector = TRUE;
      }      
      prop = GetProperty(dictPointer, "VRAM");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.VRAM = LShiftU64((UINTN)prop->string, 20);
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.VRAM = LShiftU64(StrDecimalToUintn((CHAR16*)&UStr[0]), 20);  //Mb -> bytes
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
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.VideoPorts = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
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
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.DualLink = (UINT32)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
      }
      //InjectEDID
      prop = GetProperty(dictPointer, "InjectEDID");
      gSettings.InjectEDID = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.InjectEDID = TRUE;
      }
      prop = GetProperty(dictPointer, "CustomEDID");
      if(prop) {
        UINTN j = 128;
        gSettings.CustomEDID = GetDataSetting(dictPointer, "CustomEDID", &j);
        if (j != 128) {
          DBG("CustomEDID has wrong length=%d\n", j);
        } else {
          DBG("CustomEDID ok\n");
        }
      }
      prop = GetProperty(dictPointer, "ig-platform-id");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.IgPlatform = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.IgPlatform = (UINT32)StrHexToUint64((CHAR16*)&UStr[0]);
        }
      }
    }    
    
    dictPointer = GetProperty(dict, "PCI");
    if (dictPointer) {      
      prop = GetProperty(dictPointer, "PCIRootUID");
      gSettings.PCIRootUID = 0;
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.PCIRootUID = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.PCIRootUID = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
      }
      prop = GetProperty(dictPointer, "StringInjector");
      gSettings.StringInjector = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.StringInjector = TRUE;
      }
      prop = GetProperty(dictPointer, "DeviceProperties");
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
      prop = GetProperty(dictPointer, "LpcTune");
      gSettings.LpcTune = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.LpcTune = TRUE;
      }
      // HDA
      prop = GetProperty(dictPointer, "HDAInjection");
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
      // USB
      prop = GetProperty(dictPointer, "USBInjection");
      if(prop) {
        // enabled by default
        // syntax: USBInjection=Yes/No
        if ((prop->type == kTagTypeFalse) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'n') || (prop->string[0] == 'N')))) {
          gSettings.USBInjection = FALSE;
        }
      }
      prop = GetProperty(dictPointer, "InjectClockID");
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
      prop = GetProperty(dictPointer, "USBFixOwnership");
      if(prop) {
        if ((prop->type == kTagTypeFalse) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
          gSettings.USBFixOwnership = FALSE;
        else if ((prop->type == kTagTypeTrue) ||
                 ((prop->type == kTagTypeString) &&
                  ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.USBFixOwnership = TRUE;
          DBG("USBFixOwnership: true\n");
         }
      }
      
    }
    
    //*** ACPI ***//
    
    dictPointer = GetProperty(dict,"ACPI");
    if (dictPointer) {
      //gSettings.DsdtName by default is "DSDT.aml", but name "BIOS" will mean autopatch
      prop = GetProperty(dictPointer, "DsdtName");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, gSettings.DsdtName);
      }

      prop = GetProperty(dictPointer, "GenerateIvyStates");
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.GeneratePStates = TRUE;
          gSettings.GenerateCStates = TRUE;
          gSettings.EnableISS       = FALSE;
    //      gSettings.EnableC2        = TRUE;
    //      gSettings.EnableC6        = TRUE;
          gSettings.PluginType      = 1;
          gSettings.MinMultiplier   = 7;
          gSettings.DoubleFirstState = TRUE;
          gSettings.DropSSDT        = TRUE;
          gSettings.C3Latency       = 0x3E7;
        }
      }

      prop = GetProperty(dictPointer, "DropOemSSDT");
//      gSettings.DropSSDT = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.DropSSDT = TRUE;
      }
      prop = GetProperty(dictPointer, "GeneratePStates");
//      gSettings.GeneratePStates = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.GeneratePStates = TRUE;
      }
      prop = GetProperty(dictPointer, "GenerateCStates");
//      gSettings.GenerateCStates = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.GenerateCStates = TRUE;
      }
//      gSettings.PLimitDict = 0;
      prop = GetProperty(dictPointer, "PLimitDict");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.PLimitDict = (UINT8)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.PLimitDict = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);	
        }
      }
//      gSettings.UnderVoltStep = 0;
      prop = GetProperty(dictPointer, "UnderVoltStep");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.UnderVoltStep = (UINT8)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.UnderVoltStep = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
      }      
      prop = GetProperty(dictPointer, "DoubleFirstState");
//      gSettings.DoubleFirstState = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.DoubleFirstState = TRUE;
      }
      prop = GetProperty(dictPointer,"MinMultiplier");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.MinMultiplier = (UINT8)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.MinMultiplier = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
        DBG("Config set MinMultiplier=%d\n", gSettings.MinMultiplier);
      }      
      prop = GetProperty(dictPointer,"MaxMultiplier");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.MaxMultiplier = (UINT8)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.MaxMultiplier = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
        DBG("Config set MaxMultiplier=%d\n", gSettings.MaxMultiplier);
      }      
      prop = GetProperty(dictPointer,"PluginType");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.PluginType = (UINT8)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.PluginType = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
        DBG("Config set PluginType=%d\n", gSettings.PluginType);
      }      

      prop = GetProperty(dictPointer, "ResetAddress");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.ResetAddr = (UINT64)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.ResetAddr  = StrHexToUint64(UStr);
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
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.ResetVal = (UINT8)StrHexToUint64((CHAR16*)&UStr[0]);
        }
        DBG("Config set ResetVal=0x%x\n", gSettings.ResetVal);
      }
      //other known pair is 0x0CF9/0x06. What about 0x92/0x01 ?
      
      prop = GetProperty(dictPointer, "EnableC6");
//      gSettings.EnableC6 = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.EnableC6 = TRUE;
      }
      
      prop = GetProperty(dictPointer, "EnableC4");
//      gSettings.EnableC4 = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.EnableC4 = TRUE;
      }
      
      prop = GetProperty(dictPointer, "EnableC2");
//      gSettings.EnableC2 = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.EnableC2 = TRUE;
          DBG(" C2 enabled\n");
        }
      }
 //     gSettings.C3Latency = 0; //Usually it is 0x03e9, but if you want Turbo, you may set 0x00FA
      prop = GetProperty(dictPointer, "C3Latency");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.C3Latency = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.C3Latency = (UINT16)StrHexToUint64(UStr);
        }
      }
            
      prop = GetProperty(dictPointer, "EnableISS");
//      gSettings.EnableISS = FALSE; //we set default value in GetDefaultSettings()
      if(prop) {
        if ((prop->type == kTagTypeFalse) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
          gSettings.EnableISS = FALSE;
        else if ((prop->type == kTagTypeTrue) ||
                 ((prop->type == kTagTypeString) &&
                  ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.EnableISS = TRUE;
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
      prop = GetProperty(dictPointer, "FixDsdtMask");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.FixDsdt  = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.FixDsdt  = (UINT32)StrHexToUint64(UStr); 
        }
        DBG("Config set Fix DSDT mask=%04x\n", gSettings.FixDsdt);
      }
      prop = GetProperty(dictPointer, "DropAPIC");
      gSettings.bDropAPIC = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.bDropAPIC = TRUE;
      }
      prop = GetProperty(dictPointer, "DropMCFG");
      gSettings.bDropMCFG = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.bDropMCFG = TRUE;
      }
      prop = GetProperty(dictPointer, "DropHPET");
      gSettings.bDropHPET = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.bDropHPET = TRUE;
      }
      prop = GetProperty(dictPointer, "DropECDT");
      gSettings.bDropECDT = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.bDropECDT = TRUE;
      }
      prop = GetProperty(dictPointer, "DropDMAR");
      gSettings.bDropDMAR = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.bDropDMAR = TRUE;
      }
      prop = GetProperty(dictPointer, "DropBGRT");
      gSettings.bDropBGRT = TRUE;
      if(prop) {
        if ((prop->type == kTagTypeFalse) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
          gSettings.bDropBGRT = FALSE;
      }
/*      prop = GetProperty(dictPointer, "RememberBIOS");
      gSettings.RememberBIOS = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))))
          gSettings.RememberBIOS = TRUE;
      }
 */
      
    }
    
    //*** SMBIOS ***//
    dictPointer = GetProperty(dict,"SMBIOS");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"ProductName");
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
          Status = StrToGuidLE((CHAR16*)&UStr[0], &gSettings.SmUUID);
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
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.BoardType = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);
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
    }
    
    //CPU
    dictPointer = GetProperty(dict,"CPU");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"Turbo");
 //     gSettings.Turbo = FALSE;
      if(prop) {
        if ((prop->type == kTagTypeFalse) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'n') || (prop->string[0] == 'N'))))
          gSettings.Turbo = FALSE;
        else if ((prop->type == kTagTypeTrue) ||
                 ((prop->type == kTagTypeString) &&
                  ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
          gSettings.Turbo = TRUE;
          DBG("Config set Turbo\n");
        }
      }
      prop = GetProperty(dictPointer,"QPI");
      gSettings.QPI = (UINT16)gCPUStructure.ProcessorInterconnectSpeed; //MHz
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.QPI = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.QPI = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
        DBG("Config set QPI=%dMHz\n", gSettings.QPI);
      }
      prop = GetProperty(dictPointer,"CpuFrequencyMHz");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.CpuFreqMHz = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.CpuFreqMHz = (UINT32)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
        DBG("Config set CpuFreq=%dMHz\n", gSettings.CpuFreqMHz);
      }
      prop = GetProperty(dictPointer,"ProcessorType");
      gSettings.CpuType = GetAdvancedCpuType();
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.CpuType = (UINT16)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.CpuType = (UINT16)StrHexToUint64((CHAR16*)&UStr[0]);
        }
        DBG("Config set CpuType=%x\n", gSettings.CpuType);
      }
      
      prop = GetProperty(dictPointer,"BusSpeedkHz");
      if(prop) {
        if (prop->type == kTagTypeInteger) {
          gSettings.BusSpeed = (UINT32)(UINTN)prop->string;
        } else if (prop->type == kTagTypeString){
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.BusSpeed = (UINT32)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
        DBG("Config set BusSpeed=%dkHz\n", gSettings.BusSpeed);
      }      
    }

    //Disable drivers - move to early settings
/*    dictPointer = GetProperty(dict,"DisableDrivers");
    if(dictPointer) {
      INTN i = 0;
      do {
        CHAR8* PB = PossibleBlackList[i++];
        if (AsciiStrLen(PB) < 2) break;
        prop = GetProperty(dictPointer, PB);
        if (prop) {
          if ((prop->type == kTagTypeTrue) ||
              ((prop->type == kTagTypeString) &&
               ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))) {
            gSettings.BlackList[gSettings.BlackListCount++] = PoolPrint(L"%a", PB);
            if (gSettings.BlackListCount > 32) {
              break;
            }
          }
        }
      } while (1);
    }
*/
    // KernelAndKextPatches
    gSettings.KPKernelCpu = TRUE; // enabled by default
    gSettings.KPKextPatchesNeeded = FALSE;
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
        gSettings.KPKernelCpu = FALSE;
        if ((prop->type == kTagTypeTrue) ||
            ((prop->type == kTagTypeString) &&
             ((prop->string[0] == 'y') || (prop->string[0] == 'Y')))){
          gSettings.KPKernelCpu = TRUE;
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
        
            if (gSettings.KextPatches[gSettings.NrKexts].DataLen != j || j == 0) {
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
          AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
          gSettings.LogLineCount = (UINT32)StrDecimalToUintn((CHAR16*)&UStr[0]);
        }
        DBG("Log line count=%d\n", gSettings.LogLineCount);
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
      DBG("Warning! Your MLB is not suitable for iMessage!\n");
    }
    
    // if CustomUUID and InjectSystemID are not specified
    // then use InjectSystemID=TRUE and SMBIOS UUID
    // to get Chameleon's default behaviour (to make user's life easier)
    CopyMem((VOID*)&gUuid, (VOID*)&gSettings.SmUUID, sizeof(EFI_GUID));
    gSettings.InjectSystemID = TRUE;
    
    // SystemParameters again - values that can depend on previous params
    dictPointer = GetProperty(dict, "SystemParameters");
    if (dictPointer) {

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

EFI_STATUS GetOSVersion(IN REFIT_VOLUME *Volume)
{
	EFI_STATUS				Status = EFI_NOT_FOUND;
	CHAR8*						plistBuffer = 0;
	UINTN             plistLen;
	TagPtr						dict  = NULL;
	TagPtr						prop  = NULL;
  CHAR16*     SystemPlist = L"System\\Library\\CoreServices\\SystemVersion.plist";
  CHAR16*     ServerPlist = L"System\\Library\\CoreServices\\ServerVersion.plist";
  CHAR16*     RecoveryPlist = L"\\com.apple.recovery.boot\\SystemVersion.plist";
  CHAR16*     InstallLionPlist = L"\\Mac OS X Install Data\\com.apple.Boot.plist";
  CHAR16*     InstallMountainPlist = L"\\OS X Install Data\\com.apple.Boot.plist";
  
  if (!Volume) {
    return EFI_NOT_FOUND;
  }
  
	/* Mac OS X */ 
	if(FileExists(Volume->RootDir, SystemPlist)) 
	{
		Status = egLoadFile(Volume->RootDir, SystemPlist, (UINT8 **)&plistBuffer, &plistLen);
	}
	/* Mac OS X Server */
	else if(FileExists(Volume->RootDir, ServerPlist))
	{
		Status = egLoadFile(Volume->RootDir, ServerPlist, (UINT8 **)&plistBuffer, &plistLen);
	}
	else if(FileExists(Volume->RootDir, RecoveryPlist))
	{
		Status = egLoadFile(Volume->RootDir, RecoveryPlist, (UINT8 **)&plistBuffer, &plistLen);
	}
	/* Mac OS X Lion Installer */
  else if(FileExists(Volume->RootDir, InstallLionPlist))
	{
		Volume->OSType = OSTYPE_LION;
		Volume->OSIconName = L"mac";
    Volume->BootType = BOOTING_BY_EFI;
    Volume->OSName = L"Install Lion";
    return EFI_SUCCESS;
	}
	/* Mac OS X Mountain Lion Installer */
  else if(FileExists(Volume->RootDir, InstallMountainPlist))
	{
		Volume->OSType = OSTYPE_COUGAR;
		Volume->OSIconName = L"mac";
    Volume->BootType = BOOTING_BY_EFI;
    Volume->OSName = L"Install ML";
    return EFI_SUCCESS;
	}
  
	if(!EFI_ERROR(Status))
	{
		if(ParseXML(plistBuffer, &dict) != EFI_SUCCESS)
		{
			FreePool(plistBuffer);
			return EFI_NOT_FOUND;
		}
    
		prop = GetProperty(dict, "ProductVersion");
		if(prop != NULL)
		{
		    OSVersion = AllocateCopyPool(AsciiStrSize(prop->string), prop->string);

			// Tiger
			if(AsciiStrStr(prop->string, "10.4") != 0){
        Volume->OSType = OSTYPE_TIGER;
        Volume->OSIconName = L"tiger";
        Volume->BootType = BOOTING_BY_EFI;
        Volume->OSName = L"Tiger";
        Status = EFI_SUCCESS;
      } else
			// Leopard
      if(AsciiStrStr(prop->string, "10.5") != 0){
				Volume->OSType = OSTYPE_LEO;
        Volume->OSIconName = L"leo";
        Volume->BootType = BOOTING_BY_EFI;
        Volume->OSName = L"Leo";
        Status = EFI_SUCCESS;
      } else
			// Snow Leopard
			if(AsciiStrStr(prop->string, "10.6") != 0){
				Volume->OSType = OSTYPE_SNOW;
        Volume->OSIconName = L"snow";
        Volume->BootType = BOOTING_BY_EFI;
        Volume->OSName = L"Snow";
        Status = EFI_SUCCESS;
      } else
			// Lion
			if(AsciiStrStr(prop->string, "10.7") != 0){
				Volume->OSType = OSTYPE_LION;
        Volume->OSIconName = L"lion";
        Volume->BootType = BOOTING_BY_EFI;
        Volume->OSName = L"Lion";
        Status = EFI_SUCCESS;
      } else
      // Mountain Lion
      if(AsciiStrStr(prop->string, "10.8") != 0){
				Volume->OSType = OSTYPE_COUGAR;
        Volume->OSIconName = L"cougar";
        Volume->BootType = BOOTING_BY_EFI;
        Volume->OSName = L"ML";
        Status = EFI_SUCCESS;
      } else
        // Lynx
        if(AsciiStrStr(prop->string, "10.9") != 0){
          Volume->OSType = OSTYPE_LYNX;
          Volume->OSIconName = L"lynx";
          Volume->BootType = BOOTING_BY_EFI;
          Volume->OSName = L"Lynx";
          Status = EFI_SUCCESS;
        }
      MsgLog("  Booting OS %a\n", prop->string);
    } 
	}
	
	return Status;
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
		if(ParseXML(plistBuffer, &dict) != EFI_SUCCESS)
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


EFI_STATUS GetEdid(VOID)
{
	EFI_STATUS						Status;
	UINTN i, j;
  UINTN N;
  gEDID = NULL;
  
	Status = gBS->LocateProtocol (&gEfiEdidDiscoveredProtocolGuid, NULL, (VOID **)&EdidDiscovered);
	
	if (!EFI_ERROR (Status)) 
	{
		N = EdidDiscovered->SizeOfEdid;
    MsgLog("EdidDiscovered size=%d\n", N);
		if (N == 0) {
			return EFI_NOT_FOUND;
		}
    gEDID = AllocateAlignedPages(EFI_SIZE_TO_PAGES(N), 128);
    if (!gSettings.CustomEDID) {
      gSettings.CustomEDID = gEDID; //copy pointer but data if no CustomEDID
    }
    CopyMem(gEDID, EdidDiscovered->Edid, N);
    if (!GlobalConfig.SystemLog) {
      for (i=0; i<N; i+=10) {
        MsgLog("%02d | ", i);
        for (j=0; j<10; j++) {
          MsgLog("%02x ", EdidDiscovered->Edid[i+j]);
        }
        MsgLog("\n");
      }
    }
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
  radeon_card_info_t *info;

  NGFX = 0;
  Arpt.Valid = FALSE;
  
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
          switch (Pci.Hdr.VendorId) {
            case 0x1002:
              info = NULL;
              gGraphics[NGFX].Vendor = Ati;
              for (i = 0; radeon_cards[i].device_id ; i++)
              {
                if (radeon_cards[i].device_id == Pci.Hdr.DeviceId)
                {
                  info = &radeon_cards[i];
                  break;
                }
              }
              AsciiSPrint(gGraphics[NGFX].Model, 64, "%a", info->model_name);
              AsciiSPrint(gGraphics[NGFX].Config, 64, "%a", card_configs[info->cfg_name].name);
              gGraphics[NGFX].Ports = card_configs[info->cfg_name].ports;
              break;
            case 0x8086:
              gGraphics[NGFX].Vendor = Intel;
              AsciiSPrint(gGraphics[NGFX].Model, 64, "%a", get_gma_model(Pci.Hdr.DeviceId));
              DBG("Found GFX model=%a\n", gGraphics[NGFX].Model);
              gGraphics[NGFX].Ports = 1;
              break;
            case 0x10de:
              gGraphics[NGFX].Vendor = Nvidia;
              AsciiSPrint(gGraphics[NGFX].Model, 64, "%a",
                          get_nvidia_model(((Pci.Hdr.VendorId <<16) | Pci.Hdr.DeviceId),
                                           ((Pci.Device.SubsystemVendorID << 16) | Pci.Device.SubsystemID)));
              DBG("Found NVidia model=%a\n", gGraphics[NGFX].Model);
              gGraphics[NGFX].Ports = 2;
              break;
            default:
              break;
          }                

          NGFX++;
        }   //if gfx    
        else if((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_OTHER))
        {
 //         DBG("Found AirPort. Landing enabled...\n");
          Arpt.SegmentGroupNum = (UINT16)Segment;
          Arpt.BusNum = (UINT8)Bus;
          Arpt.DevFuncNum = (UINT8)((Device << 4) | (Function & 0x0F));
          Arpt.Valid = TRUE;
        }
      }
    }
  }
}  

VOID SetDevices(VOID)
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
  
  GetEdid();
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
        if (gSettings.GraphicsInjector &&
            (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
            (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA)) {
          
          //          gGraphics.DeviceID = Pci.Hdr.DeviceId;
          
          switch (Pci.Hdr.VendorId) {
            case 0x1002:
              //             gGraphics.Vendor = Ati;
              //              MsgLog("ATI GFX found\n");
              //can't do in one step because of C-conventions
              TmpDirty = setup_ati_devprop(&PCIdevice);
              StringDirty |=  TmpDirty;
              
              break;
            case 0x8086:
              
              TmpDirty = setup_gma_devprop(&PCIdevice);
              StringDirty |=  TmpDirty;
              //              MsgLog("Intel GFX device_id =0x%x\n", PCIdevice.device_id);
              MsgLog("Intel GFX revision  =0x%x\n", PCIdevice.revision);
              break;
            case 0x10de:
              //              gGraphics.Vendor = Nvidia;
              //              MsgLog("nVidia GFX found\n");
              TmpDirty = setup_nvidia_devprop(&PCIdevice);
              StringDirty |=  TmpDirty;
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
          //             MsgLog("USB device found\n");
          //set properties if no DSDT patch
          if (!(gSettings.FixDsdt & FIX_USB)) {
            if (gSettings.USBInjection) {
              TmpDirty = set_usb_props(&PCIdevice);
              StringDirty |=  TmpDirty;
            }
          }
        }
        
        // HDA
        else if (gSettings.HDAInjection &&
                 (Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA)) {
          //							MsgLog("HDA device found\n");
          TmpDirty = set_hda_props(PciIo, &PCIdevice);
          StringDirty |=  TmpDirty;
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

EFI_STATUS ApplySettings()
{
  UINT64  msr;

  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
     if (gCPUStructure.Turbo) {
        // Read in msr for turbo and test whether it needs disabled/enabled
        msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
        if (gSettings.Turbo) { // != ((msr & (1ULL<<38)) == 0)) {
          // Don't change cpu speed because we aren't changing control state
           if (gCPUStructure.Turbo4) {
             gCPUStructure.MaxSpeed = (UINT32)DivU64x32(gCPUStructure.CPUFrequency, Mega);
           }
           //
          //attempt to make turbo
      //    msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
          DBG("MSR_IA32_MISC_ENABLE = %lx\n", msr);
          msr &= ~(1ULL<<38);
       //   if (!gSettings.Turbo) msr |= (1ULL<<38); //0x4000000000 == 0 if Turbo enabled
          AsmWriteMsr64(MSR_IA32_MISC_ENABLE, msr);
          gBS->Stall(100);
          msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
          DBG("Set turbo: MSR_IA32_MISC_ENABLE = %lx\n", msr);
          // Don't set performance control state, let OS handle it - apianti
        }
     }
     //Slice: I disable this until to be clear why it should be disabled any way
     // moreover ISS is not EIST, I may enable or not ISS but I always want EIST.
   /*  if (gSettings.EnableISS != ((msr & (1ULL<<16)) != 0)){
      //attempt to speedstep
      msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
      DBG("MSR_IA32_MISC_ENABLE = %lx\n", msr);
      msr &= ~(1ULL<<16);
      if (gSettings.EnableISS) msr |= (1ULL<<16);
      AsmWriteMsr64(MSR_IA32_MISC_ENABLE, msr);
      gBS->Stall(100);
      msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
      DBG("Set speedstep: MSR_IA32_MISC_ENABLE = %lx\n", msr);
      }
   */
  }
  return EFI_SUCCESS;
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
/*
  MsgLog("Finally: Bus=%ldMHz CPU=%ldMHz\n",
         DivU64x32(gCPUStructure.FSBFrequency, Mega),
         gCPUStructure.MaxSpeed);
*/  
  return EFI_SUCCESS;
}

//dmazar
CHAR16* GetExtraKextsDir(REFIT_VOLUME *Volume)
{
  CHAR16                      *OSTypeStr = NULL;
  CHAR16                      *SrcDir = NULL;
  
  // get os version as string
  switch (Volume->OSType) {
    case OSTYPE_TIGER:
      OSTypeStr = L"10.4";
      break;
      
    case OSTYPE_LEO:
      OSTypeStr = L"10.5";
      break;
      
    case OSTYPE_SNOW:
      OSTypeStr = L"10.6";
      break;
      
    case OSTYPE_LION:
      OSTypeStr = L"10.7";
      break;
      
    case OSTYPE_COUGAR:
      OSTypeStr = L"10.8";
      break;
      
    case OSTYPE_LYNX:
      OSTypeStr = L"10.9";
      break;

    default:
      OSTypeStr = L"Other";
      break;
  }
  //MsgLog("OS=%s\n", OSTypeStr);
  
  // find source injection folder with kexts
  // note: we are just checking for existance of particular folder, not checking if it is empty or not
  // check OEM subfolders: version speciffic or default to Other
  SrcDir = PoolPrint(L"%s\\kexts\\%s", OEMPath, OSTypeStr);
  if (!FileExists(SelfVolume->RootDir, SrcDir)) {
    FreePool(SrcDir);
    SrcDir = PoolPrint(L"%s\\kexts\\Other", OEMPath);
    if (!FileExists(SelfVolume->RootDir, SrcDir)) {
      FreePool(SrcDir);
      SrcDir = NULL;
    }
  }
  if (SrcDir == NULL) {
    // if not found, check EFI\kexts\...
    SrcDir = PoolPrint(L"\\EFI\\CLOVER\\kexts\\%s", OSTypeStr);
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
    
    MsgLog("FSInjection:");
    
    Volume = Entry->Volume;
    
    // some checks?
    if (Volume->BootType != BOOTING_BY_EFI) {
        MsgLog("not started - not an EFI boot\n");
        return EFI_UNSUPPORTED;
    }
    
    // get FSINJECTION_PROTOCOL
    Status = gBS->LocateProtocol(&gFSInjectProtocolGuid, NULL, (void **)&FSInject);
    if (EFI_ERROR(Status)) {
        //Print(L"- No FSINJECTION_PROTOCOL, Status = %r\n", Status);
        MsgLog(" not started - gFSInjectProtocolGuid not found\n");
        return EFI_NOT_STARTED;
    }
    
    // check if blocking of caches is needed
    if (Entry->LoadOptions != NULL && StrStr(Entry->LoadOptions, L"NoCaches") != NULL) {
        MsgLog(" blocking caches");
        BlockCaches = TRUE;
        // add caches to blacklist
        Blacklist = FSInject->CreateStringList();
        if (Blacklist == NULL) {
            MsgLog(" - not enough memory!\n");
            return EFI_NOT_STARTED;
        }
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\kernelcache");
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\Extensions.mkext");
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Extensions.mkext");
        FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\kernelcache");
        FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\Extensions.mkext");
    }
    
    // check if kext injection is needed
    // (will be done only if caches are blocked or if boot.efi refuses to load kernelcache)
    SrcDir = NULL;
    if (Entry->LoadOptions != NULL && StrStr(Entry->LoadOptions, L"WithKexts") != NULL) {
        SrcDir = GetExtraKextsDir(Volume);
        if (SrcDir != NULL) {
          // we have found it - injection will be done
          MsgLog(", injecting kexts from: '%s'", SrcDir);
          InjectionNeeded = TRUE;
        } else {
          MsgLog(", skipping kext injection (kexts folder not found)\n");
        }
    } else {
        MsgLog(", skipping kext injection (not requested)\n");
    }
    
    // prepare list of kext that will be forced to load
    ForceLoadKexts = FSInject->CreateStringList();
    if (ForceLoadKexts == NULL) {
        MsgLog(" - not enough memory!\n");
        return EFI_NOT_STARTED;
    }
    KextPatcherRegisterKexts(FSInject, ForceLoadKexts);

    Status = FSInject->Install(Volume->DeviceHandle, L"\\System\\Library\\Extensions",
                               SelfVolume->DeviceHandle, SrcDir,
                               Blacklist, ForceLoadKexts);
    
    if (SrcDir != NULL) FreePool(SrcDir);
    
    if (EFI_ERROR(Status)) {
        MsgLog(" - not done - could not install injection!\n");
        return EFI_NOT_STARTED;
    }
    
    // reinit Volume->RootDir? it seems it's not needed.
    
    MsgLog(" - done!\n");
	return Status;
}

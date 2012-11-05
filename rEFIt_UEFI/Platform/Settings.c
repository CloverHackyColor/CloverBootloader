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
BOOLEAN                         gFirmwarePhoenix = FALSE;
UINTN                           gEvent;
UINT16                          gBacklightLevel;



VOID WaitForSts(VOID) {
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

EFI_STATUS GetUserSettings(IN EFI_FILE *RootDir)
{
  EFI_STATUS	Status = EFI_NOT_FOUND;
  UINTN       size;
  TagPtr      dict, dict2;
  TagPtr      prop;
  TagPtr      dictPointer;
  CHAR8*      gConfigPtr = NULL;
  UINTN       i;
  CHAR8       ANum[4];

  
  CHAR16      UStr[64];
  CHAR16*     ConfigPlistPath = L"EFI\\config.plist";
  CHAR16*     ConfigOemPath = PoolPrint(L"%s\\config.plist", OEMPath);
  
  // load config
  if (FileExists(SelfRootDir, ConfigOemPath)) {
    Status = egLoadFile(SelfRootDir, ConfigOemPath, (UINT8**)&gConfigPtr, &size);
  } else {
    DBG("Oem config.plist not found at path: %s\n", ConfigOemPath);
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
  
  
  if(EFI_ERROR(Status)) {
    DBG("Error loading config.plist! Status=%r\n", Status);
    return Status;
  }
  if(gConfigPtr) {
    if(ParseXML((const CHAR8*)gConfigPtr, &dict) != EFI_SUCCESS) {
      DBG(" config error\n");
      return EFI_UNSUPPORTED;
    }
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
        } else if (AsciiStrStr(prop->string, "it")) {
          gLanguage = italian;
        } else if (AsciiStrStr(prop->string, "es")) {
          gLanguage = spanish;
        } else if (AsciiStrStr(prop->string, "pt")) {
          gLanguage = portuguese; 
        } else if (AsciiStrStr(prop->string, "pl")) {
          gLanguage = polish; 
        } else if (AsciiStrStr(prop->string, "ge")) {
          gLanguage = german;
        } else if (AsciiStrStr(prop->string, "id")) {
          gLanguage = indonesian; 
        } else if (AsciiStrStr(prop->string, "fr")) {
          gLanguage = french;
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
      prop = GetProperty(dictPointer, "CustomUUID");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, gSettings.CustomUuid);
        Status = StrToGuidLE(gSettings.CustomUuid, &gUuid);
        //else value from SMBIOS
      }  
      prop = GetProperty(dictPointer, "InjectSystemID");
      gSettings.InjectSystemID = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.InjectSystemID = TRUE;
      }
      
      prop = GetProperty(dictPointer, "LegacyBoot");
      if(prop)  {
        AsciiStrToUnicodeStr(prop->string, gSettings.LegacyBoot);
      }
      //BacklightLevel
      prop = GetProperty(dictPointer, "BacklightLevel");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.BacklightLevel = (UINT16)StrHexToUint64((CHAR16*)&UStr[0]);	
      }
      
      // iCloudFix
      gSettings.iCloudFix = TRUE;
      prop = GetProperty(dictPointer, "iCloudFix");
      if(prop) {
        if ((prop->string[0] == 'n') || (prop->string[0] == 'N'))
          gSettings.iCloudFix = FALSE;
      }      
    }

    dictPointer = GetProperty(dict, "Pointer");
    if (dictPointer) {
      prop = GetProperty(dictPointer, "Speed");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.PointerSpeed = StrDecimalToUintn((CHAR16*)&UStr[0]);
      }
      prop = GetProperty(dictPointer, "DoubleClickTime");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.DoubleClickTime = StrDecimalToUintn((CHAR16*)&UStr[0]);
      }
      
    }
    
    //Graphics
    
    dictPointer = GetProperty(dict, "Graphics");
    if (dictPointer) {
      prop = GetProperty(dictPointer, "GraphicsInjector");
      if(prop) {
        if ((prop->string[0] == 'n') || (prop->string[0] == 'N'))
          gSettings.GraphicsInjector = FALSE;
        else if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.GraphicsInjector = TRUE;
      }      
      prop = GetProperty(dictPointer, "VRAM");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.VRAM = (UINT64)StrDecimalToUintn((CHAR16*)&UStr[0]) << 20;  //bytes
      }
      
      prop = GetProperty(dictPointer, "LoadVBios");
      gSettings.LoadVBios = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.LoadVBios = TRUE;
      }
      for (i=0; i<NGFX; i++) {
        gGraphics[i].LoadVBios = gSettings.LoadVBios; //default
      }
      //InjectEDID
      prop = GetProperty(dictPointer, "InjectEDID");
      gSettings.InjectEDID = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.InjectEDID = TRUE;
      }
      prop = GetProperty(dictPointer, "CustomEDID");
      if(prop) {  
        UINTN j = 128;
        gSettings.CustomEDID = GetDataSetting(dictPointer, "CustomEDID", &j);
        if (j != 128) {
          DBG("CustomEDID has wrong length=%d\n", j);
        }
      }      
      
      prop = GetProperty(dictPointer, "PatchVBios");
      gSettings.PatchVBios = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.PatchVBios = TRUE;
      }
      prop = GetProperty(dictPointer, "VideoPorts");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.VideoPorts = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
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
    }    
    
    dictPointer = GetProperty(dict, "PCI");
    if (dictPointer) {      
      prop = GetProperty(dictPointer, "PCIRootUID");
      gSettings.PCIRootUID = 0;
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.PCIRootUID = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
      }
      prop = GetProperty(dictPointer, "StringInjector");
      gSettings.StringInjector = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
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
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.LpcTune = TRUE;
      }
      // HDA
      prop = GetProperty(dictPointer, "HDAInjection");
      if(prop) {
        // enabled by default
        // syntax:
        // - HDAInjection=No - disables injection
        // - HDAInjection=887 - injects layout-id 887 decimal (0x00000377)
        // - HDAInjection=0x377 - injects layout-id 887 decimal (0x00000377)
        // - HDAInjection=Detect - reads codec device id (eg. 0x0887)
        //   converts it to decimal 887 and injects this as layout-id.
        //   if hex device is cannot be converted to decimal, injects legacy value 12 decimal
        // - all other values are equal to HDAInjection=Detect
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
      // USB
      prop = GetProperty(dictPointer, "USBInjection");
      if(prop) {
        // enabled by default
        // syntax: USBInjection=Yes/No
        if ((prop->string[0] == 'n') || (prop->string[0] == 'N')) {
          gSettings.USBInjection = FALSE;
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
      
      prop = GetProperty(dictPointer, "DropOemSSDT");
      gSettings.DropSSDT = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.DropSSDT = TRUE;
      }
      prop = GetProperty(dictPointer, "GeneratePStates");
      gSettings.GeneratePStates = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.GeneratePStates = TRUE;
      }
      prop = GetProperty(dictPointer, "GenerateCStates");
      gSettings.GenerateCStates = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.GenerateCStates = TRUE;
      }
      gSettings.PLimitDict = 0;
      prop = GetProperty(dictPointer, "PLimitDict");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.PLimitDict = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);	
      }
      gSettings.UnderVoltStep = 0;
      prop = GetProperty(dictPointer, "UnderVoltStep");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.UnderVoltStep = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);	
      }
      
      prop = GetProperty(dictPointer, "ResetAddress");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.ResetAddr  = StrHexToUint64(UStr); 
      }
      prop = GetProperty(dictPointer, "ResetValue");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.ResetVal = (UINT8)StrHexToUint64((CHAR16*)&UStr[0]);	
      }
      //other known pair is 0x0CF9/0x06. What about 0x92/0x01 ?
      
      prop = GetProperty(dictPointer, "EnableC6");
      gSettings.EnableC6 = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.EnableC6 = TRUE;
      }
      
      prop = GetProperty(dictPointer, "EnableC4");
      gSettings.EnableC4 = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.EnableC4 = TRUE;
      }
      
      prop = GetProperty(dictPointer, "EnableC2");
      gSettings.EnableC2 = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.EnableC2 = TRUE;
          DBG(" C2 enabled\n");
        }
      }
      gSettings.C3Latency = 0; //Usually it is 0x03e9, but if you want Turbo, you may set 0x00FA
      prop = GetProperty(dictPointer, "C3Latency");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.C3Latency  = (UINT16)StrHexToUint64(UStr); 
      }
      
      
      prop = GetProperty(dictPointer, "EnableISS");
//      gSettings.EnableISS = FALSE; //we set default value in GetDefaultSettings()
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')) {
          gSettings.EnableISS = TRUE;
        } else if ((prop->string[0] == 'n') || (prop->string[0] == 'N')) {
          gSettings.EnableISS = FALSE;  //force disable
        }
      }      
      prop = GetProperty(dictPointer, "smartUPS");
      gSettings.smartUPS = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.smartUPS = TRUE;
          DBG("Config set smartUPS present\n");
        }
      }
      prop = GetProperty(dictPointer, "PatchAPIC");
      gSettings.PatchNMI = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.PatchNMI = TRUE;
      }
      prop = GetProperty(dictPointer, "FixDsdtMask");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.FixDsdt  = (UINT32)StrHexToUint64(UStr); 
      }
      prop = GetProperty(dictPointer, "DropAPIC");
      gSettings.bDropAPIC = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.bDropAPIC = TRUE;
      }
      prop = GetProperty(dictPointer, "DropMCFG");
      gSettings.bDropMCFG = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.bDropMCFG = TRUE;
      }
      prop = GetProperty(dictPointer, "DropHPET");
      gSettings.bDropHPET = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.bDropHPET = TRUE;
      }
      prop = GetProperty(dictPointer, "DropECDT");
      gSettings.bDropECDT = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.bDropECDT = TRUE;
      }
      prop = GetProperty(dictPointer, "RememberBIOS");
      gSettings.RememberBIOS = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
          gSettings.RememberBIOS = TRUE;
      }
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
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        Status = StrToGuidLE((CHAR16*)&UStr[0], &gSettings.SmUUID);
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
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.BoardType = (UINT8)StrDecimalToUintn((CHAR16*)&UStr[0]);
      }
      
      prop = GetProperty(dictPointer,"Mobile");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y'))
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
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.ChassisType = (UINT8)StrHexToUint64((CHAR16*)&UStr[0]);
        DBG("Config set ChassisType=0x%x\n", gSettings.ChassisType);
      }
     //gFwFeatures = 0xC0001403 - by default
      prop = GetProperty(dictPointer, "FirmwareFeatures");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gFwFeatures = (UINT32)StrHexToUint64((CHAR16*)&UStr[0]);
      }
    }
    
    //CPU
    dictPointer = GetProperty(dict,"CPU");
    if (dictPointer) {
      prop = GetProperty(dictPointer,"Turbo");
 //     gSettings.Turbo = FALSE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.Turbo = TRUE;
          DBG("Config set Turbo\n");
        } else if ((prop->string[0] == 'n') || (prop->string[0] == 'N')) {
          gSettings.Turbo = FALSE; //force disable
        }
      }
      prop = GetProperty(dictPointer,"QPI");
      gSettings.QPI = (UINT16)gCPUStructure.ProcessorInterconnectSpeed; //MHz
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.QPI = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
        DBG("Config set QPI=%dMHz\n", gSettings.QPI);
      }
      prop = GetProperty(dictPointer,"CpuFrequencyMHz");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.CpuFreqMHz = (UINT16)StrDecimalToUintn((CHAR16*)&UStr[0]);
        DBG("Config set CpuFreq=%dMHz\n", gSettings.CpuFreqMHz);
      }
      prop = GetProperty(dictPointer,"ProcessorType");
      gSettings.CpuType = GetAdvancedCpuType();
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.CpuType = (UINT16)StrHexToUint64((CHAR16*)&UStr[0]);
        DBG("Config set CpuType=%x\n", gSettings.CpuType);
      }
      
      prop = GetProperty(dictPointer,"BusSpeedkHz");
      if(prop) {
        AsciiStrToUnicodeStr(prop->string, (CHAR16*)&UStr[0]);
        gSettings.BusSpeed = (UINT32)StrDecimalToUintn((CHAR16*)&UStr[0]);
        DBG("Config set BusSpeed=%dkHz\n", gSettings.BusSpeed);
      }      
    }
    
    // KernelAndKextPatches
    gSettings.KPKernelCpu = TRUE; // enabled by default
    gSettings.KPKextPatchesNeeded = FALSE;
    dictPointer = GetProperty(dict,"KernelAndKextPatches");
    if (dictPointer) {
      gSettings.KPDebug = FALSE;
      prop = GetProperty(dictPointer,"Debug");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.KPDebug = TRUE;
        }
      }
      prop = GetProperty(dictPointer,"KernelCpu");
      if(prop) {
        gSettings.KPKernelCpu = FALSE;
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.KPKernelCpu = TRUE;
        }
      }
      prop = GetProperty(dictPointer,"ATIConnectorsController");
      if(prop) {
        UINTN len = 0;
        // ATIConnectors patch
        gSettings.KPATIConnectorsController = AllocateZeroPool((AsciiStrLen(prop->string) + 1) * sizeof(CHAR16));
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
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.KPAsusAICPUPM = TRUE;
        }
      }
      gSettings.KPKextPatchesNeeded |= gSettings.KPAsusAICPUPM;
      
      prop = GetProperty(dictPointer,"AppleRTC");
      gSettings.KPAppleRTC = TRUE;
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.KPAppleRTC = TRUE;
          gSettings.KPKextPatchesNeeded = TRUE;
        } else if ((prop->string[0] == 'n') || (prop->string[0] == 'N')){
          gSettings.KPAppleRTC = FALSE;
        }
      }
      gSettings.KPKextPatchesNeeded |= gSettings.KPAppleRTC;
      
      prop = GetProperty(dictPointer,"KextsToPatch");
      if(prop) {
        UINTN  j;
        i = 0;
        do {
          AsciiSPrint(ANum, 4, "%d", i);
          dictPointer = GetProperty(prop, ANum);
          if (!dictPointer) {
            break;
          }
          dict2 = GetProperty(dictPointer,"Name");
          if (dict2) {
            gSettings.AnyKext[i] = AllocateZeroPool(256); //dunno about size of kext name
            AsciiSPrint(gSettings.AnyKext[i], 256, "%a", dict2->string);
            DBG("Prepare to patch of %a\n", gSettings.AnyKext[i]);
          }
          gSettings.KPKextPatchesNeeded = TRUE;
          gSettings.AnyKextData[i] = GetDataSetting(dictPointer,"Find",
                                                    &gSettings.AnyKextDataLen[i]);
          gSettings.AnyKextPatch[i] = GetDataSetting(dictPointer,"Replace", &j);
          if (gSettings.AnyKextDataLen[i] != j) {
            DBG("wrong data to patch kext %a\n", gSettings.AnyKext[i]);
            gSettings.AnyKext[i][0] = 0; //just erase name
            continue; //same i
          }
          DBG("... data length=%d\n", gSettings.AnyKextDataLen[i]);
          i++;
          if (i>99) {
            DBG("too many kexts to patch\n");
            break;
          }
        } while (TRUE);
        gSettings.NrKexts = (INT32)i;
        //there is one moment. This data is allocated in BS memory but will be used 
        // after OnExitBootServices. This is wrong and these arrays should be reallocated
        // but I am not sure
      }
    }
 //Volumes hiding   
    dictPointer = GetProperty(dict,"Volumes");
    if (dictPointer) {
      gSettings.HVHideAllOSX = FALSE;
      prop = GetProperty(dictPointer,"HideAllOSX");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllOSX = TRUE;
        }
      }
      gSettings.HVHideAllOSXInstall = FALSE;
      prop = GetProperty(dictPointer,"HideAllOSXInstall");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllOSXInstall = TRUE;
        }
      }
      gSettings.HVHideAllRecovery = FALSE;
      prop = GetProperty(dictPointer,"HideAllRecovery");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllRecovery = TRUE;
        }
      }
      gSettings.HVHideAllWindowsEFI = FALSE;
      prop = GetProperty(dictPointer,"HideAllWindowsEFI");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllWindowsEFI = TRUE;
        }
      }
      gSettings.HVHideAllGrub = FALSE;
      prop = GetProperty(dictPointer,"HideAllGrub");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllGrub = TRUE;
        }
      }
      gSettings.HVHideAllGentoo = FALSE;
      prop = GetProperty(dictPointer,"HideAllGentoo");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllGentoo = TRUE;
        }
      }
      gSettings.HVHideAllRedHat = FALSE;
      prop = GetProperty(dictPointer,"HideAllRedHat");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllRedHat = TRUE;
        }
      }
      gSettings.HVHideAllUbuntu = FALSE;
      prop = GetProperty(dictPointer,"HideAllUbuntu");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllUbuntu = TRUE;
        }
      }
      gSettings.HVHideAllLinuxMint = FALSE;
      prop = GetProperty(dictPointer,"HideAllLinuxMint");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllLinuxMint = TRUE;
        }
      }
      gSettings.HVHideAllSuSe = FALSE;
      prop = GetProperty(dictPointer,"HideAllSuSe");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllSuSe = TRUE;
        }
      }
      gSettings.HVHideAllUEFI = FALSE;
      prop = GetProperty(dictPointer,"HideAllUEFI");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllUEFI = TRUE;
        }
      }
      gSettings.HVHideAllLegacy = FALSE;
      prop = GetProperty(dictPointer,"HideAllLegacy");
      if(prop) {
        if ((prop->string[0] == 'y') || (prop->string[0] == 'Y')){
          gSettings.HVHideAllLegacy = TRUE;
        }
      }
      
      prop = GetProperty(dictPointer,"HideVolumes");
      if(prop) {
        i = 0;
        do {
          AsciiSPrint(ANum, 4, "%d", i);
          dictPointer = GetProperty(prop, ANum);
          if (!dictPointer) {
            break;
          }
          dict2 = GetProperty(dictPointer,"VolumeString");
          if (dict2) {
            gSettings.HVHideStrings[i] = AllocateZeroPool(256);
            UnicodeSPrint(gSettings.HVHideStrings[i], 256, L"%a", dict2->string);
            DBG("Hiding Volume with string: %s\n", gSettings.HVHideStrings[i]);
          }
          i++;
          if (i>99) {
            break;
          }
        } while (TRUE); // What is this for?
        gSettings.HVCount = (INT32)i;
      }
    
    }
    
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
    return EFI_SUCCESS;
	}
	/* Mac OS X Mountain Lion Installer */
  else if(FileExists(Volume->RootDir, InstallMountainPlist))
	{
		Volume->OSType = OSTYPE_COUGAR;
		Volume->OSIconName = L"mac";
    Volume->BootType = BOOTING_BY_EFI;
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
		    OSVersion = AllocateZeroPool(AsciiStrLen(prop->string));
    		AsciiStrCpy(OSVersion, prop->string);

			// Tiger
			if(AsciiStrStr(prop->string, "10.4") != 0){
        Volume->OSType = OSTYPE_TIGER;
        Volume->OSIconName = L"tiger";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      } else
			// Leopard
      if(AsciiStrStr(prop->string, "10.5") != 0){
				Volume->OSType = OSTYPE_LEO;
        Volume->OSIconName = L"leo";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      } else
			// Snow Leopard
			if(AsciiStrStr(prop->string, "10.6") != 0){
				Volume->OSType = OSTYPE_SNOW;
        Volume->OSIconName = L"snow";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      } else
			// Lion
			if(AsciiStrStr(prop->string, "10.7") != 0){
				Volume->OSType = OSTYPE_LION;
        Volume->OSIconName = L"lion";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      } else
      // Mountain Lion
      if(AsciiStrStr(prop->string, "10.8") != 0){
				Volume->OSType = OSTYPE_COUGAR;
        Volume->OSIconName = L"cougar";
        Volume->BootType = BOOTING_BY_EFI;
        Status = EFI_SUCCESS;
      }
      MsgLog("Booting OS %a\n", prop->string);
    } 
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
		for (i=0; i<N; i+=10) {
			MsgLog("%02d | ", i);
			for (j=0; j<10; j++) {
				MsgLog("%02x ", EdidDiscovered->Edid[i+j]);
			}
			MsgLog("\n");		   
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
          DBG("Found AirPort. Landing enabled...\n");
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
      //       gCPUStructure.MaxSpeed = (UINT32)(DivU64x32(MultU64x64(gCPUStructure.FSBFrequency, gCPUStructure.Turbo4), Mega * 10)); 
           //gCPUStructure.CPUFrequency = DivU64x32(MultU64x64(gCPUStructure.Turbo4, gCPUStructure.FSBFrequency), 10);
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
          /* Don't set performance control state, let OS handle it - apianti
           if (TurboMsr != 0) {
           AsmWriteMsr64(MSR_IA32_PERF_CONTROL, TurboMsr);
           gBS->Stall(100);
           WaitForSts();
           }
           msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);
           DBG("set turbo state msr=%x \n", msr);
           tmpU = gCPUStructure.CPUFrequency; //
           DBG("set CPU to %ld\n", tmpU);
           tmpU = DivU64x32(gCPUStructure.CPUFrequency, Mega);
           DBG("... CPU to %ldMHz\n", tmpU);
           //    DBG("set turbo state msr=%x CPU=%dMHz\n", msr, (INT32)DivU64x32(gCPUStructure.CPUFrequency, Mega));
           AlreadyDone = TRUE;
           // */
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
  //TODO - SetVariable()..
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
  
  MsgLog("Finally: Bus=%ldMHz CPU=%ldMHz\n",
         DivU64x32(gCPUStructure.FSBFrequency, Mega),
         gCPUStructure.MaxSpeed);
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
      
    default:
      OSTypeStr = L"Other";
      break;
  }
  MsgLog("OS=%s ", OSTypeStr);
  
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
    SrcDir = PoolPrint(L"\\EFI\\kexts\\%s", OSTypeStr);
    if (!FileExists(SelfVolume->RootDir, SrcDir)) {
      FreePool(SrcDir);
      SrcDir = PoolPrint(L"\\EFI\\kexts\\Other", gSettings.OEMProduct);
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
    if (Volume->BootType != BOOTING_BY_EFI) {
        MsgLog("not started - not an EFI boot\n");
        return EFI_UNSUPPORTED;
    }
    
    if (Entry->LoadOptions == NULL || StrStr(Entry->LoadOptions, L"WithKexts") == NULL) {
        // FS injection not requested
        MsgLog("not requested\n");
        return EFI_NOT_STARTED;
    }
    
    // get FSINJECTION_PROTOCOL
    Status = gBS->LocateProtocol(&gFSInjectProtocolGuid, NULL, (void **)&FSInject);
    if (EFI_ERROR(Status)) {
        //Print(L"- No FSINJECTION_PROTOCOL, Status = %r\n", Status);
        MsgLog("not started - gFSInjectProtocolGuid not found\n");
        return EFI_NOT_STARTED;
    }
    
    if (StrStr(Entry->LoadOptions, L"WithKexts") != NULL) {
        
        SrcDir = GetExtraKextsDir(Volume);
        if (SrcDir != NULL) {
            // we have found it - injection will be done
            MsgLog("Injecting kexts from: '%s' ", SrcDir);
            BlockCaches = TRUE;
            InjectionNeeded = TRUE;
            
        } else {
            MsgLog("Skipping kext injection (kexts folder not found) ");
        }
    }
    
    if (!InjectionNeeded) {
        MsgLog("- not done!\n");
        return EFI_NOT_STARTED;
    }
    
    if (BlockCaches) {
        // add caches to blacklist
        Blacklist = FSInject->CreateStringList();
        if (Blacklist == NULL) {
            MsgLog("- not enough memory!\n");
            return EFI_NOT_STARTED;
        }
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\kernelcache");
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\Extensions.mkext");
        FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Extensions.mkext");
        FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\kernelcache");
        FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\Extensions.mkext");
        
    }
    
    // prepare list of kext that will be forced to load
    ForceLoadKexts = FSInject->CreateStringList();
    if (ForceLoadKexts == NULL) {
        MsgLog("- not enough memory!\n");
        return EFI_NOT_STARTED;
    }
    KextPatcherRegisterKexts(FSInject, ForceLoadKexts);

    Status = FSInject->Install(Volume->DeviceHandle, L"\\System\\Library\\Extensions",
                               SelfVolume->DeviceHandle, SrcDir,
                               Blacklist, ForceLoadKexts);
    
    if (SrcDir != NULL) FreePool(SrcDir);
    
    if (EFI_ERROR(Status)) {
        MsgLog("not done - could not install injection!\n");
        return EFI_NOT_STARTED;
    }
    
    // reinit Volume->RootDir? it seems it's not needed.
    
    MsgLog("done!\n");
	return Status;
}

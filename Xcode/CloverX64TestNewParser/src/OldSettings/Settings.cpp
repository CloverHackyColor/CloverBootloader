/*
 Slice 2012
 */

/*
 * Jief : this is from version cc3c8fe0a7bab653101244a661bb12636f4c54a1, 2021-04-23, just before the switch to new xml parser
 */
 
#include <Platform.h>
#include "Settings.h"
#include "../../../../rEFIt_UEFI/Platform/FixBiosDsdt.h"
#include "../../../../rEFIt_UEFI/Platform/../include/VolumeTypes.h"
#include "../../../../rEFIt_UEFI/Platform/../include/OSFlags.h"
#include "../../../../rEFIt_UEFI/Platform/../include/OSTypes.h"
#include "../../../../rEFIt_UEFI/Platform/../include/BootTypes.h"
#include "../../../../rEFIt_UEFI/Platform/../include/QuirksCodes.h"
#include "../../../../rEFIt_UEFI/Platform/../entry_scan/loader.h"
#include "../../../../rEFIt_UEFI/Platform/../Platform/BootLog.h"
#include "../../../../rEFIt_UEFI/Platform/../entry_scan/secureboot.h"
#include "../../../../rEFIt_UEFI/Platform/../libeg/XTheme.h"
#include "../../../../rEFIt_UEFI/Platform/cpu.h"
#include "../../../../rEFIt_UEFI/Platform/VersionString.h"
#include "../../../../rEFIt_UEFI/Platform/card_vlist.h"
#include "../../../../rEFIt_UEFI/Platform/Injectors.h"
#include "../../../../rEFIt_UEFI/Platform/../include/Pci.h"
#include "../../../../rEFIt_UEFI/Platform/../include/Devices.h"
#include "../../../../rEFIt_UEFI/Platform/smbios.h"
#include "../../../../rEFIt_UEFI/Platform/Nvram.h"
#include "../../../../rEFIt_UEFI/Platform/BootOptions.h"
#include "../../../../rEFIt_UEFI/Settings/SelfOem.h"
#include "../../../../rEFIt_UEFI/Platform/ati_reg.h"
#include "../../../../rEFIt_UEFI/Platform/ati.h"
#include "../../../../rEFIt_UEFI/Platform/nvidia.h"
#include "../../../../rEFIt_UEFI/Platform/gma.h"
#include "../../../../rEFIt_UEFI/Platform/Edid.h"
#include "../../../../rEFIt_UEFI/Platform/hda.h"
#include "../../../../rEFIt_UEFI/Platform/../../Version.h"
#include "../../../../rEFIt_UEFI/Platform/../entry_scan/bootscreen.h"

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





static EFI_STATUS
SaveSettings(SETTINGS_DATA& settingsData);

//
//ACPI_NAME_LIST *
//ParseACPIName(const XString8& String)
//{
//  ACPI_NAME_LIST* List = NULL;
//  ACPI_NAME_LIST* Next = NULL;
//  INTN i, j, Len, pos0, pos1;
//  Len = String.length();
//  //  DBG("parse ACPI name: %s\n", String);
//  if (Len > 0)   {
//    //Parse forward but put in stack LIFO "_SB.PCI0.RP02.PXSX"  -1,3,8,13,18
//    pos0 = -1;
//    while (pos0 < Len) {
//      List = (__typeof__(List))AllocateZeroPool(sizeof(ACPI_NAME_LIST));
//      List->Next = Next;
//      List->Name = (__typeof__(List->Name))AllocateZeroPool(5);
//      pos1 = pos0 + 1;
//      while ((pos1 < Len) && String[pos1] != '.') pos1++; // 3,8,13,18
//      //    if ((pos1 == Len) || (String[pos1] == ',')) { //always
//      for (i = pos0 + 1, j = 0; i < pos1; i++) {
//        List->Name[j++] = String.data()[i]; // String[i] return a char32_t. what if there is an utf8 char ?
//                                            // Jief : if it's an utf8 multibytes char, it'll be properly converted to the corresponding UTF32 char.
//                                            //        So this is an unsafe downcast !
//                                            //        Plus : this can write more than 5 bytes in List->Name !!
//      }
//      // extend by '_' up to 4 symbols
//      if (j < 4) {
//        SetMem(List->Name + j, 4 - j, '_');
//      }
//      List->Name[4] = '\0';
//      //    }
//      //      DBG("string between [%d,%d]: %s\n", pos0, pos1, List->Name);
//      pos0 = pos1; //comma or zero@end
//      Next = List;
//    }
//  }
//  return List;
//}




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
      size_t Len = (UINT32)Prop->getString()->stringValue().length() >> 1; // number of hex digits
      Data = (__typeof__(Data))AllocateZeroPool(Len); // 2 chars per byte, one more byte for odd number
      Len  = hex2bin(Prop->getString()->stringValue(), Data, Len);

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

//STATIC BOOLEAN AddCustomLoaderEntry(IN CUSTOM_LOADER_ENTRY *Entry)
//{
//  if (Entry == NULL) return FALSE;
//  settingsData.GUI.CustomEntries.AddReference(Entry, true);
//  return TRUE;
//}

//STATIC BOOLEAN AddCustomLegacyEntry (IN CUSTOM_LEGACY_ENTRY_SETTINGS *Entry)
//{
//  if (Entry == NULL) return FALSE;
//  settingsData.GUI.CustomLegacy.AddReference(Entry, true);
//  return TRUE;
//}
//STATIC
//BOOLEAN
//AddCustomToolEntry (
//                    IN CUSTOM_TOOL_ENTRY *Entry
//                    )
//{
//  if (Entry == NULL) return FALSE;
//  settingsData.GUI.CustomTool.AddReference(Entry, true);
//  return TRUE;
//}

//STATIC
//BOOLEAN
//AddCustomSubEntry (
//                   IN OUT  CUSTOM_LOADER_ENTRY *Entry,
//                   IN      CUSTOM_LOADER_ENTRY *SubEntry)
//{
//  if ((Entry == NULL) || (SubEntry == NULL)) return FALSE;
//  Entry->SubEntries.AddReference(Entry, true);
//  return TRUE;
//}

//
//STATIC
//CUSTOM_LOADER_SUBENTRY_SETTINGS
//*DuplicateCustomEntryToSubEntry (
//                       IN CUSTOM_LOADER_ENTRY_SETTINGS *Entry
//                       )
//{
//  if (Entry == NULL) {
//    return NULL;
//  }
//
//  CUSTOM_LOADER_SUBENTRY_SETTINGS* DuplicateEntry = new CUSTOM_LOADER_SUBENTRY_SETTINGS;
//  if (DuplicateEntry != NULL) {
////    DuplicateEntry->Volume           = Entry->Volume;          //ok
////    DuplicateEntry->Path             = Entry->Path;            //ok
////    DuplicateEntry->LoadOptions      = Entry->LoadOptions;
//    DuplicateEntry->FullTitle        = Entry->FullTitle;       //ok
//    DuplicateEntry->Title            = Entry->Title;           //ok
////    DuplicateEntry->ImagePath        = Entry->ImagePath;       //ok
////    DuplicateEntry->BootBgColor      = Entry->BootBgColor;     //ok
////    DuplicateEntry->Image            = Entry->Image;
////    DuplicateEntry->Hotkey           = Entry->Hotkey;          //ok
////    DuplicateEntry->Flags            = Entry->Flags;
////    DuplicateEntry->Type             = Entry->Type;            //ok
////    DuplicateEntry->VolumeType       = Entry->VolumeType;      //ok
////    DuplicateEntry->KernelScan       = Entry->KernelScan;      //ok
////    DuplicateEntry->CustomLogoType   = Entry->CustomLogoType;
////    DuplicateEntry->CustomLogoAsXString8 = Entry->CustomLogoAsXString8;  //ok
////    DuplicateEntry->CustomLogoAsData = Entry->CustomLogoAsData;          //ok
////    DuplicateEntry->CustomLogoImage  = Entry->CustomLogoImage;
////    DuplicateEntry->KernelAndKextPatches = Entry->KernelAndKextPatches;
//  }
//
//  return DuplicateEntry;
//}

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
    Patches->_KPKernelPm = IsPropertyNotNullAndTrue(Prop);
  }
  
  Prop = DictPointer->propertyForKey("PanicNoKextDump");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->KPPanicNoKextDump = IsPropertyNotNullAndTrue(Prop);
  }

  Prop = DictPointer->propertyForKey("AppleIntelCPUPM");
  if (Prop != NULL || GlobalConfig.gBootChanged) {
    Patches->_KPAppleIntelCPUPM = IsPropertyNotNullAndTrue(Prop);
  }
// //anyway
//  if (GlobalConfig.NeedPMfix) {
//    Patches->KPKernelPm = TRUE;
//    Patches->KPAppleIntelCPUPM = TRUE;
//  }

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
//  gRemapSmBiosIsRequire = Patches->KPDELLSMBIOS;

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
        || Patches->KPATIConnectorsData.size() != Patches->KPATIConnectorsPatch.size()) {
      // invalid params - no patching
      DBG("ATIConnectors patch: invalid parameters!\n");

      Patches->KPATIConnectorsController.setEmpty();
      Patches->KPATIConnectorsData.setEmpty();
      Patches->KPATIConnectorsPatch.setEmpty();
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

        if ( Prop2->getString()->stringValue().notEmpty() && Prop2->getString()->stringValue() != "\\"_XS8 ) {
          Patches->ForceKextsToLoad.Add(Prop2->getString()->stringValue());
          DBG(" - [%zu]: %ls\n", Patches->ForceKextsToLoad.size(), Patches->ForceKextsToLoad[Patches->ForceKextsToLoad.size()-1].wc_str());
        }
      }
    }
  }

  // KextsToPatch is an array of dict
  arrayProp = DictPointer->arrayPropertyForKey("KextsToPatch");
  if (arrayProp != NULL) {
    INTN Count = arrayProp->arrayContent().size();
    Patches->KextPatches.setEmpty();
    
    if (Count > 0) {
      const TagDict* Prop2 = NULL;
      const TagStruct* Dict = NULL;

      DBG("KextsToPatch: %lld requested\n", Count);
      for (INTN i = 0; i < Count; i++) {
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

        KEXT_PATCH* newKextPatchPtr = new KEXT_PATCH();
        KEXT_PATCH& newKextPatch = *newKextPatchPtr;
        
        newKextPatch.Name = Dict->getString()->stringValue();
        newKextPatch.Label.takeValueFrom(newKextPatch.Name);

        Dict = Prop2->propertyForKey("Comment");
        if (Dict != NULL) {
          newKextPatch.Label += " (";
          newKextPatch.Label += Dict->getString()->stringValue();
          newKextPatch.Label += ")";

        } else {
          newKextPatch.Label += " (NoLabel)";
        }
        DBG(" %s", newKextPatch.Label.c_str());

     //   newPatch.MenuItem.BValue     = TRUE;
        Dict = Prop2->propertyForKey("Disabled");
        newKextPatch.Disabled = IsPropertyNotNullAndTrue(Dict); //if absent then false, BValue = !Disabled
        newKextPatch.MenuItem.BValue = !IsPropertyNotNullAndTrue(Dict); //if absent then false, BValue = !Disabled
        
     //   if ((Dict != NULL) && IsPropertyNotNullAndTrue(Dict)) {
     //     newPatch.MenuItem.BValue     = FALSE;
     //   }
        
        
        Dict = Prop2->propertyForKey("RangeFind");
        newKextPatch.SearchLen = GetPropertyAsInteger(Dict, 0); //default 0 will be calculated later

        Dict = Prop2->propertyForKey("Skip");
        newKextPatch.Skip = GetPropertyAsInteger(Dict, 0); //default 0 will be calculated later

        UINT8* TmpData = GetDataSetting(Prop2, "StartPattern", &FindLen);
        if (TmpData != NULL) {
          newKextPatch.StartPattern.stealValueFrom(TmpData, FindLen);
        }
        
        TmpData    = GetDataSetting (Prop2, "MaskStart", &ReplaceLen);
        ReplaceLen = MIN(ReplaceLen, FindLen);
        if (FindLen != 0) {
          if (TmpData != NULL) newKextPatch.StartMask.ncpy(TmpData, ReplaceLen); // KextsToPatch
          newKextPatch.StartMask.setSize(FindLen, 0xFF);
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
            newKextPatch.ProcedureName = Dict->getString()->stringValue();
          }else{
            MsgLog("ATTENTION : Procedure property not string in KextsToPatch\n");
          }
        }


        newKextPatch.Find.stealValueFrom(TmpData, FindLen);
        
        TmpData    = GetDataSetting (Prop2, "MaskFind", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;

        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newKextPatch.MaskFind.ncpy(TmpData, MaskLen);
          newKextPatch.MaskFind.setSize(FindLen, 0xFF);
        }
        FreePool(TmpData);
        // take into account a possibility to set ReplaceLen < FindLen. In this case assumes MaskReplace = 0 for the rest of bytes 
        ReplaceLen = MIN(ReplaceLen, FindLen);
        newKextPatch.Replace.ncpy(TmpPatch, ReplaceLen);
        newKextPatch.Replace.setSize(FindLen, 0);
        FreePool(TmpPatch);
        
        MaskLen = 0;
        TmpData    = GetDataSetting (Prop2, "MaskReplace", &MaskLen);
        MaskLen = MIN(ReplaceLen, MaskLen);
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newKextPatch.MaskReplace.ncpy(TmpData, MaskLen); //other bytes are zeros, means no replace
          newKextPatch.MaskReplace.setSize(FindLen, 0);
        }
        FreePool(TmpData);
        
        newKextPatch.Count = 1;
        Dict = Prop2->propertyForKey("Count");
        if (Dict != NULL) {
          newKextPatch.Count = GetPropertyAsInteger(Dict, 1);
        }

        // check enable/disabled patch (OS based) by Micky1979
        Dict = Prop2->propertyForKey("MatchOS");
        if ((Dict != NULL) && (Dict->isString())) {
          newKextPatch.MatchOS = Dict->getString()->stringValue();
          DBG(" :: MatchOS: %s", newKextPatch.MatchOS.c_str());
        }

        Dict = Prop2->propertyForKey("MatchBuild");
        if ((Dict != NULL) && (Dict->isString())) {
          newKextPatch.MatchBuild = Dict->getString()->stringValue();
          DBG(" :: MatchBuild: %s", newKextPatch.MatchBuild.c_str());
        }

        // check if this is Info.plist patch or kext binary patch
        Dict = Prop2->propertyForKey("InfoPlistPatch");
        newKextPatch.IsPlistPatch = IsPropertyNotNullAndTrue(Dict);

        if (newKextPatch.IsPlistPatch) {
          DBG(" :: PlistPatch");
        } else {
          DBG(" :: BinPatch");
        }

        DBG(" :: data len: %zu\n", newKextPatch.Find.size());
        if (!newKextPatch.MenuItem.BValue) {
          DBG("        patch disabled at config\n");
        }
        Patches->KextPatches.AddReference(newKextPatchPtr, true);
      }
    }

    //settingsData.NrKexts = (INT32)i;
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

        KERNEL_PATCH* newKernelPatchPtr = new KERNEL_PATCH;
        KERNEL_PATCH& newKernelPatch = *newKernelPatchPtr;

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
        
//        newKernelPatch.Name = "kernel"_XS8;

        prop3 = Prop2->propertyForKey("Disabled");
        newKernelPatch.Disabled   = IsPropertyNotNullAndTrue(prop3);
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
          if (TmpData != NULL) newKernelPatch.StartMask.ncpy(TmpData, ReplaceLen); // KernelToPatch
          newKernelPatch.StartMask.setSize(FindLen, 0xFF);
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


        newKernelPatch.Find.stealValueFrom(TmpData, FindLen);

        TmpData    = GetDataSetting (Prop2, "MaskFind", &MaskLen);
        MaskLen = (MaskLen > FindLen)? FindLen : MaskLen;
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newKernelPatch.MaskFind.ncpy(TmpData, MaskLen);
          newKernelPatch.MaskFind.setSize(FindLen, 0xFF);
        }
        FreePool(TmpData);
        // this is "Replace" string len of ReplaceLen
        ReplaceLen = MIN(ReplaceLen, FindLen);
        newKernelPatch.Replace.ncpy(TmpPatch, ReplaceLen);
        newKernelPatch.Replace.setSize(FindLen, 0);
        FreePool(TmpPatch);
        MaskLen = 0;
        TmpData    = GetDataSetting (Prop2, "MaskReplace", &MaskLen); //reuse MaskLen
        MaskLen = MIN(ReplaceLen, MaskLen);
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newKernelPatch.MaskReplace.ncpy(TmpData, MaskLen);
          newKernelPatch.MaskReplace.setSize(FindLen, 0);
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
        DBG(" :: data len: %zu\n", newKernelPatch.Find.size());
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

        BOOT_PATCH* newBootPatchPtr = new BOOT_PATCH;
        BOOT_PATCH& newBootPatch = *newBootPatchPtr;

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
        
//        newBootPatch.Name = "boot.efi"_XS8;

        prop3 = Prop2->propertyForKey("Disabled");
        newBootPatch.Disabled   = IsPropertyNotNullAndTrue(prop3);
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
          if (TmpData != NULL) {
            newBootPatch.StartMask.ncpy(TmpData, ReplaceLen);
          }
          newBootPatch.StartMask.setSize(FindLen, 0xFF);
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
        newBootPatch.Find.stealValueFrom(TmpData, FindLen);

        MaskLen = 0;
        TmpData    = GetDataSetting(Prop2, "MaskFind", &MaskLen);
        MaskLen = MIN(FindLen, MaskLen);
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newBootPatch.MaskFind.ncpy(TmpData, MaskLen);
          newBootPatch.MaskFind.setSize(FindLen, 0xFF);
        }
        FreePool(TmpData);
        newBootPatch.Replace.ncpy(TmpPatch, ReplaceLen);
        newBootPatch.Replace.setSize(FindLen, 0);
        FreePool(TmpPatch);
        MaskLen = 0;
        TmpData    = GetDataSetting(Prop2, "MaskReplace", &MaskLen);
        MaskLen = MIN(ReplaceLen, MaskLen);
        if (TmpData == NULL || MaskLen == 0) {
        } else {
          newBootPatch.MaskReplace.ncpy(TmpData, MaskLen);
          newBootPatch.MaskReplace.setSize(FindLen, 0);
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

        DBG(" :: data len: %zu\n", newBootPatch.Find.size());
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
//      if ( osToc[1].isEqualIC("x") ) return true;
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
//                 && osToc[2].isEqualIC("x") ) {
//        ret = TRUE;
//      }
//    } else if (currOStoc.size() == 2) {
//      if ( osToc[0] == currOStoc[0]
//          && osToc[1] ==  currOStoc[1] ) {
//        ret = TRUE;
//      } else if ( osToc[0] == currOStoc[0]
//                 && osToc[1] ==  currOStoc[1]
//                 && osToc[2].isEqualIC("x") == 0 ) {
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
  if (Prop->getString()->stringValue().isEqualIC("Internal")) {
    VolumeTypeTmp |= VOLTYPE_INTERNAL;
  } else if (Prop->getString()->stringValue().isEqualIC("External")) {
    VolumeTypeTmp |= VOLTYPE_EXTERNAL;
  } else if (Prop->getString()->stringValue().isEqualIC("Optical")) {
    VolumeTypeTmp |= VOLTYPE_OPTICAL;
  } else if (Prop->getString()->stringValue().isEqualIC("FireWire")) {
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



BOOLEAN
FillinCustomSubEntry (
                   UINT8 parentType,
                   IN OUT  CUSTOM_LOADER_SUBENTRY_SETTINGS *Entry,
                   const TagDict* DictPointer,
                   IN      BOOLEAN SubEntry,
                   SETTINGS_DATA& settingsData)
{
  const TagStruct* Prop;

  if ( Entry == NULL ) panic("Entry == NULL");
  if ( DictPointer == NULL ) panic("DictPointer == NULL");

  Prop = DictPointer->propertyForKey("Disabled");
  Entry->Disabled = IsPropertyNotNullAndTrue(Prop);

//  Prop = DictPointer->propertyForKey("Volume");
//  if (Prop != NULL && (Prop->isString())) {
//    Entry->Volume = Prop->getString()->stringValue();
//  }

//  Prop = DictPointer->propertyForKey("Path");
//  if (Prop != NULL && (Prop->isString())) {
//    Entry->Path = Prop->getString()->stringValue();
//  }

//  Prop = DictPointer->propertyForKey("Settings");
//  if (Prop != NULL && (Prop->isString())) {
//    Entry->Settings = Prop->getString()->stringValue();
//  }

//  Prop = DictPointer->propertyForKey("CommonSettings");
//  Entry->CommonSettings = IsPropertyNotNullAndTrue(Prop);


  Prop = DictPointer->propertyForKey("AddArguments");
  if (Prop != NULL && (Prop->isString())) {
//    if (Entry->LoadOptions.notEmpty()) {
//      Entry->Options.SPrintf("%s %s", Entry->Options.c_str(), Prop->getString()->stringValue());
//    } else {
//      Entry->Options.SPrintf("%s", Prop->getString()->stringValue());
//    }
    Entry->_AddArguments = Prop->getString()->stringValue();
//    Entry->LoadOptions.import(Split<XString8Array>(Prop->getString()->stringValue(), " "));
  } else {
    Prop = DictPointer->propertyForKey("Arguments");
    if (Prop != NULL && (Prop->isString())) {
//      Entry->Options.SPrintf("%s", Prop->getString()->stringValue());
      Entry->_Arguments = Prop->getString()->stringValue();
//      Entry->LoadOptions = Split<XString8Array>(Prop->getString()->stringValue(), " ");
//      Entry->Flags       = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);
    }
  }
  Prop = DictPointer->propertyForKey("Title");
  if (Prop != NULL && (Prop->isString())) {
    Entry->_Title = Prop->getString()->stringValue();
//    Entry->FullTitle.setEmpty(); // jief : erase the copy from the parent
  }
  Prop = DictPointer->propertyForKey("FullTitle");
  if (Prop != NULL && (Prop->isString())) {
    Entry->_FullTitle = Prop->getString()->stringValue();
//    Entry->Title.setEmpty(); // jief : erase the copy from the parent. Could also be the previous settings, but Title is not used when FullTitle exists.
  }

//  Entry->ImageData.setEmpty();
//  Prop = DictPointer->propertyForKey("Image");
//  if (Prop != NULL) {
//    Entry->ImagePath.setEmpty();
//    Entry->Image.setEmpty();
//    if (Prop->isString()) {
//      Entry->ImagePath = SWPrintf("%s", Prop->getString()->stringValue().c_str());
//    }
//    // we can't load the file yet, as ThemeDir is not initialized
//  } else {
//    UINTN DataLen = 0;
//    UINT8 *TmpData = GetDataSetting (DictPointer, "ImageData", &DataLen);
//    if (TmpData) {
//      Entry->ImageData.stealValueFrom(TmpData, DataLen);
//// TODO remove from settings
//      if (!EFI_ERROR(Entry->Image.Image.FromPNG(TmpData, DataLen))) {
//        Entry->Image.setFilled();
//      }
//    }
//  }
//
//  Prop = DictPointer->propertyForKey("Hotkey");
//  if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
//    if ( Prop->getString()->stringValue()[0] < __WCHAR_MAX__ ) {
//      Entry->Hotkey = (wchar_t)(Prop->getString()->stringValue()[0]);
//    }
//  }

//  // Whether or not to draw boot screen
//  Prop = DictPointer->propertyForKey("CustomLogo");
//  if (Prop != NULL) {
//    if (IsPropertyNotNullAndTrue(Prop)) {
//      Entry->CustomLogoType    = CUSTOM_BOOT_APPLE;
//    } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
//      Entry->CustomLogoAsXString8 = Prop->getString()->stringValue();
//      if (Prop->getString()->stringValue().isEqualIC("Apple")) {
//        Entry->CustomLogoType  = CUSTOM_BOOT_APPLE;
//      } else if (Prop->getString()->stringValue().isEqualIC("Alternate")) {
//        Entry->CustomLogoType  = CUSTOM_BOOT_ALT_APPLE;
//      } else if (Prop->getString()->stringValue().isEqualIC("Theme")) {
//        Entry->CustomLogoType  = CUSTOM_BOOT_THEME;
//      } else {
//        XStringW customLogo = XStringW() = Prop->getString()->stringValue();
//        Entry->CustomLogoType  = CUSTOM_BOOT_USER;
//// TODO : remove reading of image from settings
//        Entry->CustomLogoImage.LoadXImage(&self.getSelfVolumeRootDir(), customLogo);
//        if (Entry->CustomLogoImage.isEmpty()) {
//          DBG("Custom boot logo not found at path `%ls`!\n", customLogo.wc_str());
//          Entry->CustomLogoType = CUSTOM_BOOT_DISABLED;
//        }
//      }
//    } else if ( Prop->isData() && Prop->getData()->dataLenValue() > 0 ) {
//      Entry->CustomLogoType = CUSTOM_BOOT_USER;
//      Entry->CustomLogoAsData = Prop->getData()->data();
//// TODO : remove reading of image from settings
//      Entry->CustomLogoImage.FromPNG(Prop->getData()->dataValue(), Prop->getData()->dataLenValue());
//      if (Entry->CustomLogoImage.isEmpty()) {
//        DBG("Custom boot logo not decoded from data!\n"/*, Prop->getString()->stringValue().c_str()*/);
//        Entry->CustomLogoType = CUSTOM_BOOT_DISABLED;
//      }
//    } else {
//      Entry->CustomLogoType = CUSTOM_BOOT_USER_DISABLED;
//    }
//    DBG("Custom entry boot %s LogoWidth = (0x%lld)\n", CustomBootModeToStr(Entry->CustomLogoType), Entry->CustomLogoImage.GetWidth());
//  } else {
//    Entry->CustomLogoType = CUSTOM_BOOT_DISABLED;
//  }

//  Prop = DictPointer->propertyForKey("BootBgColor");
//  if (Prop != NULL && Prop->isString()) {
//    UINTN   Color;
//    Color = AsciiStrHexToUintn(Prop->getString()->stringValue());
//
//    Entry->BootBgColor.Red = (Color >> 24) & 0xFF;
//    Entry->BootBgColor.Green = (Color >> 16) & 0xFF;
//    Entry->BootBgColor.Blue = (Color >> 8) & 0xFF;
//    Entry->BootBgColor.Reserved = (Color >> 0) & 0xFF;
//  }
//
//  // Hidden Property, Values:
//  // - No (show the entry)
//  // - Yes (hide the entry but can be show with F3)
//  // - Always (always hide the entry)
//  Prop = DictPointer->propertyForKey("Hidden");
//  if (Prop != NULL) {
//    if ((Prop->isString()) &&
//        (Prop->getString()->stringValue().isEqualIC("Always"))) {
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
//    } else if (IsPropertyNotNullAndTrue(Prop)) {
//      DBG("     hiding entry because Hidden flag is YES\n");
//      Entry->Hidden = true;
//    } else {
//      Entry->Hidden = false;
//    }
//  }
//
//  Prop = DictPointer->propertyForKey("Type");
//  if (Prop != NULL && (Prop->isString())) {
//    if ((Prop->getString()->stringValue().isEqualIC("OSX")) ||
//        (Prop->getString()->stringValue().isEqualIC("macOS"))) {
//      Entry->Type = OSTYPE_OSX;
//    } else if (Prop->getString()->stringValue().isEqualIC("OSXInstaller")) {
//      Entry->Type = OSTYPE_OSX_INSTALLER;
//    } else if (Prop->getString()->stringValue().isEqualIC("OSXRecovery")) {
//      Entry->Type = OSTYPE_RECOVERY;
//    } else if (Prop->getString()->stringValue().isEqualIC("Windows")) {
//      Entry->Type = OSTYPE_WINEFI;
//    } else if (Prop->getString()->stringValue().isEqualIC("Linux")) {
//      Entry->Type = OSTYPE_LIN;
//// TODO remove from here
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);
//    } else if (Prop->getString()->stringValue().isEqualIC("LinuxKernel")) {
//      Entry->Type = OSTYPE_LINEFI;
//    } else {
//      DBG("** Warning: unknown custom entry Type '%s'\n", Prop->getString()->stringValue().c_str());
//      Entry->Type = OSTYPE_OTHER;
//    }
//  } else {
//    if (Entry->Type == 0 && Entry->Path.notEmpty()) {
//      // Try to set Entry->type from Entry->Path
//      Entry->Type = GetOSTypeFromPath(Entry->Path);
//    }
//  }

//  Entry->VolumeType = GetVolumeType(DictPointer);

//  if (Entry->LoadOptions.isEmpty() && OSTYPE_IS_WINDOWS(parentType)) {
//    Entry->LoadOptions.Add("-s");
//    Entry->LoadOptions.Add("-h");
//  }
  if (Entry->_Title.dgetValue().isEmpty()) {
    if (OSTYPE_IS_OSX_RECOVERY(parentType)) {
      Entry->_Title = "Recovery"_XS8;
    } else if (OSTYPE_IS_OSX_INSTALLER(parentType)) {
      Entry->_Title = "Install macOS"_XS8;
    }
  }
//  if (Entry->Image.isEmpty() && (Entry->ImagePath.isEmpty())) {
//    if (OSTYPE_IS_OSX_RECOVERY(parentType)) {
//      Entry->ImagePath = L"mac"_XSW;
//    }
//  }
  // OS Specific flags
  if (OSTYPE_IS_OSX(parentType) || OSTYPE_IS_OSX_RECOVERY(parentType) || OSTYPE_IS_OSX_INSTALLER(parentType)) {

    // InjectKexts default values
//    Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_CHECKFAKESMC);
    //  Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_WITHKEXTS);

//    Prop = DictPointer->propertyForKey("InjectKexts");
//    if (Prop != NULL) {
//      if ( Prop->isTrueOrYes() ) {
//        Entry->InjectKexts = 1;
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
//      } else if ( Prop->isFalseOrNn() ) {
//        Entry->InjectKexts = 0;
//        // nothing to do
//      } else if ( Prop->isString() && Prop->getString()->stringValue().isEqualIC("Detect") ) {
//        Entry->InjectKexts = 2;
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
//      } else {
//        Entry->InjectKexts = -1;
//        DBG("** Warning: unknown custom entry InjectKexts value '%s'\n", Prop->getString()->stringValue().c_str());
//      }
//    } else {
//      Entry->InjectKexts = -1;
//      // Use global settings
//      if (settingsData.WithKexts) {
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
//      }
//      if (settingsData.WithKextsIfNoFakeSMC) {
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
//      }
//    }

    // NoCaches default value
//    Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_NOCACHES);

    Prop = DictPointer->propertyForKey("NoCaches");
    if (Prop != NULL) {
      if (IsPropertyNotNullAndTrue(Prop)) {
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
          Entry->_NoCaches = true;
      } else {
        // Use global settings
        if (settingsData.SystemParameters.NoCaches) {
//          Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
          Entry->_NoCaches = false;
        }
      }
    }

//    // KernelAndKextPatches
//    if (!SubEntry) { // CopyKernelAndKextPatches already in: DuplicateCustomEntry if SubEntry == TRUE
//      //DBG("Copying global patch settings\n");
////      CopyKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)),
// //                               (KERNEL_AND_KEXT_PATCHES *)(((UINTN)&settingsData) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)));
//
////      CopyKernelAndKextPatches(&Entry->KernelAndKextPatches, &settingsData.KernelAndKextPatches);
//      Entry->KernelAndKextPatches = settingsData.KernelAndKextPatches;
//
//      //#ifdef DUMP_KERNEL_KEXT_PATCHES
//      //    DumpKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)));
//      //#endif
//
//    }

  }

//  if (Entry->Type == OSTYPE_LINEFI) {
//    Prop = DictPointer->propertyForKey("Kernel");
//    if (Prop != NULL) {
//      if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
//        if ((Prop->getString()->stringValue()[0] == 'N') || (Prop->getString()->stringValue()[0] == 'n')) {
//          Entry->KernelScan = KERNEL_SCAN_NEWEST;
//        } else if ((Prop->getString()->stringValue()[0] == 'O') || (Prop->getString()->stringValue()[0] == 'o')) {
//          Entry->KernelScan = KERNEL_SCAN_OLDEST;
//        } else if ((Prop->getString()->stringValue()[0] == 'F') || (Prop->getString()->stringValue()[0] == 'f')) {
//          Entry->KernelScan = KERNEL_SCAN_FIRST;
//        } else if ((Prop->getString()->stringValue()[0] == 'L') || (Prop->getString()->stringValue()[0] == 'l')) {
//          Entry->KernelScan = KERNEL_SCAN_LAST;
//        } else if ((Prop->getString()->stringValue()[0] == 'M') || (Prop->getString()->stringValue()[0] == 'm')) {
//          Entry->KernelScan = KERNEL_SCAN_MOSTRECENT;
//        } else if ((Prop->getString()->stringValue()[0] == 'E') || (Prop->getString()->stringValue()[0] == 'e')) {
//          Entry->KernelScan = KERNEL_SCAN_EARLIEST;
//        }
//      }
//    }
//  }

//  /*
//   * Sub entries
//   * an array of dict OR a bool
//  */
//  Prop = DictPointer->propertyForKey("SubEntries");
//  if (Prop != NULL) {
//    if ( Prop->isBool() && Prop->getBool()->boolValue() ) {
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
//    } else if ( Prop->isArray() ) {
//      CUSTOM_LOADER_SUBENTRY_SETTINGS *CustomSubEntry;
//      INTN   i;
//      INTN   Count = Prop->getArray()->arrayContent().size();
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
//      for (i = 0; i < Count; i++) {
//        const TagDict* Dict = Prop->getArray()->dictElementAt(i, "SubEntries"_XS8);
//        // Allocate a sub entry
//        CustomSubEntry = DuplicateCustomSubEntry(Entry);
//        if (CustomSubEntry) {
//          if ( FillinCustomSubEntry(CustomSubEntry, Dict, TRUE) ) {
//            Entry->SubEntriesSettings.AddReference(CustomSubEntry, true);
//          }else{
//            delete CustomSubEntry;
//          }
//        }
//      }
//    }else{
//      MsgLog("MALFORMED PLIST : SubEntries must be a bool OR an array of dict");
//    }
//  }
  return TRUE;
}


BOOLEAN
FillinCustomEntry (
                   IN OUT  CUSTOM_LOADER_ENTRY_SETTINGS *Entry,
                   const TagDict* DictPointer,
                   IN      BOOLEAN SubEntry,
                   SETTINGS_DATA& settingsData)
{
  const TagStruct* Prop;

  if ( Entry == NULL ) panic("Entry == NULL");
  if ( DictPointer == NULL ) panic("DictPointer == NULL");

  Prop = DictPointer->propertyForKey("Disabled");
  Entry->Disabled = IsPropertyNotNullAndTrue(Prop);

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
    Entry->AddArguments = Prop->getString()->stringValue();
//    Entry->LoadOptions.import(Split<XString8Array>(Prop->getString()->stringValue(), " "));
  } else {
    Prop = DictPointer->propertyForKey("Arguments");
    if (Prop != NULL && (Prop->isString())) {
      Entry->Arguments = Prop->getString()->stringValue();
//      Entry->Options.SPrintf("%s", Prop->getString()->stringValue());
//      Entry->LoadOptions = Split<XString8Array>(Prop->getString()->stringValue(), " ");
//      Entry->Flags       = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);
    }
  }
  Prop = DictPointer->propertyForKey("Title");
  if (Prop != NULL && (Prop->isString())) {
    Entry->m_Title = Prop->getString()->stringValue();
  }
  Prop = DictPointer->propertyForKey("FullTitle");
  if (Prop != NULL && (Prop->isString())) {
    Entry->FullTitle = Prop->getString()->stringValue();
  }

  Entry->m_ImagePath.setEmpty();
  Entry->ImageData.setEmpty();
  Prop = DictPointer->propertyForKey("Image");
  if (Prop != NULL) {
    Entry->m_ImagePath.setEmpty();
//    Entry->Image.setEmpty();
    if (Prop->isString()) {
      Entry->m_ImagePath = SWPrintf("%s", Prop->getString()->stringValue().c_str());
    }
    // we can't load the file yet, as ThemeDir is not initialized
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "ImageData", &DataLen);
    if (TmpData) {
      Entry->ImageData.stealValueFrom(TmpData, DataLen);
//      if (!EFI_ERROR(Entry->Image.Image.FromPNG(TmpData, DataLen))) {
//        Entry->Image.setFilled();
//      }
    }
  }

  Entry->m_DriveImagePath.setEmpty();
  Entry->DriveImageData.setEmpty();
  Prop = DictPointer->propertyForKey("DriveImage");
  if (Prop != NULL) {
//    Entry->DriveImage.setEmpty();
    if (Prop->isString()) {
      Entry->m_DriveImagePath = SWPrintf("%s", Prop->getString()->stringValue().c_str());
    }
    // we can't load the file yet, as ThemeDir is not initialized
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "DriveImageData", &DataLen);
    if (TmpData) {
      Entry->DriveImageData.stealValueFrom(TmpData, DataLen);
//      if (!EFI_ERROR(Entry->DriveImage.Image.FromPNG(TmpData, DataLen))) {
//        Entry->DriveImage.setFilled();
//      }
//      FreePool(TmpData);
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
      Entry->CustomLogoTypeSettings    = CUSTOM_BOOT_APPLE;
    } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
      Entry->CustomLogoAsXString8 = Prop->getString()->stringValue();
      if (Prop->getString()->stringValue().isEqualIC("Apple")) {
        Entry->CustomLogoTypeSettings  = CUSTOM_BOOT_APPLE;
      } else if (Prop->getString()->stringValue().isEqualIC("Alternate")) {
        Entry->CustomLogoTypeSettings  = CUSTOM_BOOT_ALT_APPLE;
      } else if (Prop->getString()->stringValue().isEqualIC("Theme")) {
        Entry->CustomLogoTypeSettings  = CUSTOM_BOOT_THEME;
      } else {
        XStringW customLogo = XStringW() = Prop->getString()->stringValue();
        Entry->CustomLogoTypeSettings  = CUSTOM_BOOT_USER;
//        Entry->CustomLogoImage.LoadXImage(&self.getSelfVolumeRootDir(), customLogo);
//        if (Entry->CustomLogoImage.isEmpty()) {
//          DBG("Custom boot logo not found at path `%ls`!\n", customLogo.wc_str());
//          Entry->CustomLogoType = CUSTOM_BOOT_DISABLED;
//        }
      }
    } else if ( Prop->isData() && Prop->getData()->dataLenValue() > 0 ) {
      Entry->CustomLogoTypeSettings = CUSTOM_BOOT_USER;
      Entry->CustomLogoAsData = Prop->getData()->data();
//      Entry->CustomLogoImage.FromPNG(Prop->getData()->dataValue(), Prop->getData()->dataLenValue());
//      if (Entry->CustomLogoImage.isEmpty()) {
//        DBG("Custom boot logo not decoded from data!\n"/*, Prop->getString()->stringValue().c_str()*/);
//        Entry->CustomLogoType = CUSTOM_BOOT_DISABLED;
//      }
    } else {
      Entry->CustomLogoTypeSettings = CUSTOM_BOOT_USER_DISABLED;
    }
    DBG("Custom entry boot %s\n", CustomBootModeToStr(Entry->CustomLogoTypeSettings));
  } else {
    Entry->CustomLogoTypeSettings = CUSTOM_BOOT_DISABLED;
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
  Entry->AlwaysHidden = false;
  Entry->Hidden = false;
  Prop = DictPointer->propertyForKey("Hidden");
  if (Prop != NULL) {
    if ((Prop->isString()) &&
        (Prop->getString()->stringValue().isEqualIC("Always"))) {
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
      Entry->AlwaysHidden = true;
    } else if (IsPropertyNotNullAndTrue(Prop)) {
      DBG("     hiding entry because Hidden flag is YES\n");
      Entry->Hidden = true;
    }
  }

  Prop = DictPointer->propertyForKey("Type");
  if (Prop != NULL && (Prop->isString())) {
    if ((Prop->getString()->stringValue().isEqualIC("OSX")) ||
        (Prop->getString()->stringValue().isEqualIC("macOS"))) {
      Entry->Type = OSTYPE_OSX;
    } else if (Prop->getString()->stringValue().isEqualIC("OSXInstaller")) {
      Entry->Type = OSTYPE_OSX_INSTALLER;
    } else if (Prop->getString()->stringValue().isEqualIC("OSXRecovery")) {
      Entry->Type = OSTYPE_RECOVERY;
    } else if (Prop->getString()->stringValue().isEqualIC("Windows")) {
      Entry->Type = OSTYPE_WINEFI;
    } else if (Prop->getString()->stringValue().isEqualIC("Linux")) {
      Entry->Type = OSTYPE_LIN;
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTARGS);
    } else if (Prop->getString()->stringValue().isEqualIC("LinuxKernel")) {
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

//  if (Entry->LoadOptions.isEmpty() && OSTYPE_IS_WINDOWS(Entry->Type)) {
//    Entry->LoadOptions.Add("-s");
//    Entry->LoadOptions.Add("-h");
//  }
//  if (Entry->Title.isEmpty()) {
//    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
//      Entry->Title = L"Recovery"_XSW;
//    } else if (OSTYPE_IS_OSX_INSTALLER(Entry->Type)) {
//      Entry->Title = L"Install macOS"_XSW;
//    }
//  }
//  if (Entry->Image.isEmpty() && (Entry->ImagePath.isEmpty())) {
//    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
//      Entry->ImagePath = L"mac"_XSW;
//    }
//  }
//  if (Entry->DriveImage.isEmpty() && (Entry->DriveImagePath.isEmpty())) {
//    if (OSTYPE_IS_OSX_RECOVERY(Entry->Type)) {
//      Entry->DriveImagePath = L"recovery"_XSW;
//    }
//  }
  // OS Specific flags
  if (OSTYPE_IS_OSX(Entry->Type) || OSTYPE_IS_OSX_RECOVERY(Entry->Type) || OSTYPE_IS_OSX_INSTALLER(Entry->Type)) {

    // InjectKexts default values
//    Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_CHECKFAKESMC);
    //  Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_WITHKEXTS);

    Prop = DictPointer->propertyForKey("InjectKexts");
    if (Prop != NULL) {
      if ( Prop->isTrueOrYes() ) {
        Entry->InjectKexts = 1;
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      } else if ( Prop->isFalseOrNn() ) {
        Entry->InjectKexts = 0;
        // nothing to do
      } else if ( Prop->isString() && Prop->getString()->stringValue().isEqualIC("Detect") ) {
        Entry->InjectKexts = 2;
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      } else {
        Entry->InjectKexts = -2;
        DBG("** Warning: unknown custom entry InjectKexts value '%s'\n", Prop->getString()->stringValue().c_str());
      }
    } else {
      Entry->InjectKexts = -1;
      // Use global settings
      if (settingsData.SystemParameters.WithKexts) {
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
      if (settingsData.SystemParameters.WithKextsIfNoFakeSMC) {
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_CHECKFAKESMC);
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_WITHKEXTS);
      }
    }

    // NoCaches default value
//    Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_NOCACHES);

    Prop = DictPointer->propertyForKey("NoCaches");
    if (Prop != NULL) {
      if (IsPropertyNotNullAndTrue(Prop)) {
//        Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
        Entry->NoCaches = true;
      } else {
        // Use global settings
        if (settingsData.SystemParameters.NoCaches) {
//          Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NOCACHES);
        }
      }
    }

    // KernelAndKextPatches
    if (!SubEntry) { // CopyKernelAndKextPatches already in: DuplicateCustomEntry if SubEntry == TRUE
      //DBG("Copying global patch settings\n");
//      CopyKernelAndKextPatches ((KERNEL_AND_KEXT_PATCHES *)(((UINTN)Entry) + OFFSET_OF(CUSTOM_LOADER_ENTRY, KernelAndKextPatches)),
 //                               (KERNEL_AND_KEXT_PATCHES *)(((UINTN)&settingsData) + OFFSET_OF(SETTINGS_DATA, KernelAndKextPatches)));

//      CopyKernelAndKextPatches(&Entry->KernelAndKextPatches, &settingsData.KernelAndKextPatches);
//      Entry->KernelAndKextPatches = settingsData.KernelAndKextPatches;

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
    /*if ( Prop->isBool() && Prop->getBool()->boolValue() ) {
      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
    } else*/ if ( Prop->isArray() ) {
      INTN   Count = Prop->getArray()->arrayContent().size();
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_NODEFAULTMENU);
      for (INTN i = 0; i < Count; i++) {
        const TagDict* Dict = Prop->getArray()->dictElementAt(i, "SubEntries"_XS8);
        // Allocate a sub entry
//        CustomSubEntry = DuplicateCustomEntryToSubEntry(Entry);
        CUSTOM_LOADER_SUBENTRY_SETTINGS* CustomSubEntry = new CUSTOM_LOADER_SUBENTRY_SETTINGS;
        if (CustomSubEntry) {
          if ( FillinCustomSubEntry(Entry->Type, CustomSubEntry, Dict, TRUE, settingsData) ) {
            Entry->SubEntriesSettings.AddReference(CustomSubEntry, true);
          }else{
            delete CustomSubEntry;
          }
        }
      }
    }else{
      MsgLog("MALFORMED PLIST : SubEntries must be an array of dict");
    }
  }
  return TRUE;
}

static BOOLEAN
FillingCustomLegacy (
                    IN OUT CUSTOM_LEGACY_ENTRY_SETTINGS *Entry,
                    const TagDict* DictPointer
                    )
{
  const TagStruct* Prop;
  if ((Entry == NULL) || (DictPointer == NULL)) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Disabled");
  if (IsPropertyNotNullAndTrue(Prop)) {
    Entry->Disabled = true;
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
      Entry->ImagePath = Prop->getString()->stringValue();
//      Entry->Image.LoadXImage(&ThemeX.getThemeDir(), Prop->getString()->stringValue());
    }
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "ImageData", &DataLen);
    if (TmpData) {
      Entry->ImageData.stealValueFrom(TmpData, DataLen);
//      if (!EFI_ERROR(Entry->Image.Image.FromPNG(TmpData, DataLen))) {
//        Entry->Image.setFilled();
//      }
//      FreePool(TmpData);
    }
  }

  Prop = DictPointer->propertyForKey("DriveImage");
  if (Prop != NULL) {
    if (Prop->isString()) {
      Entry->DriveImagePath = Prop->getString()->stringValue();
//      Entry->Image.LoadXImage(&ThemeX.getThemeDir(), Prop->getString()->stringValue());
    }
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "DriveImageData", &DataLen);
    if (TmpData) {
      Entry->DriveImageData.stealValueFrom(TmpData, DataLen);
//      if (!EFI_ERROR(Entry->DriveImage.Image.FromPNG(TmpData, DataLen))) {
//        Entry->DriveImage.setFilled();
//      }
//      FreePool(TmpData);
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
        (Prop->getString()->stringValue().isEqualIC("Always"))) {
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
      Entry->AlwaysHidden = true;
    } else if (IsPropertyNotNullAndTrue(Prop)) {
      DBG("     hiding entry because Hidden flag is YES\n");
      Entry->Hidden = true;
    } else {
      Entry->Hidden = false;
    }
  }

  Prop = DictPointer->propertyForKey("Type");
  if (Prop != NULL && (Prop->isString())) {
    if (Prop->getString()->stringValue().isEqualIC("Windows")) {
      Entry->Type = OSTYPE_WIN;
    } else if (Prop->getString()->stringValue().isEqualIC("Linux")) {
      Entry->Type = OSTYPE_LIN;
    } else {
      Entry->Type = OSTYPE_OTHER;
    }
  }

  Entry->VolumeType = GetVolumeType(DictPointer);
  return TRUE;
}

static BOOLEAN
FillingCustomTool (IN OUT CUSTOM_TOOL_ENTRY_SETTINGS *Entry, const TagDict* DictPointer)
{
  const TagStruct* Prop;
  if ((Entry == NULL) || (DictPointer == NULL)) {
    return FALSE;
  }

  Prop = DictPointer->propertyForKey("Disabled");
  if (IsPropertyNotNullAndTrue(Prop)) {
    Entry->Disabled = true;
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
//      Entry->LoadOptions = Split<XString8Array>(Prop->getString()->stringValue(), " ");
    Entry->Arguments = Prop->getString()->stringValue();
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
//    Entry->Image.LoadXImage(&ThemeX.getThemeDir(), Entry->ImagePath);
  } else {
    UINTN DataLen = 0;
    UINT8 *TmpData = GetDataSetting (DictPointer, "ImageData", &DataLen);
    if (TmpData) {
      Entry->ImageData.stealValueFrom(TmpData, DataLen);
//      if (!EFI_ERROR(Entry->Image.Image.FromPNG(TmpData, DataLen))) {
//        Entry->Image.setFilled();
//      }
//      FreePool(TmpData);
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
        (Prop->getString()->stringValue().isEqualIC("Always"))) {
//      Entry->Flags = OSFLAG_SET(Entry->Flags, OSFLAG_DISABLED);
      Entry->AlwaysHidden = true;
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

/*
 * To ease copy/paste and text replacement from GetUserSettings, the parameter has the same name as the global
 * and is passed by non-const reference.
 * This temporary during the refactoring
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
// EDID reworked by Sherlocks
static void
GetEDIDSettings(const TagDict* DictPointer, SETTINGS_DATA& settingsData)
{
  #pragma GCC diagnostic pop
  const TagStruct* Prop;
  const TagDict* Dict;
  UINTN  j = 128;

  Dict = DictPointer->dictPropertyForKey("EDID");
  if (Dict != NULL) {
    Prop = Dict->propertyForKey("Inject");
    settingsData.Graphics.EDID.InjectEDID = IsPropertyNotNullAndTrue(Prop); // default = false!

    if (settingsData.Graphics.EDID.InjectEDID){
      //DBG("Inject EDID\n");
      Prop = Dict->propertyForKey("Custom");
      if (Prop != NULL) {
        UINT8* Data = GetDataSetting(Dict, "Custom", &j);
        settingsData.Graphics.EDID.CustomEDID.stealValueFrom(Data, j);
        if ((j % 128) != 0) {
          DBG(" Custom EDID has wrong length=%llu\n", j);
          settingsData.Graphics.EDID.CustomEDID.setEmpty();
        } else {
          DBG(" Custom EDID is ok\n");
//          settingsData.CustomEDIDsize = (UINT16)j;
//          InitializeEdidOverride();
        }
      }

      Prop = Dict->propertyForKey("VendorID");
      if (Prop) {
        settingsData.Graphics.EDID.VendorEDID = (UINT16)GetPropertyAsInteger(Prop, settingsData.Graphics.EDID.VendorEDID);
        //DBG("  VendorID = 0x%04lx\n", settingsData.Graphics.EDID.VendorEDID);
      }

      Prop = Dict->propertyForKey("ProductID");
      if (Prop) {
        settingsData.Graphics.EDID.ProductEDID = (UINT16)GetPropertyAsInteger(Prop, settingsData.Graphics.EDID.ProductEDID);
        //DBG("  ProductID = 0x%04lx\n", settingsData.Graphics.EDID.ProductEDID);
      }

      Prop = Dict->propertyForKey("HorizontalSyncPulseWidth");
      if (Prop) {
        settingsData.Graphics.EDID.EdidFixHorizontalSyncPulseWidth = (UINT16)GetPropertyAsInteger(Prop, settingsData.Graphics.EDID.EdidFixHorizontalSyncPulseWidth);
        //DBG("  EdidFixHorizontalSyncPulseWidth = 0x%02lx\n", settingsData.Graphics.EDID.EdidFixHorizontalSyncPulseWidth);
      }

      Prop = Dict->propertyForKey("VideoInputSignal");
      if (Prop) {
        settingsData.Graphics.EDID.EdidFixVideoInputSignal = (UINT8)GetPropertyAsInteger(Prop, settingsData.Graphics.EDID.EdidFixVideoInputSignal);
        //DBG("  EdidFixVideoInputSignal = 0x%02lx\n", settingsData.Graphics.EDID.EdidFixVideoInputSignal);
      }
    } else {
      //DBG("Not Inject EDID\n");
    }
  }
}

// Jief : GetEarlyUserSettings is ready to disappear...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
EFI_STATUS GetEarlyUserSettings (
                      const TagDict* CfgDict,
                      SETTINGS_DATA& settingsData
                      )
{
  #pragma GCC diagnostic pop
  
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

  GlobalConfig.KextPatchesAllowed              = TRUE;
  GlobalConfig.KernelPatchesAllowed            = TRUE;
  settingsData.KernelAndKextPatches.KPAppleRTC = TRUE;
  settingsData.KernelAndKextPatches.KPDELLSMBIOS = FALSE; // default is false

  if (CfgDict != NULL) {
    //DBG("Loading early settings\n");
    DbgHeader("GetEarlyUserSettings");

    const TagDict* BootDict = CfgDict->dictPropertyForKey("Boot");
    if (BootDict != NULL) {
      const TagStruct* Prop = BootDict->propertyForKey("Timeout");
      if (Prop != NULL) {
        settingsData.Boot.Timeout = GetPropertyAsInteger(Prop, settingsData.Boot.Timeout);
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
          if (Prop->getString()->stringValue().isEqualIC("LastBootedVolume")) {
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
          if ( Prop->getString()->stringValue().isEqualIC("true") ) settingsData.Boot.DebugLog = true;
          else if ( Prop->getString()->stringValue().isEqualIC("false") ) settingsData.Boot.DebugLog = false;
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
      settingsData.Boot.SecureSetting = -1;
      Prop = BootDict->propertyForKey("Secure");
      if (Prop != NULL) {
        if ( Prop->isTrue() ) settingsData.Boot.SecureSetting = 1;
        if ( Prop->isFalse() ) settingsData.Boot.SecureSetting = 0;
//        if ( Prop->isFalse() ) {
//          // Only disable setup mode, we want always secure boot
//          settingsData.Boot.SecureBootSetupMode = 0;
//        } else if ( Prop->isTrue()  &&  !settingsData.Boot.SecureBoot ) {
//          // This mode will force boot policy even when no secure boot or it is disabled
//          settingsData.Boot.SecureBootSetupMode = 1;
//          settingsData.Boot.SecureBoot          = 1;
//        }
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
          settingsData.Boot.SecureBootWhiteList.setEmpty();
          for (i = 0; i < Count; i++) {
            const TagStruct* prop2 = &arrayProp->arrayContent()[i];
            if ( !prop2->isString() ) {
              MsgLog("MALFORMED PLIST : WhiteList must be an array of string");
              continue;
            }
            if ( prop2->getString()->stringValue().notEmpty() ) {
              settingsData.Boot.SecureBootWhiteList.AddNoNull(prop2->getString()->stringValue());
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
          settingsData.Boot.SecureBootBlackList.setEmpty();
          for (i = 0; i < Count; i++) {
            const TagStruct* prop2 = &arrayProp->arrayContent()[i];
            if ( !prop2->isString() ) {
              MsgLog("MALFORMED PLIST : BlackList must be an array of string");
              continue;
            }
            if ( prop2->getString()->stringValue().notEmpty() ) {
              settingsData.Boot.SecureBootBlackList.AddNoNull(prop2->getString()->stringValue());
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
      settingsData.Boot.CustomLogoAsXString8.setEmpty();
      settingsData.Boot.CustomLogoAsData.setEmpty();
      Prop = BootDict->propertyForKey("CustomLogo");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          settingsData.Boot.CustomLogoType   = CUSTOM_BOOT_APPLE;
        } else if ((Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
          settingsData.Boot.CustomLogoAsXString8 = Prop->getString()->stringValue();
          if (Prop->getString()->stringValue().isEqualIC("Apple")) {
            settingsData.Boot.CustomLogoType = CUSTOM_BOOT_APPLE;
          } else if (Prop->getString()->stringValue().isEqualIC("Alternate")) {
            settingsData.Boot.CustomLogoType = CUSTOM_BOOT_ALT_APPLE;
          } else if (Prop->getString()->stringValue().isEqualIC("Theme")) {
            settingsData.Boot.CustomLogoType = CUSTOM_BOOT_THEME;
          } else {
            settingsData.Boot.CustomLogoType = CUSTOM_BOOT_USER;
//            if (settingsData.Boot.CustomLogo != NULL) {
//              delete settingsData.Boot.CustomLogo;
//            }
//            settingsData.Boot.CustomLogo = new XImage;
//            settingsData.Boot.CustomLogo->LoadXImage(&self.getSelfVolumeRootDir(), customLogo);
//            if (settingsData.Boot.CustomLogo->isEmpty()) {
//              DBG("Custom boot logo not found at path `%ls`!\n", customLogo.wc_str());
//              settingsData.Boot.CustomBoot = CUSTOM_BOOT_DISABLED;
//            }
          }
        } else if ( Prop->isData()  && Prop->getData()->dataLenValue() > 0 ) {
          settingsData.Boot.CustomLogoAsData = Prop->getData()->data();
          settingsData.Boot.CustomLogoType = CUSTOM_BOOT_USER;
//          if (settingsData.Boot.CustomLogo != NULL) {
//            delete settingsData.Boot.CustomLogo;
//          }
//          settingsData.Boot.CustomLogo = new XImage;
//          settingsData.Boot.CustomLogo->FromPNG(Prop->getData()->dataValue(), Prop->getData()->dataLenValue());
//          if (settingsData.Boot.CustomLogo->isEmpty()) {
//            DBG("Custom boot logo not decoded from data!\n"/*, Prop->getString()->stringValue().c_str()*/);
//            settingsData.Boot.CustomBoot = CUSTOM_BOOT_DISABLED;
//          }
        } else {
          settingsData.Boot.CustomLogoType = CUSTOM_BOOT_USER_DISABLED;
        }
      } else {
        settingsData.Boot.CustomLogoType   = CUSTOM_BOOT_DISABLED;
      }

    }

    //*** SYSTEM ***
    settingsData.SystemParameters.WithKexts            = TRUE;  //default
    const TagDict* SystemParametersDict = CfgDict->dictPropertyForKey("SystemParameters");
    if (SystemParametersDict != NULL) {
      // Inject kexts
      const TagStruct* Prop = SystemParametersDict->propertyForKey("InjectKexts");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          settingsData.SystemParameters.WithKexts            = TRUE;
        } else if ((Prop->isString()) &&
                   (Prop->getString()->stringValue().isEqualIC("Detect"))) {
          //   settingsData.WithKexts            = TRUE;
          settingsData.SystemParameters.WithKextsIfNoFakeSMC = TRUE;
        }
      }

      // No caches - obsolete
      Prop = SystemParametersDict->propertyForKey("NoCaches");
      if (IsPropertyNotNullAndTrue(Prop)) {
        settingsData.SystemParameters.NoCaches = TRUE;
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
//      //initialize Daylight when we know timezone
//#ifdef CLOVER_BUILD
//      EFI_TIME          Now;
//      gRT->GetTime(&Now, NULL);
//      INT32 NowHour = Now.Hour + settingsData.GUI.Timezone;
//      if (NowHour <  0 ) NowHour += 24;
//      if (NowHour >= 24 ) NowHour -= 24;
//      ThemeX.Daylight = (NowHour > 8) && (NowHour < 20);
//#endif

      Prop = GUIDict->propertyForKey("Theme");
      if (Prop != NULL && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
//        ThemeX.Theme.takeValueFrom(Prop->getString()->stringValue());
        settingsData.GUI.Theme.takeValueFrom(Prop->getString()->stringValue());
//        DBG("Default theme: %ls\n", settingsData.GUI.Theme.wc_str());
//        OldChosenTheme = 0xFFFF; //default for embedded
//        for (UINTN i = 0; i < ThemeNameArray.size(); i++) {
//          //now comparison is case sensitive
//          if ( settingsData.GUI.Theme.isEqualIC(ThemeNameArray[i]) ) {
//            OldChosenTheme = i;
//            break;
//          }
//        }
      }
      // get embedded theme property even when starting with other themes, as they may be changed later
      Prop = GUIDict->propertyForKey("EmbeddedThemeType");
      if (Prop && (Prop->isString()) && Prop->getString()->stringValue().notEmpty()) {
        settingsData.GUI.EmbeddedThemeType = Prop->getString()->stringValue();
        if (Prop->getString()->stringValue().isEqualIC("Dark")) {
//          settingsData.GUI.DarkEmbedded = TRUE;
          //ThemeX.Font = FONT_GRAY;
        } else if (Prop->getString()->stringValue().isEqualIC("Light")) {
//          settingsData.GUI.DarkEmbedded = FALSE;
          //ThemeX.Font = FONT_ALFA;
        } else if (Prop->getString()->stringValue().isEqualIC("Daytime")) {
//          settingsData.GUI.DarkEmbedded = !ThemeX.Daylight;
          //ThemeX.Font = ThemeX.Daylight?FONT_ALFA:FONT_GRAY;
        }
      }
      Prop = GUIDict->propertyForKey("PlayAsync"); //PlayAsync
      settingsData.GUI.PlayAsync = IsPropertyNotNullAndTrue(Prop);

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
      settingsData.GUI.ProvideConsoleGop = !IsPropertyNotNullAndFalse(Prop); //default is true

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
        settingsData.GUI.Language = Prop->getString()->stringValue();
        if ( Prop->getString()->stringValue().contains("en") ) {
          settingsData.GUI.languageCode = english;
//          settingsData.GUI.Codepage = 0xC0;
//          settingsData.GUI.CodepageSize = 0;
        } else if ( Prop->getString()->stringValue().contains("ru")) {
          settingsData.GUI.languageCode = russian;
//          settingsData.GUI.Codepage = 0x410;
//          settingsData.GUI.CodepageSize = 0x40;
        } else if ( Prop->getString()->stringValue().contains("ua")) {
          settingsData.GUI.languageCode = ukrainian;
//          settingsData.GUI.Codepage = 0x400;
//          settingsData.GUI.CodepageSize = 0x60;
        } else if ( Prop->getString()->stringValue().contains("fr")) {
          settingsData.GUI.languageCode = french; //default is extended latin
        } else if ( Prop->getString()->stringValue().contains("it")) {
          settingsData.GUI.languageCode = italian;
        } else if ( Prop->getString()->stringValue().contains("es")) {
          settingsData.GUI.languageCode = spanish;
        } else if ( Prop->getString()->stringValue().contains("pt")) {
          settingsData.GUI.languageCode = portuguese;
        } else if ( Prop->getString()->stringValue().contains("br")) {
          settingsData.GUI.languageCode = brasil;
        } else if ( Prop->getString()->stringValue().contains("de")) {
          settingsData.GUI.languageCode = german;
        } else if ( Prop->getString()->stringValue().contains("nl")) {
          settingsData.GUI.languageCode = dutch;
        } else if ( Prop->getString()->stringValue().contains("pl")) {
          settingsData.GUI.languageCode = polish;
        } else if ( Prop->getString()->stringValue().contains("cz")) {
          settingsData.GUI.languageCode = czech;
        } else if ( Prop->getString()->stringValue().contains("hr")) {
          settingsData.GUI.languageCode = croatian;
        } else if ( Prop->getString()->stringValue().contains("id")) {
          settingsData.GUI.languageCode = indonesian;
        } else if ( Prop->getString()->stringValue().contains("zh_CN")) {
          settingsData.GUI.languageCode = chinese;
//          settingsData.GUI.Codepage = 0x3400;
//          settingsData.GUI.CodepageSize = 0x19C0;
        } else if ( Prop->getString()->stringValue().contains("ro")) {
          settingsData.GUI.languageCode = romanian;
        } else if ( Prop->getString()->stringValue().contains("ko")) {
          settingsData.GUI.languageCode = korean;
//          settingsData.GUI.Codepage = 0x1100;
//          settingsData.GUI.CodepageSize = 0x100;
        }
      }

//      if (settingsData.Language != NULL) { // settingsData.Language != NULL cannot be false because settingsData.Language is dclared as CHAR8 Language[16]; Must we replace by settingsData.Language[0] != NULL
      Prop = GUIDict->propertyForKey("KbdPrevLang");
      if (Prop != NULL) {
        settingsData.GUI.KbdPrevLang = IsPropertyNotNullAndTrue(Prop);
      }
//      }

      const TagDict* MouseDict = GUIDict->dictPropertyForKey("Mouse");
      if (MouseDict != NULL) {
        const TagStruct* prop = MouseDict->propertyForKey("Speed");
        if (prop != NULL) {
          settingsData.GUI.Mouse.PointerSpeed = (INT32)GetPropertyAsInteger(prop, 0);
          settingsData.GUI.Mouse.PointerEnabled = (settingsData.GUI.Mouse.PointerSpeed != 0);
        }
        //but we can disable mouse even if there was positive speed
        prop = MouseDict->propertyForKey("Enabled");
        if (IsPropertyNotNullAndFalse(prop)) {
          settingsData.GUI.Mouse.PointerEnabled = FALSE;
        }

        prop = MouseDict->propertyForKey("Mirror");
        if (IsPropertyNotNullAndTrue(prop)) {
          settingsData.GUI.Mouse.PointerMirror = TRUE;
        }

        prop = MouseDict->propertyForKey("DoubleClickTime");
        if (prop != NULL) {
          settingsData.GUI.Mouse.DoubleClickTime = (UINT64)GetPropertyAsInteger(prop, 500);
        }
      }
      // hide by name/uuid. Array of string
      const TagArray* HideArray = GUIDict->arrayPropertyForKey("Hide");
      if (HideArray != NULL) {
        INTN   i;
        INTN   Count = HideArray->arrayContent().size();
        if (Count > 0) {
          settingsData.GUI.HVHideStrings.setEmpty();
          for (i = 0; i < Count; i++) {
            const TagStruct* prop2 = &HideArray->arrayContent()[i];
            if ( !prop2->isString()) {
              MsgLog("MALFORMED PLIST : Hide must be an array of string");
              continue;
            }
            if ( prop2->getString()->stringValue().notEmpty() ) {
              settingsData.GUI.HVHideStrings.Add(prop2->getString()->stringValue());
              DBG("Hiding entries with string %s\n", prop2->getString()->stringValue().c_str());
            }
          }
        }
      }
      settingsData.GUI.Scan.LinuxScan = TRUE;
      // Disable loader scan
      Prop = GUIDict->propertyForKey("Scan");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndFalse(Prop)) {
          settingsData.GUI.Scan.DisableEntryScan = TRUE;
          settingsData.GUI.Scan.DisableToolScan  = TRUE;
          settingsData.GUI.Scan.NoLegacy      = TRUE;
        } else if (Prop->isDict()) {
          const TagStruct* prop2 = Prop->getDict()->propertyForKey("Entries");
          if (IsPropertyNotNullAndFalse(prop2)) {
            settingsData.GUI.Scan.DisableEntryScan = TRUE;
          }
          prop2 = Prop->getDict()->propertyForKey("Tool");
          if (IsPropertyNotNullAndFalse(prop2)) {
            settingsData.GUI.Scan.DisableToolScan = TRUE;
          }
          prop2 = Prop->getDict()->propertyForKey("Linux");
          settingsData.GUI.Scan.LinuxScan = !IsPropertyNotNullAndFalse(prop2);
          prop2 = Prop->getDict()->propertyForKey("Legacy");
          if (prop2 != NULL) {
            if (prop2->isFalse()) {
              settingsData.GUI.Scan.NoLegacy = TRUE;
            } else if ((prop2->isString()) && prop2->getString()->stringValue().notEmpty() ) {
              if ((prop2->getString()->stringValue()[0] == 'N') || (prop2->getString()->stringValue()[0] == 'n')) {
                settingsData.GUI.Scan.NoLegacy = TRUE;
              } else if ((prop2->getString()->stringValue()[0] == 'F') || (prop2->getString()->stringValue()[0] == 'f')) {
                settingsData.GUI.Scan.LegacyFirst = TRUE;
               }
            }
          }
          prop2 = Prop->getDict()->propertyForKey("Kernel");
          if (prop2 != NULL) {
            if (prop2->isFalse()) {
              settingsData.GUI.Scan.KernelScan = KERNEL_SCAN_NONE;
            } else if ((prop2->isString()) && prop2->getString()->stringValue().notEmpty() ) {
              if ((prop2->getString()->stringValue()[0] == 'N') || (prop2->getString()->stringValue()[0] == 'n')) {
                settingsData.GUI.Scan.KernelScan = ( prop2->getString()->stringValue().length() > 1  &&  (prop2->getString()->stringValue()[1] == 'E' || prop2->getString()->stringValue()[1] == 'e') ) ? KERNEL_SCAN_NEWEST : KERNEL_SCAN_NONE;
              } else if ((prop2->getString()->stringValue()[0] == 'O') || (prop2->getString()->stringValue()[0] == 'o')) {
                settingsData.GUI.Scan.KernelScan = KERNEL_SCAN_OLDEST;
              } else if ((prop2->getString()->stringValue()[0] == 'F') || (prop2->getString()->stringValue()[0] == 'f')) {
                settingsData.GUI.Scan.KernelScan = KERNEL_SCAN_FIRST;
              } else if ((prop2->getString()->stringValue()[0] == 'L') || (prop2->getString()->stringValue()[0] == 'l')) {
                settingsData.GUI.Scan.KernelScan = KERNEL_SCAN_LAST;
              } else if ((prop2->getString()->stringValue()[0] == 'M') || (prop2->getString()->stringValue()[0] == 'm')) {
                settingsData.GUI.Scan.KernelScan = KERNEL_SCAN_MOSTRECENT;
              } else if ((prop2->getString()->stringValue()[0] == 'E') || (prop2->getString()->stringValue()[0] == 'e')) {
                settingsData.GUI.Scan.KernelScan = KERNEL_SCAN_EARLIEST;
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
              CUSTOM_LOADER_ENTRY_SETTINGS* Entry = new CUSTOM_LOADER_ENTRY_SETTINGS;
              // Fill it in
              if ( FillinCustomEntry(Entry, Dict3, FALSE, settingsData) ) {
                settingsData.GUI.CustomEntriesSettings.AddReference(Entry, true);
              }else{
                delete Entry;
              }
            }
          }
        }
        const TagArray* LegacyArray = CustomDict2->arrayPropertyForKey("Legacy"); // is an array of dict
        if (LegacyArray != NULL) {
          CUSTOM_LEGACY_ENTRY_SETTINGS *Entry;
          INTN   i;
          INTN   Count = LegacyArray->arrayContent().size();
          if (Count > 0) {
            for (i = 0; i < Count; i++) {
              const TagDict* Dict3 = LegacyArray->dictElementAt(i, "Legacy"_XS8);
              // Allocate an entry
              Entry = new CUSTOM_LEGACY_ENTRY_SETTINGS;
              // Fill it in
              if ( FillingCustomLegacy(Entry, Dict3) ) {
                settingsData.GUI.CustomLegacySettings.AddReference(Entry, true);
              }else{
                delete Entry;
              }
            }
          }
        }
        const TagArray* ToolArray = CustomDict2->arrayPropertyForKey("Tool"); // is an array of dict
        if (ToolArray != NULL) {
          INTN   i;
          INTN   Count = ToolArray->arrayContent().size();
          if (Count > 0) {
            for (i = 0; i < Count; i++) {
              const TagDict* Dict3 = ToolArray->dictElementAt(i, "Tool"_XS8);
              // Allocate an entry
              CUSTOM_TOOL_ENTRY_SETTINGS* Entry = new CUSTOM_TOOL_ENTRY_SETTINGS;
              // Fill it in
              if ( FillingCustomTool(Entry, Dict3) ) {
                settingsData.GUI.CustomToolSettings.AddReference(Entry, true);
              }else{
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
      settingsData.Graphics.PatchVBios              = IsPropertyNotNullAndTrue(Prop);

      settingsData.Graphics.PatchVBiosBytes.setEmpty();

      const TagArray* Dict2 = GraphicsDict->arrayPropertyForKey("PatchVBiosBytes"); // array of dict
      if (Dict2 != NULL) {
        INTN   Count = Dict2->arrayContent().size();
        if (Count > 0) {
          UINTN             FindSize    = 0;
          UINTN             ReplaceSize = 0;
          BOOLEAN           Valid;
          // alloc space for up to 16 entries
//          settingsData.Graphics.PatchVBiosBytes = (__typeof__(settingsData.Graphics.PatchVBiosBytes))AllocateZeroPool(Count * sizeof(VBIOS_PATCH_BYTES));

          // get all entries
          for (INTN i = 0; i < Count; i++) {
            const TagDict* dict3 = Dict2->dictElementAt(i, "Graphics/PatchVBiosBytes"_XS8);
            Valid = TRUE;
            // read entry
            VBIOS_PATCH* VBiosPatchPtr = new VBIOS_PATCH;
            VBIOS_PATCH& VBiosPatch = *VBiosPatchPtr;
//            VBiosPatch          = &settingsData.Graphics.PatchVBiosBytes[settingsData.Graphics.PatchVBiosBytesCount];
            UINT8* DataSetting = GetDataSetting (dict3, "Find",    &FindSize);
            VBiosPatch.Find.stealValueFrom(DataSetting, FindSize);
            DataSetting = GetDataSetting (dict3, "Replace", &ReplaceSize);
            VBiosPatch.Replace.stealValueFrom(DataSetting, ReplaceSize);

            if ( VBiosPatch.Find.size() == 0 ) {
              Valid = FALSE;
              DBG("PatchVBiosBytes[%lld]: missing Find data\n", i);
            }

            if ( VBiosPatch.Replace.size() == 0 ) {
              Valid = FALSE;
              DBG("PatchVBiosBytes[%lld]: missing Replace data\n", i);
            }

            if (VBiosPatch.Find.size() != VBiosPatch.Replace.size()) {
              Valid = FALSE;
              DBG("PatchVBiosBytes[%lld]: Find and Replace data are not the same size\n", i);
            }

            if (Valid) {
//              VBiosPatch->NumberOfBytes = FindSize;
              // go to next entry
//              ++settingsData.Graphics.PatchVBiosBytesCount;
              settingsData.Graphics.PatchVBiosBytes.AddReference(VBiosPatchPtr, true);
            } else {
              // error - release mem
              delete VBiosPatchPtr;
//              if (VBiosPatch->Find != NULL) {
//                FreePool(VBiosPatch->Find);
//                VBiosPatch->Find = NULL;
//              }
//              if (VBiosPatch->Replace != NULL) {
//                FreePool(VBiosPatch->Replace);
//                VBiosPatch->Replace = NULL;
//              }
            }
          }

//          if (settingsData.Graphics.PatchVBiosBytesCount == 0) {
//            FreePool(settingsData.Graphics.PatchVBiosBytes);
//            settingsData.Graphics.PatchVBiosBytes = NULL;
//          }
        }
      }

      GetEDIDSettings(GraphicsDict, settingsData);
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
          if ( Prop->getString()->stringValue().notEmpty() )
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
        settingsData.Devices.Audio.ResetHDA = IsPropertyNotNullAndTrue(Prop);
      }
    }

    const TagDict* RtVariablesDict = CfgDict->dictPropertyForKey("RtVariables");
    if (RtVariablesDict != NULL) {
      const TagStruct* Prop = RtVariablesDict->propertyForKey("ROM");
      if (Prop != NULL) {
        if ( Prop->isString()  &&  Prop->getString()->stringValue().isEqualIC("UseMacAddr0") ) {
          settingsData.RtVariables.RtROMAsString = Prop->getString()->stringValue();
        } else if ( Prop->isString()  &&  Prop->getString()->stringValue().isEqualIC("UseMacAddr1") ) {
          settingsData.RtVariables.RtROMAsString = Prop->getString()->stringValue();
        } else if ( Prop->isString()  ||  Prop->isData() ) { // GetDataSetting accept both
          UINTN ROMLength         = 0;
          uint8_t* ROM = GetDataSetting(RtVariablesDict, "ROM", &ROMLength);
          settingsData.RtVariables.RtROMAsData.stealValueFrom(ROM, ROMLength);
        } else {
          MsgLog("MALFORMED PLIST : property not string or data in RtVariables/ROM\n");
        }
      }
    }

    settingsData.Quirks.mmioWhiteListArray.setEmpty();
 //   const TagDict* OcQuirksDict = CfgDict->dictPropertyForKey("OcQuirks");
//if ( OcQuirksDict ) panic("config.plist/OcQuirks has been renamed Quirks. Update your config.plist");

    const TagDict* OcQuirksDict = CfgDict->dictPropertyForKey("Quirks");
//if ( !OcQuirksDict ) panic("Cannot find config.plist/Quirks");
    if (OcQuirksDict != NULL) {
      const TagStruct* Prop;
      Prop               = OcQuirksDict->propertyForKey("AvoidRuntimeDefrag");
//if ( !Prop ) panic("Cannot find AvoidRuntimeDefrag in OcQuirks under root (OC booter quirks)");
      settingsData.Quirks.OcBooterQuirks.AvoidRuntimeDefrag = !IsPropertyNotNullAndFalse(Prop); //true if absent so no panic
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.AvoidRuntimeDefrag? QUIRK_DEFRAG:0;
      Prop               = OcQuirksDict->propertyForKey( "DevirtualiseMmio");
      settingsData.Quirks.OcBooterQuirks.DevirtualiseMmio   = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.DevirtualiseMmio? QUIRK_MMIO:0;
      Prop               = OcQuirksDict->propertyForKey( "DisableSingleUser");
      settingsData.Quirks.OcBooterQuirks.DisableSingleUser  = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.DisableSingleUser? QUIRK_SU:0;
      Prop               = OcQuirksDict->propertyForKey( "DisableVariableWrite");
      settingsData.Quirks.OcBooterQuirks.DisableVariableWrite = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.DisableVariableWrite? QUIRK_VAR:0;
      Prop               = OcQuirksDict->propertyForKey( "DiscardHibernateMap");
      settingsData.Quirks.OcBooterQuirks.DiscardHibernateMap = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.DiscardHibernateMap? QUIRK_HIBER:0;
      Prop               = OcQuirksDict->propertyForKey( "EnableSafeModeSlide");
      settingsData.Quirks.OcBooterQuirks.EnableSafeModeSlide = !IsPropertyNotNullAndFalse(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.EnableSafeModeSlide? QUIRK_SAFE:0;
      Prop               = OcQuirksDict->propertyForKey( "EnableWriteUnprotector");
      settingsData.Quirks.OcBooterQuirks.EnableWriteUnprotector = !IsPropertyNotNullAndFalse(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.EnableWriteUnprotector? QUIRK_UNPROT:0;
      Prop               = OcQuirksDict->propertyForKey( "ForceExitBootServices");
      settingsData.Quirks.OcBooterQuirks.ForceExitBootServices = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.ForceExitBootServices? QUIRK_EXIT:0;
      Prop               = OcQuirksDict->propertyForKey( "ProtectMemoryRegions");
      settingsData.Quirks.OcBooterQuirks.ProtectMemoryRegions = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.ProtectMemoryRegions? QUIRK_REGION:0;
      Prop               = OcQuirksDict->propertyForKey( "ProtectSecureBoot");
      settingsData.Quirks.OcBooterQuirks.ProtectSecureBoot = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.ProtectSecureBoot? QUIRK_SECURE:0;
      Prop               = OcQuirksDict->propertyForKey( "ProtectUefiServices");
      settingsData.Quirks.OcBooterQuirks.ProtectUefiServices = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.ProtectUefiServices? QUIRK_UEFI:0;
      //it is in GUI section
//      Prop               = OcQuirksDict->propertyForKey( "ProvideConsoleGopEnable");
//      settingsData.ProvideConsoleGop = !IsPropertyNotNullAndFalse(Prop);
      Prop               = OcQuirksDict->propertyForKey( "ProvideCustomSlide");
      settingsData.Quirks.OcBooterQuirks.ProvideCustomSlide = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.ProvideCustomSlide? QUIRK_CUSTOM:0;
      Prop               = OcQuirksDict->propertyForKey( "ProvideMaxSlide");
      settingsData.Quirks.OcBooterQuirks.ProvideMaxSlide = (UINT8)GetPropertyAsInteger(Prop, 0); // cast will be safe when the new parser will ensure that the value is UINT8
      Prop               = OcQuirksDict->propertyForKey( "RebuildAppleMemoryMap");
      settingsData.Quirks.OcBooterQuirks.RebuildAppleMemoryMap = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.RebuildAppleMemoryMap? QUIRK_MAP:0;
      Prop               = OcQuirksDict->propertyForKey( "SetupVirtualMap");
      settingsData.Quirks.OcBooterQuirks.SetupVirtualMap = !IsPropertyNotNullAndFalse(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.SetupVirtualMap? QUIRK_VIRT:0;
      Prop               = OcQuirksDict->propertyForKey( "SignalAppleOS");
      settingsData.Quirks.OcBooterQuirks.SignalAppleOS = IsPropertyNotNullAndTrue(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.SignalAppleOS? QUIRK_OS:0;
      Prop               = OcQuirksDict->propertyForKey( "SyncRuntimePermissions");
      settingsData.Quirks.OcBooterQuirks.SyncRuntimePermissions = !IsPropertyNotNullAndFalse(Prop);
      settingsData.Quirks.QuirksMask  |= settingsData.Quirks.OcBooterQuirks.SyncRuntimePermissions? QUIRK_PERM:0;
      settingsData.Quirks.mmioWhiteListArray.setEmpty();

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
            SETTINGS_DATA::QuirksClass::MMIOWhiteList* mmioWhiteListPtr = new SETTINGS_DATA::QuirksClass::MMIOWhiteList();
            SETTINGS_DATA::QuirksClass::MMIOWhiteList& mmioWhiteList = *mmioWhiteListPtr;

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
            settingsData.Quirks.mmioWhiteListArray.AddReference(mmioWhiteListPtr, true);
          }
        }
      }

      Prop = OcQuirksDict->propertyForKey("FuzzyMatch");
      if (Prop != NULL || GlobalConfig.gBootChanged) {
        settingsData.Quirks.FuzzyMatch = !IsPropertyNotNullAndFalse(Prop);
      }

      Prop = OcQuirksDict->propertyForKey("KernelCache");
      if (Prop != NULL || GlobalConfig.gBootChanged) {
        if ( Prop->isString() ) {
          if ( Prop->getString()->stringValue().notEmpty() ) {
            settingsData.Quirks.OcKernelCache = Prop->getString()->stringValue();
          }else{
            settingsData.Quirks.OcKernelCache = "Auto"_XS8;
          }
        }else{
          MsgLog("MALFORMED PLIST : Quirks/KernelCache must be a string");
          settingsData.Quirks.OcKernelCache = "Auto"_XS8;
        }
      }


      // Booter Quirks
//      Prop = OcQuirksDict->propertyForKey("AppleCpuPmCfgLock");
//      settingsData.KernelAndKextPatches.OcKernelQuirks.AppleCpuPmCfgLock = IsPropertyNotNullAndTrue(Prop);
//      settingsData.Quirks.OcKernelQuirks.AppleCpuPmCfgLock = settingsData.KernelAndKextPatches.KPAppleIntelCPUPM || GlobalConfig.NeedPMfix;

//      Prop = OcQuirksDict->propertyForKey("AppleXcpmCfgLock"); //
//      settingsData.KernelAndKextPatches.OcKernelQuirks.AppleXcpmCfgLock = IsPropertyNotNullAndTrue(Prop);
//      settingsData.Quirks.OcKernelQuirks.AppleXcpmCfgLock = GlobalConfig.KPKernelPm || GlobalConfig.NeedPMfix;

      Prop = OcQuirksDict->propertyForKey("AppleXcpmExtraMsrs");
      settingsData.Quirks.OcKernelQuirks.AppleXcpmExtraMsrs = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("AppleXcpmForceBoost");
      settingsData.Quirks.OcKernelQuirks.AppleXcpmForceBoost = IsPropertyNotNullAndTrue(Prop);

// We can't use that Quirks because we don't delegate SMBios to OC.
//      Prop = OcQuirksDict->propertyForKey("CustomSMBIOSGuid");
//      settingsData.KernelAndKextPatches.OcKernelQuirks.CustomSmbiosGuid = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("DisableIoMapper");
//if ( !Prop ) panic("Cannot find DisableIoMapper in config.plist/Quirks. You forgot to merge your quirks into one section. Update your config.plist");
      settingsData.Quirks.OcKernelQuirks.DisableIoMapper = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("DisableLinkeditJettison");
      settingsData.Quirks.OcKernelQuirks.DisableLinkeditJettison = IsPropertyNotNullAndTrue(Prop);

 //     Prop = OcQuirksDict->propertyForKey("DisableRtcChecksum");
 //     settingsData.KernelAndKextPatches.OcKernelQuirks.DisableRtcChecksum = IsPropertyNotNullAndTrue(Prop);
//      settingsData.Quirks.OcKernelQuirks.DisableRtcChecksum = settingsData.KernelAndKextPatches.KPAppleRTC;

      Prop = OcQuirksDict->propertyForKey("DummyPowerManagement");
      settingsData.Quirks.OcKernelQuirks.DummyPowerManagement = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("ExternalDiskIcons");
      settingsData.Quirks.OcKernelQuirks.ExternalDiskIcons = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("IncreasePciBarSize");
      settingsData.Quirks.OcKernelQuirks.IncreasePciBarSize = IsPropertyNotNullAndTrue(Prop);

 //     Prop = OcQuirksDict->propertyForKey("LapicKernelPanic");
 //     settingsData.KernelAndKextPatches.OcKernelQuirks.LapicKernelPanic = IsPropertyNotNullAndTrue(Prop);
//      settingsData.Quirks.OcKernelQuirks.LapicKernelPanic = settingsData.KernelAndKextPatches.KPKernelLapic;

//      Prop = OcQuirksDict->propertyForKey("PanicNoKextDump");
//      settingsData.KernelAndKextPatches.OcKernelQuirks.PanicNoKextDump = IsPropertyNotNullAndTrue(Prop); //KPPanicNoKextDump
//      settingsData.Quirks.OcKernelQuirks.PanicNoKextDump = settingsData.KernelAndKextPatches.KPPanicNoKextDump;

      Prop = OcQuirksDict->propertyForKey("PowerTimeoutKernelPanic");
      settingsData.Quirks.OcKernelQuirks.PowerTimeoutKernelPanic = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("ThirdPartyDrives");
      settingsData.Quirks.OcKernelQuirks.ThirdPartyDrives = IsPropertyNotNullAndTrue(Prop);

      Prop = OcQuirksDict->propertyForKey("XhciPortLimit");
      settingsData.Quirks.OcKernelQuirks.XhciPortLimit = IsPropertyNotNullAndTrue(Prop);
    }
  }

  return Status;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

void
ParseSMBIOSSettings(SETTINGS_DATA& settingsData, const TagDict* DictPointer)
{
#pragma GCC diagnostic pop

  const TagStruct* Prop;
  const TagStruct* Prop1;
  
  
  // TODO!!!
  BOOLEAN Default = FALSE;


  Prop = DictPointer->propertyForKey("ProductName");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in ProductName\n");
    }else{
 DBG("ProductName SETTINGS_DATA=%s\n", settingsData.Smbios.ProductName.c_str());
 DBG("ProductName setting=%s\n", Prop->getString()->stringValue().c_str());
      MACHINE_TYPES Model;
      settingsData.Smbios.ProductName = Prop->getString()->stringValue();
 DBG("ProductName new SETTINGS_DATA=%s\n", settingsData.Smbios.ProductName.c_str());
      // let's fill all other fields based on this ProductName
      // to serve as default
      Model = GetModelFromString(settingsData.Smbios.ProductName);
      if (Model != MaxMachineType) {
 DBG("SetDMISettingsForModel=%d\n", Model);
        SetDMISettingsForModel(settingsData, Model, FALSE);
        GlobalConfig.CurrentModel = Model;
        Default = TRUE;
      } else {
        //if new model then fill at least as iMac13,2, except custom ProductName
        // something else?
        SetDMISettingsForModel(settingsData, iMac132, FALSE);
        GlobalConfig.CurrentModel = MaxMachineType;
      }
    }
  }
  DBG("Using ProductName : %s\n", settingsData.Smbios.ProductName.c_str());

  Prop = DictPointer->propertyForKey("SmbiosVersion");
  settingsData.Smbios.SmbiosVersion = (UINT16)GetPropertyAsInteger(Prop, 0x204);

  // Check for BiosVersion and BiosReleaseDate by Sherlocks
  Prop = DictPointer->propertyForKey("BiosVersion");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      DBG("BiosVersion: not set, Using BiosVersion from clover\n");
    }else{
      settingsData.Smbios._RomVersion = Prop->getString()->stringValue();

      const CHAR8* i = GlobalConfig.RomVersionUsed.c_str();
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
            GlobalConfig.RomVersionUsed = Prop->getString()->stringValue();
            DBG("Using latest BiosVersion from config\n");
          }
        } else {
          GlobalConfig.RomVersionUsed = Prop->getString()->stringValue();
          DBG("Using latest BiosVersion from config\n");
        }
      } else {
        GlobalConfig.RomVersionUsed = Prop->getString()->stringValue();
        DBG("Using latest BiosVersion from config\n");
      }
    }
  } else {
    DBG("BiosVersion: not set, Using BiosVersion from clover\n");
  }
  DBG("BiosVersion: %s\n", GlobalConfig.RomVersionUsed.c_str());

  Prop1 = DictPointer->propertyForKey("BiosReleaseDate");
  if (Prop1 != NULL) {
    if ( !Prop1->isString() ) {
      MsgLog("ATTENTION : property not string in BiosReleaseDate\n");
    }else{
      settingsData.Smbios._ReleaseDate = Prop1->getString()->stringValue();
      if (Prop != NULL) { // Prop is BiosVersion
        const CHAR8* i = GlobalConfig.ReleaseDateUsed.c_str();
        const CHAR8* j = settingsData.Smbios._ReleaseDate.c_str();

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
                GlobalConfig.ReleaseDateUsed = settingsData.Smbios._ReleaseDate;
                //DBG("Using latest BiosReleaseDate from config\n");
              }
            } else {
              GlobalConfig.ReleaseDateUsed = settingsData.Smbios._ReleaseDate;
              //DBG("Using latest BiosReleaseDate from config\n");
            }
          } else {
            GlobalConfig.ReleaseDateUsed = settingsData.Smbios._ReleaseDate;
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
                GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
                //DBG("Using latest BiosReleaseDate from config\n");
              }
            } else {
              GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
              //DBG("Using latest BiosReleaseDate from config\n");
            }
          } else {
            GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/%c%c\n", j[0], j[1], j[3], j[4], j[8], j[9]);
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
                GlobalConfig.ReleaseDateUsed = Prop1->getString()->stringValue();
                //DBG("Using latest BiosReleaseDate from config\n");
              }
            } else {
              GlobalConfig.ReleaseDateUsed = Prop1->getString()->stringValue();
              //DBG("Using latest BiosReleaseDate from config\n");
            }
          } else {
            GlobalConfig.ReleaseDateUsed = Prop1->getString()->stringValue();
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
                GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
                //DBG("Using latest BiosReleaseDate from config\n");
              }
            } else {
              GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
              //DBG("Using latest BiosReleaseDate from config\n");
            }
          } else {
            GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/20%c%c\n", j[0], j[1], j[3], j[4], j[6], j[7]);
            //DBG("Using latest BiosReleaseDate from config\n");
          }
        } else {
          //DBG("Found unknown date format from config\n");
          if (Prop != NULL) {
            i = GlobalConfig.ReleaseDateUsed.c_str();
            j = GlobalConfig.RomVersionUsed.c_str();

            j += AsciiStrLen(j);
            while (*j != '.') {
              j--;
            }

            if ((AsciiStrLen(i) == 8)) {
              GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
              //DBG("Using the date of used BiosVersion\n");
            } else if ((AsciiStrLen(i) == 10)) {
              GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/20%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
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
      const CHAR8* i = GlobalConfig.ReleaseDateUsed.c_str();
      const CHAR8* j = GlobalConfig.RomVersionUsed.c_str();

      j += AsciiStrLen(j);
      while (*j != '.') {
        j--;
      }

      if ((AsciiStrLen(i) == 8)) {
        GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
        //DBG("BiosReleaseDate: not set, Using the date of used BiosVersion\n");
      } else if ((AsciiStrLen(i) == 10)) {
        GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/20%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
        //DBG("BiosReleaseDate: not set, Using the date of used BiosVersion\n");
      }
    } else {
      //DBG("BiosReleaseDate: not set, Using BiosReleaseDate from clover\n");
    }
  }
  DBG("BiosReleaseDate: %s\n", GlobalConfig.ReleaseDateUsed.c_str());

  Prop = DictPointer->propertyForKey("EfiVersion");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in EfiVersion\n");
      if ( GlobalConfig.EfiVersionUsed.notEmpty() ) {
        DBG("Using EfiVersion from clover: %s\n", GlobalConfig.EfiVersionUsed.c_str());
      }
    }else{
      settingsData.Smbios._EfiVersion = Prop->getString()->stringValue();
      settingsData.Smbios._EfiVersion.trim();
      if (AsciiStrVersionToUint64(GlobalConfig.EfiVersionUsed, 4, 5) > AsciiStrVersionToUint64(settingsData.Smbios._EfiVersion, 4, 5)) {
        DBG("Using latest EfiVersion from clover: %s\n", GlobalConfig.EfiVersionUsed.c_str());
      } else if (AsciiStrVersionToUint64(GlobalConfig.EfiVersionUsed, 4, 5) < AsciiStrVersionToUint64(settingsData.Smbios._EfiVersion, 4, 5)) {
        GlobalConfig.EfiVersionUsed = settingsData.Smbios._EfiVersion;
        DBG("Using latest EfiVersion from config: %s\n", GlobalConfig.EfiVersionUsed.c_str());
      } else {
        DBG("Using EfiVersion from clover: %s\n", GlobalConfig.EfiVersionUsed.c_str());
      }
    }
  } else if ( GlobalConfig.EfiVersionUsed.notEmpty() ) {
    DBG("Using EfiVersion from clover: %s\n", GlobalConfig.EfiVersionUsed.c_str());
  }

  Prop = DictPointer->propertyForKey("FirmwareFeatures");
  if (Prop != NULL) {
    settingsData.Smbios.gFwFeatures = (UINT32)GetPropertyAsInteger(Prop, settingsData.Smbios.gFwFeatures);
    DBG("Using FirmwareFeatures from config: 0x%08X\n", settingsData.Smbios.gFwFeatures);
  } else {
    DBG("Using FirmwareFeatures from clover: 0x%08X\n", settingsData.Smbios.gFwFeatures);
  }

  Prop = DictPointer->propertyForKey("FirmwareFeaturesMask");
  if (Prop != NULL) {
    settingsData.Smbios.gFwFeaturesMask = (UINT32)GetPropertyAsInteger(Prop, settingsData.Smbios.gFwFeaturesMask);
    DBG("Using FirmwareFeaturesMask from config: 0x%08X\n", settingsData.Smbios.gFwFeaturesMask);
  } else {
    DBG("Using FirmwareFeaturesMask from clover: 0x%08X\n", settingsData.Smbios.gFwFeaturesMask);
  }

  Prop = DictPointer->propertyForKey("PlatformFeature");
  if (Prop != NULL) {
    settingsData.Smbios.gPlatformFeature = (UINT64)GetPropertyAsInteger(Prop, (INTN)settingsData.Smbios.gPlatformFeature);
  } else {
    if (settingsData.Smbios.gPlatformFeature == 0xFFFF) {
      DBG("PlatformFeature will not set in SMBIOS\n");
    } else {
DBG("Using PlatformFeature from clover: 0x%llX\n", settingsData.Smbios.gPlatformFeature);
    }
  }

  Prop = DictPointer->propertyForKey("BiosVendor");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in BiosVendor\n");
    }else{
      settingsData.Smbios.BiosVendor = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("Manufacturer");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in Manufacturer\n");
    }else{
      settingsData.Smbios.ManufactureName = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("Version");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in Version\n");
    }else{
      settingsData.Smbios.VersionNr = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("Family");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in Family\n");
    }else{
      settingsData.Smbios.FamilyName = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("SerialNumber");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in SerialNumber\n");
    }else{
      settingsData.Smbios.SerialNr = Prop->getString()->stringValue();
DBG("settingsData.Smbios.SerialNr: %s\n", settingsData.Smbios.SerialNr.c_str());
    }
  }

  Prop = DictPointer->propertyForKey("SmUUID");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in SmUUID\n");
    }else{
      if (IsValidGuidString(Prop->getString()->stringValue())) {
        settingsData.Smbios.SmUUID = Prop->getString()->stringValue();
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
        settingsData.Smbios.BoardManufactureName = Prop->getString()->stringValue();
      }
    }
  }

  Prop = DictPointer->propertyForKey("BoardSerialNumber");
  if ( Prop != NULL ) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in BoardSerialNumber\n");
    }else{
      if( Prop->getString()->stringValue().notEmpty() ) {
        settingsData.Smbios.BoardSerialNumber = Prop->getString()->stringValue();
      }
    }
  }

  Prop = DictPointer->propertyForKey("Board-ID");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in Board-ID\n");
    }else{
      settingsData.Smbios.BoardNumber = Prop->getString()->stringValue();
      DBG("Board-ID set from config as %s\n", settingsData.Smbios.BoardNumber.c_str());
    }
  }

  if (!Default) {
    settingsData.Smbios.BoardVersion = settingsData.Smbios.ProductName;
  }
  Prop = DictPointer->propertyForKey("BoardVersion");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in BoardVersion\n");
    }else{
      settingsData.Smbios.BoardVersion = Prop->getString()->stringValue();
DBG("%s : BoardVersion: %s\n",__PRETTY_FUNCTION__, settingsData.Smbios.BoardVersion.c_str());
    }
  }

  Prop = DictPointer->propertyForKey("BoardType");
  if (Prop != NULL) {
    settingsData.Smbios.BoardType = (UINT8)GetPropertyAsInteger(Prop, settingsData.Smbios.BoardType);
    DBG("BoardType: 0x%hhX\n", settingsData.Smbios.BoardType);
  }

  Prop = DictPointer->propertyForKey("Mobile");
  if (Prop != NULL) {
    if (IsPropertyNotNullAndFalse(Prop))
      settingsData.Smbios.Mobile = FALSE;
    else if (IsPropertyNotNullAndTrue(Prop))
      settingsData.Smbios.Mobile = TRUE;
  } else if (!Default) {
    settingsData.Smbios.Mobile = settingsData.Smbios.ProductName.contains("MacBook");
  }

  Prop = DictPointer->propertyForKey("LocationInChassis");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in LocationInChassis\n");
    }else{
      settingsData.Smbios.LocationInChassis = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("ChassisManufacturer");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in ChassisManufacturer\n");
    }else{
      settingsData.Smbios.ChassisManufacturer = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("ChassisAssetTag");
  if (Prop != NULL) {
    if ( !Prop->isString() ) {
      MsgLog("ATTENTION : property not string in ChassisAssetTag\n");
    }else{
      settingsData.Smbios.ChassisAssetTag = Prop->getString()->stringValue();
    }
  }

  Prop = DictPointer->propertyForKey("ChassisType");
  if (Prop != NULL) {
    settingsData.Smbios.ChassisType = (UINT8)GetPropertyAsInteger(Prop, settingsData.Smbios.ChassisType);
    DBG("ChassisType: 0x%hhX\n", settingsData.Smbios.ChassisType);
  }

  Prop = DictPointer->propertyForKey("NoRomInfo");
  if (Prop != NULL) {
    settingsData.Smbios.NoRomInfo = IsPropertyNotNullAndTrue(Prop);
  }
}

/*
 * To ease copy/paste and text replacement from GetUserSettings, the parameter has the same name as the global
 * and is passed by non-const reference.
 * This temporary during the refactoring
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
static void getACPISettings(const TagDict *CfgDict, SETTINGS_DATA& settingsData)
{
  #pragma GCC diagnostic pop
  const TagDict* ACPIDict = CfgDict->dictPropertyForKey("ACPI");
  if (ACPIDict) {
    const TagArray* DropTablesArray = ACPIDict->arrayPropertyForKey("DropTables"); // array of dict
    if (DropTablesArray) {
      INTN   i;
      INTN Count = DropTablesArray->arrayContent().size();
      
      if (Count > 0) {
//        DBG("Table to drop %lld tables:\n", Count);
        
        for (i = 0; i < Count; i++)
        {
          SETTINGS_DATA::ACPIClass::ACPIDropTablesClass* ACPIDropTables = new SETTINGS_DATA::ACPIClass::ACPIDropTablesClass;
//          UINT32 Signature = 0;
//          UINT32 TabLength = 0;
//          UINT64 TableId = 0;
//          BOOLEAN OtherOS = FALSE;
          
          const TagDict* Dict2 = DropTablesArray->dictElementAt(i, "ACPI/DropTables"_XS8);
          
//          DBG(" - [%02lld]: Drop table ", i);
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
            ACPIDropTables->Signature = SIGNATURE_32(s1, s2, s3, s4);
//            DBG(" signature=\"%c%c%c%c\" (%8.8X)\n", s1, s2, s3, s4, ACPIDropTables->Signature);
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
              
              CopyMem(&ACPIDropTables->TableId, (CHAR8*)&Id[0], 8);
//              DBG(" table-id=\"%s\" (%16.16llX)\n", Id, ACPIDropTables->TableId);
            }
          }
          // Get the table len to drop
          Prop2 = Dict2->propertyForKey("Length");
          if (Prop2 != NULL) {
            ACPIDropTables->TabLength = (UINT32)GetPropertyAsInteger(Prop2, 0);
//            DBG(" length=%d(0x%X)", ACPIDropTables->TabLength, ACPIDropTables->TabLength);
          }
          // Check if to drop for other OS as well
          Prop2 = Dict2->propertyForKey("DropForAllOS");
          if (Prop2 != NULL) {
            ACPIDropTables->OtherOS = IsPropertyNotNullAndTrue(Prop2);
          }
          settingsData.ACPI.ACPIDropTablesArray.AddReference(ACPIDropTables, true);
//          DBG("----\n");
          //set to drop
//          if (GlobalConfig.ACPIDropTables) {
//            ACPI_DROP_TABLE *DropTable = GlobalConfig.ACPIDropTables;
//            DBG("         - set table: %08X, %16llx to drop:", Signature, TableId);
//            Dropped = FALSE;
//            while (DropTable) {
//              if (((Signature == DropTable->Signature) &&
//                   (!TableId || (DropTable->TableId == TableId)) &&
//                   (!TabLength || (DropTable->Length == TabLength))) ||
//                  (!Signature && (DropTable->TableId == TableId))) {
//                DropTable->MenuItem.BValue = TRUE;
//                DropTable->OtherOS = OtherOS;
//                settingsData.ACPI.SSDT.DropSSDT         = FALSE; //if one item=true then dropAll=false by default
//                //DBG(" true");
//                Dropped = TRUE;
//              }
//              DropTable = DropTable->Next;
//            }
//            DBG(" %s\n", Dropped ? "yes" : "no");
//          }
        }
      }
    }
    
    const TagDict* DSDTDict = ACPIDict->dictPropertyForKey("DSDT");
    if (DSDTDict) {
      //settingsData.ACPI.DSDT.FixDsdt by default is "DSDT.aml", but name "BIOS" will mean autopatch
      const TagStruct* Prop = DSDTDict->propertyForKey("Name");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in DSDT/Name\n");
        }else{
          settingsData.ACPI.DSDT.DsdtName = Prop->getString()->stringValue();
        }
      }
      
      Prop = DSDTDict->propertyForKey("Debug");
      settingsData.ACPI.DSDT.DebugDSDT = IsPropertyNotNullAndTrue(Prop);
      
      Prop = DSDTDict->propertyForKey("Rtc8Allowed");
      settingsData.ACPI.DSDT.Rtc8Allowed = IsPropertyNotNullAndTrue(Prop);
      
      Prop = DSDTDict->propertyForKey("PNLF_UID");
      settingsData.ACPI.DSDT.PNLF_UID = (UINT8)GetPropertyAsInteger(Prop, 0x0A);
      
      Prop = DSDTDict->propertyForKey("FixMask");
      settingsData.ACPI.DSDT.FixDsdt = (UINT32)GetPropertyAsInteger(Prop, settingsData.ACPI.DSDT.FixDsdt);
      
      const TagDict* FixesDict = DSDTDict->dictPropertyForKey("Fixes");
      if (FixesDict != NULL) {
        UINTN Index;
        //         DBG("Fixes will override DSDT fix mask %08X!\n", settingsData.ACPI.DSDT.FixDsdt);
        settingsData.ACPI.DSDT.FixDsdt = 0;
        for (Index = 0; Index < sizeof(FixesConfig)/sizeof(FixesConfig[0]); Index++) {
          const TagStruct* Prop2 = FixesDict->propertyForKey(FixesConfig[Index].newName);
          if (!Prop2 && FixesConfig[Index].oldName) {
            Prop2 = FixesDict->propertyForKey(FixesConfig[Index].oldName);
          }
          if (IsPropertyNotNullAndTrue(Prop2)) {
            settingsData.ACPI.DSDT.FixDsdt |= FixesConfig[Index].bitData;
          }
        }
      }
      DBG(" - final DSDT Fix mask=%08X\n", settingsData.ACPI.DSDT.FixDsdt);
      
      const TagArray* PatchesArray = DSDTDict->arrayPropertyForKey("Patches"); // array of dict
      if (PatchesArray != NULL) {
        INTN   i;
        INTN Count = PatchesArray->arrayContent().size();
        if (Count > 0) {
//          settingsData.ACPI.DSDT.DSDTPatchArray.size()      = (UINT32)Count;
//          settingsData.PatchDsdtFind = (__typeof__(settingsData.PatchDsdtFind))AllocateZeroPool(Count * sizeof(UINT8*));
//          settingsData.PatchDsdtReplace = (__typeof__(settingsData.PatchDsdtReplace))AllocateZeroPool(Count * sizeof(UINT8*));
//          settingsData.PatchDsdtTgt = (__typeof__(settingsData.PatchDsdtTgt))AllocateZeroPool(Count * sizeof(UINT8*));
//          settingsData.LenToFind = (__typeof__(settingsData.LenToFind))AllocateZeroPool(Count * sizeof(UINT32));
//          settingsData.LenToReplace = (__typeof__(settingsData.LenToReplace))AllocateZeroPool(Count * sizeof(UINT32));
//          settingsData.PatchDsdtLabel = (__typeof__(settingsData.PatchDsdtLabel))AllocateZeroPool(Count * sizeof(UINT8*));
//          settingsData.PatchDsdtMenuItem = new INPUT_ITEM[Count];
          DBG("PatchesDSDT: %lld requested\n", Count);
          settingsData.ACPI.DSDT.DSDTPatchArray.setEmpty();
          for (i = 0; i < Count; i++)
          {
            SETTINGS_DATA::ACPIClass::DSDTClass::DSDT_Patch* dsdtPatchPtr = new SETTINGS_DATA::ACPIClass::DSDTClass::DSDT_Patch();
            SETTINGS_DATA::ACPIClass::DSDTClass::DSDT_Patch& dsdtPatch = *dsdtPatchPtr;
            UINTN Size = 0;
            const TagDict* Prop2 = PatchesArray->dictElementAt(i,"DSDT/Patches"_XS8);
            DBG(" - [%02lld]:", i);
            XString8 DSDTPatchesLabel;
            
            const TagStruct* Prop3 = Prop2->propertyForKey("Comment");
            if (Prop3 != NULL && (Prop3->isString()) && Prop3->getString()->stringValue().notEmpty()) {
              DSDTPatchesLabel = Prop3->getString()->stringValue();
            } else {
              DSDTPatchesLabel = "(NoLabel)"_XS8;
            }
            dsdtPatch.PatchDsdtLabel = DSDTPatchesLabel;
            DBG(" (%s)", dsdtPatch.PatchDsdtLabel.c_str());
            
            Prop3 = Prop2->propertyForKey("Disabled");
            dsdtPatch.Disabled = IsPropertyNotNullAndTrue(Prop3);
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
            settingsData.ACPI.DSDT.DSDTPatchArray.AddReference(dsdtPatchPtr, true);
          }
        } //if count > 0
      } //if prop PatchesDSDT
      
      Prop = DSDTDict->propertyForKey("ReuseFFFF");
      if (IsPropertyNotNullAndTrue(Prop)) {
        settingsData.ACPI.DSDT.ReuseFFFF = TRUE;
      }
      
      Prop = DSDTDict->propertyForKey("SuspendOverride");
      if (IsPropertyNotNullAndTrue(Prop)) {
        settingsData.ACPI.DSDT.SuspendOverride = TRUE;
      }
      /*
       Prop   = GetProperty(Dict2, "DropOEM_DSM");
       defDSM = FALSE;
       
       if (Prop != NULL) {
       defDSM = TRUE; //set by user
       if (IsPropertyTrue(Prop)) {
       settingsData.DropOEM_DSM = 0xFFFF;
       } else if (IsPropertyFalse(Prop)) {
       settingsData.DropOEM_DSM = 0;
       } else if (Prop->isInt()) {
       settingsData.DropOEM_DSM = (UINT16)(UINTN)Prop->??;
       } else if (Prop->type == kTagTypeDict) {
       Prop2 = GetProperty(Prop, "ATI");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_ATI;
       }
       
       Prop2 = GetProperty(Prop, "NVidia");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_NVIDIA;
       }
       
       Prop2 = GetProperty(Prop, "IntelGFX");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_INTEL;
       }
       
       Prop2 = GetProperty(Prop, "HDA");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_HDA;
       }
       
       Prop2 = GetProperty(Prop, "HDMI");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_HDMI;
       }
       
       Prop2 = GetProperty(Prop, "SATA");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_SATA;
       }
       
       Prop2 = GetProperty(Prop, "LAN");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_LAN;
       }
       
       Prop2 = GetProperty(Prop, "WIFI");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_WIFI;
       }
       
       Prop2 = GetProperty(Prop, "USB");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_USB;
       }
       
       Prop2 = GetProperty(Prop, "LPC");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_LPC;
       }
       
       Prop2 = GetProperty(Prop, "SmBUS");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_SMBUS;
       }
       
       Prop2 = GetProperty(Prop, "Firewire");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_FIREWIRE;
       }
       
       Prop2 = GetProperty(Prop, "IDE");
       if (IsPropertyTrue(Prop2)) {
       settingsData.DropOEM_DSM |= DEV_IDE;
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
          settingsData.ACPI.SSDT.Generate.GeneratePStates = TRUE;
          settingsData.ACPI.SSDT.Generate.GenerateCStates = TRUE;
          settingsData.ACPI.SSDT.Generate.GenerateAPSN = TRUE;
          settingsData.ACPI.SSDT.Generate.GenerateAPLF = TRUE;
          settingsData.ACPI.SSDT.Generate.GeneratePluginType = TRUE;
          
        } else if (IsPropertyNotNullAndFalse(Prop2)) {
          settingsData.ACPI.SSDT.Generate.GeneratePStates = FALSE;
          settingsData.ACPI.SSDT.Generate.GenerateCStates = FALSE;
          settingsData.ACPI.SSDT.Generate.GenerateAPSN = FALSE;
          settingsData.ACPI.SSDT.Generate.GenerateAPLF = FALSE;
          settingsData.ACPI.SSDT.Generate.GeneratePluginType = FALSE;
          
        } else if (Prop2->isDict()) {
          const TagStruct* Prop = Prop2->getDict()->propertyForKey("PStates");
          if (Prop) {
            settingsData.ACPI.SSDT.Generate.GeneratePStates = IsPropertyNotNullAndTrue(Prop);
          }
          Prop = Prop2->getDict()->propertyForKey("CStates");
          if (Prop) {
            settingsData.ACPI.SSDT.Generate.GenerateCStates = IsPropertyNotNullAndTrue(Prop);
          }
          Prop = Prop2->getDict()->propertyForKey("APSN");
          if (Prop) {
            settingsData.ACPI.SSDT.Generate.GenerateAPSN = IsPropertyNotNullAndTrue(Prop);
          }
          Prop = Prop2->getDict()->propertyForKey("APLF");
          if (Prop) {
            settingsData.ACPI.SSDT.Generate.GenerateAPLF = IsPropertyNotNullAndTrue(Prop);
          }
          Prop = Prop2->getDict()->propertyForKey("PluginType");
          if (Prop) {
            settingsData.ACPI.SSDT.Generate.GeneratePluginType = IsPropertyNotNullAndTrue(Prop);
          }
        }
      }
      
      const TagStruct* Prop = SSDTDict->propertyForKey("DropOem");
      settingsData.ACPI.SSDT.DropSSDTSetting  = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("NoOemTableId"); // to disable OEM table ID on ACPI/orgin/SSDT file names
      settingsData.ACPI.SSDT.NoOemTableId = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("NoDynamicExtract"); // to disable extracting child SSDTs
      settingsData.ACPI.SSDT.NoDynamicExtract = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("UseSystemIO");
      settingsData.ACPI.SSDT.EnableISS = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("EnableC7");
      if (Prop) {
        settingsData.ACPI.SSDT.EnableC7 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC7: %s\n", settingsData.ACPI.SSDT.EnableC7 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC6");
      if (Prop) {
        settingsData.ACPI.SSDT._EnableC6 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC6: %s\n", settingsData.ACPI.SSDT._EnableC6 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC4");
      if (Prop) {
        settingsData.ACPI.SSDT._EnableC4 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC4: %s\n", settingsData.ACPI.SSDT._EnableC4 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("EnableC2");
      if (Prop) {
        settingsData.ACPI.SSDT._EnableC2 = IsPropertyNotNullAndTrue(Prop);
        DBG("EnableC2: %s\n", settingsData.ACPI.SSDT._EnableC2 ? "yes" : "no");
      }
      
      Prop = SSDTDict->propertyForKey("C3Latency");
        settingsData.ACPI.SSDT._C3Latency = (UINT16)GetPropertyAsInteger(Prop, settingsData.ACPI.SSDT._C3Latency);
        DBG("C3Latency: %d\n", settingsData.ACPI.SSDT._C3Latency);
      
      Prop                       = SSDTDict->propertyForKey("PLimitDict");
      settingsData.ACPI.SSDT.PLimitDict       = (UINT8)GetPropertyAsInteger(Prop, 0);
      
      Prop                       = SSDTDict->propertyForKey("UnderVoltStep");
      settingsData.ACPI.SSDT.UnderVoltStep    = (UINT8)GetPropertyAsInteger(Prop, 0);
      
      Prop                       = SSDTDict->propertyForKey("DoubleFirstState");
      settingsData.ACPI.SSDT.DoubleFirstState = IsPropertyNotNullAndTrue(Prop);
      
      Prop = SSDTDict->propertyForKey("MinMultiplier");
        settingsData.ACPI.SSDT.MinMultiplier  = (UINT8)GetPropertyAsInteger(Prop, settingsData.ACPI.SSDT.MinMultiplier);
        DBG("MinMultiplier: %d\n", settingsData.ACPI.SSDT.MinMultiplier);
      
      Prop = SSDTDict->propertyForKey("MaxMultiplier");
        settingsData.ACPI.SSDT.MaxMultiplier = (UINT8)GetPropertyAsInteger(Prop, settingsData.ACPI.SSDT.MaxMultiplier);
        DBG("MaxMultiplier: %d\n", settingsData.ACPI.SSDT.MaxMultiplier);
      
      Prop = SSDTDict->propertyForKey("PluginType");
        settingsData.ACPI.SSDT.PluginType = (UINT8)GetPropertyAsInteger(Prop, settingsData.ACPI.SSDT.PluginType);
        DBG("PluginType: %d\n", settingsData.ACPI.SSDT.PluginType);
      }
    
    //     Prop               = GetProperty(DictPointer, "DropMCFG");
    //     settingsData.DropMCFG = IsPropertyTrue(Prop);
    
    const TagStruct* Prop = ACPIDict->propertyForKey("ResetAddress");
    if (Prop) {
      settingsData.ACPI.ResetAddr = (UINT32)GetPropertyAsInteger(Prop, 0x64);
      DBG("ResetAddr: 0x%llX\n", settingsData.ACPI.ResetAddr);
      
      if (settingsData.ACPI.ResetAddr  == 0x64) {
        settingsData.ACPI.ResetVal = 0xFE;
      } else if  (settingsData.ACPI.ResetAddr  == 0xCF9) {
        settingsData.ACPI.ResetVal = 0x06;
      }
      
      DBG("Calc ResetVal: 0x%hhX\n", settingsData.ACPI.ResetVal);
    }
    
    Prop = ACPIDict->propertyForKey("ResetValue");
      settingsData.ACPI.ResetVal = (UINT8)GetPropertyAsInteger(Prop, settingsData.ACPI.ResetVal);
      DBG("ResetVal: 0x%hhX\n", settingsData.ACPI.ResetVal);
    //other known pair is 0x0CF9/0x06. What about 0x92/0x01 ?
    
    Prop = ACPIDict->propertyForKey("HaltEnabler");
    settingsData.ACPI.SlpSmiEnable = IsPropertyNotNullAndTrue(Prop);
    
    //
    Prop = ACPIDict->propertyForKey("FixHeaders");
    settingsData.ACPI.FixHeaders = IsPropertyNotNullAndTrue(Prop);
    
    Prop = ACPIDict->propertyForKey("FixMCFG");
    settingsData.ACPI.FixMCFG = IsPropertyNotNullAndTrue(Prop);
    
    
    Prop = ACPIDict->propertyForKey("DisableASPM");
    settingsData.ACPI.NoASPM = IsPropertyNotNullAndTrue(Prop);
    
    Prop = ACPIDict->propertyForKey("smartUPS");
    if (Prop) {
      settingsData.ACPI.smartUPS   = IsPropertyNotNullAndTrue(Prop);
      DBG("smartUPS: present\n");
    }
    
    Prop               = ACPIDict->propertyForKey("PatchAPIC");
    settingsData.ACPI.PatchNMI = IsPropertyNotNullAndTrue(Prop);
    
    const TagArray* SortedOrderArray = ACPIDict->arrayPropertyForKey("SortedOrder"); // array of string
    if (SortedOrderArray) {
      INTN   i;
      INTN Count = SortedOrderArray->arrayContent().size();
      const TagStruct* Prop2 = NULL;
      if (Count > 0) {
        settingsData.ACPI.SortedACPI.setEmpty();
        
        for (i = 0; i < Count; i++) {
          Prop2 = &SortedOrderArray->arrayContent()[i];
          if ( !Prop2->isString()) {
            MsgLog("MALFORMED PLIST : SortedOrder must be an array of string");
            continue;
          }
          settingsData.ACPI.SortedACPI.Add(Prop2->getString()->stringValue());
        }
      }
    }
    
    Prop = ACPIDict->propertyForKey("AutoMerge");
    settingsData.ACPI.AutoMerge  = IsPropertyNotNullAndTrue(Prop);
    
    const TagArray* DisabledAMLArray = ACPIDict->arrayPropertyForKey("DisabledAML"); // array of string
    if (DisabledAMLArray) {
      INTN   i;
      INTN Count = DisabledAMLArray->arrayContent().size();
      const TagStruct* Prop2 = NULL;
      if (Count > 0) {
        settingsData.ACPI.DisabledAML.setEmpty();
        for (i = 0; i < Count; i++) {
          Prop2 = &DisabledAMLArray->arrayContent()[i];
          if ( !Prop2->isString()) {
            MsgLog("MALFORMED PLIST : DisabledAML must be an array of string");
            continue;
          }
          settingsData.ACPI.DisabledAML.Add(Prop2->getString()->stringValue());
        }
      }
    }
    //arrayPropertyForKey
    const TagDict* RenameDevicesDict = NULL;
    INTN arraySize = 0;
    const TagArray* RenameDevicesArray = ACPIDict->arrayPropertyForKey("RenameDevices");
    bool RenDevIsArray = (RenameDevicesArray != NULL);
    if (RenDevIsArray) {
      arraySize = RenameDevicesArray->arrayContent().size();
    } else {
      RenameDevicesDict = ACPIDict->dictPropertyForKey("RenameDevices"); // dict of key/string
      if (RenameDevicesDict) {
        arraySize = 1;
      }
    }
    if (arraySize > 0) {
      settingsData.ACPI.DeviceRename.setEmpty(); //else will not change
    }
    for (INTN i = 0; i < arraySize; i++) {
      if (RenDevIsArray) {
        RenameDevicesDict = RenameDevicesArray->dictElementAt(i, "RenameDevices"_XS8);
      }
      if (!RenameDevicesDict) break;
      INTN Count = RenameDevicesDict->dictKeyCount();
      if (Count > 0) {
        for (INTN j = 0; j < Count; j++) {
          const TagKey* key;
          const TagStruct* value;
          if ( !EFI_ERROR(RenameDevicesDict->getKeyAndValueAtIndex(j, &key, &value)) ) {
            if ( value->isString() && value->getString()->stringValue().notEmpty() ) {
              ACPI_RENAME_DEVICE* List2 = new ACPI_RENAME_DEVICE();
              List2->acpiName.Name = key->keyStringValue();
              List2->renameTo = value->getString()->stringValue();
              settingsData.ACPI.DeviceRename.AddReference(List2, false);
              // Debug print. We use getSplittedName() and then ConcatAll because getSplittedName() format components (trim or expand to 4 chars).
              DBG("'%s' -> will be renamed to '%s'\n", List2->acpiName.getSplittedName().ConcatAll(":").c_str(), List2->renameTo.c_str());
            }
          }
        } //for j < dict size
      }
    } //for i < array size
  }
}

/*
 * 2021-04, Jief : this is untouched old code (except a small bug when name starts with '#') to create ArbProperties the old way
 * Old way = only one structure DEV_PROPERTY, Properties and ArbProperties are mixed together, no objetcs (XString, XBuffer).
 * This is temporary and is used to check that new code gives the same result as the old. To be remove in a while
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
void SETTINGS_DATA::DevicesClass::FillDevicePropertiesOld(SETTINGS_DATA& settingsData, const TagDict* DevicesDict)
{
#pragma GCC diagnostic pop
      /*
       * Properties is a single string, or a dict
       */
      const TagStruct* Prop = DevicesDict->propertyForKey("Properties");
      if (Prop != NULL) {
        if (Prop->isString()) {
        }
        else if ( Prop->isDict() ) {
          INTN i;
          const TagDict* PropertiesDict = Prop->getDict();
          INTN Count = PropertiesDict->dictKeyCount(); //ok
          //settingsData.Devices.AddProperties = new DEV_PROPERTY[Count]; // seems bug, only ArbProperties is used in this block
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
                if (key->keyStringValue().startWithOrEqualTo("#")) {
                      continue;
                }

                // when key in Devices/Properties is one of the strings "PrimaryGPU" / "SecondaryGPU", use device path of first / second gpu accordingly
#ifdef CLOVER_BUILD
                if ( DevicePathStr.isEqualIC("PrimaryGPU") ) {
                  DevicePath = DevicePathFromHandle(gGraphics[0].Handle); // first gpu
                } else if ( DevicePathStr.isEqualIC("SecondaryGPU") && NGFX > 1) {
                  DevicePath = DevicePathFromHandle(gGraphics[1].Handle); // second gpu
                } else {
                  DevicePath = ConvertTextToDevicePath(DevicePathStr.wc_str()); //TODO
                }
                if (DevicePath == NULL) {
                  continue;
                }
#endif
                //Create Device node
                DevPropDevice = settingsData.Devices.ArbProperties;
                settingsData.Devices.ArbProperties = new DEV_PROPERTY;
                settingsData.Devices.ArbProperties->Next = DevPropDevice; //next device
                settingsData.Devices.ArbProperties->Child = NULL;
                settingsData.Devices.ArbProperties->Device = 0; //to differ from arbitrary
                settingsData.Devices.ArbProperties->DevicePath = DevicePath; //this is pointer
                settingsData.Devices.ArbProperties->Label = S8Printf("%s", key->keyStringValue().c_str()).forgetDataWithoutFreeing();
                Child = &(settingsData.Devices.ArbProperties->Child);

                if ((value != NULL) && (value->isDict())) {
                  INTN PropCount = 0;
                  const TagDict* valueDict = value->getDict();
                  PropCount = valueDict->dictKeyCount();
                  //         DBG("Add %d properties:\n", PropCount);
                  for (INTN j = 0; j < PropCount; j++) {
                    DevProps = *Child;

                    const TagKey* key2;
                    const TagStruct* value2;
                    if (EFI_ERROR(valueDict->getKeyAndValueAtIndex(j, &key2, &value2))) {
                      continue;
                    }
                    if (key2->keyStringValue().isEmpty()) {
                      continue;
                    }
                    if (key2->keyStringValue()[0] == '#') {
                      continue;
                    }

                    *Child = new DEV_PROPERTY;
                    (*Child)->Next = DevProps;
//                    if (key2->keyStringValue()[0] != '#') {
                      (*Child)->MenuItem.BValue = TRUE;
                      (*Child)->Key = S8Printf("%s", key2->keyStringValue().c_str()).forgetDataWithoutFreeing();
//                    }
//                    else {
//                      (*Child)->MenuItem.BValue = FALSE;
//                      (*Child)->Key = S8Printf("%s", key2->keyStringValue().c_str() + 1).forgetDataWithoutFreeing();
//                    }

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

      const TagArray* ArbitraryTagArray = DevicesDict->arrayPropertyForKey("Arbitrary"); // array of dict
      if (ArbitraryTagArray != NULL) {
        INTN Index;
        INTN   Count = ArbitraryTagArray->arrayContent().size();
        DEV_PROPERTY *DevProp;

        if (Count > 0) {
          DBG("Add %lld devices (Arbitrary):\n", Count);
          for (Index = 0; Index < Count; Index++) {
            UINTN DeviceAddr = 0U;
            XString8 Label;
            DBG(" - [%02lld]:", Index);
            const TagDict* Dict2 = ArbitraryTagArray->dictElementAt(Index, "Arbitrary"_XS8);
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
                DevProp = settingsData.Devices.ArbProperties;
                settingsData.Devices.ArbProperties = new DEV_PROPERTY;
                settingsData.Devices.ArbProperties->Next = DevProp;

                settingsData.Devices.ArbProperties->Device = (UINT32)DeviceAddr;
                settingsData.Devices.ArbProperties->Label = (__typeof__(settingsData.Devices.ArbProperties->Label))AllocateCopyPool(Label.sizeInBytesIncludingTerminator(), Label.c_str());

                const TagStruct* DisabledProp = Dict3->propertyForKey("Disabled");
                settingsData.Devices.ArbProperties->MenuItem.BValue = !IsPropertyNotNullAndTrue(DisabledProp);

                DisabledProp = Dict3->propertyForKey("Key");
                if (DisabledProp && (DisabledProp->isString()) && DisabledProp->getString()->stringValue().notEmpty()) {
                  settingsData.Devices.ArbProperties->Key = S8Printf("%s", DisabledProp->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
                }

                DisabledProp = Dict3->propertyForKey("Value");
                if (DisabledProp && (DisabledProp->isString()) && DisabledProp->getString()->stringValue().notEmpty()) {
                  //first suppose it is Ascii string
                  settingsData.Devices.ArbProperties->Value = (UINT8*)S8Printf("%s", DisabledProp->getString()->stringValue().c_str()).forgetDataWithoutFreeing();
                  settingsData.Devices.ArbProperties->ValueLen = DisabledProp->getString()->stringValue().sizeInBytesIncludingTerminator();
                  settingsData.Devices.ArbProperties->ValueType = kTagTypeString;
                } else if (DisabledProp && (DisabledProp->isInt64())) {
                  if ( DisabledProp->getInt64()->intValue() < MIN_INT32  ||  DisabledProp->getInt64()->intValue() > MAX_INT32 ) {
                    MsgLog("Invalid int value for key 'Value'\n");
                  }else{
                    INT32 intValue = (INT32)DisabledProp->getInt64()->intValue();
                    settingsData.Devices.ArbProperties->Value = (__typeof__(settingsData.Devices.ArbProperties->Value))AllocatePool(sizeof(intValue));
  //                    CopyMem(settingsData.ArbProperties->Value, &Prop3->intValue, 4);
                    *(INT32*)(settingsData.Devices.ArbProperties->Value) = intValue;
                    settingsData.Devices.ArbProperties->ValueLen = sizeof(intValue);
                    settingsData.Devices.ArbProperties->ValueType = kTagTypeInteger;
                  }
                } else if ( DisabledProp && DisabledProp->isTrue() ) {
                  settingsData.Devices.ArbProperties->Value = (__typeof__(settingsData.Devices.ArbProperties->Value))AllocateZeroPool(4);
                  settingsData.Devices.ArbProperties->Value[0] = TRUE;
                  settingsData.Devices.ArbProperties->ValueLen = 1;
                  settingsData.Devices.ArbProperties->ValueType = kTagTypeTrue;
                } else if ( DisabledProp && DisabledProp->isFalse() ) {
                  settingsData.Devices.ArbProperties->Value = (__typeof__(settingsData.Devices.ArbProperties->Value))AllocateZeroPool(4);
                  //settingsData.ArbProperties->Value[0] = FALSE;
                  settingsData.Devices.ArbProperties->ValueLen = 1;
                  settingsData.Devices.ArbProperties->ValueType = kTagTypeFalse;
                } else {
                  //else  data
                  UINTN Size = 0;
                  settingsData.Devices.ArbProperties->Value = GetDataSetting (Dict3, "Value", &Size);
                  settingsData.Devices.ArbProperties->ValueLen = Size;
                  settingsData.Devices.ArbProperties->ValueType = kTagTypeData;
                }

                //Special case. In future there must be more such cases
                if ((AsciiStrStr(settingsData.Devices.ArbProperties->Key, "-platform-id") != NULL)) {
                  CopyMem((CHAR8*)&settingsData.Graphics._IgPlatform, settingsData.Devices.ArbProperties->Value, 4);
                }
              }   //for() device properties
            }
          } //for() devices
        }
      //        settingsData.NrAddProperties = 0xFFFE;
    }
}


/*
 * To ease copy/paste and text replacement from GetUserSettings, the parameter has the same name as the global
 * and is passed by non-const reference.
 * This temporary during the refactoring
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
EFI_STATUS GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& settingsData)
{
#pragma GCC diagnostic pop
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
      settingsData.Boot.NeverDoRecovery  = IsPropertyNotNullAndTrue(Prop);
    }

    //Graphics

    const TagDict* GraphicsDict = CfgDict->dictPropertyForKey("Graphics");
    if (GraphicsDict != NULL) {
      INTN i;
      const TagStruct* Prop     = GraphicsDict->propertyForKey("Inject");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndTrue(Prop)) {
          settingsData.Graphics.InjectAsDict.GraphicsInjector = TRUE;
          settingsData.Graphics.InjectAsDict.InjectIntel      = TRUE;
          settingsData.Graphics.InjectAsDict.InjectATI        = TRUE;
          settingsData.Graphics.InjectAsDict.InjectNVidia     = TRUE;
        } else if (Prop->isDict()) {
          const TagDict* Dict2 = Prop->getDict();
          const TagStruct* Prop2 = Dict2->propertyForKey("Intel");
          if (Prop2 != NULL) {
            settingsData.Graphics.InjectAsDict.InjectIntel = IsPropertyNotNullAndTrue(Prop2);
          }

          Prop2 = Dict2->propertyForKey("ATI");
          if (Prop2 != NULL) {
            settingsData.Graphics.InjectAsDict.InjectATI = IsPropertyNotNullAndTrue(Prop2);
          }

          Prop2 = Dict2->propertyForKey("NVidia");
          if (Prop2 != NULL) {
            settingsData.Graphics.InjectAsDict.InjectNVidia = IsPropertyNotNullAndTrue(Prop2);
          }
        } else {
          settingsData.Graphics.InjectAsDict.GraphicsInjector = FALSE;
          settingsData.Graphics.InjectAsDict.InjectIntel      = FALSE;
          settingsData.Graphics.InjectAsDict.InjectATI        = FALSE;
          settingsData.Graphics.InjectAsDict.InjectNVidia     = FALSE;
        }
      }

      Prop = GraphicsDict->propertyForKey("RadeonDeInit");
      settingsData.Graphics.RadeonDeInit = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("VRAM");
      settingsData.Graphics.VRAM = (UINTN)GetPropertyAsInteger(Prop, (INTN)settingsData.Graphics.VRAM); //Mb
      //
      Prop = GraphicsDict->propertyForKey("RefCLK");
      settingsData.Graphics.RefCLK = (UINT16)GetPropertyAsInteger(Prop, 0);

      Prop = GraphicsDict->propertyForKey("LoadVBios");
      settingsData.Graphics.LoadVBios = IsPropertyNotNullAndTrue(Prop);
//
//      for (i = 0; i < (INTN)NGFX; i++) {
//        gGraphics[i].LoadVBios = settingsData.Graphics.LoadVBios; //default
//      }

      Prop = GraphicsDict->propertyForKey("VideoPorts");
      settingsData.Graphics.VideoPorts   = (UINT16)GetPropertyAsInteger(Prop, settingsData.Graphics.VideoPorts);

      Prop = GraphicsDict->propertyForKey("BootDisplay");
      settingsData.Graphics.BootDisplay = (INT8)GetPropertyAsInteger(Prop, -1);

      Prop = GraphicsDict->propertyForKey("FBName");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in FBName\n");
        }else{
          settingsData.Graphics.FBName = Prop->getString()->stringValue();
        }
      }

      Prop = GraphicsDict->propertyForKey("NVCAP");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in NVCAP\n");
        }else{
          hex2bin (Prop->getString()->stringValue(), (UINT8*)&settingsData.Graphics.NVCAP[0], sizeof(settingsData.Graphics.NVCAP));
          DBG("Read NVCAP:");

          for (i = 0; i<20; i++) {
            DBG("%02hhX", settingsData.Graphics.NVCAP[i]);
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
          hex2bin (Prop->getString()->stringValue(), (UINT8*)&settingsData.Graphics.Dcfg[0], sizeof(settingsData.Graphics.Dcfg));
        }
      }

      Prop = GraphicsDict->propertyForKey("DualLink");
      settingsData.Graphics.DualLink = (UINT32)GetPropertyAsInteger(Prop, settingsData.Graphics.DualLink);

      //InjectEDID - already done in earlysettings
      //No! Take again
      GetEDIDSettings(GraphicsDict, settingsData);

      // ErmaC: NvidiaGeneric
      Prop = GraphicsDict->propertyForKey("NvidiaGeneric");
      settingsData.Graphics.NvidiaGeneric = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("NvidiaNoEFI");
      settingsData.Graphics.NvidiaNoEFI = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("NvidiaSingle");
      settingsData.Graphics.NvidiaSingle = IsPropertyNotNullAndTrue(Prop);

      Prop = GraphicsDict->propertyForKey("ig-platform-id");
      settingsData.Graphics._IgPlatform = (UINT32)GetPropertyAsInteger(Prop, settingsData.Graphics._IgPlatform);

      Prop = GraphicsDict->propertyForKey("snb-platform-id");
      settingsData.Graphics._IgPlatform = (UINT32)GetPropertyAsInteger(Prop, settingsData.Graphics._IgPlatform);

      FillCardList(GraphicsDict, settingsData); //#@ Getcardslist
    }

    const TagDict* DevicesDict = CfgDict->dictPropertyForKey("Devices");
    if (DevicesDict != NULL) {
      const TagStruct* Prop = DevicesDict->propertyForKey("Inject");
      settingsData.Devices.StringInjector = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("SetIntelBacklight");
      settingsData.Devices.IntelBacklight = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("SetIntelMaxBacklight");
      settingsData.Devices.IntelMaxBacklight = IsPropertyNotNullAndTrue(Prop);

      Prop = DevicesDict->propertyForKey("IntelMaxValue");
      settingsData.Devices.IntelMaxValue = (UINT16)GetPropertyAsInteger(Prop, settingsData.Devices.IntelMaxValue);


      settingsData.Devices.FillDevicePropertiesOld(settingsData, DevicesDict);

      /*
       * Properties is a single string, or a dict
       */
      Prop = DevicesDict->propertyForKey("Properties");
      if (Prop != NULL) {
        if (Prop->isString()) {
          settingsData.Devices.Properties.propertiesAsString = Prop->getString()->stringValue();
          size_t binaryPropSize = hex2bin(settingsData.Devices.Properties.propertiesAsString, NULL, 0);
          if ( binaryPropSize > MAX_UINT32 ) {
            MsgLog("settingsData.Devices.Properties.cDeviceProperties is too big");
            settingsData.Devices.Properties.propertiesAsString.setEmpty();
          }
        }
        else if ( Prop->isDict() ) {
          INTN i;
          const TagDict* PropertiesDict = Prop->getDict();
          INTN Count = PropertiesDict->dictKeyCount(); //ok

          if (Count > 0) {
            DBG("Add %lld devices (kTagTypeDict):\n", Count);

            for (i = 0; i < Count; i++) {
              const TagKey* key;
              const TagStruct* value;
//              EFI_DEVICE_PATH_PROTOCOL* DevicePath = NULL;
              if ( !EFI_ERROR(PropertiesDict->getKeyAndValueAtIndex(i, &key, &value)) ) {  //take a <key> with DevicePath. If GetKeyValueAtIndex return success, key and value != NULL
              
                if ( key->keyStringValue().startWithOrEqualTo('#') ) continue; // Commented out, ignored. This is a tempororay litle change of behavior because that's how the new parser will works.
                if ( key->keyStringValue().isEqual("!") ) {
                  // '!' means disabled. If Label is only '!', means an empty disabled label...
                  continue;
                }

                XStringW DevicePathStr = key->keyStringValue();
                //         DBG("Device: %ls\n", DevicePathStr);

                // when key in Devices/Properties is one of the strings "PrimaryGPU" / "SecondaryGPU", use device path of first / second gpu accordingly
//#ifdef CLOVER_BUILD
//                if ( DevicePathStr.isEqualIC("PrimaryGPU") ) {
//                  DevicePath = DevicePathFromHandle(gGraphics[0].Handle); // first gpu
//                } else if ( DevicePathStr.isEqualIC("SecondaryGPU") && NGFX > 1) {
//                  DevicePath = DevicePathFromHandle(gGraphics[1].Handle); // second gpu
//                } else {
//                  DevicePath = ConvertTextToDevicePath(DevicePathStr.wc_str()); //TODO
//                }
//                if (DevicePath == NULL) {
//                  continue;
//                }
//#endif
                //Create Device node
//                DevPropDevice = settingsData.Devices.ArbProperties;
                SETTINGS_DATA::DevicesClass::PropertiesClass::PropertyClass* devProperty = new SETTINGS_DATA::DevicesClass::PropertiesClass::PropertyClass;
//                devProperty->DevicePath = DevicePath; //this is pointer TODO ?
                devProperty->DevicePathAsString.SWPrintf("%s", key->keyStringValue().c_str());
                if ( key->keyStringValue().startWith('!') ) {
                  // '!' means disabled. If Label is only '!', means an empty disabled label...
                  devProperty->DevicePathAsString = key->keyStringValue().subString(1, MAX_XSIZE);
                  devProperty->Enabled = false;
//                  devProperty->MenuItem.BValue = true;
                }else{
                  devProperty->DevicePathAsString = key->keyStringValue();
                  devProperty->Enabled = true;
//                  devProperty->MenuItem.BValue = false;
                }

//                settingsData.Devices.ArbProperties.InsertRef(devProperty, 0, false); // TODO
                settingsData.Devices.Properties.PropertyArray.AddReference(devProperty, true);

                if ((value != NULL) && (value->isDict())) {
                  INTN PropCount = 0;
                  const TagDict* valueDict = value->getDict();
                  PropCount = valueDict->dictKeyCount();
                  //         DBG("Add %d properties:\n", PropCount);
                  for (INTN j = 0; j < PropCount; j++) {
                    const TagKey* key2;
                    const TagStruct* value2;
                    if (EFI_ERROR(valueDict->getKeyAndValueAtIndex(j, &key2, &value2))) {
                      continue;
                    }
                    if ( key2->keyStringValue().startWithOrEqualTo('#') ) continue;
                    if ( key2->keyStringValue().isEqual("!") ) continue;

                    SETTINGS_DATA::DevicesClass::SimplePropertyClass* ChildPtr = new SETTINGS_DATA::DevicesClass::SimplePropertyClass;
                    SETTINGS_DATA::DevicesClass::SimplePropertyClass& Child = *ChildPtr;

                    if ( key2->keyStringValue().startWith('!') ) { // startWaith return false if XString == '#'
                      Child.MenuItem.BValue = FALSE;
                      Child.Key = key2->keyStringValue().subString(1, MAX_XSIZE);
                    }
                    else {
                      Child.MenuItem.BValue = devProperty->Enabled;
                      Child.Key = key2->keyStringValue();
                    }


                    //    DBG("<key>%s\n  <value> type %d\n", Child.Key, Prop3->type);
                    if (value2 && (value2->isString()) && value2->getString()->stringValue().notEmpty()) {
                      //first suppose it is Ascii string
                      Child.Value.ncpy(value2->getString()->stringValue().c_str(), value2->getString()->stringValue().sizeInBytesIncludingTerminator());
                      Child.ValueType = kTagTypeString;
                    }
                    else if (value2 && (value2->isInt64())) {
                      if ( value2->getInt64()->intValue() < MIN_INT32  ||  value2->getInt64()->intValue() > MAX_INT32 ) {
                        MsgLog("Invalid int value for key %s\n", key2->keyStringValue().c_str());
                      }else{
                        INT32 intValue = (INT32)value2->getInt64()->intValue();
                        Child.Value.cpy(intValue);
                        Child.ValueType = kTagTypeInteger;
                      }
                    }
                    else if (value2 && value2->isTrue() ) {
                      uint8_t b = 1;
                      Child.Value.cpy(b);
                      Child.ValueType = kTagTypeTrue;
                    }
                    else if ( value2 && value2->isFalse() ) {
                      uint8_t b = 0;
                      Child.Value.cpy(b);
                      Child.ValueType = kTagTypeFalse;
                    }
                    else if (value2 && (value2->isData())) {
                      Child.Value = value2->getData()->data();
                      Child.ValueType = kTagTypeData;
                    }
                    devProperty->propertiesArray.AddReference(ChildPtr, true);
                  }
                }
              }
            }
          }
        }
      }

      Prop  = DevicesDict->propertyForKey("LANInjection");
      settingsData.Devices.LANInjection = !IsPropertyNotNullAndFalse(Prop);  //default = TRUE

      Prop  = DevicesDict->propertyForKey("HDMIInjection");
      settingsData.Devices.HDMIInjection = IsPropertyNotNullAndTrue(Prop);

      Prop  = DevicesDict->propertyForKey("NoDefaultProperties");
      settingsData.Devices.NoDefaultProperties = !IsPropertyNotNullAndFalse(Prop);

      const TagArray* ArbitraryArray = DevicesDict->arrayPropertyForKey("Arbitrary"); // array of dict
      if (ArbitraryArray != NULL) {
        INTN Index;
        INTN   Count = ArbitraryArray->arrayContent().size();

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
              SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass* arbProp = NULL;

              for (PropIndex = 0; PropIndex < PropCount; PropIndex++) {
                Dict3 = CustomPropertiesArray->dictElementAt(PropIndex, "CustomProperties"_XS8);
                SETTINGS_DATA::DevicesClass::SimplePropertyClass* newDevProp = new SETTINGS_DATA::DevicesClass::SimplePropertyClass;

                const TagStruct* DisabledProp = Dict3->propertyForKey("Disabled");
                newDevProp->MenuItem.BValue = !IsPropertyNotNullAndTrue(DisabledProp);

                DisabledProp = Dict3->propertyForKey("Key");
                if (DisabledProp && (DisabledProp->isString()) && DisabledProp->getString()->stringValue().notEmpty()) {
                  newDevProp->Key = DisabledProp->getString()->stringValue();
                }

                DisabledProp = Dict3->propertyForKey("Value");
                if (DisabledProp && (DisabledProp->isString()) && DisabledProp->getString()->stringValue().notEmpty()) {
                  //first suppose it is Ascii string
                  newDevProp->Value.ncpy(DisabledProp->getString()->stringValue().c_str(), DisabledProp->getString()->stringValue().sizeInBytesIncludingTerminator());
                  newDevProp->ValueType = kTagTypeString;
                } else if (DisabledProp && (DisabledProp->isInt64())) {
                  if ( DisabledProp->getInt64()->intValue() < MIN_INT32  ||  DisabledProp->getInt64()->intValue() > MAX_INT32 ) {
                    MsgLog("Invalid int value for key 'Value'\n");
                  }else{
                    INT32 intValue = (INT32)DisabledProp->getInt64()->intValue();
                    newDevProp->Value.cpy(intValue);
                    newDevProp->ValueType = kTagTypeInteger;
                  }
                } else if ( DisabledProp && DisabledProp->isTrue() ) {
                  newDevProp->Value.cpy((uint32_t)1);
                  newDevProp->ValueType = kTagTypeTrue;
                } else if ( DisabledProp && DisabledProp->isFalse() ) {
                  newDevProp->Value.cpy((uint32_t)0);
                  newDevProp->ValueType = kTagTypeFalse;
                } else {
                  //else  data
                  UINTN Size = 0;
                  uint8_t* Data = GetDataSetting (Dict3, "Value", &Size);
                  newDevProp->Value.stealValueFrom(Data, Size);
                  newDevProp->ValueType = kTagTypeData;
                }

                //Special case. In future there must be more such cases
//                if ( newDevProp->Key.contains("-platform-id") ) {
//                  CopyMem((CHAR8*)&settingsData.Graphics.IgPlatform, newDevProp->Value.data(), 4);
//                }
                if ( arbProp == NULL ) {
                  arbProp = new SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass();
                  arbProp->Device = (UINT32)DeviceAddr;
                  arbProp->Label = Label;
                  settingsData.Devices.ArbitraryArray.AddReference(arbProp, true);
                }
                arbProp->CustomPropertyArray.AddReference(newDevProp, true);
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
          settingsData.Devices.AddPropertyArray.setEmpty();

          for (i = 0; i < Count; i++) {
            UINTN Size = 0;
        DBG(" - [%02lld]:", i);
            const TagDict* Dict2 = AddPropertiesArray->dictElementAt(i, "AddProperties"_XS8);
            const TagStruct* DeviceProp = Dict2->propertyForKey("Device");
            SETTINGS_DATA::DevicesClass::AddPropertyClass* Property = new SETTINGS_DATA::DevicesClass::AddPropertyClass();

            if (DeviceProp && (DeviceProp->isString()) && DeviceProp->getString()->stringValue().notEmpty()) {
              if (DeviceProp->getString()->stringValue().isEqualIC("ATI")) {
                Property->Device = (UINT32)DEV_ATI;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("NVidia")) {
                Property->Device = (UINT32)DEV_NVIDIA;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("IntelGFX")) {
                Property->Device = (UINT32)DEV_INTEL;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("LAN")) {
                Property->Device = (UINT32)DEV_LAN;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("WIFI")) {
                Property->Device = (UINT32)DEV_WIFI;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("Firewire")) {
                Property->Device = (UINT32)DEV_FIREWIRE;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("SATA")) {
                Property->Device = (UINT32)DEV_SATA;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("IDE")) {
                Property->Device = (UINT32)DEV_IDE;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("HDA")) {
                Property->Device = (UINT32)DEV_HDA;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("HDMI")) {
                Property->Device = (UINT32)DEV_HDMI;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("LPC")) {
                Property->Device = (UINT32)DEV_LPC;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("SmBUS")) {
                Property->Device = (UINT32)DEV_SMBUS;
              } else if (DeviceProp->getString()->stringValue().isEqualIC("USB")) {
                Property->Device = (UINT32)DEV_USB;
              } else {
                DBG(" unknown device, ignored\n"/*, i*/);
                continue;
              }
            }

            if ( DeviceProp->isString() ) DBG(" %s ", DeviceProp->getString()->stringValue().c_str());

            const TagStruct* Prop2 = Dict2->propertyForKey("Disabled");
            Property->MenuItem.BValue = !IsPropertyNotNullAndTrue(Prop2);

            Prop2 = Dict2->propertyForKey("Key");
            if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
              Property->Key = Prop2->getString()->stringValue();
            }

            Prop2 = Dict2->propertyForKey("Value");
            if (Prop2 && (Prop2->isString()) && Prop2->getString()->stringValue().notEmpty()) {
              //first suppose it is Ascii string
              Property->Value.ncpy(Prop2->getString()->stringValue().c_str(), Prop2->getString()->stringValue().sizeInBytesIncludingTerminator());
              Property->ValueType = kTagTypeString;
            } else if (Prop2 && (Prop2->isInt64())) {
              if ( Prop2->getInt64()->intValue() < MIN_INT32  ||  Prop2->getInt64()->intValue() > MAX_INT32 ) {
                MsgLog("Invalid int value for key 'Value'\n");
              }else{
                INT32 intValue = (INT32)Prop2->getInt64()->intValue();
                Property->Value.cat(intValue);
                Property->ValueType = kTagTypeInteger;
//                Property->Value = (__typeof__(Property->Value))AllocatePool (sizeof(intValue));
//  //              CopyMem(settingsData.AddProperties[Index].Value, &Prop2->intValue, 4);
//                *(INT32*)(Property->Value) = intValue;
//                Property->ValueLen = sizeof(intValue);
              }
            } else {
              //else  data
              uint8_t* Data = GetDataSetting (Dict2, "Value", &Size);
              Property->Value.stealValueFrom(Data, Size);
              Property->ValueType = kTagTypeData;
            }

            DBG("Key: %s, len: %zu\n", Property->Key.c_str(), Property->Value.size());

            if (!Property->MenuItem.BValue) {
              DBG("  property disabled at config\n");
            }

            settingsData.Devices.AddPropertyArray.AddReference(Property, true);
            ++Index;
          }

//          settingsData.Devices.AddPropertyArray.size() = Index;
        }
      }
      //end AddProperties

      const TagDict* FakeIDDict = DevicesDict->dictPropertyForKey("FakeID");
      if (FakeIDDict != NULL) {
        const TagStruct* Prop2 = FakeIDDict->propertyForKey("ATI");
        if (Prop2 && (Prop2->isString())) {
          settingsData.Devices.FakeID.FakeATI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("NVidia");
        if (Prop2 && (Prop2->isString())) {
          settingsData.Devices.FakeID.FakeNVidia  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("IntelGFX");
        if (Prop2 && (Prop2->isString())) {
          settingsData.Devices.FakeID.FakeIntel  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("LAN");
        if (Prop2 && (Prop2->isString())) {
          settingsData.Devices.FakeID.FakeLAN  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("WIFI");
        if (Prop2 && (Prop2->isString())) {
          settingsData.Devices.FakeID.FakeWIFI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("SATA");
        if (Prop2 && (Prop2->isString())) {
          settingsData.Devices.FakeID.FakeSATA  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("XHCI");
        if (Prop2 && (Prop2->isString())) {
          settingsData.Devices.FakeID.FakeXHCI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }

        Prop2 = FakeIDDict->propertyForKey("IMEI");
        if (Prop2 && (Prop2->isString())) {
          settingsData.Devices.FakeID.FakeIMEI  = (UINT32)AsciiStrHexToUint64(Prop2->getString()->stringValue());
        }
      }

      Prop                   = DevicesDict->propertyForKey("UseIntelHDMI");
      settingsData.Devices.UseIntelHDMI = IsPropertyNotNullAndTrue(Prop);

      Prop                = DevicesDict->propertyForKey("ForceHPET");
      settingsData.Devices.ForceHPET = IsPropertyNotNullAndTrue(Prop);

      Prop                = DevicesDict->propertyForKey("DisableFunctions");
      if (Prop && (Prop->isString())) {
        settingsData.Devices.DisableFunctions  = (UINT32)AsciiStrHexToUint64(Prop->getString()->stringValue());
      }

      Prop                = DevicesDict->propertyForKey("AirportBridgeDeviceName");
      if (Prop && (Prop->isString())) {
        if ( Prop->getString()->stringValue().length() != 4 ) {
           MsgLog("ERROR IN PLIST : AirportBridgeDeviceName must 4 chars long");
        }else{
          settingsData.Devices.AirportBridgeDeviceName = Prop->getString()->stringValue();
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
            settingsData.Devices.Audio.HDALayoutId = (INT32)Prop->getInt64()->intValue(); //must be signed
            settingsData.Devices.Audio.HDAInjection = (settingsData.Devices.Audio.HDALayoutId > 0);
          } else if (Prop->isString()){
            if ( Prop->getString()->stringValue().notEmpty()  &&  (Prop->getString()->stringValue()[0] == 'n' || Prop->getString()->stringValue()[0] == 'N') ) {
              // if starts with n or N, then no HDA injection
              settingsData.Devices.Audio.HDAInjection = FALSE;
            } else if ( Prop->getString()->stringValue().length() > 1  &&
                        Prop->getString()->stringValue()[0] == '0'  &&
                        ( Prop->getString()->stringValue()[1] == 'x' || Prop->getString()->stringValue()[1] == 'X' ) ) {
              // assume it's a hex layout id
              settingsData.Devices.Audio.HDALayoutId = (INT32)AsciiStrHexToUintn(Prop->getString()->stringValue());
              settingsData.Devices.Audio.HDAInjection = TRUE;
            } else {
              // assume it's a decimal layout id
              settingsData.Devices.Audio.HDALayoutId = (INT32)AsciiStrDecimalToUintn(Prop->getString()->stringValue());
              settingsData.Devices.Audio.HDAInjection = TRUE;
            }
          }
        }

        Prop = AudioDict->propertyForKey("AFGLowPowerState");
        settingsData.Devices.Audio.AFGLowPowerState = IsPropertyNotNullAndTrue(Prop);
      }

      const TagDict* USBDict = DevicesDict->dictPropertyForKey("USB");
      if (USBDict != NULL) {
        // USB
        Prop = USBDict->propertyForKey("Inject");
        settingsData.Devices.USB.USBInjection = !IsPropertyNotNullAndFalse(Prop); // enabled by default

        Prop = USBDict->propertyForKey("AddClockID");
        settingsData.Devices.USB.InjectClockID = IsPropertyNotNullAndTrue(Prop); // disabled by default
        // enabled by default for CloverEFI
        // disabled for others
        settingsData.Devices.USB.USBFixOwnership = gFirmwareClover;
        Prop = USBDict->propertyForKey("FixOwnership");
        if (Prop != NULL) {
          settingsData.Devices.USB.USBFixOwnership = IsPropertyNotNullAndTrue(Prop);
        }
        DBG("USB FixOwnership: %s\n", settingsData.Devices.USB.USBFixOwnership?"yes":"no");

        Prop = USBDict->propertyForKey("HighCurrent");
        settingsData.Devices.USB.HighCurrent = IsPropertyNotNullAndTrue(Prop);

        Prop = USBDict->propertyForKey("NameEH00");
        settingsData.Devices.USB.NameEH00 = IsPropertyNotNullAndTrue(Prop);
      }
    }

    //*** ACPI ***//

    getACPISettings(CfgDict, settingsData);




    //done until here






    //*** SMBIOS ***//
    const TagDict* SMBIOSDict = CfgDict->dictPropertyForKey("SMBIOS");
    if (SMBIOSDict != NULL) {

      ParseSMBIOSSettings(settingsData, SMBIOSDict);

      const TagStruct* Prop = SMBIOSDict->propertyForKey("Trust");
      if (Prop != NULL) {
        if (IsPropertyNotNullAndFalse(Prop)) {
          settingsData.Smbios.TrustSMBIOS = FALSE;
        } else if (IsPropertyNotNullAndTrue(Prop)) {
          settingsData.Smbios.TrustSMBIOS = TRUE;
        }
      }
      Prop = SMBIOSDict->propertyForKey("MemoryRank");
      settingsData.Smbios.Attribute = (INT8)GetPropertyAsInteger(Prop, -1); //1==Single Rank, 2 == Dual Rank, 0==undefined -1 == keep as is

      // Delete the user memory when a new config is selected
      INTN i = 0;
      for (i = 0; i < settingsData.Smbios.Memory.SlotCounts && i < MAX_RAM_SLOTS; i++) {
        settingsData.Smbios.Memory.User[i].ModuleSize = 0;
        settingsData.Smbios.Memory.User[i].InUse = 0;
      }
      settingsData.Smbios.Memory.SlotCounts = 0;
      settingsData.Smbios.Memory.UserChannels = 0;
      settingsData.Smbios.InjectMemoryTables = FALSE;

      // Inject memory tables into SMBIOS
      const TagDict* MemoryDict = SMBIOSDict->dictPropertyForKey("Memory");
      if (MemoryDict != NULL){
        // Get memory table count
        const TagStruct* Prop2   = MemoryDict->propertyForKey("SlotCount");
        settingsData.Smbios.Memory.SlotCounts = (UINT8)GetPropertyAsInteger(Prop2, 0);
        // Get memory channels
        Prop2             = MemoryDict->propertyForKey("Channels");
        settingsData.Smbios.Memory.UserChannels = (UINT8)GetPropertyAsInteger(Prop2, 0);
        // Get memory tables
        const TagArray* ModulesArray = MemoryDict->arrayPropertyForKey("Modules"); // array of dict
        if (ModulesArray != NULL) {
          INTN Count = ModulesArray->arrayContent().size();
          for (i = 0; i < Count; i++) {
            const TagDict* Prop3 = ModulesArray->dictElementAt(i, "SMBIOS/Memory/Modules"_XS8);
            int Slot = MAX_RAM_SLOTS;
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

            SlotPtr = &settingsData.Smbios.Memory.User[Slot];

            // Get memory size
            Prop2 = Prop3->propertyForKey("Size");
            SlotPtr->ModuleSize = (UINT32)GetPropertyAsInteger(Prop2, SlotPtr->ModuleSize);
            // Get memory frequency
            Prop2 = Prop3->propertyForKey("Frequency");
            SlotPtr->Frequency  = (UINT32)GetPropertyAsInteger(Prop2, SlotPtr->Frequency);
            // Get memory vendor
            Prop2 = Prop3->propertyForKey("Vendor");
            if (Prop2 && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              SlotPtr->Vendor.S8Printf("%s", Prop2->getString()->stringValue().c_str());
            }
            // Get memory part number
            Prop2 = Prop3->propertyForKey("Part");
            if (Prop2 && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              SlotPtr->PartNo.S8Printf("%s", Prop2->getString()->stringValue().c_str());
            }
            // Get memory serial number
            Prop2 = Prop3->propertyForKey("Serial");
            if (Prop2 && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              SlotPtr->SerialNo.S8Printf("%s", Prop2->getString()->stringValue().c_str());
            }
            // Get memory type
            SlotPtr->Type = MemoryTypeDdr3;
            Prop2 = Prop3->propertyForKey("Type");
            if (Prop2 && Prop2->isString() && Prop2->getString()->stringValue().notEmpty()) {
              if (Prop2->getString()->stringValue().isEqualIC("DDR2")) {
                SlotPtr->Type = MemoryTypeDdr2;
              } else if (Prop2->getString()->stringValue().isEqualIC("DDR3")) {
                SlotPtr->Type = MemoryTypeDdr3;
              } else if (Prop2->getString()->stringValue().isEqualIC("DDR4")) {
                SlotPtr->Type = MemoryTypeDdr4;
              } else if (Prop2->getString()->stringValue().isEqualIC("DDR")) {
                SlotPtr->Type = MemoryTypeDdr;
              }
            }

            SlotPtr->InUse = (SlotPtr->ModuleSize > 0);
            if (SlotPtr->InUse) {
              if (settingsData.Smbios.Memory.SlotCounts <= Slot) {
                settingsData.Smbios.Memory.SlotCounts = Slot + 1;
              }
            }
          }

          if (settingsData.Smbios.Memory.SlotCounts > 0) {
            settingsData.Smbios.InjectMemoryTables = TRUE;
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
            if (Prop2->getString()->stringValue().isEqualIC("ATI")) {
              DeviceN = 0;
            } else if (Prop2->getString()->stringValue().isEqualIC("NVidia")) {
              DeviceN = 1;
            } else if (Prop2->getString()->stringValue().isEqualIC("IntelGFX")) {
              DeviceN = 2;
            } else if (Prop2->getString()->stringValue().isEqualIC("LAN")) {
              DeviceN = 5;
            } else if (Prop2->getString()->stringValue().isEqualIC("WIFI")) {
              DeviceN = 6;
            } else if (Prop2->getString()->stringValue().isEqualIC("Firewire")) {
              DeviceN = 12;
            } else if (Prop2->getString()->stringValue().isEqualIC("HDMI")) {
              DeviceN = 4;
            } else if (Prop2->getString()->stringValue().isEqualIC("USB")) {
              DeviceN = 11;
            } else if (Prop2->getString()->stringValue().isEqualIC("NVME")) {
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
            SLOT_DEVICE *SlotDevice = &settingsData.Smbios.SlotDevices[DeviceN];
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
              SlotDevice->SlotName = Prop2->getString()->stringValue();
            } else {
              SlotDevice->SlotName.S8Printf("PCI Slot %lld", DeviceN);
            }

            DBG(" - %s\n", SlotDevice->SlotName.c_str());
          }
        }
      }
    }

    //CPU
    settingsData.CPU.CpuType = GetAdvancedCpuType(); //let it be default
    settingsData.CPU.SavingMode = 0xFF; //default
    const TagDict* CPUDict = CfgDict->dictPropertyForKey("CPU");
    if (CPUDict != NULL) {
      const TagStruct* Prop = CPUDict->propertyForKey("QPI");
      if (Prop != NULL) {
        settingsData.CPU.QPI = (UINT16)GetPropertyAsInteger(Prop, settingsData.CPU.QPI);
        DBG("QPI: %dMHz\n", settingsData.CPU.QPI);
      }

      Prop = CPUDict->propertyForKey("FrequencyMHz");
      if (Prop != NULL) {
        settingsData.CPU.CpuFreqMHz = (UINT32)GetPropertyAsInteger(Prop, settingsData.CPU.CpuFreqMHz);
        DBG("CpuFreq: %dMHz\n", settingsData.CPU.CpuFreqMHz);
      }

      Prop = CPUDict->propertyForKey("Type");
      if (Prop != NULL) {
        settingsData.CPU.CpuType = (UINT16)GetPropertyAsInteger(Prop, settingsData.CPU.CpuType);
        DBG("CpuType: %hX\n", settingsData.CPU.CpuType);
      }

      Prop = CPUDict->propertyForKey("QEMU");
      settingsData.CPU.QEMU = IsPropertyNotNullAndTrue(Prop);
      if (settingsData.CPU.QEMU) {
        DBG("QEMU: true\n");
      }

      Prop = CPUDict->propertyForKey("UseARTFrequency");
      if (Prop != NULL) {
        settingsData.CPU.UseARTFreq = IsPropertyNotNullAndTrue(Prop);
      }

      settingsData.CPU.UserChange = FALSE;
      Prop = CPUDict->propertyForKey("BusSpeedkHz");
      if (Prop != NULL) {
        settingsData.CPU.BusSpeed = (UINT32)GetPropertyAsInteger(Prop, settingsData.CPU.BusSpeed);
        DBG("BusSpeed: %dkHz\n", settingsData.CPU.BusSpeed);
        settingsData.CPU.UserChange = TRUE;
      }

      Prop = CPUDict->propertyForKey("C6");
      if (Prop != NULL) {
//        settingsData.ACPI.SSDT.EnableC6 = IsPropertyNotNullAndTrue(Prop);
        settingsData.CPU._EnableC6 = IsPropertyNotNullAndTrue(Prop);
      }

      Prop = CPUDict->propertyForKey("C4");
      if (Prop != NULL) {
//        settingsData.ACPI.SSDT.EnableC4 = IsPropertyNotNullAndTrue(Prop);
        settingsData.CPU._EnableC4 = IsPropertyNotNullAndTrue(Prop);
      }

      Prop = CPUDict->propertyForKey("C2");
      if (Prop != NULL) {
//        settingsData.ACPI.SSDT.EnableC2 = IsPropertyNotNullAndTrue(Prop);
        settingsData.CPU._EnableC2 = IsPropertyNotNullAndTrue(Prop);
      }

      //Usually it is 0x03e9, but if you want Turbo, you may set 0x00FA
      Prop                 = CPUDict->propertyForKey("Latency");
//      settingsData.ACPI.SSDT.C3Latency  = (UINT16)GetPropertyAsInteger(Prop, settingsData.ACPI.SSDT.C3Latency);
      if ( Prop != NULL ) settingsData.CPU._C3Latency  = (UINT16)GetPropertyAsInteger(Prop, 0);

      Prop                 = CPUDict->propertyForKey("SavingMode");
      settingsData.CPU.SavingMode = (UINT8)GetPropertyAsInteger(Prop, 0xFF); //the default value means not set

      Prop                 = CPUDict->propertyForKey("HWPEnable");
      settingsData.CPU.HWPEnable = IsPropertyNotNullAndTrue(Prop);
//      if (settingsData.CPU.HWPEnable && (gCPUStructure.Model >= CPU_MODEL_SKYLAKE_U)) {
//        GlobalConfig.HWP = TRUE;
//#ifdef CLOVER_BUILD
//        AsmWriteMsr64 (MSR_IA32_PM_ENABLE, 1);
//#endif
//      }
      Prop                 = CPUDict->propertyForKey("HWPValue");
      if ( Prop ) settingsData.CPU.HWPValue = (UINT32)GetPropertyAsInteger(Prop, 0);
//      if (Prop && GlobalConfig.HWP) {
//#ifdef CLOVER_BUILD
//        AsmWriteMsr64 (MSR_IA32_HWP_REQUEST, settingsData.CPU.HWPValue);
//#endif
//      }

      Prop                 = CPUDict->propertyForKey("TDP");
      settingsData.CPU.TDP  = (UINT8)GetPropertyAsInteger(Prop, 0);

      Prop                 = CPUDict->propertyForKey("TurboDisable");
      if (Prop && IsPropertyNotNullAndTrue(Prop)) {
#ifdef CLOVER_BUILD
        UINT64 msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
#endif
        settingsData.CPU.TurboDisabled = 1;
#ifdef CLOVER_BUILD
        msr &= ~(1ULL<<38);
        AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, msr);
#endif
      }
    }

    // RtVariables
    settingsData.RtVariables.RtROMAsData.setEmpty();
    const TagDict* RtVariablesDict = CfgDict->dictPropertyForKey("RtVariables");
    if (RtVariablesDict != NULL) {
      // ROM: <data>bin data</data> or <string>base 64 encoded bin data</string>
      const TagStruct* Prop = RtVariablesDict->propertyForKey("ROM");
      if (Prop != NULL) {
        if ( Prop->isString()  &&  Prop->getString()->stringValue().isEqualIC("UseMacAddr0") ) {
          settingsData.RtVariables.RtROMAsString = Prop->getString()->stringValue();
        } else if ( Prop->isString()  &&  Prop->getString()->stringValue().isEqualIC("UseMacAddr1") ) {
          settingsData.RtVariables.RtROMAsString = Prop->getString()->stringValue();
        } else if ( Prop->isString()  ||  Prop->isData() ) { // GetDataSetting accept both
          UINTN ROMLength         = 0;
          uint8_t* ROM = GetDataSetting(RtVariablesDict, "ROM", &ROMLength);
          settingsData.RtVariables.RtROMAsData.stealValueFrom(ROM, ROMLength);
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
            settingsData.RtVariables.RtMLBSetting = Prop->getString()->stringValue();
          }
        }
      }
      // CsrActiveConfig
      Prop = RtVariablesDict->propertyForKey("CsrActiveConfig");
      settingsData.RtVariables.CsrActiveConfig = (UINT32)GetPropertyAsInteger(Prop, 0x2E7); //the value 0xFFFF means not set

      //BooterConfig
      Prop = RtVariablesDict->propertyForKey("BooterConfig");
      settingsData.RtVariables.BooterConfig = (UINT16)GetPropertyAsInteger(Prop, 0); //the value 0 means not set
      //let it be string like "log=0"
      Prop = RtVariablesDict->propertyForKey("BooterCfg");
      if ( Prop != NULL ) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in RtVariables/BooterCfg\n");
        }else{
          if( Prop->getString()->stringValue().notEmpty() ) {
            settingsData.RtVariables.BooterCfgStr = Prop->getString()->stringValue();
          }
        }
      }
      //Block external variables
      const TagArray* BlockArray = RtVariablesDict->arrayPropertyForKey("Block"); // array of dict
      if (BlockArray != NULL) {
        INTN   i;
        INTN Count = BlockArray->arrayContent().size();
        settingsData.RtVariables.BlockRtVariableArray.setEmpty();
        SETTINGS_DATA::RtVariablesClass::RT_VARIABLES* RtVariablePtr = new SETTINGS_DATA::RtVariablesClass::RT_VARIABLES();
        SETTINGS_DATA::RtVariablesClass::RT_VARIABLES& RtVariable = *RtVariablePtr;
        for (i = 0; i < Count; i++) {
          const TagDict* BlockDict = BlockArray->dictElementAt(i, "Block"_XS8);
          const TagStruct* Prop2 = BlockDict->propertyForKey("Comment");
          if ( Prop2 != NULL ) {
            if ( !Prop2->isString() ) {
              MsgLog("ATTENTION : property not string in Block/Comment\n");
            }else{
              if( Prop2->getString()->stringValue().notEmpty() ) {
                DBG(" %s\n", Prop2->getString()->stringValue().c_str());
                RtVariable.Comment = Prop2->getString()->stringValue();
              }
            }
          }
          Prop2 = BlockDict->propertyForKey("Disabled");
          if (IsPropertyNotNullAndTrue(Prop2)) {
            RtVariable.Disabled = true;
//            continue;
          }
          Prop2 = BlockDict->propertyForKey("Guid");
          if ( Prop2 != NULL ) {
            if ( !Prop2->isString() ) {
              MsgLog("ATTENTION : property not string in Block/Guid\n");
            }else{
              if( Prop2->getString()->stringValue().notEmpty() ) {
                if (IsValidGuidString(Prop2->getString()->stringValue())) {
                  StrToGuidLE(Prop2->getString()->stringValue(), &RtVariable.Guid);
                }else{
                  StrToGuidLE(nullGuidAsString, &RtVariable.Guid);
                  DBG("Error: invalid GUID for RT var '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop2->getString()->stringValue().c_str());
                }
              }
            }
          }

          Prop2 = BlockDict->propertyForKey("Name");
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
          settingsData.RtVariables.BlockRtVariableArray.AddReference(RtVariablePtr, true);
        }
      }
    }

//    if (settingsData.RtVariables.RtROM.isEmpty()) {
//      EFI_GUID uuid;
//      StrToGuidLE(settingsData.Smbios.SmUUID, &uuid);
//      settingsData.RtVariables.RtROM.ncpy(&uuid.Data4[2], 6);
//    }

//    if (settingsData.RtVariables.RtMLB.isEmpty()) {
//      settingsData.RtVariables.RtMLB       = settingsData.Smbios.BoardSerialNumber;
//    }

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
        settingsData.SystemParameters.BacklightLevel       = (UINT16)GetPropertyAsInteger(Prop, settingsData.SystemParameters.BacklightLevel);
        settingsData.SystemParameters.BacklightLevelConfig = TRUE;
      }

      Prop = SystemParametersDict->propertyForKey("CustomUUID");
      if (Prop != NULL) {
        if ( !Prop->isString() ) {
          MsgLog("ATTENTION : property not string in SystemParameters/CustomUUID\n");
        }else{
          if (IsValidGuidString(Prop->getString()->stringValue())) {
          settingsData.SystemParameters.CustomUuid = Prop->getString()->stringValue();
            // if CustomUUID specified, then default for InjectSystemID=FALSE
            // to stay compatibile with previous Clover behaviour
            DBG("The UUID is valid\n");
          }else{
            DBG("Error: invalid CustomUUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX\n", Prop->getString()->stringValue().c_str());
            settingsData.SystemParameters.CustomUuid = {0};
          }
        }
      }
      //else gUuid value from SMBIOS
      //     DBG("Finally use %s\n", strguid(&gUuid));

      settingsData.SystemParameters._InjectSystemID = 2;
      Prop                     = SystemParametersDict->propertyForKey("InjectSystemID");
      if ( Prop ) {
        if ( Prop->isBool() ) settingsData.SystemParameters._InjectSystemID = Prop->getBool()->boolValue();
        else if (  Prop->isString() ) {
          // TODO a function that takes a string and return if it's true or false
          if ( Prop->getString()->stringValue().isEqualIC("true") ) settingsData.SystemParameters._InjectSystemID = 1;
          else if ( Prop->getString()->stringValue()[0] == 'y' ) settingsData.SystemParameters._InjectSystemID = 1;
          else if ( Prop->getString()->stringValue()[0] == 'Y' ) settingsData.SystemParameters._InjectSystemID = 1;
          else if ( Prop->getString()->stringValue().isEqualIC("false") ) settingsData.SystemParameters._InjectSystemID = 0;
          else if ( Prop->getString()->stringValue().isEqualIC("n") ) settingsData.SystemParameters._InjectSystemID = 0;
          else if ( Prop->getString()->stringValue().isEqualIC("N") ) settingsData.SystemParameters._InjectSystemID = 0;
          else {
            DBG("MALFORMED PLIST : SMBIOS/InjectSystemID must be true, yes, false, no, or non existant");
          }
        }else{
          DBG("MALFORMED PLIST : SMBIOS/InjectSystemID must be <true/>, <false/> or non existant");
        }
      }

      Prop                     = SystemParametersDict->propertyForKey("NvidiaWeb");
      settingsData.SystemParameters.NvidiaWeb      = IsPropertyNotNullAndTrue(Prop);

    }


    const TagDict* BootGraphicsDict = CfgDict->dictPropertyForKey("BootGraphics");
    if (BootGraphicsDict != NULL) {
      const TagStruct* Prop = BootGraphicsDict->propertyForKey("DefaultBackgroundColor");
      settingsData.BootGraphics.DefaultBackgroundColor = (UINT32)GetPropertyAsInteger(Prop, 0x80000000); //the value 0x80000000 means not set

      Prop = BootGraphicsDict->propertyForKey("UIScale");
      settingsData.BootGraphics.UIScale = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

      Prop = BootGraphicsDict->propertyForKey("EFILoginHiDPI");
      settingsData.BootGraphics.EFILoginHiDPI = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

      Prop = BootGraphicsDict->propertyForKey("flagstate");
      settingsData.BootGraphics._flagstate = (UINT32)GetPropertyAsInteger(Prop, 0x80000000);

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
    SaveSettings(settingsData);
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




EFI_STATUS
static SaveSettings(SETTINGS_DATA& settingsData)
{
  // TODO: SetVariable()..
  // here we can apply user settings instead of default one
  gMobile                       = settingsData.Smbios.Mobile;

  if ((settingsData.CPU.BusSpeed != 0) && (settingsData.CPU.BusSpeed > 10 * Kilo) && (settingsData.CPU.BusSpeed < 500 * Kilo)) {
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
        gCPUStructure.ExternalClock = settingsData.CPU.BusSpeed;
        //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
        break;
      default:
        //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(settingsData.BusSpeed, Kilo)));

        // for sandy bridge or newer
        // to match ExternalClock 25 MHz like real mac, divide BusSpeed by 4
        gCPUStructure.ExternalClock = (settingsData.CPU.BusSpeed + 3) / 4;
        //DBG("Corrected ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
        break;
    }

    gCPUStructure.FSBFrequency  = MultU64x64 (settingsData.CPU.BusSpeed, Kilo); //kHz -> Hz
    gCPUStructure.MaxSpeed      = (UINT32)(DivU64x32 ((UINT64)settingsData.CPU.BusSpeed * gCPUStructure.MaxRatio, 10000)); //kHz->MHz
  }

  if ((settingsData.CPU.CpuFreqMHz > 100) && (settingsData.CPU.CpuFreqMHz < 20000)) {
    gCPUStructure.MaxSpeed      = settingsData.CPU.CpuFreqMHz;
  }

  // to determine the use of Table 132
  if (settingsData.CPU.QPI) {
    GlobalConfig.SetTable132 = TRUE;
    //DBG("QPI: use Table 132\n");
  }
  else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
      case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
      case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
      case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
        GlobalConfig.SetTable132 = TRUE;
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




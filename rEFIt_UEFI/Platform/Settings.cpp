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
#include "../include/QuirksCodes.h"
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
#include "../Settings/SelfOem.h"
#include "ati_reg.h"
#include "ati.h"
#include "nvidia.h"
#include "gma.h"
#include "Edid.h"
#include "hda.h"
#include "../entry_scan/bootscreen.h"
#include "../Settings/ConfigPlist/ConfigPlistClass.h"
#include "../Settings/ConfigManager.h"

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

XObjArray<ACPI_PATCHED_AML>     ACPIPatchedAML;
//SYSVARIABLES                    *SysVariables;
CHAR16                          *IconFormat = NULL;

//TagDict*                          gConfigDict[NUM_OF_CONFIGS] = {NULL, NULL, NULL};

SETTINGS_DATA                   gSettings;

//GFX_PROPERTIES                  gGraphics[4]; //no more then 4 graphics cards
//HDA_PROPERTIES                  gAudios[4]; //no more then 4 Audio Controllers
//SLOT_DEVICE                     Arpt;
EFI_EDID_DISCOVERED_PROTOCOL    *EdidDiscovered;
//UINT8                           *gEDID = NULL;
//EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;
//UINT16                          gCPUtype;
//UINTN                           NGFX                        = 0; // number of GFX
//UINTN                           NHDA                        = 0; // number of HDA Devices


XStringWArray                   ThemeNameArray;
UINTN                           ConfigsNum;
CHAR16                          *ConfigsList[20];
UINTN                           DsdtsNum = 0;
CHAR16                          *DsdtsList[20];
XObjArray<HDA_OUTPUTS>          AudioList;

// firmware
BOOLEAN                         gFirmwareClover             = FALSE;
UINTN                           gEvent;
UINT16                          gBacklightLevel;
//BOOLEAN                         defDSM;
//UINT16                          dropDSM;

BOOLEAN                         ResumeFromCoreStorage = false;
//BOOLEAN                         gRemapSmBiosIsRequire;

// QPI
//BOOLEAN                         SetTable132                 = FALSE;

//EG_PIXEL SelectionBackgroundPixel = { 0xef, 0xef, 0xef, 0xff }; //define in lib.h
const INTN BCSMargin = 11;

//
DRIVERS_FLAGS gDriversFlags;  //the initializer is not needed for global variables

EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl = NULL;

// global configuration with default values
REFIT_CONFIG   GlobalConfig;



CONST CHAR8* AudioOutputNames[] = {
  "LineOut",
  "Speaker",
  "Headphones",
  "SPDIF",
  "Garniture",
  "HDMI",
  "Other"
};

const XString8 defaultInstallTitle = "Install macOS"_XS8;
const XString8 defaultRecoveryTitle = "Recovery"_XS8;
const XStringW defaultRecoveryImagePath = L"mac"_XSW;
const XStringW defaultRecoveryDriveImagePath = L"mac"_XSW;
const SETTINGS_DATA::SmbiosClass::SlotDeviceClass SETTINGS_DATA::SmbiosClass::SlotDeviceClass::NullSlotDevice;

CUSTOM_LOADER_ENTRY::CUSTOM_LOADER_ENTRY(const CUSTOM_LOADER_ENTRY_SETTINGS& _settings) : settings(_settings)
{
  if ( settings.ImageData.notEmpty() ) {
    if ( !EFI_ERROR(Image.Image.FromPNG(settings.ImageData.data(), settings.ImageData.size())) ) {
      Image.setFilled();
    }
  }
  if ( settings.DriveImageData.notEmpty() ) {
    if ( !EFI_ERROR(DriveImage.Image.FromPNG(settings.DriveImageData.data(), settings.DriveImageData.size())) ) {
      DriveImage.setFilled();
    }
  }

  if ( settings.CustomLogoTypeSettings == CUSTOM_BOOT_USER  &&  settings.CustomLogoAsXString8.notEmpty() ) {
    CustomLogoImage.LoadXImage(&self.getSelfVolumeRootDir(), settings.CustomLogoAsXString8);
    if (CustomLogoImage.isEmpty()) {
      DBG("Custom boot logo not found at path '%s'!\n", settings.CustomLogoAsXString8.c_str());
      CustomLogoType = CUSTOM_BOOT_DISABLED;
    }
  } else if ( settings.CustomLogoTypeSettings == CUSTOM_BOOT_USER  &&  settings.CustomLogoAsData.notEmpty() ) {
    CustomLogoImage.FromPNG(settings.CustomLogoAsData.data(), settings.CustomLogoAsData.size());
    if (CustomLogoImage.isEmpty()) {
      DBG("Custom boot logo not decoded from data!\n");
      CustomLogoType = CUSTOM_BOOT_DISABLED;
    }
  }
  DBG("Sub entry custom boot %s (0x%llX)\n", CustomBootModeToStr(settings.CustomLogoTypeSettings), (uintptr_t)&CustomLogoImage);

  for ( size_t idx = 0 ; idx < settings.SubEntriesSettings.size() ; ++idx ) {
    const CUSTOM_LOADER_SUBENTRY_SETTINGS& CustomEntrySettings = settings.SubEntriesSettings[idx];
    CUSTOM_LOADER_SUBENTRY* entry = new CUSTOM_LOADER_SUBENTRY(*this, CustomEntrySettings);
    SubEntries.AddReference(entry, true);
  }

  KernelAndKextPatches = gSettings.KernelAndKextPatches; // Jief : why do we have a duplicated KernelAndKextPatches var inside CUSTOM_LOADER_ENTRY ?

}

XString8Array CUSTOM_LOADER_SUBENTRY::getLoadOptions() const
{
  if ( settings._Arguments.isDefined() ) return Split<XString8Array>(settings._Arguments.value(), " ");
  XString8Array LoadOptions = parent.getLoadOptions();
  LoadOptions.import(Split<XString8Array>(settings._AddArguments, " "));
  if (LoadOptions.isEmpty() && OSTYPE_IS_WINDOWS(parent.settings.Type)) {
    LoadOptions.Add("-s");
    LoadOptions.Add("-h");
  }

  return LoadOptions;
}

UINT8 CUSTOM_LOADER_SUBENTRY::getFlags(bool NoCachesDefault) const
{
  UINT8 Flags = parent.getFlags(NoCachesDefault);
  if ( settings._Arguments.isDefined() ) Flags = OSFLAG_SET(Flags, OSFLAG_NODEFAULTARGS);
  if ( settings._NoCaches.isDefined() ) {
    if ( settings._NoCaches.value() ) {
      Flags = OSFLAG_SET(Flags, OSFLAG_NOCACHES);
    } else {
      if (NoCachesDefault) {
        Flags = OSFLAG_SET(Flags, OSFLAG_NOCACHES);
      }
    }
  }
  return Flags;
}

XString8Array CUSTOM_LOADER_ENTRY::getLoadOptions() const
{
  if ( settings.Arguments.isDefined() ) return Split<XString8Array>(settings.Arguments.value(), " ");
  XString8Array LoadOptions;
  LoadOptions.import(Split<XString8Array>(settings.AddArguments, " "));
  if (LoadOptions.isEmpty() && OSTYPE_IS_WINDOWS(settings.Type)) {
    LoadOptions.Add("-s");
    LoadOptions.Add("-h");
  }

  return LoadOptions;
}
const XString8& CUSTOM_LOADER_SUBENTRY::getTitle() const {
  if ( settings._Title.isDefined() ) return settings._Title.value();
  if ( settings._FullTitle.isDefined() ) return NullXString8;
  return parent.settings.dgetTitle();
};
const XString8& CUSTOM_LOADER_SUBENTRY::getFullTitle() const {
  if ( settings._FullTitle.isDefined() ) return settings._FullTitle.value();
  if ( settings._Title.isDefined() ) return NullXString8;
  return parent.settings.FullTitle;
};


bool SETTINGS_DATA::GUIClass::getDarkEmbedded(bool isDaylight) const {
  if ( EmbeddedThemeType.isEqualIC("Dark") ) return true;
  if ( EmbeddedThemeType.isEqualIC("Daytime") ) return !isDaylight;
  return false;
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



void afterGetUserSettings(SETTINGS_DATA& settingsData)
{

  // Secure boot
  /* this parameter, which should be called SecureBootSetupMode, is ignored if :
   *   it is true
   *   SecureBoot is already true.
   */
  if ( settingsData.Boot.SecureSetting == 0 ) {
    // Only disable setup mode, we want always secure boot
    GlobalConfig.SecureBootSetupMode = 0;
  } else if ( settingsData.Boot.SecureSetting == 1  &&  !GlobalConfig.SecureBoot  ) {
    // This mode will force boot policy even when no secure boot or it is disabled
    GlobalConfig.SecureBootSetupMode = 1;
    GlobalConfig.SecureBoot          = 1;
  }


  //set to drop
  GlobalConfig.DropSSDT = settingsData.ACPI.SSDT.DropSSDTSetting;
  if (GlobalConfig.ACPIDropTables) {
    for ( size_t idx = 0 ; idx < settingsData.ACPI.ACPIDropTablesArray.size() ; ++idx)
    {
      ACPI_DROP_TABLE *DropTable = GlobalConfig.ACPIDropTables;
      DBG(" - [%02zd]: Drop table : %08X, %16llx : ", idx, settingsData.ACPI.ACPIDropTablesArray[idx].Signature, settingsData.ACPI.ACPIDropTablesArray[idx].TableId);
      bool Dropped = FALSE;
      while (DropTable) {
        if (((settingsData.ACPI.ACPIDropTablesArray[idx].Signature == DropTable->Signature) &&
             (!settingsData.ACPI.ACPIDropTablesArray[idx].TableId || (DropTable->TableId == settingsData.ACPI.ACPIDropTablesArray[idx].TableId)) &&
             (!settingsData.ACPI.ACPIDropTablesArray[idx].TabLength || (DropTable->Length == settingsData.ACPI.ACPIDropTablesArray[idx].TabLength))) ||
            (!settingsData.ACPI.ACPIDropTablesArray[idx].Signature && (DropTable->TableId == settingsData.ACPI.ACPIDropTablesArray[idx].TableId))) {
          DropTable->MenuItem.BValue = TRUE;
          DropTable->OtherOS = settingsData.ACPI.ACPIDropTablesArray[idx].OtherOS;
          GlobalConfig.DropSSDT         = FALSE; // if one item=true then dropAll=false by default
          //DBG(" true");
          Dropped = TRUE;
        }
        DropTable = DropTable->Next;
      }
      DBG(" %s\n", Dropped ? "yes" : "no");
    }
  }

  // Whether or not to draw boot screen
  GlobalConfig.CustomLogoType = settingsData.Boot.CustomLogoType;
  if ( settingsData.Boot.CustomLogoType == CUSTOM_BOOT_USER  &&  settingsData.Boot.CustomLogoAsXString8.notEmpty() ) {
    if (GlobalConfig.CustomLogo != NULL) {
      delete GlobalConfig.CustomLogo;
    }
    GlobalConfig.CustomLogo = new XImage;
    GlobalConfig.CustomLogo->LoadXImage(&self.getSelfVolumeRootDir(), settingsData.Boot.CustomLogoAsXString8);
    if (GlobalConfig.CustomLogo->isEmpty()) {
      DBG("Custom boot logo not found at path '%s'!\n", settingsData.Boot.CustomLogoAsXString8.c_str());
      GlobalConfig.CustomLogoType = CUSTOM_BOOT_DISABLED;
    }
  } else if ( settingsData.Boot.CustomLogoType == CUSTOM_BOOT_USER  &&  settingsData.Boot.CustomLogoAsData.notEmpty() ) {
    if (GlobalConfig.CustomLogo != NULL) {
      delete GlobalConfig.CustomLogo;
    }
    GlobalConfig.CustomLogo = new XImage;
    GlobalConfig.CustomLogo->FromPNG(settingsData.Boot.CustomLogoAsData.data(), settingsData.Boot.CustomLogoAsData.size());
    if (GlobalConfig.CustomLogo->isEmpty()) {
      DBG("Custom boot logo not decoded from data!\n"/*, Prop->getString()->stringValue().c_str()*/);
      GlobalConfig.CustomLogoType = CUSTOM_BOOT_DISABLED;
    }
  }
  DBG("Custom boot %s (0x%llX)\n", CustomBootModeToStr(GlobalConfig.CustomLogoType), (uintptr_t)GlobalConfig.CustomLogo);

  GlobalConfig.EnableC6 = settingsData.getEnableC6();
  GlobalConfig.EnableC4 = settingsData.getEnableC4();
  GlobalConfig.EnableC2 = settingsData.getEnableC2();
//  GlobalConfig.C3Latency = settingsData.getC3Latency();
//  DBG("4: GlobalConfig.C3Latency=%x\n", GlobalConfig.C3Latency);

  if (settingsData.CPU.HWPEnable && (gCPUStructure.Model >= CPU_MODEL_SKYLAKE_U)) {
    GlobalConfig.HWP = TRUE;
    AsmWriteMsr64 (MSR_IA32_PM_ENABLE, 1);
    if ( settingsData.CPU.HWPValue.isDefined() ) {
      AsmWriteMsr64 (MSR_IA32_HWP_REQUEST, settingsData.CPU.HWPValue.value());
    }
  }

  for ( size_t idx = 0 ; idx < settingsData.GUI.CustomEntriesSettings.size() ; ++idx ) {
    const CUSTOM_LOADER_ENTRY_SETTINGS& CustomEntrySettings = settingsData.GUI.CustomEntriesSettings[idx];
    CUSTOM_LOADER_ENTRY* entry = new CUSTOM_LOADER_ENTRY(CustomEntrySettings);
    GlobalConfig.CustomEntries.AddReference(entry, true);
  }

  for ( size_t idx = 0 ; idx < settingsData.GUI.CustomLegacySettings.size() ; ++idx ) {
    const CUSTOM_LEGACY_ENTRY_SETTINGS& CustomLegacySettings = settingsData.GUI.CustomLegacySettings[idx];
    CUSTOM_LEGACY_ENTRY* entry = new CUSTOM_LEGACY_ENTRY(CustomLegacySettings, ThemeX.getThemeDir());
    GlobalConfig.CustomLegacyEntries.AddReference(entry, true);
  }

  for ( size_t idx = 0 ; idx < settingsData.GUI.CustomToolSettings.size() ; ++idx ) {
    const CUSTOM_TOOL_ENTRY_SETTINGS& CustomToolSettings = settingsData.GUI.CustomToolSettings[idx];
    CUSTOM_TOOL_ENTRY* entry = new CUSTOM_TOOL_ENTRY(CustomToolSettings, ThemeX.getThemeDir());
    GlobalConfig.CustomToolsEntries.AddReference(entry, true);
  }

  if ( settingsData.GUI.Theme.notEmpty() )
  {
    ThemeX.Theme.takeValueFrom(settingsData.GUI.Theme);
    DBG("Default theme: %ls\n", settingsData.GUI.Theme.wc_str());

    OldChosenTheme = 0xFFFF; //default for embedded
    for (UINTN i = 0; i < ThemeNameArray.size(); i++) {
      //now comparison is case sensitive
      if ( settingsData.GUI.Theme.isEqualIC(ThemeNameArray[i]) ) {
        OldChosenTheme = i;
        break;
      }
    }
  }

  EFI_TIME          Now;
  gRT->GetTime(&Now, NULL);
  if (settingsData.GUI.Timezone != 0xFF) {
    INT32 NowHour = Now.Hour + settingsData.GUI.Timezone;
    if (NowHour <  0 ) NowHour += 24;
    if (NowHour >= 24 ) NowHour -= 24;
    ThemeX.Daylight = (NowHour > 8) && (NowHour < 20);
  } else {
    ThemeX.Daylight = TRUE;
  }

  ThemeX.DarkEmbedded = settingsData.GUI.getDarkEmbedded(ThemeX.Daylight);

  if ( settingsData.GUI.languageCode == english ) {
    GlobalConfig.Codepage = 0xC0;
    GlobalConfig.CodepageSize = 0;
  } else if ( settingsData.GUI.languageCode == russian ) {
    GlobalConfig.Codepage = 0x410;
    GlobalConfig.CodepageSize = 0x40;
  } else if ( settingsData.GUI.languageCode == ukrainian ) {
    GlobalConfig.Codepage = 0x400;
    GlobalConfig.CodepageSize = 0x60;
  } else if ( settingsData.GUI.languageCode == chinese ) {
    GlobalConfig.Codepage = 0x3400;
    GlobalConfig.CodepageSize = 0x19C0;
  } else if ( settingsData.GUI.languageCode == korean ) {
    GlobalConfig.Codepage = 0x1100;
    GlobalConfig.CodepageSize = 0x100;
  }

  if (settingsData.Graphics.EDID.InjectEDID){
    //DBG("Inject EDID\n");
    if ( settingsData.Graphics.EDID.CustomEDID.size() > 0  &&  settingsData.Graphics.EDID.CustomEDID.size() % 128 == 0 ) {
      InitializeEdidOverride();
    }
  }

  GlobalConfig.KPKernelPm = settingsData.KernelAndKextPatches._KPKernelPm || GlobalConfig.NeedPMfix;
  GlobalConfig.KPAppleIntelCPUPM = settingsData.KernelAndKextPatches._KPAppleIntelCPUPM || GlobalConfig.NeedPMfix;

  if ( settingsData.RtVariables.RtROMAsString.isEqualIC("UseMacAddr0") ) {
    if ( gConf.LanCardArray.size() > 0 ) GlobalConfig.RtROM.ncpy(&gConf.LanCardArray[0].MacAddress[0], 6);
    else GlobalConfig.RtROM.memset(0, 6);
  } else if ( settingsData.RtVariables.RtROMAsString.isEqualIC("UseMacAddr1") ) {
    if ( gConf.LanCardArray.size() > 1 ) GlobalConfig.RtROM.ncpy(&gConf.LanCardArray[1].MacAddress[0], 6);
    else GlobalConfig.RtROM.memset(0, 6);
  }else{
    GlobalConfig.RtROM = settingsData.RtVariables.RtROMAsData;
  }
  if ( GlobalConfig.RtROM.isEmpty() ) {
    EFI_GUID uuid;
    StrToGuidBE(settingsData.Smbios.SmUUID, &uuid);
    GlobalConfig.RtROM.ncpy(&uuid.Data4[2], 6);
  }
  GlobalConfig.RtMLB = settingsData.RtVariables.RtMLBSetting;
  if ( GlobalConfig.RtMLB.isEmpty() ) {
    GlobalConfig.RtMLB = settingsData.Smbios.BoardSerialNumber;
  }

  for (size_t idx = 0 ; idx < gConf.GfxPropertiesArrayNonConst.size() ; idx++ ) {
    gConf.GfxPropertiesArrayNonConst[idx].LoadVBios = settingsData.Graphics.LoadVBios; //default
  }

  if ( settingsData.CPU.TurboDisabled ) {
    GlobalConfig.Turbo = false;
  }else{
    GlobalConfig.Turbo = gCPUStructure.Turbo;
  }

  // Jief : Shouldn't this injection made at StartLoader only ? And only for macOS ?
  if ( settingsData.Devices.Properties.propertiesAsString.notEmpty() )
  {
    size_t binaryPropSize = hex2bin(settingsData.Devices.Properties.propertiesAsString, NULL, 0); // check of correct length is supposed to have been done when reading settings.
    UINTN nbPages = EFI_SIZE_TO_PAGES(binaryPropSize);
    EFI_PHYSICAL_ADDRESS  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS; //0xFE000000;
    EFI_STATUS Status = gBS->AllocatePages (
                                 AllocateMaxAddress,
                                 EfiACPIReclaimMemory,
                                 nbPages,
                                 &BufferPtr
                                 );

    if (!EFI_ERROR(Status)) {
      cProperties = (UINT8*)(UINTN)BufferPtr;
      cPropSize = (UINT32)hex2bin(settingsData.Devices.Properties.propertiesAsString, cProperties, EFI_PAGES_TO_SIZE(nbPages)); // cast should be safe hex2bin return  < MAX_UINT32

      DBG("Injected EFIString of length %d\n", cPropSize);
    }else{
      MsgLog("AllocatePages failed (%s), Properties not injected", efiStrError(Status));
    }
  }
  //---------
  GlobalConfig.IgPlatform = settingsData.Graphics._IgPlatform;
  for ( size_t idx = 0 ; idx < settingsData.Devices.ArbitraryArray.size() ; ++idx ) {
    const SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass& arbitraryProperty = settingsData.Devices.ArbitraryArray[idx];
    for ( size_t jdx = 0 ; jdx < arbitraryProperty.CustomPropertyArray.size() ; ++jdx ) {
      const SETTINGS_DATA::DevicesClass::SimplePropertyClass& customProperty = arbitraryProperty.CustomPropertyArray[jdx];
      if ( customProperty.Key.contains("-platform-id") ) {
        memcpy(&GlobalConfig.IgPlatform, customProperty.Key.data(), 4);
      }
    }
  }
}


//void
//GetDevices ()
//{
//  EFI_STATUS          Status;
//  UINTN               HandleCount  = 0;
//  EFI_HANDLE          *HandleArray = NULL;
//  EFI_PCI_IO_PROTOCOL *PciIo;
//  PCI_TYPE00          Pci;
//  UINTN               Index;
//  UINTN               Segment      = 0;
//  UINTN               Bus          = 0;
//  UINTN               Device       = 0;
//  UINTN               Function     = 0;
//  UINTN               i;
//  UINT32              Bar0;
//  //  UINT8               *Mmio        = NULL;
//  radeon_card_info_t  *info;
//  SLOT_DEVICE         *SlotDevice;
//
//  NGFX = 0;
//  NHDA = 0;
//  AudioList.setEmpty();
//  //Arpt.Valid = FALSE; //global variables initialized by 0 - c-language
//  XStringW GopDevicePathStr;
//  XStringW DevicePathStr;
//
//  DbgHeader("GetDevices");
//
//  // Get GOP handle, in order to check to which GPU the monitor is currently connected
//  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleArray);
//  if (!EFI_ERROR(Status)) {
//    GopDevicePathStr = DevicePathToXStringW(DevicePathFromHandle(HandleArray[0]));
//    DBG("GOP found at: %ls\n", GopDevicePathStr.wc_str());
//  }
//
//  // Scan PCI handles
//  Status = gBS->LocateHandleBuffer (
//                                    ByProtocol,
//                                    &gEfiPciIoProtocolGuid,
//                                    NULL,
//                                    &HandleCount,
//                                    &HandleArray
//                                    );
//
//  if (!EFI_ERROR(Status)) {
//    for (Index = 0; Index < HandleCount; ++Index) {
//      Status = gBS->HandleProtocol(HandleArray[Index], &gEfiPciIoProtocolGuid, (void **)&PciIo);
//      if (!EFI_ERROR(Status)) {
//        // Read PCI BUS
//        PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
//        Status = PciIo->Pci.Read (
//                                  PciIo,
//                                  EfiPciIoWidthUint32,
//                                  0,
//                                  sizeof (Pci) / sizeof (UINT32),
//                                  &Pci
//                                  );
//
//      DBG("PCI (%02llX|%02llX:%02llX.%02llX) : %04hX %04hX class=%02hhX%02hhX%02hhX\n",
//             Segment,
//             Bus,
//             Device,
//             Function,
//             Pci.Hdr.VendorId,
//             Pci.Hdr.DeviceId,
//             Pci.Hdr.ClassCode[2],
//             Pci.Hdr.ClassCode[1],
//             Pci.Hdr.ClassCode[0]
//             );
//
//        // GFX
//        //if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
//        //    (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA) &&
//        //    (NGFX < 4)) {
//
//        if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
//            ((Pci.Hdr.ClassCode[1] == (PCI_CLASS_DISPLAY_VGA)) ||
//             (Pci.Hdr.ClassCode[1] == (PCI_CLASS_DISPLAY_OTHER))) &&
//            (NGFX < 4)) {
//          CONST CHAR8 *CardFamily = "";
//          UINT16 UFamily;
//          GFX_PROPERTIES *gfx = &gGraphics[NGFX];
//
//          // GOP device path should contain the device path of the GPU to which the monitor is connected
//          DevicePathStr = DevicePathToXStringW(DevicePathFromHandle(HandleArray[Index]));
//          if (StrStr(GopDevicePathStr.wc_str(), DevicePathStr.wc_str())) {
//            DBG(" - GOP: Provided by device\n");
//            if (NGFX != 0) {
//               // we found GOP on a GPU scanned later, make space for this GPU at first position
//               for (i=NGFX; i>0; i--) {
//                 CopyMem(&gGraphics[i], &gGraphics[i-1], sizeof(GFX_PROPERTIES));
//               }
//               ZeroMem(&gGraphics[0], sizeof(GFX_PROPERTIES));
//               gfx = &gGraphics[0]; // GPU with active GOP will be added at the first position
//            }
//          }
//
//          gfx->DeviceID       = Pci.Hdr.DeviceId;
//          gfx->Segment        = Segment;
//          gfx->Bus            = Bus;
//          gfx->Device         = Device;
//          gfx->Function       = Function;
//          gfx->Handle         = HandleArray[Index];
//
//          switch (Pci.Hdr.VendorId) {
//            case 0x1002:
//              info        = NULL;
//              gfx->Vendor = Ati;
//
//              i = 0;
//              do {
//                info      = &radeon_cards[i];
//                if (info->device_id == Pci.Hdr.DeviceId) {
//                  break;
//                }
//              } while (radeon_cards[i++].device_id != 0);
//
//          snprintf (gfx->Model,  64, "%s", info->model_name);
//          snprintf (gfx->Config, 64, "%s", card_configs[info->cfg_name].name);
//              gfx->Ports                  = card_configs[info->cfg_name].ports;
//              DBG(" - GFX: Model=%s (ATI/AMD)\n", gfx->Model);
//
//              //get mmio
//              if (info->chip_family < CHIP_FAMILY_HAINAN) {
//                gfx->Mmio = (UINT8 *)(UINTN)(Pci.Device.Bar[2] & ~0x0f);
//              } else {
//                gfx->Mmio = (UINT8 *)(UINTN)(Pci.Device.Bar[5] & ~0x0f);
//              }
//              gfx->Connectors = *(UINT32*)(gfx->Mmio + RADEON_BIOS_0_SCRATCH);
//              //           DBG(" - RADEON_BIOS_0_SCRATCH = 0x%08X\n", gfx->Connectors);
//              gfx->ConnChanged = FALSE;
//
//              SlotDevice                  = &gSettings.Smbios.SlotDevices[0];
//              SlotDevice->SegmentGroupNum = (UINT16)Segment;
//              SlotDevice->BusNum          = (UINT8)Bus;
//              SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
//              SlotDevice->Valid           = TRUE;
//              SlotDevice->SlotName        = "PCI Slot 0"_XS8;
//              SlotDevice->SlotID          = 1;
//              SlotDevice->SlotType        = SlotTypePciExpressX16;
//              break;
//
//            case 0x8086:
//              gfx->Vendor                 = Intel;
//          snprintf (gfx->Model, 64, "%s", get_gma_model (Pci.Hdr.DeviceId));
//              DBG(" - GFX: Model=%s (Intel)\n", gfx->Model);
//              gfx->Ports = 1;
//              gfx->Connectors = (1 << NGFX);
//              gfx->ConnChanged = FALSE;
//              break;
//
//            case 0x10de:
//              gfx->Vendor = Nvidia;
//              Bar0        = Pci.Device.Bar[0];
//              gfx->Mmio   = (UINT8*)(UINTN)(Bar0 & ~0x0f);
//              //DBG("BAR: 0x%p\n", Mmio);
//              // get card type
//              gfx->Family = (REG32(gfx->Mmio, 0) >> 20) & 0x1ff;
//              UFamily = gfx->Family & 0x1F0;
//              if ((UFamily == NV_ARCH_KEPLER1) ||
//                  (UFamily == NV_ARCH_KEPLER2) ||
//                  (UFamily == NV_ARCH_KEPLER3)) {
//                CardFamily = "Kepler";
//              }
//              else if ((UFamily == NV_ARCH_FERMI1) ||
//                       (UFamily == NV_ARCH_FERMI2)) {
//                CardFamily = "Fermi";
//              }
//              else if ((UFamily == NV_ARCH_MAXWELL1) ||
//                       (UFamily == NV_ARCH_MAXWELL2)) {
//                CardFamily = "Maxwell";
//              }
//              else if (UFamily == NV_ARCH_PASCAL) {
//                CardFamily = "Pascal";
//              }
//              else if (UFamily == NV_ARCH_VOLTA) {
//                CardFamily = "Volta";
//              }
//              else if (UFamily == NV_ARCH_TURING) {
//                CardFamily = "Turing";
//              }
//              else if ((UFamily >= NV_ARCH_TESLA) && (UFamily < 0xB0)) { //not sure if 0xB0 is Tesla or Fermi
//                CardFamily = "Tesla";
//              } else {
//                CardFamily = "NVidia unknown";
//              }
//
//              snprintf (
//                          gfx->Model,
//                          64,
//                          "%s",
//                          get_nvidia_model (((Pci.Hdr.VendorId << 16) | Pci.Hdr.DeviceId),
//                                            ((Pci.Device.SubsystemVendorID << 16) | Pci.Device.SubsystemID),
//                                             NULL) //NULL: get from generic lists
//                          );
//
//            DBG(" - GFX: Model=%s family %hX (%s)\n", gfx->Model, gfx->Family, CardFamily);
//              gfx->Ports                  = 0;
//
//              SlotDevice                  = &gSettings.Smbios.SlotDevices[1];
//              SlotDevice->SegmentGroupNum = (UINT16)Segment;
//              SlotDevice->BusNum          = (UINT8)Bus;
//              SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
//              SlotDevice->Valid           = TRUE;
//              SlotDevice->SlotName = "PCI Slot 0"_XS8;
//              SlotDevice->SlotID          = 1;
//              SlotDevice->SlotType        = SlotTypePciExpressX16;
//              break;
//
//            default:
//              gfx->Vendor = Unknown;
//          snprintf (gfx->Model, 64, "pci%hx,%hx", Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
//              gfx->Ports  = 1;
//              gfx->Connectors = (1 << NGFX);
//              gfx->ConnChanged = FALSE;
//
//              break;
//          }
//
//          NGFX++;
//        }   //if gfx
//
//        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
//                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_OTHER)) {
//          SlotDevice                  = &gSettings.Smbios.SlotDevices[6];
//          SlotDevice->SegmentGroupNum = (UINT16)Segment;
//          SlotDevice->BusNum          = (UINT8)Bus;
//          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
//          SlotDevice->Valid           = TRUE;
//          SlotDevice->SlotName = "AirPort"_XS8;
//          SlotDevice->SlotID          = 0;
//          SlotDevice->SlotType        = SlotTypePciExpressX1;
//          DBG(" - WIFI: Vendor= ");
//          switch (Pci.Hdr.VendorId) {
//            case 0x11ab:
//              DBG("Marvell\n");
//              break;
//            case 0x10ec:
//              DBG("Realtek\n");
//              break;
//            case 0x14e4:
//              DBG("Broadcom\n");
//              break;
//            case 0x1969:
//            case 0x168C:
//              DBG("Atheros\n");
//              break;
//            case 0x1814:
//              DBG("Ralink\n");
//              break;
//            case 0x8086:
//              DBG("Intel\n");
//              break;
//
//            default:
//              DBG(" 0x%04X\n", Pci.Hdr.VendorId);
//              break;
//          }
//        }
//
//        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
//                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_ETHERNET)) {
//          SlotDevice                  = &gSettings.Smbios.SlotDevices[5];
//          SlotDevice->SegmentGroupNum = (UINT16)Segment;
//          SlotDevice->BusNum          = (UINT8)Bus;
//          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
//          SlotDevice->Valid           = TRUE;
//          SlotDevice->SlotName = "Ethernet"_XS8;
//          SlotDevice->SlotID          = 2;
//          SlotDevice->SlotType        = SlotTypePciExpressX1;
//          gLanVendor[nLanCards]       = Pci.Hdr.VendorId;
//          Bar0                        = Pci.Device.Bar[0];
//          gLanMmio[nLanCards++]       = (UINT8*)(UINTN)(Bar0 & ~0x0f);
//          if (nLanCards >= 4) {
//            DBG(" - [!] too many LAN card in the system (upto 4 limit exceeded), overriding the last one\n");
//            nLanCards = 3; // last one will be rewritten
//          }
//          DBG(" - LAN: %llu Vendor=", nLanCards-1);
//          switch (Pci.Hdr.VendorId) {
//            case 0x11ab:
//              DBG("Marvell\n");
//              break;
//            case 0x10ec:
//              DBG("Realtek\n");
//              break;
//            case 0x14e4:
//              DBG("Broadcom\n");
//              break;
//            case 0x1969:
//            case 0x168C:
//              DBG("Atheros\n");
//              break;
//            case 0x8086:
//              DBG("Intel\n");
//              break;
//            case 0x10de:
//              DBG("Nforce\n");
//              break;
//
//            default:
//              DBG("Unknown\n");
//              break;
//          }
//        }
//
//        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
//                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_FIREWIRE)) {
//          SlotDevice = &gSettings.Smbios.SlotDevices[12];
//          SlotDevice->SegmentGroupNum = (UINT16)Segment;
//          SlotDevice->BusNum          = (UINT8)Bus;
//          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
//          SlotDevice->Valid           = TRUE;
//          SlotDevice->SlotName = "FireWire"_XS8;
//          SlotDevice->SlotID          = 3;
//          SlotDevice->SlotType        = SlotTypePciExpressX4;
//        }
//
//        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
//                 ((Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA) ||
//                  (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_AUDIO)) &&
//                 (NHDA < 4)) {
//          HDA_PROPERTIES *hda = &gAudios[NHDA];
//
//          // Populate Controllers IDs
//          hda->controller_vendor_id       = Pci.Hdr.VendorId;
//          hda->controller_device_id       = Pci.Hdr.DeviceId;
//
//          // HDA Controller Info
//          HdaControllerGetName(((hda->controller_device_id << 16) | hda->controller_vendor_id), &hda->controller_name);
//
//
//          if (IsHDMIAudio(HandleArray[Index])) {
//            DBG(" - HDMI Audio: \n");
//
//            SlotDevice = &gSettings.Smbios.SlotDevices[4];
//            SlotDevice->SegmentGroupNum = (UINT16)Segment;
//            SlotDevice->BusNum          = (UINT8)Bus;
//            SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
//            SlotDevice->Valid           = TRUE;
//            SlotDevice->SlotName = "HDMI port"_XS8;
//            SlotDevice->SlotID          = 5;
//            SlotDevice->SlotType        = SlotTypePciExpressX4;
//          }
//          if (gSettings.Devices.Audio.ResetHDA) {
//            //Slice method from VoodooHDA
//            //PCI_HDA_TCSEL_OFFSET = 0x44
//            UINT8 Value = 0;
//            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);
//
//            if (EFI_ERROR(Status)) {
//              continue;
//            }
//
//            Value &= 0xf8;
//            PciIo->Pci.Write (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);
//            //ResetControllerHDA();
//          }
//          NHDA++;
//        } // if Audio device
//      }
//    }
//  }
//}

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
  UINT32              Rcba;
  UINT32              Hptc;
  DevPropDevice *device = NULL;

  GetEdidDiscovered ();

  //First make string from Device->Properties
  device = NULL;
  if (!device_inject_string) {
    device_inject_string = devprop_create_string();
  }
  for ( size_t idx = 0 ; idx < gSettings.Devices.Properties.PropertyArray.size() ; ++idx ) {
    const SETTINGS_DATA::DevicesClass::PropertiesClass::PropertyClass& Prop = gSettings.Devices.Properties.PropertyArray[idx];
//    if (Prop->Device != 0) {
//      Prop = Prop->Next; //skip CustomProperties
//      continue;
//    }
    EFI_DEVICE_PATH_PROTOCOL* DevicePath = Prop.getDevicePath();
    if ( DevicePath == NULL ) {
      MsgLog("Properties with Label=%ls ignored because getDevicePath() return NULL\n", Prop.DevicePathAsString.wc_str());
      continue;
    }
    device = devprop_add_device_pci(device_inject_string, NULL, DevicePath);
    DBG("add device: %ls\n", Prop.DevicePathAsString.wc_str());
    for ( size_t jdx = 0 ; jdx < Prop.propertiesArray.size() ; ++jdx ) {
      const SETTINGS_DATA::DevicesClass::SimplePropertyClass& Prop2 = Prop.propertiesArray[jdx];
      if (Prop2.MenuItem.BValue) {
        if ( Prop2.Key.contains("-platform-id") ) {
          if ( GlobalConfig.IgPlatform == 0 && Prop2.Value.size() == sizeof(GlobalConfig.IgPlatform) ) {
            CopyMem((UINT8*)&GlobalConfig.IgPlatform, Prop2.Value.data(), sizeof(GlobalConfig.IgPlatform));
          }
          devprop_add_value(device, Prop2.Key.c_str(), (UINT8*)&GlobalConfig.IgPlatform, 4);
          DBG("   Add key=%s valuelen=%zu\n", Prop2.Key.c_str(), Prop2.Value.size());
        } else if ( (Prop2.Key.contains("override-no-edid") || Prop2.Key.contains("override-no-connect"))
          && gSettings.Graphics.EDID.InjectEDID && gSettings.Graphics.EDID.CustomEDID.notEmpty()) {
          // special case for EDID properties
          devprop_add_value(device, Prop2.Key.c_str(), gSettings.Graphics.EDID.CustomEDID.data(), 128);
          DBG("   Add key=%s from custom EDID\n", Prop2.Key.c_str());
        } else {
          devprop_add_value(device, Prop2.Key, Prop2.Value);
          DBG("   Add key=%s valuelen=%zu\n", Prop2.Key.c_str(), Prop2.Value.size());
        }
      }else{
        DBG("Skip disabled properties with Path=%ls, Key=%s\n", Prop.DevicePathAsString.wc_str(), Prop2.Key.c_str());
      }
      StringDirty = TRUE;
    }
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

        //if (gSettings.Devices.AddPropertyArray.size() == 0xFFFE) {  //yyyy it means Arbitrary  // Looks like NrAddProperties == 0xFFFE is not used anymore
        //------------------
        device = NULL;
        /*       if (!string) {
         string = devprop_create_string();
         } */
        for ( size_t idx = 0 ; idx < gSettings.Devices.ArbitraryArray.size() ; ++idx ) {
          const SETTINGS_DATA::DevicesClass::ArbitraryPropertyClass& Prop = gSettings.Devices.ArbitraryArray[idx];
          if (Prop.Device != PCIdevice.dev.addr) {
            continue;
          }
          if (!PCIdevice.used) {
            device = devprop_add_device_pci(device_inject_string, &PCIdevice, NULL);
            PCIdevice.used = TRUE;
          }
          for ( size_t jdx = 0 ; jdx < Prop.CustomPropertyArray.size() ; ++jdx ) {
            const SETTINGS_DATA::DevicesClass::SimplePropertyClass& Prop2 = Prop.CustomPropertyArray[jdx];
          //special corrections
            if (Prop2.MenuItem.BValue) {
              if ( Prop2.Key.contains("-platform-id") ) {
                devprop_add_value(device, Prop2.Key.c_str(), (UINT8*)&GlobalConfig.IgPlatform, sizeof(GlobalConfig.IgPlatform));
              } else {
                devprop_add_value(device, Prop2.Key, Prop2.Value);
              }
            }
            StringDirty = TRUE;
          }
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
              if (gSettings.Graphics.InjectAsDict.InjectATI) {
                //can't do this in one step because of C-conventions
                TmpDirty    = setup_ati_devprop(Entry, &PCIdevice);
                StringDirty |=  TmpDirty;
              } else {
                MsgLog ("ATI injection not set\n");
              }

              for (j = 0; j < gConf.GfxPropertiesArrayNonConst.size(); j++) {
                if (gConf.GfxPropertiesArrayNonConst[j].Handle == PCIdevice.DeviceHandle) {
                  if (gConf.GfxPropertiesArrayNonConst[j].ConnChanged) {
                    *(UINT32*)(gConf.GfxPropertiesArrayNonConst[j].Mmio + RADEON_BIOS_0_SCRATCH) = gConf.GfxPropertiesArrayNonConst[j].Connectors;
                  }
                  break;
                }
              }

              if (gSettings.Graphics.RadeonDeInit) {
                for (j = 0; j < gConf.GfxPropertiesArrayNonConst.size(); j++) {
                  if (gConf.GfxPropertiesArrayNonConst[j].Handle == PCIdevice.DeviceHandle) {
                    *(UINT32*)(gConf.GfxPropertiesArrayNonConst[j].Mmio + 0x6848) = 0; //EVERGREEN_GRPH_FLIP_CONTROL, 1<<0 SURFACE_UPDATE_H_RETRACE_EN
                    *(UINT32*)(gConf.GfxPropertiesArrayNonConst[j].Mmio + 0x681C) = 0; //EVERGREEN_GRPH_PRIMARY_SURFACE_ADDRESS_HIGH
                    *(UINT32*)(gConf.GfxPropertiesArrayNonConst[j].Mmio + 0x6820) = 0; //EVERGREEN_GRPH_SECONDARY_SURFACE_ADDRESS_HIGH
                    *(UINT32*)(gConf.GfxPropertiesArrayNonConst[j].Mmio + 0x6808) = 0; //EVERGREEN_GRPH_LUT_10BIT_BYPASS_CONTROL, EVERGREEN_LUT_10BIT_BYPASS_EN  (1 << 8)
                    *(UINT32*)(gConf.GfxPropertiesArrayNonConst[j].Mmio + 0x6800) = 1; //EVERGREEN_GRPH_ENABLE
                    *(UINT32*)(gConf.GfxPropertiesArrayNonConst[j].Mmio + 0x6EF8) = 0; //EVERGREEN_MASTER_UPDATE_MODE
                    //*(UINT32*)(gConf.GfxPropertiesArrayNonConst[j].Mmio + R600_BIOS_0_SCRATCH) = 0x00810000;
                    DBG("Device %llu deinited\n", j);
                  }
                }
              }
              break;

            case 0x8086:
              if (gSettings.Graphics.InjectAsDict.InjectIntel) {
                TmpDirty    = setup_gma_devprop(Entry->macOSVersion, Entry->BuildVersion, Entry->Volume->RootDir, &PCIdevice);
                StringDirty |=  TmpDirty;
                MsgLog ("Intel GFX revision  = 0x%hhX\n", PCIdevice.revision);
              } else {
                MsgLog ("Intel GFX injection not set\n");
              }

              // IntelBacklight reworked by Sherlocks. 2018.10.07
              if (gSettings.Devices.IntelBacklight || gSettings.Devices.IntelMaxBacklight) {
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
                    if (GlobalConfig.IgPlatform) {
                      switch (GlobalConfig.IgPlatform) {
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
                    if (GlobalConfig.IgPlatform) {
                      switch (GlobalConfig.IgPlatform) {
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
                    if (GlobalConfig.IgPlatform) {
                      switch (GlobalConfig.IgPlatform) {
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
                    if (GlobalConfig.IgPlatform) {
                      switch (GlobalConfig.IgPlatform) {
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
                    if (gSettings.Devices.IntelBacklight) {
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
                    if (gSettings.Devices.IntelMaxBacklight) {
                      if (!LEVL) {
                        LEVL = FBLEVX;
                        MsgLog ("  Found invalid LEVL, set LEVL: 0x%X\n", LEVL);
                      }

                      if (!LEVX) {
                        ShiftLEVX = FBLEVX;
                        MsgLog ("  Found invalid LEVX, set LEVX: 0x%X\n", ShiftLEVX);
                      }

                      if (gSettings.Devices.IntelMaxValue) {
                        FBLEVX = gSettings.Devices.IntelMaxValue;
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
                    if (gSettings.Devices.IntelMaxBacklight) {
                      if (gSettings.Devices.IntelMaxValue) {
                        FBLEVX = gSettings.Devices.IntelMaxValue;
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
                    if (gSettings.Devices.IntelMaxBacklight) {
                      if (gSettings.Devices.IntelMaxValue) {
                        FBLEVX = gSettings.Devices.IntelMaxValue;
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

                if (gSettings.Devices.FakeID.FakeIntel == 0x00008086) {
                  UINT32 IntelDisable = 0x03;
                  PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, 0x50, 1, &IntelDisable);
                }
              }
              break;

            case 0x10de:
              if (gSettings.Graphics.InjectAsDict.InjectNVidia) {
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
          if (gSettings.Devices.USB.USBInjection) {
            TmpDirty = set_usb_props (&PCIdevice);
            StringDirty |=  TmpDirty;
          }
        }

        // HDA
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
                 ((Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA) ||
                  (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_AUDIO))) {
                   // HDMI injection inside
          if (gSettings.Devices.Audio.HDAInjection ) {
            TmpDirty    = setup_hda_devprop (PciIo, &PCIdevice, Entry->macOSVersion);
            StringDirty |= TmpDirty;
          }
          if (gSettings.Devices.Audio.ResetHDA) {
            
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
// 2021-04 Jief : LpcTune doesn't exist in Settings.cpp. Never set to true.
//          if (gSettings.LpcTune) {
//            UINT16 PmCon;
//            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, GEN_PMCON_1, 1, &PmCon);
//            MsgLog ("Initial PmCon value=%hX\n", PmCon);
//
//            if (GlobalConfig.EnableC6) {
//              PmCon |= 1 << 11;
//              DBG("C6 enabled\n");
//            } else {
//              PmCon &= ~(1 << 11);
//              DBG("C6 disabled\n");
//            }
//            /*
//             if (GlobalConfig.EnableC2) {
//             PmCon |= 1 << 10;
//             DBG("BIOS_PCIE enabled\n");
//             } else {
//             PmCon &= ~(1 << 10);
//             DBG("BIOS_PCIE disabled\n");
//             }
//             */
//            if (GlobalConfig.EnableC4) {
//              PmCon |= 1 << 7;
//              DBG("C4 enabled\n");
//            } else {
//              PmCon &= ~(1 << 7);
//              DBG("C4 disabled\n");
//            }
//
//            if (gSettings.ACPI.SSDT.EnableISS) {
//              PmCon |= 1 << 3;
//              DBG("SpeedStep enabled\n");
//            } else {
//              PmCon &= ~(1 << 3);
//              DBG("SpeedStep disabled\n");
//            }
//
//            PciIo->Pci.Write (PciIo, EfiPciIoWidthUint16, GEN_PMCON_1, 1, &PmCon);
//
//            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16,GEN_PMCON_1, 1, &PmCon);
//            MsgLog ("Set PmCon value=%hX\n", PmCon);
//
//          }
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
          if (gSettings.Devices.ForceHPET) {
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

          if (gSettings.Devices.DisableFunctions){
            UINT32 FD = REG32 ((UINTN)Rcba, 0x3418);
            DBG("Initial value of FD register 0x%X\n", FD);
            FD |= gSettings.Devices.DisableFunctions;
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
    
    XBuffer<char> newDeviceProperties = devprop_generate_string(device_inject_string);

    size_t binaryPropSize = hex2bin(newDeviceProperties, NULL, 0);
    if ( binaryPropSize > MAX_UINT32 ) {
      MsgLog("devprop_generate_string(device_inject_string) is too big");
      newDeviceProperties.setEmpty();
    }else{
      DBG("stringlength = %zu\n", newDeviceProperties.size());

      UINTN nbPages = EFI_SIZE_TO_PAGES(binaryPropSize);
      Status = gBS->AllocatePages (
                                   AllocateMaxAddress,
                                   EfiACPIReclaimMemory,
                                   nbPages,
                                   &BufferPtr
                                   );

      if (!EFI_ERROR(Status)) {
        mProperties       = (UINT8*)(UINTN)BufferPtr;
        //     DBG(gDeviceProperties);
        //     DBG("\n");
        //     StringDirty = FALSE;
        //-------
        mPropSize = (UINT32)hex2bin(newDeviceProperties, mProperties, EFI_PAGES_TO_SIZE(nbPages)); // cast should be safe as hex2bin return <= MAXUINT32
        gDeviceProperties = newDeviceProperties.forgetDataWithoutFreeing(); // do this AFTER hex2bin
        //     DBG("Final size of mProperties=%d\n", mPropSize);
        //---------
        //      Status = egSaveFile(&self.getSelfRootDir(),  SWPrintf("%ls\\misc\\devprop.bin", self.getCloverDirFullPath().wc_str()).wc_str()    , (UINT8*)mProperties, mPropSize);
      }else{
        MsgLog("AllocatePages failed (%s), device_inject_string not injected\n", efiStrError(Status));
      }
    }
  }

  MsgLog ("CurrentMode: Width=%lld Height=%lld\n", UGAWidth, UGAHeight);
}

EFI_STATUS
ApplySettings()
{
  // TODO: SetVariable()..
  // here we can apply user settings instead of default one
  gMobile                       = gSettings.Smbios.Mobile;

  if ((gSettings.CPU.BusSpeed != 0) && (gSettings.CPU.BusSpeed > 10 * Kilo) && (gSettings.CPU.BusSpeed < 500 * Kilo)) {
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
        gCPUStructure.ExternalClock = gSettings.CPU.BusSpeed;
        //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
        break;
      default:
        //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gSettings.BusSpeed, Kilo)));

        // for sandy bridge or newer
        // to match ExternalClock 25 MHz like real mac, divide BusSpeed by 4
        gCPUStructure.ExternalClock = (gSettings.CPU.BusSpeed + 3) / 4;
        //DBG("Corrected ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
        break;
    }

    gCPUStructure.FSBFrequency  = MultU64x64 (gSettings.CPU.BusSpeed, Kilo); //kHz -> Hz
    gCPUStructure.MaxSpeed      = (UINT32)(DivU64x32 ((UINT64)gSettings.CPU.BusSpeed * gCPUStructure.MaxRatio, 10000)); //kHz->MHz
  }

  if ((gSettings.CPU.CpuFreqMHz > 100) && (gSettings.CPU.CpuFreqMHz < 20000)) {
    gCPUStructure.MaxSpeed      = gSettings.CPU.CpuFreqMHz;
  }

  // to determine the use of Table 132
  if (gSettings.CPU.QPI) {
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
//
//EFI_STATUS
//InjectKextsFromDir (
//                    EFI_STATUS Status,
//                    CONST CHAR16 *SrcDir
//                    )
//{
//
//  if (EFI_ERROR(Status)) {
//    MsgLog (" - ERROR: Kext injection failed!\n");
//    return EFI_NOT_STARTED;
//  }
//
//  return Status;
//}
//
// Do we need that with OC ? For old version ?
//EFI_STATUS LOADER_ENTRY::SetFSInjection()
//{
//  EFI_STATUS           Status;
//  FSINJECTION_PROTOCOL *FSInject;
//  XStringW              SrcDir;
//  //BOOLEAN              InjectionNeeded = FALSE;
//  //BOOLEAN              BlockCaches     = FALSE;
//  FSI_STRING_LIST      *Blacklist      = 0;
//  FSI_STRING_LIST      *ForceLoadKexts = NULL;
//
//  MsgLog ("Beginning FSInjection\n");
//
//  // get FSINJECTION_PROTOCOL
//  Status = gBS->LocateProtocol(&gFSInjectProtocolGuid, NULL, (void **)&FSInject);
//  if (EFI_ERROR(Status)) {
//    //Print (L"- No FSINJECTION_PROTOCOL, Status = %s\n", efiStrError(Status));
//    MsgLog (" - ERROR: gFSInjectProtocolGuid not found!\n");
//    return EFI_NOT_STARTED;
//  }
//
//  // check if blocking of caches is needed
//  if (  OSFLAG_ISSET(Flags, OSFLAG_NOCACHES) || LoadOptions.contains("-f")  ) {
//    MsgLog ("Blocking kext caches\n");
//    //  BlockCaches = TRUE;
//    // add caches to blacklist
//    Blacklist = FSInject->CreateStringList();
//    if (Blacklist == NULL) {
//      MsgLog (" - ERROR: Not enough memory!\n");
//      return EFI_NOT_STARTED;
//    }
//
//    /*
//     From 10.7 to 10.9, status of directly restoring ESD files or update from Appstore cannot block kernel cache. because there are boot.efi and kernelcache file without kernel file.
//     After macOS installed, boot.efi can call kernel file from S/L/Kernels.
//     For this reason, long time ago, chameleon's user restored Base System.dmg to made USB installer and added kernel file in root and custom kexts in S/L/E. then used "-f" option.
//     From 10.10+, boot.efi call only prelinkedkernel file without kernel file. we can never block only kernelcache.
//     The use of these block caches is meaningless in modern macOS. Unlike the old days, we do not have to do the tedious task of putting the files needed for booting into the S/L/E.
//     Caution! Do not add this list. If add this list, will see "Kernel cache load error (0xe)". This is just a guideline.
//     by Sherlocks, 2017.11
//     */
//
//    // Installed/createinstallmedia
//    //FSInject->AddStringToList(Blacklist, L"\\System\\Library\\PrelinkedKernels\\prelinkedkernel"); // 10.10+/10.13.4+
//
//    // Recovery
//    //FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\kernelcache"); // 10.7 - 10.10
//    //FSInject->AddStringToList(Blacklist, L"\\com.apple.recovery.boot\\prelinkedkernel"); // 10.11+
//
//    // BaseSytem/InstallESD
//    //FSInject->AddStringToList(Blacklist, L"\\kernelcache"); // 10.7 - 10.9/(10.7/10.8)
//
//    // 1st stage - createinstallmedia
//    //FSInject->AddStringToList(Blacklist, L"\\.IABootFiles\\kernelcache"); // 10.9/10.10
//    //FSInject->AddStringToList(Blacklist, L"\\.IABootFiles\\prelinkedkernel"); // 10.11 - 10.13.3
//
//    // 2nd stage - InstallESD/AppStore/startosinstall
//    //FSInject->AddStringToList(Blacklist, L"\\Mac OS X Install Data\\kernelcache"); // 10.7
//    //FSInject->AddStringToList(Blacklist, L"\\OS X Install Data\\kernelcache"); // 10.8 - 10.10
//    //FSInject->AddStringToList(Blacklist, L"\\OS X Install Data\\prelinkedkernel"); // 10.11
//    //FSInject->AddStringToList(Blacklist, L"\\macOS Install Data\\prelinkedkernel"); // 10.12 - 10.12.3
//    //FSInject->AddStringToList(Blacklist, L"\\macOS Install Data\\Locked Files\\Boot Files\\prelinkedkernel");// 10.12.4+
//
//    // 2nd stage - Fusion Drive
//    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.R\\System\\Library\\PrelinkedKernels\\prelinkedkernel"); // 10.11
//    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.P\\System\\Library\\PrelinkedKernels\\prelinkedkernel"); // 10.11
//    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.S\\System\\Library\\PrelinkedKernels\\prelinkedkernel"); // 10.11
//    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.R\\prelinkedkernel"); // 10.12+
//    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.P\\prelinkedkernel"); // 10.12+
//    //FSInject->AddStringToList(Blacklist, L"\\com.apple.boot.S\\prelinkedkernel"); // 10.12+
//
//    // NetInstall
//    //FSInject->AddStringToList(Blacklist, L"\\NetInstall macOS High Sierra.nbi\\i386\\x86_64\\kernelcache");
//
//
//    // Block Caches list
//    // InstallDVD/Installed
//    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\Extensions.mkext"); // 10.6
//    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Extensions.mkext"); // 10.6
//    FSInject->AddStringToList(Blacklist, L"\\System\\Library\\Caches\\com.apple.kext.caches\\Startup\\kernelcache"); // 10.6/10.6 - 10.9
//
//    if (gSettings.BlockKexts.notEmpty()) {
//      FSInject->AddStringToList(Blacklist, SWPrintf("\\System\\Library\\Extensions\\%ls", gSettings.BlockKexts.wc_str()).wc_str());
//    }
//  }
//
//  // check if kext injection is needed
//  // (will be done only if caches are blocked or if boot.efi refuses to load kernelcache)
//  //SrcDir = NULL;
//  if (OSFLAG_ISSET(Flags, OSFLAG_WITHKEXTS)) {
//    SrcDir = GetOtherKextsDir(TRUE);
//    Status = FSInject->Install(
//                                Volume->DeviceHandle,
//                                L"\\System\\Library\\Extensions",
//                                SelfVolume->DeviceHandle,
//                                //GetOtherKextsDir (),
//                                SrcDir.wc_str(),
//                                Blacklist,
//                                ForceLoadKexts
//                                );
//    //InjectKextsFromDir(Status, GetOtherKextsDir());
//    InjectKextsFromDir(Status, SrcDir.wc_str());
//
//    SrcDir = GetOSVersionKextsDir(macOSVersion);
//    Status = FSInject->Install(
//                                Volume->DeviceHandle,
//                                L"\\System\\Library\\Extensions",
//                                SelfVolume->DeviceHandle,
//                                //GetOSVersionKextsDir(OSVersion),
//                                SrcDir.wc_str(),
//                                Blacklist,
//                                ForceLoadKexts
//                                );
//    //InjectKextsFromDir(Status, GetOSVersionKextsDir(OSVersion));
//    InjectKextsFromDir(Status, SrcDir.wc_str());
//  } else {
//    MsgLog("skipping kext injection (not requested)\n");
//  }
//
//  // prepare list of kext that will be forced to load
//  ForceLoadKexts = FSInject->CreateStringList();
//  if (ForceLoadKexts == NULL) {
//    MsgLog(" - Error: not enough memory!\n");
//    return EFI_NOT_STARTED;
//  }
//
//  KextPatcherRegisterKexts(FSInject, ForceLoadKexts);
//
//  // reinit Volume->RootDir? it seems it's not needed.
//
//  return Status;
//}


const XString8& SETTINGS_DATA::getUUID()
{
  if ( SystemParameters.CustomUuid.notEmpty() ) return SystemParameters.CustomUuid;
  return Smbios.SmUUID;
}

const XString8& SETTINGS_DATA::getUUID(EFI_GUIDClass *uuid)
{
  if ( SystemParameters.CustomUuid.notEmpty() ) {
    if ( uuid ) {
      EFI_STATUS Status = StrToGuidBE(SystemParameters.CustomUuid, uuid);
      if ( EFI_ERROR(Status) ) {
        log_technical_bug("CustomUuid(%s) is not valid", SystemParameters.CustomUuid.c_str()); // it's a technical bug. Validity is checked when imported from settings, so that must never happen.
        *uuid = EFI_GUIDClass();
        return nullGuidAsString;
      }
    }
    return SystemParameters.CustomUuid;
  }
  if ( uuid ) {
    EFI_STATUS Status = StrToGuidBE(Smbios.SmUUID, uuid);
    if ( EFI_ERROR(Status) ) {
      log_technical_bug("SmUUID(%s) is not valid", Smbios.SmUUID.c_str()); // same as before
      *uuid = EFI_GUIDClass();
      return nullGuidAsString;
    }
  }
  return Smbios.SmUUID;
}


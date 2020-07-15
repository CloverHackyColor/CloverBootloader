#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/OcStorageLib.h>
#include <Library/OcSerializeLib.h>
#include <Library/OcTemplateLib.h>
#include <Library/OcAfterBootCompatLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/OcConsoleLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/OcQuirksProtocol.h>

#define ROOT_PATH   L"EFI\\CLOVER"
//#define CONFIG_PATH L"drivers\\UEFI\\OcQuirks.plist"

#define MAX_DATA_SIZE 10000
/*
STATIC
OC_SCHEMA
mMmioWhitelistEntry[] = {
  OC_SCHEMA_INTEGER_IN  ("Address", OC_MMIO_WL_STRUCT, Address),
  OC_SCHEMA_STRING_IN   ("Comment", OC_MMIO_WL_STRUCT, Comment),
  OC_SCHEMA_BOOLEAN_IN  ("Enabled", OC_MMIO_WL_STRUCT, Enabled),
};

STATIC
OC_SCHEMA
mMmioWhitelist = OC_SCHEMA_DICT (NULL, mMmioWhitelistEntry);

STATIC
OC_SCHEMA
mConfigNodes[] = {
  OC_SCHEMA_BOOLEAN_IN ("AvoidRuntimeDefrag"      , OC_QUIRKS, AvoidRuntimeDefrag),
  OC_SCHEMA_BOOLEAN_IN ("DevirtualiseMmio"        , OC_QUIRKS, DevirtualiseMmio),
  OC_SCHEMA_BOOLEAN_IN ("DisableSingleUser"       , OC_QUIRKS, DisableSingleUser),
  OC_SCHEMA_BOOLEAN_IN ("DisableVariableWrite"    , OC_QUIRKS, DisableVariableWrite),
  OC_SCHEMA_BOOLEAN_IN ("DiscardHibernateMap"     , OC_QUIRKS, DiscardHibernateMap),
  OC_SCHEMA_BOOLEAN_IN ("EnableSafeModeSlide"     , OC_QUIRKS, EnableSafeModeSlide),
  OC_SCHEMA_BOOLEAN_IN ("EnableWriteUnprotector"  , OC_QUIRKS, EnableWriteUnprotector),
  OC_SCHEMA_BOOLEAN_IN ("ForceExitBootServices"   , OC_QUIRKS, ForceExitBootServices),
  OC_SCHEMA_ARRAY_IN   ("MmioWhitelist"           , OC_QUIRKS, MmioWhitelist, &mMmioWhitelist),
  OC_SCHEMA_BOOLEAN_IN ("ProtectMemoryRegions"    , OC_QUIRKS, ProtectMemoryRegions),
  OC_SCHEMA_BOOLEAN_IN ("ProtectSecureBoot"       , OC_QUIRKS, ProtectSecureBoot),
  OC_SCHEMA_BOOLEAN_IN ("ProtectUefiServices"     , OC_QUIRKS, ProtectUefiServices),
  OC_SCHEMA_BOOLEAN_IN ("ProvideConsoleGopEnable" , OC_QUIRKS, ProvideConsoleGopEnable),
  OC_SCHEMA_BOOLEAN_IN ("ProvideCustomSlide"      , OC_QUIRKS, ProvideCustomSlide),
  OC_SCHEMA_INTEGER_IN ("ProvideMaxSlide"         , OC_QUIRKS, ProvideMaxSlide),
  OC_SCHEMA_BOOLEAN_IN ("RebuildAppleMemoryMap"   , OC_QUIRKS, RebuildAppleMemoryMap),
  OC_SCHEMA_BOOLEAN_IN ("SetupVirtualMap"         , OC_QUIRKS, SetupVirtualMap),
  OC_SCHEMA_BOOLEAN_IN ("SignalAppleOS"           , OC_QUIRKS, SignalAppleOS),
  OC_SCHEMA_BOOLEAN_IN ("SyncRuntimePermissions"  , OC_QUIRKS, SyncRuntimePermissions)
};

STATIC
OC_SCHEMA_INFO
mConfigInfo = {
  .Dict = {mConfigNodes, ARRAY_SIZE (mConfigNodes)}
};

STATIC
BOOLEAN
QuirksProvideConfig (
  OUT OC_QUIRKS *Config,
  IN EFI_HANDLE Handle
  )
{
  EFI_STATUS                        Status;
  EFI_LOADED_IMAGE_PROTOCOL         *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FileSystem;
  OC_STORAGE_CONTEXT                Storage;
  CHAR8                             *ConfigData;
  UINT32                            ConfigDataSize;
  
  // Load SimpleFileSystem Protocol
  Status = gBS->HandleProtocol (
    Handle,
    &gEfiLoadedImageProtocolGuid,
    (VOID **) &LoadedImage
    );
  
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  FileSystem = LocateFileSystem (
    LoadedImage->DeviceHandle,
    LoadedImage->FilePath
    );
  
  if (FileSystem == NULL) {
    return FALSE;
  }
  
  // Init OcStorage as it already handles
  // reading Unicode files
  Status = OcStorageInitFromFs (
    &Storage,
    FileSystem,
    ROOT_PATH,
    NULL
    );
  
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  ConfigData = OcStorageReadFileUnicode (
    &Storage,
    CONFIG_PATH,
    &ConfigDataSize
    );
  
  // If no config data or greater than max size, fail and use defaults
  if (ConfigDataSize == 0 || ConfigDataSize > MAX_DATA_SIZE) {
    if (ConfigData != NULL) {
      FreePool(ConfigData);
    }

    return FALSE;
  }
  //ConfigData is still XML file content. Now parsing
  BOOLEAN Success = ParseSerialized (Config, &mConfigInfo, ConfigData, ConfigDataSize);
  
  FreePool(ConfigData);
  
  return Success;
}
*/
OCQUIRKS_PROTOCOL *mQuirks = NULL;

EFI_STATUS
EFIAPI
QuirksEntryPoint (
  IN EFI_HANDLE        Handle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS              Status;
  OC_ABC_SETTINGS         AbcSettings = { TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE,
                                          FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, NULL, 0, NULL, NULL, NULL, NULL };
  BOOLEAN                 ProvideConsoleGopEnable = TRUE;
  
  Status = gBS->LocateProtocol(&gOcQuirksProtocolGuid, NULL, (VOID **)&mQuirks);
  
// if not found then use default values
//  if (EFI_ERROR(Status)) {
//    return Status;
//  }
//  OC_QUIRKS Config;
  
//  OC_QUIRKS_CONSTRUCT (&Config, sizeof (Config));
//  QuirksProvideConfig(&Config, Handle);
  
  if (mQuirks) {
    Status = mQuirks->GetConfig(mQuirks, &AbcSettings, &ProvideConsoleGopEnable);
  }
  if (EFI_ERROR(Status)) {
//    OC_QUIRKS_DESTRUCT (&Config, sizeof(Config));
//    return Status;
    DEBUG ((DEBUG_INFO, L"config not found, use default"));
  }
/*
  OC_ABC_SETTINGS AbcSettings = {
  
    .AvoidRuntimeDefrag	    = Config.AvoidRuntimeDefrag,
    .DevirtualiseMmio       = Config.DevirtualiseMmio,
    .DisableSingleUser      = Config.DisableSingleUser,
    .DisableVariableWrite   = Config.DisableVariableWrite,
    .DiscardHibernateMap    = Config.DiscardHibernateMap,
    .EnableSafeModeSlide    = Config.EnableSafeModeSlide,
    .EnableWriteUnprotector = Config.EnableWriteUnprotector,
    .ForceExitBootServices  = Config.ForceExitBootServices,
    .ProtectMemoryRegions   = Config.ProtectMemoryRegions,
    .ProtectSecureBoot      = Config.ProtectSecureBoot,
    .ProtectUefiServices    = Config.ProtectUefiServices,
    .ProvideCustomSlide     = Config.ProvideCustomSlide,
    .ProvideMaxSlide        = Config.ProvideMaxSlide,
    .RebuildAppleMemoryMap  = Config.RebuildAppleMemoryMap,
    .SetupVirtualMap        = Config.SetupVirtualMap,
    .SignalAppleOS          = Config.SignalAppleOS,
    .SyncRuntimePermissions = Config.SyncRuntimePermissions
  };
  
  if (Config.DevirtualiseMmio && Config.MmioWhitelist.Count > 0) {
    AbcSettings.MmioWhitelist = AllocatePool (
      Config.MmioWhitelist.Count * sizeof (AbcSettings.MmioWhitelist[0])
      );
    
    if (AbcSettings.MmioWhitelist != NULL) {
      UINT32 abcIndex = 0;
      UINT32 configIndex = 0;
      
      for (configIndex = 0; configIndex < Config.MmioWhitelist.Count; configIndex++) {
        if (Config.MmioWhitelist.Values[configIndex]->Enabled) {
          AbcSettings.MmioWhitelist[abcIndex] = Config.MmioWhitelist.Values[configIndex]->Address;
          abcIndex++;
        }
      }
      
      AbcSettings.MmioWhitelistSize = abcIndex;
    } // Else couldn't allocate slots for mmio addresses
  }
*/
  if (ProvideConsoleGopEnable) {
  	OcProvideConsoleGop(TRUE);
  }
  
//  OC_QUIRKS_DESTRUCT (&Config, sizeof (Config));
  
  return OcAbcInitialize(&AbcSettings);
}

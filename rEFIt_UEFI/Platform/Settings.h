#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define CLOVER_SIGN             SIGNATURE_32('C','l','v','r')
#define HEIGHT_2K 1100

#include "../gui/menu_items/menu_items.h"
#include "../Platform/plist.h"

//// SysVariables
//typedef struct SYSVARIABLES SYSVARIABLES;
//struct SYSVARIABLES
//{
//  SYSVARIABLES      *Next;
//  CHAR16            *Key;
//  INPUT_ITEM        MenuItem;
//};

typedef struct {
  CHAR16          *Name;
//  CHAR8           *LineName;
  INTN            Index;
  EFI_HANDLE      Handle;
  EFI_AUDIO_IO_PROTOCOL_DEVICE Device;
} HDA_OUTPUTS;

typedef enum {
  Unknown,
  Ati,      /* 0x1002 */
  Intel,    /* 0x8086 */
  Nvidia,   /* 0x10de */
  RDC,  /* 0x17f3 */
  VIA,  /* 0x1106 */
  SiS,  /* 0x1039 */
  ULI  /* 0x10b9 */
} HRDW_MANUFACTERER;

typedef struct {
  HRDW_MANUFACTERER  Vendor;
  UINT8             Ports;
  UINT16            DeviceID;
  UINT16            Family;
//UINT16            Width;
//UINT16            Height;
  CHAR8             Model[64];
  CHAR8             Config[64];
  BOOLEAN           LoadVBios;
//BOOLEAN           PatchVBios;
  UINTN             Segment;
  UINTN             Bus;
  UINTN             Device;
  UINTN             Function;
  EFI_HANDLE        Handle;
  UINT8             *Mmio;
  UINT32            Connectors;
  BOOLEAN           ConnChanged;
} GFX_PROPERTIES;

typedef struct {
    HRDW_MANUFACTERER  Vendor;
    UINT16            controller_vendor_id;
    UINT16            controller_device_id;
    CHAR16            *controller_name;
// -- Codec Info -- //
    UINT16            codec_vendor_id;
    UINT16            codec_device_id;
    UINT8             codec_revision_id;
    UINT8             codec_stepping_id;
    UINT8             codec_maj_rev;
    UINT8             codec_min_rev;
    UINT8             codec_num_function_groups;
    CHAR16            *codec_name;
} HDA_PROPERTIES;

typedef struct ACPI_NAME_LIST ACPI_NAME_LIST;
struct ACPI_NAME_LIST {
	ACPI_NAME_LIST *Next;
	CHAR8          *Name;
};

typedef struct ACPI_DROP_TABLE ACPI_DROP_TABLE;
struct ACPI_DROP_TABLE
{
  ACPI_DROP_TABLE *Next;
  UINT32          Signature;
  UINT32          Length;
  UINT64          TableId;
  INPUT_ITEM      MenuItem;
  BOOLEAN         OtherOS;
};

typedef struct CUSTOM_LOADER_ENTRY CUSTOM_LOADER_ENTRY;
struct CUSTOM_LOADER_ENTRY {
  CUSTOM_LOADER_ENTRY     *Next;
  CUSTOM_LOADER_ENTRY     *SubEntries;
  XImage                  Image;
  XImage                  DriveImage;
  CONST CHAR16            *ImagePath;
  CONST CHAR16            *DriveImagePath;
  CONST CHAR16            *Volume;
  XStringW                Path;
  XStringArray            LoadOptions;

  XStringW FullTitle;
  XStringW Title;
  CONST CHAR16            *Settings;
  CHAR16                  Hotkey;
  BOOLEAN                 CommonSettings;
  UINT8                   Flags;
  UINT8                   Type;
  UINT8                   VolumeType;
  UINT8                   KernelScan;
  UINT8                   CustomBoot;
  XImage                  CustomLogo;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL BootBgColor;
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches;

  CUSTOM_LOADER_ENTRY() : Next(0), SubEntries(0), ImagePath(0), DriveImagePath(0), Volume(0), Settings(0), Hotkey(0), CommonSettings(0), Flags(0), Type(0), VolumeType(0),
                          KernelScan(0), CustomBoot(0), BootBgColor({0,0,0,0})
						{ memset(&KernelAndKextPatches, 0, sizeof(KernelAndKextPatches)); }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  CUSTOM_LOADER_ENTRY(const CUSTOM_LOADER_ENTRY&) = delete;
  CUSTOM_LOADER_ENTRY& operator=(const CUSTOM_LOADER_ENTRY&) = delete;

};

typedef struct CUSTOM_LEGACY_ENTRY CUSTOM_LEGACY_ENTRY;
struct CUSTOM_LEGACY_ENTRY {
  CUSTOM_LEGACY_ENTRY   *Next;
  XImage                Image;
  XImage                DriveImage;
  CONST CHAR16          *ImagePath;
  CONST CHAR16          *DriveImagePath;
  CONST CHAR16          *Volume;
  XStringW              FullTitle;
  XStringW              Title;
  CHAR16              Hotkey;
  UINT8               Flags;
  UINT8               Type;
  UINT8               VolumeType;
};

typedef struct CUSTOM_TOOL_ENTRY CUSTOM_TOOL_ENTRY;
struct CUSTOM_TOOL_ENTRY {
  CUSTOM_TOOL_ENTRY *Next;
  XImage            Image;
  CHAR16            *ImagePath;
  CHAR16            *Volume;
  XStringW          Path;
  XStringArray      LoadOptions;
  XStringW          FullTitle;
  XStringW          Title;
  CHAR16            Hotkey;
  UINT8             Flags;
  UINT8             VolumeType;
};

typedef enum {
  kTagTypeNone,
  kTagTypeDict,
  kTagTypeKey,
  kTagTypeString,
  kTagTypeInteger,
  kTagTypeData,
  kTagTypeDate,
  kTagTypeFalse,
  kTagTypeTrue,
  kTagTypeArray,
  kTagTypeFloat
} TAG_TYPE;

typedef struct DEV_PROPERTY DEV_PROPERTY; //yyyy
struct DEV_PROPERTY {
  UINT32        Device;
  EFI_DEVICE_PATH_PROTOCOL* DevicePath;
  CHAR8         *Key;
  UINT8         *Value;
  UINTN         ValueLen;
  DEV_PROPERTY  *Next;   //next device or next property
  DEV_PROPERTY  *Child;  // property list of the device
  CHAR8         *Label;
  INPUT_ITEM    MenuItem;
  TAG_TYPE      ValueType;
};

typedef struct {

  // SMBIOS TYPE0
  CHAR8                   VendorName[64];
  CHAR8                   RomVersion[64];
  CHAR8                   EfiVersion[64];
  CHAR8                   ReleaseDate[64];
  // SMBIOS TYPE1
  CHAR8                   ManufactureName[64];
  CHAR8                   ProductName[64];
  CHAR8                   VersionNr[64];
  CHAR8                   SerialNr[64];
  EFI_GUID                SmUUID;
  BOOLEAN                 SmUUIDConfig;
  CHAR8                   pad0[7];
//CHAR8                    Uuid[64];
//CHAR8                    SKUNumber[64];
  CHAR8                   FamilyName[64];
  CHAR8                   OEMProduct[64];
  CHAR8                   OEMVendor[64];
  // SMBIOS TYPE2
  CHAR8                   BoardManufactureName[64];
  CHAR8                   BoardSerialNumber[64];
  CHAR8                   BoardNumber[64]; //Board-ID
  CHAR8                   LocationInChassis[64];
  CHAR8                   BoardVersion[64];
  CHAR8                   OEMBoard[64];
  UINT8                   BoardType;
  UINT8                   Pad1;
  // SMBIOS TYPE3
  BOOLEAN                 Mobile;
  UINT8                   ChassisType;
  CHAR8                   ChassisManufacturer[64];
  CHAR8                   ChassisAssetTag[64];
  // SMBIOS TYPE4
  UINT32                  CpuFreqMHz;
  UINT32                  BusSpeed; //in kHz
  BOOLEAN                 Turbo;
  UINT8                   EnabledCores;
  BOOLEAN                 UserChange;
  BOOLEAN                 QEMU;
  // SMBIOS TYPE17
  UINT16                  SmbiosVersion;
  INT8                    Attribute;
  INT8                    Pad17[1];
  CHAR8                   MemoryManufacturer[64];
  CHAR8                   MemorySerialNumber[64];
  CHAR8                   MemoryPartNumber[64];
  CHAR8                   MemorySpeed[64];
  // SMBIOS TYPE131
  UINT16                  CpuType;
  // SMBIOS TYPE132
  UINT16                  QPI;
  BOOLEAN                 SetTable132;
  BOOLEAN                 TrustSMBIOS;
  BOOLEAN                 InjectMemoryTables;
  INT8                    XMPDetection;
  BOOLEAN                 UseARTFreq;
  // SMBIOS TYPE133
  UINT64                  PlatformFeature;
	
  // PatchTableType11
  BOOLEAN                 NoRomInfo;

  // OS parameters
  CHAR8                   Language[16];
  CHAR8                   BootArgs[256];
  CHAR16                  CustomUuid[40];

  CHAR16                  *DefaultVolume;
  CHAR16                  *DefaultLoader;
//Boot
  BOOLEAN                 LastBootedVolume;
  BOOLEAN                 SkipHibernateTimeout;
//Monitor
  BOOLEAN                 IntelMaxBacklight;
//  UINT8                   Pad21[1];
  UINT16                  VendorEDID;
  UINT16                  ProductEDID;
  UINT16                  BacklightLevel;
  BOOLEAN                 BacklightLevelConfig;
  BOOLEAN                 IntelBacklight;
//Boot options
  BOOLEAN                 MemoryFix;
  BOOLEAN                 WithKexts;
  BOOLEAN                 WithKextsIfNoFakeSMC;
  BOOLEAN                 FakeSMCFound;
  BOOLEAN                 NoCaches;

  // GUI parameters
  BOOLEAN                 Debug;
//  BOOLEAN                 Proportional; //never used
  UINT8                   Pad22[1];
  UINT32                  DefaultBackgroundColor;

  //ACPI
  UINT64                  ResetAddr;
  UINT8                   ResetVal;
  BOOLEAN                 NoASPM;
  BOOLEAN                 DropSSDT;
  BOOLEAN                 NoOemTableId;
  BOOLEAN                 NoDynamicExtract;
  BOOLEAN                 AutoMerge;
  BOOLEAN                 GeneratePStates;
  BOOLEAN                 GenerateCStates;
  BOOLEAN                 GenerateAPSN;
  BOOLEAN                 GenerateAPLF;
  BOOLEAN                 GeneratePluginType;
  UINT8                   PLimitDict;
  UINT8                   UnderVoltStep;
  BOOLEAN                 DoubleFirstState;
  BOOLEAN                 SuspendOverride;
  BOOLEAN                 EnableC2;
  BOOLEAN                 EnableC4;
  BOOLEAN                 EnableC6;
  BOOLEAN                 EnableISS;
  BOOLEAN                 SlpSmiEnable;
  BOOLEAN                 FixHeaders;
  UINT16                  C3Latency;
  BOOLEAN                 smartUPS;
  BOOLEAN                 PatchNMI;
  BOOLEAN                 EnableC7;
  UINT8                   SavingMode;

  CHAR16                  DsdtName[28];
  UINT32                  FixDsdt;
  UINT8                   MinMultiplier;
  UINT8                   MaxMultiplier;
  UINT8                   PluginType;
//  BOOLEAN                 DropMCFG;
  BOOLEAN                 FixMCFG;

  UINT32                  DeviceRenameCount;
  ACPI_NAME_LIST          *DeviceRename;
  //Injections
  BOOLEAN                 StringInjector;
  BOOLEAN                 InjectSystemID;
  BOOLEAN                 NoDefaultProperties;

  BOOLEAN                 ReuseFFFF;

  //PCI devices
  UINT32                  FakeATI;    //97
  UINT32                  FakeNVidia;
  UINT32                  FakeIntel;
  UINT32                  FakeLAN;   //100
  UINT32                  FakeWIFI;
  UINT32                  FakeSATA;
  UINT32                  FakeXHCI;  //103
  UINT32                  FakeIMEI;  //106

  //Graphics
//  UINT16                  PCIRootUID;
  BOOLEAN                 GraphicsInjector;
  BOOLEAN                 InjectIntel;
  BOOLEAN                 InjectATI;
  BOOLEAN                 InjectNVidia;
  BOOLEAN                 DeInit;
  BOOLEAN                 LoadVBios;
  BOOLEAN                 PatchVBios;
  VBIOS_PATCH_BYTES       *PatchVBiosBytes;
  UINTN                   PatchVBiosBytesCount;
  BOOLEAN                 InjectEDID;
  BOOLEAN                 LpcTune;
  UINT16                  DropOEM_DSM;
  UINT8                   *CustomEDID;
  UINT16                  CustomEDIDsize;
  UINT16                  EdidFixHorizontalSyncPulseWidth;
  UINT8                   EdidFixVideoInputSignal;

  CHAR16                  FBName[16];
  UINT16                  VideoPorts;
  BOOLEAN                 NvidiaGeneric;
  BOOLEAN                 NvidiaNoEFI;
  BOOLEAN                 NvidiaSingle;
  UINT64                  VRAM;
  UINT8                   Dcfg[8];
  UINT8                   NVCAP[20];
  INT8                    BootDisplay;
  BOOLEAN                 NvidiaWeb;
  UINT8                   pad41[2];
  UINT32                  DualLink;
  UINT32                  IgPlatform;

  // Secure boot white/black list
  UINT32                  SecureBootWhiteListCount;
  UINT32                  SecureBootBlackListCount;
  CHAR16                  **SecureBootWhiteList;

  CHAR16                  **SecureBootBlackList;

  // Secure boot
  UINT8                   SecureBoot;
  UINT8                   SecureBootSetupMode;
  UINT8                   SecureBootPolicy;

  // HDA
  BOOLEAN                 HDAInjection;
  INT32                   HDALayoutId;

  // USB DeviceTree injection
  BOOLEAN                 USBInjection;
  BOOLEAN                 USBFixOwnership;
  BOOLEAN                 InjectClockID;
  BOOLEAN                 HighCurrent;
  BOOLEAN                 NameEH00;
  BOOLEAN                 NameXH00;
	
  BOOLEAN                 LANInjection;
  BOOLEAN                 HDMIInjection;

 // UINT8                   pad61[2];

  // LegacyBoot
  CHAR16                  LegacyBoot[32];
  UINT16                  LegacyBiosDefaultEntry;

  //SkyLake
  BOOLEAN                 HWP;
  UINT8                   TDP;
  UINT32                  HWPValue;

  //Volumes hiding
  CHAR16                  **HVHideStrings;

  INTN                    HVCount;

  // KernelAndKextPatches
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches;  //zzzz
  BOOLEAN                 KextPatchesAllowed;
  BOOLEAN                 KernelPatchesAllowed; //From GUI: Only for user patches, not internal Clover

  CHAR8                   AirportBridgeDeviceName[5];

  // Pre-language
  BOOLEAN                 KbdPrevLang;

  //Pointer
  BOOLEAN                 PointerEnabled;
  INTN                    PointerSpeed;
  UINT64                  DoubleClickTime;
  BOOLEAN                 PointerMirror;

//  UINT8                   pad7[6];
  UINT8                   CustomBoot;
  XImage                  *CustomLogo;

  UINT32                  RefCLK;

  // SysVariables
  CHAR8                   *RtMLB;
  UINT8                   *RtROM;
  UINTN                   RtROMLen;

  UINT32                  CsrActiveConfig;
  UINT16                  BooterConfig;
  CHAR8                   BooterCfgStr[64];
  BOOLEAN                 DisableCloverHotkeys;
  BOOLEAN                 NeverDoRecovery;

  // Multi-config
  CHAR16                  ConfigName[30];
  CHAR16                  *MainConfigName;

  //Drivers
  INTN                    BlackListCount;
  CHAR16                  **BlackList;

  //SMC keys
  CHAR8                   RPlt[8];
  CHAR8                   RBr[8];
  UINT8                   EPCI[4];
  UINT8                   REV[6];

  //other devices
  BOOLEAN                 Rtc8Allowed;
  BOOLEAN                 ForceHPET;
  BOOLEAN                 ResetHDA;
  BOOLEAN                 PlayAsync;
  UINT32                  DisableFunctions;

  //Patch DSDT arbitrary
  UINT32                  PatchDsdtNum;
  UINT8                   **PatchDsdtFind;
  UINT32 *LenToFind;
  UINT8  **PatchDsdtReplace;

  UINT32 *LenToReplace;
  BOOLEAN                 DebugDSDT;
  BOOLEAN                 SlpWak;
  BOOLEAN                 UseIntelHDMI;
  UINT8                   AFGLowPowerState;
  UINT8                   PNLF_UID;
//  UINT8                   pad83[4];


  // Table dropping
  ACPI_DROP_TABLE         *ACPIDropTables;

  // Custom entries
  BOOLEAN                 DisableEntryScan;
  BOOLEAN                 DisableToolScan;
  BOOLEAN                 ShowHiddenEntries;
  UINT8                   KernelScan;
  BOOLEAN                 LinuxScan;
//  UINT8                   pad84[3];
  CUSTOM_LOADER_ENTRY     *CustomEntries;
  CUSTOM_LEGACY_ENTRY     *CustomLegacy;
  CUSTOM_TOOL_ENTRY       *CustomTool;

  //Add custom properties
  UINTN                   NrAddProperties;
  DEV_PROPERTY            *AddProperties;

  //BlackListed kexts
  CHAR16                  BlockKexts[64];

  // Disable inject kexts
//  UINT32                  DisableInjectKextCount;
//  CHAR16                  **DisabledInjectKext;
//  INPUT_ITEM              *InjectKextMenuItem;

  //ACPI tables
  UINTN                   SortedACPICount;
  CHAR16                  **SortedACPI;

  // ACPI/PATCHED/AML
  UINT32                  DisabledAMLCount;
  CHAR16                  **DisabledAML;
  CHAR8                   **PatchDsdtLabel; //yyyy
  CHAR8                   **PatchDsdtTgt;
  INPUT_ITEM              *PatchDsdtMenuItem;

  //other
  UINT32                  IntelMaxValue;
//  UINT32                  AudioVolume;

  // boot.efi
  UINT32 OptionsBits;
  UINT32 FlagsBits;
  UINT32 UIScale;
  UINT32 EFILoginHiDPI;
  UINT8  flagstate[32];

  DEV_PROPERTY            *ArbProperties;

} SETTINGS_DATA;

typedef enum {
  english = 0,  //en
  russian,    //ru
  french,     //fr
  german,     //de
  dutch,      //nl
  italian,    //it
  spanish,    //es
  portuguese, //pt
  brasil,     //br
  polish,     //pl
  ukrainian,  //ua
  croatian,   //hr
  czech,      //cs
  indonesian, //id
  korean,     //ko
  chinese,    //cn
  romanian    //ro
  //something else? add, please
} LANGUAGES;

typedef struct _DRIVERS_FLAGS {
  BOOLEAN EmuVariableLoaded;
  BOOLEAN VideoLoaded;
  BOOLEAN PartitionLoaded;
  BOOLEAN MemFixLoaded;
  BOOLEAN AptioFixLoaded;
  BOOLEAN AptioFix2Loaded;
  BOOLEAN AptioFix3Loaded;
  BOOLEAN AptioMemFixLoaded;
  BOOLEAN HFSLoaded;
  BOOLEAN APFSLoaded;
} DRIVERS_FLAGS;

typedef struct {
  UINT16            SegmentGroupNum;
  UINT8             BusNum;
  UINT8             DevFuncNum;
  BOOLEAN           Valid;
//UINT8             DeviceN;
  UINT8             SlotID;
  UINT8             SlotType;
  CHAR8             SlotName[31];
} SLOT_DEVICE;

// ACPI/PATCHED/AML
typedef struct ACPI_PATCHED_AML ACPI_PATCHED_AML;
struct ACPI_PATCHED_AML
{
  ACPI_PATCHED_AML  *Next;
  CHAR16            *FileName;
  INPUT_ITEM        MenuItem;
};

// syscl - Side load kext
typedef struct SIDELOAD_KEXT SIDELOAD_KEXT;
struct SIDELOAD_KEXT {
  SIDELOAD_KEXT  *Next;
  SIDELOAD_KEXT  *PlugInList;
  CHAR16         *FileName;
  CHAR16         *KextDirNameUnderOEMPath;
  CHAR16         *Version;
  INPUT_ITEM     MenuItem;
};

typedef struct RT_VARIABLES RT_VARIABLES;
struct RT_VARIABLES {
//  BOOLEAN  Disabled;
  CHAR16   *Name;
  EFI_GUID VarGuid;
};
extern RT_VARIABLES                 *RtVariables;

extern UINTN       AudioNum;
extern HDA_OUTPUTS AudioList[20];

extern CONST CHAR16* ThemesList[100]; //no more then 100 themes?
extern CHAR16*       ConfigsList[20];
extern CHAR16*       DsdtsList[20];
extern UINTN DsdtsNum;
extern UINTN ThemesNum;
extern UINTN ConfigsNum;
extern INTN                     ScrollButtonsHeight;
extern INTN    ScrollBarDecorationsHeight;
extern INTN    ScrollScrollDecorationsHeight;
extern INTN LayoutBannerOffset;
extern INTN LayoutButtonOffset;
extern INTN LayoutTextOffset;
// this should go in a globals, not in settings

extern INTN                            OldChosenTheme;
extern INTN                            OldChosenConfig;
extern INTN                            OldChosenDsdt;
extern UINTN                            OldChosenAudio;
extern BOOLEAN                        SavePreBootLog;
extern UINT8                            DefaultAudioVolume;


extern GFX_PROPERTIES                 gGraphics[];
extern HDA_PROPERTIES                 gAudios[];
extern UINTN                          NGFX;
extern UINTN                          NHDA;
extern CONST CHAR16						 **SystemPlists;
extern CONST CHAR16                        **InstallPlists;
extern CONST CHAR16						 **RecoveryPlists;
//extern UINT16                         gCPUtype;
extern SETTINGS_DATA                  gSettings;
extern LANGUAGES                      gLanguage;
extern BOOLEAN                        gFirmwareClover;
extern DRIVERS_FLAGS                  gDriversFlags;
extern SLOT_DEVICE                    SlotDevices[];
extern EFI_EDID_DISCOVERED_PROTOCOL   *EdidDiscovered;
//extern UINT8                          *gEDID;

extern UINTN                           gEvent;

extern UINT16                          gBacklightLevel;

extern BOOLEAN                         defDSM;
extern UINT16                          dropDSM;

extern TagPtr                          gConfigDict[];

// ACPI/PATCHED/AML
extern ACPI_PATCHED_AML                *ACPIPatchedAML;

// Sideload/inject kext
extern SIDELOAD_KEXT                   *InjectKextList;

// SysVariables
//extern SYSVARIABLES                   *SysVariables;

// Hold theme fixed IconFormat / extension
extern CHAR16                         *IconFormat;

extern CONST CHAR16                   *gFirmwareRevision;
extern CONST CHAR8* gRevisionStr;
extern CONST CHAR8* gFirmwareBuildDate;
extern CONST CHAR8* gBuildInfo;

extern BOOLEAN                        ResumeFromCoreStorage;
extern BOOLEAN                        gRemapSmBiosIsRequire;  // syscl: pass argument for Dell SMBIOS here

extern EFI_GUID                       gUuid;

extern EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl;


//
// config module
//

typedef struct {
  INTN        Timeout;
  UINTN       DisableFlags; //to disable some volume types (optical, firewire etc)
  BOOLEAN     TextOnly;
  BOOLEAN     Quiet;
  BOOLEAN     LegacyFirst;
  BOOLEAN     NoLegacy;
  BOOLEAN     DebugLog;
  BOOLEAN     FastBoot;
  BOOLEAN     NeverHibernate;
  BOOLEAN     StrictHibernate;
  BOOLEAN     RtcHibernateAware;
  BOOLEAN     HibernationFixup;
  BOOLEAN     SignatureFixup;
  CHAR16      *Theme;
  CHAR16      *ScreenResolution;
  INTN        ConsoleMode;
  BOOLEAN     CustomIcons;
  INTN        IconFormat;
  BOOLEAN     NoEarlyProgress;
  INT32       Timezone;
  BOOLEAN     ShowOptimus;
  INTN        Codepage;
  INTN        CodepageSize;
} REFIT_CONFIG;


extern REFIT_CONFIG GlobalConfig;


EFI_STATUS
SetFSInjection (
  IN LOADER_ENTRY *Entry
  );

VOID
SetDevices (
  LOADER_ENTRY *Entry
  );
//
// check if this entry corresponds to Boot# variable and then set BootCurrent
//
VOID
SetBootCurrent(REFIT_MENU_ITEM_BOOTNUM *LoadedEntry);


CHAR8
*GetOSVersion (
  IN  LOADER_ENTRY *Entry
  );


VOID GetListOfThemes(VOID);
VOID GetListOfConfigs(VOID);
VOID GetListOfACPI(VOID);
VOID GetListOfDsdts(VOID);

// syscl - get list of inject kext(s)
VOID GetListOfInjectKext(CHAR16 *);

UINT32
GetCrc32 (
  UINT8 *Buffer,
  UINTN Size
  );

VOID
GetDevices(VOID);


CONST XStringW
GetOSIconName (
  IN  CONST CHAR8 *OSVersion
  );

EFI_STATUS
GetRootUUID (
  IN OUT REFIT_VOLUME *Volume
  );

EFI_STATUS
GetEarlyUserSettings (
  IN  EFI_FILE *RootDir,
      TagPtr   CfgDict
  );

EFI_STATUS
GetUserSettings (
  IN  EFI_FILE *RootDir,
      TagPtr CfgDict
  );

EFI_STATUS
InitTheme (
  BOOLEAN  UseThemeDefinedInNVRam,
  EFI_TIME *Time
  );

CHAR16*
GetOtherKextsDir (BOOLEAN On);

CHAR16*
GetOSVersionKextsDir (
  CHAR8 *OSVersion
  );

EFI_STATUS
InjectKextsFromDir (
  EFI_STATUS Status,
  CHAR16 *SrcDir
  );

VOID
ParseLoadOptions (
  OUT  CHAR16 **Conf,
  OUT  TagPtr *Dict
  );

EFI_STATUS
SaveSettings (VOID);



/** Returns a boolean and then enable disable the patch if MachOSEntry have a match for the booted OS. */
BOOLEAN IsPatchEnabled(CHAR8 *MatchOSEntry, CHAR8 *CurrOS);

/** return true if a given os contains '.' as separator,
 and then match components of the current booted OS. Also allow 10.10.x format meaning all revisions
 of the 10.10 OS */
BOOLEAN IsOSValid(CHAR8 *MatchOS, CHAR8 *CurrOS);

// Micky1979: Next five functions (+ needed struct) are to split a string like "10.10.5,10.7,10.11.6,10.8.x"
// in their components separated by comma (in this case)
struct MatchOSes {
    INTN   count;
    CHAR8* array[100];
};


/** return MatchOSes struct (count+array) with the components of str that contains the given char sep as separator. */
struct
MatchOSes *GetStrArraySeparatedByChar(CHAR8 *str, CHAR8 sep);

/** trim spaces in MatchOSes struct array */
VOID
TrimMatchOSArray(struct MatchOSes *s);

/** free MatchOSes struct and its array. */
VOID deallocMatchOSes(struct MatchOSes *s);

/** count occurrences of a given char in a char* string. */
INTN
countOccurrences(CHAR8 *s, CHAR8 c);


//get default boot
VOID GetBootFromOption(VOID);
VOID
InitKextList(VOID);

EFI_STATUS
LoadUserSettings (
  IN  EFI_FILE *RootDir,
      CONST CHAR16   *ConfName,
      TagPtr   *dict
  );

VOID
ParseSMBIOSSettings (
  TagPtr dictPointer
  );


#endif

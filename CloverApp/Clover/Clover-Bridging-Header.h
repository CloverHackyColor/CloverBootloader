//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

#ifndef CLOVERAPPLICATION
#define CLOVERAPPLICATION 1
#endif
/*
#ifdef DEBUG
#define DEBUG_BACKUP DEBUG // backup original
#undef DEBUG
#endif

#include "../../rEFIt_UEFI/Platform/Platform.h"

#ifdef DEBUG_BACKUP
#undef DEBUG
#define DEBUG DEBUG_BACKUP // restore original
#endif
*/

#import "NSWindowFix.h"
#import "ThemeImage.h"
#import "gfxutil.h"
#import "efidevp.h"

#define VOID void
#define CONST const

typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef UINT8               BOOLEAN;
typedef char                CHAR8;
typedef unsigned short      CHAR16;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef long long           INT64;
typedef INT64               INTN;
typedef unsigned long long  UINT64;
typedef UINT64              UINTN;
typedef  float              FLOAT;
typedef  double             DOUBLE;

typedef struct {
  UINT8 Type;       ///< 0x01 Hardware Device Path.
  ///< 0x02 ACPI Device Path.
  ///< 0x03 Messaging Device Path.
  ///< 0x04 Media Device Path.
  ///< 0x05 BIOS Boot Specification Device Path.
  ///< 0x7F End of Hardware Device Path.
  UINT8 SubType;    ///< Varies by Type
  ///< 0xFF End Entire Device Path, or
  ///< 0x01 End This Instance of a Device Path and start a new
  ///< Device Path.
  UINT8 Length[2];  ///< Specific Device Path data. Type and Sub-Type define
  ///< type of data. Size of data is included in Length.
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} EFI_GUID;

typedef struct {
  UINT8 b, g, r, a;
} EG_PIXEL;

typedef struct {
  INTN      Width;
  INTN      Height;
  EG_PIXEL    *PixelData;
  BOOLEAN     HasAlpha;   //moved here to avoid alignment issue
} EG_IMAGE;

typedef enum {
  BoolValue,
  Decimal,
  Hex,
  ASString,
  UNIString,
  RadioSwitch,
  CheckBit,
  
} ITEM_TYPE;

typedef struct {
  ITEM_TYPE ItemType; //string, value, boolean
  BOOLEAN Valid;
  BOOLEAN BValue;
  UINT8   Pad8;
  UINT32  IValue;
  //  UINT64  UValue;
  //  CHAR8*  AValue;
  CHAR16* SValue; // Max Size (see below) so the field can be edit by the GUI
  UINTN   LineShift;
} INPUT_ITEM;

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
  kTagTypeArray
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

typedef struct KEXT_PATCH KEXT_PATCH;
struct KEXT_PATCH
{
  CHAR8       *Name;
  CHAR8       *Label;
  BOOLEAN     IsPlistPatch;
  CHAR8       align[7];
  INT64        DataLen;
  UINT8       *Data;
  UINT8       *Patch;
  UINT8       *MaskFind;
  UINT8       *MaskReplace;
  CHAR8       *MatchOS;
  CHAR8       *MatchBuild;
  INPUT_ITEM  MenuItem;
};

typedef struct {
  CHAR8       *Label;
  INTN        DataLen;
  UINT8       *Data;
  UINT8       *Patch;
  UINT8       *MaskFind;
  UINT8       *MaskReplace;
  INTN        Count;
  CHAR8       *MatchOS;
  CHAR8       *MatchBuild;
  INPUT_ITEM  MenuItem;
} KERNEL_PATCH;

typedef struct KERNEL_AND_KEXT_PATCHES
{
  BOOLEAN KPDebug;
  BOOLEAN KPKernelCpu;
  BOOLEAN KPKernelLapic;
  BOOLEAN KPKernelXCPM;
  BOOLEAN KPKernelPm;
  BOOLEAN KPAppleIntelCPUPM;
  BOOLEAN KPAppleRTC;
  BOOLEAN KPDELLSMBIOS;  // Dell SMBIOS patch
  BOOLEAN KPPanicNoKextDump;
  UINT8   pad[3];
  UINT32  FakeCPUID;
  //  UINT32  align0;
  CHAR16  *KPATIConnectorsController;
#if defined(MDE_CPU_IA32)
  UINT32  align1;
#endif
  
  UINT8   *KPATIConnectorsData;
#if defined(MDE_CPU_IA32)
  UINT32  align2;
#endif
  
  UINTN   KPATIConnectorsDataLen;
#if defined(MDE_CPU_IA32)
  UINT32  align3;
#endif
  UINT8   *KPATIConnectorsPatch;
#if defined(MDE_CPU_IA32)
  UINT32  align4;
#endif
  
  INT32   NrKexts;
  UINT32  align40;
  KEXT_PATCH *KextPatches;   //zzzz
#if defined(MDE_CPU_IA32)
  UINT32  align5;
#endif
  
  INT32    NrForceKexts;
  UINT32  align50;
  CHAR16 **ForceKexts;
#if defined(MDE_CPU_IA32)
  UINT32 align6;
#endif
  INT32   NrKernels;
  KERNEL_PATCH *KernelPatches;
  INT32   NrBoots;
  KERNEL_PATCH *BootPatches;
  
} KERNEL_AND_KEXT_PATCHES;

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
};

typedef struct RT_VARIABLES RT_VARIABLES;
struct RT_VARIABLES {
  //  BOOLEAN  Disabled;
  CHAR16   *Name;
  EFI_GUID VarGuid;
};

typedef struct CUSTOM_LOADER_ENTRY CUSTOM_LOADER_ENTRY;
struct CUSTOM_LOADER_ENTRY {
  CUSTOM_LOADER_ENTRY     *Next;
  CUSTOM_LOADER_ENTRY     *SubEntries;
  EG_IMAGE                *Image;
  EG_IMAGE                *DriveImage;
  CONST CHAR16                  *ImagePath;
  CONST CHAR16                  *DriveImagePath;
  CONST CHAR16                  *Volume;
  CONST CHAR16                  *Path;
  CONST CHAR16                  *Options;
  CONST CHAR16                  *FullTitle;
  CONST CHAR16                  *Title;
  CONST CHAR16                  *Settings;
  CHAR16                  Hotkey;
  BOOLEAN                 CommonSettings;
  UINT8                   Flags;
  UINT8                   Type;
  UINT8                   VolumeType;
  UINT8                   KernelScan;
  UINT8                   CustomBoot;
  EG_IMAGE                *CustomLogo;
  EG_PIXEL                *BootBgColor;
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches; //zzzz
};

typedef struct CUSTOM_LEGACY_ENTRY CUSTOM_LEGACY_ENTRY;
struct CUSTOM_LEGACY_ENTRY {
  CUSTOM_LEGACY_ENTRY *Next;
  EG_IMAGE            *Image;
  EG_IMAGE            *DriveImage;
  CONST CHAR16              *ImagePath;
  CONST CHAR16              *DriveImagePath;
  CONST CHAR16              *Volume;
  CONST CHAR16              *FullTitle;
  CONST CHAR16              *Title;
  CHAR16              Hotkey;
  UINT8               Flags;
  UINT8               Type;
  UINT8               VolumeType;
};

typedef struct CUSTOM_TOOL_ENTRY CUSTOM_TOOL_ENTRY;
struct CUSTOM_TOOL_ENTRY {
  CUSTOM_TOOL_ENTRY *Next;
  EG_IMAGE          *Image;
  CHAR16            *ImagePath;
  CHAR16            *Volume;
  CHAR16            *Path;
  CHAR16            *Options;
  CHAR16            *FullTitle;
  CHAR16            *Title;
  CHAR16            Hotkey;
  UINT8             Flags;
  UINT8             VolumeType;
};

// Set of Search & replace bytes for VideoBiosPatchBytes().
typedef struct _VBIOS_PATCH_BYTES {
  VOID    *Find;
  VOID    *Replace;
  UINTN   NumberOfBytes;
} VBIOS_PATCH_BYTES;

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
  BOOLEAN                 Proportional;
  //  UINT8                   Pad22[1];
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
  EG_IMAGE                *CustomLogo;
  
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


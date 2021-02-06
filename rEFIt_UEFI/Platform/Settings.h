#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <Efi.h>
#include "../gui/menu_items/menu_items.h"

#include "../Platform/plist/plist.h"
#include "../Platform/guid.h"
#include "MacOsVersion.h"
#include "KERNEL_AND_KEXT_PATCHES.h"
#include "../libeg/XIcon.h"


#define CLOVER_SIGN             SIGNATURE_32('C','l','v','r')


#define QUIRK_DEFRAG  bit(0)
#define QUIRK_MMIO    bit(1)
#define QUIRK_SU      bit(2)
#define QUIRK_VAR     bit(3)
#define QUIRK_HIBER   bit(4)
#define QUIRK_SAFE    bit(5)
#define QUIRK_UNPROT  bit(6)
#define QUIRK_EXIT    bit(7)
#define QUIRK_REGION  bit(8)
#define QUIRK_SECURE  bit(9)
#define QUIRK_UEFI    bit(10)
#define QUIRK_CUSTOM  bit(11)
#define QUIRK_MAP     bit(12)
#define QUIRK_VIRT    bit(13)
#define QUIRK_OS      bit(14)
#define QUIRK_PERM    bit(15)

//// SysVariables
//typedef struct SYSVARIABLES SYSVARIABLES;
//struct SYSVARIABLES
//{
//  SYSVARIABLES      *Next;
//  CHAR16            *Key;
//  INPUT_ITEM        MenuItem;
//};

extern CONST CHAR8      *AudioOutputNames[];

class HDA_OUTPUTS
{
public:
  XStringW        Name;
//  CHAR8           *LineName;
  UINT8            Index;
  EFI_HANDLE      Handle = NULL;
  EFI_AUDIO_IO_PROTOCOL_DEVICE Device = EfiAudioIoDeviceOther;

  HDA_OUTPUTS() : Name(), Index(0) {}
  HDA_OUTPUTS(const HDA_OUTPUTS& other) = delete; // Can be defined if needed
  const HDA_OUTPUTS& operator = ( const HDA_OUTPUTS & ) = delete; // Can be defined if needed
  ~HDA_OUTPUTS() {}
};

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

class ACPI_DROP_TABLE
{
public:
  ACPI_DROP_TABLE *Next;
  UINT32          Signature;
  UINT32          Length;
  UINT64          TableId;
  INPUT_ITEM      MenuItem = INPUT_ITEM();
  BOOLEAN         OtherOS;

  ACPI_DROP_TABLE() : Next(0), Signature(0), Length(0), TableId(0), OtherOS(0) {}
  ACPI_DROP_TABLE(const ACPI_DROP_TABLE& other) = delete; // Can be defined if needed
  const ACPI_DROP_TABLE& operator = ( const ACPI_DROP_TABLE & ) = delete; // Can be defined if needed
  ~ACPI_DROP_TABLE() {}
};

class CUSTOM_LOADER_ENTRY
{
public:
  CUSTOM_LOADER_ENTRY     *Next;
  CUSTOM_LOADER_ENTRY     *SubEntries;
  XIcon                  Image = XIcon();
  XIcon                  DriveImage = XIcon();
  XStringW               ImagePath;
  XStringW               DriveImagePath;
  XStringW               Volume;
  XStringW               Path;
  XString8Array           LoadOptions;

  XStringW FullTitle;
  XStringW Title;
  XStringW Settings;
  CHAR16                  Hotkey;
  BOOLEAN                 CommonSettings;
  UINT8                   Flags;
  bool                    Hidden;
  UINT8                   Type;
  UINT8                   VolumeType;
  UINT8                   KernelScan;
  UINT8                   CustomBoot;
  XImage                  CustomLogo = XImage();
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL BootBgColor = EFI_GRAPHICS_OUTPUT_BLT_PIXEL({0,0,0,0});
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches;

  CUSTOM_LOADER_ENTRY() : Next(0), SubEntries(0), ImagePath(), DriveImagePath(), Volume(), Path(), LoadOptions(),
                          FullTitle(), Title(), Settings(), Hotkey(0), CommonSettings(0), Flags(0), Hidden(0), Type(0), VolumeType(0),
                          KernelScan(0), CustomBoot(0), KernelAndKextPatches()
						{ }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  CUSTOM_LOADER_ENTRY(const CUSTOM_LOADER_ENTRY&) = delete;
  CUSTOM_LOADER_ENTRY& operator=(const CUSTOM_LOADER_ENTRY&) = delete;

};

class CUSTOM_LEGACY_ENTRY
{
public:
  CUSTOM_LEGACY_ENTRY* Next;
  XIcon                Image = XIcon();
  XIcon                DriveImage = XIcon();
  XStringW             ImagePath;
  XStringW             DriveImagePath;
  XStringW             Volume;
  XStringW             FullTitle;
  XStringW             Title;
  CHAR16               Hotkey;
  UINT8                Flags;
  bool                 Hidden;
  UINT8                Type;
  UINT8                VolumeType;

  CUSTOM_LEGACY_ENTRY() : Next(0), ImagePath(), DriveImagePath(), Volume(), FullTitle(), Title(), Hotkey(0), Flags(0), Hidden(0), Type(0), VolumeType(0) { }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  CUSTOM_LEGACY_ENTRY(const CUSTOM_LEGACY_ENTRY&) = delete;
  CUSTOM_LEGACY_ENTRY& operator=(const CUSTOM_LEGACY_ENTRY&) = delete;
};

class CUSTOM_TOOL_ENTRY
{
public:
  CUSTOM_TOOL_ENTRY *Next;
  XIcon              Image = XIcon();
  XStringW           ImagePath;
  XStringW           Volume;
  XStringW           Path;
  XString8Array       LoadOptions;
  XStringW           FullTitle;
  XStringW           Title;
  CHAR16             Hotkey;
  UINT8              Flags;
  bool               Hidden;
  UINT8              VolumeType;

  CUSTOM_TOOL_ENTRY() : Next(0), ImagePath(), Volume(), Path(), LoadOptions(), FullTitle(), Title(), Hotkey(0), Flags(0), Hidden(0), VolumeType(0) { }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  CUSTOM_TOOL_ENTRY(const CUSTOM_TOOL_ENTRY&) = delete;
  CUSTOM_TOOL_ENTRY& operator=(const CUSTOM_TOOL_ENTRY&) = delete;
};

class DEV_PROPERTY
{
public:
  UINT32        Device;
  EFI_DEVICE_PATH_PROTOCOL* DevicePath = NULL;
  CHAR8         *Key;
  UINT8         *Value;
  UINTN         ValueLen;
  DEV_PROPERTY  *Next;   //next device or next property
  DEV_PROPERTY  *Child;  // property list of the device
  CHAR8         *Label;
  INPUT_ITEM    MenuItem = INPUT_ITEM();
  TAG_TYPE      ValueType = kTagTypeNone;

  DEV_PROPERTY() : Device(0), Key(0), Value(0), ValueLen(0), Next(0), Child(0), Label(0)  { }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  DEV_PROPERTY(const DEV_PROPERTY&) = delete;
  DEV_PROPERTY& operator=(const DEV_PROPERTY&) = delete;
};

//#pragma GCC diagnostic error "-Wpadded"

class DSDT_Patch
{
public :
  XBuffer<UINT8>   PatchDsdtFind;
  XBuffer<UINT8>   PatchDsdtReplace;
  XString8         PatchDsdtLabel;
  XBuffer<UINT8>   PatchDsdtTgt;
  INPUT_ITEM       PatchDsdtMenuItem = INPUT_ITEM();

  DSDT_Patch() : PatchDsdtFind(), PatchDsdtReplace(), PatchDsdtLabel(), PatchDsdtTgt() { }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  DSDT_Patch(const DSDT_Patch&) = delete;
  DSDT_Patch& operator=(const DSDT_Patch&) = delete;
};

class MMIOWhiteList
{
public :
  UINTN        address;
  XString8     comment;
  bool         enabled;

  MMIOWhiteList() : address(0), comment(), enabled(false) { }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  MMIOWhiteList(const MMIOWhiteList&) = delete;
  MMIOWhiteList& operator=(const MMIOWhiteList&) = delete;
};

class SETTINGS_DATA {
public:

  class BootClass {
    public:
      INTN                    Timeout = -1;
      bool                    SkipHibernateTimeout = false;
      bool                    DisableCloverHotkeys = false;
      XString8                BootArgs = XString8();
      bool                    LastBootedVolume = false;
      XStringW                DefaultVolume = XStringW();
      XStringW                DefaultLoader = XStringW();
      bool                    DebugLog = false;
      bool                    FastBoot = false;
      bool                    NoEarlyProgress = false;
      bool                    NeverHibernate = false;
      bool                    StrictHibernate = false;
      bool                    RtcHibernateAware = false;
      bool                    HibernationFixup = false;
      bool                    SignatureFixup = false;
      UINT8                   SecureBoot = 0;
      UINT8                   SecureBootSetupMode = 0;
      UINT8                   SecureBootPolicy = 0;
      // Secure boot white/black list
      UINT32                  SecureBootWhiteListCount = 0;
      UINT32                  SecureBootBlackListCount = 0;
      CHAR16                  **SecureBootWhiteList = 0;
      CHAR16                  **SecureBootBlackList = 0;
      INT8                    XMPDetection = 0;
      // LegacyBoot
      XStringW                LegacyBoot = XStringW();
      UINT16                  LegacyBiosDefaultEntry = 0;
      UINT8                   CustomBoot = 0;
      XImage                  *CustomLogo = 0;

      
  } Boot = BootClass();

  class GUIClass {
    public:
      INT32                   Timezone = 0xFF;
      XStringW                Theme = XStringW();
      bool                    CustomIcons = false;
      bool                    TextOnly = false;
      bool                    ShowOptimus = false;
      XStringW                ScreenResolution = XStringW();
      INTN                    ConsoleMode = 0;
      INTN                    Codepage = 0xC0;
      INTN                    CodepageSize = 0xC0;
      bool                    LegacyFirst = false;
      bool                    NoLegacy = false;
  } GUI = GUIClass();

  // SMBIOS TYPE0
  XString8                VendorName;
  XString8                RomVersion;
  XString8                EfiVersion;
  XString8                ReleaseDate;
  // SMBIOS TYPE1
  XString8                   ManufactureName;
  XString8                ProductName;
  XString8                   VersionNr;
  XString8                   SerialNr;
  XString8                SmUUID;
  CHAR8                   pad0[7];
//CHAR8                    Uuid;
//CHAR8                    SKUNumber;
  XString8                   FamilyName;
  XString8                   OEMProduct;
  XString8                   OEMVendor;
  // SMBIOS TYPE2
  XString8                   BoardManufactureName;
  XString8                   BoardSerialNumber;
  XString8                   BoardNumber; //Board-ID
  XString8                   LocationInChassis;
  XString8                   BoardVersion;
  XString8                   OEMBoard;
  UINT8                   BoardType;
  UINT8                   pad1;
  // SMBIOS TYPE3
  BOOLEAN                 Mobile;
  UINT8                   ChassisType;
  XString8                   ChassisManufacturer;
  XString8                   ChassisAssetTag;
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
  INT8                    pad17[1];
  XString8                   MemoryManufacturer;
  XString8                   MemorySerialNumber;
  XString8                   MemoryPartNumber;
  XString8                   MemorySpeed;
  // SMBIOS TYPE131
  UINT16                  CpuType;
  // SMBIOS TYPE132
  UINT16                  QPI;
  BOOLEAN                 SetTable132;
  BOOLEAN                 TrustSMBIOS;
  BOOLEAN                 InjectMemoryTables;
  BOOLEAN                 UseARTFreq;
  INT8                    pad18[3];

  // SMBIOS TYPE133
  UINT64                  PlatformFeature;
	
  // PatchTableType11
  BOOLEAN                 NoRomInfo;

  // OS parameters
  INT8                    pad181[7];
  XString8                Language;
  INT8                    pad19[2];
  XString8                CustomUuid;

  INT8                    pad20[6];

//Monitor
  BOOLEAN                 IntelMaxBacklight;
  UINT8                   pad21[1];
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
  UINT8                   pad22[2];
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
  UINT8                   pad23[1];
  UINT16                  C3Latency;
  BOOLEAN                 smartUPS;
  BOOLEAN                 PatchNMI;
  BOOLEAN                 EnableC7;
  UINT8                   SavingMode;

  XStringW                DsdtName;
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
  UINT8                   InjectSystemID_; // 0=false, 1=true, other value = default.
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
  UINT8                   pad24[5];
  VBIOS_PATCH_BYTES       *PatchVBiosBytes;
  UINTN                   PatchVBiosBytesCount;
  BOOLEAN                 InjectEDID;
  BOOLEAN                 LpcTune;
  UINT16                  DropOEM_DSM; //vacant
  UINT8                   pad25[4];
  UINT8                   *CustomEDID;
  UINT16                  CustomEDIDsize;
  UINT16                  EdidFixHorizontalSyncPulseWidth;
  UINT8                   EdidFixVideoInputSignal;

  UINT8                   pad26[1];
  XStringW                FBName;
  UINT16                  VideoPorts;
  BOOLEAN                 NvidiaGeneric;
  BOOLEAN                 NvidiaNoEFI;
  BOOLEAN                 NvidiaSingle;
  UINT8                   pad27[5];
  UINT64                  VRAM;
  UINT8                   Dcfg[8];
  UINT8                   NVCAP[20];
  INT8                    BootDisplay;
  BOOLEAN                 NvidiaWeb;
  UINT8                   pad41[2];
  UINT32                  DualLink;
  UINT32                  IgPlatform;


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


  //SkyLake
  BOOLEAN                 HWP;
  UINT8                   TDP;
  UINT32                  HWPValue;

  //Volumes hiding
  XString8Array           HVHideStrings;

  // KernelAndKextPatches
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches;
  BOOLEAN                 KextPatchesAllowed;
  BOOLEAN                 KernelPatchesAllowed; //From GUI: Only for user patches, not internal Clover

  XString8                AirportBridgeDeviceName;

  // Pre-language
  BOOLEAN                 KbdPrevLang;

  //Pointer
  BOOLEAN                 PointerEnabled;
  UINT8                   pad28[7];
  INTN                    PointerSpeed;
  UINT64                  DoubleClickTime;
  BOOLEAN                 PointerMirror;

//  UINT8                   pad7[6];
  UINT8                   pad29[6];

  UINT32                  RefCLK;

  // SysVariables
  UINT8                   pad30[4];
  XString8                RtMLB;
  XBuffer<UINT8>          RtROM;

  UINT32                  CsrActiveConfig;
  UINT16                  BooterConfig;
  XString8                BooterCfgStr;
  BOOLEAN                 NeverDoRecovery;

  // Multi-config
  CHAR16                  ConfigName[30];
  UINT8                   pad31[4];
//  XString8                MainConfigName;

  //Drivers
  XStringWArray           DisabledDriverArray;

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
  UINT8                   pad32[2];
  UINT32                  DisableFunctions;

  //Patch DSDT arbitrary
  XObjArray<DSDT_Patch>   DSDTPatchArray;
//  UINT32                  PatchDsdtNum;
//  UINT8                   **PatchDsdtFind;
//  UINT32                  *LenToFind;
//  UINT8                   **PatchDsdtReplace;
//  UINT32                  *LenToReplace;
//  CHAR8                   **PatchDsdtLabel;
//  CHAR8                   **PatchDsdtTgt;
//  INPUT_ITEM              *PatchDsdtMenuItem;

  BOOLEAN                 DebugDSDT;
  BOOLEAN                 SlpWak;
  BOOLEAN                 UseIntelHDMI;
  UINT8                   AFGLowPowerState;
  UINT8                   PNLF_UID;
//  UINT8                   pad83[4];


  // Table dropping
  UINT8                   pad34[3];
  ACPI_DROP_TABLE         *ACPIDropTables;

  // Custom entries
  BOOLEAN                 DisableEntryScan;
  BOOLEAN                 DisableToolScan;
  UINT8                   KernelScan;
  BOOLEAN                 LinuxScan;
  UINT8                   pad35[3];
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
  UINT8                   pad36[4];
  CHAR16                  **DisabledAML;

  //other
  UINT32                  IntelMaxValue;
//  UINT32                  AudioVolume;

  // boot.efi
  UINT32 OptionsBits;
  UINT32 FlagsBits;
  UINT32 UIScale;
  UINT32 EFILoginHiDPI;
  UINT8  flagstate[32];

  UINT8                   pad37[4];
  DEV_PROPERTY            *ArbProperties;
  
  UINT32 QuirksMask;
  UINT8                   pad38[4];
  UINTN MaxSlide;

  OC_BOOTER_QUIRKS   ocBooterQuirks;
  XObjArray<MMIOWhiteList> mmioWhiteListArray;

  BOOLEAN ProvideConsoleGop;


  SETTINGS_DATA() : VendorName(), RomVersion(), EfiVersion(), ReleaseDate(), ManufactureName(), ProductName(), VersionNr(), SerialNr(), SmUUID(),
                    pad0{0}, FamilyName(), OEMProduct(), OEMVendor(), BoardManufactureName(), BoardSerialNumber(), BoardNumber(), LocationInChassis(),
                    BoardVersion(), OEMBoard(), BoardType(0), pad1(0), Mobile(0), ChassisType(0), ChassisManufacturer(), ChassisAssetTag(), CpuFreqMHz(0),
                    BusSpeed(0), Turbo(0), EnabledCores(0), UserChange(0), QEMU(0), SmbiosVersion(0), Attribute(0), pad17{0}, MemoryManufacturer(),
                    MemorySerialNumber(), MemoryPartNumber(), MemorySpeed(), CpuType(0), QPI(0), SetTable132(0), TrustSMBIOS(0), InjectMemoryTables(0),
                    UseARTFreq(0), PlatformFeature(0), NoRomInfo(0), Language(), CustomUuid(),
                    IntelMaxBacklight(0), VendorEDID(0), ProductEDID(0), BacklightLevel(0), BacklightLevelConfig(0), IntelBacklight(0), MemoryFix(0), WithKexts(0),
                    WithKextsIfNoFakeSMC(0), FakeSMCFound(0), NoCaches(0), Debug(0), pad22{0}, DefaultBackgroundColor(0), ResetAddr(0), ResetVal(0), NoASPM(0),
                    DropSSDT(0), NoOemTableId(0), NoDynamicExtract(0), AutoMerge(0), GeneratePStates(0), GenerateCStates(0), GenerateAPSN(0), GenerateAPLF(0), GeneratePluginType(0),
                    PLimitDict(0), UnderVoltStep(0), DoubleFirstState(0), SuspendOverride(0), EnableC2(0), EnableC4(0), EnableC6(0), EnableISS(0), SlpSmiEnable(0),
                    FixHeaders(0), C3Latency(0), smartUPS(0), PatchNMI(0), EnableC7(0), SavingMode(0), DsdtName(), FixDsdt(0), MinMultiplier(0),
                    MaxMultiplier(0), PluginType(1), FixMCFG(0), DeviceRenameCount(0), DeviceRename(0), StringInjector(0), InjectSystemID_(0), NoDefaultProperties(0), ReuseFFFF(0),
                    FakeATI(0), FakeNVidia(0), FakeIntel(0), FakeLAN(0), FakeWIFI(0), FakeSATA(0), FakeXHCI(0), FakeIMEI(0), GraphicsInjector(0),
                    InjectIntel(0), InjectATI(0), InjectNVidia(0), DeInit(0), LoadVBios(0), PatchVBios(0), PatchVBiosBytes(0), PatchVBiosBytesCount(0), InjectEDID(0),
                    LpcTune(0), DropOEM_DSM(0), CustomEDID(0), CustomEDIDsize(0), EdidFixHorizontalSyncPulseWidth(0), EdidFixVideoInputSignal(0), FBName(), VideoPorts(0), NvidiaGeneric(0),
                    NvidiaNoEFI(0), NvidiaSingle(0), VRAM(0), Dcfg{0}, NVCAP{0}, BootDisplay(0), NvidiaWeb(0), pad41{0}, DualLink(0),
                    IgPlatform(0), HDAInjection(0),
                    HDALayoutId(0), USBInjection(0), USBFixOwnership(0), InjectClockID(0), HighCurrent(0), NameEH00(0), NameXH00(0), LANInjection(0), HDMIInjection(0),
                    HWP(0), TDP(0), HWPValue(0), HVHideStrings(), KernelAndKextPatches(), KextPatchesAllowed(0),
                    KernelPatchesAllowed(0), AirportBridgeDeviceName(), KbdPrevLang(0), PointerEnabled(0), PointerSpeed(0), DoubleClickTime(0), PointerMirror(0),
                    RefCLK(0), RtMLB(), RtROM(), CsrActiveConfig(0), BooterConfig(0), BooterCfgStr(), NeverDoRecovery(0),
                    ConfigName{0}, /*MainConfigName(0),*/ /*BlackListCount(0),*/ DisabledDriverArray(), RPlt{0}, RBr{0}, EPCI{0}, REV{0}, Rtc8Allowed(0),
                    ForceHPET(0), ResetHDA(0), PlayAsync(0), DisableFunctions(0), DSDTPatchArray(), DebugDSDT(0), SlpWak(0), UseIntelHDMI(0),
                    AFGLowPowerState(0), PNLF_UID(0), ACPIDropTables(0), DisableEntryScan(0), DisableToolScan(0), KernelScan(0), LinuxScan(0), CustomEntries(0),
                    CustomLegacy(0), CustomTool(0), NrAddProperties(0), AddProperties(0), BlockKexts{0}, SortedACPICount(0), SortedACPI(0), DisabledAMLCount(0), DisabledAML(0),
                    IntelMaxValue(0), OptionsBits(0), FlagsBits(0), UIScale(0), EFILoginHiDPI(0), flagstate{0},
                    ArbProperties(0), QuirksMask(0), MaxSlide(0), ocBooterQuirks{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, mmioWhiteListArray(), ProvideConsoleGop(0)
                  {};
  SETTINGS_DATA(const SETTINGS_DATA& other) = delete; // Can be defined if needed
  const SETTINGS_DATA& operator = ( const SETTINGS_DATA & ) = delete; // Can be defined if needed

  XBuffer<UINT8> serialize() const;

  ~SETTINGS_DATA() {}

  const XString8& getUUID();
  const XString8& getUUID(EFI_GUID* efiGuid);
  // If CustomUuid is defined, return false by default
  // If SmUUID is defined, return true by default.
  bool ShouldInjectSystemID() {
    if ( CustomUuid.notEmpty() &&  CustomUuid != nullGuid ) {
      if ( InjectSystemID_ == 2 ) return false;
      else return InjectSystemID_;
    }
    if ( SmUUID.isEmpty() || SmUUID == nullGuid ) return false;
    if ( InjectSystemID_ == 2 ) return true;
    return InjectSystemID_;
  }
};

//#pragma GCC diagnostic ignored "-Wpadded"

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
class ACPI_PATCHED_AML
{
public:
  ACPI_PATCHED_AML  *Next;
  CHAR16            *FileName;
  INPUT_ITEM        MenuItem = INPUT_ITEM();

  ACPI_PATCHED_AML() : Next(0), FileName(0) {};
  ACPI_PATCHED_AML(const ACPI_PATCHED_AML& other) = delete; // Can be defined if needed
  const ACPI_PATCHED_AML& operator = ( const ACPI_PATCHED_AML & ) = delete; // Can be defined if needed
  ~ACPI_PATCHED_AML() { }
};

// syscl - Side load kext
class SIDELOAD_KEXT
{
public:
  XObjArray<SIDELOAD_KEXT> PlugInList;
  XStringW       FileName;
  XStringW       KextDirNameUnderOEMPath;
  XStringW       Version;
  INPUT_ITEM     MenuItem = INPUT_ITEM();
  
  SIDELOAD_KEXT() : PlugInList(), FileName(), KextDirNameUnderOEMPath(), Version() {};
  SIDELOAD_KEXT(const SIDELOAD_KEXT& other) = delete; // Can be defined if needed
  const SIDELOAD_KEXT& operator = ( const SIDELOAD_KEXT & ) = delete; // Can be defined if needed
  ~SIDELOAD_KEXT() { }
};

class RT_VARIABLES
{
public:
  XStringW Name;
  EFI_GUID VarGuid;

RT_VARIABLES() : Name(), VarGuid{0,0,0,{0}} {};
  RT_VARIABLES(const RT_VARIABLES& other) = delete; // Can be defined if needed
  const RT_VARIABLES& operator = ( const RT_VARIABLES & ) = delete; // Can be defined if needed
  ~RT_VARIABLES() { }
};



extern XObjArray<RT_VARIABLES> BlockRtVariableArray;
extern XObjArray<HDA_OUTPUTS> AudioList;

extern XStringWArray ThemeNameArray;
extern CHAR16*       ConfigsList[20];
extern CHAR16*       DsdtsList[20];
extern UINTN DsdtsNum;
extern UINTN ConfigsNum;
//extern INTN    ScrollButtonsHeight;
//extern INTN    ScrollBarDecorationsHeight;
//extern INTN    ScrollScrollDecorationsHeight;
//extern INTN LayoutBannerOffset;
//extern INTN LayoutButtonOffset;
//extern INTN LayoutTextOffset;
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

//extern BOOLEAN                         defDSM;
//extern UINT16                          dropDSM;

extern TagDict*                          gConfigDict[];

// ACPI/PATCHED/AML
extern ACPI_PATCHED_AML                *ACPIPatchedAML;


// SysVariables
//extern SYSVARIABLES                   *SysVariables;

// Hold theme fixed IconFormat / extension
extern CHAR16                         *IconFormat;

extern CONST CHAR16                   *gFirmwareRevision;
extern CONST CHAR8* gRevisionStr;
extern CONST CHAR8* gFirmwareBuildDate;
extern CONST CHAR8* gBuildInfo;
extern const LString8  gBuildId;
extern const LString8  path_independant;
extern const LString8  gBuildIdGrepTag;


extern BOOLEAN                        ResumeFromCoreStorage;
extern BOOLEAN                        gRemapSmBiosIsRequire;  // syscl: pass argument for Dell SMBIOS here

extern EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl;


//
// config module
//

class REFIT_CONFIG
{
public:
//  INTN        Timeout;
  UINTN       DisableFlags; //to disable some volume types (optical, firewire etc)
//  BOOLEAN     TextOnly;
  BOOLEAN     Quiet;
//  BOOLEAN     LegacyFirst;
//  BOOLEAN     NoLegacy;
//  BOOLEAN     DebugLog;
  BOOLEAN     SpecialBootMode; // content of nvram var "aptiofixflag"
//  BOOLEAN     NeverHibernate;
//  BOOLEAN     StrictHibernate;
//  BOOLEAN     RtcHibernateAware;
//  BOOLEAN     HibernationFixup;
//  BOOLEAN     SignatureFixup;
//  XStringW    Theme;
//  XStringW    ScreenResolution;
//  INTN        ConsoleMode;
//  BOOLEAN     CustomIcons;
//  INTN        IconFormat = ICON_FORMAT_DEF; // not used anymore
//  BOOLEAN     NoEarlyProgress;
//  INT32       Timezone;
//  BOOLEAN     ShowOptimus;
//  INTN        Codepage;
//  INTN        CodepageSize;

  BOOLEAN       gBootChanged = FALSE;
  BOOLEAN       gThemeChanged = FALSE;
  BOOLEAN       NeedPMfix = FALSE;



  /*
   * Defqult ctor :
   *   -1,             // INTN        Timeout;
   *   0,              // UINTN       DisableFlags;
   *   FALSE,          // BOOLEAN     TextOnly;
   *   TRUE,           // BOOLEAN     Quiet;
   *   FALSE,          // BOOLEAN     LegacyFirst;
   *   FALSE,          // BOOLEAN     NoLegacy;
   *   FALSE,          // BOOLEAN     DebugLog;
   *   FALSE,          // BOOLEAN     FastBoot;
   *   FALSE,          // BOOLEAN     NeverHibernate;
   *   FALSE,          // BOOLEAN     StrictHibernate;
   *   FALSE,          // BOOLEAN     RtcHibernateAware;
   *   FALSE,          // BOOLEAN     HibernationFixup;
   *   FALSE,          // BOOLEAN     SignatureFixup;
   *   L""_XSW,        // XStringW    Theme;
   *   L""_XSW,        // XStringW    ScreenResolution;
   *   0,              // INTN        ConsoleMode;
   *   FALSE,          // BOOLEAN     CustomIcons;
   *   ICON_FORMAT_DEF, // INTN       IconFormat;
   *   FALSE,          // BOOLEAN     NoEarlyProgress;
   *   0xFF,           // INT32       Timezone; / 0xFF - not set
   *   FALSE,          // BOOLEAN     ShowOptimus;
   *   0xC0,           // INTN        Codepage;
   *   0xC0,           // INTN        CodepageSize; //extended latin
};
   *
   */
  REFIT_CONFIG() : DisableFlags(0), Quiet(TRUE),
                   SpecialBootMode(FALSE)
                    {};
  REFIT_CONFIG(const REFIT_CONFIG& other) = delete; // Can be defined if needed
  const REFIT_CONFIG& operator = ( const REFIT_CONFIG & ) = delete; // Can be defined if needed
  ~REFIT_CONFIG() {  }

  bool isFastBoot() { return SpecialBootMode || gSettings.Boot.FastBoot; }

} ;


extern REFIT_CONFIG GlobalConfig;


EFI_STATUS
SetFSInjection (
  IN LOADER_ENTRY *Entry
  );

void
SetDevices (
  LOADER_ENTRY *Entry
  );
//
// check if this entry corresponds to Boot# variable and then set BootCurrent
//
void
SetBootCurrent(REFIT_MENU_ITEM_BOOTNUM *LoadedEntry);

XString8 GetAuthRootDmg(const EFI_FILE& dir, const XStringW& path);

MacOsVersion GetMacOSVersionFromFolder(const EFI_FILE& dir, const XStringW& path);
MacOsVersion GetOSVersion(int LoaderType, const XStringW& APFSTargetUUID, const REFIT_VOLUME* Volume, XString8* BuildVersionPtr);

inline MacOsVersion GetOSVersion (IN LOADER_ENTRY *Entry) { return GetOSVersion(Entry->LoaderType, Entry->APFSTargetUUID, Entry->Volume, &Entry->BuildVersion); };


UINT32
GetCrc32 (
  UINT8 *Buffer,
  UINTN Size
  );

void
GetDevices(void);


CONST XStringW
GetOSIconName (
  const MacOsVersion& OSVersion
  );

EFI_STATUS
GetRootUUID (
  IN OUT REFIT_VOLUME *Volume
  );

EFI_STATUS
GetEarlyUserSettings (
  const TagDict*   CfgDict,
  SETTINGS_DATA& gSettings
  );

EFI_STATUS
GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& gSettings);

XStringW
GetOtherKextsDir (BOOLEAN On);

XStringW GetOSVersionKextsDir(const MacOsVersion& OSVersion);

EFI_STATUS
InjectKextsFromDir (
  EFI_STATUS Status,
  CHAR16 *SrcDir
  );

void
ParseLoadOptions (
  OUT  XStringW* ConfName,
  OUT  TagDict** Dict
  );

EFI_STATUS
SaveSettings (void);




/** return true if a given os contains '.' as separator,
 and then match components of the current booted OS. Also allow 10.10.x format meaning all revisions
 of the 10.10 OS */
//BOOLEAN IsOSValid(const XString8& MatchOS, const MacOsVersion& CurrOS);


//get default boot
void GetBootFromOption(void);

EFI_STATUS
LoadUserSettings (
    const XStringW& ConfName,
    TagDict** dict
  );

void
ParseSMBIOSSettings (
  const TagDict* dictPointer
  );


void testConfigPlist();

#endif

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <Efi.h>
#include "../gui/menu_items/menu_items.h"
#include "../include/OSFlags.h"
#include "../include/OSTypes.h"
#include "../include/Languages.h"
#include "../Platform/plist/plist.h"
#include "../Platform/guid.h"
#include "MacOsVersion.h"
#include "KERNEL_AND_KEXT_PATCHES.h"
#include "../libeg/XIcon.h"
#include "../cpp_lib/undefinable.h"
#include "../entry_scan/loader.h" // for KERNEL_SCAN_xxx constants


#define CLOVER_SIGN             SIGNATURE_32('C','l','v','r')


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

class CUSTOM_LOADER_SUBENTRY_SETTINGS;
class CUSTOM_LOADER_SUBENTRY;

//class ConfigPlistClass;
//class ConfigPlistClass::GUI_Custom_Entry_Class;
//template<class C> class XmlArray;

//void CompareCustomSubEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_SUBENTRY_SETTINGS>& olDCustomEntries, const XmlArray<GUI_Custom_SubEntry_Class>& newCustomEntries);
//BOOLEAN FillinCustomSubEntry(UINT8 parentType, IN OUT  CUSTOM_LOADER_SUBENTRY_SETTINGS *Entry, const TagDict* DictPointer, IN BOOLEAN SubEntry);
                   
class CUSTOM_LOADER_SUBENTRY_SETTINGS
{
public:
  bool                   Disabled = 0;
public: // temporary, must be protected:
  // member defined with _ prefix should not be accessed from outside. I left them public for now for CompareCustomEntries()
  undefinable_XString8   _Arguments = undefinable_XString8();
  XString8               _AddArguments = XString8();

  undefinable_XString8   _FullTitle = undefinable_XString8();
  undefinable_XString8   _Title = undefinable_XString8();

  undefinable_bool       _NoCaches = undefinable_bool();

public:

//  friend void ::CompareCustomSubEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_SUBENTRY_SETTINGS>& olDCustomEntries, const XmlArray<GUI_Custom_SubEntry_Class>& newCustomEntries);
//  friend BOOLEAN FillinCustomSubEntry(UINT8 parentType, IN OUT  CUSTOM_LOADER_SUBENTRY_SETTINGS *Entry, const TagDict* DictPointer, IN BOOLEAN SubEntry);
//  friend class ::CUSTOM_LOADER_SUBENTRY;
};

class CUSTOM_LOADER_ENTRY;

class CUSTOM_LOADER_SUBENTRY
{
public:
  const CUSTOM_LOADER_ENTRY& parent;
  const CUSTOM_LOADER_SUBENTRY_SETTINGS& settings = CUSTOM_LOADER_SUBENTRY_SETTINGS();

  CUSTOM_LOADER_SUBENTRY(const CUSTOM_LOADER_ENTRY& _customLoaderEntry, const CUSTOM_LOADER_SUBENTRY_SETTINGS& _settings) : parent(_customLoaderEntry), settings(_settings) {}
  
  XString8Array getLoadOptions() const;
  UINT8 getFlags(bool NoCachesDefault) const;

  const XString8& getTitle() const;
  const XString8& getFullTitle() const;
};

//class GUI_Custom_Entry_Class;
class CUSTOM_LOADER_ENTRY_SETTINGS;
//
//void CompareCustomEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS>& olDCustomEntries, const XmlArray<GUI_Custom_Entry_Class>& newCustomEntries);
//BOOLEAN FillinCustomEntry(IN OUT  CUSTOM_LOADER_ENTRY_SETTINGS *Entry, const TagDict* DictPointer, IN BOOLEAN SubEntry);

extern const XString8 defaultInstallTitle;
extern const XString8 defaultRecoveryTitle;
extern const XStringW defaultRecoveryImagePath;
extern const XStringW defaultRecoveryDriveImagePath;

class CUSTOM_LOADER_ENTRY_SETTINGS
{
public:
  bool                    Disabled = 0;
  XBuffer<UINT8>          ImageData = XBuffer<UINT8>();
  XBuffer<UINT8>          DriveImageData = XBuffer<UINT8>();
  XStringW                Volume = XStringW();
  XStringW                Path = XStringW();
  undefinable_XString8    Arguments = undefinable_XString8();
  XString8                AddArguments = XString8();
  XString8                FullTitle = XStringW();
  XStringW                Settings = XStringW(); // path of a config.plist that'll be read at the beginning of startloader
  CHAR16                  Hotkey = 0;
  BOOLEAN                 CommonSettings = 0;
//  UINT8                   Flags = 0;
  bool                    Hidden = 0;
  bool                    AlwaysHidden = 0;
  UINT8                   Type = 0;
  UINT8                   VolumeType = 0;
  UINT8                   KernelScan = KERNEL_SCAN_ALL;
  XString8                CustomLogoAsXString8 = XString8();
  XBuffer<UINT8>          CustomLogoAsData = XBuffer<UINT8>();
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL BootBgColor = EFI_GRAPHICS_OUTPUT_BLT_PIXEL({0,0,0,0});
  INT8                    InjectKexts = -1;
  undefinable_bool        NoCaches = undefinable_bool();
  XObjArray<CUSTOM_LOADER_SUBENTRY_SETTINGS> SubEntriesSettings = XObjArray<CUSTOM_LOADER_SUBENTRY_SETTINGS>();
public: // temporary, must be protected:
  XStringW                m_DriveImagePath = XStringW();
  XString8                m_Title = XStringW();
  UINT8                   CustomLogoTypeSettings = 0;
  XStringW                m_ImagePath = XStringW();

public:
  friend class ::CUSTOM_LOADER_ENTRY;
//  friend void ::CompareCustomEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS>& olDCustomEntries, const XmlArray<GUI_Custom_Entry_Class>& newCustomEntries);
  friend BOOLEAN FillinCustomEntry(IN OUT  CUSTOM_LOADER_ENTRY_SETTINGS *Entry, const TagDict* DictPointer, IN BOOLEAN SubEntry);


  const XString8& dgetTitle() const {
    if ( m_Title.notEmpty() ) return m_Title;
    if (OSTYPE_IS_OSX_RECOVERY(Type)) {
      return defaultRecoveryTitle;
    } else if (OSTYPE_IS_OSX_INSTALLER(Type)) {
      return defaultInstallTitle;
    }
    return NullXString8;
  }

  const XStringW& dgetImagePath() const {
    if ( m_ImagePath.notEmpty() ) return m_ImagePath;
    if ( ImageData.notEmpty() ) return NullXStringW;
    if (OSTYPE_IS_OSX_RECOVERY(Type)) return defaultRecoveryImagePath;
    return NullXStringW;
  }
  const XStringW& dgetDriveImagePath() const {
    if ( m_DriveImagePath.notEmpty() ) return m_DriveImagePath;
    if ( DriveImageData.notEmpty() ) return NullXStringW;
    if (OSTYPE_IS_OSX_RECOVERY(Type)) return defaultRecoveryDriveImagePath;
    return NullXStringW;
  }

};

class CUSTOM_LOADER_ENTRY
{
public:
  const CUSTOM_LOADER_ENTRY_SETTINGS& settings = CUSTOM_LOADER_ENTRY_SETTINGS();
  XObjArray<CUSTOM_LOADER_SUBENTRY> SubEntries = XObjArray<CUSTOM_LOADER_SUBENTRY>();
  XIcon                   Image = XIcon();
  XIcon                   DriveImage = XIcon();
  XImage                  CustomLogoImage = XImage(); // Todo : remove from settings.
  UINT8                   CustomLogoType = 0;
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches = KERNEL_AND_KEXT_PATCHES();

  CUSTOM_LOADER_ENTRY(const CUSTOM_LOADER_ENTRY_SETTINGS& _settings);

  XString8Array getLoadOptions() const;
  
  UINT8 getFlags(bool NoCachesDefault) const {
    UINT8 Flags = 0;
    if ( settings.Arguments.isDefined() ) Flags = OSFLAG_SET(Flags, OSFLAG_NODEFAULTARGS);
    if ( settings.AlwaysHidden ) Flags = OSFLAG_SET(Flags, OSFLAG_DISABLED);
    if ( settings.Type == OSTYPE_LIN ) Flags = OSFLAG_SET(Flags, OSFLAG_NODEFAULTARGS);
    if (OSTYPE_IS_OSX(settings.Type) || OSTYPE_IS_OSX_RECOVERY(settings.Type) || OSTYPE_IS_OSX_INSTALLER(settings.Type)) {
      Flags = OSFLAG_UNSET(Flags, OSFLAG_NOCACHES);
    }
    if ( settings.NoCaches.isDefined() ) {
      if ( settings.NoCaches ) Flags = OSFLAG_SET(Flags, OSFLAG_NOCACHES);
    }else{
      if (NoCachesDefault) {
        Flags = OSFLAG_SET(Flags, OSFLAG_NOCACHES);
      }
    }
    if ( SubEntries.notEmpty() ) Flags = OSFLAG_SET(Flags, OSFLAG_NODEFAULTMENU);
    return Flags;
  }
};

class CUSTOM_LEGACY_ENTRY_SETTINGS
{
public:
  bool                 Disabled = 0;
  XStringW             ImagePath = XStringW();
  XBuffer<UINT8>       ImageData = XBuffer<UINT8>();
  XStringW             DriveImagePath = XStringW();
  XBuffer<UINT8>       DriveImageData = XBuffer<UINT8>();
  XStringW             Volume = XStringW();
  XStringW             FullTitle = XStringW();
  XStringW             Title = XStringW();
  CHAR16               Hotkey = 0;
//  UINT8                Flags = 0;
  bool                 Hidden = 0;
  bool                 AlwaysHidden = 0;
  UINT8                Type = 0;
  UINT8                VolumeType = 0;
};

class CUSTOM_LEGACY_ENTRY
{
public:
  const CUSTOM_LEGACY_ENTRY_SETTINGS& settings = CUSTOM_LEGACY_ENTRY_SETTINGS();
  XIcon                Image = XIcon();
  XIcon                DriveImage = XIcon();

  CUSTOM_LEGACY_ENTRY(const CUSTOM_LEGACY_ENTRY_SETTINGS& _settings, const EFI_FILE& ThemeDir) : settings(_settings)
  {
    if ( settings.ImagePath.notEmpty() ) {
      Image.LoadXImage(&ThemeDir, settings.ImagePath);
    }else if ( settings.ImageData.notEmpty() ) {
      if ( !EFI_ERROR(Image.Image.FromPNG(settings.ImageData.data(), settings.ImageData.size())) ) {
        Image.setFilled();
      }
    }
    if ( settings.DriveImagePath.notEmpty() ) {
      DriveImage.LoadXImage(&ThemeDir, settings.DriveImagePath);
    }else if ( settings.DriveImageData.notEmpty() ) {
      if ( !EFI_ERROR(DriveImage.Image.FromPNG(settings.DriveImageData.data(), settings.DriveImageData.size())) ) {
        DriveImage.setFilled();
      }
    }
  }
  
  UINT8 getFlags() const {
    UINT8 Flags = 0;
    if ( settings.Disabled || settings.AlwaysHidden ) Flags = OSFLAG_SET(Flags, OSFLAG_DISABLED);
    return Flags;
  }
};

class CUSTOM_TOOL_ENTRY_SETTINGS
{
public:
  bool               Disabled = 0;
  XStringW           ImagePath = XStringW();
  XBuffer<UINT8>     ImageData = XBuffer<UINT8>();
  XStringW           Volume = XStringW();
  XStringW           Path = XStringW();
//  XString8Array      LoadOptions = XString8Array();
  XString8           Arguments = XString8();
  XStringW           FullTitle = XStringW();
  XStringW           Title = XStringW();
  CHAR16             Hotkey = 0;
//  UINT8              Flags = 0;
  bool               Hidden = 0;
  bool               AlwaysHidden = 0;
  UINT8              VolumeType = 0;

};

class CUSTOM_TOOL_ENTRY
{
public:
  XIcon              Image = XIcon();

  const CUSTOM_TOOL_ENTRY_SETTINGS& settings = CUSTOM_TOOL_ENTRY_SETTINGS();
  
  CUSTOM_TOOL_ENTRY(const CUSTOM_TOOL_ENTRY_SETTINGS& _settings, const EFI_FILE& ThemeDir) : settings(_settings)
  {
    if ( settings.ImagePath.notEmpty() ) {
      Image.LoadXImage(&ThemeDir, settings.ImagePath);
    } else if ( settings.ImageData.notEmpty() ) {
      if ( !EFI_ERROR(Image.Image.FromPNG(settings.ImageData.data(), settings.ImageData.size())) ) {
        Image.setFilled();
      }
    }
  }
  UINT8 getFlags() const {
    UINT8 Flags = 0;
    if ( settings.Disabled || settings.AlwaysHidden ) Flags = OSFLAG_SET(Flags, OSFLAG_DISABLED);
    return Flags;
  }
  XString8Array getLoadOptions() const {
    return Split<XString8Array>(settings.Arguments, " ");
  }
};

class DEV_PROPERTY
{
public:
  UINT32                       Device = 0;
  EFI_DEVICE_PATH_PROTOCOL*    DevicePath = NULL;
  CHAR8*                       Key = 0;
  UINT8*                       Value = 0;
  UINTN                        ValueLen = 0;
  DEV_PROPERTY*                Next = 0;   //next device or next property
  DEV_PROPERTY*                Child = 0;  // property list of the device
  CHAR8*                       Label = 0;
  INPUT_ITEM                   MenuItem = INPUT_ITEM();
  TAG_TYPE                     ValueType = kTagTypeNone;

  DEV_PROPERTY() {};
  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  DEV_PROPERTY(const DEV_PROPERTY&) { panic("nope"); };
  DEV_PROPERTY& operator=(const DEV_PROPERTY&) { panic("nope"); };
};

class DEV_ADDPROPERTY
{
public:
  UINT32                       Device = 0;
  XString8                     Key = XString8();
  XBuffer<uint8_t>             Value = XBuffer<uint8_t>();
  INPUT_ITEM                   MenuItem = INPUT_ITEM();
};

/**
  Set of Search & replace bytes for VideoBiosPatchBytes().
  this is supposed to be a replacement of VBIOS_PATCH_BYTES, but that would need VbiosPatchLibrary to be update to C++. Quite easy, but need cpp_fundation to become a library. TODO
**/
class VBIOS_PATCH {
public:
  XBuffer<uint8_t> Find = XBuffer<uint8_t>();
  XBuffer<uint8_t> Replace = XBuffer<uint8_t>();
};

class PatchVBiosBytesNewClass : public XObjArray<VBIOS_PATCH>
{
  mutable XArray<VBIOS_PATCH_BYTES> VBIOS_PATCH_BYTES_array = XArray<VBIOS_PATCH_BYTES>();
public:
  // Temporary bridge to old struct.
  const VBIOS_PATCH_BYTES* getVBIOS_PATCH_BYTES() {
    VBIOS_PATCH_BYTES_array.setSize(size());
    for ( size_t idx = 0 ; idx < size() ; ++idx ) {
      VBIOS_PATCH_BYTES_array[idx].Find = ElementAt(idx).Find.data();
      VBIOS_PATCH_BYTES_array[idx].Replace = ElementAt(idx).Replace.data();
      VBIOS_PATCH_BYTES_array[idx].NumberOfBytes = ElementAt(idx).Replace.size();
    }
    return VBIOS_PATCH_BYTES_array;
  }
  size_t getVBIOS_PATCH_BYTES_count() const {
    return size();
  }
};


class SETTINGS_DATA;
class ConfigPlistClass;
class TagDict;
//bool CompareOldNewSettings(const SETTINGS_DATA& , const ConfigPlistClass& );
EFI_STATUS GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& gSettings);

class SETTINGS_DATA {
public:

  class BootClass {
    public:
      INTN                    Timeout = -1;
      bool                    SkipHibernateTimeout = false;
      bool                    DisableCloverHotkeys = false;
      XString8                BootArgs = XString8();
      bool                    NeverDoRecovery = 0;
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
      INT8                   SecureSetting = 0; // 0 == false, 1 == true, -1 == undefined
      //UINT8                   SecureBoot = 0;
      //UINT8                   SecureBootSetupMode = 0;
      UINT8                   SecureBootPolicy = 0;
      // Secure boot white/black list
      XStringWArray           SecureBootWhiteList = XStringWArray();
      XStringWArray           SecureBootBlackList = XStringWArray();
      INT8                    XMPDetection = 0;
      // LegacyBoot
      XStringW                LegacyBoot = XStringW();
      UINT16                  LegacyBiosDefaultEntry = 0;
      UINT8                   CustomLogoType = 0;
      XString8                CustomLogoAsXString8 = XString8();
      XBuffer<UINT8>          CustomLogoAsData = XBuffer<UINT8>();
  };
  
  class ACPIClass
  {
    public:
      class ACPIDropTablesClass
      {
        public:
          UINT32   Signature = 0;
          UINT64   TableId = 0;
          UINT32   TabLength = 0;
          bool     OtherOS = 0;
      };
      
      class DSDTClass
      {
        public:
          class DSDT_Patch
          {
          public :
            bool             Disabled = bool();
            XString8         PatchDsdtLabel = XString8();
            XBuffer<UINT8>   PatchDsdtFind = XBuffer<UINT8>();
            XBuffer<UINT8>   PatchDsdtReplace = XBuffer<UINT8>();
            XBuffer<UINT8>   PatchDsdtTgt = XBuffer<UINT8>();
            INPUT_ITEM       PatchDsdtMenuItem = INPUT_ITEM();
          };

          XStringW                DsdtName = XStringW();
          bool                    DebugDSDT = 0;
          bool                    Rtc8Allowed = 0;
          UINT8                   PNLF_UID = 0;
          UINT32                  FixDsdt = 0;
          bool                    ReuseFFFF = 0;
          bool                    SuspendOverride = 0;
          XObjArray<DSDT_Patch>   DSDTPatchArray = XObjArray<DSDT_Patch>();
      };
      
      class SSDTClass
      {
        public:
          class GenerateClass
          {
            public:
              bool                 GeneratePStates = 0;
              bool                 GenerateCStates = 0;
              bool                 GenerateAPSN = 0;
              bool                 GenerateAPLF = 0;
              bool                 GeneratePluginType = 0;
          };

          bool                    DropSSDTSetting = 0;
          bool                    NoOemTableId = 0;
          bool                    NoDynamicExtract = 0;
          bool                    EnableISS = 0;
          bool                    EnableC7 = 0;
          bool                    _EnableC6 = 0;
          bool                    _EnableC4 = 0;
          bool                    _EnableC2 = 0;
          UINT16                  _C3Latency = 0;
          UINT8                   PLimitDict = 0;
          UINT8                   UnderVoltStep = 0;
          bool                    DoubleFirstState = 0;
          UINT8                   MinMultiplier = 0;
          UINT8                   MaxMultiplier = 0;
          UINT8                   PluginType = 0;
          GenerateClass           Generate = GenerateClass();
      };

      UINT64                            ResetAddr = 0;
      UINT8                             ResetVal = 0;
      bool                              SlpSmiEnable = 0;
      bool                              FixHeaders = 0;
      bool                              FixMCFG = 0;
      bool                              NoASPM = 0;
      bool                              smartUPS = 0;
      bool                              PatchNMI = 0;
      bool                              AutoMerge = 0;
      XStringWArray                     DisabledAML = XStringWArray();
      XString8Array                     SortedACPI = XString8Array();
      XObjArray<ACPI_NAME_LIST>         DeviceRename = XObjArray<ACPI_NAME_LIST>();
      XObjArray<ACPIDropTablesClass>    ACPIDropTablesArray = XObjArray<ACPIDropTablesClass>();
      DSDTClass DSDT =                  DSDTClass();
      SSDTClass SSDT =                  SSDTClass();
  };

  class GUIClass {
    public:
      class MouseClass {
        public:
          INTN                    PointerSpeed = 0;
          bool                 PointerEnabled = 0;
          UINT64                  DoubleClickTime = 0;
          bool                 PointerMirror = 0;
      } ;
      class ScanClass {
        public:
          bool                 DisableEntryScan = 0;
          bool                 DisableToolScan = 0;
          UINT8                KernelScan = 0;
          bool                 LinuxScan = 0;
          bool                 LegacyFirst = false;
          bool                 NoLegacy = false;
      };

      INT32                   Timezone = -1;
      XStringW                Theme = XStringW();
      //bool                    DarkEmbedded = 0;
      XString8                EmbeddedThemeType = XString8();
      bool                    PlayAsync = 0;
      bool                    CustomIcons = false;
      bool                    TextOnly = false;
      bool                    ShowOptimus = false;
      XStringW                ScreenResolution = XStringW();
      bool                    ProvideConsoleGop = 0;
      INTN                    ConsoleMode = 0;
      LANGUAGES               Language = english;
      bool                    KbdPrevLang = 0;
      XString8Array           HVHideStrings = XString8Array();
      ScanClass Scan =        ScanClass();
      MouseClass Mouse =      MouseClass();
      XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS> CustomEntriesSettings = XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS>();
      XObjArray<CUSTOM_LEGACY_ENTRY_SETTINGS> CustomLegacySettings = XObjArray<CUSTOM_LEGACY_ENTRY_SETTINGS>();
      XObjArray<CUSTOM_TOOL_ENTRY_SETTINGS>   CustomToolSettings = XObjArray<CUSTOM_TOOL_ENTRY_SETTINGS>();

      bool getDarkEmbedded(bool isDaylight) const;

  };

  class CPUClass {
    public:
      UINT16                  QPI = 0;
      UINT32                  CpuFreqMHz = 0;
      UINT16                  CpuType = 0;
      bool                    QEMU = 0;
      bool                    UseARTFreq = 0;
      UINT32                  BusSpeed = 0; //in kHz
      bool                    UserChange = 0;
      UINT8                   SavingMode = 0;
      bool                    HWPEnable = false;
      undefinable_uint32      HWPValue = undefinable_uint32();
      UINT8                   TDP = 0;
      bool                    TurboDisabled = 0;
      undefinable_bool        _EnableC6 = undefinable_bool();
      undefinable_bool        _EnableC4 = undefinable_bool();
      undefinable_bool        _EnableC2 = undefinable_bool();
      undefinable_uint16      _C3Latency = undefinable_uint16();
  };

  class SystemParametersClass {
    public:
      bool                 WithKexts = true;
      bool                 WithKextsIfNoFakeSMC = 0;
      bool                 NoCaches = 0;
      uint16_t   BacklightLevel = 0xFFFF;
      bool BacklightLevelConfig = false;
      XString8             CustomUuid = XString8();
    public: // temporary, must be protected:
      UINT8                _InjectSystemID = 2; // 0=false, 1=true, other value = default.
    public:
      bool                 NvidiaWeb = 0;
      
      friend class ::SETTINGS_DATA;
      friend unsigned long long ::GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& gSettings);
  };

  class GraphicsClass {
    public:
      class EDIDClass {
        public:
          bool                    InjectEDID = bool();
          XBuffer<UINT8>          CustomEDID = XBuffer<UINT8> ();
          UINT16                  VendorEDID = UINT16();
          UINT16                  ProductEDID = UINT16();
          UINT16                  EdidFixHorizontalSyncPulseWidth = UINT16();
          UINT8                   EdidFixVideoInputSignal = UINT8();
      };
      
      class InjectAsDictClass {
        public:
          bool GraphicsInjector = bool();
          bool InjectIntel = bool();
          bool InjectATI = bool();
          bool InjectNVidia = bool();
      };

      class GRAPHIC_CARD {
        public:
          UINT32            Signature = 0;
          XString8          Model = XString8();
          UINT32            Id = 0;
          UINT32            SubId = 0;
          UINT64            VideoRam = 0;
          UINTN             VideoPorts = 0;
          bool           LoadVBios = 0;
      };

      bool                     PatchVBios = bool();
      PatchVBiosBytesNewClass  PatchVBiosBytes = PatchVBiosBytesNewClass();
//      undefinable_bool InjectAsBool = undefinable_bool();
      bool                 RadeonDeInit = bool();
      bool                 LoadVBios = bool();
      UINT64               VRAM = bool();
      UINT32               RefCLK = UINT32();
      XStringW             FBName = XStringW();
      UINT16               VideoPorts = UINT16();
      bool                 NvidiaGeneric = bool();
      bool                 NvidiaNoEFI = bool();
      bool                 NvidiaSingle = bool();
      UINT8                Dcfg[8] = {0};
      UINT8                NVCAP[20] = {0};
      INT8                 BootDisplay = INT8();
      UINT32               DualLink = UINT32();
      UINT32               IgPlatform = UINT32(); //could also be snb-platform-id
      EDIDClass            EDID = EDIDClass();
      InjectAsDictClass    InjectAsDict = InjectAsDictClass();
      XObjArray<GRAPHIC_CARD> ATICardList = XObjArray<GRAPHIC_CARD>();
      XObjArray<GRAPHIC_CARD> NVIDIACardList = XObjArray<GRAPHIC_CARD>();

      
      //bool getGraphicsInjector() const { return InjectAsBool.isDefined() ? InjectAsBool.value() : InjectAsDict.GraphicsInjector; }
      //bool InjectIntel() const { return InjectAsBool.isDefined() ? InjectAsBool.value() : InjectAsDict.InjectIntel; }
      //bool InjectATI() const { return InjectAsBool.isDefined() ? InjectAsBool.value() : InjectAsDict.InjectATI; }
      //bool InjectNVidia() const { return InjectAsBool.isDefined() ? InjectAsBool.value() : InjectAsDict.InjectNVidia; }

  };
  
  class DevicesClass {
    public:
      
      class AudioClass {
        public:
          bool                    ResetHDA = bool();
          bool                 HDAInjection = bool();
          INT32                   HDALayoutId = INT32();
          UINT8                   AFGLowPowerState = UINT8();
      };
      class USBClass {
        public:
          bool                 USBInjection = bool();
          bool                 USBFixOwnership = bool();
          bool                 InjectClockID = bool();
          bool                 HighCurrent = bool();
          bool                 NameEH00 = bool();
          bool                 NameXH00 = bool();
      };
      class PropertiesClass {
        public:
          XString8 cDeviceProperties = XString8();
      };

      class FakeIDClass {
        public:
          //PCI devices
          UINT32                  FakeATI = UINT32();    //97
          UINT32                  FakeNVidia = UINT32();
          UINT32                  FakeIntel = UINT32();
          UINT32                  FakeLAN = UINT32();   //100
          UINT32                  FakeWIFI = UINT32();
          UINT32                  FakeSATA = UINT32();
          UINT32                  FakeXHCI = UINT32();  //103
          UINT32                  FakeIMEI = UINT32();  //106
      };

      bool                 StringInjector = bool();
      bool                 IntelMaxBacklight = bool();
      bool                 IntelBacklight = bool();
      UINT32               IntelMaxValue = UINT32();
      bool                 LANInjection = bool();
      bool                 HDMIInjection = bool();
      bool                 NoDefaultProperties = bool();
      bool                 UseIntelHDMI = bool();
      bool                 ForceHPET = bool();
      UINT32               DisableFunctions = UINT32();
      XString8             AirportBridgeDeviceName = XString8();
      AudioClass           Audio = AudioClass();
      USBClass             USB = USBClass();
      PropertiesClass      Properties = PropertiesClass();
      FakeIDClass          FakeID = FakeIDClass();
      
//      UINTN                NrAddProperties;
//      DEV_PROPERTY        *AddProperties;
      XObjArray<DEV_ADDPROPERTY>   AddProperties = XObjArray<DEV_ADDPROPERTY>();
      DEV_PROPERTY                *ArbProperties = 0;

  };

  class QuirksClass {
    public:
      class MMIOWhiteList
      {
        public :
          UINTN        address = 0;
          XString8     comment = XString8();
          bool         enabled = 0;
      };
    
      bool                     FuzzyMatch = bool();
      XString8                 OcKernelCache = XString8();
//      UINTN MaxSlide;
      OC_KERNEL_QUIRKS         OcKernelQuirks = OC_KERNEL_QUIRKS();
      OC_BOOTER_QUIRKS         ocBooterQuirks = OC_BOOTER_QUIRKS();
      XObjArray<MMIOWhiteList> mmioWhiteListArray = XObjArray<MMIOWhiteList>();
      UINT32                   QuirksMask = 0;
  };

  class RtVariablesClass {
    public:
      class RT_VARIABLES
      {
        public:
          bool     Disabled = bool();
          XString8 Comment = XStringW();
          XStringW Name = XStringW();
          EFI_GUID Guid = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
      };
        
      XString8                RtROMAsString = XString8();
      XBuffer<UINT8>          RtROMAsData = XBuffer<UINT8>();
      XString8                RtMLBSetting = XString8();
      UINT32                  CsrActiveConfig = UINT32();
      UINT16                  BooterConfig = UINT16();
      XString8                BooterCfgStr = XString8();
      XObjArray<RT_VARIABLES> BlockRtVariableArray = XObjArray<RT_VARIABLES>();

      bool GetLegacyLanAddress() const {
        return RtROMAsString.equalIC("UseMacAddr0") || RtROMAsString.equalIC("UseMacAddr1");
      }

  };


  BootClass Boot = BootClass();
  ACPIClass ACPI = ACPIClass();
  GUIClass GUI = GUIClass();
  CPUClass CPU = CPUClass();
  SystemParametersClass SystemParameters = SystemParametersClass();
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches = KERNEL_AND_KEXT_PATCHES();
  GraphicsClass Graphics = GraphicsClass();
  XStringWArray           DisabledDriverArray = XStringWArray();
  QuirksClass Quirks = QuirksClass();
  RtVariablesClass RtVariables = RtVariablesClass();
  DevicesClass Devices = DevicesClass();

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
  // SMBIOS TYPE3
  BOOLEAN                 Mobile;
  UINT8                   ChassisType;
  XString8                   ChassisManufacturer;
  XString8                   ChassisAssetTag;
  // SMBIOS TYPE4
  UINT8                   EnabledCores;
  // SMBIOS TYPE17
  UINT16                  SmbiosVersion;
  INT8                    Attribute;
  XString8                   MemoryManufacturer;
  XString8                   MemorySerialNumber;
  XString8                   MemoryPartNumber;
  XString8                   MemorySpeed;
  // SMBIOS TYPE131
  // SMBIOS TYPE132
  BOOLEAN                 TrustSMBIOS = 0;
  BOOLEAN                 InjectMemoryTables;

  // SMBIOS TYPE133
  UINT64                  PlatformFeature;
	
  // PatchTableType11
  BOOLEAN                 NoRomInfo;

  // OS parameters
  XString8                Language;


//Monitor
//Boot options
  BOOLEAN                 MemoryFix;
  BOOLEAN                 FakeSMCFound;

  // GUI parameters
  BOOLEAN                 Debug;
  UINT32                  DefaultBackgroundColor;

  //ACPI

//  BOOLEAN                 DropMCFG;

  //Injections



  //Graphics
//  UINT16                  PCIRootUID;
  BOOLEAN                 LpcTune;
  UINT16                  DropOEM_DSM; //vacant




  // USB DeviceTree injection
	


  //SkyLake

  //Volumes hiding

  // KernelAndKextPatches
  BOOLEAN                 KextPatchesAllowed;
  BOOLEAN                 KernelPatchesAllowed; //From GUI: Only for user patches, not internal Clover


  // Pre-language




  // SysVariables


  // Multi-config
  CHAR16                  ConfigName[30];
//  XString8                MainConfigName;

  //Drivers

  //SMC keys
  CHAR8                   RPlt[8];
  CHAR8                   RBr[8];
  UINT8                   EPCI[4];
  UINT8                   REV[6];

  //other devices


  BOOLEAN                 SlpWak;
  BOOLEAN                 UseIntelHDMI;


  //Add custom properties

  //BlackListed kexts
  CHAR16                  BlockKexts[64];

  // Disable inject kexts
//  UINT32                  DisableInjectKextCount;
//  CHAR16                  **DisabledInjectKext;
//  INPUT_ITEM              *InjectKextMenuItem;

  //ACPI tables

  //other
//  UINT32                  AudioVolume;

  // boot.efi
  UINT32 OptionsBits;
  UINT32 FlagsBits;
  UINT32 UIScale;
  UINT32 EFILoginHiDPI;
  UINT8  flagstate[32];

  


  SETTINGS_DATA() : VendorName(), RomVersion(), EfiVersion(), ReleaseDate(), ManufactureName(), ProductName(), VersionNr(), SerialNr(), SmUUID(),
                    FamilyName(), OEMProduct(), OEMVendor(), BoardManufactureName(), BoardSerialNumber(), BoardNumber(), LocationInChassis(),
                    BoardVersion(), OEMBoard(), BoardType(0), Mobile(0), ChassisType(0), ChassisManufacturer(), ChassisAssetTag(),
                    EnabledCores(0), SmbiosVersion(0), Attribute(0), MemoryManufacturer(),
                    MemorySerialNumber(), MemoryPartNumber(), MemorySpeed(), InjectMemoryTables(0),
                    PlatformFeature(0), NoRomInfo(0), Language(),
                    MemoryFix(0),
                    FakeSMCFound(0), Debug(0), DefaultBackgroundColor(0),
                    LpcTune(0), DropOEM_DSM(0),
                    KextPatchesAllowed(0),
                    KernelPatchesAllowed(0),
                    ConfigName{0}, /*MainConfigName(0),*/ /*BlackListCount(0),*/ RPlt{0}, RBr{0}, EPCI{0}, REV{0},
                     SlpWak(0), UseIntelHDMI(0),
                    BlockKexts{0},
                    OptionsBits(0), FlagsBits(0), UIScale(0), EFILoginHiDPI(0), flagstate{0}
                  {};
  SETTINGS_DATA(const SETTINGS_DATA& other) = delete; // Can be defined if needed
  const SETTINGS_DATA& operator = ( const SETTINGS_DATA & ) = delete; // Can be defined if needed

//  XBuffer<UINT8> serialize() const;

  ~SETTINGS_DATA() {}

  const XString8& getUUID();
  const XString8& getUUID(EFI_GUID* efiGuid);
  // If CustomUuid is defined, return false by default
  // If SmUUID is defined, return true by default.
  bool ShouldInjectSystemID() {
    if ( SystemParameters.CustomUuid.notEmpty() &&  SystemParameters.CustomUuid != nullGuidAsString ) {
      if ( SystemParameters._InjectSystemID == 2 ) return false;
      else return SystemParameters._InjectSystemID;
    }
    if ( SmUUID.isEmpty() || SmUUID == nullGuidAsString ) return false;
    if ( SystemParameters._InjectSystemID == 2 ) return true;
    return SystemParameters._InjectSystemID;
  }
  
  bool getEnableC6() const {
    if ( CPU._EnableC6.isDefined() ) return CPU._EnableC6.value();
    return ACPI.SSDT._EnableC6;
  }
  bool getEnableC4() const {
    if ( CPU._EnableC4.isDefined() ) return CPU._EnableC4.value();
    return ACPI.SSDT._EnableC4;
  }
  bool getEnableC2() const {
    if ( CPU._EnableC2.isDefined() ) return CPU._EnableC2.value();
    return ACPI.SSDT._EnableC2;
  }
  bool getC3Latency() const {
    if ( CPU._C3Latency.isDefined() ) return CPU._C3Latency.value();
    return ACPI.SSDT._C3Latency;
  }

};

//#pragma GCC diagnostic ignored "-Wpadded"

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
  XString8         FileName = XString8();
  INPUT_ITEM       MenuItem = INPUT_ITEM();

  ACPI_PATCHED_AML() {};
  ACPI_PATCHED_AML(const ACPI_PATCHED_AML& other) = delete; // Can be defined if needed
  const ACPI_PATCHED_AML& operator = ( const ACPI_PATCHED_AML & ) = delete; // Can be defined if needed
  ~ACPI_PATCHED_AML() { }
};

// syscl - Side load kext
class SIDELOAD_KEXT
{
public:
  XObjArray<SIDELOAD_KEXT> PlugInList;
  XStringW       FileName = XStringW();
  XStringW       KextDirNameUnderOEMPath = XStringW();
  XStringW       Version = XStringW();
  INPUT_ITEM     MenuItem = INPUT_ITEM();
  
  SIDELOAD_KEXT() : PlugInList() {};
  SIDELOAD_KEXT(const SIDELOAD_KEXT& other) = delete; // Can be defined if needed
  const SIDELOAD_KEXT& operator = ( const SIDELOAD_KEXT & ) = delete; // Can be defined if needed
  ~SIDELOAD_KEXT() { }
};



//extern XObjArray<RT_VARIABLES> gSettings.RtVariables.BlockRtVariableArray;
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
extern XObjArray<ACPI_PATCHED_AML>       ACPIPatchedAML;


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
//extern BOOLEAN                        gRemapSmBiosIsRequire;  // syscl: pass argument for Dell SMBIOS here

extern EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl;


//
// config module
//

class REFIT_CONFIG
{
public:
  UINTN       DisableFlags = 0; //to disable some volume types (optical, firewire etc)
  BOOLEAN     Quiet = true;
  BOOLEAN     SpecialBootMode = false; // content of nvram var "aptiofixflag"

  BOOLEAN       gBootChanged = FALSE;
  BOOLEAN       gThemeChanged = FALSE;
  BOOLEAN       NeedPMfix = FALSE;
  ACPI_DROP_TABLE         *ACPIDropTables = NULL;

  UINT8                   CustomLogoType = 0; // this will be initialized with gSettings.Boot.CustomBoot and set back to CUSTOM_BOOT_DISABLED if CustomLogo could not be loaded or decoded (see afterGetUserSettings)
  XImage                  *CustomLogo = 0;

  bool                    DropSSDT = 0; // init with gSettings.Boot.DropSSDTSetting. Put back to false is one table is dropped (see afterGetUserSettings)

  UINT8                   SecureBoot = 0;
  UINT8                   SecureBootSetupMode = 0;

  BOOLEAN                 SetTable132 = 0;
  BOOLEAN                 HWP = 0;

  bool                EnableC6 = 0;
  bool                EnableC4 = 0;
  bool                EnableC2 = 0;
  uint16_t              C3Latency = 0;

  XObjArray<CUSTOM_LOADER_ENTRY> CustomEntries = XObjArray<CUSTOM_LOADER_ENTRY>();
  XObjArray<CUSTOM_LEGACY_ENTRY> CustomLegacyEntries = XObjArray<CUSTOM_LEGACY_ENTRY>();
  XObjArray<CUSTOM_TOOL_ENTRY> CustomToolsEntries = XObjArray<CUSTOM_TOOL_ENTRY>();

  INTN                    Codepage = 0xC0;
  INTN                    CodepageSize = 0xC0;

  bool KPKernelPm = bool();
  bool KPAppleIntelCPUPM = bool();

  XBuffer<UINT8>          RtROM = XBuffer<UINT8>();
  XString8                RtMLB = XString8();

  bool Turbo = true;

  REFIT_CONFIG() {};
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

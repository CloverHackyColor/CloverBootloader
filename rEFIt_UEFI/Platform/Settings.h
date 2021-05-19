#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <Efi.h>
#include "../gui/menu_items/menu_items.h" // TODO: break that dependency
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
//#include "../Platform/smbios.h"
#include "../Platform/platformdata.h"
#include "../Settings/ConfigPlist/ConfigPlistClass.h"
#include "../Platform/guid.h"
#include "../Platform/SettingsUtils.h"
#include "../Platform/hda.h"
#include "../Settings/ConfigManager.h"

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
extern BOOLEAN           gFirmwareClover;


class HDA_OUTPUTS
{
public:
  XStringW        Name;
  UINT8           Index;
  EFI_HANDLE      Handle = NULL;
  EFI_AUDIO_IO_PROTOCOL_DEVICE Device = EfiAudioIoDeviceOther;

  HDA_OUTPUTS() : Name(), Index(0) {}
  HDA_OUTPUTS(const HDA_OUTPUTS& other) = delete; // Can be defined if needed
  const HDA_OUTPUTS& operator = ( const HDA_OUTPUTS & ) = delete; // Can be defined if needed
  ~HDA_OUTPUTS() {}
};

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
//    UINT16            codec_vendor_id;
//    UINT16            codec_device_id;
//    UINT8             codec_revision_id;
//    UINT8             codec_stepping_id;
//    UINT8             codec_maj_rev;
//    UINT8             codec_min_rev;
//    UINT8             codec_num_function_groups;
//    CHAR16            *codec_name;
} HDA_PROPERTIES;

class ACPI_NAME
{
public:
	XString8 Name = XString8();
  
#if __cplusplus > 201703L
  bool operator == (const ACPI_NAME&) const = default;
#endif
  bool isEqual(const ACPI_NAME& other) const
  {
    if ( !(Name == other.Name) ) return false;
    return true;
  }
  void takeValueFrom(const ACPI_NAME& configPlist)
  {
    //Name = configPlist.dgetName();
  }

  XString8Array getSplittedName() const {
    XString8Array splittedName = Split<XString8Array>(Name, ".");
    for ( size_t idx = 0 ; idx < splittedName.size() ; ++idx) {
      XString8& name = splittedName[idx];
      while ( name.length() > 4 ) name.deleteCharsAtPos(name.length()-1);
      while ( name.length() < 4 ) name.strcat('_');
    }
    for ( size_t idx = 1 ; idx < splittedName.size() ; ++idx) {
      splittedName.insertReferenceAtPos(splittedName.ExtractFromPos(idx), 0, true); // A swap method in XObjARray would be slightly better to avoid memcpy in XObjArray when an object is removed.
    }
    return splittedName;
  }
};

class ACPI_RENAME_DEVICE
{
public:
  ACPI_NAME acpiName = ACPI_NAME();
  XString8 renameTo = XString8();
  
#if __cplusplus > 201703L
  bool operator == (const ACPI_RENAME_DEVICE&) const = default;
#endif
  bool isEqual(const ACPI_RENAME_DEVICE& other) const
  {
    if ( !acpiName.isEqual(other.acpiName) ) return false;
    if ( !(renameTo == other.renameTo) ) return false;
    return true;
  }
  void takeValueFrom(const XmlAddKey<XmlKey, XmlString8>& configPlist)
  {
    acpiName.Name = configPlist.key();
    renameTo = configPlist.value();
  }

  XString8 getRenameTo() const {
    if ( renameTo.length() == 4 ) return renameTo;
    XString8 newName =  renameTo;
    while ( newName.length() > 4 ) newName.deleteCharsAtPos(newName.length()-1);
    while ( newName.length() < 4 ) newName.strcat('_');
    return newName;
  }

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
  
#if __cplusplus > 201703L
  bool operator == (const CUSTOM_LOADER_SUBENTRY_SETTINGS&) const = default;
#endif
  bool isEqual(const CUSTOM_LOADER_SUBENTRY_SETTINGS& other) const
  {
    if ( !(Disabled == other.Disabled) ) return false;
    if ( !(_Arguments == other._Arguments) ) return false;
    if ( !(_AddArguments == other._AddArguments) ) return false;
    if ( !(_FullTitle == other._FullTitle) ) return false;
    if ( !(_Title == other._Title) ) return false;
    if ( !(_NoCaches == other._NoCaches) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_SubEntry_Class& configPlist)
  {
    Disabled = configPlist.dgetDisabled();
    _Arguments = configPlist.dget_Arguments();
    _AddArguments = configPlist.dget_AddArguments();
    _FullTitle = configPlist.dget_FullTitle();
    _Title = configPlist.dget_Title();
    _NoCaches = configPlist.dget_NoCaches();
  }

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

/*
 * Wrapper class to bring some syntaxic sugar : initialisation at construction, assignment, == operator, etc.
 */
class EFI_GRAPHICS_OUTPUT_BLT_PIXELClass : public EFI_GRAPHICS_OUTPUT_BLT_PIXEL
{
public:
	EFI_GRAPHICS_OUTPUT_BLT_PIXELClass() { Blue = 0; Green = 0; Red = 0; Reserved = 0; }

  EFI_GRAPHICS_OUTPUT_BLT_PIXELClass(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& other) { Blue = other.Blue; Green = other.Green; Red = other.Red; Reserved = other.Reserved; }
  
	bool operator == (const EFI_GRAPHICS_OUTPUT_BLT_PIXELClass& other) const {
		if ( !(Blue == other.Blue) ) return false;
		if ( !(Green == other.Green) ) return false;
		if ( !(Red == other.Red) ) return false;
		if ( !(Reserved == other.Reserved) ) return false;
		return true;
	}
};

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
  char32_t                Hotkey = 0;
  BOOLEAN                 CommonSettings = 0;
//  UINT8                   Flags = 0;
  bool                    Hidden = 0;
  bool                    AlwaysHidden = 0;
  UINT8                   Type = 0;
  UINT8                   VolumeType = 0;
  UINT8                   KernelScan = KERNEL_SCAN_ALL;
  XString8                CustomLogoAsXString8 = XString8();
  XBuffer<UINT8>          CustomLogoAsData = XBuffer<UINT8>();
  EFI_GRAPHICS_OUTPUT_BLT_PIXELClass BootBgColor = EFI_GRAPHICS_OUTPUT_BLT_PIXELClass();
  INT8                    InjectKexts = -1;
  undefinable_bool        NoCaches = undefinable_bool();
  XObjArrayWithTakeValueFromXmlArray<CUSTOM_LOADER_SUBENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_SubEntry_Class>
                          SubEntriesSettings = XObjArrayWithTakeValueFromXmlArray<CUSTOM_LOADER_SUBENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_SubEntry_Class>();

public: // temporary, must be protected:
  XStringW                m_DriveImagePath = XStringW();
  XString8                m_Title = XStringW();
  UINT8                   CustomLogoTypeSettings = 0;
  XStringW                m_ImagePath = XStringW();

  bool                 ForceTextMode = 0; // 2021-04-22

public:
  
#if __cplusplus > 201703L
  bool operator == (const CUSTOM_LOADER_ENTRY_SETTINGS&) const = default;
#endif
  bool isEqual(const CUSTOM_LOADER_ENTRY_SETTINGS& other) const
  {
    if ( !(Disabled == other.Disabled) ) return false;
    if ( !(ImageData == other.ImageData) ) return false;
    if ( !(DriveImageData == other.DriveImageData) ) return false;
    if ( !(Volume == other.Volume) ) return false;
    if ( !(Path == other.Path) ) return false;
    if ( !(Arguments == other.Arguments) ) return false;
    if ( !(AddArguments == other.AddArguments) ) return false;
    if ( !(FullTitle == other.FullTitle) ) return false;
    if ( !(Settings == other.Settings) ) return false;
    if ( !(Hotkey == other.Hotkey) ) return false;
    if ( !(CommonSettings == other.CommonSettings) ) return false;
    if ( !(Hidden == other.Hidden) ) return false;
    if ( !(AlwaysHidden == other.AlwaysHidden) ) return false;
    if ( !(Type == other.Type) ) return false;
    if ( !(VolumeType == other.VolumeType) ) return false;
    if ( !(KernelScan == other.KernelScan) ) return false;
    if ( !(CustomLogoAsXString8 == other.CustomLogoAsXString8) ) return false;
    if ( !(CustomLogoAsData == other.CustomLogoAsData) ) return false;
    if ( memcmp(&BootBgColor, &other.BootBgColor, sizeof(BootBgColor)) != 0 ) return false;
    if ( !(InjectKexts == other.InjectKexts) ) return false;
    if ( !(NoCaches == other.NoCaches) ) return false;
    if ( !SubEntriesSettings.isEqual(other.SubEntriesSettings) ) return false;
    if ( !(m_DriveImagePath == other.m_DriveImagePath) ) return false;
    if ( !(m_Title == other.m_Title) ) return false;
    if ( !(CustomLogoTypeSettings == other.CustomLogoTypeSettings) ) return false;
    if ( !(m_ImagePath == other.m_ImagePath) ) return false;
    if ( !(ForceTextMode == other.ForceTextMode) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class& configPlist)
  {
    Disabled = configPlist.dgetDisabled();
    ImageData = configPlist.dgetImageData();
    DriveImageData = configPlist.dgetDriveImageData();
    Volume = configPlist.dgetVolume();
    Path = configPlist.dgetPath();
    Arguments = configPlist.dgetArguments();
    AddArguments = configPlist.dgetAddArguments();
    FullTitle = configPlist.dgetFullTitle();
    Settings = configPlist.dgetSettings();
    Hotkey = configPlist.dgetHotkey();
    CommonSettings = configPlist.dgetCommonSettings();
    Hidden = configPlist.dgetHidden();
    AlwaysHidden = configPlist.dgetAlwaysHidden();
    Type = configPlist.dgetType();
    VolumeType = configPlist.dgetVolumeType();
    KernelScan = configPlist.dgetKernelScan();
    CustomLogoAsXString8 = configPlist.dgetCustomLogoAsXString8();
    CustomLogoAsData = configPlist.dgetCustomLogoAsData();
    BootBgColor = configPlist.dgetBootBgColor();
    InjectKexts = configPlist.dgetInjectKexts();
    NoCaches = configPlist.dgetNoCaches();
    SubEntriesSettings.takeValueFrom(configPlist.SubEntries);
    m_DriveImagePath = configPlist.dgetm_DriveImagePath();
    m_Title = configPlist.dgetm_Title();
    CustomLogoTypeSettings = configPlist.dgetCustomLogoTypeSettings();
    m_ImagePath = configPlist.dgetm_ImagePath();
    ForceTextMode = configPlist.dgetForceTextMode();
  }

  friend class ::CUSTOM_LOADER_ENTRY;
//  friend void ::CompareCustomEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS>& olDCustomEntries, const XmlArray<GUI_Custom_Entry_Class>& newCustomEntries);


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
  char32_t             Hotkey = 0;
//  UINT8                Flags = 0;
  bool                 Hidden = 0;
  bool                 AlwaysHidden = 0;
  UINT8                Type = 0;
  UINT8                VolumeType = 0;

#if __cplusplus > 201703L
  bool operator == (const CUSTOM_LEGACY_ENTRY_SETTINGS&) const = default;
#endif
  bool isEqual(const CUSTOM_LEGACY_ENTRY_SETTINGS& other) const
  {
    if ( !(Disabled == other.Disabled) ) return false;
    if ( !(ImagePath == other.ImagePath) ) return false;
    if ( !(ImageData == other.ImageData) ) return false;
    if ( !(DriveImagePath == other.DriveImagePath) ) return false;
    if ( !(DriveImageData == other.DriveImageData) ) return false;
    if ( !(Volume == other.Volume) ) return false;
    if ( !(FullTitle == other.FullTitle) ) return false;
    if ( !(Title == other.Title) ) return false;
    if ( !(Hotkey == other.Hotkey) ) return false;
    if ( !(Hidden == other.Hidden) ) return false;
    if ( !(AlwaysHidden == other.AlwaysHidden) ) return false;
    if ( !(Type == other.Type) ) return false;
    if ( !(VolumeType == other.VolumeType) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class& configPlist)
  {
    Disabled = configPlist.dgetDisabled();
    ImagePath = configPlist.dgetImagePath();
    ImageData = configPlist.dgetImageData();
    DriveImagePath = configPlist.dgetDriveImagePath();
    DriveImageData = configPlist.dgetDriveImageData();
    Volume = configPlist.dgetVolume();
    FullTitle = configPlist.dgetFullTitle();
    Title = configPlist.dgetTitle();
    Hotkey = configPlist.dgetHotkey();
    Hidden = configPlist.dgetHidden();
    AlwaysHidden = configPlist.dgetAlwaysHidden();
    Type = configPlist.dgetType();
    VolumeType = configPlist.dgetVolumeType();
  }
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
  char32_t           Hotkey = 0;
//  UINT8              Flags = 0;
  bool               Hidden = 0;
  bool               AlwaysHidden = 0;
  UINT8              VolumeType = 0;
  
#if __cplusplus > 201703L
  bool operator == (const CUSTOM_TOOL_ENTRY_SETTINGS&) const = default;
#endif
  bool isEqual(const CUSTOM_TOOL_ENTRY_SETTINGS& other) const
  {
    if ( !(Disabled == other.Disabled) ) return false;
    if ( !(ImagePath == other.ImagePath) ) return false;
    if ( !(ImageData == other.ImageData) ) return false;
    if ( !(Volume == other.Volume) ) return false;
    if ( !(Path == other.Path) ) return false;
    if ( !(Arguments == other.Arguments) ) return false;
    if ( !(FullTitle == other.FullTitle) ) return false;
    if ( !(Title == other.Title) ) return false;
    if ( !(Hotkey == other.Hotkey) ) return false;
    if ( !(Hidden == other.Hidden) ) return false;
    if ( !(AlwaysHidden == other.AlwaysHidden) ) return false;
    if ( !(VolumeType == other.VolumeType) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class& configPlist)
  {
    Disabled = configPlist.dgetDisabled();
    ImagePath = configPlist.dgetImagePath();
    ImageData = configPlist.dgetImageData();
    Volume = configPlist.dgetVolume();
    Path = configPlist.dgetPath();
    Arguments = configPlist.dgetArguments();
    FullTitle = configPlist.dgetFullTitle();
    Title = configPlist.dgetTitle();
    Hotkey = configPlist.dgetHotkey();
    Hidden = configPlist.dgetHidden();
    AlwaysHidden = configPlist.dgetAlwaysHidden();
    VolumeType = configPlist.dgetVolumeType();
  }

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

/*
 * From config.plist,
 * /Devices/Properties will construct a XObjArray<DEV_PROPERTY>, stored in ArbProperties
 * /Devices/Arbitrary/CustomProperties will construct a XObjArray<DEV_PROPERTY>, stored in ArbProperties
 */
// 2021-04 Jief : this is the old structure. Kept here for be able to compare old and new code.
class DEV_PROPERTY
{
public:
  UINT32        Device = 0;
  EFI_DEVICE_PATH_PROTOCOL* DevicePath = NULL;
  CHAR8         *Key = 0;
  UINT8         *Value = 0;
  UINTN         ValueLen = 0;
  DEV_PROPERTY  *Next = 0;   //next device or next property
  DEV_PROPERTY  *Child = 0;  // property list of the device
  CHAR8         *Label = 0;
  INPUT_ITEM    MenuItem = INPUT_ITEM();
  TAG_TYPE      ValueType = kTagTypeNone;

  DEV_PROPERTY() { }

  // Not sure if default are valid. Delete them. If needed, proper ones can be created
  DEV_PROPERTY(const DEV_PROPERTY&) { panic("nope"); }
  DEV_PROPERTY& operator=(const DEV_PROPERTY&) = delete;
};


/**
  Set of Search & replace bytes for VideoBiosPatchBytes().
  this is supposed to be a replacement of VBIOS_PATCH_BYTES, but that would need VbiosPatchLibrary to be update to C++. Quite easy, but need cpp_fundation to become a library. TODO
**/
class VBIOS_PATCH {
public:
  XBuffer<uint8_t> Find = XBuffer<uint8_t>();
  XBuffer<uint8_t> Replace = XBuffer<uint8_t>();

#if __cplusplus > 201703L
		bool operator == (const VBIOS_PATCH&) const = default;
#endif
    bool isEqual(const VBIOS_PATCH& other) const
    {
      if ( !(Find == other.Find) ) return false;
      if ( !(Replace == other.Replace) ) return false;
      return true;
    }
    void takeValueFrom(const ConfigPlistClass::Graphics_Class::Graphics_PatchVBiosBytes_Class& configPlist)
    {
      Find = configPlist.dgetFind();
      Replace = configPlist.dgetReplace();
    }
};

class PatchVBiosBytesNewClass : public XObjArrayWithTakeValueFromXmlArray<VBIOS_PATCH, ConfigPlistClass::Graphics_Class::Graphics_PatchVBiosBytes_Class>
{
  mutable XArray<VBIOS_PATCH_BYTES> VBIOS_PATCH_BYTES_array = XArray<VBIOS_PATCH_BYTES>();
public:
  
#if __cplusplus > 201703L
		bool operator == (const PatchVBiosBytesNewClass& other) const { return XObjArray<VBIOS_PATCH>::operator ==(other); }
#endif

  // Temporary bridge to old struct.
  const VBIOS_PATCH_BYTES* getVBIOS_PATCH_BYTES() const {
    VBIOS_PATCH_BYTES_array.setSize(size());
    for ( size_t idx = 0 ; idx < size() ; ++idx ) {
      VBIOS_PATCH_BYTES_array[idx].Find = ElementAt(idx).Find.vdata();
      VBIOS_PATCH_BYTES_array[idx].Replace = ElementAt(idx).Replace.vdata();
      VBIOS_PATCH_BYTES_array[idx].NumberOfBytes = ElementAt(idx).Replace.size();
    }
    return VBIOS_PATCH_BYTES_array;
  }
  size_t getVBIOS_PATCH_BYTES_count() const {
    return size();
  }

  bool isEqual(const PatchVBiosBytesNewClass& other) const
  {
  	return XObjArray<VBIOS_PATCH>::isEqual(other);
//  	getVBIOS_PATCH_BYTES();
//    if ( VBIOS_PATCH_BYTES_array.size() != other.VBIOS_PATCH_BYTES_array.size() ) return false;
//    for ( size_t idx = 0 ; idx < VBIOS_PATCH_BYTES_array.size() ; ++idx ) {
//      if ( VBIOS_PATCH_BYTES_array[idx].NumberOfBytes != other[idx].Find.size() ) return false;
//      if ( memcmp(VBIOS_PATCH_BYTES_array[idx].Find, other.VBIOS_PATCH_BYTES_array[idx].Find, VBIOS_PATCH_BYTES_array[idx].NumberOfBytes) != 0 ) return false;
//      if ( memcmp(VBIOS_PATCH_BYTES_array[idx].Replace, other.VBIOS_PATCH_BYTES_array[idx].Replace, VBIOS_PATCH_BYTES_array[idx].NumberOfBytes) != 0 ) return false;
//    }
//    return true;
  }
//  void takeValueFrom(const PatchVBiosBytesNewClass& configPlist)
//  {
//  }

};


class SETTINGS_DATA;
class ConfigPlistClass;
class TagDict;
//bool CompareOldNewSettings(const SETTINGS_DATA& , const ConfigPlistClass& );
//EFI_STATUS GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& gSettings);

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
      
#if __cplusplus > 201703L
		bool operator == (const BootClass&) const = default;
#endif
      bool isEqual(const BootClass& other) const
      {
        if ( !(Timeout == other.Timeout) ) return false;
        if ( !(SkipHibernateTimeout == other.SkipHibernateTimeout) ) return false;
        if ( !(DisableCloverHotkeys == other.DisableCloverHotkeys) ) return false;
        if ( !(BootArgs == other.BootArgs) ) return false;
        if ( !(NeverDoRecovery == other.NeverDoRecovery) ) return false;
        if ( !(LastBootedVolume == other.LastBootedVolume) ) return false;
        if ( !(DefaultVolume == other.DefaultVolume) ) return false;
        if ( !(DefaultLoader == other.DefaultLoader) ) return false;
        if ( !(DebugLog == other.DebugLog) ) return false;
        if ( !(FastBoot == other.FastBoot) ) return false;
        if ( !(NoEarlyProgress == other.NoEarlyProgress) ) return false;
        if ( !(NeverHibernate == other.NeverHibernate) ) return false;
        if ( !(StrictHibernate == other.StrictHibernate) ) return false;
        if ( !(RtcHibernateAware == other.RtcHibernateAware) ) return false;
        if ( !(HibernationFixup == other.HibernationFixup) ) return false;
        if ( !(SignatureFixup == other.SignatureFixup) ) return false;
        if ( !(SecureSetting == other.SecureSetting) ) return false;
        if ( !(SecureBootPolicy == other.SecureBootPolicy) ) return false;
        if ( !(SecureBootWhiteList == other.SecureBootWhiteList) ) return false;
        if ( !(SecureBootBlackList == other.SecureBootBlackList) ) return false;
        if ( !(XMPDetection == other.XMPDetection) ) return false;
        if ( !(LegacyBoot == other.LegacyBoot) ) return false;
        if ( !(LegacyBiosDefaultEntry == other.LegacyBiosDefaultEntry) ) return false;
        if ( !(CustomLogoType == other.CustomLogoType) ) return false;
        if ( !(CustomLogoAsXString8 == other.CustomLogoAsXString8) ) return false;
        if ( !(CustomLogoAsData == other.CustomLogoAsData) ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::Boot_Class& configPlist)
      {
        Timeout = configPlist.dgetTimeout();
        SkipHibernateTimeout = configPlist.dgetSkipHibernateTimeout();
        DisableCloverHotkeys = configPlist.dgetDisableCloverHotkeys();
        BootArgs = configPlist.dgetBootArgs();
        NeverDoRecovery = configPlist.dgetNeverDoRecovery();
        LastBootedVolume = configPlist.dgetLastBootedVolume();
        DefaultVolume = configPlist.dgetDefaultVolume();
        DefaultLoader = configPlist.dgetDefaultLoader();
        DebugLog = configPlist.dgetDebugLog();
        FastBoot = configPlist.dgetFastBoot();
        NoEarlyProgress = configPlist.dgetNoEarlyProgress();
        NeverHibernate = configPlist.dgetNeverHibernate();
        StrictHibernate = configPlist.dgetStrictHibernate();
        RtcHibernateAware = configPlist.dgetRtcHibernateAware();
        HibernationFixup = configPlist.dgetHibernationFixup();
        SignatureFixup = configPlist.dgetSignatureFixup();
        SecureSetting = configPlist.dgetSecureSetting();
        SecureBootPolicy = configPlist.dgetSecureBootPolicy();
        SecureBootWhiteList = configPlist.dgetSecureBootWhiteList();
        SecureBootBlackList = configPlist.dgetSecureBootBlackList();
        XMPDetection = configPlist.dgetXMPDetection();
        LegacyBoot = configPlist.dgetLegacyBoot(gFirmwareClover);
        LegacyBiosDefaultEntry = configPlist.dgetLegacyBiosDefaultEntry();
        CustomLogoType = configPlist.dgetCustomLogoType();
        CustomLogoAsXString8 = configPlist.dgetCustomLogoAsXString8();
        CustomLogoAsData = configPlist.dgetCustomLogoAsData();
      }
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
          
#if __cplusplus > 201703L
          bool operator == (const ACPIDropTablesClass&) const = default;
#endif
          bool isEqual(const ACPIDropTablesClass& other) const
          {
            if ( !(Signature == other.Signature) ) return false;
            if ( !(TableId == other.TableId) ) return false;
            if ( !(TabLength == other.TabLength) ) return false;
            if ( !(OtherOS == other.OtherOS) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class& configPlist)
          {
            Signature = configPlist.dgetSignature();
            TableId = configPlist.dgetTableId();
            TabLength = configPlist.dgetTabLength();
            OtherOS = configPlist.dgetOtherOS();
          }
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
            INPUT_ITEM       PatchDsdtMenuItem = INPUT_ITEM(); // Not read from config.plist. Should be moved out.

#if __cplusplus > 201703L
            bool operator == (const DSDT_Patch&) const = default;
#endif
            bool isEqual(const DSDT_Patch& other) const
            {
              if ( !(Disabled == other.Disabled) ) return false;
              if ( !(PatchDsdtLabel == other.PatchDsdtLabel) ) return false;
              if ( !(PatchDsdtFind == other.PatchDsdtFind) ) return false;
              if ( !(PatchDsdtReplace == other.PatchDsdtReplace) ) return false;
              if ( !(PatchDsdtTgt == other.PatchDsdtTgt) ) return false;
              if ( !(PatchDsdtMenuItem == other.PatchDsdtMenuItem) ) return false;
              return true;
            }
            void takeValueFrom(const ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Patch_Class& configPlist)
            {
              Disabled = configPlist.dgetDisabled();
              PatchDsdtLabel = configPlist.dgetPatchDsdtLabel();
              PatchDsdtFind = configPlist.dgetPatchDsdtFind();
              PatchDsdtReplace = configPlist.dgetPatchDsdtReplace();
              PatchDsdtTgt = configPlist.dgetPatchDsdtTgt();
              PatchDsdtMenuItem.BValue = !configPlist.dgetDisabled();
            }
          };

          XStringW                DsdtName = XStringW();
          bool                    DebugDSDT = 0;
          bool                    Rtc8Allowed = 0;
          UINT8                   PNLF_UID = 0;
          UINT32                  FixDsdt = 0;
          bool                    ReuseFFFF = 0;
          bool                    SuspendOverride = 0;
//          XObjArray<DSDT_Patch>   DSDTPatchArray = XObjArray<DSDT_Patch>();
          XObjArrayWithTakeValueFromXmlArray<DSDT_Patch, ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Patch_Class>
                                  DSDTPatchArray = XObjArrayWithTakeValueFromXmlArray<DSDT_Patch, ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Patch_Class>();

#if __cplusplus > 201703L
          bool operator == (const DSDTClass&) const = default;
#endif
          bool isEqual(const DSDTClass& other) const
          {
            if ( !(DsdtName == other.DsdtName) ) return false;
            if ( !(DebugDSDT == other.DebugDSDT) ) return false;
            if ( !(Rtc8Allowed == other.Rtc8Allowed) ) return false;
            if ( !(PNLF_UID == other.PNLF_UID) ) return false;
            if ( !(FixDsdt == other.FixDsdt) ) return false;
            if ( !(ReuseFFFF == other.ReuseFFFF) ) return false;
            if ( !(SuspendOverride == other.SuspendOverride) ) return false;
            if ( !DSDTPatchArray.isEqual(other.DSDTPatchArray) ) return false;
            return true;
          }
        void takeValueFrom(const ConfigPlistClass::ACPI_Class::DSDT_Class& configPlist)
          {
            DsdtName = configPlist.dgetDsdtName();
            DebugDSDT = configPlist.dgetDebugDSDT();
            Rtc8Allowed = configPlist.dgetRtc8Allowed();
            PNLF_UID = configPlist.dgetPNLF_UID();
            FixDsdt = configPlist.dgetFixDsdt();
            ReuseFFFF = configPlist.dgetReuseFFFF();
            SuspendOverride = configPlist.dgetSuspendOverride();
            DSDTPatchArray.takeValueFrom(configPlist.Patches);
          }
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

#if __cplusplus > 201703L
              bool operator == (const GenerateClass&) const = default;
#endif
              bool isEqual(const GenerateClass& other) const
              {
                if ( !(GeneratePStates == other.GeneratePStates) ) return false;
                if ( !(GenerateCStates == other.GenerateCStates) ) return false;
                if ( !(GenerateAPSN == other.GenerateAPSN) ) return false;
                if ( !(GenerateAPLF == other.GenerateAPLF) ) return false;
                if ( !(GeneratePluginType == other.GeneratePluginType) ) return false;
                return true;
              }
              void takeValueFrom(const ConfigPlistClass::ACPI_Class::SSDT_Class::XmlUnionGenerate& configPlist)
              {
                GeneratePStates = configPlist.dgetGeneratePStates();
                GenerateCStates = configPlist.dgetGenerateCStates();
                GenerateAPSN = configPlist.dgetGenerateAPSN();
                GenerateAPLF = configPlist.dgetGenerateAPLF();
                GeneratePluginType = configPlist.dgetGeneratePluginType();
              }
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
          
#if __cplusplus > 201703L
          bool operator == (const SSDTClass&) const = default;
#endif
          bool isEqual(const SSDTClass& other) const
          {
            if ( !(DropSSDTSetting == other.DropSSDTSetting) ) return false;
            if ( !(NoOemTableId == other.NoOemTableId) ) return false;
            if ( !(NoDynamicExtract == other.NoDynamicExtract) ) return false;
            if ( !(EnableISS == other.EnableISS) ) return false;
            if ( !(EnableC7 == other.EnableC7) ) return false;
            if ( !(_EnableC6 == other._EnableC6) ) return false;
            if ( !(_EnableC4 == other._EnableC4) ) return false;
            if ( !(_EnableC2 == other._EnableC2) ) return false;
            if ( !(_C3Latency == other._C3Latency) ) return false;
            if ( !(PLimitDict == other.PLimitDict) ) return false;
            if ( !(UnderVoltStep == other.UnderVoltStep) ) return false;
            if ( !(DoubleFirstState == other.DoubleFirstState) ) return false;
            if ( !(MinMultiplier == other.MinMultiplier) ) return false;
            if ( !(MaxMultiplier == other.MaxMultiplier) ) return false;
            if ( !(PluginType == other.PluginType) ) return false;
            if ( !Generate.isEqual(other.Generate) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::ACPI_Class::SSDT_Class& configPlist)
          {
            DropSSDTSetting = configPlist.dgetDropSSDTSetting();
            NoOemTableId = configPlist.dgetNoOemTableId();
            NoDynamicExtract = configPlist.dgetNoDynamicExtract();
            EnableISS = configPlist.dgetEnableISS();
            EnableC7 = configPlist.dgetEnableC7();
            _EnableC6 = configPlist.dget_EnableC6();
            _EnableC4 = configPlist.dget_EnableC4();
            _EnableC2 = configPlist.dget_EnableC2();
            _C3Latency = configPlist.dget_C3Latency();
            PLimitDict = configPlist.dgetPLimitDict();
            UnderVoltStep = configPlist.dgetUnderVoltStep();
            DoubleFirstState = configPlist.dgetDoubleFirstState();
            MinMultiplier = configPlist.dgetMinMultiplier();
            MaxMultiplier = configPlist.dgetMaxMultiplier();
            PluginType = configPlist.dgetPluginType();
            Generate.takeValueFrom(configPlist.Generate);
          }
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
//      XObjArray<ACPI_RENAME_DEVICE>     DeviceRename = XObjArray<ACPI_RENAME_DEVICE>();
//      XObjArrayWithTakeValueFrom<ACPI_RENAME_DEVICE, ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class>
//                                        DeviceRename = XObjArrayWithTakeValueFrom<ACPI_RENAME_DEVICE, ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class>();
      class DeviceRename_Array : public XObjArray<ACPI_RENAME_DEVICE> {
       public:
        void takeValueFrom(const ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class& configPlist)
        {
          size_t idx;
          for ( idx = 0 ; idx < configPlist.size() ; ++idx ) {
            if ( idx < size() ) ElementAt(idx).takeValueFrom(configPlist.getAtIndex(idx));
            else {
              ACPI_RENAME_DEVICE* s = new ACPI_RENAME_DEVICE;
              s->takeValueFrom(configPlist.getAtIndex(idx));
              AddReference(s, true);
            }
          }
          while ( idx < size() ) RemoveAtIndex(idx);
        }
      } DeviceRename = DeviceRename_Array();
//      XObjArray<ACPIDropTablesClass>    ACPIDropTablesArray = XObjArray<ACPIDropTablesClass>();
      XObjArrayWithTakeValueFromXmlArray<ACPIDropTablesClass, ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class>
                                        ACPIDropTablesArray = XObjArrayWithTakeValueFromXmlArray<ACPIDropTablesClass, ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class>();
//      class ACPIDropTablesArrayClass : public XObjArray<ACPIDropTablesClass> {
//       public:
//        void takeValueFrom(const XmlArray<ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class>& configPlist)
//        {
//          size_t idx;
//          for ( idx = 0 ; idx < configPlist.size() ; ++idx ) {
//            if ( idx < size() ) ElementAt(idx).takeValueFrom(configPlist[idx]);
//            else {
//              ACPIDropTablesClass* s = new ACPIDropTablesClass;
//              s->takeValueFrom(configPlist[idx]);
//              AddReference(s, true);
//            }
//          }
//          while ( idx < size() ) RemoveAtIndex(idx);
//        }
//      } ACPIDropTablesArray = ACPIDropTablesArrayClass();
      DSDTClass DSDT =                  DSDTClass();
      SSDTClass SSDT =                  SSDTClass();
        
#if __cplusplus > 201703L
      bool operator == (const ACPIClass&) const = default;
#endif
      bool isEqual(const ACPIClass& other) const
      {
        if ( !(ResetAddr == other.ResetAddr) ) return false;
        if ( !(ResetVal == other.ResetVal) ) return false;
        if ( !(SlpSmiEnable == other.SlpSmiEnable) ) return false;
        if ( !(FixHeaders == other.FixHeaders) ) return false;
        if ( !(FixMCFG == other.FixMCFG) ) return false;
        if ( !(NoASPM == other.NoASPM) ) return false;
        if ( !(smartUPS == other.smartUPS) ) return false;
        if ( !(PatchNMI == other.PatchNMI) ) return false;
        if ( !(AutoMerge == other.AutoMerge) ) return false;
        if ( !(DisabledAML == other.DisabledAML) ) return false;
        if ( !(SortedACPI == other.SortedACPI) ) return false;
        if ( !DeviceRename.isEqual(other.DeviceRename) ) return false;
        if ( !ACPIDropTablesArray.isEqual(other.ACPIDropTablesArray) ) return false;
        if ( !DSDT.isEqual(other.DSDT) ) return false;
        if ( !SSDT.isEqual(other.SSDT) ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::ACPI_Class& configPlist)
      {
        ResetAddr = configPlist.dgetResetAddr();
        ResetVal = configPlist.dgetResetVal();
        SlpSmiEnable = configPlist.dgetSlpSmiEnable();
        FixHeaders = configPlist.dgetFixHeaders() || configPlist.DSDT.Fixes.dgetFixHeaders();
        FixMCFG = configPlist.dgetFixMCFG();
        NoASPM = configPlist.dgetNoASPM();
        smartUPS = configPlist.dgetsmartUPS();
        PatchNMI = configPlist.dgetPatchNMI();
        AutoMerge = configPlist.dgetAutoMerge();
        DisabledAML = configPlist.dgetDisabledAML();
        SortedACPI = configPlist.dgetSortedACPI();
        DeviceRename.takeValueFrom(configPlist.RenameDevices);
        ACPIDropTablesArray.takeValueFrom(configPlist.ACPIDropTablesArray);
        DSDT.takeValueFrom(configPlist.DSDT);
        SSDT.takeValueFrom(configPlist.SSDT);
      }
  };

  class GUIClass {
    public:
      class MouseClass {
        public:
          INTN                 PointerSpeed = 0;
          bool                 PointerEnabled = 0;
          UINT64               DoubleClickTime = 0;
          bool                 PointerMirror = 0;
          
#if __cplusplus > 201703L
          bool operator == (const MouseClass&) const = default;
#endif
          bool isEqual(const MouseClass& other) const
          {
            if ( !(PointerSpeed == other.PointerSpeed) ) return false;
            if ( !(PointerEnabled == other.PointerEnabled) ) return false;
            if ( !(DoubleClickTime == other.DoubleClickTime) ) return false;
            if ( !(PointerMirror == other.PointerMirror) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Mouse_Class& configPlist)
          {
            PointerSpeed = configPlist.dgetPointerSpeed();
            PointerEnabled = configPlist.dgetPointerEnabled();
            DoubleClickTime = configPlist.dgetDoubleClickTime();
            PointerMirror = configPlist.dgetPointerMirror();
          }
      } ;
      class ScanClass {
        public:
          bool                 DisableEntryScan = 0;
          bool                 DisableToolScan = 0;
          UINT8                KernelScan = 0;
          bool                 LinuxScan = 0;
          bool                 LegacyFirst = false;
          bool                 NoLegacy = false;
          
#if __cplusplus > 201703L
          bool operator == (const ScanClass&) const = default;
#endif
          bool isEqual(const ScanClass& other) const
          {
            if ( !(DisableEntryScan == other.DisableEntryScan) ) return false;
            if ( !(DisableToolScan == other.DisableToolScan) ) return false;
            if ( !(KernelScan == other.KernelScan) ) return false;
            if ( !(LinuxScan == other.LinuxScan) ) return false;
            if ( !(LegacyFirst == other.LegacyFirst) ) return false;
            if ( !(NoLegacy == other.NoLegacy) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Scan_Class& configPlist)
          {
            DisableEntryScan = configPlist.dgetDisableEntryScan();
            DisableToolScan = configPlist.dgetDisableToolScan();
            KernelScan = configPlist.dgetKernelScan();
            LinuxScan = configPlist.dgetLinuxScan();
            LegacyFirst = configPlist.dgetLegacyFirst();
            NoLegacy = configPlist.dgetNoLegacy();
          }
      };

      INT32                   Timezone = 0xFF;
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
      XString8                Language = XString8();
      LanguageCode            languageCode = english;
      bool                    KbdPrevLang = 0;
      XString8Array           HVHideStrings = XString8Array();
      ScanClass               Scan =        ScanClass();
      MouseClass              Mouse =      MouseClass();
//      XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS> CustomEntriesSettings = XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS>();
      XObjArrayWithTakeValueFromXmlArray<CUSTOM_LOADER_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class>
                              CustomEntriesSettings = XObjArrayWithTakeValueFromXmlArray<CUSTOM_LOADER_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class>();
//      XObjArray<CUSTOM_LEGACY_ENTRY_SETTINGS> CustomLegacySettings = XObjArray<CUSTOM_LEGACY_ENTRY_SETTINGS>();
      XObjArrayWithTakeValueFromXmlArray<CUSTOM_LEGACY_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class>
                              CustomLegacySettings = XObjArrayWithTakeValueFromXmlArray<CUSTOM_LEGACY_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class>();
//      XObjArray<CUSTOM_TOOL_ENTRY_SETTINGS>   CustomToolSettings = XObjArray<CUSTOM_TOOL_ENTRY_SETTINGS>();
    XObjArrayWithTakeValueFromXmlArray<CUSTOM_TOOL_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class>
                            CustomToolSettings = XObjArrayWithTakeValueFromXmlArray<CUSTOM_TOOL_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class>();

      bool getDarkEmbedded(bool isDaylight) const;
    
#if __cplusplus > 201703L
      bool operator == (const GUIClass&) const = default;
#endif
      bool isEqual(const GUIClass& other) const
      {
        if ( !(Timezone == other.Timezone) ) return false;
        if ( !(Theme == other.Theme) ) return false;
        if ( !(EmbeddedThemeType == other.EmbeddedThemeType) ) return false;
        if ( !(PlayAsync == other.PlayAsync) ) return false;
        if ( !(CustomIcons == other.CustomIcons) ) return false;
        if ( !(TextOnly == other.TextOnly) ) return false;
        if ( !(ShowOptimus == other.ShowOptimus) ) return false;
        if ( !(ScreenResolution == other.ScreenResolution) ) return false;
        if ( !(ProvideConsoleGop == other.ProvideConsoleGop) ) return false;
        if ( !(ConsoleMode == other.ConsoleMode) ) return false;
        if ( !(Language == other.Language) ) return false;
        if ( !(languageCode == other.languageCode) ) return false;
        if ( !(KbdPrevLang == other.KbdPrevLang) ) return false;
        if ( !(HVHideStrings == other.HVHideStrings) ) return false;
        if ( !Scan.isEqual(other.Scan) ) return false;
        if ( !Mouse.isEqual(other.Mouse) ) return false;
        if ( !CustomEntriesSettings.isEqual(other.CustomEntriesSettings) ) return false;
        if ( !CustomLegacySettings.isEqual(other.CustomLegacySettings) ) return false;
        if ( !CustomToolSettings.isEqual(other.CustomToolSettings) ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::GUI_Class& configPlist)
      {
        Timezone = configPlist.dgetTimezone();
        Theme = configPlist.dgetTheme();
        EmbeddedThemeType = configPlist.dgetEmbeddedThemeType();
        PlayAsync = configPlist.dgetPlayAsync();
        CustomIcons = configPlist.dgetCustomIcons();
        TextOnly = configPlist.dgetTextOnly();
        ShowOptimus = configPlist.dgetShowOptimus();
        ScreenResolution = configPlist.dgetScreenResolution();
        ProvideConsoleGop = configPlist.dgetProvideConsoleGop();
        ConsoleMode = configPlist.dgetConsoleMode();
        Language = configPlist.dgetLanguage();
        languageCode = configPlist.dgetlanguageCode();
        KbdPrevLang = configPlist.dgetKbdPrevLang();
        HVHideStrings = configPlist.dgetHVHideStrings();
        Scan.takeValueFrom(configPlist.Scan);
        Mouse.takeValueFrom(configPlist.Mouse);
        CustomEntriesSettings.takeValueFrom(configPlist.Custom.Entries);
        CustomLegacySettings.takeValueFrom(configPlist.Custom.Legacy);
        CustomToolSettings.takeValueFrom(configPlist.Custom.Tool);
      }

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
      
#if __cplusplus > 201703L
		bool operator == (const CPUClass&) const = default;
#endif
    bool isEqual(const CPUClass& other) const
    {
      if ( !(QPI == other.QPI) ) return false;
      if ( !(CpuFreqMHz == other.CpuFreqMHz) ) return false;
      if ( !(CpuType == other.CpuType) ) return false;
      if ( !(QEMU == other.QEMU) ) return false;
      if ( !(UseARTFreq == other.UseARTFreq) ) return false;
      if ( !(BusSpeed == other.BusSpeed) ) return false;
      if ( !(UserChange == other.UserChange) ) return false;
      if ( !(SavingMode == other.SavingMode) ) return false;
      if ( !(HWPEnable == other.HWPEnable) ) return false;
      if ( !(HWPValue == other.HWPValue) ) return false;
      if ( !(TDP == other.TDP) ) return false;
      if ( !(TurboDisabled == other.TurboDisabled) ) return false;
      if ( !(_EnableC6 == other._EnableC6) ) return false;
      if ( !(_EnableC4 == other._EnableC4) ) return false;
      if ( !(_EnableC2 == other._EnableC2) ) return false;
      if ( !(_C3Latency == other._C3Latency) ) return false;
      return true;
    }
    void takeValueFrom(const ConfigPlistClass::CPU_Class& configPlist)
    {
      QPI = configPlist.dgetQPI();
      CpuFreqMHz = configPlist.dgetCpuFreqMHz();
      CpuType = configPlist.dgetCpuType();
      QEMU = configPlist.dgetQEMU();
      UseARTFreq = configPlist.dgetUseARTFreq();
      BusSpeed = configPlist.dgetBusSpeed();
      UserChange = configPlist.dgetUserChange();
      SavingMode = configPlist.dgetSavingMode();
      HWPEnable = configPlist.dgetHWPEnable();
      HWPValue = configPlist.dgetHWPValue();
      TDP = configPlist.dgetTDP();
      TurboDisabled = configPlist.dgetTurboDisabled();
      _EnableC6 = configPlist.dget_EnableC6();
      _EnableC4 = configPlist.dget_EnableC4();
      _EnableC2 = configPlist.dget_EnableC2();
      _C3Latency = configPlist.dget_C3Latency();
    }
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
      
//      friend class ::SETTINGS_DATA;
//      friend unsigned long long ::GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& gSettings);
        
#if __cplusplus > 201703L
      bool operator == (const SystemParametersClass&) const = default;
#endif
      bool isEqual(const SystemParametersClass& other) const
      {
        if ( !(WithKexts == other.WithKexts) ) return false;
        if ( !(WithKextsIfNoFakeSMC == other.WithKextsIfNoFakeSMC) ) return false;
        if ( !(NoCaches == other.NoCaches) ) return false;
        if ( !(BacklightLevel == other.BacklightLevel) ) return false;
        if ( !(BacklightLevelConfig == other.BacklightLevelConfig) ) return false;
        if ( !(CustomUuid == other.CustomUuid) ) return false;
        if ( !(_InjectSystemID == other._InjectSystemID) ) return false;
        if ( !(NvidiaWeb == other.NvidiaWeb) ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::SystemParameters_Class& configPlist)
      {
        WithKexts = configPlist.dgetWithKexts();
        WithKextsIfNoFakeSMC = configPlist.dgetWithKextsIfNoFakeSMC();
        NoCaches = configPlist.dgetNoCaches();
        BacklightLevel = configPlist.dgetBacklightLevel();
        BacklightLevelConfig = configPlist.dgetBacklightLevelConfig();
        CustomUuid = configPlist.dgetCustomUuid();
        _InjectSystemID = configPlist.dget_InjectSystemID();
        NvidiaWeb = configPlist.dgetNvidiaWeb();
      }
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
          
#if __cplusplus > 201703L
          bool operator == (const EDIDClass&) const = default;
#endif
          bool isEqual(const EDIDClass& other) const
          {
            if ( !(InjectEDID == other.InjectEDID) ) return false;
            if ( !(CustomEDID == other.CustomEDID) ) return false;
            if ( !(VendorEDID == other.VendorEDID) ) return false;
            if ( !(ProductEDID == other.ProductEDID) ) return false;
            if ( !(EdidFixHorizontalSyncPulseWidth == other.EdidFixHorizontalSyncPulseWidth) ) return false;
            if ( !(EdidFixVideoInputSignal == other.EdidFixVideoInputSignal) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::Graphics_Class::Graphics_EDID_Class& configPlist)
          {
            InjectEDID = configPlist.dgetInjectEDID();
            CustomEDID = configPlist.dgetCustomEDID();
            VendorEDID = configPlist.dgetVendorEDID();
            ProductEDID = configPlist.dgetProductEDID();
            EdidFixHorizontalSyncPulseWidth = configPlist.dgetEdidFixHorizontalSyncPulseWidth();
            EdidFixVideoInputSignal = configPlist.dgetEdidFixVideoInputSignal();
          }
      };
      
      class InjectAsDictClass {
        public:
          bool GraphicsInjector = bool();
          bool InjectIntel = bool();
          bool InjectATI = bool();
          bool InjectNVidia = bool();
        
#if __cplusplus > 201703L
        bool operator == (const InjectAsDictClass&) const = default;
#endif
        bool isEqual(const InjectAsDictClass& other) const
        {
          if ( !(GraphicsInjector == other.GraphicsInjector) ) return false;
          if ( !(InjectIntel == other.InjectIntel) ) return false;
          if ( !(InjectATI == other.InjectATI) ) return false;
          if ( !(InjectNVidia == other.InjectNVidia) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::Graphics_Class::XmlInjectUnion& configPlist)
        {
          GraphicsInjector = configPlist.dgetGraphicsInjector();
          InjectIntel = configPlist.dgetInjectIntel();
          InjectATI = configPlist.dgetInjectATI();
          InjectNVidia = configPlist.dgetInjectNVidia();
        }
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
        
#if __cplusplus > 201703L
        bool operator == (const GRAPHIC_CARD&) const = default;
#endif
        bool isEqual(const GRAPHIC_CARD& other) const
        {
          if ( !(Signature == other.Signature) ) return false;
          if ( !(Model == other.Model) ) return false;
          if ( !(Id == other.Id) ) return false;
          if ( !(SubId == other.SubId) ) return false;
          if ( !(VideoRam == other.VideoRam) ) return false;
          if ( !(VideoPorts == other.VideoPorts) ) return false;
          if ( !(LoadVBios == other.LoadVBios) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class& configPlist)
        {
          Signature = configPlist.dgetSignature();
          Model = configPlist.dgetModel();
          Id = configPlist.dgetId();
          SubId = configPlist.dgetSubId();
          VideoRam = configPlist.dgetVideoRam();
          VideoPorts = configPlist.dgetVideoPorts();
          LoadVBios = configPlist.dgetLoadVBios();
        }
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
      XArray<UINT8>        Dcfg = XArray<UINT8>();
      XArray<UINT8>        NVCAP = XArray<UINT8>();
      INT8                 BootDisplay = INT8();
      UINT32               DualLink = UINT32();
      UINT32               _IgPlatform = UINT32(); //could also be snb-platform-id
      EDIDClass            EDID = EDIDClass();
      InjectAsDictClass    InjectAsDict = InjectAsDictClass();
      XObjArrayWithTakeValueFromXmlArray<GRAPHIC_CARD, ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class> ATICardList = XObjArrayWithTakeValueFromXmlArray<GRAPHIC_CARD, ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class>();
      XObjArrayWithTakeValueFromXmlArray<GRAPHIC_CARD, ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class> NVIDIACardList = XObjArrayWithTakeValueFromXmlArray<GRAPHIC_CARD, ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class>();

      GraphicsClass() {
        Dcfg.setSize(8);
        memset(Dcfg.data(), 0, 8);
        NVCAP.setSize(20);
        memset(NVCAP.data(), 0, 20);
      }
      
#if __cplusplus > 201703L
      bool operator == (const GraphicsClass&) const = default;
#endif
      bool isEqual(const GraphicsClass& other) const
      {
        if ( !(PatchVBios == other.PatchVBios) ) return false;
        if ( !PatchVBiosBytes.isEqual(other.PatchVBiosBytes) ) return false;
        if ( !(RadeonDeInit == other.RadeonDeInit) ) return false;
        if ( !(LoadVBios == other.LoadVBios) ) return false;
        if ( !(VRAM == other.VRAM) ) return false;
        if ( !(RefCLK == other.RefCLK) ) return false;
        if ( !(FBName == other.FBName) ) return false;
        if ( !(VideoPorts == other.VideoPorts) ) return false;
        if ( !(NvidiaGeneric == other.NvidiaGeneric) ) return false;
        if ( !(NvidiaNoEFI == other.NvidiaNoEFI) ) return false;
        if ( !(NvidiaSingle == other.NvidiaSingle) ) return false;
        if ( !(Dcfg == other.Dcfg) ) return false;
        if ( !(NVCAP == other.NVCAP) ) return false;
        if ( !(BootDisplay == other.BootDisplay) ) return false;
        if ( !(DualLink == other.DualLink) ) return false;
        if ( !(_IgPlatform == other._IgPlatform) ) return false;
        if ( !EDID.isEqual(other.EDID) ) return false;
        if ( !InjectAsDict.isEqual(other.InjectAsDict) ) return false;
        if ( !ATICardList.isEqual(other.ATICardList) ) return false;
        if ( !NVIDIACardList.isEqual(other.NVIDIACardList) ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::Graphics_Class& configPlist)
      {
        PatchVBios = configPlist.dgetPatchVBios();
        PatchVBiosBytes.takeValueFrom(configPlist.PatchVBiosBytesArray);
        RadeonDeInit = configPlist.dgetRadeonDeInit();
        LoadVBios = configPlist.dgetLoadVBios();
        VRAM = configPlist.dgetVRAM();
        RefCLK = configPlist.dgetRefCLK();
        FBName = configPlist.dgetFBName();
        VideoPorts = configPlist.dgetVideoPorts();
        NvidiaGeneric = configPlist.dgetNvidiaGeneric();
        NvidiaNoEFI = configPlist.dgetNvidiaNoEFI();
        NvidiaSingle = configPlist.dgetNvidiaSingle();
        Dcfg = configPlist.dgetDcfg();
        NVCAP = configPlist.dgetNVCAP();
        BootDisplay = configPlist.dgetBootDisplay();
        DualLink = configPlist.dgetDualLink();
        _IgPlatform = configPlist.dget_IgPlatform();
        EDID.takeValueFrom(configPlist.EDID);
        InjectAsDict.takeValueFrom(configPlist.Inject);
        ATICardList.takeValueFrom(configPlist.ATI);
        NVIDIACardList.takeValueFrom(configPlist.NVIDIA);
      }

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
          bool                    HDAInjection = bool();
          INT32                   HDALayoutId = INT32();
          bool                    AFGLowPowerState = bool();
        
#if __cplusplus > 201703L
        bool operator == (const AudioClass&) const = default;
#endif
        bool isEqual(const AudioClass& other) const
        {
          if ( !(ResetHDA == other.ResetHDA) ) return false;
          if ( !(HDAInjection == other.HDAInjection) ) return false;
          if ( !(HDALayoutId == other.HDALayoutId) ) return false;
          if ( !(AFGLowPowerState == other.AFGLowPowerState) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_Audio_Class& configPlist)
        {
          ResetHDA = configPlist.dgetResetHDA();
          HDAInjection = configPlist.dgetHDAInjection();
          HDALayoutId = configPlist.dgetHDALayoutId();
          AFGLowPowerState = configPlist.dgetAFGLowPowerState();
        }
      };
      class USBClass {
        public:
          bool                 USBInjection = bool();
          bool                 USBFixOwnership = bool();
          bool                 InjectClockID = bool();
          bool                 HighCurrent = bool();
          bool                 NameEH00 = bool();
          bool                 NameXH00 = bool(); // is it used?
        
#if __cplusplus > 201703L
        bool operator == (const USBClass&) const = default;
#endif
        bool isEqual(const USBClass& other) const
        {
          if ( !(USBInjection == other.USBInjection) ) return false;
          if ( !(USBFixOwnership == other.USBFixOwnership) ) return false;
          if ( !(InjectClockID == other.InjectClockID) ) return false;
          if ( !(HighCurrent == other.HighCurrent) ) return false;
          if ( !(NameEH00 == other.NameEH00) ) return false;
          if ( !(NameXH00 == other.NameXH00) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_USB_Class& configPlist)
        {
          USBInjection = configPlist.dgetUSBInjection();
          USBFixOwnership = configPlist.dgetUSBFixOwnership();
          InjectClockID = configPlist.dgetInjectClockID();
          HighCurrent = configPlist.dgetHighCurrent();
          NameEH00 = configPlist.dgetNameEH00();
          //NameXH00 = configPlist.dgetNameXH00();
        }
      };

      class AddPropertyClass
      {
      public:
        uint32_t                     Device = 0;
        XString8                     Key = XString8();
        XBuffer<uint8_t>             Value = XBuffer<uint8_t>();
        TAG_TYPE                     ValueType = kTagTypeNone;
        INPUT_ITEM                   MenuItem = INPUT_ITEM();
//        XString8                     DevicePathAsString = XString8();
//        XString8                     Label = XString8();
        
        AddPropertyClass() {}

#if __cplusplus > 201703L
        bool operator == (const AddPropertyClass&) const = default;
#endif
        bool isEqual(const AddPropertyClass& other) const
        {
          if ( !(Device == other.Device) ) return false;
          if ( !(Key == other.Key) ) return false;
          if ( !(Value == other.Value) ) return false;
          if ( !(ValueType == other.ValueType) ) return false;
          if ( !(MenuItem == other.MenuItem) ) return false;
//          if ( !(DevicePathAsString == other.DevicePathAsString) ) return false;
//          if ( !(Label == other.Label) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class& configPlist)
        {
          Device = configPlist.dgetDevice();
          Key = configPlist.dgetKey();
          Value = configPlist.dgetValue();
          ValueType = configPlist.dgetValueType();
          MenuItem.BValue = !configPlist.dgetDisabled();
//          DevicePathAsString = configPlist.dgetDevicePathAsString();
//          Label = configPlist.dgetLabel();
        }
      };

      // This is shared by PropertiesClass and ArbitraryClass
      class SimplePropertyClass
      {
      public:
        XString8                     Key = XString8();
        XBuffer<uint8_t>             Value = XBuffer<uint8_t>();
        TAG_TYPE                     ValueType = kTagTypeNone; // only used in CreateMenuProps()
        INPUT_ITEM                   MenuItem = INPUT_ITEM();  // Will get the Disabled value
        
        SimplePropertyClass() {}

#if __cplusplus > 201703L
        bool operator == (const SimplePropertyClass&) const = default;
#endif
        bool isEqual(const SimplePropertyClass& other) const
        {
          if ( !(Key == other.Key) ) return false;
          if ( !(Value == other.Value) ) return false;
          if ( !(ValueType == other.ValueType) ) return false;
          if ( !(MenuItem == other.MenuItem) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::SimplePropertyClass_Class& configPlist)
        {
          Key = configPlist.dgetKey();
          Value = configPlist.dgetValue();
          ValueType = configPlist.dgetValueType();
          MenuItem.BValue = !configPlist.dgetDisabled();
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::PropertiesUnion::Property& configPlist)
        {
          Key = configPlist.dgetKey();
          Value = configPlist.dgetValue();
          ValueType = configPlist.dgetValueType();
          MenuItem.BValue = configPlist.dgetBValue();
        }
      };

      // Property don't have Device. Before it was always Device = 0 to differentiate from Arbitrary properties.
      class PropertiesClass {
        public:

          class PropertyClass
          {
          public:
            
            bool                            Enabled = true;
            XStringW                        DevicePathAsString = XStringW();
            // XString8                     Label = XString8(); // Label is the same as DevicePathAsString, so it's not needed.
            XObjArrayWithTakeValueFromXmlRepeatingDict<SimplePropertyClass, ConfigPlistClass::DevicesClass::PropertiesUnion::Property> propertiesArray = XObjArrayWithTakeValueFromXmlRepeatingDict<SimplePropertyClass, ConfigPlistClass::DevicesClass::PropertiesUnion::Property>();

            PropertyClass() {}

#if !defined(DONT_DEFINE_GLOBALS)
            EFI_DEVICE_PATH_PROTOCOL* getDevicePath() const
            {
              EFI_DEVICE_PATH_PROTOCOL* DevicePath;
              if ( DevicePathAsString.isEqualIC("PrimaryGPU") && gConf.GfxPropertiesArray.size() > 0 ) {
                DevicePath = DevicePathFromHandle(gConf.GfxPropertiesArray[0].Handle); // first gpu
              } else if ( DevicePathAsString.isEqualIC("SecondaryGPU") && gConf.GfxPropertiesArray.size() > 1 ) {
                DevicePath = DevicePathFromHandle(gConf.GfxPropertiesArray[1].Handle); // second gpu
              } else {
                DevicePath = ConvertTextToDevicePath(DevicePathAsString.wc_str()); //TODO
              }
              return DevicePath;
            }
#endif

#if __cplusplus > 201703L
            bool operator == (const PropertyClass&) const = default;
#endif
            bool isEqual(const PropertyClass& other) const
            {
              if ( !(Enabled == other.Enabled) ) return false;
              if ( !(DevicePathAsString == other.DevicePathAsString) ) return false;
//              if ( !(Label == other.Label) ) return false;
              if ( !propertiesArray.isEqual(other.propertiesArray) ) return false;
              return true;
            }
            void takeValueFrom(const ConfigPlistClass::DevicesClass::PropertiesUnion::Properties4DeviceClass& configPlist)
            {
              Enabled = configPlist.dgetEnabled();
              DevicePathAsString = configPlist.dgetDevicePathAsString();
//              Label = configPlist.dgetLabel();
              propertiesArray.takeValueFrom(configPlist);
            }
          };

          XString8 propertiesAsString = XString8();
          XObjArrayWithTakeValueFromXmlRepeatingDict<PropertyClass, ConfigPlistClass::DevicesClass::PropertiesUnion::Properties4DeviceClass> PropertyArray = XObjArrayWithTakeValueFromXmlRepeatingDict<PropertyClass, ConfigPlistClass::DevicesClass::PropertiesUnion::Properties4DeviceClass>();
        
#if __cplusplus > 201703L
          bool operator == (const PropertiesClass&) const = default;
#endif
          bool isEqual(const PropertiesClass& other) const
          {
            if ( !(propertiesAsString == other.propertiesAsString) ) return false;
            if ( !PropertyArray.isEqual(other.PropertyArray) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::DevicesClass::PropertiesUnion& configPlist)
          {
            propertiesAsString = configPlist.dgetpropertiesAsString();
            PropertyArray.takeValueFrom(configPlist.PropertiesAsDict);
          }
      };

      class ArbitraryPropertyClass {
        public:
          uint32_t                     Device = 0;
          XString8                     Label = XString8();
        XObjArrayWithTakeValueFromXmlArray<SimplePropertyClass, ConfigPlistClass::DevicesClass::SimplePropertyClass_Class>   CustomPropertyArray = XObjArrayWithTakeValueFromXmlArray<SimplePropertyClass, ConfigPlistClass::DevicesClass::SimplePropertyClass_Class>();
        
          ArbitraryPropertyClass() {}
#if __cplusplus > 201703L
          bool operator == (const ArbitraryPropertyClass&) const = default;
#endif
          bool isEqual(const ArbitraryPropertyClass& other) const
          {
            if ( !(Device == other.Device) ) return false;
            if ( !(Label == other.Label) ) return false;
            if ( !CustomPropertyArray.isEqual(other.CustomPropertyArray) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class& configPlist)
          {
            Device = configPlist.dgetDevice();
            Label = configPlist.dgetLabel();
            CustomPropertyArray.takeValueFrom(configPlist.CustomProperties);
          }
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
        
#if __cplusplus > 201703L
        bool operator == (const FakeIDClass&) const = default;
#endif
        bool isEqual(const FakeIDClass& other) const
        {
          if ( !(FakeATI == other.FakeATI) ) return false;
          if ( !(FakeNVidia == other.FakeNVidia) ) return false;
          if ( !(FakeIntel == other.FakeIntel) ) return false;
          if ( !(FakeLAN == other.FakeLAN) ) return false;
          if ( !(FakeWIFI == other.FakeWIFI) ) return false;
          if ( !(FakeSATA == other.FakeSATA) ) return false;
          if ( !(FakeXHCI == other.FakeXHCI) ) return false;
          if ( !(FakeIMEI == other.FakeIMEI) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_FakeID_Class& configPlist)
        {
          FakeATI = configPlist.dgetFakeATI();
          FakeNVidia = configPlist.dgetFakeNVidia();
          FakeIntel = configPlist.dgetFakeIntel();
          FakeLAN = configPlist.dgetFakeLAN();
          FakeWIFI = configPlist.dgetFakeWIFI();
          FakeSATA = configPlist.dgetFakeSATA();
          FakeXHCI = configPlist.dgetFakeXHCI();
          FakeIMEI = configPlist.dgetFakeIMEI();
        }
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
      FakeIDClass          FakeID = FakeIDClass();
      
      XObjArrayWithTakeValueFromXmlArray<AddPropertyClass, ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class> AddPropertyArray = XObjArrayWithTakeValueFromXmlArray<AddPropertyClass, ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class>();
      PropertiesClass Properties = PropertiesClass();
      XObjArrayWithTakeValueFromXmlArray<ArbitraryPropertyClass, ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class> ArbitraryArray = XObjArrayWithTakeValueFromXmlArray<ArbitraryPropertyClass, ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class>();

    
#if __cplusplus > 201703L
		bool operator == (const DevicesClass&) const = default;
#endif
    bool isEqual(const DevicesClass& other) const
    {
      if ( !(StringInjector == other.StringInjector) ) return false;
      if ( !(IntelMaxBacklight == other.IntelMaxBacklight) ) return false;
      if ( !(IntelBacklight == other.IntelBacklight) ) return false;
      if ( !(IntelMaxValue == other.IntelMaxValue) ) return false;
      if ( !(LANInjection == other.LANInjection) ) return false;
      if ( !(HDMIInjection == other.HDMIInjection) ) return false;
      if ( !(NoDefaultProperties == other.NoDefaultProperties) ) return false;
      if ( !(UseIntelHDMI == other.UseIntelHDMI) ) return false;
      if ( !(ForceHPET == other.ForceHPET) ) return false;
      if ( !(DisableFunctions == other.DisableFunctions) ) return false;
      if ( !(AirportBridgeDeviceName == other.AirportBridgeDeviceName) ) return false;
      if ( !Audio.isEqual(other.Audio) ) return false;
      if ( !USB.isEqual(other.USB) ) return false;
      if ( !FakeID.isEqual(other.FakeID) ) return false;
      if ( !AddPropertyArray.isEqual(other.AddPropertyArray) ) return false;
      if ( !Properties.isEqual(other.Properties) ) return false;
      if ( !ArbitraryArray.isEqual(other.ArbitraryArray) ) return false;
      return true;
    }
    void takeValueFrom(const ConfigPlistClass::DevicesClass& configPlist)
    {
      StringInjector = configPlist.dgetStringInjector();
      IntelMaxBacklight = configPlist.dgetIntelMaxBacklight();
      IntelBacklight = configPlist.dgetIntelBacklight();
      IntelMaxValue = configPlist.dgetIntelMaxValue();
      LANInjection = configPlist.dgetLANInjection();
      HDMIInjection = configPlist.dgetHDMIInjection();
      NoDefaultProperties = configPlist.dgetNoDefaultProperties();
      UseIntelHDMI = configPlist.dgetUseIntelHDMI();
      ForceHPET = configPlist.dgetForceHPET();
      DisableFunctions = configPlist.dgetDisableFunctions();
      AirportBridgeDeviceName = configPlist.dgetAirportBridgeDeviceName();
      Audio.takeValueFrom(configPlist.Audio);
      USB.takeValueFrom(configPlist.USB);
      FakeID.takeValueFrom(configPlist.FakeID);
      AddPropertyArray.takeValueFrom(configPlist.AddProperties);
      Properties.takeValueFrom(configPlist.Properties);
      ArbitraryArray.takeValueFrom(configPlist.Arbitrary);
    }

      // 2021-04 : Following is temporary to compare with old way of storing properties.
      // Let's keep it few months until I am sure the refactoring isomorphic
    private:
      DEV_PROPERTY                *ArbProperties = 0; // the old version.
    public:
      void FillDevicePropertiesOld(SETTINGS_DATA& gSettings, const TagDict* DevicesDict);
      
      bool compareDevProperty(const XString8& label, const DEV_PROPERTY& oldProp, const DEV_PROPERTY& newProp) const
      {
        if ( newProp.Device != oldProp.Device )
        {
          printf("%s.Device !=\n", label.c_str());
          return false;
        }
        if ( newProp.Key || oldProp.Key ) {
          if ( !newProp.Key || !oldProp.Key ) {
            printf("%s.Key !=\n", label.c_str());
            return false;
          }
#ifdef JIEF_DEBUG
if ( strcmp(oldProp.Key, "10") == 0 ) {
printf("%s", "");
}
#endif
          if ( strcmp(newProp.Key, oldProp.Key) != 0 )
          {
            printf("%s.Key !=\n", label.c_str());
            return false;
          }
        }
        if ( newProp.ValueLen != oldProp.ValueLen )
        {
          printf("%s.Value.ValueLen %lld != %lld\n", label.c_str(), oldProp.ValueLen, newProp.ValueLen);
          return false;
        } else
        {
          if ( newProp.ValueLen > 0 ) {
            if ( memcmp(newProp.Value, oldProp.Value, oldProp.ValueLen) != 0 )
            {
              printf("%s.Value !=\n", label.c_str());
              return false;
            }
          }
        }
        if ( newProp.ValueType != oldProp.ValueType )
        {
          printf("%s.ValueType !=\n", label.c_str());
          return false;
        }
        if ( newProp.Label || oldProp.Label ) {
          if ( !newProp.Label || !oldProp.Label ) {
            printf("%s.Label !=\n", label.c_str());
            return false;
          }
          if ( strcmp(newProp.Label, oldProp.Label) != 0 )
          {
            printf("%s.Label != : old:%s new:%s\n", label.c_str(), oldProp.Label, newProp.Label);
            return false;
          }
        }
        if ( newProp.MenuItem.BValue != oldProp.MenuItem.BValue )
        {
          printf("%s.MenuItem.BValue !=\n", label.c_str());
          return false;
        }
        DEV_PROPERTY *oldChild = oldProp.Child;
        DEV_PROPERTY *newChild = newProp.Child;
        size_t jdx = 0;
        while ( oldChild && newChild )
        {
          compareDevProperty(S8Printf("%s.child[%zu]", label.c_str(), jdx), *oldChild, *newChild);
          oldChild = oldChild->Next;
          newChild = newChild->Next;
          jdx++;
        }
        if ( oldChild != newChild )
        {
          printf("%s.Child count\n", label.c_str());
          return false;
        }
        return true;
      }
      
      bool compareOldAndCompatibleArb()
      {
//        {
//          size_t oldArbIdx = 0;
//          const DEV_PROPERTY* arb = ArbProperties;
//          while ( arb ) {
//            printf("ArbProperties[%zu].Key = %s\n", oldArbIdx++, arb->Key);
//            arb = arb->Next;
//          }
//        }
        const XObjArray<DEV_PROPERTY> compatibleArbProperties = getCompatibleArbProperty();
        size_t oldArbIdx = 0;
        for ( size_t idx = 0 ; idx < compatibleArbProperties.size() ; ++idx )
        {
          if ( ArbProperties == NULL ) {
            printf("ArbProperties.size < compatibleArbProperties.size()");
            return false;
          }
          if ( !compareDevProperty(S8Printf("ArbProperties[%zu]", idx), *ArbProperties, compatibleArbProperties[idx]) ) {
            return false;
          }
          ++oldArbIdx;
          ArbProperties = ArbProperties->Next;
        }
        if ( ArbProperties != NULL ) {
          printf("ArbProperties.size > compatibleArbProperties.size()");
          return false;
        }
        return true;
      }

    public:
      XObjArray<DEV_PROPERTY> getCompatibleArbProperty() const
      {
        XObjArray<DEV_PROPERTY> arb;
        for ( size_t idx = ArbitraryArray.size() ; idx-- > 0  ; ) {
          const ArbitraryPropertyClass& newArb = ArbitraryArray[idx];
          for ( size_t jdx = newArb.CustomPropertyArray.size() ; jdx-- > 0 ;  ) {
            const SimplePropertyClass& newArbProp = newArb.CustomPropertyArray[jdx];
            DEV_PROPERTY* newProp = new DEV_PROPERTY;
            newProp->Device = newArb.Device;
            newProp->Key = const_cast<char*>(newArbProp.Key.c_str()); // const_cast !!! So ugly. It is just because it's temporary. If ArbProperties is modified after this, a lot a memory problem will happen. I could have done some strdup, but that way I don't use memory and don't have to free it.
            newProp->Value = const_cast<unsigned char*>(newArbProp.Value.data());
            newProp->ValueLen = newArbProp.Value.size();
            newProp->ValueType = newArbProp.ValueType;
            newProp->MenuItem.BValue = newArbProp.MenuItem.BValue ;
            newProp->Label = const_cast<char*>(newArb.Label.c_str());
            arb.AddReference(newProp, true);
//            printf("Add prop key=%s label=%s at index %zu\n", newProp->Key.c_str(), newProp->Label.c_str(), arb.size()-1);
          }
        }
        // Non arb : device = 0
        for ( size_t idx = Properties.PropertyArray.size() ; idx-- > 0 ;  ) {
          const PropertiesClass::PropertyClass& Prop = Properties.PropertyArray[idx];
          DEV_PROPERTY* newProp = new DEV_PROPERTY;
          newProp->Device = 0;
          newProp->Key = 0;
          if ( Prop.Enabled ) newProp->Label = XString8(Prop.DevicePathAsString).forgetDataWithoutFreeing();
          else newProp->Label = S8Printf("!%ls", Prop.DevicePathAsString.wc_str()).forgetDataWithoutFreeing();
          newProp->Child = NULL;
          for ( size_t jdx = Properties.PropertyArray[idx].propertiesArray.size() ; jdx-- > 0  ; ) {
            const SimplePropertyClass& SubProp = Prop.propertiesArray[jdx];
            DEV_PROPERTY* newSubProp = new DEV_PROPERTY;
            newSubProp->Device = 0;
            newSubProp->Key = const_cast<char*>(SubProp.Key.c_str());
//            newSubProp->Key = NULL;
            newSubProp->Value = const_cast<unsigned char*>(SubProp.Value.data());
            newSubProp->ValueLen = SubProp.Value.size();
            newSubProp->ValueType = SubProp.ValueType;
            newSubProp->MenuItem.BValue = true;
            if ( newProp->Child ) {
              DEV_PROPERTY* childs;
              for ( childs = newProp->Child ; childs->Next ; childs = childs->Next );
              childs->Next = newSubProp;
            }else{
              newProp->Child = newSubProp;
            }
          }
          arb.AddReference(newProp, true);
//          MsgLog("Add prop %s at index %zu\n", newProp->Label, arb.size()-1);
        }
        return arb;
      };
  };

  class QuirksClass {
    public:
      class MMIOWhiteList
      {
        public :
          UINTN        address = 0;
          XString8     comment = XString8();
          bool         enabled = 0;
          
	#if __cplusplus > 201703L
          bool operator == (const MMIOWhiteList&) const = default;
	#endif
          bool isEqual(const MMIOWhiteList& other) const
          {
            if ( !(address == other.address) ) return false;
            if ( !(comment == other.comment) ) return false;
            if ( !(enabled == other.enabled) ) return false;
            return true;
          }
        void takeValueFrom(const ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class& configPlist)
          {
            address = configPlist.dgetaddress();
            comment = configPlist.dgetcomment();
            enabled = configPlist.dgetenabled();
          }
      };
      class OcKernelQuirksClass
      {
        public:
        //  bool AppleCpuPmCfgLock = false;
        //  bool AppleXcpmCfgLock = false;
          bool AppleXcpmExtraMsrs = false;
          bool AppleXcpmForceBoost = false;
        //  bool CustomSmbiosGuid = false;
          bool DisableIoMapper = false;
          bool DisableLinkeditJettison = false;
        //  bool DisableRtcChecksum = false;
          bool DummyPowerManagement = false;
          bool ExternalDiskIcons = false;
          bool IncreasePciBarSize = false;
        //  bool LapicKernelPanic = false;
        //  bool PanicNoKextDump = false;
          bool PowerTimeoutKernelPanic = false;
          bool ThirdPartyDrives = false;
          bool XhciPortLimit = false;
          
#if __cplusplus > 201703L
          bool operator == (const OcKernelQuirksClass&) const = default;
#endif
          bool isEqual(const OcKernelQuirksClass& other) const
          {
            if ( !(AppleXcpmExtraMsrs == other.AppleXcpmExtraMsrs) ) return false;
            if ( !(AppleXcpmForceBoost == other.AppleXcpmForceBoost) ) return false;
            if ( !(DisableIoMapper == other.DisableIoMapper) ) return false;
            if ( !(DisableLinkeditJettison == other.DisableLinkeditJettison) ) return false;
            if ( !(DummyPowerManagement == other.DummyPowerManagement) ) return false;
            if ( !(ExternalDiskIcons == other.ExternalDiskIcons) ) return false;
            if ( !(IncreasePciBarSize == other.IncreasePciBarSize) ) return false;
            if ( !(PowerTimeoutKernelPanic == other.PowerTimeoutKernelPanic) ) return false;
            if ( !(ThirdPartyDrives == other.ThirdPartyDrives) ) return false;
            if ( !(XhciPortLimit == other.XhciPortLimit) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::Quirks_Class::OcKernelQuirks_Class& configPlist)
          {
            AppleXcpmExtraMsrs = configPlist.dgetAppleXcpmExtraMsrs();
            AppleXcpmForceBoost = configPlist.dgetAppleXcpmForceBoost();
            DisableIoMapper = configPlist.dgetDisableIoMapper();
            DisableLinkeditJettison = configPlist.dgetDisableLinkeditJettison();
            DummyPowerManagement = configPlist.dgetDummyPowerManagement();
            ExternalDiskIcons = configPlist.dgetExternalDiskIcons();
            IncreasePciBarSize = configPlist.dgetIncreasePciBarSize();
            PowerTimeoutKernelPanic = configPlist.dgetPowerTimeoutKernelPanic();
            ThirdPartyDrives = configPlist.dgetThirdPartyDrives();
            XhciPortLimit = configPlist.dgetXhciPortLimit();
          }
      };
    
      class OcBooterQuirksClass
      {
       public:
        bool AvoidRuntimeDefrag = false;
        bool DevirtualiseMmio = false;
        bool DisableSingleUser = false;
        bool DisableVariableWrite = false;
        bool DiscardHibernateMap = false;
        bool EnableSafeModeSlide = false;
        bool EnableWriteUnprotector = false;
        bool ForceExitBootServices = false;
        bool ProtectMemoryRegions = false;
        bool ProtectSecureBoot = false;
        bool ProtectUefiServices = false;
        bool ProvideCustomSlide = false;
        uint8_t ProvideMaxSlide = false;
        bool RebuildAppleMemoryMap = false;
        bool SetupVirtualMap = false;
        bool SignalAppleOS = false;
        bool SyncRuntimePermissions = false;
        
#if __cplusplus > 201703L
        bool operator == (const OcBooterQuirksClass&) const = default;
#endif
        bool isEqual(const OcBooterQuirksClass& other) const
        {
          if ( !(AvoidRuntimeDefrag == other.AvoidRuntimeDefrag) ) return false;
          if ( !(DevirtualiseMmio == other.DevirtualiseMmio) ) return false;
          if ( !(DisableSingleUser == other.DisableSingleUser) ) return false;
          if ( !(DisableVariableWrite == other.DisableVariableWrite) ) return false;
          if ( !(DiscardHibernateMap == other.DiscardHibernateMap) ) return false;
          if ( !(EnableSafeModeSlide == other.EnableSafeModeSlide) ) return false;
          if ( !(EnableWriteUnprotector == other.EnableWriteUnprotector) ) return false;
          if ( !(ForceExitBootServices == other.ForceExitBootServices) ) return false;
          if ( !(ProtectSecureBoot == other.ProtectSecureBoot) ) return false;
          if ( !(ProtectUefiServices == other.ProtectUefiServices) ) return false;
          if ( !(ProtectUefiServices == other.ProtectUefiServices) ) return false;
          if ( !(ProvideCustomSlide == other.ProvideCustomSlide) ) return false;
          if ( !(ProvideMaxSlide == other.ProvideMaxSlide) ) return false;
          if ( !(RebuildAppleMemoryMap == other.RebuildAppleMemoryMap) ) return false;
          if ( !(SetupVirtualMap == other.SetupVirtualMap) ) return false;
          if ( !(SignalAppleOS == other.SignalAppleOS) ) return false;
          if ( !(SyncRuntimePermissions == other.SyncRuntimePermissions) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::Quirks_Class::OcBooterQuirks_Class& configPlist)
        {
          AvoidRuntimeDefrag = configPlist.dgetAvoidRuntimeDefrag();
          DevirtualiseMmio = configPlist.dgetDevirtualiseMmio();
          DisableSingleUser = configPlist.dgetDisableSingleUser();
          DisableVariableWrite = configPlist.dgetDisableVariableWrite();
          DiscardHibernateMap = configPlist.dgetDiscardHibernateMap();
          EnableSafeModeSlide = configPlist.dgetEnableSafeModeSlide();
          EnableWriteUnprotector = configPlist.dgetEnableWriteUnprotector();
          ForceExitBootServices = configPlist.dgetForceExitBootServices();
          ProtectSecureBoot = configPlist.dgetProtectSecureBoot();
          ProtectUefiServices = configPlist.dgetProtectUefiServices();
          ProtectUefiServices = configPlist.dgetProtectUefiServices();
          ProvideCustomSlide = configPlist.dgetProvideCustomSlide();
          ProvideMaxSlide = configPlist.dgetProvideMaxSlide();
          RebuildAppleMemoryMap = configPlist.dgetRebuildAppleMemoryMap();
          SetupVirtualMap = configPlist.dgetSetupVirtualMap();
          SignalAppleOS = configPlist.dgetSignalAppleOS();
          SyncRuntimePermissions = configPlist.dgetSyncRuntimePermissions();
        }

      };
      
      bool                     FuzzyMatch = bool();
      XString8                 OcKernelCache = XString8();
//      UINTN MaxSlide;
      OcKernelQuirksClass         OcKernelQuirks = OcKernelQuirksClass();
      OcBooterQuirksClass         OcBooterQuirks = OcBooterQuirksClass();
      XObjArrayWithTakeValueFromXmlArray<MMIOWhiteList, ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class> mmioWhiteListArray = XObjArrayWithTakeValueFromXmlArray<MMIOWhiteList, ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class>();
      UINT32                   QuirksMask = 0;
    
#if __cplusplus > 201703L
      bool operator == (const QuirksClass&) const = default;
#endif
      bool isEqual(const QuirksClass& other) const
      {
        if ( !(FuzzyMatch == other.FuzzyMatch) ) return false;
        if ( !(OcKernelCache == other.OcKernelCache) ) return false;
        if ( !(OcKernelQuirks.isEqual(other.OcKernelQuirks)) ) return false;
        if ( !(OcBooterQuirks.isEqual(other.OcBooterQuirks)) ) return false;
        if ( !mmioWhiteListArray.isEqual(other.mmioWhiteListArray) ) return false;
        if ( !(QuirksMask == other.QuirksMask) ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::Quirks_Class& configPlist)
      {
        FuzzyMatch = configPlist.dgetFuzzyMatch();
        OcKernelCache = configPlist.dgetOcKernelCache();
        OcKernelQuirks.takeValueFrom(configPlist.OcKernelQuirks);
        OcBooterQuirks.takeValueFrom(configPlist.OcBooterQuirks);
        mmioWhiteListArray.takeValueFrom(configPlist.MmioWhitelist);
        QuirksMask = configPlist.dgetQuirksMask();
      }
};

  class RtVariablesClass {
    public:
      class RT_VARIABLES
      {
        public:
          bool     Disabled = bool();
          XString8 Comment = XStringW();
          XStringW Name = XStringW();
          EFI_GUIDClass Guid = EFI_GUIDClass();

#if __cplusplus > 201703L
          bool operator == (const RT_VARIABLES&) const = default;
#endif
          bool isEqual(const RT_VARIABLES& other) const
          {
            if ( !(Disabled == other.Disabled) ) return false;
            if ( !(Comment == other.Comment) ) return false;
            if ( !(Name == other.Name) ) return false;
            if ( memcmp(&Guid, &other.Guid, sizeof(Guid)) != 0 ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block& configPlist)
          {
            Disabled = configPlist.dgetDisabled();
            Comment = configPlist.dgetComment();
            Name = configPlist.dgetName();
            Guid = configPlist.dgetGuid();
          }
      };
        
      XString8                RtROMAsString = XString8();
      XBuffer<UINT8>          RtROMAsData = XBuffer<UINT8>();
      XString8                RtMLBSetting = XString8();
      UINT32                  CsrActiveConfig = UINT32();
      UINT16                  BooterConfig = UINT16();
      XString8                BooterCfgStr = XString8();
    XObjArrayWithTakeValueFromXmlArray<RT_VARIABLES, ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block> BlockRtVariableArray = XObjArrayWithTakeValueFromXmlArray<RT_VARIABLES, ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block>();

      bool GetLegacyLanAddress() const {
        return RtROMAsString.isEqualIC("UseMacAddr0") || RtROMAsString.isEqualIC("UseMacAddr1");
      }
    
#if __cplusplus > 201703L
		bool operator == (const RtVariablesClass&) const = default;
#endif
    bool isEqual(const RtVariablesClass& other) const
    {
      if ( !(RtROMAsString == other.RtROMAsString) ) return false;
      if ( !(RtROMAsData == other.RtROMAsData) ) return false;
      if ( !(RtMLBSetting == other.RtMLBSetting) ) return false;
      if ( !(CsrActiveConfig == other.CsrActiveConfig) ) return false;
      if ( !(BooterConfig == other.BooterConfig) ) return false;
      if ( !(BooterCfgStr == other.BooterCfgStr) ) return false;
      if ( !BlockRtVariableArray.isEqual(other.BlockRtVariableArray) ) return false;
      return true;
    }
    void takeValueFrom(const ConfigPlistClass::RtVariables_Class& configPlist)
    {
      RtROMAsString = configPlist.dgetRtROMAsString();
      RtROMAsData = configPlist.dgetRtROMAsData();
      RtMLBSetting = configPlist.dgetRtMLBSetting();
      CsrActiveConfig = configPlist.dgetCsrActiveConfig();
      BooterConfig = configPlist.dgetBooterConfig();
      BooterCfgStr = configPlist.dgetBooterCfgStr();
      BlockRtVariableArray.takeValueFrom(configPlist.Block);
    }

  };

  class SmbiosClass {
    public:



        class SlotDeviceClass
        {
            public:
              static const SlotDeviceClass NullSlotDevice;
            public:
              uint8_t           SmbiosIndex = 0xff;
              UINT8             SlotID = UINT8();
              MISC_SLOT_TYPE    SlotType = MISC_SLOT_TYPE();
              XString8          SlotName = XString8();

              SlotDeviceClass() {}

#if __cplusplus > 201703L
              bool operator == (const SLOT_DEVICE&) const = default;
#endif
              bool isEqual(const SlotDeviceClass& other) const
              {
                if ( !(SmbiosIndex == other.SmbiosIndex) ) return false;
                if ( !(SlotID == other.SlotID) ) return false;
                if ( !(SlotType == other.SlotType) ) return false;
                if ( !(SlotName == other.SlotName) ) return false;
                return true;
              }
              void takeValueFrom(const SmbiosPlistClass::SmbiosDictClass::SlotDeviceDictClass& configPlist)
              {
                SmbiosIndex = configPlist.dgetDeviceN();
                SlotID = configPlist.dgetSlotID();
                SlotType = configPlist.dgetSlotType();
                SlotName = configPlist.dgetSlotName();
              }
        };
        
        class SlotDeviceArrayClass : public XObjArrayWithTakeValueFromXmlArray<SlotDeviceClass, SmbiosPlistClass::SmbiosDictClass::SlotDeviceDictClass>
        {
          public:
            bool doesSlotForIndexExist(uint8_t idx2Look4) const {
              for ( size_t idx = 0 ; idx < size() ; ++idx ) {
                if ( ElementAt(idx).SmbiosIndex == idx2Look4 ) return true;
              }
              return false;
            }
            const SlotDeviceClass& getSlotForIndex(uint8_t idx2Look4) const {
              for ( size_t idx = 0 ; idx < size() ; ++idx ) {
                if ( ElementAt(idx).SmbiosIndex == idx2Look4 ) return ElementAt(idx);
              }
              log_technical_bug("%s : no idx==%hhd", __PRETTY_FUNCTION__, idx2Look4);
              return SlotDeviceClass::NullSlotDevice;
            }
            const SlotDeviceClass& getSlotForIndexOrNull(uint8_t idx2Look4) const {
              for ( size_t idx = 0 ; idx < size() ; ++idx ) {
                if ( ElementAt(idx).SmbiosIndex == idx2Look4 ) return ElementAt(idx);
              }
              return SlotDeviceClass::NullSlotDevice;
            }
        };

        class RamSlotInfo {
        public:
          UINT64  Slot = UINT64();
          UINT32  ModuleSize = UINT32();
          UINT32  Frequency = UINT32();
          XString8 Vendor = XString8();
          XString8 PartNo = XString8();
          XString8 SerialNo = XString8();
          UINT8   Type = UINT8();
          bool  InUse = bool();

          RamSlotInfo() {}

          #if __cplusplus > 201703L
            bool operator == (const RamSlotInfo&) const = default;
          #endif
          bool isEqual(const RamSlotInfo& other) const
          {
            if ( !(Slot == other.Slot ) ) return false;
            if ( !(ModuleSize == other.ModuleSize ) ) return false;
            if ( !(Frequency == other.Frequency ) ) return false;
            if ( !(Vendor == other.Vendor ) ) return false;
            if ( !(PartNo == other.PartNo ) ) return false;
            if ( !(SerialNo == other.SerialNo ) ) return false;
            if ( !(Type == other.Type ) ) return false;
            if ( !(InUse == other.InUse ) ) return false;
            return true;
          }
          bool takeValueFrom(const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleDictClass& other)
          {
            Slot = other.dgetSlotNo();
            ModuleSize = other.dgetModuleSize();
            Frequency = other.dgetFrequency();
            Vendor = other.dgetVendor();
            PartNo = other.dgetPartNo();
            SerialNo = other.dgetSerialNo();
            Type = other.dgetType();
            InUse = other.dgetInUse();
            return true;
          }
        };

        class RamSlotInfoArrayClass {
          public:
            UINT8         SlotCounts = UINT8();
            UINT8         UserChannels = UINT8();
            XObjArrayWithTakeValueFromXmlArray<RamSlotInfo, SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleDictClass> User = XObjArrayWithTakeValueFromXmlArray<RamSlotInfo, SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleDictClass>();

            RamSlotInfoArrayClass() {}

#if __cplusplus > 201703L
            bool operator == (const RamSlotInfoArrayClass&) const = default;
#endif
            bool isEqual(const RamSlotInfoArrayClass& other) const
            {
              if ( !(SlotCounts == other.SlotCounts) ) return false;
              if ( !(UserChannels == other.UserChannels) ) return false;
              if ( !(User.isEqual(other.User)) ) return false;
              return true;
            }
            void takeValueFrom(const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass& configPlist)
            {
              SlotCounts = configPlist.dgetSlotCounts();
              UserChannels = configPlist.dgetUserChannels();
              User.takeValueFrom(configPlist.Modules);
            }
        };



      // SMBIOS TYPE0
      XString8                BiosVendor = XString8();
      XString8                BiosVersion = XString8();
      XString8                EfiVersion = XString8();
      XString8                BiosReleaseDate = XString8();
      // SMBIOS TYPE1
      XString8                ManufactureName = XString8();
      XString8                ProductName = XString8();
      XString8                SystemVersion = XString8();
      XString8                SerialNr = XString8();
      XString8                SmUUID = XString8();
      XString8                FamilyName = XString8();
      // SMBIOS TYPE2
      XString8                BoardManufactureName = XString8();
      XString8                BoardSerialNumber = XString8();
      XString8                BoardNumber = XString8(); //Board-ID
      XString8                LocationInChassis = XString8();
      XString8                BoardVersion = XString8();
      UINT8                   BoardType = UINT8();
      // SMBIOS TYPE3
      bool                    Mobile = bool();
      UINT8                   ChassisType = UINT8();
      XString8                ChassisManufacturer = XString8();
      XString8                ChassisAssetTag = XString8();
      // SMBIOS TYPE4
      // SMBIOS TYPE17
      UINT16                  SmbiosVersion = UINT16();
      INT8                    Attribute = INT8();
// These were set but never used.
//      XString8                   MemoryManufacturer;
//      XString8                   MemorySerialNumber;
//      XString8                   MemoryPartNumber;
//      XString8                   MemorySpeed;
      // SMBIOS TYPE131
      // SMBIOS TYPE132
      bool                    TrustSMBIOS = 0;
      bool                    InjectMemoryTables = bool(); // same as Memory.SlotCounts
      // SMBIOS TYPE133
      UINT64                  gPlatformFeature = UINT64();
      // PatchTableType11
      bool                    NoRomInfo = bool();

      UINT32                  FirmwareFeatures = UINT32();
      UINT32                  FirmwareFeaturesMask = UINT32();
      RamSlotInfoArrayClass   RamSlotInfoArray = RamSlotInfoArrayClass();
      SlotDeviceArrayClass    SlotDevices = SlotDeviceArrayClass();

    SmbiosClass() {}
    
#if __cplusplus > 201703L
    bool operator == (const SmbiosClass&) const = default;
#endif
    bool isEqual(const SmbiosClass& other) const
    {
      // SMBIOS TYPE0
      if ( !(BiosVendor == other.BiosVendor) ) return false;
      if ( !(BiosVersion == other.BiosVersion) ) return false;
      if ( !(EfiVersion == other.EfiVersion) ) return false;
      if ( !(BiosReleaseDate == other.BiosReleaseDate) ) return false;
      // SMBIOS TYPE1
      if ( !(ManufactureName == other.ManufactureName) ) return false;
      if ( !(ProductName == other.ProductName) ) return false;
      if ( !(SystemVersion == other.SystemVersion) ) return false;
      if ( !(SerialNr == other.SerialNr) ) return false;
      if ( !(SmUUID == other.SmUUID) ) return false;
      if ( !(FamilyName == other.FamilyName) ) return false;
      // SMBIOS TYPE2
      if ( !(BoardManufactureName == other.BoardManufactureName) ) return false;
      if ( !(BoardSerialNumber == other.BoardSerialNumber) ) return false;
      if ( !(BoardNumber == other.BoardNumber) ) return false;
      if ( !(LocationInChassis == other.LocationInChassis) ) return false;
      if ( !(BoardVersion == other.BoardVersion) ) return false;
      if ( !(BoardType == other.BoardType) ) return false;
      // SMBIOS TYPE3
      if ( !(Mobile == other.Mobile) ) return false;
      if ( !(ChassisType == other.ChassisType) ) return false;
      if ( !(ChassisManufacturer == other.ChassisManufacturer) ) return false;
      if ( !(ChassisAssetTag == other.ChassisAssetTag) ) return false;
      // SMBIOS TYPE17
      if ( !(SmbiosVersion == other.SmbiosVersion) ) return false;
      if ( !(Attribute == other.Attribute) ) return false;
      // SMBIOS TYPE132
      if ( !(TrustSMBIOS == other.TrustSMBIOS) ) return false;
      if ( !(InjectMemoryTables == other.InjectMemoryTables) ) return false;
      // SMBIOS TYPE133
      if ( !(gPlatformFeature == other.gPlatformFeature) ) return false;
      // PatchTableType11
      if ( !(NoRomInfo == other.NoRomInfo) ) return false;

      if ( !(FirmwareFeatures == other.FirmwareFeatures) ) return false;
      if ( !(FirmwareFeaturesMask == other.FirmwareFeaturesMask) ) return false;
      if ( !RamSlotInfoArray.isEqual(other.RamSlotInfoArray) ) return false;
      if ( !SlotDevices.isEqual(other.SlotDevices) ) return false;

//      if ( memcmp(RPlt, other.RPlt, sizeof(RPlt)) != 0 ) return false;
//      if ( memcmp(RBr, other.RBr, sizeof(RBr)) != 0 ) return false;
//      if ( memcmp(EPCI, other.EPCI, sizeof(EPCI)) != 0 ) return false;
//      if ( memcmp(REV, other.REV, sizeof(REV)) != 0 ) return false;
      return true;
    }
    void takeValueFrom(const SmbiosPlistClass::SmbiosDictClass& configPlist)
    {
      // SMBIOS TYPE0
      BiosVendor = configPlist.dgetBiosVendor();
      BiosVersion = configPlist.dgetBiosVersion();
      EfiVersion = configPlist.dgetEfiVersion();
      BiosReleaseDate = configPlist.dgetBiosReleaseDate();
      // SMBIOS TYPE1
      ManufactureName = configPlist.dgetManufactureName();
      ProductName = configPlist.dgetProductName();
      SystemVersion = configPlist.dgetSystemVersion();
      SerialNr = configPlist.dgetSerialNr();
      SmUUID = configPlist.dgetSmUUID();
      FamilyName = configPlist.dgetFamilyName();
      // SMBIOS TYPE2
      BoardManufactureName = configPlist.dgetBoardManufactureName();
      BoardSerialNumber = configPlist.dgetBoardSerialNumber();
      BoardNumber = configPlist.dgetBoardNumber();
      LocationInChassis = configPlist.dgetLocationInChassis();
      BoardVersion = configPlist.dgetBoardVersion();
      BoardType = configPlist.dgetBoardType();
      // SMBIOS TYPE3
      Mobile = configPlist.dgetMobile();
      ChassisType = configPlist.dgetChassisType();
      ChassisManufacturer = configPlist.dgetChassisManufacturer();
      ChassisAssetTag = configPlist.dgetChassisAssetTag();
      // SMBIOS TYPE17
      SmbiosVersion = configPlist.dgetSmbiosVersion();
      Attribute = configPlist.dgetAttribute();
      // SMBIOS TYPE132
      TrustSMBIOS = configPlist.dgetTrustSMBIOS();
      InjectMemoryTables = configPlist.dgetInjectMemoryTables();
      // SMBIOS TYPE133
      gPlatformFeature = configPlist.dgetgPlatformFeature();
      // PatchTableType11
      NoRomInfo = configPlist.dgetNoRomInfo();

      FirmwareFeatures = configPlist.dgetFirmwareFeatures();
      FirmwareFeaturesMask = configPlist.dgetFirmwareFeaturesMask();
      RamSlotInfoArray.takeValueFrom(configPlist.Memory);
      SlotDevices.takeValueFrom(configPlist.Slots);
    }

  };
  class BootGraphicsClass {
    public:
      UINT32 DefaultBackgroundColor = 0;
      UINT32 UIScale = 0;
      UINT32 EFILoginHiDPI = 0;
//      UINT8  flagstate[32] = {0};
      uint32_t _flagstate = uint32_t();

      BootGraphicsClass() {
//      	flagstate.memset(0, 32);
      }
      
#if __cplusplus > 201703L
      bool operator == (const BootGraphicsClass&) const = default;
#endif
      bool isEqual(const BootGraphicsClass& other) const
      {
        if ( !(DefaultBackgroundColor == other.DefaultBackgroundColor) ) return false;
        if ( !(UIScale == other.UIScale) ) return false;
        if ( !(EFILoginHiDPI == other.EFILoginHiDPI) ) return false;
        if ( _flagstate != other._flagstate ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::BootGraphics_Class& configPlist)
      {
        DefaultBackgroundColor = configPlist.dgetDefaultBackgroundColor();
        UIScale = configPlist.dgetUIScale();
        EFILoginHiDPI = configPlist.dgetEFILoginHiDPI();
        _flagstate = configPlist.dget_flagstate();
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
  SmbiosClass Smbios = SmbiosClass();
  BootGraphicsClass BootGraphics = BootGraphicsClass();


//other
//  UINT16                  DropOEM_DSM; // not used anymore.
//  BOOLEAN                 LpcTune; // never set to true.

  SETTINGS_DATA() {}
//  SETTINGS_DATA(const SETTINGS_DATA& other) = delete; // Can be defined if needed
//  const SETTINGS_DATA& operator = ( const SETTINGS_DATA & ) = delete; // Can be defined if needed

#if __cplusplus > 201703L
  bool operator == (const SETTINGS_DATA&) const = default;
#endif
  bool isEqual(const SETTINGS_DATA& other) const
  {
    if ( !Boot.isEqual(other.Boot) ) return false;
    if ( !ACPI.isEqual(other.ACPI) ) return false;
    if ( !GUI.isEqual(other.GUI) ) return false;
    if ( !CPU.isEqual(other.CPU) ) return false;
    if ( !SystemParameters.isEqual(other.SystemParameters) ) return false;
    if ( !KernelAndKextPatches.isEqual(other.KernelAndKextPatches) ) return false;
    if ( !Graphics.isEqual(other.Graphics) ) return false;
    if ( !(DisabledDriverArray == other.DisabledDriverArray) ) return false;
    if ( !Quirks.isEqual(other.Quirks) ) return false;
    if ( !RtVariables.isEqual(other.RtVariables) ) return false;
    if ( !Devices.isEqual(other.Devices) ) return false;
    if ( !Smbios.isEqual(other.Smbios) ) return false;
    if ( !BootGraphics.isEqual(other.BootGraphics) ) return false;
    return true;
  }

  void takeValueFrom(const ConfigPlistClass& configPlist)
  {
    Boot.takeValueFrom(configPlist.Boot);
    ACPI.takeValueFrom(configPlist.ACPI);
    GUI.takeValueFrom(configPlist.GUI);
    CPU.takeValueFrom(configPlist.CPU);
    SystemParameters.takeValueFrom(configPlist.SystemParameters);
    KernelAndKextPatches.takeValueFrom(configPlist.KernelAndKextPatches);
    Graphics.takeValueFrom(configPlist.Graphics);
    DisabledDriverArray = configPlist.dgetDisabledDriverArray();
    Quirks.takeValueFrom(configPlist.Quirks);
    RtVariables.takeValueFrom(configPlist.RtVariables);
    Devices.takeValueFrom(configPlist.Devices);
    Smbios.takeValueFrom(configPlist.getSMBIOS());
    BootGraphics.takeValueFrom(configPlist.BootGraphics);
  }

  ~SETTINGS_DATA() {}

  const XString8& getUUID();
  const XString8& getUUID(EFI_GUIDClass* efiGuid);
  // If CustomUuid is defined, return false by default
  // If SmUUID is defined, return true by default.
  bool ShouldInjectSystemID() {
    if ( SystemParameters.CustomUuid.notEmpty() &&  SystemParameters.CustomUuid != nullGuidAsString ) {
      if ( SystemParameters._InjectSystemID == 2 ) return false;
      else return SystemParameters._InjectSystemID;
    }
    if ( Smbios.SmUUID.isEmpty() || Smbios.SmUUID == nullGuidAsString ) return false;
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


//extern GFX_PROPERTIES                 gGraphics[];
//extern HDA_PROPERTIES                 gAudios[];
//extern UINTN                          NGFX;
//extern UINTN                          NHDA;
//extern UINT16                         gCPUtype;
extern SETTINGS_DATA                  gSettings;
extern BOOLEAN                        gFirmwareClover;
extern DRIVERS_FLAGS                  gDriversFlags;
extern EFI_EDID_DISCOVERED_PROTOCOL   *EdidDiscovered;
//extern UINT8                          *gEDID;

extern UINTN                           gEvent;

extern UINT16                          gBacklightLevel;

//extern BOOLEAN                         defDSM;
//extern UINT16                          dropDSM;

//extern TagDict*                          gConfigDict[];

// ACPI/PATCHED/AML
extern XObjArray<ACPI_PATCHED_AML>       ACPIPatchedAML;


// SysVariables
//extern SYSVARIABLES                   *SysVariables;

// Hold theme fixed IconFormat / extension
extern CHAR16                         *IconFormat;


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

  bool                    DropSSDT = 0; // init with gSettings.Boot.DropSSDTSetting. Put back to false if one table is dropped (see afterGetUserSettings)

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

  XString8                   OEMProductFromSmbios = XString8();
  XString8                   OEMVendorFromSmbios = XString8();
  XString8                   OEMBoardFromSmbios = XString8();
  UINT8                      EnabledCores = 0;

//  XStringW                   ConfigName; // Set but never used

  UINT32 OptionsBits = 0;
  UINT32 FlagsBits = 0;

  XStringW                    BlockKexts = XStringW();
  // KernelAndKextPatches
  BOOLEAN                 KextPatchesAllowed = true;
  BOOLEAN                 KernelPatchesAllowed = true; //From GUI: Only for user patches, not internal Clover

  XString8 BiosVersionUsed = XString8();
  XString8 EfiVersionUsed = XString8();
  XString8 ReleaseDateUsed = XString8();
  
  UINT8  flagstate[32] = {0};
  MACHINE_TYPES CurrentModel = MaxMachineType;

  UINT32               IgPlatform = UINT32(); //could also be snb-platform-id


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




//void
//GetDevices(void);



void afterGetUserSettings(SETTINGS_DATA& gSettings);

XStringW
GetOtherKextsDir (BOOLEAN On);

XStringW GetOSVersionKextsDir(const MacOsVersion& OSVersion);

EFI_STATUS
InjectKextsFromDir (
  EFI_STATUS Status,
  CHAR16 *SrcDir
  );


EFI_STATUS
ApplySettings(void);


#endif

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
extern XBool             gFirmwareClover;


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
  XBool             LoadVBios;
//XBool             PatchVBios;
  UINTN             Segment;
  UINTN             Bus;
  UINTN             Device;
  UINTN             Function;
  EFI_HANDLE        Handle;
  UINT8             *Mmio;
  UINT32            Connectors;
  XBool             ConnChanged;
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
  XBool operator == (const ACPI_NAME&) const = default;
#endif
  XBool isEqual(const ACPI_NAME& other) const
  {
    if ( !(Name == other.Name) ) return false;
    return true;
  }
//  void takeValueFrom(const ACPI_NAME& other)
//  {
//    Name = other.Name;
//  }

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
  XBool operator == (const ACPI_RENAME_DEVICE&) const = default;
#endif
  XBool isEqual(const ACPI_RENAME_DEVICE& other) const
  {
    if ( !acpiName.isEqual(other.acpiName) ) return false;
    if ( !(renameTo == other.renameTo) ) return false;
    return true;
  }
  void takeValueFrom(const XmlAddKey<XmlKey, XmlString8>& other)
  {
    acpiName.Name = other.key();
    renameTo = other.value();
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
  union {
    UINT32   Signature = 0;
    char     SignatureAs4Chars[4];
  };
  UINT32          Length;
  UINT64          TableId;
  INPUT_ITEM      MenuItem = INPUT_ITEM();
  XBool           OtherOS;

  ACPI_DROP_TABLE() : Next(0), Signature(0), Length(0), TableId(0), OtherOS(false) {}
  ACPI_DROP_TABLE(const ACPI_DROP_TABLE& other) = delete; // Can be defined if needed
  const ACPI_DROP_TABLE& operator = ( const ACPI_DROP_TABLE & ) = delete; // Can be defined if needed
  ~ACPI_DROP_TABLE() {}
};

class CUSTOM_LOADER_SUBENTRY_SETTINGS;
class CUSTOM_LOADER_SUBENTRY;

                   
class CUSTOM_LOADER_SUBENTRY_SETTINGS
{
public:
  XBool                  Disabled = false;
public: // temporary, must be protected:
  // member defined with _ prefix should not be accessed from outside. I left them public for now for CompareCustomEntries()
  undefinable_XString8   _Arguments = undefinable_XString8();
  XString8               _AddArguments = XString8();

  undefinable_XString8   _FullTitle = undefinable_XString8();
  undefinable_XString8   _Title = undefinable_XString8();

  undefinable_bool       _NoCaches = undefinable_bool();
  
#if __cplusplus > 201703L
  XBool operator == (const CUSTOM_LOADER_SUBENTRY_SETTINGS&) const = default;
#endif
  XBool isEqual(const CUSTOM_LOADER_SUBENTRY_SETTINGS& other) const
  {
    if ( !(Disabled == other.Disabled) ) return false;
    if ( !(_Arguments == other._Arguments) ) return false;
    if ( !(_AddArguments == other._AddArguments) ) return false;
    if ( !(_FullTitle == other._FullTitle) ) return false;
    if ( !(_Title == other._Title) ) return false;
    if ( !(_NoCaches == other._NoCaches) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_SubEntry_Class& other)
  {
    Disabled = other.dgetDisabled();
    _Arguments = other.dget_Arguments();
    _AddArguments = other.dget_AddArguments();
    _FullTitle = other.dget_FullTitle();
    _Title = other.dget_Title();
    _NoCaches = other.dget_NoCaches();
  }

};

class CUSTOM_LOADER_ENTRY;

class CUSTOM_LOADER_SUBENTRY
{
public:
  const CUSTOM_LOADER_ENTRY& parent;
  const CUSTOM_LOADER_SUBENTRY_SETTINGS& settings = CUSTOM_LOADER_SUBENTRY_SETTINGS();

  CUSTOM_LOADER_SUBENTRY(const CUSTOM_LOADER_ENTRY& _customLoaderEntry, const CUSTOM_LOADER_SUBENTRY_SETTINGS& _settings) : parent(_customLoaderEntry), settings(_settings) {}
  
  XString8Array getLoadOptions() const;
  UINT8 getFlags(XBool NoCachesDefault) const;

  const XString8& getTitle() const;
  const XString8& getFullTitle() const;
};

class CUSTOM_LOADER_ENTRY_SETTINGS;

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
  
  XBool operator == (const EFI_GRAPHICS_OUTPUT_BLT_PIXELClass& other) const {
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
  XBool                   Disabled = false;
  XBuffer<UINT8>          ImageData = XBuffer<UINT8>();
  XBuffer<UINT8>          DriveImageData = XBuffer<UINT8>();
  XStringW                Volume = XStringW();
  XStringW                Path = XStringW();
  undefinable_XString8    Arguments = undefinable_XString8();
  XString8                AddArguments = XString8();
  XString8                FullTitle = XStringW();
  XStringW                Settings = XStringW(); // path of a config.plist that'll be read at the beginning of startloader
  char32_t                Hotkey = 0;
  XBool                   CommonSettings = false;
//  UINT8                   Flags = 0;
  XBool                   Hidden = false;
  XBool                   AlwaysHidden = false;
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

  XBool                   ForceTextMode = false; // 2021-04-22

public:
  
#if __cplusplus > 201703L
  XBool operator == (const CUSTOM_LOADER_ENTRY_SETTINGS&) const = default;
#endif
  XBool isEqual(const CUSTOM_LOADER_ENTRY_SETTINGS& other) const
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
  void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class& other)
  {
    Disabled = other.dgetDisabled();
    ImageData = other.dgetImageData();
    DriveImageData = other.dgetDriveImageData();
    Volume = other.dgetVolume();
    Path = other.dgetPath();
    Arguments = other.dgetArguments();
    AddArguments = other.dgetAddArguments();
    FullTitle = other.dgetFullTitle();
    Settings = other.dgetSettings();
    Hotkey = other.dgetHotkey();
    CommonSettings = other.dgetCommonSettings();
    Hidden = other.dgetHidden();
    AlwaysHidden = other.dgetAlwaysHidden();
    Type = other.dgetType();
    VolumeType = other.dgetVolumeType();
    KernelScan = other.dgetKernelScan();
    CustomLogoAsXString8 = other.dgetCustomLogoAsXString8();
    CustomLogoAsData = other.dgetCustomLogoAsData();
    BootBgColor = other.dgetBootBgColor();
    InjectKexts = other.dgetInjectKexts();
    NoCaches = other.dgetNoCaches();
    SubEntriesSettings.takeValueFrom(other.SubEntries);
    m_DriveImagePath = other.dgetm_DriveImagePath();
    m_Title = other.dgetm_Title();
    CustomLogoTypeSettings = other.dgetCustomLogoTypeSettings();
    m_ImagePath = other.dgetm_ImagePath();
    ForceTextMode = other.dgetForceTextMode();
  }


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
  
  UINT8 getFlags(XBool NoCachesDefault) const {
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
  XBool                Disabled = false;
  XStringW             ImagePath = XStringW();
  XBuffer<UINT8>       ImageData = XBuffer<UINT8>();
  XStringW             DriveImagePath = XStringW();
  XBuffer<UINT8>       DriveImageData = XBuffer<UINT8>();
  XStringW             Volume = XStringW();
  XStringW             FullTitle = XStringW();
  XStringW             Title = XStringW();
  char32_t             Hotkey = 0;
//  UINT8                Flags = 0;
  XBool                Hidden = false;
  XBool                AlwaysHidden = false;
  UINT8                Type = 0;
  UINT8                VolumeType = 0;

#if __cplusplus > 201703L
  XBool operator == (const CUSTOM_LEGACY_ENTRY_SETTINGS&) const = default;
#endif
  XBool isEqual(const CUSTOM_LEGACY_ENTRY_SETTINGS& other) const
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
  void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class& other)
  {
    Disabled = other.dgetDisabled();
    ImagePath = other.dgetImagePath();
    ImageData = other.dgetImageData();
    DriveImagePath = other.dgetDriveImagePath();
    DriveImageData = other.dgetDriveImageData();
    Volume = other.dgetVolume();
    FullTitle = other.dgetFullTitle();
    Title = other.dgetTitle();
    Hotkey = other.dgetHotkey();
    Hidden = other.dgetHidden();
    AlwaysHidden = other.dgetAlwaysHidden();
    Type = other.dgetType();
    VolumeType = other.dgetVolumeType();
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
  XBool              Disabled = false;
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
  XBool              Hidden = false;
  XBool              AlwaysHidden = false;
  UINT8              VolumeType = 0;
  
#if __cplusplus > 201703L
  XBool operator == (const CUSTOM_TOOL_ENTRY_SETTINGS&) const = default;
#endif
  XBool isEqual(const CUSTOM_TOOL_ENTRY_SETTINGS& other) const
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
  void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class& other)
  {
    Disabled = other.dgetDisabled();
    ImagePath = other.dgetImagePath();
    ImageData = other.dgetImageData();
    Volume = other.dgetVolume();
    Path = other.dgetPath();
    Arguments = other.dgetArguments();
    FullTitle = other.dgetFullTitle();
    Title = other.dgetTitle();
    Hotkey = other.dgetHotkey();
    Hidden = other.dgetHidden();
    AlwaysHidden = other.dgetAlwaysHidden();
    VolumeType = other.dgetVolumeType();
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
    XBool operator == (const VBIOS_PATCH&) const = default;
#endif
    XBool isEqual(const VBIOS_PATCH& other) const
    {
      if ( !(Find == other.Find) ) return false;
      if ( !(Replace == other.Replace) ) return false;
      return true;
    }
    void takeValueFrom(const ConfigPlistClass::Graphics_Class::Graphics_PatchVBiosBytes_Class& other)
    {
      Find = other.dgetFind();
      Replace = other.dgetReplace();
    }
};

class PatchVBiosBytesNewClass : public XObjArrayWithTakeValueFromXmlArray<VBIOS_PATCH, ConfigPlistClass::Graphics_Class::Graphics_PatchVBiosBytes_Class>
{
  mutable XArray<VBIOS_PATCH_BYTES> VBIOS_PATCH_BYTES_array = XArray<VBIOS_PATCH_BYTES>();
public:
  
#if __cplusplus > 201703L
    XBool operator == (const PatchVBiosBytesNewClass& other) const { return XObjArray<VBIOS_PATCH>::operator ==(other); }
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

  XBool isEqual(const PatchVBiosBytesNewClass& other) const
  {
    return XObjArray<VBIOS_PATCH>::isEqual(other);
//    getVBIOS_PATCH_BYTES();
//    if ( VBIOS_PATCH_BYTES_array.size() != other.VBIOS_PATCH_BYTES_array.size() ) return false;
//    for ( size_t idx = 0 ; idx < VBIOS_PATCH_BYTES_array.size() ; ++idx ) {
//      if ( VBIOS_PATCH_BYTES_array[idx].NumberOfBytes != other[idx].Find.size() ) return false;
//      if ( memcmp(VBIOS_PATCH_BYTES_array[idx].Find, other.VBIOS_PATCH_BYTES_array[idx].Find, VBIOS_PATCH_BYTES_array[idx].NumberOfBytes) != 0 ) return false;
//      if ( memcmp(VBIOS_PATCH_BYTES_array[idx].Replace, other.VBIOS_PATCH_BYTES_array[idx].Replace, VBIOS_PATCH_BYTES_array[idx].NumberOfBytes) != 0 ) return false;
//    }
//    return true;
  }
//  void takeValueFrom(const PatchVBiosBytesNewClass& other)
//  {
//  }

};


class SETTINGS_DATA;
class ConfigPlistClass;
class TagDict;
//XBool CompareOldNewSettings(const SETTINGS_DATA& , const ConfigPlistClass& );
//EFI_STATUS GetUserSettings(const TagDict* CfgDict, SETTINGS_DATA& gSettings);

class SETTINGS_DATA {
public:

  class BootClass {
    public:
      INTN                    Timeout = -1;
      XBool                   SkipHibernateTimeout = false;
      XBool                   DisableCloverHotkeys = false;
      XString8                BootArgs = XString8();
      XBool                   NeverDoRecovery = false;
      XBool                   LastBootedVolume = false;
      XStringW                DefaultVolume = XStringW();
      XStringW                DefaultLoader = XStringW();
      XBool                   DebugLog = false;
      XBool                   FastBoot = false;
      XBool                   NoEarlyProgress = false;
      XBool                   NeverHibernate = false;
      XBool                   StrictHibernate = false;
      XBool                   RtcHibernateAware = false;
      XBool                   HibernationFixup = false;
      XBool                   SignatureFixup = false;
      INT8                    SecureSetting = 0; // 0 == false, 1 == true, -1 == undefined
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
    XBool operator == (const BootClass&) const = default;
#endif
      XBool isEqual(const BootClass& other) const
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
      void takeValueFrom(const ConfigPlistClass::Boot_Class& other)
      {
        Timeout = other.dgetTimeout();
        SkipHibernateTimeout = other.dgetSkipHibernateTimeout();
        DisableCloverHotkeys = other.dgetDisableCloverHotkeys();
        BootArgs = other.dgetBootArgs();
        NeverDoRecovery = other.dgetNeverDoRecovery();
        LastBootedVolume = other.dgetLastBootedVolume();
        DefaultVolume = other.dgetDefaultVolume();
        DefaultLoader = other.dgetDefaultLoader();
        DebugLog = other.dgetDebugLog();
        FastBoot = other.dgetFastBoot();
        NoEarlyProgress = other.dgetNoEarlyProgress();
        NeverHibernate = other.dgetNeverHibernate();
        StrictHibernate = other.dgetStrictHibernate();
        RtcHibernateAware = other.dgetRtcHibernateAware();
        HibernationFixup = other.dgetHibernationFixup();
        SignatureFixup = other.dgetSignatureFixup();
        SecureSetting = other.dgetSecureSetting();
        SecureBootPolicy = other.dgetSecureBootPolicy();
        SecureBootWhiteList = other.dgetSecureBootWhiteList();
        SecureBootBlackList = other.dgetSecureBootBlackList();
        XMPDetection = other.dgetXMPDetection();
        LegacyBoot = other.dgetLegacyBoot(gFirmwareClover);
        LegacyBiosDefaultEntry = other.dgetLegacyBiosDefaultEntry();
        CustomLogoType = other.dgetCustomLogoType();
        CustomLogoAsXString8 = other.dgetCustomLogoAsXString8();
        CustomLogoAsData = other.dgetCustomLogoAsData();
      }
  };
  
  class ACPIClass
  {
    public:
      class ACPIDropTablesClass
      {
        public:
          union {
            UINT32   Signature = 0;
            char     SignatureAs4Chars[4];
          };
          UINT64   TableId = 0;
          UINT32   TabLength = 0;
          XBool    OtherOS = false;
          
#if __cplusplus > 201703L
          XBool operator == (const ACPIDropTablesClass&) const = default;
#endif
          XBool isEqual(const ACPIDropTablesClass& other) const
          {
            if ( !(Signature == other.Signature) ) return false;
            if ( !(TableId == other.TableId) ) return false;
            if ( !(TabLength == other.TabLength) ) return false;
            if ( !(OtherOS == other.OtherOS) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class& other)
          {
            Signature = other.dgetSignature();
            TableId = other.dgetTableId();
            TabLength = other.dgetTabLength();
            OtherOS = other.dgetOtherOS();
          }
      };
      
      class DSDTClass
      {
        public:
          class DSDT_Patch
          {
          public :
            XBool            Disabled = XBool();
            XString8         PatchDsdtLabel = XString8();
            XBuffer<UINT8>   PatchDsdtFind = XBuffer<UINT8>();
            XBuffer<UINT8>   PatchDsdtReplace = XBuffer<UINT8>();
            XBuffer<UINT8>   PatchDsdtTgt = XBuffer<UINT8>();
            INPUT_ITEM       PatchDsdtMenuItem = INPUT_ITEM(); // Not read from config.plist. Should be moved out.

#if __cplusplus > 201703L
            XBool operator == (const DSDT_Patch&) const = default;
#endif
            XBool isEqual(const DSDT_Patch& other) const
            {
              if ( !(Disabled == other.Disabled) ) return false;
              if ( !(PatchDsdtLabel == other.PatchDsdtLabel) ) return false;
              if ( !(PatchDsdtFind == other.PatchDsdtFind) ) return false;
              if ( !(PatchDsdtReplace == other.PatchDsdtReplace) ) return false;
              if ( !(PatchDsdtTgt == other.PatchDsdtTgt) ) return false;
              if ( !(PatchDsdtMenuItem == other.PatchDsdtMenuItem) ) return false;
              return true;
            }
            void takeValueFrom(const ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Patch_Class& other)
            {
              Disabled = other.dgetDisabled();
              PatchDsdtLabel = other.dgetPatchDsdtLabel();
              PatchDsdtFind = other.dgetPatchDsdtFind();
              PatchDsdtReplace = other.dgetPatchDsdtReplace();
              PatchDsdtTgt = other.dgetPatchDsdtTgt();
              PatchDsdtMenuItem.BValue = !other.dgetDisabled();
            }
          };

          XStringW                DsdtName = XStringW();
          XBool                   DebugDSDT = false;
          XBool                   Rtc8Allowed = false;
          UINT8                   PNLF_UID = 0;
          UINT32                  FixDsdt = 0;
          XBool                   ReuseFFFF = false;
          XBool                   SuspendOverride = false;
          XObjArrayWithTakeValueFromXmlArray<DSDT_Patch, ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Patch_Class>
                                  DSDTPatchArray = XObjArrayWithTakeValueFromXmlArray<DSDT_Patch, ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Patch_Class>();

#if __cplusplus > 201703L
          XBool operator == (const DSDTClass&) const = default;
#endif
          XBool isEqual(const DSDTClass& other) const
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
        void takeValueFrom(const ConfigPlistClass::ACPI_Class::DSDT_Class& other)
          {
            DsdtName = other.dgetDsdtName();
            DebugDSDT = other.dgetDebugDSDT();
            Rtc8Allowed = other.dgetRtc8Allowed();
            PNLF_UID = other.dgetPNLF_UID();
            FixDsdt = other.dgetFixDsdt();
            ReuseFFFF = other.dgetReuseFFFF();
            SuspendOverride = other.dgetSuspendOverride();
            DSDTPatchArray.takeValueFrom(other.Patches);
          }
      };
      
      class SSDTClass
      {
        public:
          class GenerateClass
          {
            public:
              XBool                GeneratePStates = false;
              XBool                GenerateCStates = false;
              XBool                GenerateAPSN = false;
              XBool                GenerateAPLF = false;
              XBool                GeneratePluginType = false;

#if __cplusplus > 201703L
              XBool operator == (const GenerateClass&) const = default;
#endif
              XBool isEqual(const GenerateClass& other) const
              {
                if ( !(GeneratePStates == other.GeneratePStates) ) return false;
                if ( !(GenerateCStates == other.GenerateCStates) ) return false;
                if ( !(GenerateAPSN == other.GenerateAPSN) ) return false;
                if ( !(GenerateAPLF == other.GenerateAPLF) ) return false;
                if ( !(GeneratePluginType == other.GeneratePluginType) ) return false;
                return true;
              }
              void takeValueFrom(const ConfigPlistClass::ACPI_Class::SSDT_Class::XmlUnionGenerate& other)
              {
                GeneratePStates = other.dgetGeneratePStates();
                GenerateCStates = other.dgetGenerateCStates();
                GenerateAPSN = other.dgetGenerateAPSN();
                GenerateAPLF = other.dgetGenerateAPLF();
                GeneratePluginType = other.dgetGeneratePluginType();
              }
          };

          XBool                   DropSSDTSetting = false;
          XBool                   NoOemTableId = false;
          XBool                   NoDynamicExtract = false;
          XBool                   EnableISS = false;
          XBool                   EnableC7 = false;
          XBool                   _EnableC6 = false;
          XBool                   _EnableC4 = false;
          XBool                   _EnableC2 = false;
          UINT16                  _C3Latency = 0;
          UINT8                   PLimitDict = 0;
          UINT8                   UnderVoltStep = 0;
          XBool                   DoubleFirstState = false;
          UINT8                   MinMultiplier = 0;
          UINT8                   MaxMultiplier = 0;
          UINT8                   PluginType = 0;
          GenerateClass           Generate = GenerateClass();
          
#if __cplusplus > 201703L
          XBool operator == (const SSDTClass&) const = default;
#endif
          XBool isEqual(const SSDTClass& other) const
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
          void takeValueFrom(const ConfigPlistClass::ACPI_Class::SSDT_Class& other)
          {
            DropSSDTSetting = other.dgetDropSSDTSetting();
            NoOemTableId = other.dgetNoOemTableId();
            NoDynamicExtract = other.dgetNoDynamicExtract();
            EnableISS = other.dgetEnableISS();
            EnableC7 = other.dgetEnableC7();
            _EnableC6 = other.dget_EnableC6();
            _EnableC4 = other.dget_EnableC4();
            _EnableC2 = other.dget_EnableC2();
            _C3Latency = other.dget_C3Latency();
            PLimitDict = other.dgetPLimitDict();
            UnderVoltStep = other.dgetUnderVoltStep();
            DoubleFirstState = other.dgetDoubleFirstState();
            MinMultiplier = other.dgetMinMultiplier();
            MaxMultiplier = other.dgetMaxMultiplier();
            PluginType = other.dgetPluginType();
            Generate.takeValueFrom(other.Generate);
          }
      };

      UINT64                            ResetAddr = 0;
      UINT8                             ResetVal = 0;
      XBool                             SlpSmiEnable = false;
      XBool                             FixHeaders = false;
      XBool                             FixMCFG = false;
      XBool                             NoASPM = false;
      XBool                             smartUPS = false;
      XBool                             PatchNMI = false;
      XBool                             AutoMerge = false;
      XStringWArray                     DisabledAML = XStringWArray();
      XString8Array                     SortedACPI = XString8Array();
//      XObjArray<ACPI_RENAME_DEVICE>     DeviceRename = XObjArray<ACPI_RENAME_DEVICE>();
//      XObjArrayWithTakeValueFrom<ACPI_RENAME_DEVICE, ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class>
//                                        DeviceRename = XObjArrayWithTakeValueFrom<ACPI_RENAME_DEVICE, ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class>();
      class DeviceRename_Array : public XObjArray<ACPI_RENAME_DEVICE> {
       public:
        void takeValueFrom(const ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class& other)
        {
          size_t idx;
          for ( idx = 0 ; idx < other.size() ; ++idx ) {
            if ( idx < size() ) ElementAt(idx).takeValueFrom(other.getAtIndex(idx));
            else {
              ACPI_RENAME_DEVICE* s = new ACPI_RENAME_DEVICE;
              s->takeValueFrom(other.getAtIndex(idx));
              AddReference(s, true);
            }
          }
          while ( idx < size() ) RemoveAtIndex(idx);
        }
      } DeviceRename = DeviceRename_Array();
      XObjArrayWithTakeValueFromXmlArray<ACPIDropTablesClass, ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class>
                                        ACPIDropTablesArray = XObjArrayWithTakeValueFromXmlArray<ACPIDropTablesClass, ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class>();
      DSDTClass DSDT =                  DSDTClass();
      SSDTClass SSDT =                  SSDTClass();
        
#if __cplusplus > 201703L
      XBool operator == (const ACPIClass&) const = default;
#endif
      XBool isEqual(const ACPIClass& other) const
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
      void takeValueFrom(const ConfigPlistClass::ACPI_Class& other)
      {
        ResetAddr = other.dgetResetAddr();
        ResetVal = other.dgetResetVal();
        SlpSmiEnable = other.dgetSlpSmiEnable();
        FixHeaders = other.dgetFixHeaders() || other.DSDT.Fixes.dgetFixHeaders();
        FixMCFG = other.dgetFixMCFG();
        NoASPM = other.dgetNoASPM();
        smartUPS = other.dgetsmartUPS();
        PatchNMI = other.dgetPatchNMI();
        AutoMerge = other.dgetAutoMerge();
        DisabledAML = other.dgetDisabledAML();
        SortedACPI = other.dgetSortedACPI();
        DeviceRename.takeValueFrom(other.RenameDevices);
        ACPIDropTablesArray.takeValueFrom(other.ACPIDropTablesArray);
        DSDT.takeValueFrom(other.DSDT);
        SSDT.takeValueFrom(other.SSDT);
      }
  };

  class GUIClass {
    public:
      class MouseClass {
        public:
          INTN                 PointerSpeed = 0;
          XBool                PointerEnabled = false;
          UINT64               DoubleClickTime = 0;
          XBool                PointerMirror = false;
          
#if __cplusplus > 201703L
          XBool operator == (const MouseClass&) const = default;
#endif
          XBool isEqual(const MouseClass& other) const
          {
            if ( !(PointerSpeed == other.PointerSpeed) ) return false;
            if ( !(PointerEnabled == other.PointerEnabled) ) return false;
            if ( !(DoubleClickTime == other.DoubleClickTime) ) return false;
            if ( !(PointerMirror == other.PointerMirror) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Mouse_Class& other)
          {
            PointerSpeed = other.dgetPointerSpeed();
            PointerEnabled = other.dgetPointerEnabled();
            DoubleClickTime = other.dgetDoubleClickTime();
            PointerMirror = other.dgetPointerMirror();
          }
      } ;
      class ScanClass {
        public:
          XBool                DisableEntryScan = false;
          XBool                DisableToolScan = false;
          UINT8                KernelScan = 0;
          XBool                LinuxScan = false;
          XBool                LegacyFirst = false;
          XBool                NoLegacy = false;
          
#if __cplusplus > 201703L
          XBool operator == (const ScanClass&) const = default;
#endif
          XBool isEqual(const ScanClass& other) const
          {
            if ( !(DisableEntryScan == other.DisableEntryScan) ) return false;
            if ( !(DisableToolScan == other.DisableToolScan) ) return false;
            if ( !(KernelScan == other.KernelScan) ) return false;
            if ( !(LinuxScan == other.LinuxScan) ) return false;
            if ( !(LegacyFirst == other.LegacyFirst) ) return false;
            if ( !(NoLegacy == other.NoLegacy) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::GUI_Class::GUI_Scan_Class& other)
          {
            DisableEntryScan = other.dgetDisableEntryScan();
            DisableToolScan = other.dgetDisableToolScan();
            KernelScan = other.dgetKernelScan();
            LinuxScan = other.dgetLinuxScan();
            LegacyFirst = other.dgetLegacyFirst();
            NoLegacy = other.dgetNoLegacy();
          }
      };

      INT32                   Timezone = 0xFF;
      XStringW                Theme = XStringW();
      //XBool                   DarkEmbedded = 0;
      XString8                EmbeddedThemeType = XString8();
      XBool                   PlayAsync = false;
      XBool                   CustomIcons = false;
      XBool                   TextOnly = false;
      XBool                   ShowOptimus = false;
      XStringW                ScreenResolution = XStringW();
      XBool                   ProvideConsoleGop = false;
      INTN                    ConsoleMode = 0;
      XString8                Language = XString8();
      LanguageCode            languageCode = english;
      XBool                   KbdPrevLang = false;
      XString8Array           HVHideStrings = XString8Array();
      ScanClass               Scan =        ScanClass();
      MouseClass              Mouse =      MouseClass();
      XObjArrayWithTakeValueFromXmlArray<CUSTOM_LOADER_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class>
                              CustomEntriesSettings = XObjArrayWithTakeValueFromXmlArray<CUSTOM_LOADER_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class>();
      XObjArrayWithTakeValueFromXmlArray<CUSTOM_LEGACY_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class>
                              CustomLegacySettings = XObjArrayWithTakeValueFromXmlArray<CUSTOM_LEGACY_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Legacy_Class>();
      XObjArrayWithTakeValueFromXmlArray<CUSTOM_TOOL_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class>
                              CustomToolSettings = XObjArrayWithTakeValueFromXmlArray<CUSTOM_TOOL_ENTRY_SETTINGS, ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Tool_Class>();

      XBool getDarkEmbedded(XBool isDaylight) const;
    
#if __cplusplus > 201703L
      XBool operator == (const GUIClass&) const = default;
#endif
      XBool isEqual(const GUIClass& other) const
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
      void takeValueFrom(const ConfigPlistClass::GUI_Class& other)
      {
        Timezone = other.dgetTimezone();
        Theme = other.dgetTheme();
        EmbeddedThemeType = other.dgetEmbeddedThemeType();
        PlayAsync = other.dgetPlayAsync();
        CustomIcons = other.dgetCustomIcons();
        TextOnly = other.dgetTextOnly();
        ShowOptimus = other.dgetShowOptimus();
        ScreenResolution = other.dgetScreenResolution();
        ProvideConsoleGop = other.dgetProvideConsoleGop();
        ConsoleMode = other.dgetConsoleMode();
        Language = other.dgetLanguage();
        languageCode = other.dgetlanguageCode();
        KbdPrevLang = other.dgetKbdPrevLang();
        HVHideStrings = other.dgetHVHideStrings();
        Scan.takeValueFrom(other.Scan);
        Mouse.takeValueFrom(other.Mouse);
        CustomEntriesSettings.takeValueFrom(other.Custom.Entries);
        CustomLegacySettings.takeValueFrom(other.Custom.Legacy);
        CustomToolSettings.takeValueFrom(other.Custom.Tool);
      }

  };

  class CPUClass {
    public:
      UINT16                  QPI = 0;
      UINT32                  CpuFreqMHz = 0;
      UINT16                  CpuType = 0;
      XBool                   QEMU = false;
      XBool                   UseARTFreq = false;
      UINT32                  BusSpeed = 0; //in kHz
      XBool                   UserChange = false;
      UINT8                   SavingMode = 0;
      XBool                   HWPEnable = false;
      undefinable_uint32      HWPValue = undefinable_uint32();
      UINT8                   TDP = 0;
      XBool                   TurboDisabled = false;
      undefinable_bool        _EnableC6 = undefinable_bool();
      undefinable_bool        _EnableC4 = undefinable_bool();
      undefinable_bool        _EnableC2 = undefinable_bool();
      
#if __cplusplus > 201703L
    XBool operator == (const CPUClass&) const = default;
#endif
    XBool isEqual(const CPUClass& other) const
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
      return true;
    }
    void takeValueFrom(const ConfigPlistClass::CPU_Class& other)
    {
      QPI = other.dgetQPI();
      CpuFreqMHz = other.dgetCpuFreqMHz();
      CpuType = other.dgetCpuType();
      QEMU = other.dgetQEMU();
      UseARTFreq = other.dgetUseARTFreq();
      BusSpeed = other.dgetBusSpeed();
      UserChange = other.dgetUserChange();
      SavingMode = other.dgetSavingMode();
      HWPEnable = other.dgetHWPEnable();
      HWPValue = other.dgetHWPValue();
      TDP = other.dgetTDP();
      TurboDisabled = other.dgetTurboDisabled();
      _EnableC6 = other.dget_EnableC6();
      _EnableC4 = other.dget_EnableC4();
      _EnableC2 = other.dget_EnableC2();
    }
  };

  class SystemParametersClass {
    public:
      XBool                WithKexts = true;
      XBool                WithKextsIfNoFakeSMC = false;
      XBool                NoCaches = false;
      uint16_t   BacklightLevel = 0xFFFF;
      XBool BacklightLevelConfig = false;
      XString8             CustomUuid = XString8();
    public: // temporary, must be protected:
      UINT8                _InjectSystemID = 2; // 0=false, 1=true, other value = default.
    public:
      XBool                NvidiaWeb = false;

#if __cplusplus > 201703L
      XBool operator == (const SystemParametersClass&) const = default;
#endif
      XBool isEqual(const SystemParametersClass& other) const
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
      void takeValueFrom(const ConfigPlistClass::SystemParameters_Class& other)
      {
        WithKexts = other.dgetWithKexts();
        WithKextsIfNoFakeSMC = other.dgetWithKextsIfNoFakeSMC();
        NoCaches = other.dgetNoCaches();
        BacklightLevel = other.dgetBacklightLevel();
        BacklightLevelConfig = other.dgetBacklightLevelConfig();
        CustomUuid = other.dgetCustomUuid();
        _InjectSystemID = other.dget_InjectSystemID();
        NvidiaWeb = other.dgetNvidiaWeb();
      }
  };

  class GraphicsClass {
    public:
      class EDIDClass {
        public:
          XBool                   InjectEDID = XBool();
          XBuffer<UINT8>          CustomEDID = XBuffer<UINT8> ();
          UINT16                  VendorEDID = UINT16();
          UINT16                  ProductEDID = UINT16();
          UINT16                  EdidFixHorizontalSyncPulseWidth = UINT16();
          UINT8                   EdidFixVideoInputSignal = UINT8();
          
#if __cplusplus > 201703L
          XBool operator == (const EDIDClass&) const = default;
#endif
          XBool isEqual(const EDIDClass& other) const
          {
            if ( !(InjectEDID == other.InjectEDID) ) return false;
            if ( !(CustomEDID == other.CustomEDID) ) return false;
            if ( !(VendorEDID == other.VendorEDID) ) return false;
            if ( !(ProductEDID == other.ProductEDID) ) return false;
            if ( !(EdidFixHorizontalSyncPulseWidth == other.EdidFixHorizontalSyncPulseWidth) ) return false;
            if ( !(EdidFixVideoInputSignal == other.EdidFixVideoInputSignal) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::Graphics_Class::Graphics_EDID_Class& other)
          {
            InjectEDID = other.dgetInjectEDID();
            CustomEDID = other.dgetCustomEDID();
            VendorEDID = other.dgetVendorEDID();
            ProductEDID = other.dgetProductEDID();
            EdidFixHorizontalSyncPulseWidth = other.dgetEdidFixHorizontalSyncPulseWidth();
            EdidFixVideoInputSignal = other.dgetEdidFixVideoInputSignal();
          }
      };
      
      class InjectAsDictClass {
        public:
          XBool GraphicsInjector = XBool();
          XBool InjectIntel = XBool();
          XBool InjectATI = XBool();
          XBool InjectNVidia = XBool();
        
#if __cplusplus > 201703L
        XBool operator == (const InjectAsDictClass&) const = default;
#endif
        XBool isEqual(const InjectAsDictClass& other) const
        {
          if ( !(GraphicsInjector == other.GraphicsInjector) ) return false;
          if ( !(InjectIntel == other.InjectIntel) ) return false;
          if ( !(InjectATI == other.InjectATI) ) return false;
          if ( !(InjectNVidia == other.InjectNVidia) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::Graphics_Class::XmlInjectUnion& other)
        {
          GraphicsInjector = other.dgetGraphicsInjector();
          InjectIntel = other.dgetInjectIntel();
          InjectATI = other.dgetInjectATI();
          InjectNVidia = other.dgetInjectNVidia();
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
          XBool          LoadVBios = false;
        
#if __cplusplus > 201703L
        XBool operator == (const GRAPHIC_CARD&) const = default;
#endif
        XBool isEqual(const GRAPHIC_CARD& other) const
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
        void takeValueFrom(const ConfigPlistClass::Graphics_Class::Graphics_ATI_NVIDIA_Class& other)
        {
          Signature = other.dgetSignature();
          Model = other.dgetModel();
          Id = other.dgetId();
          SubId = other.dgetSubId();
          VideoRam = other.dgetVideoRam();
          VideoPorts = other.dgetVideoPorts();
          LoadVBios = other.dgetLoadVBios();
        }
      };

      XBool                    PatchVBios = XBool();
      PatchVBiosBytesNewClass  PatchVBiosBytes = PatchVBiosBytesNewClass();
//      undefinable_bool InjectAsBool = undefinable_bool();
      XBool                RadeonDeInit = XBool();
      XBool                LoadVBios = XBool();
      UINT64               VRAM = XBool();
      UINT32               RefCLK = UINT32();
      XStringW             FBName = XStringW();
      UINT16               VideoPorts = UINT16();
      XBool                NvidiaGeneric = XBool();
      XBool                NvidiaNoEFI = XBool();
      XBool                NvidiaSingle = XBool();
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
      XBool operator == (const GraphicsClass&) const = default;
#endif
      XBool isEqual(const GraphicsClass& other) const
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
      void takeValueFrom(const ConfigPlistClass::Graphics_Class& other)
      {
        PatchVBios = other.dgetPatchVBios();
        PatchVBiosBytes.takeValueFrom(other.PatchVBiosBytesArray);
        RadeonDeInit = other.dgetRadeonDeInit();
        LoadVBios = other.dgetLoadVBios();
        VRAM = other.dgetVRAM();
        RefCLK = other.dgetRefCLK();
        FBName = other.dgetFBName();
        VideoPorts = other.dgetVideoPorts();
        NvidiaGeneric = other.dgetNvidiaGeneric();
        NvidiaNoEFI = other.dgetNvidiaNoEFI();
        NvidiaSingle = other.dgetNvidiaSingle();
        Dcfg = other.dgetDcfg();
        NVCAP = other.dgetNVCAP();
        BootDisplay = other.dgetBootDisplay();
        DualLink = other.dgetDualLink();
        _IgPlatform = other.dget_IgPlatform();
        EDID.takeValueFrom(other.EDID);
        InjectAsDict.takeValueFrom(other.Inject);
        ATICardList.takeValueFrom(other.ATI);
        NVIDIACardList.takeValueFrom(other.NVIDIA);
      }

      //XBool getGraphicsInjector() const { return InjectAsBool.isDefined() ? InjectAsBool.value() : InjectAsDict.GraphicsInjector; }
      //XBool InjectIntel() const { return InjectAsBool.isDefined() ? InjectAsBool.value() : InjectAsDict.InjectIntel; }
      //XBool InjectATI() const { return InjectAsBool.isDefined() ? InjectAsBool.value() : InjectAsDict.InjectATI; }
      //XBool InjectNVidia() const { return InjectAsBool.isDefined() ? InjectAsBool.value() : InjectAsDict.InjectNVidia; }

  };
  
  class DevicesClass {
    public:
      
      class AudioClass {
        public:
          XBool                   ResetHDA = XBool();
          XBool                   HDAInjection = XBool();
          INT32                   HDALayoutId = INT32();
          XBool                   AFGLowPowerState = XBool();
        
#if __cplusplus > 201703L
        XBool operator == (const AudioClass&) const = default;
#endif
        XBool isEqual(const AudioClass& other) const
        {
          if ( !(ResetHDA == other.ResetHDA) ) return false;
          if ( !(HDAInjection == other.HDAInjection) ) return false;
          if ( !(HDALayoutId == other.HDALayoutId) ) return false;
          if ( !(AFGLowPowerState == other.AFGLowPowerState) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_Audio_Class& other)
        {
          ResetHDA = other.dgetResetHDA();
          HDAInjection = other.dgetHDAInjection();
          HDALayoutId = other.dgetHDALayoutId();
          AFGLowPowerState = other.dgetAFGLowPowerState();
        }
      };
      class USBClass {
        public:
          XBool                USBInjection = XBool();
          XBool                USBFixOwnership = XBool();
          XBool                InjectClockID = XBool();
          XBool                HighCurrent = XBool();
          XBool                NameEH00 = XBool();
          XBool                NameXH00 = XBool(); // is it used?
        
#if __cplusplus > 201703L
        XBool operator == (const USBClass&) const = default;
#endif
        XBool isEqual(const USBClass& other) const
        {
          if ( !(USBInjection == other.USBInjection) ) return false;
          if ( !(USBFixOwnership == other.USBFixOwnership) ) return false;
          if ( !(InjectClockID == other.InjectClockID) ) return false;
          if ( !(HighCurrent == other.HighCurrent) ) return false;
          if ( !(NameEH00 == other.NameEH00) ) return false;
          if ( !(NameXH00 == other.NameXH00) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_USB_Class& other)
        {
          USBInjection = other.dgetUSBInjection();
          USBFixOwnership = other.dgetUSBFixOwnership();
          InjectClockID = other.dgetInjectClockID();
          HighCurrent = other.dgetHighCurrent();
          NameEH00 = other.dgetNameEH00();
          //NameXH00 = other.dgetNameXH00();
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
        XBool operator == (const AddPropertyClass&) const = default;
#endif
        XBool isEqual(const AddPropertyClass& other) const
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
        void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class& other)
        {
          Device = other.dgetDevice();
          Key = other.dgetKey();
          Value = other.dgetValue();
          ValueType = other.dgetValueType();
          MenuItem.BValue = !other.dgetDisabled();
//          DevicePathAsString = other.dgetDevicePathAsString();
//          Label = other.dgetLabel();
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
        XBool operator == (const SimplePropertyClass&) const = default;
#endif
        XBool isEqual(const SimplePropertyClass& other) const
        {
          if ( !(Key == other.Key) ) return false;
          if ( !(Value == other.Value) ) return false;
          if ( !(ValueType == other.ValueType) ) return false;
          if ( !(MenuItem == other.MenuItem) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::SimplePropertyClass_Class& other)
        {
          Key = other.dgetKey();
          Value = other.dgetValue();
          ValueType = other.dgetValueType();
          MenuItem.BValue = !other.dgetDisabled();
        }
        void takeValueFrom(const ConfigPlistClass::DevicesClass::PropertiesUnion::Property& other)
        {
          Key = other.dgetKey();
          Value = other.dgetValue();
          ValueType = other.dgetValueType();
          MenuItem.BValue = other.dgetBValue();
        }
      };

      // Property don't have Device. Before it was always Device = 0 to differentiate from Arbitrary properties.
      class PropertiesClass {
        public:

          class PropertyClass
          {
          public:
            
            XBool                           Enabled = true;
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
            XBool operator == (const PropertyClass&) const = default;
#endif
            XBool isEqual(const PropertyClass& other) const
            {
              if ( !(Enabled == other.Enabled) ) return false;
              if ( !(DevicePathAsString == other.DevicePathAsString) ) return false;
//              if ( !(Label == other.Label) ) return false;
              if ( !propertiesArray.isEqual(other.propertiesArray) ) return false;
              return true;
            }
            void takeValueFrom(const ConfigPlistClass::DevicesClass::PropertiesUnion::Properties4DeviceClass& other)
            {
              Enabled = other.dgetEnabled();
              DevicePathAsString = other.dgetDevicePathAsString();
//              Label = other.dgetLabel();
              propertiesArray.takeValueFrom(other);
            }
          };

          XString8 propertiesAsString = XString8();
          XObjArrayWithTakeValueFromXmlRepeatingDict<PropertyClass, ConfigPlistClass::DevicesClass::PropertiesUnion::Properties4DeviceClass> PropertyArray = XObjArrayWithTakeValueFromXmlRepeatingDict<PropertyClass, ConfigPlistClass::DevicesClass::PropertiesUnion::Properties4DeviceClass>();
        
#if __cplusplus > 201703L
          XBool operator == (const PropertiesClass&) const = default;
#endif
          XBool isEqual(const PropertiesClass& other) const
          {
            if ( !(propertiesAsString == other.propertiesAsString) ) return false;
            if ( !PropertyArray.isEqual(other.PropertyArray) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::DevicesClass::PropertiesUnion& other)
          {
            propertiesAsString = other.dgetpropertiesAsString();
            PropertyArray.takeValueFrom(other.PropertiesAsDict);
          }
      };

      class ArbitraryPropertyClass {
        public:
          uint32_t                     Device = 0;
          XString8                     Label = XString8();
        XObjArrayWithTakeValueFromXmlArray<SimplePropertyClass, ConfigPlistClass::DevicesClass::SimplePropertyClass_Class>   CustomPropertyArray = XObjArrayWithTakeValueFromXmlArray<SimplePropertyClass, ConfigPlistClass::DevicesClass::SimplePropertyClass_Class>();
        
          ArbitraryPropertyClass() {}
#if __cplusplus > 201703L
          XBool operator == (const ArbitraryPropertyClass&) const = default;
#endif
          XBool isEqual(const ArbitraryPropertyClass& other) const
          {
            if ( !(Device == other.Device) ) return false;
            if ( !(Label == other.Label) ) return false;
            if ( !CustomPropertyArray.isEqual(other.CustomPropertyArray) ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class& other)
          {
            Device = other.dgetDevice();
            Label = other.dgetLabel();
            CustomPropertyArray.takeValueFrom(other.CustomProperties);
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
        XBool operator == (const FakeIDClass&) const = default;
#endif
        XBool isEqual(const FakeIDClass& other) const
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
        void takeValueFrom(const ConfigPlistClass::DevicesClass::Devices_FakeID_Class& other)
        {
          FakeATI = other.dgetFakeATI();
          FakeNVidia = other.dgetFakeNVidia();
          FakeIntel = other.dgetFakeIntel();
          FakeLAN = other.dgetFakeLAN();
          FakeWIFI = other.dgetFakeWIFI();
          FakeSATA = other.dgetFakeSATA();
          FakeXHCI = other.dgetFakeXHCI();
          FakeIMEI = other.dgetFakeIMEI();
        }
      };

      XBool                StringInjector = XBool();
      XBool                IntelMaxBacklight = XBool();
      XBool                IntelBacklight = XBool();
      UINT32               IntelMaxValue = UINT32();
      XBool                LANInjection = XBool();
      XBool                HDMIInjection = XBool();
      XBool                NoDefaultProperties = XBool();
      XBool                UseIntelHDMI = XBool();
      XBool                ForceHPET = XBool();
      UINT32               DisableFunctions = UINT32();
      XString8             AirportBridgeDeviceName = XString8();
      AudioClass           Audio = AudioClass();
      USBClass             USB = USBClass();
      FakeIDClass          FakeID = FakeIDClass();
      
      XObjArrayWithTakeValueFromXmlArray<AddPropertyClass, ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class> AddPropertyArray = XObjArrayWithTakeValueFromXmlArray<AddPropertyClass, ConfigPlistClass::DevicesClass::Devices_AddProperties_Dict_Class>();
      PropertiesClass Properties = PropertiesClass();
      XObjArrayWithTakeValueFromXmlArray<ArbitraryPropertyClass, ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class> ArbitraryArray = XObjArrayWithTakeValueFromXmlArray<ArbitraryPropertyClass, ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class>();

    
#if __cplusplus > 201703L
    XBool operator == (const DevicesClass&) const = default;
#endif
    XBool isEqual(const DevicesClass& other) const
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
    void takeValueFrom(const ConfigPlistClass::DevicesClass& other)
    {
      StringInjector = other.dgetStringInjector();
      IntelMaxBacklight = other.dgetIntelMaxBacklight();
      IntelBacklight = other.dgetIntelBacklight();
      IntelMaxValue = other.dgetIntelMaxValue();
      LANInjection = other.dgetLANInjection();
      HDMIInjection = other.dgetHDMIInjection();
      NoDefaultProperties = other.dgetNoDefaultProperties();
      UseIntelHDMI = other.dgetUseIntelHDMI();
      ForceHPET = other.dgetForceHPET();
      DisableFunctions = other.dgetDisableFunctions();
      AirportBridgeDeviceName = other.dgetAirportBridgeDeviceName();
      Audio.takeValueFrom(other.Audio);
      USB.takeValueFrom(other.USB);
      FakeID.takeValueFrom(other.FakeID);
      AddPropertyArray.takeValueFrom(other.AddProperties);
      Properties.takeValueFrom(other.Properties);
      ArbitraryArray.takeValueFrom(other.Arbitrary);
    }

      // 2021-04 : Following is temporary to compare with old way of storing properties.
      // Let's keep it few months until I am sure the refactoring isomorphic
    private:
      DEV_PROPERTY                *ArbProperties = 0; // the old version.
    public:
      void FillDevicePropertiesOld(SETTINGS_DATA& gSettings, const TagDict* DevicesDict);
      
      XBool compareDevProperty(const XString8& label, const DEV_PROPERTY& oldProp, const DEV_PROPERTY& newProp) const
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
      
      XBool compareOldAndCompatibleArb()
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
          XBool        enabled = false;
          
  #if __cplusplus > 201703L
          XBool operator == (const MMIOWhiteList&) const = default;
  #endif
          XBool isEqual(const MMIOWhiteList& other) const
          {
            if ( !(address == other.address) ) return false;
            if ( !(comment == other.comment) ) return false;
            if ( !(enabled == other.enabled) ) return false;
            return true;
          }
        void takeValueFrom(const ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class& other)
          {
            address = other.dgetaddress();
            comment = other.dgetcomment();
            enabled = other.dgetenabled();
          }
      };
      class OcKernelQuirksClass
      {
        public:
        //  XBool AppleCpuPmCfgLock = false;
        //  XBool AppleXcpmCfgLock = false;
          XBool AppleXcpmExtraMsrs = false;
          XBool AppleXcpmForceBoost = false;
        //  XBool CustomSmbiosGuid = false;
          XBool DisableIoMapper = false;
          XBool DisableLinkeditJettison = false;
        //  XBool DisableRtcChecksum = false;
          XBool DummyPowerManagement = false;
          XBool ExtendBTFeatureFlags = false;
          XBool ExternalDiskIcons = false;
          XBool IncreasePciBarSize = false;
        //  XBool LapicKernelPanic = false;
        //  XBool PanicNoKextDump = false;
          XBool PowerTimeoutKernelPanic = false;
          XBool ThirdPartyDrives = false;
          XBool XhciPortLimit = false;

          
#if __cplusplus > 201703L
          XBool operator == (const OcKernelQuirksClass&) const = default;
#endif
          XBool isEqual(const OcKernelQuirksClass& other) const
          {
            if ( !(AppleXcpmExtraMsrs == other.AppleXcpmExtraMsrs) ) return false;
            if ( !(AppleXcpmForceBoost == other.AppleXcpmForceBoost) ) return false;
            if ( !(DisableIoMapper == other.DisableIoMapper) ) return false;
            if ( !(DisableLinkeditJettison == other.DisableLinkeditJettison) ) return false;
            if ( !(DummyPowerManagement == other.DummyPowerManagement) ) return false;
            if ( !(ExtendBTFeatureFlags == other.ExtendBTFeatureFlags) ) return false;
            if ( !(ExternalDiskIcons == other.ExternalDiskIcons) ) return false;
            if ( !(IncreasePciBarSize == other.IncreasePciBarSize) ) return false;
            if ( !(PowerTimeoutKernelPanic == other.PowerTimeoutKernelPanic) ) return false;
            if ( !(ThirdPartyDrives == other.ThirdPartyDrives) ) return false;
            if ( !(XhciPortLimit == other.XhciPortLimit) ) return false;

            return true;
          }
          void takeValueFrom(const ConfigPlistClass::Quirks_Class::OcKernelQuirks_Class& other)
          {
            AppleXcpmExtraMsrs = other.dgetAppleXcpmExtraMsrs();
            AppleXcpmForceBoost = other.dgetAppleXcpmForceBoost();
            DisableIoMapper = other.dgetDisableIoMapper();
            DisableLinkeditJettison = other.dgetDisableLinkeditJettison();
            DummyPowerManagement = other.dgetDummyPowerManagement();
            ExtendBTFeatureFlags = other.dgetExtendBTFeatureFlags();
            ExternalDiskIcons = other.dgetExternalDiskIcons();
            IncreasePciBarSize = other.dgetIncreasePciBarSize();
            PowerTimeoutKernelPanic = other.dgetPowerTimeoutKernelPanic();
            ThirdPartyDrives = other.dgetThirdPartyDrives();
            XhciPortLimit = other.dgetXhciPortLimit();

          }
      };
    
      class OcBooterQuirksClass
      {
       public:
        XBool AvoidRuntimeDefrag = false;
        XBool DevirtualiseMmio = false;
        XBool DisableSingleUser = false;
        XBool DisableVariableWrite = false;
        XBool DiscardHibernateMap = false;
        XBool EnableSafeModeSlide = false;
        XBool EnableWriteUnprotector = false;
        XBool ForceExitBootServices = false;
        XBool ProtectMemoryRegions = false;
        XBool ProtectSecureBoot = false;
        XBool ProtectUefiServices = false;
        XBool ProvideCustomSlide = false;
        uint8_t ProvideMaxSlide = 0;
        XBool RebuildAppleMemoryMap = false;
        XBool SetupVirtualMap = false;
        XBool SignalAppleOS = false;
        XBool SyncRuntimePermissions = false;
        int8_t ResizeAppleGpuBars = 0; // 0 is NOT the default value if not set in config.plist. Default value if not set is returned by dgetResizeAppleGpuBars()
        XBool ForceOcWriteFlash = false;
        
#if __cplusplus > 201703L
        XBool operator == (const OcBooterQuirksClass&) const = default;
#endif
        XBool isEqual(const OcBooterQuirksClass& other) const
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
          if ( !(ResizeAppleGpuBars == other.ResizeAppleGpuBars) ) return false;
          if ( !(SetupVirtualMap == other.SetupVirtualMap) ) return false;
          if ( !(SignalAppleOS == other.SignalAppleOS) ) return false;
          if ( !(SyncRuntimePermissions == other.SyncRuntimePermissions) ) return false;
          if ( !(ForceOcWriteFlash == other.ForceOcWriteFlash) ) return false;
          return true;
        }
        void takeValueFrom(const ConfigPlistClass::Quirks_Class::OcBooterQuirks_Class& other)
        {
          AvoidRuntimeDefrag = other.dgetAvoidRuntimeDefrag();
          DevirtualiseMmio = other.dgetDevirtualiseMmio();
          DisableSingleUser = other.dgetDisableSingleUser();
          DisableVariableWrite = other.dgetDisableVariableWrite();
          DiscardHibernateMap = other.dgetDiscardHibernateMap();
          EnableSafeModeSlide = other.dgetEnableSafeModeSlide();
          EnableWriteUnprotector = other.dgetEnableWriteUnprotector();
          ForceExitBootServices = other.dgetForceExitBootServices();
          ProtectSecureBoot = other.dgetProtectSecureBoot();
          ProtectUefiServices = other.dgetProtectUefiServices();
          ProtectUefiServices = other.dgetProtectUefiServices();
          ProvideCustomSlide = other.dgetProvideCustomSlide();
          ProvideMaxSlide = other.dgetProvideMaxSlide();
          RebuildAppleMemoryMap = other.dgetRebuildAppleMemoryMap();
          ResizeAppleGpuBars = other.dgetResizeAppleGpuBars();
          SetupVirtualMap = other.dgetSetupVirtualMap();
          SignalAppleOS = other.dgetSignalAppleOS();
          SyncRuntimePermissions = other.dgetSyncRuntimePermissions();
          ForceOcWriteFlash = other.dgetForceOcWriteFlash();
        }

      };
      
      XBool                    FuzzyMatch = XBool();
      XString8                 OcKernelCache = XString8();
//      UINTN MaxSlide;
      OcKernelQuirksClass         OcKernelQuirks = OcKernelQuirksClass();
      OcBooterQuirksClass         OcBooterQuirks = OcBooterQuirksClass();
      XObjArrayWithTakeValueFromXmlArray<MMIOWhiteList, ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class> mmioWhiteListArray = XObjArrayWithTakeValueFromXmlArray<MMIOWhiteList, ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class>();
      UINT32                   QuirksMask = 0;
    
#if __cplusplus > 201703L
      XBool operator == (const QuirksClass&) const = default;
#endif
      XBool isEqual(const QuirksClass& other) const
      {
        if ( !(FuzzyMatch == other.FuzzyMatch) ) return false;
        if ( !(OcKernelCache == other.OcKernelCache) ) return false;
        if ( !(OcKernelQuirks.isEqual(other.OcKernelQuirks)) ) return false;
        if ( !(OcBooterQuirks.isEqual(other.OcBooterQuirks)) ) return false;
        if ( !mmioWhiteListArray.isEqual(other.mmioWhiteListArray) ) return false;
        if ( !(QuirksMask == other.QuirksMask) ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::Quirks_Class& other)
      {
        FuzzyMatch = other.dgetFuzzyMatch();
        OcKernelCache = other.dgetOcKernelCache();
        OcKernelQuirks.takeValueFrom(other.OcKernelQuirks);
        OcBooterQuirks.takeValueFrom(other.OcBooterQuirks);
        mmioWhiteListArray.takeValueFrom(other.MmioWhitelist);
        QuirksMask = other.dgetQuirksMask();
      }
};

  class RtVariablesClass {
    public:
      class RT_VARIABLES
      {
        public:
          XBool    Disabled = XBool();
          XString8 Comment = XStringW();
          XStringW Name = XStringW();
          EFI_GUIDClass Guid = EFI_GUIDClass();

#if __cplusplus > 201703L
          XBool operator == (const RT_VARIABLES&) const = default;
#endif
          XBool isEqual(const RT_VARIABLES& other) const
          {
            if ( !(Disabled == other.Disabled) ) return false;
            if ( !(Comment == other.Comment) ) return false;
            if ( !(Name == other.Name) ) return false;
            if ( memcmp(&Guid, &other.Guid, sizeof(Guid)) != 0 ) return false;
            return true;
          }
          void takeValueFrom(const ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block& other)
          {
            Disabled = other.dgetDisabled();
            Comment = other.dgetComment();
            Name = other.dgetName();
            Guid = other.dgetGuid();
          }
      };
        
      XString8                RtROMAsString = XString8();
      XBuffer<UINT8>          RtROMAsData = XBuffer<UINT8>();
      XString8                RtMLBSetting = XString8();
      UINT32                  CsrActiveConfig = UINT32();
      UINT16                  BooterConfig = UINT16();
      XString8                BooterCfgStr = XString8();
      XString8                HWTarget = XString8();
      XObjArrayWithTakeValueFromXmlArray<RT_VARIABLES, ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block> BlockRtVariableArray = XObjArrayWithTakeValueFromXmlArray<RT_VARIABLES, ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block>();

      XBool GetLegacyLanAddress() const {
        return RtROMAsString.isEqualIC("UseMacAddr0") || RtROMAsString.isEqualIC("UseMacAddr1");
      }
    
#if __cplusplus > 201703L
    XBool operator == (const RtVariablesClass&) const = default;
#endif
    XBool isEqual(const RtVariablesClass& other) const
    {
      if ( !(RtROMAsString == other.RtROMAsString) ) return false;
      if ( !(RtROMAsData == other.RtROMAsData) ) return false;
      if ( !(RtMLBSetting == other.RtMLBSetting) ) return false;
      if ( !(CsrActiveConfig == other.CsrActiveConfig) ) return false;
      if ( !(BooterConfig == other.BooterConfig) ) return false;
      if ( !(BooterCfgStr == other.BooterCfgStr) ) return false;
      if ( !(HWTarget == other.HWTarget) ) return false;
      if ( !BlockRtVariableArray.isEqual(other.BlockRtVariableArray) ) return false;
      return true;
    }
    void takeValueFrom(const ConfigPlistClass::RtVariables_Class& other)
    {
      RtROMAsString = other.dgetRtROMAsString();
      RtROMAsData = other.dgetRtROMAsData();
      RtMLBSetting = other.dgetRtMLBSetting();
      CsrActiveConfig = other.dgetCsrActiveConfig();
      BooterConfig = other.dgetBooterConfig();
      BooterCfgStr = other.dgetBooterCfgStr();
      BlockRtVariableArray.takeValueFrom(other.Block);
      HWTarget = other.dgetHWTarget();
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
              XBool operator == (const SLOT_DEVICE&) const = default;
#endif
              XBool isEqual(const SlotDeviceClass& other) const
              {
                if ( !(SmbiosIndex == other.SmbiosIndex) ) return false;
                if ( !(SlotID == other.SlotID) ) return false;
                if ( !(SlotType == other.SlotType) ) return false;
                if ( !(SlotName == other.SlotName) ) return false;
                return true;
              }
              void takeValueFrom(const SmbiosPlistClass::SmbiosDictClass::SlotDeviceDictClass& other)
              {
                SmbiosIndex = other.dgetDeviceN();
                SlotID = other.dgetSlotID();
                SlotType = other.dgetSlotType();
                SlotName = other.dgetSlotName();
              }
        };
        
        class SlotDeviceArrayClass : public XObjArrayWithTakeValueFromXmlArray<SlotDeviceClass, SmbiosPlistClass::SmbiosDictClass::SlotDeviceDictClass>
        {
          public:
            XBool doesSlotForIndexExist(uint8_t idx2Look4) const {
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
          UINT64  SlotIndex = UINT64();
          UINT32  ModuleSize = UINT32();
          UINT32  Frequency = UINT32();
          XString8 Vendor = XString8();
          XString8 PartNo = XString8();
          XString8 SerialNo = XString8();
          UINT8   Type = UINT8();

          RamSlotInfo() {}

          #if __cplusplus > 201703L
            XBool operator == (const RamSlotInfo&) const = default;
          #endif
          XBool isEqual(const RamSlotInfo& other) const
          {
            if ( !(SlotIndex == other.SlotIndex ) ) return false;
            if ( !(ModuleSize == other.ModuleSize ) ) return false;
            if ( !(Frequency == other.Frequency ) ) return false;
            if ( !(Vendor == other.Vendor ) ) return false;
            if ( !(PartNo == other.PartNo ) ) return false;
            if ( !(SerialNo == other.SerialNo ) ) return false;
            if ( !(Type == other.Type ) ) return false;
            return true;
          }
          XBool takeValueFrom(const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleDictClass& other)
          {
            SlotIndex = other.dgetSlotIndex();
            ModuleSize = other.dgetModuleSize();
            Frequency = other.dgetFrequency();
            Vendor = other.dgetVendor();
            PartNo = other.dgetPartNo();
            SerialNo = other.dgetSerialNo();
            Type = other.dgetType();
            return true;
          }
        };

        class RamSlotInfoArrayClass {
          public:
            UINT8         SlotCount = UINT8();
            UINT8         UserChannels = UINT8();
            XObjArrayWithTakeValueFromXmlArray<RamSlotInfo, SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleDictClass> User = XObjArrayWithTakeValueFromXmlArray<RamSlotInfo, SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleDictClass>();

            RamSlotInfoArrayClass() {}

#if __cplusplus > 201703L
            XBool operator == (const RamSlotInfoArrayClass&) const = default;
#endif
            XBool isEqual(const RamSlotInfoArrayClass& other) const
            {
              if ( !(SlotCount == other.SlotCount) ) return false;
              if ( !(UserChannels == other.UserChannels) ) return false;
              if ( !(User.isEqual(other.User)) ) return false;
              return true;
            }
            void takeValueFrom(const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass& other)
            {
              SlotCount = other.dgetSlotCount();
              UserChannels = other.dgetUserChannels();
              User.takeValueFrom(other.Modules);
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
      XBool                   Mobile = XBool();
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
      XBool                   TrustSMBIOS = false;
      XBool                   InjectMemoryTables = XBool(); // same as Memory.SlotCounts
      // SMBIOS TYPE133
      UINT64                  gPlatformFeature = UINT64();
      // PatchTableType11
      XBool                   NoRomInfo = XBool();

      UINT32                  FirmwareFeatures = UINT32();
      UINT32                  FirmwareFeaturesMask = UINT32();
      UINT64                  ExtendedFirmwareFeatures = UINT64();
      UINT64                  ExtendedFirmwareFeaturesMask = UINT64();
      RamSlotInfoArrayClass   RamSlotInfoArray = RamSlotInfoArrayClass();
      SlotDeviceArrayClass    SlotDevices = SlotDeviceArrayClass();

    SmbiosClass() {}
    
#if __cplusplus > 201703L
    XBool operator == (const SmbiosClass&) const = default;
#endif
    XBool isEqual(const SmbiosClass& other) const
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
      if ( !(ExtendedFirmwareFeatures == other.ExtendedFirmwareFeatures) ) return false;
      if ( !(ExtendedFirmwareFeaturesMask == other.ExtendedFirmwareFeaturesMask) ) return false;
      if ( !RamSlotInfoArray.isEqual(other.RamSlotInfoArray) ) return false;
      if ( !SlotDevices.isEqual(other.SlotDevices) ) return false;

//      if ( memcmp(RPlt, other.RPlt, sizeof(RPlt)) != 0 ) return false;
//      if ( memcmp(RBr, other.RBr, sizeof(RBr)) != 0 ) return false;
//      if ( memcmp(EPCI, other.EPCI, sizeof(EPCI)) != 0 ) return false;
//      if ( memcmp(REV, other.REV, sizeof(REV)) != 0 ) return false;
      return true;
    }
    void takeValueFrom(const SmbiosPlistClass::SmbiosDictClass& other)
    {
      // SMBIOS TYPE0
      BiosVendor = other.dgetBiosVendor();
      BiosVersion = other.dgetBiosVersion();
      EfiVersion = other.dgetEfiVersion();
      BiosReleaseDate = other.dgetBiosReleaseDate();
      // SMBIOS TYPE1
      ManufactureName = other.dgetManufactureName();
      ProductName = other.dgetProductName();
      SystemVersion = other.dgetSystemVersion();
      SerialNr = other.dgetSerialNr();
      SmUUID = other.dgetSmUUID();
      FamilyName = other.dgetFamilyName();
      // SMBIOS TYPE2
      BoardManufactureName = other.dgetBoardManufactureName();
      BoardSerialNumber = other.dgetBoardSerialNumber();
      BoardNumber = other.dgetBoardNumber();
      LocationInChassis = other.dgetLocationInChassis();
      BoardVersion = other.dgetBoardVersion();
      BoardType = other.dgetBoardType();
      // SMBIOS TYPE3
      Mobile = other.dgetMobile();
      ChassisType = other.dgetChassisType();
      ChassisManufacturer = other.dgetChassisManufacturer();
      ChassisAssetTag = other.dgetChassisAssetTag();
      // SMBIOS TYPE17
      SmbiosVersion = other.dgetSmbiosVersion();
      Attribute = other.dgetAttribute();
      // SMBIOS TYPE132
      TrustSMBIOS = other.dgetTrustSMBIOS();
      InjectMemoryTables = other.dgetInjectMemoryTables();
      // SMBIOS TYPE133
      gPlatformFeature = other.dgetgPlatformFeature();
      // PatchTableType11
      NoRomInfo = other.dgetNoRomInfo();
      //SMBIOS TYPE128
      FirmwareFeatures = other.dgetFirmwareFeatures();
      FirmwareFeaturesMask = other.dgetFirmwareFeaturesMask();
      ExtendedFirmwareFeatures = other.dgetExtendedFirmwareFeatures();
      ExtendedFirmwareFeaturesMask = other.dgetExtendedFirmwareFeaturesMask();
      RamSlotInfoArray.takeValueFrom(other.Memory);
      SlotDevices.takeValueFrom(other.Slots);
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
//        flagstate.memset(0, 32);
      }
      
#if __cplusplus > 201703L
      XBool operator == (const BootGraphicsClass&) const = default;
#endif
      XBool isEqual(const BootGraphicsClass& other) const
      {
        if ( !(DefaultBackgroundColor == other.DefaultBackgroundColor) ) return false;
        if ( !(UIScale == other.UIScale) ) return false;
        if ( !(EFILoginHiDPI == other.EFILoginHiDPI) ) return false;
        if ( _flagstate != other._flagstate ) return false;
        return true;
      }
      void takeValueFrom(const ConfigPlistClass::BootGraphics_Class& other)
      {
        DefaultBackgroundColor = other.dgetDefaultBackgroundColor();
        UIScale = other.dgetUIScale();
        EFILoginHiDPI = other.dgetEFILoginHiDPI();
        _flagstate = other.dget_flagstate();
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
//  XBool                LpcTune; // never set to true.

  SETTINGS_DATA() {}
//  SETTINGS_DATA(const SETTINGS_DATA& other) = delete; // Can be defined if needed
//  const SETTINGS_DATA& operator = ( const SETTINGS_DATA & ) = delete; // Can be defined if needed

#if __cplusplus > 201703L
  XBool operator == (const SETTINGS_DATA&) const = default;
#endif
  XBool isEqual(const SETTINGS_DATA& other) const
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

  void takeValueFrom(const ConfigPlistClass& other)
  {
    Boot.takeValueFrom(other.Boot);
    ACPI.takeValueFrom(other.ACPI);
    GUI.takeValueFrom(other.GUI);
    CPU.takeValueFrom(other.CPU);
    SystemParameters.takeValueFrom(other.SystemParameters);
    KernelAndKextPatches.takeValueFrom(other.KernelAndKextPatches);
    Graphics.takeValueFrom(other.Graphics);
    DisabledDriverArray = other.dgetDisabledDriverArray();
    Quirks.takeValueFrom(other.Quirks);
    RtVariables.takeValueFrom(other.RtVariables);
    Devices.takeValueFrom(other.Devices);
    Smbios.takeValueFrom(other.getSMBIOS());
    BootGraphics.takeValueFrom(other.BootGraphics);
  }

  ~SETTINGS_DATA() {}

  const XString8& getUUID();
  const XString8& getUUID(EFI_GUIDClass* efiGuid);
  // If CustomUuid is defined, return false by default
  // If SmUUID is defined, return true by default.
  XBool ShouldInjectSystemID() {
    if ( SystemParameters.CustomUuid.notEmpty() &&  SystemParameters.CustomUuid != nullGuidAsString ) {
      if ( SystemParameters._InjectSystemID == 2 ) return false;
      else return SystemParameters._InjectSystemID != 0;
    }
    if ( Smbios.SmUUID.isEmpty() || Smbios.SmUUID == nullGuidAsString ) return false;
    if ( SystemParameters._InjectSystemID == 2 ) return true;
    return SystemParameters._InjectSystemID != 0;
  }
  
  XBool getEnableC6() const {
    if ( CPU._EnableC6.isDefined() ) return CPU._EnableC6.value();
    return ACPI.SSDT._EnableC6;
  }
  XBool getEnableC4() const {
    if ( CPU._EnableC4.isDefined() ) return CPU._EnableC4.value();
    return ACPI.SSDT._EnableC4;
  }
  XBool getEnableC2() const {
    if ( CPU._EnableC2.isDefined() ) return CPU._EnableC2.value();
    return ACPI.SSDT._EnableC2;
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
extern XStringWArray ConfigsList;
extern XStringWArray DsdtsList;
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
extern XBool                       SavePreBootLog;
extern UINT8                            DefaultAudioVolume;


//extern GFX_PROPERTIES                 gGraphics[];
//extern HDA_PROPERTIES                 gAudios[];
//extern UINTN                          NGFX;
//extern UINTN                          NHDA;
//extern UINT16                         gCPUtype;
extern SETTINGS_DATA                  gSettings;
extern XBool                       gFirmwareClover;
extern DRIVERS_FLAGS                  gDriversFlags;
extern EFI_EDID_DISCOVERED_PROTOCOL   *EdidDiscovered;
//extern UINT8                          *gEDID;

extern UINTN                           gEvent;

extern UINT16                          gBacklightLevel;

//extern XBool                        defDSM;
//extern UINT16                          dropDSM;

//extern TagDict*                          gConfigDict[];

// ACPI/PATCHED/AML
extern XObjArray<ACPI_PATCHED_AML>       ACPIPatchedAML;


// SysVariables
//extern SYSVARIABLES                   *SysVariables;

// Hold theme fixed IconFormat / extension
extern CHAR16                         *IconFormat;


extern XBool                       ResumeFromCoreStorage;
//extern XBool                       gRemapSmBiosIsRequire;  // syscl: pass argument for Dell SMBIOS here

extern EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl;


//
// config module
//

class REFIT_CONFIG
{
public:
  UINTN       DisableFlags = 0; //to disable some volume types (optical, firewire etc)
  XBool       Quiet = true;
  XBool       SpecialBootMode = false; // content of nvram var "aptiofixflag"

  XBool       gBootChanged = false;
  XBool       gThemeChanged = false;
  XBool       NeedPMfix = false;
  ACPI_DROP_TABLE         *ACPIDropTables = NULL;

  UINT8                   CustomLogoType = 0; // this will be initialized with gSettings.Boot.CustomBoot and set back to CUSTOM_BOOT_DISABLED if CustomLogo could not be loaded or decoded (see afterGetUserSettings)
  XImage                  *CustomLogo = 0;

  XBool                   DropSSDT = false; // init with gSettings.Boot.DropSSDTSetting. Put back to false if one table is dropped (see afterGetUserSettings)

  UINT8                   SecureBoot = 0;
  UINT8                   SecureBootSetupMode = 0;

  XBool                   SetTable132 = false;
  XBool                   HWP = false;

  XBool                   EnableC6 = false;
  XBool                   EnableC4 = false;
  XBool                   EnableC2 = false;
  uint16_t                C3Latency = 0;

  XObjArray<CUSTOM_LOADER_ENTRY> CustomEntries = XObjArray<CUSTOM_LOADER_ENTRY>();
  XObjArray<CUSTOM_LEGACY_ENTRY> CustomLegacyEntries = XObjArray<CUSTOM_LEGACY_ENTRY>();
  XObjArray<CUSTOM_TOOL_ENTRY> CustomToolsEntries = XObjArray<CUSTOM_TOOL_ENTRY>();

  INTN                    Codepage = 0xC0;
  INTN                    CodepageSize = 0xC0;

  XBool                   KPKernelPm = XBool();
  XBool                   KPAppleIntelCPUPM = XBool();

  XBuffer<UINT8>          RtROM = XBuffer<UINT8>();
  XString8                RtMLB = XString8();

  XBool Turbo = true;

  XString8                OEMProductFromSmbios = XString8();
  XString8                OEMVendorFromSmbios = XString8();
  XString8                OEMBoardFromSmbios = XString8();
  UINT8                   EnabledCores = 0;

//  XStringW                   ConfigName; // Set but never used

  UINT32                  OptionsBits = 0;
  UINT32                  FlagsBits = 0;

  XStringW                    BlockKexts = XStringW();
  // KernelAndKextPatches
  XBool                   KextPatchesAllowed = true;
  XBool                   KernelPatchesAllowed = true; //From GUI: Only for user patches, not internal Clover
  
  UINT8  flagstate[32] = {0};
  MacModel CurrentModel = MaxMacModel;

  UINT32               IgPlatform = UINT32(); //could also be snb-platform-id


  REFIT_CONFIG() {};
  REFIT_CONFIG(const REFIT_CONFIG& other) = delete; // Can be defined if needed
  const REFIT_CONFIG& operator = ( const REFIT_CONFIG & ) = delete; // Can be defined if needed
  ~REFIT_CONFIG() {  }

  XBool isFastBoot() { return SpecialBootMode || gSettings.Boot.FastBoot; }

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
GetOtherKextsDir (XBool On);

XStringW GetOSVersionKextsDir(const MacOsVersion& OSVersion);

EFI_STATUS
InjectKextsFromDir (
  EFI_STATUS Status,
  CHAR16 *SrcDir
  );


EFI_STATUS
ApplySettings(void);


#endif

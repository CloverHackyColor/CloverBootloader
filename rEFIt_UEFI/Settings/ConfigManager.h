/*
 * ConfigManager.h
 *
 *  Created on: Apr 21, 2021
 *      Author: jief
 */

#ifndef PLATFORM_CONFIGMANAGER_H_
#define PLATFORM_CONFIGMANAGER_H_

#include "ConfigPlist/ConfigPlistClass.h"
#include "ConfigPlist/SMBIOSPlist.h"
#include "../Platform/hda.h" // for HRDW_MANUFACTERER


class DiscoveredSlotDeviceClass
{
public:
  uint8_t           Index = 0xFF;
  UINT16            SegmentGroupNum = 0;
  UINT8             BusNum = 0;
  UINT8             DevFuncNum = 0;
  UINT8             SlotID = 0;
  MISC_SLOT_TYPE    SlotType = MISC_SLOT_TYPE();
  XString8          SlotName = XString8();

  DiscoveredSlotDeviceClass() {}

};

class SlotDeviceArrayClass : public XObjArray<DiscoveredSlotDeviceClass>
{
public:
  SlotDeviceArrayClass() {}
};



class DiscoveredGfx
{
public:
  HRDW_MANUFACTERER  Vendor = Unknown;
  UINT8             Ports = 0;
  UINT16            DeviceID = 0;
  UINT16            Family = 0;
  XString8          Model = XString8();
  XString8          Config = XString8();
  UINTN             Segment = 0;
  UINTN             Bus = 0;
  UINTN             Device = 0;
  UINTN             Function = 0;
  EFI_HANDLE        Handle = 0;
  UINT8             *Mmio = 0;
  UINT32            Connectors = 0;
  BOOLEAN           ConnChanged = 0;

  // ATTENTION : this is not discovered. This will be assigned once config plist is read.
  bool LoadVBios = 0;

  DiscoveredGfx() {}
  DiscoveredGfx(const DiscoveredGfx&) = default; // default copy is ok because we can copy Mmio, because it's not allocated and still make sense once copied.
  DiscoveredGfx& operator = (const DiscoveredGfx&) = default;
};

class GfxPropertiesArrayClass : public XObjArray<DiscoveredGfx>
{
public:

  
  bool hasBrand(HRDW_MANUFACTERER brand) const {
    for ( size_t idx = 0 ; idx < size() ; ++idx ) {
      if ( ElementAt(idx).Vendor == brand ) return true;
    }
    return false;
  }
  

  bool hasNvidia() const { return hasBrand(Nvidia); }
  bool hasIntel() const { return hasBrand(Intel); }
  
  bool isCardAtPosIntel(size_t pos) const { return size() > pos && ElementAt(pos).Vendor == Intel; }
  bool isCardAtPosNvidia(size_t pos) const { return size() > pos && ElementAt(pos).Vendor == Nvidia; }

};



class DiscoveredHdaProperties
{
public:
  UINT16            controller_vendor_id = 0;
  UINT16            controller_device_id = 0;
  CHAR16            *controller_name = 0;

  DiscoveredHdaProperties() {}
};

class HdaPropertiesArrayClass : public XObjArray<DiscoveredHdaProperties>
{
};



class LanCardClass
{
  public:
    UINT8     MacAddress[6] = {0};  // MAC address

    LanCardClass() {}
};

class LanCardArrayClass : public XObjArray<LanCardClass>
{
public:
  /* Requirment : MacAddressToLookFor is 6 chars long */
  bool containsMacAddress(const UINT8* MacAddressToLookFor) const {
    for ( size_t idx = 0 ; idx < size() ; ++idx ) {
      if ( memcmp(MacAddressToLookFor, ElementAt(idx).MacAddress, 6) == 0 ) return true;
    }
    return false;
  }
};






class ConfigManager
{
protected:
  /* this is for internal usage */
  ConfigPlistClass configPlist = ConfigPlistClass(); // current config.plist. Values are "transfered" into SETTINGS_DATA
  SmbiosPlistClass smbiosPlist = SmbiosPlistClass();
  

public:
  /*
   * For now, the non-const version are exposed to public.
   * The goal is that non modification is made from outside of this object.
   * Refactoring is in progress and this variable names make it easier to track down where changes happen
   */

  // Discovered hardware. This will be used to create the data to patch Smbios, for example.
  SlotDeviceArrayClass SlotDeviceArrayNonConst = SlotDeviceArrayClass();
  GfxPropertiesArrayClass GfxPropertiesArrayNonConst = GfxPropertiesArrayClass();
  HdaPropertiesArrayClass HdaPropertiesArrayNonConst = HdaPropertiesArrayClass();
  LanCardArrayClass LanCardArrayNonConst = LanCardArrayClass();

  /*
   * Const version of above members. The ones to mainly use. Eventually the ones to ONLY use.
   * The goal is that non modification is made from outside of this object.
   */
  const SlotDeviceArrayClass& SlotDeviceArray = SlotDeviceArrayNonConst;
  const GfxPropertiesArrayClass& GfxPropertiesArray = GfxPropertiesArrayNonConst;
  const HdaPropertiesArrayClass& HdaPropertiesArray = HdaPropertiesArrayNonConst;
  const LanCardArrayClass& LanCardArray = LanCardArrayNonConst;

  ConfigManager () {};
  ~ConfigManager () {};

  ConfigManager (const ConfigManager &other) = delete;
  ConfigManager (ConfigManager &&other) = delete;
  ConfigManager& operator= (const ConfigManager &other) = delete;
  ConfigManager& operator= (ConfigManager &&other) = delete;

  void FillSmbiosWithDefaultValue(MACHINE_TYPES Model, const SmbiosPlistClass::SmbiosDictClass& smbiosDictClass);

  /*
   * Look for {ConfName}.plist and smbios.plist and load them, "transfer" the settings into gSettings and call afterGetUserSettings()
   * ConfName : name of the file, without .plist extension. File will be searched in OEM or main folder
   * This is for live reload (from the menu) of a new config.plist.
   * 2021-04 : not really tested yet.
   */
  EFI_STATUS ReLoadConfig(const XStringW& ConfName);

  /*
   * Populate SlotDeviceArray, GfxPropertiesArray, HdaPropertiesArray, LanCardArray
   * Only call once in the lifetime
   */
  EFI_STATUS InitialisePlatform();

protected:
  void DiscoverDevices();
  void is2ndGreaterThen1st (const CHAR8 *i, const CHAR8 *j);
  void applySettings() const;
  void GetUEFIMacAddress();
  EFI_STATUS LoadConfigPlist(const XStringW& ConfName);
  EFI_STATUS LoadSMBIOSPlist(const XStringW& ConfName);
  EFI_STATUS LoadConfig(const XStringW& ConfName);
};

#if !defined(DONT_DEFINE_GLOBALS)
extern ConfigManager                   gConf;
#endif

#endif /* PLATFORM_CONFIGMANAGER_H_ */

/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsBoot.h"
#include <Platform.h>
#include "AssignField.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/SMBIOSPlist.h"

void AssignMemoryUserSlot(const XString8& label, RAM_SLOT_INFO& oldS, const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleDictClass& newS)
{
  Assign(ModuleSize);
  Assign(Frequency);
  Assign(Vendor);
  Assign(PartNo);
  Assign(SerialNo);
  Assign(Type);
  Assign(InUse);
}

void AssignMemoryUser(const XString8& label, XObjArray<RAM_SLOT_INFO>& oldS, size_t count, const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleArrayClass& newS)
{
    for ( size_t idx = 0 ; idx < MAX_RAM_SLOTS ; ++idx )
    {
      AssignMemoryUserSlot(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS.dgetSolt(idx));
    }
}

void AssignMemory(const XString8& label, SETTINGS_DATA::SmbiosClass::MemoryClass& oldS, const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass& newS)
{
  Assign(SlotCounts);
  Assign(UserChannels);
  AssignMemoryUser(S8Printf("%s.Memory", label.c_str()), oldS.User, oldS.SlotCounts, newS.Modules);

}

//-------------------------------------------------------------------------------------------------------

void AssignSlotDevice(const XString8& label, SLOT_DEVICE& oldS, const SmbiosPlistClass::SmbiosDictClass::SlotDeviceDictClass& newS)
{
  Assign(SlotID);
  Assign(SlotType);
  Assign(SlotName);
  // othger field are assigned by GetDevices after settings are read.
}

void AssignSlotDevices(const XString8& label, XObjArray<SLOT_DEVICE>& oldS, const SmbiosPlistClass::SmbiosDictClass::SlotDeviceArrayClass& newS)
{
    for ( size_t idx = 0 ; idx < 16 ; ++idx )
    {
      AssignSlotDevice(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS.dgetSoltDevice(idx));
    }
}

//-------------------------------------------------------------------------------------------------------

#define AssignSMBIOS(oldField, newField) if ( smbiosPlist.isDefined() && smbiosPlist.SMBIOS.get##newField().isDefined() ) oldS.oldField = smbiosPlist.SMBIOS.dget##oldField(); else oldS.oldField = newS.dget##oldField();

void AssignSmbios(const XString8& label, SETTINGS_DATA::SmbiosClass& oldS, const SmbiosPlistClass::SmbiosDictClass& newS, const SmbiosPlistClass& smbiosPlist)
{
  if ( smbiosPlist.isDefined() && smbiosPlist.SMBIOS.getBiosVendor().isDefined() ) oldS.BiosVendor = smbiosPlist.SMBIOS.dgetBiosVendor();
  else oldS.BiosVendor = newS.dgetBiosVendor();
  AssignSMBIOS(_RomVersion, BiosVersion);
  AssignSMBIOS(_EfiVersion, EfiVersion);
  AssignSMBIOS(_ReleaseDate, BiosReleaseDate);
  AssignSMBIOS(ManufactureName, Manufacturer);
  AssignSMBIOS(ProductName, ProductName);
  AssignSMBIOS(VersionNr, Version);
  AssignSMBIOS(SerialNr, SerialNumber);
  AssignSMBIOS(SmUUID, SmUUID);
  AssignSMBIOS(FamilyName, Family);
  AssignSMBIOS(BoardManufactureName, BoardManufacturer);
  AssignSMBIOS(BoardSerialNumber, BoardSerialNumber);
  AssignSMBIOS(BoardNumber, BoardID);
  AssignSMBIOS(LocationInChassis, LocationInChassis);
  AssignSMBIOS(BoardVersion, BoardVersion);
  AssignSMBIOS(BoardType, BoardType);
  if ( smbiosPlist.isDefined() && smbiosPlist.SMBIOS.getMobile().isDefined() ) oldS.Mobile = smbiosPlist.SMBIOS.dgetMobile(gMobile);
  else oldS.Mobile = newS.dgetMobile(gMobile);
  AssignSMBIOS(ChassisType, ChassisType);
  AssignSMBIOS(ChassisManufacturer, ChassisManufacturer);
  AssignSMBIOS(ChassisAssetTag, ChassisAssetTag);
  AssignSMBIOS(SmbiosVersion, SmbiosVersion);
  Assign(Attribute);
  Assign(TrustSMBIOS);
  if ( smbiosPlist.isDefined() && smbiosPlist.SMBIOS.getMobile().isDefined() ) oldS.Mobile = smbiosPlist.SMBIOS.dgetMobile(gMobile);
  else oldS.Mobile = newS.dgetMobile(gMobile);
  Assign(InjectMemoryTables);
  AssignSMBIOS(gPlatformFeature, PlatformFeature);
  AssignSMBIOS(NoRomInfo, NoRomInfo);
  AssignSMBIOS(gFwFeatures, FirmwareFeatures);
  AssignSMBIOS(gFwFeaturesMask, FirmwareFeaturesMask);
  AssignMemory(S8Printf("%s.Memory", label.c_str()), oldS.Memory, newS.Memory);
  AssignSlotDevices(S8Printf("%s.SlotDevices", label.c_str()), oldS.SlotDevices, newS.Slots);
}

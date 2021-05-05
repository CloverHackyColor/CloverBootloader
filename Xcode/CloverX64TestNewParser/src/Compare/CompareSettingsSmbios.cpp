/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsBoot.h"
#include <Platform.h>
#include "CompareField.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void CompareMemoryUserSlot(const XString8& label, const RAM_SLOT_INFO& oldS, const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleDictClass& newS)
{
  compare(ModuleSize);
  compare(Frequency);
  compare(Vendor);
  compare(PartNo);
  compare(SerialNo);
  compare(Type);
  compare(InUse);
}

void CompareMemoryUser(const XString8& label, const XObjArray<RAM_SLOT_INFO>& oldS, size_t count, const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass::ModuleArrayClass& newS)
{
    for ( size_t idx = 0 ; idx < MAX_RAM_SLOTS ; ++idx )
    {
      CompareMemoryUserSlot(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS.dgetSolt(idx));
    }
}

void CompareMemory(const XString8& label, const SETTINGS_DATA::SmbiosClass::MemoryClass& oldS, const SmbiosPlistClass::SmbiosDictClass::MemoryDictClass& newS)
{
  compare(SlotCounts);
  compare(UserChannels);
  CompareMemoryUser(S8Printf("%s.Memory", label.c_str()), oldS.User, oldS.SlotCounts, newS.Modules);

}

//-------------------------------------------------------------------------------------------------------

void CompareSlotDevice(const XString8& label, const SLOT_DEVICE& oldS, const SmbiosPlistClass::SmbiosDictClass::SlotDeviceDictClass& newS)
{
  compare(SlotID);
  compare(SlotType);
  compare(SlotName);
  // othger field are assigned by GetDevices after settings are read.
}

void CompareSlotDevices(const XString8& label, const XObjArray<SLOT_DEVICE>& oldS, const SmbiosPlistClass::SmbiosDictClass::SlotDeviceArrayClass& newS)
{
    for ( size_t idx = 0 ; idx < 16 ; ++idx )
    {
      CompareSlotDevice(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS.dgetSoltDevice(idx));
    }
}

//-------------------------------------------------------------------------------------------------------

void CompareSmbios(const XString8& label, const SETTINGS_DATA::SmbiosClass& oldS, const SmbiosPlistClass::SmbiosDictClass& newS)
{
  compare(BiosVendor);
  compare(_RomVersion);
  compare(_EfiVersion);
  compare(_ReleaseDate);
  compare(ManufactureName);
  compare(ProductName);
  compare(VersionNr);
  compare(SerialNr);
  compare(SmUUID);
  compare(FamilyName);
  compare(BoardManufactureName);
  compare(BoardSerialNumber);
  compare(BoardNumber);
  compare(LocationInChassis);
  compare(BoardVersion);
  compare(BoardType);
  compareField(oldS.Mobile, newS.dgetMobile(gMobile), S8Printf("%s.Mobile", label.c_str()));
  compare(ChassisType);
  compare(ChassisManufacturer);
  compare(ChassisAssetTag);
  compare(SmbiosVersion);
  compare(Attribute);
  compare(TrustSMBIOS);
  compare(InjectMemoryTables);
  compare(gPlatformFeature);
  compare(NoRomInfo);
  compare(gFwFeatures);
  compare(gFwFeaturesMask);
  CompareMemory(S8Printf("%s.Memory", label.c_str()), oldS.Memory, newS.Memory);
  CompareSlotDevices(S8Printf("%s.SlotDevices", label.c_str()), oldS.SlotDevices, newS.Slots);
//  char RBr[8];
//  newS.dgetRBr(RBr);
//  compareField((void*)oldS.RBr, 8, (void*)RBr, 8, S8Printf("%s.RBr", label.c_str()));
}

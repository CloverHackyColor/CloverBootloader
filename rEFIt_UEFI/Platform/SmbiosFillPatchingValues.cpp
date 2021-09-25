/*
 * SmbiosFillPatchingValues.cpp
 *
 *  Created on: Apr 28, 2021
 *      Author: jief
 */

#define DONT_DEFINE_GLOBALS

#include <Platform.h>
#include "SmbiosFillPatchingValues.h"
#include "../Platform/Settings.h"
#include "../Platform/smbios.h"

static void SmbiosFillPatchingValues(const DiscoveredSlotDeviceClass& other, SLOT_DEVICE* slotDevicePtr)
{
  SLOT_DEVICE& slotDevice = *slotDevicePtr;

  slotDevice.Index = other.Index;
  slotDevice.SegmentGroupNum = other.SegmentGroupNum;
  slotDevice.BusNum = other.BusNum;
  slotDevice.DevFuncNum = other.DevFuncNum;
  slotDevice.SlotID = other.SlotID;
  slotDevice.SlotType = other.SlotType;
  slotDevice.SlotName = other.SlotName;
}

static void SmbiosFillPatchingValues(const SETTINGS_DATA::SmbiosClass::SlotDeviceClass& other, SLOT_DEVICE* slotDevicePtr)
{
  SLOT_DEVICE& slotDevice = *slotDevicePtr;

  slotDevice.Index = other.SmbiosIndex;
  slotDevice.SlotID = other.SlotID;
  slotDevice.SlotType = other.SlotType;
  slotDevice.SlotName = other.SlotName;
}

static void SmbiosFillPatchingValues(const SETTINGS_DATA::SmbiosClass::RamSlotInfo& other, RAM_SLOT_INFO* ramSlotInfoPtr)
{
  RAM_SLOT_INFO& ramSlotInfo = *ramSlotInfoPtr;

  ramSlotInfo.Slot = other.Slot;
  ramSlotInfo.ModuleSize = other.ModuleSize;
  ramSlotInfo.Frequency = other.Frequency;
  ramSlotInfo.Vendor = other.Vendor;
  ramSlotInfo.PartNo = other.PartNo;
  ramSlotInfo.SerialNo = other.SerialNo;
  ramSlotInfo.Type = other.Type;
  ramSlotInfo.InUse = other.InUse;
}

static void SmbiosFillPatchingValues(const XObjArray<SETTINGS_DATA::SmbiosClass::RamSlotInfo>& settingRamSlotInfoArray, XObjArray<RAM_SLOT_INFO>* ramSlotInfoArrayPtr)
{
  XObjArray<RAM_SLOT_INFO>& ramSlotInfoArray = *ramSlotInfoArrayPtr;

  ramSlotInfoArray.setEmpty();
  for ( size_t idx = 0 ; idx < settingRamSlotInfoArray.size() ; ++idx ) {
    RAM_SLOT_INFO* ramSlotInfo = new RAM_SLOT_INFO;
    SmbiosFillPatchingValues(settingRamSlotInfoArray[idx], ramSlotInfo);
    ramSlotInfoArray.AddReference(ramSlotInfo, true);
  }
}

static void SmbiosFillPatchingValues(const SETTINGS_DATA::SmbiosClass::RamSlotInfoArrayClass& other, SmbiosMemoryConfigurationClass* MemoryPtr)
{
  SmbiosMemoryConfigurationClass& Memory = *MemoryPtr;
  
  Memory.SlotCounts = other.SlotCounts;
  Memory.UserChannels = other.UserChannels;
  SmbiosFillPatchingValues(other.User, &Memory._User);
}

void SmbiosFillPatchingValues(bool _SetTable132, uint8_t pEnabledCores, uint16_t pRamSlotCount, const SlotDeviceArrayClass& SlotDeviceArray, const SETTINGS_DATA& globalSettings, const CPU_STRUCTURE& CPUStructure, SmbiosInjectedSettings* smbiosInjectedSettingsPtr)
{
  SmbiosInjectedSettings& smbiosInjectedSetting = *smbiosInjectedSettingsPtr;
  // from CPUStructure
  smbiosInjectedSetting.Cores = CPUStructure.Cores;
  smbiosInjectedSetting.MaxSpeed = CPUStructure.MaxSpeed;
  smbiosInjectedSetting.Threads = CPUStructure.Threads;
  smbiosInjectedSetting.Features = CPUStructure.Features;
  smbiosInjectedSetting.ExternalClock = CPUStructure.ExternalClock;
  smbiosInjectedSetting.Model = CPUStructure.Model;
  smbiosInjectedSetting.Mobile = CPUStructure.Mobile;  //not for i3-i7
  smbiosInjectedSetting.Family = CPUStructure.Family;
  smbiosInjectedSetting.Type = CPUStructure.Type;
  smbiosInjectedSetting.BrandString.takeValueFrom(CPUStructure.BrandString);
  smbiosInjectedSetting.Extmodel = CPUStructure.Extmodel;
  smbiosInjectedSetting.ExtFeatures = CPUStructure.ExtFeatures;
  smbiosInjectedSetting.MicroCode = CPUStructure.MicroCode;
  smbiosInjectedSetting.Extfamily = CPUStructure.Extfamily;
  smbiosInjectedSetting.Stepping = CPUStructure.Stepping;

  // from SETTINGS_DATA
  smbiosInjectedSetting.BiosVendor = globalSettings.Smbios.BiosVendor;
  smbiosInjectedSetting.BiosVersionUsed = globalSettings.Smbios.BiosVersion;
  smbiosInjectedSetting.EfiVersionUsed = globalSettings.Smbios.EfiVersion;
  smbiosInjectedSetting.ReleaseDateUsed = globalSettings.Smbios.BiosReleaseDate;
  smbiosInjectedSetting.ManufactureName = globalSettings.Smbios.ManufactureName;
  smbiosInjectedSetting.ProductName = globalSettings.Smbios.ProductName;
  smbiosInjectedSetting.SystemVersion = globalSettings.Smbios.SystemVersion;
  smbiosInjectedSetting.SerialNr = globalSettings.Smbios.SerialNr;
  smbiosInjectedSetting.BoardNumber = globalSettings.Smbios.BoardNumber;
  smbiosInjectedSetting.BoardManufactureName = globalSettings.Smbios.BoardManufactureName;
  smbiosInjectedSetting.BoardVersion = globalSettings.Smbios.BoardVersion;
  smbiosInjectedSetting.BoardSerialNumber = globalSettings.Smbios.BoardSerialNumber;
  smbiosInjectedSetting.LocationInChassis = globalSettings.Smbios.LocationInChassis;
  smbiosInjectedSetting.ChassisManufacturer = globalSettings.Smbios.ChassisManufacturer;
  smbiosInjectedSetting.ChassisAssetTag = globalSettings.Smbios.ChassisAssetTag;
  smbiosInjectedSetting.FamilyName = globalSettings.Smbios.FamilyName;
  smbiosInjectedSetting.SmUUID = globalSettings.Smbios.SmUUID;
  smbiosInjectedSetting.NoRomInfo = globalSettings.Smbios.NoRomInfo;
  smbiosInjectedSetting.EnabledCores = pEnabledCores;
  smbiosInjectedSetting.TrustSMBIOS = globalSettings.Smbios.TrustSMBIOS;
  smbiosInjectedSetting.InjectMemoryTables = globalSettings.Smbios.InjectMemoryTables;
  smbiosInjectedSetting.BoardType = globalSettings.Smbios.BoardType;
  smbiosInjectedSetting.ChassisType = globalSettings.Smbios.ChassisType;
  {
    smbiosInjectedSetting.SlotDevices.setEmpty();
    for ( size_t idx = 0 ; idx < SlotDeviceArray.size() ; ++idx ) {
      if ( SlotDeviceArray[idx].Index >= MAX_RAM_SLOTS ) {
        log_technical_bug("slotDevice->Index >= MAX_RAM_SLOTS");
      }else{
        SLOT_DEVICE* slotDevice = new SLOT_DEVICE;
        SmbiosFillPatchingValues(SlotDeviceArray[idx], slotDevice);
        if ( globalSettings.Smbios.SlotDevices.doesSlotForIndexExist(slotDevice->Index) ) {
          SmbiosFillPatchingValues(globalSettings.Smbios.SlotDevices.getSlotForIndex(slotDevice->Index), slotDevice);
        }
        smbiosInjectedSetting.SlotDevices.AddReference(slotDevice, true);
      }
    }
  }
  SmbiosFillPatchingValues(globalSettings.Smbios.RamSlotInfoArray, &smbiosInjectedSetting.Memory);

  smbiosInjectedSetting.gPlatformFeature = globalSettings.Smbios.gPlatformFeature;
  smbiosInjectedSetting.FirmwareFeatures = globalSettings.Smbios.FirmwareFeatures;
  smbiosInjectedSetting.FirmwareFeaturesMask = globalSettings.Smbios.FirmwareFeaturesMask;
  smbiosInjectedSetting.ExtendedFirmwareFeatures = globalSettings.Smbios.ExtendedFirmwareFeatures;
  smbiosInjectedSetting.ExtendedFirmwareFeaturesMask = globalSettings.Smbios.ExtendedFirmwareFeaturesMask;
  smbiosInjectedSetting.Attribute = globalSettings.Smbios.Attribute;

  smbiosInjectedSetting.KPDELLSMBIOS = globalSettings.KernelAndKextPatches.KPDELLSMBIOS;

  smbiosInjectedSetting.CpuType = globalSettings.CPU.CpuType;
  smbiosInjectedSetting.SetTable132 = _SetTable132;
  smbiosInjectedSetting.QPI = globalSettings.CPU.QPI;

  smbiosInjectedSetting.RamSlotCount = pRamSlotCount;
}





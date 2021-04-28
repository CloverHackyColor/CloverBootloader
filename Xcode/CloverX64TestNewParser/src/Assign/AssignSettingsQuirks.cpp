/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsBoot.h"
#include <Platform.h>
#include "AssignField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignOcKernelQuirks(const XString8& label, SETTINGS_DATA::QuirksClass::OcKernelQuirksClass& oldS, const ConfigPlistClass::Quirks_Class::OcKernelQuirks_Class& newS)
{
//  oldS.AppleCpuPmCfgLock = 0;
//  oldS.AppleXcpmCfgLock = 0;
  Assign(AppleXcpmExtraMsrs);
  Assign(AppleXcpmForceBoost);
//  oldS.CustomSmbiosGuid = 0;
  Assign(DisableIoMapper);
  oldS.DisableLinkeditJettison = newS.dgetDisableLinkeditJettison();
//  oldS.DisableRtcChecksum = 0;
  Assign(DummyPowerManagement);
  Assign(ExternalDiskIcons);
  Assign(IncreasePciBarSize);
//  oldS.LapicKernelPanic = 0;
//  oldS.PanicNoKextDump = 0;
  Assign(PowerTimeoutKernelPanic);
  Assign(ThirdPartyDrives);
  Assign(XhciPortLimit);
}

void AssignOcBooterQuirks(const XString8& label, SETTINGS_DATA::QuirksClass::OcBooterQuirksClass& oldS, const ConfigPlistClass::Quirks_Class::OcBooterQuirks_Class& newS)
{
  Assign(AvoidRuntimeDefrag);
  Assign(DevirtualiseMmio);
  Assign(DisableSingleUser);
  Assign(DisableVariableWrite);
  Assign(DiscardHibernateMap);
  Assign(EnableSafeModeSlide);
  Assign(EnableWriteUnprotector);
  Assign(ForceExitBootServices);
  Assign(ProtectMemoryRegions);
  Assign(ProtectSecureBoot);
  Assign(ProtectUefiServices);
  Assign(ProvideCustomSlide);
  Assign(ProvideMaxSlide);
  Assign(RebuildAppleMemoryMap);
  Assign(SetupVirtualMap);
  Assign(SignalAppleOS);
  Assign(SyncRuntimePermissions);
}

void AssignMmioWhiteList(const XString8& label, SETTINGS_DATA::QuirksClass::MMIOWhiteList& oldS, const ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class& newS)
{
  Assign(address);
  Assign(comment);
  Assign(enabled);
}

void AssignMmioWhiteListArray(const XString8& label, XObjArray<SETTINGS_DATA::QuirksClass::MMIOWhiteList>& oldS, const XmlArray<ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignMmioWhiteList(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignQuirks(const XString8& label, SETTINGS_DATA::QuirksClass& oldS, const ConfigPlistClass::Quirks_Class& newS)
{
  Assign(FuzzyMatch);
  Assign(OcKernelCache);
  AssignOcKernelQuirks(S8Printf("%s.OcKernelQuirks", label.c_str()), oldS.OcKernelQuirks, newS.OcKernelQuirks);
  AssignOcBooterQuirks(S8Printf("%s.OcBooterQuirks", label.c_str()), oldS.OcBooterQuirks, newS.OcBooterQuirks);
  AssignMmioWhiteListArray(S8Printf("%s.mmioWhiteListArray", label.c_str()), oldS.mmioWhiteListArray, newS.MmioWhitelist);
  Assign(QuirksMask);
}

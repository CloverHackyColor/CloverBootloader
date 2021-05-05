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

void CompareOcKernelQuirks(const XString8& label, const SETTINGS_DATA::QuirksClass::OcKernelQuirksClass& oldS, const ConfigPlistClass::Quirks_Class::OcKernelQuirks_Class& newS)
{
//  compare(AppleCpuPmCfgLock);
//  compare(AppleXcpmCfgLock);
  compare(AppleXcpmExtraMsrs);
  compare(AppleXcpmForceBoost);
//  compare(CustomSmbiosGuid);
  compare(DisableIoMapper);
  compare(DisableLinkeditJettison);
//  compare(DisableRtcChecksum);
  compare(DummyPowerManagement);
  compare(ExternalDiskIcons);
  compare(IncreasePciBarSize);
//  compare(LapicKernelPanic);
//  compare(PanicNoKextDump);
  compare(PowerTimeoutKernelPanic);
  compare(ThirdPartyDrives);
  compare(XhciPortLimit);
}

void CompareOcBooterQuirks(const XString8& label, const SETTINGS_DATA::QuirksClass::OcBooterQuirksClass& oldS, const ConfigPlistClass::Quirks_Class::OcBooterQuirks_Class& newS)
{
  compare(AvoidRuntimeDefrag);
  compare(DevirtualiseMmio);
  compare(DisableSingleUser);
  compare(DisableVariableWrite);
  compare(DiscardHibernateMap);
  compare(EnableSafeModeSlide);
  compare(EnableWriteUnprotector);
  compare(ForceExitBootServices);
  compare(ProtectMemoryRegions);
  compare(ProtectSecureBoot);
  compare(ProtectUefiServices);
  compare(ProvideCustomSlide);
  compare(ProvideMaxSlide);
  compare(RebuildAppleMemoryMap);
  compare(SetupVirtualMap);
  compare(SignalAppleOS);
  compare(SyncRuntimePermissions);
}

void CompareMmioWhiteList(const XString8& label, const SETTINGS_DATA::QuirksClass::MMIOWhiteList& oldS, const ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class& newS)
{
  compare(address);
  compare(comment);
  compare(enabled);
}

void CompareMmioWhiteListArray(const XString8& label, const XObjArray<SETTINGS_DATA::QuirksClass::MMIOWhiteList>& oldS, const XmlArray<ConfigPlistClass::Quirks_Class::Quirks_MmioWhitelist_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareMmioWhiteList(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareQuirks(const XString8& label, const SETTINGS_DATA::QuirksClass& oldS, const ConfigPlistClass::Quirks_Class& newS)
{
  compare(FuzzyMatch);
  compare(OcKernelCache);
  CompareOcKernelQuirks(S8Printf("%s.OcKernelQuirks", label.c_str()), oldS.OcKernelQuirks, newS.OcKernelQuirks);
  CompareOcBooterQuirks(S8Printf("%s.OcBooterQuirks", label.c_str()), oldS.OcBooterQuirks, newS.OcBooterQuirks);
  CompareMmioWhiteListArray(S8Printf("%s.mmioWhiteListArray", label.c_str()), oldS.mmioWhiteListArray, newS.MmioWhitelist);
  compare(QuirksMask);
}

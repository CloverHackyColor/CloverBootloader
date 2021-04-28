/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsBoot.h"
#include <Platform.h>
#include "CompareField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareBoot(const XString8& label, const SETTINGS_DATA::BootClass& oldS, const ConfigPlistClass::Boot_Class& newS)
{
  compare(Timeout);
  compare(SkipHibernateTimeout);
  compare(DisableCloverHotkeys);
  compare(BootArgs);
  compare(NeverDoRecovery);
  compare(LastBootedVolume);
  compare(DefaultVolume);
  compare(DefaultLoader);
  compare(DebugLog);
  compare(FastBoot);
  compare(NoEarlyProgress);
  compare(NeverHibernate);
  compare(StrictHibernate);
  compare(RtcHibernateAware);
  compare(HibernationFixup);
  compare(SignatureFixup);
  compare(SecureSetting);
  compare(SecureBootPolicy);
  compare(SecureBootWhiteList);
  compare(SecureBootBlackList);
  compare(XMPDetection);
  compareField(oldS.LegacyBoot, newS.dgetLegacyBoot(gFirmwareClover), S8Printf("%s.LegacyBoot", label.c_str()));
//  compare(LegacyBoot);
  compare(LegacyBiosDefaultEntry);
  compare(CustomLogoType);
  compare(CustomLogoAsXString8);
  compare(CustomLogoAsData);
}

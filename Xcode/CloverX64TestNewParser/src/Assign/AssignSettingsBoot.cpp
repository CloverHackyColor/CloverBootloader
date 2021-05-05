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

void AssignBoot(const XString8& label, SETTINGS_DATA::BootClass& oldS, const ConfigPlistClass::Boot_Class& newS)
{
  Assign(Timeout);
  Assign(SkipHibernateTimeout);
  Assign(DisableCloverHotkeys);
  Assign(BootArgs);
  Assign(NeverDoRecovery);
  Assign(LastBootedVolume);
  Assign(DefaultVolume);
  Assign(DefaultLoader);
  Assign(DebugLog);
  Assign(FastBoot);
  Assign(NoEarlyProgress);
  Assign(NeverHibernate);
  Assign(StrictHibernate);
  Assign(RtcHibernateAware);
  Assign(HibernationFixup);
  Assign(SignatureFixup);
  Assign(SecureSetting);
  Assign(SecureBootPolicy);
  Assign(SecureBootWhiteList);
  Assign(SecureBootBlackList);
  Assign(XMPDetection);
  AssignField(oldS.LegacyBoot, newS.dgetLegacyBoot(gFirmwareClover), S8Printf("%s.LegacyBoot", label.c_str()));
//  Assign(LegacyBoot);
  Assign(LegacyBiosDefaultEntry);
  Assign(CustomLogoType);
  Assign(CustomLogoAsXString8);
  Assign(CustomLogoAsData);
}

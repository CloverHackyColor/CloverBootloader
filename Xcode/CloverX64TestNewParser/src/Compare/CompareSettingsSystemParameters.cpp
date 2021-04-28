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

void CompareSystemParameters(const XString8& label, const SETTINGS_DATA::SystemParametersClass& oldS, const ConfigPlistClass::SystemParameters_Class& newS)
{
  compare(WithKexts);
  compare(WithKextsIfNoFakeSMC);
  compare(NoCaches);
  compare(BacklightLevel);
  compare(BacklightLevelConfig);
  compare(CustomUuid);
  compare(_InjectSystemID);
  compare(NvidiaWeb);
}

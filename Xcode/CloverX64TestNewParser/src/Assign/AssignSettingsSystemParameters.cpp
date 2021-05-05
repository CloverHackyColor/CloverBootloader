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

void AssignSystemParameters(const XString8& label, SETTINGS_DATA::SystemParametersClass& oldS, const ConfigPlistClass::SystemParameters_Class& newS)
{
  Assign(WithKexts);
  Assign(WithKextsIfNoFakeSMC);
  Assign(NoCaches);
  Assign(BacklightLevel);
  Assign(BacklightLevelConfig);
  Assign(CustomUuid);
  Assign(_InjectSystemID);
  Assign(NvidiaWeb);
}

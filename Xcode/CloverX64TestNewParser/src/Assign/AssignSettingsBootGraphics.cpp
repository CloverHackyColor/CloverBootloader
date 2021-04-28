/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsBootGraphics.h"
#include <Platform.h>
#include "AssignField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignBootGraphics(const XString8& label, SETTINGS_DATA::BootGraphicsClass& oldS, const ConfigPlistClass::BootGraphics_Class& newS)
{
  Assign(DefaultBackgroundColor);
  Assign(UIScale);
  Assign(EFILoginHiDPI);
  oldS._flagstate = newS.dget_flagstate();
}

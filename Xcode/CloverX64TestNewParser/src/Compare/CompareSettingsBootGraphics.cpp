/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsBootGraphics.h"
#include <Platform.h>
#include "CompareField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareBootGraphics(const XString8& label, const SETTINGS_DATA::BootGraphicsClass& oldS, const ConfigPlistClass::BootGraphics_Class& newS)
{
  compare(DefaultBackgroundColor);
  compare(UIScale);
  compare(EFILoginHiDPI);
  compare(_flagstate);
}

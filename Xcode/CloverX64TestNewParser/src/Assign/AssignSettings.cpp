/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettings.h"
#include <Platform.h>

#include "AssignField.h"
#include "AssignSettingsBoot.h"
#include "AssignSettingsACPI.h"
#include "AssignSettingsGUI.h"
#include "AssignSettingsCPU.h"
#include "AssignSettingsSystemParameters.h"
#include "AssignSettingsKernelAndKextPatches.h"
#include "AssignSettingsGraphics.h"
#include "AssignSettingsDevices.h"
#include "AssignSettingsQuirks.h"
#include "AssignSettingsRtVariables.h"
#include "AssignSettingsSmbios.h"
#include "AssignSettingsBootGraphics.h"


uint64_t AssignOldNewSettings(SETTINGS_DATA& oldS, const ConfigPlistClass& newS, const SmbiosPlistClass& smbiosPlist, const XString8& label)
{
  AssignField_nbError = 0;
  AssignField_firstErrorField.setEmpty();
  
  AssignBoot("Boot"_XS8, oldS.Boot, newS.Boot);
  AssignAcpi("ACPI"_XS8, oldS.ACPI, newS.ACPI);
  AssignGUI("GUI"_XS8, oldS.GUI, newS.GUI);
  AssignCPU("CPU"_XS8, oldS.CPU, newS.CPU);
  AssignSystemParameters("CPU"_XS8, oldS.SystemParameters, newS.SystemParameters);
  AssignKernelAndKextPatches("KernelAndKextPatches"_XS8, oldS.KernelAndKextPatches, newS.KernelAndKextPatches);
  AssignGraphics("Graphics"_XS8, oldS.Graphics, newS.Graphics);
  AssignField(oldS.DisabledDriverArray, newS.dgetDisabledDriverArray(), "DisabledDriverArray"_XS8);
  AssignDevices("Devices"_XS8, oldS.Devices, newS.Devices);
  AssignQuirks("Quirks"_XS8, oldS.Quirks, newS.Quirks);
  AssignRtVariables("RtVariables"_XS8, oldS.RtVariables, newS.RtVariables);
  AssignSmbios("Smbios"_XS8, oldS.Smbios, newS.getSMBIOS(), smbiosPlist);
  AssignBootGraphics("BootGraphics"_XS8, oldS.BootGraphics, newS.BootGraphics);


  if ( AssignField_nbError == 0 ) {
//    DebugLog(2, "Great, new parsing give the same result\n");
//    PauseForKey(""_XSW);
    return AssignField_nbError;
  }else{
//    DebugLog(2, "New parsing gives %llu error(s)\n", nbError);
//    PauseForKey(L"");
    return AssignField_nbError;
  }
}


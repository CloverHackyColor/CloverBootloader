/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettings.h"
#include <Platform.h>
//#include "../../include/OSFlags.h"
#include "CompareField.h"
#include "CompareSettingsBoot.h"
#include "CompareSettingsACPI.h"
#include "CompareSettingsGUI.h"
#include "CompareSettingsCPU.h"
#include "CompareSettingsSystemParameters.h"
#include "CompareSettingsKernelAndKextPatches.h"
#include "CompareSettingsGraphics.h"
#include "CompareSettingsDevices.h"
#include "CompareSettingsQuirks.h"
#include "CompareSettingsRtVariables.h"
#include "CompareSettingsSmbios.h"
#include "CompareSettingsBootGraphics.h"


uint64_t CompareOldNewSettings(const SETTINGS_DATA& oldS, const ConfigPlistClass& newS, const XString8& label)
{
  compareField_nbError = 0;
  compareField_firstErrorField.setEmpty();
  
  CompareBoot("Boot"_XS8, oldS.Boot, newS.Boot);
  CompareAcpi("ACPI"_XS8, oldS.ACPI, newS.ACPI);
  CompareGUI("GUI"_XS8, oldS.GUI, newS.GUI);
  CompareCPU("CPU"_XS8, oldS.CPU, newS.CPU);
  CompareSystemParameters("CPU"_XS8, oldS.SystemParameters, newS.SystemParameters);
  CompareKernelAndKextPatches("KernelAndKextPatches"_XS8, oldS.KernelAndKextPatches, newS.KernelAndKextPatches);
  CompareGraphics("Graphics"_XS8, oldS.Graphics, newS.Graphics);
  compareField(oldS.DisabledDriverArray, newS.dgetDisabledDriverArray(), "DisabledDriverArray"_XS8);
  CompareDevices("Devices"_XS8, oldS.Devices, newS.Devices);
  CompareQuirks("Quirks"_XS8, oldS.Quirks, newS.Quirks);
  CompareRtVariables("RtVariables"_XS8, oldS.RtVariables, newS.RtVariables);
  CompareSmbios("Smbios"_XS8, oldS.Smbios, newS.getSMBIOS());
  CompareBootGraphics("BootGraphics"_XS8, oldS.BootGraphics, newS.BootGraphics);


  if ( compareField_nbError == 0 ) {
//    DebugLog(2, "Great, new parsing give the same result\n");
//    PauseForKey(""_XSW);
    return compareField_nbError;
  }else{
//    DebugLog(2, "New parsing gives %llu error(s)\n", nbError);
//    PauseForKey(L"");
    return compareField_nbError;
  }
}


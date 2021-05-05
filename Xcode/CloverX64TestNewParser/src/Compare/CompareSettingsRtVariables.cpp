/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsRtVariables.h"
#include <Platform.h>
#include "CompareField.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void CompareRT_VARIABLE(const XString8& label, const SETTINGS_DATA::RtVariablesClass::RT_VARIABLES& oldS, const ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block& newS)
{
  compare(Disabled);
  compare(Comment);
  compare(Name);
  EFI_GUID guid = newS.dgetGuid();
  compareField((void*)&oldS.Guid, sizeof(oldS.Guid), (void*)&guid, sizeof(guid), S8Printf("%s.Guid", label.c_str()));
}

void CompareRT_VARIABLES(const XString8& label, const XObjArray<SETTINGS_DATA::RtVariablesClass::RT_VARIABLES>& oldS, const XmlArray<ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareRT_VARIABLE(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareRtVariables(const XString8& label, const SETTINGS_DATA::RtVariablesClass& oldS, const ConfigPlistClass::RtVariables_Class& newS)
{
  compare(RtROMAsString);
  compare(RtROMAsData);
  compare(RtMLBSetting);
  compare(CsrActiveConfig);
  compare(BooterConfig);
  compare(BooterCfgStr);
  CompareRT_VARIABLES(S8Printf("%s.BlockRtVariableArray", label.c_str()), oldS.BlockRtVariableArray, newS.Block);
}

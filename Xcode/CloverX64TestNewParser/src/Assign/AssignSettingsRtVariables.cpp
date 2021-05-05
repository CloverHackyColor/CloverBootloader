/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsRtVariables.h"
#include <Platform.h>
#include "AssignField.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void AssignRT_VARIABLE(const XString8& label, SETTINGS_DATA::RtVariablesClass::RT_VARIABLES& oldS, const ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block& newS)
{
  Assign(Disabled);
  Assign(Comment);
  Assign(Name);
  oldS.Guid = newS.dgetGuid();
}

void AssignRT_VARIABLES(const XString8& label, XObjArray<SETTINGS_DATA::RtVariablesClass::RT_VARIABLES>& oldS, const XmlArray<ConfigPlistClass::RtVariables_Class::Devices_RtVariables_Block>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignRT_VARIABLE(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignRtVariables(const XString8& label, SETTINGS_DATA::RtVariablesClass& oldS, const ConfigPlistClass::RtVariables_Class& newS)
{
  Assign(RtROMAsString);
  Assign(RtROMAsData);
  Assign(RtMLBSetting);
  Assign(CsrActiveConfig);
  Assign(BooterConfig);
  Assign(BooterCfgStr);
  AssignRT_VARIABLES(S8Printf("%s.BlockRtVariableArray", label.c_str()), oldS.BlockRtVariableArray, newS.Block);
}

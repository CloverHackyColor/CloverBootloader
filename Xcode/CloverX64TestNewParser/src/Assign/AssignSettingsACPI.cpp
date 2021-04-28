/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsACPI.h"
#include <Platform.h>
#include "AssignField.h"


void AssignAcpi_ACPIDropTable(const XString8& label, SETTINGS_DATA::ACPIClass::ACPIDropTablesClass& oldS, const ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class& newS)
{
  Assign(Signature);
  Assign(TableId);
  Assign(TabLength);
  Assign(OtherOS);
}

void AssignACPI_DropTableArray(const XString8 &label, XObjArray<SETTINGS_DATA::ACPIClass::ACPIDropTablesClass> &oldS, const XmlArray<ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class> &newS) {
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0 ; idx < newS.size() ; ++idx ) {
      oldS.AddReference(new SETTINGS_DATA::ACPIClass::ACPIDropTablesClass(), true);
      AssignAcpi_ACPIDropTable(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignAcpi_DSDT_DSDTPatch(const XString8& label, SETTINGS_DATA::ACPIClass::DSDTClass::DSDT_Patch& oldS, const ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Patch_Class& newS)
{
  Assign(Disabled);
  Assign(PatchDsdtLabel);
  Assign(PatchDsdtFind);
  Assign(PatchDsdtReplace);
  Assign(PatchDsdtTgt);
  oldS.PatchDsdtMenuItem.BValue = !newS.dgetDisabled();
}

void AssignAcpi_DSDT(const XString8& label, SETTINGS_DATA::ACPIClass::DSDTClass& oldS, const ConfigPlistClass::ACPI_Class::DSDT_Class& newS)
{
  Assign(DsdtName);
  Assign(DebugDSDT);
  Assign(Rtc8Allowed);
  Assign(PNLF_UID);
  Assign(FixDsdt);
  Assign(ReuseFFFF);
  Assign(SuspendOverride);
//  if ( AssignField(oldS.DSDTPatchArray.size(), newS.Patches.size(), S8Printf("%s.DSDTPatchArray.size()", label.c_str())) ) {
    for ( size_t idx = 0 ; idx < newS.Patches.size() ; ++idx ) {
      oldS.DSDTPatchArray.AddReference(new SETTINGS_DATA::ACPIClass::DSDTClass::DSDT_Patch(), true);
      AssignAcpi_DSDT_DSDTPatch(S8Printf("%s.DSDTPatchArray[%zu]", label.c_str(), idx), oldS.DSDTPatchArray[idx], newS.Patches[idx]);
    }
//  }
}

void AssignAcpi_SSDT_Generate(const XString8& label, SETTINGS_DATA::ACPIClass::SSDTClass::GenerateClass& oldS, const ConfigPlistClass::ACPI_Class::SSDT_Class::XmlUnionGenerate& newS)
{
  Assign(GeneratePStates);
  Assign(GenerateCStates);
  Assign(GenerateAPSN);
  Assign(GenerateAPLF);
  Assign(GeneratePluginType);
}

void AssignAcpi_SSDT(const XString8& label, SETTINGS_DATA::ACPIClass::SSDTClass& oldS, const ConfigPlistClass::ACPI_Class::SSDT_Class& newS)
{
  Assign(DropSSDTSetting);
  Assign(NoOemTableId);
  Assign(NoDynamicExtract);
  Assign(EnableISS);
  Assign(EnableC7);
  Assign(_EnableC6);
  Assign(_EnableC4);
  Assign(_EnableC2);
  Assign(_C3Latency);
  Assign(PLimitDict);
  Assign(UnderVoltStep);
  Assign(DoubleFirstState);
  Assign(MinMultiplier);
  Assign(MaxMultiplier);
  Assign(PluginType);
  AssignAcpi_SSDT_Generate(S8Printf("%s.Generate", label.c_str()), oldS.Generate, newS.Generate);
}


//----------------------------------------------------------------------------------------------------------

void AssignAcpi_ACPI_NAME(const XString8& label, ACPI_NAME& oldS, const XString8& newS)
{
  AssignField(oldS.Name, newS, S8Printf("%s.Name", label.c_str()));
}

void AssignAcpi_ACPI_RENAME_DEVICE(const XString8& label, ACPI_RENAME_DEVICE& oldS, const XmlAddKey<XmlKey, XmlString8>& newS)
{
  AssignAcpi_ACPI_NAME(S8Printf("%s.acpiName", label.c_str()), oldS.acpiName, newS.key().value());
  AssignField(oldS.renameTo, newS.value(), S8Printf("%s.renameTo", label.c_str()));
}

void AssignACPI_ACPI_RENAME_DEVICEArray(const XString8 &label, XObjArray<ACPI_RENAME_DEVICE> &oldS, const ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class& newS)
{
//  if ( AssignField(oldS.size(), newS.size(), S8Printf("%s size", label.c_str()))  ) {
    for ( size_t idx = 0 ; idx < newS.size() ; ++idx ) {
      oldS.AddReference(new ACPI_RENAME_DEVICE(), true);
      AssignAcpi_ACPI_RENAME_DEVICE(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS.getAtIndex(idx));
    }
//  }
}


//----------------------------------------------------------------------------------------------------------


void AssignAcpi(const XString8& label, SETTINGS_DATA::ACPIClass& oldS, const ConfigPlistClass::ACPI_Class& newS)
{
  Assign(ResetAddr);
  Assign(ResetVal);
  Assign(SlpSmiEnable);
  Assign(FixHeaders);
  Assign(FixMCFG);
  Assign(NoASPM);
  Assign(smartUPS);
  Assign(PatchNMI);
  Assign(AutoMerge);
  Assign(DisabledAML);
  Assign(SortedACPI);
  AssignACPI_ACPI_RENAME_DEVICEArray(S8Printf("%s.DeviceRename", label.c_str()), oldS.DeviceRename, newS.RenameDevices);
  AssignACPI_DropTableArray(S8Printf("%s.ACPIDropTablesArray", label.c_str()), oldS.ACPIDropTablesArray, newS.ACPIDropTablesArray);
  AssignAcpi_DSDT(S8Printf("%s.DSDT", label.c_str()), oldS.DSDT, newS.DSDT);
  AssignAcpi_SSDT(S8Printf("%s.SSDT", label.c_str()), oldS.SSDT, newS.SSDT);
}

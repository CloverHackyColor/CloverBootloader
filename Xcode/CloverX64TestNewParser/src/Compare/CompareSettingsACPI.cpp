/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsACPI.h"
#include <Platform.h>
#include "CompareField.h"


void CompareAcpi_ACPIDropTable(const XString8& label, const SETTINGS_DATA::ACPIClass::ACPIDropTablesClass& oldS, const ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class& newS)
{
  compare(Signature);
  compare(TableId);
  compare(TabLength);
  compare(OtherOS);
}

void CompareACPI_DropTableArray(const XString8 &label, const XObjArray<SETTINGS_DATA::ACPIClass::ACPIDropTablesClass> &oldS, const XmlArray<ConfigPlistClass::ACPI_Class::ACPI_DropTables_Class> &newS) {
  if ( fcompare(size()) ) {
    for ( size_t idx = 0 ; idx < oldS.size() ; ++idx ) {
      CompareAcpi_ACPIDropTable(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareAcpi_DSDT_DSDTPatch(const XString8& label, const SETTINGS_DATA::ACPIClass::DSDTClass::DSDT_Patch& oldS, const ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Patch_Class& newS)
{
  compare(Disabled);
  compare(PatchDsdtLabel);
  compare(PatchDsdtFind);
  compare(PatchDsdtReplace);
  compare(PatchDsdtTgt);
  compareField(oldS.PatchDsdtMenuItem.BValue, (uint8_t)(!newS.dgetBValue()), S8Printf("%s.PatchDsdtMenuItem.BValue", label.c_str()));
}

void CompareAcpi_DSDT(const XString8& label, const SETTINGS_DATA::ACPIClass::DSDTClass& oldS, const ConfigPlistClass::ACPI_Class::DSDT_Class& newS)
{
  compare(DsdtName);
  compare(DebugDSDT);
  compare(Rtc8Allowed);
  compare(PNLF_UID);
  compare(FixDsdt);
  compare(ReuseFFFF);
  compare(SuspendOverride);
  if ( compareField(oldS.DSDTPatchArray.size(), newS.Patches.size(), S8Printf("%s.DSDTPatchArray.size()", label.c_str())) ) {
    for ( size_t idx = 0 ; idx < oldS.DSDTPatchArray.size() ; ++idx ) {
      CompareAcpi_DSDT_DSDTPatch(S8Printf("%s.DSDTPatchArray[%zu]", label.c_str(), idx), oldS.DSDTPatchArray[idx], newS.Patches[idx]);
    }
  }
}

void CompareAcpi_SSDT_Generate(const XString8& label, const SETTINGS_DATA::ACPIClass::SSDTClass::GenerateClass& oldS, const ConfigPlistClass::ACPI_Class::SSDT_Class::XmlUnionGenerate& newS)
{
  compare(GeneratePStates);
  compare(GenerateCStates);
  compare(GenerateAPSN);
  compare(GenerateAPLF);
  compare(GeneratePluginType);
}

void CompareAcpi_SSDT(const XString8& label, const SETTINGS_DATA::ACPIClass::SSDTClass& oldS, const ConfigPlistClass::ACPI_Class::SSDT_Class& newS)
{
  compare(DropSSDTSetting);
  compare(NoOemTableId);
  compare(NoDynamicExtract);
  compare(EnableISS);
  compare(EnableC7);
  compare(_EnableC6);
  compare(_EnableC4);
  compare(_EnableC2);
  compare(_C3Latency);
  compare(PLimitDict);
  compare(UnderVoltStep);
  compare(DoubleFirstState);
  compare(MinMultiplier);
  compare(MaxMultiplier);
  compare(PluginType);
  CompareAcpi_SSDT_Generate(S8Printf("%s.Generate", label.c_str()), oldS.Generate, newS.Generate);
}


//----------------------------------------------------------------------------------------------------------

void CompareAcpi_ACPI_NAME(const XString8& label, const ACPI_NAME& oldS, const XString8& newS)
{
  compareField(oldS.Name, newS, S8Printf("%s.Name", label.c_str()));
}

void CompareAcpi_ACPI_RENAME_DEVICE(const XString8& label, const ACPI_RENAME_DEVICE& oldS, const XmlAddKey<XmlKey, XmlString8>& newS)
{
  CompareAcpi_ACPI_NAME(S8Printf("%s.acpiName", label.c_str()), oldS.acpiName, newS.key().value());
  compareField(oldS.renameTo, newS.value(), S8Printf("%s.renameTo", label.c_str()));
}

void CompareACPI_ACPI_RENAME_DEVICEArray(const XString8 &label, const XObjArray<ACPI_RENAME_DEVICE> &oldS, const ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class& newS)
{
  if ( compareField(oldS.size(), newS.size(), S8Printf("%s size", label.c_str()))  ) {
    for ( size_t idx = 0 ; idx < oldS.size() ; ++idx ) {
      CompareAcpi_ACPI_RENAME_DEVICE(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS.getAtIndex(idx));
    }
  }
}


//----------------------------------------------------------------------------------------------------------


void CompareAcpi(const XString8& label, const SETTINGS_DATA::ACPIClass& oldS, const ConfigPlistClass::ACPI_Class& newS)
{
  compare(ResetAddr);
  compare(ResetVal);
  compare(SlpSmiEnable);
  compare(FixHeaders);
  compare(FixMCFG);
  compare(NoASPM);
  compare(smartUPS);
  compare(PatchNMI);
  compare(AutoMerge);
  compare(DisabledAML);
  compare(SortedACPI);
  CompareACPI_ACPI_RENAME_DEVICEArray(S8Printf("%s.DeviceRename", label.c_str()), oldS.DeviceRename, newS.RenameDevices);
  CompareACPI_DropTableArray(S8Printf("%s.ACPIDropTablesArray", label.c_str()), oldS.ACPIDropTablesArray, newS.ACPIDropTablesArray);
  CompareAcpi_DSDT(S8Printf("%s.DSDT", label.c_str()), oldS.DSDT, newS.DSDT);
  CompareAcpi_SSDT(S8Printf("%s.SSDT", label.c_str()), oldS.SSDT, newS.SSDT);
}

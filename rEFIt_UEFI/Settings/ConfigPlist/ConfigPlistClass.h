/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_H_
#define _CONFIGPLISTCLASS_H_

#include "../../include/VolumeTypes.h"
#include "../../include/OSTypes.h"
#include "../../include/OSFlags.h"
#include "../../include/BootTypes.h"
#include "../../include/Languages.h"
#include "../../include/Devices.h"
#include "../../include/QuirksCodes.h"
#include "../../include/TagTypes.h"
#include "../../include/Pci.h"
#include "../../entry_scan/loader.h" // for KERNEL_SCAN_xxx constants
#include <IndustryStandard/SmBios.h> // for Smbios memory type
#include "../../Platform/guid.h"
#include "../../entry_scan/secureboot.h"
extern "C" {
  #include <Protocol/GraphicsOutput.h>
}
#include "../../Platform/cpu.h"
//#include "../../Platform/nvidia.h"
extern UINT8 default_NVCAP[]; // dependecy problem. TODO
extern const UINT8 default_dcfg_0[];
extern const UINT8 default_dcfg_1[];


#include "../../cpp_lib/undefinable.h"
#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteCompositeTypes.h"
#include "../../cpp_lib/XmlLiteDictTypes.h"
#include "../../cpp_lib/XmlLiteArrayTypes.h"
#include "../../cpp_lib/XmlLiteUnionTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"

#include "../../cpp_foundation/XString.h"
#include "../../cpp_foundation/XStringArray.h"
#include "../../cpp_foundation/XArray.h"
#include "../../cpp_foundation/XObjArray.h"
#include "../../Platform/Utils.h"

#include "ConfigPlistAbstract.h"
#include "SMBIOSPlist.h"

class ConfigPlistClass : public ConfigPlistAbstractClass
{
  using super = ConfigPlistAbstractClass;
public:
  #include "Config_ACPI.h"
  #include "Config_Boot.h"
  #include "Config_BootGraphics.h"
  #include "Config_CPU.h"
  #include "Config_Devices.h"
  #include "Config_GUI.h"
  #include "Config_Graphics.h"
  #include "Config_KernelAndKextPatches.h"
  #include "Config_SystemParameters.h"
  #include "Config_RtVariables.h"
  #include "Config_Quirks.h"

  ACPI_Class ACPI = ACPI_Class();
  Boot_Class Boot = Boot_Class();
  BootGraphics_Class BootGraphics = BootGraphics_Class();
  CPU_Class CPU = CPU_Class();
  DevicesClass Devices = DevicesClass();
  XmlStringWArray DisableDrivers = XmlStringWArray();
  GUI_Class GUI = GUI_Class();
  Graphics_Class Graphics; // Cannot do this :  = Graphics_Class(*this); because of a MSVC bug. Compilation failed at ssignment of m_fields because all member become const.
  KernelAndKextPatches_Class KernelAndKextPatches = KernelAndKextPatches_Class();
protected:
  SmbiosPlistClass::SmbiosDictClass SMBIOS = SmbiosPlistClass::SmbiosDictClass(); // use the same dict as for standalone smbios plist
public:
  SystemParameters_Class SystemParameters = SystemParameters_Class();
  RtVariables_Class RtVariables = RtVariables_Class();
  Quirks_Class Quirks = Quirks_Class();

  XmlDictField m_fields[13] = {
    {"ACPI", ACPI},
    {"Boot", Boot},
    {"BootGraphics", BootGraphics},
    {"CPU", CPU},
    {"Devices", Devices},
    {"DisableDrivers", DisableDrivers},
    {"GUI", GUI},
    {"Graphics", Graphics},
    {"KernelAndKextPatches", KernelAndKextPatches},
    {"SMBIOS", SMBIOS},
    {"SystemParameters", SystemParameters},
    {"RtVariables", RtVariables},
    {"Quirks", Quirks},
  };

public:
  ConfigPlistClass() : Graphics(*this) {}
  
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

  const decltype(DisableDrivers)::ValueType& dgetDisabledDriverArray() const { return DisableDrivers.isDefined() ? DisableDrivers.value() : DisableDrivers.nullValue; };
  const decltype(SMBIOS)& getSMBIOS() const { return SMBIOS; };

};

#endif /* _CONFIGPLISTCLASS_H_ */

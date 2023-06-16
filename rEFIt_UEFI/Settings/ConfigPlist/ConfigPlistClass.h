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
  Graphics_Class Graphics; // Cannot do this :  = Graphics_Class(*this); because of a MSVC bug. Compilation failed at assignment of m_fields because all member become const.
  KernelAndKextPatches_Class KernelAndKextPatches = KernelAndKextPatches_Class();
  SmbiosPlistClass::SmbiosDictClass SMBIOS = SmbiosPlistClass::SmbiosDictClass();  // use the same dict as for standalone smbios plist
  SmbiosPlistClass::SmbiosDictClass SMBIOS_lion = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_mav = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_cap = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_hsierra = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_moja = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_cata = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_bigsur = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_monterey = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_ventura = SmbiosPlistClass::SmbiosDictClass();
  SmbiosPlistClass::SmbiosDictClass SMBIOS_sonoma = SmbiosPlistClass::SmbiosDictClass();
//  SmbiosPlistClass::SmbiosDictClass SMBIOS_lion = SmbiosPlistClass::SmbiosDictClass();
public:
  SystemParameters_Class SystemParameters = SystemParameters_Class();
  RtVariables_Class RtVariables = RtVariables_Class();
  Quirks_Class Quirks = Quirks_Class();

  XmlDictField m_fields[23] = {
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
    {"SMBIOS_lion", SMBIOS_lion},
    {"SMBIOS_mav", SMBIOS_mav},
    {"SMBIOS_cap", SMBIOS_cap},
    {"SMBIOS_hsierra", SMBIOS_hsierra},
    {"SMBIOS_moja", SMBIOS_moja},
    {"SMBIOS_cata", SMBIOS_cata},
    {"SMBIOS_bigsur", SMBIOS_bigsur},
    {"SMBIOS_monterey", SMBIOS_monterey},
    {"SMBIOS_ventura", SMBIOS_ventura},
    {"SMBIOS_sonoma", SMBIOS_sonoma},
    {"SystemParameters", SystemParameters},
    {"RtVariables", RtVariables},
    {"Quirks", Quirks},
  };

public:
  ConfigPlistClass() : Graphics(*this) {}
  
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

  virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
    bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
    if ( LString8(ACPI.DSDT.Fixes.ACPI_DSDT_Fixe_Array[29].getNewName()) != "FixHeaders_20000000"_XS8 ) {
      log_technical_bug("ACPI_DSDT_Fixe_Array[29].getNewName() != \"FixHeaders_20000000\"");
    }
    if ( ACPI.getFixHeaders().isDefined()  &&  ACPI.DSDT.Fixes.ACPI_DSDT_Fixe_Array[29].isDefined() ) {
      if ( ACPI.getFixHeaders().value() == ACPI.DSDT.Fixes.ACPI_DSDT_Fixe_Array[29].dgetEnabled() ) {
        xmlLiteParser->addWarning(generateErrors, S8Printf("FixHeaders exists in ACPI and ACPI/DSDT/Fixes. Delete FixHeaders from ACPI/DSDT/Fixes."));
      }else{
        if ( ACPI.getFixHeaders().value()  ||  ACPI.DSDT.Fixes.ACPI_DSDT_Fixe_Array[29].dgetEnabled() ) {
          if ( ACPI.getFixHeaders().value() ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("FixHeaders exists in ACPI and ACPI/DSDT/Fixes with a different value. Using value of ACPI/FixHeaders. Delete FixHeaders from ACPI/DSDT/Fixes."));
          }else{
            xmlLiteParser->addWarning(generateErrors, S8Printf("FixHeaders exists in ACPI and ACPI/DSDT/Fixes with a different value. Using value of ACPI/DSDT/Fixes/FixHeaders. Delete FixHeaders from ACPI/DSDT/Fixes and set ACPI/FixHeaders to true."));
          }
        }
      }
    }
    return b;
  }

  const decltype(DisableDrivers)::ValueType& dgetDisabledDriverArray() const { return DisableDrivers.isDefined() ? DisableDrivers.value() : DisableDrivers.nullValue; };
  const decltype(SMBIOS)& getSMBIOS() const {
    for (size_t i=0; i<sizeof(m_fields)/sizeof(m_fields[0]); i++) {
      XStringW h;
      h.takeValueFrom(m_fields[i].m_name);
      if (SmbiosList.size() > 0 && SmbiosList[OldChosenSmbios] == h) {
        return static_cast<SmbiosPlistClass::SmbiosDictClass&>(m_fields[i].xmlAbstractType);
      }
    }
    return SMBIOS;
  };
};

#endif /* _CONFIGPLISTCLASS_H_ */

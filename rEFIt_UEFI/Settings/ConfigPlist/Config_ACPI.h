/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_ACPI_H_
#define _CONFIGPLISTCLASS_ACPI_H_


#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"


class ACPI_Class : public XmlDict
{
  using super = XmlDict;
public:


  #include "Config_ACPI_DSDT.h"
  #include "Config_ACPI_SSDT.h"

  class RenameDict : public XmlRepeatingDict<XmlAddKey<XmlKey, XmlString8>>
  {
    public:
      RenameDict() { ignoreCommented = false; } // TODO: put back true.
  };
  
  class ACPI_RenamesDevices_Class : public XmlUnion
  {
    protected:
    public:
      RenameDict RenamesDevicesAsDict = RenameDict();
      XmlArray<RenameDict> RenamesDevicesAsArray = XmlArray<RenameDict>();

    protected:
      XmlUnionField m_fieldsZ[2] = { RenamesDevicesAsDict, RenamesDevicesAsArray };
      virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fieldsZ; *nb = sizeof(m_fieldsZ)/sizeof(m_fieldsZ[0]); };

    public:
      ACPI_RenamesDevices_Class() { RenamesDevicesAsDict.ignoreCommented = false; };

      size_t size() const {
        if ( RenamesDevicesAsDict.isDefined() ) return RenamesDevicesAsDict.valueArray().size();
        size_t size = 0;
        for ( size_t idx = 0 ; idx < RenamesDevicesAsArray.size() ; ++idx ) {
          auto RenameDict = RenamesDevicesAsArray[idx];
          size += RenameDict.valueArray().size();
        }
        return size;
      }
      
      const XmlAddKey<XmlKey, XmlString8>& getAtIndex(size_t pos) const {
        if ( RenamesDevicesAsDict.isDefined() ) return RenamesDevicesAsDict.valueArray()[pos];
        size_t currentPos = 0;
        for ( size_t idx = 0 ; idx < RenamesDevicesAsArray.size() ; ++idx ) {
          auto& RenameDict = RenamesDevicesAsArray[idx];
          for ( size_t jdx = 0 ; jdx < RenameDict.valueArray().size() ; ++jdx ) {
            if ( currentPos == pos ) {
              auto& v = RenameDict.valueArray()[jdx];
              return v;
            }
            ++currentPos;
          }
        }
        panic("pos > length");
      }
  };

  class ACPI_DropTables_Class : public XmlDict
  {
    using super = XmlDict;
    protected:
      class Signature_Class : public XmlString8AllowEmpty
      {
        using super = XmlString8AllowEmpty;
        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
          if ( !isDefined() ) return true;
          if ( xstring8.length() == 4 ) return true;
          xmlLiteParser->addWarning(generateErrors, S8Printf("Expect a string of 4 chars for tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
          return true; // TODO : we should return false, but currently Clover accept string that are not 4 chars long.
//          return false; // parsing can continue.
        }

      } Signature = Signature_Class();
      XmlString8AllowEmpty TableId = XmlString8AllowEmpty(); // TODO:chekc that all chars are ASCII
      XmlUInt32  Length = XmlUInt32();
      XmlBool DropForAllOS = XmlBool();

      XmlDictField m_fields[4] = {
        {"Signature", Signature},
        {"TableId", TableId},
        {"Length", Length},
        {"DropForAllOS", DropForAllOS},
      };

    public:
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

      UINT32 dgetSignature() const { return Signature.isDefined() ? SIGNATURE_32(Signature.value()[0], Signature.value()[1], Signature.value()[2], Signature.value()[3]) : 0; };
      UINT64 dgetTableId() const {
        if ( !TableId.isDefined() ) return 0;
        CHAR8  Id[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        for ( size_t idx = 0 ; idx < TableId.value().length() ; idx++ ) {
          //      DBG("%c", *Str);
          Id[idx] = (char)TableId.value()[idx]; // safe cast when TableId would have been validated !
        }
        return *(UINT64*)&Id;
      };
      UINT32 dgetTabLength() const { return Length.isDefined() ? Length.value() : 0; };
      bool dgetOtherOS() const { return DropForAllOS.isDefined() ? DropForAllOS.value() : false; };


  };




  XmlArray<ACPI_DropTables_Class> ACPIDropTablesArray = XmlArray<ACPI_DropTables_Class>();
  DSDT_Class DSDT = DSDT_Class();
  SSDT_Class SSDT = SSDT_Class();
protected:
  XmlUInt64 ResetAddress = XmlUInt64();
  XmlUInt8 ResetValue = XmlUInt8();
  XmlBool HaltEnabler = XmlBool();
  XmlBool FixHeaders = XmlBool();
  XmlBool FixMCFG = XmlBool();
  XmlBool DisableASPM = XmlBool();
  XmlBool smartUPS = XmlBool();
  XmlBool PatchAPIC = XmlBool();
  XmlString8Array SortedOrder = XmlString8Array();
  XmlBool AutoMerge = XmlBool();
  XmlStringWArray DisabledAML = XmlStringWArray();
public:
  ACPI_RenamesDevices_Class RenameDevices = ACPI_RenamesDevices_Class();

  XmlDictField m_fields[15] = {
    {"DropTables", ACPIDropTablesArray},
    {"DSDT", DSDT},
    {"SSDT", SSDT},
    {"ResetAddress", ResetAddress},
    {"ResetValue", ResetValue},
    {"HaltEnabler", HaltEnabler},
    {"FixHeaders", FixHeaders},
    {"FixMCFG", FixMCFG},
    {"DisableASPM", DisableASPM},
    {"smartUPS", smartUPS},
    {"PatchAPIC", PatchAPIC},
    {"SortedOrder", SortedOrder},
    {"AutoMerge", AutoMerge},
    {"DisabledAML", DisabledAML},
    {"RenameDevices", RenameDevices},
  };

public:
  ACPI_Class() {};
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  
//  const XmlArray<ACPI_DropTables_Class>& getDropTables() const { return DropTables; };
//  const DSDT_Class& getDSDT() const { return DSDT; };
//  const SSDT_Class& getSSDT() const { return SSDT; };
  uint64_t dgetResetAddr() const { return ResetAddress.isDefined() ? ResetAddress.value() : 0; };
  uint8_t dgetResetVal() const
  {
    if ( ResetValue.isDefined() ) return ResetValue.value();
    if ( dgetResetAddr() == 0x64) return 0xFE;
    if ( dgetResetAddr() == 0xCF9) return 0x06;
    return 0;
  }
  
  const decltype(FixHeaders)& getFixHeaders() const { return FixHeaders; };

  bool dgetSlpSmiEnable() const { return HaltEnabler.isDefined() ? HaltEnabler.value() : false; };
  bool dgetFixHeaders() const { return FixHeaders.isDefined() ? FixHeaders.value() : false; };
  bool dgetFixMCFG() const { return FixMCFG.isDefined() ? FixMCFG.value() : false; };
  bool dgetNoASPM() const { return DisableASPM.isDefined() ? DisableASPM.value() : false; };
  bool dgetsmartUPS() const { return smartUPS.isDefined() ? smartUPS.value() : false; };
  bool dgetPatchNMI() const { return PatchAPIC.isDefined() ? PatchAPIC.value() : false; };
  bool dgetAutoMerge() const { return AutoMerge.isDefined() ? AutoMerge.value() : false; };
  const decltype(DisabledAML)::ValueType& dgetDisabledAML() const { return DisabledAML.isDefined() ? DisabledAML.value() : DisabledAML.nullValue; };
  const decltype(SortedOrder)::ValueType& dgetSortedACPI() const { return SortedOrder.isDefined() ? SortedOrder.value() : SortedOrder.nullValue; };

//  const XmlArray<ACPI_DropTables_Class>& dgetACPIDropTablesArray() const { return DropTables.isDefined() ? DropTables : XmlArray<ACPI_DropTables_Class>::NullArray; };

};


#endif /* _CONFIGPLISTCLASS_ACPI_H_ */

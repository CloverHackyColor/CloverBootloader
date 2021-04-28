/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_ACPI_SSDT_H_
#define _CONFIGPLISTCLASS_ACPI_SSDT_H_


#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"


class SSDT_Generate_Class : public XmlDict
{
  using super = XmlDict;
protected:
//  // Instead of being a dict, Generate can be just a property true or false.
//  XmlBool isTrue;
  
  // If isTrue is undefined.
  XmlBool PStates = XmlBool();
  XmlBool CStates = XmlBool();
  XmlBool APSN = XmlBool();
  XmlBool APLF = XmlBool();
  XmlBool PluginType = XmlBool();

  XmlDictField m_fields[5] = {
    {"PStates", PStates},
    {"CStates", CStates},
    {"APSN", APSN},
    {"APLF", APLF},
    {"PluginType", PluginType},
  };
  public:
    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

  bool dgetPStates() const { return PStates.isDefined() ? PStates.value() : false; };
  bool dgetCStates() const { return CStates.isDefined() ? CStates.value() : false; };
  bool dgetAPSN() const { return APSN.isDefined() ? APSN.value() : false; };
  bool dgetAPLF() const { return APLF.isDefined() ? APLF.value() : false; };
  bool dgetPluginType() const { return PluginType.isDefined() ? PluginType.value() : false; };

  undefinable_bool getPStates() const { return PStates.isDefined() ? undefinable_bool(PStates.value()) : undefinable_bool(); };
  undefinable_bool getCStates() const { return CStates.isDefined() ? undefinable_bool(CStates.value()) : undefinable_bool(); };
  undefinable_bool getAPSN() const { return APSN.isDefined() ? undefinable_bool(APSN.value()) : undefinable_bool(); };
  undefinable_bool getAPLF() const { return APLF.isDefined() ? undefinable_bool(APLF.value()) : undefinable_bool(); };
  undefinable_bool getPluginType() const { return PluginType.isDefined() ? undefinable_bool(PluginType.value()) : undefinable_bool(); };

};

class SSDT_Class : public XmlDict
{
  using super = XmlDict;
public:
//Generate
  class XmlUnionGenerate : public XmlUnion
  {
  protected:
    XmlBool xmlBool = XmlBool();
    SSDT_Generate_Class xmlDict = SSDT_Generate_Class();
    XmlUnionField m_fields[2] = { xmlBool, xmlDict};
  public:
    virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
    
    bool dgetGeneratePStates() const {
      if ( xmlBool.isDefined() ) return xmlBool.value();
      return xmlDict.dgetPStates();
    }
    bool dgetGenerateCStates() const {
      if ( xmlBool.isDefined() ) return xmlBool.value();
      return xmlDict.dgetCStates();
    }
    bool dgetGenerateAPSN() const {
      if ( xmlBool.isDefined() ) return xmlBool.value();
      return xmlDict.dgetAPSN();
    }
    bool dgetGenerateAPLF() const {
      if ( xmlBool.isDefined() ) return xmlBool.value();
      return xmlDict.dgetAPLF();
    }
    bool dgetGeneratePluginType() const {
      if ( xmlBool.isDefined() ) return xmlBool.value();
      return xmlDict.dgetPluginType();
    }

    
    undefinable_bool getGeneratePStates() const {
      if ( xmlBool.isDefined() ) return undefinable_bool(xmlBool.value());
      return xmlDict.getPStates();
    }
    undefinable_bool getGenerateCStates() const {
      if ( xmlBool.isDefined() ) return undefinable_bool(xmlBool.value());
      return xmlDict.getCStates();
    }
    undefinable_bool getGenerateAPSN() const {
      if ( xmlBool.isDefined() ) return undefinable_bool(xmlBool.value());
      return xmlDict.getAPSN();
    }
    undefinable_bool getGenerateAPLF() const {
      if ( xmlBool.isDefined() ) return undefinable_bool(xmlBool.value());
      return xmlDict.getAPLF();
    }
    undefinable_bool getGeneratePluginType() const {
      if ( xmlBool.isDefined() ) return undefinable_bool(xmlBool.value());
      return xmlDict.getPluginType();
    }

  };

public:
  XmlUnionGenerate Generate = XmlUnionGenerate();
protected:
  XmlBool DropOem = XmlBool();
  XmlBool NoOemTableId = XmlBool();
  XmlBool NoDynamicExtract = XmlBool();
  XmlBool UseSystemIO = XmlBool();
  XmlBool EnableC7 = XmlBool();
  XmlBool EnableC6 = XmlBool();
  XmlBool EnableC4 = XmlBool();
  XmlBool EnableC2 = XmlBool();
  XmlUInt16 C3Latency = XmlUInt16();
  XmlUInt8 PLimitDict = XmlUInt8();
  XmlUInt8 UnderVoltStep = XmlUInt8();
  XmlBool DoubleFirstState = XmlBool();
  XmlUInt8 MinMultiplier = XmlUInt8();
  XmlUInt8 MaxMultiplier = XmlUInt8();
  XmlUInt8 PluginType = XmlUInt8();
  
  XmlDictField m_fields[16] = {
    {"Generate", Generate},
    {"DropOem", DropOem},
    {"NoOemTableId", NoOemTableId},
    {"NoDynamicExtract", NoDynamicExtract},
    {"UseSystemIO", UseSystemIO},
    {"EnableC7", EnableC7},
    {"EnableC6", EnableC6},
    {"EnableC4", EnableC4},
    {"EnableC2", EnableC2},
    {"C3Latency", C3Latency},
    {"PLimitDict", PLimitDict},
    {"UnderVoltStep", UnderVoltStep},
    {"DoubleFirstState", DoubleFirstState},
    {"MinMultiplier", MinMultiplier},
    {"MaxMultiplier", MaxMultiplier},
    {"PluginType", PluginType},
  };


public:
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  
  bool dgetDropSSDTSetting() const { return DropOem.isDefined() ? DropOem.value() : false; };
  bool dgetNoOemTableId() const { return NoOemTableId.isDefined() ? NoOemTableId.value() : false; };
  bool dgetNoDynamicExtract() const { return NoDynamicExtract.isDefined() ? NoDynamicExtract.value() : false; };
  bool dgetEnableISS() const { return UseSystemIO.isDefined() ? UseSystemIO.value() : false; };
  bool dgetEnableC7() const { return EnableC7.isDefined() ? EnableC7.value() : false; };
  bool dget_EnableC6() const { return EnableC6.isDefined() ? EnableC6.value() : false; };
  bool dget_EnableC4() const { return EnableC4.isDefined() ? EnableC4.value() : false; };
  bool dget_EnableC2() const { return EnableC2.isDefined() ? EnableC2.value() : false; };
  uint16_t dget_C3Latency() const { return C3Latency.isDefined() ? C3Latency.value() : 0; };
  uint8_t dgetPLimitDict() const { return PLimitDict.isDefined() ? PLimitDict.value() : 0; };
  uint8_t dgetUnderVoltStep() const { return UnderVoltStep.isDefined() ? UnderVoltStep.value() : 0; };
  bool dgetDoubleFirstState() const { return DoubleFirstState.isDefined() ? DoubleFirstState.value() : false; };
  uint8_t dgetMinMultiplier() const { return MinMultiplier.isDefined() ? MinMultiplier.value() : 0; };
  uint8_t dgetMaxMultiplier() const { return MaxMultiplier.isDefined() ? MaxMultiplier.value() : 0; };
  uint8_t dgetPluginType() const { return PluginType.isDefined() ? PluginType.value() : 0; };

  undefinable_bool getEnableC6() const { return EnableC6.isDefined() ? undefinable_bool(EnableC6.value()) : undefinable_bool(); };
  undefinable_bool getPluginType() const { return PluginType.isDefined() ? undefinable_bool(PluginType.value()) : undefinable_bool(); };
  undefinable_uint8 getMinMultiplier() const { return MinMultiplier.isDefined() ? undefinable_uint8(MinMultiplier.value()) : undefinable_uint8(); };
  undefinable_uint16 getC3Latency() const { return C3Latency.isDefined() ? undefinable_uint16(C3Latency.value()) : undefinable_uint16(); };

};


#endif /* _CONFIGPLISTCLASS_ACPI_SSDT_H_ */

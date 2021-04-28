/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_DEVICES_H_
#define _CONFIGPLISTCLASS_DEVICES_H_



class DevicesClass : public XmlDict
{
  using super = XmlDict;
public:

  class XmlPropertyValue: public XmlUnion
  {
    protected:
      XmlString8AllowEmpty xmlString8 = XmlString8AllowEmpty();
      XmlUInt32 xmlUInt32 = XmlUInt32();
      XmlBool xmlBool = XmlBool();
      XmlData xmlData = XmlData();
      XmlUnionField m_fields[3] = { xmlString8, xmlData, xmlUInt32 };
      virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
    public:
      const XBuffer<uint8_t> value() const {
        if ( !isDefined()) return XBuffer<uint8_t>::NullXBuffer;
        if ( xmlString8.isDefined() ) return XBuffer<uint8_t>(xmlString8.value().c_str(), xmlString8.value().sizeInBytesIncludingTerminator());
        if ( xmlUInt32.isDefined() ) return XBuffer<uint8_t>(&xmlUInt32.value(), sizeof(decltype(xmlUInt32.value())));
        if ( xmlBool.isDefined() ) { uint32_t ui32 = xmlBool.value(); return XBuffer<uint8_t>(&ui32, sizeof(ui32)); };
        if ( xmlData.isDefined() ) return xmlData.value();
        return XBuffer<uint8_t>::NullXBuffer;
      }
    TAG_TYPE valueType() const {
      if ( !isDefined()) return kTagTypeNone;
      if ( xmlString8.isDefined() ) return kTagTypeString;
      if ( xmlUInt32.isDefined() ) return kTagTypeInteger;
      if ( xmlData.isDefined() ) return kTagTypeData;
      panic("There is a bug : one of the field must be defined the union is defined");
    }
  };

  class SimplePropertyClass_Class : public XmlDict
  {
    using super = XmlDict;
  public:
    XmlBool                 disabled = XmlBool();
    XmlString8AllowEmpty    key = XmlString8AllowEmpty();
    XmlPropertyValue    value = XmlPropertyValue();

    XmlDictField m_fields[3] = {
      {"disabled", disabled},
      {"key", key},
      {"value", value},
    };

    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

    const decltype(key)::ValueType& dgetKey() const { return key.isDefined() ? key.value() : key.nullValue; };
    XBuffer<uint8_t> dgetValue() const { return value.isDefined() ? value.value() : XBuffer<uint8_t>::NullXBuffer; };
    TAG_TYPE dgetValueType() const { return value.isDefined() ? value.valueType() : kTagTypeNone; };
    uint8_t dgetDisabled() const { return disabled.isDefined() ? disabled.value() : disabled.nullValue; };

  };

  #include "Config_Devices_Audio.h"
  #include "Config_Devices_AddProperties.h"
  #include "Config_Devices_Arbitrary.h"
  #include "Config_Devices_Properties.h"
  #include "Config_Devices_FakeID.h"
  #include "Config_Devices_USB.h"

protected:
  
  XmlBool Inject = XmlBool();
  XmlBool SetIntelBacklight = XmlBool();
  XmlBool SetIntelMaxBacklight = XmlBool();
  XmlUInt32 IntelMaxValue = XmlUInt32();
  XmlBool LANInjection = XmlBool();
  XmlBool HDMIInjection = XmlBool();
  XmlBool NoDefaultProperties = XmlBool();
  XmlBool UseIntelHDMI = XmlBool();
  XmlBool ForceHPET = XmlBool();
  XmlUInt32 DisableFunctions = XmlUInt32();
  XmlString8AllowEmpty AirportBridgeDeviceName = XmlString8AllowEmpty();
public:
  Devices_Audio_Class Audio = Devices_Audio_Class();
  Devices_FakeID_Class FakeID = Devices_FakeID_Class();
  Devices_USB_Class USB = Devices_USB_Class();
  
  PropertiesUnion Properties = PropertiesUnion();
  XmlArray<Devices_Arbitrary_Class> Arbitrary = XmlArray<Devices_Arbitrary_Class>();
  XmlArray<Devices_AddProperties_Dict_Class> AddProperties = XmlArray<Devices_AddProperties_Dict_Class>();

protected:
  XmlDictField m_fields[17] = {
    {"Inject", Inject},
    {"SetIntelBacklight", SetIntelBacklight},
    {"SetIntelMaxBacklight", SetIntelMaxBacklight},
    {"IntelMaxValue", IntelMaxValue},
    {"LANInjection", LANInjection},
    {"HDMIInjection", HDMIInjection},
    {"NoDefaultProperties", NoDefaultProperties},
    {"FakeID", FakeID},
    {"UseIntelHDMI", UseIntelHDMI},
    {"ForceHPET", ForceHPET},
    {"DisableFunctions", DisableFunctions},
    {"AirportBridgeDeviceName", AirportBridgeDeviceName},
    {"Audio", Audio},
    {"USB", USB},
    {"Properties", Properties},
    {"Arbitrary", Arbitrary},
    {"AddProperties", AddProperties},
  };
  
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  
  public:
    const decltype(Inject)::ValueType& dgetStringInjector() const { return Inject.isDefined() ? Inject.value() : Inject.nullValue; };
    const decltype(SetIntelBacklight)::ValueType& dgetIntelBacklight() const { return SetIntelBacklight.isDefined() ? SetIntelBacklight.value() : SetIntelBacklight.nullValue; };
    const decltype(SetIntelMaxBacklight)::ValueType& dgetIntelMaxBacklight() const { return SetIntelMaxBacklight.isDefined() ? SetIntelMaxBacklight.value() : SetIntelMaxBacklight.nullValue; };
    const decltype(IntelMaxValue)::ValueType& dgetIntelMaxValue() const { return IntelMaxValue.isDefined() ? IntelMaxValue.value() : IntelMaxValue.nullValue; };
    bool dgetLANInjection() const { return isDefined() ? LANInjection.isDefined() ? LANInjection.value() : true : false; }; // TODO: different default value if section is not defined
    const decltype(HDMIInjection)::ValueType& dgetHDMIInjection() const { return HDMIInjection.isDefined() ? HDMIInjection.value() : HDMIInjection.nullValue; };
    bool dgetNoDefaultProperties() const { return isDefined() ? NoDefaultProperties.isDefined() ? NoDefaultProperties.value() : true : false; }; // TODO: different default value if section is not defined
    const decltype(UseIntelHDMI)::ValueType& dgetUseIntelHDMI() const { return UseIntelHDMI.isDefined() ? UseIntelHDMI.value() : UseIntelHDMI.nullValue; };
    const decltype(ForceHPET)::ValueType& dgetForceHPET() const { return ForceHPET.isDefined() ? ForceHPET.value() : ForceHPET.nullValue; };
    const decltype(DisableFunctions)::ValueType& dgetDisableFunctions() const { return DisableFunctions.isDefined() ? DisableFunctions.value() : DisableFunctions.nullValue; };
    const decltype(AirportBridgeDeviceName)::ValueType& dgetAirportBridgeDeviceName() const { return AirportBridgeDeviceName.isDefined() ? AirportBridgeDeviceName.value() : AirportBridgeDeviceName.nullValue; };
};


#endif /* _CONFIGPLISTCLASS_DEVICES_H_ */

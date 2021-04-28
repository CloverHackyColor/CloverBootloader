/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_DEVICES_ADDPROPERTIES_H_
#define _CONFIGPLISTCLASS_DEVICES_ADDPROPERTIES_H_

class Devices_AddProperties_Dict_Class : public XmlDict
{
  using super = XmlDict;
public:

  XmlString8AllowEmpty Device = XmlString8AllowEmpty();
  XmlBool Disabled = XmlBool();
  XmlString8AllowEmpty Key = XmlString8AllowEmpty();
  XmlPropertyValue Value = XmlPropertyValue();

  XmlDictField m_fields[4] = {
    {"Device", Device},
    {"Disabled", Disabled},
    {"Key", Key},
    {"Value", Value},
  };
  
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  
public:
  uint32_t dgetDevice() const {
    if ( !Device.isDefined() ) return 0;
    if ( Device.value().isEqualIC("ATI") ) return DEV_ATI;
    else if ( Device.value().isEqualIC("NVidia") ) return DEV_NVIDIA;
    else if ( Device.value().isEqualIC("IntelGFX") ) return DEV_INTEL;
    else if ( Device.value().isEqualIC("LAN") ) return DEV_LAN;
    else if ( Device.value().isEqualIC("WIFI") ) return DEV_WIFI;
    else if ( Device.value().isEqualIC("Firewire") ) return DEV_FIREWIRE;
    else if ( Device.value().isEqualIC("SATA") ) return DEV_SATA;
    else if ( Device.value().isEqualIC("IDE") ) return DEV_IDE;
    else if ( Device.value().isEqualIC("HDA") ) return DEV_HDA;
    else if ( Device.value().isEqualIC("HDMI") ) return DEV_HDMI;
    else if ( Device.value().isEqualIC("LPC") ) return DEV_LPC;
    else if ( Device.value().isEqualIC("SmBUS") ) return DEV_SMBUS;
    else if ( Device.value().isEqualIC("USB") ) return DEV_USB;
    else return 0;
  }
  const decltype(Disabled)::ValueType& dgetDisabled() const { return Disabled.isDefined() ? Disabled.value() : Disabled.nullValue; };
  const decltype(Key)::ValueType& dgetKey() const { return Key.isDefined() ? Key.value() : Key.nullValue; };
  XBuffer<uint8_t> dgetValue() const { return Value.isDefined() ? Value.value() : XBuffer<uint8_t>::NullXBuffer; };
  TAG_TYPE dgetValueType() const { return Value.isDefined() ? Value.valueType() : kTagTypeNone; };
};


#endif /* _CONFIGPLISTCLASS_DEVICES_ADDPROPERTIES_H_ */

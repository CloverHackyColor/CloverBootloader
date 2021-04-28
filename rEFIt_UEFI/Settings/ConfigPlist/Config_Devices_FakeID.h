/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_DEVICES_FAKEID_H_
#define _CONFIGPLISTCLASS_DEVICES_FAKEID_H_

class Devices_FakeID_Class : public XmlDict
{
  using super = XmlDict;
protected:
  XmlUInt32 ATI = XmlUInt32();
  XmlUInt32 NVidia = XmlUInt32();
  XmlUInt32 IntelGFX = XmlUInt32();
  XmlUInt32 LAN = XmlUInt32();
  XmlUInt32 WIFI = XmlUInt32();
  XmlUInt32 SATA = XmlUInt32();
  XmlUInt32 XHCI = XmlUInt32();
  XmlUInt32 IMEI = XmlUInt32();

  XmlDictField m_fields[8] = {
    {"ATI", ATI},
    {"NVidia", NVidia},
    {"IntelGFX", IntelGFX},
    {"LAN", LAN},
    {"WIFI", WIFI},
    {"SATA", SATA},
    {"XHCI", XHCI},
    {"IMEI", IMEI},
  };

  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  
public:
  const decltype(ATI)::ValueType& dgetFakeATI() const { return ATI.isDefined() ? ATI.value() : ATI.nullValue; };
  const decltype(NVidia)::ValueType& dgetFakeNVidia() const { return NVidia.isDefined() ? NVidia.value() : NVidia.nullValue; };
  const decltype(IntelGFX)::ValueType& dgetFakeIntel() const { return IntelGFX.isDefined() ? IntelGFX.value() : IntelGFX.nullValue; };
  const decltype(LAN)::ValueType& dgetFakeLAN() const { return LAN.isDefined() ? LAN.value() : LAN.nullValue; };
  const decltype(WIFI)::ValueType& dgetFakeWIFI() const { return WIFI.isDefined() ? WIFI.value() : WIFI.nullValue; };
  const decltype(SATA)::ValueType& dgetFakeSATA() const { return SATA.isDefined() ? SATA.value() : SATA.nullValue; };
  const decltype(XHCI)::ValueType& dgetFakeXHCI() const { return XHCI.isDefined() ? XHCI.value() : XHCI.nullValue; };
  const decltype(IMEI)::ValueType& dgetFakeIMEI() const { return IMEI.isDefined() ? IMEI.value() : IMEI.nullValue; };

};


#endif /* _CONFIGPLISTCLASS_DEVICES_FAKEID_H_ */

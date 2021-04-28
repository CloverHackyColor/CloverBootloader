/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_DEVICES_USB_H_
#define _CONFIGPLISTCLASS_DEVICES_USB_H_


#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteCompositeTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"

class Devices_USB_Class : public XmlDict
{
  using super = XmlDict;
protected:
  XmlBool Inject = XmlBool();
  XmlBool AddClockID = XmlBool();
  XmlBool FixOwnership = XmlBool();
  XmlBool HighCurrent = XmlBool();
  XmlBool NameEH00 = XmlBool();

  XmlDictField m_fields[5] = {
    {"Inject", Inject},
    {"AddClockID", AddClockID},
    {"FixOwnership", FixOwnership},
    {"HighCurrent", HighCurrent},
    {"NameEH00", NameEH00},
  };

  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

public:
  decltype(Inject)::ValueType dgetUSBInjection() const { return Inject.isDefined() ? Inject.value() : true; };
  const decltype(AddClockID)::ValueType& dgetInjectClockID() const { return AddClockID.isDefined() ? AddClockID.value() : AddClockID.nullValue; };
  const decltype(FixOwnership)::ValueType& dgetUSBFixOwnership() const { return FixOwnership.isDefined() ? FixOwnership.value() : FixOwnership.nullValue; };
  const decltype(HighCurrent)::ValueType& dgetHighCurrent() const { return HighCurrent.isDefined() ? HighCurrent.value() : HighCurrent.nullValue; };
  const decltype(NameEH00)::ValueType& dgetNameEH00() const { return NameEH00.isDefined() ? NameEH00.value() : NameEH00.nullValue; };

};


#endif /* _CONFIGPLISTCLASS_DEVICES_FAKEID_H_ */

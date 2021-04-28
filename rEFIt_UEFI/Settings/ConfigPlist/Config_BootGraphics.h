/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_BootGraphics_H_
#define _CONFIGPLISTCLASS_BootGraphics_H_


#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteCompositeTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"

class BootGraphics_Class : public XmlDict
{
  using super = XmlDict;
protected:
  XmlUInt32 DefaultBackgroundColor = XmlUInt32();
  XmlUInt32 UIScale = XmlUInt32();
  XmlUInt32 EFILoginHiDPI = XmlUInt32();
  XmlUInt32 flagstate = XmlUInt32();
  
  XmlDictField m_fields[4] = {
    {"DefaultBackgroundColor", DefaultBackgroundColor},
    {"UIScale", UIScale},
    {"EFILoginHiDPI", EFILoginHiDPI},
    {"flagstate", flagstate},
  };
  
public:
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

  decltype(DefaultBackgroundColor)::ValueType dgetDefaultBackgroundColor() const { return DefaultBackgroundColor.isDefined() ? DefaultBackgroundColor.value() : 0x80000000; };
  decltype(UIScale)::ValueType dgetUIScale() const { return UIScale.isDefined() ? UIScale.value() : 0x80000000; };
  decltype(EFILoginHiDPI)::ValueType dgetEFILoginHiDPI() const { return EFILoginHiDPI.isDefined() ? EFILoginHiDPI.value() : 0x80000000; };
  decltype(flagstate)::ValueType dget_flagstate() const { return flagstate.isDefined() ? flagstate.value() : 0x80000000; };
};


#endif /* _CONFIGPLISTCLASS_BootGraphics_H_ */

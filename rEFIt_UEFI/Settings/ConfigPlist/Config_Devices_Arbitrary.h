/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_DEVICES_ARBITRARY_H_
#define _CONFIGPLISTCLASS_DEVICES_ARBITRARY_H_

class Devices_Arbitrary_Class : public XmlDict
{
  using super = XmlDict;
public:

//    class PciAddrClass : public XmlString8AllowEmpty {
//      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
//        RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
//        if ( isDefined() && value().length() != 8 ) return xmlLiteParser->addError(generateErrors, S8Printf("PciAddr must be 8 chars long (ex 00:00.01) at line %d.", keyPos.getLine()));
//        return true;
//      }
//    };

protected:
  XmlString8AllowEmpty PciAddr = XmlString8AllowEmpty();
  XmlString8AllowEmpty Comment = XmlString8AllowEmpty();
public:
  XmlArray<SimplePropertyClass_Class> CustomProperties = XmlArray<SimplePropertyClass_Class>();
  
  XmlDictField m_fields[3] = {
    {"PciAddr", PciAddr},
    {"Comment", Comment},
    {"CustomProperties", CustomProperties},
  };

  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

  const decltype(PciAddr)::ValueType& dgetPciAddr() const { return PciAddr.isDefined() ? PciAddr.value() : PciAddr.nullValue; };
  decltype(Comment)::ValueType dgetLabel() const {
    XString8 Label;
    if ( PciAddr.isDefined()) {
      uint8_t Bus   = hexstrtouint8(PciAddr.value().c_str());
      uint8_t Dev   = hexstrtouint8(PciAddr.value().c_str() + 3);
      uint8_t Func  = hexstrtouint8(PciAddr.value().c_str() + 6);
      Label.S8Catf("[%02hhX:%02hhX.%02hhX] ", Bus, Dev, Func);
    }
    if ( Comment.isDefined() ) Label.strcat(Comment.value());
    return Label;
  }

  uint32_t dgetDevice() const {
    if ( !PciAddr.isDefined() ) return 0;
    uint8_t Bus   = hexstrtouint8(PciAddr.value().c_str());
    uint8_t Dev   = hexstrtouint8(PciAddr.value().c_str() + 3);
    uint8_t Func  = hexstrtouint8(PciAddr.value().c_str() + 6);
    return PCIADDR(Bus, Dev, Func);
  }
};



#endif /* _CONFIGPLISTCLASS_DEVICES_ARBITRARY_H_ */

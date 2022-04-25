/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_SYSTEMPARAMETERS_H_
#define _CONFIGPLISTCLASS_SYSTEMPARAMETERS_H_


class SystemParameters_Class : public XmlDict
{
  using super = XmlDict;
public:
  class InjectKextsClass: public XmlBoolOrString {
    using super = XmlBoolOrString;
    virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
      bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
      if ( xmlString8.isDefined() ) {
        if ( !xmlString8.value().isEqualIC("Detect") ) b = xmlLiteParser->addWarning(generateErrors, S8Printf("InjectKexts must be a boolean or \"Detect\" in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
      }
      return b;
    }
  };
  class CustomUUIDClass : public XmlString8AllowEmpty {
    using super = XmlString8AllowEmpty;
    virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
      bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
      if ( !EFI_GUID::IsValidGuidString(xstring8) ) b = xmlLiteParser->addWarning(generateErrors, S8Printf(" invalid CustomUUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX in dict '%s:%d'", xstring8.c_str(), xmlPath.c_str(), keyPos.getLine()));
      return b;
    }
  };

  class BacklightLevelClass : public XmlUnion
  {
    using super = XmlUnion;
  public:
    XmlInt16 xmlInt16 = XmlInt16();
    XmlString8AllowEmpty xmlString8 = XmlString8AllowEmpty();
    virtual const char* getDescription() override { return "int16|string"; };
    XmlUnionField m_fields[2] = { xmlInt16, xmlString8 };
    virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  };

  InjectKextsClass InjectKexts = InjectKextsClass();
  XmlBool NoCaches = XmlBool();
  //XmlFloat BlueValue = XmlFloat(); TODO
  BacklightLevelClass BacklightLevel = BacklightLevelClass();
  CustomUUIDClass CustomUUID = CustomUUIDClass();
  XmlBool InjectSystemID = XmlBool();
  XmlBool NvidiaWeb = XmlBool();

  XmlDictField m_fields[6] = {
    {"InjectKexts", InjectKexts},
    {"NoCaches", NoCaches},
  //  {"BlueValue", BlueValue},
    {"BacklightLevel", BacklightLevel},
    {"CustomUUID", CustomUUID},
    {"InjectSystemID", InjectSystemID},
    {"NvidiaWeb", NvidiaWeb},
  };

  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
  
  virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
    bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
    return b;
  }

  XBool dgetWithKexts() const { return true; } // looks like it can never be false anymore...
  XBool dgetWithKextsIfNoFakeSMC() const { return InjectKexts.xmlString8.isDefined() ? InjectKexts.xmlString8.value().isEqualIC("Detect") : false; }
  XBool dgetNoCaches() const { return NoCaches.isDefined() ? NoCaches.value() : XBool(false); }
  XBool dgetBacklightLevelConfig() const { return BacklightLevel.isDefined(); }
  uint16_t dgetBacklightLevel() const { return BacklightLevel.isDefined() &&  BacklightLevel.xmlInt16.isDefined() ? BacklightLevel.xmlInt16.value() : 0xFFFF; }
  EFI_GUID dgetCustomUuid() const {
    if ( !CustomUUID.isDefined() ) return nullGuid;
    EFI_GUID g;
    g.takeValueFrom(CustomUUID.value());
    return g;
  }
  UINT8 dget_InjectSystemID() const { return InjectSystemID.isDefined() ? (int)InjectSystemID.value() : 2; }
  XBool dgetNvidiaWeb() const { return NvidiaWeb.isDefined() ? NvidiaWeb.value() : XBool(false); }
};


#endif /* _CONFIGPLISTCLASS_SYSTEMPARAMETERS_H_ */

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
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
      if ( isDefined() && xmlString8.isDefined() ) {
        if ( !xmlString8.value().isEqualIC("Detect") ) return xmlLiteParser->addWarning(generateErrors, S8Printf("InjectKexts must be a boolean or \"Detect\" in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
      }
      return true;
    }
  };
  class CustomUUIDClass : public XmlString8AllowEmpty {
    using super = XmlString8AllowEmpty;
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
      if ( !isDefined() ) return true;
      if ( !IsValidGuidString(xstring8) ) return xmlLiteParser->addWarning(generateErrors, S8Printf(" invalid CustomUUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX in dict '%s:%d'", xstring8.c_str(), xmlPath.c_str(), keyPos.getLine()));
      return true;
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
  
  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
    if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
    bool b = true;
    return b;
  }

  bool dgetWithKexts() const { return true; } // looks like it can never be false anymore...
  bool dgetWithKextsIfNoFakeSMC() const { return InjectKexts.xmlString8.isDefined() ? InjectKexts.xmlString8.value().isEqualIC("Detect") : false; }
  bool dgetNoCaches() const { return NoCaches.isDefined() ? NoCaches.value() : false; }
  bool dgetBacklightLevelConfig() const { return BacklightLevel.isDefined(); }
  uint16_t dgetBacklightLevel() const { return BacklightLevel.isDefined() &&  BacklightLevel.xmlInt16.isDefined() ? BacklightLevel.xmlInt16.value() : 0xFFFF; }
  const XString8& dgetCustomUuid() const { return CustomUUID.isDefined() ? CustomUUID.value() : NullXString8; }
  UINT8 dget_InjectSystemID() const { return InjectSystemID.isDefined() ? InjectSystemID.value() : 2; }
  bool dgetNvidiaWeb() const { return NvidiaWeb.isDefined() ? NvidiaWeb.value() : 0; }
};


#endif /* _CONFIGPLISTCLASS_SYSTEMPARAMETERS_H_ */

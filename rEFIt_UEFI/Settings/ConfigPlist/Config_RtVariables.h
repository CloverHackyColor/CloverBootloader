/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_RTVARIABLES_H_
#define _CONFIGPLISTCLASS_RTVARIABLES_H_


class RtVariables_Class : public XmlDict
{
  using super = XmlDict;
public:

    class Devices_RtVariables_Block : public XmlDict
    {
      using super = XmlDict;
      public:
        class GuidClass : public XmlString8AllowEmpty {
          using super = XmlString8AllowEmpty;
          virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
            if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
            if ( !isDefined() ) return true;
            if ( !IsValidGuidString(xstring8) ) return xmlLiteParser->addWarning(generateErrors, S8Printf("Invalid GUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX in dict '%s:%d'", xstring8.c_str(), xmlPath.c_str(), keyPos.getLine()));
            return true;
          }
        };
      protected:
        XmlString8AllowEmpty Comment = XmlString8AllowEmpty();
        XmlBool Disabled = XmlBool();
        XmlStringW Name = XmlStringW();
      public:
        GuidClass Guid = GuidClass();

        XmlDictField m_fields[4] = {
          {"Comment", Comment},
          {"Disabled", Disabled},
          {"Guid", Guid},
          {"Name", Name},
        };

        virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

      public:
        const decltype(Comment)::ValueType& dgetComment() const { return Comment.isDefined() ? Comment.value() : Comment.nullValue; };
        const decltype(Disabled)::ValueType& dgetDisabled() const { return Disabled.isDefined() ? Disabled.value() : Disabled.nullValue; };
        uint8_t dgetBValue() const { return Disabled.isDefined() ? Disabled.value() : Disabled.nullValue; };
        const EFI_GUID dgetGuid() const {
          EFI_GUID efiGuid;
          EFI_STATUS Status;
          if ( Guid.isDefined() ) {
            Status = StrToGuidBE(Guid.value(), &efiGuid);
            if ( EFI_ERROR(Status) ) panic("StrToGuidBE failed. This could not happen because Guid is checked to be valid. Did you comment out the field validation ?");
            return efiGuid;
          }
          return nullGuid;
        };
        const decltype(Name)::ValueType& dgetName() const { return Name.isDefined() ? Name.value() : Name.nullValue; };
    };

  class ROMClass: public XmlUnion
  {
    protected:
      XmlString8AllowEmpty xmlString8 = XmlString8AllowEmpty();
      XmlData    xmlData = XmlData();
      XmlUnionField m_fields[2] = { xmlString8, xmlData};
      virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
//    virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override
//    {
//      WARNING_IF_DEFINED;
//
//      XmlParserPosition pos = xmlLiteParser->getPosition();
//      if ( xmlString8.parseFromXmlLite(xmlLiteParser, xmlPath, false ) ) {
//        if ( xmlString8.value().isEqualIC("UseMacAddr0") ) return true;
//        if ( xmlString8.value().isEqualIC("UseMacAddr1") ) return true;
//      }
//      xmlLiteParser->restorePosition(pos);
//      if ( xmlData.parseFromXmlLite(xmlLiteParser, xmlPath, false) ) return true;
//      xmlLiteParser->addError(generateErrors, S8Printf("Expecting \"UseMacAddr0\", \"UseMacAddr1\", a string or data for tag '%s:%d'.", xmlPath.c_str(), pos.getLine()));
//      return false;
//    }
//    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
//if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
//      if ( !xmlString8.isDefined() ) return true;
//      if ( xmlString8.value().isEqualIC("UseMacAddr0") ) return true;
//      if ( xmlString8.value().isEqualIC("UseMacAddr1") ) return true;
//      xmlLiteParser->addWarning(generateErrors, S8Printf("Expecting an integer or \"Detect\" or \"No\" for tag '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
//      return false;
//    }
    public:
//      const decltype(xmlString8)::ValueType& dgetRtROMAsString() const { return xmlString8.isDefined() ? xmlString8.value() : xmlString8.nullValue; };
//      const decltype(xmlData)::ValueType& dgetRtROMAsData() const { return xmlData.isDefined() ? xmlData.value() : xmlData.nullValue; };

      const XString8& dgetRtROMAsString() const {
        if ( xmlString8.isDefined() ) {
          if ( xmlString8.value().isEqualIC("UseMacAddr0") ) return xmlString8.value();
          if ( xmlString8.value().isEqualIC("UseMacAddr1") ) return xmlString8.value();
        }
        return xmlString8.nullValue;
      };

      XBuffer<uint8_t> dgetRtROMAsData() const {
        XBuffer<uint8_t> return_value;
        if ( xmlString8.isDefined() ) {
          if ( xmlString8.value().isEqualIC("UseMacAddr0") ) return return_value;
          if ( xmlString8.value().isEqualIC("UseMacAddr1") ) return return_value;
          size_t binary_size = hex2bin(xmlString8.value(), NULL, 0);
          return_value.memset(0, binary_size);
          hex2bin(xmlString8.value(), return_value.data(), return_value.size());
          return return_value;
        }
        if ( xmlData.isDefined() ) return xmlData.value();
        return return_value;
      };

  };

//  class ROMClass: public XmlData {
//    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
//if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
//      if ( "UseMacAddr0"_XS8.isEqualIC(value().CData()) ) return true;
//      if ( "UseMacAddr1"_XS8.isEqualIC(value().CData()) ) return true;
//      // TODO check length and format of ROM
//      return true;
//      xmlLiteParser->addWarning(generateErrors, S8Printf("Expecting \"UseMacAddr0\", \"UseMacAddr0\" or data tag '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
//      return false;
//    }
//  };

protected:
  ROMClass ROM = ROMClass();
  XmlString8AllowEmpty MLB = XmlString8AllowEmpty();
  XmlUInt32 CsrActiveConfig = XmlUInt32();
  XmlUInt16 BooterConfig = XmlUInt16();
  XmlString8AllowEmpty BooterCfg = XmlString8AllowEmpty();
public:
  XmlArray<Devices_RtVariables_Block> Block = XmlArray<Devices_RtVariables_Block>();

  XmlDictField m_fields[6] = {
    {"ROM", ROM},
    {"MLB", MLB},
    {"CsrActiveConfig", CsrActiveConfig},
    {"BooterConfig", BooterConfig},
    {"BooterCfg", BooterCfg},
    {"Block", Block},
  };

  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

//  const XString8 dgetRtROMAsString() const {
//    if ( ROM.isDefined() && ROM.xmlString8.isDefined() ) {
//      if ( "UseMacAddr0"_XS8.isEqualIC(ROM.xmlString8.value()) ) return "UseMacAddr0"_XS8;
//      if ( "UseMacAddr1"_XS8.isEqualIC(ROM.xmlString8.value()) ) return "UseMacAddr0"_XS8;
//    }
//    return NullXString8;
//  }
  decltype(declval<ROMClass>().dgetRtROMAsString()) dgetRtROMAsString() const { return ROM.dgetRtROMAsString(); };
  decltype(declval<ROMClass>().dgetRtROMAsData()) dgetRtROMAsData() const { return ROM.dgetRtROMAsData(); };

//  const XBuffer<uint8_t> dgetRtROMAsData() const {
//    if ( !ROM.isDefined() || dgetRtROMAsString().notEmpty() ) return ROM.nullValue;
//    return ROM.value();
//  }
//
//  const decltype(ROM)::ValueType& dgetROM() const { return ROM.isDefined() ? ROM.value() : ROM.nullValue; };
  const decltype(MLB)::ValueType& dgetRtMLBSetting() const { return MLB.isDefined() ? MLB.value() : MLB.nullValue; };
  decltype(CsrActiveConfig)::ValueType dgetCsrActiveConfig() const { return CsrActiveConfig.isDefined() ? CsrActiveConfig.value() : 0xFFFF; }; // 0xFFFF = not set
  const decltype(BooterConfig)::ValueType& dgetBooterConfig() const { return BooterConfig.isDefined() ? BooterConfig.value() : BooterConfig.nullValue; };
  const decltype(BooterCfg)::ValueType& dgetBooterCfgStr() const { return BooterCfg.isDefined() ? BooterCfg.value() : BooterCfg.nullValue; };

};


#endif /* _CONFIGPLISTCLASS_RTVARIABLES_H_ */

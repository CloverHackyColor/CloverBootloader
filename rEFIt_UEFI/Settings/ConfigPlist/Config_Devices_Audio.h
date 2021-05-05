//
//  Config_Devices_Audio.h
//  ConfigPlistValidator
//
//  Created by Jief on 13/10/2020.
//  Copyright © 2020 Jief. All rights reserved.
//

#ifndef Config_Devices_Audio_h
#define Config_Devices_Audio_h

class Devices_Audio_Class : public XmlDict
{
  using super = XmlDict;
  protected:
    class InjectUnion: public XmlUnion
    {
      using super = XmlUnion;
    public:
      XmlInt32 xmlInt32 = XmlInt32();
      XmlString8AllowEmpty xmlString8 = XmlString8AllowEmpty(); // TODO: change XmlString8AllowEmpty for XmlString8AllowEmpty
      XmlUnionField m_fields[2] = { xmlInt32, xmlString8};
      virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
        RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
        if ( !xmlString8.isDefined() ) return true;
//        if ( xmlString8.value().isEqualIC("Detect") ) return true;
        if ( xmlString8.value().isEqualIC("No") ) return true;
        // TODO:check that it's an integer decimal or hex
        // TODO: check it's < INT32
//        xmlLiteParser->addWarning(generateErrors, S8Printf("Expecting an integer or \"No\" for tag '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
//        return false;
        return true;
      }
    };

    XmlBool ResetHDA = XmlBool();
    InjectUnion Inject = InjectUnion();
    XmlBool AFGLowPowerState = XmlBool();

    XmlDictField m_fields[3] = {
      {"ResetHDA", ResetHDA},
      {"Inject", Inject},
      {"AFGLowPowerState", AFGLowPowerState},
    };
  public:
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
//  virtual bool validate(XmlLiteParser* xmlLiteParser, const char* name, XmlAbstractType* xmlTyp, const XString8& xmlPath, const XmlParserPosition& pos, bool generateErrors) override;

  const decltype(ResetHDA)::ValueType& dgetResetHDA() const { return ResetHDA.isDefined() ? ResetHDA.value() : ResetHDA.nullValue; };
  const decltype(AFGLowPowerState)::ValueType& dgetAFGLowPowerState() const { return AFGLowPowerState.isDefined() ? AFGLowPowerState.value() : AFGLowPowerState.nullValue; };

  bool dgetHDAInjection() const {
     if ( !Inject.isDefined() ) return false;
     if ( Inject.xmlInt32.isDefined() ) return Inject.xmlInt32.value();
     if ( Inject.xmlString8.isDefined() ) {
       if ( Inject.xmlString8.value().startWithOrEqualToIC("n") ) return false;
       if ( Inject.xmlString8.value().startWithOrEqualToIC("0x") ) return true;
       return true;
     }
     return false;
   }
  INT32 dgetHDALayoutId() const {
    if ( !Inject.isDefined() ) return 0;
    if ( Inject.xmlInt32.isDefined() ) return Inject.xmlInt32.value();
    if ( Inject.xmlString8.isDefined() ) {
      if ( Inject.xmlString8.value().startWithOrEqualToIC("n") ) return 0;
      if ( Inject.xmlString8.value().startWithOrEqualToIC("0x") ) return (INT32)AsciiStrHexToUintn(Inject.xmlString8.value());
      UINTN n = AsciiStrDecimalToUintn(Inject.xmlString8.value());
      if ( n <= MAX_INT32 ) return (INT32)n;
      return 0;
    }
    return 0;
  }

};

#endif /* Config_Devices_Audio_h */

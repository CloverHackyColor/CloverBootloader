/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_Boot_H_
#define _CONFIGPLISTCLASS_Boot_H_


#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteCompositeTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"
#include "../../include/BootTypes.h"



class Boot_Class : public XmlDict
{
  using super = XmlDict;
  
  class XMPDetectionClass : public XmlUnion
  {
    using super = XmlUnion;
  protected:
    XmlStrictBool xmlBool = XmlStrictBool();
    XmlInt8 xmlInt8 = XmlInt8();
    XmlString8AllowEmpty xmlString8 = XmlString8AllowEmpty();
    
    XmlUnionField m_fields[3] = { xmlBool, xmlInt8, xmlString8 };
    virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
      if ( xmlBool.isDefined() ) return true; // ok, whatever bool value is
      if ( xmlInt8.isDefined() ) {
        if ( xmlInt8.value() < -1 || xmlInt8.value() > 2 ) {
          xmlLiteParser->addWarning(generateErrors, S8Printf("XMPDetection can -1, 0, 1 or 2 for tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
          xmlInt8.reset();
          xmlInt8.setInt8Value(-1);
        }
        return true;
      }
      if ( xmlString8.value().startWithOrEqualToIC("n") ) return true;
      if ( xmlString8.value().startWithOrEqualToIC("-") ) return true;
      xmlLiteParser->addWarning(generateErrors, S8Printf("Expect a boolean, an int8 or a string starting with 'n' or '-' for '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
      return false; // parsing can continue.
    }
    
    public:
    int8_t dgetValue() const {
      if ( xmlBool.isDefined() ) return xmlBool.value() ? 0 : -1;
      if ( xmlInt8.isDefined() ) return xmlInt8.value();
      if ( xmlString8.isDefined() ) {
        if ( xmlString8.value().startWithOrEqualToIC("n") ) return -1;
        if ( xmlString8.value().startWithOrEqualToIC("-") ) return -1;
      }
      return 0;
    }
  };
  
public:
  XmlInt32 Timeout = XmlInt32();
  XmlBool SkipHibernateTimeout = XmlBool();
  XmlBool DisableCloverHotkeys = XmlBool();
  XmlString8AllowEmpty BootArgs = XmlString8AllowEmpty();
  XmlBool NeverDoRecovery = XmlBool();
  XmlStringW DefaultVolume = XmlStringW();
  XmlString8AllowEmpty DefaultLoader = XmlString8AllowEmpty();
  XmlBoolYesNo Debug = XmlBoolYesNo();
  XmlBool FastBoot = XmlBool();
  XmlBool NoEarlyProgress = XmlBool();
  XmlBool NeverHibernate = XmlBool();
  XmlBool StrictHibernate = XmlBool();
  XmlBool RtcHibernateAware = XmlBool();
  XmlBool HibernationFixup = XmlBool();
  XmlBool SignatureFixup = XmlBool();
  XmlBool SecureBootSetupMode = XmlBool();
  XmlString8AllowEmpty SecureBootPolicy = XmlString8AllowEmpty();
  XmlStringWArray WhiteList = XmlStringWArray();
  XmlStringWArray BlackList = XmlStringWArray();
  XMPDetectionClass XMPDetection = XMPDetectionClass();
  XmlString8AllowEmpty Legacy = XmlString8AllowEmpty();
  XmlUInt16 LegacyBiosDefaultEntry = XmlUInt16();
  class CustomLogoUnion: public XmlUnion
  {
    using super = XmlUnion;
  public:
    XmlBool xmlBool = XmlBool();
    XmlString8AllowEmpty xmlString8 = XmlString8AllowEmpty();
    XmlData xmlData = XmlData();
    XmlUnionField m_fields[3] = { xmlBool, xmlString8, xmlData};
    virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
      if ( !xmlString8.isDefined() ) return true;
      if ( xmlString8.value().isEqualIC("Apple") ) return true;
      if ( xmlString8.value().isEqualIC("Alternate") ) return true;
      if ( xmlString8.value().isEqualIC("Theme") ) return true;
      xmlLiteParser->addWarning(generateErrors, S8Printf("Expect a boolean or \"Apple\", \"Alternate\", \"Theme\" or an image pathname for tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
      return false; // parsing can continue.
    }
  } CustomLogo = CustomLogoUnion();

  XmlDictField m_fields[23] = {
    {"Timeout", Timeout},
    {"SkipHibernateTimeout", SkipHibernateTimeout},
    {"DisableCloverHotkeys", DisableCloverHotkeys},
    {"Arguments", BootArgs},
    {"NeverDoRecovery", NeverDoRecovery},
    {"DefaultVolume", DefaultVolume},
    {"DefaultLoader", DefaultLoader},
    {"Debug", Debug},
    {"Fast", FastBoot},
    {"NoEarlyProgress", NoEarlyProgress},
    {"NeverHibernate", NeverHibernate},
    {"StrictHibernate", StrictHibernate},
    {"RtcHibernateAware", RtcHibernateAware},
    {"HibernationFixup", HibernationFixup},
    {"SignatureFixup", SignatureFixup},
    {"Secure", SecureBootSetupMode},
    {"Policy", SecureBootPolicy},
    {"WhiteList", WhiteList},
    {"BlackList", BlackList},
    {"XMPDetection", XMPDetection},
    {"Legacy", Legacy},
    {"LegacyBiosDefaultEntry", LegacyBiosDefaultEntry},
    {"CustomLogo", CustomLogo},
  };

public:
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

  /* dget method means get value and returns default if undefined. dget = short for defaultget */
  int64_t dgetTimeout() const { return Timeout.isDefined() ? Timeout.value() : -1; };
  bool dgetSkipHibernateTimeout() const { return SkipHibernateTimeout.isDefined() ? SkipHibernateTimeout.value() : false; };
  bool dgetDisableCloverHotkeys() const { return DisableCloverHotkeys.isDefined() ? DisableCloverHotkeys.value() : false; };
  const XString8& dgetBootArgs() const { return BootArgs.isDefined() ? BootArgs.value() : NullXString8; };
  bool dgetNeverDoRecovery() const { return NeverDoRecovery.isDefined() ? NeverDoRecovery.value() : false; };
  const decltype(DefaultVolume)::ValueType&  dgetDefaultVolume() const {
    if ( !DefaultVolume.isDefined() ) return DefaultVolume.nullValue;
    if ( DefaultVolume.value().isEqualIC("LastBootedVolume") ) return DefaultVolume.nullValue;
    return DefaultVolume.isDefined() ? DefaultVolume.value() : DefaultVolume.nullValue;
  };
  const XString8&  dgetDefaultLoader() const { return DefaultLoader.isDefined() ? DefaultLoader.value() : NullXString8; };
  bool dgetDebugLog() const { return Debug.isDefined() ? Debug.value() : false; };
  bool dgetFastBoot() const { return FastBoot.isDefined() ? FastBoot.value() : false; };
  bool dgetNoEarlyProgress() const { return NoEarlyProgress.isDefined() ? NoEarlyProgress.value() : false; };
  bool dgetNeverHibernate() const { return NeverHibernate.isDefined() ? NeverHibernate.value() : false; };
  bool dgetStrictHibernate() const { return StrictHibernate.isDefined() ? StrictHibernate.value() : false; };
  bool dgetRtcHibernateAware() const { return RtcHibernateAware.isDefined() ? RtcHibernateAware.value() : false; };
  bool dgetHibernationFixup() const { return HibernationFixup.isDefined() ? HibernationFixup.value() : false; };
  bool dgetSignatureFixup() const { return SignatureFixup.isDefined() ? SignatureFixup.value() : false; };
  INT8 dgetSecureSetting() const { return isDefined() ? SecureBootSetupMode.isDefined() ? SecureBootSetupMode.value() : -1 : 0; }; // to Rename to SecureBootSetupMode // TODO: different default value if section is not defined
  UINT8 dgetSecureBootPolicy() const {
    if ( !SecureBootPolicy.isDefined() ) return 0;
    if ( SecureBootPolicy.value().length() < 1 ) return 0;
    if ((SecureBootPolicy.value()[0] == 'D') || (SecureBootPolicy.value()[0] == 'd')) {
      // Deny all images
      return SECURE_BOOT_POLICY_DENY;
    } else if ((SecureBootPolicy.value()[0] == 'A') || (SecureBootPolicy.value()[0] == 'a')) {
      // Allow all images
      return SECURE_BOOT_POLICY_ALLOW;
    } else if ((SecureBootPolicy.value()[0] == 'Q') || (SecureBootPolicy.value()[0] == 'q')) {
      // Query user
      return SECURE_BOOT_POLICY_QUERY;
    } else if ((SecureBootPolicy.value()[0] == 'I') || (SecureBootPolicy.value()[0] == 'i')) {
      // Insert
      return SECURE_BOOT_POLICY_INSERT;
    } else if ((SecureBootPolicy.value()[0] == 'W') || (SecureBootPolicy.value()[0] == 'w')) {
      // White list
      return SECURE_BOOT_POLICY_WHITELIST;
    } else if ((SecureBootPolicy.value()[0] == 'B') || (SecureBootPolicy.value()[0] == 'b')) {
      // Black list
      return SECURE_BOOT_POLICY_BLACKLIST;
    } else if ((SecureBootPolicy.value()[0] == 'U') || (SecureBootPolicy.value()[0] == 'u')) {
      // User policy
      return SECURE_BOOT_POLICY_USER;
    }
    return 0;
  }
  
  const decltype(WhiteList)::ValueType& dgetSecureBootWhiteList() const { return WhiteList.isDefined() ? WhiteList.value() : WhiteList.nullValue; };
  const decltype(BlackList)::ValueType& dgetSecureBootBlackList() const { return BlackList.isDefined() ? BlackList.value() : BlackList.nullValue; };

//  int8_t dgetXMPDetection() const {
//    if ( XMPDetection.xmlBool.isDefined() ) return XMPDetection.xmlBool.value();
//    if ( XMPDetection.xmlInt8.isDefined() ) return XMPDetection.xmlInt8.value();
//    return 0;
//  };
  int8_t dgetXMPDetection() const { return XMPDetection.dgetValue(); }
  XString8 dgetLegacyBoot(bool isFirmwareClover) const { return isDefined() ? Legacy.isDefined() ? Legacy.value() : isFirmwareClover ? "PBR"_XS8 : "LegacyBiosDefault"_XS8 : NullXString8; }; // TODO: different default value if section is not defined
  UINT16 dgetLegacyBiosDefaultEntry() const { return LegacyBiosDefaultEntry.isDefined() ? LegacyBiosDefaultEntry.value() : 0; };

  const XString8& dgetCustomLogoAsXString8() const  { return CustomLogo.xmlString8.isDefined() ? CustomLogo.xmlString8.value() : NullXString8; };
  const XBuffer<UINT8>& dgetCustomLogoAsData() const  { return CustomLogo.xmlData.isDefined() ? CustomLogo.xmlData.value() : XBuffer<UINT8>::NullXBuffer; };

/* calculated values */
  bool dgetLastBootedVolume() const { return DefaultVolume.isDefined() ? DefaultVolume.value() == "LastBootedVolume"_XS8 : false; }
  UINT8 dgetCustomLogoType() const {
    if ( CustomLogo.xmlBool.isDefined() ) return CustomLogo.xmlBool.value() ? CUSTOM_BOOT_APPLE : CUSTOM_BOOT_USER_DISABLED;
    if ( CustomLogo.xmlString8.isDefined() ) {
      if ( CustomLogo.xmlString8.value() == "Apple"_XS8 ) return CUSTOM_BOOT_APPLE;
      if ( CustomLogo.xmlString8.value() == "Alternate"_XS8 ) return CUSTOM_BOOT_ALT_APPLE;
      if ( CustomLogo.xmlString8.value() == "Theme"_XS8 ) return CUSTOM_BOOT_THEME;
      return CUSTOM_BOOT_USER;
    }
    if ( CustomLogo.xmlData.isDefined() ) {
      return CUSTOM_BOOT_USER;
    }
    return CUSTOM_BOOT_DISABLED;
  }

/**/
  virtual void reset() override {
    super::reset();
  };
private:


};


#endif /* _CONFIGPLISTCLASS_Boot_H_ */

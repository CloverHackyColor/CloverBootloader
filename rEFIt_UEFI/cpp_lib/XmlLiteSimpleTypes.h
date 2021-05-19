/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#ifndef CPP_LIB_XML_TYPES_H_
#define CPP_LIB_XML_TYPES_H_

#ifndef NOP
#define NOP do { int i=0 ; (void)i; } while (0) // for debugging
#endif

#include "../cpp_foundation/XBuffer.h"
#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XToolsCommon.h"

#include "XmlLiteParser.h"

#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

static inline void WARNING_IF_DEFINED_breakpoint()
{
  (void)0;
}

#define WARNING_IF_DEFINED \
  do { \
    if ( isDefined() ) { \
      WARNING_IF_DEFINED_breakpoint(); \
      xmlLiteParser->addWarning(true, S8Printf("Tag '%s:%d' is already defined. Previous value ignored.", xmlPath.c_str(), xmlLiteParser->getLine())); \
      reset(); \
    } \
  } while (0); \


class XmlAbstractType
{
private:
    bool m_isDefined;

public:
  constexpr XmlAbstractType() : m_isDefined(false) {};
  virtual ~XmlAbstractType() {};
  
  virtual const char* getDescription() = 0;
  virtual void setDefined() {
    if ( m_isDefined ) {
      panic("m_isDefined");
    }
    m_isDefined = true;
  };
  virtual bool isDefined() const { return m_isDefined; };
  virtual void reset() { m_isDefined = false; };
  
  virtual bool isKey() const { return false; }
  virtual const XString8& getKey() const { return NullXString8; }
//  virtual void setKey(const char* keyValue, size_t keyValueLength) {};

  
  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) = 0;
  /*
   * Parse just get the tag and value and translate it to the underlying representation.
   * Return false if there is a parsing problem.
   * If it's a structural XML problem, xmlLiteParser::xmlParsingError is set to true. In that case, parsing must stop.
   * If xmlLiteParser::xmlParsingError is false, the problem is with the content of the tag (for example alpha chars in integer tag).
   * In that case, ignoring the tag and trying to continue is possible.
   * It MUST NOT call validate.
   */
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) = 0;

  /*
   * Validate the value. Intended for subclasser.
   * Return false if value is not accepted.
   */
  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) { (void)xmlLiteParser; (void)xmlPath; (void)keyPos; (void)generateErrors; return true; };

};

class NullXmlType : public XmlAbstractType
{
  virtual const char* getDescription() override { return "null"; };
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override { (void)xmlLiteParser; (void)xmlPath; (void)generateErrors; panic("NullXmlType : can't call parseFromXmlLite"); };
  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { (void)xmlLiteParser; return false; };
};

extern NullXmlType nullXmlType;

class XmlStrictBool : public XmlAbstractType
{
  using super = XmlAbstractType;
public:
  using ValueType = bool;
  static const ValueType nullValue;
protected:
  bool booll = false;
public:
  constexpr XmlStrictBool() : super() {};
  constexpr XmlStrictBool(bool b) : booll(b) { /*if (b) b = 1;*/ }

  virtual void reset() override { super::reset(); booll = 0; };
  virtual const char* getDescription() override { return "boolean"; };

  const bool& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return booll; }

  bool setBoolValue(bool b) { setDefined(); booll = b; return true; }

  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("true") || xmlLiteParser->nextTagIsOpeningTag("false"); };
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlBool : public XmlStrictBool
{
  using super = XmlStrictBool;
public:
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override;
};


class XmlBoolYesNo : public XmlStrictBool
{
  using super = XmlBoolYesNo;
public:
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override;
};



class XmlString8 : public XmlAbstractType
{
  using super = XmlAbstractType;
public:
  using ValueType = XString8;
  static const ValueType nullValue;

protected:
  bool canBeEmpty = false;
  ValueType xstring8 = ValueType();
  
public:
  XmlString8() : super() {};
  XmlString8(bool _canBeEmpty) : super(), canBeEmpty(_canBeEmpty) {};
  virtual ~XmlString8() {};

  virtual const char* getDescription() override { return "string"; };
  virtual void reset() override {
    super::reset(); xstring8.setEmpty();
  };

//  ValueType& value() { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return xstring8; }
  virtual const ValueType& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return xstring8; }

  virtual bool setStringValue(const ValueType& _value) { setDefined(); xstring8 = _value; return true; }
  virtual bool setStringValue(const char* s, size_t size) { setDefined(); xstring8.strsicpy(s, size); return true; }
  virtual bool stealStringValue(char* s, size_t size) { setDefined(); xstring8.stealValueFrom(s, size); return true; }

  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("string"); }
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
  // TODO validate !_canBeEmpty
  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
#ifdef JIEF_DEBUG
if (xmlPath.containsIC("BoardSerialNumber"_XS8) ) {
  NOP;
}
#endif
    if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
    if ( !canBeEmpty && xstring8.isEmpty() ) {
      xmlLiteParser->addWarning(generateErrors, S8Printf("String cannot be empty for tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
      return false;
    }
    return true;
  }

//  operator const XString8& () const { return xstring8; };
};

class XmlStringW : public XmlAbstractType
{
  using super = XmlAbstractType;
public:
  using ValueType = XStringW;
  static const ValueType nullValue;

protected:
  bool canBeEmpty = false;
  XStringW xstringW = ValueType();
  
public:
  XmlStringW() : super() {};
  XmlStringW(bool _canBeEmpty) : super(), canBeEmpty(_canBeEmpty) {};
  virtual ~XmlStringW() {};

  virtual const char* getDescription() override { return "string"; };
  virtual void reset() override { super::reset(); xstringW.setEmpty(); };

  virtual const ValueType& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return xstringW; }
  virtual bool setStringValue(const char* s, size_t size) { setDefined(); xstringW.strsicpy(s, size); return true; }

  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("string"); }
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
  // TODO validate !_canBeEmpty
  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
    if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
    if ( !canBeEmpty && xstringW.isEmpty() ) {
      xmlLiteParser->addWarning(generateErrors, S8Printf("String cannot be empty for tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
      return false;
    }
    return true;
  }

  operator const XStringW& () const { return xstringW; };
};

class XmlString8AllowEmpty : public XmlString8
{
  using super = XmlString8;
public:
  XmlString8AllowEmpty() : super(true) {};
};

class XmlString8Trimed : public XmlString8
{
  using super = XmlString8;
public:
  XmlString8Trimed() : super(false) {};
  XmlString8Trimed(bool allowEmpty) : super(allowEmpty) {};
  virtual bool setStringValue(const char* s, size_t size) override { setDefined(); xstring8.strsicpy(s, size); xstring8.trim(); return true; }
};

class XmlKey : public XmlAbstractType
{
  using super = XmlAbstractType;
public:
  using ValueType = XString8;
  static const ValueType nullValue;
  
  ValueType xstring8 = ValueType();
  
public:
  XmlKey() : super() {};
  ~XmlKey() {};

  virtual const char* getDescription() override { return "key"; };
  virtual void reset() override { super::reset(); xstring8.setEmpty(); };

  virtual bool isKey() const override { return true; }
  virtual const XString8& getKey() const override { return value(); }

  const ValueType& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return xstring8; }

  virtual bool setStringValue(const char* s, size_t size) { setDefined(); xstring8.strsicpy(s, size); return true; }

  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("key"); }
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const char** keyValuePtr, size_t* keyValueLengthPtr);
  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
    if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
    if ( xstring8.isEmpty() ) return false;
    return true;
  }

  operator const XString8& () const { return xstring8; };
};

class XmlKeyDisablable : public XmlKey
{
  using super = XmlKey;
protected:
  bool Disabled = false;
public:
  XmlKeyDisablable() {};
  ~XmlKeyDisablable() {};

  virtual void reset() override { super::reset(); Disabled = false; };

  virtual const char* getDescription() override { return "key(dis)"; };

  bool isDisabled() const { return Disabled; }
  bool isEnabled() const { return !Disabled; }
  const ValueType& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return xstring8; }

  virtual bool setStringValue(const char* s, size_t size) override {
    setDefined();
    if ( size > 0 && s[0] == '!' ) {
      xstring8.strsicpy(s+1, size-1);
      Disabled = true;
    } else {
      xstring8.strsicpy(s, size);
    }
    return true;
  }

//  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
    if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
    if ( xstring8.isEqual("!") ) return false; // it's an empty disabled key
    return true;
  }

  operator const XString8& () const { return xstring8; };
};

/*
* If it's a string tag, the content of the string is put as is in the XBuffer.
* If it's a data tag, data are base64 decoded before being put in XBuffer.
*/
class XmlData : public XmlAbstractType
{
  using super = XmlAbstractType;
public:
  using ValueType = XBuffer<uint8_t>;
  static const ValueType nullValue;
protected:
  ValueType m_value = ValueType();
public:

  XmlData() : super() {};

  virtual const char* getDescription() override { return "data"; };
  virtual void reset() override { super::reset(); m_value.setEmpty(); };

  ValueType& modifiableValue() { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return m_value; }
  const ValueType& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return m_value; }

  bool setDataValue(const uint8_t* s, size_t size) { setDefined(); m_value.ncpy(s, size); return true; }
  bool stealDataValue(uint8_t* s, size_t size) { setDefined(); m_value.stealValueFrom(s, size); return true; }
  bool stealDataValue(uint8_t* s, size_t size, size_t allocatedSize) { setDefined(); m_value.stealValueFrom(s, size, allocatedSize); return true; }

  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("string") || xmlLiteParser->nextTagIsOpeningTag("data"); }
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlIntegerAbstract : public XmlAbstractType
{
public:
  bool isTheNextTag(XmlLiteParser* xmlLiteParser) override;
  bool parseXmlInteger(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, uint64_t* result, bool* negative, int64_t minimum, uint64_t maximum, bool generateErrors);
};

template<typename IntegralType/*, enable_if(is_integral(IntegralType))*/>
class XmlInteger : public XmlIntegerAbstract
{
    using super = XmlIntegerAbstract;
  public:
    using ValueType = IntegralType;
    static const ValueType nullValue;
  protected:
    ValueType m_value = 0;

  public:
  // these method are public for unit tests.
  // Do NOT instanciate XmlInteger
  // Do NOT call these methods directly.
    XmlInteger() : super() {};
    virtual ~XmlInteger() {};

    virtual void reset() override { super::reset(); m_value = 0; };
    
    const ValueType& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return m_value; }

//    bool parseXmlInteger(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, UINTN* result, bool* negative, INTN minimum, UINTN maximum, bool generateErrors);
};
template <class T>
const typename XmlInteger<T>::ValueType XmlInteger<T>::nullValue = typename XmlInteger<T>::ValueType();

class XmlUInt8 : public XmlInteger<uint8_t>
{
  using super = XmlInteger<uint8_t>;
public:
  XmlUInt8() : super() {};

  virtual const char* getDescription() override { return "uint8"; };

  bool setUInt8Value(ValueType ui8) { setDefined(); m_value = ui8; return true; }
  bool setUInt8Value(ValueType value, bool sign) {
    if ( sign ) { log_technical_bug("XmlUInt8 sign=true"); setUInt8Value(0); return false; }
    setUInt8Value(value);
    return true;
  }
//  bool setUInt8Value(uint8_t value, bool sign, XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& pos, bool generateErrors) {
//    setUInt8Value(value, sign);
//    return validate(xmlLiteParser, xmlPath, pos, generateErrors);
//  }

  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlInt8 : public XmlInteger<int8_t>
{
  using super = XmlInteger<int8_t>;
public:
  XmlInt8() : super() {};

  virtual const char* getDescription() override { return "int8"; };

  bool setInt8Value(ValueType i8) { setDefined(); m_value = i8; return true; }
  bool setInt8Value(ValueType value, bool sign) { if ( sign ) return setInt8Value(-value); else setInt8Value(value); return true; }

  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlUInt16 : public XmlInteger<uint16_t>
{
  using super = XmlInteger<uint16_t>;
public:
  XmlUInt16() : super() {};
  
  virtual const char* getDescription() override { return "uint16"; };

  bool setUInt16Value(ValueType ui16) { setDefined(); m_value = ui16; return true; }
  bool setUInt16Value(ValueType value, bool sign) {
    if ( sign ) { log_technical_bug("XmlUInt16 sign=true"); setUInt16Value(0); return false; }
    setUInt16Value(value);
    return true;
  }

  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlInt16 : public XmlInteger<int16_t>
{
  using super = XmlInteger<int16_t>;
public:
  XmlInt16() : super() {};

  virtual const char* getDescription() override { return "unt16"; };

  bool setInt16Value(ValueType ui16) { setDefined(); m_value = ui16; return true; }
  bool setInt16Value(ValueType value, bool sign) { if ( sign ) return setInt16Value(-value); else setInt16Value(value); return true; }

  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlUInt32 : public XmlInteger<uint32_t>
{
  using super = XmlInteger<uint32_t>;
public:
  XmlUInt32() : super() {};

  virtual const char* getDescription() override { return "uint32"; };

  bool setUInt32Value(ValueType ui32) { setDefined(); m_value = ui32; return true; }
  bool setUInt32Value(ValueType value, bool sign) {
    if ( sign ) { log_technical_bug("XmlUInt32 sign=true"); setUInt32Value(0); return false; }
    setUInt32Value(value);
    return true;
  }

  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlInt32 : public XmlInteger<int32_t>
{
  using super = XmlInteger<int32_t>;
public:
  XmlInt32() : super() {};

  virtual const char* getDescription() override { return "int32"; };

  bool setInt32Value(ValueType i32) { setDefined(); m_value = i32; return true; }
  bool setInt32Value(ValueType value, bool sign) { if ( sign ) return setInt32Value(-value); else setInt32Value(value); return true; }

  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlUInt64 : public XmlInteger<uint64_t>
{
  using super = XmlInteger<uint64_t>;
public:
  XmlUInt64() : super() {};

  virtual const char* getDescription() override { return "uint64"; };

  bool setUInt64Value(ValueType ui64) { setDefined(); m_value = ui64; return true; }
  bool setUInt64Value(ValueType value, bool sign) {
    if ( sign ) { log_technical_bug("XmlUInt64 sign=true"); setUInt64Value(0); return false; }
    setUInt64Value(value);
    return true;
  }

  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};

class XmlInt64 : public XmlInteger<int64_t>
{
  using super = XmlInteger<int64_t>;
public:
  XmlInt64() : super() {};

  virtual const char* getDescription() override { return "int64"; };

  bool setInt64Value(ValueType i64) { setDefined(); m_value = i64; return true; }
  bool setInt64Value(ValueType value, bool sign) { if ( sign ) return setInt64Value(-value); else setInt64Value(value); return true; }

  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
};


#endif /* CPP_LIB_XML_TYPES_H_ */

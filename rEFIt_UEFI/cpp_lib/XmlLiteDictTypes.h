/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#include "../cpp_foundation/XToolsCommon.h"

#ifndef XmlLiteDictTypes_h
#define XmlLiteDictTypes_h

#include <stdio.h>
#include "XmlLiteSimpleTypes.h"
#include "XmlLiteDictTypes.h"

#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

class XmlDictField
{
public:
  const char* m_name;
  XmlAbstractType& xmlAbstractType;
  XmlDictField(const char* name, XmlAbstractType& XmlAbstractType) : m_name(name), xmlAbstractType(XmlAbstractType) {};
};

class XmlDict : public XmlAbstractType
{
  using super = XmlAbstractType;
public:
  XmlDict() : super() {};
  ~XmlDict() {};

  virtual const char* getDescription() override { return "dict"; };
  virtual void reset() override;

  virtual void getFields(XmlDictField** fields, size_t* nb) { *fields = NULL; *nb = 0; };

  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("dict"); }

  virtual XmlAbstractType& parseValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const XmlParserPosition &keyPos, const char *keyValue, size_t keyValueLength, bool* keyFound);
  virtual XmlAbstractType& parseKeyAndValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const char** keyValuePtr, size_t* keyValueLengthPtr);
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;

  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override;
};




template<class XmlKeyClass, class XmlValueClass, typename T = void>
class XmlAddKey : public XmlValueClass
{
    using super = XmlValueClass;
  private:
    XmlKeyClass m_key = XmlKeyClass();
//    using ValueType = MyXmlAddKey<XmlClass>;

  public:
    using keyType = XmlKeyClass;
    
//    virtual void setKey(const char* keyValue, size_t keyValueLength) {
//      m_key.strncpy(keyValue, keyValueLength);
//    }
//          XmlKeyClass& key()       { return m_key; }
    const XmlKeyClass& xmlKey() const { return m_key; }
    decltype(m_key.value()) key() const { return m_key.value(); }

    void setKey(const XmlKeyClass& aKey) {
//      if ( aKey.isEmpty() ) {
//        log_technical_bug("%s : aKey.length() == 0", __PRETTY_FUNCTION__);
//      }
      m_key = aKey;
    }
//    const XmlClass& value() const { return value(); }
};







template<class XmlKeyType, class XmlValueType, class ValueType>
class _XmlRepeatingDict : public XmlAbstractType
{
  using super = XmlAbstractType;
protected:
  using ValueArrayType = XObjArray<ValueType>;
//  XString8Array keyArray;
  XmlKeyType keyTmp = XmlKeyType();
  ValueArrayType m_valueArray = ValueArrayType();
public:
  bool ignoreCommented = true;
  
  _XmlRepeatingDict() : super() {};
  ~_XmlRepeatingDict() {};

  virtual const char* getDescription() override { return "dict"; };
  virtual void reset() override { super::reset(); m_valueArray.setEmpty(); };

  virtual XmlValueType* getNewInstance() { return new XmlValueType; }
    
  const ValueArrayType& valueArray() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return m_valueArray; }

  virtual void addValue(XmlValueType* xmlValueType) = 0;

  virtual void getFields(XmlDictField** fields, size_t* nb) { panic("BUG : repeating dict don't use getFields()"); };

  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("dict"); }

//  virtual XmlAbstractType& parseValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const XmlParserPosition &keyPos, const char *keyValue, size_t keyValueLength, bool* keyFound);
  virtual XmlValueType* parseKeyAndValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors);
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;

//  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override;
};


template<class XmlKeyType, class XmlValueType, class ValueType>
XmlValueType* _XmlRepeatingDict<XmlKeyType, XmlValueType, ValueType>::parseKeyAndValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  keyTmp.reset();
  if ( keyTmp.parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors) ) {
    XmlValueType* newValue = getNewInstance();
    if ( newValue->parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors) ) {
      return newValue;
    }
  }
  return NULL;
//  const char* tag;
//  size_t tagLength;
//
//  XmlParserPosition pos = xmlLiteParser->getPosition();
//  if ( !xmlLiteParser->getSimpleTag(&tag, &tagLength, keyValuePtr, keyValueLengthPtr, NULL, generateErrors) ) {
//    return NULL;
//  }
//  if ( !strnIsEqual(tag, tagLength, "key") ) {
//    xmlLiteParser->addXmlError(generateErrors, S8Printf("Expecting a <key> tag in '%s' at line %d.", xmlPath.c_str(), pos.getLine()));
//    return NULL;
//  }
//  XmlValueType* kv = getNewInstance();
//  if ( kv->parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors) ) {
//    return kv;
//  }
//  delete kv;
//  return NULL;
}

template<class XmlKeyType, class XmlValueType, class ValueType>
bool _XmlRepeatingDict<XmlKeyType, XmlValueType, ValueType>::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;

  RETURN_IF_FALSE ( xmlLiteParser->consumeOpeningTag("dict", generateErrors) );
  setDefined();

#ifdef JIEF_DEBUG
if ( xmlPath.startWithOrEqualToIC("/Devices/Properties"_XS8) ) {
  int i=0; (void)i;
}
#endif

//  const char* keyValue;
//  size_t keyValueLength;
//  XString8 xmlSubPath;

  size_t n = 0;

  while ( !xmlLiteParser->nextTagIsClosingTag("dict") )
  {
//    RETURN_IF_FALSE( xmlLiteParser->getKeyTagValue(&keyValue, &keyValueLength, &keyPos, true) );
//    xmlSubPath = xmlPath;
//    xmlSubPath.S8Catf("/%.*s", (int)keyValueLength, keyValue);
//
#ifdef DEBUG
XmlParserPosition valuePos = xmlLiteParser->getPosition();
(void)valuePos;
#endif
//    bool keyFound;
//    XmlAbstractType& xmlAbstractType = parseValueFromXmlLite(xmlLiteParser, xmlSubPath, generateErrors, keyPos, keyValue, keyValueLength, &keyFound);

    XmlParserPosition keyPos = xmlLiteParser->getPosition();

    XString8 xmlSubPath = xmlPath;
    xmlSubPath.S8Catf("[%zu]", n);

    XmlValueType* xmlValueType = parseKeyAndValueFromXmlLite(xmlLiteParser, xmlSubPath, generateErrors); // key goes in keyTmp

    if ( xmlLiteParser->xmlParsingError ) {
      return false;
    }
    if ( keyTmp.value().length() > 0 )
    {
      if ( !( keyTmp.value()[0] == '#' && ignoreCommented ) )
      {
        if ( xmlValueType != NULL )
        {
          if ( !xmlValueType->isDefined() ) panic("BUG: parseKeyAndValueFromXmlLite must not return an undefined item");
          bool validated = xmlValueType->validate(xmlLiteParser, S8Printf("%s/%s", xmlPath.c_str(), keyTmp.value().c_str()), keyPos, generateErrors);
          if ( validated ) {
            xmlValueType->setKey(keyTmp);
            addValue(xmlValueType);
          }
        }else{
          // wrong value. Let's try to ignore and continue
        }
      }else{
        // Commented out.
      }
    }else{
      xmlLiteParser->addWarning(generateErrors, S8Printf("Empty key, following value is ignored at line %d", keyPos.getLine()));
      // if !xmlAbstractType.isDefined(), it's because a unknown key, or a comment.
      // Just try to continue.
    }
    ++n;
  }
  RETURN_IF_FALSE ( xmlLiteParser->consumeClosingTag("dict", generateErrors) );
  return true;
}



//
//template <class _Tp> struct _XmlRepeatingDict_has_ValueType  { typedef void Typp; };
//
//template<class XmlValueType, typename T = void>
//class XmlRepeatingDict : public _XmlRepeatingDict<XmlValueType, XmlValueType>
//{
//  using super = _XmlRepeatingDict<XmlValueType, XmlValueType>;
//
//  virtual void addValue(XmlValueType* xmlValueType) {
//    super::arrayValue().AddReference(xmlValueType, true);
//  }
//};
//
//template<class XmlValueType>
//class XmlRepeatingDict<XmlValueType, typename _XmlRepeatingDict_has_ValueType<typename XmlValueType::ValueType>::Typp> : public _XmlRepeatingDict<XmlValueType, typename XmlValueType::ValueType>
//{
//  using super = _XmlRepeatingDict<XmlValueType, typename XmlValueType::ValueType>;
//
//  virtual void addValue(XmlValueType* xmlValueType) {
//    super::arrayValue().AddCopy(xmlValueType->value());
//    delete xmlValueType; // TODO improve to avoid memory allocation. At least use stealValueFrom
//  }
//};
//
//
//
//








template <class _Tp> _Tp&& __declval(int);
template <class _Tp> _Tp   __declval(long);

template <class _Tp>
decltype(__declval<_Tp>(0))
declval() ;


template<typename T>
struct HasUsedMemoryMethod
{
    template<typename U, size_t (U::*)() const> struct SFINAE {};
    template<typename U> static char Test(SFINAE<U, &U::setKey>*);
    template<typename U> static int Test(...);
    static const bool Has = sizeof(Test<T>(0)) == sizeof(char);
};


template <class _Tp> struct __xmldict__declare_void  { typedef void Typp; };


template<class XmlAddKeyType>
class XmlRepeatingDict : public _XmlRepeatingDict<typename XmlAddKeyType::keyType, XmlAddKeyType, XmlAddKeyType>
{
  using super = _XmlRepeatingDict<typename XmlAddKeyType::keyType, XmlAddKeyType, XmlAddKeyType>;
  virtual void addValue(XmlAddKeyType* xmlValueType) {
    super::m_valueArray.AddReference(xmlValueType, true);
  }
};


//
//template<class XmkKeyType, class XmlValueType, typename T = void, typename U = void>
//class XmlRepeatingDict : public _XmlRepeatingDict<XmkKeyType, XmlValueType, XmlValueType>
//{
////  using super = _XmlRepeatingDict<XmlValueType, XmlValueType>;
////public:
////  virtual void addValue(XmlValueType* xmlValueType, const char* keyValue, size_t keyLength) {
////    super::valueArray.AddReference(xmlValueType, true);
////  }
//};

template<class XmkKeyType, class XmlValueType>
class XmlRepeatingDict<XmlAddKey<XmkKeyType, XmlValueType>> : public _XmlRepeatingDict<XmkKeyType, XmlAddKey<XmkKeyType, XmlValueType>, XmlAddKey<XmkKeyType, XmlValueType>>
{
  using super = _XmlRepeatingDict<XmkKeyType, XmlAddKey<XmkKeyType, XmlValueType>, XmlAddKey<XmkKeyType, XmlValueType>>;
//public:
  virtual void addValue(XmlAddKey<XmkKeyType, XmlValueType>* xmlValueType) {
    super::m_valueArray.AddReference(xmlValueType, true);
  }
};



//
//template<class XmlValueType>
//class XmlRepeatingDict<XmlValueType,
//                       typename __xmldict__declare_void<typename XmlValueType::ValueType>::Typp
//                      >
//      : public _XmlRepeatingDict<XmlValueType, typename XmlValueType::ValueType>
//{
//  using super = _XmlRepeatingDict<XmlValueType, typename XmlValueType::ValueType>;
////  decltype(declval<XmlValueType>().setKey()) a;
//
//  virtual void addValue(XmlValueType* xmlValueType, const char* keyValue, size_t keyLength) {
//        super::valueArray.AddCopy(xmlValueType->value(), true);
//        delete xmlValueType;
//  }
//};
//




//
//template<class XmlClass>
//class XmlAddKey<XmlClass, typename __xmldict__declare_void<typename XmlClass::ValueType>::Typp> : public XmlClass
//{
//    using super = XmlClass;
//  private:
//    XString8 m_key;
////    using ValueType = MyXmlAddKey<XmlClass>;
//
//  public:
//    virtual void setKey(const char* keyValue, size_t keyValueLength) {
//      m_key.strncpy(keyValue, keyValueLength);
//    }
//    const XString8& key() const { return m_key; }
//    const typename XmlClass::ValueType& value() const { return super::value(); }
//
////    template<class OtherXStringArrayClass>
////    bool operator ==(const OtherXStringArrayClass &aXStrings) const { return super::operator ==(aXStrings); }
////  template<class OtherXStringArrayClass>
////  bool operator !=(const OtherXStringArrayClass &aXStrings) const { return super::operator !=(aXStrings); }
//
//};
//
//
//
//
//
//template<class XmlValueType, typename T = void, typename U = void>
//class XmlRepeatingDict : public _XmlRepeatingDict<XmlValueType, XmlValueType>
//{
//  using super = _XmlRepeatingDict<XmlValueType, XmlValueType>;
//public:
//  virtual void addValue(XmlValueType* xmlValueType, const char* keyValue, size_t keyLength) {
//    super::arrayValue().AddReference(xmlValueType, true);
//  }
//};
//
//template<class XmlValueType>
//class XmlRepeatingDict<XmlValueType,
//                       typename __xmldict__declare_void<typename XmlValueType::ValueType>::Typp
//                      >
//      : public _XmlRepeatingDict<XmlAddKey<XmlValueType>, XmlAddKey<XmlValueType>>
//{
//  using super = _XmlRepeatingDict<XmlAddKey<XmlValueType>, XmlAddKey<XmlValueType>>;
////  decltype(declval<XmlValueType>().setKey()) a;
//
//  virtual void addValue(XmlAddKey<XmlValueType>* xmlValueType, const char* keyValue, size_t keyLength) {
////    super::arrayValue().AddCopy(xmlValueType->value());
////    delete xmlValueType; // TODO improve to avoid memory allocation. At least use stealValueFrom
//        xmlValueType->setKey(keyValue, keyLength);
//        super::valueArray.AddReference(xmlValueType, true);
//  }
//};
//
////
////template<class XmlValueType>
////class XmlRepeatingDict<XmlAddKey<XmlValueType>>
////      : public _XmlRepeatingDict<XmlAddKey<XmlValueType>, XmlAddKey<XmlValueType>>
////{
////  using super = _XmlRepeatingDict<XmlAddKey<XmlValueType>, XmlAddKey<XmlValueType>>;
////public:
////
//////  decltype(declval<XmlValueType>().setKey()) a;
////  XString8Array keyArray = XString8Array();
////
////  virtual void addValue(XmlAddKey<XmlValueType>* xmlValueType, const char* keyValue, size_t keyLength) {
////    xmlValueType->setKey(keyValue, keyLength);
////    super::valueArray.AddReference(xmlValueType, true);
////  }
////};
////
////
//
//
//
////
////
////template<class XmlValueType>
////class XmlRepeatingDict<XmlValueType,
////                       typename __xmldict__declare_void<typename XmlValueType::ValueType>::Typp,
////                       typename __xmldict__declare_void<decltype(XmlValueType().setKey(declval<const char*>(), declval<size_t>()))>::Typp
////                      >
////      : public _XmlRepeatingDict<XmlValueType, typename XmlValueType::ValueType>
////{
////  using super = _XmlRepeatingDict<XmlValueType, typename XmlValueType::ValueType>;
////public:
////
//////  decltype(declval<XmlValueType>().setKey()) a;
////  XString8Array keyArray = XString8Array();
////
////  virtual void addValue(XmlValueType* xmlValueType, const char* keyValue, size_t keyLength) {
//////    super::arrayValue().AddCopy(xmlValueType->value());
//////    keyArray.Add();
////    delete xmlValueType; // TODO improve to avoid memory allocation. At least use stealValueFrom
////  }
////};
////
//


#endif /* XmlLiteCompositeTypes_h */

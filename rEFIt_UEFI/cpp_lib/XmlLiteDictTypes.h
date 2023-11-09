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

  virtual XBool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("dict"); }

  virtual XmlAbstractType& parseValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, XBool generateErrors, const XmlParserPosition &keyPos, const char *keyValue, size_t keyValueLength, XBool* keyFound);
  virtual XmlAbstractType& parseKeyAndValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, XBool generateErrors, const char** keyValuePtr, size_t* keyValueLengthPtr);
  virtual XBool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, XBool generateErrors) override;

  virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override;
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
  XBool ignoreCommented = true;
  
  _XmlRepeatingDict() : super() {};
  ~_XmlRepeatingDict() {};

  virtual const char* getDescription() override { return "dict"; };
  virtual void reset() override { super::reset(); m_valueArray.setEmpty(); };

  virtual XmlValueType* getNewInstance() { return new XmlValueType; }
    
  const ValueArrayType& valueArray() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return m_valueArray; }

  virtual void addValue(XmlValueType* xmlValueType) = 0;

  virtual void getFields(XmlDictField** fields, size_t* nb) { panic("BUG : repeating dict don't use getFields()"); };

  virtual XBool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("dict"); }

  virtual XmlValueType* parseKeyAndValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, XBool generateErrors);
  virtual XBool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, XBool generateErrors) override;

};


template<class XmlKeyType, class XmlValueType, class ValueType>
XmlValueType* _XmlRepeatingDict<XmlKeyType, XmlValueType, ValueType>::parseKeyAndValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, XBool generateErrors)
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
XBool _XmlRepeatingDict<XmlKeyType, XmlValueType, ValueType>::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, XBool generateErrors)
{
  WARNING_IF_DEFINED;

  RETURN_IF_FALSE ( xmlLiteParser->consumeOpeningTag("dict", generateErrors) );
  setDefined();

#ifdef JIEF_DEBUG
if ( xmlPath.startWithOrEqualToIC("/Devices/Properties"_XS8) ) {
  int i=0; (void)i;
}
#endif

  size_t n = 0;

  while ( !xmlLiteParser->nextTagIsClosingTag("dict") )
  {

#ifdef JIEF_DEBUG
XmlParserPosition valuePos = xmlLiteParser->getPosition();
(void)valuePos;
#endif
    XmlParserPosition keyPos = xmlLiteParser->getPosition();

    XString8 xmlSubPath = xmlPath;
    xmlSubPath.S8Catf("[%zu]", n);

    XmlValueType* xmlValueType = parseKeyAndValueFromXmlLite(xmlLiteParser, xmlSubPath, generateErrors); // key goes in keyTmp

    if ( xmlLiteParser->xmlParsingError ) {
      return false;
    }
    size_t keyLen = keyTmp.value().length();
    if ( keyLen > 0 )
    {
      if ( !( keyTmp.value()[0] == '#' && ignoreCommented ) && !(keyTmp.value()[keyLen-1] == '?') )
      {
        if ( xmlValueType != NULL )
        {
         // if ( !xmlValueType->isDefined() ) panic("BUG: parseKeyAndValueFromXmlLite must not return an undefined item");
          XBool validated = xmlValueType->validate(xmlLiteParser, S8Printf("%s/%s", xmlPath.c_str(), keyTmp.value().c_str()), keyPos, generateErrors);
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


template<class XmkKeyType, class XmlValueType>
class XmlRepeatingDict<XmlAddKey<XmkKeyType, XmlValueType>> : public _XmlRepeatingDict<XmkKeyType, XmlAddKey<XmkKeyType, XmlValueType>, XmlAddKey<XmkKeyType, XmlValueType>>
{
  using super = _XmlRepeatingDict<XmkKeyType, XmlAddKey<XmkKeyType, XmlValueType>, XmlAddKey<XmkKeyType, XmlValueType>>;
//public:
  virtual void addValue(XmlAddKey<XmkKeyType, XmlValueType>* xmlValueType) {
    super::m_valueArray.AddReference(xmlValueType, true);
  }
};


#endif /* XmlLiteCompositeTypes_h */

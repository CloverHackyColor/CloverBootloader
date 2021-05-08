/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#ifndef XmlLiteArrayTypes_h
#define XmlLiteArrayTypes_h

#include <stdio.h>
#include "XmlLiteSimpleTypes.h"


#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifndef NOP
#define NOP do { int i=0 ; (void)i; } while (0) // for debugging
#endif


template <class T>
class XmlArray : public XmlAbstractType, public XObjArray<T>
{
    using super1 = XmlAbstractType;
    using super2 = XObjArray<T>;

  public:
    using ValueType = XObjArray<T>;
    static const ValueType nullValue;
//    XObjArray<T> array = XObjArray<T>();

  public:
    XmlArray<T>() {}
    virtual ~XmlArray<T>() {}

    virtual const char* getDescription() override { return "array"; };
    virtual void reset() override { super1::reset(); super2::setEmpty(); };

    const ValueType& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return *this; }
//    const ValueType& v() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return *this; }

    virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override { return xmlLiteParser->nextTagIsOpeningTag("array"); }
    virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
    // Validate remove invalid elements, but doesn't invalidate the whole array. So this is always returning true.
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override;
};


template <class T>
const typename XmlArray<T>::ValueType XmlArray<T>::nullValue;


template <class T>
bool XmlArray<T>::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  WARNING_IF_DEFINED;
  RETURN_IF_FALSE ( xmlLiteParser->consumeOpeningTag("array") );

  setDefined();

  size_t n = 0;
  XString8 xmlSubPath;
//  XmlParserPosition pos;

  while ( !xmlLiteParser->nextTagIsClosingTag("array") )
  {
    xmlSubPath = xmlPath;
    xmlSubPath.S8Catf("[%zu]", n);

    XmlParserPosition beforePos = xmlLiteParser->getPosition();
    T* newT = new T;
    if ( newT->parseFromXmlLite(xmlLiteParser, xmlSubPath, generateErrors) ) {
      super2::AddReference(newT, true);
    }else{
      delete newT;
      if ( xmlLiteParser->getPosition() == beforePos ) {
        // position is still the same. So we'll endlessly loop if we do not move forward
        if ( *xmlLiteParser->getPosition().p == '<' ) xmlLiteParser->skipNextTag(false);
        else xmlLiteParser->moveForwardUntil('<');
        if ( xmlLiteParser->getPosition() == beforePos ) {
          // still not moved ???
          return false;
        }
      }
    }
    n += 1;
  }
  xmlLiteParser->consumeClosingTag("array", generateErrors);
//  return validate(xmlLiteParser, xmlPath, pos, generateErrors);
  return true;
}

template <class T>
bool XmlArray<T>::validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& pos, bool generateErrors)
{
  for ( size_t idx = 0 ; idx < super2::size() ; ) {
    if ( !super2::ElementAt(idx).validate(xmlLiteParser, xmlPath, pos, generateErrors) ) {
      super2::RemoveAtIndex(idx);
    }else{
      ++idx;
    }
  }
  return true;
}


class XmlString8Array : public XmlArray<XmlString8>
{
    using super = XmlArray<XmlString8>;

  public:
    using ValueType = ConstXString8Array;
    static const ValueType nullValue;
    ValueType array = ValueType();

    const ValueType& value() const { if ( !isDefined() ) panic("%s : value is not defined", __PRETTY_FUNCTION__); return array; }

    virtual void reset() override { super::reset(); array.setEmpty(); };
    virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override
    {
      bool b = super::parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors);
      for ( size_t idx = 0 ; idx < super::size() ; idx++ ) {
        array.AddReference(&super::ElementAt(idx).value(), false);
      }
      return b;
    }
    // Validate can remove invalid element. So we have to regenerate 'array'. There is no object copy, so it's very quick
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override
    {
      bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
      array.setEmpty();
      for ( size_t idx = 0 ; idx < super::size() ; idx++ ) {
        array.AddReference(&super::ElementAt(idx).value(), false);
      }
      return b;
    }

};

//class XmlString8AllowEmptyArray : public XmlArray<XmlString8AllowEmpty>
//{
//};

class XmlStringWArray : public XmlArray<XmlStringW>
{
    using super = XmlArray<XmlStringW>;
  public:
    using ValueType = ConstXStringWArray;
    static const ValueType nullValue;
    ValueType array = ValueType();

          ValueType& value()       { if ( !isDefined() ) log_technical_bug("%s : value is not defined", __PRETTY_FUNCTION__); return array; }
    const ValueType& value() const { if ( !isDefined() ) log_technical_bug("%s : value is not defined", __PRETTY_FUNCTION__); return array; }

    virtual void reset() override { super::reset(); array.setEmpty(); };
    virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override
    {
#ifdef JIEF_DEBUG
if (xmlPath == "/DisableDrivers"_XS8) {
NOP;
}
#endif
      bool b = super::parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors);
      array.setEmpty();
      for ( size_t idx = 0 ; idx < super::size() ; idx++ ) {
        array.AddReference(&super::ElementAt(idx).value(), false);
      }
      return b;
    }
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override
    {
      bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
      // Validate can remove invalid element. So we have to regenerate 'array'. There is no object copy, so it's very quick
      array.setEmpty();
      for ( size_t idx = 0 ; idx < super::size() ; idx++ ) {
        array.AddReference(&super::ElementAt(idx).value(), false);
      }
      return b;
    }

};


#endif /* XmlLiteArrayTypes_h */

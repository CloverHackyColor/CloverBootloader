/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#include "../cpp_foundation/XToolsCommon.h"

#ifndef XmlLiteCompositeTypes_h
#define XmlLiteCompositeTypes_h

#include <stdio.h>
#include "XmlLiteSimpleTypes.h"
#include "XmlLiteCompositeTypes.h"



/*-----------------------------------------------  Composition  */

//
///*
// * "Composition" here means : all the fields, in order.
// */
//class XmlCompositeField
//{
//public:
//  XmlAbstractType& xmlAbstractType;
//  XmlCompositeField(XmlAbstractType& XmlAbstractType) : xmlAbstractType(XmlAbstractType) {};
//};
//
//class XmlComposite : public XmlAbstractType
//{
//  using super = XmlAbstractType;
//protected:
//  XString8 description = XString8();
//public:
//  XmlComposite() : super() {};
//  ~XmlComposite() {};
//
//  virtual const char* getDescription() override {
//    XmlCompositeField* fields;
//    size_t nb;
//    getFields(&fields, &nb);
//    if ( nb == 0 ) panic("nb==0");
//    for ( size_t idx = 0 ; idx < nb ; idx++ ) {
//      if ( idx > 0 ) description += '+';
//      XmlCompositeField& xmlCompositeField = fields[idx];
//      XmlAbstractType& xmlAbstractType = xmlCompositeField.xmlAbstractType;
//      description += xmlAbstractType.getDescription();
//    }
//    return description.c_str();
//  };
//  virtual void getFields(XmlCompositeField** fields, size_t* nb) = 0;
//
//  virtual void reset() override;
//
//  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override;
//  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
//  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override;
//};
//
//
//
///*-----------------------------------------------  Some predefined composition  */
//
//template<class ValueClass>
//class XmlKeyAndValue : public XmlComposite
//{
//  using super = XmlComposite;
//  private:
//  public:
//    XmlKey xmlKey = XmlKey();
//    ValueClass xmlValue = ValueClass();
//  public:
//    XmlCompositeField m_fields[2] = { xmlKey, xmlValue };
//    virtual void getFields(XmlCompositeField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
//
//    // Super class parseFromXmlLite would be fine, execpt for the calculation of xmlPath
//    virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override
//    {
//      if ( !xmlKey.parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors) ) return false;
//      if ( !xmlValue.parseFromXmlLite(xmlLiteParser, S8Printf("%s/%s", xmlPath.c_str(), xmlKey.xstring8.c_str()), generateErrors) ) return false;
//      return true;
//    }
//};
//




#endif /* XmlLiteCompositeTypes_h */

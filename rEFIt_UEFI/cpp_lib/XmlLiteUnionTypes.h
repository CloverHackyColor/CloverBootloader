/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#include "../cpp_foundation/XToolsCommon.h"

#ifndef XmlLiteUnionTypes_h
#define XmlLiteUnionTypes_h

#include <stdio.h>
#include "XmlLiteSimpleTypes.h"
#include "XmlLiteUnionTypes.h"

/*
 * "Union" here means, a bit like in C : it's one and only one of the defined fields.
 */
class XmlUnionField
{
public:
  XmlAbstractType& xmlAbstractType;
  XmlUnionField(XmlAbstractType& XmlAbstractType) : xmlAbstractType(XmlAbstractType) {};
};


#define XmlUnionSuper XmlAbstractType
class XmlUnion : public XmlUnionSuper
{
protected:
  XString8 description = XString8();
public:
  XmlUnion() : XmlUnionSuper() {};
  ~XmlUnion() {};

  virtual const char* getDescription() override {
    XmlUnionField* fields;
    size_t nb;
    getFields(&fields, &nb);
    if ( nb == 0 ) panic("nb==0");
    for ( size_t idx = 0 ; idx < nb ; idx++ ) {
      if ( idx > 0 ) description += '|';
      XmlUnionField& xmlUnionField = fields[idx];
      XmlAbstractType& xmlAbstractType = xmlUnionField.xmlAbstractType;
      description += xmlAbstractType.getDescription();
    }
    return description.c_str();
  };
  virtual void getFields(XmlUnionField** fields, size_t* nb) = 0;

  virtual void reset() override;

  virtual bool isTheNextTag(XmlLiteParser* xmlLiteParser) override;
  virtual bool parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors) override;
  virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override;
};




/*-----------------------------------------------  Some predefined unions  */

class XmlBoolOrString : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlBool xmlBool = XmlBool();
  XmlString8 xmlString8 = XmlString8();
  virtual const char* getDescription() override { return "bool or string"; };
  XmlUnionField m_fields[2] = { xmlBool, xmlString8 };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};


class XmlInt8OrBool : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlInt8 xmlInt8 = XmlInt8();
  XmlBool xmlBool = XmlBool();
  virtual const char* getDescription() override { return "int8 or bool"; };
  XmlUnionField m_fields[2] = { xmlInt8, xmlBool };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};

class XmlInt64OrBool : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlInt64 xmlInt64 = XmlInt64();
  XmlBool xmlBool = XmlBool();
  virtual const char* getDescription() override { return "int64 or bool"; };
  XmlUnionField m_fields[2] = { xmlInt64, xmlBool };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};


class XmlBoolOrStringOrData : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlBool xmlBool = XmlBool();
  XmlString8 xmlString8 = XmlString8();
  XmlData xmlData = XmlData();
  
  virtual const char* getDescription() override { return "bool|string|data"; };
  XmlUnionField m_fields[3] = { xmlBool, xmlString8, xmlData };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};


class XmlInt32OrString : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlInt32 xmlInt32 = XmlInt32();
  XmlString8 xmlString8 = XmlString8();
  virtual const char* getDescription() override { return "int32|string"; };
  XmlUnionField m_fields[2] = { xmlInt32, xmlString8 };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};


class XmlInt16OrString : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlInt16 xmlInt16 = XmlInt16();
  XmlString8 xmlString8 = XmlString8();
  virtual const char* getDescription() override { return "int16|string"; };
  XmlUnionField m_fields[2] = { xmlInt16, xmlString8 };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};



class XmlInt32OrBoolOrData : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlInt32 xmlInt32 = XmlInt32();
  XmlBool  xmlBool = XmlBool();
  XmlData  xmlData = XmlData();
  virtual const char* getDescription() override { return "int32|string|data"; };
  XmlUnionField m_fields[3] = { xmlInt32, xmlBool, xmlData };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};


class XmlInt32OrData : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlInt32 xmlInt32 = XmlInt32();
  XmlData xmlData = XmlData();
  virtual const char* getDescription() override { return "int32|data"; };
  XmlUnionField m_fields[2] = { xmlInt32, xmlData };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};



class XmlInt64OrString : public XmlUnion
{
  using super = XmlUnion;
public:
  XmlInt64 xmlInt64 = XmlInt64();
  XmlString8 xmlString8 = XmlString8();
  virtual const char* getDescription() override { return "int64|string"; };
  XmlUnionField m_fields[2] = { xmlInt64, xmlString8 };
  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};

//
//#define XmlInt64OrStringNotEmptySuper XmlUnion
//class XmlInt64OrStringNotEmpty : public XmlInt64OrStringNotEmptySuper
//{
//public:
//  XmlInt64 xmlInt64 = XmlInt64();
//  XmlString8 xmlString8 = XmlString8();
//  virtual const char* getDescription() override { return "int64|string"; };
//  XmlUnionField m_fields[2] = { xmlInt64, xmlString8 };
//  virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
//};
//


#endif /* XmlLiteCompositeTypes_h */

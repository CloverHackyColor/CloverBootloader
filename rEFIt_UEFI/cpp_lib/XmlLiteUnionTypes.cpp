/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#include "XmlLiteUnionTypes.h"


void XmlUnion::reset()
{
  XmlUnionSuper::reset();
  XmlUnionField* fields;
  size_t nb;
  getFields(&fields, &nb);
  if ( nb == 0 ) panic("XmlUnion::reset() : no field defined");
  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
    XmlUnionField& xmlUnionField = fields[idx];
    XmlAbstractType& xmlAbstractType = xmlUnionField.xmlAbstractType;
    xmlAbstractType.reset();
  }
}

bool XmlUnion::isTheNextTag(XmlLiteParser* xmlLiteParser)
{
  XmlUnionField* fields;
  size_t nb;
  getFields(&fields, &nb);
  if ( nb == 0 ) panic("XmlUnion::reset() : no field defined");
  
  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
    XmlUnionField& xmlUnionField = fields[idx];
    XmlAbstractType& xmlAbstractType = xmlUnionField.xmlAbstractType;
    if ( xmlAbstractType.isTheNextTag(xmlLiteParser) ) return true;
  }
  return false;
}

bool XmlUnion::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
{
  XmlParserPosition pos = xmlLiteParser->getPosition();
#ifdef JIEF_DEBUG
if ( xmlPath == "/Boot/XMPDetection"_XS8 ) {
  int i=0; (void)i;
}
#endif

  XmlUnionField* fields;
  size_t nb;
  getFields(&fields, &nb);
  if ( nb == 0 ) panic("XmlUnion::parseFromXmlLite() : no field defined");
  
  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
    XmlUnionField& xmlUnionField = fields[idx];
    XmlAbstractType& xmlAbstractType = xmlUnionField.xmlAbstractType;
    if ( xmlAbstractType.isTheNextTag(xmlLiteParser) ) {
      if ( xmlAbstractType.parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors) ) {
        setDefined();
        return true;
      }
      return false;
    }else{
    }
  }
  xmlLiteParser->addError(generateErrors, S8Printf("Expecting %s for tag '%s:%d'", getDescription(), xmlPath.c_str(), pos.getLine()));
  return false;
}

bool XmlUnion::validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& pos, bool generateErrors)
{
  XmlUnionField* fields;
  size_t nb;
  getFields(&fields, &nb);
  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
    XmlUnionField& xmlUnionField = fields[idx];
    XmlAbstractType& xmlAbstractType = xmlUnionField.xmlAbstractType;
    if ( xmlAbstractType.isDefined()  &&  !xmlAbstractType.validate(xmlLiteParser, xmlPath, pos, generateErrors) ) {
      return false;
    }
  }
  return true;
}






/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#include "XmlLiteCompositeTypes.h"


//
//
//void XmlComposite::reset()
//{
//  super::reset();
//  XmlCompositeField* fields;
//  size_t nb;
//  getFields(&fields, &nb);
//  if ( nb == 0 ) panic("XmlComposite::reset() : no field defined");
//  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
//    XmlCompositeField& xmlCompositeField = fields[idx];
//    XmlAbstractType& xmlAbstractType = xmlCompositeField.xmlAbstractType;
//    xmlAbstractType.reset();
//  }
//}
//
//bool XmlComposite::isTheNextTag(XmlLiteParser* xmlLiteParser)
//{
//  XmlCompositeField* fields;
//  size_t nb;
//  getFields(&fields, &nb);
//  if ( nb == 0 ) panic("XmlComposite::isTheNextTag() : no field defined");
//  
//  if ( nb > 0 ) {
//    XmlCompositeField& xmlCompositeField = fields[0];
//    XmlAbstractType& xmlAbstractType = xmlCompositeField.xmlAbstractType;
//    if ( xmlAbstractType.isTheNextTag(xmlLiteParser) ) return true;
//  }
//  return false;
//}
//
//bool XmlComposite::parseFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors)
//{
//  XmlCompositeField* fields;
//  size_t nb;
//  getFields(&fields, &nb);
//  if ( nb == 0 ) panic("XmlComposite::parseFromXmlLite() : no field defined");
//  
//#ifdef JIEF_DEBUG
//  if ( xmlPath.startWithOrEqualToIC("/Devices/Properties") ) {
//    printf("%s", "");
//  }
//#endif
//
//  XString8 xmlSubPath = xmlPath; // TODO : concat to xmlPath and then truncate instead of constructing new ones.
//  
//  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
//    XmlCompositeField& xmlCompositeField = fields[idx];
//    XmlAbstractType& xmlAbstractType = xmlCompositeField.xmlAbstractType;
//    if ( !xmlAbstractType.parseFromXmlLite(xmlLiteParser, xmlSubPath, generateErrors) ) return false;
//    if ( xmlAbstractType.isKey() ) xmlSubPath.S8Catf("/%s", xmlAbstractType.getKey().c_str());
//  }
//  setDefined();
//  return true;
//}
//
//
//bool XmlComposite::validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& pos, bool generateErrors)
//{
//  XmlCompositeField* fields;
//  size_t nb;
//  getFields(&fields, &nb);
//  for ( size_t idx = 0 ; idx < nb ; idx++ ) {
//    XmlCompositeField& xmlCompositeField = fields[idx];
//    XmlAbstractType& xmlAbstractType = xmlCompositeField.xmlAbstractType;
//    if ( !xmlAbstractType.validate(xmlLiteParser, xmlPath, pos, generateErrors) ) {
//      return false;
//    }
//  }
//  return true;
//}
//
//

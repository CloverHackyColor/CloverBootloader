/*
 * ConfigPlist.cpp
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#include "ConfigPlistAbstract.h"
#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"



XBool ConfigPlistAbstractClass::parse(XmlLiteParser* xmlLiteParser, const XString8& xmlPath)
{
  xmlLiteParser->moveForwardUntilSignificant();
  xmlLiteParser->skipHeader();
  auto pos = xmlLiteParser->getPosition();
  XBool b = parseFromXmlLite(xmlLiteParser, xmlPath, true);
  if ( !b ) return false;
  b = validate(xmlLiteParser, xmlPath, pos, true);
  return b;
}

XBool ConfigPlistAbstractClass::parse(const LString8& buf, size_t size, const XString8& xmlPath, XmlLiteParser* xmlLiteParser)
{
  xmlLiteParser->init(buf.c_str(), size);
  return parse(xmlLiteParser, xmlPath);
}


XBool ConfigPlistAbstractClass::parse(const LString8& buf, size_t size, const XString8& xmlPath)
{
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(buf.c_str(), size);
  return parse(&xmlLiteParser, xmlPath);
}

XBool ConfigPlistAbstractClass::parse(const LString8& buf, size_t size)
{
  return parse(buf, size, ""_XS8);
}

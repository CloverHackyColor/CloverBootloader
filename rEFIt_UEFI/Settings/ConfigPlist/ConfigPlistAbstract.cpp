/*
 * ConfigPlist.cpp
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#include "ConfigPlistAbstract.h"
#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"



bool ConfigPlistAbstractClass::parse(XmlLiteParser* xmlLiteParser, const XString8& xmlPath)
{
  xmlLiteParser->moveForwardUntilSignificant();
  xmlLiteParser->skipHeader();
  auto pos = xmlLiteParser->getPosition();
  bool b = parseFromXmlLite(xmlLiteParser, xmlPath, true);
  if ( !b ) return false;
  b = validate(xmlLiteParser, xmlPath, pos, true);
  return b;
}

bool ConfigPlistAbstractClass::parse(const LString8& buf, size_t size, const XString8& xmlPath, XmlLiteParser* xmlLiteParser)
{
  xmlLiteParser->init(buf.c_str(), size);
  return parse(xmlLiteParser, xmlPath);
}


bool ConfigPlistAbstractClass::parse(const LString8& buf, size_t size, const XString8& xmlPath)
{
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(buf.c_str(), size);
  return parse(&xmlLiteParser, xmlPath);
}

bool ConfigPlistAbstractClass::parse(const LString8& buf, size_t size)
{
  return parse(buf, size, ""_XS8);
}

/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTABSTRACTCLASS_H_
#define _CONFIGPLISTABSTRACTCLASS_H_

#include "../../cpp_lib/XmlLiteDictTypes.h"


class ConfigPlistAbstractClass : public XmlDict
{
  using super = XmlDict;
public:

  ConfigPlistAbstractClass() {};

  bool parse(XmlLiteParser* xmlLiteParser, const XString8& xmlPath);
  bool parse(const LString8& buf, size_t size, const XString8& xmlPath, XmlLiteParser* xmlLiteParser);
  bool parse(const LString8& buf, size_t size, const XString8& xmlPath);
  bool parse(const LString8& buf, size_t size);

};

#endif /* _CONFIGPLISTABSTRACTCLASS_H_ */

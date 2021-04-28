/*
 * ConfigPlist.cpp
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#include "ConfigPlistClass.h"
#include "Config_ACPI.h"
#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"


//
//XmlAbstractType& ConfigPlistClass::ACPI_Class::ACPI_RenamesDevices_Class::parseKeyAndValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const char** keyValuePtr, size_t* keyValueLengthPtr)
//{
//  XmlKeyAndValue<XmlString8AllowEmpty>* kv = new XmlKeyAndValue<XmlString8AllowEmpty>();
//  if ( kv->parseFromXmlLite(xmlLiteParser, xmlPath, generateErrors) ) {
//    KV.AddReference(kv, true);
//    *keyValuePtr = kv->xmlKey.xstring8.c_str();
//    *keyValueLengthPtr = kv->xmlKey.xstring8.sizeInBytes();
//    return *kv;
//  }else{
//    delete kv;
//    return nullXmlType;
//  }
//}

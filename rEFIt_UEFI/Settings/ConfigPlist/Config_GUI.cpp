/*
 * ConfigPlist.cpp
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

//#include "Config_CPU.h"
//#include "../../cpp_lib/XmlLiteSimpleTypes.h"
//#include "../../cpp_lib/XmlLiteParser.h"
//
//#include "Config_GUI.h"

#include "ConfigPlistClass.h"


bool ConfigPlistClass::GUI_Class::GUI_Custom_Class::GUI_Custom_Entry_Class::validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) {
  if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
  for ( size_t idx=0 ; idx < SubEntries.size() ; ++idx ) SubEntries.ElementAt(idx).Parent = this;
  if ( Arguments.isDefined() && AddArguments.isDefined() ) {
    xmlLiteParser->addError(generateErrors, S8Printf("Arguments is ignored because AddArguments is defined. Line %d.", keyPos.getLine()));
    Arguments.reset();
  }
  return true;
}

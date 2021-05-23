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



XmlAbstractType& ConfigPlistClass::ACPI_Class::DSDT_Class::ACPI_DSDT_Fixes_Class::parseValueFromXmlLite(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, bool generateErrors, const XmlParserPosition &keyPos, const char *keyValue, size_t keyValueLength, bool* keyFound)
{
#ifdef JIEF_DEBUG
  if ( strncmp(keyValue, "FixHeaders", strlen("FixHeaders")) == 0 ) {
    NOP;
  }
#endif
    for ( size_t idx = 0 ; idx < sizeof(ACPI_DSDT_Fixe_Array)/sizeof(ACPI_DSDT_Fixe_Array[0]) ; idx++ )
    {
      if ( ACPI_DSDT_Fixe_Array[idx].getNewName() && strnIsEqualIC(keyValue, keyValueLength, ACPI_DSDT_Fixe_Array[idx].getNewName()) ) {
        // new name
        if ( ACPI_DSDT_Fixe_Array[idx].oldEnabled.isDefined() ) {
          xmlLiteParser->addWarning(true, S8Printf("Tag '%s:%d' was already defined with the old name '%s'. Previous value ignored.", xmlPath.c_str(), keyPos.getLine(), ACPI_DSDT_Fixe_Array[idx].m_oldName));
          ACPI_DSDT_Fixe_Array[idx].oldEnabled.reset();
        }
        if ( ACPI_DSDT_Fixe_Array[idx].newEnabled.isDefined() ) {
          xmlLiteParser->addWarning(true, S8Printf("Tag '%s:%d' was already defined. Previous value ignored.", xmlPath.c_str(), keyPos.getLine()));
          ACPI_DSDT_Fixe_Array[idx].newEnabled.reset();
        }
        ACPI_DSDT_Fixe_Array[idx].newEnabled.parseFromXmlLite(xmlLiteParser, xmlPath, true);
        ACPI_DSDT_Fixe_Array[idx].setDefined();
        *keyFound = true;
        return ACPI_DSDT_Fixe_Array[idx].newEnabled;
      }else
      if ( ACPI_DSDT_Fixe_Array[idx].m_oldName && strnIsEqualIC(keyValue, keyValueLength, ACPI_DSDT_Fixe_Array[idx].m_oldName) ) {
        // old name
        if ( ACPI_DSDT_Fixe_Array[idx].newEnabled.isDefined() ) {
          xmlLiteParser->addWarning(true, S8Printf("Tag '%s:%d' was already defined with the new name '%s'. Previous value ignored.", xmlPath.c_str(), keyPos.getLine(), ACPI_DSDT_Fixe_Array[idx].getNewName()));
          ACPI_DSDT_Fixe_Array[idx].newEnabled.reset();
        }
        if ( ACPI_DSDT_Fixe_Array[idx].oldEnabled.isDefined() ) {
          xmlLiteParser->addWarning(true, S8Printf("Tag '%s:%d' was already defined. Previous value ignored.", xmlPath.c_str(), keyPos.getLine()));
          ACPI_DSDT_Fixe_Array[idx].oldEnabled.reset();
        }
        ACPI_DSDT_Fixe_Array[idx].oldEnabled.parseFromXmlLite(xmlLiteParser, xmlPath, true);
        ACPI_DSDT_Fixe_Array[idx].setDefined();
        *keyFound = true;
        return ACPI_DSDT_Fixe_Array[idx].oldEnabled;
      }
    }
    *keyFound = false;
    return nullXmlType;
}



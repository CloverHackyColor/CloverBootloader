/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSACPI_H_
#define _CONFIGPLIST_COMPARESETTINGSACPI_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareAcpi(const XString8& label, const SETTINGS_DATA::ACPIClass& oldS, const ConfigPlistClass::ACPI_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSACPI_H_ */

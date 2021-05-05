/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSACPI_H_
#define _CONFIGPLIST_AssignSETTINGSACPI_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void AssignAcpi(const XString8& label, SETTINGS_DATA::ACPIClass& oldS, const ConfigPlistClass::ACPI_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSACPI_H_ */

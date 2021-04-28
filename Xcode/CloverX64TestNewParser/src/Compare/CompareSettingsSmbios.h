/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSSMBIOS_H_
#define _CONFIGPLIST_COMPARESETTINGSSMBIOS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareSmbios(const XString8& label, const SETTINGS_DATA::SmbiosClass& oldS, const SmbiosPlistClass::SmbiosDictClass& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSSMBIOS_H_ */

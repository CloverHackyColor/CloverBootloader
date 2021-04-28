/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSBOOT_H_
#define _CONFIGPLIST_COMPARESETTINGSBOOT_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareBoot(const XString8& label, const SETTINGS_DATA::BootClass& oldS, const ConfigPlistClass::Boot_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSBOOT_H_ */

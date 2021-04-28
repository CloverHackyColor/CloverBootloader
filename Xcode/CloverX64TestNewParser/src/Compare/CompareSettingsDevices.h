/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSDEVICES_H_
#define _CONFIGPLIST_COMPARESETTINGSDEVICES_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareDevices(const XString8& label, const SETTINGS_DATA::DevicesClass& oldS, const ConfigPlistClass::DevicesClass& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSDEVICES_H_ */

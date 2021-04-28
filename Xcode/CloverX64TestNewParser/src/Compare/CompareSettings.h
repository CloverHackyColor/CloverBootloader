/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGS_H_
#define _CONFIGPLIST_COMPARESETTINGS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

uint64_t CompareOldNewSettings(const SETTINGS_DATA& olDSettings, const ConfigPlistClass& configPlist, const XString8& label = ""_XS8);



#endif /* _CONFIGPLIST_COMPARESETTINGS_H_ */

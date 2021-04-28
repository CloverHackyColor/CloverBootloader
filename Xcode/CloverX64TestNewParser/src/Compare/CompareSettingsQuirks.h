/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSQUIRKS_H_
#define _CONFIGPLIST_COMPARESETTINGSQUIRKS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareQuirks(const XString8& label, const SETTINGS_DATA::QuirksClass& oldS, const ConfigPlistClass::Quirks_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSQUIRKS_H_ */

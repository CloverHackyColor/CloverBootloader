/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSRTVARIABLES_H_
#define _CONFIGPLIST_COMPARESETTINGSRTVARIABLES_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareRtVariables(const XString8& label, const SETTINGS_DATA::RtVariablesClass& oldS, const ConfigPlistClass::RtVariables_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSRTVARIABLES_H_ */

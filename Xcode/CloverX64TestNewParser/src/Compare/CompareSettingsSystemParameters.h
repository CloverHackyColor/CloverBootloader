/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSSYSTEMPARAMETERS_H_
#define _CONFIGPLIST_COMPARESETTINGSSYSTEMPARAMETERS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void CompareSystemParameters(const XString8& label, const SETTINGS_DATA::SystemParametersClass& oldS, const ConfigPlistClass::SystemParameters_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSSYSTEMPARAMETERS_H_ */

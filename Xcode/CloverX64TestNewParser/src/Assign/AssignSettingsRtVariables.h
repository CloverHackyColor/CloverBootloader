/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSRTVARIABLES_H_
#define _CONFIGPLIST_AssignSETTINGSRTVARIABLES_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignRtVariables(const XString8& label, SETTINGS_DATA::RtVariablesClass& oldS, const ConfigPlistClass::RtVariables_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSRTVARIABLES_H_ */

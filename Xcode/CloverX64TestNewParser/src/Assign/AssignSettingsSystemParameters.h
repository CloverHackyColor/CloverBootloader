/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSSYSTEMPARAMETERS_H_
#define _CONFIGPLIST_AssignSETTINGSSYSTEMPARAMETERS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void AssignSystemParameters(const XString8& label, SETTINGS_DATA::SystemParametersClass& oldS, const ConfigPlistClass::SystemParameters_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSSYSTEMPARAMETERS_H_ */

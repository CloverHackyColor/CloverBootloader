/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSBOOT_H_
#define _CONFIGPLIST_AssignSETTINGSBOOT_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignBoot(const XString8& label, SETTINGS_DATA::BootClass& oldS, const ConfigPlistClass::Boot_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSBOOT_H_ */

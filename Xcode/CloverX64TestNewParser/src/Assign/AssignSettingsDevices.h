/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSDEVICES_H_
#define _CONFIGPLIST_AssignSETTINGSDEVICES_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignDevices(const XString8& label, SETTINGS_DATA::DevicesClass& oldS, const ConfigPlistClass::DevicesClass& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSDEVICES_H_ */

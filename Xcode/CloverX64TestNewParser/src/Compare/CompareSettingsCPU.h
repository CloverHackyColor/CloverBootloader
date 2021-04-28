/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSCPU_H_
#define _CONFIGPLIST_COMPARESETTINGSCPU_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareCPU(const XString8& label, const SETTINGS_DATA::CPUClass& oldS, const ConfigPlistClass::CPU_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSBOOT_H_ */

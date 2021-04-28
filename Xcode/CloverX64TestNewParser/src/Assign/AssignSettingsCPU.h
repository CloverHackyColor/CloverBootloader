/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSCPU_H_
#define _CONFIGPLIST_AssignSETTINGSCPU_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignCPU(const XString8& label, SETTINGS_DATA::CPUClass& oldS, const ConfigPlistClass::CPU_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSBOOT_H_ */

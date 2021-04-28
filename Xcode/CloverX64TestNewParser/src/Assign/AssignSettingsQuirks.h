/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSQUIRKS_H_
#define _CONFIGPLIST_AssignSETTINGSQUIRKS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignQuirks(const XString8& label, SETTINGS_DATA::QuirksClass& oldS, const ConfigPlistClass::Quirks_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSQUIRKS_H_ */

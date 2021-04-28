/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSKERNELANDKEXTPATCH_H_
#define _CONFIGPLIST_AssignSETTINGSKERNELANDKEXTPATCH_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignKernelAndKextPatches(const XString8& label, KERNEL_AND_KEXT_PATCHES& oldS, const ConfigPlistClass::KernelAndKextPatches_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGS_H_ */

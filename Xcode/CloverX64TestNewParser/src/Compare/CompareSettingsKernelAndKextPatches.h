/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSKERNELANDKEXTPATCH_H_
#define _CONFIGPLIST_COMPARESETTINGSKERNELANDKEXTPATCH_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void CompareKernelAndKextPatches(const XString8& label, const KERNEL_AND_KEXT_PATCHES& oldS, const ConfigPlistClass::KernelAndKextPatches_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGS_H_ */

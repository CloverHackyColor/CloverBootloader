/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSSMBIOS_H_
#define _CONFIGPLIST_AssignSETTINGSSMBIOS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignSmbios(const XString8& label, SETTINGS_DATA::SmbiosClass& oldS, const SmbiosPlistClass::SmbiosDictClass& newS, const SmbiosPlistClass& smbiosPlist);



#endif /* _CONFIGPLIST_AssignSETTINGSSMBIOS_H_ */

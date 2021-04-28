/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGS_H_
#define _CONFIGPLIST_AssignSETTINGS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/SMBIOSPlist.h"

uint64_t AssignOldNewSettings(SETTINGS_DATA& olDSettings, const ConfigPlistClass& configPlist, const SmbiosPlistClass& smbiosPlist, const XString8& label = ""_XS8);



#endif /* _CONFIGPLIST_AssignSETTINGS_H_ */

/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSGRAPHICS_H_
#define _CONFIGPLIST_AssignSETTINGSGRAPHICS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void AssignGraphics(const XString8& label, SETTINGS_DATA::GraphicsClass& oldS, const ConfigPlistClass::Graphics_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSGRAPHICS_H_ */

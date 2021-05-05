/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignSETTINGSGUI_H_
#define _CONFIGPLIST_AssignSETTINGSGUI_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void AssignGUI(const XString8& label, SETTINGS_DATA::GUIClass& oldS, const ConfigPlistClass::GUI_Class& newS);



#endif /* _CONFIGPLIST_AssignSETTINGSGUI_H_ */

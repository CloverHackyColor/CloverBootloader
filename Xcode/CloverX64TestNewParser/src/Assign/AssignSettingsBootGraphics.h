/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSBOOTGRAPHICS_H_
#define _CONFIGPLIST_COMPARESETTINGSBOOTGRAPHICS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void AssignBootGraphics(const XString8& label, SETTINGS_DATA::BootGraphicsClass& oldS, const ConfigPlistClass::BootGraphics_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSBOOTGRAPHICS_H_ */

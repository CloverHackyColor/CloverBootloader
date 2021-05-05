/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPARESETTINGSGRAPHICS_H_
#define _CONFIGPLIST_COMPARESETTINGSGRAPHICS_H_

#include "../../Platform/Settings.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void CompareGraphics(const XString8& label, const SETTINGS_DATA::GraphicsClass& oldS, const ConfigPlistClass::Graphics_Class& newS);



#endif /* _CONFIGPLIST_COMPARESETTINGSGRAPHICS_H_ */

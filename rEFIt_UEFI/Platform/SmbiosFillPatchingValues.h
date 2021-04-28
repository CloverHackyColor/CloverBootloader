/*
 * SmbiosFillPatchingValues.h
 *
 *  Created on: Apr 28, 2021
 *      Author: jief
 */

#ifndef PLATFORM_SMBIOSFILLPATCHINGVALUES_H_
#define PLATFORM_SMBIOSFILLPATCHINGVALUES_H_

#include "../Settings/ConfigManager.h"
#include "../Platform/Settings.h"
#include "../Platform/smbios.h"

void SmbiosFillPatchingValues(bool _SetTable132, uint8_t pEnabledCores, uint16_t pRamSlotCount, const SlotDeviceArrayClass& SlotDeviceArray, const SETTINGS_DATA& globalSettings, const CPU_STRUCTURE& CPUStructure, SmbiosInjectedSettings* smbiosInjectedSettingsPtr);



#endif /* PLATFORM_SMBIOSFILLPATCHINGVALUES_H_ */

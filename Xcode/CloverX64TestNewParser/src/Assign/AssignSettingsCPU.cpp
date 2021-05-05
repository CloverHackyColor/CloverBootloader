/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsBoot.h"
#include <Platform.h>
#include "AssignField.h"
#include "../../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

void AssignCPU(const XString8& label, SETTINGS_DATA::CPUClass& oldS, const ConfigPlistClass::CPU_Class& newS)
{
  Assign(QPI);
  Assign(CpuFreqMHz);
  Assign(CpuType);
  Assign(QEMU);
  Assign(UseARTFreq);
  Assign(BusSpeed);
  Assign(UserChange);
  Assign(SavingMode);
  Assign(HWPEnable);
  Assign(HWPValue);
  Assign(TDP);
  Assign(TurboDisabled);
  Assign(_EnableC6);
  Assign(_EnableC4);
  Assign(_EnableC2);
  Assign(_C3Latency);
}

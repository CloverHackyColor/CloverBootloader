/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsBoot.h"
#include <Platform.h>
#include "CompareField.h"
#include "../../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"

void CompareCPU(const XString8& label, const SETTINGS_DATA::CPUClass& oldS, const ConfigPlistClass::CPU_Class& newS)
{
  compare(QPI);
  compare(CpuFreqMHz);
  compare(CpuType);
  compare(QEMU);
  compare(UseARTFreq);
  compare(BusSpeed);
  compare(UserChange);
  compare(SavingMode);
  compare(HWPEnable);
  compare(HWPValue);
  compare(TDP);
  compare(TurboDisabled);
  compare(_EnableC6);
  compare(_EnableC4);
  compare(_EnableC2);
  compare(_C3Latency);
}

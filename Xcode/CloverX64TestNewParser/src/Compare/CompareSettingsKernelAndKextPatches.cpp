/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettingsKernelAndKextPatches.h"
#include <Platform.h>
//#include "../../include/OSFlags.h"
#include "CompareField.h"


void CompareAbstractPtach(const XString8& label, const ABSTRACT_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_AbstractPatch_Class& newS)
{
  compare(Disabled);
  compare(Find);
  compare(Replace);
  compare(MaskFind);
  compare(MaskReplace);
  compare(StartPattern);
  compare(StartMask);
  compare(SearchLen);
  compare(Count);
  compare(Skip);
  compare(MatchOS);
  compare(MatchBuild);
  compare(Label);
  compareField(oldS.MenuItem.BValue, (uint8_t)(!newS.dgetDisabled()), S8Printf("%s.MenuItem.BValue", label.c_str()));
}

void CompareABSTRACT_KEXT_OR_KERNEL_PATCH(const XString8& label, const ABSTRACT_KEXT_OR_KERNEL_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::ABSTRACT_KEXT_OR_KERNEL_PATCH& newS)
{
  CompareAbstractPtach(label, oldS, newS);
  compare(ProcedureName);
}

void CompareKextPatch(const XString8& label, const KEXT_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KextsToPatch_Class& newS)
{
  CompareABSTRACT_KEXT_OR_KERNEL_PATCH(label, oldS, newS);
  compare(IsPlistPatch);
  compare(Name);
}

void CompareKextPatches(const XString8& label, const XObjArray<KEXT_PATCH>& oldS, const XmlArray<ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KextsToPatch_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareKextPatch(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareKernelPatch(const XString8& label, const KERNEL_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KernelToPatch_Class& newS)
{
  CompareABSTRACT_KEXT_OR_KERNEL_PATCH(label, oldS, newS);
}

void CompareKernelPatches(const XString8& label, const XObjArray<KERNEL_PATCH>& oldS, const XmlArray<ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KernelToPatch_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareKernelPatch(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareBootPatch(const XString8& label, const BOOT_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_BootPatch_Class& newS)
{
  CompareAbstractPtach(label, oldS, newS);
}

void CompareBootPatches(const XString8& label, const XObjArray<BOOT_PATCH>& oldS, const XmlArray<ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_BootPatch_Class>& newS)
{
  if ( fcompare(size()) ) {
    for ( size_t idx = 0; idx < oldS.size (); ++idx )
    {
      CompareBootPatch(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void CompareKernelAndKextPatches(const XString8& label, const KERNEL_AND_KEXT_PATCHES& oldS, const ConfigPlistClass::KernelAndKextPatches_Class& newS)
{
  compare(KPDebug);
  compare(KPKernelLapic);
  compare(KPKernelXCPM);
  compare(_KPKernelPm);
  compare(KPPanicNoKextDump);
  compare(_KPAppleIntelCPUPM);
  compare(KPAppleRTC);
  compare(EightApple);
  compare(KPDELLSMBIOS);
  compare(FakeCPUID);
  compare(KPATIConnectorsController);
  compare(KPATIConnectorsData);
  compare(KPATIConnectorsPatch);
  compare(ForceKextsToLoad);
  CompareKextPatches(S8Printf("%s.KextPatches", label.c_str()), oldS.KextPatches, newS.KextsToPatch);
  CompareKernelPatches(S8Printf("%s.KernelPatches", label.c_str()), oldS.KernelPatches, newS.KernelToPatch);
  CompareBootPatches(S8Printf("%s.BootPatches", label.c_str()), oldS.BootPatches, newS.BootPatches);

}


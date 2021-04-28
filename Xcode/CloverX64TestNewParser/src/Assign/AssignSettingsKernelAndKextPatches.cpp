/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettingsKernelAndKextPatches.h"
#include <Platform.h>
//#include "../../include/OSFlags.h"
#include "AssignField.h"


void AssignAbstractPtach(const XString8& label, ABSTRACT_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_AbstractPatch_Class& newS)
{
  Assign(Disabled);
  Assign(Find);
  Assign(Replace);
  Assign(MaskFind);
  Assign(MaskReplace);
  Assign(StartPattern);
  Assign(StartMask);
  Assign(SearchLen);
  Assign(Count);
  Assign(Skip);
  Assign(MatchOS);
  Assign(MatchBuild);
  Assign(Label);
  oldS.MenuItem.BValue = (uint8_t)(!newS.dgetDisabled());

}

void AssignABSTRACT_KEXT_OR_KERNEL_PATCH(const XString8& label, ABSTRACT_KEXT_OR_KERNEL_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::ABSTRACT_KEXT_OR_KERNEL_PATCH& newS)
{
  AssignAbstractPtach(label, oldS, newS);
  Assign(ProcedureName);
}

void AssignKextPatch(const XString8& label, KEXT_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KextsToPatch_Class& newS)
{
  AssignABSTRACT_KEXT_OR_KERNEL_PATCH(label, oldS, newS);
  Assign(IsPlistPatch);
  Assign(Name);
}

void AssignKextPatches(const XString8& label, XObjArray<KEXT_PATCH>& oldS, const XmlArray<ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KextsToPatch_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignKextPatch(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignKernelPatch(const XString8& label, KERNEL_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KernelToPatch_Class& newS)
{
  AssignABSTRACT_KEXT_OR_KERNEL_PATCH(label, oldS, newS);
}

void AssignKernelPatches(const XString8& label, XObjArray<KERNEL_PATCH>& oldS, const XmlArray<ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KernelToPatch_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignKernelPatch(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignBootPatch(const XString8& label, BOOT_PATCH& oldS, const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_BootPatch_Class& newS)
{
  AssignAbstractPtach(label, oldS, newS);
  Assign(Label);
}

void AssignBootPatches(const XString8& label, XObjArray<BOOT_PATCH>& oldS, const XmlArray<ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_BootPatch_Class>& newS)
{
  if ( sizeAssign(size()) ) {
    for ( size_t idx = 0; idx < newS.size (); ++idx )
    {
      oldS.AddReference(new (remove_ref(decltype(oldS))::type)(), true);
      AssignBootPatch(S8Printf("%s[%zu]", label.c_str(), idx), oldS[idx], newS[idx]);
    }
  }
}

void AssignKernelAndKextPatches(const XString8& label, KERNEL_AND_KEXT_PATCHES& oldS, const ConfigPlistClass::KernelAndKextPatches_Class& newS)
{
  Assign(KPDebug);
  Assign(KPKernelLapic);
  Assign(KPKernelXCPM);
  Assign(_KPKernelPm);
  Assign(KPPanicNoKextDump);
  Assign(_KPAppleIntelCPUPM);
  Assign(KPAppleRTC);
  Assign(EightApple);
  Assign(KPDELLSMBIOS);
  Assign(FakeCPUID);
  Assign(KPATIConnectorsController);
  Assign(KPATIConnectorsData);
  Assign(KPATIConnectorsPatch);
  Assign(ForceKextsToLoad);
  AssignKextPatches(S8Printf("%s.KextPatches", label.c_str()), oldS.KextPatches, newS.KextsToPatch);
  AssignKernelPatches(S8Printf("%s.KernelPatches", label.c_str()), oldS.KernelPatches, newS.KernelToPatch);
  AssignBootPatches(S8Printf("%s.BootPatches", label.c_str()), oldS.BootPatches, newS.BootPatches);

}


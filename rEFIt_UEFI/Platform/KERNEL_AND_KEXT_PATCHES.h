/*
 * KERNEL_AND_KEXT_PATCHES.h
 *
 *  Created on: 4 Feb 2021
 *      Author: jief
 */

#ifndef __KERNEL_AND_KEXT_PATCHES_H__
#define __KERNEL_AND_KEXT_PATCHES_H__

#include "../cpp_foundation/XBuffer.h"
#include "../libeg/libeg.h"
#include "MacOsVersion.h"
#include "../Settings/ConfigPlist/ConfigPlistClass.h"
#include "../Platform/SettingsUtils.h"

extern "C" {
#  include <Library/OcConfigurationLib.h>
}


class ABSTRACT_PATCH
{
public:
  XBool            Disabled = XBool();
  XBuffer<UINT8>   Find = XBuffer<UINT8> ();
  XBuffer<UINT8>   Replace = XBuffer<UINT8> ();
  XBuffer<UINT8>   MaskFind = XBuffer<UINT8> ();
  XBuffer<UINT8>   MaskReplace = XBuffer<UINT8> ();
  XBuffer<UINT8>   StartPattern = XBuffer<UINT8> ();
  XBuffer<UINT8>   StartMask = XBuffer<UINT8> ();
  INTN             SearchLen = INTN();
  INTN             Count = INTN();
  INTN             Skip = INTN();
  XString8         MatchOS = XString8();
  XString8         MatchBuild = XString8();
  INPUT_ITEM       MenuItem = INPUT_ITEM();

  // Computed
  virtual XString8 getName() const = 0;
  XString8         Label = XString8(); // TODO : it's a calculated value from comment field.

  virtual ~ABSTRACT_PATCH() {}
  
	#if __cplusplus > 201703L
		XBool operator == (const ABSTRACT_PATCH&) const = default;
	#endif
  XBool isEqual(const ABSTRACT_PATCH& other) const
  {
    if ( !(Disabled == other.Disabled ) ) return false;
    if ( !(Find == other.Find ) ) return false;
    if ( !(Replace == other.Replace ) ) return false;
    if ( !(MaskFind == other.MaskFind ) ) return false;
    if ( !(MaskReplace == other.MaskReplace ) ) return false;
    if ( !(StartPattern == other.StartPattern ) ) return false;
    if ( !(StartMask == other.StartMask ) ) return false;
    if ( !(SearchLen == other.SearchLen ) ) return false;
    if ( !(Count == other.Count ) ) return false;
    if ( !(Skip == other.Skip ) ) return false;
    if ( !(MatchOS == other.MatchOS ) ) return false;
    if ( !(MatchBuild == other.MatchBuild ) ) return false;
    if ( MenuItem != other.MenuItem ) return false;
    if ( !(Label == other.Label ) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_AbstractPatch_Class& other)
  {
    Disabled = other.dgetDisabled();
    Find = other.dgetFind();
    Replace = other.dgetReplace();
    MaskFind = other.dgetMaskFind();
    MaskReplace = other.dgetMaskReplace();
    StartPattern = other.dgetStartPattern();
    StartMask = other.dgetStartMask();
    SearchLen = other.dgetSearchLen();
    Count = other.dgetCount();
    Skip = other.dgetSkip();
    MatchOS = other.dgetMatchOS();
    MatchBuild = other.dgetMatchBuild();
    MenuItem.BValue = !other.dgetDisabled();
    Label = other.dgetLabel();
  }

/** Returns a boolean and then enable disable the patch if MachOSEntry have a match for the booted OS. */
  XBool IsPatchEnabledByBuildNumber(const XString8& Build);
  XBool IsPatchEnabled(const MacOsVersion& CurrOS);


};

class ABSTRACT_KEXT_OR_KERNEL_PATCH : public ABSTRACT_PATCH
{
  using super = ABSTRACT_PATCH;
public:
  XString8         ProcedureName = XString8(); //procedure len will be StartPatternLen


	#if __cplusplus > 201703L
		XBool operator == (const ABSTRACT_KEXT_OR_KERNEL_PATCH&) const = default;
	#endif
  XBool isEqual(const ABSTRACT_KEXT_OR_KERNEL_PATCH& other) const
  {
    if ( !super::isEqual (other) ) return false;
    if ( !(ProcedureName == other.ProcedureName ) ) return false;
    return true;
  }
  XBool takeValueFrom(const ConfigPlistClass::KernelAndKextPatches_Class::ABSTRACT_KEXT_OR_KERNEL_PATCH& other)
  {
    super::takeValueFrom(other);
    ProcedureName = other.dgetProcedureName();
    return true;
  }
};



class KEXT_PATCH : public ABSTRACT_KEXT_OR_KERNEL_PATCH
{
  using super = ABSTRACT_KEXT_OR_KERNEL_PATCH;
public:
  XString8         Name = XString8();
  XBool            IsPlistPatch = XBool();


  virtual XString8 getName() const { return Name; }

	#if __cplusplus > 201703L
		XBool operator == (const KEXT_PATCH&) const = default;
	#endif
  XBool isEqual(const KEXT_PATCH& other) const
  {
    if ( !super::isEqual (other) ) return false;
    if ( !(Name == other.Name ) ) return false;
    if ( !(IsPlistPatch == other.IsPlistPatch ) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KextsToPatch_Class& other)
  {
    super::takeValueFrom(other);
    Name = other.dgetName();
    IsPlistPatch = other.dgetIsPlistPatch();
  }
};

class KERNEL_PATCH : public ABSTRACT_KEXT_OR_KERNEL_PATCH
{
  using super = ABSTRACT_KEXT_OR_KERNEL_PATCH;
public:
  
  virtual XString8 getName() const { return "kernel"_XS8; }

	#if __cplusplus > 201703L
		XBool operator == (const KERNEL_PATCH&) const = default;
	#endif
  XBool isEqual(const KERNEL_PATCH& other) const
  {
    if ( !super::isEqual (other) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KernelToPatch_Class& other)
  {
    super::takeValueFrom(other);
  }
};

class BOOT_PATCH : public ABSTRACT_PATCH
{
  using super = ABSTRACT_PATCH;
public:
  
  virtual XString8 getName() const { return "boot.efi"_XS8; }

	#if __cplusplus > 201703L
		XBool operator == (const BOOT_PATCH&) const = default;
	#endif
  XBool isEqual(const BOOT_PATCH& other) const
  {
    if ( !super::isEqual (other) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_BootPatch_Class& other)
  {
    super::takeValueFrom(other);
  }
};

class KERNEL_AND_KEXT_PATCHES
{
public:
  XBool KPDebug = XBool();
  XBool KPKernelLapic = XBool();
  XBool KPKernelXCPM = XBool();
  XBool _KPKernelPm = XBool();
  XBool KPPanicNoKextDump = XBool();
  XBool _KPAppleIntelCPUPM = XBool();
  XBool KPAppleRTC = XBool();
  XBool BlockSkywalk = XBool();
  XBool EightApple = XBool();
  XBool KPDELLSMBIOS = XBool();  // Dell SMBIOS patch
  UINT32  FakeCPUID = UINT32();
  XString8 KPATIConnectorsController = XString8();
  XBuffer<UINT8> KPATIConnectorsData = XBuffer<UINT8>();
  XBuffer<UINT8> KPATIConnectorsPatch = XBuffer<UINT8>();
  XStringWArray ForceKextsToLoad/* = XStringWArray()*/;
  XObjArrayWithTakeValueFromXmlArray<KEXT_PATCH, ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KextsToPatch_Class> KextPatches/* = XObjArrayWithTakeValueFromXmlArray<KEXT_PATCH>()*/;
  XObjArrayWithTakeValueFromXmlArray<KERNEL_PATCH, ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_KernelToPatch_Class> KernelPatches/* = XObjArrayWithTakeValueFromXmlArray<KERNEL_PATCH>()*/;
  XObjArrayWithTakeValueFromXmlArray<BOOT_PATCH, ConfigPlistClass::KernelAndKextPatches_Class::KernelAndKextPatches_BootPatch_Class> BootPatches/* = XObjArrayWithTakeValueFromXmlArray<BOOT_PATCH>()*/;
  
  KERNEL_AND_KEXT_PATCHES() : ForceKextsToLoad(), KextPatches(), KernelPatches(), BootPatches() {}
  
	#if __cplusplus > 201703L
		XBool operator == (const KERNEL_AND_KEXT_PATCHES&) const = default;
	#endif
  XBool isEqual(const KERNEL_AND_KEXT_PATCHES& other) const
  {
    if ( !(KPDebug == other.KPDebug ) ) return false;
    if ( !(KPKernelLapic == other.KPKernelLapic ) ) return false;
    if ( !(KPKernelXCPM == other.KPKernelXCPM ) ) return false;
    if ( !(_KPKernelPm == other._KPKernelPm ) ) return false;
    if ( !(KPPanicNoKextDump == other.KPPanicNoKextDump ) ) return false;
    if ( !(_KPAppleIntelCPUPM == other._KPAppleIntelCPUPM ) ) return false;
    if ( !(KPAppleRTC == other.KPAppleRTC ) ) return false;
    if ( !(EightApple == other.EightApple ) ) return false;
    if ( !(BlockSkywalk == other.BlockSkywalk ) ) return false;
    if ( !(KPDELLSMBIOS == other.KPDELLSMBIOS ) ) return false;
    if ( !(FakeCPUID == other.FakeCPUID ) ) return false;
    if ( !(KPATIConnectorsController == other.KPATIConnectorsController ) ) return false;
    if ( !(KPATIConnectorsData == other.KPATIConnectorsData ) ) return false;
    if ( !(KPATIConnectorsPatch == other.KPATIConnectorsPatch ) ) return false;
    if ( !(ForceKextsToLoad == other.ForceKextsToLoad ) ) return false;
    if ( !KextPatches.isEqual(other.KextPatches) ) return false;
    if ( !KernelPatches.isEqual(other.KernelPatches) ) return false;
    if ( !BootPatches.isEqual(other.BootPatches) ) return false;
    return true;
  }
  void takeValueFrom(const ConfigPlistClass::KernelAndKextPatches_Class& other)
  {
    KPDebug = other.dgetKPDebug();
    KPKernelLapic = other.dgetKPKernelLapic();
    KPKernelXCPM = other.dgetKPKernelXCPM();
    _KPKernelPm = other.dget_KPKernelPm();
    KPPanicNoKextDump = other.dgetKPPanicNoKextDump();
    _KPAppleIntelCPUPM = other.dget_KPAppleIntelCPUPM();
    KPAppleRTC = other.dgetKPAppleRTC();
    BlockSkywalk = other.dgetBlockSkywalk();
    EightApple = other.dgetEightApple();
    KPDELLSMBIOS = other.dgetKPDELLSMBIOS();
    FakeCPUID = other.dgetFakeCPUID();
    KPATIConnectorsController = other.dgetKPATIConnectorsController();
    KPATIConnectorsData = other.dgetKPATIConnectorsData();
    KPATIConnectorsPatch = other.dgetKPATIConnectorsPatch();
    ForceKextsToLoad = other.dgetForceKextsToLoad();
    KextPatches.takeValueFrom(other.KextsToPatch);
    KernelPatches.takeValueFrom(other.KernelToPatch);
    BootPatches.takeValueFrom(other.BootPatches);
  }
} ;


#endif

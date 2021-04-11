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

extern "C" {
#  include <Library/OcConfigurationLib.h>
}


class ABSTRACT_PATCH
{
public:
  bool             Disabled = bool();
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
		bool operator == (const ABSTRACT_PATCH&) const = default;
	#endif
  bool isEqual(const ABSTRACT_PATCH& other) const
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

/** Returns a boolean and then enable disable the patch if MachOSEntry have a match for the booted OS. */
  bool IsPatchEnabledByBuildNumber(const XString8& Build);
  bool IsPatchEnabled(const MacOsVersion& CurrOS);


};

class ABSTRACT_KEXT_OR_KERNEL_PATCH : public ABSTRACT_PATCH
{
public:
  XString8         ProcedureName = XString8(); //procedure len will be StartPatternLen


	#if __cplusplus > 201703L
		bool operator == (const ABSTRACT_KEXT_OR_KERNEL_PATCH&) const = default;
	#endif
  bool isEqual(const ABSTRACT_KEXT_OR_KERNEL_PATCH& other) const
  {
    if ( !ABSTRACT_PATCH::isEqual (other) ) return false;
    if ( !(ProcedureName == other.ProcedureName ) ) return false;
    return true;
  }
};



class KEXT_PATCH : public ABSTRACT_KEXT_OR_KERNEL_PATCH
{
public:
  XString8         Name = XString8();
  bool             IsPlistPatch = BOOLEAN();


  virtual XString8 getName() const { return Name; }

	#if __cplusplus > 201703L
		bool operator == (const KEXT_PATCH&) const = default;
	#endif
  bool isEqual(const KEXT_PATCH& other) const
  {
    if ( !ABSTRACT_KEXT_OR_KERNEL_PATCH::isEqual (other) ) return false;
    if ( !(IsPlistPatch == other.IsPlistPatch ) ) return false;
    return true;
  }
};

class KERNEL_PATCH : public ABSTRACT_KEXT_OR_KERNEL_PATCH
{
public:
  
  virtual XString8 getName() const { return "kernel"_XS8; }

	#if __cplusplus > 201703L
		bool operator == (const KERNEL_PATCH&) const = default;
	#endif
  bool isEqual(const KERNEL_PATCH& other) const
  {
    if ( !ABSTRACT_KEXT_OR_KERNEL_PATCH::isEqual (other) ) return false;
    return true;
  }
};

class BOOT_PATCH : public ABSTRACT_PATCH
{
public:
  
  virtual XString8 getName() const { return "boot.efi"_XS8; }

	#if __cplusplus > 201703L
		bool operator == (const BOOT_PATCH&) const = default;
	#endif
  bool isEqual(const BOOT_PATCH& other) const
  {
    if ( !ABSTRACT_PATCH::isEqual (other) ) return false;
    return true;
  }
};

class KERNEL_AND_KEXT_PATCHES
{
public:
  bool KPDebug = bool();
  bool KPKernelLapic = bool();
  bool KPKernelXCPM = bool();
  bool _KPKernelPm = bool();
  bool KPPanicNoKextDump = bool();
  bool _KPAppleIntelCPUPM = bool();
  bool KPAppleRTC = bool();
  bool EightApple = bool();
  bool KPDELLSMBIOS = bool();  // Dell SMBIOS patch
  UINT32  FakeCPUID = UINT32();
  XString8 KPATIConnectorsController = XString8();
  XBuffer<UINT8> KPATIConnectorsData = XBuffer<UINT8>();
  XBuffer<UINT8> KPATIConnectorsPatch = XBuffer<UINT8>();
  XStringWArray ForceKextsToLoad/* = XStringWArray()*/;
  XObjArray<KEXT_PATCH> KextPatches/* = XObjArray<KEXT_PATCH>()*/;
  XObjArray<KERNEL_PATCH> KernelPatches/* = XObjArray<KERNEL_PATCH>()*/;
  XObjArray<BOOT_PATCH> BootPatches/* = XObjArray<BOOT_PATCH>()*/;
  
  KERNEL_AND_KEXT_PATCHES() : ForceKextsToLoad(), KextPatches(), KernelPatches(), BootPatches() {}
  
	#if __cplusplus > 201703L
		bool operator == (const KERNEL_AND_KEXT_PATCHES&) const = default;
	#endif
  bool isEqual(const KERNEL_AND_KEXT_PATCHES& other) const
  {
    if ( !(KPDebug == other.KPDebug ) ) return false;
    if ( !(KPKernelLapic == other.KPKernelLapic ) ) return false;
    if ( !(KPKernelXCPM == other.KPKernelXCPM ) ) return false;
    if ( !(_KPKernelPm == other._KPKernelPm ) ) return false;
    if ( !(KPPanicNoKextDump == other.KPPanicNoKextDump ) ) return false;
    if ( !(_KPAppleIntelCPUPM == other._KPAppleIntelCPUPM ) ) return false;
    if ( !(KPAppleRTC == other.KPAppleRTC ) ) return false;
    if ( !(EightApple == other.EightApple ) ) return false;
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
} ;


#endif

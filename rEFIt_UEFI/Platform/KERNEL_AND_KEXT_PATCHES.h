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
//  XString8         Comment = XString8();
  XBuffer<UINT8>   Data = XBuffer<UINT8> ();
  XBuffer<UINT8>   Patch = XBuffer<UINT8> ();
  XBuffer<UINT8>   MaskFind = XBuffer<UINT8> ();
  XBuffer<UINT8>   MaskReplace = XBuffer<UINT8> ();
  XBuffer<UINT8>   StartPattern = XBuffer<UINT8> ();
  XBuffer<UINT8>   StartMask = XBuffer<UINT8> ();
  INTN             SearchLen = INTN();
  INTN             Count = INTN();
  INTN             Skip = INTN();
  XString8         MatchOS = XString8();
  XString8         MatchBuild = XString8();

  // Computed
  XString8         Name = XString8();
  XString8         Label = XString8(); // TODO : it's a calculated value from comment field.

/** Returns a boolean and then enable disable the patch if MachOSEntry have a match for the booted OS. */
  bool IsPatchEnabledByBuildNumber(const XString8& Build);
  bool IsPatchEnabled(const MacOsVersion& CurrOS);

};

class ABSTRACT_KEXT_OR_KERNEL_PATCH : public ABSTRACT_PATCH
{
public:
  bool             IsPlistPatch = BOOLEAN();
  XString8         ProcedureName = XString8(); //procedure len will be StartPatternLen
  INPUT_ITEM       MenuItem = INPUT_ITEM();
};



class KEXT_PATCH : public ABSTRACT_KEXT_OR_KERNEL_PATCH
{
public:
  bool             IsPlistPatch = BOOLEAN();
  INPUT_ITEM       MenuItem = INPUT_ITEM();
};

class KERNEL_PATCH : public ABSTRACT_KEXT_OR_KERNEL_PATCH
{
public:
  INPUT_ITEM       MenuItem = INPUT_ITEM();
};

class BOOT_PATCH : public ABSTRACT_PATCH
{
public:
  INPUT_ITEM       MenuItem = INPUT_ITEM();
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
  XStringWArray ForceKextsToLoad = XStringWArray();
  XObjArray<KEXT_PATCH> KextPatches = XObjArray<KEXT_PATCH>();
  XObjArray<KERNEL_PATCH> KernelPatches = XObjArray<KERNEL_PATCH>();
  XObjArray<BOOT_PATCH> BootPatches = XObjArray<BOOT_PATCH>();
} ;


#endif

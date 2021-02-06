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


class KEXT_PATCH
{
public:
  XString8         Name;
  XString8         Label;
  BOOLEAN          IsPlistPatch;
  XBuffer<UINT8>   Data;
  XBuffer<UINT8>   Patch;
  XBuffer<UINT8>   MaskFind;
  XBuffer<UINT8>   MaskReplace;
  XBuffer<UINT8>   StartPattern;
  XBuffer<UINT8>   StartMask;
  INTN        SearchLen;
  XString8         ProcedureName; //procedure len will be StartPatternLen
  INTN             Count;
  INTN             Skip;
  XString8         MatchOS;
  XString8         MatchBuild;
  INPUT_ITEM  MenuItem = INPUT_ITEM();

  KEXT_PATCH() : Name(), Label(), IsPlistPatch(0), Data(), Patch(), MaskFind(), MaskReplace(),
                   StartPattern(), StartMask(), SearchLen(0), ProcedureName(), Count(-1), Skip(0), MatchOS(), MatchBuild()
                 { }
  KEXT_PATCH(const KEXT_PATCH& other) = default; // default is fine if there is only native type and objects that have copy ctor
  KEXT_PATCH& operator = ( const KEXT_PATCH & ) = default; // default is fine if there is only native type and objects that have copy ctor
  ~KEXT_PATCH() {}

/** Returns a boolean and then enable disable the patch if MachOSEntry have a match for the booted OS. */
  bool IsPatchEnabledByBuildNumber(const XString8& Build);
  bool IsPatchEnabled(const MacOsVersion& CurrOS);

};

class KERNEL_AND_KEXT_PATCHES
{
public:
  BOOLEAN FuzzyMatch;
  XString8 OcKernelCache;
  OC_KERNEL_QUIRKS OcKernelQuirks;
  BOOLEAN KPDebug;
//  BOOLEAN KPKernelCpu;
  BOOLEAN KPKernelLapic;
  BOOLEAN KPKernelXCPM;
  BOOLEAN KPKernelPm;
  BOOLEAN KPAppleIntelCPUPM;
  BOOLEAN KPAppleRTC;
  BOOLEAN KPDELLSMBIOS;  // Dell SMBIOS patch
  BOOLEAN KPPanicNoKextDump;
  BOOLEAN EightApple;
  UINT8   pad[7];
  UINT32  FakeCPUID;
  //  UINT32  align0;
  XString8 KPATIConnectorsController;
#if defined(MDE_CPU_IA32)
  UINT32  align1;
#endif

  XBuffer<UINT8> KPATIConnectorsData;
#if defined(MDE_CPU_IA32)
  UINT32  align2;
#endif

#if defined(MDE_CPU_IA32)
  UINT32  align3;
#endif
  XBuffer<UINT8> KPATIConnectorsPatch;
#if defined(MDE_CPU_IA32)
  UINT32  align4;
#endif

//  INT32   NrKexts;
  UINT32  align40;
  XObjArray<KEXT_PATCH> KextPatches;
#if defined(MDE_CPU_IA32)
  UINT32  align5;
#endif

//  INT32    NrForceKexts;
  UINT32  align50;
//  CHAR16 **ForceKexts;
  XStringWArray ForceKexts;
#if defined(MDE_CPU_IA32)
  UINT32 align6;
#endif
//  INT32   NrKernels;
  XObjArray<KEXT_PATCH> KernelPatches;
//  INT32   NrBoots;
  XObjArray<KEXT_PATCH> BootPatches;

  KERNEL_AND_KEXT_PATCHES() : FuzzyMatch(0), OcKernelCache(), OcKernelQuirks{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, KPDebug(0), KPKernelLapic(0), KPKernelXCPM(0), KPKernelPm(0), KPAppleIntelCPUPM(0), KPAppleRTC(0), KPDELLSMBIOS(0), KPPanicNoKextDump(0),
                   EightApple(0), pad{0}, FakeCPUID(0), KPATIConnectorsController(0), KPATIConnectorsData(),
                   KPATIConnectorsPatch(), align40(0), KextPatches(), align50(0), ForceKexts(),
                   KernelPatches(), BootPatches()
                 { }
  KERNEL_AND_KEXT_PATCHES(const KERNEL_AND_KEXT_PATCHES& other) = default; // Can be defined if needed
  KERNEL_AND_KEXT_PATCHES& operator = ( const KERNEL_AND_KEXT_PATCHES & ) = default; // Can be defined if needed
  ~KERNEL_AND_KEXT_PATCHES() {}

//  /** Returns a boolean and then enable disable the patch if MachOSEntry have a match for the booted OS. */
//  bool IsPatchEnabledByBuildNumber(const XString8& Build);

} ;


#endif

/* 

 cpu.c
 implementation for cpu

 Remade by Slice 2011 based on Apple's XNU sources
 Portion copyright from Chameleon project. Thanks to all who involved to it.
 */
/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#include "Platform.h"

#ifndef DEBUG_ALL
#define DEBUG_CPU 1
//#define DEBUG_PCI 1
#else
#define DEBUG_CPU DEBUG_ALL
//#define DEBUG_PCI DEBUG_ALL
#endif

#if DEBUG_CPU == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_CPU, __VA_ARGS__)  
#endif

#define VIRTUAL 0
#if VIRTUAL == 1
#define AsmReadMsr64(x) 0ULL
#define AsmWriteMsr64(m, x)
#endif

#define DivU64(x, y) DivU64x64Remainder((x), (y), NULL)

UINT8              gDefaultType; 
CPU_STRUCTURE      gCPUStructure;
UINT64            TurboMsr;
BOOLEAN           NeedPMfix = FALSE;

//this must not be defined at LegacyBios calls
#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

#define MSR_AMD_INT_PENDING_CMP_HALT    0xC0010055
#define AMD_ACTONCMPHALT_SHIFT  27
#define AMD_ACTONCMPHALT_MASK   3
// Bronya C1E fix
// * Portions Copyright 2009 Advanced Micro Devices, Inc.
void post_startup_cpu_fixups(void)
{
  /*
   * Some AMD processors support C1E state. Entering this state will
   * cause the local APIC timer to stop, which we can't deal with at
   * this time.
   */

  UINT64 reg;
  DBG("\tLooking to disable C1E if is already enabled by the BIOS:\n");
  reg = AsmReadMsr64(MSR_AMD_INT_PENDING_CMP_HALT);
  /* Disable C1E state if it is enabled by the BIOS */
  if ((reg >> AMD_ACTONCMPHALT_SHIFT) & AMD_ACTONCMPHALT_MASK)
  {
    reg &= ~(AMD_ACTONCMPHALT_MASK << AMD_ACTONCMPHALT_SHIFT);
    AsmWriteMsr64(MSR_AMD_INT_PENDING_CMP_HALT, reg);
    DBG("\tC1E disabled!\n");
  }
}


VOID DoCpuid(UINT32 selector, UINT32 *data)
{
  AsmCpuid(selector, data, data+1, data+2, data+3);
}

//
// Should be used after PrepatchSmbios() but before users's config.plist reading
//
VOID GetCPUProperties (VOID)
{
  UINT32    reg[4];
  UINT64    msr = 0;
  
  EFI_STATUS      Status;
  EFI_HANDLE      *HandleBuffer;
  //  EFI_GUID        **ProtocolGuidArray;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;
  UINTN         HandleCount;
  //  UINTN         ArrayCount;
  UINTN         HandleIndex;
  //  UINTN         ProtocolIndex;
  UINT64        qpibusspeed; //units=kHz
  UINT32        qpimult = 2;
  UINT32        BusSpeed = 0; //units kHz
  UINT64        ExternalClock;
  UINT64        tmpU;
  UINT16        did, vid;
  UINTN         Segment;
  UINTN         Bus;
  UINTN         Device;
  UINTN         Function;
  CHAR8         str[128];

  DbgHeader("GetCPUProperties");
  
  //initial values
  gCPUStructure.MaxRatio = 10; //keep it as K*10
  gCPUStructure.MinRatio = 10; //same
  gCPUStructure.SubDivider = 0;
  gSettings.CpuFreqMHz = 0;
  gCPUStructure.FSBFrequency = MultU64x32(gCPUStructure.ExternalClock, kilo); //kHz -> Hz
  gCPUStructure.ProcessorInterconnectSpeed = 0;
  gCPUStructure.Mobile = FALSE; //not same as gMobile
  
  if (!gCPUStructure.CurrentSpeed) {
    gCPUStructure.CurrentSpeed = (UINT32)DivU64x32(gCPUStructure.TSCCalibr + (Mega >> 1), Mega);
  }
  if (!gCPUStructure.MaxSpeed) {
    gCPUStructure.MaxSpeed = gCPUStructure.CurrentSpeed;
  }
  
  /* get CPUID Values */
  DoCpuid(0, gCPUStructure.CPUID[CPUID_0]);
  gCPUStructure.Vendor  = gCPUStructure.CPUID[CPUID_0][EBX];
  /*
   * Get processor signature and decode
   * and bracket this with the approved procedure for reading the
   * the microcode version number a.k.a. signature a.k.a. BIOS ID
   */
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
    AsmWriteMsr64(MSR_IA32_BIOS_SIGN_ID, 0);
  }
  DoCpuid(1, gCPUStructure.CPUID[CPUID_1]);
  gCPUStructure.Signature = gCPUStructure.CPUID[CPUID_1][EAX];
  DBG("CPU Vendor = %X Model=%X\n", gCPUStructure.Vendor, gCPUStructure.Signature);
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
    msr = AsmReadMsr64(MSR_IA32_BIOS_SIGN_ID);
    gCPUStructure.MicroCode = RShiftU64(msr, 32);
    /* Get "processor flag"; necessary for microcode update matching */
    gCPUStructure.ProcessorFlag = (RShiftU64(AsmReadMsr64(MSR_IA32_PLATFORM_ID), 50)) & 3;
  }
  
  //  DoCpuid(2, gCPUStructure.CPUID[2]);
  
  DoCpuid(0x80000000, gCPUStructure.CPUID[CPUID_80]);
  if((gCPUStructure.CPUID[CPUID_80][EAX] & 0x0000000f) >= 1){
    DoCpuid(0x80000001, gCPUStructure.CPUID[CPUID_81]);
  }
  
  gCPUStructure.Stepping  = (UINT8) bitfield(gCPUStructure.Signature, 3, 0);
  gCPUStructure.Model     = (UINT8) bitfield(gCPUStructure.Signature, 7, 4);
  gCPUStructure.Family    = (UINT8) bitfield(gCPUStructure.Signature, 11, 8);
  gCPUStructure.Type      = (UINT8) bitfield(gCPUStructure.Signature, 13, 12);
  gCPUStructure.Extmodel  = (UINT8) bitfield(gCPUStructure.Signature, 19, 16);
  gCPUStructure.Extfamily = (UINT8) bitfield(gCPUStructure.Signature, 27, 20);
  gCPUStructure.Features  = quad(gCPUStructure.CPUID[CPUID_1][ECX], gCPUStructure.CPUID[CPUID_1][EDX]);
  gCPUStructure.ExtFeatures  = quad(gCPUStructure.CPUID[CPUID_81][ECX], gCPUStructure.CPUID[CPUID_81][EDX]);

  DBG(" The CPU%s supported SSE4.1\n", (gCPUStructure.Features & CPUID_FEATURE_SSE4_1)?"":" not");
  /* Pack CPU Family and Model */
  if (gCPUStructure.Family == 0x0f) {
    gCPUStructure.Family += gCPUStructure.Extfamily;
  }
  gCPUStructure.Model += (gCPUStructure.Extmodel << 4);
  
  /* get BrandString (if supported) */
  if (gCPUStructure.CPUID[CPUID_80][EAX] >= 0x80000004) {
    CHAR8         *s;
    ZeroMem(str, 128);
    /*
     * The BrandString 48 bytes (max), guaranteed to
     * be NULL terminated.
     */
    DoCpuid(0x80000002, reg);
    CopyMem(&str[0], (CHAR8 *)reg,  16);
    DoCpuid(0x80000003, reg);
    CopyMem(&str[16], (CHAR8 *)reg,  16);
    DoCpuid(0x80000004, reg);
    CopyMem(&str[32], (CHAR8 *)reg,  16);
    for (s = str; *s != '\0'; s++){
      if (*s != ' ') break; //remove leading spaces
    }
    AsciiStrnCpyS(gCPUStructure.BrandString, 48, s, 48);
    
    if (!AsciiStrnCmp((const CHAR8*)gCPUStructure.BrandString, (const CHAR8*)CPU_STRING_UNKNOWN, iStrLen((gCPUStructure.BrandString) + 1, 48)))
    {
      gCPUStructure.BrandString[0] = '\0';
    }
    gCPUStructure.BrandString[47] = '\0';
    DBG("BrandString = %s\n", gCPUStructure.BrandString);
  }
  
  //Calculate Nr of Cores
  if (gCPUStructure.Features & CPUID_FEATURE_HTT) {
    gCPUStructure.LogicalPerPackage  = (UINT32)bitfield(gCPUStructure.CPUID[CPUID_1][EBX], 23, 16); //Atom330 = 4
  } else {
    gCPUStructure.LogicalPerPackage  = 1;
  }
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
    DoCpuid(4, gCPUStructure.CPUID[CPUID_4]);
    if (gCPUStructure.CPUID[CPUID_4][EAX]) {
      gCPUStructure.CoresPerPackage =  (UINT32)bitfield(gCPUStructure.CPUID[CPUID_4][EAX], 31, 26) + 1; //Atom330 = 2
      DBG("CPUID_4_eax=%X\n", gCPUStructure.CPUID[CPUID_4][EAX]);
      DoCpuid(4, gCPUStructure.CPUID[CPUID_4]);
      DBG("CPUID_4_eax=%X\n", gCPUStructure.CPUID[CPUID_4][EAX]);
      DoCpuid(4, gCPUStructure.CPUID[CPUID_4]);
      DBG("CPUID_4_eax=%X\n", gCPUStructure.CPUID[CPUID_4][EAX]);
    } else {
      gCPUStructure.CoresPerPackage = (UINT32)bitfield(gCPUStructure.CPUID[CPUID_1][EBX], 18, 16);
      if (gCPUStructure.CoresPerPackage) {
        DBG("got cores from CPUID_1 = %d\n", gCPUStructure.CoresPerPackage);
      }
    }
  } else if (gCPUStructure.Vendor == CPU_VENDOR_AMD) {

    post_startup_cpu_fixups();
    if(gCPUStructure.CPUID[CPUID_80][EAX] >= 0x80000008){
      DoCpuid(0x80000008, gCPUStructure.CPUID[CPUID_88]);
    }
    if (gCPUStructure.Extfamily < 0x8) {
      gCPUStructure.CoresPerPackage =  (gCPUStructure.CPUID[CPUID_88][ECX] & 0xFF) + 1;
    } else {
      // Bronya : test for SMT
      INTN Logical = 1;
      if(gCPUStructure.CPUID[CPUID_80][EAX] >= 0x8000001E) {
        DoCpuid(0x8000001E, gCPUStructure.CPUID[CPUID_81E]);
        Logical = (INTN)bitfield(gCPUStructure.CPUID[CPUID_81E][EBX], 15, 8) + 1;
      }
      gCPUStructure.CoresPerPackage =  (UINT32)(((gCPUStructure.CPUID[CPUID_88][ECX] & 0xFF) + 1) / Logical);
    }
    gCPUStructure.Cores = (UINT8)gCPUStructure.CoresPerPackage;
    gCPUStructure.Threads = (UINT8)gCPUStructure.LogicalPerPackage;
    if (gCPUStructure.Cores == 0) {
      gCPUStructure.Cores = 1;
    }

  }
  
  if (gCPUStructure.CoresPerPackage == 0) {
    gCPUStructure.CoresPerPackage = 1;
  }
  
  /* Fold in the Invariant TSC feature bit, if present */
  if(gCPUStructure.CPUID[CPUID_80][EAX] >= 0x80000007){
    DoCpuid(0x80000007, gCPUStructure.CPUID[CPUID_87]);
    gCPUStructure.ExtFeatures |=
    gCPUStructure.CPUID[CPUID_87][EDX] & (UINT32)CPUID_EXTFEATURE_TSCI;
  }
  
  if ((bit(9) & gCPUStructure.CPUID[CPUID_1][ECX]) != 0) {
    SSSE3 = TRUE;
  }
  gCPUStructure.Turbo = FALSE;
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
    // Determine turbo boost support
    DoCpuid(6, gCPUStructure.CPUID[CPUID_6]);
    gCPUStructure.Turbo = ((gCPUStructure.CPUID[CPUID_6][EAX] & (1 << 1)) != 0);
    DBG(" The CPU%s supported turbo\n", gCPUStructure.Turbo?"":" not");
    //get cores and threads
    switch (gCPUStructure.Model)
    {
      case CPU_MODEL_NEHALEM: // Intel Core i7 LGA1366 (45nm)
      case CPU_MODEL_FIELDS: // Intel Core i5, i7 LGA1156 (45nm)
      case CPU_MODEL_CLARKDALE: // Intel Core i3, i5, i7 LGA1156 (32nm)
      case CPU_MODEL_NEHALEM_EX:
      case CPU_MODEL_JAKETOWN:
      case CPU_MODEL_SANDY_BRIDGE:
      case CPU_MODEL_IVY_BRIDGE:
      case CPU_MODEL_IVY_BRIDGE_E5:
      case CPU_MODEL_HASWELL:
      case CPU_MODEL_HASWELL_U5:
      case CPU_MODEL_HASWELL_E:
      case CPU_MODEL_HASWELL_ULT:
      case CPU_MODEL_CRYSTALWELL:
      case CPU_MODEL_BROADWELL_HQ:
      case CPU_MODEL_AIRMONT:
      case CPU_MODEL_AVOTON:
      case CPU_MODEL_SKYLAKE_U:
      case CPU_MODEL_BROADWELL_DE:
      case CPU_MODEL_BROADWELL_E5:
      case CPU_MODEL_KNIGHT:
      case CPU_MODEL_MOOREFIELD:
      case CPU_MODEL_GOLDMONT:
      case CPU_MODEL_ATOM_X3:
      case CPU_MODEL_SKYLAKE_D:
      case CPU_MODEL_SKYLAKE_S:
      case CPU_MODEL_CANNONLAKE:
      case CPU_MODEL_KABYLAKE1:
      case CPU_MODEL_KABYLAKE2:
        msr = AsmReadMsr64(MSR_CORE_THREAD_COUNT);  //0x35
			DBG("MSR 0x35    %16llX\n", msr);
        gCPUStructure.Cores   = (UINT8)bitfield((UINT32)msr, 31, 16);
        gCPUStructure.Threads = (UINT8)bitfield((UINT32)msr, 15,  0);
        break;
        
      case CPU_MODEL_DALES:
      case CPU_MODEL_WESTMERE: // Intel Core i7 LGA1366 (32nm) 6 Core
      case CPU_MODEL_WESTMERE_EX:
        msr = AsmReadMsr64(MSR_CORE_THREAD_COUNT);
        gCPUStructure.Cores   = (UINT8)bitfield((UINT32)msr, 19, 16);
        gCPUStructure.Threads = (UINT8)bitfield((UINT32)msr, 15,  0);
        break;
      case CPU_MODEL_ATOM_3700:
        gCPUStructure.Cores   = 4;
        gCPUStructure.Threads = 4;
        break;
      case CPU_MODEL_ATOM:
        gCPUStructure.Cores   = 2;
        gCPUStructure.Threads = 2;
        break;
        
      default:
        gCPUStructure.Cores = 0;
        break;
    }
  }

  //workaround for Xeon Harpertown and Yorkfield
  if ((gCPUStructure.Model == CPU_MODEL_PENRYN) &&
      (gCPUStructure.Cores == 0)) {
    if ((AsciiStrStr(gCPUStructure.BrandString, "X54")) ||
        (AsciiStrStr(gCPUStructure.BrandString, "E54")) ||
        (AsciiStrStr(gCPUStructure.BrandString, "W35")) ||
        (AsciiStrStr(gCPUStructure.BrandString, "X34")) ||
        (AsciiStrStr(gCPUStructure.BrandString, "X33")) ||
        (AsciiStrStr(gCPUStructure.BrandString, "L33")) ||
        (AsciiStrStr(gCPUStructure.BrandString, "X32")) ||
        (AsciiStrStr(gCPUStructure.BrandString, "L3426")) ||
        (AsciiStrStr(gCPUStructure.BrandString, "L54"))) {
      gCPUStructure.Cores   = 4;
      gCPUStructure.Threads = 4;
    } else if (AsciiStrStr(gCPUStructure.BrandString, "W36")) {
      gCPUStructure.Cores   = 6;
      gCPUStructure.Threads = 6;
    } else { //other Penryn and Wolfdale
      gCPUStructure.Cores   = 0;
      gCPUStructure.Threads = 0;
    }
  }
  
  if (gCPUStructure.Cores == 0) {
    gCPUStructure.Cores   = (UINT8)(gCPUStructure.CoresPerPackage & 0xff);
    gCPUStructure.Threads = (UINT8)(gCPUStructure.LogicalPerPackage & 0xff);
    if (gCPUStructure.Cores > gCPUStructure.Threads) {
      gCPUStructure.Threads = gCPUStructure.Cores;
    }
  }
  
  //workaround for N270. I don't know why it detected wrong
  if ((gCPUStructure.Model == CPU_MODEL_ATOM) &&
      (AsciiStrStr(gCPUStructure.BrandString, "270"))) {
    gCPUStructure.Cores   = 1;
    gCPUStructure.Threads = 2;
  }
  
  //workaround for Quad
  if (AsciiStrStr(gCPUStructure.BrandString, "Quad")) {
    gCPUStructure.Cores   = 4;
    gCPUStructure.Threads = 4;
  }
  
  //New for SkyLake 0x4E, 0x5E
  if(gCPUStructure.CPUID[CPUID_0][EAX] >= 0x15) {
    UINT32 Num, Denom;
    DoCpuid(0x15, gCPUStructure.CPUID[CPUID_15]);
    Num = gCPUStructure.CPUID[CPUID_15][EBX];
    Denom = gCPUStructure.CPUID[CPUID_15][EAX];
    DBG(" TSC/CCC Information Leaf:\n");
    DBG("  numerator     : %d\n", Num);
    DBG("  denominator   : %d\n", Denom);
    if (Num && Denom) {
      gCPUStructure.ARTFrequency = DivU64x32(MultU64x32(gCPUStructure.TSCCalibr, Denom), Num);
      DBG(" Calibrated ARTFrequency: %lld\n", gCPUStructure.ARTFrequency);
      UINT64 Stokg = DivU64x32(gCPUStructure.ARTFrequency + 49999, 100000);
      gCPUStructure.ARTFrequency = MultU64x32(Stokg, 100000);
      DBG(" Rounded ARTFrequency: %lld\n", gCPUStructure.ARTFrequency);
    }
  }

  //get Min and Max Ratio Cpu/Bus
  /*  if (QEMU) {
   
   0x06170C2D06000C2DULL
   } */
  
  if(gCPUStructure.Vendor == CPU_VENDOR_INTEL &&
     ((gCPUStructure.Family == 0x06 && gCPUStructure.Model >= 0x0c) ||
      (gCPUStructure.Family == 0x0f && gCPUStructure.Model >= 0x03))) {
       if (gCPUStructure.Family == 0x06) {
         //        DBG("Get min and max ratio\n");
         switch (gCPUStructure.Model) {
           case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
           case CPU_MODEL_FIELDS:// Core i7, i5 LGA1156, "Clarksfield", "Lynnfield", "Jasper", 45nm
           case CPU_MODEL_DALES:// Core i7, i5, Nehalem
           case CPU_MODEL_CLARKDALE:// Core i7, i5, i3 LGA1156, "Westmere", "Clarkdale", , 32nm
           case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
           case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
           case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
             //since rev 553 bcc9 patch
             gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.CurrentSpeed, Mega); //MHz -> Hz
             gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
             msr = AsmReadMsr64(MSR_FLEX_RATIO);
             if ((RShiftU64(msr, 16) & 0x01) != 0) {
               UINT8 flex_ratio = RShiftU64(msr, 8) & 0xff;
				 MsgLog("non-usable FLEX_RATIO = %llX\n", msr);
               if (flex_ratio == 0) {
                 AsmWriteMsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL));
                 gBS->Stall(10);
                 msr = AsmReadMsr64(MSR_FLEX_RATIO);
				   MsgLog("corrected FLEX_RATIO = %llX\n", msr);
               }
             }
             //
             msr = AsmReadMsr64(MSR_PLATFORM_INFO);     //0xCE
             gCPUStructure.MinRatio = (UINT8)RShiftU64(msr, 40) & 0xff;
             // msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);
             gCPUStructure.MaxRatio = (UINT8)(RShiftU64(msr, 8) & 0xff);
             TurboMsr = msr + 1;
             
             if(gCPUStructure.MaxRatio) {
               gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.MaxRatio);
             } else {
               gCPUStructure.FSBFrequency = 133333333ULL; // 133 MHz
             }
             
             // This makes no sense and seems arbitrary - apianti
             if (gCPUStructure.Turbo) {
               msr = AsmReadMsr64(MSR_TURBO_RATIO_LIMIT);
               
               gCPUStructure.Turbo1 = (UINT8)(RShiftU64(msr, 0) & 0xff);
               gCPUStructure.Turbo2 = (UINT8)(RShiftU64(msr, 8) & 0xff);
               gCPUStructure.Turbo3 = (UINT8)(RShiftU64(msr, 16) & 0xff);
               gCPUStructure.Turbo4 = (UINT8)(RShiftU64(msr, 24) & 0xff); //later
             }
             
             gCPUStructure.MaxRatio *= 10;
             gCPUStructure.MinRatio *= 10;
             gCPUStructure.Turbo1 *= 10;
             gCPUStructure.Turbo2 *= 10;
             gCPUStructure.Turbo3 *= 10;
             gCPUStructure.Turbo4 *= 10;
             
             break;
           case CPU_MODEL_SANDY_BRIDGE:// Sandy Bridge, 32nm
           case CPU_MODEL_IVY_BRIDGE:
           case CPU_MODEL_IVY_BRIDGE_E5:
           case CPU_MODEL_JAKETOWN:
           case CPU_MODEL_ATOM_3700:
           case CPU_MODEL_HASWELL:
           case CPU_MODEL_HASWELL_U5:
           case CPU_MODEL_HASWELL_E:
           case CPU_MODEL_HASWELL_ULT:
           case CPU_MODEL_CRYSTALWELL:
           case CPU_MODEL_BROADWELL_HQ:
           case CPU_MODEL_BROADWELL_E5:
           case CPU_MODEL_BROADWELL_DE:
           case CPU_MODEL_AIRMONT:
           case CPU_MODEL_SKYLAKE_U:
           case CPU_MODEL_SKYLAKE_D:
           case CPU_MODEL_SKYLAKE_S:
           case CPU_MODEL_GOLDMONT:
           case CPU_MODEL_KABYLAKE1:
           case CPU_MODEL_KABYLAKE2:
           case CPU_MODEL_CANNONLAKE:
             gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.CurrentSpeed, Mega); //MHz -> Hz
             gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
             
             
             //----test C3 patch
             msr = AsmReadMsr64(MSR_PKG_CST_CONFIG_CONTROL); //0xE2
				 MsgLog("MSR 0xE2 before patch %08llX\n", msr);
             if (msr & 0x8000) {
               MsgLog("MSR 0xE2 is locked, PM patches will be turned on\n");
               NeedPMfix = TRUE;
             }
             //   AsmWriteMsr64(MSR_PKG_CST_CONFIG_CONTROL, (msr & 0x8000000ULL));
             //   msr = AsmReadMsr64(MSR_PKG_CST_CONFIG_CONTROL);
             //   MsgLog("MSR 0xE2 after  patch %08X\n", msr);
             //   msr = AsmReadMsr64(MSR_PMG_IO_CAPTURE_BASE);
             //   MsgLog("MSR 0xE4              %08X\n", msr);
             //------------
             msr = AsmReadMsr64(MSR_PLATFORM_INFO);       //0xCE
				 MsgLog("MSR 0xCE              %08llX_%08llX\n", (msr>>32), msr & 0xFFFFFFFFull);
             gCPUStructure.MaxRatio = (UINT8)RShiftU64(msr, 8) & 0xff;
             gCPUStructure.MinRatio = (UINT8)MultU64x32(RShiftU64(msr, 40) & 0xff, 10);
             //--- Check if EIST locked
             msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE); //0x1A0
             if (msr & _Bit(20)) {
				 MsgLog("MSR 0x1A0             %08llX\n", msr);
               MsgLog("   EIST is locked and %s\n", (msr & _Bit(16))?"enabled":"disabled");
             }
             
             if (gCPUStructure.Model != CPU_MODEL_GOLDMONT && gCPUStructure.Model != CPU_MODEL_AIRMONT &&
                 gCPUStructure.Model != CPU_MODEL_AVOTON) {
               msr = AsmReadMsr64(MSR_FLEX_RATIO);   //0x194
               if ((RShiftU64(msr, 16) & 0x01) != 0) {
                 // bcc9 patch
                 UINT8 flex_ratio = RShiftU64(msr, 8) & 0xff;
                 // MsgLog("non-usable FLEX_RATIO = %X\n", msr);
                 if (flex_ratio == 0) {
                   AsmWriteMsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL));
                   gBS->Stall(10);
                   msr = AsmReadMsr64(MSR_FLEX_RATIO);
					 MsgLog("corrected FLEX_RATIO = %llX\n", msr);
                 }
               }
             }
             if ((gCPUStructure.CPUID[CPUID_6][ECX] & (1 << 3)) != 0) {
               msr = AsmReadMsr64(IA32_ENERGY_PERF_BIAS); //0x1B0
				 MsgLog("MSR 0x1B0             %08llX\n", msr);
             }
             
             if(gCPUStructure.MaxRatio) {
               gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.MaxRatio);
             } else {
               gCPUStructure.FSBFrequency = 100000000ULL; //100*Mega
             }
             
             msr = AsmReadMsr64(MSR_TURBO_RATIO_LIMIT);   //0x1AD
             gCPUStructure.Turbo1 = (UINT8)(RShiftU64(msr, 0) & 0xff);
             gCPUStructure.Turbo2 = (UINT8)(RShiftU64(msr, 8) & 0xff);
             gCPUStructure.Turbo3 = (UINT8)(RShiftU64(msr, 16) & 0xff);
             gCPUStructure.Turbo4 = (UINT8)(RShiftU64(msr, 24) & 0xff);
             
             if (gCPUStructure.Turbo4 == 0) {
               gCPUStructure.Turbo4 = gCPUStructure.Turbo1;
               if (gCPUStructure.Turbo4 == 0) {
                 gCPUStructure.Turbo4 = (UINT16)gCPUStructure.MaxRatio;
               }
             }
             
             //Slice - we found that for some i5-2400 and i7-2600 MSR 1AD reports wrong turbo mult
             // another similar bug in i7-3820
             //MSR 000001AD  0000-0000-3B3B-3B3B - from AIDA64
             // so there is a workaround
             if ((gCPUStructure.Turbo4 == 0x3B) || (gCPUStructure.Turbo4 == 0x39)) {
               gCPUStructure.Turbo4 = (UINT16)gCPUStructure.MaxRatio + (gCPUStructure.Turbo?1:0);
               //this correspond to 2nd-gen-core-desktop-specification-update.pdf
             }
             
             gCPUStructure.MaxRatio *= 10;
             gCPUStructure.Turbo1 *= 10;
             gCPUStructure.Turbo2 *= 10;
             gCPUStructure.Turbo3 *= 10;
             gCPUStructure.Turbo4 *= 10;
             break;
           case CPU_MODEL_PENTIUM_M:
           case CPU_MODEL_ATOM://  Atom
           case CPU_MODEL_DOTHAN:// Pentium M, Dothan, 90nm
           case CPU_MODEL_YONAH:// Core Duo/Solo, Pentium M DC
           case CPU_MODEL_MEROM:// Core Xeon, Core 2 Duo, 65nm, Mobile
             //case CPU_MODEL_CONROE:// Core Xeon, Core 2 Duo, 65nm, Desktop like Merom but not mobile
           case CPU_MODEL_CELERON:
           case CPU_MODEL_PENRYN:// Core 2 Duo/Extreme, Xeon, 45nm , Mobile
             //case CPU_MODEL_WOLFDALE:// Core 2 Duo/Extreme, Xeon, 45nm, Desktop like Penryn but not Mobile
             if(AsmReadMsr64(MSR_IA32_PLATFORM_ID) & (1 << 28)){
               gCPUStructure.Mobile = TRUE;
             }
             gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.MaxSpeed, Mega); //MHz -> Hz
             gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
             msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);
             gCPUStructure.MaxRatio = (UINT32)(RShiftU64(msr, 8)) & 0x1f;
             TurboMsr = (UINT32)(RShiftU64(msr, 40)) & 0x1f;
             if ((TurboMsr > gCPUStructure.MaxRatio) && (gCPUStructure.Model == CPU_MODEL_MEROM)) {
				 DBG(" CPU works at low speed, MaxRatio=%llu CurrRatio=%d\n", TurboMsr,
                   gCPUStructure.MaxRatio);
               gCPUStructure.MaxRatio = (UINT32)TurboMsr;
             }
             gCPUStructure.SubDivider = (UINT32)(RShiftU64(msr, 46)) & 0x1;
             gCPUStructure.MinRatio = 60;
             if(!gCPUStructure.MaxRatio) gCPUStructure.MaxRatio = 6; // :(
             msr = AsmReadMsr64(MSR_FSB_FREQ); //0xCD
             gCPUStructure.FSBFrequency = DivU64x32(LShiftU64(gCPUStructure.TSCFrequency, 1),
                                                    gCPUStructure.MaxRatio * 2 + gCPUStructure.SubDivider);
             if ((msr & 3) == 2 && (gCPUStructure.FSBFrequency < 196 * Mega)) {
               DBG("wrong MaxRatio = %d.%d, corrected\n", gCPUStructure.MaxRatio, gCPUStructure.SubDivider * 5);
               gCPUStructure.MaxRatio = (UINT32)DivU64x32(gCPUStructure.TSCFrequency, 200 * Mega);
             }
             gCPUStructure.MaxRatio = gCPUStructure.MaxRatio * 10 + gCPUStructure.SubDivider * 5;
             if (AsciiStrStr(gCPUStructure.BrandString, "P8400")) {
               gCPUStructure.MaxRatio = 85;
               gCPUStructure.FSBFrequency = DivU64x32(MultU64x32(gCPUStructure.TSCFrequency, 10), gCPUStructure.MaxRatio);
               DBG("workaround for Code2Duo P8400, MaxRatio=8.5\n");
             }
             if (TurboMsr == 6) {
               TurboMsr = AsmReadMsr64(MSR_PLATFORM_INFO); //0xCE
               gCPUStructure.Turbo4 = ((UINT32)(RShiftU64(TurboMsr, 8)) & 0x1f) * 10; //workaround for Harpertown
             } else {
               gCPUStructure.Turbo4 = (UINT16)(gCPUStructure.MaxRatio + 10);
             }
             DBG("MSR dumps:\n");
				 DBG("\t@0x00CD=%llx\n", msr);
				 DBG("\t@0x0198=%llx\n", AsmReadMsr64(MSR_IA32_PERF_STATUS));
             break;
           default:
             gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.CurrentSpeed, Mega); //MHz -> Hz
             gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
             gCPUStructure.MinRatio = 60;
             if (!gCPUStructure.FSBFrequency) {
               gCPUStructure.FSBFrequency = 100000000ULL; //100*Mega
             }
             gCPUStructure.MaxRatio = (UINT32)(MultU64x32(DivU64x64Remainder(gCPUStructure.TSCFrequency, gCPUStructure.FSBFrequency, NULL), 10));
             gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
             break;
         }
       }
       else //Family !=6 i.e. Pentium 4
       {
         gCPUStructure.TSCFrequency = MultU64x32((gCPUStructure.Model >= 3) ? gCPUStructure.MaxSpeed : gCPUStructure.CurrentSpeed, Mega); //MHz -> Hz
         gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
         msr = AsmReadMsr64(MSR_IA32_PLATFORM_ID);
         TurboMsr = 0;
         if (!gCPUStructure.FSBFrequency) {
           gCPUStructure.FSBFrequency = 100000000ULL; //100*Mega
         }
         if ((RShiftU64(msr, 31) & 0x01) != 0) {
           gCPUStructure.MaxRatio = (UINT8)MultU64x32((RShiftU64(msr, 8) & 0x1f), 10);
           gCPUStructure.MinRatio = gCPUStructure.MaxRatio; //no speedstep
         } else {
           gCPUStructure.MaxRatio = (UINT32)DivU64x64Remainder(gCPUStructure.TSCFrequency, gCPUStructure.FSBFrequency, 0);
         }
         gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
       }
     }
  
  else if(gCPUStructure.Vendor == CPU_VENDOR_AMD ) {
    
    UINT32 cpudid_zen = 0x17;
    INTN  currcoef = 0;
    INTN  cpuMultN2 = 0;
    INTN  currdiv = 0;
    UINT64	busFCvtt2n;
    UINT64	tscFCvtt2n;
    UINT64	busFCvtn2t = 0;
    UINT64	busFrequency		= 0;
    UINT64	cpuFrequency		= 0;


    
    gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.CurrentSpeed, Mega); //MHz -> Hz
	  DBG("CurrentSpeed: %llu\n", DivU64x32(gCPUStructure.TSCFrequency, Mega));
    
    switch (gCPUStructure.Family)
    {
        
        //if(gCPUStructure.Extfamily == 0x00 /* K8 */)
        /*case 0xf:
         {
         msr = AsmReadMsr64(K8_FIDVID_STATUS);
         gCPUStructure.MaxRatio = (UINT32)(RShiftU64((RShiftU64(msr, 16) & 0x3f), 2) + 4);
         gCPUStructure.MinRatio = (UINT32)(RShiftU64((RShiftU64(msr, 8) & 0x3f), 2) + 4);
         } break;
         */
        //if(gCPUStructure.Family >= 0x01 /* K10+ */) {
        
      case 0xF: //// AMD Family 8h ////
      {
        UINT64 fidvid = 0;
        UINT64 cpuMult;
        UINT64 fid;
        
        fidvid = AsmReadMsr64(K8_FIDVID_STATUS);
        fid = bitfield(fidvid, 5, 0);
        
        cpuMult = (fid + 8);// / 2;
        
        gCPUStructure.MinRatio = (UINT32)(RShiftU64((RShiftU64(fidvid, 8) & 0x3f), 2) + 4);
        currcoef = (INTN)cpuMult;
        gCPUStructure.MaxRatio = (UINT32)cpuMult;
        
        cpuMultN2 = (fidvid & (UINT64)bit(0));
        currdiv = cpuMultN2;
        
        gCPUStructure.MaxRatio =  (gCPUStructure.MaxRatio * 5) ;
        /////// Addon END ///////
      }
        break;
        
      case 0x10: //// AMD Family 10h ////
      {
        
        UINT64 msr_min, msr_max;
        UINT64 cpuMult;
        UINT64 CpuDid;
        UINT64 CpuFid;
        // UINT64 divisor = 0;
        
        
        msr_min = AsmReadMsr64(K10_COFVID_LIMIT);
        cpudid_zen = (UINT32)bitfield(msr_min, 6 , 4);
        msr_min = AsmReadMsr64(K10_PSTATE_STATUS + cpudid_zen);
        gCPUStructure.MinRatio = 5 * (UINT32)DivU64(((msr_min & 0x3f) + 0x08), LShiftU64(1ULL, ((RShiftU64(msr_min, 6) & 0x7))));
        
        msr_max = AsmReadMsr64(K10_PSTATE_STATUS);
        
        CpuDid = bitfield(msr_max, 8, 6);
        CpuFid = bitfield(msr_max, 5, 0);
        
        /* if (CpuDid == 0) divisor = 2;
         else if (CpuDid == 1) divisor = 4;
         else if (CpuDid == 2) divisor = 8;
         else if (CpuDid == 3) divisor = 16;
         else if (CpuDid == 4) divisor = 32;
         gCPUStructure.CpuDid = divisor;
         */
        cpuMult = DivU64((CpuFid + 0x10), LShiftU64(1ULL, (UINTN)CpuDid));
        currcoef = (INTN)cpuMult;
        gCPUStructure.MaxRatio = (UINT32)cpuMult;
        
        cpuMultN2 = (msr_max & (UINT64)bit(0));
        currdiv = cpuMultN2;

        
        /////// Addon END ///////
      }
        break;
        
      case 0x11: //// AMD Family 11h ////
      {
        UINT64 cofvid = 0;
        UINT64 cpuMult;
        // UINT64 divisor = 0;
        UINT64 CpuDid;
        UINT64 CpuFid;
        UINT64 msr_min = 0;
        
        msr_min = AsmReadMsr64(K10_COFVID_LIMIT);
        msr_min = AsmReadMsr64(K10_PSTATE_STATUS + (RShiftU64(msr_min, 4) & 0x07));
        
        gCPUStructure.MinRatio = 5 * (UINT32)DivU64(((msr_min & 0x3f) + 0x08), LShiftU64(1ULL, ((RShiftU64(msr_min, 6) & 0x7))));
        
        cofvid  = AsmReadMsr64(K10_PSTATE_STATUS);
        CpuDid = bitfield(cofvid, 8, 6);
        CpuFid = bitfield(cofvid, 5, 0);
        //if (CpuDid == 0) divisor = 2;
        //else if (CpuDid == 1) divisor = 4;
        //else if (CpuDid == 2) divisor = 8;
        //else if (CpuDid == 3) divisor = 16;
        //else if (did == 4) divisor = 32;
        
        cpuMult = DivU64((CpuFid + 0x8), LShiftU64(1ULL, (UINTN)CpuDid));
        
        currcoef = (INTN)cpuMult;
        gCPUStructure.MaxRatio = (UINT32)cpuMult;
        
        cpuMultN2 = (cofvid & (UINT64)bit(0));
        currdiv = cpuMultN2;
        /////// Addon END ///////
      }
        break;
        
      case 0x12: //// AMD Family 12h ////
      {
        // 8:4 CpuFid: current CPU core frequency ID
        // 3:0 CpuDid: current CPU core divisor ID
        UINT64 prfsts,CpuFid,CpuDid;
        
        UINT64 msr_min = 0;
        
        msr_min = AsmReadMsr64(K10_COFVID_LIMIT);
        msr_min = AsmReadMsr64(K10_PSTATE_STATUS + (RShiftU64(msr_min, 4) & 0x07));
        
        gCPUStructure.MinRatio = 5 * (UINT32)DivU64(((msr_min & 0x3f) + 0x08), LShiftU64(1ULL, ((RShiftU64(msr_min, 0) & 0x7))));
        
        prfsts = AsmReadMsr64(K10_PSTATE_STATUS);
        
        CpuDid = bitfield(prfsts, 3, 0) ;
        CpuFid = bitfield(prfsts, 8, 4) ;
        //uint64_t divisor;
        /*switch (CpuDid)
         {
         case 0: divisor = 1; break;
         case 1: divisor = (3/2); break;
         case 2: divisor = 2; break;
         case 3: divisor = 3; break;
         case 4: divisor = 4; break;
         case 5: divisor = 6; break;
         case 6: divisor = 8; break;
         case 7: divisor = 12; break;
         case 8: divisor = 16; break;
         default: divisor = 1; break;
         }*/
        
        currcoef = (INTN)DivU64((CpuFid + 0x10), LShiftU64(1ULL, (UINTN)CpuDid));
        gCPUStructure.MaxRatio = (UINT32)currcoef;
        
        cpuMultN2 = (prfsts & (UINT64)bit(0));
        currdiv = cpuMultN2;
        
      }
        break;
        
      case 0x14: //// AMD Family 14h ////
        
      {
        // 8:4: current CPU core divisor ID most significant digit
        // 3:0: current CPU core divisor ID least significant digit
        UINT64 prfsts;
        UINT64 msr_min = 0;
        
        msr_min = AsmReadMsr64(K10_COFVID_LIMIT);
        msr_min = AsmReadMsr64(K10_PSTATE_STATUS + (RShiftU64(msr_min, 4) & 0x07));
        
        gCPUStructure.MinRatio = 5 * (UINT32)DivU64(((msr_min & 0x3f) + 0x08), LShiftU64(1ULL, ((RShiftU64(msr_min, 0) & 0x7))));
        
        prfsts = AsmReadMsr64(K10_PSTATE_STATUS);
        
        UINT64 CpuDidMSD,CpuDidLSD;
        CpuDidMSD = bitfield(prfsts, 8, 4) ;
        CpuDidLSD  = bitfield(prfsts, 3, 0) ;
        
        UINT64 frequencyId = DivU64x32(gCPUStructure.CPUFrequency, Mega);
        
        //Bronya :  i think that this fixed, need test this ...
        currcoef = (INTN)DivU64((DivU64x32((frequencyId + 5), 100) + 0x10), (UINT64)(CpuDidMSD + DivU64x32(CpuDidLSD, 4) + 1));
        gCPUStructure.MaxRatio = (UINT32)currcoef;
        
        currdiv = (INTN)(((CpuDidMSD) + 1) << 2);
        currdiv += (INTN)bitfield(prfsts, 3, 0);
        
        cpuMultN2 = currdiv;//(prfsts & (UINT64)bit(0));
        //currdiv = cpuMultN2;
      }
        
        break;
        
      case 0x15: //// AMD Family 15h ////
      case 0x06: //// AMD Family 06h ////
      {
        
        UINT64 cofvid = 0;
        UINT64 cpuMult;
        //UINT64 divisor = 0;
        UINT64 CpuDid;
        UINT64 CpuFid;
        
        UINT64 msr_min = 0;
        
        msr_min = AsmReadMsr64(K10_COFVID_LIMIT);
        msr_min = AsmReadMsr64(K10_PSTATE_STATUS + (RShiftU64(msr_min, 4) & 0x07));
        
        gCPUStructure.MinRatio = 5 * (UINT32)DivU64(((msr_min & 0x3f) + 0x08), LShiftU64(1ULL, ((RShiftU64(msr_min, 6) & 0x7))));
        
        cofvid  = AsmReadMsr64(K10_PSTATE_STATUS);
        CpuDid = bitfield(cofvid, 8, 6);
        CpuFid = bitfield(cofvid, 5, 0);
        
        /* if (CpuDid == 0) divisor = 2;
         else if (CpuDid == 1) divisor = 4;
         else if (CpuDid == 2) divisor = 8;
         else if (CpuDid == 3) divisor = 16;
         else if (CpuDid == 4) divisor = 32;
         */
        cpuMult = DivU64((CpuFid + 0x10), LShiftU64(1ULL, (UINTN)CpuDid));
        
        currcoef = (INTN)cpuMult;
        gCPUStructure.MaxRatio = (UINT32)cpuMult;
        //printf("cpuMult %d\n",currcoef);
        
        cpuMultN2 = (cofvid & (UINT64)bit(0));
        currdiv = cpuMultN2;
      }
        break;
        
      case 0x16: //// AMD Family 16h kabini ////
      {
        UINT64 cofvid = 0;
        UINT64 cpuMult;
        //UINT64 divisor = 0;
        UINT64 CpuDid;
        UINT64 CpuFid;
        UINT64 msr_min = 0;
        
        msr_min = AsmReadMsr64(K10_COFVID_LIMIT);
        msr_min = AsmReadMsr64(K10_PSTATE_STATUS + (RShiftU64(msr_min, 4) & 0x07));
        
        gCPUStructure.MinRatio = 5 * (UINT32)DivU64(((msr_min & 0x3f) + 0x08), LShiftU64(1ULL, ((RShiftU64(msr_min, 6) & 0x7))));
        
        
        cofvid  = AsmReadMsr64(K10_PSTATE_STATUS);
        CpuDid = bitfield(cofvid, 8, 6);
        CpuFid = bitfield(cofvid, 5, 0);
        /* if (CpuDid == 0) divisor = 2;
         else if (CpuDid == 1) divisor = 4;
         else if (CpuDid == 2) divisor = 8;
         else if (CpuDid == 3) divisor = 16;
         else if (CpuDid == 4) divisor = 32;
         */
        cpuMult = DivU64((CpuFid + 0x10), LShiftU64(1ULL, (UINTN)CpuDid));
        
        currcoef = (INTN)cpuMult;
        gCPUStructure.MaxRatio = (UINT32)cpuMult;
        
        cpuMultN2 = (cofvid & (UINT64)bit(0));
        currdiv = cpuMultN2;
        
      }
        break;
        
      case 0x17: //Bronya: For AMD Family 17h Ryzen ! //
      {
        // CoreCOF = (Core::X86::Msr::PStateDef[CpuFid[7:0]]/Core::X86::Msr::PStateDef[CpuDfsId])*200
        
        //gCPUStructure.MaxRatio = gCPUStructure.TSCFrequency  / (100 * Mega);//Mhz ;
        
        UINT64 cofvid = 0 , msr_min = 0;
        UINT64 cpuMult;
        UINT64 CpuDfsId;
        UINT64 CpuFid;
        
        msr_min = AsmReadMsr64(K10_COFVID_LIMIT);
        msr_min = AsmReadMsr64(K10_PSTATE_STATUS + (RShiftU64(msr_min, 4) & 0x7));
        gCPUStructure.MinRatio = ((UINT32)DivU64x32(((msr_min & 0xFF)), (RShiftU64(msr_min, 8) & 0x3f)))*20;
        
        cofvid = AsmReadMsr64(K10_PSTATE_STATUS);
        CpuDfsId =  bitfield(cofvid, 13, 8);
        CpuFid = bitfield(cofvid, 7, 0);
        
        cpuMult = DivU64(CpuFid, CpuDfsId) * 2 * 2 ; //Bronya: This add * 2 <- Interested ))
        //cpuMult = (UINT32)DivU64x32(((cofvid & 0xFF)), (RShiftU64(cofvid, 8) & 0x3f))*2;
        currcoef = (INTN)cpuMult;
        gCPUStructure.MaxRatio = (UINT32)cpuMult;
        
        cpuMultN2 = (cofvid & (UINT64)bit(0));
        currdiv = cpuMultN2;
        cpudid_zen = (UINT32)(RShiftU64(cofvid, 8) & 0xff); //for mult
        
        /////// Addon END ///////
      }
        break;
        
      default:
      {
        gCPUStructure.MaxRatio = (UINT32)DivU64x32(gCPUStructure.TSCFrequency, (200 * Mega));//hz / (200 * Mega);
        currcoef = gCPUStructure.MaxRatio;
      }
    }
    
    if (currcoef) {
      if (currdiv) {
        
        busFrequency = DivU64((gCPUStructure.TSCFrequency * 2), ((currcoef * 2) + 1));
        busFCvtt2n = DivU64(((1 * Giga) << 32), busFrequency);
        busFCvtn2t = DivU64(0xFFFFFFFFFFFFFFFFULL, busFCvtt2n);
        tscFCvtt2n = DivU64(busFCvtt2n * 2, (1 + (2 * currcoef)));
        cpuFrequency = DivU64(((1 * Giga)  << 32), tscFCvtt2n);
        
        gCPUStructure.FSBFrequency = busFrequency ;
        gCPUStructure.CPUFrequency = cpuFrequency ;
        
        
        //gCPUStructure.MaxRatio = cpuFrequency / busFrequency;
        
        // DBG("maxratio (n/2) %d.%d\n", (gCPUStructure.MaxRatio) / currdiv, (((gCPUStructure.MaxRatio) % currdiv) * 100) / currdiv);
        DBG("cpudid_zen(n/2) %d\n", cpudid_zen);
        
        // DBG("busFrequency(N/2): %d \n currcoef(N/2): %hhd \n cpuFrequency(N/2): %lld \n tscFreq(N/2): %lld",(uint32_t)(busFrequency / Mega),currcoef,cpuFrequency /1000,tscFreq/1000);
      } else {
        
        //currcoef = tscFreq / (200 * Mega);//hz / (200 * Mega);
        
        busFrequency = DivU64(gCPUStructure.TSCFrequency, currcoef);
        busFCvtt2n = DivU64(((1 * Giga) << 32), busFrequency);
        busFCvtn2t = DivU64(0xFFFFFFFFFFFFFFFFULL, busFCvtt2n);
        tscFCvtt2n = DivU64(busFCvtt2n, currcoef);
        cpuFrequency = DivU64(((1 * Giga)  << 32), tscFCvtt2n);
        
        gCPUStructure.FSBFrequency = busFrequency;
        gCPUStructure.CPUFrequency = cpuFrequency;
        
        //gCPUStructure.MaxRatio = cpuFrequency / busFrequency;
        DBG("maxratio %d\n", gCPUStructure.MaxRatio);
        DBG("cpudid_zen %d\n", cpudid_zen);
        
        //DBG("busFrequency: %6d MHz \n, cpuMult: %lld \n, cpuFrequency: %lld \n",(uint32_t)(busFrequency / Mega),currcoef, cpuFrequency);
      }
    }
    
    // gCPUStructure.MaxRatio >>= 1;
    if (!gCPUStructure.MaxRatio) {
      gCPUStructure.MaxRatio = 1; //??? to avoid zero division
    }
    //gCPUStructure.FSBFrequency = DivU64x32(LShiftU64(gCPUStructure.TSCFrequency, 1), gCPUStructure.MaxRatio);
    gCPUStructure.MaxRatio =  (gCPUStructure.MaxRatio * 5) ;
    // gCPUStructure.FSBFrequency = DivU64x32(LShiftU64(gCPUStructure.CPUFrequency, 1), gCPUStructure.MaxRatio);

  }

  // ExternalClock and QPI were fixed by Sherlocks
  // Read original ExternalClock
  switch (gCPUStructure.Model)
  {
    case CPU_MODEL_PENTIUM_M:
    case CPU_MODEL_ATOM://  Atom
    case CPU_MODEL_DOTHAN:// Pentium M, Dothan, 90nm
    case CPU_MODEL_YONAH:// Core Duo/Solo, Pentium M DC
    case CPU_MODEL_MEROM:// Core Xeon, Core 2 Duo, 65nm, Mobile
    //case CPU_MODEL_CONROE:// Core Xeon, Core 2 Duo, 65nm, Desktop like Merom but not mobile
    case CPU_MODEL_CELERON:
    case CPU_MODEL_PENRYN:// Core 2 Duo/Extreme, Xeon, 45nm , Mobile
    case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
    case CPU_MODEL_FIELDS:// Core i7, i5 LGA1156, "Clarksfield", "Lynnfield", "Jasper", 45nm
    case CPU_MODEL_DALES:// Core i7, i5, Nehalem
    case CPU_MODEL_CLARKDALE:// Core i7, i5, i3 LGA1156, "Westmere", "Clarkdale", , 32nm
    case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
    case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
    case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
      ExternalClock = gCPUStructure.ExternalClock;
      //DBG("Read original ExternalClock: %d MHz\n", (INT32)(DivU64x32(ExternalClock, kilo)));
      break;
    default:
      ExternalClock = gCPUStructure.ExternalClock;
      //DBG("Read original ExternalClock: %d MHz\n", (INT32)(DivU64x32(ExternalClock, kilo)));

      // for sandy bridge or newer
      // to match ExternalClock 25 MHz like real mac, divide ExternalClock by 4
      gCPUStructure.ExternalClock = (ExternalClock + 3) / 4;
      //DBG("Corrected ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, kilo)));
      break;
  }

  // DBG("take FSB\n");
  tmpU = gCPUStructure.FSBFrequency;
  //  DBG("divide by 1000\n");
  BusSpeed = (UINT32)DivU64x32(tmpU, kilo); //Hz -> kHz
	DBG ("FSBFrequency = %llu MHz, DMI FSBFrequency = %llu MHz, ", DivU64x32 (tmpU + Mega - 1, Mega), DivU64x32 (ExternalClock + 499, kilo));
  //now check if SMBIOS has ExternalClock = 4xBusSpeed
  if ((BusSpeed > 50*kilo) &&
      ((ExternalClock > BusSpeed * 3) || (ExternalClock < 50*kilo))) { //khz
    gCPUStructure.ExternalClock = BusSpeed;
  } else {
    tmpU = MultU64x32(ExternalClock, kilo); //kHz -> Hz
    gCPUStructure.FSBFrequency = tmpU;
  }
  tmpU = gCPUStructure.FSBFrequency;
	DBG("Corrected FSBFrequency = %llu MHz\n", DivU64x32(tmpU, Mega));
  
  if ((gCPUStructure.Vendor == CPU_VENDOR_INTEL) && (gCPUStructure.Model == CPU_MODEL_NEHALEM)) {
    //Slice - for Nehalem we can do more calculation as in Cham
    // but this algo almost always wrong
    //
    // thanks to dgobe for i3/i5/i7 bus speed detection
    // TODO: consider more Nehalem based CPU(?) ex. CPU_MODEL_NEHALEM_EX, CPU_MODEL_WESTMERE, CPU_MODEL_WESTMERE_EX
    // info: https://en.wikipedia.org/wiki/List_of_Intel_Xeon_microprocessors#Nehalem-based_Xeons
    qpimult = 2; //init
    /* Scan PCI BUS For QPI Frequency */
    // get all PciIo handles
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPciIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (Status == EFI_SUCCESS) {
      for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        Status = gBS->HandleProtocol(HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID **) &PciIo);
        if (!EFI_ERROR(Status)) {
          /* Read PCI BUS */
          Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
          if ((Bus & 0x3F) != 0x3F) {
            continue;
          }
          Status = PciIo->Pci.Read (
                                    PciIo,
                                    EfiPciIoWidthUint32,
                                    0,
                                    sizeof (Pci) / sizeof (UINT32),
                                    &Pci
                                    );
          vid = Pci.Hdr.VendorId & 0xFFFF;
          did = Pci.Hdr.DeviceId & 0xFF00;
          if ((vid == 0x8086) && (did >= 0x2C00)
              //Slice - why 2:1? Intel spec said 3:4 - QCLK_RATIO at offset 0x50
              //  && (Device == 2) && (Function == 1)) {
              && (Device == 3) && (Function == 4)) {
			  DBG("Found QCLK_RATIO at bus 0x%02llX dev=%llX funs=%llX\n", Bus, Device, Function);
            Status = PciIo->Mem.Read (
                                      PciIo,
                                      EfiPciIoWidthUint32,
                                      EFI_PCI_IO_PASS_THROUGH_BAR,
                                      0x50,
                                      1,
                                      &qpimult
                                      );
            DBG("qpi read from PCI %X\n", qpimult & 0x1F);
            if (EFI_ERROR(Status)) continue;
            qpimult &= 0x1F; //bits 0:4
            break;
          }
        }
      }
    }
    
    DBG("qpimult %d\n", qpimult);
    qpibusspeed = MultU64x32(gCPUStructure.ExternalClock, qpimult * 2); //kHz
	  DBG("qpibusspeed %llukHz\n", qpibusspeed);
    gCPUStructure.ProcessorInterconnectSpeed = DivU64x32(qpibusspeed, kilo); //kHz->MHz
    // set QPI for Nehalem
    gSettings.QPI = (UINT16)gCPUStructure.ProcessorInterconnectSpeed;
    
  } else {
    gCPUStructure.ProcessorInterconnectSpeed = DivU64x32(LShiftU64(gCPUStructure.ExternalClock, 2), kilo); //kHz->MHz
  }
  gCPUStructure.MaxSpeed = (UINT32)(DivU64x32(MultU64x64(gCPUStructure.FSBFrequency, gCPUStructure.MaxRatio), Mega * 10)); //kHz->MHz
  
//  DBG("Vendor/Model/Stepping: 0x%X/0x%X/0x%X\n", gCPUStructure.Vendor, gCPUStructure.Model, gCPUStructure.Stepping);
//  DBG("Family/ExtFamily: 0x%X/0x%X\n", gCPUStructure.Family, gCPUStructure.Extfamily);
  DBG("MaxDiv/MinDiv: %d.%d/%d\n", gCPUStructure.MaxRatio/10, gCPUStructure.MaxRatio%10 , gCPUStructure.MinRatio/10);
  DBG("Turbo: %d/%d/%d/%d\n", gCPUStructure.Turbo4/10, gCPUStructure.Turbo3/10, gCPUStructure.Turbo2/10, gCPUStructure.Turbo1/10);
	DBG("Features: 0x%llX\n",gCPUStructure.Features);
  DBG("Threads: %d\n",gCPUStructure.Threads);
  DBG("Cores: %d\n",gCPUStructure.Cores);
  DBG("FSB: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.FSBFrequency, Mega)));
  DBG("CPU: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.CPUFrequency, Mega)));
  DBG("TSC: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.TSCFrequency, Mega)));
  DBG("PIS: %d MHz\n", (INT32)gCPUStructure.ProcessorInterconnectSpeed);
  DBG("ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock + 499, kilo)));
  //#if DEBUG_PCI
  
  //  WaitForKeyPress("waiting for key press...\n");
  //#endif  
  
  //  return;
}

VOID SetCPUProperties (VOID)
{
  UINT64    msr = 0;

  if ((gCPUStructure.CPUID[CPUID_6][ECX] & (1 << 3)) != 0) {
    if (gSettings.SavingMode != 0xFF) {
      msr = gSettings.SavingMode;
      AsmWriteMsr64(IA32_ENERGY_PERF_BIAS, msr);
      msr = AsmReadMsr64(IA32_ENERGY_PERF_BIAS); //0x1B0
		MsgLog("MSR 0x1B0   set to        %llX\n", msr);
    }
  }

}

//PCI info
/*
typedef struct {
  UINT16  VendorId;
  UINT16  DeviceId;
  UINT16  Command;
  UINT16  Status;
  UINT8   RevisionID;
  UINT8   ClassCode[3];
  UINT8   CacheLineSize;
  UINT8   LatencyTimer;
  UINT8   HeaderType;
  UINT8   BIST;
} PCI_DEVICE_INDEPENDENT_REGION;

///
/// PCI Device header region in PCI Configuration Space
/// Section 6.1, PCI Local Bus Specification, 2.2
///
typedef struct {
  UINT32  Bar[6];
  UINT32  CISPtr;
  UINT16  SubsystemVendorID;
  UINT16  SubsystemID;
  UINT32  ExpansionRomBar;
  UINT8   CapabilityPtr;
  UINT8   Reserved1[3];
  UINT32  Reserved2;
  UINT8   InterruptLine;
  UINT8   InterruptPin;
  UINT8   MinGnt;
  UINT8   MaxLat;
} PCI_DEVICE_HEADER_TYPE_REGION;

///
/// PCI Device Configuration Space
/// Section 6.1, PCI Local Bus Specification, 2.2
///
typedef struct {
  PCI_DEVICE_INDEPENDENT_REGION Hdr;
  PCI_DEVICE_HEADER_TYPE_REGION Device;
} PCI_TYPE00;
 

// Definitions of PCI Config Registers 
enum {
    kIOPCIConfigVendorID                = 0x00,
    kIOPCIConfigDeviceID                = 0x02,
    kIOPCIConfigCommand                 = 0x04,
    kIOPCIConfigStatus                  = 0x06,
    kIOPCIConfigRevisionID              = 0x08,
    kIOPCIConfigClassCode               = 0x09,
    kIOPCIConfigCacheLineSize           = 0x0C,
    kIOPCIConfigLatencyTimer            = 0x0D,
    kIOPCIConfigHeaderType              = 0x0E,
    kIOPCIConfigBIST                    = 0x0F,
    kIOPCIConfigBaseAddress0            = 0x10,
    kIOPCIConfigBaseAddress1            = 0x14,
    kIOPCIConfigBaseAddress2            = 0x18,
    kIOPCIConfigBaseAddress3            = 0x1C,
    kIOPCIConfigBaseAddress4            = 0x20,
    kIOPCIConfigBaseAddress5            = 0x24,
    kIOPCIConfigCardBusCISPtr           = 0x28,
    kIOPCIConfigSubSystemVendorID       = 0x2C,
    kIOPCIConfigSubSystemID             = 0x2E,
    kIOPCIConfigExpansionROMBase        = 0x30,
    kIOPCIConfigCapabilitiesPtr         = 0x34,
    kIOPCIConfigInterruptLine           = 0x3C,
    kIOPCIConfigInterruptPin            = 0x3D,
    kIOPCIConfigMinimumGrant            = 0x3E,
    kIOPCIConfigMaximumLatency          = 0x3F
};
*/

UINT16 GetStandardCpuType()
{
  if (gCPUStructure.Threads >= 4) {  
    return 0x402;   // Quad-Core Xeon
  }
  else if (gCPUStructure.Threads == 1) {  
    return 0x201;   // Core Solo
  }
  return 0x301;   // Core 2 Duo
}

UINT16 GetAdvancedCpuType ()
{  
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {  
    switch (gCPUStructure.Family) {  
      case 0x06:  
      {      
        switch (gCPUStructure.Model) {
          case CPU_MODEL_PENTIUM_M:
          case CPU_MODEL_DOTHAN:// Dothan
          case CPU_MODEL_YONAH: // Yonah
            return 0x201;
          case CPU_MODEL_CELERON: //M520
          case CPU_MODEL_MEROM: // Merom
          case CPU_MODEL_PENRYN:// Penryn
            if (AsciiStrStr(gCPUStructure.BrandString, "Xeon"))
              return 0x402; // Xeon
          case CPU_MODEL_ATOM:  // Atom (45nm)
            return GetStandardCpuType();
            
          case CPU_MODEL_NEHALEM_EX: //Xeon 5300
            return 0x402;
            
          case CPU_MODEL_NEHALEM: // Intel Core i7 LGA1366 (45nm)
            if (AsciiStrStr(gCPUStructure.BrandString, "Xeon"))
               return 0x501; // Xeon
            return 0x701; // Core i7
            
          case CPU_MODEL_FIELDS: // Lynnfield, Clarksfield, Jasper
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x601; // Core i5
            return 0x701; // Core i7
            
          case CPU_MODEL_DALES: // Intel Core i5, i7 LGA1156 (45nm) (Havendale, Auburndale)
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
              return 0x901; // Core i3 //why not 902? Ask Apple
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x602; // Core i5
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7"))
              return 0x702; // Core i7
            if (gCPUStructure.Cores <= 2) {
              return 0x602;
            }
            return 0x702; // Core i7
            
          //case CPU_MODEL_ARRANDALE:
          case CPU_MODEL_CLARKDALE: // Intel Core i3, i5, i7 LGA1156 (32nm) (Clarkdale, Arrandale)
            
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
              return 0x901; // Core i3
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x601; // Core i5 - (M540 -> 0x0602)
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7"))
              return 0x701; // Core i7
            if (gCPUStructure.Cores <= 2) {
              return 0x601;
            }
            return 0x701; // Core i7
            
          case CPU_MODEL_WESTMERE: // Intel Core i7 LGA1366 (32nm) 6 Core (Gulftown, Westmere-EP, Westmere-WS)
          case CPU_MODEL_WESTMERE_EX: // Intel Core i7 LGA1366 (45nm) 6 Core ???
            if (AsciiStrStr(gCPUStructure.BrandString, "Xeon"))
              return 0x501; // Xeon
            return 0x701; // Core i7
          case CPU_MODEL_SANDY_BRIDGE:  
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
              return 0x903; // Core i3
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x603; // Core i5
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7"))
              return 0x703; // Core i7
            if (gCPUStructure.Cores <= 2) {
              return 0x603;
            }
            return 0x703;
          case CPU_MODEL_IVY_BRIDGE:             
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
              return 0x903; // Core i3 - Apple doesn't use it
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x604; // Core i5
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7"))
              return 0x704; // Core i7
            if (gCPUStructure.Cores <= 2) {
              return 0x604;
            }
            return 0x704;
          case CPU_MODEL_HASWELL_U5:
   //       case CPU_MODEL_SKYLAKE_S:
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) M"))
              return 0xB06; // Core M
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
              return 0x906; // Core i3 - Apple doesn't use it
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x606; // Core i5
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7"))
              return 0x706; // Core i7
            if (gCPUStructure.Cores <= 2) {
              return 0x606;
            }
            return 0x706;
          case CPU_MODEL_HASWELL_E:
            return 0x507;
          case CPU_MODEL_IVY_BRIDGE_E5:
            return 0xA01;
          case CPU_MODEL_BROADWELL_E5:
            return 0xA02; //0xA02 or 0xA03
          case CPU_MODEL_ATOM_3700:
          case CPU_MODEL_HASWELL:
          case CPU_MODEL_HASWELL_ULT:
          case CPU_MODEL_CRYSTALWELL:
          case CPU_MODEL_BROADWELL_HQ:
          case CPU_MODEL_SKYLAKE_U:
          case CPU_MODEL_SKYLAKE_D:
          case CPU_MODEL_SKYLAKE_S:
          case CPU_MODEL_KABYLAKE1:
          case CPU_MODEL_KABYLAKE2:
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
              return 0x905; // Core i3 - Apple doesn't use it
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x605; // Core i5
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7-8"))
              return 0x709; // Core i7 CoffeeLake
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7-9"))
              return 0x1005; // Core i7 CoffeeLake
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7"))
              return 0x705; // Core i7
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i9"))
              return 0x1009; // Core i7 CoffeeLake
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) m3"))
              return 0xC05;
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) m5"))
              return 0xD05;
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) m7"))
              return 0xE05;
            if (AsciiStrStr(gCPUStructure.BrandString, "Xeon"))
              return 0xF01;
            if (gCPUStructure.Cores <= 2) {
              return 0x605;
            }
            return 0x705;
        }
      }
    }
  }
  return GetStandardCpuType();
}

MACHINE_TYPES GetDefaultModel()
{
  MACHINE_TYPES DefaultType = iMac132;
  if (gCPUStructure.Vendor != CPU_VENDOR_INTEL) {
    return MacPro31;
  }
  // TODO: Add more CPU models and configure the correct machines per CPU/GFX model
  if(gMobile) {
    switch (gCPUStructure.Model)
    {
      case CPU_MODEL_ATOM:
        DefaultType = MacBookAir31; //MacBookAir1,1 doesn't support _PSS for speedstep!
        break;
      case CPU_MODEL_DOTHAN:
        DefaultType = MacBook11;
        break;
      case CPU_MODEL_YONAH:
      case CPU_MODEL_PENTIUM_M:
        DefaultType = MacBook11;
        break;
      case CPU_MODEL_CELERON:
      case CPU_MODEL_MEROM:
        DefaultType = MacBook21;
        break;
      case CPU_MODEL_PENRYN:
        if ((gGraphics[0].Vendor == Nvidia) ||
            (gGraphics[1].Vendor == Nvidia)) {
          DefaultType = MacBookPro51;
        } else
          DefaultType = MacBook41;
        break;
      case CPU_MODEL_CLARKDALE:
        DefaultType = MacBookPro62;
        break;
      case CPU_MODEL_JAKETOWN:
      case CPU_MODEL_SANDY_BRIDGE:
        if((AsciiStrStr(gCPUStructure.BrandString, "i3")) ||
           (AsciiStrStr(gCPUStructure.BrandString, "i5"))) {
          DefaultType = MacBookPro81;
          break;
        }
        DefaultType = MacBookPro83;
        break;
      case CPU_MODEL_IVY_BRIDGE:
      case CPU_MODEL_IVY_BRIDGE_E5:
        DefaultType = MacBookAir52;
        break;
      case CPU_MODEL_HASWELL:
      case CPU_MODEL_HASWELL_E:
      case CPU_MODEL_ATOM_3700:
        DefaultType = MacBookAir62;
        break;
      case CPU_MODEL_HASWELL_ULT:
      case CPU_MODEL_CRYSTALWELL:
      case CPU_MODEL_BROADWELL_HQ:
        DefaultType = MacBookPro111;
        break;
      case CPU_MODEL_HASWELL_U5:  // Broadwell Mobile
        if(AsciiStrStr(gCPUStructure.BrandString, "M")) {
           DefaultType = MacBook81;
           break;
        }
        DefaultType = MacBookPro121;
        break;
      case CPU_MODEL_SKYLAKE_U:
        if(AsciiStrStr(gCPUStructure.BrandString, "m")) {
           DefaultType = MacBook91;
           break;
        }
        DefaultType = MacBookPro131;
        break;
      case CPU_MODEL_KABYLAKE1:
        if(AsciiStrStr(gCPUStructure.BrandString, "Y")) {
           DefaultType = MacBook101;
           break;
        }
        DefaultType = MacBookPro141;
        break;

      case CPU_MODEL_SKYLAKE_D:
        DefaultType = MacBookPro133;
        break;
      case CPU_MODEL_KABYLAKE2:
        DefaultType = MacBookPro143;
        break;
      default:
        if ((gGraphics[0].Vendor == Nvidia) ||
            (gGraphics[1].Vendor == Nvidia)) {
          DefaultType = MacBookPro51;
        } else
          DefaultType = MacBookPro83;
        break;
    }
  } else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_CELERON:
        DefaultType = MacMini21;
        break;
      case CPU_MODEL_LINCROFT:
        DefaultType = MacMini21;
        break;
      case CPU_MODEL_ATOM:
        DefaultType = MacMini21;
        break;
      case CPU_MODEL_CONROE:   //Conroe
//        DefaultType = iMac81; 
//        break;
      case CPU_MODEL_WOLFDALE:  //Wolfdale, Hapertown
        DefaultType = iMac101;  //MacPro31 - speedstep without patching; but it is Hackintosh
        break;
      case CPU_MODEL_NEHALEM:
        DefaultType = iMac111;
        break;
      case CPU_MODEL_NEHALEM_EX:
        DefaultType = MacPro41;
        break;
      case CPU_MODEL_FIELDS:
        if(AsciiStrStr(gCPUStructure.BrandString, "Xeon")) {
          DefaultType = MacPro41;
          break;
        }
        DefaultType = iMac113;
        break;
      case CPU_MODEL_DALES:
        DefaultType = iMac112;
        break;
      case CPU_MODEL_CLARKDALE:
        DefaultType = iMac112;
        break;
      case CPU_MODEL_WESTMERE:
        DefaultType = MacPro51;
        break;
      case CPU_MODEL_WESTMERE_EX:
        DefaultType = MacPro51;
        break;
      case CPU_MODEL_SANDY_BRIDGE:
        if (gGraphics[0].Vendor == Intel) {
          DefaultType = MacMini51;
          break;
        }
        if((AsciiStrStr(gCPUStructure.BrandString, "i3")) ||
           (AsciiStrStr(gCPUStructure.BrandString, "i5"))) {
          DefaultType = iMac121;
          break;
        }
        if(AsciiStrStr(gCPUStructure.BrandString, "i7")) {
          DefaultType = iMac122;
          break;
        }
        DefaultType = MacPro51;
        break;
      case CPU_MODEL_IVY_BRIDGE:
      case CPU_MODEL_IVY_BRIDGE_E5:
        DefaultType = iMac132;
        if (gGraphics[0].Vendor == Intel) {
          DefaultType = MacMini62;
          break;
        }
        if (AsciiStrStr(gCPUStructure.BrandString, "i3")) {
          DefaultType = iMac131;
          break;
        }
        break;
      case CPU_MODEL_JAKETOWN:
        DefaultType = MacPro41;
        break;
      case CPU_MODEL_HASWELL_U5:
        DefaultType = iMac151;
        break;
      case CPU_MODEL_SKYLAKE_D:  
      case CPU_MODEL_SKYLAKE_S:
        DefaultType = iMac171;
        break;
      case CPU_MODEL_KABYLAKE1:
      case CPU_MODEL_KABYLAKE2:
        if (AsciiStrStr(gCPUStructure.BrandString, "i5")) {
          DefaultType = iMac182;
          break;
        }
        DefaultType = iMac183;
        break;
      case CPU_MODEL_HASWELL:
      case CPU_MODEL_HASWELL_E:
        DefaultType = iMac142;
        if (AsciiStrStr(gCPUStructure.BrandString, "70S")) {
          DefaultType = iMac141;
          break;
        }
        break;
      case CPU_MODEL_BROADWELL_E5:
        DefaultType = MacPro61;
        break;
      default:
        DefaultType = iMac132;
        break;
    }
  }
  return DefaultType;
}

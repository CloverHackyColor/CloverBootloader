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

UINT8							gDefaultType; 
CPU_STRUCTURE			gCPUStructure;
UINT64            TurboMsr;
BOOLEAN           NeedPMfix = FALSE;

//this must not be defined at LegacyBios calls
#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3


VOID DoCpuid(UINT32 selector, UINT32 *data)
{
	AsmCpuid(selector, data, data+1, data+2, data+3);
}

VOID GetCPUProperties (VOID)
{
	UINT32		reg[4];
	UINT64		msr = 0;
  
	EFI_STATUS			Status;
	EFI_HANDLE			*HandleBuffer;
//	EFI_GUID        **ProtocolGuidArray;
	EFI_PCI_IO_PROTOCOL *PciIo;
	PCI_TYPE00          Pci;
	UINTN         HandleCount;
//	UINTN         ArrayCount;
	UINTN         HandleIndex;
//	UINTN         ProtocolIndex;
	UINT32				qpibusspeed; //units=kHz
	UINT32				qpimult = 2;
  UINT32        BusSpeed = 0; //units kHz
  UINT64        tmpU;
	UINT16				did, vid;
	UINTN         Segment;
	UINTN         Bus;
	UINTN         Device;
	UINTN         Function;
  CHAR8         str[128];
	
  //initial values 
	gCPUStructure.MaxRatio = 10; //keep it as K*10
	gCPUStructure.MinRatio = 10; //same
  gCPUStructure.SubDivider = 0;
	//gCPUStructure.MaxSpeed = 0;
	gSettings.CpuFreqMHz = 0;
	gCPUStructure.FSBFrequency = MultU64x32(gCPUStructure.ExternalClock, kilo); //kHz -> Hz
//	gCPUStructure.CPUFrequency = 0;
//	gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.CurrentSpeed, Mega); //MHz -> Hz
//  gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
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
  gCPUStructure.Vendor	= gCPUStructure.CPUID[CPUID_0][EBX];
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
  DBG("CPU Vendor = %x Model=%x\n", gCPUStructure.Vendor, gCPUStructure.Signature);
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
	
	gCPUStructure.Stepping	= (UINT8) bitfield(gCPUStructure.Signature, 3, 0);
	gCPUStructure.Model     = (UINT8) bitfield(gCPUStructure.Signature, 7, 4);
	gCPUStructure.Family    = (UINT8) bitfield(gCPUStructure.Signature, 11, 8);
	gCPUStructure.Type      = (UINT8) bitfield(gCPUStructure.Signature, 13, 12);
	gCPUStructure.Extmodel	= (UINT8) bitfield(gCPUStructure.Signature, 19, 16);
	gCPUStructure.Extfamily = (UINT8) bitfield(gCPUStructure.Signature, 27, 20);
	gCPUStructure.Features  = quad(gCPUStructure.CPUID[CPUID_1][ECX], gCPUStructure.CPUID[CPUID_1][EDX]);
	gCPUStructure.ExtFeatures  = quad(gCPUStructure.CPUID[CPUID_81][ECX], gCPUStructure.CPUID[CPUID_81][EDX]);
	/* Pack CPU Family and Model */
	if (gCPUStructure.Family == 0x0f) {
		gCPUStructure.Family += gCPUStructure.Extfamily;
	}
	gCPUStructure.Model += (gCPUStructure.Extmodel << 4);  
  
  //Calculate Nr or Cores
	if (gCPUStructure.Features & CPUID_FEATURE_HTT) {
		gCPUStructure.LogicalPerPackage	= (UINT32)bitfield(gCPUStructure.CPUID[CPUID_1][EBX], 23, 16); //Atom330 = 4
	} else {
		gCPUStructure.LogicalPerPackage	= 1;
	}
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
    DoCpuid(4, gCPUStructure.CPUID[CPUID_4]);
    gCPUStructure.CoresPerPackage =  (UINT32)bitfield(gCPUStructure.CPUID[CPUID_4][EAX], 31, 26) + 1; //Atom330 = 2
  } else if (gCPUStructure.Vendor == CPU_VENDOR_AMD) {
    DoCpuid(0x80000008, gCPUStructure.CPUID[CPUID_88]);
    gCPUStructure.CoresPerPackage =  (gCPUStructure.CPUID[CPUID_88][ECX] & 0xFF) + 1;
  }
  
	if (gCPUStructure.CoresPerPackage == 0) {
		gCPUStructure.CoresPerPackage = 1;
	}
  
  /* Fold in the Invariant TSC feature bit, if present */
	if((gCPUStructure.CPUID[CPUID_80][EAX] & 0x0000000f) >= 7){
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
    DBG("The CPU%a supported turbo\n", gCPUStructure.Turbo?"":" not");
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
      case CPU_MODEL_HASWELL_MB:
      case CPU_MODEL_HASWELL_ULT:
      case CPU_MODEL_HASWELL_ULX:
        msr = AsmReadMsr64(MSR_CORE_THREAD_COUNT);  //0x35
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
        
      default:		
//        gCPUStructure.Cores   = (UINT8)(bitfield(gCPUStructure.CPUID[CPUID_1][EBX], 23, 16));
//        gCPUStructure.Threads = (UINT8)(gCPUStructure.LogicalPerPackage & 0xff);
        gCPUStructure.Cores = 0;
        break;
    }    
  }
  
	if (gCPUStructure.Cores == 0) {
      gCPUStructure.Cores   = (UINT8)(gCPUStructure.CoresPerPackage & 0xff);
      gCPUStructure.Threads = (UINT8)(gCPUStructure.LogicalPerPackage & 0xff);
	}
		
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
		AsciiStrnCpy(gCPUStructure.BrandString, s, 48);
		
		if (!AsciiStrnCmp((const CHAR8*)gCPUStructure.BrandString, (const CHAR8*)CPU_STRING_UNKNOWN, iStrLen((gCPUStructure.BrandString) + 1, 48))) 
		{
			gCPUStructure.BrandString[0] = '\0'; 
		}
		gCPUStructure.BrandString[47] = '\0'; 
		DBG("BrandString = %a\n", gCPUStructure.BrandString);
	}

  //workaround for N270. I don't know why it detected wrong
  if ((gCPUStructure.Model == CPU_MODEL_ATOM) &&
      (AsciiStrStr(gCPUStructure.BrandString, "270"))) {
    gCPUStructure.Cores   = 1;
    gCPUStructure.Threads = 2;
  }


	//get Min and Max Ratio Cpu/Bus
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
              MsgLog("non-usable FLEX_RATIO = %x\n", msr);
              if (flex_ratio == 0) { 
                AsmWriteMsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL)); 
                gBS->Stall(10);
                msr = AsmReadMsr64(MSR_FLEX_RATIO);
                MsgLog("corrected FLEX_RATIO = %x\n", msr);
              }
            }
     //       
            msr = AsmReadMsr64(MSR_PLATFORM_INFO);            
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
            if (gCPUStructure.Turbo /* &&
                (gCPUStructure.Model != CPU_MODEL_NEHALEM_EX) &&
                (gCPUStructure.Model != CPU_MODEL_WESTMERE_EX)  &&
                (gCPUStructure.Model != CPU_MODEL_FIELDS) */ ) {
              msr = AsmReadMsr64(MSR_TURBO_RATIO_LIMIT);

              gCPUStructure.Turbo1 = (UINT8)(RShiftU64(msr, 0) & 0xff);
              gCPUStructure.Turbo2 = (UINT8)(RShiftU64(msr, 8) & 0xff);
              gCPUStructure.Turbo3 = (UINT8)(RShiftU64(msr, 16) & 0xff);
              gCPUStructure.Turbo4 = (UINT8)(RShiftU64(msr, 24) & 0xff); //later
            }
            /* Not sure what this is here for - apianti
            } else {
              gCPUStructure.Turbo4 = (UINT16)(gCPUStructure.MaxRatio + 1);
            }

            
            if (gCPUStructure.Cores < 4) {
              gCPUStructure.Turbo4 = gCPUStructure.Turbo1;
            } */

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
          case CPU_MODEL_HASWELL:
          case CPU_MODEL_HASWELL_MB:
          case CPU_MODEL_HASWELL_ULT:
          case CPU_MODEL_HASWELL_ULX:
            gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.CurrentSpeed, Mega); //MHz -> Hz
            gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
            //----test C3 patch
            msr = AsmReadMsr64(MSR_PKG_CST_CONFIG_CONTROL); //0xE2
            MsgLog("MSR 0xE2 before patch %08x\n", msr);
            if (msr & 0x8000) {
              MsgLog("MSR 0xE2 is locked, PM patches will be turned on\n");
              NeedPMfix = TRUE;
            }
    //        AsmWriteMsr64(MSR_PKG_CST_CONFIG_CONTROL, (msr & 0x8000000ULL));
    //        msr = AsmReadMsr64(MSR_PKG_CST_CONFIG_CONTROL);
    //        MsgLog("MSR 0xE2 after  patch %08x\n", msr);
            msr = AsmReadMsr64(MSR_PMG_IO_CAPTURE_BASE);
            MsgLog("MSR 0xE4              %08x\n", msr);
            //------------
            msr = AsmReadMsr64(MSR_PLATFORM_INFO);       //0xCE  
            MsgLog("MSR 0xCE              %08x_%08x\n", (msr>>32), msr);
            gCPUStructure.MaxRatio = (UINT8)RShiftU64(msr, 8) & 0xff;
            gCPUStructure.MinRatio = (UINT8)MultU64x32(RShiftU64(msr, 40) & 0xff, 10);
            msr = AsmReadMsr64(MSR_FLEX_RATIO);   //0x194
            if ((RShiftU64(msr, 16) & 0x01) != 0) {
              // bcc9 patch
              UINT8 flex_ratio = RShiftU64(msr, 8) & 0xff;
              MsgLog("non-usable FLEX_RATIO = %x\n", msr);
              if (flex_ratio == 0) { 
                AsmWriteMsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL)); 
                gBS->Stall(10);
                msr = AsmReadMsr64(MSR_FLEX_RATIO);
                MsgLog("corrected FLEX_RATIO = %x\n", msr);
              }
              /*else { 
                if(gCPUStructure.BusRatioMax > flex_ratio) 
                  gCPUStructure.BusRatioMax = (UINT8)flex_ratio;
              }*/
            }
            if ((gCPUStructure.CPUID[CPUID_6][ECX] & (1 << 3)) != 0) {
              msr = AsmReadMsr64(IA32_ENERGY_PERF_BIAS); //0x1B0
              MsgLog("MSR 0x1B0             %08x\n", msr);
            }
            
            if(gCPUStructure.MaxRatio) {
              gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.MaxRatio);
            } else {
              gCPUStructure.FSBFrequency = 100000000ULL; //100*Mega
            }

            /* Unneccessary - apianti
            msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);  //0x198
            gCPUStructure.MaxRatio = (UINT8)((msr >> 8) & 0xff);
            TurboMsr = msr + (1 << 8);
            */
            
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
            //MSR 000001AD  0000-0000-3B3B-3B3B - from AIDA64
            // so there is a workaround
            if (gCPUStructure.Turbo4 == 0x3B) {
              gCPUStructure.Turbo4 = (UINT16)gCPUStructure.MaxRatio + (gCPUStructure.Turbo?1:0);
              //this correspond to 2nd-gen-core-desktop-specification-update.pdf
            }
            
            gCPUStructure.MaxRatio *= 10;
            gCPUStructure.Turbo1 *= 10;
            gCPUStructure.Turbo2 *= 10;
            gCPUStructure.Turbo3 *= 10;
            gCPUStructure.Turbo4 *= 10;
            break;
          case CPU_MODEL_ATOM://  Atom
          case CPU_MODEL_DOTHAN:// Pentium M, Dothan, 90nm
          case CPU_MODEL_YONAH:// Core Duo/Solo, Pentium M DC
          case CPU_MODEL_MEROM:// Core Xeon, Core 2 Duo, 65nm, Mobile
          //case CPU_MODEL_CONROE:// Core Xeon, Core 2 Duo, 65nm, Desktop like Merom but not mobile
          case CPU_MODEL_PENRYN:// Core 2 Duo/Extreme, Xeon, 45nm , Mobile
          //case CPU_MODEL_WOLFDALE:// Core 2 Duo/Extreme, Xeon, 45nm, Desktop like Penryn but not Mobile
            if(AsmReadMsr64(MSR_IA32_PLATFORM_ID) & (1 << 28)){
              gCPUStructure.Mobile = TRUE;
            }
            gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.MaxSpeed, Mega); //MHz -> Hz
            gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
            msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);
     //       TurboMsr = msr + (1 << 8);
            gCPUStructure.MaxRatio = (UINT32)(RShiftU64(msr, 8)) & 0x1f;
            TurboMsr = (UINT32)(RShiftU64(msr, 40)) & 0x1f;
            if ((TurboMsr > gCPUStructure.MaxRatio) && (gCPUStructure.Model == CPU_MODEL_MEROM)) {
              DBG(" CPU works at low speed, MaxRatio=%d CurrRatio=%d\n", TurboMsr,
                  gCPUStructure.MaxRatio);
              gCPUStructure.MaxRatio = (UINT32)TurboMsr;
            }
            gCPUStructure.SubDivider = (UINT32)(RShiftU64(msr, 14)) & 0x1;
            gCPUStructure.MinRatio = 60;
            if(!gCPUStructure.MaxRatio) gCPUStructure.MaxRatio = 6; // :(
            msr = AsmReadMsr64(0xCD);
           // gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency * 2,
           //                                     gCPUStructure.MaxRatio * 2 + gCPUStructure.SubDivider);
            gCPUStructure.FSBFrequency = DivU64x32(LShiftU64(gCPUStructure.TSCFrequency, 1),
                                gCPUStructure.MaxRatio * 2 + gCPUStructure.SubDivider);
            if ((msr & 3) == 2 && (gCPUStructure.FSBFrequency < 196 * Mega)) {
              DBG("wrong MaxRatio = %d.%d, corrected\n", gCPUStructure.MaxRatio, gCPUStructure.SubDivider * 5);
              gCPUStructure.MaxRatio = DivU64x32(gCPUStructure.TSCFrequency, 200 * Mega);
            }
            gCPUStructure.MaxRatio = gCPUStructure.MaxRatio * 10 + gCPUStructure.SubDivider * 5; 
            gCPUStructure.Turbo4 = (UINT16)(gCPUStructure.MaxRatio + 10);
            DBG("MSR dumps:\n");
            DBG("\t@0x00CD=%lx\n", msr);
            DBG("\t@0x0198=%lx\n", AsmReadMsr64(MSR_IA32_PERF_STATUS));
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
      gCPUStructure.TSCFrequency = MultU64x32(gCPUStructure.CurrentSpeed, Mega); //MHz -> Hz
      gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
		if(gCPUStructure.Extfamily == 0x00 /* K8 */) {
			msr = AsmReadMsr64(K8_FIDVID_STATUS);
      gCPUStructure.MaxRatio = (UINT32)(RShiftU64((RShiftU64(msr, 16) & 0x3f), 2) + 4);
      gCPUStructure.MinRatio = (UINT32)(RShiftU64((RShiftU64(msr, 8) & 0x3f), 2) + 4);
		}
		else if(gCPUStructure.Extfamily >= 0x01 /* K10+ */) {
			msr = AsmReadMsr64(K10_COFVID_STATUS);  //30BA-0063-3C00-180D
/*			if(gCPUStructure.Extfamily == 0x01 ) { //K10
				gCPUStructure.MaxRatio = (UINT32)DivU64x32(((msr & 0x3f) + 0x10), (1 << ((RShiftU64(msr, 6) & 0x7))));
        // = 0x1D , expected 14.5*2
            }
			else {// K11+ msr = 0280-0006-3602-1A1A */
				gCPUStructure.MaxRatio = (UINT32)DivU64x32(((msr & 0x3f) + 0x10), (1 << ((RShiftU64(msr, 6) & 0x7))));
        // = 0x2A , expected 21*2
 //     }
         // Get min ratio
      msr = AsmReadMsr64(K10_COFVID_LIMIT);
      msr = AsmReadMsr64(K10_PSTATE_STATUS + ((RShiftU64(msr, 4) & 0x07)));
/*      if(gCPUStructure.Extfamily == 0x01) { // K10 
				gCPUStructure.MinRatio = 5 * (UINT32)DivU64x32(((msr & 0x3f) + 0x10), (1 << ((RShiftU64(msr, 6) & 0x7))));
        // bred
      } else  {// K11+ msr = 0000-0000-0000-0040, 0000-0173-0000-181A */
				gCPUStructure.MinRatio = 5 * (UINT32)DivU64x32(((msr & 0x3f) + 0x08), (1 << ((RShiftU64(msr, 6) & 0x7))));
        // bred
 //     }                                             
		}
//    gCPUStructure.MaxRatio >>= 1;
    if (!gCPUStructure.MaxRatio) {
      gCPUStructure.MaxRatio = 1; //??? to avoid zero division
    }
    gCPUStructure.FSBFrequency = DivU64x32(LShiftU64(gCPUStructure.TSCFrequency, 1), gCPUStructure.MaxRatio);
    gCPUStructure.MaxRatio *= 5;
	}
  
 // DBG("take FSB\n");
  tmpU = gCPUStructure.FSBFrequency;
//  DBG("divide by 1000\n");
  BusSpeed = (UINT32)DivU64x32(tmpU, kilo); //Hz -> kHz
  DBG("FSBFrequency=%dMHz\n", DivU64x32(tmpU, Mega));
     //now check if SMBIOS has ExternalClock = 4xBusSpeed
  if ((BusSpeed > 50*kilo) && (gCPUStructure.ExternalClock > BusSpeed * 3)) { //khz
    gCPUStructure.ExternalClock = BusSpeed;
  } else {
    tmpU = MultU64x32(gCPUStructure.ExternalClock, kilo); //kHz -> Hz
    gCPUStructure.FSBFrequency = tmpU;
  }
  tmpU = gCPUStructure.FSBFrequency;
  DBG("Corrected FSBFrequency=%dMHz\n", DivU64x32(tmpU, Mega));
	
	if ((gCPUStructure.Vendor == CPU_VENDOR_INTEL) && (gCPUStructure.Model == CPU_MODEL_NEHALEM)) {
		//Slice - for Nehalem we can do more calculation as in Cham
    // but this algo almost always wrong
    //
		// thanks to dgobe for i3/i5/i7 bus speed detection
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
              //	&& (Device == 2) && (Function == 1)) {
              && (Device == 3) && (Function == 4)) {
            DBG("Found QCLK_RATIO at bus 0x%02x dev=%x funs=%x\n", Bus, Device, Function);                 
            Status = PciIo->Mem.Read (
                                      PciIo,
                                      EfiPciIoWidthUint32,
                                      EFI_PCI_IO_PASS_THROUGH_BAR,           
                                      0x50,
                                      1,
                                      &qpimult
                                      );
            DBG("qpi read from PCI %x\n", qpimult);
            if (EFI_ERROR(Status)) continue;
            qpimult &= 0x1F; //bits 0:4
            break;
          }
        }
			}
		}

		DBG("qpimult %d\n", qpimult);
		qpibusspeed = qpimult * 2 * gCPUStructure.ExternalClock; //kHz
		DBG("qpibusspeed %dkHz\n", qpibusspeed);
		gCPUStructure.ProcessorInterconnectSpeed = DivU64x32(qpibusspeed, kilo); //kHz->MHz

	} else {
    gCPUStructure.ProcessorInterconnectSpeed = DivU64x32(LShiftU64(gCPUStructure.ExternalClock, 2), kilo); //kHz->MHz
	}
	gCPUStructure.MaxSpeed = (UINT32)(DivU64x32(MultU64x64(gCPUStructure.FSBFrequency, gCPUStructure.MaxRatio), Mega * 10)); //kHz->MHz

	DBG("Vendor/Model/Stepping: 0x%x/0x%x/0x%x\n", gCPUStructure.Vendor, gCPUStructure.Model, gCPUStructure.Stepping);
	DBG("Family/ExtFamily: 0x%x/0x%x\n", gCPUStructure.Family, gCPUStructure.Extfamily);
	DBG("MaxDiv/MinDiv: %d.%d/%d\n", gCPUStructure.MaxRatio/10, gCPUStructure.MaxRatio%10 , gCPUStructure.MinRatio/10);
  DBG("Turbo: %d/%d/%d/%d\n", gCPUStructure.Turbo4/10, gCPUStructure.Turbo3/10, gCPUStructure.Turbo2/10, gCPUStructure.Turbo1/10);
	DBG("Features: 0x%08x\n",gCPUStructure.Features);
	DBG("Threads: %d\n",gCPUStructure.Threads);
	DBG("Cores: %d\n",gCPUStructure.Cores);
	DBG("FSB: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, kilo)));
	DBG("CPU: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.CPUFrequency, Mega)));
	DBG("TSC: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.TSCFrequency, Mega)));
	DBG("PIS: %d MHz\n", (INT32)gCPUStructure.ProcessorInterconnectSpeed);
//#if DEBUG_PCI
	
//	WaitForKeyPress("waiting for key press...\n");
//#endif	

//	return;
}

VOID SetCPUProperties (VOID)
{
	UINT64		msr = 0;

  if ((gCPUStructure.CPUID[CPUID_6][ECX] & (1 << 3)) != 0) {
    if (gSettings.SavingMode != 0xFF) {
      msr = gSettings.SavingMode;
      AsmWriteMsr64(IA32_ENERGY_PERF_BIAS, msr);
      msr = AsmReadMsr64(IA32_ENERGY_PERF_BIAS); //0x1B0
      MsgLog("MSR 0x1B0   set to        %08x\n", msr);
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
					case CPU_MODEL_DOTHAN:// Dothan
					case CPU_MODEL_YONAH: // Yonah
						return 0x201;
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
          case CPU_MODEL_IVY_BRIDGE_E5:
            return 0xA01;
          case CPU_MODEL_HASWELL:
          case CPU_MODEL_HASWELL_MB:
          case CPU_MODEL_HASWELL_ULT:
          case CPU_MODEL_HASWELL_ULX:
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
							return 0x905; // Core i3 - Apple doesn't use it
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x605; // Core i5
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i7"))
              return 0x705; // Core i7
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
  MACHINE_TYPES DefaultType = MacPro31;
  if (gCPUStructure.Vendor != CPU_VENDOR_INTEL) {
    return DefaultType;
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
				DefaultType = MacBook11;
				break;
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
      case CPU_MODEL_HASWELL_MB:
        DefaultType = MacBookAir62;
				break;  
      case CPU_MODEL_HASWELL_ULT:
      case CPU_MODEL_HASWELL_ULX:
        DefaultType = MacBookPro111;
				break;  
			default:
				if ((gGraphics[0].Vendor == Nvidia) ||
            (gGraphics[1].Vendor == Nvidia)) {
					DefaultType = MacBookPro51;
				} else
					DefaultType = MacBook52;
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
				DefaultType = iMac81;
				break;
			case CPU_MODEL_WOLFDALE:	//Wolfdale, Hapertown
				DefaultType = iMac101;//MacPro31 - speedstep without patching; but it is Hackintosh
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
      case CPU_MODEL_HASWELL:
      case CPU_MODEL_HASWELL_MB:
        DefaultType = iMac142;
        if (AsciiStrStr(gCPUStructure.BrandString, "70S")) {
        	DefaultType = iMac141;
        	break;
        }
        break;        
			default:
				DefaultType = MacPro31;
				break;
		}
	}	
	return DefaultType;
}

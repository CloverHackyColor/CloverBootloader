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


#define DEBUG_CPU 1
#define DEBUG_PCI 1

#if DEBUG_CPU == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_CPU == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

UINT8							gDefaultType; 
CPU_STRUCTURE			gCPUStructure;

/* CPU defines */
#define bit(n)			(1UL << (n))
#define _Bit(n)			(1ULL << n)
#define _HBit(n)		(1ULL << ((n)+32))

#define bitmask(h,l)	((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)	(((x) & bitmask(h,l)) >> l)
#define quad(hi,lo)     (((UINT64)(hi)) << 32 | (lo))

//definitions from Apple XNU
/*
 * The CPUID_FEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 1: 
 */
#define	CPUID_FEATURE_FPU     _Bit(0)	/* Floating point unit on-chip */
#define	CPUID_FEATURE_VME     _Bit(1)	/* Virtual Mode Extension */
#define	CPUID_FEATURE_DE      _Bit(2)	/* Debugging Extension */
#define	CPUID_FEATURE_PSE     _Bit(3)	/* Page Size Extension */
#define	CPUID_FEATURE_TSC     _Bit(4)	/* Time Stamp Counter */
#define	CPUID_FEATURE_MSR     _Bit(5)	/* Model Specific Registers */
#define CPUID_FEATURE_PAE     _Bit(6)	/* Physical Address Extension */
#define	CPUID_FEATURE_MCE     _Bit(7)	/* Machine Check Exception */
#define	CPUID_FEATURE_CX8     _Bit(8)	/* CMPXCHG8B */
#define	CPUID_FEATURE_APIC    _Bit(9)	/* On-chip APIC */
#define CPUID_FEATURE_SEP     _Bit(11)	/* Fast System Call */
#define	CPUID_FEATURE_MTRR    _Bit(12)	/* Memory Type Range Register */
#define	CPUID_FEATURE_PGE     _Bit(13)	/* Page Global Enable */
#define	CPUID_FEATURE_MCA     _Bit(14)	/* Machine Check Architecture */
#define	CPUID_FEATURE_CMOV    _Bit(15)	/* Conditional Move Instruction */
#define CPUID_FEATURE_PAT     _Bit(16)	/* Page Attribute Table */
#define CPUID_FEATURE_PSE36   _Bit(17)	/* 36-bit Page Size Extension */
#define CPUID_FEATURE_PSN     _Bit(18)	/* Processor Serial Number */
#define CPUID_FEATURE_CLFSH   _Bit(19)	/* CLFLUSH Instruction supported */
#define CPUID_FEATURE_DS      _Bit(21)	/* Debug Store */
#define CPUID_FEATURE_ACPI    _Bit(22)	/* Thermal monitor and Clock Ctrl */
#define CPUID_FEATURE_MMX     _Bit(23)	/* MMX supported */
#define CPUID_FEATURE_FXSR    _Bit(24)	/* Fast floating pt save/restore */
#define CPUID_FEATURE_SSE     _Bit(25)	/* Streaming SIMD extensions */
#define CPUID_FEATURE_SSE2    _Bit(26)	/* Streaming SIMD extensions 2 */
#define CPUID_FEATURE_SS      _Bit(27)	/* Self-Snoop */
#define CPUID_FEATURE_HTT     _Bit(28)	/* Hyper-Threading Technology */
#define CPUID_FEATURE_TM      _Bit(29)	/* Thermal Monitor (TM1) */
#define CPUID_FEATURE_PBE     _Bit(31)	/* Pend Break Enable */

#define CPUID_FEATURE_SSE3    _HBit(0)	/* Streaming SIMD extensions 3 */
#define CPUID_FEATURE_PCLMULQDQ _HBit(1) /* PCLMULQDQ Instruction */

#define CPUID_FEATURE_MONITOR _HBit(3)	/* Monitor/mwait */
#define CPUID_FEATURE_DSCPL   _HBit(4)	/* Debug Store CPL */
#define CPUID_FEATURE_VMX     _HBit(5)	/* VMX */
#define CPUID_FEATURE_SMX     _HBit(6)	/* SMX */
#define CPUID_FEATURE_EST     _HBit(7)	/* Enhanced SpeedsTep (GV3) */
#define CPUID_FEATURE_TM2     _HBit(8)	/* Thermal Monitor 2 */
#define CPUID_FEATURE_SSSE3   _HBit(9)	/* Supplemental SSE3 instructions */
#define CPUID_FEATURE_CID     _HBit(10)	/* L1 Context ID */

#define CPUID_FEATURE_CX16    _HBit(13)	/* CmpXchg16b instruction */
#define CPUID_FEATURE_xTPR    _HBit(14)	/* Send Task PRiority msgs */
#define CPUID_FEATURE_PDCM    _HBit(15)	/* Perf/Debug Capability MSR */

#define CPUID_FEATURE_DCA     _HBit(18)	/* Direct Cache Access */
#define CPUID_FEATURE_SSE4_1  _HBit(19)	/* Streaming SIMD extensions 4.1 */
#define CPUID_FEATURE_SSE4_2  _HBit(20)	/* Streaming SIMD extensions 4.2 */
#define CPUID_FEATURE_xAPIC   _HBit(21)	/* Extended APIC Mode */
#define CPUID_FEATURE_POPCNT  _HBit(23)	/* POPCNT instruction */
#define CPUID_FEATURE_AES     _HBit(25)	/* AES instructions */
#define CPUID_FEATURE_VMM     _HBit(31)	/* VMM (Hypervisor) present */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000001: 
 */
#define CPUID_EXTFEATURE_SYSCALL   _Bit(11)	/* SYSCALL/sysret */
#define CPUID_EXTFEATURE_XD		   _Bit(20)	/* eXecute Disable */
#define CPUID_EXTFEATURE_1GBPAGE   _Bit(26)     /* 1G-Byte Page support */
#define CPUID_EXTFEATURE_RDTSCP	   _Bit(27)	/* RDTSCP */
#define CPUID_EXTFEATURE_EM64T	   _Bit(29)	/* Extended Mem 64 Technology */

//#define CPUID_EXTFEATURE_LAHF	   _HBit(20)	/* LAFH/SAHF instructions */
// New definition with Snow kernel
#define CPUID_EXTFEATURE_LAHF	   _HBit(0)	/* LAHF/SAHF instructions */
/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000007: 
 */
#define CPUID_EXTFEATURE_TSCI      _Bit(8)	/* TSC Invariant */

#define	CPUID_CACHE_SIZE	16	/* Number of descriptor values */

#define CPUID_MWAIT_EXTENSION	_Bit(0)	/* enumeration of WMAIT extensions */
#define CPUID_MWAIT_BREAK	_Bit(1)	/* interrupts are break events	   */

/* Unknown CPU */
#define CPU_STRING_UNKNOWN	"Unknown CPU Type"

/* Known MSR registers */
#define MSR_IA32_PLATFORM_ID        0x0017	 
#define MSR_CORE_THREAD_COUNT       0x0035	 /* limited use - not for Penryn or older	*/
#define MSR_IA32_BIOS_SIGN_ID       0x008B   /* microcode version */
#define MSR_FSB_FREQ                0x00CD	 /* limited use - not for i7						*/
#define	MSR_PLATFORM_INFO           0x00CE   /* limited use - MinRatio for i7 but Max for Yonah	*/
                                             /* turbo for penryn */
#define MSR_IA32_EXT_CONFIG         0x00EE	 /* limited use - not for i7						*/
#define MSR_FLEX_RATIO              0x0194	 /* limited use - not for Penryn or older			*/
          //see no value on most CPUs
#define	MSR_IA32_PERF_STATUS        0x0198
#define MSR_IA32_PERF_CONTROL       0x0199
#define MSR_IA32_CLOCK_MODULATION   0x019A
#define MSR_THERMAL_STATUS          0x019C
#define MSR_IA32_MISC_ENABLE        0x01A0
#define MSR_THERMAL_TARGET          0x01A2	 /* limited use - not for Penryn or older			*/
#define MSR_TURBO_RATIO_LIMIT       0x01AD	 /* limited use - not for Penryn or older			*/

//Copied from revogirl
#define IA32_ENERGY_PERF_BIAS		0x01B0
#define IA32_PLATFORM_DCA_CAP		0x01F8


// Sandy Bridge & JakeTown specific 'Running Average Power Limit' MSR's.
#define MSR_RAPL_POWER_UNIT			0x606

#define MSR_PKG_RAPL_POWER_LIMIT	0x610
#define MSR_PKG_ENERGY_STATUS		0x611
#define MSR_PKG_PERF_STATUS			0x613
#define MSR_PKG_POWER_INFO			0x614

// Sandy Bridge IA (Core) domain MSR's.
#define MSR_PP0_POWER_LIMIT			0x638
#define MSR_PP0_ENERGY_STATUS		0x639
#define MSR_PP0_POLICY          0x63A
#define MSR_PP0_PERF_STATUS			0x63B

// Sandy Bridge Uncore (IGPU) domain MSR's (Not on JakeTown).
#define MSR_PP1_POWER_LIMIT			0x640
#define MSR_PP1_ENERGY_STATUS		0x641
#define MSR_PP1_POLICY          0x642

// JakeTown only Memory MSR's.
#define MSR_DRAM_POWER_LIMIT		0x618
#define MSR_DRAM_ENERGY_STATUS	0x619
#define MSR_DRAM_PERF_STATUS		0x61B
#define MSR_DRAM_POWER_INFO			0x61C


//AMD
#define K8_FIDVID_STATUS        0xC0010042
#define K10_COFVID_STATUS       0xC0010071
#define DEFAULT_FSB             100000          /* for now, hardcoding 100MHz for old CPUs */

VOID DoCpuid(UINT32 selector, UINT32 *data)
{
	AsmCpuid(selector, data, data+1, data+2, data+3);
}

VOID GetCPUProperties (VOID)
{
	UINT32		reg[4];
	UINT64		msr = 0;
	BOOLEAN		fix_fsb, core_i, turbo, isatom, fsbad;
  
	EFI_STATUS			Status;
	EFI_HANDLE			*HandleBuffer;
	EFI_GUID        **ProtocolGuidArray;
	EFI_PCI_IO_PROTOCOL *PciIo;
	PCI_TYPE00          Pci;
	UINTN         HandleCount;
	UINTN         ArrayCount;
	UINTN         HandleIndex;
	UINTN         ProtocolIndex;
	UINT16				qpibusspeed; //units=MHz
	UINT16				qpimult = 2;
  UINT32        BusSpeed = 0; //units kHz

	UINT16				did, vid;
	UINTN         Segment;
	UINTN         Bus;
	UINTN         Device;
	UINTN         Function;
  CHAR8         str[128], *s;
	
  //initial values 
	gCPUStructure.MaxRatio = 1;
	gCPUStructure.MinRatio = 1;
  gCPUStructure.SubDivider = 0;
	gCPUStructure.MaxSpeed = 0;
	gSettings.CpuFreqMHz = gCPUStructure.CurrentSpeed;
	gCPUStructure.FSBFrequency = gCPUStructure.ExternalClock  * kilo; //kHz -> Hz
	gCPUStructure.CPUFrequency = 0;
	gCPUStructure.TSCFrequency = gCPUStructure.CurrentSpeed * Mega; //MHz -> Hz
	gCPUStructure.ProcessorInterconnectSpeed = 0;
	gCPUStructure.Mobile = FALSE; //not same as gMobile

	/* get CPUID Values */
  DoCpuid(0, gCPUStructure.CPUID[CPUID_0]);
  /*
	 * Get processor signature and decode
	 * and bracket this with the approved procedure for reading the
	 * the microcode version number a.k.a. signature a.k.a. BIOS ID
	 */  
  AsmWriteMsr64(MSR_IA32_BIOS_SIGN_ID, 0);
  DoCpuid(1, gCPUStructure.CPUID[CPUID_1]);
  msr = AsmReadMsr64(MSR_IA32_BIOS_SIGN_ID);
  gCPUStructure.MicroCode = msr >> 32;
  /* Get "processor flag"; necessary for microcode update matching */
  gCPUStructure.ProcessorFlag = (AsmReadMsr64(MSR_IA32_PLATFORM_ID) >> 50) & 3;
  
//  DoCpuid(2, gCPUStructure.CPUID[2]);

	DoCpuid(0x80000000, gCPUStructure.CPUID[CPUID_80]);
	if((gCPUStructure.CPUID[CPUID_80][EAX] & 0x0000000f) >= 1){
		DoCpuid(0x80000001, gCPUStructure.CPUID[CPUID_81]); 
	}
	
	gCPUStructure.Vendor	= gCPUStructure.CPUID[CPUID_0][EBX];
	gCPUStructure.Signature = gCPUStructure.CPUID[CPUID_1][EAX];
	gCPUStructure.Stepping	= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][EAX], 3, 0);
	gCPUStructure.Model		= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][EAX], 7, 4);
	gCPUStructure.Family	= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][EAX], 11, 8);
	gCPUStructure.Type		= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][EAX], 13, 12);
	gCPUStructure.Extmodel	= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][EAX], 19, 16);
	gCPUStructure.Extfamily = (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][EAX], 27, 20);
	gCPUStructure.Features  = quad(gCPUStructure.CPUID[CPUID_1][ECX], gCPUStructure.CPUID[CPUID_1][EDX]);
	gCPUStructure.ExtFeatures  = quad(gCPUStructure.CPUID[CPUID_81][ECX], gCPUStructure.CPUID[CPUID_81][EDX]);
	/* Pack CPU Family and Model */
	if (gCPUStructure.Family == 0x0f) {
		gCPUStructure.Family += gCPUStructure.Extfamily;
	}
	gCPUStructure.Model += (gCPUStructure.Extmodel << 4);  
  
  //Calculate Nr or Cores
	if (gCPUStructure.Features & CPUID_FEATURE_HTT) {
		gCPUStructure.LogicalPerPackage	= bitfield(gCPUStructure.CPUID[CPUID_1][EBX], 23, 16); //Atom330 = 4
	} else {
		gCPUStructure.LogicalPerPackage	= 1;
	}
  DoCpuid(4, gCPUStructure.CPUID[CPUID_4]);
	gCPUStructure.CoresPerPackage =  bitfield(gCPUStructure.CPUID[CPUID_4][EAX], 31, 26) + 1; //Atom330 = 2
	if (gCPUStructure.CoresPerPackage == 0) {
		gCPUStructure.CoresPerPackage = 1;
	}
  
  /* Fold in the Invariant TSC feature bit, if present */
	if((gCPUStructure.CPUID[CPUID_80][EAX] & 0x0000000f) >= 7){
		DoCpuid(0x80000007, gCPUStructure.CPUID[CPUID_87]); 
  gCPUStructure.ExtFeatures |=
    gCPUStructure.CPUID[CPUID_87][EDX] & (UINT32)CPUID_EXTFEATURE_TSCI;
	}
  
  
  
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
    switch (gCPUStructure.Model)
    {
      case CPU_MODEL_NEHALEM: // Intel Core i7 LGA1366 (45nm)
      case CPU_MODEL_FIELDS: // Intel Core i5, i7 LGA1156 (45nm)
      case CPU_MODEL_CLARKDALE: // Intel Core i3, i5, i7 LGA1156 (32nm) 
      case CPU_MODEL_NEHALEM_EX:	
      case CPU_MODEL_JAKETOWN:
      case CPU_MODEL_SANDY_BRIDGE:					
      {
        msr = AsmReadMsr64(MSR_CORE_THREAD_COUNT);
        gCPUStructure.Cores   = (UINT8)bitfield((UINT32)msr, 31, 16);
        gCPUStructure.Threads = (UINT8)bitfield((UINT32)msr, 15,  0);
      } break;
        
      case CPU_MODEL_DALES:
      case CPU_MODEL_WESTMERE: // Intel Core i7 LGA1366 (32nm) 6 Core
      case CPU_MODEL_WESTMERE_EX:
        
      {
        msr = AsmReadMsr64(MSR_CORE_THREAD_COUNT);
        gCPUStructure.Cores   = (UINT8)bitfield((UINT32)msr, 19, 16);
        gCPUStructure.Threads = (UINT8)bitfield((UINT32)msr, 15,  0);
        break;
      }
        
      default:		
        gCPUStructure.Cores   = (UINT8)(bitfield(gCPUStructure.CPUID[CPUID_1][EBX], 23, 16));
        gCPUStructure.Threads = (UINT8)(gCPUStructure.LogicalPerPackage & 0xff);
        break;
    }    
  }
	if (gCPUStructure.Cores == 0) {
      gCPUStructure.Cores   = (UINT8)(gCPUStructure.CoresPerPackage & 0xff);
      gCPUStructure.Threads = (UINT8)(gCPUStructure.LogicalPerPackage & 0xff);
	}
		
	/* get BrandString (if supported) */
	if(gCPUStructure.CPUID[CPUID_80][EAX] >= 0x80000004){
 
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

	//get Min and Max Ratio Cpu/Bus
	if(gCPUStructure.Vendor == CPU_VENDOR_INTEL && 
     ((gCPUStructure.Family == 0x06 && gCPUStructure.Model >= 0x0c) ||
			(gCPUStructure.Family == 0x0f && gCPUStructure.Model >= 0x03)))
	{
			if (gCPUStructure.Family == 0x06)
			{
				// port from valv branch - start
				fix_fsb = FALSE;
				core_i	= FALSE;
				turbo	= FALSE;
				isatom	= FALSE;
				fsbad	= FALSE;

				switch (gCPUStructure.Model)
				{            
          case CPU_MODEL_SANDY_BRIDGE:// Sandy Bridge, 32nm
          case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
          case CPU_MODEL_FIELDS:// Core i7, i5 LGA1156, "Clarksfield", "Lynnfield", "Jasper", 45nm
          case CPU_MODEL_DALES:// Core i7, i5, Nehalem
          case CPU_MODEL_CLARKDALE:// Core i7, i5, i3 LGA1156, "Westmere", "Clarkdale", , 32nm
          case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
          case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
          case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
          case CPU_MODEL_JAKETOWN:		
            core_i = TRUE;
            msr = AsmReadMsr64(MSR_PLATFORM_INFO);
            gCPUStructure.MaxRatio = (UINT8)(msr >> 8) & 0xff;
            gCPUStructure.MinRatio = (UINT8)(msr >> 40) & 0xff;
            
            if(gCPUStructure.MaxRatio) {
              gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.MaxRatio);
            }
            
            if ((gCPUStructure.Model != CPU_MODEL_NEHALEM_EX)
                && (gCPUStructure.Model != CPU_MODEL_WESTMERE_EX))
            {
              turbo = TRUE;
              msr = AsmReadMsr64(MSR_TURBO_RATIO_LIMIT);
              
              gCPUStructure.Turbo1 = (UINT8)(msr >> 0) & 0xff;
              gCPUStructure.Turbo2 = (UINT8)(msr >> 8) & 0xff;
              gCPUStructure.Turbo3 = (UINT8)(msr >> 16) & 0xff;
              gCPUStructure.Turbo4 = (UINT8)(msr >> 24) & 0xff;
            }
            
            if (turbo && gSettings.Turbo){
              gCPUStructure.CPUFrequency = gCPUStructure.Turbo4 * gCPUStructure.FSBFrequency;
            } else 
              gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
            break;
          case CPU_MODEL_ATOM:// Core i7 | Atom
          case CPU_MODEL_DOTHAN:// Pentium M, Dothan, 90nm
          case CPU_MODEL_YONAH:// Core Duo/Solo, Pentium M DC
          case CPU_MODEL_MEROM:// Core Xeon, Core 2 DC, 65nm
          case CPU_MODEL_PENRYN:// Core 2 Duo/Extreme, Xeon, 45nm
            if(AsmReadMsr64(MSR_IA32_PLATFORM_ID) & (1<<28)){
              gCPUStructure.Mobile = TRUE;
            }
            msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);
            gCPUStructure.MaxRatio = (UINT32)(msr >> 8) & 0xff;
            gCPUStructure.SubDivider = (UINT32)(msr >> 46) & 0x1;
            gCPUStructure.MinRatio = 6;
            gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency * 2,
                                                   gCPUStructure.MaxRatio * 2 + gCPUStructure.SubDivider);
            gCPUStructure.Turbo4 = gCPUStructure.MaxRatio + 1;
            if (gSettings.Turbo){
              gCPUStructure.CPUFrequency = gCPUStructure.Turbo4 * gCPUStructure.FSBFrequency;
            } else 
              gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
            
            break;
          default:	
            gCPUStructure.MinRatio = 6;
            gCPUStructure.MaxRatio = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.FSBFrequency);
            gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
            break;
				}
			}
			else //Family !=6 i.e. Pentium 4 
			{
        msr = AsmReadMsr64(MSR_IA32_PLATFORM_ID);
        if (((msr >> 31) & 0x01) != 0) {
          gCPUStructure.MaxRatio = (UINT8)(msr >> 8) & 0x1f;
          gCPUStructure.MinRatio = gCPUStructure.MaxRatio; //no speedstep
        } else {
          gCPUStructure.MaxRatio = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.FSBFrequency);
        }
        gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
			}
	}
#if 1 //NOTNOW
  else if(gCPUStructure.Vendor == CPU_VENDOR_AMD /* AMD */) 
	{
		if(gCPUStructure.Extfamily == 0x00 /* K8 */)
		{
			msr = AsmReadMsr64(K8_FIDVID_STATUS);
			gCPUStructure.MaxRatio = (msr & 0x3f) / 2 + 4;
			gCPUStructure.SubDivider = (msr & 0x01) * 2;
		}
		else if(gCPUStructure.Extfamily >= 0x01 /* K10+ */)
		{
			msr = AsmReadMsr64(K10_COFVID_STATUS);
			if(gCPUStructure.Extfamily == 0x01 /* K10 */)
				gCPUStructure.MaxRatio = (msr & 0x3f) + 0x10;
			else /* K11+ */
				gCPUStructure.MaxRatio = (msr & 0x3f) + 0x08;
			gCPUStructure.SubDivider = (2 << ((msr >> 6) & 0x07));
		}
    
		if (gCPUStructure.MaxRatio)
		{
			if (gCPUStructure.SubDivider)
			{
				gCPUStructure.FSBFrequency = DivU64x32(
                                               MultU64x32(gCPUStructure.TSCFrequency,
                                                         gCPUStructure.SubDivider),
                                               gCPUStructure.MaxRatio);
		//		DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
			}
			else
			{
				gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.MaxRatio);
			}
      DBG("AMD divider: %d\n", gCPUStructure.MaxRatio);
			gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
		}
	}
  
#endif
  BusSpeed = (UINT32)DivU64x32(gCPUStructure.FSBFrequency, kilo); //Hz -> kHz
     //now check if SMBIOS has ExternalClock = 4xBusSpeed
  if ((BusSpeed > 50*Mega) && (gCPUStructure.ExternalClock > BusSpeed * 3)) {
    gCPUStructure.ExternalClock = BusSpeed;
  } else {
    gCPUStructure.FSBFrequency = MultU64x32(gCPUStructure.ExternalClock, kilo); //kHz -> Hz
  }
	
	if (gCPUStructure.Model >= CPU_MODEL_NEHALEM) {
		//Slice - for Nehalem we can do more calculation as in Cham
    // but this algo almost always wrong
		// thanks to dgobe for i3/i5/i7 bus speed detection
		qpimult = 2; //init
		/* Scan PCI BUS For QPI Frequency */
		Status = gBS->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
		if (!EFI_ERROR(Status))
		{	
			for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++)
			{
				Status = gBS->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
				if (!EFI_ERROR(Status))
				{			
					for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++)
					{
						if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex]))
						{
							Status = gBS->OpenProtocol(HandleBuffer[HandleIndex],&gEfiPciIoProtocolGuid,(VOID **)&PciIo,gImageHandle,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
							if (!EFI_ERROR(Status))
							{
								/* Read PCI BUS */
								Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
								if ((Bus & 0x3F) != 0x3F) {
									continue;
								}
								DBG("Found CPU at bus 0x%02x dev=%x funs=%x\n", Bus, Device, Function);
								Status = PciIo->Pci.Read (
									PciIo,
									EfiPciIoWidthUint32,
									0,
									sizeof (Pci) / sizeof (UINT32),
									&Pci
									);
								vid = Pci.Hdr.VendorId & 0xFFFF;
								did = Pci.Hdr.DeviceId & 0xFF00;
								/*for(i = 0; i < sizeof(possible_nhm_bus); i++)
								{
									vid = (UINT32) MmioRead16(PCIADDR(possible_nhm_bus[i], 3, 4));
									did = (UINT32) MmioRead16(PCIADDR(possible_nhm_bus[i], 3, 4) + 0x02);
									vid &= 0xFFFF;
									did &= 0xFF00;

									if(vid == 0x8086 && did >= 0x2C00)
										nhm_bus = possible_nhm_bus[i]; 
								}*/
								if ((vid == 0x8086) && (did >= 0x2C00)
									&& (Device == 2) && (Function == 1)) {
									Status = PciIo->Mem.Read (
															  PciIo,
															  EfiPciIoWidthUint32,
                                 EFI_PCI_IO_PASS_THROUGH_BAR,           
															  0x50,
															  1,
															  &qpimult
															  );
									DBG("qpi read from PCI %d\n", qpimult);
									if (EFI_ERROR(Status)) continue;
									qpimult &= 0x7F;
									break;
								}
								//qpimult = (UINT16) MmioRead32(PCIADDR(nhm_bus, 2, 1) + 0x50);
								
							}
						}
					}
				}
			}
		}

		DBG("qpimult %d\n", qpimult);
		qpibusspeed = qpimult * 2 * gCPUStructure.ExternalClock; //kHz
		DBG("qpibusspeed %dkHz\n", qpibusspeed);
		gCPUStructure.ProcessorInterconnectSpeed = DivU64x32(qpibusspeed, kilo); //kHz->MHz

	} else {
		gCPUStructure.ProcessorInterconnectSpeed = DivU64x32(gCPUStructure.ExternalClock << 2, kilo); //kHz->MHz
	}
	
	DBG("Vendor/Model/Stepping: 0x%x/0x%x/0x%x\n", gCPUStructure.Vendor, gCPUStructure.Model, gCPUStructure.Stepping);
	DBG("Family/ExtFamily: 0x%x/0x%x\n", gCPUStructure.Family, gCPUStructure.Extfamily);
	DBG("MaxDiv/MinDiv: 0x%x/0x%x\n", gCPUStructure.MaxRatio, gCPUStructure.MinRatio);
  DBG("Turbo1: %d, Turbo4: %d\n", gCPUStructure.Turbo1, gCPUStructure.Turbo4);
	DBG("Features: 0x%08x\n",gCPUStructure.Features);
	DBG("Threads: %d\n",gCPUStructure.Threads);
	DBG("Cores: %d\n",gCPUStructure.Cores);
	DBG("FSB: %d MHz\n",DivU64x32(gCPUStructure.ExternalClock, kilo));
	DBG("CPU: %d MHz\n", DivU64x32(gCPUStructure.CPUFrequency, Mega));
	DBG("TSC: %d MHz\n",gCPUStructure.CurrentSpeed);
	DBG("PIS: %d MHz\n",gCPUStructure.ProcessorInterconnectSpeed);
#if DEBUG_PCI
	
	/* Scan PCI BUS */
	Status = gBS->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
	if (!EFI_ERROR(Status))
	{	
		for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++)
		{
			Status = gBS->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
			if (!EFI_ERROR(Status))
			{			
				for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++)
				{
					if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex]))
					{
						Status = gBS->OpenProtocol(HandleBuffer[HandleIndex],&gEfiPciIoProtocolGuid,(VOID **)&PciIo,gImageHandle,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
						if (!EFI_ERROR(Status))
						{
							/* Read PCI BUS */
							Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
							Status = PciIo->Pci.Read (
													  PciIo,
													  EfiPciIoWidthUint32,
													  0,
													  sizeof (Pci) / sizeof (UINT32),
													  &Pci
													  );
							vid = Pci.Hdr.VendorId & 0xFFFF;
							did = (Pci.Hdr.VendorId >> 16) & 0xFF00;
							//UINT32 class = Pci.Hdr.ClassCode[0];
							DBG("PCI (%02x|%02x:%02x.%02x) : %04x %04x class=%02x%02x%02x\n",
									Segment, Bus, Device, Function,
									Pci.Hdr.VendorId, Pci.Hdr.DeviceId,
									Pci.Hdr.ClassCode[2], Pci.Hdr.ClassCode[1], Pci.Hdr.ClassCode[0]);
						}
					}
				}
			}
		}
	}
//	WaitForKeyPress("waiting for key press...\n");
#endif	

	return;
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
/*
 VOID DumpCPU()
 {
 //	if(AsciiStrStr(gSettings.Debug,"y") || AsciiStrStr(gSettings.Debug,"Y")) {
	Print(L"CPU:	BrandString - %a\n", gCPUStructure.BrandString);
	Print(L"Vendor/Model/ExtModel: 0x%x/0x%x/0x%x\n", gCPUStructure.Vendor,  gCPUStructure.Model, gCPUStructure.Extmodel);
	Print(L"Family/ExtFamily:      0x%x/0x%x\n", gCPUStructure.Family,  gCPUStructure.Extfamily);
	Print(L"MaxCoef/CurrCoef:      0x%x/0x%x\n", gCPUStructure.MaxCoef,  gCPUStructure.CurrCoef);
	Print(L"MaxDiv/CurrDiv:        0x%x/0x%x\n", gCPUStructure.MaxDiv,  gCPUStructure.CurrDiv);
	Print(L"Features: 0x%08x\n",gCPUStructure.Features);
	Print(L"Threads: %d\n",gCPUStructure.Threads);
	Print(L"Cores: %d\n",gCPUStructure.Cores);
	Print(L"FSB: %d MHz\n",gCPUStructure.ExternalClock);
	Print(L"CPU: %d MHz\n",gCPUStructure.MaxSpeed);
	Print(L"TSC: %d MHz\n",gCPUStructure.CurrentSpeed);	
	Print(L"TjMax: %d C\n", tjmax);	
	
		//		Print(L"press key to continue...\n");
	WaitForKeyPress("waiting for key press...\n");
//	}	
}
*/
/*
VOID ShowCPU()
{
	int i;
#define MAX_CPU_PROPS 11	
	CHAR8* buf;
	CHAR8* msg[MAX_CPU_PROPS];
	CHAR8* msgru[MAX_CPU_PROPS];
	for (i=0; i<MAX_CPU_PROPS; i++) {
		msg[i]   = AllocateZeroPool(64);
		msgru[i] = AllocateZeroPool(64);
	}
	buf = AllocateZeroPool(64);
	AsciiSPrint(msg[0], 64, "CPU:	BrandString - %a", gCPUStructure.BrandString);
	AsciiSPrint(msgru[0], 64, "%a - %a", RusToAscii(buf, L"Название ЦПУ"), gCPUStructure.BrandString);
//	PrintMessage(4, msgru[0], MENU_ICON);
	AsciiSPrint(msg[1], 64, "Vendor/Model/ExtModel: 0x%x/0x%x/0x%x",
				gCPUStructure.Vendor,  gCPUStructure.Model, gCPUStructure.Extmodel);
	AsciiSPrint(msgru[1], 64, "%a: 0x%x/0x%x/0x%x", RusToAscii(buf, L"Фирма/Модель/Ревизия"),
				gCPUStructure.Vendor,  gCPUStructure.Model, gCPUStructure.Stepping);
	AsciiSPrint(msg[2], 64, "Family/ExtFamily:      0x%x/0x%x", gCPUStructure.Family,  gCPUStructure.Extfamily);
	AsciiSPrint(msgru[2], 64, "%a: 0x%x/0x%x", RusToAscii(buf, L"Семейство/Расширенное"),
				gCPUStructure.Family,  gCPUStructure.Extfamily);
	AsciiSPrint(msg[3], 64, "MaxCoef/CurrCoef:      0x%x/0x%x", gCPUStructure.MaxCoef,  gCPUStructure.CurrCoef);
	AsciiSPrint(msgru[3], 64, "%a: 0x%x/0x%x", RusToAscii(buf, L"Макс.множитель/Текущий"),
				gCPUStructure.MaxCoef,  gCPUStructure.CurrCoef);
	AsciiSPrint(msg[4], 64, "Features: 0x%08x",gCPUStructure.Features);
	AsciiSPrint(msgru[4], 64, "%a: 0x%08x", RusToAscii(buf, L"Возможности"), gCPUStructure.Features);
	AsciiSPrint(msg[5], 64, "Threads: %d",gCPUStructure.Threads);
	AsciiSPrint(msgru[5], 64, "%a: %d", RusToAscii(buf, L"Число потоков"), gCPUStructure.Threads);
	AsciiSPrint(msg[6], 64, "Cores: %d",gCPUStructure.Cores);
	AsciiSPrint(msgru[6], 64, "%a: %d", RusToAscii(buf, L"Число ядер"), gCPUStructure.Cores);
	AsciiSPrint(msg[7], 64, "FSB: %d MHz",gCPUStructure.ExternalClock);
	AsciiSPrint(msgru[7], 64, "%a: %d MHz", RusToAscii(buf, L"Частота шины"), gCPUStructure.ExternalClock);
	AsciiSPrint(msg[8], 64, "CPU: %d MHz",gCPUStructure.MaxSpeed);
	AsciiSPrint(msgru[8], 64, "%a: %d MHz", RusToAscii(buf, L"Частота процессора"), gCPUStructure.MaxSpeed);
	AsciiSPrint(msg[9], 64, "TSC: %d MHz", gCPUStructure.CurrentSpeed);
	AsciiSPrint(msgru[9], 64, "%a: %d MHz",RusToAscii(buf, L"Частота генератора"), gCPUStructure.CurrentSpeed);
	AsciiSPrint(msg[10], 64, "TjMax: %d%cC", tjmax, 0xB0);
	AsciiSPrint(msgru[10], 64, "%a: %d%cC", RusToAscii(buf, L"Критическая температура"), tjmax, 0xB0);
	
//	PrintTable (msg, msgru, MAX_CPU_PROPS);
	for (i=0; i<MAX_CPU_PROPS; i++) {		
		FreePool(msg[i]);
		FreePool(msgru[i]);
	}
	FreePool(buf);
}
*/
UINT16 GetStandardCpuType()
{
	if (gCPUStructure.Threads >= 4) 
	{	
		return 0x402;   // Quad-Core Xeon
	}
	else if (gCPUStructure.Threads == 1) 
	{	
		return 0x201;   // Core Solo
	};
	return 0x301;   // Core 2 Duo
}

UINT16 GetAdvancedCpuType ()
{	
	if (gCPUStructure.Vendor == 0x756E6547)
	{	
		switch (gCPUStructure.Family) {	
			case 0x06:	
			{			
				switch (gCPUStructure.Model)
				{
					case CPU_MODEL_DOTHAN:// Dothan
					case CPU_MODEL_YONAH: // Yonah
						return 0x201;
					case CPU_MODEL_MEROM: // Merom
					case CPU_MODEL_PENRYN:// Penryn
					case CPU_MODEL_ATOM:  // Atom (45nm)
						return GetStandardCpuType();
						
					case CPU_MODEL_NEHALEM_EX: //Xeon 5300
						return 0x402;
						
					case CPU_MODEL_NEHALEM: // Intel Core i7 LGA1366 (45nm)
						return 0x701; // Core i7
						
					case CPU_MODEL_FIELDS: // Lynnfield, Clarksfield, Jasper
						if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
							return 0x601; // Core i5
						return 0x701; // Core i7
						
					case CPU_MODEL_DALES: // Intel Core i5, i7 LGA1156 (45nm) (Havendale, Auburndale)
						if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
							return 0x602; // Core i5
						return 0x702; // Core i7
						
					//case CPU_MODEL_ARRANDALE:
					case CPU_MODEL_CLARKDALE: // Intel Core i3, i5, i7 LGA1156 (32nm) (Clarkdale, Arrandale)
						
						if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
							return 0x901; // Core i3
						if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
							return 0x601; // Core i5 - (M540 -> 0x0602)
						return 0x701; // Core i7
						
					case CPU_MODEL_WESTMERE: // Intel Core i7 LGA1366 (32nm) 6 Core (Gulftown, Westmere-EP, Westmere-WS)
					case CPU_MODEL_WESTMERE_EX: // Intel Core i7 LGA1366 (45nm) 6 Core ???
						return 0x701; // Core i7
					case CPU_MODEL_SANDY_BRIDGE:
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i3"))
							return 0x903; // Core i3
            if (AsciiStrStr(gCPUStructure.BrandString, "Core(TM) i5"))
              return 0x603; // Core i5
						return 0x703;
				}
			}
		}
		
	}
	return GetStandardCpuType();
}

MACHINE_TYPES GetDefaultModel()
{
  MACHINE_TYPES DefaultType = MacPro31;
	// TODO: Add more CPU models and configure the correct machines per CPU/GFX model
	if(gMobile)
	{
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
				if (gGraphics.Vendor == Nvidia)
				{
					DefaultType = MacBookPro51;
				} else
					DefaultType = MacBook41;
				break;
			default:
				if (gGraphics.Vendor == Nvidia)
				{
					DefaultType = MacBookPro51;
				} else
					DefaultType = MacBook52;
				break;
		}
	}
	else // if(gMobile==FALSE)
	{
		switch (gCPUStructure.Model)
		{
			case CPU_MODEL_CELERON:

				DefaultType = MacMini21;
				break;
	
			case CPU_MODEL_LINCROFT:
				DefaultType = MacMini21;
				break;
			case CPU_MODEL_ATOM:
				DefaultType = MacMini21;
				break;
			case CPU_MODEL_MEROM:
				DefaultType = iMac81;
				break;
			case CPU_MODEL_PENRYN:	
				DefaultType = MacPro31;//speedstep without patching; Hapertown is also a Penryn, according to Wikipedia
				break;
			case CPU_MODEL_NEHALEM:
				DefaultType = MacPro41;
				break;
			case CPU_MODEL_NEHALEM_EX:
				DefaultType = MacPro41;
				break;
			case CPU_MODEL_FIELDS:
				DefaultType = iMac112;
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
				if((AsciiStrStr(gCPUStructure.BrandString, "i3")) || 
				   (AsciiStrStr(gCPUStructure.BrandString, "i5-2390T")) || 
				   (AsciiStrStr(gCPUStructure.BrandString, "i5-2100S")))
				{
					DefaultType = iMac112;
					break;
				}
				if(AsciiStrStr(gCPUStructure.BrandString, "i7"))
				{
					DefaultType = iMac121;
					break;
				}
			case CPU_MODEL_JAKETOWN:
				DefaultType = MacPro41;
				break;
			default:
				DefaultType = MacPro31;
				break;
		}
	}	
	return DefaultType;
}

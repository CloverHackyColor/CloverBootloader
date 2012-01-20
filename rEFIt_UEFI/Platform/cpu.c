/* 

 cpu.c
 implementation for cpu

 Remade by Slice 2011 based on Apple's XNU sources
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


INTN							temp, tjmax=0;
UINT8							gDefaultType; 

/* CPU defines */
#define bit(n)			(1UL << (n))
#define _Bit(n)			(1ULL << n)
#define _HBit(n)		(1ULL << ((n)+32))

#define bitmask(h,l)	((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)	(((x) & bitmask(h,l)) >> l)
#define quad(hi,lo)     (((UINT64)(hi)) << 32 | (lo))
//Slice - should sync with Smbios.h
/* CPU Features */
/*#define CPU_FEATURE_MMX			0x00000001
#define CPU_FEATURE_SSE			0x00000002
#define CPU_FEATURE_SSE2		0x00000004
#define CPU_FEATURE_SSE3		0x00000008
#define CPU_FEATURE_SSE41		0x00000010
#define CPU_FEATURE_SSE42		0x00000020
#define CPU_FEATURE_EM64T		0x00000040
#define CPU_FEATURE_HTT			0x00000080
#define CPU_FEATURE_MOBILE		0x00000100
#define CPU_FEATURE_MSR			0x00000200
*/
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
#define MSR_CORE_THREAD_COUNT       0x0035	 /* limited use - not for Penryn or older			*/
#define MSR_FSB_FREQ                0x00CD	 /* limited use - not for i7						*/
#define	MSR_PLATFORM_INFO           0x00CE   /* limited use - MinRatio for i7 but Max for Yonah	*/
#define MSR_IA32_EXT_CONFIG         0x00EE	 /* limited use - not for i7						*/
#define MSR_FLEX_RATIO              0x0194	 /* limited use - not for Penryn or older			*/
#define	MSR_IA32_PERF_STATUS        0x0198
#define MSR_IA32_PERF_CONTROL       0x0199
#define MSR_IA32_CLOCK_MODULATION   0x019A
#define MSR_THERMAL_STATUS          0x019C
#define MSR_IA32_MISC_ENABLE        0x01A0
#define MSR_THERMAL_TARGET          0x01A2	 /* limited use - not for Penryn or older			*/
#define MSR_TURBO_RATIO_LIMIT       0x01AD	 /* limited use - not for Penryn or older			*/

VOID
DoCpuid(UINT32 selector, UINT32 *data)
{
	AsmCpuid(selector, data, data+1, data+2, data+3);
}

UINT64 GetCPUProperties (VOID)
{
	INT32		i = 0;
	UINT32		reg[4];
	UINT64		msr = 0;
	UINT64		flex_ratio = 0;
	UINT8		XE = 0;
	BOOLEAN		fix_fsb, core_i, turbo, isatom, fsbad;
	EFI_STATUS			Status;
	EFI_HANDLE			*HandleBuffer;
	EFI_GUID			**ProtocolGuidArray;
	EFI_PCI_IO_PROTOCOL *PciIo;
	PCI_TYPE00          Pci;
	UINTN				HandleCount;
	UINTN				ArrayCount;
	UINTN				HandleIndex;
	UINTN				ProtocolIndex;
	UINT16				qpibusspeed; //units=MHz
	UINT16				qpimult = 2;
	//UINT8				nhm_bus = 0x3F;
	//UINT8				possible_nhm_bus[] = {0xFF, 0x7F, 0x3F};
	UINT16				did, vid;
	UINTN                     Segment;
	UINTN                     Bus;
	UINTN                     Device;
	UINTN                     Function;
  CHAR8	str[128], *s;
	
	gCPUStructure.MaxCoef=1;
	gCPUStructure.MaxDiv=1;
	gCPUStructure.CurrCoef=1;
	gCPUStructure.CurrDiv=1;
	gCPUStructure.BusRatioMax=1;
	gCPUStructure.BusRatioMin=1;
	gCPUStructure.MaxRatio=1;
	gCPUStructure.MinRatio=1;
	gCPUStructure.FlexRatio=1;
	gCPUStructure.DID=0;
//	gCPUStructure.ExternalClock=100; //get from PrepatchSmbios
	gCPUStructure.MaxSpeed=0;
//	gCPUStructure.CurrentSpeed=1000; //get from PrepatchSmbios
	gCpuSpeed = gCPUStructure.CurrentSpeed;
	gCPUStructure.UserSetting=0;
//	gCPUStructure.FrontSideBus=0;	//what?
	gCPUStructure.FSBFrequency=gCPUStructure.ExternalClock  * 1000000ull;
	gCPUStructure.CPUFrequency=0;
	gCPUStructure.TSCFrequency=gCPUStructure.CurrentSpeed * 1000000ull;
	gCPUStructure.ProcessorInterconnectSpeed=0;
	gCPUStructure.Mobile = FALSE; //not same as gMobile

	/* get CPUID Values */
	for(i = 0; i < 4; i++)
	{
		DoCpuid(i, gCPUStructure.CPUID[i]);
	}
	DoCpuid(0x80000000, gCPUStructure.CPUID[CPUID_80]);
	if((gCPUStructure.CPUID[CPUID_80][0] & 0x0000000f) >= 1){
		DoCpuid(0x80000001, gCPUStructure.CPUID[CPUID_81]); 
	}
	
	gCPUStructure.Vendor	= gCPUStructure.CPUID[CPUID_0][1];
	gCPUStructure.Signature = gCPUStructure.CPUID[CPUID_1][0];
	gCPUStructure.Stepping	= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][0], 3, 0);
	gCPUStructure.Model		= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][0], 7, 4);
	gCPUStructure.Family	= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][0], 11, 8);
	gCPUStructure.Type		= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][0], 13, 12);
	gCPUStructure.Extmodel	= (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][0], 19, 16);
	gCPUStructure.Extfamily = (UINT8) bitfield(gCPUStructure.CPUID[CPUID_1][0], 27, 20);
	gCPUStructure.Features  = quad(gCPUStructure.CPUID[CPUID_1][2], gCPUStructure.CPUID[CPUID_1][3]);
	gCPUStructure.ExtFeatures  = quad(gCPUStructure.CPUID[CPUID_81][2], gCPUStructure.CPUID[CPUID_81][3]);
	/* Pack CPU Family */
	if (gCPUStructure.Family == 0x0f) {
		gCPUStructure.Family += gCPUStructure.Extfamily;
	}
	gCPUStructure.Model += (gCPUStructure.Extmodel << 4);
	if (gCPUStructure.Features & CPUID_FEATURE_HTT) {
		gCPUStructure.LogicalPerPackage	= bitfield(gCPUStructure.CPUID[CPUID_1][1], 23, 16);
	} else {
		gCPUStructure.LogicalPerPackage	= 1;
	}

	gCPUStructure.CoresPerPackage =  bitfield(gCPUStructure.CPUID[CPUID_4][0], 31, 26) + 1;
	if (gCPUStructure.CoresPerPackage == 0) {
		gCPUStructure.CoresPerPackage = 1;
	}
	switch (gCPUStructure.Model)
	{
		case CPU_MODEL_ATOM: 
		{
			gCPUStructure.Cores   = 1; 
			gCPUStructure.Threads = 2; 
		}
			break;
		case CPU_MODEL_NEHALEM: // Intel Core i7 LGA1366 (45nm)
		case CPU_MODEL_FIELDS: // Intel Core i5, i7 LGA1156 (45nm)
		case CPU_MODEL_DALES:
		case CPU_MODEL_NEHALEM_EX:	
		case CPU_MODEL_JAKETOWN:
		case CPU_MODEL_SANDY_BRIDGE:
					
		{
			msr = AsmReadMsr64(MSR_CORE_THREAD_COUNT);
			gCPUStructure.Cores   = (UINT8)bitfield((UINT32)msr, 31, 16);
			gCPUStructure.Threads = (UINT8)bitfield((UINT32)msr, 15,  0);
		} break;
		case CPU_MODEL_CLARKDALE: // Intel Core i3, i5, i7 LGA1156 (32nm)
		case CPU_MODEL_WESTMERE: // Intel Core i7 LGA1366 (32nm) 6 Core
		case CPU_MODEL_WESTMERE_EX:

		{
			msr = AsmReadMsr64(MSR_CORE_THREAD_COUNT);
			gCPUStructure.Cores   = (UINT8)bitfield((UINT32)msr, 19, 16);
			gCPUStructure.Threads = (UINT8)bitfield((UINT32)msr, 15,  0);
			break;
		}
			
		default:		
			gCPUStructure.Cores   = (UINT8)(gCPUStructure.CoresPerPackage & 0xff);
			gCPUStructure.Threads = (UINT8)(gCPUStructure.LogicalPerPackage & 0xff);
			break;
	}
	if (gCPUStructure.Cores == 0) {
		gCPUStructure.Cores   = (UINT8)(gCPUStructure.CoresPerPackage & 0xff);
		gCPUStructure.Threads = (UINT8)(gCPUStructure.LogicalPerPackage & 0xff);
	}
	if (gCPUStructure.Cores == 0) {
		gCPUStructure.Cores = 1;
		gCPUStructure.Threads = 1;
	}
		
	/* get BrandString (if supported) */
	if(gCPUStructure.CPUID[CPUID_80][0] >= 0x80000004){
 
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
//Slice - it is from config.plist. Not sure it is not dangerous
	gCPUStructure.UserSetting=(UINTN)AsciiStrDecimalToUint64(gSettingsFromMenu.CpuFreqMHz);

	if(gCPUStructure.UserSetting>0)
		gCPUStructure.TSCFrequency=MultU64x64(gCPUStructure.UserSetting, 1000000);
	
	if(gCPUStructure.Vendor == 0x756E6547 && ((gCPUStructure.Family == 0x06 && gCPUStructure.Model >= 0x0c)
										|| (gCPUStructure.Family == 0x0f && gCPUStructure.Model >= 0x03)))
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
					tjmax = (UINT8)(AsmReadMsr64(MSR_THERMAL_TARGET) >> 16) & 0xff;
					msr = AsmReadMsr64(MSR_PLATFORM_INFO);
					gCPUStructure.BusRatioMax = (UINT8)(msr >> 8) & 0xff;
					gCPUStructure.BusRatioMin = (UINT8)(msr >> 40) & 0xff;
					DBG("CPU: Flex-Ratio = %d ", gCPUStructure.BusRatioMax);
					gCPUStructure.MinRatio = gCPUStructure.BusRatioMin * 10;
					msr = AsmReadMsr64(MSR_FLEX_RATIO);
					//msr = 0;
					if ((msr >> 16) & 0x01)
					{
						flex_ratio = (msr >> 8) & 0xff;
						DBG(">> %d", flex_ratio);
						if (flex_ratio == 0) { 
							AsmWriteMsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL)); 
							msr = AsmReadMsr64(MSR_FLEX_RATIO);
							DBG(" Unusable Flex-Ratio detected! Patched MSR: %08x", msr & 0xffffffff);
						} else { 
						if(gCPUStructure.BusRatioMax > flex_ratio) 
							gCPUStructure.BusRatioMax = (UINT8)flex_ratio;
						}
					}
					DBG("\n");
					if(gCPUStructure.BusRatioMax) 
						gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.BusRatioMax);
					if ((gCPUStructure.Model != CPU_MODEL_NEHALEM_EX)
						&& (gCPUStructure.Model != CPU_MODEL_WESTMERE_EX))
					{
						turbo = TRUE;
						msr = AsmReadMsr64(MSR_TURBO_RATIO_LIMIT);

						gCPUStructure.Turbo1 = (UINT8)(msr >> 0) & 0xff;
						gCPUStructure.Turbo2 = (UINT8)(msr >> 8) & 0xff;
						gCPUStructure.Turbo3 = (UINT8)(msr >> 16) & 0xff;
						gCPUStructure.Turbo4 = (UINT8)(msr >> 24) & 0xff;

						gCPUStructure.CPUFrequency = gCPUStructure.BusRatioMax * gCPUStructure.FSBFrequency;
						gCPUStructure.MaxRatio = gCPUStructure.BusRatioMax * 10;
					}
					else 
						gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
					break;
				case CPU_MODEL_ATOM:// Core i7 & Atom
					switch (gCPUStructure.Stepping)
					{
						case 0xa:
							tjmax = 100;
							break;
						case 0x2:
						default:
							tjmax = 90;
							break;
					}
					break;	
				case CPU_MODEL_DOTHAN:// Pentium M, Dothan, 90nm
				case CPU_MODEL_YONAH:// Core Duo/Solo, Pentium M DC
					if(AsmReadMsr64(MSR_IA32_PLATFORM_ID) & (1<<28)){
						gCPUStructure.Mobile = TRUE;
					}
						
					tjmax = 80;  //Slice - initial value
					msr = AsmReadMsr64(MSR_IA32_EXT_CONFIG);
					if(msr & (1 << 30)) 
						tjmax = 85;
					break;
				case CPU_MODEL_MEROM:// Core Xeon, Core 2 DC, 65nm
					if(AsmReadMsr64(MSR_IA32_PLATFORM_ID) & (1<<28)){
						gCPUStructure.Mobile = TRUE;
					}	
						
					switch (gCPUStructure.Stepping)
					{
					case 0x2:
						tjmax = 95;
						break;
					case 0x6:
						if (gCPUStructure.Cores == 2) 
							tjmax = 80;
						if (gCPUStructure.Cores == 4) 
							tjmax = 90;
						else 
							tjmax = 85;
						break;
					case 0xb:
						tjmax = 90;
						break;
					case 0xd:
					default:
						msr = AsmReadMsr64(MSR_IA32_EXT_CONFIG);
						if(msr & (1 << 30)) 
							tjmax = 85;
						break;
					}
					break;	
				case CPU_MODEL_PENRYN:// Core 2 Duo/Extreme, Xeon, 45nm
					if(AsmReadMsr64(MSR_IA32_PLATFORM_ID) & (1<<28)){
						gCPUStructure.Mobile = TRUE;
					}
					tjmax = 100;	
					switch (gCPUStructure.Stepping)
					{
					case 0x7:
						tjmax = 95;
						break;
		
					case 0x6:// Mobile Core2 Duo
						tjmax = 104;
						break;
					case 0xa:// Mobile Centrino 2
						tjmax = 105;
						break;
					default:
						if (gCPUStructure.Mobile) 
							tjmax = 105;
						else tjmax = 100;
						break;
					}
					break;	
				case CPU_MODEL_CELERON:// Celeron, Core 2 SC, 65nm
				case CPU_MODEL_LINCROFT:// Atom Lincroft, 45nm
					core_i = FALSE;
					if (AsmReadMsr64(MSR_IA32_EXT_CONFIG) & (1 << 27))
					{
						AsmWriteMsr64(MSR_IA32_EXT_CONFIG, (AsmReadMsr64(MSR_IA32_EXT_CONFIG) | (1 << 28)));
						gCPUStructure.DID = AsmReadMsr64(MSR_IA32_EXT_CONFIG) & (1 << 28);
					}
				case CPU_MODEL_XEON_MP:// Xeon MP MP 7400
				case 0x2b:// SNB Xeon //XXX
					if ((gCPUStructure.Model != 0x2a) || (gCPUStructure.Model != 0x2b))
						turbo = TRUE;
				default:
					
					break;
				}
			}
			else //Family !=6 i.e. Pentium 4 or AMD or VIA
			{
				XE = (UINT8)(msr >> 31) & 0x01;
			}
			
			msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);
			gCPUStructure.MaxDiv = (UINT8)(msr >> 46) & 0x01;
			gCPUStructure.CurrDiv = (UINT8)(msr >> 15) & 0x01;
			gCPUStructure.BusRatioMin = (UINT8)(msr >> 24) & 0x1f;
			gCPUStructure.MinRatio = gCPUStructure.BusRatioMin * 10;
			if(gCPUStructure.CurrDiv) 
				gCPUStructure.MinRatio = gCPUStructure.MinRatio + 5;
			if(XE || (gCPUStructure.Family == 0x0f)) 
				gCPUStructure.BusRatioMax = (UINT8)(msr >> (40-32)) & 0x1f;
			else 
				gCPUStructure.BusRatioMax = ((UINT8)(AsmReadMsr64(MSR_IA32_PLATFORM_ID) >> 8) & 0x1f);
			
			// port from valv branch - end

			/* Nehalem CPU model */
			if (gCPUStructure.Family == 0x06 &&
				(gCPUStructure.Model == CPU_MODEL_NEHALEM || gCPUStructure.Model == CPU_MODEL_FIELDS 
				|| gCPUStructure.Model == CPU_MODEL_DALES || gCPUStructure.Model == CPU_MODEL_CLARKDALE
				|| gCPUStructure.Model == CPU_MODEL_WESTMERE)) {

					if(gCPUStructure.BusRatioMin == 0) 
						gCPUStructure.BusRatioMin = gCPUStructure.BusRatioMax;

					msr = AsmReadMsr64(MSR_PLATFORM_INFO);
					gCPUStructure.CurrCoef = (UINT8)(msr >> 8) & 0xff;
					msr = AsmReadMsr64(MSR_FLEX_RATIO);
					if ((msr >> 16) & 0x01)
					{
						gCPUStructure.FlexRatio = (UINT8)(msr >> 8) & 0xff;
						if (gCPUStructure.CurrCoef > gCPUStructure.FlexRatio)
							gCPUStructure.CurrCoef = gCPUStructure.FlexRatio;
					}

					if (gCPUStructure.CurrCoef)
					{
						gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency,gCPUStructure.CurrCoef);
					}
					gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
			}
			else
			{
				msr = AsmReadMsr64(MSR_IA32_PERF_STATUS);
				gCPUStructure.CurrCoef = (UINT8)(msr >> 8) & 0x1f;
				/* Non-integer bus ratio for the max-multi*/
				gCPUStructure.MaxDiv = (UINT8)(msr >> 46) & 0x01;
				/* Non-integer bus ratio for the current-multi (undocumented)*/
				gCPUStructure.CurrDiv = (UINT8)(msr >> 14) & 0x01;

				if ((gCPUStructure.Family == 0x06 && gCPUStructure.Model >= 0x0e) || gCPUStructure.Family == 0x0f)
				{
					/* On these models, maxcoef defines TSC freq */
					gCPUStructure.MaxCoef = (UINT8)(msr >> 40) & 0x1f;
				}
				else
				{
					/* On lower models, currcoef defines TSC freq */
					gCPUStructure.MaxCoef = gCPUStructure.CurrCoef;
				}

				if (gCPUStructure.MaxCoef)
				{
					if (gCPUStructure.MaxDiv)
						gCPUStructure.CPUFrequency = DivU64x32(MultU64x32(gCPUStructure.TSCFrequency,2), (gCPUStructure.MaxCoef * 2) + 1);
					else
						gCPUStructure.FSBFrequency = DivU64x32(gCPUStructure.TSCFrequency, gCPUStructure.MaxCoef);

					if (gCPUStructure.CurrDiv)
						gCPUStructure.CPUFrequency = MultU64x32(gCPUStructure.FSBFrequency, ((gCPUStructure.CurrCoef * 2) + 1) / 2);
					else
						gCPUStructure.CPUFrequency = MultU64x32(gCPUStructure.FSBFrequency, gCPUStructure.CurrCoef);
				}
			}		
	}

//	if(gCPUStructure.FSBFrequency==0)
//		gCPUStructure.FSBFrequency=100*1000000;
	
//	gCPUStructure.ExternalClock = (UINT16)DivU64x32(gCPUStructure.FSBFrequency,1000000); //Hz -> MHz
	
	if (gCPUStructure.Model >= CPU_MODEL_NEHALEM) {
		//Slice - for Nehalem we can do more calculation as in Cham
		// thanks to dgobe for i3/i5/i7 bus speed detection
		qpimult = 2; //init
		/* Scan PCI BUS For QPI Frequency */
		Status = gBootServices->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
		if (!EFI_ERROR(Status))
		{	
			for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++)
			{
				Status = gBootServices->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
				if (!EFI_ERROR(Status))
				{			
					for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++)
					{
						if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex]))
						{
							Status = gBootServices->OpenProtocol(HandleBuffer[HandleIndex],&gEfiPciIoProtocolGuid,(VOID **)&PciIo,gImageHandle,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
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
		qpibusspeed = qpimult * 2 * gCPUStructure.ExternalClock;
		// Rek: rounding decimals to match original mac profile info
		if (qpibusspeed%100 != 0)qpibusspeed = ((qpibusspeed+50)/100)*100;
		DBG("qpibusspeed %d\n", qpibusspeed);
		gCPUStructure.ProcessorInterconnectSpeed = qpibusspeed;

	} else {
		int res = gCPUStructure.ExternalClock % 10;
		gCPUStructure.ProcessorInterconnectSpeed = (gCPUStructure.ExternalClock << 2) + res / 3;
	}
	
	gCPUStructure.MaxSpeed=(UINT16)DivU64x32(gCPUStructure.CPUFrequency, 1000000);
	gCPUStructure.CurrentSpeed=(UINT16)DivU64x32(gCPUStructure.TSCFrequency, 1000000);
	DBG("Vendor/Model/Stepping: 0x%x/0x%x/0x%x\n", gCPUStructure.Vendor, gCPUStructure.Model, gCPUStructure.Stepping);
	DBG("Family/ExtFamily: 0x%x/0x%x\n", gCPUStructure.Family, gCPUStructure.Extfamily);
	DBG("MaxCoef/CurrCoef: 0x%x/0x%x\n", gCPUStructure.MaxCoef, gCPUStructure.CurrCoef);
	DBG("MaxDiv/CurrDiv: 0x%x/0x%x\n", gCPUStructure.MaxDiv, gCPUStructure.CurrDiv);
	DBG("Features: 0x%08x\n",gCPUStructure.Features);
	DBG("Threads: %d\n",gCPUStructure.Threads);
	DBG("Cores: %d\n",gCPUStructure.Cores);
	DBG("FSB: %d MHz\n",gCPUStructure.ExternalClock);
	DBG("CPU: %d MHz\n",gCPUStructure.MaxSpeed);
	DBG("TSC: %d MHz\n",gCPUStructure.CurrentSpeed);
	DBG("PIS: %d MHz\n",gCPUStructure.ProcessorInterconnectSpeed);
	DBG("TjMax: %d C\n", tjmax);
#if DEBUG_PCI
	
	/* Scan PCI BUS */
	Status = gBootServices->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
	if (!EFI_ERROR(Status))
	{	
		for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++)
		{
			Status = gBootServices->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
			if (!EFI_ERROR(Status))
			{			
				for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++)
				{
					if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex]))
					{
						Status = gBootServices->OpenProtocol(HandleBuffer[HandleIndex],&gEfiPciIoProtocolGuid,(VOID **)&PciIo,gImageHandle,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
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

	return gCPUStructure.FSBFrequency;
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
 //	if(AsciiStrStr(gSettingsFromMenu.Debug,"y") || AsciiStrStr(gSettingsFromMenu.Debug,"Y")) {
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
							return 0x601; // Core i5
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
						return 0x703;
				}
			}
		}
		
	}
	return GetStandardCpuType();
}

UINT16 GetDefaultModel()
{
	//Slice - move this after PrepatchSMBIOS
	// TODO: Add more CPU models and configure the correct machines per CPU/GFX model
	if(gMobile)
	{
		switch (gCPUStructure.Model)
		{
			case CPU_MODEL_ATOM:
				gDefaultType = MacBookAir11;
				break;
			case CPU_MODEL_DOTHAN:	
				gDefaultType = MacBook11;
				break;
			case CPU_MODEL_YONAH: 
				gDefaultType = MacBook11;
				break;
			case CPU_MODEL_MEROM: 
				gDefaultType = MacBook21;
				break;
			case CPU_MODEL_PENRYN:
				if (gGraphicsCard.Nvidia)
				{
					gDefaultType = MacBookPro51;
				} else
					gDefaultType = MacBook41;
				break;
			default:
				gDefaultType = MacBook52;
				break;
		}
	}
	else // if(gCPUStructure.Mobile==FALSE)
	{
		switch (gCPUStructure.Model)
		{
			case CPU_MODEL_CELERON:

				gDefaultType = MacMini21;
				break;
	
			case CPU_MODEL_LINCROFT:
				gDefaultType = MacMini21;
				break;
			case CPU_MODEL_ATOM:
				gDefaultType = MacMini21;
				break;
			case CPU_MODEL_MEROM:
				gDefaultType = iMac81;
				break;
			case CPU_MODEL_PENRYN:	
				gDefaultType = iMac101;
				break;
			case CPU_MODEL_NEHALEM:
				gDefaultType = MacPro41;
				break;
			case CPU_MODEL_NEHALEM_EX:
				gDefaultType = MacPro41;
				break;
			case CPU_MODEL_FIELDS:
				gDefaultType = iMac112;
				break;
			case CPU_MODEL_DALES: 
				gDefaultType = iMac112;
				break;
			case CPU_MODEL_CLARKDALE:
				gDefaultType = iMac112;
				break;
			case CPU_MODEL_WESTMERE:
				gDefaultType = MacPro51;
				break;
			case CPU_MODEL_WESTMERE_EX:
				gDefaultType = MacPro51;
				break;
			case CPU_MODEL_SANDY_BRIDGE:
				if((AsciiStrStr(gCPUStructure.BrandString, "i3")) || 
				   (AsciiStrStr(gCPUStructure.BrandString, "i5-2390T")) || 
				   (AsciiStrStr(gCPUStructure.BrandString, "i5-2100S")))
				{
					gDefaultType = iMac112;
					break;
				}
				if(AsciiStrStr(gCPUStructure.BrandString, "i7"))
				{
					gDefaultType = iMac121;
					break;
				}
			case CPU_MODEL_JAKETOWN:
				gDefaultType = MacPro41;
				break;
			default:
				gDefaultType = MacPro31;
				break;
		}
	}	
	return gDefaultType;
}

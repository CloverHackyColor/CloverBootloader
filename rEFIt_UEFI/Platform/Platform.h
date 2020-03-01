/*
Headers collection for procedures
*/

#ifndef __REFIT_PLATFORM_H__
#define __REFIT_PLATFORM_H__

// Set all debug options - apianti
// Uncomment to set all debug options
// Comment to use source debug options
//#define DEBUG_ALL 2

#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi.h>

#include <Guid/Acpi.h>
#include <Guid/EventGroup.h>
#include <Guid/SmBios.h>
#include <Guid/Mps.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DeviceTreeLib.h>
#include <Library/GenericBdsLib.h>
#include <Library/HiiLib.h>
#include <Library/HdaModels.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UsbMass.h>
#include <Library/VideoBiosPatchLib.h>
#include <Library/MemLogLib.h>
#include <Library/WaveLib.h>

#include <Framework/FrameworkInternalFormRepresentation.h>

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/AppleHid.h>
#include <IndustryStandard/AppleSmBios.h>
#include <IndustryStandard/AppleFeatures.h>
#include <IndustryStandard/Bmp.h>
#include <IndustryStandard/HdaCodec.h>

#include <Protocol/PciIo.h>
#include <Protocol/AudioIo.h>
#include <Protocol/Cpu.h>
#include <Protocol/CpuIo.h>
#include <Protocol/DataHub.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/EdidOverride.h>
#include <Protocol/FrameworkHii.h>
#include <Protocol/HdaIo.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/Smbios.h>
#include <Protocol/VariableWrite.h>
#include <Protocol/Variable.h>
#include <Protocol/UgaDraw.h>

#include <Protocol/FSInjectProtocol.h>
#include <Protocol/MsgLog.h>
#include <Protocol/efiConsoleControl.h>
#include <Protocol/EmuVariableControl.h>
#include <Protocol/AppleSMC.h>
#include <Protocol/AppleImageCodecProtocol.h>

#ifdef __cplusplus
}
#endif


// cpp_foundation objects has to be included before lib.h
#ifdef __cplusplus
#include "../cpp_foundation/XStringW.h"
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XObjArray.h"
#include "../cpp_util/remove_ref.h"
#endif

#include "../refit/lib.h"
#include "string.h"
#include "boot.h"
//#include "PiBootMode.h"
#ifndef CLOVERAPPLICATION
#include "../refit/IO.h"
#endif

#include "device_inject.h"

#ifdef __cplusplus
#include "kext_inject.h"
//#include "entry_scan.h"
#endif

#define CLOVER_SIGN             SIGNATURE_32('C','l','v','r')
#define NON_APPLE_SMC_SIGNATURE SIGNATURE_64('S','M','C','H','E','L','P','E')

#define PCAT_RTC_ADDRESS_REGISTER 0x70
#define PCAT_RTC_DATA_REGISTER    0x71

#ifdef _MSC_VER
#define __typeof__(x) decltype(x)
#endif
#define __typeof_am__(x) remove_ref<decltype(x)>::type


/* XML Tags */
#define kXMLTagPList     "plist"
#define kXMLTagDict      "dict"
#define kXMLTagKey       "key"
#define kXMLTagString    "string"
#define kXMLTagInteger   "integer"
#define kXMLTagData      "data"
#define kXMLTagDate      "date"
#define kXMLTagFalse     "false/"
#define kXMLTagTrue      "true/"
#define kXMLTagArray     "array"
#define kXMLTagReference "reference"
#define kXMLTagID        "ID="
#define kXMLTagIDREF     "IDREF="

#define MAX_NUM_DEVICES  64

#define HEIGHT_2K 1100

/* Decimal powers: */
#define kilo (1000ULL)
#define Mega (kilo * kilo)
#define Giga (kilo * Mega)
#define Tera (kilo * Giga)
#define Peta (kilo * Tera)

#define EBDA_BASE_ADDRESS            0x40E
#define EFI_SYSTEM_TABLE_MAX_ADDRESS 0xFFFFFFFF
#define ROUND_PAGE(x)                ((((unsigned)(x)) + EFI_PAGE_SIZE - 1) & ~(EFI_PAGE_SIZE - 1))

//
// Max bytes needed to represent ID of a SCSI device
//
#define EFI_SCSI_TARGET_MAX_BYTES (0x10)

//
// bit5..7 are for Logical unit number
// 11100000b (0xe0)
//
#define EFI_SCSI_LOGICAL_UNIT_NUMBER_MASK 0xe0

//
// Scsi Command Length
//
#define EFI_SCSI_OP_LENGTH_SIX      0x6
#define EFI_SCSI_OP_LENGTH_TEN      0xa
#define EFI_SCSI_OP_LENGTH_SIXTEEN  0x10

//#define SAFE_LOG_SIZE  80

#define MSG_LOG_SIZE (256 * 1024)
#define PREBOOT_LOG  L"EFI\\CLOVER\\misc\\preboot.log"
#define LEGBOOT_LOG  L"EFI\\CLOVER\\misc\\legacy_boot.log"
#define BOOT_LOG     L"EFI\\CLOVER\\misc\\boot.log"
#define SYSTEM_LOG   L"EFI\\CLOVER\\misc\\system.log"
#define DEBUG_LOG    L"EFI\\CLOVER\\misc\\debug.log"
#define PREWAKE_LOG  L"EFI\\CLOVER\\misc\\prewake.log"
//#define MsgLog(x...) {AsciiSPrint(msgCursor, MSG_LOG_SIZE, x); while(*msgCursor){msgCursor++;}}
//#define MsgLog(...)  {AsciiSPrint(msgCursor, (MSG_LOG_SIZE-(msgCursor-msgbuf)), __VA_ARGS__); while(*msgCursor){msgCursor++;}}
#ifndef DEBUG_ALL
#define MsgLog(...)  DebugLog(1, __VA_ARGS__)
#else
#define MsgLog(...)  DebugLog(DEBUG_ALL, __VA_ARGS__)
#endif

#define CPU_MODEL_PENTIUM_M     0x09
#define CPU_MODEL_DOTHAN        0x0D
#define CPU_MODEL_YONAH         0x0E
#define CPU_MODEL_MEROM         0x0F  /* same as CONROE but mobile */
#define CPU_MODEL_CONROE        0x0F  /* Allendale, Conroe, Kentsfield, Woodcrest, Clovertown, Tigerton */
#define CPU_MODEL_CELERON       0x16  /* ever see? */
#define CPU_MODEL_PENRYN        0x17  /* Yorkfield, Harpertown, Penryn M */
#define CPU_MODEL_WOLFDALE      0x17  /* kind of penryn but desktop */
#define CPU_MODEL_NEHALEM       0x1A  /* Bloomfield. Nehalem-EP, Nehalem-WS, Gainestown */
#define CPU_MODEL_ATOM          0x1C  /* Pineview UN */
#define CPU_MODEL_XEON_MP       0x1D  /* MP 7400 UN */
#define CPU_MODEL_FIELDS        0x1E  /* Lynnfield, Clarksfield, Jasper */
#define CPU_MODEL_DALES         0x1F  /* Havendale, Auburndale */
#define CPU_MODEL_CLARKDALE     0x25  /* Clarkdale, Arrandale */
#define CPU_MODEL_ATOM_SAN      0x26  /* Haswell H ? */
#define CPU_MODEL_LINCROFT      0x27  /* UN */
#define CPU_MODEL_SANDY_BRIDGE  0x2A
#define CPU_MODEL_WESTMERE      0x2C  /* Gulftown LGA1366 */
#define CPU_MODEL_JAKETOWN      0x2D  /* Sandy Bridge Xeon LGA2011 */
#define CPU_MODEL_NEHALEM_EX    0x2E
#define CPU_MODEL_WESTMERE_EX   0x2F
#define CPU_MODEL_ATOM_Z8000    0x35
#define CPU_MODEL_ATOM_2000     0x36  /* UN */
#define CPU_MODEL_ATOM_3700     0x37  /* Bay Trail */
#define CPU_MODEL_IVY_BRIDGE    0x3A
#define CPU_MODEL_HASWELL       0x3C  /* Haswell DT */
#define CPU_MODEL_HASWELL_U5    0x3D  /* Haswell U5  5th generation Broadwell*/
#define CPU_MODEL_IVY_BRIDGE_E5 0x3E  /* Ivy Bridge Xeon UN */
#define CPU_MODEL_HASWELL_E     0x3F  /* Haswell Extreme */
//#define CPU_MODEL_HASWELL_H    0x??  // Haswell H
#define CPU_MODEL_HASWELL_ULT   0x45  /* Haswell ULT */
#define CPU_MODEL_CRYSTALWELL   0x46  /* Haswell ULX CPUID_MODEL_CRYSTALWELL */
#define CPU_MODEL_BROADWELL_HQ  0x47  /* E3-1200 v4 */
#define CPU_MODEL_MERRIFIELD    0x4A  /* Tangier */
#define CPU_MODEL_AIRMONT       0x4C  /* CherryTrail / Braswell */
#define CPU_MODEL_AVOTON        0x4D  /* Avaton/Rangely */
#define CPU_MODEL_SKYLAKE_U     0x4E  /* Skylake Mobile */
#define CPU_MODEL_BROADWELL_E5  0x4F  /* Xeon E5-2695 */
#define CPU_MODEL_SKYLAKE_S     0x55  /* Skylake Server, Cooper Lake */
#define CPU_MODEL_BROADWELL_DE  0x56  /* Xeon BroadWell */
#define CPU_MODEL_KNIGHT        0x57  /* Knights Landing */
#define CPU_MODEL_MOOREFIELD    0x5A  /* Annidale */
#define CPU_MODEL_GOLDMONT      0x5C  /* Apollo Lake */
#define CPU_MODEL_ATOM_X3       0x5D  /* Silvermont */
#define CPU_MODEL_SKYLAKE_D     0x5E  /* Skylake Desktop */
#define CPU_MODEL_DENVERTON     0x5F  /* Goldmont Microserver */
#define CPU_MODEL_CANNONLAKE    0x66
#define CPU_MODEL_ICELAKE_A     0x6A  /* Xeon Ice Lake */
#define CPU_MODEL_ICELAKE_C     0x6C  /* Xeon Ice Lake */
#define CPU_MODEL_ATOM_GM       0x7A  /* Goldmont Plus */
#define CPU_MODEL_ICELAKE_D     0x7D
#define CPU_MODEL_ICELAKE       0x7E
#define CPU_MODEL_XEON_MILL     0x85  /* Knights Mill */
#define CPU_MODEL_ATOM_TM       0x86  /* Tremont */
#define CPU_MODEL_KABYLAKE1     0x8E  /* Kabylake Mobile */
#define CPU_MODEL_KABYLAKE2     0x9E  /* Kabylake Dektop, CoffeeLake */
#define CPU_MODEL_COMETLAKE     0xA6

#define CPU_VENDOR_INTEL        0x756E6547
#define CPU_VENDOR_AMD          0x68747541
/* Unknown CPU */
#define CPU_STRING_UNKNOWN      "Unknown CPU Type"

//definitions from Apple XNU

/* CPU defines */
#define bit(n)                  (1UL << (n))
#define _Bit(n)                 (1ULL << (n))
#define _HBit(n)                (1ULL << ((n)+32))

#define bitmask(h,l)            ((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)          RShiftU64(((x) & bitmask((h),(l))), (l))
#define quad(hi,lo)             ((LShiftU64((hi), 32) | (lo)))

/*
 * The CPUID_FEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 1:
 */
#define  CPUID_FEATURE_FPU      _Bit(0)  /* Floating point unit on-chip */
#define  CPUID_FEATURE_VME      _Bit(1)  /* Virtual Mode Extension */
#define  CPUID_FEATURE_DE       _Bit(2)  /* Debugging Extension */
#define  CPUID_FEATURE_PSE      _Bit(3)  /* Page Size Extension */
#define  CPUID_FEATURE_TSC      _Bit(4)  /* Time Stamp Counter */
#define  CPUID_FEATURE_MSR      _Bit(5)  /* Model Specific Registers */
#define CPUID_FEATURE_PAE       _Bit(6)  /* Physical Address Extension */
#define  CPUID_FEATURE_MCE      _Bit(7)  /* Machine Check Exception */
#define  CPUID_FEATURE_CX8      _Bit(8)  /* CMPXCHG8B */
#define  CPUID_FEATURE_APIC     _Bit(9)  /* On-chip APIC */
#define CPUID_FEATURE_SEP       _Bit(11)  /* Fast System Call */
#define  CPUID_FEATURE_MTRR     _Bit(12)  /* Memory Type Range Register */
#define  CPUID_FEATURE_PGE      _Bit(13)  /* Page Global Enable */
#define  CPUID_FEATURE_MCA      _Bit(14)  /* Machine Check Architecture */
#define  CPUID_FEATURE_CMOV     _Bit(15)  /* Conditional Move Instruction */
#define CPUID_FEATURE_PAT       _Bit(16)  /* Page Attribute Table */
#define CPUID_FEATURE_PSE36     _Bit(17)  /* 36-bit Page Size Extension */
#define CPUID_FEATURE_PSN       _Bit(18)  /* Processor Serial Number */
#define CPUID_FEATURE_CLFSH     _Bit(19)  /* CLFLUSH Instruction supported */
#define CPUID_FEATURE_DS        _Bit(21)  /* Debug Store */
#define CPUID_FEATURE_ACPI      _Bit(22)  /* Thermal monitor and Clock Ctrl */
#define CPUID_FEATURE_MMX       _Bit(23)  /* MMX supported */
#define CPUID_FEATURE_FXSR      _Bit(24)  /* Fast floating pt save/restore */
#define CPUID_FEATURE_SSE       _Bit(25)  /* Streaming SIMD extensions */
#define CPUID_FEATURE_SSE2      _Bit(26)  /* Streaming SIMD extensions 2 */
#define CPUID_FEATURE_SS        _Bit(27)  /* Self-Snoop */
#define CPUID_FEATURE_HTT       _Bit(28)  /* Hyper-Threading Technology */
#define CPUID_FEATURE_TM        _Bit(29)  /* Thermal Monitor (TM1) */
#define CPUID_FEATURE_PBE       _Bit(31)  /* Pend Break Enable */

#define CPUID_FEATURE_SSE3      _HBit(0)  /* Streaming SIMD extensions 3 */
#define CPUID_FEATURE_PCLMULQDQ _HBit(1) /* PCLMULQDQ Instruction */
#define CPUID_FEATURE_DTES64    _HBit(2)  /* 64-bit DS layout */
#define CPUID_FEATURE_MONITOR   _HBit(3)  /* Monitor/mwait */
#define CPUID_FEATURE_DSCPL     _HBit(4)  /* Debug Store CPL */
#define CPUID_FEATURE_VMX       _HBit(5)  /* VMX */
#define CPUID_FEATURE_SMX       _HBit(6)  /* SMX */
#define CPUID_FEATURE_EST       _HBit(7)  /* Enhanced SpeedsTep (GV3) */
#define CPUID_FEATURE_TM2       _HBit(8)  /* Thermal Monitor 2 */
#define CPUID_FEATURE_SSSE3     _HBit(9)  /* Supplemental SSE3 instructions */
#define CPUID_FEATURE_CID       _HBit(10)  /* L1 Context ID */
#define CPUID_FEATURE_SEGLIM64  _HBit(11) /* 64-bit segment limit checking */
#define CPUID_FEATURE_CX16      _HBit(13)  /* CmpXchg16b instruction */
#define CPUID_FEATURE_xTPR      _HBit(14)  /* Send Task PRiority msgs */
#define CPUID_FEATURE_PDCM      _HBit(15)  /* Perf/Debug Capability MSR */

#define CPUID_FEATURE_PCID      _HBit(17) /* ASID-PCID support */
#define CPUID_FEATURE_DCA       _HBit(18)  /* Direct Cache Access */
#define CPUID_FEATURE_SSE4_1    _HBit(19)  /* Streaming SIMD extensions 4.1 */
#define CPUID_FEATURE_SSE4_2    _HBit(20)  /* Streaming SIMD extensions 4.2 */
#define CPUID_FEATURE_xAPIC     _HBit(21)  /* Extended APIC Mode */
#define CPUID_FEATURE_MOVBE     _HBit(22) /* MOVBE instruction */
#define CPUID_FEATURE_POPCNT    _HBit(23)  /* POPCNT instruction */
#define CPUID_FEATURE_TSCTMR    _HBit(24) /* TSC deadline timer */
#define CPUID_FEATURE_AES       _HBit(25)  /* AES instructions */
#define CPUID_FEATURE_XSAVE     _HBit(26) /* XSAVE instructions */
#define CPUID_FEATURE_OSXSAVE   _HBit(27) /* XGETBV/XSETBV instructions */
#define CPUID_FEATURE_AVX1_0    _HBit(28) /* AVX 1.0 instructions */
#define CPUID_FEATURE_RDRAND    _HBit(29) /* RDRAND instruction */
#define CPUID_FEATURE_F16C      _HBit(30) /* Float16 convert instructions */
#define CPUID_FEATURE_VMM       _HBit(31)  /* VMM (Hypervisor) present */

/*
 * Leaf 7, subleaf 0 additional features.
 * Bits returned in %ebx to a CPUID request with {%eax,%ecx} of (0x7,0x0}:
 */
#define CPUID_LEAF7_FEATURE_RDWRFSGS _Bit(0)  /* FS/GS base read/write */
#define CPUID_LEAF7_FEATURE_SMEP     _Bit(7)  /* Supervisor Mode Execute Protect */
#define CPUID_LEAF7_FEATURE_ENFSTRG  _Bit(9)  /* ENhanced Fast STRinG copy */


/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000001:
 */
#define CPUID_EXTFEATURE_SYSCALL   _Bit(11)  /* SYSCALL/sysret */
#define CPUID_EXTFEATURE_XD        _Bit(20)  /* eXecute Disable */
#define CPUID_EXTFEATURE_1GBPAGE   _Bit(26)     /* 1G-Byte Page support */
#define CPUID_EXTFEATURE_RDTSCP    _Bit(27)  /* RDTSCP */
#define CPUID_EXTFEATURE_EM64T     _Bit(29)  /* Extended Mem 64 Technology */

//#define CPUID_EXTFEATURE_LAHF    _HBit(20)  /* LAFH/SAHF instructions */
// New definition with Snow kernel
#define CPUID_EXTFEATURE_LAHF      _HBit(0)  /* LAHF/SAHF instructions */
/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000007:
 */
#define CPUID_EXTFEATURE_TSCI      _Bit(8)  /* TSC Invariant */

#define  CPUID_CACHE_SIZE  16  /* Number of descriptor values */

#define CPUID_MWAIT_EXTENSION  _Bit(0)  /* enumeration of WMAIT extensions */
#define CPUID_MWAIT_BREAK  _Bit(1)  /* interrupts are break events     */

/* Known MSR registers */
#define MSR_IA32_PLATFORM_ID        0x0017
#define IA32_APIC_BASE              0x001B  /* used also for AMD */
#define MSR_CORE_THREAD_COUNT       0x0035   /* limited use - not for Penryn or older  */
#define IA32_TSC_ADJUST             0x003B
#define MSR_IA32_BIOS_SIGN_ID       0x008B   /* microcode version */
#define MSR_FSB_FREQ                0x00CD   /* limited use - not for i7            */
/*
•  101B: 100 MHz (FSB 400)
•  001B: 133 MHz (FSB 533)
•  011B: 167 MHz (FSB 667)
•  010B: 200 MHz (FSB 800)
•  000B: 267 MHz (FSB 1067)
•  100B: 333 MHz (FSB 1333)
•  110B: 400 MHz (FSB 1600)
 */
// T8300 -> 0x01A2 => 200MHz
#define  MSR_PLATFORM_INFO          0x00CE   /* limited use - MinRatio for i7 but Max for Yonah  */
                                             /* turbo for penryn */
//haswell
//Low Frequency Mode. LFM is Pn in the P-state table. It can be read at MSR CEh [47:40].
//Minimum Frequency Mode. MFM is the minimum ratio supported by the processor and can be read from MSR CEh [55:48].
#define MSR_PKG_CST_CONFIG_CONTROL  0x00E2   /* sandy and up */
#define MSR_PMG_IO_CAPTURE_BASE     0x00E4  /* sandy and up */
#define IA32_MPERF                  0x00E7   /* TSC in C0 only */
#define IA32_APERF                  0x00E8   /* actual clocks in C0 */
#define MSR_IA32_EXT_CONFIG         0x00EE   /* limited use - not for i7            */
#define MSR_FLEX_RATIO              0x0194   /* limited use - not for Penryn or older      */
                                             //see no value on most CPUs
#define  MSR_IA32_PERF_STATUS       0x0198
#define MSR_IA32_PERF_CONTROL       0x0199
#define MSR_IA32_CLOCK_MODULATION   0x019A
#define MSR_THERMAL_STATUS          0x019C
#define MSR_IA32_MISC_ENABLE        0x01A0
#define MSR_THERMAL_TARGET          0x01A2   /* TjMax limited use - not for Penryn or older      */
#define MSR_TURBO_RATIO_LIMIT       0x01AD   /* limited use - not for Penryn or older      */


#define IA32_ENERGY_PERF_BIAS       0x01B0
//MSR 000001B0                                      0000-0000-0000-0005
#define MSR_PACKAGE_THERM_STATUS    0x01B1
//MSR 000001B1                                      0000-0000-8838-0000
#define IA32_PLATFORM_DCA_CAP       0x01F8
//MSR 000001FC                                      0000-0000-0004-005F


// Sandy Bridge & JakeTown specific 'Running Average Power Limit' MSR's.
#define MSR_RAPL_POWER_UNIT         0x606     /* R/O */
//MSR 00000606                                      0000-0000-000A-1003
#define MSR_PKGC3_IRTL              0x60A    /* RW time limit to go C3 */
          // bit 15 = 1 -- the value valid for C-state PM
#define MSR_PKGC6_IRTL              0x60B    /* RW time limit to go C6 */
//MSR 0000060B                                      0000-0000-0000-8854
  //Valid + 010=1024ns + 0x54=84mks
#define MSR_PKGC7_IRTL              0x60C    /* RW time limit to go C7 */
//MSR 0000060C                                      0000-0000-0000-8854
#define MSR_PKG_C2_RESIDENCY        0x60D   /* same as TSC but in C2 only */

#define MSR_PKG_RAPL_POWER_LIMIT    0x610
//MSR 00000610                                      0000-A580-0000-8960
#define MSR_PKG_ENERGY_STATUS       0x611
//MSR 00000611                                      0000-0000-3212-A857
#define MSR_PKG_POWER_INFO          0x614
//MSR 00000614                                      0000-0000-01E0-02F8
// Sandy Bridge IA (Core) domain MSR's.
#define MSR_PP0_POWER_LIMIT         0x638
#define MSR_PP0_ENERGY_STATUS       0x639
#define MSR_PP0_POLICY              0x63A
#define MSR_PP0_PERF_STATUS         0x63B

// Sandy Bridge Uncore (IGPU) domain MSR's (Not on JakeTown).
#define MSR_PP1_POWER_LIMIT         0x640
#define MSR_PP1_ENERGY_STATUS       0x641
//MSR 00000641                                      0000-0000-0000-0000
#define MSR_PP1_POLICY              0x642

// JakeTown only Memory MSR's.
#define MSR_PKG_PERF_STATUS         0x613
#define MSR_DRAM_POWER_LIMIT        0x618
#define MSR_DRAM_ENERGY_STATUS      0x619
#define MSR_DRAM_PERF_STATUS        0x61B
#define MSR_DRAM_POWER_INFO         0x61C

//IVY_BRIDGE
#define MSR_CONFIG_TDP_NOMINAL      0x648
#define MSR_CONFIG_TDP_LEVEL1       0x649
#define MSR_CONFIG_TDP_LEVEL2       0x64A
#define MSR_CONFIG_TDP_CONTROL      0x64B  /* write once to lock */
#define MSR_TURBO_ACTIVATION_RATIO  0x64C

//Skylake
#define BASE_ART_CLOCK_SOURCE   24000000ULL  /* 24Mhz */
#define MSR_IA32_PM_ENABLE          0x770
#define MSR_IA32_HWP_REQUEST        0x774

//AMD
#define K8_FIDVID_STATUS            0xC0010042
#define K10_COFVID_LIMIT            0xC0010061 /* max enabled p-state (msr >> 4) & 7 */
#define K10_COFVID_CONTROL          0xC0010062 /* switch to p-state */
#define K10_PSTATE_STATUS           0xC0010064
#define K10_COFVID_STATUS           0xC0010071 /* current p-state (msr >> 16) & 7 */
/* specific settings
static void SavePState(unsigned int index, unsigned int lowMsr, unsigned int core)
{
  CONST unsigned int msrIndex = 0xC0010064u + index;
  CONST DWORD_PTR affinityMask = (DWORD_PTR)1 << core;

  DWORD lower, higher;
  RdmsrTx(msrIndex, &lower, &higher, affinityMask);

  CONST DWORD lowMsrMask = 0xFE40FFFFu;
  lower = (lower & ~lowMsrMask) | (lowMsr & lowMsrMask);

  WrmsrTx(msrIndex, lower, higher, affinityMask);
}

MSR C0010064  8000-0185-0000-1418 [20.00x] [1.4250 V] [13.30 A] [PState Pb0]
MSR C0010065  8000-0185-0000-1615 [18.50x] [1.4125 V] [13.30 A] [PState Pb1]
MSR C0010066  8000-0173-0000-1A1A [21.00x] [1.3875 V] [11.50 A] [PState P0]
MSR C0010067  0000-0173-0000-1A1A
MSR C0010068  0000-0173-0000-181A
MSR C0010069  0000-0173-0000-1A1A
MSR C001006A  8000-0125-0000-604C [ 7.00x] [0.9500 V] [ 3.70 A] [PState P1]
MSR C001006B  0000-0000-0000-0000
*/


#define DEFAULT_FSB                 100000          /* for now, hardcoding 100MHz for old CPUs */


/* CPUID Index */
#define CPUID_0    0
#define CPUID_1    1
#define CPUID_2    2
#define CPUID_3    3
#define CPUID_4    4
#define CPUID_5    5
#define CPUID_6    6
#define CPUID_80   7
#define CPUID_81   8
#define CPUID_87   9
#define CPUID_88   10
#define CPUID_81E  11
#define CPUID_15   15
#define CPUID_MAX  16

/* CPU Cache */
#define MAX_CACHE_COUNT  4
#define CPU_CACHE_LEVEL  3

/* PCI */
#define PCI_BASE_ADDRESS_0          0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1          0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2          0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3          0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4          0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5          0x24    /* 32 bits */

#define PCI_CLASS_MEDIA_HDA         0x03

#define GEN_PMCON_1                 0xA0

#define PCIADDR(bus, dev, func)      ((1 << 31) | ((bus) << 16) | ((dev) << 11) | ((func) << 8))
#define REG8(base, reg)              ((volatile UINT8 *)(UINTN)(base))[(reg)]
#define REG16(base, reg)             ((volatile UINT16 *)(UINTN)(base))[(reg) >> 1]
//#define REG32(base, reg)             ((volatile UINT32 *)(UINTN)(base))[(reg) >> 2]
#define REG32(base, reg)             (*(volatile UINT32 *)((UINTN)base + reg))
#define WRITEREG32(base, reg, value) REG32((base), (reg)) = value

#define EFI_HANDLE_TYPE_UNKNOWN                     0x000
#define EFI_HANDLE_TYPE_IMAGE_HANDLE                0x001
#define EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE       0x002
#define EFI_HANDLE_TYPE_DEVICE_DRIVER               0x004
#define EFI_HANDLE_TYPE_BUS_DRIVER                  0x008
#define EFI_HANDLE_TYPE_DRIVER_CONFIGURATION_HANDLE 0x010
#define EFI_HANDLE_TYPE_DRIVER_DIAGNOSTICS_HANDLE   0x020
#define EFI_HANDLE_TYPE_COMPONENT_NAME_HANDLE       0x040
#define EFI_HANDLE_TYPE_DEVICE_HANDLE               0x080
#define EFI_HANDLE_TYPE_PARENT_HANDLE               0x100
#define EFI_HANDLE_TYPE_CONTROLLER_HANDLE           0x200
#define EFI_HANDLE_TYPE_CHILD_HANDLE                0x400

#define  AML_CHUNK_NONE          0xff
#define  AML_CHUNK_ZERO          0x00
#define  AML_CHUNK_ONE           0x01
#define  AML_CHUNK_ALIAS         0x06
#define  AML_CHUNK_NAME          0x08
#define  AML_CHUNK_BYTE          0x0A
#define  AML_CHUNK_WORD          0x0B
#define  AML_CHUNK_DWORD         0x0C
#define  AML_CHUNK_STRING        0x0D
#define  AML_CHUNK_QWORD         0x0E
#define  AML_CHUNK_SCOPE         0x10
#define  AML_CHUNK_PACKAGE       0x12
#define  AML_CHUNK_METHOD        0x14
#define AML_CHUNK_RETURN         0xA4
#define AML_LOCAL0               0x60
#define AML_STORE_OP             0x70
//-----------------------------------
// defines added by pcj
#define  AML_CHUNK_BUFFER        0x11
#define  AML_CHUNK_STRING_BUFFER 0x15
#define  AML_CHUNK_OP            0x5B
#define  AML_CHUNK_REFOF         0x71
#define  AML_CHUNK_DEVICE        0x82
#define  AML_CHUNK_LOCAL0        0x60
#define  AML_CHUNK_LOCAL1        0x61
#define  AML_CHUNK_LOCAL2        0x62

#define  AML_CHUNK_ARG0          0x68
#define  AML_CHUNK_ARG1          0x69
#define  AML_CHUNK_ARG2          0x6A
#define  AML_CHUNK_ARG3          0x6B

//DSDT fixes MASK
//0x00FF
#define FIX_DTGP      bit(0)
#define FIX_WARNING   bit(1)
#define FIX_SHUTDOWN  bit(2)
#define FIX_MCHC      bit(3)
#define FIX_HPET      bit(4)
#define FIX_LPC       bit(5)
#define FIX_IPIC      bit(6)
#define FIX_SBUS      bit(7)
//0xFF00
#define FIX_DISPLAY   bit(8)
#define FIX_IDE       bit(9)
#define FIX_SATA      bit(10)
#define FIX_FIREWIRE  bit(11)
#define FIX_USB       bit(12)
#define FIX_LAN       bit(13)
#define FIX_WIFI      bit(14)
#define FIX_HDA       bit(15)
//new bits 16-31 0xFFFF0000
//#define FIX_NEW_WAY   bit(31) will be reused
#define FIX_DARWIN    bit(16)
#define FIX_RTC       bit(17)
#define FIX_TMR       bit(18)
#define FIX_IMEI      bit(19)
#define FIX_INTELGFX  bit(20)
#define FIX_WAK       bit(21)
#define FIX_UNUSED    bit(22)
#define FIX_ADP1      bit(23)
#define FIX_PNLF      bit(24)
#define FIX_S3D       bit(25)
#define FIX_ACST      bit(26)
#define FIX_HDMI      bit(27)
#define FIX_REGIONS   bit(28)
#define FIX_HEADERS   bit(29)
#define FIX_MUTEX     bit(30)

//devices
#define DEV_ATI       bit(0)
#define DEV_NVIDIA    bit(1)
#define DEV_INTEL     bit(2)
#define DEV_HDA       bit(3)
#define DEV_HDMI      bit(4)
#define DEV_LAN       bit(5)
#define DEV_WIFI      bit(6)
#define DEV_SATA      bit(7)
#define DEV_IDE       bit(8)
#define DEV_LPC       bit(9)
#define DEV_SMBUS     bit(10)
#define DEV_USB       bit(11)
#define DEV_FIREWIRE  bit(12)
#define DEV_MCHC      bit(13)
#define DEV_IMEI      bit(14)
#define DEV_BY_PCI    bit(31)

#define NUM_OF_CONFIGS 3

// Kernel scan states
#define KERNEL_SCAN_ALL        (0)
#define KERNEL_SCAN_NEWEST     (1)
#define KERNEL_SCAN_OLDEST     (2)
#define KERNEL_SCAN_FIRST      (3)
#define KERNEL_SCAN_LAST       (4)
#define KERNEL_SCAN_MOSTRECENT (5)
#define KERNEL_SCAN_EARLIEST   (6)
#define KERNEL_SCAN_NONE       (100)

// Secure boot policies
// Deny all images
#define SECURE_BOOT_POLICY_DENY      (0)
// Allow all images
#define SECURE_BOOT_POLICY_ALLOW     (1)
// Query the user to choose action
#define SECURE_BOOT_POLICY_QUERY     (2)
// Insert signature into db
#define SECURE_BOOT_POLICY_INSERT    (3)
// White list
#define SECURE_BOOT_POLICY_WHITELIST (4)
// Black list
#define SECURE_BOOT_POLICY_BLACKLIST (5)
// User policy, white and black list with query
#define SECURE_BOOT_POLICY_USER      (6)

// ADDRESS_OF
/// Get the address of a structure member
/// @param INSTANCETYPE The type of the instance structure
/// @param Instance     An instance of a structure to get the address of a member
/// @param FIELDTYPE    The type of the member field
/// @param Field        The name of the field of which to get the address
/// @return The address of the offset of the member field in the instance structure
//#define ADDRESS_OF(INSTANCETYPE, Instance, FIELDTYPE, Field) (FIELDTYPE *)(((UINT8 *)(Instance)) + OFFSET_OF(INSTANCETYPE, Field))


struct aml_chunk
{
  UINT8              Type;
  UINT8              pad;
  UINT16             Length;
  UINT32             pad2;
  CHAR8              *Buffer;

  UINT16             Size;
  UINT16             pad3[3];

  struct aml_chunk*  Next;
  struct aml_chunk*  First;
  struct aml_chunk*  Last;
};
typedef struct aml_chunk AML_CHUNK;

struct p_state_vid_fid
{
  UINT8 VID;  // Voltage ID
  UINT8 FID;  // Frequency ID
};

union p_state_control
{
  UINT16 Control;
  struct p_state_vid_fid VID_FID;
};

struct p_state
{
  union p_state_control Control;

  UINT32 CID;    // Compare ID
  UINT32 Frequency;
};
typedef struct p_state P_STATE;

struct _oper_region {
  CHAR8  Name[8];
  UINT32 Address;
  struct _oper_region *next;
};
typedef struct _oper_region OPER_REGION;


typedef enum {
  kTagTypeNone,
  kTagTypeDict,
  kTagTypeKey,
  kTagTypeString,
  kTagTypeInteger,
  kTagTypeData,
  kTagTypeDate,
  kTagTypeFalse,
  kTagTypeTrue,
  kTagTypeArray
} TAG_TYPE;

typedef struct _DRIVERS_FLAGS {
  BOOLEAN EmuVariableLoaded;
  BOOLEAN VideoLoaded;
  BOOLEAN PartitionLoaded;
  BOOLEAN MemFixLoaded;
  BOOLEAN AptioFixLoaded;
  BOOLEAN AptioFix2Loaded;
  BOOLEAN AptioFix3Loaded;
  BOOLEAN AptioMemFixLoaded;
  BOOLEAN HFSLoaded;
  BOOLEAN APFSLoaded;
} DRIVERS_FLAGS;

struct Symbol {
  UINTN         refCount;
  struct Symbol *next;
  CHAR8         string[1];
};

typedef struct Symbol Symbol, *SymbolPtr;

typedef struct TagStruct {

  UINTN  type;
  CHAR8  *string;
  UINT8  *data;
  UINTN  dataLen;
  UINTN  offset;
  struct TagStruct *tag;
  struct TagStruct *tagNext;

} TagStruct, *TagPtr;

#pragma pack(push)
#pragma pack(1)

typedef struct {

  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT32                      Entry;

} RSDT_TABLE;

typedef struct {

  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT64                      Entry;

} XSDT_TABLE;
/*
typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} GUID;
*/

#pragma pack(pop)

typedef struct DEV_PROPERTY DEV_PROPERTY; //yyyy
struct DEV_PROPERTY {
  UINT32        Device;
  EFI_DEVICE_PATH_PROTOCOL* DevicePath;
  CHAR8         *Key;
  UINT8         *Value;
  UINTN         ValueLen;
  DEV_PROPERTY  *Next;   //next device or next property
  DEV_PROPERTY  *Child;  // property list of the device
  CHAR8         *Label;
  INPUT_ITEM    MenuItem;
  TAG_TYPE      ValueType;
};

typedef struct ACPI_NAME_LIST ACPI_NAME_LIST;
struct ACPI_NAME_LIST {
	ACPI_NAME_LIST *Next;
	CHAR8          *Name;
};

typedef struct RT_VARIABLES RT_VARIABLES;
struct RT_VARIABLES {
//  BOOLEAN  Disabled;
  CHAR16   *Name;
  EFI_GUID VarGuid;
};

typedef struct CUSTOM_LOADER_ENTRY CUSTOM_LOADER_ENTRY;
struct CUSTOM_LOADER_ENTRY {
  CUSTOM_LOADER_ENTRY     *Next;
  CUSTOM_LOADER_ENTRY     *SubEntries;
  EG_IMAGE                *Image;
  EG_IMAGE                *DriveImage;
  CONST CHAR16                  *ImagePath;
  CONST CHAR16                  *DriveImagePath;
  CONST CHAR16                  *Volume;
  CONST CHAR16                  *Path;
  CONST CHAR16                  *Options;
  CONST CHAR16                  *FullTitle;
  CONST CHAR16                  *Title;
  CONST CHAR16                  *Settings;
  CHAR16                  Hotkey;
  BOOLEAN                 CommonSettings;
  UINT8                   Flags;
  UINT8                   Type;
  UINT8                   VolumeType;
  UINT8                   KernelScan;
  UINT8                   CustomBoot;
  EG_IMAGE                *CustomLogo;
  EG_PIXEL                *BootBgColor;
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches; //zzzz
};

typedef struct CUSTOM_LEGACY_ENTRY CUSTOM_LEGACY_ENTRY;
struct CUSTOM_LEGACY_ENTRY {
  CUSTOM_LEGACY_ENTRY *Next;
  EG_IMAGE            *Image;
  EG_IMAGE            *DriveImage;
  CONST CHAR16              *ImagePath;
  CONST CHAR16              *DriveImagePath;
  CONST CHAR16              *Volume;
  CONST CHAR16              *FullTitle;
  CONST CHAR16              *Title;
  CHAR16              Hotkey;
  UINT8               Flags;
  UINT8               Type;
  UINT8               VolumeType;
};

typedef struct CUSTOM_TOOL_ENTRY CUSTOM_TOOL_ENTRY;
struct CUSTOM_TOOL_ENTRY {
  CUSTOM_TOOL_ENTRY *Next;
  EG_IMAGE          *Image;
  CHAR16            *ImagePath;
  CHAR16            *Volume;
  CHAR16            *Path;
  CHAR16            *Options;
  CHAR16            *FullTitle;
  CHAR16            *Title;
  CHAR16            Hotkey;
  UINT8             Flags;
  UINT8             VolumeType;
};

typedef struct ACPI_DROP_TABLE ACPI_DROP_TABLE;
struct ACPI_DROP_TABLE
{
  ACPI_DROP_TABLE *Next;
  UINT32          Signature;
  UINT32          Length;
  UINT64          TableId;
  INPUT_ITEM      MenuItem;
};

// ACPI/PATCHED/AML
typedef struct ACPI_PATCHED_AML ACPI_PATCHED_AML;
struct ACPI_PATCHED_AML
{
  ACPI_PATCHED_AML  *Next;
  CHAR16            *FileName;
  INPUT_ITEM        MenuItem;
};

// syscl - Side load kext
typedef struct SIDELOAD_KEXT SIDELOAD_KEXT;
struct SIDELOAD_KEXT {
  SIDELOAD_KEXT  *Next;
  SIDELOAD_KEXT  *PlugInList;
  CHAR16         *FileName;
  CHAR16         *KextDirNameUnderOEMPath;
  CHAR16         *Version;
  INPUT_ITEM     MenuItem;
};

// SysVariables
typedef struct SYSVARIABLES SYSVARIABLES;
struct SYSVARIABLES
{
  SYSVARIABLES      *Next;
  CHAR16            *Key;
  INPUT_ITEM        MenuItem;
};

//
// rellocate new guid for smbios table type 1
//
#define REMAP_SMBIOS_TABLE_GUID { 0xeb9d2d35, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d } }


typedef struct {

  // SMBIOS TYPE0
  CHAR8                   VendorName[64];
  CHAR8                   RomVersion[64];
  CHAR8                   EfiVersion[64];
  CHAR8                   ReleaseDate[64];
  // SMBIOS TYPE1
  CHAR8                   ManufactureName[64];
  CHAR8                   ProductName[64];
  CHAR8                   VersionNr[64];
  CHAR8                   SerialNr[64];
  EFI_GUID                SmUUID;
  BOOLEAN                 SmUUIDConfig;
  CHAR8                   pad0[7];
//CHAR8                    Uuid[64];
//CHAR8                    SKUNumber[64];
  CHAR8                   FamilyName[64];
  CHAR8                   OEMProduct[64];
  CHAR8                   OEMVendor[64];
  // SMBIOS TYPE2
  CHAR8                   BoardManufactureName[64];
  CHAR8                   BoardSerialNumber[64];
  CHAR8                   BoardNumber[64]; //Board-ID
  CHAR8                   LocationInChassis[64];
  CHAR8                   BoardVersion[64];
  CHAR8                   OEMBoard[64];
  UINT8                   BoardType;
  UINT8                   Pad1;
  // SMBIOS TYPE3
  BOOLEAN                 Mobile;
  UINT8                   ChassisType;
  CHAR8                   ChassisManufacturer[64];
  CHAR8                   ChassisAssetTag[64];
  // SMBIOS TYPE4
  UINT32                  CpuFreqMHz;
  UINT32                  BusSpeed; //in kHz
  BOOLEAN                 Turbo;
  UINT8                   EnabledCores;
  BOOLEAN                 UserChange;
  BOOLEAN                 QEMU;
  // SMBIOS TYPE17
  UINT16                  SmbiosVersion;
  INT8                    Attribute;
  INT8                    Pad17[1];
  CHAR8                   MemoryManufacturer[64];
  CHAR8                   MemorySerialNumber[64];
  CHAR8                   MemoryPartNumber[64];
  CHAR8                   MemorySpeed[64];
  // SMBIOS TYPE131
  UINT16                  CpuType;
  // SMBIOS TYPE132
  UINT16                  QPI;
  BOOLEAN                 SetTable132;
  BOOLEAN                 TrustSMBIOS;
  BOOLEAN                 InjectMemoryTables;
  INT8                    XMPDetection;
  BOOLEAN                 UseARTFreq;
  // SMBIOS TYPE133
  UINT64                  PlatformFeature;
    
  // PatchTableType11
  BOOLEAN                 NoRomInfo;

  // OS parameters
  CHAR8                   Language[16];
  CHAR8                   BootArgs[256];
  CHAR16                  CustomUuid[40];

  CHAR16                  *DefaultVolume;
  CHAR16                  *DefaultLoader;
//Boot
  BOOLEAN                 LastBootedVolume;
  BOOLEAN                 SkipHibernateTimeout;
//Monitor
  BOOLEAN                 IntelMaxBacklight;
//  UINT8                   Pad21[1];
  UINT16                  VendorEDID;
  UINT16                  ProductEDID;
  UINT16                  BacklightLevel;
  BOOLEAN                 BacklightLevelConfig;
  BOOLEAN                 IntelBacklight;
//Boot options
  BOOLEAN                 MemoryFix;
  BOOLEAN                 WithKexts;
  BOOLEAN                 WithKextsIfNoFakeSMC;
  BOOLEAN                 FakeSMCFound;
  BOOLEAN                 NoCaches;

  // GUI parameters
  BOOLEAN                 Debug;
  BOOLEAN                 Proportional;
//  UINT8                   Pad22[1];
  UINT32                  DefaultBackgroundColor;

  //ACPI
  UINT64                  ResetAddr;
  UINT8                   ResetVal;
  BOOLEAN                 NoASPM;
  BOOLEAN                 DropSSDT;
  BOOLEAN                 NoOemTableId;
  BOOLEAN                 NoDynamicExtract;
  BOOLEAN                 AutoMerge;
  BOOLEAN                 GeneratePStates;
  BOOLEAN                 GenerateCStates;
  BOOLEAN                 GenerateAPSN;
  BOOLEAN                 GenerateAPLF;
  BOOLEAN                 GeneratePluginType;
  UINT8                   PLimitDict;
  UINT8                   UnderVoltStep;
  BOOLEAN                 DoubleFirstState;
  BOOLEAN                 SuspendOverride;
  BOOLEAN                 EnableC2;
  BOOLEAN                 EnableC4;
  BOOLEAN                 EnableC6;
  BOOLEAN                 EnableISS;
  BOOLEAN                 SlpSmiEnable;
  BOOLEAN                 FixHeaders;
  UINT16                  C3Latency;
  BOOLEAN                 smartUPS;
  BOOLEAN                 PatchNMI;
  BOOLEAN                 EnableC7;
  UINT8                   SavingMode;

  CHAR16                  DsdtName[28];
  UINT32                  FixDsdt;
  UINT8                   MinMultiplier;
  UINT8                   MaxMultiplier;
  UINT8                   PluginType;
//  BOOLEAN                 DropMCFG;
  BOOLEAN                 FixMCFG;

  UINT32                  DeviceRenameCount;
  ACPI_NAME_LIST          *DeviceRename;
  //Injections
  BOOLEAN                 StringInjector;
  BOOLEAN                 InjectSystemID;
  BOOLEAN                 NoDefaultProperties;

  BOOLEAN                 ReuseFFFF;

  //PCI devices
  UINT32                  FakeATI;    //97
  UINT32                  FakeNVidia;
  UINT32                  FakeIntel;
  UINT32                  FakeLAN;   //100
  UINT32                  FakeWIFI;
  UINT32                  FakeSATA;
  UINT32                  FakeXHCI;  //103
  UINT32                  FakeIMEI;  //106

  //Graphics
//  UINT16                  PCIRootUID;
  BOOLEAN                 GraphicsInjector;
  BOOLEAN                 InjectIntel;
  BOOLEAN                 InjectATI;
  BOOLEAN                 InjectNVidia;
  BOOLEAN                 DeInit;
  BOOLEAN                 LoadVBios;
  BOOLEAN                 PatchVBios;
  VBIOS_PATCH_BYTES       *PatchVBiosBytes;
  UINTN                   PatchVBiosBytesCount;
  BOOLEAN                 InjectEDID;
  BOOLEAN                 LpcTune;
  UINT16                  DropOEM_DSM;
  UINT8                   *CustomEDID;
  UINT16                  CustomEDIDsize;
  UINT16                  EdidFixHorizontalSyncPulseWidth;
  UINT8                   EdidFixVideoInputSignal;

  CHAR16                  FBName[16];
  UINT16                  VideoPorts;
  BOOLEAN                 NvidiaGeneric;
  BOOLEAN                 NvidiaNoEFI;
  BOOLEAN                 NvidiaSingle;
  UINT64                  VRAM;
  UINT8                   Dcfg[8];
  UINT8                   NVCAP[20];
  INT8                    BootDisplay;
  BOOLEAN                 NvidiaWeb;
  UINT8                   pad41[2];
  UINT32                  DualLink;
  UINT32                  IgPlatform;

  // Secure boot white/black list
  UINT32                  SecureBootWhiteListCount;
  UINT32                  SecureBootBlackListCount;
  CHAR16                  **SecureBootWhiteList;

  CHAR16                  **SecureBootBlackList;

  // Secure boot
  UINT8                   SecureBoot;
  UINT8                   SecureBootSetupMode;
  UINT8                   SecureBootPolicy;

  // HDA
  BOOLEAN                 HDAInjection;
  INT32                   HDALayoutId;

  // USB DeviceTree injection
  BOOLEAN                 USBInjection;
  BOOLEAN                 USBFixOwnership;
  BOOLEAN                 InjectClockID;
  BOOLEAN                 HighCurrent;
  BOOLEAN                 NameEH00;
  BOOLEAN                 NameXH00;
  
  BOOLEAN                 LANInjection;
  BOOLEAN                 HDMIInjection;

 // UINT8                   pad61[2];

  // LegacyBoot
  CHAR16                  LegacyBoot[32];
  UINT16                  LegacyBiosDefaultEntry;

  //SkyLake
  BOOLEAN                 HWP;
  UINT8                   TDP;
  UINT32                  HWPValue;

  //Volumes hiding
  CHAR16                  **HVHideStrings;

  INTN                    HVCount;

  // KernelAndKextPatches
  KERNEL_AND_KEXT_PATCHES KernelAndKextPatches;  //zzzz
  BOOLEAN                 KextPatchesAllowed;
  BOOLEAN                 KernelPatchesAllowed; //From GUI: Only for user patches, not internal Clover

  CHAR8                   AirportBridgeDeviceName[5];

  // Pre-language
  BOOLEAN                 KbdPrevLang;

  //Pointer
  BOOLEAN                 PointerEnabled;
  INTN                    PointerSpeed;
  UINT64                  DoubleClickTime;
  BOOLEAN                 PointerMirror;

//  UINT8                   pad7[6];
  UINT8                   CustomBoot;
  EG_IMAGE                *CustomLogo;

  UINT32                  RefCLK;

  // SysVariables
  CHAR8                   *RtMLB;
  UINT8                   *RtROM;
  UINTN                   RtROMLen;

  UINT32                  CsrActiveConfig;
  UINT16                  BooterConfig;
  CHAR8                   BooterCfgStr[64];
  BOOLEAN                 DisableCloverHotkeys;
  BOOLEAN                 NeverDoRecovery;

  // Multi-config
  CHAR16                  ConfigName[30];
  CHAR16                  *MainConfigName;

  //Drivers
  INTN                    BlackListCount;
  CHAR16                  **BlackList;

  //SMC keys
  CHAR8                   RPlt[8];
  CHAR8                   RBr[8];
  UINT8                   EPCI[4];
  UINT8                   REV[6];

  //other devices
  BOOLEAN                 Rtc8Allowed;
  BOOLEAN                 ForceHPET;
  BOOLEAN                 ResetHDA;
  BOOLEAN                 PlayAsync;
  UINT32                  DisableFunctions;

  //Patch DSDT arbitrary
  UINT32                  PatchDsdtNum;
  UINT8                   **PatchDsdtFind;
  UINT32 *LenToFind;
  UINT8  **PatchDsdtReplace;

  UINT32 *LenToReplace;
  BOOLEAN                 DebugDSDT;
  BOOLEAN                 SlpWak;
  BOOLEAN                 UseIntelHDMI;
  UINT8                   AFGLowPowerState;
  UINT8                   PNLF_UID;
//  UINT8                   pad83[4];


  // Table dropping
  ACPI_DROP_TABLE         *ACPIDropTables;

  // Custom entries
  BOOLEAN                 DisableEntryScan;
  BOOLEAN                 DisableToolScan;
  BOOLEAN                 ShowHiddenEntries;
  UINT8                   KernelScan;
  BOOLEAN                 LinuxScan;
//  UINT8                   pad84[3];
  CUSTOM_LOADER_ENTRY     *CustomEntries;
  CUSTOM_LEGACY_ENTRY     *CustomLegacy;
  CUSTOM_TOOL_ENTRY       *CustomTool;

  //Add custom properties
  UINTN                   NrAddProperties;
  DEV_PROPERTY            *AddProperties;

  //BlackListed kexts
  CHAR16                  BlockKexts[64];

  // Disable inject kexts
//  UINT32                  DisableInjectKextCount;
//  CHAR16                  **DisabledInjectKext;
//  INPUT_ITEM              *InjectKextMenuItem;

  //ACPI tables
  UINTN                   SortedACPICount;
  CHAR16                  **SortedACPI;

  // ACPI/PATCHED/AML
  UINT32                  DisabledAMLCount;
  CHAR16                  **DisabledAML;
  CHAR8                   **PatchDsdtLabel; //yyyy
  CHAR8                   **PatchDsdtTgt;
  INPUT_ITEM              *PatchDsdtMenuItem;

  //other
  UINT32                  IntelMaxValue;
//  UINT32                  AudioVolume;

  // boot.efi
  UINT32 OptionsBits;
  UINT32 FlagsBits;
  UINT32 UIScale;
  UINT32 EFILoginHiDPI;
  UINT8  flagstate[32];

  DEV_PROPERTY            *ArbProperties;

} SETTINGS_DATA;

typedef struct {
 //values from CPUID
  UINT32                  CPUID[CPUID_MAX][4];
  UINT32                  Vendor;
  UINT32                  Signature;
  UINT32                  Family;
  UINT32                  Model;
  UINT32                  Stepping;
  UINT32                  Type;
  UINT32                  Extmodel;
  UINT32                  Extfamily;
  UINT64                  Features;
  UINT64                  ExtFeatures;
  UINT32                  CoresPerPackage;
  UINT32                  LogicalPerPackage;
  CHAR8                   BrandString[48];

  //values from BIOS
  UINT64                  ExternalClock;
  UINT32                  MaxSpeed;       //MHz
  UINT32                  CurrentSpeed;   //MHz
//  UINT32                  Pad;

  //calculated from MSR
  UINT64                  MicroCode;
  UINT64                  ProcessorFlag;
  UINT32                  MaxRatio;
  UINT32                  SubDivider;
  UINT32                  MinRatio;
  UINT32                  DynFSB;
  UINT64                  ProcessorInterconnectSpeed; //MHz
  UINT64                  FSBFrequency; //Hz
  UINT64                  CPUFrequency;
  UINT64                  TSCFrequency;
  UINT8                   Cores;
  UINT8                   EnabledCores;
  UINT8                   Threads;
  UINT8                   Mobile;  //not for i3-i7
  BOOLEAN                 Turbo;
  UINT8                   Pad2[3];

  /* Core i7,5,3 */
  UINT16                  Turbo1; //1 Core
  UINT16                  Turbo2; //2 Core
  UINT16                  Turbo3; //3 Core
  UINT16                  Turbo4; //4 Core

  UINT64                  TSCCalibr;
  UINT64                  ARTFrequency;

} CPU_STRUCTURE;

typedef enum {

  MacBook11,
  MacBook21,
  MacBook31,
  MacBook41,
  MacBook51,
  MacBook52,
  MacBook61,
  MacBook71,
  MacBook81,
  MacBook91,
  MacBook101,
  MacBookPro11,
  MacBookPro12,
  MacBookPro21,
  MacBookPro22,
  MacBookPro31,
  MacBookPro41,
  MacBookPro51,
  MacBookPro52,
  MacBookPro53,
  MacBookPro54,
  MacBookPro55,
  MacBookPro61,
  MacBookPro62,
  MacBookPro71,
  MacBookPro81,
  MacBookPro82,
  MacBookPro83,
  MacBookPro91,
  MacBookPro92,
  MacBookPro101,
  MacBookPro102,
  MacBookPro111,
  MacBookPro112,
  MacBookPro113,
  MacBookPro114,
  MacBookPro115,
  MacBookPro121,
  MacBookPro131,
  MacBookPro132,
  MacBookPro133,
  MacBookPro141,
  MacBookPro142,
  MacBookPro143,
  MacBookPro151,
  MacBookPro152,
  MacBookPro153,
  MacBookPro154,
  MacBookPro161,
  MacBookAir11,
  MacBookAir21,
  MacBookAir31,
  MacBookAir32,
  MacBookAir41,
  MacBookAir42,
  MacBookAir51,
  MacBookAir52,
  MacBookAir61,
  MacBookAir62,
  MacBookAir71,
  MacBookAir72,
  MacBookAir81,
  MacBookAir82,
  MacMini11,
  MacMini21,
  MacMini31,
  MacMini41,
  MacMini51,
  MacMini52,
  MacMini53,
  MacMini61,
  MacMini62,
  MacMini71,
  MacMini81,
  iMac41,
  iMac42,
  iMac51,
  iMac52,
  iMac61,
  iMac71,
  iMac81,
  iMac91,
  iMac101,
  iMac111,
  iMac112,
  iMac113,
  iMac121,
  iMac122,
  iMac131,
  iMac132,
  iMac133,
  iMac141,
  iMac142,
  iMac143,
  iMac144,
  iMac151,
  iMac161,
  iMac162,
  iMac171,
  iMac181,
  iMac182,
  iMac183,
  iMac191,
  iMac192,
  iMacPro11,
  MacPro11,
  MacPro21,
  MacPro31,
  MacPro41,
  MacPro51,
  MacPro61,
  MacPro71,
  Xserve11,
  Xserve21,
  Xserve31,

  MaxMachineType

} MACHINE_TYPES;

typedef struct {
  BOOLEAN  InUse;
  UINT8   Type;
  UINT16  pad0;
  UINT32  pad1;
  UINT32  ModuleSize;
  UINT32  Frequency;
  CONST CHAR8*  Vendor;
  CHAR8*  PartNo;
  CHAR8*  SerialNo;
} RAM_SLOT_INFO;

// The maximum number of RAM slots to detect
// even for 3-channels chipset X58 there are no more then 8 slots
#define MAX_RAM_SLOTS 24
// The maximum sane frequency for a RAM module
#define MAX_RAM_FREQUENCY 5000

typedef struct {

  UINT32        Frequency;
  UINT32        Divider;
  UINT8         TRC;
  UINT8         TRP;
  UINT8         RAS;
  UINT8         Channels;
  UINT8         Slots;
  UINT8         Type;
  UINT8         SPDInUse;
  UINT8         SMBIOSInUse;
  UINT8         UserInUse;
  UINT8         UserChannels;
  UINT8         pad[2];

  RAM_SLOT_INFO SPD[MAX_RAM_SLOTS * 4];
  RAM_SLOT_INFO SMBIOS[MAX_RAM_SLOTS * 4];
  RAM_SLOT_INFO User[MAX_RAM_SLOTS * 4];

} MEM_STRUCTURE;
//unused
/*
typedef struct {
  UINT8     MaxMemorySlots;      // number of memory slots polulated by SMBIOS
  UINT8     CntMemorySlots;      // number of memory slots counted
  UINT16    MemoryModules;      // number of memory modules installed
  UINT8    DIMM[MAX_RAM_SLOTS];  // Information and SPD mapping for each slot
} DMI;
*/

typedef enum {
  english = 0,  //en
  russian,    //ru
  french,     //fr
  german,     //de
  dutch,      //nl
  italian,    //it
  spanish,    //es
  portuguese, //pt
  brasil,     //br
  polish,     //pl
  ukrainian,  //ua
  croatian,   //hr
  czech,      //cs
  indonesian, //id
  korean,     //ko
  chinese,    //cn
  romanian    //ro
  //something else? add, please
} LANGUAGES;

typedef enum {
  Unknown,
  Ati,      /* 0x1002 */
  Intel,    /* 0x8086 */
  Nvidia,   /* 0x10de */
  RDC,  /* 0x17f3 */
  VIA,  /* 0x1106 */
  SiS,  /* 0x1039 */
  ULI  /* 0x10b9 */
} HRDW_MANUFACTERER;

typedef struct {
  HRDW_MANUFACTERER  Vendor;
  UINT8             Ports;
  UINT16            DeviceID;
  UINT16            Family;
//UINT16            Width;
//UINT16            Height;
  CHAR8             Model[64];
  CHAR8             Config[64];
  BOOLEAN           LoadVBios;
//BOOLEAN           PatchVBios;
  UINTN             Segment;
  UINTN             Bus;
  UINTN             Device;
  UINTN             Function;
  EFI_HANDLE        Handle;
  UINT8             *Mmio;
  UINT32            Connectors;
  BOOLEAN           ConnChanged;
} GFX_PROPERTIES;

typedef struct {
    HRDW_MANUFACTERER  Vendor;
    UINT16            controller_vendor_id;
    UINT16            controller_device_id;
    CHAR16            *controller_name;
// -- Codec Info -- //
    UINT16            codec_vendor_id;
    UINT16            codec_device_id;
    UINT8             codec_revision_id;
    UINT8             codec_stepping_id;
    UINT8             codec_maj_rev;
    UINT8             codec_min_rev;
    UINT8             codec_num_function_groups;
    CHAR16            *codec_name;
} HDA_PROPERTIES;

typedef struct {
  CHAR16          *Name;
//  CHAR8           *LineName;
  INTN            Index;
  EFI_HANDLE      Handle;
  EFI_AUDIO_IO_PROTOCOL_DEVICE Device;
} HDA_OUTPUTS;

typedef struct {
  UINT16            SegmentGroupNum;
  UINT8             BusNum;
  UINT8             DevFuncNum;
  BOOLEAN           Valid;
//UINT8             DeviceN;
  UINT8             SlotID;
  UINT8             SlotType;
  CHAR8             SlotName[31];
} SLOT_DEVICE;

typedef struct {
  UINT32            Signature;
  LIST_ENTRY        Link;
  CHAR8             Model[64];
  UINT32            Id;
  UINT32            SubId;
  UINT64            VideoRam;
  UINTN             VideoPorts;
  BOOLEAN           LoadVBios;
} CARDLIST;

typedef struct {
    ///
    /// XXXX in BootXXXX.
    ///
  UINT16                     BootNum;
    ///
    /// Pointer to raw EFI_LOAD_OPTION (BootXXXX) variable content.
    ///
    VOID                     *Variable;
    ///
    /// Variable size in bytes.
    ///
    UINTN                    VariableSize;
    ///
    /// BootOption Attributes (first 4 bytes from Variable).
    ///
    UINT32                   Attributes;
    ///
    /// BootOption FilePathListLength (next 2 bytes from Variable).
    ///
    UINT16                   FilePathListLength;
    ///
    /// Null terminated BootOption Description (pointer to 6th byte of Variable).
    ///
    CONST CHAR16                   *Description;
    ///
    /// Size in bytes of BootOption Description.
    ///
    UINTN                    DescriptionSize;
    ///
    /// Pointer to BootOption FilePathList.
    ///
    EFI_DEVICE_PATH_PROTOCOL *FilePathList;
    ///
    /// Pointer to BootOption OptionalData.
    ///
    UINT8                    *OptionalData;
    ///
    /// BootOption OptionalData size in bytes.
    ///
    UINTN                    OptionalDataSize;
} BO_BOOT_OPTION;

#define CARDLIST_SIGNATURE SIGNATURE_32('C','A','R','D')



//extern CHAR8                          *msgbuf;
//extern CHAR8                          *msgCursor;
extern APPLE_SMBIOS_STRUCTURE_POINTER SmbiosTable;
extern GFX_PROPERTIES                 gGraphics[];
extern HDA_PROPERTIES                 gAudios[];
extern UINTN                          NGFX;
extern UINTN                          NHDA;
extern BOOLEAN                        gMobile;
extern BOOLEAN                        DoHibernateWake;
/* Switch for APFS support */
extern UINTN 						  APFSUUIDBankCounter;
extern UINT8 						 *APFSUUIDBank;
extern CONST CHAR16						 **SystemPlists;
extern CONST CHAR16                        **InstallPlists;
extern CONST CHAR16						 **RecoveryPlists;
extern EFI_GUID                        APFSSignature;
extern BOOLEAN                         APFSSupport;
//extern UINT32                         gCpuSpeed;  //kHz
//extern UINT16                         gCPUtype;
extern UINT64                         TurboMsr;
extern CONST CHAR8                          *BiosVendor;
extern EFI_GUID                       *gEfiBootDeviceGuid;
extern EFI_DEVICE_PATH_PROTOCOL       *gEfiBootDeviceData;
extern CHAR8                          *AppleSystemVersion[];
extern CHAR8                          *AppleFirmwareVersion[];
extern CHAR8                          *AppleReleaseDate[];
extern CONST CHAR8                          *AppleManufacturer;
extern CHAR8                          *AppleProductName[];
extern CHAR8                          *AppleSystemVersion[];
extern CHAR8                          *AppleSerialNumber[];
extern CHAR8                          *AppleFamilies[];
extern CHAR8                          *AppleBoardID[];
extern CHAR8                          *AppleChassisAsset[];
extern CONST CHAR8                          *AppleBoardSN;
extern CONST CHAR8                          *AppleBoardLocation;
extern EFI_SYSTEM_TABLE               *gST;
extern EFI_BOOT_SERVICES              *gBS;
extern SETTINGS_DATA                  gSettings;
extern LANGUAGES                      gLanguage;
extern BOOLEAN                        gFirmwareClover;
extern DRIVERS_FLAGS                  gDriversFlags;
extern UINT32                         gFwFeatures;
extern UINT32                         gFwFeaturesMask;
extern UINT64                         gPlatformFeature;
extern CPU_STRUCTURE                  gCPUStructure;
extern EFI_GUID                       gUuid;
extern SLOT_DEVICE                    SlotDevices[];
extern EFI_EDID_DISCOVERED_PROTOCOL   *EdidDiscovered;
//extern UINT8                          *gEDID;
extern UINT32                         mPropSize;
extern UINT8                          *mProperties;
extern CHAR8                          *gDeviceProperties;
extern UINT32                         cPropSize;
extern UINT8                          *cProperties;
extern CHAR8                          *cDeviceProperties;
extern INPUT_ITEM                     *InputItems;
extern BOOLEAN                        SavePreBootLog;
extern CHAR8                          *BootOSName;
//extern EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
extern UINT64                    machineSignature;

extern EFI_GUID                        gEfiAppleBootGuid;
extern EFI_GUID                        gEfiAppleNvramGuid;
extern EFI_GUID                        AppleSystemInfoProducerName;
extern EFI_GUID                        gAppleDevicePropertyProtocolGuid;
extern EFI_GUID                        gAppleFramebufferInfoProtocolGuid;
extern EFI_GUID                        gEfiAppleVendorGuid;
extern EFI_GUID                        gEfiPartTypeSystemPartGuid;
extern EFI_GUID                        gMsgLogProtocolGuid;
extern EFI_GUID                        gEfiLegacy8259ProtocolGuid;

extern EFI_EVENT                       mVirtualAddressChangeEvent;
extern EFI_EVENT                       OnReadyToBootEvent;
extern EFI_EVENT                       ExitBootServiceEvent;
extern EFI_EVENT                       mSimpleFileSystemChangeEvent;
extern UINTN                           gEvent;

extern UINT16                          gBacklightLevel;
extern UINT32                          devices_number;
//mouse
extern ACTION                          gAction;
extern UINTN                           gItemID;
extern INTN                            OldChosenTheme;
extern INTN                            OldChosenConfig;
extern INTN                            OldChosenDsdt;
extern UINTN                            OldChosenAudio;
extern UINT8                            DefaultAudioVolume;

//CHAR8*   orgBiosDsdt;
extern UINT64                          BiosDsdt;
extern UINT32                          BiosDsdtLen;
#define                                acpi_cpu_max 128
extern UINT8                           acpi_cpu_count;
extern CHAR8                           *acpi_cpu_name[];
extern UINT8                           acpi_cpu_processor_id[];
extern CHAR8                           *acpi_cpu_score;
extern BOOLEAN                         SSSE3;
extern BOOLEAN                         defDSM;
extern UINT16                          dropDSM;

extern TagPtr                          gConfigDict[];

// ACPI/PATCHED/AML
extern ACPI_PATCHED_AML                *ACPIPatchedAML;

// Sideload/inject kext
extern SIDELOAD_KEXT                   *InjectKextList;

// SysVariables
//extern SYSVARIABLES                   *SysVariables;

// Hold theme fixed IconFormat / extension
extern CHAR16                         *IconFormat;
extern CONST CHAR16                   *gFirmwareRevision;

extern BOOLEAN                        ResumeFromCoreStorage;
extern BOOLEAN                        gRemapSmBiosIsRequire;  // syscl: pass argument for Dell SMBIOS here

#ifdef _cplusplus
extern XObjArray<REFIT_VOLUME> Volumes;
#endif

//-----------------------------------

VOID
FixBiosDsdt (
  UINT8                                     *Dsdt,
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *fadt,
  CHAR8                                     *OSVersion
  );

VOID
RenameDevices(UINT8* table);

VOID
GetBiosRegions (
  UINT8  *buffer
  );

INT32
FindBin (
  UINT8  *Array,
  UINT32 ArrayLen,
  UINT8  *Pattern,
  UINT32 PatternLen
  );

EFI_STATUS
MouseBirth (VOID);

VOID
KillMouse (VOID);

VOID
HidePointer (VOID);

VOID
InitBooterLog (VOID);

EFI_STATUS
SetupBooterLog (
  BOOLEAN AllowGrownSize
  );

EFI_STATUS
SaveBooterLog (
  IN  EFI_FILE_HANDLE BaseDir  OPTIONAL,
  IN  CONST CHAR16 *FileName
  );

VOID
EFIAPI
DebugLog (
  IN        INTN  DebugMode,
  IN  CONST CHAR8 *FormatString, ...);

/** Prints series of bytes. */
VOID
PrintBytes (
  IN  VOID *Bytes,
  IN  UINTN Number
  );

VOID
SetDMISettingsForModel (
  MACHINE_TYPES Model,
  BOOLEAN Redefine
  );

MACHINE_TYPES GetModelFromString (
  CHAR8 *ProductName
  );

VOID
GetDefaultSettings(VOID);

VOID
FillInputs (
  BOOLEAN New
  );

VOID
ApplyInputs (VOID);


BOOLEAN
IsValidGuidAsciiString (
  IN CHAR8 *Str
  );


EFI_STATUS
StrToGuidLE (
  IN      CHAR16   *Str,
     OUT  EFI_GUID *Guid);

CHAR16 * GuidBeToStr(EFI_GUID *Guid);
CHAR16 * GuidLEToStr(EFI_GUID *Guid);

EFI_STATUS
InitializeConsoleSim (VOID);

EFI_STATUS
GuiEventsInitialize (VOID);

EFI_STATUS
InitializeEdidOverride (VOID);

UINT8*
getCurrentEdid (VOID);

EFI_STATUS
GetEdidDiscovered (VOID);

//Settings.c
UINT32
GetCrc32 (
  UINT8 *Buffer,
  UINTN Size
  );

VOID
GetCPUProperties (VOID);

VOID
GetDevices(VOID);

MACHINE_TYPES
GetDefaultModel (VOID);

UINT16
GetAdvancedCpuType (VOID);

CONST CHAR16
*GetOSIconName (
  IN  CONST CHAR8 *OSVersion
  );

EFI_STATUS
GetRootUUID (
  IN OUT REFIT_VOLUME *Volume
  );

EFI_STATUS
GetEarlyUserSettings (
  IN  EFI_FILE *RootDir,
      TagPtr   CfgDict
  );

EFI_STATUS
GetUserSettings (
  IN  EFI_FILE *RootDir,
      TagPtr CfgDict
  );

EFI_STATUS
InitTheme (
  BOOLEAN  UseThemeDefinedInNVRam,
  EFI_TIME *Time
  );

EFI_STATUS
StartupSoundPlay(EFI_FILE *Dir, CONST CHAR16* SoundFile);

VOID GetOutputs();

EFI_STATUS CheckSyncSound();

CHAR16*
GetOtherKextsDir (BOOLEAN On);

CHAR16*
GetOSVersionKextsDir (
  CHAR8 *OSVersion
  );

EFI_STATUS
InjectKextsFromDir (
  EFI_STATUS Status,
  CHAR16 *SrcDir
  );

VOID
ParseLoadOptions (
  OUT  CHAR16 **Conf,
  OUT  TagPtr *Dict
  );

//
// Nvram.c
//
VOID
*GetNvramVariable (
  IN      CONST CHAR16   *VariableName,
  IN      EFI_GUID *VendorGuid,
     OUT  UINT32   *Attributes    OPTIONAL,
     OUT  UINTN    *DataSize      OPTIONAL
     );

EFI_STATUS
AddNvramVariable (
  IN  CONST CHAR16   *VariableName,
  IN  EFI_GUID *VendorGuid,
  IN  UINT32   Attributes,
  IN  UINTN    DataSize,
  IN  VOID     *Data
  );

EFI_STATUS
SetNvramVariable (
  IN  CONST CHAR16      *VariableName,
  IN  EFI_GUID    *VendorGuid,
  IN  UINT32       Attributes,
  IN  UINTN        DataSize,
  IN  CONST VOID  *Data
  );

EFI_STATUS
DeleteNvramVariable (
  IN  CONST CHAR16   *VariableName,
  IN  EFI_GUID *VendorGuid
  );

VOID
ResetNvram (VOID);

BOOLEAN
IsDeletableVariable (
  IN CHAR16    *Name,
  IN EFI_GUID  *Guid
  );

EFI_STATUS
ResetNativeNvram (VOID);
;

EFI_STATUS
GetEfiBootDeviceFromNvram (VOID);

EFI_GUID
*FindGPTPartitionGuidInDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL *DevicePath
  );

VOID
PutNvramPlistToRtVars (VOID);

VOID
GetSmcKeys(BOOLEAN WriteToSMC);

//VOID DumpSmcKeys(VOID);

VOID
GetMacAddress(VOID);

EFI_STATUS
SetStartupDiskVolume (
  IN  REFIT_VOLUME *Volume,
  IN  CONST CHAR16       *LoaderPath
  );

VOID
RemoveStartupDiskVolume (VOID);

UINT64
GetEfiTimeInMs (IN EFI_TIME *T);


EFI_STATUS
EFIAPI
LogDataHub (
  EFI_GUID *TypeGuid,
  CONST CHAR16   *Name,
  VOID     *Data,
  UINT32   DataSize
  );

VOID
EFIAPI
SetupDataForOSX (BOOLEAN Hibernate);

EFI_STATUS
SetPrivateVarProto (VOID);

VOID
ScanSPD (VOID);

CONST CHAR8
*get_gma_model (
  IN UINT16 DeviceID
  );

#define BOOT_CHIME_VAR_ATTRIBUTES   (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)
#define BOOT_CHIME_VAR_DEVICE       (L"Device")
#define BOOT_CHIME_VAR_DEVICE_PATH  (L"device_path")
#define BOOT_CHIME_VAR_INDEX        (L"Index")
#define BOOT_CHIME_VAR_VOLUME       (L"Volume")


BOOLEAN
setup_hda_devprop (
  EFI_PCI_IO_PROTOCOL *PciIo,
  pci_dt_t *hda_dev,
  CHAR8 *OSVersion
  );


BOOLEAN
setup_nvidia_devprop (
  pci_dt_t *nvda_dev
  );

CONST CHAR8
*get_nvidia_model (
  UINT32 device_id,
  UINT32 subsys_id,
  CARDLIST * nvcard
  );

UINT32 PciAddrFromDevicePath(EFI_DEVICE_PATH_PROTOCOL* DevicePath);
//EFI_STATUS AddAudioOutput(EFI_HANDLE PciDevHandle);

VOID
FillCardList (
  TagPtr CfgDict
  );

CARDLIST
*FindCardWithIds (
  UINT32 Id,
  UINT32 SubId
  );

VOID
AddCard (
  CONST CHAR8 *Model,
  UINT32      Id,
  UINT32      SubId,
  UINT64      VideoRam,
  UINTN       VideoPorts,
  BOOLEAN     LoadVBios
  );

EG_IMAGE
*egDecodePNG (
  IN UINT8 *FileData,
  IN UINTN FileDataLength,
  IN BOOLEAN WantAlpha
  );

//ACPI
EFI_STATUS
PatchACPI(IN REFIT_VOLUME *Volume, CHAR8 *OSVersion);

EFI_STATUS
PatchACPI_OtherOS(CONST CHAR16* OsSubdir, BOOLEAN DropSSDT);

UINT8
Checksum8 (
  VOID *startPtr,
  UINT32 len
  );

void FixChecksum(EFI_ACPI_DESCRIPTION_HEADER* Table);

/*
BOOLEAN
tableSign (
  CHAR8       *table,
  CONST CHAR8 *sgn);
 */

VOID
SaveOemDsdt (
  BOOLEAN FullPatch
  );

VOID
SaveOemTables (VOID);

EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE
*GetFadt (VOID);

UINT32
FixAny (
  UINT8* dsdt,
  UINT32 len,
  UINT8* ToFind,
  UINT32 LenTF,
  UINT8* ToReplace,
  UINT32 LenTR
  );

VOID
GetAcpiTablesList (VOID);

EFI_STATUS
EjectVolume (
  IN REFIT_VOLUME *Volume
  );

EFI_STATUS
bootElTorito (
  IN REFIT_VOLUME *volume
  );

EFI_STATUS
bootMBR (
  IN REFIT_VOLUME *volume
  );

EFI_STATUS
bootPBR (
  IN REFIT_VOLUME *volume, BOOLEAN SataReset
  );

EFI_STATUS
bootPBRtest (
  IN REFIT_VOLUME *volume
  );

EFI_STATUS
bootLegacyBiosDefault (
  IN  UINT16 LegacyBiosDefaultEntry
  );

VOID
DumpBiosMemoryMap (VOID);

CHAR8*
XMLDecode (
  CHAR8 *src
  );

EFI_STATUS
ParseXML (
  CONST CHAR8  *buffer,
        TagPtr *dict,
        UINT32 bufSize
  );

EFI_STATUS ParseSVGTheme(CONST CHAR8* buffer, TagPtr * dict, UINT32 bufSize);
//VOID RenderSVGfont(NSVGfont  *fontSVG);

TagPtr
GetProperty (
        TagPtr dict,
  CONST CHAR8* key
  );

EFI_STATUS
XMLParseNextTag (
  CHAR8  *buffer,
  TagPtr *tag,
  UINT32 *lenPtr
  );

VOID
FreeTag (
  TagPtr tag
  );

EFI_STATUS
GetNextTag (
  UINT8  *buffer,
  CHAR8  **tag,
  UINT32 *start,
  UINT32 *length
  );

INTN
GetTagCount (
  TagPtr dict
  );

EFI_STATUS
GetElement (
  TagPtr dict,
  INTN   id,
  TagPtr *dict1
);

BOOLEAN
IsPropertyTrue (
  TagPtr Prop
  );

BOOLEAN
IsPropertyFalse (
  TagPtr Prop
  );

INTN
GetPropertyInteger (
  TagPtr Prop,
  INTN Default
  );

EFI_STATUS
SaveSettings (VOID);

UINTN
iStrLen(
  CONST CHAR8* String,
  UINTN  MaxLen
  );

EFI_STATUS
PrepatchSmbios (VOID);

VOID
PatchSmbios (VOID);

VOID
FinalizeSmbios (VOID);

EFI_STATUS
FixOwnership (VOID);

UINT8
*Base64DecodeClover (
  IN      CHAR8 *EncodedData,
     OUT  UINTN *DecodedSize
  );

UINT64
TimeDiff(
  UINT64 t0,
  UINT64 t1);

VOID
SetCPUProperties (VOID);

// Settings.c
// Micky1979: Next five functions (+ needed struct) are to split a string like "10.10.5,10.7,10.11.6,10.8.x"
// in their components separated by comma (in this case)
struct MatchOSes {
    INTN   count;
    CHAR8* array[100];
};

/** Returns a boolean and then enable disable the patch if MachOSEntry have a match for the booted OS. */
BOOLEAN IsPatchEnabled(CHAR8 *MatchOSEntry, CHAR8 *CurrOS);

/** return true if a given os contains '.' as separator,
 and then match components of the current booted OS. Also allow 10.10.x format meaning all revisions
 of the 10.10 OS */
BOOLEAN IsOSValid(CHAR8 *MatchOS, CHAR8 *CurrOS);

/** return MatchOSes struct (count+array) with the components of str that contains the given char sep as separator. */
struct
MatchOSes *GetStrArraySeparatedByChar(CHAR8 *str, CHAR8 sep);

/** trim spaces in MatchOSes struct array */
VOID
TrimMatchOSArray(struct MatchOSes *s);

/** free MatchOSes struct and its array. */
VOID deallocMatchOSes(struct MatchOSes *s);

/** count occurrences of a given char in a char* string. */
INTN
countOccurrences(CHAR8 *s, CHAR8 c);


CHAR16 *AddLoadOption(IN CONST CHAR16 *LoadOptions, IN CONST CHAR16 *LoadOption);
CHAR16 *RemoveLoadOption(IN CONST CHAR16 *LoadOptions, IN CONST CHAR16 *LoadOption);

//
// BootOptions.c
//

/** Finds and returns pointer to specified DevPath node in DevicePath or NULL. */
EFI_DEVICE_PATH_PROTOCOL *
FindDevicePathNodeWithType (
  IN  EFI_DEVICE_PATH_PROTOCOL *DevicePath,
  IN  UINT8                    Type,
  IN  UINT8                    SubType      OPTIONAL
  );

BOOLEAN EFIAPI IsHDMIAudio(EFI_HANDLE PciDevHandle);

//Parses BootXXXX (XXXX = BootNum) var (from BootOption->Variable) and returns it in BootOption.
EFI_STATUS
ParseBootOption (OUT BO_BOOT_OPTION  *BootOption);

/** Prints BootXXXX vars found listed in BootOrder, plus print others if AllBootOptions == TRUE. */
VOID
PrintBootOptions (
  IN  BOOLEAN AllBootOptions
  );

/** Prints BootOrder with DBG. */
VOID
PrintBootOrder (
    IN  UINT16 BootOrder[],
    IN  UINTN  BootOrderLen
                );

/** Reads BootXXXX (XXXX = BootNum) var, parses it and returns in BootOption.
 *  Caller is responsible for releasing BootOption->Variable with FreePool().
 */
EFI_STATUS
GetBootOption (
  IN      UINT16         BootNum,
     OUT  BO_BOOT_OPTION *BootOption
  );

/** Returns gEfiGlobalVariableGuid:BootOrder as UINT16 array and it's length (num of elements).
 *  Caller is responsible for releasing BootOrder mem (FreePool()).
 */
EFI_STATUS
GetBootOrder (
  OUT  UINT16 *BootOrder[],
  OUT  UINTN  *BootOrderLen
  );

/** Searches BootXXXX vars for entry that points to given FileDeviceHandle/FileName
 *  and returns BootNum (XXXX in BootXXXX variable name) and BootIndex (index in BootOrder)
 *  if found.
 */
EFI_STATUS
FindBootOptionForFile (
       IN      EFI_HANDLE FileDeviceHandle,
       IN      CHAR16     *FileName,
       OUT     UINT16     *BootNum,
       OUT     UINTN      *BootIndex
    );

/** Adds new boot option for given file system device handle FileDeviceHandle, file path FileName
 *  and Description, to be BootIndex in the list of options (0 based).
 *  If UseShortForm == TRUE, then only the hard drive media dev path will be used instead
 *  of full device path.
 *  Long (full) form:
 *   PciRoot(0x0)/Pci(0x1f,0x2)/Sata(0x1,0x0)/HD(1,GPT,96004846-a018-49ad-bc9f-4e5a340adc4b,0x800,0x64000)/\EFI\BOOT\File.efi
 *  Short form:
 *   HD(1,GPT,96004846-a018-49ad-bc9f-4e5a340adc4b,0x800,0x64000)/\EFI\BOOT\File.efi
 */
EFI_STATUS
AddBootOptionForFile (
                      IN  EFI_HANDLE FileDeviceHandle,
                      IN  CONST CHAR16     *FileName,
                      IN  BOOLEAN    UseShortForm,
                      IN  CONST CHAR16     *Description,
                      IN  UINT8      *OptionalData,
                      IN  UINTN      OptionalDataSize,
                      IN  UINTN      BootIndex,
                      OUT UINT16     *BootNum
                      );

/** Deletes boot option specified with BootNum (XXXX in BootXXXX var name). */
EFI_STATUS
DeleteBootOption (
  IN  UINT16 BootNum
  );


/** Deletes boot option for file specified with FileDeviceHandle and FileName. */
EFI_STATUS
DeleteBootOptionForFile (
  IN  EFI_HANDLE FileDeviceHandle,
  IN  CONST CHAR16     *FileName
  );

/** Deletes all boot option that points to a file which contains FileName in it's path. */
EFI_STATUS
DeleteBootOptionsContainingFile (
  IN  CHAR16 *FileName
  );

//get default boot
VOID GetBootFromOption(VOID);
VOID
InitKextList(VOID);

//
// PlatformDriverOverride.c
//
/** Registers given PriorityDrivers (NULL terminated) to highest priority during connecting controllers.
 *  Does this by installing our EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL
 *  or by overriding existing EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL.GetDriver.
 */
VOID
RegisterDriversToHighestPriority (
  IN  EFI_HANDLE *PriorityDrivers
  );

EFI_STATUS
LoadUserSettings (
  IN  EFI_FILE *RootDir,
      CONST CHAR16   *ConfName,
      TagPtr   *dict
  );

VOID
ParseSMBIOSSettings (
  TagPtr dictPointer
  );

UINT8 *APFSContainer_Support(VOID);

VOID SystemVersionInit(VOID);

EFI_GUID *APFSPartitionUUIDExtract(
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath
  );

UINTN
NodeParser  (UINT8 *DevPath, UINTN PathSize, UINT8 Type);

/** Prepares nvram vars needed for boot.efi to wake from hibernation. */
BOOLEAN
PrepareHibernation (
  IN REFIT_VOLUME *Volume
  );

//
// entry_scan
//
INTN
StrniCmp (
  IN CONST CHAR16 *Str1,
  IN CONST CHAR16 *Str2,
  IN UINTN  Count
  );

CONST CHAR16
*StriStr(
  IN CONST CHAR16 *Str,
  IN CONST CHAR16 *SearchFor
  );

VOID
StrToLower (
  IN CHAR16 *Str
  );

VOID
AlertMessage (
  IN CONST CHAR16 *Title,
  IN CONST CHAR16 *Message
  );

BOOLEAN
YesNoMessage (
  IN CONST CHAR16 *Title,
  IN CONST CHAR16 *Message);


#endif

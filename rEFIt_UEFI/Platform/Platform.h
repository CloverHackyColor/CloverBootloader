/*
Headers collection for procedures
*/

#ifndef __REFIT_PLATFORM_H__
#define __REFIT_PLATFORM_H__


#include <Uefi.h>

#include <Guid/Acpi.h>
#include <Guid/EventGroup.h>
#include <Guid/SmBios.h>
#include <Guid/Mps.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
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

#include <Framework/FrameworkInternalFormRepresentation.h>

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>

#include <Protocol/Cpu.h>
#include <Protocol/CpuIo.h>
#include <Protocol/DataHub.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/FrameworkHii.h>
#include <Protocol/Smbios.h>
#include <Protocol/VariableWrite.h>
#include <Protocol/Variable.h>

#include <Protocol/FSInjectProtocol.h>
#include <Protocol/MsgLog.h>

#include "lib.h"
#include "boot.h"
#include "BiosVideo.h"
#include "Bmp.h"
#include "efiConsoleControl.h"
#include "SmBios.h"
#include "EfiLib/GenericBdsLib.h"
#include "device_inject.h"
#include "UsbMass.h"

/* Decimal powers: */
#define kilo (1000ULL)
#define Mega (kilo * kilo)
#define Giga (kilo * Mega)
#define Tera (kilo * Giga)
#define Peta (kilo * Tera)

#define IS_COMMA(a)                ((a) == L',')
#define IS_HYPHEN(a)               ((a) == L'-')
#define IS_DOT(a)                  ((a) == L'.')
#define IS_LEFT_PARENTH(a)         ((a) == L'(')
#define IS_RIGHT_PARENTH(a)        ((a) == L')')
#define IS_SLASH(a)                ((a) == L'/')
#define IS_NULL(a)                 ((a) == L'\0')
#define IS_DIGIT(a)            (((a) >= '0') && ((a) <= '9'))
#define IS_HEX(a)            (((a) >= 'a') && ((a) <= 'f'))
#define IS_UPPER(a)          (((a) >= 'A') && ((a) <= 'Z'))
#define IS_ALFA(x) (((x >= 'a') && (x <='z')) || ((x >= 'A') && (x <='Z')))
#define IS_ASCII(x) ((x>=0x20) && (x<=0x7F))
#define IS_PUNCT(x) ((x == '.') || (x == '-'))


#define EBDA_BASE_ADDRESS 0x40E
#define EFI_SYSTEM_TABLE_MAX_ADDRESS 0xFFFFFFFF
#define ROUND_PAGE(x)  ((((unsigned)(x)) + EFI_PAGE_SIZE - 1) & ~(EFI_PAGE_SIZE - 1))

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

#define SAFE_LOG_SIZE	80

#define MSG_LOG_SIZE	(64 * 1024)
#define MsgLog(x...) {AsciiSPrint(msgCursor, MSG_LOG_SIZE, x); while(*msgCursor){msgCursor++;}}

#define CPU_MODEL_DOTHAN        0x0D
#define CPU_MODEL_YONAH         0x0E
#define CPU_MODEL_MEROM         0x0F  /* same as CONROE but mobile */
#define CPU_MODEL_CONROE        0x0F  
#define CPU_MODEL_CELERON       0x16  /* ever see? */
#define CPU_MODEL_PENRYN        0x17  
#define CPU_MODEL_WOLFDALE      0x17  
#define CPU_MODEL_NEHALEM       0x1A
#define CPU_MODEL_ATOM          0x1C
#define CPU_MODEL_XEON_MP       0x1D  /* ever see? */
#define CPU_MODEL_FIELDS        0x1E
#define CPU_MODEL_DALES         0x1F
#define CPU_MODEL_CLARKDALE     0x25
#define CPU_MODEL_LINCROFT      0x27
#define CPU_MODEL_SANDY_BRIDGE	0x2A
#define CPU_MODEL_WESTMERE      0x2C
#define CPU_MODEL_JAKETOWN      0x2D  /* ever see? */
#define CPU_MODEL_NEHALEM_EX    0x2E
#define CPU_MODEL_WESTMERE_EX   0x2F

#define CPU_VENDOR_INTEL  0x756E6547
#define CPU_VENDOR_AMD    0x68747541
/* Unknown CPU */
#define CPU_STRING_UNKNOWN	"Unknown CPU Type"

//definitions from Apple XNU

/* CPU defines */
#define bit(n)			(1UL << (n))
#define _Bit(n)			(1ULL << n)
#define _HBit(n)		(1ULL << ((n)+32))

#define bitmask(h,l)	((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)	(((x) & bitmask(h,l)) >> l)
#define quad(hi,lo)     (((UINT64)(hi)) << 32 | (lo))

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
//MSR 000001B0                                      0000-0000-0000-0005
//MSR 000001B1                                      0000-0000-8838-0000
#define IA32_PLATFORM_DCA_CAP		0x01F8
//MSR 000001FC                                      0000-0000-0004-005F


// Sandy Bridge & JakeTown specific 'Running Average Power Limit' MSR's.
#define MSR_RAPL_POWER_UNIT			0x606
//MSR 00000606                                      0000-0000-000A-1003
//MSR 0000060B                                      0000-0000-0000-8854
//MSR 0000060C                                      0000-0000-0000-8854

#define MSR_PKG_RAPL_POWER_LIMIT	0x610
//MSR 00000610                                      0000-A580-0000-8960
#define MSR_PKG_ENERGY_STATUS		0x611
//MSR 00000611                                      0000-0000-3212-A857
#define MSR_PKG_PERF_STATUS			0x613
#define MSR_PKG_POWER_INFO			0x614
//MSR 00000614                                      0000-0000-01E0-02F8
// Sandy Bridge IA (Core) domain MSR's.
#define MSR_PP0_POWER_LIMIT			0x638
#define MSR_PP0_ENERGY_STATUS		0x639
#define MSR_PP0_POLICY          0x63A
#define MSR_PP0_PERF_STATUS			0x63B

// Sandy Bridge Uncore (IGPU) domain MSR's (Not on JakeTown).
#define MSR_PP1_POWER_LIMIT			0x640
#define MSR_PP1_ENERGY_STATUS		0x641
//MSR 00000641                                      0000-0000-0000-0000
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


/* CPUID Index */ 
#define CPUID_0		0 
#define CPUID_1		1 
#define CPUID_2		2 
#define CPUID_3		3 
#define CPUID_4		4 
#define CPUID_80	5 
#define CPUID_81	6
#define CPUID_87  7
#define CPUID_MAX	8

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

/* CPU Cache */
#define MAX_CACHE_COUNT  4
#define CPU_CACHE_LEVEL  3

/* PCI */
#define PCI_BASE_ADDRESS_0					0x10		/* 32 bits */
#define PCI_BASE_ADDRESS_1					0x14		/* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2					0x18		/* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3					0x1c		/* 32 bits */
#define PCI_BASE_ADDRESS_4					0x20		/* 32 bits */
#define PCI_BASE_ADDRESS_5					0x24		/* 32 bits */

#define PCI_CLASS_MEDIA_HDA         0x03

#define GEN_PMCON_1                 0xA0

#define PCIADDR(bus, dev, func) ((1 << 31) | (bus << 16) | (dev << 11) | (func << 8))
#define REG8(base, reg)  ((volatile UINT8 *)(UINTN)base)[(reg)]
#define REG16(base, reg)  ((volatile UINT16 *)(UINTN)base)[(reg) >> 1]
#define REG32(base, reg)  ((volatile UINT32 *)(UINTN)base)[(reg) >> 2]
#define WRITEREG32(base, reg, value) REG32(base, reg) = value

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

#define	AML_CHUNK_NONE		0xff
#define	AML_CHUNK_ZERO		0x00
#define	AML_CHUNK_ONE     0x01
#define	AML_CHUNK_ALIAS		0x06
#define	AML_CHUNK_NAME		0x08
#define	AML_CHUNK_BYTE		0x0A
#define	AML_CHUNK_WORD		0x0B
#define	AML_CHUNK_DWORD		0x0C
#define	AML_CHUNK_STRING	0x0D
#define	AML_CHUNK_QWORD		0x0E
#define	AML_CHUNK_SCOPE		0x10
#define	AML_CHUNK_PACKAGE	0x12
#define	AML_CHUNK_METHOD	0x14
#define AML_CHUNK_RETURN  0xA4
#define AML_LOCAL0        0x60
#define AML_STORE_OP      0x70

struct aml_chunk 
{
	UINT8     Type;
	UINT16		Length;
	CHAR8*		Buffer;
	
	UINT16		Size;
	
	struct aml_chunk*	Next;
	struct aml_chunk*	First;
	struct aml_chunk*	Last;
};

typedef struct aml_chunk AML_CHUNK;

struct p_state 
{
	union 
	{
		UINT16 Control;
		struct 
		{
			UINT8 VID;	// Voltage ID
			UINT8 FID;	// Frequency ID
		};
	};
	
	UINT32		CID;		// Compare ID
	UINT32	Frequency;
};

typedef struct p_state P_STATE;

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

#pragma pack(1)

struct Symbol {
	UINT32        refCount;
	struct Symbol *next;
	CHAR8         string[1];
};

typedef struct Symbol Symbol, *SymbolPtr;

typedef struct {
  
  UINT32		type;
  CHAR8     *string;
  UINT32		offset;
  VOID      *tag;
  VOID      *tagNext;
  
}Tag, *TagPtr;

typedef struct {
  
  EFI_ACPI_DESCRIPTION_HEADER   Header;
  UINT32						Entry;
  
} RSDT_TABLE;

typedef struct {
  
  EFI_ACPI_DESCRIPTION_HEADER   Header;
  UINT64						Entry;
  
} XSDT_TABLE;

typedef struct {
  
	// SMBIOS TYPE0
	CHAR8	VendorName[64];
	CHAR8	RomVersion[64];
	CHAR8	ReleaseDate[64];
	// SMBIOS TYPE1
	CHAR8	ManufactureName[64];
	CHAR8	ProductName[64];
	CHAR8	VersionNr[64];
	CHAR8	SerialNr[64];
//	CHAR8	Uuid[64];
//	CHAR8	SKUNumber[64];
	CHAR8	FamilyName[64];
  CHAR8 OEMProduct[64];
	// SMBIOS TYPE2
	CHAR8	BoardManufactureName[64];
	CHAR8	BoardSerialNumber[64];
	CHAR8	BoardNumber[64]; //Board-ID
	CHAR8	LocationInChassis[64];
  CHAR8 BoardVersion[64];
  CHAR8 OEMBoard[64];
	// SMBIOS TYPE3
  BOOLEAN Mobile;
	CHAR8	ChassisManufacturer[64];
	CHAR8	ChassisAssetTag[64]; 
	// SMBIOS TYPE4
	UINT16	CpuFreqMHz;
	UINT32	BusSpeed; //in kHz
  BOOLEAN Turbo;
  
	// SMBIOS TYPE17
	CHAR8	MemoryManufacturer[64];
	CHAR8	MemorySerialNumber[64];
	CHAR8	MemoryPartNumber[64];
	CHAR8	MemorySpeed[64];
	// SMBIOS TYPE131
	UINT16	CpuType;
  // SMBIOS TYPE132
  UINT16	QPI;
  
	// OS parameters
	CHAR8 	Language[16];
	CHAR8   BootArgs[256];
	CHAR16	CustomUuid[40];
  CHAR16  DefaultBoot[40];
	
	// GUI parameters
	BOOLEAN	Debug;
  
	//ACPI
	UINT64	ResetAddr;
	UINT8 	ResetVal;
	BOOLEAN	UseDSDTmini;  
	BOOLEAN	DropSSDT;
	BOOLEAN	GeneratePStates;
  BOOLEAN	GenerateCStates;
  UINT8   PLimitDict;
  UINT8   UnderVoltStep;
  BOOLEAN LpcTune;
  BOOLEAN EnableC2;
  BOOLEAN EnableC4;
  BOOLEAN EnableC6;
  BOOLEAN EnableISS;
	BOOLEAN	smartUPS;
  BOOLEAN PatchNMI;
	CHAR16	DsdtName[60];
  
  //Injections
  BOOLEAN StringInjector;
  BOOLEAN InjectSystemID;
  //Graphics
  UINT16  PCIRootUID;
  BOOLEAN GraphicsInjector;
  BOOLEAN LoadVBios;
  BOOLEAN PatchVBios;
  CHAR16  FBName[16];
  UINT16  VideoPorts;
  UINT64  VRAM;
  UINT8   Dcfg[8];
  UINT8   NVCAP[20];
 	
 	// HDA
 	BOOLEAN HDAInjection;
 	UINTN   HDALayoutId;
  
} SETTINGS_DATA;

typedef struct {
 //values from CPUID 
	UINT32	CPUID[CPUID_MAX][4];
	UINT32  Vendor;
	UINT32	Signature;
	UINT32	Family;
	UINT32	Model;
	UINT32	Stepping;
	UINT32	Type;
	UINT32	Extmodel;
	UINT32	Extfamily;
	UINT64  Features;
	UINT64  ExtFeatures;
	UINT32	CoresPerPackage;
	UINT32  LogicalPerPackage;  
	CHAR8   BrandString[48];
  
  //values from BIOS
	UINT32	ExternalClock; //keep this values as kHz
	UINT32	MaxSpeed;       //MHz
	UINT32	CurrentSpeed;   //MHz
  UINT32  Pad;
  
  //calculated from MSR
  UINT64  MicroCode;
  UINT64  ProcessorFlag;
	UINT32	MaxRatio;
  UINT32  SubDivider;
	UINT32	MinRatio;
  UINT32  DynFSB;
	UINT64  ProcessorInterconnectSpeed;
	UINT64	FSBFrequency; //Hz
	UINT64	CPUFrequency;
	UINT64	TSCFrequency;
	UINT8   Cores;
  UINT8   EnabledCores;
	UINT8   Threads;
	UINT8   Mobile;  //not for i3-i7
  
	/* Core i7,5,3 */
	UINT16  Turbo1; //1 Core
	UINT16  Turbo2; //2 Core
	UINT16  Turbo3; //3 Core
	UINT16  Turbo4; //4 Core
    
} CPU_STRUCTURE;

typedef enum {
  
	MacBook11,
	MacBook21,
	MacBook41,
	MacBook52,
	MacBookPro51,
	MacBookPro81,
	MacBookPro83,
	MacBookAir31,
	MacMini21,
	iMac81,
	iMac101,
	iMac112,
	iMac121,
  iMac122,
	MacPro31,
	MacPro41,
	MacPro51
  
} MACHINE_TYPES;

typedef struct {
	UINT8   Type;
	UINT8   BankConnections;
	UINT8   BankConnectionCount;
	UINT32	ModuleSize;
	UINT32	Frequency;
	CHAR8*	Vendor;
	CHAR8*	PartNo;
	CHAR8*	SerialNo;
	UINT8   *spd;
	BOOLEAN	InUse;
} RAM_SLOT_INFO; 

#define MAX_SLOT_COUNT	8
#define MAX_RAM_SLOTS	16

typedef struct {
  
	UINT64		Frequency;
	UINT32		Divider;
	UINT8			TRC;
	UINT8			TRP;
	UINT8			RAS;
	UINT8			Channels;
	UINT8			Slots;
	UINT8			Type;
  
	RAM_SLOT_INFO	DIMM[MAX_RAM_SLOTS];
  
} MEM_STRUCTURE;
//unused
typedef struct {
	UINT8     MaxMemorySlots;			// number of memory slots polulated by SMBIOS
	UINT8     CntMemorySlots;			// number of memory slots counted
	UINT16		MemoryModules;			// number of memory modules installed
	UINT32		DIMM[MAX_RAM_SLOTS];	// Information and SPD mapping for each slot
} DMI;

typedef enum {
  english,  //en
  russian,  //ru
  chinese   //cn
  //something else? add, please
} LANGUAGES;

typedef enum {
  Unknown,
	Ati,
	Intel,
	Nvidia
  
} GFX_MANUFACTERER;

typedef struct {
  GFX_MANUFACTERER  Vendor;
  UINT8             Ports;  
  UINT16            DeviceID;
  UINT16            Width;
  UINT16            Height;
  CHAR8             Model[64];
  CHAR8             Config[64];
  BOOLEAN           LoadVBios;
//  BOOLEAN           PatchVBios;
} GFX_PROPERTIES;

#pragma pack(0)
extern CHAR8                    *msgbuf;
extern CHAR8                    *msgCursor;
extern SMBIOS_STRUCTURE_POINTER	SmbiosTable;
extern GFX_PROPERTIES           gGraphics[];
extern UINTN                    NGFX;
extern BOOLEAN                  gMobile;
//extern UINT32                   gCpuSpeed;  //kHz
//extern UINT16                   gCPUtype;
extern UINT64                   TurboMsr;
extern CHAR8*                   BiosVendor;
extern UINT32                   mPropSize;
extern UINT8*                   mProperties;
extern CHAR8                    gSelectedUUID[];
extern CHAR8*                   AppleSystemVersion[];
extern CHAR8*	                  AppleFirmwareVersion[];
extern CHAR8*	                  AppleReleaseDate[];
extern CHAR8*	                  AppleManufacturer;
extern CHAR8*	                  AppleProductName[];
extern CHAR8*	                  AppleSystemVersion[];
extern CHAR8*	                  AppleSerialNumber[];
extern CHAR8*	                  AppleFamilies[];
extern CHAR8*	                  AppleBoardID[];
extern CHAR8*	                  AppleChassisAsset[];
extern CHAR8*	                  AppleBoardSN;
extern CHAR8*	                  AppleBoardLocation;
extern EFI_SYSTEM_TABLE*        gST;
extern EFI_BOOT_SERVICES*       gBS; 
extern SETTINGS_DATA            gSettings;
extern LANGUAGES                gLanguage;
extern BOOLEAN                  gFirmwareClover;
extern CPU_STRUCTURE            gCPUStructure;
extern EFI_GUID                 gUuid;
extern EFI_EDID_DISCOVERED_PROTOCOL*            EdidDiscovered;
extern UINT8                                    *gEDID;
extern CHAR8*                   gDeviceProperties;
extern CHAR8*                   cDeviceProperties;
extern INPUT_ITEM               *InputItems;

extern EFI_GUID	gEfiAppleBootGuid;
extern EFI_GUID	gEfiAppleNvramGuid;
extern EFI_GUID AppleSystemInfoProducerName;
extern EFI_GUID AppleDevicePropertyProtocolGuid;
extern EFI_GUID gEfiAppleScreenInfoGuid;
extern EFI_GUID gEfiAppleVendorGuid;
extern EFI_GUID gEfiPartTypeSystemPartGuid;
extern EFI_GUID gMsgLogProtocolGuid;
extern EFI_GUID gEfiLegacy8259ProtocolGuid;

extern EFI_EVENT  mVirtualAddressChangeEvent;
extern EFI_EVENT  OnReadyToBootEvent;
extern EFI_EVENT  ExitBootServiceEvent;
extern EFI_EVENT  mSimpleFileSystemChangeEvent;
extern UINTN      gEvent;

VOID        WaitForSts(VOID);

VOID        InitBooterLog(VOID);
EFI_STATUS  SetupBooterLog(VOID);
VOID        GetDefaultSettings(VOID);
VOID        FillInputs(VOID);
VOID        ApplyInputs(VOID);

EFI_STATUS  StrToGuid (IN  CHAR16   *Str, OUT EFI_GUID *Guid);
EFI_STATUS  StrToGuidLE (IN  CHAR16   *Str, OUT EFI_GUID *Guid);
BOOLEAN     hex2bin(IN CHAR8 *hex, OUT UINT8 *bin, INT32 len);
UINT8       hexstrtouint8 (CHAR8* buf); //one or two hex letters to one byte

EFI_STATUS  InitializeConsoleSim (VOID);
EFI_STATUS  GuiEventsInitialize (VOID);
//Settings.c
UINT32          GetCrc32(UINT8 *Buffer, UINTN Size);
VOID            GetCPUProperties (VOID);
VOID            GetDevices(VOID);
MACHINE_TYPES   GetDefaultModel(VOID);
UINT16          GetAdvancedCpuType(VOID);
EFI_STATUS      GetOSVersion(IN REFIT_VOLUME *Volume);
EFI_STATUS      GetUserSettings(IN EFI_FILE *RootDir);
EFI_STATUS      GetNVRAMSettings(IN EFI_FILE *RootDir, CHAR16* NVRAMPlistPath);
EFI_STATUS      GetEdid(VOID);
EFI_STATUS      SetFSInjection(IN LOADER_ENTRY *Entry);

EFI_STATUS
LogDataHub(
           EFI_GUID					*TypeGuid,
           CHAR16           *Name,
           VOID             *Data,
           UINT32           DataSize);

EFI_STATUS SetVariablesForOSX();
VOID       SetupDataForOSX();
EFI_STATUS SetPrivateVarProto(VOID);
VOID       SetDevices(VOID);
VOID       ScanSPD();
BOOLEAN setup_ati_devprop(pci_dt_t *ati_dev);
BOOLEAN setup_gma_devprop(pci_dt_t *gma_dev);
CHAR8*  get_gma_model(IN UINT16 DeviceID);
BOOLEAN setup_nvidia_devprop(pci_dt_t *nvda_dev);
CHAR8*  get_nvidia_model(IN UINT16 DeviceID);


EG_IMAGE * egDecodePNG(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);

EFI_STATUS  PatchACPI(IN REFIT_VOLUME *Volume);
EFI_STATUS  PatchACPI_OtherOS(CHAR16* OsSubdir, BOOLEAN DropSSDT);
UINT8       Checksum8(VOID * startPtr, UINT32 len);
BOOLEAN     tableSign(CHAR8 *table, CONST CHAR8 *sgn);
VOID        SaveOemDsdt(VOID);

EFI_STATUS  EventsInitialize(VOID);
EFI_STATUS  EjectVolume(IN REFIT_VOLUME *Volume);

EFI_STATUS  bootElTorito(IN REFIT_VOLUME*	volume);
EFI_STATUS  bootMBR(IN REFIT_VOLUME* volume);
EFI_STATUS  bootPBR(IN REFIT_VOLUME* volume);

CHAR8*      XMLDecode(const CHAR8* src);
EFI_STATUS  ParseXML(const CHAR8* buffer, TagPtr * dict);
TagPtr      GetProperty( TagPtr dict, const CHAR8* key );
EFI_STATUS  XMLParseNextTag(CHAR8* buffer, TagPtr * tag, UINT32* lenPtr);
VOID        FreeTag( TagPtr tag );
EFI_STATUS  GetNextTag( UINT8* buffer, CHAR8** tag, UINT32* start,UINT32* length);

EFI_STATUS  SaveSettings(VOID);

UINTN       iStrLen(CHAR8* String, UINTN MaxLen);
EFI_STATUS  PrepatchSmbios(VOID);
VOID        PatchSmbios(VOID);
VOID        FinalizeSmbios(VOID);

EFI_STATUS  DisableUsbLegacySupport(VOID);

#endif

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

#include "lib.h"
#include "boot.h"
#include "BiosVideo.h"
#include "Bmp.h"
#include "efiConsoleControl.h"
#include "SmBios.h"
//#include "GenericBdsLib.h"
#include "device_inject.h"

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
#define IS_ALFA(x) (((x >= 'a') && (x <='z')) || ((x >= 'A') && (x <='Z')))
#define IS_ASCII(x) ((x>=0x20) && (x<=0x7F))
#define IS_PUNCT(x) ((x == '.') || (x == '-'))



#define EBDA_BASE_ADDRESS 0x40E
#define EFI_SYSTEM_TABLE_MAX_ADDRESS 0xFFFFFFFF
#define ROUND_PAGE(x)  ((((unsigned)(x)) + EFI_PAGE_SIZE - 1) & ~(EFI_PAGE_SIZE - 1))


#define SAFE_LOG_SIZE	80

#define BOOTER_LOG_SIZE	(64 * 1024)
#define MsgLog(x...) {AsciiSPrint(msgCursor, BOOTER_LOG_SIZE, x); while(*msgCursor){msgCursor++;}}

#define CPU_MODEL_DOTHAN        0x0D
#define CPU_MODEL_YONAH         0x0E
#define CPU_MODEL_MEROM         0x0F  /* same as CONROE but mobile */
#define CPU_MODEL_CELERON       0x16  /* ever see? */
#define CPU_MODEL_PENRYN        0x17  
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

#define PCIADDR(bus, dev, func) ((1 << 31) | (bus << 16) | (dev << 11) | (func << 8))
#define REG8(base, reg)  ((volatile UINT8 *)(UINTN)base)[(reg)]
#define REG16(base, reg)  ((volatile UINT16 *)(UINTN)base)[(reg) >> 1]
#define REG32(base, reg)  ((volatile UINT32 *)(UINTN)base)[(reg) >> 2]
#define WRITEREG32(base, reg, value) REG32(base, reg) = value

enum {
	kTagTypeNone = 0,
	kTagTypeDict,
	kTagTypeKey,
	kTagTypeString,
	kTagTypeInteger,
	kTagTypeData,
	kTagTypeDate,
	kTagTypeFalse,
	kTagTypeTrue,
	kTagTypeArray
};

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
  CHAR16 OEMProduct[64];
	// SMBIOS TYPE2
	CHAR8	BoardManufactureName[64];
	CHAR8	BoardSerialNumber[64];
	CHAR8	BoardNumber[64]; //Board-ID
	CHAR8	LocationInChassis[64];
	// SMBIOS TYPE3
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
  
	// OS parameters
	CHAR16	Language[10];
	CHAR8   BootArgs[120];
	CHAR16	CustomUuid[40];
  CHAR16  DefaultBoot[40];
	
	// GUI parameters
	BOOLEAN	Debug;
  
	//ACPI
	UINT16	ResetAddr;
	UINT16	ResetVal;
	BOOLEAN	UseDSDTmini;  
	BOOLEAN	DropSSDT;
	BOOLEAN	smartUPS;
	CHAR16	DsdtName[60];
    
  //Graphics
  BOOLEAN GraphicInjector;
  BOOLEAN LoadVBios;
  CHAR16  FBName[16];
  UINT16  VideoPorts;
  
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
  
  //calculated from MSR
  UINT64  MicroCode;
  UINT64  ProcessorFlag;
	UINT32	MaxRatio;
  UINT32  SubDivider;
	UINT32	MinRatio;
	UINT64  ProcessorInterconnectSpeed;
	UINT64	FSBFrequency; //Hz
	UINT64	CPUFrequency;
	UINT64	TSCFrequency;
	UINT8   Cores;
  UINT8   EnabledCores;
	UINT8   Threads;
	UINT8   Mobile;  //not for i3-i7
  
	/* Core i7,5,3 */
	UINT8   Turbo1; //1 Core
	UINT8   Turbo2; //2 Core
	UINT8   Turbo3; //3 Core
	UINT8   Turbo4; //4 Core
    
} CPU_STRUCTURE;

typedef enum {
  
	MacBook11,
	MacBook21,
	MacBook41,
	MacBook52,
	MacBookPro51,
	MacBookAir31,
	MacMini21,
	iMac81,
	iMac101,
	iMac112,
	iMac121,
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
  Unknown,
	Ati,
	Intel,
	Nvidia
  
} GFX_MANUFACTERER;

typedef struct {
  GFX_MANUFACTERER  Vendor;
  UINT16            DeviceID;
  UINT16            Width;
  UINT16            Height;
} GFX_PROPERTIES;
#pragma pack(0)
extern CHAR8                    *msgbuf;
extern CHAR8                    *msgCursor;
extern SMBIOS_STRUCTURE_POINTER	SmbiosTable;
extern GFX_PROPERTIES           gGraphics;
extern BOOLEAN                  gMobile;
extern UINT32                   gCpuSpeed;  //kHz
extern UINT32                   gBusSpeed;  //kHz
extern UINT16                   gCPUtype;
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
extern CPU_STRUCTURE            gCPUStructure;
extern EFI_GUID                 gUuid;
extern CHAR8                    gOEMProduct[];  //original name from SMBIOS
extern EFI_EDID_DISCOVERED_PROTOCOL*            EdidDiscovered;
extern CHAR8*                   gDeviceProperties;
extern UINT16                   gResetAddress;
extern UINT16                   gResetValue;

extern EFI_GUID	gEfiAppleBootGuid;
extern EFI_GUID	gEfiAppleNvramGuid;
extern EFI_GUID AppleSystemInfoProducerName;
extern EFI_GUID AppleDevicePropertyProtocolGuid;
extern EFI_GUID gEfiAppleScreenInfoGuid;
extern EFI_GUID gEfiAppleVendorGuid;
extern EFI_GUID gEfiPartTypeSystemPartGuid;
extern EFI_GUID gMsgLogProtocolGuid;
extern EFI_GUID gEfiLegacy8259ProtocolGuid;


VOID        InitBooterLog(VOID);
EFI_STATUS  SetupBooterLog(VOID);
VOID        GetDefaultSettings(VOID);

EFI_STATUS StrToGuid (IN  CHAR16   *Str, OUT EFI_GUID *Guid);
BOOLEAN hex2bin(IN CHAR8 *hex, OUT UINT8 *bin, INT32 len);

EFI_STATUS InitializeConsoleSim (VOID);

//Settings.c
VOID            GetCPUProperties (VOID);
MACHINE_TYPES   GetDefaultModel(VOID);
UINT16          GetAdvancedCpuType(VOID);
EFI_STATUS      GetOSVersion(IN REFIT_VOLUME *Volume);
EFI_STATUS      GetUserSettings(IN EFI_FILE *RootDir);
EFI_STATUS      GetNVRAMSettings(IN EFI_FILE *RootDir, CHAR16* NVRAMPlistPath);
EFI_STATUS      GetEdid(VOID);

EFI_STATUS
LogDataHub(
           EFI_GUID					*TypeGuid,
           CHAR16           *Name,
           VOID             *Data,
           UINT32           DataSize);

EFI_STATUS SetVariablesForOSX();
VOID       SetupDataForOSX();
EFI_STATUS SetPrivateVarProto(VOID);
VOID       SetGraphics(VOID);
VOID       ScanSPD();
BOOLEAN setup_ati_devprop(pci_dt_t *ati_dev);
BOOLEAN setup_gma_devprop(pci_dt_t *gma_dev);
BOOLEAN setup_nvidia_devprop(pci_dt_t *nvda_dev);


EG_IMAGE * egDecodePNG(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);

EFI_STATUS  PatchACPI(IN REFIT_VOLUME *Volume);
UINT8       Checksum8(VOID * startPtr, UINT32 len);

EFI_STATUS  EventsInitialize(VOID);

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

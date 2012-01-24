/*
Headers collection for procedures
*/

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
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

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


#include "boot.h"
#include "BiosVideo.h"
#include "Bmp.h"
#include "efiConsoleControl.h"

#define SAFE_LOG_SIZE	80

#define BOOTER_LOG_SIZE	(64 * 1024)
#define MsgLog(x...) {AsciiSPrint(msgCursor, BOOTER_LOG_SIZE, x); while(*msgCursor){msgCursor++;}}


typedef struct {
  
  UINT32		type;
  CHAR8     *string;
  UINT32		offset;
  VOID      *tag;
  VOID      *tagNext;
  
}Tag, *TagPtr;

typedef struct {
  
	// SMBIOS TYPE0
	CHAR16	VendorName[64];
	CHAR16	RomVersion[64];
	CHAR16	ReleaseDate[64];
	// SMBIOS TYPE1
	CHAR16	ManufactureName[64];
	CHAR16	ProductName[64];
	CHAR16	VersionNr[64];
	CHAR16	SerialNr[64];
	CHAR16	Uuid[64];
	CHAR16	SKUNumber[64];
	CHAR16	FamilyName[64];
	// SMBIOS TYPE2
	CHAR16	BoardManufactureName[64];
	CHAR16	BoardSerialNumber[64];
	CHAR16	BoardNumber[64]; //Board-ID
	CHAR16	LocationInChassis[64];
	// SMBIOS TYPE3
	CHAR16	ChassisManufacturer[64];
	CHAR16	ChassisAssetTag[64]; 
	// SMBIOS TYPE4
	CHAR16	ProcessorTray[64];
	CHAR16	CpuFreqMHz[5];
	CHAR16	CPUSerial[10];  //microcode?
                          // SMBIOS TYPE16
	CHAR16	NumberOfMemorySlots[3];
	// SMBIOS TYPE17
	CHAR16	MemoryManufacturer[64];
	CHAR16	MemorySerialNumber[64];
	CHAR16	MemoryPartNumber[64];
	CHAR16	MemorySpeed[64];
	// SMBIOS TYPE131
	CHAR16	CpuType[10];
	// SMBIOS TYPE132
	CHAR16	BusSpeed[10];
	// OS X Args
	CHAR16	Language[10];
	CHAR16	KernelFlags[120];
	CHAR16	DsdtName[60];
	
	// System parameters
	BOOLEAN	Debug;
	BOOLEAN	DropSSDT;
	BOOLEAN	smartUPS;
	BOOLEAN	ShowLegacyBoot;
	BOOLEAN	UseDSDTmini;
	// User TimeOut
	CHAR16	TimeOut[3];
  
	CHAR16	CustomUuid[40];
	//FADT
	CHAR16	ResetAddr[7];
	CHAR16	ResetVal[5];
} SETTINGS_DATA;

typedef struct {
  
	CHAR8	BrandString[48];
	UINT8	Cores;
	UINT8	Threads;
	UINT8	Mobile;
	UINT8	MaxCoef;
	UINT8	MaxDiv;
	UINT8	CurrCoef;
	UINT8	CurrDiv;
	UINT8	FlexRatio;
	UINT8	BusRatioMax;
	UINT8	BusRatioMin;
	UINT16	ExternalClock;
	UINT16	MaxSpeed;
	UINT16	CurrentSpeed;
	UINT32  Vendor;
	UINT32	Family;
	UINT32	Model;
	UINT32	Stepping;
	UINT32	Type;
	UINT32	Extmodel;
	UINT32	Extfamily;
	UINT32	Signature;
	UINT64  Features;
	UINT64  ExtFeatures;
	UINT32	MaxRatio;
	UINT32	MinRatio;
	UINT32	TMS;
	UINT32	IDA;
  //	UINT64	FrontSideBus;
	UINT64  ProcessorInterconnectSpeed;
	UINT64  UserSetting;
	UINT64	FSBFrequency;
	UINT64	CPUFrequency;
	UINT64	TSCFrequency;
	UINT32	CoresPerPackage;
	UINT32  LogicalPerPackage;
  
	/* Core i7,5,3 */
	UINT8	Turbo1; //1 Core
	UINT8	Turbo2; //2 Core
	UINT8	Turbo3; //3 Core
	UINT8	Turbo4; //4 Core
  
	UINT32	CPUID[CPUID_MAX][4];
  
} CPU_STRUCTURE;

typedef struct {
	UINT8			Type;
	UINT8			BankConnections;
	UINT8			BankConnectionCount;
	UINT32			ModuleSize;
	UINT32			Frequency;
	CHAR8*			Vendor;
	CHAR8*			PartNo;
	CHAR8*			SerialNo;
	UINT8			*spd;
	BOOLEAN			InUse;
} RAM_SLOT_INFO; 

#define MAX_SLOT_COUNT	8
#define MAX_RAM_SLOTS	16

typedef struct {
  
	UINT64			Frequency;
	UINT32			Divider;
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

typedef struct {
  
	BOOLEAN	Ati;
	BOOLEAN	Intel;
	BOOLEAN	Nvidia;
  
} GFX_MANUFACTERER;

extern CHAR8                    *msgbuf;
extern CHAR8                    *msgCursor;
extern SMBIOS_STRUCTURE_POINTER	SmbiosTable;
extern GFX_MANUFACTERER         gGraphicsCard;
extern CHAR8*                   cpuFrequencyMHz;
extern BOOLEAN                  gMobile;
extern UINT32                   gCpuSpeed;  //kHz
extern UINT32                   gBusSpeed;  //kHz
extern CHAR8*                   BiosVendor;
extern UINT32                   mPropSize;
extern UINT8*                   mProperties;
extern CHAR8                    gSelectedUUID[];
extern CHAR8*                   AppleSystemVersion[];
extern UINT8                    gDefaultType;
extern EFI_SYSTEM_TABLE*        gST;
extern EFI_BOOT_SERVICES*       gBS; 
extern SETTINGS_DATA            gSettings;
extern CPU_STRUCTURE            gCPUStructure;
extern EFI_GUID                 gUuid;
extern CHAR8                    gOEMProduct[];  //original name from SMBIOS
extern EFI_EDID_DISCOVERED_PROTOCOL*            EdidDiscovered;


VOID InitBooterLog(VOID);
EFI_STATUS SetupBooterLog(VOID);

EFI_STATUS
EFIAPI
InitializeConsoleSim (
                      IN EFI_HANDLE           ImageHandle,
                      IN EFI_SYSTEM_TABLE     *SystemTable
                      );

UINT64 GetCPUProperties (VOID);

EFI_STATUS EFIAPI
LogDataHub(
           EFI_GUID					*TypeGuid,
           CHAR16                      *Name,
           VOID                        *Data,
           UINT32                       DataSize);

EFI_STATUS SetVariablesForOSX();
VOID       SetupDataForOSX();
EFI_STATUS SetPrivateVarProto(VOID);

EFI_STATUS PatchACPI(IN REFIT_VOLUME *Volume);

EFI_STATUS EventsInitialize (
                  IN EFI_HANDLE                             ImageHandle,
                  IN EFI_SYSTEM_TABLE                       *SystemTable
                  );

EFI_STATUS  bootElTorito(REFIT_VOLUME*	volume);
EFI_STATUS  bootMBR(REFIT_VOLUME* volume);
EFI_STATUS  bootPBR(REFIT_VOLUME* volume);

CHAR8*      XMLDecode(const CHAR8* src);
EFI_STATUS  ParseXML(const CHAR8* buffer, TagPtr * dict);
TagPtr      GetProperty( TagPtr dict, const CHAR8* key );
EFI_STATUS  XMLParseNextTag(CHAR8* buffer, TagPtr * tag, UINT32* lenPtr);

VOID        SaveSettings(VOID);

UINTN       iStrLen(CHAR8* String, UINTN MaxLen);
EFI_STATUS  PrepatchSmbios(VOID);
VOID        PatchSmbios(VOID);
VOID        FinalizeSmbios(VOID);

EFI_STATUS  DisableUsbLegacySupport(VOID);

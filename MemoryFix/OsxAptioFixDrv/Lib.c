/**

  Various helper functions.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
//#include <Library/DeviceTreeLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/SimpleFileSystem.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>

#include "NVRAMDebug.h"
#include "Lib.h"


// DBG_TO: 0=no debug, 1=serial, 2=console
// serial requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file
#define DBG_TO 0

#if DBG_TO == 2
	#define DBG(...) AsciiPrint(__VA_ARGS__);
#elif DBG_TO == 1
	#define DBG(...) DebugPrint(1, __VA_ARGS__);
#else
	#define DBG(...)
#endif


CHAR16 *EfiMemoryTypeDesc[EfiMaxMemoryType] = {
	L"reserved",
	L"LoaderCode",
	L"LoaderData",
	L"BS_code",
	L"BS_data",
	L"RT_code",
	L"RT_data",
	L"available",
	L"Unusable",
	L"ACPI_recl",
	L"ACPI_NVS",
	L"MemMapIO",
	L"MemPortIO",
	L"PAL_code"
};

CHAR16 *EfiAllocateTypeDesc[MaxAllocateType] = {
	L"AllocateAnyPages",
	L"AllocateMaxAddress",
	L"AllocateAddress"
};

CHAR16 *EfiLocateSearchType[] = {
	L"AllHandles",
	L"ByRegisterNotify",
	L"ByProtocol"
};

EFI_GUID gVendorGuid = {0xe62111ab, 0xcf4d, 0x4137, {0x87, 0x5c, 0x88, 0xde, 0xee, 0x34, 0xc3, 0xb3}};


EFI_GUID gEfiConsoleControlProtocolGuid			= {0xF42F7782, 0x012E, 0x4C12, {0x99, 0x56, 0x49, 0xF9, 0x43, 0x04, 0xF7, 0x21}};
EFI_GUID gAppleFirmwarePasswordProtocolGuid	= {0x8FFEEB3A, 0x4C98, 0x4630, {0x80, 0x3F, 0x74, 0x0F, 0x95, 0x67, 0x09, 0x1D}};
EFI_GUID gEfiGlobalVarGuid						= {0x8BE4DF61, 0x93CA, 0x11D2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}};
EFI_GUID gEfiDevicePathPropertyDatabaseProtocolGuid					= {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}};
EFI_GUID gEfiAppleBootGuid						= {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}};
EFI_GUID gEfiAppleNvramGuid						= {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}};
EFI_GUID gAppleFramebufferInfoProtocolGuid				= {0xE316E100, 0x0751, 0x4C49, {0x90, 0x56, 0x48, 0x6C, 0x7E, 0x47, 0x29, 0x03}};
EFI_GUID gAppleKeyStateProtocolGuid			= {0x5B213447, 0x6E73, 0x4901, {0xA4, 0xF1, 0xB8, 0x64, 0xF3, 0xB7, 0xA1, 0x72}};
EFI_GUID gAppleNetBootProtocolGuid				= {0x78EE99FB, 0x6A5E, 0x4186, {0x97, 0xDE, 0xCD, 0x0A, 0xBA, 0x34, 0x5A, 0x74}};
EFI_GUID gAppleImageCodecProtocolGuid			= {0x0DFCE9F6, 0xC4E3, 0x45EE, {0xA0, 0x6A, 0xA8, 0x61, 0x3B, 0x98, 0xA5, 0x07}};
EFI_GUID gEfiAppleVendorGuid					= {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
EFI_GUID gAppleEFINVRAMTRBSecureGuid			= {0xF68DA75E, 0x1B55, 0x4E70, {0xB4, 0x1B, 0xA7, 0xB7, 0xA5, 0xB7, 0x58, 0xEA}};
EFI_GUID gDataHubOptionsGuid					= {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
EFI_GUID gNotifyMouseActivity					= {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
//EFI_GUID gEfiDataHubProtocolGuid				= {0xae80d021, 0x618e, 0x11d4, {0xbc, 0xd7, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};
//EFI_GUID gEfiMiscSubClassGuid					= {0x772484B2, 0x7482, 0x4b91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}};
//EFI_GUID gEfiProcessorSubClassGuid				= {0x26fdeb7e, 0xb8af, 0x4ccf, {0xaa, 0x97, 0x02, 0x63, 0x3c, 0xe4, 0x8c, 0xa7}};
//EFI_GUID gEfiMemorySubClassGuid					= {0x4E8F4EBB, 0x64B9, 0x4e05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}};
EFI_GUID gMsgLogProtocolGuid					= {0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03}};
EFI_GUID gEfiLegacy8259ProtocolGuid				= {0x38321dba, 0x4fe0, 0x4e17, {0x8a, 0xec, 0x41, 0x30, 0x55, 0xea, 0xed, 0xc1}};


MAP_EFI_GUID_STR EfiGuidStrMap[] = {
	{NULL, L"Tmp buffer AE074D26-6E9E-11E1-A5B8-9BFC4824019B"},
/*	{&gEfiFileInfoGuid, L"gEfiFileInfoGuid"},
	{&gEfiFileSystemInfoGuid, L"gEfiFileSystemInfoGuid"},
	{&gEfiFileSystemVolumeLabelInfoIdGuid, L"gEfiFileSystemVolumeLabelInfoIdGuid"},
	{&gEfiLoadedImageProtocolGuid, L"gEfiLoadedImageProtocolGuid"},
	{&gEfiDevicePathProtocolGuid, L"gEfiDevicePathProtocolGuid"},
	{&gEfiSimpleFileSystemProtocolGuid, L"gEfiSimpleFileSystemProtocolGuid"},
	{&gEfiBlockIoProtocolGuid, L"gEfiBlockIoProtocolGuid"},
	{&gEfiBlockIo2ProtocolGuid, L"gEfiBlockIo2ProtocolGuid"},
	{&gEfiDiskIoProtocolGuid, L"gEfiDiskIoProtocolGuid"},
	{&gEfiDiskIo2ProtocolGuid, L"gEfiDiskIo2ProtocolGuid"},
	{&gEfiGraphicsOutputProtocolGuid, L"gEfiGraphicsOutputProtocolGuid"},
	
	{&gEfiConsoleControlProtocolGuid, L"gEfiConsoleControlProtocolGuid"},
	{&gAppleFirmwarePasswordProtocolGuid, L"gAppleFirmwarePasswordProtocolGuid"},
	{&gEfiGlobalVarGuid, L"gEfiGlobalVarGuid"},
	{&gEfiDevicePathPropertyDatabaseProtocolGuid, L"gEfiDevicePathPropertyDatabaseProtocolGuid"},
	{&gEfiAppleBootGuid, L"gEfiAppleBootGuid"},
	{&gEfiAppleNvramGuid, L"gEfiAppleNvramGuid"},
	{&gAppleFramebufferInfoProtocolGuid, L"gAppleFramebufferInfoProtocolGuid"},
	{&gAppleKeyStateProtocolGuid, L"gAppleKeyStateProtocolGuid"},
	{&gAppleNetBootProtocolGuid, L"gAppleNetBootProtocolGuid"},
	{&gAppleImageCodecProtocolGuid, L"gAppleImageCodecProtocolGuid"},
	{&gEfiAppleVendorGuid, L"gEfiAppleVendorGuid"},
	{&gAppleEFINVRAMTRBSecureGuid, L"gAppleEFINVRAMTRBSecureGuid"},
	{&gDataHubOptionsGuid, L"gDataHubOptionsGuid"},
	{&gNotifyMouseActivity, L"gNotifyMouseActivity"},
	{&gEfiDataHubProtocolGuid, L"gEfiDataHubProtocolGuid"},
	{&gEfiMiscSubClassGuid, L"gEfiMiscSubClassGuid"},
	{&gEfiProcessorSubClassGuid, L"gEfiProcessorSubClassGuid"},
	{&gEfiMemorySubClassGuid, L"gEfiMemorySubClassGuid"},
	{&gMsgLogProtocolGuid, L"gMsgLogProtocolGuid"},
	{&gEfiLegacy8259ProtocolGuid, L"gEfiLegacy8259ProtocolGuid"}, */
	{NULL, NULL}
};


/** Returns GUID as string, with friendly name for known guids. */
CHAR16*
EFIAPI
GuidStr(IN EFI_GUID *Guid)
{
	UINTN		i;
	CHAR16		*Str = NULL;
	
	for(i = 1; EfiGuidStrMap[i].Guid != NULL; i++) {
		if (CompareGuid(EfiGuidStrMap[i].Guid, Guid)) {
			Str = EfiGuidStrMap[i].Str;
			break;
		}
	}
	if (Str == NULL) {
		UnicodeSPrint(EfiGuidStrMap[0].Str, 47 * 2, L"%g", Guid); 
		Str = EfiGuidStrMap[0].Str;
	}
	return Str;
}

/** Returns pointer to last Char in String or NULL. */
CHAR16*
EFIAPI
GetStrLastChar(IN CHAR16 *String)
{
	CHAR16		*Pos;
	
	if (String == NULL || *String == L'\0') {
		return NULL;
	}
	
	// go to end
	Pos = String;
	while (*Pos != L'\0') {
		Pos++;
	}
	Pos--;
	return Pos;
}

/** Returns pointer to last occurence of Char in String or NULL. */
CHAR16*
EFIAPI
GetStrLastCharOccurence(IN CHAR16 *String, IN CHAR16 Char)
{
	CHAR16		*Pos;
	
	if (String == NULL || *String == L'\0') {
		return NULL;
	}
	
	// go to end
	Pos = String;
	while (*Pos != L'\0') {
		Pos++;
	}
	// search for Char
	while (*Pos != Char && Pos != String) {
		Pos--;
	}
	return (*Pos == Char) ? Pos : NULL;
}

/** Returns upper case version of char - valid only for ASCII chars in unicode. */
CHAR16
EFIAPI
ToUpperChar(IN CHAR16 Chr)
{
	CHAR8	C;
	
	if (Chr > 0xFF) return Chr;
	C = (CHAR8)Chr;
	return ((C >= 'a' && C <= 'z') ? C - ('a' - 'A') : C);
}


/** Returns 0 if two strings are equal, !=0 otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive.. */
UINTN
EFIAPI
StrCmpiBasic(IN CHAR16 *String1, IN CHAR16 *String2)
{
	CHAR16	Chr1;
	CHAR16	Chr2;
	
	if (String1 == NULL || String2 == NULL) {
		return 1;
	}
	if (*String1 == L'\0' && *String2 == L'\0') {
		return 0;
	}
	if (*String1 == L'\0' || *String2 == L'\0') {
		return 1;
	}
	
	Chr1 = ToUpperChar(*String1);
	Chr2 = ToUpperChar(*String2);
	while ((*String1 != L'\0') && (Chr1 == Chr2)) {
		String1++;
		String2++;
		Chr1 = ToUpperChar(*String1);
		Chr2 = ToUpperChar(*String2);
	}
	
	return Chr1 - Chr2;
}

/** Returns the first occurrence of a Null-terminated Unicode SearchString
  * in a Null-terminated Unicode String.
  * Compares just first 8 bits of chars (valid for ASCII), case insensitive.
  * Copied from MdePkg/Library/BaseLib/String.c and modified
  */
CHAR16*
EFIAPI
StrStriBasic(
	IN CONST CHAR16		*String,
	IN CONST CHAR16		*SearchString
  )
{
  CONST CHAR16 *FirstMatch;
  CONST CHAR16 *SearchStringTmp;


  if (*SearchString == L'\0') {
	return (CHAR16 *) String;
  }

  while (*String != L'\0') {
	SearchStringTmp = SearchString;
	FirstMatch = String;
	
	while ((ToUpperChar(*String) == ToUpperChar(*SearchStringTmp)) 
			&& (*String != L'\0')) {
		String++;
		SearchStringTmp++;
	} 
	
	if (*SearchStringTmp == L'\0') {
		return (CHAR16 *) FirstMatch;
	}

	if (*String == L'\0') {
		return NULL;
	}

	String = FirstMatch + 1;
  }

  return NULL;
}

/** Returns TRUE if String1 starts with String2, FALSE otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive.. */
BOOLEAN
EFIAPI
StriStartsWithBasic(IN CHAR16 *String1, IN CHAR16 *String2)
{
	CHAR16	Chr1;
	CHAR16	Chr2;
	BOOLEAN Result;
	
	if (String1 == NULL || String2 == NULL) {
		return FALSE;
	}
	if (*String1 == L'\0' && *String2 == L'\0') {
		return TRUE;
	}
	if (*String1 == L'\0' || *String2 == L'\0') {
		return FALSE;
	}
	
	Chr1 = ToUpperChar(*String1);
	Chr2 = ToUpperChar(*String2);
	while ((Chr1 != L'\0') && (Chr2 != L'\0') && (Chr1 == Chr2)) {
		String1++;
		String2++;
		Chr1 = ToUpperChar(*String1);
		Chr2 = ToUpperChar(*String2);
	}
	
	Result = ((Chr1 == L'\0') && (Chr2 == L'\0'))
	|| ((Chr1 != L'\0') && (Chr2 == L'\0'));
	
	return Result;
}

VOID EFIAPI
FixMemMap(
	IN UINTN					MemoryMapSize,
	IN EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	IN UINTN					DescriptorSize,
	IN UINT32					DescriptorVersion
)
{
	UINTN					NumEntries;
	UINTN					Index;
	EFI_MEMORY_DESCRIPTOR	*Desc;
	UINTN					BlockSize;
	UINTN					PhysicalEnd;

	DBG("FixMemMap: Size=%d, Addr=%p, DescSize=%d\n", MemoryMapSize, MemoryMap, DescriptorSize);
	DBGnvr("FixMemMap ...\n");
	
	Desc = MemoryMap;
	NumEntries = MemoryMapSize / DescriptorSize;
	
	for (Index = 0; Index < NumEntries; Index++) {
		BlockSize = EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages);
		PhysicalEnd = Desc->PhysicalStart + BlockSize;
		
		//
		// Some UEFIs end up with "reserved" area with EFI_MEMORY_RUNTIME flag set
		// when Intel HD3000 or HD4000 is used. We will remove that flag here.
		//
		if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0 && Desc->Type == EfiReservedMemoryType) {
			DBGnvr(" %s as RT: %lx (0x%x) - Att: %lx",
				   EfiMemoryTypeDesc[Desc->Type], Desc->PhysicalStart, Desc->NumberOfPages, Desc->Attribute);
			Desc->Attribute = Desc->Attribute & (~EFI_MEMORY_RUNTIME);
			DBGnvr(" -> %lx\n", Desc->Attribute);
			
			/* This one is not working - blocks during DefragmentRuntimeServices()
			DBGnvr(" %s as RT: %lx (0x%x) - %s",
				EfiMemoryTypeDesc[Desc->Type], Desc->PhysicalStart, Desc->NumberOfPages, EfiMemoryTypeDesc[Desc->Type]);
			Desc->Type = EfiMemoryMappedIO;
			DBGnvr(" -> %s\n", EfiMemoryTypeDesc[Desc->Type]);
			*/
			
			/* Another possible solution - mark the range as MMIO.
			DBGnvr(" %s as RT: %lx (0x%x) - %s",
				EfiMemoryTypeDesc[Desc->Type], Desc->PhysicalStart, Desc->NumberOfPages, EfiMemoryTypeDesc[Desc->Type]);
			Desc->Type = EfiRuntimeServicesData;
			DBGnvr(" -> %s\n", EfiMemoryTypeDesc[Desc->Type]);
			*/
		}
        
		//
        // Fix by Slice - fixes sleep/wake on GB boards.
        //
		//    if ((Desc->PhysicalStart >= 0x9e000) && (Desc->PhysicalStart < 0xa0000)) {
		if ((Desc->PhysicalStart < 0xa0000) && (PhysicalEnd >= 0x9e000)) {
			Desc->Type = EfiACPIMemoryNVS;
			Desc->Attribute = 0;
		}
#if 0
    if ((Desc->PhysicalStart < 0x100000) && (PhysicalEnd >= 0xE0000)) {
			Desc->Type = EfiACPIMemoryNVS;
			Desc->Attribute = 0;
		}

#endif
        
		//
		// Also do some checking
		//
		if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0) {
			//
			// block with RT flag.
			// if it is not RT or MMIO, then report to log
			//
			if (Desc->Type != EfiRuntimeServicesCode
				&& Desc->Type != EfiRuntimeServicesData
				&& Desc->Type != EfiMemoryMappedIO
				&& Desc->Type != EfiMemoryMappedIOPortSpace
				)
			{
				DBGnvr(" %s with RT flag: %lx (0x%x) - ???\n", EfiMemoryTypeDesc[Desc->Type], Desc->PhysicalStart, Desc->NumberOfPages);
			}
		} else {
			//
			// block without RT flag.
			// if it is RT or MMIO, then report to log
			//
			if (Desc->Type == EfiRuntimeServicesCode
				|| Desc->Type == EfiRuntimeServicesData
				|| Desc->Type == EfiMemoryMappedIO
				|| Desc->Type == EfiMemoryMappedIOPortSpace
				)
			{
				DBGnvr(" %s without RT flag: %lx (0x%x) - ???\n", EfiMemoryTypeDesc[Desc->Type], Desc->PhysicalStart, Desc->NumberOfPages);
			}
		}
		
		Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
	}
}

VOID EFIAPI
ShrinkMemMap(
	IN UINTN					*MemoryMapSize,
	IN EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	IN UINTN					DescriptorSize,
	IN UINT32					DescriptorVersion
)
{
	UINTN					SizeFromDescToEnd;
	UINT64					Bytes;
	EFI_MEMORY_DESCRIPTOR	*PrevDesc;
	EFI_MEMORY_DESCRIPTOR	*Desc;
	BOOLEAN					CanBeJoined;
	BOOLEAN					HasEntriesToRemove;
	
	PrevDesc = MemoryMap;
	Desc = NEXT_MEMORY_DESCRIPTOR(PrevDesc, DescriptorSize);
	SizeFromDescToEnd = *MemoryMapSize - DescriptorSize;
	*MemoryMapSize = DescriptorSize;
	HasEntriesToRemove = FALSE;
	while (SizeFromDescToEnd > 0) {
		Bytes = (((UINTN) PrevDesc->NumberOfPages) * EFI_PAGE_SIZE);
		CanBeJoined = FALSE;
		if ((Desc->Attribute == PrevDesc->Attribute) && (PrevDesc->PhysicalStart + Bytes == Desc->PhysicalStart)) {
			if (Desc->Type == EfiBootServicesCode
				|| Desc->Type == EfiBootServicesData
				//|| Desc->Type == EfiConventionalMemory
				//|| Desc->Type == EfiLoaderCode
				//|| Desc->Type == EfiLoaderData
				)
			{
				CanBeJoined = PrevDesc->Type == EfiBootServicesCode
					|| PrevDesc->Type == EfiBootServicesData
					//|| PrevDesc->Type == EfiConventionalMemory
					//|| PrevDesc->Type == EfiLoaderCode
					//|| PrevDesc->Type == EfiLoaderData
					;
			}
		}
		
		if (CanBeJoined) {
			// two entries are the same/similar - join them
			PrevDesc->NumberOfPages += Desc->NumberOfPages;
			HasEntriesToRemove = TRUE;
		} else {
			// can not be joined - we need to move to next
			*MemoryMapSize += DescriptorSize;
			PrevDesc = NEXT_MEMORY_DESCRIPTOR(PrevDesc, DescriptorSize);
			if (HasEntriesToRemove) {
				// have entries between PrevDesc and Desc which are joined to PrevDesc
				// we need to copy [Desc, end of list] to PrevDesc + 1
				CopyMem(PrevDesc, Desc, SizeFromDescToEnd);
				Desc = PrevDesc;
			}
			HasEntriesToRemove = FALSE;
		}
		// move to next
		Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
		SizeFromDescToEnd -= DescriptorSize;
	}
}

VOID EFIAPI
PrintMemMap(
	IN UINTN					MemoryMapSize,
	IN EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	IN UINTN					DescriptorSize,
	IN UINT32					DescriptorVersion
)
{
#if DBG_TO
	UINTN					NumEntries;
	UINTN					Index;
	UINT64					Bytes;
	EFI_MEMORY_DESCRIPTOR	*Desc;
	
	Desc = MemoryMap;
	NumEntries = MemoryMapSize / DescriptorSize;
	DBG("MEMMAP: Size=%d, Addr=%p, DescSize=%d, DescVersion: 0x%x\n", MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
	DBG("Type       Start            End       VStart               # Pages          Attributes\n");
	for (Index = 0; Index < NumEntries; Index++) {
	
		Bytes = (((UINTN) Desc->NumberOfPages) * EFI_PAGE_SIZE);
		
		DBG("%-12s %lX-%lX %lX  %lX %lX\n",
			EfiMemoryTypeDesc[Desc->Type],
			Desc->PhysicalStart,
			Desc->PhysicalStart + Bytes - 1,
			Desc->VirtualStart,
			Desc->NumberOfPages,
			Desc->Attribute
		);
		Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
		//if ((Index + 1) % 16 == 0) {
		//	WaitForKeyPress(L"press a key to continue\n");
		//}
	}
	//WaitForKeyPress(L"End: press a key to continue\n");
#endif	
}

VOID EFIAPI
PrintSystemTable(IN EFI_SYSTEM_TABLE  *ST)
{
	UINTN			Index;
	
	
	DBG("SysTable: %p\n", ST);
	DBG("- FirmwareVendor: %p, %s\n", ST->FirmwareVendor, ST->FirmwareVendor);
	DBG("- ConsoleInHandle: %p, ConIn: %p\n", ST->ConsoleInHandle, ST->ConIn);
	DBG("- RuntimeServices: %p, BootServices: %p, ConfigurationTable: %p\n", ST->RuntimeServices, ST->BootServices, ST->ConfigurationTable);
	DBG("RT:\n");
	DBG("- GetVariable: %p, SetVariable: %p\n", ST->RuntimeServices->GetVariable, ST->RuntimeServices->SetVariable);
	
	DBGnvr("SysTable: %p\n", ST);
	DBGnvr("- FirmwareVendor: %p, %s\n", ST->FirmwareVendor, ST->FirmwareVendor);
	DBGnvr("- ConsoleInHandle: %p, ConIn: %p\n", ST->ConsoleInHandle, ST->ConIn);
	DBGnvr("- RuntimeServices: %p, BootServices: %p, ConfigurationTable: %p\n", ST->RuntimeServices, ST->BootServices, ST->ConfigurationTable);
	DBGnvr("RT GetVariable: %p, SetVariable: %p\n", ST->RuntimeServices->GetVariable, ST->RuntimeServices->SetVariable);
	
	for(Index = 0; Index < ST->NumberOfTableEntries; Index++) {
		DBGnvr("ConfTab: %p\n", ST->ConfigurationTable[Index].VendorTable);
	}
}

VOID
WaitForKeyPress(CHAR16 *Message)
{
	EFI_STATUS		Status;
	UINTN			index;
	EFI_INPUT_KEY	key;
	
	Print(Message);
	do {
		Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
	} while(Status == EFI_SUCCESS);
	gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
	do {
		Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
	} while(Status == EFI_SUCCESS);
}

/** Returns file path from FilePathProto in allocated memory. Mem should be released by caler.*/
CHAR16 *
EFIAPI
FileDevicePathToText(EFI_DEVICE_PATH_PROTOCOL *FilePathProto)
{
	EFI_STATUS					Status;
	FILEPATH_DEVICE_PATH 				*FilePath;
	CHAR16								FilePathText[256]; // possible problem: if filepath is bigger
	CHAR16								*OutFilePathText;
	UINTN								Size;
	UINTN								SizeAll;
	UINTN								i;
	
	FilePathText[0] = L'\0';
	i = 4;
	SizeAll = 0;
	//DBG("FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto));
	while (FilePathProto != NULL && FilePathProto->Type != END_DEVICE_PATH_TYPE && i > 0) {
		if (FilePathProto->Type == MEDIA_DEVICE_PATH && FilePathProto->SubType == MEDIA_FILEPATH_DP) {
			FilePath = (FILEPATH_DEVICE_PATH *) FilePathProto;
			Size = (DevicePathNodeLength(FilePathProto) - 4) / 2;
			if (SizeAll + Size < 256) {
				if (SizeAll > 0 && FilePathText[SizeAll / 2 - 2] != L'\\') {
					StrCatS(FilePathText, 256, L"\\");
				}
				StrCatS(FilePathText, 256, FilePath->PathName);
				SizeAll = StrSize(FilePathText);
			}
		}
		FilePathProto = NextDevicePathNode(FilePathProto);
		//DBG("FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto));
		i--;
		//DBG("FilePathText: %s\n", FilePathText);
	}
	
	OutFilePathText = NULL;
	Size = StrSize(FilePathText);
	if (Size > 2) {
		// we are allocating mem here - should be released by caller
		Status = gBS->AllocatePool(EfiBootServicesData, Size, (VOID*)&OutFilePathText);
		if (Status == EFI_SUCCESS) {
			StrCpyS(OutFilePathText, Size/sizeof(CHAR16), FilePathText);
		} else {
			OutFilePathText = NULL;
		}
	}
	
	return OutFilePathText;
}

/** Helper function that calls GetMemoryMap(), allocates space for mem map and returns it. */
EFI_STATUS
EFIAPI
GetMemoryMapAlloc (
	IN EFI_GET_MEMORY_MAP				GetMemoryMapFunction,
	OUT UINTN						*MemoryMapSize,
	OUT EFI_MEMORY_DESCRIPTOR		**MemoryMap,
	OUT UINTN						*MapKey,
	OUT UINTN						*DescriptorSize,
	OUT UINT32						*DescriptorVersion
)
{
	EFI_STATUS					Status;
	
	*MemoryMapSize = 0;
	*MemoryMap = NULL;
	Status = GetMemoryMapFunction(MemoryMapSize, *MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		// OK. Space needed for mem map is in MemoryMapSize
		// Important: next AllocatePool can increase mem map size - we must add some space for this
		*MemoryMapSize += 256;
		*MemoryMap = AllocatePool(*MemoryMapSize);
		Status = GetMemoryMapFunction(MemoryMapSize, *MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
		if (EFI_ERROR(Status)) {
			FreePool(*MemoryMap);
		}
	}
	
	return Status;
}

/** Alloctes Pages from the top of mem, up to address specified in Memory. Returns allocated address in Memory. */
EFI_STATUS
EFIAPI
AllocatePagesFromTop(
	IN EFI_MEMORY_TYPE				MemoryType,
	IN UINTN						Pages,
	IN OUT EFI_PHYSICAL_ADDRESS		*Memory
)
{
	EFI_STATUS				Status;
	UINTN					MemoryMapSize;
	EFI_MEMORY_DESCRIPTOR	*MemoryMap;
	UINTN					 MapKey;
	UINTN					DescriptorSize;
	UINT32					DescriptorVersion;
	EFI_MEMORY_DESCRIPTOR	*MemoryMapEnd;
	EFI_MEMORY_DESCRIPTOR	*Desc;
	
	
	Status = GetMemoryMapAlloc(gBS->GetMemoryMap, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	/*
	MemoryMapSize = 0;
	MemoryMap = NULL;
	Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		MemoryMapSize += 256; // allocating pool can increase future mem map size
		MemoryMap = AllocatePool(MemoryMapSize);
		Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
		if (EFI_ERROR(Status)) {
			FreePool(*MemoryMap);
		}
	}
	 */
	
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	Status = EFI_NOT_FOUND;
	
	//DBG("Search for Pages=%x, TopAddr=%lx\n", Pages, *Memory);
	//DBG("MEMMAP: Size=%d, Addr=%p, DescSize=%d, DescVersion: 0x%x\n", MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
	//DBG("Type       Start            End       VStart               # Pages          Attributes\n");
	MemoryMapEnd = NEXT_MEMORY_DESCRIPTOR(MemoryMap, MemoryMapSize);
	Desc = PREV_MEMORY_DESCRIPTOR(MemoryMapEnd, DescriptorSize);
	for ( ; Desc >= MemoryMap; Desc = PREV_MEMORY_DESCRIPTOR(Desc, DescriptorSize)) {
		/*
		DBG("%-12s %lX-%lX %lX  %lX %lX\n",
			EfiMemoryTypeDesc[Desc->Type],
			Desc->PhysicalStart,
			Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Desc->NumberOfPages) - 1,
			Desc->VirtualStart,
			Desc->NumberOfPages,
			Desc->Attribute
		);
		*/
		if (   (Desc->Type == EfiConventionalMemory)						// free mem
			&& (Pages <= Desc->NumberOfPages)								// contains enough space
			&& (Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Pages) <= *Memory)	// contains space below specified Memory
		)
		{
			// free block found
			if (Desc->PhysicalStart + EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages) <= *Memory) {
				// the whole block is unded Memory - allocate from the top of the block
				*Memory = Desc->PhysicalStart + EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages - Pages);
				//DBG("found the whole block under top mem, allocating at %lx\n", *Memory);
			} else {
				// the block contains enough pages under Memory, but spans above it - allocate below Memory.
				*Memory = *Memory - EFI_PAGES_TO_SIZE(Pages);
				//DBG("found the whole block under top mem, allocating at %lx\n", *Memory);
			}
			Status = gBS->AllocatePages(AllocateAddress,
										MemoryType,
										Pages,
										Memory);
			//DBG("Alloc Pages=%x, Addr=%lx, Status=%r\n", Pages, *Memory, Status);
			break;
		}
	}
	
	// release mem
	FreePool(MemoryMap);
	
	return Status;
}

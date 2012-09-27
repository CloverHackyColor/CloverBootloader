/** @file

  Various helper functions.

  By dmazar, 26/09/2012             

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/SimpleFileSystem.h>

#include <Include/Guid/FileInfo.h>
#include <Include/Guid/FileSystemInfo.h>
#include <Include/Guid/FileSystemVolumeLabelInfo.h>

#include <Include/Guid/Acpi.h>
#include <Include/Guid/DebugImageInfoTable.h>
#include <Include/Guid/DxeServices.h>
#include <Include/Guid/HobList.h>
#include <Include/Guid/Mps.h>
#include <Include/Guid/SmBios.h>
#include <Include/Guid/SmBios.h>

#include "Common.h"


/** Memory type to string conversion */
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

/** Allocation type to string conversion */
CHAR16 *EfiAllocateTypeDesc[MaxAllocateType] = {
	L"AllocateAnyPages",
	L"AllocateMaxAddress",
	L"AllocateAddress"
};

/** Locate search type to string conversion */
CHAR16 *EfiLocateSearchType[] = {
	L"AllHandles",
	L"ByRegisterNotify",
	L"ByProtocol"
};

/** Reset type to string conversion */
CHAR16 *EfiResetType[] = {
	L"EfiResetCold",
	L"EfiResetWarm",
	L"EfiResetShutdown"
};

//
// Some known guids
//
EFI_GUID gEfiConsoleControlProtocolGuid         = {0xF42F7782, 0x012E, 0x4C12, {0x99, 0x56, 0x49, 0xF9, 0x43, 0x04, 0xF7, 0x21}};
EFI_GUID gEfiAppleFirmwarePasswordProtocolGuid  = {0x8FFEEB3A, 0x4C98, 0x4630, {0x80, 0x3F, 0x74, 0x0F, 0x95, 0x67, 0x09, 0x1D}};
EFI_GUID gEfiGlobalVarGuid                      = {0x8BE4DF61, 0x93CA, 0x11D2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}};
EFI_GUID gDevicePropertiesGuid                  = {0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}};
EFI_GUID gEfiAppleBootGuid                      = {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}};
EFI_GUID gEfiAppleNvramGuid                     = {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}};
EFI_GUID gEfiAppleScreenInfoGuid                = {0xE316E100, 0x0751, 0x4C49, {0x90, 0x56, 0x48, 0x6C, 0x7E, 0x47, 0x29, 0x03}};
EFI_GUID AppleBootKeyPressProtocolGuid          = {0x5B213447, 0x6E73, 0x4901, {0xA4, 0xF1, 0xB8, 0x64, 0xF3, 0xB7, 0xA1, 0x72}};
EFI_GUID AppleNetBootProtocolGuid               = {0x78EE99FB, 0x6A5E, 0x4186, {0x97, 0xDE, 0xCD, 0x0A, 0xBA, 0x34, 0x5A, 0x74}};
EFI_GUID AppleImageCodecProtocolGuid            = {0x0DFCE9F6, 0xC4E3, 0x45EE, {0xA0, 0x6A, 0xA8, 0x61, 0x3B, 0x98, 0xA5, 0x07}};
EFI_GUID gEfiAppleVendorGuid                    = {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
EFI_GUID gAppleEFINVRAMTRBSecureGuid            = {0xF68DA75E, 0x1B55, 0x4E70, {0xB4, 0x1B, 0xA7, 0xB7, 0xA5, 0xB7, 0x58, 0xEA}};
EFI_GUID gDataHubOptionsGuid                    = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
EFI_GUID gNotifyMouseActivity                   = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
EFI_GUID gEfiDataHubProtocolGuid                = {0xae80d021, 0x618e, 0x11d4, {0xbc, 0xd7, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};
EFI_GUID gEfiMiscSubClassGuid                   = {0x772484B2, 0x7482, 0x4b91, {0x9F, 0x9A, 0xAD, 0x43, 0xF8, 0x1C, 0x58, 0x81}};
EFI_GUID gEfiProcessorSubClassGuid              = {0x26fdeb7e, 0xb8af, 0x4ccf, {0xaa, 0x97, 0x02, 0x63, 0x3c, 0xe4, 0x8c, 0xa7}};
EFI_GUID gEfiMemorySubClassGuid                 = {0x4E8F4EBB, 0x64B9, 0x4e05, {0x9B, 0x18, 0x4C, 0xFE, 0x49, 0x23, 0x50, 0x97}};
EFI_GUID gMsgLogProtocolGuid                    = {0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03}};
EFI_GUID gEfiLegacy8259ProtocolGuid             = {0x38321dba, 0x4fe0, 0x4e17, {0x8a, 0xec, 0x41, 0x30, 0x55, 0xea, 0xed, 0xc1}};

// Shell guids
EFI_GUID ShellInt                               = {0x47c7b223, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b};
EFI_GUID SEnv                                   = {0x47c7b224, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b};
EFI_GUID ShellDevPathMap                        = {0x47c7b225, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b};
EFI_GUID ShellProtId                            = {0x47c7b226, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b};
EFI_GUID ShellAlias                             = {0x47c7b227, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b};


/** Map of known guids and friendly names. Searchable with GuidStr(). */
MAP_EFI_GUID_STR EfiGuidStrMap[] = {
	{NULL, L"Tmp buffer AE074D26-6E9E-11E1-A5B8-9BFC4824019B"},
	{&gEfiFileInfoGuid, L"gEfiFileInfoGuid"},
	{&gEfiFileSystemInfoGuid, L"gEfiFileSystemInfoGuid"},
	{&gEfiFileSystemVolumeLabelInfoIdGuid, L"gEfiFileSystemVolumeLabelInfoIdGuid"},
	{&gEfiLoadedImageProtocolGuid, L"gEfiLoadedImageProtocolGuid"},
	{&gEfiDevicePathProtocolGuid, L"gEfiDevicePathProtocolGuid"},
	{&gEfiSimpleFileSystemProtocolGuid, L"gEfiSimpleFileSystemProtocolGuid"},
	{&gEfiBlockIoProtocolGuid, L"gEfiBlockIoProtocolGuid"},
	{&gEfiDiskIoProtocolGuid, L"gEfiDiskIoProtocolGuid"},
	{&gEfiGraphicsOutputProtocolGuid, L"gEfiGraphicsOutputProtocolGuid"},
	
	{&gEfiConsoleControlProtocolGuid, L"gEfiConsoleControlProtocolGuid"},
	{&gEfiAppleFirmwarePasswordProtocolGuid, L"gEfiAppleFirmwarePasswordProtocolGuid"},
	{&gEfiGlobalVarGuid, L"gEfiGlobalVarGuid"},
	{&gDevicePropertiesGuid, L"gDevicePropertiesGuid"},
	{&gEfiAppleBootGuid, L"gEfiAppleBootGuid"},
	{&gEfiAppleNvramGuid, L"gEfiAppleNvramGuid"},
	{&gEfiAppleScreenInfoGuid, L"gEfiAppleScreenInfoGuid"},
	{&AppleBootKeyPressProtocolGuid, L"AppleBootKeyPressProtocolGuid"},
	{&AppleNetBootProtocolGuid, L"AppleNetBootProtocolGuid"},
	{&AppleImageCodecProtocolGuid, L"AppleImageCodecProtocolGuid"},
	{&gEfiAppleVendorGuid, L"gEfiAppleVendorGuid"},
	{&gAppleEFINVRAMTRBSecureGuid, L"gAppleEFINVRAMTRBSecureGuid"},
	{&gDataHubOptionsGuid, L"gDataHubOptionsGuid"},
	{&gNotifyMouseActivity, L"gNotifyMouseActivity"},
	{&gEfiDataHubProtocolGuid, L"gEfiDataHubProtocolGuid"},
	{&gEfiMiscSubClassGuid, L"gEfiMiscSubClassGuid"},
	{&gEfiProcessorSubClassGuid, L"gEfiProcessorSubClassGuid"},
	{&gEfiMemorySubClassGuid, L"gEfiMemorySubClassGuid"},
	{&gMsgLogProtocolGuid, L"gMsgLogProtocolGuid"},
	{&gEfiLegacy8259ProtocolGuid, L"gEfiLegacy8259ProtocolGuid"},
	
	{&gEfiAcpi10TableGuid, L"gEfiAcpi10TableGuid"},
	{&gEfiAcpi20TableGuid, L"gEfiAcpi20TableGuid"},
	{&gEfiDebugImageInfoTableGuid, L"gEfiDebugImageInfoTableGuid"},
	{&gEfiDxeServicesTableGuid, L"gEfiDxeServicesTableGuid"},
	{&gEfiHobListGuid, L"gEfiHobListGuid"},
	{&gEfiMpsTableGuid, L"gEfiMpsTableGuid"},
	{&gEfiSmbiosTableGuid, L"gEfiSmbiosTableGuid"},
	
	{&ShellInt, L"ShellInt"},
	{&SEnv, L"SEnv"},
	{&ShellDevPathMap, L"ShellDevPathMap"},
	{&ShellProtId, L"ShellProtId"},
	{&ShellAlias, L"ShellAlias"},
	
	{NULL, NULL}
};

/** Print buffer for unknown GUID printing. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
CHAR16	*GuidPrintBuffer = NULL;
#define GUID_PRINT_BUFFER_SIZE	((36+1) * sizeof(CHAR16))

/** Buffer for RT variable names. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
CHAR16 *gVariableNameBuffer = NULL;
#define VARIABLE_NAME_BUFFER_SIZE	1024

/** Buffer for RT variable data. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
CHAR8 *gVariableDataBuffer = NULL;
#define VARIABLE_DATA_BUFFER_SIZE	(64 * 1024)


/** Returns GUID as string, with friendly name for known guids. */
CHAR16*
EFIAPI
GuidStr(IN EFI_GUID *Guid)
{
	UINTN		i;
	CHAR16		*Str = NULL;
	
	if (GuidPrintBuffer == NULL) {
		GuidPrintBuffer = AllocateRuntimePool(GUID_PRINT_BUFFER_SIZE);
	}
	
	for(i = 1; EfiGuidStrMap[i].Guid != NULL; i++) {
		if (CompareGuid(EfiGuidStrMap[i].Guid, Guid)) {
			Str = EfiGuidStrMap[i].Str;
			break;
		}
	}
	if (Str == NULL) {
		UnicodeSPrint(GuidPrintBuffer, GUID_PRINT_BUFFER_SIZE, L"%g", Guid); 
		Str = GuidPrintBuffer;
	}
	return Str;
}


/** Prints Number of bytes in a row (hex and ascii). Row size is MaxNumber. */
VOID
EFIAPI
PrintBytesRow(IN CHAR8 *Bytes, IN UINTN Number, IN UINTN MaxNumber)
{
	UINTN	Index;
	
	// print hex vals
	for (Index = 0; Index < Number; Index++) {
		PRINT("%02x ", (UINT8)Bytes[Index]);
	}
	
	// pad to MaxNumber if needed
	for (; Index < MaxNumber; Index++) {
		PRINT("   ");
	}
	
	PRINT("| ");
	
	// print ASCII
	for (Index = 0; Index < Number; Index++) {
		if (Bytes[Index] >= 0x20 && Bytes[Index] <= 0x7e) {
			PRINT("%c", (CHAR16)Bytes[Index]);
		} else {
			PRINT("%c", L'.');
		}
	}
	
	PRINT("\n");
}

/** Prints series of bytes. */
VOID
EFIAPI
PrintBytes(IN CHAR8 *Bytes, IN UINTN Number)
{
	UINTN	Index;
	
	for (Index = 0; Index < Number; Index += 16) {
		PrintBytesRow(Bytes + Index, (Index + 16 < Number ? 16 : Number - Index), 16);
	}
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
	CONST CHAR16	*FirstMatch;
	CONST CHAR16	*SearchStringTmp;


	if (*SearchString == L'\0') {
		return (CHAR16 *) String;
	}

	while (*String != L'\0') {
		SearchStringTmp = SearchString;
		FirstMatch = String;
	
		while ((ToUpperChar(*String) == ToUpperChar(*SearchStringTmp)) 
			&& (*String != L'\0'))
		{
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
	UINTN					NumEntries;
	UINTN					Index;
	UINT64					Bytes;
	EFI_MEMORY_DESCRIPTOR	*Desc;
	
	Desc = MemoryMap;
	NumEntries = MemoryMapSize / DescriptorSize;
	PRINT("MEMMAP: Size=%d, Addr=%p, DescSize=%d, DescVersion: 0x%x\n", MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
	PRINT("Type       Start            End       VStart               # Pages          Attributes\n");
	for (Index = 0; Index < NumEntries; Index++) {
		Bytes = (((UINTN) Desc->NumberOfPages) * EFI_PAGE_SIZE);
		PRINT("%-12s %lX-%lX %lX  %lX %lX\n",
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
}

VOID EFIAPI
PrintSystemTable(IN EFI_SYSTEM_TABLE  *ST)
{
	UINTN			Index;
	
	PRINT("SysTable: %p\n", ST);
	PRINT("- FirmwareVendor: %p, %s\n", ST->FirmwareVendor, ST->FirmwareVendor);
	PRINT("- FirmwareRevision: %x\n", ST->FirmwareRevision);
	PRINT("- ConsoleInHandle: %p, ConIn: %p\n", ST->ConsoleInHandle, ST->ConIn);
	PRINT("- ConsoleOutHandle: %p, ConOut: %p\n", ST->ConsoleOutHandle, ST->ConOut);
	PRINT("- StandardErrorHandle: %p, StdErr: %p\n", ST->StandardErrorHandle, ST->StdErr);
	PRINT("- RuntimeServices: %p, BootServices: %p\n", ST->RuntimeServices, ST->BootServices);
	PRINT("- ConfigurationTable: %p\n", ST->ConfigurationTable);
	for(Index = 0; Index < ST->NumberOfTableEntries; Index++) {
		PRINT("  %p - %s\n", ST->ConfigurationTable[Index].VendorTable, GuidStr(&ST->ConfigurationTable[Index].VendorGuid));
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
	//PRINT("FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto));
	while (FilePathProto != NULL && FilePathProto->Type != END_DEVICE_PATH_TYPE && i > 0) {
		if (FilePathProto->Type == MEDIA_DEVICE_PATH && FilePathProto->SubType == MEDIA_FILEPATH_DP) {
			FilePath = (FILEPATH_DEVICE_PATH *) FilePathProto;
			Size = (DevicePathNodeLength(FilePathProto) - 4) / 2;
			if (SizeAll + Size < 256) {
				if (SizeAll > 0 && FilePathText[SizeAll / 2 - 2] != L'\\') {
					StrCat(FilePathText, L"\\");
				}
				StrCat(FilePathText, FilePath->PathName);
				SizeAll = StrSize(FilePathText);
			}
		}
		FilePathProto = NextDevicePathNode(FilePathProto);
		//PRINT("FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto));
		i--;
		//PRINT("FilePathText: %s\n", FilePathText);
	}
	
	OutFilePathText = NULL;
	Size = StrSize(FilePathText);
	if (Size > 2) {
		// we are allocating mem here - should be released by caller
		Status = gBS->AllocatePool(EfiBootServicesData, Size, (VOID*)&OutFilePathText);
		if (Status == EFI_SUCCESS) {
			StrCpy(OutFilePathText, FilePathText);
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
	
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	Status = EFI_NOT_FOUND;
	
	//PRINT("Search for Pages=%x, TopAddr=%lx\n", Pages, *Memory);
	//PRINT("MEMMAP: Size=%d, Addr=%p, DescSize=%d, DescVersion: 0x%x\n", MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
	//PRINT("Type       Start            End       VStart               # Pages          Attributes\n");
	MemoryMapEnd = NEXT_MEMORY_DESCRIPTOR(MemoryMap, MemoryMapSize);
	Desc = PREV_MEMORY_DESCRIPTOR(MemoryMapEnd, DescriptorSize);
	for ( ; Desc >= MemoryMap; Desc = PREV_MEMORY_DESCRIPTOR(Desc, DescriptorSize)) {
		/*
		PRINT("%-12s %lX-%lX %lX  %lX %lX\n",
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
			if (Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Desc->NumberOfPages) <= *Memory) {
				// the whole block is unded Memory - allocate from the top of the block
				*Memory = Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Desc->NumberOfPages - Pages);
				//PRINT("found the whole block under top mem, allocating at %lx\n", *Memory);
			} else {
				// the block contains enough pages under Memory, but spans above it - allocate below Memory.
				*Memory = *Memory - EFI_PAGES_TO_SIZE(Pages);
				//PRINT("found the whole block under top mem, allocating at %lx\n", *Memory);
			}
			Status = gBS->AllocatePages(AllocateAddress,
										MemoryType,
										Pages,
										Memory);
			//PRINT("Alloc Pages=%x, Addr=%lx, Status=%r\n", Pages, *Memory, Status);
			break;
		}
	}
	
	// release mem
	FreePool(MemoryMap);
	
	return Status;
}

/** Prints RT vars. */
VOID
EFIAPI
PrintRTVariables(
	IN EFI_RUNTIME_SERVICES	*RT
)
{
	EFI_STATUS		Status;
	UINT32			Attributes;
	UINT64			MaximumVariableStorageSize;
	UINT64			RemainingVariableStorageSize;
	UINT64			MaximumVariableSize;
	UINTN			VariableNameSize;
	UINTN			VariableNameBufferSize;
	UINTN			VariableDataSize;
	EFI_GUID		VendorGuid;
	BOOLEAN			IsDataPrintDisabled;
	
	//
	// Print storage info
	//
	PRINT("Vars storage:\n");
	PRINT("   Attrib: MaximumVariableStorageSize, RemainingVariableStorageSize, MaximumVariableSize\n");
	// NV+BS
	PRINT(" NV+BS   : ");
	Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
	Status = RT->QueryVariableInfo(Attributes, &MaximumVariableStorageSize, &RemainingVariableStorageSize, &MaximumVariableSize);
	if (EFI_ERROR(Status)) {
		PRINT("%r\n", Status);
	} else {
		PRINT("%ld, %ld, %ld\n", MaximumVariableStorageSize, RemainingVariableStorageSize, MaximumVariableSize);
	}
	// NV+BS+RT
	PRINT(" NV+BS+RT: ");
	Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
	Status = RT->QueryVariableInfo(Attributes, &MaximumVariableStorageSize, &RemainingVariableStorageSize, &MaximumVariableSize);
	if (EFI_ERROR(Status)) {
		PRINT("%r\n", Status);
	} else {
		PRINT("%ld, %ld, %ld\n", MaximumVariableStorageSize, RemainingVariableStorageSize, MaximumVariableSize);
	}
	
	//
	// Print all vars
	//
	PRINT("Variables:\n");
	VariableNameBufferSize = VARIABLE_NAME_BUFFER_SIZE;
	if (gVariableNameBuffer == NULL) {
		// init var name buffer
		// note: this must be called during boot services,
		// so if vars are going to be printed during runtime
		// they must be first printed during boot services
		// to init this buffer.
		if (InBootServices) {
			gVariableNameBuffer = AllocateRuntimePool(VariableNameBufferSize);
		} else {
			// error: buffer not inited during boot services
			PRINT("ERROR: gVariableNameBuffer not inited\n");
			return;
		}
	}
	// first call to GetNextVariableName must be with empty string
	gVariableNameBuffer[0] = L'\0';
	
	while (TRUE) {
		VariableNameSize = VariableNameBufferSize;
		Status = RT->GetNextVariableName(&VariableNameSize, gVariableNameBuffer, &VendorGuid);
		
		if (Status == EFI_BUFFER_TOO_SMALL) {
			// we will not handle this to avoid problems during calling in runtime
			PRINT("ERROR: gVariableNameBuffer too small\n");
			return;
		}
		if (Status == EFI_NOT_FOUND) {
			// no more vars
			break;
		}
		if (EFI_ERROR (Status)) {
			// no more vars or error
			PRINT("ERROR: GetNextVariableName: %r\n", Status);
			return;
		}
		
		// prepare for var data if needed
		if (gVariableDataBuffer == NULL) {
			if (InBootServices) {
				gVariableDataBuffer = AllocateRuntimePool(VARIABLE_DATA_BUFFER_SIZE);
			} else {
				// error: buffer not inited during boot services
				PRINT("ERROR: gVariableDataBuffer not inited\n");
				return;
			}
		}
		
		IsDataPrintDisabled = FALSE;
		
		#if PRINT_SHELL_VARS == 0
		{
			BOOLEAN			IsShellVar;
			
			IsShellVar = CompareGuid(&VendorGuid, &ShellInt) 
				|| CompareGuid(&VendorGuid, &SEnv) 
				|| CompareGuid(&VendorGuid, &ShellDevPathMap) 
				|| CompareGuid(&VendorGuid, &ShellProtId) 
				|| CompareGuid(&VendorGuid, &ShellAlias);
			
			IsDataPrintDisabled = IsShellVar;
		}
		#endif
		
		// get and print this var
		VariableDataSize = VARIABLE_DATA_BUFFER_SIZE;
		Status = RT->GetVariable(gVariableNameBuffer, &VendorGuid, &Attributes, &VariableDataSize, gVariableDataBuffer);
		if (EFI_ERROR(Status)) {
			PRINT(" %s:%s = %r\n", GuidStr(&VendorGuid), gVariableNameBuffer, Status);
		} else {
			PRINT("%08x ", Attributes);
			PRINT("%a", (Attributes & EFI_VARIABLE_NON_VOLATILE) ? "NV+" : "   ");
			PRINT("%a", (Attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS) ? "BS+" : "   ");
			PRINT("%a", (Attributes & EFI_VARIABLE_RUNTIME_ACCESS) ? "RT+" : "   ");
			PRINT("%a", (Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) ? "HW+" : "   ");
			PRINT(" %s:%s, DataSize = %x\n", GuidStr(&VendorGuid), gVariableNameBuffer, VariableDataSize);
			if (!IsDataPrintDisabled) {
				PrintBytes(gVariableDataBuffer, VariableDataSize);
			}
		}
	}
}
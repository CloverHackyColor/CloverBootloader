/** @file
  Sample ACPI Platform Driver

  Copyright (c) 2008 - 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 
/*
 Slice - corrections for MacOS
 Load and install patched DSDT.aml, SSDT-N.aml
 */
#include <Uefi.h>
#include <PiDxe.h>

#include <Protocol/AcpiTable.h>
//#include <Protocol/FirmwareVolume2.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

#include <Guid/HobList.h>
#include <Guid/FileInfo.h>
#include <Guid/Acpi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/PrintLib.h>

#include <IndustryStandard/Acpi.h>
//#include "HobGeneration.h"

#pragma pack(1)

typedef struct {
	EFI_ACPI_DESCRIPTION_HEADER  Header;
	UINT32                       Entry;
} RSDT_TABLE;

typedef struct {
	EFI_ACPI_DESCRIPTION_HEADER  Header;
	UINT64                       Entry;
} XSDT_TABLE;

typedef union {
		UINT32 Sign;
		CHAR8  ASign[4];
} SIGNAT;

#pragma pack()

VOID
InstallLegacyTables (
	EFI_ACPI_TABLE_PROTOCOL         *AcpiTable,
	EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp
					 )
{
	EFI_STATUS                      Status;
	UINTN							Index;
	UINTN                           TableHandle;
	UINTN                           TableSize;
	UINT32							EntryCount;
	UINT32							*EntryPtr;
	EFI_ACPI_DESCRIPTION_HEADER		*Table;
	RSDT_TABLE						*Rsdt;
	XSDT_TABLE						*Xsdt;
//	EFI_ACPI_COMMON_HEADER          *Dsdt;
//	EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *Facs;
//	UINTN							BasePtr;
	//	UINT32                       Signature;
	SIGNAT							Signature;
	
	EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE   *Fadt;
//	BOOLEAN							Found = FALSE;
	
	TableHandle = 0;
	
//	if ((int)gHob->Acpi20.Table != -1) {
//		Rsdp = (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)(UINTN)gHob->Acpi20.Table;
		Rsdt = (RSDT_TABLE *)Rsdp->RsdtAddress; //(UINTN)
		Xsdt = NULL;
		if ((Rsdp->Revision >= 2) && (Rsdp->XsdtAddress < (UINT64)(UINTN)-1)) {
			Xsdt = (XSDT_TABLE *)(UINTN)Rsdp->XsdtAddress;
		}
		if (Rsdt) {
			//Install Rsdt
			TableSize = Rsdt->Header.Length;
			Signature.Sign = Rsdt->Header.Signature;
			Print(L"Install table: %c%c%c%c\n",
				  Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
			Status = AcpiTable->InstallAcpiTable (
												  AcpiTable,
												  Rsdt,
												  TableSize,
												  &TableHandle
												  );
			if (EFI_ERROR(Status)) {
				return;
			}
			//First scan for RSDT
			EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);
			
			EntryPtr = &Rsdt->Entry;
			Fadt = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE   *)EntryPtr;
			for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
				Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
				TableSize = Table->Length;
				Signature.Sign = Table->Signature;
				Print(L"Install table: %c%c%c%c\n", 
					  Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
				Status = AcpiTable->InstallAcpiTable (
													  AcpiTable,
													  Table,
													  TableSize,
													  &TableHandle
													  );
				if (EFI_ERROR(Status)) {
					continue;
				}
			}
			//Now find Fadt and install dsdt and facs
			EntryPtr = &Fadt->Dsdt;
			Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
			TableSize = Table->Length;
			Signature.Sign = Table->Signature;
			Print(L"Install table: %c%c%c%c\n", 
				  Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
			
			Status = AcpiTable->InstallAcpiTable (
												  AcpiTable,
												  Table,
												  TableSize,
												  &TableHandle
												  );
			EntryPtr++; //nows facs
			Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
			TableSize = Table->Length;
			Signature.Sign = Table->Signature;
			Print(L"Install table: %c%c%c%c\n", 
				  Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
			Status = AcpiTable->InstallAcpiTable (
												  AcpiTable,
												  Table,
												  TableSize,
												  &TableHandle
												  );			
		}
		//Install Xsdt if any	
		if (Xsdt) {
			TableSize = Xsdt->Header.Length;
			Signature.Sign = Xsdt->Header.Signature;
			Print(L"Install table: %c%c%c%c\n",
				  Signature.ASign[0], Signature.ASign[1], Signature.ASign[2], Signature.ASign[3]);
			
			Status = AcpiTable->InstallAcpiTable (
												  AcpiTable,
												  Xsdt,
												  TableSize,
												  &TableHandle
												  );
			if (EFI_ERROR(Status)) {
				return;
			}
			//Second scan for Xsdt
/*			EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
			
			BasePtr = (UINTN)(&(Xsdt->Entry));
			for (Index = 0; Index < EntryCount; Index ++) {
				CopyMem (&EntryPtr, (VOID *)(BasePtr + Index * sizeof(UINT64)), sizeof(UINT64));
				Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(EntryPtr));
				Status = AcpiTable->InstallAcpiTable (
													  AcpiTable,
													  Table,
													  TableSize,
													  &TableHandle
													  );
				if (EFI_ERROR(Status)) {
					continue;
				}
			}
 */
		}
//	}
}

CHAR16* ACPInames[] = {
	L"DSDT.aml",
	L"SSDT.aml",
	L"SSDT-1.aml",
	L"SSDT-2.aml",
	L"SSDT-3.aml",
	L"SSDT-4.aml",
	L"SSDT-5.aml",
	L"SSDT-6.aml",
	L"SSDT-7.aml"
};

// Slice: New signature compare function
BOOLEAN tableSign(CHAR8 *table, CONST CHAR8 *sgn)
{
	int i;
	for (i=0; i<4; i++) {
		if ((table[i] &~0x20) != (sgn[i] &~0x20)) {
			return FALSE;
		}
	}
	return TRUE;
}

/**
  This function calculates and updates an UINT8 checksum.

  @param  Buffer          Pointer to buffer to checksum
  @param  Size            Number of bytes to checksum

**/
VOID
AcpiPlatformChecksum (
  IN UINT8      *Buffer,
  IN UINTN      Size
  )
{
  UINTN ChecksumOffset;

  ChecksumOffset = OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum);

  //
  // Set checksum to 0 first
  //
  Buffer[ChecksumOffset] = 0;

  //
  // Update checksum value
  //
  Buffer[ChecksumOffset] = CalculateCheckSum8(Buffer, Size);
}


/**
  Entrypoint of Acpi Platform driver.

  @param  ImageHandle
  @param  SystemTable

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
	EFI_STATUS                      Status;
	EFI_ACPI_TABLE_PROTOCOL         *AcpiTable;
	INTN                            Instance;
	EFI_ACPI_COMMON_HEADER          *CurrentTable;
	UINTN                           TableHandle;
//	UINT32                          FvStatus;
	UINTN                           TableSize;
//	UINTN                           Size;
	UINTN							Index;
	CHAR16*							FileName;
	EFI_LOADED_IMAGE_PROTOCOL		*LoadedImage;
	VOID							*FileBuffer;
	UINTN							FileSize;
	UINTN							BufferSize;
//	UINTN							Key;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
	EFI_FILE_INFO                   *Info;
	EFI_FILE_HANDLE                 Root = NULL;
	EFI_FILE_HANDLE                 ThisFile = NULL;
	EFI_PHYSICAL_ADDRESS			*Acpi20;
	EFI_PEI_HOB_POINTERS        GuidHob;
	EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp;
/*	EFI_ACPI_DESCRIPTION_HEADER *Rsdt, *Xsdt;
	EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *Fadt;
*/
	
	Instance     = 0;
	CurrentTable = NULL;
	TableHandle  = 0;
	
	//
	// Find the AcpiTable protocol
	//
	Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID**)&AcpiTable);
	if (EFI_ERROR (Status)) {
		return EFI_ABORTED;
	}
	GuidHob.Raw = GetFirstGuidHob (&gEfiAcpi20TableGuid);
	if (GuidHob.Raw == NULL) {
		GuidHob.Raw = GetFirstGuidHob (&gEfiAcpiTableGuid);
		if (GuidHob.Raw == NULL) {
			return EFI_ABORTED;
		}
		//Slice: TODO if we found only Acpi1.0 we need to convert it to Acpi2.0
		// like I did in Chameleon
	}
	Acpi20 = GET_GUID_HOB_DATA (GuidHob.Guid);
	if (Acpi20 == NULL) {
		return EFI_ABORTED;
	}
	Rsdp = (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)(UINTN)*Acpi20;		
	InstallLegacyTables(AcpiTable, Rsdp);
	Print(L"LegacyTables installed\n");
//  Looking for a volume from what we boot
	
/*	TODO - look for a volume we want to boot System
	it is possible if we fix in BdsBoot.c 
	gRT->SetVariable (
		L"BootCurrent",
		&gEfiGlobalVariableGuid,
		EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
			sizeof (UINT16),
		&Option->BootCurrent
	);
 gRT->GetVariable (
		L"BootNext",
		&gEfiGlobalVariableGuid,
		EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
		0,
		&BootNext
		);
 and extract DevicePath from BootNext - first available :(
 In Gui.efi we can repeat this patch with DSDT.aml loaded from another place
 */
	Status = gBS->HandleProtocol (
								  ImageHandle,
								  &gEfiLoadedImageProtocolGuid,
								  (VOID*)&LoadedImage
								  );
	if (EFI_ERROR (Status)) {
		return EFI_ABORTED;
	}
	Status = gBS->HandleProtocol (
								  LoadedImage->DeviceHandle,
								  &gEfiSimpleFileSystemProtocolGuid,
								  (VOID *) &Volume
								  );
	if (EFI_ERROR (Status)) {
		return EFI_ABORTED;
	}
		Print(L"Volume found\n");
	//
	// Open the root directory of the volume
	//
	if (!EFI_ERROR (Status)) {
		Status = Volume->OpenVolume (
									 Volume,
									 &Root
									 );
	}
	FileName = AllocateZeroPool(32); //Should be enough
	//
	// Read tables from the storage file.
	//
	for (Index=0; Index<9; Index++) {
		StrCpy(FileName, ACPInames[Index]);
		Print(L"File probe %s\n", FileName);
		Status = Root->Open (Root, &ThisFile, FileName, EFI_FILE_MODE_READ, 0);
		if (EFI_ERROR (Status)) {
			continue;
		}
		/* Get right size we need to allocate */
		Status = ThisFile->GetInfo (
									ThisFile,
									&gEfiFileInfoGuid,
									&BufferSize,
									Info
									);
		if (EFI_ERROR(Status) && Status != EFI_BUFFER_TOO_SMALL) {
			continue;
		}
		Print(L"Buffer size %d\n", BufferSize);
		//		Print(L"GetInfo success!\n");
		Status = gBS->AllocatePool (EfiBootServicesData, BufferSize, (VOID **) &Info);
		if (EFI_ERROR (Status)) {
			//			Print(L"No pool!\n");
			continue;
		}
		Status = ThisFile->GetInfo (
									ThisFile,
									&gEfiFileInfoGuid,
									&BufferSize,
									Info
									);
		FileSize = Info->FileSize;
				Print(L"FileSize = %d!\n", FileSize);
		gBS->FreePool (Info);
		
		FileBuffer = AllocateZeroPool(FileSize);
		//Slice - may be this is more correct memory for ACPI tables?
/*		Status = gBS->AllocatePages (
									 AllocateMaxAddress,
									 EfiACPIMemoryNVS,
									 EFI_SIZE_TO_PAGES(FileSize),
									 &FileBuffer
									 );
 */
		Status = ThisFile->Read (ThisFile, &FileSize, FileBuffer); //(VOID**)&
				Print(L"FileRead success! \n");
				
		if (!EFI_ERROR(Status)) {
			//
			// Add the table
			//
			TableHandle = 0;
			
			TableSize = ((EFI_ACPI_DESCRIPTION_HEADER *) FileBuffer)->Length;
			//ASSERT (BufferSize >= TableSize);
			if (FileSize < TableSize) {
				//Data incorrect. What TODO? Quick fix
				((EFI_ACPI_DESCRIPTION_HEADER *) FileBuffer)->Length = FileSize;
				TableSize = FileSize;
			}			
			//
			// Checksum ACPI table
			//
			AcpiPlatformChecksum ((UINT8*)FileBuffer, TableSize);			
			//
			// Install ACPI table
			//
			Status = AcpiTable->InstallAcpiTable (
												  AcpiTable,
												  FileBuffer,
												  TableSize,
												  &TableHandle
												  );
			if (EFI_ERROR(Status)) {
				return EFI_ABORTED;
			}
			Print(L"Table installed \n");
			//
			// Increment the instance
			//
			Instance++;   //for a what?
			FileBuffer = NULL;
		}
	}
	
	return EFI_SUCCESS;
}


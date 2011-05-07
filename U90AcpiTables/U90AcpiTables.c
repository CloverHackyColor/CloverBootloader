#define DBG(x...)  Print(x)
//typedef UINTN EFI_BOOT_MODE;
//Constant DSDT loader
//(void)usr-sse2
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/AcpiTable.h>
#include <Uefi.h>
#include <PiDxe.h>

#include <Protocol/AcpiTable.h>
//#include <Protocol/FirmwareVolume2.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

#include <Guid/HobList.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/Acpi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
//#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
//#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/PrintLib.h>

#include <IndustryStandard/Acpi.h>
//#include "HobGeneration.h"
//#include "AcpiTable.h"
//#include "DSDT.h"
#include <Library/GenericBdsLib.h>

#define NUM_TABLES 15
CHAR16* ACPInames[NUM_TABLES] = {
	L"FACP",
	L"DSDT",
	L"FACS",
	L"APIC",
	L"HPET",
	L"MCFG",
	L"SLIC",
	L"SSDT",
	L"SSDT-1",
	L"SSDT-2",
	L"SSDT-3",
	L"SSDT-4",
	L"SSDT-5",
	L"SSDT-6",
	L"SSDT-7"
};

EFI_STATUS
InstallAcpiTable (VOID* Table, UINTN Size, CHAR16* Name, EFI_ACPI_TABLE_PROTOCOL* AcpiTableProtocol) {
	UINTN Key;
	Key=0;
	AcpiTableProtocol->InstallAcpiTable(
		AcpiTableProtocol,
		Table,
		Size,
		&Key
	);
	
	Print(Name);
	if (Key != 0) {
		AsciiPrint(" installed, key = %d\n", Key);
		return EFI_SUCCESS;
	}
	AsciiPrint(" failed to install\n");
	return EFI_UNSUPPORTED;		
}


EFI_STATUS EFIAPI
InitAcpiTables(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS			rc;
	EFI_STATUS			Status;
	EFI_ACPI_TABLE_PROTOCOL		*AcpiTableProtocol;
	UINTN				Index;
	CHAR16				FileName[256];	
	EFI_LOADED_IMAGE_PROTOCOL	*LoadedImage;
	VOID				*FileBuffer;
	UINTN				FileSize;
	UINTN				TableSize;
	UINTN				BufferSize;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
	EFI_FILE_INFO                   *Info;
	EFI_FILE_HANDLE                 Root = NULL;
	EFI_FILE_HANDLE                 ThisFile = NULL;
	EFI_HANDLE       SelfImageHandle;
	EFI_LOADED_IMAGE *SelfLoadedImage;
	//EFI_FILE         *SelfRootDir;
	//EFI_FILE         *SelfDir;
	//CHAR16           SelfDirPath[256];
	
    //CHAR16      *DevicePathAsString;
    //CHAR16      BaseDirectory[256];
    //UINTN       i;
    
    SelfImageHandle = ImageHandle;
    Status = gBS->HandleProtocol(SelfImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &SelfLoadedImage);
    if (Status != EFI_SUCCESS)
        return EFI_LOAD_ERROR;
    
    /*
    if (SelfLoadedImage->LoadOptionsSize > 0) {
        CHAR16 Buffer[1024];
        UINTN Length = SelfLoadedImage->LoadOptionsSize / 2;
        if (Length > 1023)
            Length = 1023;
        CopyMem(Buffer, SelfLoadedImage->LoadOptions, SelfLoadedImage->LoadOptionsSize);
        Buffer[Length] = 0;
        Print(L"Load options: '%s'\n", Buffer);
        CheckError(EFI_LOAD_ERROR, L"FOR DEBUGGING");
    }
    */
    
    // find the current directory
    /*DevicePathAsString = DevicePathToStr(SelfLoadedImage->FilePath);
    if (DevicePathAsString != NULL) {
        Print(L"%s\n", DevicePathAsString);
        StrCpy(BaseDirectory, DevicePathAsString);
        //FreePool(DevicePathAsString);
	Print(L"%s\n", BaseDirectory);
        for (i = StrLen(BaseDirectory); i > 0 && BaseDirectory[i] != '\\' && BaseDirectory[i] != '/'; i--) ;
        BaseDirectory[i] = 0;
    } else
        BaseDirectory[0] = 0; 
    Print(L"%s\n", BaseDirectory);
    //SelfDirPath = BaseDirectory;
    UnicodeSPrint (SelfDirPath, 255, L"%s", BaseDirectory);
    Print(L"%s\n", SelfDirPath);*/
	

	AcpiTableProtocol = NULL;
	rc = gBS->LocateProtocol(
		&gEfiAcpiTableProtocolGuid,
		NULL,
		(VOID **)&AcpiTableProtocol
	);

	if (AcpiTableProtocol == NULL) {
		AsciiPrint("ACPI table protocol not found!\n");
		return EFI_NOT_FOUND;
	}	
	ASSERT_EFI_ERROR(rc);
	
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

	//		DBG(L"Volume found\n");
	//
	// Open the root directory of the volume
	//
	if (!EFI_ERROR (Status)) {
		Status = Volume->OpenVolume (Volume, &Root);
	}
	
	//
	// Read tables from the first volume.
	//
	for (Index=0; Index<NUM_TABLES; Index++) {
		UnicodeSPrint(FileName, 256, L"\\acpitbls\\%s.aml", /*SelfDirPath,*/ ACPInames[Index]);
		DBG(L"Probing file %s\n", FileName);
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
		DBG(L"Buffer size %d\n", BufferSize);
		//		DBG(L"GetInfo success!\n");
		Info = AllocatePool (BufferSize);
		if (!Info) {
			DBG(L"No pool!\n");
			continue;
		}
		Status = ThisFile->GetInfo (
			ThisFile,
			&gEfiFileInfoGuid,
			&BufferSize,
			Info
		);
		
		FileSize = Info->FileSize;
		DBG(L"FileSize = %d!\n", FileSize);
		//FreePool (Info);
//Slice - this is the problem. 		
		FileBuffer = AllocatePool(FileSize);
		//Status = gBS->AllocatePool (EfiBootServicesData, FileSize, (VOID **) &FileBuffer);
		if (!FileBuffer) {
			DBG(L"No pool for FileBuffer size %d!\n", FileSize);
			continue;
		}
		
//should use ACPI memory
//		Status=gBS->AllocatePages(AllocateMaxAddress,
//					EfiACPIReclaimMemory,RoundPage(FileSize)/EFI_PAGE_SIZE, FileBuffer);
		DBG(L"FileBuffer @ %x\n", (UINTN)FileBuffer);
		
		Status = ThisFile->Read (ThisFile, &FileSize, FileBuffer); //(VOID**)&
//		DBG(L"FileRead status=%x\n", 	Status);	
		if (!EFI_ERROR(Status)) {
			//
			// Add the table
			//
//			TableHandle = 0;
			if (ThisFile != NULL) {
				ThisFile->Close (ThisFile); //close file before use buffer?! Flush?!
			}
			
//			DBG(L"FileRead success: %c%c%c%c\n",
//				  ((CHAR8*)FileBuffer)[0], ((CHAR8*)FileBuffer)[1], ((CHAR8*)FileBuffer)[2], ((CHAR8*)FileBuffer)[3]);
			TableSize = ((EFI_ACPI_DESCRIPTION_HEADER *) FileBuffer)->Length;
			//ASSERT (BufferSize >= TableSize);
			DBG(L"Table size=%d\n", TableSize);
			if (FileSize < TableSize) {
				//Data incorrect. What TODO? Quick fix
				DBG(L"Table size > file size :(\n");
				continue; //do nothing with broken table
			}			
					
			//
			// Install ACPI table
			//
			InstallAcpiTable (FileBuffer, TableSize, FileName, AcpiTableProtocol);
			
			//FreePool (FileBuffer);
			FileBuffer = NULL;
		}
	}
	/*if (Root != NULL) {
		Root->Close (Root);
	}*/
	
	Print(L"All done!\n");
	return EFI_SUCCESS;
}
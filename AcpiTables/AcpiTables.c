//Constant DSDT loader
//(void)usr-sse2
#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
//#include <Library/UefiApplicationEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/GenericBdsLib.h>

#include <Protocol/AcpiTable.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/SimpleFileSystem.h>

#include <Guid/FileInfo.h>


//#include <Library/EfiFileLib.h>
#include <Library/PrintLib.h>
EFI_STATUS EFIAPI
InitAcpiTables(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{	
	EFI_STATUS							Status;
//	CHAR16								FileName[256];
	
	EFI_ACPI_TABLE_PROTOCOL		*AcpiTableProtocol;
	EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
	VOID						*FileBuffer;
	UINTN						FileSize;
	UINTN						BufferSize;
	UINTN						Key;
	EFI_DEVICE_PATH_PROTOCOL    *FilePath;
	EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *DevPathToText;
	CHAR16                           *ToText;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
	EFI_FILE_INFO                    *Info;
	EFI_FILE_HANDLE                 Root = NULL;
	EFI_FILE_HANDLE                  ThisFile = NULL;

	
	Print(L"Welcome to ACPI Patcher!\n");
	gBS = SystemTable->BootServices;
	//
    // Retrieve the Loaded Image Protocol
    //
    Status = gBS->HandleProtocol (
								  ImageHandle,
								  &gEfiLoadedImageProtocolGuid,
								  (VOID*)&LoadedImage
								  );
    ASSERT_EFI_ERROR (Status);
	// Retrieve the Device Path Protocol from the DeviceHandle from which this driver was loaded
    //
    Status = gBS->HandleProtocol (
								  LoadedImage->DeviceHandle,
								  &gEfiDevicePathProtocolGuid,
								  (VOID*)&FilePath
								  );
    ASSERT_EFI_ERROR (Status);
	Status = gBS->LocateProtocol (
								  &gEfiDevicePathToTextProtocolGuid,
								  NULL,
								  (VOID **) &DevPathToText
								  );
	if (!EFI_ERROR (Status)) {
		ToText = DevPathToText->ConvertDevicePathToText (
														 FilePath,
														 TRUE,
														 TRUE
														 );
		
		Print(L"%s\n", ToText);
	}
	
	//
	// File the file system interface to the device
	//
	Status = gBS->HandleProtocol (
								  LoadedImage->DeviceHandle,
								  &gEfiSimpleFileSystemProtocolGuid,
								  (VOID *) &Volume
								  );
	
	//
	// Open the root directory of the volume
	//
	if (!EFI_ERROR (Status)) {
		Status = Volume->OpenVolume (
									 Volume,
									 &Root
									 );
	}
//	FileName = L"DSDT.aml";
	Status = Root->Open (Root, &ThisFile, L"DSDT.aml", EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR (Status)) {
		Print(L"Can't open root\n");
		return EFI_SUCCESS;
	}
	ASSERT (ThisFile != NULL);
	/* Get right size we need to allocate */
    Status = ThisFile->GetInfo (
								ThisFile,
								&gEfiFileInfoGuid,
								&BufferSize,
								Info
								);
    if (EFI_ERROR(Status) && Status != EFI_BUFFER_TOO_SMALL)
    {
		Print(L"EFI_BUFFER_TOO_SMALL\n");
        return EFI_SUCCESS;
    }
	Print(L"GetInfo success!\n");
    Status = gBS->AllocatePool (EfiBootServicesData, BufferSize, (VOID **) &Info);
    if (EFI_ERROR (Status)) {
		Print(L"No pool!\n");
		return EFI_SUCCESS;
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
	Status = ThisFile->Read (ThisFile, &FileSize, FileBuffer);
	Print(L"FileRead success! \n");
	
	
	Status = gBS->LocateProtocol(
							 &gEfiAcpiTableProtocolGuid,
							 NULL,
							 (VOID **)&AcpiTableProtocol
							 );
	Print(L"Locate ACPI Protocol with status %x\n", Status);
	ASSERT_EFI_ERROR(Status);
	Print(L"Begin InstallAcpiTable\n");
	
	AcpiTableProtocol->InstallAcpiTable(
										AcpiTableProtocol,
										FileBuffer,
										FileSize,
										&Key
										);
	
	Print(L"Finish ACPI patch!\n");
	if (ThisFile != NULL) {
		ThisFile->Close (ThisFile);
	}
	if (Root != NULL) {
		Root->Close (Root);
	}	
	Print(L"File closed\n");
	
	return EFI_SUCCESS;
}


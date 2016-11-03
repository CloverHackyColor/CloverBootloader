/** @file

  Driver for dumping boot loader UEFI calls.

  By dmazar, 26/09/2012

**/

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PeCoffLib.h>

#include <Protocol/LoadedImage.h>
//#include <Protocol/Runtime.h>

#include "Common.h"

/** Original StartImage UEFI method. */
EFI_IMAGE_START OrgStartImage = NULL;

/** Installs our overrides of UEFI services.*/
EFI_STATUS
StartOverrides()
{
	EFI_TIME          Now;
	gRT->GetTime(&Now, NULL);
	PRINT("DumpUefiCalls overrides started on %04d.%02d.%02d (yyyy.mm.dd), at %02d:%02d:%02d.\n",
	       Now.Year, Now.Month, Now.Day, Now.Hour, Now.Minute, Now.Second);
	#if CAPTURE_FILESYSTEM_ACCESS == 1
	OvrFs();
	#endif
	OvrBootServices(gBS);
	OvrRuntimeServices(gRT);
	#if PRINT_DUMPS >= 2
//	OvrDataHub();
	#endif
  OvrAppleSMC();
//  OvrAppleImageCodec();
  OvrAppleKeyState();
//  OvrOSInfo();
//  OvrGraphConfig();
//  OvrFirmwareVolume();
//  OvrEfiKeyboardInfo();
  OvrAppleKeyMapDb();
  return EFI_SUCCESS;
}

CHAR16					*BootLoaders[] = BOOT_LOADERS;
/** Our implementation of StartImage. Used to detect boot loader start. */
EFI_STATUS EFIAPI
OStartImage(
	IN EFI_HANDLE			ImageHandle,
	OUT UINTN         *ExitDataSize,
	OUT CHAR16				**ExitData  OPTIONAL
)
{
	EFI_STATUS				Status;
	EFI_LOADED_IMAGE_PROTOCOL		*Image;
	CHAR16            *FilePathText = NULL;
//	CHAR16					*BootLoaders[] = BOOT_LOADERS;
	UINTN					Index;
	
	PRINT("->StartImage(0x%lx, , )\n", ImageHandle);

	//
	// Get gEfiLoadedImageProtocolGuid for image that is starting
	//
	Status = gBS->OpenProtocol (
		ImageHandle,
		&gEfiLoadedImageProtocolGuid,
		(VOID **) &Image,
		gImageHandle,
		NULL,
		EFI_OPEN_PROTOCOL_GET_PROTOCOL
	);
	if (Status != EFI_SUCCESS) {
		PRINT("ERROR: OStartImage: OpenProtocol(gEfiLoadedImageProtocolGuid) = %r\n", Status);
		return EFI_INVALID_PARAMETER;
	}
	
	//
	// Extract file path from image device file path
	//
	FilePathText = FileDevicePathToText(Image->FilePath);
	if (FilePathText == NULL) {
		PRINT("ERROR: OStartImage: image file path is NULL\n");
		return EFI_INVALID_PARAMETER;
	}
	PRINT(" File: %s\n", FilePathText);
	PRINT(" Image: %p - %x (%x)\n", Image->ImageBase, (UINTN)Image->ImageBase + Image->ImageSize, Image->ImageSize);
	
	Status = gBS->CloseProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, gImageHandle, NULL);
	if (EFI_ERROR(Status)) {
		PRINT("CloseProtocol error: %r\n", Status);
	}
	
	//
	// Check if this is some known boot manager/loader
	//
	for (Index = 0; BootLoaders[Index] != NULL && !StrStriBasic(FilePathText, BootLoaders[Index]); Index++);
	if (BootLoaders[Index] != NULL) {
		//
		// it is
		// restore original StartImage
		// and start our overrides
		//
		gBS->StartImage = OrgStartImage;
		
		StartOverrides();
		PRINT("\nSTARTING: %s\n\n", FilePathText);
	}
	
	//
	// Start image by calling original StartImage
	//
	Status = OrgStartImage(ImageHandle, ExitDataSize, ExitData);
	
	FreePool(FilePathText);
	return Status;
}

/** Driver's entry point. Installs our StartImage to detect boot loader start. */
EFI_STATUS
EFIAPI
DumpUefiCallsEntrypoint (
	IN EFI_HANDLE				ImageHandle,
	IN EFI_SYSTEM_TABLE			*SystemTable
)
{
	//
	// Override StartImage
	// other overrides will be done from there when boot loader is started
	//
	OrgStartImage = gBS->StartImage;
	gBS->StartImage = OStartImage;
	gBS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
	
	return EFI_SUCCESS;
}

/**

 Low memory fixes for some UEFI boards to load OSX.

 by dmazar, based on modbin info:
 http://www.projectosx.com/forum/index.php?showtopic=2428&st=0&p=17766&#entry17766
 
 confirmed by akbar102 with Aspire 5750G with InsydeH2O UEFI:
 http://www.projectosx.com/forum/index.php?showtopic=2428&st=300&p=19078&#entry19078

**/

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/CpuLib.h>

#include <Protocol/LoadedImage.h>

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

#include "../Version.h"
CONST CHAR8* CloverRevision = REVISION_STR;


// the highest address that kernel will use
#define KERNEL_TOP_ADDRESS		0x10000000		// 256MB

EFI_IMAGE_START 			gStartImage = NULL;

/** Free all mem regions between 0x100000 and KERNEL_TOP_ADDRESS */
EFI_STATUS
DoFixes(VOID)
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
	
	//DBG("MEMMAP: Size=%d, Addr=%p, DescSize=%d, DescVersion: 0x%x\n", MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
	//DBG("Type       Start            End       VStart               # Pages          Attributes\n");
	MemoryMapEnd = NEXT_MEMORY_DESCRIPTOR(MemoryMap, MemoryMapSize);
	for (Desc = MemoryMap ; Desc < MemoryMapEnd; Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize)) {
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
		if (Desc->Type == EfiConventionalMemory												// free mem
			|| Desc->PhysicalStart + EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages) <= 0x100000		// below critical space
			|| Desc->PhysicalStart >= KERNEL_TOP_ADDRESS									// above critical space
			)
		{
			continue;
		}
		
		// occupied mem block - free it
		Status = gBS->FreePages(Desc->PhysicalStart, Desc->NumberOfPages);
		DBG("FreePages: At %lx, Num=0x%x = %r\n", Desc->PhysicalStart, Desc->NumberOfPages, Status);
	}
	
	// release mem
	FreePool(MemoryMap);
	
	return Status;
}


/** Called to start an image. If this is boot.efi, then do our fixes.*/
EFI_STATUS
EFIAPI
MOStartImage (
	IN EFI_HANDLE				ImageHandle,
	OUT UINTN					*ExitDataSize,
	OUT CHAR16					**ExitData  OPTIONAL
	)
{
	EFI_STATUS					Status;
	EFI_LOADED_IMAGE_PROTOCOL	*Image;
	CHAR16						*FilePathText = NULL;
	
	DBG("StartImage(%lx)\n", ImageHandle);

	// find out image name from EfiLoadedImageProtocol
	Status = gBS->OpenProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &Image, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (Status != EFI_SUCCESS) {
		DBG("ERROR: MOStartImage: OpenProtocol(gEfiLoadedImageProtocolGuid) = %r\n", Status);
		return EFI_INVALID_PARAMETER;
	}
	FilePathText = FileDevicePathToText(Image->FilePath);
	if (FilePathText != NULL) {
		DBG("FilePath: %s\n", FilePathText);
	}
	DBG("ImageBase: %p - %lx (%lx)\n", Image->ImageBase, (UINT64)Image->ImageBase + Image->ImageSize, Image->ImageSize);
	Status = gBS->CloseProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, gImageHandle, NULL);
	if (EFI_ERROR(Status)) {
		DBG("CloseProtocol error: %r\n", Status);
	}
	
	// check if this is boot.efi
	if (StrStriBasic(FilePathText, L"boot.efi")) {
		// it is - fix lower mem
		DoFixes();
	}
	
	// call original to do the job
	Status = gStartImage(ImageHandle, ExitDataSize, ExitData);

	if (FilePathText != NULL) {
		gBS->FreePool(FilePathText);
	}
	return Status;
}


/** Entry point. Installs StartImage override to detect boot.efi start. */
EFI_STATUS
EFIAPI
OsxLowMemFixDrvEntrypoint (
	IN EFI_HANDLE				ImageHandle,
	IN EFI_SYSTEM_TABLE			*SystemTable
	)
{
	//EFI_PHYSICAL_ADDRESS		Addr;
	
	// install StartImage override
	gStartImage = gBS->StartImage;
	gBS->StartImage = MOStartImage;
	gBS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gBS, sizeof(EFI_BOOT_SERVICES), &gBS->Hdr.CRC32);
	
	/*
	// alloc test regions
	Addr = 0x2000000;
	gBS->AllocatePages(AllocateAddress, EfiBootServicesData, 0xcc, &Addr);
	Addr = 0x0F0000;
	gBS->AllocatePages(AllocateAddress, EfiBootServicesData, 0x40, &Addr);
	Addr = 0x160000;
	gBS->AllocatePages(AllocateAddress, EfiBootServicesData, 0xaa, &Addr);
	Addr = 0x5FFF000;
	gBS->AllocatePages(AllocateAddress, EfiBootServicesData, 0x4, &Addr);
	*/
	return EFI_SUCCESS;
}


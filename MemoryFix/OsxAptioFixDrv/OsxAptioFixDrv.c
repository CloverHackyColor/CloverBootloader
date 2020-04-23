/**

  UEFI driver for enabling loading of OSX by using memory relocation.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/CpuLib.h>

#include <Guid/GlobalVariable.h>

#include <Protocol/LoadedImage.h>

#include "BootFixes.h"
#include "DecodedKernelCheck.h"
#include "BootArgs.h"
#include "AsmFuncs.h"
#include "VMem.h"
#include "Lib.h"
#include "Hibernate.h"
#include "NVRAMDebug.h"


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
STATIC UINTN Counter = 0;


// defines the size of block that will be allocated for kernel image relocation,
//   without RT and MMIO regions
// rehabman - Increase the size for ElCapitan to 128Mb 0x8000
// stinga11 - 0x6000
#define KERNEL_BLOCK_NO_RT_SIZE_PAGES	0x8000

// TRUE if we are doing hibernate wake
BOOLEAN gHibernateWake = FALSE;


// placeholders for storing original Boot Services functions
EFI_ALLOCATE_PAGES 			gStoredAllocatePages = NULL;
EFI_GET_MEMORY_MAP 			gStoredGetMemoryMap = NULL;
EFI_EXIT_BOOT_SERVICES 		gStoredExitBootServices = NULL;
EFI_IMAGE_START 			gStartImage = NULL;
EFI_HANDLE_PROTOCOL			gHandleProtocol = NULL;


// monitoring AlocatePages
EFI_PHYSICAL_ADDRESS gMinAllocatedAddr = 0;
EFI_PHYSICAL_ADDRESS gMaxAllocatedAddr = 0;

// relocation base address
EFI_PHYSICAL_ADDRESS gRelocBase = 0;
// relocation block size in pages
UINTN gRelocSizePages = 0;

// location of memory allocated by boot.efi for hibernate image
EFI_PHYSICAL_ADDRESS gHibernateImageAddress = 0;

// last memory map obtained by boot.efi
UINTN					gLastMemoryMapSize = 0;
EFI_MEMORY_DESCRIPTOR	*gLastMemoryMap = NULL;
UINTN					gLastDescriptorSize = 0;
UINT32					gLastDescriptorVersion = 0;

/** Helper function that calls GetMemoryMap() and returns new MapKey.
 * Uses gStoredGetMemoryMap, so can be called only after gStoredGetMemoryMap is set.
 */
EFI_STATUS
GetMemoryMapKey(OUT UINTN *MapKey)
{
	EFI_STATUS					Status;
	UINTN						MemoryMapSize;
	EFI_MEMORY_DESCRIPTOR		*MemoryMap;
	UINTN						DescriptorSize;
	UINT32						DescriptorVersion;
	
	Status = GetMemoryMapAlloc(gStoredGetMemoryMap, &MemoryMapSize, &MemoryMap, MapKey, &DescriptorSize, &DescriptorVersion);
	return Status;
}

/** Helper function that calculates number of RT and MMIO pages from mem map. */
EFI_STATUS
GetNumberOfRTPages(OUT UINTN *NumPages)
{
	EFI_STATUS					Status;
	UINTN						MemoryMapSize;
	EFI_MEMORY_DESCRIPTOR		*MemoryMap;
	UINTN						MapKey;
	UINTN						DescriptorSize;
	UINT32						DescriptorVersion;
	UINTN						NumEntries;
	UINTN						Index;
	EFI_MEMORY_DESCRIPTOR		*Desc;
	
	Status = GetMemoryMapAlloc(gBS->GetMemoryMap, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	//
	// Apply some fixes
	//
	FixMemMap(MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
	
	//
	// Sum RT and MMIO areas - all that have runtime attribute
	//
	
	*NumPages = 0;
	Desc = MemoryMap;
	NumEntries = MemoryMapSize / DescriptorSize;
	
	for (Index = 0; Index < NumEntries; Index++) {
		if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0) {
			*NumPages += Desc->NumberOfPages;
		}
		Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
	}

	return Status;
}


/** Calculate the size of reloc block.
  * gRelocSizePages = KERNEL_BLOCK_NO_RT_SIZE_PAGES + RT&MMIO pages
  */
EFI_STATUS
CalculateRelocBlockSize(VOID)
{
	EFI_STATUS				Status;
	UINTN					NumPagesRT;
	
	
	// Sum pages needed for RT and MMIO areas
	Status = GetNumberOfRTPages(&NumPagesRT);
	if (EFI_ERROR(Status)) {
		DBGnvr("GetNumberOfRTPages: %r\n", Status);
		DBG("OsxAptioFixDrv: CalculateRelocBlockSize(): GetNumberOfRTPages: %r\n", Status);
		Print(L"OsxAptioFixDrv: CalculateRelocBlockSize(): GetNumberOfRTPages: %r\n", Status);
		return Status;
	}
	
	gRelocSizePages = KERNEL_BLOCK_NO_RT_SIZE_PAGES + NumPagesRT;
	DBGnvr("Reloc block: %x pages (%d MB) = kernel %x (%d MB) + RT&MMIO %x (%d MB)\n",
		   gRelocSizePages, EFI_PAGES_TO_SIZE(gRelocSizePages) >> 20,
		   KERNEL_BLOCK_NO_RT_SIZE_PAGES, EFI_PAGES_TO_SIZE(KERNEL_BLOCK_NO_RT_SIZE_PAGES) >> 20,
		   NumPagesRT, EFI_PAGES_TO_SIZE(NumPagesRT) >> 20
		   );
	
	return Status;
}

/** Allocate free block on top of mem for kernel image relocation (will be returned to boot.efi for kernel boot image). */
EFI_STATUS
AllocateRelocBlock()
{
	EFI_STATUS				Status;
	EFI_PHYSICAL_ADDRESS	Addr;
	
	
	// calculate the needed size for reloc block
	CalculateRelocBlockSize();
	
	gRelocBase = 0;
	Addr = 0x100000000; // max address
	Status = AllocatePagesFromTop(EfiBootServicesData, gRelocSizePages, &Addr);
	if (Status != EFI_SUCCESS) {
		DBG("OsxAptioFixDrv: AllocateRelocBlock(): can not allocate relocation block (0x%x pages below 0x%lx): %r\n",
			gRelocSizePages, Addr, Status);
		Print(L"OsxAptioFixDrv: AllocateRelocBlock(): can not allocate relocation block (0x%x pages below 0x%lx): %r\n",
			gRelocSizePages, Addr, Status);
	} else {
		gRelocBase = Addr;
		DBG("OsxAptioFixDrv: AllocateRelocBlock(): gRelocBase set to %lx - %lx\n", gRelocBase, gRelocBase + EFI_PAGES_TO_SIZE(gRelocSizePages) - 1);
		DBGnvr("gRelocBase set to %lx - %lx\n", gRelocBase, gRelocBase + EFI_PAGES_TO_SIZE(gRelocSizePages) - 1);
	}

	// set reloc addr in runtime vars for boot manager
	//Print(L"OsxAptioFixDrv: AllocateRelocBlock(): gRelocBase set to %lx - %lx\n", gRelocBase, gRelocBase + EFI_PAGES_TO_SIZE(gRelocSizePages) - 1);
	/*Status = */gRT->SetVariable(L"OsxAptioFixDrv-RelocBase", &gEfiAppleBootGuid, 
							  /*   EFI_VARIABLE_NON_VOLATILE |*/ EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
							  sizeof(gRelocBase) ,&gRelocBase);
	return Status;
}

/** Releases relocation block. */
EFI_STATUS
FreeRelocBlock()
{
	
	return gBS->FreePages(gRelocBase, gRelocSizePages);
}



/** gBS->HandleProtocol override:
  * Boot.efi requires EfiGraphicsOutputProtocol on ConOutHandle, but it is not present
  * there on Aptio 2.0. EfiGraphicsOutputProtocol exists on some other handle.
  * If this is the case, we'll intercept that call and return EfiGraphicsOutputProtocol
  * from that other handle.
  */
EFI_STATUS EFIAPI
MOHandleProtocol(
	IN EFI_HANDLE		Handle,
	IN EFI_GUID			*Protocol,
	OUT VOID			**Interface
)
{
	EFI_STATUS			res;
	EFI_GRAPHICS_OUTPUT_PROTOCOL	*GraphicsOutput;
	
	// special handling if gEfiGraphicsOutputProtocolGuid is requested by boot.efi
	if (CompareGuid(Protocol, &gEfiGraphicsOutputProtocolGuid)) {
		res = gHandleProtocol(Handle, Protocol, Interface);
		if (res != EFI_SUCCESS) {
			// let's find it on some other handle
			res = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&GraphicsOutput);
			if (res == EFI_SUCCESS) {
				// return it
				*Interface = GraphicsOutput;
	//			DBG("->HandleProtocol(%p, %s, %p) = %r (returning from other handle)\n", Handle, GuidStr(Protocol), *Interface, res);
				DBGnvr("->HandleProtocol(%p, %s, %p) = %r (from other handle)\n", Handle, GuidStr(Protocol), *Interface, res);
				return res;
			}
		}
		DBGnvr("->HandleProtocol(%p, %s, %p) = %r\n", Handle, GuidStr(Protocol), *Interface, res);
	} else {
		res = gHandleProtocol(Handle, Protocol, Interface);
	}
//	DBG("->HandleProtocol(%p, %s, %p) = %r\n", Handle, GuidStr(Protocol), *Interface, res);
	return res;
}

/** gBS->AllocatePages override:
  * Returns pages from free memory block to boot.efi for kernel boot image.
  */
EFI_STATUS
EFIAPI
MOAllocatePages (
	IN EFI_ALLOCATE_TYPE		Type,
	IN EFI_MEMORY_TYPE			MemoryType,
	IN UINTN					NumberOfPages,
	IN OUT EFI_PHYSICAL_ADDRESS	*Memory
	)
{
	EFI_STATUS					Status;
	EFI_PHYSICAL_ADDRESS		UpperAddr;
//	EFI_PHYSICAL_ADDRESS		MemoryIn;
//	BOOLEAN						FromRelocBlock = FALSE;
	

//	MemoryIn = *Memory;
	
	if (Type == AllocateAddress && MemoryType == EfiLoaderData) {
		// called from boot.efi
		
		UpperAddr = *Memory + EFI_PAGES_TO_SIZE(NumberOfPages);
		
		// check if the requested mem can be served from reloc block
    // the upper address is compared to the size of the relocation block to achieve Address + gRelocBase for all
    // allocations, so that the entire block can be copied to the proper location on kernel entry
    // Comparing only the number of pages will not only give wrong results as gRelocSizePages is not decreased,
    // but also implies memory is 'stacked', which it is not.
    if (UpperAddr >= EFI_PAGES_TO_SIZE(gRelocSizePages)) {
			// no - exceeds our block - signal error
			Print(L"OsxAptioFixDrv: Error - requested memory exceeds our allocated relocation block\n");
			Print(L"Requested mem: %lx - %lx, Pages: %x, Size: %lx\n",
				  *Memory, UpperAddr - 1,
				  NumberOfPages, EFI_PAGES_TO_SIZE(NumberOfPages)
				  );
			Print(L"Reloc block: %lx - %lx, Pages: %x, Size: %lx\n",
				  gRelocBase, gRelocBase + EFI_PAGES_TO_SIZE(gRelocSizePages) - 1,
				  gRelocSizePages, EFI_PAGES_TO_SIZE(gRelocSizePages)
				  );
			Print(L"Reloc block can handle mem requests: %lx - %lx\n",
				  0, EFI_PAGES_TO_SIZE(gRelocSizePages) - 1
				  );
			Print(L"Exiting in 30 secs ...\n");
			gBS->Stall(30 * 1000000);
			
			return EFI_OUT_OF_RESOURCES;
		}
		
		// store min and max mem - can be used later to determine start and end of kernel boot image
		if (gMinAllocatedAddr == 0 || *Memory < gMinAllocatedAddr) gMinAllocatedAddr = *Memory;
		if (UpperAddr > gMaxAllocatedAddr) gMaxAllocatedAddr = UpperAddr;
		
		// give it from our allocated block
		*Memory += gRelocBase;
		//Status = gStoredAllocatePages(Type, MemoryType, NumberOfPages, Memory);
//		FromRelocBlock = TRUE;
		Status = EFI_SUCCESS;
		
	} else {
		// default page allocation
		Status = gStoredAllocatePages(Type, MemoryType, NumberOfPages, Memory);
	}

	//DBG("AllocatePages(%s, %s, %x, %lx/%lx) = %r %c\n",
	//	EfiAllocateTypeDesc[Type], EfiMemoryTypeDesc[MemoryType], NumberOfPages, MemoryIn, *Memory, Status, FromRelocBlock ? L'+' : L' ');
	return Status;
}


/** gBS->GetMemoryMap override:
  * Returns shrinked memory map. I think kernel can handle up to 128 entries.
  */
EFI_STATUS
EFIAPI
MOGetMemoryMap (
	IN OUT UINTN					*MemoryMapSize,
	IN OUT EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	OUT UINTN						*MapKey,
	OUT UINTN						*DescriptorSize,
	OUT UINT32						*DescriptorVersion
	)
{
	EFI_STATUS						Status;

	Status = gStoredGetMemoryMap(MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
	//PrintMemMap(*MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
	DBGnvr("GetMemoryMap: %p = %r\n", MemoryMap, Status);
	if (Status == EFI_SUCCESS) {
		FixMemMap(*MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
		//ShrinkMemMap(MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
		//PrintMemMap(*MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
		
		// remember last/final memmap
		gLastMemoryMapSize = *MemoryMapSize;
		gLastMemoryMap = MemoryMap;
		gLastDescriptorSize = *DescriptorSize;
		gLastDescriptorVersion = *DescriptorVersion;
	}
	return Status;
}

/** gBS->ExitBootServices override:
  * Patches kernel entry point with jump to our KernelEntryPatchJumpBack().
  */
EFI_STATUS
EFIAPI
MOExitBootServices (
	IN EFI_HANDLE				ImageHandle,
	IN UINTN					MapKey
	)
{
	EFI_STATUS					Status;
	UINTN					 	NewMapKey;
	UINTN						SlideAddr = 0;
	VOID						*MachOImage = NULL;
	
	// for  tests: we can just return EFI_SUCCESS and continue using Print for debug.
//	Status = EFI_SUCCESS;
	//Print(L"ExitBootServices()\n");
	Status = gStoredExitBootServices(ImageHandle, MapKey);
	DBGnvr("ExitBootServices:  = %r\n", Status);
	if (EFI_ERROR(Status)) {
		// just report error as var in nvram to be visible from OSX with "nvrap -p"
		gRT->SetVariable(L"OsxAptioFixDrv-ErrorExitingBootServices",
						 &gEfiAppleBootGuid,
						 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
						 3,
						 "Yes"
						 );
		
		Status = GetMemoryMapKey(&NewMapKey);
		DBGnvr("ExitBootServices: GetMemoryMapKey = %r\n", Status);
		if (Status == EFI_SUCCESS) {
			// we have latest mem map and NewMapKey
			// we'll try again ExitBootServices with NewMapKey
			Status = gStoredExitBootServices(ImageHandle, NewMapKey);
			DBGnvr("ExitBootServices: 2nd try = %r\n", Status);
			if (EFI_ERROR(Status)) {
				// Error!
				Print(L"OsxAptioFixDrv: Error ExitBootServices() 2nd try = Status: %r\n", Status);
			}
		} else {
			Print(L"OsxAptioFixDrv: Error ExitBootServices(), GetMemoryMapKey() = Status: %r\n", Status);
			Status = EFI_INVALID_PARAMETER;
		}
		
	}
	
    if (EFI_ERROR(Status)) {
		Print(L"... waiting 10 secs ...\n");
		gBS->Stall(10*1000000);
        return Status;
    }
	
	DBG("ExitBootServices: gMinAllocatedAddr: %lx, gMaxAllocatedAddr: %lx\n", gMinAllocatedAddr, gMaxAllocatedAddr);
	MachOImage = (VOID*)(UINTN)(gRelocBase + 0x200000);
	KernelEntryFromMachOPatchJump(MachOImage, SlideAddr);
	
	return Status;
}


/** Callback called when boot.efi jumps to kernel. */
UINTN
EFIAPI
KernelEntryPatchJumpBack(UINTN bootArgs, BOOLEAN ModeX64)
{
	
	DBGnvr("\nBACK FROM KERNEL: BootArgs = %x, KernelEntry: %x, Kernel called in %s bit mode\n", bootArgs, AsmKernelEntry, (ModeX64 ? L"64" : L"32"));
	
	bootArgs = FixBootingWithRelocBlock(bootArgs, ModeX64);
	
	DBGnvr("BACK TO KERNEL: BootArgs = %x, KImgStartReloc = %x, KImgStart = %x, KImgSize = %x\n",
		   bootArgs, AsmKernelImageStartReloc, AsmKernelImageStart, AsmKernelImageSize);
	
	// debug for jumping back to kernel
	// put HLT to kernel entry point to stop there
	//SetMem((VOID*)(UINTN)(AsmKernelEntry + gRelocBase), 1, 0xF4);
	// put 0 to kernel entry point to restart
	//SetMem64((VOID*)(UINTN)(AsmKernelEntry + gRelocBase), 1, 0);
	
	return bootArgs;
}



/** SWITCH_STACK_ENTRY_POINT implementation:
  * Allocates kernel image reloc block, installs UEFI overrides and starts given image.
  * If image returns, then deinstalls overrides and releases kernel image reloc block.
  *
  * If started with ImgContext->JumpBuffer, then it will return with LongJump().
  */
EFI_STATUS
RunImageWithOverrides(IN EFI_HANDLE ImageHandle, OUT UINTN *ExitDataSize, OUT CHAR16 **ExitData  OPTIONAL)
{
	EFI_STATUS					Status;
	
	// save current 64bit state - will be restored later in callback from kernel jump
	// and relocate MyAsmCopyAndJumpToKernel32 code to higher mem (for copying kernel back to
	// proper place and jumping back to it)
	Status = PrepareJumpFromKernel();
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	// init VMem memory pool - will be used after ExitBootServices
	Status = VmAllocateMemoryPool();
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	// allocate block for kernel image relocation
	Status = AllocateRelocBlock();
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	// clear monitoring vars
	gMinAllocatedAddr = 0;
	gMaxAllocatedAddr = 0;
	
	// save original BS functions
	gStoredAllocatePages = gBS->AllocatePages;
	gStoredGetMemoryMap = gBS->GetMemoryMap;
	gStoredExitBootServices = gBS->ExitBootServices;
	gHandleProtocol = gBS->HandleProtocol;
	
	// install our overrides
	gBS->AllocatePages = MOAllocatePages;
	gBS->GetMemoryMap = MOGetMemoryMap;
	gBS->ExitBootServices = MOExitBootServices;
	gBS->HandleProtocol = MOHandleProtocol;

	gBS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
	
	// run image
	Status = gStartImage(ImageHandle, ExitDataSize, ExitData);
	
	// if we get here then boot.efi did not start kernel
	// and we'll try to do some cleanup ...
	
	// return back originals
	gBS->AllocatePages = gStoredAllocatePages;
	gBS->GetMemoryMap = gStoredGetMemoryMap;
	gBS->ExitBootServices = gStoredExitBootServices;
	gBS->HandleProtocol = gHandleProtocol;

	gBS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
	
	// release reloc block
	FreeRelocBlock();
	
	return Status;
}

/** gBS->StartImage override:
  * Called to start an efi image.
  *
  * If this is boot.efi, then run it with our overrides.
  */
EFI_STATUS
EFIAPI
MOStartImage (
	IN EFI_HANDLE			ImageHandle,
	OUT UINTN					*ExitDataSize,
	OUT CHAR16				**ExitData  OPTIONAL
	)
{
	EFI_STATUS				Status;
	EFI_LOADED_IMAGE_PROTOCOL	*Image;
	CHAR16						*FilePathText = NULL;
	UINTN						Size = 0;
  	VOID                        *Value        = NULL;
  	UINTN                       Size2         = 0;
  	CHAR16                      *StartFlag    = NULL;	
	
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

	//the presence of the variable means HibernateWake
	//if the wake is canceled then the variable must be deleted
	Status = gRT->GetVariable(L"boot-switch-vars", &gEfiAppleBootGuid, NULL, &Size, NULL);
	gHibernateWake = (Status == EFI_BUFFER_TOO_SMALL);
	
    if (StrStriBasic(FilePathText,L"boot.efi") /*|| StrStriBasic(FilePathText,L"booter")*/) {
      Status = GetVariable2 (L"aptiofixflag", &gEfiAppleBootGuid, &Value, &Size2);
      if (!EFI_ERROR(Status)) {
        Status = gRT->SetVariable(L"recovery-boot-mode", &gEfiAppleBootGuid,
                                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                  Size2, Value);
        if (EFI_ERROR(Status)) {
          DBG(" Something goes wrong while setting recovery-boot-mode\n");
        }
        Status = gRT->SetVariable (L"aptiofixflag", &gEfiAppleBootGuid, 0, 0, NULL);
        FreePool(Value);
      }
    
      Size2 =0;
      //Check recovery-boot-mode present for nested boot.efi
      Status = GetVariable2 (L"recovery-boot-mode", &gEfiAppleBootGuid, &Value, &Size2);
      if (!EFI_ERROR(Status)) {
        //If it presents, then wait for \com.apple.recovery.boot\boot.efi boot
        DBG(" recovery-boot-mode present\n");
        StartFlag = StrStriBasic(FilePathText,L"\\com.apple.recovery.boot\\boot.efi");
        if (Counter > 0x00){
          Status = gRT->SetVariable(L"aptiofixflag", &gEfiAppleBootGuid,
                                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                    Size2, Value);
          if (EFI_ERROR(Status)) {
            DBG("Something goes wrong! \n");
          }
          gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
        }
      } else {
        StartFlag = StrStriBasic(FilePathText,L"boot.efi");
        /*if (!StartFlag) {
          StartFlag = StrStriBasic(FilePathText,L"booter") ;
        }*/
      }
      FreePool(Value);
    }

	// check if this is boot.efi
	if (StartFlag && !gHibernateWake) {
		Counter++;
		Print(L"OsxAptioFixDrv: Starting overrides for %s\nUsing reloc block: yes, hibernate wake: %s \n",
			  FilePathText, gHibernateWake ? L"yes" : L"no");
		//gBS->Stall(2000000);

		// run with our overrides
		Status = RunImageWithOverrides(ImageHandle, ExitDataSize, ExitData);
		
	} else {
		// call original function to do the job
		Status = gStartImage(ImageHandle, ExitDataSize, ExitData);
	}
	
	if (FilePathText != NULL) {
		gBS->FreePool(FilePathText);
	}
	return Status;
}


/** Entry point. Installs our StartImage override.
  * All other stuff will be installed from there when boot.efi is started.
  */
EFI_STATUS
EFIAPI
OsxAptioFixDrvEntrypoint (
	IN EFI_HANDLE				ImageHandle,
	IN EFI_SYSTEM_TABLE			*SystemTable
	)
{
	// install StartImage override
	// all other overrides will be started when boot.efi is started
	gStartImage = gBS->StartImage;
	gBS->StartImage = MOStartImage;
	gBS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
	
	return EFI_SUCCESS;
}


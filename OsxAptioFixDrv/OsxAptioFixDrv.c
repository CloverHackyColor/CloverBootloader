/**

  Memory overrides to enable ASUS board UEFI to load OSX.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/CpuLib.h>

#include <Protocol/LoadedImage.h>

#include "BootFixes.h"
#include "DecodedKernelCheck.h"
#include "BootArgs.h"
#include "AsmFuncs.h"
#include "VMem.h"
#include "Lib.h"
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



// defines the size of block that will be allocated for kernel image relocation,
// without RT and MMIO regions
#define KERNEL_BLOCK_NO_RT_SIZE_PAGES	0x4000 // 64MB for


typedef struct _START_IMG_CONTEXT {
	VOID						*JumpBufferBase;
	BASE_LIBRARY_JUMP_BUFFER	*JumpBuffer;
	EFI_HANDLE					ImageHandle;
	UINTN						*ExitDataSize;
	CHAR16						**ExitData;
	EFI_STATUS					StartImageStatus;
} START_IMG_CONTEXT;


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

// start of our image
EFI_PHYSICAL_ADDRESS gOurImageStart = 0;


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


/** Allocates StackSizePages for stack high in memory. Returns stack bottom and top. */
EFI_STATUS
AllocateHighStack(IN UINTN StackSizePages, OUT EFI_PHYSICAL_ADDRESS *StackBottom, OUT EFI_PHYSICAL_ADDRESS *StackTop)
{
	EFI_STATUS					Status;

	// set max address and allocate mem
	*StackBottom = 0x100000000;
	Status = AllocatePagesFromTop(EfiBootServicesData, StackSizePages, StackBottom);
	if (Status != EFI_SUCCESS) {
		Print(L"OsxAptioFixDrv: AllocateHighStack(): can not allocate mem for stack (0x%x pages on mem top): %r\n",
			  StackSizePages, Status);
		return Status;
	}
	
	// set stack top
	*StackTop = *StackBottom + EFI_PAGES_TO_SIZE(StackSizePages) - 8;

	DBG("Stack alloc ok: %lx - %lx\n", *StackBottom, *StackTop);
	DBGnvr("Stack alloc ok: %lx - %lx\n", *StackBottom, *StackTop);
	
	return EFI_SUCCESS;
}

/** Releases mem allocated for stack. */
EFI_STATUS
ReleaseHighStack(IN EFI_PHYSICAL_ADDRESS StackBottom, IN UINTN StackSizePages)
{
	
	return gBS->FreePages(StackBottom, StackSizePages);
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
	
	
	gRelocBase = 0;
	Addr = 0x100000000; // max address
	Status = AllocatePagesFromTop(EfiBootServicesData, gRelocSizePages, &Addr);
	if (Status != EFI_SUCCESS) {
		DBG("OsxAptioFixDrv: AllocateRelocBlock(): can not allocate relocation block (0x%x pages below 0x%lx): %r\n",
			gRelocSizePages, 0x100000000, Status);
		Print(L"OsxAptioFixDrv: AllocateRelocBlock(): can not allocate relocation block (0x%x pages below 0x%lx): %r\n",
			gRelocSizePages, 0x100000000, Status);
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
	EFI_PHYSICAL_ADDRESS		MemoryIn;
	BOOLEAN						FromRelocBlock = FALSE;
	

	MemoryIn = *Memory;
	if (Type == AllocateAddress && MemoryType == EfiLoaderData) {
		// called from boot.efi
		
		UpperAddr = *Memory + EFI_PAGES_TO_SIZE(NumberOfPages);
		
		// check if the requested mem can be served from reloc block
		if (UpperAddr >= EFI_PAGES_TO_SIZE(gRelocSizePages)) {
			// no - exceeds our block - signal error
			Print(L"OsxAptipFixDrv: Error - requested memory exceeds our allocated relocation block\n");
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
		FromRelocBlock = TRUE;
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
UINTN LastMapKey = 0;

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
	}
	LastMapKey = (Status == EFI_SUCCESS) ? *MapKey : 0;
	return Status;
}

/** gBS->ExitBootServices override:
  * Patches kernel entry point with jump to our code.
  */
EFI_STATUS
EFIAPI
MOExitBootServices (
	IN EFI_HANDLE				ImageHandle,
	IN UINTN					MapKey
	)
{
	EFI_STATUS					Status;
	//UINT64						*p64;
	UINT64						RSP;
	UINTN					 	NewMapKey;
	//VOID						*BootArgs;
	
	
	
	// we'll try to resolve the need for kernel entry point
	// with KernelEntryPatchJumpFill(). see this func for details.
	
	//CheckDecodedKernel();
	
	// check again for stack - if relocation did not work for some reason
	RSP = MyAsmReadSp();
	if (RSP < gMaxAllocatedAddr) {
		Print(L"\nOsxAptioFixDrv: Stack too low! Currently at %lx, and kernel would be %lx - %lx\n",
			RSP, gMinAllocatedAddr, gMaxAllocatedAddr);
		Print(L"Waiting 20 seconds then exiting ...\n");
		gBS->Stall(20 * 1000 * 1000);
		return EFI_NOT_FOUND;
	}
	
	// BootArgs test
	//BootArgs = BootArgsFind(gRelocBase + 0x0200000);
	//BootArgsPrint(BootArgs);
	
	// for  tests: we can just return EFI_SUCCESS and continue using Print for debug.
	Status = EFI_SUCCESS;
	//Print(L"ExitBootServices()\n");
	Status = gStoredExitBootServices(ImageHandle, MapKey);
	DBGnvr("ExitBootServices:  = %r\n", Status);
	if (EFI_ERROR (Status)) {
		Print(L"OsxAptioFixDrv: Error ExitBootServices() = Status: %r\n", Status);
		Print(L"MapKey = %lx, LastMapKey = %lx\n", MapKey, LastMapKey);
		Print(L"This is an error and should be resolved.\nFor now, we will force ExitBootServices() once again in 10 secs with new GetMemoryMap ...\n");

		gBS->Stall(10*1000000);
		//CpuDeadLoop();
		
		Status = GetMemoryMapKey(&NewMapKey);
		DBGnvr("ExitBootServices: GetMemoryMapKey = %r\n", Status);
		if (Status == EFI_SUCCESS) {
			// we have latest mem map and NewMapKey
			// we'll try again ExitBootServices with NewMapKey
			Status = gStoredExitBootServices(ImageHandle, NewMapKey);
			DBGnvr("ExitBootServices: 2nd try = %r\n", Status);
			if (EFI_ERROR (Status)) {
				// Error!
				Print(L"OsxAptioFixDrv: Error ExitBootServices() 2nd try = Status: %r\n", Status);
			}
		} else {
			Print(L"OsxAptioFixDrv: Error ExitBootServices(), GetMemoryMapKey() = Status: %r\n", Status);
			Status = EFI_INVALID_PARAMETER;
		}
		
	}
	
	if (Status == EFI_SUCCESS) {
		//KernelEntryPatchJumpFill();
		KernelEntryFromMachOPatchJump();
		//CpuDeadLoop();
	} else {
		Print(L"... waiting 10 secs ...\n");
		gBS->Stall(10*1000000);
	}
	
	return Status;
}

/* for test
EFI_SET_VIRTUAL_ADDRESS_MAP OrgSetVirtualAddressMap = NULL;
EFI_CONVERT_POINTER	OrgConvertPointer = NULL;

UINT32 OrgRTCRC32 = 0;

VOID
AddVirtualToPhysicalMappings(
	IN UINTN					MemoryMapSize,
	IN UINTN					DescriptorSize,
	IN UINT32					DescriptorVersion,
	IN EFI_MEMORY_DESCRIPTOR	*VirtualMap
)
{
	UINTN					NumEntries;
	UINTN					Index;
	EFI_MEMORY_DESCRIPTOR	*Desc;
	PAGE_MAP_AND_DIRECTORY_POINTER	*PageTable;
	UINTN					Flags;
	
	Desc = VirtualMap;
	NumEntries = MemoryMapSize / DescriptorSize;
	
	// get current VM page table
	GetCurrentPageTable(&PageTable, &Flags);
	
	for (Index = 0; Index < NumEntries; Index++) {
		// assign virtual addresses to all EFI_MEMORY_RUNTIME marked pages (including MMIO)
		if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0) {
			DBGnvr("Map pages: %lx (%x) -> %lx\n", Desc->VirtualStart, Desc->NumberOfPages, Desc->PhysicalStart);
			VmMapVirtualPages(PageTable, Desc->VirtualStart, Desc->NumberOfPages, Desc->PhysicalStart);
		}
		Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
	}
	VmFlashCaches();
}

EFI_STATUS EFIAPI
OvrSetVirtualAddressMap(
	IN UINTN			MemoryMapSize,
	IN UINTN			DescriptorSize,
	IN UINT32			DescriptorVersion,
	IN EFI_MEMORY_DESCRIPTOR	*VirtualMap
)
{
	EFI_STATUS			Status;
	
	DBGnvr("->SetVirtualAddressMap(%d, %d, 0x%x, %p) START ...\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
	//PrintSystemTable(gST);
	//WaitForKeyPress(L"SetVirtualAddressMap: press a key to continue\n");
	//PrintMemMap(MemoryMapSize, VirtualMap, DescriptorSize, DescriptorVersion);
	//Status = EFI_SUCCESS;
	
	// restore origs
	gRT->Hdr.CRC32 = OrgRTCRC32;
	gRT->SetVirtualAddressMap = OrgSetVirtualAddressMap;
	
	AddVirtualToPhysicalMappings(MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
	
	Status = OrgSetVirtualAddressMap(MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
	DBGnvr("->SetVirtualAddressMap() = %r\n", Status);
	
	//DBG("->SetVirtualAddressMap(%d, %d, 0x%x, %p) START ...\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
	//PrintSystemTable(gST);
	//WaitForKeyPress(L"SetVirtualAddressMap: press a key to continue\n");
	//PrintMemMap(MemoryMapSize, VirtualMap, DescriptorSize, DescriptorVersion);
	//Status = EFI_SUCCESS;
	//Status = OrgSetVirtualAddressMap(MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
	//DBG("->SetVirtualAddressMap(%d, %d, 0x%x, %p) END = %r\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap, Status);
	//PrintSystemTable(gST);
	return Status;
}

EFI_STATUS EFIAPI
OvrConvertPointer(
	IN UINTN			DebugDisposition,
	IN OUT VOID			**Address
)
{
	EFI_STATUS			Status;
	VOID				*AddressIn = *Address;
	
	Status = OrgConvertPointer(DebugDisposition, Address);
	DBGnvr("->ConvertPointer(%d, %p/%p) = %r\n", DebugDisposition, AddressIn, *Address, Status);
	return Status;
}
*/




/** SWITCH_STACK_ENTRY_POINT implementation:
  * Allocates kernel image reloc block, installs UEFI overrides and starts given image.
  * If image returns, then deinstalls overrides and releases kernel image reloc block.
  *
  * If started with ImgContext->JumpBuffer, then it will return with LongJump().
  */
VOID EFIAPI
RunImageWithOverrides(IN VOID *Context1, IN VOID *Context2)
{
	START_IMG_CONTEXT			*ImgContext;
	/*
	UINT64						RSP;
	
	RSP = MyAsmReadSp();
	Print(L"RunImageWithOverrides: Current stack: RSP=%lx\n", RSP);
	gBS->Stall(10 * 1000000);
	*/
	
	// get our ImgContext
	ImgContext = (START_IMG_CONTEXT *)Context1;
	
	// save current 64bit state - will be restored later in callback from kernel jump
	// and relocate MyAsmCopyAndJumpToKernel32 code to higher mem (for copying kernel back to
	// proper place and jumping back to it)
	ImgContext->StartImageStatus = PrepareJumpFromKernel();
	if (EFI_ERROR(ImgContext->StartImageStatus)) {
		return;
	}
	
	// init VMem memory pool - will be used after ExitBootServices
	ImgContext->StartImageStatus = VmAllocateMemoryPool();
	if (EFI_ERROR(ImgContext->StartImageStatus)) {
		return;
	}
	
	// allocate block for kernel image relocation
	ImgContext->StartImageStatus = AllocateRelocBlock();
	if (EFI_ERROR(ImgContext->StartImageStatus)) {
		return;
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
	
	/* for test
	OrgRTCRC32 = gRT->Hdr.CRC32;
	OrgSetVirtualAddressMap = gRT->SetVirtualAddressMap;
	gRT->SetVirtualAddressMap = OvrSetVirtualAddressMap;
	//OrgConvertPointer = gRT->ConvertPointer;
	//gRT->ConvertPointer = OvrConvertPointer;
	gRT->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
	*/
	
	// run image
	ImgContext->StartImageStatus = gStartImage(ImgContext->ImageHandle, ImgContext->ExitDataSize, ImgContext->ExitData);
	
	// cleanup ...
	
	// return back originals
	gBS->AllocatePages = gStoredAllocatePages;
	gBS->GetMemoryMap = gStoredGetMemoryMap;
	gBS->ExitBootServices = gStoredExitBootServices;
	gBS->HandleProtocol = gHandleProtocol;

	gBS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
	
	/* for test
	gRT->SetVirtualAddressMap = OrgSetVirtualAddressMap;
	//gRT->ConvertPointer = OrgConvertPointer;
	gRT->Hdr.CRC32 = OrgRTCRC32;
	*/
	
	// release reloc block
	FreeRelocBlock();
	
	// if we got here with SwitchStack,
	// then return back via LongJump.
	if (ImgContext->JumpBuffer != NULL) {
		// jump back to old stack
		LongJump(ImgContext->JumpBuffer, 1); 
	}
	
	// else, just normal return
	return;
}

/** Runs given image with our overrides by calling RunImageWithOverrides().
  * If stack is too low, then allocate new stack higher in mem and call RunImageWithOverrides()
  * with SwitchStack and prepare return with SetJump().
  */
EFI_STATUS
RunImageWithOverridesAndHighStack(IN EFI_HANDLE ImageHandle, OUT UINTN *ExitDataSize, OUT CHAR16 **ExitData  OPTIONAL)
{
	EFI_STATUS					Status;
	START_IMG_CONTEXT			*ImgContext;
	UINTN						JumpRes;
	UINT64						RSP;
	EFI_PHYSICAL_ADDRESS		StackBottom;
	EFI_PHYSICAL_ADDRESS		StackTop;
	UINTN						StackSizePages = 32;
	
	// allocate image context
	ImgContext = AllocateZeroPool(sizeof(START_IMG_CONTEXT));
	if (ImgContext == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}
	ImgContext->ImageHandle = ImageHandle;
	ImgContext->ExitDataSize = ExitDataSize;
	ImgContext->ExitData = ExitData;
	ImgContext->StartImageStatus = EFI_SUCCESS;
	
	// calculate the needed size for reloc block
	CalculateRelocBlockSize();
	
	// read current stack
	RSP = MyAsmReadSp();
	DBG("Current stack: RSP=%lx\n", RSP);
	DBGnvr("Current stack: RSP=%lx\n", RSP);
		
	// check if stack is too low - must be > gRelocSize
	if (RSP < EFI_PAGES_TO_SIZE(gRelocSizePages)) {
		
		// too low - allocate new stack higher in memory
		Status = AllocateHighStack(StackSizePages, &StackBottom, &StackTop);
		if (Status != EFI_SUCCESS) {
			FreePool(ImgContext);
			return Status;
		}
		
		// prepare JumpBuffer for return from RunImageWithOverrides()
		ImgContext->JumpBufferBase = AllocatePool(sizeof(BASE_LIBRARY_JUMP_BUFFER) + BASE_LIBRARY_JUMP_BUFFER_ALIGNMENT);
		ImgContext->JumpBuffer = ALIGN_POINTER(ImgContext->JumpBufferBase, BASE_LIBRARY_JUMP_BUFFER_ALIGNMENT);
		
		// set JumpBuffer
		JumpRes = SetJump(ImgContext->JumpBuffer);
		if (JumpRes == 0) {
			// normal return from SetJump()
			// set new stack and call RunImageWithOverrides
			SwitchStack(RunImageWithOverrides, ImgContext, NULL, (VOID*)StackTop);
			// SwitchStack never returns
		}
		
		// we got back here from RunImageWithOverrides() with LongJump()
		
		// cleanup ...
		
		// release JumpBuffer
		FreePool(ImgContext->JumpBufferBase);
		ImgContext->JumpBufferBase = NULL;
		ImgContext->JumpBuffer = NULL;
		
		// release high stack
		ReleaseHighStack(StackBottom, StackSizePages);
		
	} else {
		// stack is already high enough - just use normal function call
		RunImageWithOverrides(ImgContext, NULL);
	}
	
	// release mem and return Status
	Status = ImgContext->StartImageStatus;
	FreePool(ImgContext);
	return Status;
}

/** gBS->ExitBootServices override:
  * Called to start an image. If this is boot.efi, then run it with high stack and our overrides.
  */
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

	//Print(L"OsxAptioFixDrv: Starting image %s\n", FilePathText);
	// check if this is boot.efi
	if (StrStriBasic(FilePathText, L"boot.efi")) {
		Print(L"OsxAptioFixDrv: Starting overrides for %s\n", FilePathText);

		// run with our overrides
		Status = RunImageWithOverridesAndHighStack(ImageHandle, ExitDataSize, ExitData);
		
	} else {
		// call original function to do the job
		Status = gStartImage(ImageHandle, ExitDataSize, ExitData);
	}
	
	if (FilePathText != NULL) {
		gBS->FreePool(FilePathText);
	}
	return Status;
}


/** Saves the lowest mem address of our image. Will be used later
  * in MOExitBootServices() for checking if we are safe to copy
  * kernel boot image to a proper low mem address.
  */
EFI_STATUS
SaveOurImageStart(VOID)
{
	EFI_LOADED_IMAGE_PROTOCOL	*Image;
	EFI_STATUS					Status;
	
	Status = gBS->OpenProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &Image, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (Status == EFI_SUCCESS) {
		gOurImageStart = (EFI_PHYSICAL_ADDRESS)Image->ImageBase;
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
	EFI_STATUS					Status;
	
	// find out where we are loaded so we can notify later if kernel could overwrite us
	Status = SaveOurImageStart();
	if (Status != EFI_SUCCESS) {
		Print(L"OsxAptioFixDrv: SaveOurImageStart = %r\n", Status);
		return Status;
	}
	
	// install StartImage override
	// all others will be started when boot.efi is started
	gStartImage = gBS->StartImage;
	gBS->StartImage = MOStartImage;
	gBS->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
	
	return EFI_SUCCESS;
}


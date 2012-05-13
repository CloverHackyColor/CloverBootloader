/**

  Methods for setting callback jump from kernel entry point, callback, fixes to kernel boot image.
  
  by dmazar

**/

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "BootFixes.h"
#include "AsmFuncs.h"
#include "BootArgs.h"
#include "VMem.h"
#include "Lib.h"
#include "FlatDevTree/device_tree.h"


// set to 1 to print calls to console
#define CONSOLE_OUTPUT 0
// set to 1 to print calls to serial
// requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file
#define SERIAL_OUTPUT 1

#if CONSOLE_OUTPUT && SERIAL_OUTPUT
	#define PRINT(...) {\
		DebugPrint(1, __VA_ARGS__);\
		AsciiPrint(__VA_ARGS__);\
	}
#elif CONSOLE_OUTPUT
	#define PRINT(...) AsciiPrint(__VA_ARGS__);
#elif SERIAL_OUTPUT
	#define PRINT(...) DebugPrint(1, __VA_ARGS__);
	//#define PRINT(...) NVRAMDebugLog(__VA_ARGS__);
#else
	#define PRINT(...)
#endif





// kernel start and size - from boot args
UINT32	kaddr;
UINT32	ksize;


void PrintSample2(unsigned char *sample, int size) {
	int i;
	for (i = 0; i < size; i++) {
		PRINT(" %02x", *sample);
		sample++;
	}
}

/** Patches kernel entry point with jump to MyAsmJumpFromKernel32 (AsmFuncsX64). This will then call KernelEntryPatchJumpBack. */
EFI_STATUS
KernelEntryPatchJump(UINT32 KernelEntry)
{
	EFI_STATUS				Status;
	unsigned char			*p;
	UINTN					MyAsmJumpFromKernel32Addr;
	
	Status = EFI_SUCCESS;

	PRINT("KernelEntryPatchJump KernelEntry (reloc): %lx (%lx)\n", KernelEntry, KernelEntry + gRelocBase);
	
	// patch real KernelEntry with 32 bit opcode for:
	//   mov ecx, MyAsmJumpFromKernel32
	//   jmp ecx	
	// = B9, <4 bytes address of MyAsmJumpFromKernel32>, FF, E1
	// note: KernelEntry + gRelocBase is not patched - no need for this
	p = (UINT8 *)(UINTN) KernelEntry;
	p[0] = 0xB9;
	MyAsmJumpFromKernel32Addr = (UINTN)MyAsmJumpFromKernel32;
	CopyMem((VOID *) (p + 1), (VOID *)&MyAsmJumpFromKernel32Addr, 4);
	p[5] = 0xFF; p[6] = 0xE1;
	//p[5] = 0xF4; //HLT - works

	PRINT("\nEntry point %p is now: ", KernelEntry);
	PrintSample2(p, 12);
	
	// pass KernelEntry to assembler funcs
	AsmKernelEntry = KernelEntry;
	
	return Status;
}

/** Fills every 8 bytes from 0x10.0000 - 0x40.0000 with jump to MyAsmJumpFromKernel32 (AsmFuncsX64).
  * This will then call KernelEntryPatchJumpBack.
  *
  * Note: This is a trick that covers different OSXes, but may not work in general.
  * We are assuming here that kernel entry is between 0x10.0000 - 0x40.0000 and
  * that it is 8-byte aligned. Kernel sources ususally have this entry
  * forced to be 4-byte aligned, but in practice it seems that they are 8-byte aligned.
  */
EFI_STATUS
KernelEntryPatchJumpFill(VOID)
{
	EFI_STATUS				Status = EFI_SUCCESS;
	UINT64					*Start = (UINT64*)(UINTN)0x100000;
	UINT8					*p;
	UINTN					Length =  0x300000;
	UINT64					JumpCode;
	UINTN					MyAsmJumpFromKernel32Addr;
	
	PRINT("KernelEntryPatchJumpFill: %p - %p\n", Start, Start + Length);
	
	// patch with 32 bit opcode for:
	//   mov ecx, MyAsmJumpFromKernel32
	//   call ecx	
	// = B9, <4 bytes address of MyAsmJumpFromKernel32>, FF, D1
	p = (UINT8*)Start;
	p[0] = 0xB9;
	MyAsmJumpFromKernel32Addr = (UINTN)MyAsmJumpFromKernel32;
	CopyMem((VOID *) (p + 1), (VOID *)&MyAsmJumpFromKernel32Addr, 4);
	p[5] = 0xFF; p[6] = 0xD1;
	
	// pick up jump code in JumpCode and spread it around
	JumpCode = *Start;
	SetMem64(Start, Length, JumpCode); 
	
	return Status;
}

/** Patches kernel entry point HLT - used for testing to cause system halt. */
EFI_STATUS
KernelEntryPatchHalt(UINT32 KernelEntry)
{
	EFI_STATUS				Status;
	unsigned char			*p;
	
	Status = EFI_SUCCESS;
	
	PRINT("KernelEntryPatchHalt KernelEntry (reloc): %lx (%lx)", KernelEntry, KernelEntry + gRelocBase);
	p = (UINT8 *)(UINTN) KernelEntry;
	*p= 0xf4; // HLT instruction
	PrintSample2(p, 4);
	PRINT("\n");
	
	return Status;
}

/** Patches kernel entry point with zeros - used for testing to cause restart. */
EFI_STATUS
KernelEntryPatchZero (UINT32 KernelEntry)
{
	EFI_STATUS				Status;
	unsigned char			*p;
	
	Status = EFI_SUCCESS;
	
	PRINT("KernelEntryPatchZero KernelEntry (reloc): %lx (%lx)", KernelEntry, KernelEntry + gRelocBase);
	p = (UINT8 *)(UINTN) KernelEntry;
	//*p= 0xf4;
	p[0]= 0; p[1]= 0; p[2]= 0; p[3]= 0; // invalid instruction
	PrintSample2(p, 4);
	PRINT("\n");
	
	return Status;
}


/** Assignes virtual addresses to runtime areas in memory map
  * and adds virtual to physical mapping to system's pagemap
  * because calling of SetVirtualAddressMap() does not work
  * on Aptio without that.
  */
VOID
AssignVirtualAddressesToMemMap(VOID *pBootArgs)
{
	BootArgs1		*BA1 = pBootArgs;
	BootArgs2		*BA2 = pBootArgs;
	
	UINTN					MemoryMapSize;
	EFI_MEMORY_DESCRIPTOR	*MemoryMap;
	UINTN					DescriptorSize;
	EFI_PHYSICAL_ADDRESS	KernelRTBlock;
	
	UINTN					NumEntries;
	UINTN					Index;
	EFI_MEMORY_DESCRIPTOR	*Desc;
	UINTN					BlockSize;
	PAGE_MAP_AND_DIRECTORY_POINTER	*PageTable;
	UINTN					Flags;
	EFI_STATUS				Status;
	
	if (BA1->Version == kBootArgsVersion1) {
		// pre Lion
		MemoryMapSize = BA1->MemoryMapSize;
		MemoryMap = (EFI_MEMORY_DESCRIPTOR*)(UINTN)BA1->MemoryMap;
		DescriptorSize = BA1->MemoryMapDescriptorSize;
		KernelRTBlock = EFI_PAGES_TO_SIZE(BA1->efiRuntimeServicesPageStart);
	} else {
		// Lion and up
		MemoryMapSize = BA2->MemoryMapSize;
		MemoryMap = (EFI_MEMORY_DESCRIPTOR*)(UINTN)BA2->MemoryMap;
		DescriptorSize = BA2->MemoryMapDescriptorSize;
		KernelRTBlock = EFI_PAGES_TO_SIZE(BA2->efiRuntimeServicesPageStart);
	}
	
	Desc = MemoryMap;
	NumEntries = MemoryMapSize / DescriptorSize;
	PRINT("AssignVirtualAddressesToMemMap: Size=%d, Addr=%p, DescSize=%d\n", MemoryMapSize, MemoryMap, DescriptorSize);

	// get current VM page table
	GetCurrentPageTable(&PageTable, &Flags);
	
	for (Index = 0; Index < NumEntries; Index++) {
		// assign virtual addresses to all EFI_MEMORY_RUNTIME marked pages (including MMIO)
		if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0) {
			BlockSize = EFI_PAGES_TO_SIZE(Desc->NumberOfPages);
			if (Desc->Type == EfiRuntimeServicesCode || Desc->Type == EfiRuntimeServicesData) {
				// for RT block - assign from kernel block
				Desc->VirtualStart = KernelRTBlock + 0xffffff8000000000;
				// map RT area virtual addresses - SetVirtualAddresMap on Ami Aptio does not work without this
				PRINT("Adding mapping 0x%x pages: VA %lx => PH %lx ", Desc->NumberOfPages, Desc->VirtualStart, Desc->PhysicalStart);
				Status = VmMapVirtualPages(PageTable, Desc->VirtualStart, Desc->NumberOfPages, Desc->PhysicalStart);
				PRINT("%r\n", Status);
				// next kernel block
				KernelRTBlock += BlockSize;
			} else {
				// for MMIO block - assign from kernel block
				Desc->VirtualStart = KernelRTBlock + 0xffffff8000000000;
				PRINT("Adding mapping 0x%x pages: VA %lx => PH %lx ", Desc->NumberOfPages, Desc->VirtualStart, Desc->PhysicalStart);
				Status = VmMapVirtualPages(PageTable, Desc->VirtualStart, Desc->NumberOfPages, Desc->PhysicalStart);
				PRINT("%r\n", Status);
				// next kernel block
				KernelRTBlock += BlockSize;
			}
			PRINT("=> 0x%lx -> 0x%lx\n", Desc->PhysicalStart, Desc->VirtualStart);
		}
		Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
		//WaitForKeyPress(L"End: press a key to continue\n");
	}
	VmFlashCaches();
	//WaitForKeyPress(L"End: press a key to continue\n");
}

/** Copies RT code and data blocks to reserved area inside kernel boot image. */
VOID
DefragmentRuntimeServices(VOID *pBootArgs)
{
	BootArgs1		*BA1 = pBootArgs;
	BootArgs2		*BA2 = pBootArgs;
	
	UINTN					NumEntries;
	UINTN					Index;
	EFI_MEMORY_DESCRIPTOR	*Desc;
	UINTN					DescriptorSize;
	UINT8					*KernelRTBlock;
	UINTN					BlockSize;
	UINT32					*efiSystemTable;
	
	if (BA1->Version == kBootArgsVersion1) {
		// pre Lion
		efiSystemTable = &BA1->efiSystemTable;
		Desc = (EFI_MEMORY_DESCRIPTOR*)(UINTN)BA1->MemoryMap;
		DescriptorSize = BA1->MemoryMapDescriptorSize;
		NumEntries = BA1->MemoryMapSize / BA1->MemoryMapDescriptorSize;
	} else {
		// Lion and up
		efiSystemTable = &BA2->efiSystemTable;
		Desc = (EFI_MEMORY_DESCRIPTOR*)(UINTN)BA2->MemoryMap;
		DescriptorSize = BA2->MemoryMapDescriptorSize;
		NumEntries = BA2->MemoryMapSize / BA2->MemoryMapDescriptorSize;
	}
	
	PRINT("DefragmentRuntimeServices: pBootArgs->efiSystemTable = %x\n", *efiSystemTable);
	
	for (Index = 0; Index < NumEntries; Index++) {
		// defragment only RT blocks
		if (Desc->Type == EfiRuntimeServicesCode || Desc->Type == EfiRuntimeServicesData) {
			// phisycal addr from virtual
			KernelRTBlock = (UINT8*)(UINTN)(Desc->VirtualStart & 0x7FFFFFFFFF);

			BlockSize = EFI_PAGES_TO_SIZE(Desc->NumberOfPages);
			
			PRINT("-Copy %p <- %p, size=0x%lx\n", KernelRTBlock + gRelocBase, (VOID*)(UINTN)Desc->PhysicalStart, BlockSize);
			CopyMem(KernelRTBlock + gRelocBase, (VOID*)(UINTN)Desc->PhysicalStart, BlockSize);
						
			if (Desc->PhysicalStart <= *efiSystemTable &&  *efiSystemTable < (Desc->PhysicalStart + BlockSize)) {
				// block contains sys table - update bootArgs with new address
				*efiSystemTable = (UINT32)((UINTN)KernelRTBlock + (*efiSystemTable - Desc->PhysicalStart));
				PRINT("new pBootArgs->efiSystemTable = %x\n", *efiSystemTable);
			}
			
			// mark old RT block in MemMap as free mem
			Desc->Type = EfiConventionalMemory;
			Desc->Attribute = Desc->Attribute & (~EFI_MEMORY_RUNTIME);
		}
		Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
	}
	WaitForKeyPress(L"END press a key to continue\n");

	// no need to further fix mem map - OSX is fine with RT stuff reported only in bootArgs
}

/** Fixes RT vars in bootArgs, virtualizes and defragments RT blocks. */
VOID
RuntimeServicesFix(VOID *pBootArgs)
{
	BootArgs1				*BA1 = pBootArgs;
	BootArgs2				*BA2 = pBootArgs;
	
	EFI_STATUS				Status;
	UINT32					gRelocBasePage = (UINT32)EFI_SIZE_TO_PAGES(gRelocBase);
	
	UINTN					MemoryMapSize;
	EFI_MEMORY_DESCRIPTOR	*MemoryMap;
	UINTN					DescriptorSize;
	UINT32					DescriptorVersion;
	
	UINT32					*efiRuntimeServicesPageStart;
	UINT32					efiRuntimeServicesPageCount;
	UINT64					*efiRuntimeServicesVirtualPageStart;
	
	if (BA1->Version == kBootArgsVersion1) {
		// pre Lion
		efiRuntimeServicesPageStart = &BA1->efiRuntimeServicesPageStart;
		efiRuntimeServicesPageCount = BA1->efiRuntimeServicesPageCount;
		efiRuntimeServicesVirtualPageStart = &BA1->efiRuntimeServicesVirtualPageStart;
		
		MemoryMapSize = BA1->MemoryMapSize;
		MemoryMap = (EFI_MEMORY_DESCRIPTOR*)(UINTN)BA1->MemoryMap;
		DescriptorSize = BA1->MemoryMapDescriptorSize;
		DescriptorVersion = BA1->MemoryMapDescriptorVersion;
	} else {
		// Lion and up
		efiRuntimeServicesPageStart = &BA2->efiRuntimeServicesPageStart;
		efiRuntimeServicesPageCount = BA2->efiRuntimeServicesPageCount;
		efiRuntimeServicesVirtualPageStart = &BA2->efiRuntimeServicesVirtualPageStart;
		
		MemoryMapSize = BA2->MemoryMapSize;
		MemoryMap = (EFI_MEMORY_DESCRIPTOR*)(UINTN)BA2->MemoryMap;
		DescriptorSize = BA2->MemoryMapDescriptorSize;
		DescriptorVersion = BA2->MemoryMapDescriptorVersion;
	}
	
	PRINT("RuntimeServicesFix: efiRSPageStart=%x, efiRSPageCount=%x, efiRSVirtualPageStart=%lx\n",
		*efiRuntimeServicesPageStart, efiRuntimeServicesPageCount, *efiRuntimeServicesVirtualPageStart);
	// fix runtime entries
	*efiRuntimeServicesPageStart -= gRelocBasePage;
	// VirtualPageStart is ok in boot args (a miracle!), but we'll do it anyway
	*efiRuntimeServicesVirtualPageStart = 0x000ffffff8000000 + *efiRuntimeServicesPageStart;
	PRINT("RuntimeServicesFix: efiRSPageStart=%x, efiRSPageCount=%x, efiRSVirtualPageStart=%lx\n",
		*efiRuntimeServicesPageStart, efiRuntimeServicesPageCount, *efiRuntimeServicesVirtualPageStart);
	
	// assign virtual addresses
	AssignVirtualAddressesToMemMap(pBootArgs);
	
	PrintMemMap(MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
	// virtualize them
	Status = gRT->SetVirtualAddressMap(MemoryMapSize, DescriptorSize, DescriptorVersion, MemoryMap);
	
	PRINT("SetVirtualAddressMap() = Status: %r\n", Status);
	if (EFI_ERROR (Status)) {
		CpuDeadLoop();
	}
	
	// and defragment
	DefragmentRuntimeServices(pBootArgs);
}

/** DevTree contains /chosen/memory-map with properties with 8 byte values
 * (DTMemMapEntry: UINT32 Address, UINT32 Length):
 * "name" = this is exception - not DTMemMapEntry
 * "BootCLUT" = 8bit boot time colour lookup table
 * "Pict-FailedBoot" = picture shown if booting fails
 * "RAMDisk" = ramdisk
 * "Driver-<hex addr of BooterKextFileInfo>" = Kext, UINT32 Address points to BooterKextFileInfo
 * "DriversPackage-..." = MKext, UINT32 Address points to mkext_header (libkern/libkern/mkext.h), UINT32 length
 *
 * We are fixing here DTMemMapEntry.Address for all those entries.
 * Plus, for every loaded kext, Address points to BooterKextFileInfo,
 * and we are fixing it's pointers also.
*/
VOID
DevTreeFix(VOID *pBootArgs)
{
	BootArgs1			*BA1 = pBootArgs;
	BootArgs2			*BA2 = pBootArgs;
	
	DTEntry				DevTree;
	DTEntry				MemMap;
	struct OpaqueDTPropertyIterator OPropIter;
	DTPropertyIterator	PropIter = &OPropIter;
	CHAR8				*PropName;
	DTMemMapEntry		*PropValue;
	BooterKextFileInfo	*KextInfo;


	if (BA1->Version == kBootArgsVersion1) {
		// pre Lion
		DevTree = (DTEntry)(UINTN)BA1->deviceTreeP;
	} else {
		// Lion and up
		DevTree = (DTEntry)(UINTN)BA2->deviceTreeP;
	}
	PRINT("Fixing DevTree at %p\n", DevTree);
	DTInit(DevTree);
	if (DTLookupEntry(NULL, "/chosen/memory-map", &MemMap) == kSuccess) {
		PRINT("Found /chosen/memory-map\n");
		if (DTCreatePropertyIteratorNoAlloc(MemMap, PropIter) == kSuccess) {
			PRINT("DTCreatePropertyIterator OK\n");
			while (DTIterateProperties(PropIter, &PropName) == kSuccess) {
				PRINT("= %a, val len=%d: ", PropName, PropIter->currentProperty->length);
				// all /chosen/memory-map props have DTMemMapEntry (address, length)
				// values. we need to correct the address
				
				// basic check that value is 2 * UINT32
				if (PropIter->currentProperty->length != 2 * sizeof(UINT32)) {
					// not DTMemMapEntry, usually "name" property
					PRINT("NOT DTMemMapEntry\n");
					continue;
				}
				
				// get value (Address and Length)
				PropValue = (DTMemMapEntry*)(((UINT8*)PropIter->currentProperty) + sizeof(DeviceTreeNodeProperty));
				PRINT("MM Addr = %x, Len = %x ", PropValue->Address, PropValue->Length);
				
				// second check - Address is in our reloc block
				if ((PropValue->Address < gRelocBase + kaddr)
					|| (PropValue->Address >= gRelocBase + kaddr + ksize))
				{
					PRINT("DTMemMapEntry->Address is not in reloc block, skipping\n");
					continue;
				}
				
				// check if this is Kext entry
				if (AsciiStrnCmp(PropName, BOOTER_KEXT_PREFIX, AsciiStrLen(BOOTER_KEXT_PREFIX)) == 0) {
					// yes - fix kext pointers
					KextInfo = (BooterKextFileInfo*)(UINTN)PropValue->Address;
					PRINT(" = KEXT %a at %x ", (CHAR8*)(UINTN)KextInfo->bundlePathPhysAddr, KextInfo->infoDictPhysAddr);
					KextInfo->infoDictPhysAddr -= (UINT32)gRelocBase;
					KextInfo->executablePhysAddr -= (UINT32)gRelocBase;
					KextInfo->bundlePathPhysAddr -= (UINT32)gRelocBase;
					PRINT("-> %x ", KextInfo->infoDictPhysAddr);
				}
				
				// fix address in mem map entry
				PropValue->Address -= (UINT32)gRelocBase;
				PRINT("=> Fixed MM Addr = %x\n", PropValue->Address);
			}
		}
	}
	
}

/** Callback called when boot.efi jumps to kernel. */
UINTN
EFIAPI
KernelEntryPatchJumpBack(UINTN bootArgs)
{
	VOID 				*pBootArgs = (VOID*)bootArgs;
	BootArgs1			*BA1 = pBootArgs;
	BootArgs2			*BA2 = pBootArgs;
	
	PRINT("BACK FROM KERNEL: BootArgs = %x, KernelEntry: %x\n", bootArgs, AsmKernelEntry);
	
	if (gRelocBase > 0) {

		if (BA1->Version == kBootArgsVersion1) {
			// pre Lion
			kaddr = BA1->kaddr - (UINT32)gRelocBase;
			ksize = BA1->ksize;
		} else {
			// Lion and up
			kaddr = BA2->kaddr - (UINT32)gRelocBase;
			ksize = BA2->ksize;
		}

		
		// fix runtime stuff
		RuntimeServicesFix(pBootArgs);
		
		// fix some values in dev tree
		DevTreeFix(pBootArgs);
		
		// fix boot args
		//BootArgsPrint(pBootArgs);
		BootArgsFix(pBootArgs, gRelocBase);
		//BootArgsPrint(pBootArgs);
		
		
		// and finally copy kernel boot image to a proper place
		PRINT("CopyMem %p <= %p (%x)\n",
			(VOID *)(UINTN)kaddr,
			(VOID *)(UINTN)(gRelocBase + kaddr),
			ksize);
		CopyMem((VOID *)(UINTN)kaddr,
				(VOID *)(UINTN)(gRelocBase + kaddr),
				ksize);
		bootArgs = bootArgs - gRelocBase;
	}
	PRINT("BACK TO KERNEL: BootArgs = %x\n", bootArgs);
	
	return bootArgs;
}



/**

  Methods for finding, checking and fixing boot args

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>

#include "BootArgs.h"
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


VOID
EFIAPI
BootArgsPrint(VOID *bootArgs)
{

#if (DBG_TO > 0) || (DEBUG_TO_NVRAM > 0)
	
	BootArgs1		*BA1 = bootArgs;
	BootArgs2		*BA2 = bootArgs;
	
	if (BA1->Version == kBootArgsVersion1) {
		
		// pre Lion
		DBG("BootArgs at 0x%p\n", BA1);
		DBGnvr("BootArgs at 0x%p\n", BA1);
		
		DBG(" Revision: 0x%x, Version: 0x%x, efiMode: 0x%x (%d), res10: %x, res11: %x, res12: %x, res20: %x\n",
			BA1->Revision, BA1->Version, BA1->efiMode, BA1->efiMode,
			BA1->__reserved1[0], BA1->__reserved1[1], BA1->__reserved1[2],
			BA1->__reserved2[0]
			);
		DBGnvr(" Revision: 0x%x, Version: 0x%x, efiMode: 0x%x (%d), res10: %x, res11: %x, res12: %x, res20: %x\n",
			   BA1->Revision, BA1->Version, BA1->efiMode, BA1->efiMode,
			   BA1->__reserved1[0], BA1->__reserved1[1], BA1->__reserved1[2],
			   BA1->__reserved2[0]
			   );
		
		DBG(" CommandLine: %a\n", BA1->CommandLine);
		DBGnvr(" CommandLine: %a\n", BA1->CommandLine);
		
		DBG(" MemoryMap: 0x%x, MMSize: 0x%x, MMDescSize: 0x%x, MMDescVersion: 0x%x\n",
			BA1->MemoryMap, BA1->MemoryMapSize, BA1->MemoryMapDescriptorSize, BA1->MemoryMapDescriptorVersion
			);
		DBGnvr(" MemoryMap: 0x%x, MMSize: 0x%x, MMDescSize: 0x%x, MMDescVersion: 0x%x\n",
			BA1->MemoryMap, BA1->MemoryMapSize, BA1->MemoryMapDescriptorSize, BA1->MemoryMapDescriptorVersion
			);
		
		DBG(" Boot_Video v_baseAddr: 0x%x, v_display: 0x%x, v_rowBytes: 0x%x, v_width: %d, v_height: %d, v_depth: %d\n",
			BA1->Video.v_baseAddr, BA1->Video.v_display,
			BA1->Video.v_rowBytes, BA1->Video.v_width, BA1->Video.v_height, BA1->Video.v_depth
			);
		DBGnvr(" Boot_Video v_baseAddr: 0x%x, v_display: 0x%x, v_rowBytes: 0x%x, v_width: %d, v_height: %d, v_depth: %d\n",
			BA1->Video.v_baseAddr, BA1->Video.v_display,
			BA1->Video.v_rowBytes, BA1->Video.v_width, BA1->Video.v_height, BA1->Video.v_depth
			);
		
		DBG(" deviceTreeP: 0x%x, deviceTreeLength: 0x%x, kaddr: 0x%x, ksize: 0x%x\n",
			BA1->deviceTreeP, BA1->deviceTreeLength, BA1->kaddr, BA1->ksize
			);
		DBGnvr(" deviceTreeP: 0x%x, deviceTreeLength: 0x%x, kaddr: 0x%x, ksize: 0x%x\n",
			BA1->deviceTreeP, BA1->deviceTreeLength, BA1->kaddr, BA1->ksize
			);
		
		DBG(" efiRTServPgStart: 0x%x, efiRTServPgCount: 0x%x, efiRTServVPgStart: 0x%lx\n",
			BA1->efiRuntimeServicesPageStart, BA1->efiRuntimeServicesPageCount, BA1->efiRuntimeServicesVirtualPageStart
			);
		DBGnvr(" efiRTServPgStart: 0x%x, efiRTServPgCount: 0x%x, efiRTServVPgStart: 0x%lx\n",
			BA1->efiRuntimeServicesPageStart, BA1->efiRuntimeServicesPageCount, BA1->efiRuntimeServicesVirtualPageStart
			);
		
		DBG(" efiSystemTable: 0x%x, res2: 0x%x, perfDataStart: 0x%x, perfDataSize: 0x%x\n",
			BA1->efiSystemTable, BA1->__reserved2, BA1->performanceDataStart, BA1->performanceDataSize
			);
		DBGnvr(" efiSystemTable: 0x%x, res2: 0x%x, perfDataStart: 0x%x, perfDataSize: 0x%x\n",
			BA1->efiSystemTable, BA1->__reserved2, BA1->performanceDataStart, BA1->performanceDataSize
			);
		
		DBG(" res30: %x, res31: %x\n", BA1->__reserved3[0], BA1->__reserved3[1]);
		DBGnvr(" res30: %x, res31: %x\n", BA1->__reserved3[0], BA1->__reserved3[1]);
		
	} else {
		// Lion and up
		DBG("BootArgs at 0x%p\n", BA2);
		DBGnvr("BootArgs at 0x%p\n", BA2);
		
		DBG(" Revision: 0x%x, Version: 0x%x, efiMode: 0x%x (%d), debugMode: 0x%x, flags: 0x%x\n",
			BA2->Revision, BA2->Version, BA2->efiMode, BA2->efiMode, BA2->debugMode, BA2->flags
			);
		DBGnvr(" Revision: 0x%x, Version: 0x%x, efiMode: 0x%x (%d), debugMode: 0x%x, flags: 0x%x\n",
			BA2->Revision, BA2->Version, BA2->efiMode, BA2->efiMode, BA2->debugMode, BA2->flags
			);
		
		DBG(" CommandLine: %a\n", BA2->CommandLine);
		DBGnvr(" CommandLine: %a\n", BA2->CommandLine);
		
		DBG(" MemoryMap: 0x%x, MMSize: 0x%x, MMDescSize: 0x%x, MMDescVersion: 0x%x\n",
			BA2->MemoryMap, BA2->MemoryMapSize, BA2->MemoryMapDescriptorSize, BA2->MemoryMapDescriptorVersion
			);
		DBGnvr(" MemoryMap: 0x%x, MMSize: 0x%x, MMDescSize: 0x%x, MMDescVersion: 0x%x\n",
			BA2->MemoryMap, BA2->MemoryMapSize, BA2->MemoryMapDescriptorSize, BA2->MemoryMapDescriptorVersion
			);
		
		DBG(" Boot_Video v_baseAddr: 0x%x, v_display: 0x%x, v_rowBytes: 0x%x, v_width: %d, v_height: %d, v_depth: %d\n",
			BA2->Video.v_baseAddr, BA2->Video.v_display,
			BA2->Video.v_rowBytes, BA2->Video.v_width, BA2->Video.v_height, BA2->Video.v_depth
			);
		DBGnvr(" Boot_Video v_baseAddr: 0x%x, v_display: 0x%x, v_rowBytes: 0x%x, v_width: %d, v_height: %d, v_depth: %d\n",
			BA2->Video.v_baseAddr, BA2->Video.v_display,
			BA2->Video.v_rowBytes, BA2->Video.v_width, BA2->Video.v_height, BA2->Video.v_depth
			);
		
		DBG(" deviceTreeP: 0x%x, deviceTreeLength: 0x%x, kaddr: 0x%x, ksize: 0x%x\n",
			BA2->deviceTreeP, BA2->deviceTreeLength, BA2->kaddr, BA2->ksize
			);
		DBGnvr(" deviceTreeP: 0x%x, deviceTreeLength: 0x%x, kaddr: 0x%x, ksize: 0x%x\n",
			BA2->deviceTreeP, BA2->deviceTreeLength, BA2->kaddr, BA2->ksize
			);
		
		DBG(" efiRTServPgStart: 0x%x, efiRTServPgCount: 0x%x, efiRTServVPgStart: 0x%lx\n",
			BA2->efiRuntimeServicesPageStart, BA2->efiRuntimeServicesPageCount, BA2->efiRuntimeServicesVirtualPageStart
			);
		DBGnvr(" efiRTServPgStart: 0x%x, efiRTServPgCount: 0x%x, efiRTServVPgStart: 0x%lx\n",
			BA2->efiRuntimeServicesPageStart, BA2->efiRuntimeServicesPageCount, BA2->efiRuntimeServicesVirtualPageStart
			);
		
		DBG(" efiSystemTable: 0x%x, kslide: 0x%x, perfDataStart: 0x%x, perfDataSize: 0x%x\n",
			BA2->efiSystemTable, BA2->kslide, BA2->performanceDataStart, BA2->performanceDataSize
			);
		DBGnvr(" efiSystemTable: 0x%x, kslide: 0x%x, perfDataStart: 0x%x, perfDataSize: 0x%x\n",
			BA2->efiSystemTable, BA2->kslide, BA2->performanceDataStart, BA2->performanceDataSize
			);
		
		DBG(" keyStDtStart: 0x%x, keyStDtSize: 0x%x, bootMemStart: 0x%lx, bootMemSize: 0x%lx\n",
			BA2->keyStoreDataStart, BA2->keyStoreDataSize, BA2->bootMemStart, BA2->bootMemSize
			);
		DBGnvr(" keyStDtStart: 0x%x, keyStDtSize: 0x%x, bootMemStart: 0x%lx, bootMemSize: 0x%lx\n",
			BA2->keyStoreDataStart, BA2->keyStoreDataSize, BA2->bootMemStart, BA2->bootMemSize
			);
		
		DBG(" PhysicalMemorySize: 0x%lx (%d GB), FSBFrequency: 0x%lx (%d MHz)\n",
			BA2->PhysicalMemorySize, BA2->PhysicalMemorySize / (1024 * 1024 * 1024),
			BA2->FSBFrequency, BA2->FSBFrequency / 1000000
			);
		DBGnvr(" PhysicalMemorySize: 0x%lx (%d GB), FSBFrequency: 0x%lx (%d MHz)\n",
			BA2->PhysicalMemorySize, BA2->PhysicalMemorySize / (1024 * 1024 * 1024),
			BA2->FSBFrequency, BA2->FSBFrequency / 1000000
			);
		
		DBG(" pciConfigSpaceBaseAddress: 0x%lx, pciConfigSpaceStartBusNumber: 0x%x, pciConfigSpaceEndBusNumber: 0x%x\n",
			BA2->pciConfigSpaceBaseAddress, BA2->pciConfigSpaceStartBusNumber, BA2->pciConfigSpaceEndBusNumber
			);
		DBGnvr(" pciConfigSpaceBaseAddress: 0x%lx, pciConfigSpaceStartBusNumber: 0x%x, pciConfigSpaceEndBusNumber: 0x%x\n",
			BA2->pciConfigSpaceBaseAddress, BA2->pciConfigSpaceStartBusNumber, BA2->pciConfigSpaceEndBusNumber
			);
	}
#endif
}

BootArgs gBootArgs;

BootArgs*
EFIAPI
GetBootArgs(VOID *bootArgs)
{
	BootArgs1		*BA1 = bootArgs;
	BootArgs2		*BA2 = bootArgs;
	
    ZeroMem(&gBootArgs, sizeof(gBootArgs));
    
	if (BA1->Version == kBootArgsVersion1) {
		// pre Lion
        gBootArgs.MemoryMap = &BA1->MemoryMap;
        gBootArgs.MemoryMapSize = &BA1->MemoryMapSize;
        gBootArgs.MemoryMapDescriptorSize = &BA1->MemoryMapDescriptorSize;
        gBootArgs.MemoryMapDescriptorVersion = &BA1->MemoryMapDescriptorVersion;
        
        gBootArgs.deviceTreeP = &BA1->deviceTreeP;
        gBootArgs.deviceTreeLength = &BA1->deviceTreeLength;
        
        gBootArgs.kaddr = &BA1->kaddr;
        gBootArgs.ksize = &BA1->ksize;
        
        gBootArgs.efiRuntimeServicesPageStart = &BA1->efiRuntimeServicesPageStart;
        gBootArgs.efiRuntimeServicesPageCount = &BA1->efiRuntimeServicesPageCount;
        gBootArgs.efiRuntimeServicesVirtualPageStart = &BA1->efiRuntimeServicesVirtualPageStart;
        gBootArgs.efiSystemTable = &BA1->efiSystemTable;
	} else {
		// Lion and up
        gBootArgs.MemoryMap = &BA2->MemoryMap;
        gBootArgs.MemoryMapSize = &BA2->MemoryMapSize;
        gBootArgs.MemoryMapDescriptorSize = &BA2->MemoryMapDescriptorSize;
        gBootArgs.MemoryMapDescriptorVersion = &BA2->MemoryMapDescriptorVersion;
        
        gBootArgs.deviceTreeP = &BA2->deviceTreeP;
        gBootArgs.deviceTreeLength = &BA2->deviceTreeLength;
        
        gBootArgs.kaddr = &BA2->kaddr;
        gBootArgs.ksize = &BA2->ksize;
        
        gBootArgs.efiRuntimeServicesPageStart = &BA2->efiRuntimeServicesPageStart;
        gBootArgs.efiRuntimeServicesPageCount = &BA2->efiRuntimeServicesPageCount;
        gBootArgs.efiRuntimeServicesVirtualPageStart = &BA2->efiRuntimeServicesVirtualPageStart;
        gBootArgs.efiSystemTable = &BA2->efiSystemTable;
	}
    
    return &gBootArgs;
}


VOID
EFIAPI
BootArgsFix(BootArgs *BA, EFI_PHYSICAL_ADDRESS gRellocBase)
{
    *BA->MemoryMap = *BA->MemoryMap - (UINT32)gRellocBase;
    *BA->deviceTreeP = *BA->deviceTreeP - (UINT32)gRellocBase;
    *BA->kaddr = *BA->kaddr - (UINT32)gRellocBase;
}

/** Searches for bootArgs from Start and returns pointer to bootArgs or ... does not return if can not be found.  **/
VOID *
EFIAPI
BootArgsFind(IN EFI_PHYSICAL_ADDRESS Start)
{
	UINT8			*ptr;
	UINT8			archMode = sizeof(UINTN) * 8;
	BootArgs1		*BA1;
	BootArgs2		*BA2;
	
	// start searching from 0x200000.
	ptr = (UINT8*)(UINTN)Start;
	
	
	while(TRUE) {
		
		// check bootargs for 10.7 and up
		BA2 = (BootArgs2*)ptr;
		
		if (BA2->Version==2 && BA2->Revision==0
			// plus additional checks - some values are not inited by boot.efi yet
			&& BA2->efiMode == archMode
			&& BA2->kaddr == 0 && BA2->ksize == 0
			&& BA2->efiSystemTable == 0
			)
		{
			break;
		}
		
		// check bootargs for 10.4 - 10.6.x
		BA1 = (BootArgs1*)ptr;
		
		if (BA1->Version==1
			&& (BA1->Revision==6 || BA1->Revision==5 || BA1->Revision==4)
			// plus additional checks - some values are not inited by boot.efi yet
			&& BA1->efiMode == archMode
			&& BA1->kaddr == 0 && BA1->ksize == 0
			&& BA1->efiSystemTable == 0
			)
		{
			break;
		}
		
		ptr += 0x1000;
	}
	
	DBG("Found bootArgs2 at %p\n", ptr);
	return ptr;
}



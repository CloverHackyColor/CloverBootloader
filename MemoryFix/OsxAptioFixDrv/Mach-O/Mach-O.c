/**

  Basic functions for parsing Mach-O kernel.
  
  by dmazar

**/

//#include <IndustryStandard/MachO-loader.h>
#include <UefiLoader.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

//#include "UefiLoader.h"
#include "Mach-O.h"


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


/** Adds Offset bytes to SourcePtr and returns new pointer as ReturnType. */
#define PTR_OFFSET(SourcePtr, Offset, ReturnType)	((ReturnType)(((UINT8*)SourcePtr) + Offset))


/** Returns Mach-O entry point from LC_UNIXTHREAD loader command. */
UINTN
EFIAPI
MachOGetEntryAddress(IN VOID *MachOImage)
{
	struct mach_header		*MHdr;
	struct mach_header_64	*MHdr64;
	BOOLEAN					Is64Bit;
	UINT32					NCmds;
	struct load_command		*LCmd;
	UINTN					Index;
	i386_thread_state_t		*ThreadState;
	x86_thread_state64_t	*ThreadState64;
	UINTN					Address;
	
	
	Address = 0;
	MHdr = (struct mach_header *)MachOImage;
	MHdr64 = (struct mach_header_64 *)MachOImage;
	DBG("MachOImage: %p, magic: %x", MachOImage, MHdr->magic);
	
	if (MHdr->magic == MH_MAGIC || MHdr->magic == MH_CIGAM) {
		// 32 bit header
		DBG(" -> 32 bit\n");
		Is64Bit = FALSE;
		NCmds = MHdr->ncmds;
		LCmd = PTR_OFFSET(MachOImage, sizeof(struct mach_header), struct load_command *);
	} else if (MHdr64->magic == MH_MAGIC_64 || MHdr64->magic == MH_CIGAM_64) {
		// 64 bit header
		DBG(" -> 64 bit\n");
		Is64Bit = TRUE;
		NCmds = MHdr64->ncmds;
		LCmd = PTR_OFFSET(MachOImage, sizeof(struct mach_header_64), struct load_command *);
	} else {
		// invalid MachOImage
		return Address;
	}
	DBG("ncmds: %d\n", NCmds, LCmd);
	//gBS->Stall(10 * 1000000);
	
	// iterate over load commands
	for (Index = 0; Index < NCmds; Index++) {
		
		DBG("%d. LCmd: %p, cmd: %x, size: %d\n", Index, LCmd, LCmd->cmd, LCmd->cmdsize);
		
		if (LCmd->cmd == LC_UNIXTHREAD) {
			
			DBG("LC_UNIXTHREAD\n");
			//
			// extract thread state
			// LCmd =
			//  struct load_command {
			//   uint32_t cmd
			//   uint32_t cmdsize
			//  }
			// 	uint32_t flavor		   flavor of thread state */
			//  uint32_t count		   count of longs in thread state */
			//  struct XXX_thread_state state   thread state for this flavor */
			//
			ThreadState = PTR_OFFSET(LCmd, sizeof(struct load_command) + 2 * sizeof(UINT32), i386_thread_state_t *);
			ThreadState64 = (x86_thread_state64_t *)ThreadState;
			
			if (Is64Bit) {
				Address = (UINTN)ThreadState64->rip;
			} else {
				Address = (UINTN)ThreadState->eip;
			}
			break;
		}
		
		// next command
		LCmd = PTR_OFFSET(LCmd, LCmd->cmdsize, struct load_command *);
		
	}
	
	DBG("Address: %lx\n", Address);
	//gBS->Stall(20 * 1000000);
	return Address;
}

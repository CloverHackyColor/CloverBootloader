/**

  Some assembler helper functions plus boot.efi kernel jump callback

  by dmazar

**/

#include <Library/BaseLib.h>

/** Read stack pointer. */
UINT64 EFIAPI MyAsmReadSp(VOID);

/** Save 64 bit state that will be restored on callback. */
VOID EFIAPI MyAsmPrepareJumpFromKernel(VOID);

/** 32 bit callback function. */
VOID EFIAPI MyAsmJumpFromKernel32(VOID);

/** 32 bit function start and end that copies kernel to proper mem and jumps to kernel. */
extern UINT8 MyAsmCopyAndJumpToKernel32;
extern UINT8 MyAsmCopyAndJumpToKernel32End;

extern	UINTN			SavedCR3;
extern	IA32_DESCRIPTOR	SavedGDTR;
extern	IA32_DESCRIPTOR	SavedIDTR;

extern	UINT32			AsmKernelEntry;
extern	UINT32			AsmKernelImageStartReloc;
extern	UINT32			AsmKernelImageStart;
extern	UINT32			AsmKernelImageSize;

extern	UINT32			MyAsmCopyAndJumpToKernel32Addr;

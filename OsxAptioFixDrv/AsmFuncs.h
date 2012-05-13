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

extern	UINTN			SavedCR3;
extern	IA32_DESCRIPTOR	SavedGDTR;
extern	IA32_DESCRIPTOR	SavedIDTR;
extern	UINT32			AsmKernelEntry;


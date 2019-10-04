/**

  Some assembler helper functions plus boot.efi kernel jump callback

  by dmazar

**/

#include <Library/BaseLib.h>

/** Read stack pointer. */
UINT64 EFIAPI MyAsmReadSp(VOID);

/** Save 64 bit state that will be restored on callback. */
VOID EFIAPI MyAsmPrepareJumpFromKernel(VOID);

/** Start and end address of the 32 and 64 bit code
 that is copied to kernel entry address to jump back
 to our code, to MyAsmJumpFromKernel().
 */
extern UINT8  MyAsmEntryPatchCode;
extern UINT8  MyAsmEntryPatchCodeEnd;

/** Callback function, 32 and 64 bit, that is called when boot.efi jumps to kernel address. */
VOID EFIAPI MyAsmJumpFromKernel(VOID);

/** 32 bit function start and end that copies kernel to proper mem and jumps to kernel. */
extern UINT8 MyAsmCopyAndJumpToKernel;
extern UINT8 MyAsmCopyAndJumpToKernel32;
extern UINT8 MyAsmCopyAndJumpToKernel64;
extern UINT8 MyAsmCopyAndJumpToKernelEnd;

extern	UINTN			SavedCR3;
extern	IA32_DESCRIPTOR	SavedGDTR;
extern	IA32_DESCRIPTOR	SavedIDTR;

extern	UINT64			AsmKernelEntry;
extern	UINT64			AsmKernelImageStartReloc;
extern	UINT64			AsmKernelImageStart;
extern	UINT64			AsmKernelImageSize;

extern	UINT64			MyAsmCopyAndJumpToKernel32Addr;
extern	UINT64			MyAsmCopyAndJumpToKernel64Addr;

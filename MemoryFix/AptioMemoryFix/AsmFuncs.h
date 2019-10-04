/**

  Some assembler helper functions plus boot.efi kernel jump callback

  by dmazar

**/

#ifndef APTIOFIX_ASM_FUNCS_H
#define APTIOFIX_ASM_FUNCS_H

#include <Library/BaseLib.h>

/** Save 64 bit state that will be restored on callback. */
VOID
EFIAPI
AsmPrepareJumpFromKernel (
  VOID
  );

/** Start and end address of the 32 and 64 bit code
 that is copied to kernel entry address to jump back
 to our code, to AsmJumpFromKernel().
 */
extern UINT8           EntryPatchCode;
extern UINT8           EntryPatchCodeEnd;

/** Callback function, 32 and 64 bit, that is called when boot.efi jumps to kernel address. */
VOID
EFIAPI
AsmJumpFromKernel (
  VOID
  );

/** 32 bit function start and end that copies kernel to proper mem and jumps to kernel. */
extern UINT8           JumpToKernel;
extern UINT8           JumpToKernel32;
extern UINT8           JumpToKernel64;
extern UINT8           JumpToKernelEnd;

extern UINTN           SavedCR3;
extern IA32_DESCRIPTOR SavedGDTR;
extern IA32_DESCRIPTOR SavedIDTR;

extern UINT64          AsmKernelEntry;
extern UINT64          JumpToKernel32Addr;
extern UINT64          JumpToKernel64Addr;

#endif // APTIOFIX_ASM_FUNCS_H

#ifndef __XTOOLSCOMMON_H__
#define __XTOOLSCOMMON_H__


#define xsize UINTN
#define MAX_XSIZE MAX_UINTN

extern xsize XArrayGrowByDefault;
extern xsize XBufferGrowByDefault;

/* For convience, operator [] is define with int parameter.
 * Defining __XTOOLS_INT_CHECK__ make a check that the parameter is >= 0
 */
#define __XTOOLS_INT_CHECK__

#ifdef CLOVER_BUILD

extern "C" {
#include <Library/BaseLib.h> // for CpuDeadLoop
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h> // for CopyMen
}

#endif

#define Xalloc(AllocationSize) AllocatePool(AllocationSize)
#define Xrealloc(OldSize, NewSize, OldBuffer) ReallocatePool(OldSize, NewSize, OldBuffer)
#define Xfree(Buffer) FreePool(Buffer)
#define Xmemmove(dest,source,count) CopyMem(dest, (void*)(source), count) // that has to handle overlapping memory (prefer memmove to memcpy).



// Declare here instead of include to avoid circular dependancy.
VOID
EFIAPI
DebugLog (
  IN        INTN  DebugMode,
  IN  CONST CHAR8 *FormatString, ...);


#endif

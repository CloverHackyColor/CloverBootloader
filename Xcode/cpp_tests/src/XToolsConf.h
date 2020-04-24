#ifndef __XTOOLSCOMMON_H__
#define __XTOOLSCOMMON_H__

#include <posix.h>


#define xsize size_t
//#define xisize INTN
#define MAX_XSIZE SIZE_T_MAX
//#define MAX_XISIZE MAX_INTN

extern xsize XArrayGrowByDefault;
extern xsize XBufferGrowByDefault;

/* For convience, operator [] is define with int parameter.
 * Defining __XTOOLS_INT_CHECK__ make a check that the parameter is >= 0
 */
#define __XTOOLS_INT_CHECK__

#include "../cpp_util/panic.h"

#define realloc(ptr, newsize, oldsize) realloc(ptr, newsize)


// Declare here instead of include to avoid circular dependency.

#ifdef _MSC_VER
#define __attribute__(x)
#endif

VOID
EFIAPI
DebugLog (
  IN        INTN  DebugMode,
  IN  CONST CHAR8 *FormatString, ...) __attribute__((format(printf, 2, 3)));;



#endif

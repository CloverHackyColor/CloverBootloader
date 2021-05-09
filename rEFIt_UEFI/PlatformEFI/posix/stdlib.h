#ifndef __CLOVER_STDLIB_H__
#define __CLOVER_STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <Library/MemoryAllocationLib.h>

#ifdef __cplusplus
}
#endif

#include "stddef.h" // for size_t

#ifdef __cplusplus
extern "C" {
#endif

void abort(void);

inline void* malloc(size_t size)
{
  return AllocatePool(size);
}

inline void* reallocWithOldSize(void *ptr, size_t newsize, size_t oldsize) // not the posix realloc. For EFI we need oldsize
{
  return ReallocatePool(oldsize, newsize, ptr);
}

inline void free(void *ptr)
{
  FreePool(ptr);
}

#ifdef __cplusplus
}
#endif


#endif

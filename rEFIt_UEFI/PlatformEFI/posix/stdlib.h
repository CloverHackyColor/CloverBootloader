#ifndef __CLOVER_STDLIB_H__
#define __CLOVER_STDLIB_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <Library/MemoryAllocationLib.h>
#include <Library/MemLogLib.h>

#ifdef __cplusplus
}
#endif

#include "stddef.h" // for size_t

#ifdef __cplusplus
extern "C" {
#endif


void abort(void); // abort() is defined in stdlib.h


#ifndef OPENSSL_SMALL_FOOTPRINT
// can't use these memory function with OpenSslLib

static inline __attribute__((always_inline)) void* malloc(size_t size)
{
  void* ptr = AllocatePool(size);
//  MemLogf(false, 0, "malloc(%zd) %llx\n", size, uintptr_t(ptr));
  return ptr;
}

static inline __attribute__((always_inline)) void* reallocWithOldSize(void *ptr, size_t newsize, size_t oldsize) // not the posix realloc. For EFI we need oldsize
{
  void* newptr = ReallocatePool(oldsize, newsize, ptr);
//  MemLogf(false, 0, "reallocWithOldSize(%llx %zd %zd) %llx\n", uintptr_t(ptr), newsize, oldsize, uintptr_t(newptr));
  return newptr;
}

static inline __attribute__((always_inline)) void free(void *ptr)
{
//  MemLogf(false, 0, "free(%llx)\n", uintptr_t(ptr));
  FreePool(ptr);
}

#endif



#ifdef __cplusplus
}
#endif


#endif

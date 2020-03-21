#ifndef __CLOVER_STDLIB_H__
#define __CLOVER_STDLIB_H__


extern "C" {
#include <Library/MemoryAllocationLib.h>
}

#include "stddef.h" // for size_t

void abort(void);

inline void* malloc(size_t size)
{
	return AllocatePool(size);
}

inline void* realloc(void *ptr, size_t newsize, size_t oldsize) // not the posix realloc. For EFI we need oldsize
{
	return ReallocatePool(oldsize, newsize, ptr);
}

inline void free(void *ptr)
{
	FreePool(ptr);
}

#endif

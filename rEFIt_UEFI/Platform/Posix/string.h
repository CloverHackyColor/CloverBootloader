#ifndef __CLOVER_STRING_H__
#define __CLOVER_STRING_H__

extern "C" {
#include <Library/BaseMemoryLib.h>
}

inline void* memmove(void *dst, const void *src, size_t len)
{
	return CopyMem(dst, (void*)(src), len);
}

#endif

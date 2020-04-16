#ifndef __CLOVER_STRING_H__
#define __CLOVER_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <Library/BaseMemoryLib.h>

#ifdef __GNUC__
//void* memset(void* dst, int ch, UINT64 count) __attribute__ ((used));
void* memcpy(void *dst, const void *src, size_t len) __attribute__ ((used));
#else
//  void* memset(void* dst, int ch, UINT64 count);
  void* memcpy(void *dst, const void *src, size_t len);
#endif


inline void* memmove(void *dst, const void *src, size_t len)
{
	return CopyMem(dst, (void*)(src), len);
}

inline void* memcpy(void *dst, const void *src, size_t len)
{
	return CopyMem(dst,src,len);
}


#ifdef __cplusplus
}
#endif

#endif

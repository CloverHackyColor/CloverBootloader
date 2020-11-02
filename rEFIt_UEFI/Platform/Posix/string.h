#ifndef __CLOVER_STRING_H__
#define __CLOVER_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <Library/BaseLib.h>
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

void* memcpy(void *dst, const void *src, size_t len);

//inline void* memcpy(void *dst, const void *src, size_t len)
//{
//	return CopyMem(dst,src,len);
//}

inline char* strcat(char* s1, const char* s2)
{
	AsciiStrCatS(s1, AsciiStrLen(s1)+1, s2);
	return s1;
}

char* strncat(char* s1, const char* s2, size_t n);


inline char* strcpy(char* dst, const char* src)
{
	AsciiStrCpyS(dst,AsciiStrLen(src)+1,src);
	return dst;
}

inline char* strncpy(char * dst, const char * src, size_t len)
{
	AsciiStrnCpyS(dst,(UINTN)len+1,src,(UINTN)len);
	return dst;
}

//// edkII Strcmp seems quite inefficient, even vs a naive implementation
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

int strncmp(const char *s1, const char *s2, size_t n);

extern void* memset(void *b, int c, size_t len); // memset is defined in cpp_util/memory.cpp because it is required by c++
//inline void* memset(void *b, int c, size_t len)
//{
//  SetMem(b, len, c);
//  return b;
//}

//inline char* strncat(char *restrict s1, const char *restrict s2, size_t n)
//{
//	return AsciiStrCatS(s1, AsciiStrLen(strDest)+1,strSource)
//}
//

#ifdef __cplusplus
}
#endif

#endif // __CLOVER_STRING_H__

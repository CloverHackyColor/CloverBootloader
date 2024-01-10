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


static inline __attribute__((always_inline)) void* memmove(void *dst, const void *src, size_t len)
{
	return CopyMem(dst, (void*)(src), len);
}

void* memcpy(void *dst, const void *src, size_t len);

//static inline __attribute__((always_inline))  void* memcpy(void *dst, const void *src, size_t len)
//{
//	return CopyMem(dst,src,len);
//}

int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *str);

static inline __attribute__((always_inline)) char* strcat(char* s1, const char* s2)
{
	AsciiStrCatS(s1, AsciiStrLen(s1)+1, s2);
	return s1;
}

char* strncat(char* s1, const char* s2, size_t n);


static inline __attribute__((always_inline)) char* strcpy(char* dst, const char* src)
{
	AsciiStrCpyS(dst,AsciiStrLen(src)+1,src);
	return dst;
}

static inline __attribute__((always_inline)) char* strncpy(char * dst, const char * src, size_t len)
{
	AsciiStrnCpyS(dst,(UINTN)len+1,src,(UINTN)len);
	return dst;
}

//// edkII Strcmp seems quite inefficient, even vs a naive implementation
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

int strncmp(const char *s1, const char *s2, size_t n);

void* memset(void *b, int c, size_t len); // memset is defined in cpp_util/memory.cpp because it is required by c++
//static inline __attribute__((always_inline)) void* memset(void *b, int c, size_t len)
//{
//  SetMem(b, len, c);
//  return b;
//}

//static inline __attribute__((always_inline)) char* strncat(char *restrict s1, const char *restrict s2, size_t n)
//{
//	return AsciiStrCatS(s1, AsciiStrLen(strDest)+1,strSource)
//}
//

static inline __attribute__((always_inline)) char* strchr(const char *s, int c)
{
  return (char*)ScanMem8((void *)(s),AsciiStrSize(s),(UINT8)c);
}

/* Scan a string for the last occurrence of a character */
char* strrchr (const char  *str, int c);

char* strstr(const char *haystack, const char *needle);

/* Computes the length of the maximum initial segment of the string pointed to by s1
   which consists entirely of characters from the string pointed to by s2. */
size_t strspn(const char  *s1, const char  *s2);

/* Computes the length of the maximum initial segment of the string pointed to by s1
   which consists entirely of characters not from the string pointed to by s2. */
size_t strcspn(const char  *s1, const char  *s2);

#ifdef __cplusplus
}
#endif

#endif // __CLOVER_STRING_H__

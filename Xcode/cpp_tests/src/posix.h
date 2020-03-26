#ifndef __POSIX_H__
#define __POSIX_H__


#include <stdio.h>
#include <stdint.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if defined(__APPLE__) && defined(__clang__) && __WCHAR_MAX__ <= 0xFFFFu
// 2020-03 : w... function are broken under macOs and clang with short-wchar.
//           Currently with clang version Apple LLVM version 10.0.0 (clang-1000.11.45.5) with High Sierra
//           If it's fixed one day, a version number could added to this #ifdef

#   include "xcode_utf16.h"
#else
#   include <wchar.h>
#endif

//
//#define MAX_UINTN ULONG_MAX
#define BOOLEAN bool


#define CHAR8  char
//#define CHAR16 char16_t
#define CHAR16 wchar_t
//
#define UINT8  uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define INT8  int8_t
#define INT16 int16_t
#define INT32 int32_t
#define INT64 int64_t

#define MAX_INT8    ((INT8)0x7F)
#define MAX_UINT8   ((UINT8)0xFF)
#define MAX_INT16   ((INT16)0x7FFF)
#define MAX_UINT16  ((UINT16)0xFFFF)
#define MAX_INT32   ((INT32)0x7FFFFFFF)
#define MAX_UINT32  ((UINT32)0xFFFFFFFF)
#define MAX_INT64   ((INT64)0x7FFFFFFFFFFFFFFFULL)
//#define MAX_UINT64  ((UINT64)0xFFFFFFFFFFFFFFFFULL)
#define MAX_UINT64  0xFFFFFFFFFFFFFFFFULL

#define UINTN uint64_t
#define INTN int64_t

#define MAX_UINTN MAX_UINT64
#define MAX_INTN MAX_UINT64

////
//// Macros that directly map functions to BaseLib, BaseMemoryLib, and DebugLib functions
//// originally from OpensslLib
//
//#define memcpy(dest,source,count)         CopyMem(dest,source,(UINTN)(count))
//#define memset(dest,ch,count)             SetMem(dest,(UINTN)(count),(UINT8)(ch))
//#define memchr(buf,ch,count)              ScanMem8(buf,(UINTN)(count),(UINT8)ch)
//#define memcmp(buf1,buf2,count)           (int)(CompareMem(buf1,buf2,(UINTN)(count)))
//#define memmove(dest,source,count)        CopyMem(dest,source,(UINTN)(count))

//#define strcmp                            AsciiStrCmp
//#define strncmp(string1,string2,count)    (int)(AsciiStrnCmp(string1,string2,(UINTN)(count)))

//// edkII Strcmp seens quite inefficient, even vs a naive implementation
//xint strcmp(const char* s1, const char* s2);
//int strncmp( const char* s1, const char* s2, size_t n );
//
//#ifdef CLOVER_BUILD
//#define strcpy(strDest,strSource)         AsciiStrCpyS(strDest,AsciiStrLen(strDest)+1,strSource)
//#define strncpy(strDest,strSource,count)  AsciiStrnCpyS(strDest,(UINTN)count+1,strSource,(UINTN)count)
//#define strlen(str)                       (size_t)(AsciiStrLen(str))
//#define strcat(strDest,strSource)         AsciiStrCatS(strDest,AsciiStrLen(strDest)+1,strSource)
//#define strchr(str,ch)                    ScanMem8((VOID *)(str),AsciiStrSize(str),(UINT8)ch)
//#define strstr(a,b)                       AsciiStrStr(a,b)
//#endif
//
//void abort(void);
//
//
//inline float fabsf(float x) {
//  if (x < 0.f) return -x;
//  return x;
//}
//

//
//inline void* realloc(void *ptr, size_t newsize, size_t oldsize) // not the posix realloc. For EFI we need oldsize
//{
//	(void)oldsize;
//	return realloc(ptr, newsize);
//}


#endif

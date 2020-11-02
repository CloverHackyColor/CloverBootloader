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

#   include "xcode_utf_fixed.h"
#else
#   include <wchar.h>
#endif


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

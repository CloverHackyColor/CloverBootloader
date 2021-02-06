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
#include "posix_additions.h"

#if defined(__APPLE__) && defined(__clang__) && __WCHAR_MAX__ <= 0xFFFFu
// 2020-03 : w... function are broken under macOs and clang with short-wchar.
//           Currently with clang version Apple LLVM version 10.0.0 (clang-1000.11.45.5) with High Sierra
//           If it's fixed one day, a version number could added to this #ifdef

//#   include "../../../../../cpp_tests/Include/xcode_utf_fixed.h"
#else
#   include <wchar.h>
#endif

//
//inline void* realloc(void *ptr, size_t newsize, size_t oldsize) // not the posix realloc. For EFI we need oldsize
//{
//	(void)oldsize;
//	return realloc(ptr, newsize);
//}


#endif

#ifndef __CLOVER_STDDEF_H__
#define __CLOVER_STDDEF_H__

#include "stdint.h"

// How can we prevent VC to define this ?
#ifdef _MSC_VER

// VS define size_t

#else

		// 2020-03 : On Gcc 9.2 and Clang (Apple LLVM version 10.0.0), size_t is not builtin, but __SIZE_TYPE__ is
		typedef __SIZE_TYPE__ size_t;
		//typedef long int ssize_t; // no __SSIZE_TYPE__. We don't use ssize_t in CLover. Let's try to keep it that way.

#endif

typedef INTN ptrdiff_t;
typedef UINTN uintptr_t;

#endif

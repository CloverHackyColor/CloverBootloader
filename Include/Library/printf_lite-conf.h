//
//  printf_lite.hpp
//
//  Created by jief the 04 Apr 2019.
//  Imported in CLover the 24 Feb 2020
//
#ifndef __PRINTF_LITE_CONF_H__
#define __PRINTF_LITE_CONF_H__

//#include <stdarg.h>
//#include <stddef.h> // for size_t

#ifdef __cplusplus
extern "C" {
#endif

#include <Base.h>

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus
	#ifdef _MSC_VER
	  typedef UINT16 wchar_t;
	#endif
	typedef UINT32 char32_t;
	typedef UINT16 char16_t;
  typedef UINT32 uint32_t;
  #define PRIu32 d
#endif

#ifdef _MSC_VER

// VS define size_t

#else

    // 2020-03 : On Gcc 9.2 and Clang (Apple LLVM version 10.0.0), size_t is not builtin, but __SIZE_TYPE__ is
    typedef __SIZE_TYPE__ size_t;
    //typedef long int ssize_t; // no __SSIZE_TYPE__. We don't use ssize_t in Clover. Let's try to keep it that way.

#endif


#ifdef _MSC_VER
#   define __attribute__(x)
#endif

#ifdef DEBUG
#define DEFINE_SECTIONS 0
#endif

#define PRINTF_LITE_BUF_SIZE 255 // not more than 255
#define PRINTF_LITE_TIMESTAMP_SUPPORT 1
#define PRINTF_LITE_TIMESTAMP_CUSTOM_FUNCTION 1
#define PRINTF_EMIT_CR_SUPPORT 1


#define PRINTF_CFUNCTION_PREFIX
#define PRINTF_CFUNCTION_SUFFIX f

#define PRINTF_VA_LIST VA_LIST
#define PRINTF_VA_START VA_START
#define PRINTF_VA_ARG VA_ARG
#define PRINTF_VA_END VA_END


#endif // __PRINTF_LITE_CONF_H__

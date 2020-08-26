//
//  printf_lite.hpp
//
//  Created by jief the 04 Apr 2019.
//  Imported in CLover the 24 Feb 2020
//
#ifndef __PRINTF_LITE_CONF_H__
#define __PRINTF_LITE_CONF_H__

#include <stdarg.h>
#include <stddef.h> // for size_t

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



#endif // __PRINTF_LITE_CONF_H__

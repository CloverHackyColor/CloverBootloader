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
#include <stdint.h>
#include <inttypes.h>

#ifndef __cplusplus
	#ifdef _MSC_VER
	  typedef uint16_t wchar_t;
	#endif
	typedef uint32_t char32_t;
	typedef uint16_t char16_t;
#endif

#ifdef _MSC_VER
#   define __attribute__(x)
#endif

#ifdef DEBUG
#define DEFINE_SECTIONS 0
#endif


#define PRINTF_LITE_TIMESTAMP_SUPPORT 1
#define PRINTF_EMIT_CR_SUPPORT 1


#endif // __PRINTF_LITE_CONF_H__

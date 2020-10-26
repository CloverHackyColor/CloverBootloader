//
//  printf_lite.h
//
//  Created by jief the 04 Apr 2019.
//  Imported in CLover the 24 Feb 2020
//
#ifndef __PRINTF_LITE_H__
#define __PRINTF_LITE_H__

#include <stdarg.h>
#include <stddef.h> // for size_t

#include <printf_lite-conf.h>

#if defined(__cplusplus)
extern "C"
{
#endif

#ifdef DEBUG
#define DEFINE_SECTIONS 0
#endif

// To be able to compile on a platform where there are already std function snprintf, we need to change the name
#ifndef PRINTF_CFUNCTION_PREFIX
#define PRINTF_CFUNCTION_PREFIX
#endif
#ifndef PRINTF_CFUNCTION_SUFFIX
#define PRINTF_CFUNCTION_SUFFIX fl
#endif

#define PRINTF_MAKE_FN_NAME(prefix, root, suffix) prefix##root##suffix
#define PRINTF_FUNCTION_NAME(prefix, root, suffix) PRINTF_MAKE_FN_NAME(prefix, root, suffix)

#ifndef PRINTF_UTF8_INPUT_SUPPORT
#define PRINTF_UTF8_INPUT_SUPPORT 1
#endif
#ifndef PRINTF_UNICODE_INPUT_SUPPORT
#define PRINTF_UNICODE_INPUT_SUPPORT 1 // enable %ls. %ls = UTF16 or UTF32 string, depending of the size of wchar_t
#endif

#ifndef PRINTF_UTF8_OUTPUT_SUPPORT
#define PRINTF_UTF8_OUTPUT_SUPPORT 1
#endif
#ifndef PRINTF_UNICODE_OUTPUT_SUPPORT
#define PRINTF_UNICODE_OUTPUT_SUPPORT 1 // UTF16 or UTF32 depending of the size of wchar_t
#endif
#if PRINTF_UTF8_OUTPUT_SUPPORT == 0  &&  PRINTF_UNICODE_OUTPUT_SUPPORT == 0
#error no output format supported.
#endif

#ifndef PRINTF_LITE_SNPRINTF_SUPPORT
#define PRINTF_LITE_SNPRINTF_SUPPORT 1
#endif
#ifndef VSNWPRINTF_RETURN_MINUS1_ON_OVERFLOW
#define VSNWPRINTF_RETURN_MINUS1_ON_OVERFLOW 0
#endif

#ifndef PRINTF_CHECK_UNSUPPORTED_STRING_FORMAT
#define PRINTF_CHECK_UNSUPPORTED_STRING_FORMAT 0
#endif

#ifndef PRINTF_LITE_DOT_STAR_SUPPORT
#define PRINTF_LITE_DOT_STAR_SUPPORT 1
#endif

#ifndef PRINTF_LITE_STRING_WIDTH_SPECIFIER_SUPPORT
#define PRINTF_LITE_STRING_WIDTH_SPECIFIER_SUPPORT 1
#endif

/*
 * A buffer is not needed. On some case it could be faster. For example when using semihosting
 * It's 255 max, because of bufIdx of type uint8_t. It's possible to change it to int if bigger buffer needed.
 * This buffer isn't statically allocated so it won't use permanent RAM.
 * Not more than int because of cast in transmitBufXXX functions.
 * 2017/08/31 : Save 22 bytes + bufsize of my STM32F103
 */
#ifndef PRINTF_LITE_BUF_SIZE
# define PRINTF_LITE_BUF_SIZE 200
#else
# if PRINTF_LITE_BUF_SIZE > 255
#   error PRINTF_LITE_BUF_SIZE > 255
# endif
#endif
/*
 * Fallback on something close if specifier isn't supported.
 * Unsupported specifier are ignored.
 * if 0, any unsupported modifier or specifier will be seen as unknown.
 */
#ifndef PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED
#define PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED 0
#endif

#ifndef PRINTF_LITE_FLOAT_SUPPORT
#define PRINTF_LITE_FLOAT_SUPPORT 1
#endif
/*
 * The int part of the float (or double) will be printed if PRINTF_LITE_FLOAT_SUPPORT is enabled
 * June 2017, avr-gcc 4.9.2-atmel3.5.4-arduino2 :
 * Float support increase 828 bytes of text and 16 bytes of data
 */
#ifndef PRINTF_LITE_FLOAT_AS_INT_SUPPORT
#define PRINTF_LITE_FLOAT_AS_INT_SUPPORT 1 // if float not supported, an int will be print instead.
#endif
/*
 *  Disabling LONG LONG support has an effect on printing double. If long int is disabled, the int part of the double has to be < ULONG_MAX
 */
#ifndef PRINTF_LITE_LONGLONGINT_SUPPORT
#define PRINTF_LITE_LONGLONGINT_SUPPORT 1 // 1712 bytes
#endif
/*
 *  Disabling LONG support automatically disable LONG LONG support whatever the value of PRINTF_LITE_LONGLONGINT_SUPPORT.
 *  Disabling LONG support has an effect on printing double. The int part of the double has to be < UINT_MAX max
 */
#ifndef PRINTF_LITE_LONGINT_SUPPORT
#define PRINTF_LITE_LONGINT_SUPPORT 1 // 1712 bytes
#endif
#ifndef PRINTF_LITE_SHORTINT_SUPPORT
#define PRINTF_LITE_SHORTINT_SUPPORT 1
#endif
#ifndef PRINTF_LITE_SHORTSHORTINT_SUPPORT
#define PRINTF_LITE_SHORTSHORTINT_SUPPORT 1
#endif
#ifndef PRINTF_LITE_TIMESTAMP_SUPPORT
#define PRINTF_LITE_TIMESTAMP_SUPPORT 1 // 240 bytes
#endif
#ifndef PRINTF_LITE_TIMESTAMP_CUSTOM_FUNCTION
#define PRINTF_LITE_TIMESTAMP_CUSTOM_FUNCTION 0
#endif
#ifndef PRINTF_LITE_FIELDWIDTH_SUPPORT
#define PRINTF_LITE_FIELDWIDTH_SUPPORT 1 // 107 bytes
#endif
#ifndef PRINTF_LITE_FIELDPRECISION_SUPPORT
#define PRINTF_LITE_FIELDPRECISION_SUPPORT 1 //  bytes
#endif
#ifndef PRINTF_LITE_PADCHAR_SUPPORT
#define PRINTF_LITE_PADCHAR_SUPPORT 1 //  bytes
#endif
#ifndef PRINTF_LITE_ZSPECIFIER_SUPPORT
#define PRINTF_LITE_ZSPECIFIER_SUPPORT 1 // 230 bytes. If not supported, z modifier become llu
#endif
#ifndef PRINTF_LITE_XSPECIFIER_SUPPORT
#define PRINTF_LITE_XSPECIFIER_SUPPORT 1 // 96 bytes. If not supported, x specifier become u
#endif
#ifndef PRINTF_LITE_USPECIFIER_SUPPORT
#define PRINTF_LITE_USPECIFIER_SUPPORT 1 // 96 bytes. If not supported, u specifier become d
#endif
#ifndef PRINTF_EMIT_CR_SUPPORT
#define PRINTF_EMIT_CR_SUPPORT 0
#endif
/*=====================================================  Private definition ============================================*/
#if PRINTF_UTF8_OUTPUT_SUPPORT == 1
	typedef void (*transmitBufCallBackType)(const char* buf, unsigned int nbchar, void* context);
#endif
#if PRINTF_UNICODE_OUTPUT_SUPPORT == 1
	typedef void (*transmitWBufCallBackType)(const wchar_t* buf, unsigned int nbchar, void* context);
#endif

typedef union {
	#if PRINTF_UTF8_OUTPUT_SUPPORT == 1
		transmitBufCallBackType transmitBufCallBack;
	#endif
	#if PRINTF_UNICODE_OUTPUT_SUPPORT == 1
		transmitWBufCallBackType transmitWBufCallBack;
	#endif
} printf_callback_t;

// I need to pass a va_list by reference, NOT value, because va_list is "incremented" in printf_handle_format_char
// On macOS, the builtin va_list type seems to already be a pointer
#ifdef __APPLE__
    #define VALIST_PARAM_TYPE va_list
    #define VALIST_PARAM(valist) valist
    #define VALIST_ACCESS(valist) valist
#else
    #define VALIST_PARAM_TYPE va_list*
    #define VALIST_PARAM(valist) &valist
    #define VALIST_ACCESS(valist) (*valist)
#endif

//void printf_handle_format_char(char c, VALIST_PARAM_TYPE valist, PrintfParams* printfParams);



/*=====================================================  User function ============================================*/
// I tried to inline this in printf_lite.h, but GCC seems to not accept that an inline function could call another inline function.
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
# if PRINTF_EMIT_CR_SUPPORT == 1
  void vprintf_with_callback_timestamp_emitcr(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context, int* newline, int timestamp, int emitCr); // emitCr is a boolean flag
  void vprintf_with_callback_timestamp(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context, int* newline, int timestamp);
//  inline void vprintf_with_callback_timestamp(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context, int* newline, int timestamp) {
//    vprintf_with_callback_timestamp_emitcr(format, valist, transmitBufCallBack, context, newline, timestamp, 0);
//  }
  void vprintf_with_callback_emitcr(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context, int emitCr);
//  inline void vprintf_with_callback_emitcr(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context, int emitCr) {
//    vprintf_with_callback_timestamp_emitcr(format, valist, transmitBufCallBack, context, NULL, 0, emitCr);
//  }
  void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context);
//  inline void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context) {
//    vprintf_with_callback_timestamp_emitcr(format, valist, transmitBufCallBack, context, NULL, 0, 0);
//  }
# else
  void vprintf_with_callback_timestamp(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context, int* newline, int timestamp);
  void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context);
//  inline void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context) {
//    vprintf_with_callback_timestamp(format, valist, transmitBufCallBack, context, NULL, 0);
//  }
# endif
#else
# if PRINTF_EMIT_CR_SUPPORT == 1
  void vprintf_with_callback_emitcr(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context, int emitCr);
  void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context);
//  inline void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context) {
//    vprintf_with_callback_emitcr(format, valist, transmitBufCallBack, context, 0);
//  }
# else
  void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context);
# endif
#endif


#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
# if PRINTF_EMIT_CR_SUPPORT == 1
  void printf_with_callback_timestamp_emitcr(const char* format, transmitBufCallBackType transmitBufCallBack, void* context, int* newline, int timestamp, int emitCr, ...);
  inline void printf_with_callback_timestamp(const char* format, transmitBufCallBackType transmitBufCallBack, void* context, int* newline, int timestamp, ...) {
    va_list va;
    va_start(va, timestamp);
    vprintf_with_callback_timestamp(format, va, transmitBufCallBack, context, newline, timestamp);
    va_end(va);
  }
  inline void printf_with_callback__emitcr(const char* format, transmitBufCallBackType transmitBufCallBack, void* context, int emitCr, ...) {
    va_list va;
    va_start(va, emitCr);
    vprintf_with_callback_emitcr(format, va, transmitBufCallBack, context, emitCr);
    va_end(va);
  }
  inline void printf_with_callback(const char* format, transmitBufCallBackType transmitBufCallBack, void* context, ...) {
    va_list va;
    va_start(va, context);
    vprintf_with_callback(format, va, transmitBufCallBack, context);
    va_end(va);
  }
# else
  void printf_with_callback_timestamp(const char* format, transmitBufCallBackType transmitBufCallBack, void* context, int* newline, int timestamp, ...);
  inline void printf_with_callback(const char* format, transmitBufCallBackType transmitBufCallBack, void* context, ...) {
    va_list va;
    va_start(va, context);
    vprintf_with_callback_timestamp(format, va, transmitBufCallBack, context, NULL, 0);
    va_end(va);
  }
# endif
#else
# if PRINTF_EMIT_CR_SUPPORT == 1
  void printf_with_callback_emitcr(const char* format, transmitBufCallBackType transmitBufCallBack, void* context, bool emitCr, ...);
# else
  void printf_with_callback(const char* format, transmitBufCallBackType transmitBufCallBack, void* context, ...);
# endif
#endif



#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
void vwprintf_with_callback_timestamp(const char* format, va_list valist, transmitWBufCallBackType transmitBufCallBack, void* context, int* newline, int timestamp);
inline void vwprintf_with_callback(const char* format, va_list valist, transmitWBufCallBackType transmitWBufCallBack, void* context) {
  vwprintf_with_callback_timestamp(format, valist, transmitWBufCallBack, context, NULL, 0);
}
#else
void vwprintf_with_callback(const char* format, va_list valist, transmitWBufCallBackType transmitWBufCallBack, void* context);
#endif

#if PRINTF_LITE_SNPRINTF_SUPPORT == 1
	#if PRINTF_UTF8_OUTPUT_SUPPORT == 1
		int PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnprint, PRINTF_CFUNCTION_SUFFIX)(char*, size_t, const char *__restrict, va_list valist);
		// gcc-4.9.2-atmel3.5.4-arduino2 report snprintf to undefined. Change the name and it'll work. Strange isn't it ?
		int PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, snprint, PRINTF_CFUNCTION_SUFFIX)(char*, size_t len, const char *__restrict format, ...) __attribute__((__format__ (__printf__, 3, 4)));
	#endif
	#if PRINTF_UNICODE_OUTPUT_SUPPORT == 1
	    // ATTENTION : len is the number of wchar_t, NOT the number of bytes.
		int PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wchar_t*, size_t len, const char *__restrict, va_list valist);
		int PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, snwprint, PRINTF_CFUNCTION_SUFFIX)(wchar_t*, size_t len, const char *__restrict format, ...) __attribute__((__format__ (__printf__, 3, 4)));
	#endif

#endif


#if defined(__cplusplus)
}
#endif

//#if defined(__cplusplus)  &&  PRINTF_LITE_TIMESTAMP_SUPPORT == 1
//  void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context){
//    vprintf_with_callback(format, valist, transmitBufCallBack, NULL, 0, context);
//  }
//#endif
//#if defined(__cplusplus)  &&  PRINTF_LITE_TIMESTAMP_SUPPORT == 1
//  void printf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context){
//    printf_with_callback(format, valist, transmitBufCallBack, NULL, 0, context);
//  }
//#endif
//#if defined(__cplusplus)  &&  PRINTF_UNICODE_OUTPUT_SUPPORT == 1  &&  PRINTF_LITE_TIMESTAMP_SUPPORT == 1
//  void vwprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context){
//    vwprintf_with_callback(format, valist, transmitBufCallBack, NULL, 0, context);
//  }
//#endif

#endif // __PRINTF_LITE_H__

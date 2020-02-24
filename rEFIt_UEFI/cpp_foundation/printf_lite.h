//
//  printf_lite.hpp
//
//  Created by jief the 04 Apr 2019.
//  Imported in CLover the 24 Feb 2020
//
#ifndef __PRINTF_LITE_H__ // do not include protect, printf-lite.cpp include that twice
#define __PRINTF_LITE_H__

#include <stdarg.h>
#include <stddef.h>
#ifdef ARDUINO
	#include <WString.h>
	#ifdef HAS_LIQUID_CRYSTAL
		#include <LiquidCrystal.h>
	#endif
#endif

#define PRINTF_OUTPUT_FORMAT_UNICODE 1
#if PRINTF_OUTPUT_FORMAT_UNICODE == 1
#  define PRINTF_UTF8_SUPPORT 1
#endif

/*
 * A buffer is not needed. On some case it could be faster. For example when using semihosting
 * It's 255 max, because of bufIdx of type uint8_t. It's possible to change it to int if bigger buffer needed.
 * This buffer isn't statically allocated so it won't use permanent RAM.
 * Not more than int because of cast in transmitBufXXX functions.
 * 2017/08/31 : Save 22 bytes + bufsize of my STM32F103
 */
#ifndef PRINTF_LITE_BUF_SIZE
#define PRINTF_LITE_BUF_SIZE 0
#endif
/*
 * Fallback on something close if specifier is'nt supported.
 * Unsupported specifier are ignored.
 * if 0, any unsupported modifier or specifier will be seen as unknown.
 */
#ifndef PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED
#define PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED 1
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
#ifndef PRINTF_LITE_TIMESTAMP_SUPPORT
#define PRINTF_LITE_TIMESTAMP_SUPPORT 0 // 240 bytes
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

#ifndef PRINTF_LITE_UART_SUPPORT
#define PRINTF_LITE_UART_SUPPORT 0 //
#endif
#ifndef PRINTF_LITE_USB_SUPPORT
#define PRINTF_LITE_USB_SUPPORT 0 //
#endif
#ifndef PRINTF_LITE_SNPRINTF_SUPPORT
#define PRINTF_LITE_SNPRINTF_SUPPORT 0 //
#endif
#ifndef PRINTF_LITE_FLASHSTRING_SUPPORT
#define PRINTF_LITE_FLASHSTRING_SUPPORT 0 //
#endif


#if PRINTF_OUTPUT_FORMAT_UNICODE == 1
#  define printf_char_type wchar_t
#else
#  define printf_char_type char
#endif

typedef void (*transmitBufCallBackType)(const printf_char_type* buf, size_t nbyte);
void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
							, int* newline, int timestamp
#endif
						);

#if defined(__cplusplus)
extern "C"
{
#endif
	#if PRINTF_LITE_SNPRINTF_SUPPORT == 1
		int vsnprintf(char *__restrict, size_t, const char *__restrict, va_list valist) __attribute__ ((__format__ (__printf__, 3, 0))); // have to return int to conform stdio.h. Always return 1;

		// gcc-4.9.2-atmel3.5.4-arduino2 report snprintf to undefined. Change the name and it'll work. String isn't it ?
		int snprintf(char *__restrict buf, size_t len, const char *__restrict format, ...) __attribute__((__format__ (__printf__, 3, 4))); // have to return int to conform stdio.h. Always return 1;
	#endif

	#if PRINTF_LITE_UART_SUPPORT == 1
		void vprintf_uart(const char* str, va_list valist);
		void printf_uart(const char *str, ...) __attribute__((format(printf, 1, 2)));
		void vlogf_uart(const char* str, va_list valist);
		void logf_uart(const char *str, ...) __attribute__((format(printf, 1, 2))); // same as printf_uart but print a timestamp at beginning of line.
	#endif

	#ifdef LiquidCrystal_h
		void printf_lcd(int row, int col, const char *str, ...) __attribute__((format(printf, 3, 4)));
	#endif
	#if PRINTF_LITE_USB_SUPPORT == 1  &&  defined(__USBD_CDC_IF_H)
		void transmitBufUsb(const char* buf, size_t nbyte);
		void printf_usb(const char *str, ...) __attribute__((format(printf, 1, 2)));
		void vlogf_usb(const char* str, va_list valist);
		void logf_usb(const char *str, ...) __attribute__((format(printf, 1, 2)));
	#endif

	#if defined(OS_USE_TRACE_ITM) || defined(OS_USE_TRACE_SEMIHOSTING_DEBUG) || defined(OS_USE_TRACE_SEMIHOSTING_STDOUT)
		void vprintf_semih(const char* str, va_list valist);
		void printf_semih(const char* format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
	#endif
#if defined(__cplusplus)
}
#endif


#if defined(__cplusplus)

	#if defined(ARDUINO)  &&  PRINTF_LITE_FLASHSTRING_SUPPORT == 1
		int vsnprintf(char *__restrict, size_t, const __FlashStringHelper *__restrict, va_list valist); // have to return int to conform stdio.h. Always return 1;
		// gcc-4.9.2-atmel3.5.4-arduino2 report snprintf to undefined. Change the name and it'll work. String isn't it ?
		int snprintf(char *__restrict buf, size_t len, const __FlashStringHelper * format, ...); // have to return int to conform stdio.h. Always return 1;

		// C code can't access this because __FlashStringHelper is a class
		void vlogf_uart(const __FlashStringHelper* str, va_list valist);

		#ifdef __JIEF_FLASHSTRING_PRINTFWARNING_HACK__
            void printf_uart(const __FlashStringHelper *str, ...) __attribute__((format(printf, 1, 2)));
            void logf_uart(const __FlashStringHelper *str, ...) __attribute__((format(printf, 1, 2))); // same as printf_uart but print a timestamp at beginning of line.
            #ifdef LiquidCrystal_h
                void printf_lcd(int row, int col, const __FlashStringHelper *str, ...) __attribute__((format(printf, 3, 4)));
            #endif
         #else
            // if __JIEF_FLASHSTRING_PRINTFWARNING_HACK isn't defined, it means you don't use my hacked version of GCC. "Normal" version can't handle const __FlashStringHelper* as format for warning
            void printf_uart(const __FlashStringHelper *str, ...);
            void logf_uart(const __FlashStringHelper *str, ...); // same as printf_uart but print a timestamp at beginning of line.
            #ifdef LiquidCrystal_h
                void printf_lcd(int row, int col, const __FlashStringHelper *str, ...);
            #endif
         #endif
	#endif

#endif


#endif // __PRINTF_LITE_H__

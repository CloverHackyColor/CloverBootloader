//
//  printf_lite.hpp
//
//  Created by jief the 04 Apr 2019.
//  Imported in CLover the 24 Feb 2020
//
/* 
    This code should be pasted within the files where this function is needed.
    This function will not create any code conflicts.
    
    The function call is similar to printf: ardprintf("Test %d %s", 25, "string");

    To print the '%' character, use '%%'

    This code was first posted on http://arduino.stackexchange.com/a/201
*/

#include "printf_lite.h" // need to include that before testing #if PRINTF_LITE_USB_SUPPORT == 1

#ifdef USE_HAL_DRIVER
	#include "stm32f1xx_hal.h"
	#if PRINTF_LITE_USB_SUPPORT == 1
		#include "usb_device.h"
		#include "usbd_cdc_if.h"
		#include "printf_lite.h" // need to re-include that after included usbd_cdc_if.h to get definition under defined(__USBD_CDC_IF_H)
	#endif
#endif

#ifdef ARDUINO
	#include <Arduino.h>
//	#include <HardwareSerial.h>
#endif

#ifdef NRF51
	#include <app_timer.h>
	#include <app_uart.h>
#endif

#if defined(OS_USE_TRACE_ITM) || defined(OS_USE_TRACE_SEMIHOSTING_DEBUG) || defined(OS_USE_TRACE_SEMIHOSTING_STDOUT)
#include <trace_impl.h>
#endif

#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>

#ifdef __STM32F1xx_HAL_UART_H
extern UART_HandleTypeDef huart1;
#endif





// using int because this is what generate less code. It's a bit more on the stack but it's temporary.
typedef struct  {
  #if PRINTF_LITE_BUF_SIZE > 1
	printf_char_type buf[PRINTF_LITE_BUF_SIZE];
	uint8_t bufIdx;
  #endif
	transmitBufCallBackType transmitBufCallBack;
	int inDirective;
	int l_modifier;
  #if PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
	int z_modifier;
  #endif
  #if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
	int inWidthField;
	int width_specifier;
  #endif
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1  ||  (PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1) // in that case, we need the inPrecisionField to know we are currently ignoring precision field
	int inPrecisionField;
  #endif
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
	int precision_specifier;
  #endif
  #if PRINTF_LITE_PADCHAR_SUPPORT == 1
	char pad_char;
  #endif
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
	int *newlinePtr;
	int timestamp; // not using bool in case of C compilation
  #endif
  #if PRINTF_LITE_XSPECIFIER_SUPPORT == 1
	int uppercase;
  #endif
} PrintfParams;

#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
static void print_timestamp(PrintfParams* printfParams);
#endif

static void print_char(printf_char_type c, PrintfParams* printfParams)
{
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
	if ( printfParams->newlinePtr )
	{
		if ( *printfParams->newlinePtr )
		{
			*printfParams->newlinePtr = 0; // to do BEFORE call to printTimeStamp
			if ( printfParams->timestamp ) print_timestamp(printfParams);
		}
        #if PRINTF_LITE_BUF_SIZE > 1
            printfParams->buf[(printfParams->bufIdx)++] = c;
        #else
            printfParams->transmitBufCallBack(&c, 1);
        #endif
		if ( c == '\n' ) {
			*printfParams->newlinePtr = 1;
		}
	}else{
        #if PRINTF_LITE_BUF_SIZE > 1
            printfParams->buf[(printfParams->bufIdx)++] = c;
        #else
            printfParams->transmitBufCallBack(&c, 1);
        #endif
	}
  #else
    {
        #if PRINTF_LITE_BUF_SIZE > 1
            printfParams->buf[(printfParams->bufIdx)++] = c;
        #else
            printfParams->transmitBufCallBack(&c, 1);
        #endif
	}
  #endif
  #if PRINTF_LITE_BUF_SIZE > 1
	if ( printfParams->bufIdx == PRINTF_LITE_BUF_SIZE ) {
		printfParams->transmitBufCallBack(printfParams->buf, printfParams->bufIdx);
		printfParams->bufIdx = 0;
	}
  #endif
}



#if PRINTF_OUTPUT_FORMAT_UNICODE == 1  &&  PRINTF_UTF8_SUPPORT == 1

#define halfBase 0x0010000UL
#define halfMask 0x3FFUL
#define halfShift 10 /* used for shifting by 10 bits */
#define UNI_SUR_HIGH_START  0xD800u
#define UNI_SUR_LOW_START   0xDC00u

static void print_string(const unsigned char* s, PrintfParams* printfParams)
{
	while ( *s ) {
		char32_t c;
		if (*s & 0x80) {
			if (*(s+1) == 0) {
				// Finished in the middle of an utf8 multibyte char
				return;
			}
			if ((*(s+1) & 0xc0) != 0x80) {
				s += 1;
				continue;
			}
			if ((*s & 0xe0) == 0xe0) {
				if (*(s+2) == 0) {
				// Finished in the middle of an utf8 multibyte char
				return;
			}
				if ((*(s+2) & 0xc0) != 0x80) {
					s += 2;
					continue;
				}
				if ((*s & 0xf0) == 0xf0) {
					if (*(s+3) == 0) {
						// Finished in the middle of an utf8 multibyte char
						return;
					}
					if ((*s & 0xf8) != 0xf0 || (*(s+3) & 0xc0) != 0x80) {
						s += 3;
						continue;
					}
					/* 4-byte code */
					c = (*s & 0x7) << 18;
					c |= (*(s+1) & 0x3f) << 12;
					c |= (*(s+2) & 0x3f) << 6;
					c |= *(s+3) & 0x3f;
					s += 4;
				} else {
					/* 3-byte code */
					c = (*s & 0xf) << 12;
					c |= (*(s+1) & 0x3f) << 6;
					c |= *(s+2) & 0x3f;
					s += 3;
				}
			} else {
				/* 2-byte code */
				c = (*s & 0x1f) << 6;
				c |= *(s+1) & 0x3f;
				s += 2;
			}
		} else {
			/* 1-byte code */
			c = *s;
			s += 1;
		}
#if __WCHAR_MAX__ > 0xFFFFu
		print_char(c, printfParams);
#else
		if ( c <= 0xFFFF) {
				print_char((wchar_t)c, printfParams);
		} else {
				c -= halfBase;
				print_char((wchar_t)((c >> halfShift) + UNI_SUR_HIGH_START), printfParams);
				print_char((wchar_t)((c & halfMask) + UNI_SUR_LOW_START), printfParams);
		}
#endif
	}
}
#endif

static void print_string(const printf_char_type* s, PrintfParams* printfParams)
{
	while ( *s ) print_char(*s++, printfParams);
}

#if defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1

static void print_Fstring(const char* s, PrintfParams* printfParams)
{
	PGM_P p_to_print = reinterpret_cast<PGM_P>(s);
	unsigned char c_to_print = pgm_read_byte(p_to_print++);
	while ( c_to_print != 0 ) {
		print_char(c_to_print, printfParams);
		c_to_print = pgm_read_byte(p_to_print++);
	}
}

#endif

#if PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
	#define INT_BIGGEST_TYPE long long int
#else
	#if PRINTF_LITE_LONGINT_SUPPORT == 1
		#define INT_BIGGEST_TYPE long int
	#else
		#define INT_BIGGEST_TYPE int
	#endif
#endif

/* Jief : I found this here : https://github.com/cjlano/tinyprintf/blob/master/tinyprintf.c. Thanks CJlano */
static void print_ulonglong(unsigned INT_BIGGEST_TYPE v, unsigned int base, PrintfParams* printfParams, int printfSign)
{
    	int n = 0;
    	unsigned INT_BIGGEST_TYPE d = 1;
	#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
    	int nbDigits = 1 + printfSign;
	#endif
		while (v / d >= base) {
			d *= base;
			#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
				nbDigits += 1;
			#endif
		}
		#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  &&  PRINTF_LITE_PADCHAR_SUPPORT == 1
			if ( printfSign  &&  printfParams->pad_char != ' ' ) print_char('-', printfParams);
		#endif
		#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
			while ( printfParams->width_specifier > nbDigits ) {
				#if PRINTF_LITE_PADCHAR_SUPPORT == 1
					print_char(printfParams->pad_char, printfParams);
				#else
					print_char(' ', printfParams);
				#endif
				nbDigits += 1;
			}
		#endif
		#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  &&  PRINTF_LITE_PADCHAR_SUPPORT == 1
			if ( printfSign  &&  printfParams->pad_char == ' ' ) print_char('-', printfParams);
		#else
			if ( printfSign ) print_char('-', printfParams);
		#endif
		while (d != 0) {
			unsigned int dgt = (unsigned int)(v / d); // cast is safe as v/d < 10
			v %= d;
			d /= base;
	#if PRINTF_LITE_XSPECIFIER_SUPPORT == 1
				print_char( (char)(dgt + (dgt < 10 ? '0' : (printfParams->uppercase ? 'A' : 'a') - 10)), printfParams);
	#else
				print_char( (char)(dgt + '0'), printfParams);
	#endif
				n += 1;
		}
}

#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1

#if defined(EFIAPI)

#error TODO

#elif defined(__APPLE__)

#include <mach/mach_time.h>

uint32_t getUptimeInMilliseconds()
{
    static mach_timebase_info_data_t s_timebase_info;

    kern_return_t result = mach_timebase_info(&s_timebase_info);
    (void(result));
//    assert(result == 0);
    
    // multiply to get value in the nano seconds
    double multiply = (double)s_timebase_info.numer / (double)s_timebase_info.denom;
    // divide to get value in the seconds
    multiply /= 1000;

    return mach_absolute_time() * multiply;
}
#endif
#endif //PRINTF_LITE_TIMESTAMP_SUPPORT


static void print_longlong(INT_BIGGEST_TYPE v, int base, PrintfParams* printfParams)
{
	if ( v >= 0 ) return print_ulonglong(v, base, printfParams, 0);
	print_ulonglong(-v, base, printfParams, 1); // -v doesn't work for INT64_MIN
}

#define PRINTF_LITE_REENTRANT 1
#if PRINTF_LITE_REENTRANT == 1  &&  PRINTF_LITE_TIMESTAMP_SUPPORT == 1

void printf_with_callback(const char* format, transmitBufCallBackType transmitBufCallBack,
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
							int* newline, int timestamp,
#endif // PRINTF_LITE_TIMESTAMP_SUPPORT
						...);

#ifdef ARDUINO
void printf_with_callback(const __FlashStringHelper* format, transmitBufCallBackType transmitBufCallBack
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
							, int* newline, int timestamp,
#endif // PRINTF_LITE_TIMESTAMP_SUPPORT
						...);
#endif // ARDUINO

#include <inttypes.h> // for PRIu32

#endif // PRINTF_LITE_REENTRANT


#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
static void print_timestamp(PrintfParams* printfParams)
{
	#ifdef USE_HAL_DRIVER
		uint32_t ms = HAL_GetTick();
	#endif
	#ifdef ARDUINO
		uint32_t ms = millis();
	#endif
	#ifdef NRF51
		uint32_t p_ticks;
		uint32_t error_code = app_timer_cnt_get(&p_ticks);
		APP_ERROR_CHECK(error_code);
		uint32_t ms = p_ticks * ( ( NRF_RTC1->PRESCALER + 1 ) * 1000 ) / APP_TIMER_CLOCK_FREQ;
	#endif
    #ifdef __APPLE__
        uint32_t ms = getUptimeInMilliseconds();
    #endif
	uint32_t s = ms/1000;
	uint32_t m = s/60;
	uint32_t h = m/60;
	m %= 60;
	s %= 60;
	ms %= 1000;
#if PRINTF_LITE_REENTRANT == 1
    #ifdef ARDUINO
        printf_with_callback(F("%03" PRIu32 ":%02" PRIu32 ":%02" PRIu32 ".%03" PRIu32 " - "), printfParams->transmitBufCallBack, NULL, 0, h, m, s, ms);
    #else
        printf_with_callback("%03" PRIu32 ":%02" PRIu32 ":%02" PRIu32 ".%03" PRIu32 " - ", printfParams->transmitBufCallBack, NULL, 0, h, m, s, ms);
    #endif
#else
    // non reentrant version take a bit more code size
	#if PRINTF_LITE_PADCHAR_SUPPORT == 1
		char pad_char = printfParams->pad_char;
		printfParams->pad_char = '0';
	#endif
	#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
		int width_specifier = printfParams->width_specifier;
		printfParams->width_specifier = 3;
	#endif
		print_longlong(h, 10, printfParams);
		print_char(':', printfParams);
	#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
		printfParams->width_specifier = 2;
	#endif
		print_longlong(m, 10, printfParams);
		print_char(':', printfParams);
		print_longlong(s, 10, printfParams);
		print_char('.', printfParams);
	#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
		printfParams->width_specifier = 3;
	#endif
		print_longlong(ms, 10, printfParams);
		// version 1
	//	for (int i=0 ; i<3 ; i++) print_char('-', printfParams);
		// version 2
	//	print_char(' ', printfParams);
	//	print_char('-', printfParams);
	//	print_char(' ', printfParams);
		// version 3
	//	print_Fstring(PSTR(" - "), printfParams);
		// version 4
		print_string(" - ", printfParams); // this one seems to use less space in flash, but 4 bytes in memory, on my Arduino UNO

	#if PRINTF_LITE_PADCHAR_SUPPORT == 1
		printfParams->pad_char = pad_char;
	#endif
	#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
		printfParams->width_specifier = width_specifier;
	#endif
#endif
}
#endif

#if PRINTF_LITE_FLOAT_SUPPORT == 1
/* Jief : I found this in Arduino code */

/* According to snprintf(),
 *
 * nextafter((double)numeric_limits<long long>::max(), 0.0) ~= 9.22337e+18
 *
 * This slightly smaller value was picked semi-arbitrarily. */
#define LARGE_DOUBLE_TRESHOLD (9.1e18)

/* THIS FUNCTION SHOULDN'T BE USED IF YOU NEED ACCURATE RESULTS.
 *
 * This implementation is meant to be simple and not occupy too much
 * code size.  However, printing floating point values accurately is a
 * subtle task, best left to a well-tested library function.
 *
 * See Steele and White 2003 for more details:
 *
 * http://kurtstephens.com/files/p372-steele.pdf
 */
static void print_double(double number, PrintfParams* printfParams)
{
    // Hackish fail-fast behavior for large-magnitude doubles
    if (number >= LARGE_DOUBLE_TRESHOLD || number <= -LARGE_DOUBLE_TRESHOLD) {
        if (number < 0.0) {
            print_char('-', printfParams);
        }
#if PRINTF_OUTPUT_FORMAT_UNICODE == 1  &&  PRINTF_UTF8_SUPPORT == 0
        print_string(L"<large double>", printfParams);
#else
		print_string((unsigned char*)"<large double>", printfParams);
#endif
    }

    int negative = 0;
    if (number < 0.0) {
    	negative = 1;
        number = -number;
    }

    // Simplistic rounding strategy so that e.g. print(1.999, 2)
    // prints as "2.00"
    double rounding = 0.5;
    #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
        for (uint8_t i = 0; i < printfParams->precision_specifier; i++) {
            rounding /= 10.0;
        }
    #else
        for (uint8_t i = 0; i < 6; i++) {
            rounding /= 10.0;
        }
    #endif
    number += rounding;

    // Extract the integer part of the number and print it
    unsigned INT_BIGGEST_TYPE int_part = (unsigned INT_BIGGEST_TYPE)number; // we're sure it's positive number here.
    double remainder = number - (double)int_part;
#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
	int width_specifier = printfParams->width_specifier;
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
	printfParams->width_specifier -= printfParams->precision_specifier + (printfParams->precision_specifier ? 1 : 0); // doesn't matter if width_specifier is negative.
  #else
	printfParams->width_specifier -= 7; // doesn't matter if width_specifier is negative.
  #endif
#endif
    print_ulonglong(int_part, 10, printfParams, negative);
#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
	printfParams->width_specifier = width_specifier;
#endif

    #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
        // Print the decimal point, but only if there are digits beyond
        if (printfParams->precision_specifier > 0) {
            print_char('.', printfParams);
        }
    #else
        // if no precision support, it's always 6, so always a decimal point.
        print_char('.', printfParams);
    #endif

    // Extract digits from the remainder one at a time
    #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
      while (printfParams->precision_specifier-- > 0) {
    #else
      for ( int i=0 ; i<6 ; i++) {
    #endif
        remainder *= 10.0;
        int to_print = (int)remainder;
        print_char((char)(to_print + '0'), printfParams);
        remainder -= to_print;
    }
}
#endif

#ifdef __APPLE__
    #define VALIST_PARAM_TYPE va_list
    #define VALIST_PARAM(valist) valist
    #define VALIST_ACCESS(valist) valist
#else
    #define VALIST_PARAM_TYPE va_list*
    #define VALIST_PARAM(valist) &valist
    #define VALIST_ACCESS(valist) (*valist)
#endif
static void printf_handle_format_char(char c, VALIST_PARAM_TYPE valist, PrintfParams* printfParams)
{
	if ( printfParams->inDirective )
	{
		switch(c)
		{
#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  ||  PRINTF_LITE_FIELDPRECISION_SUPPORT == 1  ||  PRINTF_LITE_PADCHAR_SUPPORT == 1
			case '0':
  #if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
				if ( printfParams->inWidthField ) {
					printfParams->width_specifier *= 10;
				}else
  #endif
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
				if  ( printfParams->inPrecisionField ) {
					printfParams->precision_specifier *= 10;
				}
				else
  #endif
  #if PRINTF_LITE_PADCHAR_SUPPORT == 1
				printfParams->pad_char = '0';
  #else
    #if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 0
                printfParams->inDirective = 0;
    #endif
  #endif
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1  ||  (PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1)
			 	if  ( printfParams->inPrecisionField )
				{
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1 // just ignore if we don't support precision field
					printfParams->precision_specifier *= 10;
					printfParams->precision_specifier += ( c - '0');
  #endif
				}
			 	else
  #endif
				{
  #if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
                    printfParams->inWidthField = 1;
                    printfParams->width_specifier *= 10;
                    printfParams->width_specifier += ( c - '0');
  #else
    #if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1
                    // We just have to ignore field width
    #else
                    // It's considered a mistake. Get out directive. Nothing will be printed. Harder to debug the format string for the user, but save sapce.
                    printfParams->inDirective = 0;
    #endif
  #endif
                }
			 	break;

#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
			case '.':
  #if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
				printfParams->inWidthField = 0;
  #endif
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1  ||  (PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1)
				printfParams->inPrecisionField = 1;
  #endif
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
				printfParams->precision_specifier = 0;
  #endif
				break;
#endif
#endif // PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  ||  PRINTF_LITE_FIELDPRECISION_SUPPORT == 1

#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
			case 'z':
  #if PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
				printfParams->z_modifier = 1;
  #else
                printfParams->l_modifier = 2;
  #endif
				break;
#endif // PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_ZSPECIFIER_SUPPORT == 1

#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_XSPECIFIER_SUPPORT == 1
			case 'x':
			case 'X':
  #if PRINTF_LITE_XSPECIFIER_SUPPORT == 1
				printfParams->uppercase = c == 'X';
  #if PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
				if ( printfParams->z_modifier ) print_ulonglong(va_arg(VALIST_ACCESS(valist), size_t), 16, printfParams, 0);
				else
  #endif
  #if PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier == 2 ) print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned long long int), 16, printfParams, 0);
				else
  #endif
  #if PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier != 0 ) print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned long int), 16, printfParams, 0);
				else
  #endif
					print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned int), 16, printfParams, 0);
				printfParams->inDirective = 0;
				break;
  #endif
#endif // PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_XSPECIFIER_SUPPORT == 1

#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_USPECIFIER_SUPPORT == 1
			case 'u':
  #if PRINTF_LITE_USPECIFIER_SUPPORT == 1
    #if PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
				if ( printfParams->z_modifier ) print_ulonglong(va_arg(VALIST_ACCESS(valist), size_t), 10, printfParams, 0);
				else
    #endif
  #if PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier == 2 ) print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned long long int), 10, printfParams, 0);
				else
  #endif // PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
  #if PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier != 0 ) print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned long int), 10, printfParams, 0);
				else
  #endif // PRINTF_LITE_LONGINT_SUPPORT == 1
					print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned int), 10, printfParams, 0);
				printfParams->inDirective = 0;
				break;
  #endif // PRINTF_LITE_USPECIFIER_SUPPORT
#endif // PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_USPECIFIER_SUPPORT == 1

			case 'd':
  #if PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
				if ( printfParams->z_modifier ) print_longlong(va_arg(VALIST_ACCESS(valist), size_t), 10, printfParams); // we are using longlong version for every int to save code size.
				else
  #endif
  #if PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier == 2 ) print_longlong(va_arg(VALIST_ACCESS(valist), long long int), 10, printfParams); // we are using longlong version for every int to save code size.
				else
  #endif
  #if PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier != 0 ) print_longlong(va_arg(VALIST_ACCESS(valist), long int), 10, printfParams); // we are using longlong version for every int to save code size.
				else
  #endif
					print_longlong(va_arg(VALIST_ACCESS(valist), int), 10, printfParams);
				printfParams->inDirective = 0;
				break;

#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FLOAT_SUPPORT == 1
			case 'f':
  #if PRINTF_LITE_FLOAT_SUPPORT == 1
				print_double(va_arg(VALIST_ACCESS(valist), double), printfParams); // 'float' is promoted to 'double' when passed through '...'
  #elif PRINTF_LITE_FLOAT_AS_INT_SUPPORT == 1
				print_longlong((INT_BIGGEST_TYPE)va_arg(VALIST_ACCESS(valist), double), 10, printfParams); // Cost 144 byte on Arduino
  #else
				va_arg(VALIST_ACCESS(valist), double); // this cost 16 bytes on stm32, 8 bytes on Arduino
  #endif
				printfParams->inDirective = 0;
				break;
#endif // PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FLOAT_SUPPORT == 1

#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_LONGINT_SUPPORT == 1  ||  PRINTF_LITE_FLOAT_SUPPORT == 1
			case 'l':
  #if PRINTF_LITE_LONGINT_SUPPORT == 1
				printfParams->l_modifier += 1;
  #endif
				break;
#endif
			case 'c':
#if PRINTF_OUTPUT_FORMAT_UNICODE == 1  &&  PRINTF_UTF8_SUPPORT == 1
				if ( printfParams->l_modifier == 0 ) {
					char c1 = (char)va_arg(VALIST_ACCESS(valist), int);
					print_char((wchar_t)c1, printfParams); // 'char' is promoted to 'int' when passed through '...'
					printfParams->inDirective = 0;
				}else
#endif
				{
//					wchar_t tmp2 = L'a';
//					int tmp1 = va_arg(VALIST_ACCESS(valist), int);
//					int tmp3 = va_arg(VALIST_ACCESS(valist), wchar_t);
					print_char((printf_char_type)va_arg(VALIST_ACCESS(valist), int), printfParams); // 'char' is promoted to 'int' when passed through '...'
					printfParams->inDirective = 0;
				}
				break;
//#ifdef ARDUINO
//#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FLASHSTRING_SUPPORT == 1
//			case 'F':
//  #if PRINTF_LITE_FLASHSTRING_SUPPORT == 1
//				print_Fstring(va_arg(VALIST_ACCESS(valist), char *), printfParams);
//  #endif
//				printfParams->inDirective = 0;
//				break;
//#endif //PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FLASHSTRING_SUPPORT == 1
//#else //ARDUINO
//#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FLASHSTRING_SUPPORT == 0
//			case 'F':
//#endif
//#endif

#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_FLASHSTRING_SUPPORT == 1
			case 'F':
#endif
#if defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1
				print_Fstring(va_arg(VALIST_ACCESS(valist), char *), printfParams);
				printfParams->inDirective = 0;
				break;
#endif //defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1
			case 's':
#if PRINTF_OUTPUT_FORMAT_UNICODE == 1
#endif
#if PRINTF_OUTPUT_FORMAT_UNICODE == 1  &&  PRINTF_UTF8_SUPPORT == 1
				if ( printfParams->l_modifier == 0 ) {
					print_string(va_arg(VALIST_ACCESS(valist), unsigned char *), printfParams);
					printfParams->inDirective = 0;
				}else
#endif
				{
					print_string(va_arg(VALIST_ACCESS(valist), printf_char_type *), printfParams);
					printfParams->inDirective = 0;
				}
				break;
			default:  {
				print_char('%', printfParams);
				if ( c != '%' ) print_char(c, printfParams);
				printfParams->inDirective = 0;
			}
		}
	}
	else
	{
		if ( c == '%' )
		{
				printfParams->inDirective = 1;
				printfParams->l_modifier = 0;
  #if PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
				printfParams->z_modifier = 0;
  #endif
  #if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
				printfParams->inWidthField = 0;
				printfParams->width_specifier = 0;
  #endif
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1  ||  (PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1)
				printfParams->inPrecisionField = 0;
  #endif
  #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
				printfParams->precision_specifier = 6; // 6 digits for float, as specified by ANSI, if I remeber well
  #endif
  #if PRINTF_LITE_PADCHAR_SUPPORT == 1
				printfParams->pad_char = ' ';
  #endif
		}
		else
		{
			print_char(c, printfParams);
		}
	}
}

void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
							, int* newline, int timestamp
#endif
						)
{
	PrintfParams printfParams;
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
	printfParams.newlinePtr = newline;
	printfParams.timestamp = timestamp;
  #endif
  #if PRINTF_LITE_BUF_SIZE > 1
	printfParams.bufIdx = 0;
  #endif
	printfParams.inDirective = 0;
	printfParams.transmitBufCallBack = transmitBufCallBack;

	while ( 1 ) //Iterate over formatting string
	{
		char c = *format++;
		if (c == 0)	break;
		printf_handle_format_char(c, VALIST_PARAM(valist), &printfParams);
	}
  #if PRINTF_LITE_BUF_SIZE > 1
	if ( printfParams.bufIdx > 0 ) printfParams.transmitBufCallBack(printfParams.buf, printfParams.bufIdx);
  #endif

	va_end(valist);
}

void printf_with_callback(const char* format, transmitBufCallBackType transmitBufCallBack,
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
							int* newline, int timestamp,
#endif
						...)
{
	va_list valist;
    #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
        va_start(valist, timestamp);
    #else
        va_start(valist, transmitBufCallBack);
    #endif
	vprintf_with_callback(format, valist,transmitBufCallBack
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, NULL, 0
  #endif
                         );
	va_end(valist);
}


#if defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1

void vprintf_with_callback(const __FlashStringHelper *format, va_list valist, transmitBufCallBackType transmitBufCallBack
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
							, int* newline, int timestamp
#endif
						)
{
	PrintfParams printfParams;
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
	printfParams.newlinePtr = newline;
	printfParams.timestamp = timestamp;
  #endif
  #if PRINTF_LITE_BUF_SIZE > 1
	printfParams.bufIdx = 0;
  #endif
	printfParams.inDirective = 0;
	printfParams.transmitBufCallBack = transmitBufCallBack;

	PGM_P p = reinterpret_cast<PGM_P>(format);
	while (1)
	{
		char c = pgm_read_byte(p++);
		if (c == 0)	break;
		printf_handle_format_char(c, &valist, &printfParams);
	}
  #if PRINTF_LITE_BUF_SIZE > 1
	if ( printfParams.bufIdx > 0 ) printfParams.transmitBufCallBack(printfParams.buf, printfParams.bufIdx);
  #endif

	va_end(valist);
}

void printf_with_callback(const __FlashStringHelper* format, transmitBufCallBackType transmitBufCallBack,
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
					        int* newline, int timestamp,
#endif
					        ...)
{
	va_list valist;
    #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
        va_start(valist, timestamp);
    #else
        va_start(valist, transmitBufCallBack);
    #endif
	vprintf_with_callback(format, valist, transmitBufCallBack
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, NULL, 0
  #endif
                         );
	va_end(valist);
}

#endif // defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1

/* ------------------------ SPRINTF ------------------------ */

#if PRINTF_LITE_SNPRINTF_SUPPORT == 1

char* sprintfBuf;
size_t sprintfBufLen;

void transmitSprintf(const char* buf, size_t nbyte)
{
	size_t i=0;
	for ( ; sprintfBufLen>0  &&  i<nbyte ; i++) {
		*sprintfBuf++ = buf[i];
		sprintfBufLen--;
	}
}

int vsnprintf(char *__restrict buf, size_t len, const char *__restrict format, va_list valist)
{
	sprintfBuf = buf;
	sprintfBufLen = len-1;
	vprintf_with_callback(format, valist, transmitSprintf
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, NULL, 0
  #endif
                                   );
	*sprintfBuf = 0;
	return 1;
}

int snprintf(char *__restrict buf, size_t len, const char *__restrict format, ...)
{
	va_list valist;
	va_start(valist, format);
	vsnprintf(buf, len, format, valist);
	va_end(valist);
	return 1;
}

#if defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1

int vsnprintf(char *__restrict buf, size_t len, const __FlashStringHelper *__restrict format, va_list valist)
{
	sprintfBuf = buf;
	sprintfBufLen = len-1;
	vprintf_with_callback(format, valist, transmitSprintf
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, NULL, 0
  #endif
                                   );
	*sprintfBuf = 0;
	return 1;
}

int snprintf(char *__restrict buf, size_t len, const __FlashStringHelper *__restrict format, ...)
{
	va_list valist;
	va_start(valist, format);
	vsnprintf(buf, len, format, valist);
	va_end(valist);
	return 1;
}
#endif // defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1
#endif // PRINTF_LITE_SNPRINTF_SUPPORT

#if PRINTF_LITE_USB_SUPPORT == 1
#ifdef __USBD_CDC_IF_H
/* ------------------------ USB ------------------------ */

void transmitBufUsb(const char* buf, size_t nbyte)
{
	#ifdef DEBUG
		#if PRINTF_LITE_BUF_SIZE == 0
			if ( nbyte > 1 ) {
				__asm volatile ("bkpt 0");
			}
		#else
			if ( nbyte > PRINTF_LITE_BUF_SIZE ) {
				__asm volatile ("bkpt 0");
			}
		#endif
	#endif
//	uint32_t ms = HAL_GetTick() + 3000;
//	while ( HAL_GetTick() < ms  &&  CDC_Transmit_FS((char*)buf, nbyte) == USBD_BUSY ) {}; // If data isn't consumed by the host in 1 ms, it probaly won't be so we give up to not block.


	uint32_t t0 = HAL_GetTick();
	uint8_t ret = CDC_Transmit_FS((uint8_t*)buf, (uint16_t)nbyte); // nbbyte is < BUF_MAX so it's safe to assume we can cast it to uint16_t.
	if ( ret != USBD_OK )
	{
	    uint32_t now = HAL_GetTick();
		while ( now - t0 < 300  &&  ret != USBD_OK ) { // What to do if can't send ? With uart, there no flow control so char can get lost.
			ret = CDC_Transmit_FS((uint8_t*)buf, (uint16_t)nbyte);
			now = HAL_GetTick();
		}
	}
}

#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
static int usb_newline = 1;
#endif

void printf_usb(const char* format, ...)
{
	va_list valist;
	va_start(valist, format);
	vprintf_with_callback(format, valist, transmitBufUsb
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
										, &usb_newline, 0
  #endif
								   );
	va_end(valist);
}

void vlogf_usb(const char* format, va_list valist)
{
	vprintf_with_callback(format, valist, transmitBufUsb
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, &usb_newline, 1
  #endif
								);
}

void logf_usb(const char* format, ...)
{
	va_list valist;
	va_start(valist, format);
	vlogf_usb(format, valist);
	va_end(valist);
}
#endif

#endif // PRINTF_LITE_USB_SUPPORT

/* ------------------------ SERIAL ------------------------ */
#if PRINTF_LITE_UART_SUPPORT == 1

#ifdef __STM32F1xx_HAL_UART_H // STM32

	static void transmitBufUart(const char* buf, size_t nbyte)
	{
		int ret = HAL_UART_Transmit(&huart1, (uint8_t*)buf, (uint16_t)nbyte, 1000); // nbbyte is < BUF_MAX so it's safe to assume we can cast it to uint16_t.

		if ( ret != HAL_OK ) // this is only to be able to put a breakpoint in case the first HAL_UART_Transmit return !HAL_OK
		{
			while ( ret != HAL_OK )
			{
				ssize_t nbByteNotTransmitted = huart1.TxXferCount + (ret == HAL_TIMEOUT ? 1 : 0); // size to transmit.
				if ( nbByteNotTransmitted <= 0 ) {
					#ifdef DEBUG
						__asm volatile ("bkpt 0");
					#endif
				}
				buf += nbyte - nbByteNotTransmitted;
				nbyte = nbByteNotTransmitted;
				ret = HAL_UART_Transmit(&huart1, (uint8_t*)buf, (uint16_t)nbyte, 1000);
			}
		}
	}

#endif

#ifdef ARDUINO

	static void transmitBufUart(const char* buf, size_t nbyte)
	{
        for (size_t i=0 ; i<nbyte ; i++) {
            Serial.write(buf[i]);
        }
	}

#endif

#ifdef NRF51

	static void transmitBufUart(const char* buf, size_t nbyte)
	{
		for (size_t i=0 ; i<nbyte ; i++) {
			do {} while ( app_uart_put(buf[i]) != NRF_SUCCESS);
		}
	}

#endif

#ifdef __APPLE__

// so far, this is just for testing and debugging. It sends all to console.
#include <stdio.h>
	static void transmitBufUart(const char* buf, size_t nbyte)
	{
		for (size_t i=0 ; i<nbyte ; i++) {
            putchar(buf[i]);
		}
	}

#endif


#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
static int uart_newline = 1; // newline is static avoid being reinitialized at each call
#endif

void vprintf_uart(const char* format, va_list valist)
{
	vprintf_with_callback(format, valist, transmitBufUart
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, &uart_newline, 0
  #endif
								  );
}

void printf_uart(const char* format, ...)
{
	va_list valist;
	va_start(valist, format);
	vprintf_uart(format, valist);
	va_end(valist);
}

void vlogf_uart(const char* format, va_list valist)
{
	vprintf_with_callback(format, valist, transmitBufUart
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, &uart_newline, 1
  #endif
								);
}

void logf_uart(const char* format, ...)
{
	va_list valist;
	va_start(valist, format);
	vlogf_uart(format, valist);
	va_end(valist);
}

#if defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1

void vprintf_uart(const __FlashStringHelper* format, va_list valist)
{
	vprintf_with_callback(format, valist, transmitBufUart
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, &uart_newline, false
  #endif
								  );
}

void printf_uart(const __FlashStringHelper *format, ...)
{
	va_list valist;
	va_start(valist, format);
	vprintf_uart(format, valist);
	va_end(valist);
}

void vlogf_uart(const __FlashStringHelper* format, va_list valist)
{
	vprintf_with_callback(format, valist, transmitBufUart
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, &uart_newline, true
  #endif
								);
}

void logf_uart(const __FlashStringHelper *format, ...)
{
	va_list valist;
	va_start(valist, format);
	vlogf_uart(format, valist);
	va_end(valist);
}

#endif // defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1

#endif // PRINTF_LITE_UART_SUPPORT

/* ------------------------ TRACE ------------------------ */

#if defined(OS_USE_TRACE_ITM) || defined(OS_USE_TRACE_SEMIHOSTING_DEBUG) || defined(OS_USE_TRACE_SEMIHOSTING_STDOUT)

#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
static int newlinePtr = 1;
#endif

void my_trace_write(const char* buf ,size_t nbyte)
{
	trace_write(buf, nbyte);
}

void vprintf_semih(const char* format, va_list valist)
{
	vprintf_with_callback(format, valist, my_trace_write
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
										, &newlinePtr, 1
  #endif
								);

//	// TODO: rewrite it to no longer use newlib, it is way too heavy
//	// Print to the local buffer
//	ret = vsnprintf(buf, sizeof(buf), format, ap);
//	if (ret > 0) {
//		// Transfer the buffer to the device
//		ret = trace_write(buf, (size_t) ret);
//	}
//
}

void printf_semih(const char* format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);
	vprintf_semih(format, ap);
	va_end(ap);
}

#endif



/* ----------------------  LCD  --------------------------- */
#ifdef LiquidCrystal_h

extern LiquidCrystal lcd;
static int lcd_newline = 1; // newline is static avoid being reinitialized at each call

static void transmitBufLcd(const char* buf, size_t nbyte)
{
//	lcd.write(buf, nbyte);
	for (size_t i=0 ; i<nbyte ; i++) {
		lcd.write(buf[i]);
	}
}

void printf_lcd(int row, int col, const char *format, ...)
{
	lcd.setCursor(col, row);

	va_list valist;
	va_start(valist, format);
	vprintf_with_callback(format, valist, transmitBufLcd
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, &lcd_newline, 0
  #endif
								);
	va_end(valist);
}

#if defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1

void printf_lcd(int row, int col, const __FlashStringHelper *format, ...)
{
	lcd.setCursor(col, row);

	va_list valist;
	va_start(valist, format);
	vprintf_with_callback(format, valist, transmitBufLcd
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, &lcd_newline, 0
  #endif
								);

	va_end(valist);
}

#endif // defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1
#endif

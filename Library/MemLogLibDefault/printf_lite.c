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

#include "printf_lite.h"

//#include <stdio.h>
#include <stdarg.h>
//#include <stdint.h>
#include <limits.h>
#include <stdlib.h>


#if defined(__cplusplus)
extern "C"
{
#endif

#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
static void print_timestamp(PrintfParams* printfParams);
#endif


#if PRINTF_UTF8_OUTPUT_SUPPORT == 1  &&  PRINTF_UNICODE_OUTPUT_SUPPORT == 1
	#define print_char_macro(c, printfParams) printfParams->printCharFunction(c, printfParams);
#elif PRINTF_UNICODE_OUTPUT_SUPPORT == 1
	#define print_char_macro(c, printfParams) print_wchar(c, printfParams);
#elif PRINTF_UTF8_OUTPUT_SUPPORT == 1
	#define print_char_macro(c, printfParams) print_utf8_char(c, printfParams);
#endif

typedef struct PrintfParams PrintfParams;
#if PRINTF_UTF8_OUTPUT_SUPPORT == 1  &&  PRINTF_UNICODE_OUTPUT_SUPPORT == 1
	typedef void (*printCharType)(int c, PrintfParams* printfParams);
#endif

// using int for flags because this is what generate less code. It's a bit more on the stack but it's temporary.
typedef struct PrintfParams {
  #if PRINTF_LITE_BUF_SIZE > 1
	union {
  #if PRINTF_UTF8_OUTPUT_SUPPORT == 1
  # if PRINTF_UNICODE_OUTPUT_SUPPORT == 1
		char buf[PRINTF_LITE_BUF_SIZE*sizeof(wchar_t)];
  # else
		char buf[PRINTF_LITE_BUF_SIZE];
  # endif
  #endif
  #if PRINTF_UNICODE_OUTPUT_SUPPORT == 1
		wchar_t wbuf[PRINTF_LITE_BUF_SIZE];
  #endif
	} buf;
	unsigned char bufIdx;
  #endif
	printf_callback_t transmitBufCallBack;
	#if PRINTF_UTF8_OUTPUT_SUPPORT == 1  &&  PRINTF_UNICODE_OUTPUT_SUPPORT == 1
	// using a print_char function pointer forces to declare print_char and print_wchar the same prototype, withc is an int
	// that breaks compiler type checking. If print_char is called with a char > 255, that won't work !
	// if this is compiled with short-wchar and print_wchar is called with a char > 0xFFFF, that won't work !
	int unicode_output;
	printCharType printCharFunction;
	#endif
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
  void* context;
} PrintfParams;


#if PRINTF_UTF8_OUTPUT_SUPPORT == 1
// Print a char as is. No analyse is made to check if it's a utf8 partial char
// c is an int for prototype compatibility, but must be < 255
static void print_utf8_char(int c, PrintfParams* printfParams)
{
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
		if ( printfParams->newlinePtr )
		{
			if ( *printfParams->newlinePtr )
			{
				*printfParams->newlinePtr = 0; // to do BEFORE call to printTimeStamp
				if ( printfParams->timestamp ) print_timestamp(printfParams);
			}
			#if PRINTF_EMIT_CR == 1
				if ( c == '\n' ) print_utf8_char('\r', printfParams);
			#endif
			#if PRINTF_LITE_BUF_SIZE > 1
				printfParams->buf.buf[(printfParams->bufIdx)++] = (char)c;
			#else
				printfParams->transmitBufCallBack(&c, 1);
			#endif
			if ( c == '\n' ) *printfParams->newlinePtr = 1;
		}else{
			#if PRINTF_EMIT_CR == 1
				if ( c == '\n' ) print_utf8_char('\r', printfParams);
			#endif
			#if PRINTF_LITE_BUF_SIZE > 1
					printfParams->buf.buf[(printfParams->bufIdx)++] = (char)c;
			#else
					printfParams->transmitBufCallBack(&c, 1);
			#endif
		}
  #else
		{
			#if PRINTF_EMIT_CR == 1
				if ( c == '\n' ) print_utf8_char('\r', printfParams);
			#endif
			#if PRINTF_LITE_BUF_SIZE > 1
					printfParams->buf.buf[(printfParams->bufIdx)++] = (char)c;
			#else
					printfParams->transmitBufCallBack.transmitBufCallBack((const char*)&c, (size_t)1);
			#endif
		}
  #endif
  #if PRINTF_LITE_BUF_SIZE > 1
		if ( printfParams->bufIdx == PRINTF_LITE_BUF_SIZE ) {
			printfParams->transmitBufCallBack.transmitBufCallBack(printfParams->buf.buf, printfParams->bufIdx, printfParams->context);
			printfParams->bufIdx = 0;
		}
  #endif
}
#endif

#if PRINTF_UNICODE_OUTPUT_SUPPORT == 1
// print wchar_t char as is. No check is made if it's surrogate or not. Just send the wchar_t as is.
// c is an int for prototype compatibility, but it's a wchar_t. Assumption : a wchar_t cannot be bigger than an int. Don't know yet if it's 100% on embedded platform.
static void print_wchar(int c, PrintfParams* printfParams)
{
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
		if ( printfParams->newlinePtr )
		{
			if ( *printfParams->newlinePtr )
			{
				*printfParams->newlinePtr = 0; // to do BEFORE call to printTimeStamp
				if ( printfParams->timestamp ) print_timestamp(printfParams);
			}
			#if PRINTF_EMIT_CR == 1
				if ( c == '\n' ) print_wchar('\r', printfParams);
			#endif
			#if PRINTF_LITE_BUF_SIZE > 1
					printfParams->buf.wbuf[(printfParams->bufIdx)++] = (wchar_t)c;
			#else
					printfParams->transmitWBufCallBack(&c, 1);
			#endif
			if ( c == '\n' ) {
				*printfParams->newlinePtr = 1;
			}
		}else{
			#if PRINTF_EMIT_CR == 1
				if ( c == '\n' ) print_wchar('\r', printfParams);
			#endif
			#if PRINTF_LITE_BUF_SIZE > 1
					printfParams->buf.wbuf[(printfParams->bufIdx)++] = (wchar_t)c;
			#else
					printfParams->transmitWBufCallBack(&c, 1);
			#endif
		}
  #else
    {
			#if PRINTF_EMIT_CR == 1
				if ( c == '\n' ) print_wchar('\r', printfParams);
			#endif
			#if PRINTF_LITE_BUF_SIZE > 1
					printfParams->buf.wbuf[(printfParams->bufIdx)++] = (wchar_t)c; // cast suposed to be safe, as this function must be called
			#else
					printfParams->transmitWBufCallBack(&c, 1);
			#endif
    }
  #endif
  #if PRINTF_LITE_BUF_SIZE > 1
	if ( printfParams->bufIdx == PRINTF_LITE_BUF_SIZE ) {
		printfParams->transmitBufCallBack.transmitWBufCallBack(printfParams->buf.wbuf, printfParams->bufIdx, printfParams->context);
		printfParams->bufIdx = 0;
	}
  #endif
}
#endif








#if PRINTF_UTF8_OUTPUT_SUPPORT == 1 && PRINTF_UNICODE_INPUT_SUPPORT == 1

#define halfBase 0x0010000UL
#define halfMask 0x3FFUL
#define halfShift 10 /* used for shifting by 10 bits */
#define UNI_SUR_HIGH_START  0xD800u
#define UNI_SUR_LOW_START   0xDC00u

#if __WCHAR_MAX__ <= 0xFFFFu
static inline int printf_is_surrogate(char16_t uc) { return (uc - 0xd800u) < 2048u; }
static inline int printf_is_high_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xd800; }
static inline int printf_is_low_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

static inline char32_t printf_surrogate_to_utf32(char16_t high, char16_t low) {
    return (char32_t)((high << 10) + low - 0x35fdc00); // Safe cast, it fits in 32 bits
}
#endif

/*
 * Print char32_t to utf8 string.
 * Only needed if PRINTF_UNICODE_OUTPUT_SUPPORT == 1 && PRINTF_UTF8_INPUT_SUPPORT == 1
 */
static void print_char32_as_utf8_string(const char32_t utf32_char, PrintfParams* printfParams)
{
	/* assertion: utf32_char is a single UTF-4 value */
	int bits;
	
	if (utf32_char < 0x80) {
		print_utf8_char((char)utf32_char, printfParams);
		bits = -6;
	}
	else if (utf32_char < 0x800) {
		print_utf8_char((char)(((utf32_char >> 6) & 0x1F) | 0xC0), printfParams);
		bits = 0;
	}
	else if (utf32_char < 0x10000) {
		print_utf8_char((char)(((utf32_char >> 12) & 0x0F) | 0xE0), printfParams);
		bits = 6;
	}
	else {
		print_utf8_char((char)(((utf32_char >> 18) & 0x07) | 0xF0), printfParams);
		bits = 12;
	}
	for (; bits >= 0; bits -= 6) {
		print_utf8_char((char)(((utf32_char >> bits) & 0x3F) | 0x80), printfParams);
	}
}

/*
 * Print wchar string to utf8 string.
 * Only needed if PRINTF_UNICODE_OUTPUT_SUPPORT == 1 && PRINTF_UTF8_INPUT_SUPPORT == 1
 */
static void output_wchar_string_as_utf8(const wchar_t* s, PrintfParams* printfParams)
{
	if ( !s ) return;
	int width_specifier = printfParams->width_specifier;
	while ( *s  &&  (printfParams->width_specifier == 0  ||  width_specifier--) ) {
//	while ( *s ) {
#if __WCHAR_MAX__ <= 0xFFFFu
        const char16_t uc = *s++;
        if (!printf_is_surrogate(uc)) {
			print_char32_as_utf8_string((char32_t)uc, printfParams);
        } else {
            if (printf_is_high_surrogate(uc) && *s && printf_is_low_surrogate(*s)) {
				print_char32_as_utf8_string(printf_surrogate_to_utf32(uc, *s++), printfParams);
            } else {
				continue;
			}
        }
#else
		print_char32_as_utf8_string((char32_t)(*s++), printfParams);
#endif
	}
}

#endif


/*

UTF8
 Number    Bits for		First       Last
 of bytes  code point	code point	code point	Byte 1		Byte 2		Byte 3		Byte 4
 

    1		7			U+0000		U+007F		0xxxxxxx
    2		11			U+0080		U+07FF		110xxxxx	10xxxxxx
    3		16			U+0800		U+FFFF		1110xxxx	10xxxxxx	10xxxxxx
    4		21			U+10000		U+10FFFF	11110xxx	10xxxxxx	10xxxxxx	10xxxxxx




*/






#if PRINTF_UNICODE_OUTPUT_SUPPORT == 1 && PRINTF_UTF8_INPUT_SUPPORT == 1

/*
 * Print UTF8 string to wchar string.
 * Only needed if PRINTF_UNICODE_OUTPUT_SUPPORT == 1 && PRINTF_UTF8_INPUT_SUPPORT == 1
 */
static void output_utf8_string_as_wchar(const char* s, PrintfParams* printfParams)
{
	if ( !s ) return;
	int width_specifier = printfParams->width_specifier;
	while ( *s  &&  (printfParams->width_specifier == 0  ||  width_specifier--) ) {
		char32_t c;
		if (  *((unsigned char*)s) & 0x80  ) {
			if (*(s+1) == 0) {
				// Finished in the middle of an utf8 multibyte char
				return;
			}
			if ((*(((unsigned char*)s)+1) & 0xc0) != 0x80) {
				s += 1;
				continue;
			}
			if ((*((unsigned char*)s) & 0xe0) == 0xe0) {
				if (*(s+2) == 0) {
					// Finished in the middle of an utf8 multibyte char
					return;
				}
				if ((*(((unsigned char*)s)+2) & 0xc0) != 0x80) {
					s += 2;
					continue;
				}
				if ((*((unsigned char*)s) & 0xf0) == 0xf0) {
					if (*(s+3) == 0) {
						// Finished in the middle of an utf8 multibyte char
						return;
					}
					if ((*((unsigned char*)s) & 0xf8) != 0xf0 || (*(((unsigned char*)s)+3) & 0xc0) != 0x80) {
						s += 3;
						continue;
					}
					/* 4-byte code */
					c = (char32_t)((*((char32_t*)s) & 0x7) << 18);
					c |= (char32_t)((*(((unsigned char*)s)+1) & 0x3f) << 12);
					c |= (char32_t)((*(((unsigned char*)s)+2) & 0x3f) << 6);
					c |= *(((unsigned char*)s)+3) & 0x3f;
					s += 4;
				} else {
					/* 3-byte code */
					c = (char32_t)((*((unsigned char*)s) & 0xf) << 12);
					c |= (char32_t)((*(((unsigned char*)s)+1) & 0x3f) << 6);
					c |= *(((unsigned char*)s)+2) & 0x3f;
					s += 3;
				}
			} else {
				/* 2-byte code */
				c = (char32_t)((*((unsigned char*)s) & 0x1f) << 6);
				c |= *(((unsigned char*)s)+1) & 0x3f;
				s += 2;
			}
		} else {
			/* 1-byte code */
			c = *((unsigned char*)s);
			s += 1;
		}
#if __WCHAR_MAX__ > 0xFFFFu
		print_wchar((wchar_t)c, printfParams);
#else
		if ( c <= 0xFFFF) {
				print_wchar((wchar_t)c, printfParams);
		} else {
				c -= halfBase;
				print_wchar((wchar_t)((c >> halfShift) + UNI_SUR_HIGH_START), printfParams);
				print_wchar((wchar_t)((c & halfMask) + UNI_SUR_LOW_START), printfParams);
		}
#endif
	}
}
#endif

/*
 * Print string with no conversion
 */
#if DEFINE_SECTIONS == 1
__attribute__((noinline, section(".output_utf8_string")))
#elif DEFINE_SECTIONS == 2
__attribute__((noinline, section(".printf_lite")))
#endif
static void output_utf8_string_as_utf8(const char* s, PrintfParams* printfParams)
{
	if ( !s ) return;
	if ( printfParams->width_specifier ) while ( *s && printfParams->width_specifier-- ) print_utf8_char(*s++, printfParams);
	else while ( *s ) print_utf8_char(*s++, printfParams);
}

#if DEFINE_SECTIONS == 1
__attribute__((noinline, section(".output_wchar_string")))
#elif DEFINE_SECTIONS == 2
__attribute__((noinline, section(".printf_lite")))
#endif
#if PRINTF_UNICODE_OUTPUT_SUPPORT
static void output_wchar_string(const wchar_t* s, PrintfParams* printfParams)
{
	if ( !s ) return;
	if ( printfParams->width_specifier ) while ( *s && printfParams->width_specifier-- ) print_wchar(*s++, printfParams);
	else while ( *s ) print_wchar(*s++, printfParams);
}
#endif

#if defined(ARDUINO) && PRINTF_LITE_FLASHSTRING_SUPPORT == 1

static void print_Fstring(const char* s, PrintfParams* printfParams)
{
	PGM_P p_to_print = reinterpret_cast<PGM_P>(s);
	unsigned char c_to_print = pgm_read_byte(p_to_print++);
	while ( c_to_print != 0 ) {
		print_char_macro(c_to_print, printfParams);
		c_to_print = pgm_read_byte(p_to_print++);
	}
}

#endif

#if PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
	#define INT_BIGGEST_TYPE long long int
	#define UINT_BIGGEST_TYPE unsigned long long int
#else
	#if PRINTF_LITE_LONGINT_SUPPORT == 1
		#define INT_BIGGEST_TYPE long int
		#define UINT_BIGGEST_TYPE unsigned long int
	#else
		#define INT_BIGGEST_TYPE int
		#define UINT_BIGGEST_TYPE unsigned int
	#endif
#endif

/* Jief : I found this here : https://github.com/cjlano/tinyprintf/blob/master/tinyprintf.c. Thanks CJlano */
#if DEFINE_SECTIONS == 1
__attribute__((noinline, section(".print_ulonglong")))
#elif DEFINE_SECTIONS == 2
__attribute__((noinline, section(".printf_lite")))
#endif
static void print_ulonglong(UINT_BIGGEST_TYPE v, unsigned int base, PrintfParams* printfParams, int printfSign)
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
			if ( printfSign  &&  printfParams->pad_char != ' ' ) print_char_macro('-', printfParams);
		#endif
		#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
			while ( printfParams->width_specifier > nbDigits ) {
				#if PRINTF_LITE_PADCHAR_SUPPORT == 1
					print_char_macro(printfParams->pad_char, printfParams);
				#else
					printfParams->printCharFunction(' ', printfParams);
				#endif
				nbDigits += 1;
			}
		#endif
		#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  &&  PRINTF_LITE_PADCHAR_SUPPORT == 1
			if ( printfSign  &&  printfParams->pad_char == ' ' ) print_char_macro('-', printfParams);
		#else
				if ( printfSign ) print_char_macro('-', printfParams);
		#endif
		while (d != 0) {
			unsigned int dgt = (unsigned int)(v / d); // cast is safe as v/d < 10
			v %= d;
			d /= base;
	#if PRINTF_LITE_XSPECIFIER_SUPPORT == 1
			print_char_macro( (char)(dgt + (dgt < 10 ? '0' : (printfParams->uppercase ? 'A' : 'a') - 10)), printfParams);
	#else
			print_char_macro( (char)(dgt + '0'), printfParams);
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


#if DEFINE_SECTIONS == 1
__attribute__((noinline, section(".print_longlong")))
#elif DEFINE_SECTIONS == 2
__attribute__((noinline, section(".printf_lite")))
#endif
static void print_longlong(INT_BIGGEST_TYPE v, unsigned int base, PrintfParams* printfParams)
{
	if ( v >= 0 ) print_ulonglong((UINT_BIGGEST_TYPE)v, base, printfParams, 0); // cast ok, v >= 0
	else print_ulonglong((UINT_BIGGEST_TYPE)-v, base, printfParams, 1); // -(INT64_MIN) == INT64_MIN !!! But cast as UINT64, it becomes +v. Good for us.
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
		printfParams->printCharFunction(':', printfParams);
	#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
		printfParams->width_specifier = 2;
	#endif
		print_longlong(m, 10, printfParams);
		printfParams->printCharFunction(':', printfParams);
		print_longlong(s, 10, printfParams);
		printfParams->printCharFunction('.', printfParams);
	#if PRINTF_LITE_FIELDWIDTH_SUPPORT == 1
		printfParams->width_specifier = 3;
	#endif
		print_longlong(ms, 10, printfParams);
	    printfParams->printCharFunction(' ', printfParams);
		printfParams->printCharFunction('-', printfParams);
		printfParams->printCharFunction(' ', printfParams);

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
        	print_char_macro('-', printfParams);
        }
#if PRINTF_UNICODE_OUTPUT_SUPPORT == 1  &&  PRINTF_UTF8_OUTPUT_SUPPORT == 0
        output_utf8_string(L"<large double>", printfParams);
#else
		output_utf8_string_as_utf8("<large double>", printfParams);
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
        for (int i = 0; i < printfParams->precision_specifier; i++) {
            rounding /= 10.0;
        }
    #else
        for (unsigned int  i = 0; i < 6; i++) {
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
        	print_char_macro('.', printfParams);
        }
    #else
        // if no precision support, it's always 6, so always a decimal point.
        printfParams->printCharFunction('.', printfParams);
    #endif

    // Extract digits from the remainder one at a time
    #if PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
      while (printfParams->precision_specifier-- > 0) {
    #else
      for ( int i=0 ; i<6 ; i++) {
    #endif
        remainder *= 10.0;
        int to_print = (int)remainder;
        print_char_macro((char)(to_print + '0'), printfParams);
        remainder -= to_print;
    }
}
#endif

#if DEFINE_SECTIONS == 1
__attribute__((noinline, section(".printf_handle_format_char")))
#elif DEFINE_SECTIONS == 2
__attribute__((noinline, section(".printf_lite")))
#endif
void printf_handle_format_char(char c, VALIST_PARAM_TYPE valist, PrintfParams* printfParams)
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
                printfParams->l_modifier = 12;
  #endif
				break;
#endif // PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_ZSPECIFIER_SUPPORT == 1

#if PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_XSPECIFIER_SUPPORT == 1
			case 'x':
			case 'X':
  #if PRINTF_LITE_XSPECIFIER_SUPPORT == 1
				printfParams->uppercase = c == 'X';
  #if PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
				if ( printfParams->z_modifier ) {
					print_ulonglong(va_arg(VALIST_ACCESS(valist), size_t), 16, printfParams, 0);
				}else
  #endif
  #if PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier > 11 ) {
					print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned long long int), 16, printfParams, 0);
				}else
  #endif
  #if PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier == 11 ) {
					print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned long int), 16, printfParams, 0);
				}else
  #endif
  #if PRINTF_LITE_SHORTINT_SUPPORT == 1
				if ( printfParams->l_modifier == 9 ) {
					print_ulonglong((unsigned short)va_arg(VALIST_ACCESS(valist), unsigned int), 16, printfParams, 0); // we are using longlong version for every int to save code size.
				}else
  #endif
  #if PRINTF_LITE_SHORTSHORTINT_SUPPORT == 1
				if ( printfParams->l_modifier < 9 ) {
					print_ulonglong((unsigned char)va_arg(VALIST_ACCESS(valist), unsigned int), 16, printfParams, 0); // we are using longlong version for every int to save code size.
				}else
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
				if ( printfParams->l_modifier > 11 ) print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned long long int), 10, printfParams, 0);
				else
  #endif // PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
  #if PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier == 11 ) print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned long int), 10, printfParams, 0);
				else
  #endif // PRINTF_LITE_LONGINT_SUPPORT == 1
  #if PRINTF_LITE_SHORTINT_SUPPORT == 1
				if ( printfParams->l_modifier == 9 ) {
					print_ulonglong((unsigned short)va_arg(VALIST_ACCESS(valist), unsigned int), 10, printfParams, 0); // we are using longlong version for every int to save code size.
				}else
  #endif
  #if PRINTF_LITE_SHORTSHORTINT_SUPPORT == 1
				if ( printfParams->l_modifier < 9 ) {
					print_ulonglong((unsigned char)va_arg(VALIST_ACCESS(valist), unsigned int), 10, printfParams, 0); // we are using longlong version for every int to save code size.
				}else
  #endif
					print_ulonglong(va_arg(VALIST_ACCESS(valist), unsigned int), 10, printfParams, 0);
				printfParams->inDirective = 0;
				break;
  #endif // PRINTF_LITE_USPECIFIER_SUPPORT
#endif // PRINTF_LITE_FALLBACK_FOR_UNSUPPORTED == 1  ||  PRINTF_LITE_USPECIFIER_SUPPORT == 1

			case 'd':
  #if PRINTF_LITE_ZSPECIFIER_SUPPORT == 1
				if ( printfParams->z_modifier ) print_ulonglong((UINT_BIGGEST_TYPE)va_arg(VALIST_ACCESS(valist), size_t), (unsigned int)10, printfParams, 0); // we are using longlong version for every int to save code size.
				else
  #endif
  #if PRINTF_LITE_LONGLONGINT_SUPPORT == 1  &&  PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier > 11 ) print_longlong(va_arg(VALIST_ACCESS(valist), long long int), 10, printfParams); // we are using longlong version for every int to save code size.
				else
  #endif
  #if PRINTF_LITE_LONGINT_SUPPORT == 1
				if ( printfParams->l_modifier == 11 ) print_longlong(va_arg(VALIST_ACCESS(valist), long int), 10, printfParams); // we are using longlong version for every int to save code size.
				else
  #endif
//  #if PRINTF_LITE_SHORTINT_SUPPORT == 1
//				if ( printfParams->l_modifier == 9 ) print_longlong(va_arg(VALIST_ACCESS(valist), short int), 10, printfParams); // we are using longlong version for every int to save code size.
//				else
//  #endif
//  #if PRINTF_LITE_SHORTSHORTINT_SUPPORT == 1
//				if ( printfParams->l_modifier < 9 ) print_longlong(va_arg(VALIST_ACCESS(valist), char), 10, printfParams); // we are using longlong version for every int to save code size.
//				else
//  #endif
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
			case 'h':
  #if PRINTF_LITE_LONGINT_SUPPORT == 1
				printfParams->l_modifier -= 1;
  #endif
				break;
#endif
			case 'c':
			{
				int c1 = va_arg(VALIST_ACCESS(valist), int);
#if PRINTF_UTF8_INPUT_SUPPORT == 1
				{
//					wchar_t tmp2 = L'a';
//					int tmp1 = va_arg(VALIST_ACCESS(valist), int);
//					int tmp3 = va_arg(VALIST_ACCESS(valist), wchar_t);
					if ( !printfParams->unicode_output  && printfParams->l_modifier > 10 ) { // print unicode char to utf8
						print_char32_as_utf8_string((char32_t)c1, printfParams);
					}else{
						// c1 might be a char (<255) or a unicode char. UTF16 char < 255 are the same as UTF8. Ne need to check.
						print_char_macro(c1, printfParams); // 'char' is promoted to 'int' when passed through '...'
					}
				}
#endif
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
			{
#if PRINTF_CHECK_UNSUPPORTED_STRING_FORMAT == 1  &&  (PRINTF_UTF8_INPUT_SUPPORT==0 || PRINTF_UNICODE_INPUT_SUPPORT==0)
// If both input support disabled, we can't even print "unsupported"
#  if PRINTF_UTF8_INPUT_SUPPORT == 1 || PRINTF_UNICODE_INPUT_SUPPORT== 1
#    if PRINTF_UTF8_INPUT_SUPPORT == 0
                if ( printfParams->l_modifier <= 10 ) {
					va_arg(VALIST_ACCESS(valist), const char*);
#    endif
#    if PRINTF_UNICODE_INPUT_SUPPORT == 0
                if ( printfParams->l_modifier > 10 ) {
					va_arg(VALIST_ACCESS(valist), const wchar_t*);
#    endif
#    if PRINTF_UNICODE_OUTPUT_SUPPORT == 1  &&  PRINTF_UTF8_INPUT_SUPPORT == 0
				    output_utf8_string(L"unsupported", printfParams);
#    else
                    output_utf8_string("unsupported", printfParams);
#    endif
					printfParams->inDirective = 0;
				}
				else
#  endif
#endif

#if PRINTF_UNICODE_INPUT_SUPPORT == 1
					if ( printfParams->l_modifier > 10 ) {
						const wchar_t* s = va_arg(VALIST_ACCESS(valist), const wchar_t*);
						if ( printfParams->unicode_output ) {
							output_wchar_string(s, printfParams);
						}else{
							output_wchar_string_as_utf8(s, printfParams);
						}
						printfParams->inDirective = 0;
					}else
#endif
#if PRINTF_UTF8_INPUT_SUPPORT == 1
					{
						const char* s = va_arg(VALIST_ACCESS(valist), const char*);
#if PRINTF_UTF8_OUTPUT_SUPPORT == 1  &&  PRINTF_UNICODE_OUTPUT_SUPPORT == 1
						if ( printfParams->unicode_output ) {
							output_utf8_string_as_wchar(s, printfParams);
						}else{
							output_utf8_string_as_utf8(s, printfParams);
						}
#elif PRINTF_UTF8_OUTPUT_SUPPORT == 1
						output_utf8_string(s, printfParams);
#elif PRINTF_UNICODE_OUTPUT_SUPPORT == 1
						output_utf8_string_as_wchar(s, printfParams);
#endif
					}
#endif
				printfParams->inDirective = 0;
			}
//					{
//						output_utf8_string(va_arg(VALIST_ACCESS(valist), const output_char_type*), printfParams);
//						printfParams->inDirective = 0;
//					}

//#if PRINTF_UNICODE_INPUT_SUPPORT == 1  &&  PRINTF_UTF8_OUTPUT_SUPPORT == 1
//					if ( printfParams->l_modifier == 0 ) {
//						output_utf8_string(va_arg(VALIST_ACCESS(valist), const char*), printfParams);
//						printfParams->inDirective = 0;
//					}else
//#elif PRINTF_UTF8_INPUT_SUPPORT == 1  &&  PRINTF_UNICODE_OUTPUT_SUPPORT == 1
//#arning TODO
//					if ( printfParams->l_modifier == 1 ) {
//						output_utf8_string(va_arg(VALIST_ACCESS(valist), const char*), printfParams);
//						printfParams->inDirective = 0;
//					}else
//#endif
//					{
//						output_utf8_string(va_arg(VALIST_ACCESS(valist), const output_char_type*), printfParams);
//						printfParams->inDirective = 0;
//					}
			break;
			default:  {
				print_char_macro('%', printfParams);
				if ( c != '%' ) print_char_macro(c, printfParams);
				printfParams->inDirective = 0;
			}
		}
	}
	else
	{
		if ( c == '%' )
		{
				printfParams->inDirective = 1;
				printfParams->l_modifier = 10;
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
				printfParams->precision_specifier = 6; // 6 digits for float, as specified by ANSI, if I remember well
  #endif
  #if PRINTF_LITE_PADCHAR_SUPPORT == 1
				printfParams->pad_char = ' ';
  #endif
		}
		else
		{
//			print_utf8_char(c, printfParams);
			print_char_macro(c, printfParams);
		}
	}
}

#if DEFINE_SECTIONS == 1
__attribute__((noinline, section(".vprintf_with_callback")))
#elif DEFINE_SECTIONS == 2
__attribute__((noinline, section(".printf_lite")))
#endif
void vprintf_with_callback(const char* format, va_list valist, transmitBufCallBackType transmitBufCallBack, void* context
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
	#if PRINTF_UTF8_OUTPUT_SUPPORT == 1  &&  PRINTF_UNICODE_OUTPUT_SUPPORT == 1
		printfParams.unicode_output = 0;
		printfParams.printCharFunction = print_utf8_char;
	#endif
	printfParams.transmitBufCallBack.transmitBufCallBack = transmitBufCallBack;
	printfParams.context = context;

	while ( 1 ) //Iterate over formatting string
	{
		char c = *format++;
		if (c == 0)	break;
		printf_handle_format_char(c, VALIST_PARAM(valist), &printfParams);
	}
  #if PRINTF_LITE_BUF_SIZE > 1
	if ( printfParams.bufIdx > 0 ) printfParams.transmitBufCallBack.transmitBufCallBack(printfParams.buf.buf, printfParams.bufIdx, printfParams.context);
  #endif

	va_end(valist);
}

#if DEFINE_SECTIONS == 1
__attribute__((noinline, section(".printf_with_callback")))
#elif DEFINE_SECTIONS == 2
__attribute__((noinline, section(".printf_lite")))
#endif
void printf_with_callback(const char* format, transmitBufCallBackType transmitBufCallBack, void* context,
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
							int* newline, int timestamp,
#endif
						...)
{
	va_list valist;
    #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
        va_start(valist, timestamp);
    #else
        va_start(valist, context);
    #endif
	vprintf_with_callback(format, valist, transmitBufCallBack, context
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, NULL, 0
  #endif
                         );
	va_end(valist);
}

#if PRINTF_UNICODE_OUTPUT_SUPPORT == 1

/*
 *
 * Only needed if PRINTF_UNICODE_OUTPUT_SUPPORT == 1 && PRINTF_UTF8_INPUT_SUPPORT == 1
 */
static const char* get_utf32_from_utf8(const char* s, char32_t* utf32_letter)
{
tryagain:
	if (  *((unsigned char*)s) & 0x80  ) {
		// if Byte 1 is 1xxxxxxx : multi byte is at least 2 char
		if (*(s+1) == 0) {
			// Finished in the middle of an utf8 multibyte char
			return NULL;
		}
		// Byte 2 should be 0b10xxxxxx
		if ((*(((unsigned char*)s)+1) & 0xc0) != 0x80) {  // 0xC0 = 0b11000000
			// second byte is not multi byte char, ignore
			s += 1;
			goto tryagain;
		}
		// if Byte 1 is 111xxxxx : multi byte is at least 3 char
		if ((*((unsigned char*)s) & 0xe0) == 0xe0) { // 0xE0 = 0b11100000
			if (*(s+2) == 0) {
				// Finished in the middle of an utf8 multibyte char
				return NULL;
			}
			// Byte 3 should be 0b10xxxxxx
			if ((*(((unsigned char*)s)+2) & 0xc0) != 0x80) {
				s += 1;
				goto tryagain;
			}
			// if Byte 1 is 1111xxxx : multi byte is 4 char
			if ((*((unsigned char*)s) & 0xf0) == 0xf0) { // 0xF0 = 0b11110000
				if (*(s+3) == 0) {
					// Finished in the middle of an utf8 multibyte char
					return NULL;
				}
				// if Byte 1 is not 0b11110xxx  ||  Byte 4 not 0b10xxxxxx
				if ((*((unsigned char*)s) & 0xf8) != 0xf0 || (*(((unsigned char*)s)+3) & 0xc0) != 0x80) {
					s += 1;
					goto tryagain;
				}
				/* 4-byte code */
				*utf32_letter = (char32_t)((*((char32_t*)s) & 0x7) << 18);
				*utf32_letter |= (char32_t)((*(((unsigned char*)s)+1) & 0x3f) << 12);
				*utf32_letter |= (char32_t)((*(((unsigned char*)s)+2) & 0x3f) << 6);
				*utf32_letter |= *(((unsigned char*)s)+3) & 0x3f;
				return s + 4;
			} else {
				/* 3-byte code */
				*utf32_letter = (char32_t)((*((unsigned char*)s) & 0xf) << 12);
				*utf32_letter |= (char32_t)((*(((unsigned char*)s)+1) & 0x3f) << 6);
				*utf32_letter |= *(((unsigned char*)s)+2) & 0x3f;
				return s + 3;
			}
		} else {
			/* 2-byte code */
			*utf32_letter = (char32_t)((*((unsigned char*)s) & 0x1f) << 6);
			*utf32_letter |= *(((unsigned char*)s)+1) & 0x3f;
			return s + 2;
		}
	} else {
		/* 1-byte code */
		*utf32_letter = *((unsigned char*)s);
		return s + 1;
	}
}


void vwprintf_with_callback(const char* format, va_list valist, transmitWBufCallBackType transmitWBufCallBack, void* context
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
	printfParams.unicode_output = 1;
	printfParams.printCharFunction = print_wchar;
	printfParams.transmitBufCallBack.transmitWBufCallBack = transmitWBufCallBack;
	printfParams.context = context;

	while ( *format ) //Iterate over formatting string
	{
		char32_t c;
		format = get_utf32_from_utf8(format, &c);
		if (format == 0) break;
		if ( c <= 0x80) {
			printf_handle_format_char((char)c, VALIST_PARAM(valist), &printfParams);
			continue;
		}
  #if __WCHAR_MAX__ <= 0xFFFFu
		if ( c <= 0xFFFF) {
				print_wchar((wchar_t)c, &printfParams);
		} else {
				c -= halfBase;
				print_wchar((wchar_t)((c >> halfShift) + UNI_SUR_HIGH_START), &printfParams);
				print_wchar((wchar_t)((c & halfMask) + UNI_SUR_LOW_START), &printfParams);
		}
  #else
		print_wchar((wchar_t)c, &printfParams);
  #endif
	}
  #if PRINTF_LITE_BUF_SIZE > 1
	if ( printfParams.bufIdx > 0 ) printfParams.transmitBufCallBack.transmitBufCallBack(printfParams.buf.buf, printfParams.bufIdx, printfParams.context);
  #endif

	va_end(valist);
}

void wprintf_with_callback(const char* format, transmitWBufCallBackType transmitWBufCallBack, void* context,
#if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
							int* newline, int timestamp,
#endif
						...)
{
	va_list valist;
    #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
        va_start(valist, timestamp);
    #else
        va_start(valist, context);
    #endif
	vwprintf_with_callback(format, valist, transmitWBufCallBack, context
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, NULL, 0
  #endif
                         );
	va_end(valist);
}
#endif




/* ------------------------------------------------- SNPRINTF ------------------------------------------------- */

#if PRINTF_LITE_SNPRINTF_SUPPORT == 1

typedef struct SPrintfContext_t
{
	void* printf_callback_vsnprintf_buffer;
	size_t printf_callback_vsnprintf_buffer_len;
	int printf_callback_vsnprintf_count;
} SPrintfContext_t;

/* ---------------  vsnprintf,snprintf (Output UTF8)  --------------- */

void transmitSPrintf(const char* buf, unsigned int nbchar, void* context)
{
	SPrintfContext_t* SPrintfContext = (SPrintfContext_t*)context;
	unsigned int i=0;
	for ( ; SPrintfContext->printf_callback_vsnprintf_buffer_len>0  &&  i<nbchar ; i++) {
		char** sprintfBufChar = (char**)&(SPrintfContext->printf_callback_vsnprintf_buffer);
		*(*sprintfBufChar)++ = buf[i];
		SPrintfContext->printf_callback_vsnprintf_buffer_len--;
	}
	SPrintfContext->printf_callback_vsnprintf_count += nbchar;
}

int PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnprint, PRINTF_CFUNCTION_SUFFIX)(char* buf, size_t len, const char *__restrict format, va_list valist)
{
	SPrintfContext_t SPrintfContext;
	SPrintfContext.printf_callback_vsnprintf_buffer = buf;
	SPrintfContext.printf_callback_vsnprintf_buffer_len = len-1;
	SPrintfContext.printf_callback_vsnprintf_count = 0;
	vprintf_with_callback(format, valist, transmitSPrintf, (void*)&SPrintfContext
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, NULL, 0
  #endif
                                   );
	*(char*)(SPrintfContext.printf_callback_vsnprintf_buffer) = 0;
	return SPrintfContext.printf_callback_vsnprintf_count;
}

int PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, snprint, PRINTF_CFUNCTION_SUFFIX)(char* buf, size_t len, const char *__restrict format, ...)
{
	va_list valist;
	va_start(valist, format);
	int ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnprint, PRINTF_CFUNCTION_SUFFIX)(buf, len, format, valist);
	va_end(valist);
	return ret;
}

#if PRINTF_UNICODE_OUTPUT_SUPPORT == 1
/* ---------------  vsnwprintf,snwprintf (Output Unicode)  --------------- */

typedef struct SWPrintfContext_t
{
	wchar_t* printf_callback_vsnprintf_buffer;
	size_t printf_callback_vsnprintf_buffer_len;
	int printf_callback_vsnprintf_count;
} SWPrintfContext_t;

void transmitSWPrintf(const wchar_t* buf, unsigned int nbchar, void* context)
{
	SWPrintfContext_t* SWPrintfContext = (SWPrintfContext_t*)context;
	unsigned int i=0;
	for ( ; SWPrintfContext->printf_callback_vsnprintf_buffer_len>0  &&  i<nbchar ; i++) {
		wchar_t** sprintfBufChar = (wchar_t**)&(SWPrintfContext->printf_callback_vsnprintf_buffer);
		*(*sprintfBufChar)++ = buf[i];
		SWPrintfContext->printf_callback_vsnprintf_buffer_len--;
	}
	SWPrintfContext->printf_callback_vsnprintf_count += nbchar;
}

int PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wchar_t* buf, size_t len, const char *__restrict format, va_list valist)
{
	SWPrintfContext_t SWPrintfContext;
	SWPrintfContext.printf_callback_vsnprintf_buffer = buf;
	SWPrintfContext.printf_callback_vsnprintf_buffer_len = len-1;
	SWPrintfContext.printf_callback_vsnprintf_count = 0;
	vwprintf_with_callback(format, valist, transmitSWPrintf, (void*)(&SWPrintfContext)
  #if PRINTF_LITE_TIMESTAMP_SUPPORT == 1
									, NULL, 0
  #endif
                                   );
	*(wchar_t*)(SWPrintfContext.printf_callback_vsnprintf_buffer) = 0;
#if VSNWPRINTF_RETURN_MINUS1_ON_OVERFLOW == 1
	if ( (size_t)(SWPrintfContext.printf_callback_vsnprintf_count) >= len ) return -1;
#endif
	return SWPrintfContext.printf_callback_vsnprintf_count;
}

int PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, snwprint, PRINTF_CFUNCTION_SUFFIX)(wchar_t* buf, size_t len, const char *__restrict format, ...)
{
	va_list valist;
	va_start(valist, format);
	int ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(buf, len, format, valist);
	va_end(valist);
	return ret;
}
#endif // PRINTF_UNICODE_OUTPUT_SUPPORT == 1

#endif // PRINTF_LITE_SNPRINTF_SUPPORT == 1


#if defined(__cplusplus)
}
#endif


//
//  main.cpp
//  Printf-UnitTests
//
//  Created by Jief on 29/08/17.
//  Copyright © 2017 Jief. All rights reserved.
//

#include <Platform.h>
#include <limits.h>
#include "../cpp_foundation/unicode_conversions.h"
#include <printf_lite-test-cpp_conf.h>
#include "printf_lite-test.h"
#include <printf_lite-conf.h>
#include "../../Include/Library/printf_lite.h"

static int nbTestFailed = 0;
#ifdef DISPLAY_ONLY_FAILED
static bool displayOnlyFailed = true;
#else
static bool displayOnlyFailed = false;
#endif

/*
 * Print wchar string as a utf8 string.
 * This eliminate all problems about wprintf and compilation with short-wchar or long-wchar I had on macOs (2020-03)
 */
static void print_wchar_string(const wchar_t* s)
{
//	char utf8[wchar_len(s)*4+1];
// some compiler doesn't like variable length array.
// use a fixed length instead.
	char utf8[100];
	utf8_string_from_wchar_string(utf8, sizeof(utf8), s);
	if ( strlen(utf8) > sizeof(utf8)-2 ) {
		loggf("fixed size buf not big enough");
		abort();
	}
	loggf("%s", utf8);
}

static int testPrintf(const char* label, const char*  expectResult, int expectedRet, const char* format, ...) __attribute__((format(printf, 4, 5)));

static int testPrintf(const char* label, const char*  expectResult, int expectedRet, const char* format, ...)
{
	char buf[40];
	va_list valist;
	va_start(valist, format);
//	const char* c = #PRINTF_CFUNCTION_PREFIX;
	int vsnprintf_ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnprint, PRINTF_CFUNCTION_SUFFIX)(buf, sizeof(buf), format, valist);
	va_end(valist);
	if ( strcmp(buf, (char*)expectResult) != 0 ) {
		loggf(F("%s -> ERROR. Expect " PRIF " and get %s\n"), label, expectResult, buf);
        nbTestFailed += 1;
	}else if ( vsnprintf_ret != expectedRet ) {
		loggf(F("%s -> ERROR. Expect return value %d and get %d\n"), label, expectedRet, vsnprintf_ret);
        nbTestFailed += 1;
	}else if ( !displayOnlyFailed ) {
		loggf(F("%s : %s -> OK\n"), label, buf);
	}
	return 1;
}


static int testWPrintf(const char* label, const wchar_t*  expectResult, int expectedRet, const char* format, ...) __attribute__((format(printf, 4, 5)));

static int testWPrintf(const char* label, const wchar_t*  expectResult, int expectedRet, const char* format, ...)
{
	wchar_t wbuf[40];
#if VSNWPRINTF_RETURN_MINUS1_ON_OVERFLOW == 1
	if ( expectedRet >= (int)(sizeof(wbuf)/sizeof(wchar_t)) ) expectedRet = -1;
#endif
	va_list valist;
	va_start(valist, format);
	int vsnwprintf_ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wbuf, sizeof(wbuf)/sizeof(wchar_t), format, valist);
	va_end(valist);
//delay_ms(10);
	if ( memcmp(wbuf, expectResult, wchar_len(expectResult)*sizeof(expectResult[0])) != 0 ) {
//		loggf(F(" -> ERROR. Expect " PRILF " and get %ls\n"), expectResult, buf);
//      not using wprintf, it crashes sometimes, it doesn't work for short-wchar
		loggf(F("%s -> ERROR. Expect "), label);
		print_wchar_string(expectResult);
		loggf(F(" and get "));
		print_wchar_string(wbuf);
		loggf("\n");
		nbTestFailed += 1;
		va_start(valist, format);
		PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wbuf, sizeof(wbuf)/sizeof(wchar_t), format, valist); // for stepping with a debugger.
		va_end(valist);
	}else if ( vsnwprintf_ret != expectedRet ) {
		loggf(F("%s -> ERROR. Expect return value %d and get %d\n"), label, expectedRet, vsnwprintf_ret);
		nbTestFailed += 1;
		va_start(valist, format);
		PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wbuf, sizeof(wbuf)/sizeof(wchar_t), format, valist); // for stepping with a debugger.
		va_end(valist);
	}else if ( !displayOnlyFailed ) {
		loggf(F("%s : "), label);
		print_wchar_string(wbuf);
		loggf(F(" -> OK\n"));
	}
//delay_ms(10);
	return 1;
}



#define Test1arg(expectResult,format,c) \
{ \
	char label[1024]; \
	snprintf(label, sizeof(label), F("Test sprintf(" PRIF ", " PRIF ")"), F(#format), F(#c)); \
    testPrintf(label,expectResult,(int)strlen(expectResult),format,c); \
    snprintf(label, sizeof(label), F("Test swprintf(" PRIF ", " PRIF ")"), F(#format), F(#c)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),format,c); \
}

#define Test2arg(expectResult,format,c,d) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("Test sprintf(" PRIF ", " PRIF ", " PRIF ")"), F(#format), F(#c), F(#d)); \
    testPrintf(label,expectResult,(int)strlen(expectResult),format,c,d); \
    snprintf(label, sizeof(label), F("Test swprintf(" PRIF ", " PRIF ", " PRIF ")"), F(#format), F(#c), F(#d)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),format,c,d); \
}

#define Test5arg(expectResult,format,c,d,e,f,g) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("Test sprintf(" PRIF ", " PRIF ", " PRIF ", " PRIF ", " PRIF ", " PRIF ")"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testPrintf(label,expectResult,(int)strlen(expectResult),format,c,d,e,f,g); \
    snprintf(label, sizeof(label), F("Test swprintf(" PRIF ", " PRIF ", " PRIF ", " PRIF ", " PRIF ", " PRIF ")"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),format,c,d,e,f,g); \
}

#define TestLen5arg(expectResult,expectedRet,format,c,d,e,f,g) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("Test sprintf(" PRIF ", " PRIF ", " PRIF ", " PRIF ", " PRIF ", " PRIF ")"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testPrintf(label,expectResult,expectedRet,format,c,d,e,f,g); \
    snprintf(label, sizeof(label), F("Test swprintf(" PRIF ", " PRIF ", " PRIF ", " PRIF ", " PRIF ", " PRIF ")"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testWPrintf(label,L##expectResult,expectedRet,format,c,d,e,f,g); \
}


int printf_lite_tests(void)
{
#ifdef DISPLAY_START_INFO
	loggf(F("\n"));
	loggf(F("Printf unit test\n"));
	loggf(F("\n"));
	loggf(F("\n"));

    // These depends on the plateform. They are not printf unit test, but it's nice to check size of builtin type.
	loggf(F("sizeof(float)=%lu\n"), sizeof(float));
    loggf(F("sizeof(double)=%zu\n"), sizeof(double));
    loggf(F("sizeof(short int)=%zu\n"), sizeof(short int));
    loggf(F("sizeof(int)=%zu\n"), sizeof(int));
    loggf(F("sizeof(long int)=%zu\n"), sizeof(long int));// long is 64 bits
    loggf(F("sizeof(long long int)=%zu\n"), sizeof(long long int));
    loggf(F("sizeof(size_t)=%zu=%zu\n"), sizeof(size_t), sizeof(size_t));
    loggf(F("sizeof(size_t)=%zu=%zu\n"), sizeof(size_t), sizeof(size_t));
    loggf(F("sizeof(void*)=%zu\n"), sizeof(void*));
    loggf(F("UINT64_MAX=%llu\n"), UINT64_MAX);
    loggf(F("SIZE_T_MAX=%zu\n"), SIZE_T_MAX);
    loggf(F("\n"));

    loggf(F("PRId16=%s\n"), PRId16);
    loggf(F("PRIu16=%s\n"), PRIu16);
    loggf(F("PRId32=%s\n"), PRId32);
    loggf(F("PRIu32=%s\n"), PRIu32);
    loggf(F("PRId32=%s\n"), PRId32);
    loggf(F("PRIu32=%s\n"), PRIu32);
    loggf(F("PRId64=%s\n"), PRId64);
    loggf(F("PRIu64=%s\n"), PRIu64);
    loggf(F("\n"));
#endif

	
    Test1arg(F("|80123456|"), F("|%X|"), (int)0xFFFFFFFF80123456);
    Test1arg(F("|FFFFFFFF80123456|"), F("|%lX|"), 0xFFFFFFFF80123456);

	Test1arg(F("Āࠀ𐀀🧊Выход'utf8'из"), F("Āࠀ𐀀🧊Выход'%s'из"), "utf8");

	
//	char buf[256];
//	snprintf(buf, sizeof(buf), "test %s", "ascii");

// wprintf(L"%llS", (int)4); doesn't check format
//	printf("%ls", (char32_t)4);

	// in testPrintf functions, buffer is only 40 bytes, to be able to test vsnwprintf truncate correctly.
	//
//	const char* utf8 = "Āࠀ𐀀Выходиз";
//	const wchar_t* unicode = L"Āࠀ𐀀Выходиз";

//printf("%ls %r\n", "foo", 1);

//testWPrintf("", F(L"Āࠀ𐀀🧊Выход'utf16'из"), F("Āࠀ𐀀🧊Выход'%s'из"), "utf16");

	Test1arg(F("'utf8-string'"), F("'%s'"), "utf8-string");
	Test1arg(F("'utf16-string'"), F("'%ls'"), L"utf16-string");
	Test1arg(F("Āࠀ𐀀🧊Выход'utf8'из"), F("Āࠀ𐀀🧊Выход'%s'из"), "utf8");
	Test1arg(F("Āࠀ𐀀🧊Выход'utf16'из"), F("Āࠀ𐀀🧊Выход'%ls'из"), L"utf16");
	Test1arg(F("Āࠀ𐀀🧊Выхо'ыход'из"), F("Āࠀ𐀀🧊Выхо'%s'из"), "ыход");
	Test1arg(F("Āࠀ𐀀🧊Выхо'ыход'из"), F("Āࠀ𐀀🧊Выхо'%ls'из"), L"ыход");

	Test1arg(F("'u'"), F("'%s'"), (char*)L"utf16-string");


	// Check %s with width specifier
    Test1arg(F("|a|"), F("|%4s|"), "a");
    Test1arg(F("|aa|"), F("|%4s|"), "aa");
    Test1arg(F("|aaa|"), F("|%4s|"), "aaa");
    Test1arg(F("|aaaa|"), F("|%4s|"), "aaaa");
    Test1arg(F("|aaaa|"), F("|%4s|"), "aaaaa");
    Test1arg(F("|aaaa|"), F("|%4s|"), "aaaaaa");
	
	// Check %ls with width specifier
    Test1arg(F("|a|"), F("|%4ls|"), L"a");
    Test1arg(F("|aa|"), F("|%4ls|"), L"aa");
    Test1arg(F("|aaa|"), F("|%4ls|"), L"aaa");
    Test1arg(F("|aaaa|"), F("|%4ls|"), L"aaaa");
    Test1arg(F("|aaaa|"), F("|%4ls|"), L"aaaaa");
    Test1arg(F("|aaaa|"), F("|%4ls|"), L"aaaaaa");


    // These must always works. It also test that integer type are well defined
    Test1arg(F("sizeof(uint8_t)=1"), F("sizeof(uint8_t)=%zu"), sizeof(uint8_t));
    Test1arg(F("sizeof(uint16_t)=2"), F("sizeof(uint16_t)=%zu"), sizeof(uint16_t));
    Test1arg(F("sizeof(uint32_t)=4"), F("sizeof(uint32_t)=%zu"), sizeof(uint32_t));
    Test1arg(F("sizeof(uint64_t)=8"), F("sizeof(uint64_t)=%zu"), sizeof(uint64_t));
    Test1arg(F("sizeof(int8_t)=1"), F("sizeof(int8_t)=%zu"), sizeof(int8_t));
    Test1arg(F("sizeof(int16_t)=2"), F("sizeof(int16_t)=%zu"), sizeof(int16_t));
    Test1arg(F("sizeof(int32_t)=4"), F("sizeof(int32_t)=%zu"), sizeof(int32_t));
    Test1arg(F("sizeof(int64_t)=8"), F("sizeof(int64_t)=%zu"), sizeof(int64_t));
//    loggf(F("\n"));



    Test5arg(F("12 34 56.67 hi X"), F("%d %u %.2lf %s %c"), 12, 34, 56.67, "hi", 'X');

    // test format
    Test1arg(F("12"), F("%d"), 12);
    Test1arg(F("12"), F("%u"), 12);
    Test1arg(F("|abfe|"), F("|%x|"), 0xABFE);
    Test1arg(F("|ABFE|"), F("|%X|"), 0xABFE);
    Test1arg(F("12.987654"), F("%f"), 12.987654f);
    Test1arg(F("12.987654"), F("%lf"), 12.987654);

    // Test rounding
    Test1arg(F("10"), F("%1.0lf"), 10.4999);
    Test1arg(F("11"), F("%1.0lf"), 10.5001);
    Test1arg(F("10.5"), F("%1.1lf"), 10.5499);
    Test1arg(F("10.6"), F("%1.1lf"), 10.5501);
    Test1arg(F("10.005"), F("%1.3lf"), 10.0054);
    Test1arg(F("10.006"), F("%1.3lf"), 10.0056);


    // Test big numbers
    #ifdef ARDUINO
        // #define LARGE_DOUBLE_TRESHOLD (9.1e18) in printf_lite
        Test1arg(F("1234567.000000"), F("%lf"), 1234567.0);
        Test1arg(F("-1234567.000000"), F("%lf"), -1234567.0);
    #else
        // #define LARGE_DOUBLE_TRESHOLD (9.1e18) in printf_lite
        Test1arg(F("123456789012345680.000000"), F("%lf"), 123456789012345678.0);
        Test1arg(F("-123456789012345680.000000"), F("%lf"), -123456789012345678.0);
    #endif


    // test with specifier, space as pad char
    Test1arg(F("|    0|"), F("|%5d|"), 0);
    Test1arg(F("|    0|"), F("|%5u|"), 0);
    Test1arg(F("|    0|"), F("|%5x|"), 0);
    Test1arg(F("|    0|"), F("|%5X|"), 0);
    Test1arg(F("| 0.000000|"), F("|%9lf|"), 0.0);

    // test with specifier too small, space as pad char
    Test1arg(F("|1234|"), F("|%2d|"), 1234); // keep under 16 bit value, if not, on 16 bits CPU, the constant become long int and doesn't match %d
    Test1arg(F("|5678|"), F("|%2u|"), 5678); // keep under 16 bit value, if not, on 16 bits CPU, the constant become long int and doesn't match %u
    Test1arg(F("|abfe|"), F("|%2x|"), 0xABFE); // keep under 16 bit value, if not, on 16 bits CPU, the constant become long int and doesn't match %x

    // test test with specifier, space as pad char
    Test1arg(F("|   12|"), F("|%5d|"), 12);
    Test1arg(F("|   12|"), F("|%5u|"), 12);
    Test1arg(F("|    c|"), F("|%5x|"), 12);
    Test1arg(F("|    C|"), F("|%5X|"), 12);

    // test pad char but no width (no effect)
    Test1arg(F("|c|"), F("|%0x|"), 12);
    Test1arg(F("|C|"), F("|%0X|"), 12);

    // test with specifier, 0 as pad char
    Test1arg(F("|00012|"), F("|%05d|"), 12);
    Test1arg(F("|00012|"), F("|%05u|"), 12);
    Test1arg(F("|0000c|"), F("|%05x|"), 12);
    Test1arg(F("|0000C|"), F("|%05X|"), 12);
    Test1arg(F("|0A|"), F("|%02X|"), (uint8_t)0xa);
#define SMST(a) ((UINT8)((a & 0xf0) >> 4))
#define SLST(a) ((UINT8)(a & 0x0f))
    testPrintf("spd", "00000F0F04070408", strlen("00000F0F04070408"), "%02X%02X%02X%02X%02X%02X%02X%02X", SMST(0x00) , SLST(0x00), SMST(0xFF), SLST(0xFF), SMST(0x147), SLST(0x147), SMST(0x148), SLST(0x148));


    Test1arg(F("|0A23|"), F("|%04X|"), 0xa23);
    Test1arg(F("|A234|"), F("|%04X|"), 0xa234);
    Test1arg(F("|A2345|"), F("|%04X|"), 0xa2345);
    Test1arg(F("|0a23|"), F("|%04x|"), 0xA23);
    Test1arg(F("|a234|"), F("|%04x|"), 0xA234);
    Test1arg(F("|a2345|"), F("|%04x|"), 0xA2345);
    Test1arg(F("|01|"), F("|%02d|"), 1);
    Test1arg(F("|12|"), F("|%02d|"), 12);
    Test1arg(F("|120|"), F("|%02d|"), 120);


    Test1arg(F("|0|"), F("|%01d|"), 0);
    Test1arg(F("|1|"), F("|%01d|"), 1);
    Test1arg(F("|100|"), F("|%01d|"), 100);
    Test1arg(F("|10000|"), F("|%01d|"), 10000);
    Test1arg(F("|-1|"), F("|%01d|"), -1);
    Test1arg(F("|-100|"), F("|%01d|"), -100);
    Test1arg(F("|-10000|"), F("|%01d|"), -10000);



    // Test1arg float format
    Test1arg(F("|0.000000|"), F("|%0f|"), 0.0f);
    Test1arg(F("|0.000000|"), F("|%1f|"), 0.0f);
    Test1arg(F("|0.000000|"), F("|%8f|"), 0.0f);
    Test1arg(F("| 0.000000|"), F("|%9f|"), 0.0f);
    Test1arg(F("|1.789010|"), F("|%2f|"), 1.78901f);
    Test1arg(F("|1.7890|"), F("|%.4f|"), 1.78901f);
    Test1arg(F("|1.7890|"), F("|%1.4f|"), 1.78901f);
    Test1arg(F("|        -1.7890|"), F("|%15.4f|"), -1.78901f);
    Test1arg(F("|-000000001.7890|"), F("|%015.4f|"), -1.78901f);
    Test1arg(F("|     -2|"), F("|%7.0f|"), -1.78901f);
    Test1arg(F("|-000002|"), F("|%07.0f|"), -1.78901f);


//testWPrintf(F(L"big printf (biiiiiiiiiiiiiiiiiiiiiiiiii"), F("big printf (biiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiigger than buffer!) %s %d %f %s %x"), "string1", 2, 2.3f, "string2", 0xBEEF);


    // Test that sprintf will properly truncate to sizeof(buf)-1
    TestLen5arg(F("big printf (biiiiiiiiiiiiiiiiiiiiiiiiii"), 100, F("big printf (biiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiigger than buffer!) %s %d %f %s %x"), "string1", 2, 2.3f, "string2", 0xBEEF);

    // Test %F format
    Test2arg(F("Flash string |string1| |striiiing2|"), F("Flash string |" PRIF "| |" PRIF "|"), F("string1"), F("striiiing2"));

    // test limits
    int16_t i;
    i = INT16_MAX; Test1arg(F("INT16_MAX=32767"), F("INT16_MAX=%d"), i);
    i = INT16_MIN; Test1arg(F("INT16_MIN=-32768"), F("INT16_MIN=%d"), i);

    uint16_t ui16;
    ui16 = UINT16_MAX; Test1arg(F("UINT16_MAX=65535"), F("UINT16_MAX=%u"), ui16);

    int32_t i32;
    i32 = INT32_MAX; Test1arg(F("INT32_MAX=2147483647"), F("INT32_MAX=%" PRId32), i32);
    i32 = INT32_MIN; Test1arg(F("INT32_MIN=-2147483648"), F("INT32_MIN=%" PRId32), i32);

    uint32_t ui32;
    ui32 = UINT32_MAX; Test1arg(F("UINT32_MAX=4294967295"), F("UINT32_MAX=%" PRIu32), ui32);

    int64_t i64;
    i64 = INT64_MAX; Test1arg(F("INT64_MAX=9223372036854775807"), F("INT64_MAX=%" PRId64), i64);
    i64 = INT64_MIN; Test1arg(F("INT64_MIN=-9223372036854775808"), F("INT64_MIN=%" PRId64), i64);

    uint64_t ui64;
    ui64 = UINT64_MAX; Test1arg(F("UINT64_MAX=18446744073709551615"), F("UINT64_MAX=%" PRIu64), ui64);

    #if __x86_64__
    #endif

    size_t size;
    if ( SIZE_T_MAX == UINT64_MAX ) {
        size = SIZE_T_MAX; Test1arg(F("SIZE_MAX=18446744073709551615"), F("SIZE_MAX=%zu"), size);
    }else if ( SIZE_T_MAX == UINT32_MAX ) {
        size = SIZE_T_MAX; Test1arg(F("SIZE_MAX=4294967295"), F("SIZE_MAX=%zu"), size);
    }else{
	    // 16 bits size_t ? Does that exist ?
    }

    #if PRINTF_LITE_PADCHAR_SUPPORT == 1  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  &&  PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
        // Nothing specified
        Test1arg(F("-1.789010"), F("%f"), -1.78901f);
        // Pad char
        Test1arg(F("-1.789010"), F("%0f"), -1.78901f); // libc printf ignore pad char if there is no width (it's not an error)
        // Width
        Test1arg(F("      -1.789010"), F("%15f"), -1.78901f);
        // Precision
        Test1arg(F("-1.79"), F("%.2f"), -1.78901f);
        // Pad char + width
        Test1arg(F("-0000001.789010"), F("%015f"), -1.78901f);
        // Pad char + precision
        Test1arg(F("-1.79"), F("%0.2f"), -1.78901f);
        // Pad char + width + precision
        Test1arg(F("-00000000001.79"), F("%015.2f"), -1.78901f);
    #endif
    #if PRINTF_LITE_PADCHAR_SUPPORT == 1  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  &&  PRINTF_LITE_FIELDPRECISION_SUPPORT == 0
        // Nothing specified
        Test1arg(F("-1.789010"), F("%f"), -1.78901f);
        // Pad char
        Test1arg(F("-1.789010"), F("%0f"), -1.78901f); // libc printf ignore pad char if there is no width (it's not an error)
        // Width
        Test1arg(F("      -1.789010"), F("%15f"), -1.78901f);
        // Precision
        Test1arg(F("-1.789010"), F("%.2f"), -1.78901f);
        // Pad char + width
        Test1arg(F("-0000001.789010"), F("%015f"), -1.78901f);
        // Pad char + precision
        Test1arg(F("-1.789010"), F("%0.2f"), -1.78901f);
        // Pad char + width + precision
        Test1arg(F("-0000001.789010"), F("%015.2f"), -1.78901f);
    #endif
    #if PRINTF_LITE_PADCHAR_SUPPORT == 1  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 0  &&  PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
        // Nothing specified
        Test1arg(F("-1.789010"), F("%f"), -1.78901f);
        // Pad char
        Test1arg(F("-1.789010"), F("%0f"), -1.78901f); // libc printf ignore pad char if there is no width (it's not an error)
        // Width
        Test1arg(F("-1.789010"), F("%15f"), -1.78901f);
        // Precision
        Test1arg(F("-1.79"), F("%.2f"), -1.78901f);
        // Pad char + width
        Test1arg(F("-1.789010"), F("%015f"), -1.78901f);
        // Pad char + precision
        Test1arg(F("-1.79"), F("%0.2f"), -1.78901f);
        // Pad char + width + precision
        Test1arg(F("-1.79"), F("%015.2f"), -1.78901f);
    #endif
    #if PRINTF_LITE_PADCHAR_SUPPORT == 0  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  &&  PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
        // Nothing specified
        Test1arg(F("-1.789010"), F("%f"), -1.78901f);
        // Pad char
        Test1arg(F("-1.789010"), F("%0f"), -1.78901f); // libc printf ignore pad char if there is no width (it's not an error)
        // Width
        Test1arg(F("      -1.789010"), F("%15f"), -1.78901f);
        // Precision
        Test1arg(F("-1.79"), F("%.2f"), -1.78901f);
        // Pad char + width
        Test1arg(F("      -1.789010"), F("%015f"), -1.78901f);
        // Pad char + precision
        Test1arg(F("-1.79"), F("%0.2f"), -1.78901f);
        // Pad char + width + precision
        Test1arg(F("          -1.79"), F("%015.2f"), -1.78901f);
    #endif
    #if PRINTF_LITE_PADCHAR_SUPPORT == 0  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 1  &&  PRINTF_LITE_FIELDPRECISION_SUPPORT == 0
        // Nothing specified
        Test1arg(F("-1.789010"), F("%f"), -1.78901f);
        // Pad char
        Test1arg(F("-1.789010"), F("%0f"), -1.78901f); // libc printf ignore pad char if there is no width (it's not an error)
        // Width
        Test1arg(F("      -1.789010"), F("%15f"), -1.78901f);
        // Precision
        Test1arg(F("-1.789010"), F("%.2f"), -1.78901f);
        // Pad char + width
        Test1arg(F("      -1.789010"), F("%015f"), -1.78901f);
        // Pad char + precision
        Test1arg(F("-1.789010"), F("%0.2f"), -1.78901f);
        // Pad char + width + precision
        Test1arg(F("      -1.789010"), F("%015.2f"), -1.78901f);
    #endif
    #if PRINTF_LITE_PADCHAR_SUPPORT == 0  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 0  &&  PRINTF_LITE_FIELDPRECISION_SUPPORT == 1
        // Nothing specified
        Test1arg(F("-1.789010"), F("%f"), -1.78901f);
        // Pad char
        Test1arg(F("-1.789010"), F("%0f"), -1.78901f); // libc printf ignore pad char if there is no width (it's not an error)
        // Width
        Test1arg(F("-1.789010"), F("%15f"), -1.78901f);
        // Precision
        Test1arg(F("-1.79"), F("%.2f"), -1.78901f);
        // Pad char + width
        Test1arg(F("-1.789010"), F("%015f"), -1.78901f);
        // Pad char + precision
        Test1arg(F("-1.79"), F("%0.2f"), -1.78901f);
        // Pad char + width + precision
        Test1arg(F("-1.79"), F("%015.2f"), -1.78901f);
    #endif
    #if PRINTF_LITE_PADCHAR_SUPPORT == 0  &&  PRINTF_LITE_FIELDWIDTH_SUPPORT == 0  &&  PRINTF_LITE_FIELDPRECISION_SUPPORT == 0
        // Nothing specified
        Test1arg(F("-1.789010"), F("%f"), -1.78901f);
        // Pad char
        Test1arg(F("-1.789010"), F("%0f"), -1.78901f); // libc printf ignore pad char if there is no width (it's not an error)
        // Width
        Test1arg(F("-1.789010"), F("%15f"), -1.78901f);
        // Precision
        Test1arg(F("-1.789010"), F("%.2f"), -1.78901f);
        // Pad char + width
        Test1arg(F("-1.789010"), F("%015f"), -1.78901f);
        // Pad char + precision
        Test1arg(F("-1.789010"), F("%0.2f"), -1.78901f);
        // Pad char + width + precision
        Test1arg(F("-1.789010"), F("%015.2f"), -1.78901f);
    #endif

    return nbTestFailed;
}


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
#include <printlib-test-cpp_conf.h>
#include "printlib-test.h"


extern "C" {
#   undef DISABLE_PRINTLIB
#   include <Library/PrintLib.h>
}

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
	char utf8[200];
	utf8_string_from_wchar_string(utf8, sizeof(utf8), s);
	if ( strlen(utf8) > sizeof(utf8)-2 ) {
		loggf("fixed size buf not big enough");
		abort();
	}
	loggf("%s", utf8);
}


static int testPrintf(const char* label, const char*  expectResult, int expectedRet, const char* format, ...) /*__attribute__((format(printf, 4, 5)))*/;

static int testPrintf(const char* label, const char*  expectResult, int expectedRet, const char* format, ...)
{
	char buf[40];
	va_list valist;
	va_start(valist, format);
//	const char* c = #PRINTF_CFUNCTION_PREFIX;
	int vsnprintf_ret = (int)AsciiVSPrint(buf, sizeof(buf), format, valist);
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


static int testWPrintf(const char* label, const wchar_t*  expectResult, int expectedRet, const wchar_t* format, ...) /*__attribute__((format(printf, 4, 5)))*/;

static int testWPrintf(const char* label, const wchar_t*  expectResult, int expectedRet, const wchar_t* format, ...)
{
	wchar_t wbuf[40];
#if VSNWPRINTF_RETURN_MINUS1_ON_OVERFLOW == 1
	if ( expectedRet >= (int)(sizeof(wbuf)/sizeof(wchar_t)) ) expectedRet = -1;
#endif

	va_list valist;
	va_start(valist, format);
//	int vsnwprintf_ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wbuf, sizeof(wbuf)/sizeof(wchar_t), format, valist);
//	_PoolCatPrint (format, valist, &spc, _PoolPrint);
	UnicodeVSPrint(wbuf, sizeof(wbuf), format, valist);
	va_end(valist);
//delay_ms(10);
	if ( memcmp(wbuf, expectResult, size_of_utf_string(expectResult)*sizeof(expectResult[0])) != 0 ) {
//		loggf(F(" -> ERROR. Expect " PRILF " and get %ls\n"), expectResult, buf);
//      not using wprintf, it crashes sometimes, it doesn't work for short-wchar
		loggf(F("%s -> ERROR. Expect "), label);
		print_wchar_string(expectResult);
		loggf(F(" and get "));
		print_wchar_string(wbuf);
		loggf("\n");
		nbTestFailed += 1;
		va_start(valist, format);
		UnicodeVSPrint(wbuf, sizeof(wbuf), format, valist); // for stepping with a debugger.
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
	snprintf(label, sizeof(label), F("Test AsciiVSPrint(%s, %s)"), F(#format), F(#c)); \
    testPrintf(label,expectResult,(int)strlen(expectResult),format,c); \
    snprintf(label, sizeof(label), F("Test UnicodeVSPrint(%s, %s)"), F(#format), F(#c)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),L##format,c); \
}

#define Test2arg(expectResult,format,c,d) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("Test AsciiVSPrint(%s, %s, %s)"), F(#format), F(#c), F(#d)); \
    testPrintf(label,expectResult,(int)strlen(expectResult),format,c,d); \
    snprintf(label, sizeof(label), F("Test UnicodeVSPrint(%s, %s, %s)"), F(#format), F(#c), F(#d)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),L##format,c,d); \
}

#define Test5arg(expectResult,format,c,d,e,f,g) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("Test AsciiVSPrint(%s, %s, %s, %s, %s, %s)"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testPrintf(label,expectResult,(int)strlen(expectResult),format,c,d,e,f,g); \
    snprintf(label, sizeof(label), F("Test UnicodeVSPrint(%s, %s, %s, %s, %s, %s)"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),L##format,c,d,e,f,g); \
}

#define TestLen5arg(expectResult,expectedRet,format,c,d,e,f,g) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("Test AsciiVSPrint(%s, %s, %s, %s, %s, %s)"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testPrintf(label,expectResult,expectedRet,format,c,d,e,f,g); \
    snprintf(label, sizeof(label), F("Test UnicodeVSPrint(%s, %s, %s, %s, %s, %s)"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testWPrintf(label,L##expectResult,expectedRet,L##format,c,d,e,f,g); \
}



int printlib_tests(void)
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

    loggf(F("PRId16=%a\n"), PRId16);
    loggf(F("PRIu16=%a\n"), PRIu16);
    loggf(F("PRId32=%a\n"), PRId32);
    loggf(F("PRIu32=%a\n"), PRIu32);
    loggf(F("PRId32=%a\n"), PRId32);
    loggf(F("PRIu32=%a\n"), PRIu32);
    loggf(F("PRId64=%a\n"), PRId64);
    loggf(F("PRIu64=%a\n"), PRIu64);
    loggf(F("\n"));
#endif

	
//	char buf[256];
//	snprintf(buf, sizeof(buf), "test %a", "ascii");

// wprintf(L"%llS", (int)4); doesn't check format
//	printf("%ls", (char32_t)4);

	// in testPrintf functions, buffer is only 40 bytes, to be able to test vsnwprintf truncate correctly.
	//
//	const char* utf8 = "Āࠀ𐀀Выходиз";
//	const wchar_t* unicode = L"Āࠀ𐀀Выходиз";

//printf("%ls %r\n", "foo", 1);

//testWPrintf("", F(L"Āࠀ𐀀🧊Выход'utf16'из"), F("Āࠀ𐀀🧊Выход'%a'из"), "utf16");

	Test1arg(F("'utf8-string'"), F("'%a'"), "utf8-string");
	Test1arg(F("'utf16-string'"), F("'%s'"), L"utf16-string");
	Test1arg(F("Āࠀ𐀀🧊Выход'utf8'из"), F("Āࠀ𐀀🧊Выход'%a'из"), "utf8");
	Test1arg(F("Āࠀ𐀀🧊Выход'utf16'из"), F("Āࠀ𐀀🧊Выход'%s'из"), L"utf16");
//	Test1arg(F("Āࠀ𐀀🧊Выхо'ыход'из"), F("Āࠀ𐀀🧊Выхо'%a'из"), "ыход"); // utf8 chars seems not working with PrintLib
//	Test1arg(F("Āࠀ𐀀🧊Выхо'ыход'из"), F("Āࠀ𐀀🧊Выхо'%s'из"), L"ыход"); // utf16 chars seems not working with PrintLib


    // These must always works. It also test that integer type are well defined
    Test1arg(F("sizeof(uint8_t)=1"), F("sizeof(uint8_t)=%d"), sizeof(uint8_t)); // %zu, %zd not supported by PrintLib
    Test1arg(F("sizeof(uint16_t)=2"), F("sizeof(uint16_t)=%d"), sizeof(uint16_t));
    Test1arg(F("sizeof(uint32_t)=4"), F("sizeof(uint32_t)=%d"), sizeof(uint32_t));
    Test1arg(F("sizeof(uint64_t)=8"), F("sizeof(uint64_t)=%d"), sizeof(uint64_t));
    Test1arg(F("sizeof(int8_t)=1"), F("sizeof(int8_t)=%d"), sizeof(int8_t));
    Test1arg(F("sizeof(int16_t)=2"), F("sizeof(int16_t)=%d"), sizeof(int16_t));
    Test1arg(F("sizeof(int32_t)=4"), F("sizeof(int32_t)=%d"), sizeof(int32_t));
    Test1arg(F("sizeof(int64_t)=8"), F("sizeof(int64_t)=%d"), sizeof(int64_t));
//    loggf(F("\n"));



//    Test5arg(F("12 34 56.67 hi X"), F("%d %u %.2lf %a %c"), 12, 34, 56.67, "hi", 'X'); // %f, %u not supported by PoolPrint
    Test5arg(F("12 34 hi X 0"), F("%d %d %a %c %d"), 12, 34, "hi", 'X', 0); // %f, %u not supported by PoolPrint

    // %f not supported by PrintLib


    // test format
    Test1arg(F("12"), F("%d"), 12);
    Test1arg(F("|A|"), F("|%x|"), 0xA); // %x for PrintLib is %X for printf
    Test1arg(F("|AB|"), F("|%x|"), 0xAB); // %x for PrintLib is %X for printf
    Test1arg(F("|ABF|"), F("|%x|"), 0xABF); // %x for PrintLib is %X for printf
    Test1arg(F("|ABFE|"), F("|%x|"), 0xABFE); // %x for PrintLib is %X for printf
    Test1arg(F("|ABFED|"), F("|%x|"), 0xABFED); // %x for PrintLib is %X for printf
    Test1arg(F("|ABF|"), F("|%X|"), 0xABF);
    Test1arg(F("|FFFFFFF6|"), F("|%x|"), -10);
    Test1arg(F("|FFFFFFF6|"), F("|%X|"), -10);
    Test1arg(F("|FFFFFFF6|"), F("|%2X|"), -10);
    Test1arg(F("|FFFFFFF6|"), F("|%0X|"), -10);
    Test1arg(F("|            FFFFFFF6|"), F("|%20x|"), -10);
    Test1arg(F("|FFFFFFF6|"), F("|%lx|"), -10);
    Test1arg(F("|FFFFFFFFFFFFFFF6|"), F("|%lX|"), -10L);
    Test1arg(F("|0000FFFFFFFFFFFFFFF6|"), F("|%20lX|"), -10L);

    // test with specifier, space as pad char
    Test1arg(F("|    0|"), F("|%5d|"), 0);
    Test1arg(F("|    0|"), F("|%5u|"), 0);
    Test1arg(F("|    0|"), F("|%5x|"), 0); // %x for PrintLib is %X for printf
    Test1arg(F("|00000|"), F("|%5X|"), 0);

    // test with specifier too small, space as pad char
    Test1arg(F("|1234|"), F("|%2d|"), 1234); // keep under 16 bit value, if not, on 16 bits CPU, the constant become long int and doesn't match %d
    Test1arg(F("|1234|"), F("|%2u|"), 1234); // keep under 16 bit value, if not, on 16 bits CPU, the constant become long int and doesn't match %d
    Test1arg(F("|ABFE|"), F("|%2x|"), 0xABFE); // %x for PrintLib is %X for printf
    Test1arg(F("|ABFE|"), F("|%2X|"), 0xABFE);
    Test1arg(F("|0A|"), F("|%2X|"), (uint8_t)0xa);
#define SMST(a) ((UINT8)((a & 0xf0) >> 4))
#define SLST(a) ((UINT8)(a & 0x0f))
    testPrintf("spd", "00000F0F04070408", (int)strlen("00000F0F04070408"), "%2X%2X%2X%2X%2X%2X%2X%2X", SMST(0x00) , SLST(0x00), SMST(0xFF), SLST(0xFF), SMST(0x147), SLST(0x147), SMST(0x148), SLST(0x148));

    Test1arg(F("|12345|"), F("|%2X|"), 0x12345);
    Test1arg(F("|12345|"), F("|%4X|"), 0x12345);

    // test test with specifier, space as pad char
    Test1arg(F("|   12|"), F("|%5d|"), 12);
    Test1arg(F("|   12|"), F("|%5u|"), 12);
    Test1arg(F("|    C|"), F("|%5x|"), 12); // %x for PrintLib is %X for printf
    Test1arg(F("|0000C|"), F("|%5X|"), 12);

    // test with specifier, 0 as pad char
    Test1arg(F("|00012|"), F("|%05d|"), 12);
    Test1arg(F("|00012|"), F("|%05u|"), 12);
    Test1arg(F("|0000C|"), F("|%05x|"), 12); // %x for PrintLib is %X for printf
    Test1arg(F("|0000C|"), F("|%05X|"), 12);
	
    Test1arg(F("|80123456|"), F("|%03X|"), 0xFFFFFFFF80123456);
    Test1arg(F("|FFFFFFFF80123456|"), F("|%03lX|"), 0xFFFFFFFF80123456);
    Test1arg(F("|80123456|"), F("|%05X|"), 0xFFFFFFFF80123456);
    Test1arg(F("|80123456|"), F("|%07X|"), 0xFFFFFFFF80123456);
    Test1arg(F("|080123456|"), F("|%09X|"), 0xFFFFFFFF80123456);
    Test1arg(F("|00000000000080123456|"), F("|%020X|"), 0xFFFFFFFF80123456);
    Test1arg(F("|0000FFFFFFFF80123456|"), F("|%020lX|"), 0xFFFFFFFF80123456);

    // test limits
    int16_t i;
    i = INT16_MAX; Test1arg(F("INT16_MAX=32767"), F("INT16_MAX=%d"), i);
    i = INT16_MIN; Test1arg(F("INT16_MIN=-32768"), F("INT16_MIN=%d"), i);

    uint16_t ui16;
    ui16 = UINT16_MAX; Test1arg(F("UINT16_MAX=65535"), F("UINT16_MAX=%u"), ui16);

    int32_t i32;
    i32 = INT32_MAX; Test1arg(F("INT32_MAX=2147483647"), F("INT32_MAX=%d"), i32);
    i32 = INT32_MIN; Test1arg(F("INT32_MIN=-2147483648"), F("INT32_MIN=%d"), i32);

    uint32_t ui32;
    ui32 = UINT32_MAX; Test1arg(F("UINT32_MAX=4294967295"), F("UINT32_MAX=%u"), ui32);

    int64_t i64;
    i64 = INT64_MAX; Test1arg(F("INT64_MAX=9223372036854775807"), F("INT64_MAX=%ld"), i64);
    i64 = INT64_MIN; Test1arg(F("INT64_MIN=-9223372036854775808"), F("INT64_MIN=%ld"), i64);

    uint64_t ui64;
    ui64 = UINT64_MAX; Test1arg(F("UINT64_MAX=18446744073709551615"), F("UINT64_MAX=%lu" ), ui64);


    return nbTestFailed;
}


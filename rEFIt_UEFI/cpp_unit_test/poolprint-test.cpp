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
#include <poolprint-test-cpp_conf.h>
#include "poolprint-test.h"

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


VOID
EFIAPI
_PoolCatPrint (
  IN CONST CHAR16         *fmt,
  IN VA_LIST              args,
  IN OUT POOL_PRINT       *spc,
  IN EFI_STATUS
    (EFIAPI
  *Output)
    (
      POOL_PRINT *context,
      CHAR16 *str
    )
  );

static int testWPrintf(const char* label, const wchar_t*  expectResult, int expectedRet, const wchar_t* format, ...) /*__attribute__((format(printf, 4, 5)))*/;

static int testWPrintf(const char* label, const wchar_t*  expectResult, int expectedRet, const wchar_t* format, ...)
{
  POOL_PRINT  spc;

  ZeroMem (&spc, sizeof (spc));

	va_list valist;
	va_start(valist, format);
//	int vsnwprintf_ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wbuf, sizeof(wbuf)/sizeof(wchar_t), format, valist);
	_PoolCatPrint (format, valist, &spc, _PoolPrint);
	wchar_t* wbuf = spc.Str;
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
		_PoolCatPrint (format, valist, &spc, _PoolPrint); // for stepping with a debugger.
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
	/* char label[1024]; // Visual studio generates __chkstk if declared here */\
    snprintf(label, sizeof(label), F("Test swprintf(%s, %s)"), F(#format), F(#c)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),L##format,c); \
}

#define Test2arg(expectResult,format,c,d) \
{ \
	/* char label[1024]; // Visual studio generates __chkstk if declared here */\
    snprintf(label, sizeof(label), F("Test swprintf(%s, %s, %s)"), F(#format), F(#c), F(#d)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),L##format,c,d); \
}

#define Test5arg(expectResult,format,c,d,e,f,g) \
{ \
	/* char label[1024]; // Visual studio generates __chkstk if declared here */\
    snprintf(label, sizeof(label), F("Test swprintf(%s, %s, %s, %s, %s, %s)"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),L##format,c,d,e,f,g); \
}


int poolprint_tests(void)
{
	char label[1024]; // to avoid __chkstk problem in Visual studio, label is declared here to be used in TestArg macros

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
	Test1arg(F("'utf16-string'"), F("'%ls'"), L"utf16-string");
	Test1arg(F("Āࠀ𐀀🧊Выход'utf8'из"), F("Āࠀ𐀀🧊Выход'%a'из"), "utf8");
	Test1arg(F("Āࠀ𐀀🧊Выход'utf16'из"), F("Āࠀ𐀀🧊Выход'%ls'из"), L"utf16");
//	Test1arg(F("Āࠀ𐀀🧊Выхо'ыход'из"), F("Āࠀ𐀀🧊Выхо'%a'из"), "ыход"); // utf8 chars seems not working with PoolPrint
	Test1arg(F("Āࠀ𐀀🧊Выхо'ыход'из"), F("Āࠀ𐀀🧊Выхо'%ls'из"), L"ыход");


    // These must always works. It also test that integer type are well defined
    Test1arg(F("sizeof(uint8_t)=1"), F("sizeof(uint8_t)=%d"), sizeof(uint8_t)); // %zu not supported by PoolPrint
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

    // %u not supported by PoolPrint
    // %f not supported by PoolPrint


    // test format
    Test1arg(F("12"), F("%d"), 12);
    Test1arg(F("|A|"), F("|%x|"), 0xA); // %x for PoolPrint is %X for printf
    Test1arg(F("|AB|"), F("|%x|"), 0xAB); // %x for PoolPrint is %X for printf
    Test1arg(F("|ABF|"), F("|%x|"), 0xABF); // %x for PoolPrint is %X for printf
    Test1arg(F("|ABFE|"), F("|%x|"), 0xABFE); // %x for PoolPrint is %X for printf
    Test1arg(F("|ABFED|"), F("|%x|"), 0xABFED); // %x for PoolPrint is %X for printf
    Test1arg(F("|00000ABF|"), F("|%X|"), 0xABF); // %X for PoolPrint is %08X for printf, length specifier is ignored

    // test with specifier, space as pad char
    Test1arg(F("|    0|"), F("|%5d|"), 0);
    Test1arg(F("|    0|"), F("|%5x|"), 0); // %x for PoolPrint is %X for printf
    Test1arg(F("|00000000|"), F("|%5X|"), 0); // %X means %08X, length specifier is ignored

    // test with specifier too small, space as pad char
    Test1arg(F("|1234|"), F("|%2d|"), 1234); // keep under 16 bit value, if not, on 16 bits CPU, the constant become long int and doesn't match %d
    Test1arg(F("|ABFE|"), F("|%2x|"), 0xABFE); // %x for PoolPrint is %X for printf
    Test1arg(F("|0000ABFE|"), F("|%2X|"), 0xABFE); // %X for PoolPrint is %08X for printf, length specifier is ignored

    // test test with specifier, space as pad char
    Test1arg(F("|   12|"), F("|%5d|"), 12);
    Test1arg(F("|    C|"), F("|%5x|"), 12); // %x for PoolPrint is %X for printf
    Test1arg(F("|0000000C|"), F("|%5X|"), 12); // %X for PoolPrint is %08X for printf, length specifier is ignored

    // test with specifier, 0 as pad char
    Test1arg(F("|00012|"), F("|%05d|"), 12);
    Test1arg(F("|0000C|"), F("|%05x|"), 12); // %x for PoolPrint is %X for printf
    Test1arg(F("|0000000C|"), F("|%05X|"), 12); // %X for PoolPrint is %08X for printf, length specifier is ignored


    // Test %F format
    Test2arg(F("Flash string |string1| |striiiing2|"), F("Flash string |" PRIF "| |" PRIF "|"), F("string1"), F("striiiing2"));

    // test limits
    int16_t i;
    i = INT16_MAX; Test1arg(F("INT16_MAX=32767"), F("INT16_MAX=%d"), i);
    i = INT16_MIN; Test1arg(F("INT16_MIN=-32768"), F("INT16_MIN=%d"), i);

    uint16_t ui16;
    ui16 = UINT16_MAX; Test1arg(F("UINT16_MAX=65535"), F("UINT16_MAX=%d"), ui16);

    int32_t i32;
    i32 = INT32_MAX; Test1arg(F("INT32_MAX=2147483647"), F("INT32_MAX=%d"), i32);
    i32 = INT32_MIN; Test1arg(F("INT32_MIN=-2147483648"), F("INT32_MIN=%d"), i32);

//    uint32_t ui32;
//    ui32 = UINT32_MAX; Test1arg(F("UINT32_MAX=4294967295"), F("UINT32_MAX=%ld"), ui32); // PoolPrint print -1
//
//    int64_t i64;
//    i64 = INT64_MAX; Test1arg(F("INT64_MAX=9223372036854775807"), F("INT64_MAX=%ld"), i64); // PoolPrint print -1
//    i64 = INT64_MIN; Test1arg(F("INT64_MIN=-9223372036854775808"), F("INT64_MIN=%ld"), i64); // PoolPrint print 0
//
//    uint64_t ui64;
//    ui64 = UINT64_MAX; Test1arg(F("UINT64_MAX=18446744073709551615"), F("UINT64_MAX=%ld" ), ui64); // PoolPrint print -1


    return nbTestFailed;
}


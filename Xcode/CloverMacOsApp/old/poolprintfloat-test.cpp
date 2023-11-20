//
//  main.cpp
//  Printf-UnitTests
//
//  Created by Jief on 29/08/17.
//  Copyright © 2017 Jief. All rights reserved.
//

#include <Platform.h>
#include <limits.h>
#include "unicode_conversions.h"
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

#include "../libeg/FloatLib.h"

static int testPoolPrintFloat(const char* label, const wchar_t*  expectResult, float param)
{
	wchar_t* wbuf = PoolPrintFloat(param);
	if ( memcmp(wbuf, expectResult, wchar_len(expectResult)*sizeof(expectResult[0])) != 0 ) {
//		loggf(F(" -> ERROR. Expect " PRILF " and get %ls\n"), expectResult, buf);
//      not using wprintf, it crashes sometimes, it doesn't work for short-wchar
		loggf(F("%s -> ERROR. Expect "), label);
		print_wchar_string(expectResult);
		loggf(F(" and get "));
		print_wchar_string(wbuf);
		loggf("\n");
		nbTestFailed += 1;
		wchar_t* wbuf2 = PoolPrintFloat(param); // for stepping with a debugger.
		FreePool(wbuf2);
	}else if ( !displayOnlyFailed ) {
		loggf(F("%s : "), label);
		print_wchar_string(wbuf);
		loggf(F(" -> OK\n"));
	}
	FreePool(wbuf);
	return 1;
}



#define Test1arg(expectResult,param) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("testPoolPrintFloat(%s)"), F(#param)); \
    testPoolPrintFloat(label,L##expectResult,param); \
}

#define Test2arg(expectResult,format,c,d) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("Test swprintf(%s, %s, %s)"), F(#format), F(#c), F(#d)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),L##format,c,d); \
}

#define Test5arg(expectResult,format,c,d,e,f,g) \
{ \
	char label[1024]; \
    snprintf(label, sizeof(label), F("Test swprintf(%s, %s, %s, %s, %s, %s)"), F(#format), F(#c), F(#d), F(#e), F(#f), F(#g)); \
    testWPrintf(label,L##expectResult,(int)wcslen(L##expectResult),L##format,c,d,e,f,g); \
}


int poolprintfloat_tests(void)
{
#ifdef DISPLAY_START_INFO
	loggf(F("\n"));
	loggf(F("PoolPrintFloat unit test\n"));
	loggf(F("\n"));
	loggf(F("\n"));

    // These depends on the plateform. They are not printf unit test, but it's nice to check size of builtin type.
	loggf(F("sizeof(float)=%lu\n"), sizeof(float));
    loggf(F("sizeof(double)=%zu\n"), sizeof(double));
    loggf(F("\n"));

#endif

	Test1arg(F(" 0.000000"), 0.0);
	Test1arg(F(" 0.123456"), 0.1234567890);
	Test1arg(F("-0.123456"), -0.1234567890);
	Test1arg(F(" 1.100000"), 1.1);
	Test1arg(F(" -1.100000"), -1.1);
	Test1arg(F(" 123.456787"), 123.456789);
	Test1arg(F(" -123.456787"), -123.456789);
	Test1arg(F(" 1234567936.000000"), 1234567890.456789);
	Test1arg(F(" -1234567936.000000"), -1234567890.456789);
	Test1arg(F(" 0.000000"), 12345678901234567890.456789);
	Test1arg(F(" 0.000000"), -12345678901234567890.456789);


    return nbTestFailed;
}


#include <Platform.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/utf8Conversion.h"
#include "global_test.h"


//#include <wchar.h>


int XString_tests()
{

#ifdef JIEF_DEBUG
//	DebugLog(2, "XStringW_tests -> Enter\n");
#endif

	if ( global_str1 != "global_str1" ) return 1;
	if ( global_str2 != "global_str2" ) return 2;

	// Check default ctor
	{
		XString str;
		if (str.length() != 0) return 3;
		if (str.c_str() == NULL) return 4;
	}
	
	// Check ctor with value (or check takeValueFrom while we are waiting to put back ctor(const char*)
#ifdef XSTRINGW_HAS_CTOR_LITTERAL
	{
		{
			XString str("");
			if ( str != "" ) return 10;
		}
		XString str("1");
		if ( str != "1" ) return 11;
	}
#else
	{
		XString str;
		str.takeValueFrom("");
		if (str.length() != 0) return 10;
		str.takeValueFrom("1");
		if ( str != "1" ) return 11;
		str.StrCat("2");
		if ( str != "12" ) return 12;
	}
#endif

	// Check StrCat. TODO more test, and test StrnCat
	{
		XString str;
		str.takeValueFrom("1");
		str.StrCat("2");
		if ( str != "12" ) return 20;
	}
	
  // check takeValueFrom from utf8 string
	XString str;
	str.takeValueFrom("Ň"); // this is a UTF8 string 2 bytes long
	if (str != "Ň") return 30; // utf8 litteral are converted to an XStringW if ctor is available.
	str.takeValueFrom("𐌾"); // this is a UTF8 string 4 bytes long
	if ( str != "𐌾" ) return 31;
	str.takeValueFrom("𐌾"); // this is a UTF16 or UTF32 string (depending of -fshort-wchar)
	if ( str != "𐌾" ) return 32;

  // check takeValueFrom from UTF16 or 32 string
	str.takeValueFrom(L"Ň"); // this is a UTF8 string 2 bytes long
	if (str != "Ň") return 33; // utf8 litteral are converted to an XStringW if ctor is available.
	str.takeValueFrom(L"𐌾"); // this is a UTF8 string 4 bytes long
	if ( str != "𐌾" ) return 34;
	str.takeValueFrom(L"𐌾"); // this is a UTF16 or UTF32 string (depending of -fshort-wchar)
	if ( str != "𐌾" ) return 35;

  // Quick check of StrnCpy,StrnCat,Insert,+=
	str.takeValueFrom("12");
	XString str2;
	if ( !str2.isEmpty() ) return 100;
	str2.StrnCpy(str.data(), 2);
	if ( str2 != "12" ) return 101;
	str2.StrnCat("345", 2);
	if ( str2 != "1234" ) return 102;
	str2.Insert(1, str);
	if ( str2 != "112234" ) return 103;
	str2 += "6";
	if ( str2 != "1122346" ) return 104;
  // Check Insert at beginning and end
	str2.takeValueFrom("123");
	str2.Insert(0, XString().takeValueFrom("x"));
	if ( str2 != "x123" ) return 105;
	str2.Insert(4, XString().takeValueFrom("y"));
	if ( str2 != "x123y" ) return 106;

//wchar_t c2 = L'Ň';
//printf("1=%lc\n", c2);
//const char* s1 = "𐌾";
  // Check SPrintf
	str2.SPrintf("%c", 'a'); // single UTF8 ascii char
	if ( str2 != "a" ) return 200;
	#ifndef _MSC_VER
		str2.SPrintf("%lc", L'Ň'); // single UTF16(32) char. (2 bytes in total if UTF16)
		if ( str2 != "Ň" ) return 202;
	#endif
	str2.SPrintf("%s", "Ň"); // this is a UTF8 string 2 bytes long
	if (str2 != "Ň") return 203; // utf8 litteral are converted to an XStringW if ctor is available.
	str2.SPrintf("%ls", L"Ň"); // this is a UTF8 string 2 bytes long
	if (str2 != "Ň") return 204; // utf8 litteral are converted to an XStringW if ctor is available.
#if __WCHAR_MAX__ > 0xFFFFu
	str2.SPrintf("%lc", L'𐌾'); // L'𐌾' // this char cannot convert to an UTF16 char. So it doesn't compile with -fshort-wchar
	if ( str2 != "𐌾" ) return 205;
#endif

/* Stil doesn't work as VS doesn't encode correctly litteral */
#ifndef _MSC_VER

//	int i1 = sizeof(wchar_t);
//	int i2 = sizeof(char16_t);
//	const wchar_t* ls = L"Выход";
//	const char16_t* us = u"Выход";
//	const char32_t* Us = U"Выход";
	XString str3;
	str3.takeValueFrom(L"Выход");
	if ( str3 != "Выход" ) return 500;
#endif
//MsgLog("Test MsgLog ascii=%ls ucs-2=%ls\n", "a string", L"ascii char in ucs-2 string\n");
//MsgLog("Test MsgLog ascii=%ls ucs-2=%ls\n", "a string", "ascii char in ucs-2 string\n");
//MsgLog("Test MsgLog ascii=%ls ucs-2=%ls\n", "a string", "ascii char in ucs-2 string\n");
//
//MsgLog("Test MsgLog ascii=%ls ucs-2=%ls\n", "a string", L"Выход из подменю, обновление главного меню\n");
//MsgLog("Test MsgLog ascii=%ls ucs-2=%ls\n", "a string", "Выход из подменю, обновление главного меню\n");
//MsgLog("Test MsgLog ascii=%ls ucs-2=%ls\n", "a string", "Выход из подменю, обновление главного меню\n");
//
//DBG("Test ascii=%ls ucs-2=%ls\n", "a string", L"Выход из подменю, обновление главного меню\n");
//DBG("Test ascii=%ls ucs-2=%ls\n", "a string", "Выход из подменю, обновление главного меню\n");
//DBG("Test ascii=%ls ucs-2=%ls\n", "a string", "Выход из подменю, обновление главного меню\n");

	return 0;
}

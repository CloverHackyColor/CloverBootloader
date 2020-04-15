#include <Platform.h>
#include "../cpp_foundation/XStringW.h"
#include "../cpp_foundation/unicode_conversions.h"
#include "global_test.h"


//#include <wchar.h>


int XStringW_tests()
{

#ifdef JIEF_DEBUG

	XStringW a = L"toto"_XSW;

//	DebugLog(2, "XStringW_tests -> Enter\n");
#endif

	if ( global_str3 != L"global_str3" ) return 1;
	if ( global_str4 != L"global_str4" ) return 2;

	// Check default ctor
	{
		XStringW str;
		if (str.size() != 0) return 3;
		if (str.wc_str() == NULL) return 4;
	}
	

	// Check ctor with value (or check takeValueFrom while we are waiting to put back ctor(const char*)
#ifdef XSTRINGW_HAS_CTOR_LITTERAL
	{
		{
			XStringW str("");
			if ( str != "" ) return 100;
		}
		{
			XStringW str("1");
			if ( str != "1" ) return 101;
		}
	}
#else
	{
		XStringW str;
		str.takeValueFrom("");
		if (str.size() != 0) return 110;
		str.takeValueFrom("1");
		if ( str != L"1" ) return 111;
		str.StrCat(L"2");
		if ( str != L"12" ) return 112;
	}
#endif
	
//	// check [] operator
//	{
//		XStringW str;
//		str.takeValueFrom("01234567890123456789");
//		wchar_t c;
//		c = str[(char)1];
//		if ( c != '1' ) return 201;
//		c = str[(unsigned char)2];
//		if ( c != '2' ) return 202;
//		c = str[(short)3];
//		if ( c != '3' ) return 203;
//		c = str[(unsigned short)4];
//		if ( c != '4' ) return 204;
//		c = str[(int)5];
//		if ( c != '5' ) return 205;
//		c = str[(unsigned int)6];
//		if ( c != '6' ) return 206;
//		c = str[(long)7];
//		if ( c != '7' ) return 207;
//		c = str[(unsigned long)8];
//		if ( c != '8' ) return 208;
//		c = str[(long long)9];
//		if ( c != '9' ) return 209;
//		c = str[(unsigned long long)10];
//		if ( c != '0' ) return 210;
//	}

	// Quick check of StrnCpy,StrnCat,Insert,+=
	{
		XStringW str;
		str.takeValueFrom(L"12");
		XStringW str2;
		if ( !str2.isEmpty() ) return 10;
		str2.StrnCpy(str.data(), 2);
		if ( str2 != L"12" ) return 11;
		str2.StrnCat(L"345", 2);
		if ( str2 != L"1234" ) return 12;
		str2.Insert(1, str);
		if ( str2 != L"112234" ) return 13;
		str2 += L"6";
		if ( str2 != L"1122346" ) return 14;
	}

//wchar_t c2 = L'Ň';
//printf("1=%lc\n", c2);
//const char* s1 = "𐌾";
	XStringW str2;
	str2.SWPrintf("%c", 'a'); // signle UTF8 ascii char
	if ( str2 != L"a" ) return 20;
	str2.takeValueFrom(L"ab"); // UTF16(32) string containing ascii char
	if ( str2 != L"ab" ) return 21;
#ifdef _MSC_VER
	// IMPORTANT : you can't pass a litteral char in a vararg function with Visual Studio (Microsoft strikes again :-).
	//             At least, you got a warning C4066
	// IMPORTANT2 : Litteral string containing UTF16 char are WRONG. And you don't get a warning !!! If litteral is only ascii, it's ok.
	// Maybe it's compilation option but I didn't find them.
	wchar_t c = 'Ň'; // using an imtermediary var for Microsoft.

	
	wchar_t s[2]; // wchar_t s2[] = L"Ň";
	s[0] = 'Ň';
	s[1] = 0;

	str2.SWPrintf("%lc", c); // UTF16(32) char. (2 bytes in total if UTF16)
	if (str2 != s) return 22;
	str2.takeValueFrom("");
	if (str2.length() != 0) return 221;
	str2.takeValueFrom(s); // this is a UTF8 string 2 bytes long
	if (str2 != s) return 23;
#else
	str2.SWPrintf("%lc", L'Ň'); // signe UTF16(32) char. (2 bytes in total if UTF16)
	if ( str2 != L"Ň" ) return 22;
	str2.takeValueFrom("");
	if (str2.size() != 0) return 221;
#ifdef XSTRINGW_HAS_CTOR_LITTERAL
	str2.takeValueFrom("Ň"); // this is a UTF8 string 2 bytes long
	if (str2 != "Ň") return 23; // utf8 litteral are converted to an XStringW if ctor is available.
#endif
	str2.takeValueFrom("");
	if (str2.size() != 0) return 231;
#ifdef XSTRINGW_HAS_CTOR_LITTERAL
	str2.takeValueFrom(L"Ň"); // this is a UTF8 string 2 bytes long
	if (str2 != "Ň") return 24;
#endif
#endif

#if __WCHAR_MAX__ > 0xFFFFu
	str2.SWPrintf("%lc", L'𐌾'); // L'𐌾' // this char cannot convert to an UTF16 char. So it doesn't compile with -fshort-wchar
	if ( str2 != L"𐌾" ) return 30;
#endif


#ifndef _MSC_VER
	// "𐌾" in UTF16 is 2 char : 0xd800, 0xdf3e

	str2.takeValueFrom(L"𐌾"); // this is a UTF8 string 4 bytes long
	if ( str2 != L"𐌾" ) return 31;
	str2.takeValueFrom(L"𐌾"); // this is a UTF16 or UTF32 string (depending of -fshort-wchar)
	if ( str2 != L"𐌾" ) return 32;

#ifdef XSTRINGW_HAS_CTOR_LITTERAL
	{
		XStringW str3("a");
		if ( str3 != L"a" ) return 40;
		XStringW str4("aŇ𐌾");
		if ( str4 != L"aŇ𐌾" ) return 41;
	}
#endif
#endif

	{
		XStringW utf16;
		utf16.takeValueFrom(L"Выход из подменю, обновление главного меню");
		for ( size_t i = 0 ; i < utf16.size() ; i++ ) {
			if ( utf16.wc_str()[i] != utf16.wc_str()[i] ) {
				return 100;
			}
		}
		XStringW utf16_2;
		utf16_2.takeValueFrom("Выход из подменю, обновление главного меню");
		if ( utf16 != utf16_2 ) {
			return 101;
		}
	}
	
	wchar_t* s = XStringW().takeValueFrom("aa").forgetDataWithoutFreeing();
	if ( s != L"aa"_XSW ) return 102;
	
//  XStringW CommonName(L"EFI\\CLOVER\\misc\\screenshot");
//  for (UINTN Index = 0; Index < 20; Index++) {
//   XStringW Name = CommonName + SPrintf("%lld", Index) + L".png";
//    DebugLog(2, "XStringW_test shot: %s\n", Name.data());
//  }
	return 0;
}

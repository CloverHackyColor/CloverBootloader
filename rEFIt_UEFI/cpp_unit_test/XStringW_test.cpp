#include <Platform.h>
#include "../cpp_foundation/XStringW.h"
#include "../cpp_foundation/utf8Conversion.h"
#include "global_test.h"


//#include <wchar.h>


int XStringW_tests()
{

#ifdef JIEF_DEBUG
	DebugLog(2, "XStringW_tests -> Enter\n");
#endif
//XStringW a = " ";
XStringW b;
//b = a;
//b = " ";

	if ( global_str1 != L"global_str1" ) return 1;
	if ( global_str2 != L"global_str2" ) return 2;

#ifdef XSTRINGW_HAS_CTOR_LITTERAL
	XStringW str(L"1");
	if ( str != L"1" ) return 3;
	str.StrCat(L"2");
	if ( str != L"12" ) return 4;
#endif

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

//wchar_t c2 = L'Ň';
//printf("1=%lc\n", c2);
//const char* s1 = "𐌾";
	
	str2.SPrintf("%c", 'a'); // signle UTF8 ascii char
	if ( str2 != L"a" ) return 20;
	str2.takeValueFrom(L"ab"); // UTF16(32) string containing ascii char
	if ( str2 != L"ab" ) return 21;
#ifdef _MSC_VER
	// IMPORTANT : you can't pass a litteral char in a vararg function with Visual Studio (Microsoft strikes again :-). 
	//             At least, you got a warning C4066
	// IMPORTANT2 : Litteral string containing UTF16 char are WRONG. And you don't get a warning !!! If litteral is only ascii, it's ok.
	// Maybe it's compilation option butI didn't find them.
	wchar_t c = 'Ň'; // using an imtermediary var for Microsoft.

	
	wchar_t s[2]; // wchar_t s2[] = L"Ň";
	s[0] = 'Ň';
	s[1] = 0;

	str2.SPrintf("%lc", c); // UTF16(32) char. (2 bytes in total if UTF16)
	if (str2 != s) return 22;
	str2.takeValueFrom("");
	if (str2.length() != 0) return 221;
	str2.takeValueFrom(s); // this is a UTF8 string 2 bytes long
	if (str2 != s) return 23;
#else
	str2.SPrintf("%lc", L'Ň'); // signe UTF16(32) char. (2 bytes in total if UTF16)
	if ( str2 != L"Ň" ) return 22;
	str2.takeValueFrom("");
	if (str2.length() != 0) return 221;
#ifdef XSTRINGW_HAS_CTOR_LITTERAL
	str2.takeValueFrom("Ň"); // this is a UTF8 string 2 bytes long
	if (str2 != "Ň") return 23; // utf8 litteral are converted to an XStringW if ctor is available.
#endif
	str2.takeValueFrom("");
	if (str2.length() != 0) return 231;
#ifdef XSTRINGW_HAS_CTOR_LITTERAL
	str2.takeValueFrom(L"Ň"); // this is a UTF8 string 2 bytes long
	if (str2 != "Ň") return 24;
#endif
#endif

#if __WCHAR_MAX__ > 0xFFFFu
	str2.SPrintf("%lc", L'𐌾'); // L'𐌾' // this char cannot convert to an UTF16 char. So it doesn't compile with -fshort-wchar
	if ( str2 != L'𐌾' ) return 30;
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

//  XStringW CommonName(L"EFI\\CLOVER\\misc\\screenshot");
//  for (UINTN Index = 0; Index < 20; Index++) {
//   XStringW Name = CommonName + SPrintf("%lld", Index) + L".png";
//    DebugLog(2, "XStringW_test shot: %s\n", Name.data());
//  }
	return 0;
}

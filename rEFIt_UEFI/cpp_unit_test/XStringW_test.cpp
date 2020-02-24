#include "../cpp_foundation/XStringW.h"
#include "global1.h"
#include "global2.h"
#include "../cpp_foundation/utf8Conversion.h"


//#include <wchar.h>


int XStringW_tests()
{

#ifdef JIEF_DEBUG
	DebugLog(2, "XStringW_tests -> Enter\n");
#endif

	if ( global_str1 != L"global_str1" ) return 1;
	if ( global_str2 != L"global_str2" ) return 2;

	XStringW str(L"1");
	if ( str != L"1" ) return 3;
	str.StrCat(L"2");
	if ( str != L"12" ) return 4;

	XStringW str2;
	if ( str2.NotNull() ) return 10;
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
	str2.SPrintf("%ls", L"ab"); // UTF16(32) string containing ascii char
	if ( str2 != L"ab" ) return 21;

	str2.SPrintf("%lc", L'Ň'); // signe UTF16(32) char. (2 bytes in total if UTF16)
	if ( str2 != L"Ň" ) return 22;
	str2.SPrintf("%s", "Ň"); // this is a UTF8 string 2 bytes long
	if ( str2 != L"Ň" ) return 23;

#if __WCHAR_MAX__ > 0xFFFFu
	str2.SPrintf("%lc", L'𐌾'); // L'𐌾' // this char cannot convert to an UTF16 char. So it doesn't compile with -fshort-wchar
	if ( str2 != L'𐌾' ) return 30;
#endif
	str2.SPrintf("%ls", L"𐌾"); // this is a UTF8 string 4 bytes long
	if ( str2 != L"𐌾" ) return 31;
	str2.SPrintf("%ls", L"𐌾"); // this is a UTF16 or UTF32 string (depending of -fshort-wchar)
	if ( str2 != L"𐌾" ) return 32;

	{
		XStringW str3("a");
		if ( str3 != L"a" ) return 40;
		XStringW str4("aŇ𐌾");
		if ( str4 != L"aŇ𐌾" ) return 41;
	}
	return 0;
}

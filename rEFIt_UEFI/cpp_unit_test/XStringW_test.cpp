#include "../cpp_foundation/XStringW.h"
#include "global1.h"
#include "global2.h"


int XStringW_tests()
{

#ifdef JIEF_DEBUG
	DebugLog(2, "XStringW_tests -> Enter\n");
#endif

	if ( global_str1 != L"global_str1" ) return 1;
	if ( global_str2 != L"global_str2" ) return 1;

	{
		XStringW str(L"1");
		if ( str != L"1" ) return 1;
		str.StrCat(L"2");
		if ( str != L"12" ) return 1;

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
	}
	
	return 0;
}

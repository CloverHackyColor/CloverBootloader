#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
#include "global_test.h"



int BootOptions_tests()
{

#ifdef JIEF_DEBUG
//	printf("XStringW_tests -> Enter\n");
#endif

	{
		XString8Array LoadOptions;
		
		LoadOptions.AddID("opt1"_XS8);
		LoadOptions.AddID("opt2"_XS8);
		LoadOptions.AddID("opt3"_XS8);
		
		if ( LoadOptions.ConcatAll(" "_XS8) != "opt1 opt2 opt3"_XS8 ) return 30;
		
		XString8Array LoadOptions1 = LoadOptions;
		LoadOptions1.remove("opt1"_XS8);
		if ( LoadOptions1.ConcatAll(" "_XS8) != "opt2 opt3"_XS8 ) return 31;
		XString8Array LoadOptions2 = LoadOptions;
		LoadOptions2.remove("opt2"_XS8);
		if ( LoadOptions2.ConcatAll(" "_XS8) != "opt1 opt3"_XS8 ) return 32;
		XString8Array LoadOptions3 = LoadOptions;
		LoadOptions3.remove("opt3"_XS8);
		if ( LoadOptions3.ConcatAll(" "_XS8) != "opt1 opt2"_XS8 ) return 33;
	}

	return 0;
}


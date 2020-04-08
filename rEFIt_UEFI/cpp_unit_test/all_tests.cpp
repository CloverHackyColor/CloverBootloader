#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XStringW.h"
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XObjArray.h"

#include "../Platform/BasicIO.h" // for PauseForKey

#include "XArray_tests.h"
#include "XObjArray_tests.h"
#include "XStringWArray_test.h"
#include "XString_test.h"
#include "XStringW_test.h"
#include "strcmp_test.h"
#include "strncmp_test.h"
#include "strlen_test.h"
#include "printf_lite-test.h"
#include "LoadOptions_test.h"
//#include "poolprint-test.h"
//#include "printlib-test.h"


/* On macOS
 * sizeof(long)=8
 * sizeof(long long)=8
 * sizeof(size_t)=8
 */

bool all_tests()
{
  bool all_ok = true;
  int ret;

#ifdef JIEF_DEBUG
//	ret = XUINTN_tests();
//	if ( ret != 0 ) {
//		DebugLog(2, "XUINTN_tests() failed at test %d\n", ret);
//		all_ok = false;
//	}
	#if defined(CLOVER_BUILD)
//	    ret = printlib_tests();
//	    if ( ret != 0 ) {
//		    DebugLog(2, "printlib_tests() failed at test %d\n", ret);
//		    all_ok = false;
//    	}
//    	ret = poolprint_tests();
//    	if ( ret != 0 ) {
//	    	DebugLog(2, "poolprint_tests() failed at test %d\n", ret);
//		    all_ok = false;
//	    }
	#endif
#endif
#ifndef _MSC_VER
	ret = printf_lite_tests();
	if ( ret != 0 ) {
		DebugLog(2, "printf_lite_tests() failed at test %d\n", ret);
		all_ok = false;
	}
#endif
#ifdef JIEF_DEBUG
//return ret;
#endif
	ret = strlen_tests();
	if ( ret != 0 ) {
		DebugLog(2, "strlen_tests() failed at test %d\n", ret);
		all_ok = false;
	}
	ret = BootOptions_tests();
	if ( ret != 0 ) {
		DebugLog(2, "BootOptions_tests() failed at test %d\n", ret);
		all_ok = false;
	}
	ret = strcmp_tests();
	if ( ret != 0 ) {
		DebugLog(2, "strcmp_tests() failed at test %d\n", ret);
		all_ok = false;
	}
	ret = strncmp_tests();
	if ( ret != 0 ) {
		DebugLog(2, "strncmp_tests() failed at test %d\n", ret);
		all_ok = false;
	}
	ret = XArray_tests();
	if ( ret != 0 ) {
		DebugLog(2, "XArray_tests() failed at test %d\n", ret);
		all_ok = false;
	}
	ret = XObjArray_tests();
	if ( ret != 0 ) {
		DebugLog(2, "XObjArray_tests() failed at test %d\n", ret);
		all_ok = false;
	}
#ifndef _MSC_VER
	ret = XString_tests();
	if ( ret != 0 ) {
		DebugLog(2, "XString_tests() failed at test %d\n", ret);
		all_ok = false;
	}
	ret = XStringW_tests();
	if ( ret != 0 ) {
		DebugLog(2, "XStringW_tests() failed at test %d\n", ret);
		all_ok = false;
	}
#endif
	ret = XStringWArray_tests();
	if ( ret != 0 ) {
		DebugLog(2, "XStringWArray_tests() failed at test %d\n", ret);
		all_ok = false;
	}
//	ret = XUINTN_tests();
//	if ( ret != 0 ) {
//		DebugLog(2, "XUINTN_tests() failed at test %d\n", ret);
//		all_ok = false;
//	}

	if ( !all_ok ) {
		DebugLog(2, "A test failed\n");
	}
	
#if defined(JIEF_DEBUG)
	if ( !all_ok ) {
		printf("All tests are NOT ok\n");
	}else{
		printf("All tests are ok\n");
	}
#endif
	
#if defined(CLOVER_BUILD) && defined(JIEF_DEBUG)
	if ( !all_ok ) {
//		PauseForKey(L"press");
	}else{
		PauseForKey(L"press");
	}
#endif

	return all_ok;
}

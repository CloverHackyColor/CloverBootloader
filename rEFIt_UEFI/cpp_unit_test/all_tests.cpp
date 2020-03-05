#include <Platform.h>
#include "../cpp_foundation/XStringW.h"
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XObjArray.h"

#include "XArray_tests.h"
#include "XObjArray_tests.h"
#include "XStringWArray_test.h"
#include "XStringW_test.h"

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

bool all_tests()
{
  bool all_ok = true;
  int ret;

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
	ret = XStringW_tests();
	if ( ret != 0 ) {
		DebugLog(2, "XStringW_tests() failed at test %d\n", ret);
		all_ok = false;
	}
	ret = XStringWArray_tests();
	if ( ret != 0 ) {
		DebugLog(2, "XStringWArray_tests() failed at test %d\n", ret);
		all_ok = false;
	}

	if ( !all_ok ) {
		DebugLog(2, "A test failed\n");
		PauseForKey(L"press");
	}else{
		DebugLog(2, "All tests are ok\n");
#ifdef JIEF_DEBUG
//		PauseForKey(L"press");
#endif
	}
	return all_ok;
}

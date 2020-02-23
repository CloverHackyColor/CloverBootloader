#include "../cpp_foundation/XStringW.h"
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XObjArray.h"

#include "XArray_tests.h"
#include "XObjArray_tests.h"

#include "../Platform/Platform.h"

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

	if ( !all_ok ) {
		DebugLog(2, "A test failed, module %d, test %d \n");
		PauseForKey(L"press");
	}else{
#ifdef JIEF_DEBUG
		DebugLog(2, "All tests are ok\n");
		PauseForKey(L"press");
#endif
	}
	return all_ok;
}

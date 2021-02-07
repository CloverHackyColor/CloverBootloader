#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XArray.h"
#include "../cpp_foundation/XObjArray.h"

#include "../Platform/BasicIO.h" // for PauseForKey

#include "XArray_tests.h"
#include "XObjArray_tests.h"
#include "XStringArray_test.h"
#include "XString_test.h"
#include "strcmp_test.h"
#include "strncmp_test.h"
#include "strlen_test.h"
#include "printf_lite-test.h"
#include "LoadOptions_test.h"
#include "XToolsCommon_test.h"
#include "XBuffer_tests.h"
#include "plist_tests.h"
#include "find_replace_mask_Clover_tests.h"
#include "find_replace_mask_OC_tests.h"
#include "MacOsVersion_test.h"
#include "xml_lite-test.h"
#include "config-test.h"

#if defined(JIEF_DEBUG) && defined(CLOVER_BUILD)
  #include "printlib-test.h"
#endif


/* On macOS
 * sizeof(long)=8
 * sizeof(long long)=8
 * sizeof(size_t)=8
 */

bool all_tests()
{
  bool all_ok = true;


#if defined(JIEF_DEBUG)
  int ret;
  ret = xml_lite_tests();
  if ( ret != 0 ) {
    printf("xml_lite_tests() failed at test %d\n", ret);
    all_ok = false;
  }
//  ret = config_plist_tests();
//  if ( ret != 0 ) {
//    printf("config_plist_tests() failed at test %d\n", ret);
//    all_ok = false;
//  }
//
//  ret = XUINTN_tests();
//  if ( ret != 0 ) {
//      printf("XUINTN_tests() failed at test %d\n", ret);
//      all_ok = false;
//  }
#endif

#if defined(JIEF_DEBUG)

#if defined(JIEF_DEBUG) && defined(CLOVER_BUILD)
    ret = printlib_tests();
      if ( ret != 0 ) {
        printf("printlib_tests() failed at test %d\n", ret);
        all_ok = false;
      }
#endif
#ifndef _MSC_VER
  ret = printf_lite_tests();
  if ( ret != 0 ) {
    printf("printf_lite_tests() failed at test %d\n", ret);
    all_ok = false;
  }
#endif
  ret = strlen_tests();
  if ( ret != 0 ) {
    printf("strlen_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = strcmp_tests();
  if ( ret != 0 ) {
    printf("strcmp_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = strncmp_tests();
  if ( ret != 0 ) {
    printf("strncmp_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = XToolsCommon_tests();
  if ( ret != 0 ) {
    printf("printlib_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = XArray_tests();
  if ( ret != 0 ) {
    printf("XArray_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = XObjArray_tests();
  if ( ret != 0 ) {
    printf("XObjArray_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = XString_tests();
  if ( ret != 0 ) {
    printf("XString_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = XStringArray_tests();
  if ( ret != 0 ) {
    printf("XStringArray_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = BootOptions_tests();
  if ( ret != 0 ) {
    printf("BootOptions_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = XBuffer_tests();
  if ( ret != 0 ) {
    printf("XBuffer_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = find_replace_mask_Clover_tests();
  if ( ret != 0 ) {
    printf("xml_lite_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = find_replace_mask_OC_tests();
  if ( ret != 0 ) {
    printf("xml_lite_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = ParseXML_tests();
  if ( ret != 0 ) {
    printf("plist_tests() failed at test %d\n", ret);
    all_ok = false;
  }
  ret = MacOsVersion_tests();
  if ( ret != 0 ) {
    printf("MacOsVersion_tests() failed at test %d\n", ret);
    all_ok = false;
  }

#endif

  if ( !all_ok ) {
    printf("A test failed\n");
  }
  
#if defined(JIEF_DEBUG)
  if ( all_ok ) {
    printf("All tests succeeded\n");
  }
#endif
  
#if defined(CLOVER_BUILD) && defined(JIEF_DEBUG)
  if ( all_ok ) {
//    PauseForKey(L"press");
  }else{
    PauseForKey(L"press");
  }
#endif

  return all_ok;
}

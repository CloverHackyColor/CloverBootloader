#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/unicode_conversions.h"
#include "global_test.h"

//size_t clover_strlen (const char *str);

static int len(const char* s1, size_t count, int code)
{
//printf("strlen of '%s'\n", s1);

	size_t ret1 = strlen(s1);

	if ( ret1 != count ) {
		printf("strlen of '%s' gives %zu and should have given %zu\n", s1, ret1, count);
		return code;
	}
	return 0;
}

static int strlen_s1_all_length(const char* s1, size_t count, int code)
{
	for ( size_t i=0 ; i < count; i++ )
	{
		int ret = len(s1+i, count-i, code);
		if ( ret != 0 ) return ret;
	}
	return 0;

}


static int strlen_s1_all_offset(const char* s1, size_t count, int code)
{
	for ( size_t i=0 ; i < count; i++ )
	{
		int ret = strlen_s1_all_length(s1+i, count-i, code);
		if ( ret != 0 ) return ret;
	}
	return 0;
}
//
//static int compare_with_variable_sizes(int code)
//{
//	unsigned char s1[514];
//	for ( size_t i=0 ; i < sizeof(s1) ; i++ ) {
//		size_t j=0;
//		for ( j = 0 ; j < sizeof(s1) && j < i ; j++ ) { // not using strcpy to not depend of another test.
//			s1[j] = (unsigned char)(rndf()*255.0);
//		}
//		s1[i] = 0;
//		int ret = compare_s1_with_variable_sizes((const char*)s1, code+(int)i);
//		if ( ret != 0 ) return ret;
//	}
//	return 0;
//}

int strlen_tests()
{

#ifdef JIEF_DEBUG
//	printf("XStringW_tests -> Enter\n");
#endif
	const char* s;
	s = "1234567890"; // use intermediary var to not be optimized out.
	
	if ( strlen(s) != 10 ) return 1;
	
	s = "";
	if ( strlen(s) != 0 ) return 2;
	
	int ret;
	
	s = "12345678901234567890123456789012345678901234567890"; // use intermediary var to not be optimized out.
	ret = strlen_s1_all_offset(s, 50, 10);
	if ( ret != 0 ) return ret;

#ifdef CLOVER_BUILD
//	UINTN start = AsmReadTsc();
//	printf("strlen_tests -> Enter\n");
//
//	for ( UINTN i=0 ; i<100000 ; i++ ) {
//		ret = strlen_s1_all_offset(s, 50, 10);
//	}
//
//	printf("strlen_tests -> Exit\n");
//	UINTN end = AsmReadTsc();
//	printf("Strlen bench time = %d\n", end - start);
#endif


	return 0;
}


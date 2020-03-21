#include <Platform.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/utf8Conversion.h"
#include "global_test.h"

static float rndf() //expected 0..1
{
static UINT32 seed = 12345;
//  UINT16 Rand = 0;
//  AsmRdRand16(&Rand);  //it's a pity panic
//  return (float)Rand / 65536.f;
  seed = seed * 214013 + 2531011;
  float x = (float)seed / 4294967296.0f;
  return x;
}

static int sign(int ret) {
	if ( ret < 0 ) return -1;
	if ( ret > 0 ) return 1;
	return 0;
}

static size_t strlen_reference(const char *str)
{
        const char *s;

        for (s = str; *s; ++s)
                ;
        return (size_t)(s - str);
}

static int strncmp_reference( const char * s1, const char * s2, size_t n )
{
    while ( n && *s1 && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 )
    {
        return 0;
    }
    else
    {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

static int nb_compare = 0;

static int compare(const char*s1, const char*s2, size_t count)
{
	nb_compare ++;
//DebugLog(2, "Comparing '%s' and '%s' with count %d\n", s1, s2, count);
	int ret1 = strncmp(s1, s2, count);
	int ret2 = strncmp_reference(s1, s2, count);

	if ( sign(ret1) != sign(ret2) ) {
		DebugLog(2, "Comparing '%s' and '%s' with count %zu gives %d and should have given %d\n", s1, s2, count, ret1, ret2);
		int ret3 = strncmp(s1, s2, count); // for live debugging
		(void)ret3;
		return 1; // whatever if not 0
	}
	return 0;
}

static int symetric_compare(const char*s1, const char*s2, size_t count, int code)
{
  int ret;
	
	ret = compare(s1, s1, count);
	if ( ret != 0 ) return code;
	
	ret = compare(s2, s2, count);
	if ( ret != 0 ) return code;

	ret = compare(s1, s2, count);
	if ( ret != 0 ) return code;

	ret = compare(s2, s1, count);
	if ( ret != 0 ) return code;

	ret = compare(s1, "", count);
	if ( ret != 0 ) return code;

	ret = compare("", s1, count);
	if ( ret != 0 ) return code;

	ret = compare(s2, "", count);
	if ( ret != 0 ) return code;

	ret = compare("", s2, count);
	if ( ret != 0 ) return code;

	return 0;
}

// ATENTION : s must count+1 in size
static void fillRandom(char* s, size_t count)
{
	size_t i;
	for ( i = 0 ; i<count ; i++ ) { // not using strcpy to not depend of another test.
		((unsigned char *)s)[i] = (unsigned char)(rndf()*255.0);
	}
	s[i] = 0;
}

static int compare_s1_s2_with_variable_count(const char* s1, size_t s1count, const char* s2, size_t s2count, int code)
{
	size_t test_count = s1count;
	if ( s2count > s1count ) test_count = s2count;
	
	for ( size_t count=0 ; count < test_count ; count++ ) {
		int ret = 0;
		ret = symetric_compare((const char*)s2, s1, count, code);
		if ( ret != 0 ) return code;
	}
	return 0;
}

static int compare_s1_s2_with_variable_count(const char* s1, const char* s2, int code)
{
	return compare_s1_s2_with_variable_count(s1, strlen_reference(s1), s2, strlen_reference(s2), code);
}

#ifdef CLOVER_BUILD
#define NB_ITERATIONS 37
#else
#define NB_ITERATIONS 189
#endif

static int compare_s1_with_variable_sizes(const char* s1, size_t s1count, int code)
{
#ifdef CLOVER_BUILD
	char s2[514];
#else
	char s2[49];
#endif
	for ( size_t i=0 ; i < NB_ITERATIONS ; i++ )
	{
		size_t s2count = (size_t)(rndf()*sizeof(s2)-1);
		if ( s2count >= sizeof(s2) ) {
			DebugLog(2, "compare_s1_with_variable_sizes, BUG : sizeof=%lu, count=%zu\n", sizeof(s2), s2count);
			continue;
		}
		fillRandom(s2, s2count);

		int ret = 0;
		ret = compare_s1_s2_with_variable_count(s1, s1count, s2, s2count, code+(int)i);

//		for ( size_t k=0 ; k < sizeof(s2) ; k++ ) {
//		}
		if ( ret != 0 ) return code;
	}
	return 0;
}

static int compare_with_variable_sizes(int code)
{
	char s1[67];
	size_t s1count;
	for ( s1count=0 ; s1count < sizeof(s1)-1 ; s1count++ ) {
		fillRandom(s1, s1count);
		int ret = compare_s1_with_variable_sizes((const char*)s1, s1count, code);
		if ( ret != 0 ) return code;
	}
	return 0;
}

/*
 * ATTENTION : strlen and str(n)cmp seems to be optimized out if called with litteral.
 * like strlen("sdfsdf") is computed at compiled time.
 * so far, using a temporary var seems to be enough to not be optmized.
 */
int strncmp_tests()
{
	const char* s;
	int ret;

	// Efficient version of strcmp may do with blocks
	// So there maybe a bug when some string has some specific length
	s = "aaaaaaaaj";
	ret = strncmp(s, "", 8);
	s = "";
	ret = strncmp(s, "", 2);

	ret = symetric_compare("#1D1D1B", "#1D1D1B", 4, 1);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("#FFFFFF", "#FFFFFF", 4, 1);
	if ( ret != 0 ) return ret;
	
	
	ret = compare_s1_s2_with_variable_count("", "z", 1);
	if ( ret != 0 ) return ret;
	ret = compare_s1_s2_with_variable_count("a", "b", 10);
	if ( ret != 0 ) return ret;
	ret = compare_s1_s2_with_variable_count("aaaaaaa", "aaaaaaai", 80);
	if ( ret != 0 ) return ret;
	ret = compare_s1_s2_with_variable_count("aaaaaaaa", "aaaaaaaaj", 90);
	if ( ret != 0 ) return ret;
	ret = compare_s1_s2_with_variable_count("aaaaaaaaa", "aaaaaaaaak", 100);
	if ( ret != 0 ) return ret;
	ret = compare_s1_s2_with_variable_count("aaaaaaaaaa", "aaaaaaaaaal", 110);
	if ( ret != 0 ) return ret;
	ret = compare_s1_s2_with_variable_count("aaaabbbbccccddddeeeeffffA", "aaaabbbbccccddddeeeeffffM", 120);
	if ( ret != 0 ) return ret;

	ret = compare_with_variable_sizes(1000000);
	if ( ret != 0 ) return ret;

	return 0;
}

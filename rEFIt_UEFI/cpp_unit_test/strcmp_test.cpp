#include <Platform.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/unicode_conversions.h"
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

//#include <wchar.h>
static int strcmp_reference(const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;

  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0')
        return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}

static int compare(const char*s1, const char*s2)
{
	int ret1 = strcmp(s1, s2);
	int ret2 = strcmp_reference(s1, s2);;

	if ( sign(ret1) != sign(ret2) ) {
		printf("Comparing '%s' and '%s' gives %d and should have given %d\n", s1, s2, ret1, ret2);
		return 1; // whatever if not 0
	}
	return 0;
}

static int symetric_compare(const char*s1, const char*s2, int code)
{
  int ret;
	
	ret = compare(s1, s1);
	if ( ret != 0 ) return code;
	
	ret = compare(s2, s2);
	if ( ret != 0 ) return code;

	ret = compare(s1, s2);
	if ( ret != 0 ) return code;

	ret = compare(s2, s1);
	if ( ret != 0 ) return code;

	ret = compare(s1, "");
	if ( ret != 0 ) return code;

	ret = compare("", s1);
	if ( ret != 0 ) return code;

	ret = compare(s2, "");
	if ( ret != 0 ) return code;

	ret = compare("", s2);
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

#define NB_ITERATIONS 515

static int compare_s1_with_variable_sizes(const char* s1, int code)
{
	char s2[515];
	for ( size_t i=0 ; i < NB_ITERATIONS ; i++ )
	{
		size_t count = (size_t)(rndf()*sizeof(s2)-1);
		if ( count >= sizeof(s2) ) {
			printf("compare_s1_with_variable_sizes, BUG : sizeof=%lu, count=%zu\n", sizeof(s2), count);
			continue;
		}
		fillRandom(s2, count);
		int ret = symetric_compare(s2, s1, code+(int)i); // s2 is always 'superior'
		if ( ret != 0 ) return ret;
	}
	return 0;
}

static int compare_with_variable_sizes(int code)
{
	unsigned char s1[514];
	for ( size_t i=0 ; i < sizeof(s1) ; i++ ) {
		size_t j=0;
		for ( j = 0 ; j < sizeof(s1) && j < i ; j++ ) { // not using strcpy to not depend of another test.
			s1[j] = (unsigned char)(rndf()*255.0);
		}
		s1[i] = 0;
		int ret = compare_s1_with_variable_sizes((const char*)s1, code+(int)i);
		if ( ret != 0 ) return ret;
	}
	return 0;
}

int strcmp_tests()
{

	int ret;

	// Efficient version of strcmp may do with blocks
	// So there maybe a bug when some string has some specific length

	
	ret = symetric_compare("", "z", 1);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("a", "b", 10);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("a", "c", 20);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aa", "ad", 30);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaa", "aae", 40);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaaa", "aaaf", 50);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaaaa", "aaaag", 60);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaaaaa", "aaaaaah", 70);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaaaaaa", "aaaaaaai", 80);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaaaaaaa", "aaaaaaaaj", 90);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaaaaaaaa", "aaaaaaaaak", 100);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaaaaaaaaa", "aaaaaaaaaal", 110);
	if ( ret != 0 ) return ret;
	ret = symetric_compare("aaaabbbbccccddddeeeeffffA", "aaaabbbbccccddddeeeeffffM", 120);
	if ( ret != 0 ) return ret;

	ret = compare_with_variable_sizes(1000000);

	return 0;
}

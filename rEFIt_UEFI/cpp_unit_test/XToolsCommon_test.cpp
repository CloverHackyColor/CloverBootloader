#include <Platform.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/unicode_conversions.h"



static int nbTest = 0;
static int nbTestFailed = 0;
//static bool displayOnlyFailed = true;


#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)


//#include <type_traits>
//#include <typeinfo>
//#include <iostream>
//#include <libgen.h>

class C
{
  public:
	typedef char char_t;
	const char* data;
	constexpr C() : data(0) { }
};

#define ASSERT_CONST_NONCONST(test, type, expectedResult) \
    test(type, expectedResult) \
    test(const type, expectedResult) \

#define ASSERT_ALL_INTEGRAL(test, expectedResult) \
    ASSERT_CONST_NONCONST(test, short, expectedResult) \
    ASSERT_CONST_NONCONST(test, short, expectedResult) \
    ASSERT_CONST_NONCONST(test, int, expectedResult) \
    ASSERT_CONST_NONCONST(test, unsigned int, expectedResult) \
    ASSERT_CONST_NONCONST(test, long, expectedResult) \
    ASSERT_CONST_NONCONST(test, unsigned long, expectedResult) \
    ASSERT_CONST_NONCONST(test, long, expectedResult) \
    ASSERT_CONST_NONCONST(test, unsigned long long, expectedResult) \

#define ASSERT_ALL_CHAR(test, expectedResult) \
    ASSERT_CONST_NONCONST(test, char, expectedResult) \
    ASSERT_CONST_NONCONST(test, signed char, expectedResult) \
    ASSERT_CONST_NONCONST(test, unsigned char, expectedResult) \
    ASSERT_CONST_NONCONST(test, char16_t, expectedResult) \
    ASSERT_CONST_NONCONST(test, char32_t, expectedResult) \
    ASSERT_CONST_NONCONST(test, wchar_t, expectedResult) \

#define ASSERT_ALL_PTR(test, type, expectedResult) \
    ASSERT_CONST_NONCONST(test, type*, expectedResult) \
    ASSERT_CONST_NONCONST(test, type[], expectedResult) \
    ASSERT_CONST_NONCONST(test, type[10], expectedResult) \

#define ASSERT_ALL_CHAR_PTR(test, expectedResult) \
    ASSERT_ALL_PTR(test, char, expectedResult) \
    ASSERT_ALL_PTR(test, signed char, expectedResult) \
    ASSERT_ALL_PTR(test, unsigned char, expectedResult) \
    ASSERT_ALL_PTR(test, char16_t, expectedResult) \
    ASSERT_ALL_PTR(test, char32_t, expectedResult) \
    ASSERT_ALL_PTR(test, wchar_t, expectedResult) \

#define ASSERT_ALL_INTEGRAL_CHAR(test, expectedResult) \
    ASSERT_ALL_INTEGRAL(test, expectedResult) \
    ASSERT_ALL_CHAR(test, expectedResult) \

template <bool>
struct _xtools__is_unsigned_true_false : public _xtools__false_type {};
template <>
struct _xtools__is_unsigned_true_false<true> : public _xtools__true_type {};
template <>
struct _xtools__is_unsigned_true_false<false> : public _xtools__false_type {};

template <class _Tp>
struct _xtools__is_unsigned : public _xtools__is_unsigned_true_false< ( _Tp(0) < _Tp(-1) ) > {};

#define is_unsigned(x) _xtools__is_unsigned<x>::value

#define ASSERT_UNSIGNED_TYPE(type, expectedResult) \
    static_assert(is_unsigned(unsigned_type(type)) == expectedResult, "unsigned_type " STRINGIFY(type) " failed");

#define ASSERT_SIZEOF_UNSIGNED_TYPE(type, expectedResult) \
    static_assert(sizeof(unsigned_type(type)) == sizeof(type), "sizeof(unsigned_type(" STRINGIFY(type) ")) == sizeof(" STRINGIFY(type) ") failed");

#define ASSERT_IS_INTEGRAL(type, expectedResult) \
    static_assert(is_integral(type) == expectedResult, "is_integral(" STRINGIFY(type) ") failed");

#define ASSERT_IS_CHAR(type, expectedResult) \
    static_assert(is_char(type) == expectedResult, "is_char(" STRINGIFY(type) ") failed");

#define ASSERT_IS_CHAR_PTR(type, expectedResult) \
    /*printf("is_char_ptr(%s)\n", STRINGIFY(type)); */ \
    static_assert(is_char_ptr(type) == expectedResult, "is_char_ptr(" STRINGIFY(type) ") failed");


int XToolsCommon_tests()
{
    (void)nbTest;
    (void)nbTestFailed;

#ifdef JIEF_DEBUG
//	printf("XToolsCommon_tests -> Enter\n");
#endif

    /* These are static. Means that it's ok if they compile */
    ASSERT_ALL_INTEGRAL_CHAR(ASSERT_UNSIGNED_TYPE, true)
    ASSERT_ALL_INTEGRAL_CHAR(ASSERT_SIZEOF_UNSIGNED_TYPE, true) // expectedResult unused by ASSERT_SIZEOF_UNSIGNED_TYPE
    ASSERT_ALL_INTEGRAL(ASSERT_IS_INTEGRAL, true)
    ASSERT_ALL_CHAR(ASSERT_IS_INTEGRAL, false)

    ASSERT_ALL_INTEGRAL(ASSERT_IS_CHAR, false)
    ASSERT_ALL_CHAR(ASSERT_IS_CHAR, true)
    ASSERT_ALL_CHAR_PTR(ASSERT_IS_CHAR_PTR, true)



    return 0; // If a test fail, it doesn't compile.
//
//#ifdef JIEF_DEBUG
//	if ( nbTestFailed == 0 ) printf("All %d tests succeeded.\n", nbTest);
//	else printf("%d tests succeeded out of %d.\n", nbTest-nbTestFailed, nbTest);
//#endif
//	return nbTestFailed > 0;
}





//const char* p1 = "foo/bar"; // basename returns bar
//const char* p1 = "foo/"; // basename returns foo
//const char* p1 = "foo//"; // basename returns foo
//const char* p1 = "foo///"; // basename returns foo
//const char* p1 = ""; // basename returns "."
//const char* p1 = "  foo/bar  "; // basename returns "bar  "
//const char* p1 = "  foo  "; // basename returns "  foo  "
//const char* p1 = " "; // basename returns " "
//const char* p2 = basename((char*)p1);


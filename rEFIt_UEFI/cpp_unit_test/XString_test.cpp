#include <Platform.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/unicode_conversions.h"



int nbTest = 0;
int nbTestFailed = 0;
bool displayOnlyFailed = true;


#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

#define CONCATENATE(e1, e2) e1 ## e2
#define PREFIX_L(s) CONCATENATE(L, s)
#define PREFIX_u(s) CONCATENATE(u, s)
#define PREFIX_U(s) CONCATENATE(U, s)



template <class _Tp>
struct make_unsigned
{
};

//template <> struct __make_unsigned<bool,               true> {};
template <> struct make_unsigned<  signed char> {typedef unsigned short     type;};
template <> struct make_unsigned<unsigned char> {typedef unsigned short     type;};
template <> struct make_unsigned<  signed short> {typedef unsigned short     type;};
template <> struct make_unsigned<unsigned short> {typedef unsigned short     type;};
template <> struct make_unsigned<  signed int> {typedef unsigned int       type;};
template <> struct make_unsigned<unsigned int> {typedef unsigned int       type;};
template <> struct make_unsigned<  signed long> {typedef unsigned long      type;};
template <> struct make_unsigned<unsigned long> {typedef unsigned long      type;};
template <> struct make_unsigned<  signed long long> {typedef unsigned long long type;};
template <> struct make_unsigned<unsigned long long> {typedef unsigned long long type;};



/*
 * Set a breakpoint here to catch failed test under debugger
 */
void breakpoint()
{
	int a;
	(void)a;
}

class SimpleString
{
	char* data;
	size_t allocatedSize;
public:
	SimpleString() : data(NULL) {}
	SimpleString(const SimpleString& simpleString) {
		allocatedSize = strlen(simpleString.data)+1;
		data = (char*)malloc(allocatedSize);
		strcpy(data, simpleString.data);
	}
	SimpleString(const char* s) {
		allocatedSize = strlen(s)+1;
		data = (char*)malloc(allocatedSize);
		strcpy(data, s);
	}
	SimpleString(const char16_t* s) {
		#if defined(__GNUC__) && !defined(__clang__)
		    data = 0; // silence warning
		#endif
		allocatedSize = utf_size_of_utf_string(data, s)+1;
		data = (char*)malloc(allocatedSize);
		utf_string_from_utf_string(data, allocatedSize, s);
	}
	SimpleString(const char32_t* s) {
		#if defined(__GNUC__) && !defined(__clang__)
		    data = 0; // silence warning
		#endif
		allocatedSize = utf_size_of_utf_string(data, s)+1;
		data = (char*)malloc(allocatedSize);
		utf_string_from_utf_string(data, allocatedSize, s);
	}
	SimpleString(const wchar_t* s) {
		#if defined(__GNUC__) && !defined(__clang__)
		    data = 0; // silence warning
		#endif
		allocatedSize = utf_size_of_utf_string(data, s)+1;
		data = (char*)malloc(allocatedSize);
		utf_string_from_utf_string(data, allocatedSize, s);
	}
	SimpleString& operator =(const SimpleString& simpleString)
	{
		size_t newSize = strlen(simpleString.data)+1;
		data = (char*)realloc(data, newSize+1, allocatedSize);
		allocatedSize = newSize+1;
		strncpy(data, simpleString.data, allocatedSize);
		return *this;
	}

	const char* c_str() const { return data; }
	
	SimpleString& svprintf(const char* format, va_list va)
	{
		VA_LIST va2;
		VA_COPY(va2, va);
		size_t size = (size_t)vsnprintf(NULL, 0, format, va);
		data = (char*)realloc(data, size+1, allocatedSize);
		allocatedSize = size+1;
		data[size] = 0;
		vsnprintf(data, allocatedSize, format, va2);
		VA_END(va);
		return *this;
	}

	~SimpleString() { delete data; }
};

SimpleString sprintf(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)));
SimpleString sprintf(const char* format, ...)
{
	SimpleString ss;
	va_list va;
	va_start(va, format);
	ss.svprintf(format, va);
	va_end(va);
	return ss;
}



// ""
// Don't put any "" in test strings or it will break indexOf and rindexOf tests.
#define utf8_1 "Āࠀ𐀀🧊Выход'UTF16'из"
#define utf8_2 "൧൨൩൪൫൬൭൮"
#define utf8_3 "éàùœ°æÂƒÌÚ®"
#define utf8_4 "ﰨﰩﰪﰫﰬﰭﰮﰯﰰﰱﰲﰳ"
#define utf8_5 "ォオカガキギクグケゲ"
#define utf8_6 "ꇆꇇꇈꇉꇊꇋꇌꇍ"
#define utf8_7 "伻似伽伾伿佀佁佂佃佄"
#define utf8_8 "楔楕楖楗楘楙楚楛楜楝楞楟楠楡"

//#define utf8_9 "abcdefghijklmnopqrstuvwxyzàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþāăąćĉċčďđēĕėęěĝğġģĥħĩīĭįĳĵķĺļľŀłńņňŋōŏőœŕŗřśŝşšţťŧũūŭůűųŵŷÿźżžɓƃƅɔƈɖɗƌǝəɛƒɠɣɩɨƙɯɲɵơƣƥƨʃƭʈưʊʋƴƶʒƹƽǆǆǉǉǌǌǎǐǒǔǖǘ"
//#define utf8_10 "ἯἸἹἺἻἼἽἾἿὈὉὊὋὌὍὙὛὝὟὨὩὪὫὬὭὮὯάέήίόύώᾈᾉᾊᾋᾌᾍᾎᾏᾘᾙᾚᾛᾜᾝᾞᾟᾨᾩᾪᾫᾬᾭᾮᾯᾸᾹᾺΆᾼιῈΈῊΉῌΐῘῙῚΊΰῨῩῪΎῬ΅`ῸΌῺΏῼ´  ΩKÅℲⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅪⅫⅬⅭⅮⅯↃⒶⒷⒸⒹⒺⒻⒼⒽⒾⒿⓀⓁⓂⓃⓄⓅⓆⓇⓈⓉⓊⓋⓌⓍⓎⓏⰀⰁⰂⰃⰄⰅⰆⰇⰈⰉⰊⰋⰌⰍⰎⰏⰐⰑⰒⰓⰔⰕⰖⰗⰘⰙⰚⰛⰜⰝⰞⰟⰠⰡⰢⰣⰤⰥⰦⰧⰨⰩⰪⰫⰬⰭⰮⱠⱢⱣⱤⱧⱩⱫⱭⱮⱯⱰⱲⱵⱾⱿⲀⲂⲄⲆⲈⲊⲌⲎⲐⲒⲔⲖⲘⲚⲜⲞⲠⲢⲤⲦⲨⲪⲬⲮⲰⲲⲴⲶⲸⲺⲼⲾⳀⳂⳄⳆⳈⳊⳌⳎⳐⳒⳔⳖⳘⳚⳜⳞⳠⳢⳫⳭⳲ〈〉Ꙁ"
//#define utf8_11 "煉璉秊練聯輦蓮連鍊列劣咽烈裂說廉念捻殮簾獵令囹寧嶺怜玲瑩羚聆鈴零靈領例禮醴隸惡了僚寮尿料樂燎療蓼遼龍暈阮劉杻柳流溜琉留硫紐類六戮陸倫崙淪輪律慄栗率隆利吏履易李梨泥理痢罹裏裡里離匿溺吝燐璘藺隣鱗麟林淋臨立笠粒狀炙識什茶刺切度拓糖宅洞暴輻行降見廓兀嗀塚晴凞猪益礼神祥福靖精羽蘒諸逸都飯飼館鶴郞隷侮僧免勉勤卑喝嘆器塀墨層屮悔慨憎懲敏既暑梅海渚漢煮爫琢碑社祉祈祐祖祝禍禎穀突節練縉繁署者臭艹艹著褐視謁謹賓贈辶逸難響頻恵舘並况全侀充冀勇勺喝啕喙嗢塚墳奄奔婢嬨廒廙彩徭惘慎愈憎慠懲戴揄搜摒敖晴朗望杖歹殺流滛滋漢瀞煮瞧爵犯猪"




struct AbstractTestString
{
	size_t size; // size in nb of char (or char16_t, char32_t or wchar_t) not bytes, not including null termiator
	size_t utf32_length; // length in nb of UTF32 char, not including null termiator
	//size_t utf32_size; // size == length for UTF32
	const char32_t* utf32;
	AbstractTestString(size_t _size, size_t _utf32_length, const char32_t* _utf32)
	   : size(_size), utf32_length(_utf32_length), utf32(_utf32) {
	}
};

template<typename CharType>
struct TestString : public AbstractTestString
{
	const CharType* cha;
	TestString(size_t _size, const CharType* _cha, size_t _utf32_length, const char32_t* _utf32) : AbstractTestString(_size, _utf32_length, _utf32), cha(_cha) {
	}
};


struct TestStringMultiCoded
{
	TestString<char> utf8;
	TestString<char16_t> utf16;
	TestString<char32_t> utf32;
	TestString<wchar_t> wchar;
	
//	template<class TestStringRet>
//	TestStringRet& getTestString4Encoding(const char* enc) {
//		if ( strcmp(enc, "utf8") == 0 ) return utf8;
//		if ( strcmp(enc, "utf16") == 0 ) return utf16;
//		if ( strcmp(enc, "utf32") == 0 ) return utf32;
//		if ( strcmp(enc, "wchar") == 0 ) return wchar;
//		panic();
//	}
};


#define nbchar(s) (sizeof(s)/sizeof(*s)-1)

#define testStringArray_LINE(utf) { \
    TestString<char>(nbchar(utf), utf, nbchar(PREFIX_U(utf)), PREFIX_U(utf)), \
    TestString<char16_t>(nbchar(PREFIX_u(utf)), PREFIX_u(utf), nbchar(PREFIX_U(utf)), PREFIX_U(utf)), \
    TestString<char32_t>(nbchar(PREFIX_U(utf)), PREFIX_U(utf), nbchar(PREFIX_U(utf)), PREFIX_U(utf)), \
    TestString<wchar_t>(nbchar(PREFIX_L(utf)), PREFIX_L(utf), nbchar(PREFIX_U(utf)), PREFIX_U(utf)), \
}

//TestString<char> foo("", "utf8", 1, "a", 1, U"a");

const TestStringMultiCoded testStringMultiCodedArray[] = {
	testStringArray_LINE(""),
	testStringArray_LINE("a"),
	testStringArray_LINE(utf8_1),
	testStringArray_LINE(utf8_2),
	testStringArray_LINE(utf8_3),
	testStringArray_LINE(utf8_4),
	testStringArray_LINE(utf8_5),
	testStringArray_LINE(utf8_6),
	testStringArray_LINE(utf8_7),
	testStringArray_LINE(utf8_8),
//	testStringArray_LINE(utf8_9),
//	testStringArray_LINE(utf8_10),
//	testStringArray_LINE(utf8_11),
};

size_t nbTestStringMultiCoded = *(&testStringMultiCodedArray + 1) - testStringMultiCodedArray;

const TestStringMultiCoded testStringMultiCoded4CaseArray[] = {
	testStringArray_LINE("ABCDEF"),
	testStringArray_LINE("abcdeFGHIjklmn"),
};

size_t nbTestStringMultiCoded4CaseArray = *(&testStringMultiCoded4CaseArray + 1) - testStringMultiCoded4CaseArray;




template<class XStringType>
struct XStringClassInfo
{
	typedef XStringType xs_t;
	static const char* xStringClassName;
};

template<>
struct XStringClassInfo<XString>
{
	typedef char ch_t;
	typedef XString xs_t;
	static constexpr const char* prefix = "";
	static constexpr const char* xStringClassName = "XString";
};

template<>
struct XStringClassInfo<XString16>
{
	typedef char16_t ch_t;
	typedef XString16 xs_t;
	static constexpr const char* prefix = "u";
	static constexpr const char* xStringClassName = "XString16";
};

template<>
struct XStringClassInfo<XString32>
{
	typedef char32_t ch_t;
	typedef XString32 xs_t;
	static constexpr const char* prefix = "U";
	static constexpr const char* xStringClassName = "XString32";
};

template<>
struct XStringClassInfo<XStringW>
{
	typedef wchar_t ch_t;
	typedef XStringW xs_t;
	static constexpr const char* prefix = "L";
	static constexpr const char* xStringClassName = "XStringW";
};

template<>
struct XStringClassInfo<char>
{
	typedef char ch_t;
	typedef XString xs_t;
	static constexpr const char* prefix = "";
	static constexpr const char* xStringClassName = "XString";
};

template<>
struct XStringClassInfo<char16_t>
{
	typedef char16_t ch_t;
	typedef XString16 xs_t;
	static constexpr const char* prefix = "u";
	static constexpr const char* xStringClassName = "XString16";
};

template<>
struct XStringClassInfo<char32_t>
{
	typedef char32_t ch_t;
	typedef XString32 xs_t;
	static constexpr const char* prefix = "U";
	static constexpr const char* xStringClassName = "XString32";
};

template<>
struct XStringClassInfo<wchar_t>
{
	typedef wchar_t ch_t;
	typedef XStringW xs_t;
	static constexpr const char* prefix = "L";
	static constexpr const char* xStringClassName = "XStringW";
};

template<>
struct XStringClassInfo<TestString<char>>
{
	typedef char ch_t;
	typedef XString xs_t;
	static constexpr const char* prefix = "";
	static constexpr const char* xStringClassName = "XString";
};

template<>
struct XStringClassInfo<TestString<char16_t>>
{
	typedef char16_t ch_t;
	typedef XString16 xs_t;
	static constexpr const char* prefix = "u";
	static constexpr const char* xStringClassName = "XString16";
};

template<>
struct XStringClassInfo<TestString<char32_t>>
{
	typedef char32_t ch_t;
	typedef XString32 xs_t;
	static constexpr const char* prefix = "U";
	static constexpr const char* xStringClassName = "XString32";
};

template<>
struct XStringClassInfo<TestString<wchar_t>>
{
	typedef wchar_t ch_t;
	typedef XStringW xs_t;
	static constexpr const char* prefix = "L";
	static constexpr const char* xStringClassName = "XStringW";
};






SimpleString title_tmp;
bool title_tmp_needs_reprinted;
bool displayOnlyIfFailed_tmp;

#define TEST_TITLE(__displayOnlyIfFailed__, title) \
  do { \
    displayOnlyIfFailed_tmp = __displayOnlyIfFailed__; \
    title_tmp = title; \
	title_tmp_needs_reprinted = true; \
	if ( !displayOnlyIfFailed_tmp ) { \
        printf("%s", title_tmp.c_str()); \
        title_tmp_needs_reprinted = false; \
	} \
  }while(0);


#define CHECK_RESULT(condition, okmessage, failedmessage) \
  do { \
	if ( !(condition) ) { \
        if ( title_tmp_needs_reprinted ) { \
            printf("%s", title_tmp.c_str()); \
		} \
		printf(" : -> "); \
		printf("%s", failedmessage.c_str()); \
        printf("\n"); \
        title_tmp_needs_reprinted = true; \
        nbTestFailed += 1; \
	}else if ( !displayOnlyIfFailed_tmp ) { \
        if ( title_tmp_needs_reprinted ) { \
            printf("%s", title_tmp.c_str()); \
		} \
        title_tmp_needs_reprinted = true; \
		printf(" -> OK : %s\n", okmessage.c_str()); \
	} \
	nbTest += 1; \
	if ( !(condition) ) breakpoint(); \
  }while(0);





#define __TEST0(test, XStringClass, classEncoding) \
	test(XStringClass, classEncoding); \

#define __TEST1(test, XStringClass, classEncoding, encoding1) \
		test(XStringClass, classEncoding, encoding1); \

#define __TEST2(test, XStringClass, classEncoding, encoding1, encoding2) \
	test(XStringClass, classEncoding, encoding1, encoding2); \


#define __TEST_ALL_UTF2(test, XStringClass, classEncoding, encoding1) \
	__TEST2(test, XStringClass, classEncoding, encoding1, utf8); \
	__TEST2(test, XStringClass, classEncoding, encoding1, utf16); \
	__TEST2(test, XStringClass, classEncoding, encoding1, utf32); \
	__TEST2(test, XStringClass, classEncoding, encoding1, wchar); \


#define TEST_ALL_INTEGRAL(test, XStringClass, classEncoding) \
	__TEST1(test, XStringClass, classEncoding, signed char); \
	__TEST1(test, XStringClass, classEncoding, unsigned char); \
	__TEST1(test, XStringClass, classEncoding, signed short); \
	__TEST1(test, XStringClass, classEncoding, unsigned short); \


#define TEST_ALL_UTF(test, XStringClass, classEncoding) \
	__TEST1(test, XStringClass, classEncoding, utf8); \
	__TEST1(test, XStringClass, classEncoding, utf16); \
	__TEST1(test, XStringClass, classEncoding, utf32); \
	__TEST1(test, XStringClass, classEncoding, wchar); \

#define TEST_ALL_UTF_ALL_UTF(test, XStringClass, classEncoding) \
	__TEST_ALL_UTF2(test, XStringClass, classEncoding, utf8); \
	__TEST_ALL_UTF2(test, XStringClass, classEncoding, utf16); \
	__TEST_ALL_UTF2(test, XStringClass, classEncoding, utf32); \
	__TEST_ALL_UTF2(test, XStringClass, classEncoding, wchar); \

#define TEST_ALL_CLASSES(test, macro) \
	macro(test, XString, utf8); \
	macro(test, XString16, utf16); \
	macro(test, XString32, utf32); \
	macro(test, XStringW, wchar); \



/*****************************  Default ctor  *****************************/
template<class XStringClass>
SimpleString testDefaultCtor_()
{
	XStringClass xstr;
	TEST_TITLE(displayOnlyFailed, sprintf("Test default ctor of %s", XStringClassInfo<XStringClass>::xStringClassName));

	CHECK_RESULT(xstr.length() == 0,
	    sprintf("xstr.length() == 0"),
	    sprintf("xstr.length() != 0")
	);
	CHECK_RESULT(xstr.sizeInBytes() == 0,
	    sprintf("xstr.sizeInBytes() == 0"),
	    sprintf("xstr.sizeInBytes() != 0")
	);
	CHECK_RESULT(*xstr.s() == 0,
	    sprintf("*xstr.s() == 0"),
	    sprintf("*xstr.s() != 0")
	);
	return SimpleString();
}

#define testDefaultCtor(XStringClass, classEncoding) \
    printf("Test %s::testDefaultCtor\n", STRINGIFY(XStringClass)); \
    testDefaultCtor_<XStringClass>();


/*****************************  takeValueFrom(char type)  *****************************/
template<class XStringClass, class TestStringSrc, class TestStringExpectedResult>
SimpleString testTakeValueFrom_(const TestStringSrc& src, const TestStringExpectedResult& expectedResult)
{

	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::testTakeValueFrom(%s\"%s\")", XStringClassInfo<XStringClass>::xStringClassName, XStringClassInfo<TestStringSrc>::prefix, SimpleString(src.cha).c_str()));

	XStringClass xstr;
	xstr.takeValueFrom(src.cha);
	
	size_t expectedSize = expectedResult.size*sizeof(expectedResult.cha[0]);
	CHECK_RESULT(xstr.sizeInBytes() == expectedSize,
	    sprintf("xstr.sizeInBytes() == expectedSize (%zu)", expectedSize),
	    sprintf("xstr.sizeInBytes() != expectedSize (%zu!=%zu)", xstr.sizeInBytes(), expectedSize)
	);
	CHECK_RESULT(memcmp(xstr.s(), expectedResult.cha, expectedSize) == 0,
	    sprintf("memcmp(xstr.s(), expectedResult.cha, expectedSize) == 0"),
	    sprintf("memcmp(xstr.s(), expectedResult.cha, expectedSize) != 0")
	);
	
// TODO test ctor with litteral
//	XStringClass xstr2;
//	xstr2 = src.cha;

	return SimpleString();
}

#define testTakeValueFrom(XStringClass, classEncoding, encoding1) \
    printf("Test %s::testTakeValueFrom(%s)\n", STRINGIFY(XStringClass), STRINGIFY(encoding1)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
    	testTakeValueFrom_<XStringClass>(testStringMultiCodedArray[i].encoding1, testStringMultiCodedArray[i].classEncoding); \
	} \


/*****************************  takeValueFrom(XString), ctor(XString), operator =(XString)  *****************************/
template<class XStringClass, class TestStringSrc, class TestStringExpectedResult>
SimpleString testTakeValueFromXString_(const TestStringSrc& src, const TestStringExpectedResult& expectedResult)
{
// TODO test ctor with litteral

	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::testTakeValueFrom(%s\"%s\")", XStringClassInfo<XStringClass>::xStringClassName, XStringClassInfo<TestStringSrc>::prefix, SimpleString(src.cha).c_str()));

	typename XStringClassInfo<TestStringSrc>::xs_t srcXString;
	srcXString.takeValueFrom(src.cha);
	
	XStringClass xstr;
	xstr.takeValueFrom(srcXString);
	
	size_t expectedSize = expectedResult.size*sizeof(expectedResult.cha[0]);
	CHECK_RESULT(xstr.sizeInBytes() == expectedSize,
	    sprintf("xstr.sizeInBytes() == expectedSize (%zu)", expectedSize),
	    sprintf("xstr.sizeInBytes() != expectedSize (%zu!=%zu)", xstr.sizeInBytes(), expectedSize)
	);
	CHECK_RESULT(memcmp(xstr.s(), expectedResult.cha, expectedSize+sizeof(expectedResult.cha[0])) == 0,
	    sprintf("memcmp(xstr.s(), expectedResult.cha, expectedResult.size) == 0"),
	    sprintf("memcmp(xstr.s(), expectedResult.cha, expectedResult.size) != 0")
	);
	{
		XStringClass xstr2(srcXString);
		// We don't use operator == to check xstr == xstr2 because operator == is not tested yet.
		CHECK_RESULT(xstr2.sizeInBytes() == xstr.sizeInBytes(),
					 sprintf("xstr2.sizeInBytes() == xstr.sizeInBytes() (%zu)", xstr.sizeInBytes()),
					 sprintf("xstr2.sizeInBytes() != xstr.sizeInBytes() (%zu!=%zu)", xstr2.sizeInBytes(), xstr.sizeInBytes())
					 );
		CHECK_RESULT(memcmp(xstr2.s(), xstr.s(), xstr.sizeInBytes()+sizeof(xstr.s()[0])) == 0,
					 sprintf("memcmp(xstr2.s(), xstr.s(), xstr.sizeInBytes()) == 0"),
					 sprintf("memcmp(xstr2.s(), xstr.s(), xstr.sizeInBytes()) != 0")
					 );
	}
	{
		XStringClass xstr2;
		xstr2 = srcXString;
		// We don't use operator == to check xstr == xstr2 because operator == is not tested yet.
		CHECK_RESULT(xstr2.sizeInBytes() == xstr.sizeInBytes(),
					 sprintf("xstr2.sizeInBytes() == xstr.sizeInBytes() (%zu)", xstr.sizeInBytes()),
					 sprintf("xstr2.sizeInBytes() != xstr.sizeInBytes() (%zu!=%zu)", xstr2.sizeInBytes(), xstr.sizeInBytes())
					 );
		CHECK_RESULT(memcmp(xstr2.s(), xstr.s(), xstr.sizeInBytes()+sizeof(xstr.s()[0])) == 0,
					 sprintf("memcmp(xstr2.s(), xstr.s(), xstr.sizeInBytes()) == 0"),
					 sprintf("memcmp(xstr2.s(), xstr.s(), xstr.sizeInBytes()) != 0")
					 );
	}
	return SimpleString();
}

#define testTakeValueFromXString(XStringClass, classEncoding, encoding1) \
    printf("Test %s::testTakeValueFromXString(%s)\n", STRINGIFY(XStringClass), STRINGIFY(encoding1)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
    	testTakeValueFromXString_<XStringClass>(testStringMultiCodedArray[i].encoding1, testStringMultiCodedArray[i].classEncoding); \
	} \



/*****************************  Default isEmpty, SetEmpty  *****************************/
template<class XStringClass>
SimpleString testEmpty_()
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test isEmpty(),notEmpty(),setEmpty() of %s", XStringClassInfo<XStringClass>::xStringClassName));

	XStringClass str;
	str.takeValueFrom("aa");
	
	CHECK_RESULT(str.isEmpty() == false,
	    sprintf("str.isEmpty() == false"),
	    sprintf("str.isEmpty() != true")
	);
	CHECK_RESULT(str.notEmpty() == true,
	    sprintf("str.notEmpty() == true"),
	    sprintf("str.notEmpty() != false")
	);
	
	str.setEmpty();
	
	CHECK_RESULT(str.isEmpty() == true,
	    sprintf("str.isEmpty() == true"),
	    sprintf("str.isEmpty() != false")
	);
	CHECK_RESULT(str.notEmpty() == false,
	    sprintf("str.notEmpty() == false"),
	    sprintf("str.notEmpty() != true")
	);

	return SimpleString();
}

#define testEmpty(XStringClass, classEncoding) \
    printf("Test %s::testEmpty\n", STRINGIFY(XStringClass)); \
	testEmpty_<XStringClass>(); \


/*****************************  char32At_u  *****************************/
template<class XStringClass, typename integralType, class InitialValue>
SimpleString testchar32At_u_(const InitialValue& initialValue)
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::char32At_u()", XStringClassInfo<XStringClass>::xStringClassName));

	XStringClass xstr;
	xstr.takeValueFrom(initialValue.cha);
	for ( integralType i=0 ; (typename make_unsigned<integralType>::type)i < xstr.length() ; i++ )
	{
		CHECK_RESULT(xstr[i] == initialValue.utf32[i],
					 sprintf("xstr[i] == dst.cha[i] (%d)", initialValue.utf32[i]),
					 sprintf("xstr[i] != dst.cha[i] (%d!=%d)", xstr[i], initialValue.utf32[i])
					 );
    }
	return SimpleString();
}

#define testchar32At_u(XStringClass, classEncoding, integralType) \
    printf("Test %s::testchar32At_u\n", STRINGIFY(XStringClass)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
        testchar32At_u_<XStringClass, integralType>(testStringMultiCodedArray[i].classEncoding); \
	}




/*****************************  strcpy  *****************************/
template<class XStringClass, class TestStringSameAsClass, class TestStringSrc>
SimpleString teststrcpy_(const TestStringSameAsClass& encodedSameAsClass, const TestStringSrc& src)
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::strcpy(%s\"%s\")", XStringClassInfo<XStringClass>::xStringClassName, XStringClassInfo<TestStringSrc>::prefix, SimpleString(src.cha).c_str()));

	XStringClass xstr;
	xstr.takeValueFrom("foobar");
	xstr.strcpy(src.cha);
	
	size_t expectedSize = encodedSameAsClass.size*sizeof(encodedSameAsClass.cha[0]);
	CHECK_RESULT(xstr.sizeInBytes() == expectedSize,
	    sprintf("xstr.sizeInBytes() == dst.size (%zu)", expectedSize),
	    sprintf("xstr.sizeInBytes() != dst.size (%zu!=%zu)", xstr.sizeInBytes(), expectedSize)
	);
	CHECK_RESULT(memcmp(xstr.s(), encodedSameAsClass.cha, expectedSize+sizeof(encodedSameAsClass.cha[0])) == 0,
	    sprintf("memcmp(xstr.s(), dst.cha, dst.size) == 0"),
	    sprintf("memcmp(xstr.s(), dst.cha, dst.size) != 0")
	);
	return SimpleString();
}

#define teststrcpy(XStringClass, classEncoding, encoding1) \
    printf("Test %s::teststrcpy(%s)\n", STRINGIFY(XStringClass), STRINGIFY(encoding1)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
    	teststrcpy_<XStringClass>(testStringMultiCodedArray[i].classEncoding, testStringMultiCodedArray[i].encoding1); \
	} \



/*****************************  strncpy  *****************************/
template<class XStringClass, class TestStringSameAsClass, class TestStringSrc>
SimpleString teststrncpy_(const TestStringSameAsClass& encodedSameAsClass, const TestStringSrc& src)
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::strncpy(%s\"%s\")", XStringClassInfo<XStringClass>::xStringClassName, XStringClassInfo<TestStringSrc>::prefix, SimpleString(src.cha).c_str()));

	for ( size_t i = 0 ; i < length_of_utf_string(src.cha)+5 ; i++ )
	{
		XStringClass xstr;
		xstr.takeValueFrom("foobar");
		xstr.strncpy(src.cha, i);
		
		CHECK_RESULT((length_of_utf_string(encodedSameAsClass.cha) >= i && xstr.length() == i) || (xstr.length() == length_of_utf_string(encodedSameAsClass.cha)),
					 sprintf("xstr.sizeInBytes() == dst.size (%zu)", xstr.sizeInBytes()),
					 sprintf("xstr.sizeInBytes() != dst.size (%zu!=%zu)", xstr.sizeInBytes(), encodedSameAsClass.size)
					 );
		CHECK_RESULT(memcmp(xstr.s(), encodedSameAsClass.cha, xstr.sizeInBytes()) == 0,
					 sprintf("memcmp(xstr.s(), dst.cha, dst.size) == 0"),
					 sprintf("memcmp(xstr.s(), dst.cha, dst.size) != 0")
					 );
	}
	return SimpleString();
}

#define teststrncpy(XStringClass, classEncoding, encoding1) \
    printf("Test %s::teststrncpy(%s)\n", STRINGIFY(XStringClass), STRINGIFY(encoding1)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
        teststrncpy_<XStringClass>(testStringMultiCodedArray[i].classEncoding, testStringMultiCodedArray[i].encoding1); \
	} \


/*****************************  strcat  *****************************/
template<class XStringClass, typename ch_t>
static void teststrcatCheckResult(size_t expectedLength, size_t expectedSize, ch_t* expectedString, XStringClass xstr)
{
	CHECK_RESULT(xstr.length() == expectedLength,
				 sprintf("xstr.length() == expectedLength (%zu)", expectedLength),
				 sprintf("xstr.length() != expectedLength (%zu!=%zu)", xstr.length(), expectedLength)
				 );
	//expectedLength = length_of_utf_string(initialValue.cha) + length_of_utf_string(valueToCat.cha);
	//xstr.takeValueFrom(initialValue.cha);
	//xstr.strcat(valueToCat.cha);
	
	CHECK_RESULT(xstr.sizeInBytes() == expectedSize,
				 sprintf("xstr.sizeInBytes() == expectedSize (%zu)", expectedSize),
				 sprintf("xstr.sizeInBytes() != expectedSize (%zu!=%zu)", xstr.sizeInBytes(), expectedSize)
				 );
	
	CHECK_RESULT(memcmp(xstr.s(), expectedString, expectedSize+sizeof(ch_t)) == 0,
				 sprintf("memcmp(xstr.s(), dst.cha, dst.size) == 0"),
				 sprintf("memcmp(xstr.s(), dst.cha, dst.size) != 0")
				 );
}

template<class XStringClass, class InitialValue, class ValueToCat>
SimpleString teststrcat_(const InitialValue& initialValue, const ValueToCat& valueToCat)
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::strcpy(%s\"%s\") strcat(%s\"%s\")", XStringClassInfo<XStringClass>::xStringClassName, XStringClassInfo<InitialValue>::prefix, SimpleString(initialValue.cha).c_str(), XStringClassInfo<ValueToCat>::prefix, SimpleString(valueToCat.cha).c_str()));

	typedef typename XStringClassInfo<XStringClass>::ch_t ch_t;
	ch_t c; // dummy for call utf function

	size_t expectedLength = length_of_utf_string(initialValue.cha) + length_of_utf_string(valueToCat.cha);
	size_t expectedSize = (utf_size_of_utf_string(&c, initialValue.cha) + utf_size_of_utf_string(&c, valueToCat.cha))*sizeof(ch_t);
	ch_t* expectedString = (ch_t*)malloc(expectedSize + sizeof(ch_t));
	utf_string_from_utf_string(expectedString, expectedSize*sizeof(ch_t) + 1, initialValue.cha);
	utf_string_from_utf_string(expectedString + size_of_utf_string(expectedString), expectedSize*sizeof(ch_t) + 1 - size_of_utf_string(expectedString), valueToCat.cha);

	// strcat native type
	{
		XStringClass xstr;
		xstr.takeValueFrom(initialValue.cha);
		xstr.strcat(valueToCat.cha);
		teststrcatCheckResult(expectedLength, expectedSize, expectedString, xstr);
	}
	// += native type
	{
		XStringClass xstr;
		xstr.takeValueFrom(initialValue.cha);
		xstr += valueToCat.cha;
		teststrcatCheckResult(expectedLength, expectedSize, expectedString, xstr);
	}
	// strcat XString
	{
		typename XStringClassInfo<ValueToCat>::xs_t valueToCatXString;
		valueToCatXString.takeValueFrom(valueToCat.cha);
		XStringClass xstr;
		xstr.takeValueFrom(initialValue.cha);
		xstr += valueToCatXString;
		teststrcatCheckResult(expectedLength, expectedSize, expectedString, xstr);
	}
	// XString + native type
	{
		XStringClass xinitialValue;
		xinitialValue.takeValueFrom(initialValue.cha);
		XStringClass xstr;
		xstr = xinitialValue + valueToCat.cha;
		teststrcatCheckResult(expectedLength, expectedSize, expectedString, xstr);
	}
	// XString + XString
	{
		XStringClass xinitialValue;
		xinitialValue.takeValueFrom(initialValue.cha);
		typename XStringClassInfo<ValueToCat>::xs_t valueToCatXString;
		valueToCatXString.takeValueFrom(valueToCat.cha);
		XStringClass xstr;
		xstr = xinitialValue + valueToCatXString;
		teststrcatCheckResult(expectedLength, expectedSize, expectedString, xstr);
	}
	// native type + XString
	{
		typename XStringClassInfo<ValueToCat>::xs_t valueToCatXString;
		valueToCatXString.takeValueFrom(valueToCat.cha);
		XStringClass xstr;
		xstr = initialValue.cha + valueToCatXString;
		teststrcatCheckResult(expectedLength, expectedSize, expectedString, xstr);
	}

	free(expectedString);
	return SimpleString();
}

#define teststrcat(XStringClass, classEncoding, encoding1, encoding2) \
    printf("Test %s(%s)::teststrcat(%s)\n", STRINGIFY(XStringClass), STRINGIFY(encoding1), STRINGIFY(encoding2)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
    	for ( size_t j = 0 ; j < nbTestStringMultiCoded ; j++ ) { \
	        teststrcat_<XStringClass>(testStringMultiCodedArray[i].encoding1, testStringMultiCodedArray[i].encoding2); \
		} \
	} \

#define min(x,y) ( (x) < (y) ? (x) : (y) )
#define max(x,y) ( (x) > (y) ? (x) : (y) )

/*****************************  strncat  *****************************/
template<class XStringClass, class InitialValue, class ValueToCat>
SimpleString teststrncat_(const InitialValue& initialValue, const ValueToCat& valueToCat)
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::strcpy(%s\"%s\") strncat(%s\"%s\")", XStringClassInfo<XStringClass>::xStringClassName, XStringClassInfo<InitialValue>::prefix, SimpleString(initialValue.cha).c_str(), XStringClassInfo<ValueToCat>::prefix, SimpleString(valueToCat.cha).c_str()));

	for ( size_t i = 0 ; i < valueToCat.utf32_length+5 ; i++ )
	{

	    typedef typename XStringClassInfo<XStringClass>::ch_t ch_t;
		ch_t c; // dummy for call utf function

		XStringClass xstr;
		xstr.takeValueFrom(initialValue.cha);
		xstr.strncat(valueToCat.cha, i);
	
		size_t expectedLength = length_of_utf_string(initialValue.cha) + min(i, valueToCat.utf32_length);
		CHECK_RESULT(xstr.length() == expectedLength,
					 sprintf("xstr.length() == expectedLength (%zu)", expectedLength),
					 sprintf("xstr.length() != expectedLength (%zu!=%zu)", xstr.length(), expectedLength)
					 );
//expectedLength = length_of_utf_string(initialValue.cha) + min(i, valueToCat.utf32_length);
//xstr.takeValueFrom(initialValue.cha);
//xstr.strncat(valueToCat.cha, i);

		size_t expectedSize = (utf_size_of_utf_string(&c, initialValue.cha) + utf_size_of_utf_string_len(&c, valueToCat.cha, i))*sizeof(ch_t);
		CHECK_RESULT(xstr.sizeInBytes() == expectedSize,
					 sprintf("xstr.sizeInBytes() == expectedSize (%zu)", expectedSize),
					 sprintf("xstr.sizeInBytes() != expectedSize (%zu!=%zu)", xstr.sizeInBytes(), expectedSize)
					 );
	
		ch_t* expectedString = (ch_t*)malloc(expectedSize*sizeof(ch_t)*sizeof(ch_t) + 1);
		utf_string_from_utf_string(expectedString, expectedSize*sizeof(ch_t) + 1, initialValue.cha);
		utf_string_from_utf_string_len(expectedString + utf_size_of_utf_string(&c, initialValue.cha), expectedSize*sizeof(ch_t) + 1 - size_of_utf_string(expectedString), valueToCat.cha, i);
		CHECK_RESULT(memcmp(xstr.s(), expectedString, expectedSize+sizeof(ch_t)) == 0,
					 sprintf("memcmp(xstr.s(), expectedString, dst.size) == 0"),
					 sprintf("memcmp(xstr.s(), expectedString, dst.size) != 0")
					 );
//utf_string_from_utf_string(expectedString, expectedSize*sizeof(XStringCharClass) + 1, initialValue.cha);
//utf_string_from_utf_string_len(expectedString + utf_size_of_utf_string(&c, initialValue.cha), expectedSize*sizeof(XStringCharClass) + 1 - size_of_utf_string(expectedString), valueToCat.cha, i);
//expectedLength = length_of_utf_string(initialValue.cha) + min(i, valueToCat.utf32_length);
//xstr.takeValueFrom(initialValue.cha);
//xstr.strncat(valueToCat.cha, i);

	    free(expectedString);
	}
	return SimpleString();
}

#define teststrncat(XStringClass, classEncoding, encoding1, encoding2) \
    printf("Test %s(%s)::teststrncat(%s)\n", STRINGIFY(XStringClass), STRINGIFY(encoding1), STRINGIFY(encoding2)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
    	for ( size_t j = 0 ; j < nbTestStringMultiCoded ; j++ ) { \
	        teststrncat_<XStringClass>(testStringMultiCodedArray[i].encoding1, testStringMultiCodedArray[i].encoding2); \
		} \
	} \


/*****************************  subString  *****************************/
template<class XStringClass, class InitialValue>
SimpleString testSubString_(const InitialValue& initialValue)
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::subString(%s\"%s\"", XStringClassInfo<XStringClass>::xStringClassName, XStringClassInfo<InitialValue>::prefix, SimpleString(initialValue.cha).c_str()));

	typedef typename XStringClassInfo<XStringClass>::ch_t ch_t;
	ch_t c; // dummy for call utf function

	XStringClass str;
	str.takeValueFrom(initialValue.cha);
	
	for ( size_t pos = 0 ; pos < initialValue.utf32_length+3 ; pos+=1 ) {
		for ( size_t count = 0 ; count < initialValue.utf32_length-pos+3 ; count+=1 )
		{
			size_t expectedLength = 0;
			if ( pos < initialValue.utf32_length ) expectedLength = min( count, initialValue.utf32_length - pos);
			
			size_t expectedSize = 0;
			if ( pos < initialValue.utf32_length ) expectedSize = utf_size_of_utf_string_len(&c, initialValue.cha, pos+count) - utf_size_of_utf_string_len(&c, initialValue.cha, pos);

			size_t offset;
			if ( pos < initialValue.utf32_length ) offset = utf_size_of_utf_string_len(&c, initialValue.cha, pos);
			else offset = utf_size_of_utf_string(&c, initialValue.cha);
			
			XStringClass subStr = str.SubString(pos, count);
			
			CHECK_RESULT(subStr.length() == expectedLength,
						 sprintf("subStr.length() == expectedLength (%zu)", expectedLength),
						 sprintf("subStr.length() != expectedLength (%zu!=%zu)", subStr.length(), expectedLength)
						 );
			CHECK_RESULT(memcmp(subStr.s(), initialValue.cha + offset, expectedSize) == 0,
						 sprintf("memcmp == 0"),
						 sprintf("memcmp != 0)")
						 );
		}
	}

	return SimpleString();
}

#define testSubString(XStringClass, classEncoding) \
    printf("Test %s::testSubString\n", STRINGIFY(XStringClass)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
		testSubString_<XStringClass>(testStringMultiCodedArray[i].classEncoding); \
	} \


/*****************************  Compare  *****************************/
template<typename CharType>
CharType* incrementChar(const CharType* s, size_t pos, int increment)
{
	size_t initialSize = size_of_utf_string(s);
	CharType* buf = (CharType*)malloc( (initialSize+1)*sizeof(CharType) );
	size_t dst_max_len = initialSize+1;

	char32_t char32;
	size_t n = 0;
	CharType* d = buf;
	s = get_char32_from_string(s, &char32);
	while ( char32 ) {
		if ( n == pos )	{
			if ( increment >= 0 ) {
				d = store_utf_from_char32(d, &dst_max_len, char32+(unsigned int)increment);
			}else{
				d = store_utf_from_char32(d, &dst_max_len, char32-(unsigned int)(-increment)); // avoid signedness warning
			}
		}else{
			d = store_utf_from_char32(d, &dst_max_len, char32);
		}
		s = get_char32_from_string(s, &char32);
		n++;
	}
	return buf;
}

template<class XStringClass, class InitialValue>
SimpleString testCompare_(const InitialValue& initialValue)
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::strcmp(%s\"%s\")", XStringClassInfo<XStringClass>::xStringClassName, XStringClassInfo<InitialValue>::prefix, SimpleString(initialValue.cha).c_str()));

//	typedef typename XStringClassInfo<XStringClass>::ch_t xs_ch_t;
//	ch_t c; // dummy for call utf function

	XStringClass xstr;
	xstr.takeValueFrom(initialValue.cha);
	
	typename XStringClassInfo<InitialValue>::xs_t xstr2;
	xstr2.takeValueFrom(initialValue.cha);
	
	CHECK_RESULT(xstr.strcmp(xstr2.s()) == 0,
				 sprintf("subStr.length() == 0"),
				 sprintf("subStr.length() != 0 (%d)", xstr.strcmp(xstr2.s()))
				 );
	
	CHECK_RESULT(xstr == xstr2.s(),
				 sprintf("subStr.length() == 0"),
				 sprintf("subStr.length() != 0 (%d)", xstr.strcmp(xstr2.s()))
				 );
	CHECK_RESULT(xstr == xstr2,
				 sprintf("subStr.length() == 0"),
				 sprintf("subStr.length() != 0 (%d)", xstr.strcmp(xstr2.s()))
				 );
	CHECK_RESULT(!(xstr != xstr2.s()),
				 sprintf("subStr.length() == 0"),
				 sprintf("subStr.length() != 0 (%d)", xstr.strcmp(xstr2.s()))
				 );
	
	CHECK_RESULT(!(xstr != xstr2),
				 sprintf("subStr.length() == 0"),
				 sprintf("subStr.length() != 0 (%d)", xstr.strcmp(xstr2.s()))
				 );
	CHECK_RESULT(!(xstr != xstr2.s()),
				 sprintf("subStr.length() == 0"),
				 sprintf("subStr.length() != 0 (%d)", xstr.strcmp(xstr2.s()))
				 );
	CHECK_RESULT(!(xstr != xstr2),
				 sprintf("subStr.length() == 0"),
				 sprintf("subStr.length() != 0 (%d)", xstr.strcmp(xstr2.s()))
				 );
//res = xstr.strcmp(xstr2.s());

	typedef typename XStringClassInfo<InitialValue>::ch_t ch_t;

	for ( size_t pos = 0 ; pos < initialValue.utf32_length ; pos++) // this avoid empty string
	{
		const ch_t* s = incrementChar(initialValue.cha, pos, 1);
		CHECK_RESULT(xstr.strcmp(s) == -1,
					 sprintf("xstr.strcmp(s) == -1"),
					 sprintf("xstr.strcmp(s) != -1 (%d)", xstr.strcmp(s))
					 );
		/* operator comparison with native type */
		CHECK_RESULT(!(xstr == s),
					 sprintf("!(xstr == s)"),
					 sprintf("!!(xstr == s)")
					 );
		CHECK_RESULT(!(s == xstr),
					 sprintf("!(s == xstr)"),
					 sprintf("!!(s == xstr)")
					 );
		CHECK_RESULT(xstr != s,
					 sprintf("xstr != s"),
					 sprintf("!xstr != s")
					 );
		CHECK_RESULT(s != xstr,
					 sprintf("s != xstr"),
					 sprintf("!s != xstr")
					 );
		CHECK_RESULT(xstr < s,
					 sprintf("xstr < s"),
					 sprintf("!xstr < s")
					 );
		CHECK_RESULT(s > xstr,
					 sprintf("s > xstr"),
					 sprintf("!s > xstr")
					 );
		CHECK_RESULT(xstr <= s,
					 sprintf("xstr <= s"),
					 sprintf("!xstr <= s")
					 );
		CHECK_RESULT(s >= xstr,
					 sprintf("s >= xstr"),
					 sprintf("!s >= xstr")
					 );
		CHECK_RESULT(!(xstr > s),
					 sprintf("!(xstr > s)"),
					 sprintf("!!(xstr < s)")
					 );
		CHECK_RESULT(!(s < xstr),
					 sprintf("!(s < xstr)"),
					 sprintf("!!(s < xstr)")
					 );
		CHECK_RESULT(!(xstr >= s),
					 sprintf("!(xstr >= s)"),
					 sprintf("!!(xstr >= s)")
					 );
		CHECK_RESULT(!(s <= xstr),
					 sprintf("!(s <= xstr)"),
					 sprintf("!!(s <= xstr)")
					 );
		
		/* operator comparison with other XString */
		xstr2.takeValueFrom(s);
		CHECK_RESULT(!(xstr == xstr2),
					 sprintf("!(xstr == xstr2)"),
					 sprintf("!!(xstr == xstr2)")
					 );
		CHECK_RESULT(!(xstr2 == xstr),
					 sprintf("!(xstr2 == xstr)"),
					 sprintf("!!(xstr2 == xstr)")
					 );
		CHECK_RESULT(xstr != xstr2,
					 sprintf("xstr != xstr2"),
					 sprintf("!xstr != xstr2")
					 );
		CHECK_RESULT(xstr2 != xstr,
					 sprintf("xstr2 != xstr"),
					 sprintf("!xstr2 != xstr")
					 );
		CHECK_RESULT(xstr < xstr2,
					 sprintf("xstr < xstr2"),
					 sprintf("!xstr < xstr2")
					 );
		CHECK_RESULT(xstr2 > xstr,
					 sprintf("xstr2 > xstr"),
					 sprintf("!xstr2 > xstr")
					 );
		CHECK_RESULT(xstr <= xstr2,
					 sprintf("xstr <= xstr2"),
					 sprintf("!xstr <= xstr2")
					 );
		CHECK_RESULT(xstr2 >= xstr,
					 sprintf("xstr2 >= xstr"),
					 sprintf("!xstr2 >= xstr")
					 );
		CHECK_RESULT(!(xstr > xstr2),
					 sprintf("!(xstr > xstr2)"),
					 sprintf("!!(xstr < xstr2)")
					 );
		CHECK_RESULT(!(xstr2 < xstr),
					 sprintf("!(xstr2 < xstr)"),
					 sprintf("!!(xstr2 < xstr)")
					 );
		CHECK_RESULT(!(xstr >= xstr2),
					 sprintf("!(xstr >= xstr2)"),
					 sprintf("!!(xstr >= xstr2)")
					 );
		CHECK_RESULT(!(xstr2 <= xstr),
					 sprintf("!(xstr2 <= xstr)"),
					 sprintf("!!(xstr2 <= xstr)")
					 );

	}

	for ( size_t pos = 0 ; pos < initialValue.utf32_length ; pos++) // this avoid empty string
	{
		const ch_t* s = incrementChar(initialValue.cha, pos, -1);
		CHECK_RESULT(xstr.strcmp(s) == 1,
					 sprintf("xstr.strcmp(s) == 1"),
					 sprintf("xstr.strcmp(s) != 1 (%d)", xstr.strcmp(s))
					 );
//const ch_t* s2 = incrementChar(initialValue.cha, pos, 1);
	}


	return SimpleString();
}

#define testCompare(XStringClass, classEncoding, encoding) \
    printf("Test %s::strcmp(%s)\n", STRINGIFY(XStringClass), STRINGIFY(encoding)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
		testCompare_<XStringClass>(testStringMultiCodedArray[i].encoding); \
	} \


/*****************************  indexOf, rindexOf  *****************************/

template<class XStringClass, typename ch_t>
static void testIdxOf__(XStringClass subStr, bool ignoreCase,
						size_t (XStringClass::*indexOfChar)(char32_t, size_t) const,
						size_t (XStringClass::*indexOfString)(const ch_t*, size_t) const,
						size_t (XStringClass::*rindexOfChar)(char32_t, size_t) const,
						size_t (XStringClass::*rindexOfString)(const ch_t*, size_t) const
						)
{
	XStringClass testStr;
	
	testStr = subStr;
	if ( ignoreCase ) testStr.lowerAscii();
	CHECK_RESULT((testStr.*indexOfString)(subStr.s(), 0) == 0,
				 sprintf("testStr.IdxOf(subStr.s(), 0) == 0"),
				 sprintf("testStr.IdxOf(subStr.s(), 0) != 0 (%zu)", (testStr.*indexOfString)(subStr.s(), 0))
				 );
	(testStr.*indexOfString)(subStr.s(), 0);
	size_t expectedPos = subStr.length()==0 ? testStr.length() : 0;
	CHECK_RESULT((testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1) == expectedPos,
				 sprintf("testStr.IdxOf(subStr.s(), 0) == expectedPos (%zu)", expectedPos),
				 sprintf("testStr.IdxOf(subStr.s(), 0) != 0 (%zu!=%zu)", (testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1), expectedPos)
				 );
	(testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1);
	
	XStringClass apple;
	apple.takeValueFrom("");
	CHECK_RESULT((testStr.*indexOfString)(apple.s(), 0) == MAX_XSIZE,
				 sprintf("testStr.*indexOfString)(\"\", 0) == MAX_XSIZE"),
				 sprintf("testStr.*indexOfString)(\"\", 0) != MAX_XSIZE (%zu)", (testStr.*indexOfString)(subStr.s(), 0))
				 );
	//(testStr.*indexOfString)("");
	CHECK_RESULT((testStr.*rindexOfString)(apple.s(), MAX_XSIZE-1) == MAX_XSIZE,
				 sprintf("(testStr.*rindexOfString)(\"\", MAX_XSIZE-1) == MAX_XSIZE"),
				 sprintf("(testStr.*rindexOfString)(\"\", MAX_XSIZE-1) != MAX_XSIZE (%zu)", (testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1))
				 );
	
	if ( subStr.length() > 0 )
	{
		testStr.takeValueFrom("");
		testStr.strcat(subStr.s());
		if ( ignoreCase ) testStr.lowerAscii();
		CHECK_RESULT((testStr.*indexOfString)(subStr.s(), 0) == 3,
					 sprintf("testStr.IdxOf(subStr.s(), 0) == 3"),
					 sprintf("testStr.IdxOf(subStr.s(), 0) != 3 (%zu)", (testStr.*indexOfString)(subStr.s(), 0))
					 );
(testStr.*indexOfString)(subStr.s(), 0);
		CHECK_RESULT((testStr.*indexOfString)(subStr.s(), 0) == 3,
					 sprintf("(testStr.*indexOfString)(subStr.s(), 0) == 3"),
					 sprintf("(testStr.*indexOfString)(subStr.s(), 0) != 3 (%zu)", (testStr.*indexOfString)(subStr.s(), 0))
					 );
		CHECK_RESULT((testStr.*indexOfChar)(subStr[0], 0) == 3,
					 sprintf("(testStr.*indexOfString)(subStr[0]) == 3"),
					 sprintf("(testStr.*indexOfString)(subStr[0]) != 3 (%zu)", (testStr.*indexOfChar)(subStr[0], 0))
					 );
		CHECK_RESULT((testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1) == 3,
					 sprintf("testStr.IdxOf(subStr.s(), MAX_XSIZE-1) == 3"),
					 sprintf("testStr.IdxOf(subStr.s(), MAX_XSIZE-1) != 3 (%zu)", (testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1))
					 );
(testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1);
		CHECK_RESULT((testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1) == 3,
					 sprintf("(testStr.*rindexOfString)(subStr.s(), 0) == 3"),
					 sprintf("(testStr.*rindexOfString)(subStr.s(), 0) != 3 (%zu)", (testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1))
					 );
		CHECK_RESULT((testStr.*rindexOfChar)(subStr[subStr.length()-1], MAX_XSIZE-1) == 3 + subStr.length() - 1,
					 sprintf("(testStr.*rindexOfString)(subStr[subStr.length()-1]) == 3 + subStr.length() - 1 (%zu)", 3 + subStr.length() - 1),
					 sprintf("(testStr.*rindexOfString)(subStr[subStr.length()-1]) == 3 + subStr.length() - 1 (%zu!=%zu)", (testStr.*rindexOfChar)(subStr[subStr.length()-1], MAX_XSIZE-1), 3 + subStr.length() - 1)
					 );
		
		testStr.takeValueFrom("");
		testStr.strcat(subStr.s());
		testStr.strcat("");
		if ( ignoreCase ) testStr.lowerAscii();
		CHECK_RESULT((testStr.*indexOfString)(subStr.s(), 0) == 4,
					 sprintf("testStr.IdxOf(subStr.s(), 0) == 4"),
					 sprintf("testStr.IdxOf(subStr.s(), 0) != 4 (%zu)", (testStr.*indexOfString)(subStr.s(), 0))
					 );
		(testStr.*indexOfString)(subStr.s(), 0);
		CHECK_RESULT((testStr.*indexOfString)(subStr.s(), 0) == 4,
					 sprintf("(testStr.*indexOfString)(subStr.s(), 0) == 4"),
					 sprintf("(testStr.*indexOfString)(subStr.s(), 0) != 4 (%zu)", (testStr.*indexOfString)(subStr.s(), 0))
					 );
		CHECK_RESULT((testStr.*indexOfChar)(subStr[0], 0) == 4,
					 sprintf("(testStr.*indexOfString)(subStr[0]) == 4"),
					 sprintf("(testStr.*indexOfString)(subStr[0]) != 4 (%zu)", (testStr.*indexOfChar)(subStr[0], 0))
					 );
		CHECK_RESULT((testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1) == 4,
					 sprintf("testStr.IdxOf(subStr.s(), 0) == 4"),
					 sprintf("testStr.IdxOf(subStr.s(), 0) != 4 (%zu)", (testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1))
					 );
		CHECK_RESULT((testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1) == 4,
					 sprintf("(testStr.*rindexOfString)(subStr.s(), 0) == 4"),
					 sprintf("(testStr.*rindexOfString)(subStr.s(), 0) != 4 (%zu)", (testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1))
					 );
		CHECK_RESULT((testStr.*rindexOfChar)(subStr[subStr.length()-1], MAX_XSIZE-1) == 4 + subStr.length() - 1,
					 sprintf("(testStr.*rindexOfString)(subStr[subStr.length()-1]) == 4 + subStr.length() - 1 (%zu)", 4 + subStr.length() - 1),
					 sprintf("(testStr.*rindexOfString)(subStr[subStr.length()-1]) == 4 + subStr.length() - 1 (%zu!=%zu)", (testStr.*rindexOfChar)(subStr[subStr.length()-1], MAX_XSIZE-1), 4 + subStr.length() - 1)
					 );
		
		testStr.takeValueFrom("");
		testStr.strcat(subStr.s());
		testStr.strcat("");
		testStr.strcat(subStr.s());
		testStr.strcat("");
		if ( ignoreCase ) testStr.lowerAscii();
		CHECK_RESULT((testStr.*indexOfString)(subStr.s(), 0) == 5,
					 sprintf("testStr.IdxOf(subStr.s(), 0) == 5"),
					 sprintf("testStr.IdxOf(subStr.s(), 0) != 5 (%zu)", (testStr.*indexOfString)(subStr.s(), 0))
					 );
		CHECK_RESULT((testStr.*indexOfString)(subStr.s(), 0) == 5,
					 sprintf("(testStr.*indexOfString)(subStr.s(), 0) == 5"),
					 sprintf("(testStr.*indexOfString)(subStr.s(), 0) != 5 (%zu)", (testStr.*indexOfString)(subStr.s(), 0))
					 );
		CHECK_RESULT((testStr.*indexOfChar)(subStr[0], 0) == 5,
					 sprintf("(testStr.*indexOfString)(subStr[0]) == 5"),
					 sprintf("(testStr.*indexOfString)(subStr[0]) != 5 (%zu)", (testStr.*indexOfChar)(subStr[0], 0))
					 );
		CHECK_RESULT((testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1) == 5 + subStr.length() + 6,
					 sprintf("testStr.IdxOf(subStr.s(), 0) == 5 + subStr.length() + 6"),
					 sprintf("testStr.IdxOf(subStr.s(), 0) != 5 + subStr.length() + 6 (%zu!=%zu)", (testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1), 5 + subStr.length() + 6)
					 );
(testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1);
		CHECK_RESULT((testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1) == 5 + subStr.length() + 6,
					 sprintf("(testStr.*rindexOfString)(subStr.s(), 0) == 5 + subStr.length() + 6"),
					 sprintf("(testStr.*rindexOfString)(subStr.s(), 0) != 5 + subStr.length() + 6 (%zu!=%zu)", (testStr.*rindexOfString)(subStr.s(), MAX_XSIZE-1), 5 + subStr.length() + 6)
					 );
		CHECK_RESULT((testStr.*rindexOfChar)(subStr[subStr.length()-1], MAX_XSIZE-1) == 5 + subStr.length() + 6 + subStr.length() - 1,
					 sprintf("(testStr.*rindexOfString)(subStr[subStr.length()-1]) == 5 + subStr.length() + 6 + subStr.length() - 1 (%zu)", 5 + subStr.length() + 6 + subStr.length() - 1),
					 sprintf("(testStr.*rindexOfString)(subStr[subStr.length()-1]) == 5 + subStr.length() + 6 + subStr.length() - 1 (%zu!=%zu)", (testStr.*rindexOfChar)(subStr[subStr.length()-1], MAX_XSIZE-1), 5 + subStr.length() + 6 + subStr.length() - 1)
					 );
	}
}

template<class XStringClass, class InitialValue>
SimpleString testIdxOf_(const InitialValue& initialValue)
{
	TEST_TITLE(displayOnlyFailed, sprintf("Test %s::idxOf", XStringClassInfo<XStringClass>::xStringClassName));

	typedef typename XStringClassInfo<XStringClass>::xs_t ixs_t;
	typedef typename XStringClassInfo<XStringClass>::ch_t ich_t;
//	ch_t c; // dummy for call utf function

	ixs_t str;
	str.takeValueFrom(initialValue.cha);
	
	for ( size_t pos = 0 ; pos < initialValue.utf32_length+3 ; pos+=1 ) {
		for ( size_t count = 0 ; count < initialValue.utf32_length-pos+3 ; count+=1 )
		{
			ixs_t subStr = str.SubString(pos, count);
			
//			size_t (ixs_t::*p)(const ich_t* S, size_t Pos) const = &ixs_t::template indexOf<ich_t>;
			testIdxOf__<ixs_t, ich_t>(subStr, false, &ixs_t::indexOf, &ixs_t::template indexOf, &ixs_t::rindexOf, &ixs_t::template rindexOf);
		
			ixs_t subStrLower = subStr;
			testIdxOf__<ixs_t, ich_t>(subStrLower, true, &ixs_t::indexOfIC, &ixs_t::template indexOfIC, &ixs_t::rindexOfIC, &ixs_t::template rindexOfIC);
//			if ( subStrLower != subStr ) {
//			}
		}
	}

	return SimpleString();
}

#define testIdxOf(XStringClass, classEncoding, encoding) \
    printf("Test %s::testIdxOf(%s)\n", STRINGIFY(XStringClass), STRINGIFY(encoding)); \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded ; i++ ) { \
		testIdxOf_<XStringClass>(testStringMultiCodedArray[i].encoding); \
	} \
	for ( size_t i = 0 ; i < nbTestStringMultiCoded4CaseArray ; i++ ) { \
		testIdxOf_<XStringClass>(testStringMultiCoded4CaseArray[i].encoding); \
	} \




/*****************************    *****************************/
#undef realloc
//
//#include <type_traits>
//#include <typeinfo>
//#include <iostream>

//std::is_class

void func_test(XStringW& xsw)
{
	(void)xsw;
}

int XStringx_tests()
{
#ifdef JIEF_DEBUG
//	DebugLog(2, "XString16_tests -> Enter\n");
#endif

//const char c = ' ';
//const char* cc = "  ";
//char buf[64];
//char16_t* buf16 = (char16_t*)u"foo16";
XString x1;
x1.takeValueFrom("a");
XString x2;
x2.takeValueFrom("b");

XStringW xw1; xw1.takeValueFrom("c");

auto x3 = xw1 = x1;
func_test(x3);
//
//XStringAbstract__is_char_st<decltype(buf16)> a;
//printf("a.v()=%d\n", a.v());

//XStringW sw1 = buf16;

//std::is_array<decltype(buf)> b;

//XStringAbstract__enable_if_t<is_char(decltype(c))>::type a;

//printf("%ls", XStringW().takeValueFrom(" ").s());
//func_test(XStringW().takeValueFrom(" "));

//	testDefaultCtor<XString>("XString");
//	XString16 a = u"toto"_XS16;
//        teststrncpy_<XString>("utf8", testStringMultiCodedArray[1].utf8, testStringMultiCodedArray[1].wchar);
//testIdxOf(XString, utf8, utf16);
//testCompare(XString, utf8, utf8);
		testIdxOf_<XString>(testStringMultiCoded4CaseArray[0].utf8); \

//size_t size = sizeof("ギ")-1; // this char is 6 bytes long !
//size_t size = sizeof("ꇉ")-1; // this char is 3 bytes long
//size_t size = sizeof("伽")-1; // this char is 3 bytes long
//size_t size = sizeof("楘")-1; // this char is 3 bytes long
//XString str = "ギꇉ伽楘"_XS;
//char* s = str.data(42);

	TEST_ALL_CLASSES(testDefaultCtor, __TEST0);
	TEST_ALL_CLASSES(testEmpty, __TEST0);
	TEST_ALL_CLASSES(testTakeValueFrom, TEST_ALL_UTF);
	TEST_ALL_CLASSES(testTakeValueFromXString, TEST_ALL_UTF);
	TEST_ALL_CLASSES(testchar32At_u, TEST_ALL_INTEGRAL);

	TEST_ALL_CLASSES(teststrcpy, TEST_ALL_UTF);
	TEST_ALL_CLASSES(teststrncpy, TEST_ALL_UTF); // 26944 tests
	TEST_ALL_CLASSES(teststrcat, TEST_ALL_UTF_ALL_UTF);
	TEST_ALL_CLASSES(teststrncat, TEST_ALL_UTF_ALL_UTF); // 2101632 tests

	TEST_ALL_CLASSES(testSubString, __TEST0);
	TEST_ALL_CLASSES(testCompare, TEST_ALL_UTF);
	TEST_ALL_CLASSES(testIdxOf, TEST_ALL_UTF);



//
////		str2.insert(1, str);
//
//	str2.SPrintf("%c", 'a'); // signle UTF8 ascii char
//	if ( str2 != L"a" ) return


	// IMPORTANT : you can't pass a litteral char in a vararg function with Visual Studio (Microsoft strikes again :-).
	//             At least, you got a warning C4066
	// IMPORTANT2 : Litteral string containing UTF16 char are WRONG. And you don't get a warning !!! If litteral is only ascii, it's ok.
	// Maybe it's compilation option but I didn't find them.


#ifdef JIEF_DEBUG
	if ( nbTestFailed == 0 ) printf("All %d tests succeeded.\n", nbTest);
	else printf("%d tests succeeded out of %d.\n", nbTest-nbTestFailed, nbTestFailed);
#endif
	return 0;
}

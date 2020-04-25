//*************************************************************************************************
//*************************************************************************************************
//
//                                          XSTRING
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XSTRINGABSTRACT_H__)
#define __XSTRINGABSTRACT_H__

#include <XToolsConf.h>
#include "unicode_conversions.h"

#ifndef DEBUG_ALL
#define DEBUG_XStringAbstract 0
#else
#define DEBUG_TEXT DEBUG_ALL
#endif

#if DEBUG_XStringAbstract == 0
#define DBG_XSTRING(...)
#else
#define DBG_XSTRING(...) DebugLog(DEBUG_XStringAbstract, __VA_ARGS__)
#endif

//#include <type_traits>

#define LPATH_SEPARATOR L'\\'

#if __WCHAR_MAX__ <= 0xFFFFu
    #define wchar_cast char16_t
#else
    #define wchar_cast char32_t
#endif


struct XStringAbstract__false_type {
    static constexpr bool value = false;
    bool v() const { return false; }
};

struct XStringAbstract__true_type {
    static constexpr bool value = true;
    bool v() const { return true; }
};

/* make unsigned */
template <class _Tp>
struct XStringAbstract__make_unsigned {};

template <> struct XStringAbstract__make_unsigned<         char>      {typedef unsigned char      type;};
template <> struct XStringAbstract__make_unsigned<  signed char>      {typedef unsigned char      type;};
template <> struct XStringAbstract__make_unsigned<unsigned char>      {typedef unsigned char      type;};
template <> struct XStringAbstract__make_unsigned<     char16_t>      {typedef char16_t           type;};
template <> struct XStringAbstract__make_unsigned<     char32_t>      {typedef char32_t           type;};
template <> struct XStringAbstract__make_unsigned<      wchar_t>      {typedef wchar_t            type;};
template <> struct XStringAbstract__make_unsigned<  signed short>     {typedef unsigned short     type;};
template <> struct XStringAbstract__make_unsigned<unsigned short>     {typedef unsigned short     type;};
template <> struct XStringAbstract__make_unsigned<  signed int>       {typedef unsigned int       type;};
template <> struct XStringAbstract__make_unsigned<unsigned int>       {typedef unsigned int       type;};
template <> struct XStringAbstract__make_unsigned<  signed long>      {typedef unsigned long      type;};
template <> struct XStringAbstract__make_unsigned<unsigned long>      {typedef unsigned long      type;};
template <> struct XStringAbstract__make_unsigned<  signed long long> {typedef unsigned long long type;};
template <> struct XStringAbstract__make_unsigned<unsigned long long> {typedef unsigned long long type;};
#define unsigned_type(x) typename XStringAbstract__make_unsigned<x>::type

/* enable_if */
template <bool, typename T = void>
struct XStringAbstract__enable_if_t
{};

template <typename T>
struct XStringAbstract__enable_if_t<true, T> {
  typedef T type;
};
//#define enable_if(x) XStringAbstract__enable_if_t(x, void)::type
#define enable_if(x) typename enable_if_type = typename XStringAbstract__enable_if_t<x>::type
//
//template< bool B, class T = void >
//using XStringAbstract__enable_if_t = typename XStringAbstract__enable_if<B,T>::type;


// is_integral
template <class _Tp> struct XStringAbstract__is_integral_st                     : public XStringAbstract__false_type {};
template <>          struct XStringAbstract__is_integral_st<bool>               : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<char>               : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<signed char>        : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned char>      : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<wchar_t>            : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<short>              : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned short>     : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<int>                : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned int>       : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<long>               : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned long>      : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<long long>          : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned long long> : public XStringAbstract__true_type {};
#define is_integral(x) XStringAbstract__is_integral_st<x>::value


// is_char
template <class _Tp>  struct XStringAbstract__is_char_st                        : public XStringAbstract__false_type {};
template <>           struct XStringAbstract__is_char_st<char>                  : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<char[]>                : public XStringAbstract__true_type {};
template <size_t _Np> struct XStringAbstract__is_char_st<char[_Np]>             : public XStringAbstract__true_type {};
//template <>           struct XStringAbstract__is_char_st<signed char>           : public XStringAbstract__true_type {};
//template <>           struct XStringAbstract__is_char_st<unsigned char>         : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<char16_t>              : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<char16_t[]>            : public XStringAbstract__true_type {};
template <size_t _Np> struct XStringAbstract__is_char_st<char16_t[_Np]>         : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<char32_t>              : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<char32_t[]>            : public XStringAbstract__true_type {};
template <size_t _Np> struct XStringAbstract__is_char_st<char32_t[_Np]>         : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<wchar_t>               : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<wchar_t[]>             : public XStringAbstract__true_type {};
template <size_t _Np> struct XStringAbstract__is_char_st<wchar_t[_Np]>          : public XStringAbstract__true_type {};
#define is_char(x) XStringAbstract__is_char_st<x>::value







#define asciiToLower(ch) (((ch >= L'A') && (ch <= L'Z')) ? ((ch - L'A') + L'a') : ch)

template<typename S, typename O>
int XStringAbstract__compare(const S* src, const O* other, bool ignoreCase)
{
//	size_t len_s = length_of_utf_string(src);
//	size_t len_other = length_of_utf_string(other);
	size_t nb = 0;
	const S* src2 = src;
	const O* other2 = other;

	char32_t src_char32;
	char32_t other_char32;
	src2 = get_char32_from_string(src2, &src_char32);
	other2 = get_char32_from_string(other2, &other_char32);
	while ( src_char32 ) {
		if ( ignoreCase ) {
			src_char32 = asciiToLower(src_char32);
			other_char32 = asciiToLower(other_char32);
		}
		if ( src_char32 != other_char32 ) break;
		src2 = get_char32_from_string(src2, &src_char32);
		other2 = get_char32_from_string(other2, &other_char32);
		nb += 1;
	};
	if ( src_char32 == other_char32 ) return 0;
	return src_char32 > other_char32 ? 1 : -1;
}

template<typename O, typename P>
size_t XStringAbstract__indexOf(const O** s, const P* other, size_t offsetRet, bool toLower)
{
	size_t Idx = 0;

	char32_t s_char32;
	char32_t other_char32;

	do
	{
		const O* s2 = *s;
		const P* other2 = other;
		do {
			s2 = get_char32_from_string(s2, &s_char32);
			other2 = get_char32_from_string(other2, &other_char32);
			if ( toLower ) {
				s_char32 = asciiToLower(s_char32);
				other_char32 = asciiToLower(other_char32);
			}
		} while ( s_char32  &&  other_char32  && s_char32 == other_char32 );
		if ( other_char32 == 0 ) return Idx+offsetRet;
		*s = get_char32_from_string(*s, &s_char32);
		Idx++;
	} while (s_char32);
	return MAX_XSIZE;
}

template<typename O, typename P>
size_t XStringAbstract__indexOf(const O* s, size_t Pos, const P* other, bool toLower)
{
	if ( *other == 0 ) return Pos;

	char32_t char32 = 1;
	for ( size_t Idx=0 ; Idx<Pos ; Idx+=1 ) {
		s = get_char32_from_string(s, &char32);
	}
	if ( !char32 ) return MAX_XSIZE;
	return XStringAbstract__indexOf(&s, other, Pos, toLower);
}

template<typename O, typename P>
size_t XStringAbstract__rindexOf(const O* s, size_t Pos, const P* other, bool toLower)
{
	if ( *other == 0 ) return Pos > length_of_utf_string(s) ? length_of_utf_string(s) : Pos;

	size_t index = XStringAbstract__indexOf(&s, other, 0, toLower);
	size_t prev_index = index; // initialize to index in case of index is already == Pos
	
	char32_t char32;
	s = get_char32_from_string(s, &char32);
	while ( char32  && index < Pos ) {
		prev_index = index;
		index = XStringAbstract__indexOf(&s, other, index+1, toLower);
		s = get_char32_from_string(s, &char32);
	};
	if ( index == Pos ) return index;
	if ( prev_index <= Pos ) return prev_index;
	return MAX_XSIZE;
}


template<class T, class ThisXStringClass>
class XStringAbstract
{
public:
//	const SubType NullXString;
	static T nullChar;

protected:
 	T *m_data;
	size_t m_allocatedSize;
	
	// convenience method. Did it this way to avoid #define in header. They can have an impact on other headers
	size_t Xmin(size_t x1, size_t x2) const { if ( x1 < x2 ) return x1; return x2; }
	size_t Xmax(size_t x1, size_t x2) const { if ( x1 > x2 ) return x1; return x2; }

// Methods _data is protected intentionally. They are const method returning non-const pointer. That's intentional, but dangerous. Do not expose to public.
// If you need a non-const pointer for low-level access, to use dataSized and have to specify the size
	// pos is counted in logical char
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	T* _data(IntegralType pos) const
	{
		if ( pos<0 ) panic("T* data(int i) -> i < 0");
 		size_t offset = size_of_utf_string_len(m_data, (unsigned_type(IntegralType))pos); // If pos is too big, size_of_utf_string_len returns the end of the string
 		return m_data + offset;
	}
	
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	// Init , Alloc
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	
	void Init(size_t aSize=0)
	{
		//DBG_XSTRING("Init aSize=%d\n", aSize);
		// We don't allocate any memory at first. To not have to test all the time if m_data is null, we init it to an empty string
		m_data = &nullChar;
		m_allocatedSize = 0;
		// if aSize == 0, nothing is done, because m_allocatedSize == aSize
		CheckSize(aSize, 0);
	}
public:
	T *CheckSize(size_t nNewSize, size_t nGrowBy = XStringGrowByDefault) // nNewSize is in number of chars, NOT bytes
	{
		//DBG_XSTRING("CheckSize: m_size=%d, nNewSize=%d\n", m_size, nNewSize);
		
		if ( m_allocatedSize < nNewSize )
		{
			nNewSize += nGrowBy;
			if ( m_allocatedSize == 0 ) m_data = (T*)malloc( (nNewSize+1)*sizeof(T) );
			else m_data = (T*)Xrealloc(m_data, (nNewSize+1)*sizeof(T), (m_allocatedSize+1)*sizeof(T));
			if ( !m_data ) {
				panic("XStringAbstract<T>::CheckSize(%zu, %zu) : Xrealloc(%" PRIuPTR ", %lu, %zd) returned NULL. System halted\n", nNewSize, nGrowBy, uintptr_t(m_data), nNewSize*sizeof(T), m_allocatedSize*sizeof(T));
			}
			m_allocatedSize = nNewSize;
			m_data[m_allocatedSize] = 0; // we allocated one more char (nNewSize+1). This \0 is an extra precaution. It's not for the normal null terminator. All string operation must considered that only m_allocatedSize bytes were allocated.
		}
		return m_data;
	}
//	void setSize(size_t newSize) // nNewSize is in number of chars, NOT bytes
//	{
//		//DBG_XSTRING("setLength(%d)\n", len);
//		CheckSize(newSize);
//		//	if ( len >= size() ) {
//		//		DBG_XSTRING("XStringAbstract<T>::setLength(size_t len) : len >= size() (%d != %d). System halted\n", len, size());
//		//		panic();
//		//	}
//		m_data[newSize] = 0; // we may rewrite a 0 in nullChar, if no memory were allocated. That's ok.
//	}


//	T* memoryOffset(size_t i) {
//
//	}
	
public:
	XStringAbstract()
	{
		DBG_XSTRING("Construteur\n");
		Init(0);
	}

	~XStringAbstract()
	{
		//DBG_XSTRING("Destructor :%ls\n", data());
		if ( m_allocatedSize > 0 ) free((void*)m_data);
	}

	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	T* data(IntegralType pos) const { return _data(pos); }

//	template<typename IntegralType, typename XStringAbstract__enable_if<XStringAbstract__is_integral<IntegralType>::value, IntegralType>::type* = nullptr>
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	T* dataSized(IntegralType size)
	{
		if ( size<0 ) panic("T* dataSized() -> i < 0");
		if ( (unsigned_type(IntegralType))size > MAX_XSIZE ) panic("T* dataSized() -> i > MAX_XSIZE");
		CheckSize((size_t)size);
		return _data(0);
	}
//
//	// Pos is counted in logical char but size is counted in physical char (char, char16_t, char32_t or wchar_t)
//	template<typename IntegralType1, typename IntegralType2, enable_if(is_integral(IntegralType1) && is_integral(IntegralType2))>
//	T* dataSized(IntegralType1 pos, IntegralType2 size)
//	{
//		if ( pos<0 ) panic("T* dataSized(xisize i, size_t sizeMin, size_t nGrowBy) -> i < 0");
//		if ( size<0 ) panic("T* dataSized(xisize i, size_t sizeMin, size_t nGrowBy) -> i < 0");
// 		size_t offset = size_of_utf_string_len(m_data, (typename XStringAbstract__make_unsigned<IntegralType1>::type)pos); // If pos is too big, size_of_utf_string_len returns the end of the string
//		CheckSize(offset + (typename XStringAbstract__make_unsigned<IntegralType2>::type)size);
//		return _data(pos);
//	}


	T* forgetDataWithoutFreeing()
	{
		T* ret = m_data;
		Init(0);
		return ret;
	}

	size_t length() const { return length_of_utf_string(m_data); }
//	size_t sizeZZ() const { return size_of_utf_string(m_data); }
	size_t sizeInBytes() const { return size_of_utf_string(m_data)*sizeof(T); }
	size_t allocatedSize() const { return m_allocatedSize; }


	const T* wc_str() const { return m_data; }
	const T* c_str() const { return m_data; }
	const T* s() const { return m_data; }
	const T* data() const { return m_data; } // todo delete

	/* Empty ? */
	void setEmpty() { m_data[0] = 0; } // we may rewrite a 0 in nullChar if no memory were allocated (m_data == &nullChar). That's ok.
	bool isEmpty() const { return m_data == nullptr  ||  *m_data == 0; }
	bool notEmpty() const { return !isEmpty(); }


	//--------------------------------------------------------------------- cast

//	int ToInt() const;
//	size_t ToUInt() const;


	//--------------------------------------------------------------------- charAt, []

	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	char32_t char32At(IntegralType i) const
	{
		if (i < 0) {
			panic("XStringAbstract<T>::char32At(size_t i) : i < 0. System halted\n");
		}
		size_t nb = 0;
		const T *p = m_data;
		char32_t char32;
		do {
			p = get_char32_from_string(p, &char32);
			if (!char32) {
				panic("XStringAbstract::char32At(size_t i) : i >= length(). System halted\n");
			}
			nb += 1;
		} while (nb <= (unsigned_type(IntegralType))i);
		return char32;
	}
	
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	char16_t char16At(IntegralType i) const
	{
		char32_t char32 = char32At(i);
		if ( char32 >= 0x10000 ) return 0xFFFD; // � REPLACEMENT CHARACTER used to replace an unknown, unrecognized or unrepresentable character
		return (char16_t)char32;
	}
	
	/* [] */
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	char32_t operator [](IntegralType i) const { return char32At(i); }


	char32_t lastChar() const { if ( length() > 0 ) return char32At(length()-1); else return 0; }
	
	//--------------------------------------------------------------------- strcat, strcpy, operator =
	/* strncpy */
	template<typename O>
	void strncpy(const O* other, size_t other_len)
	{
		if ( other && *other && other_len > 0 ) {
			size_t newSize = utf_size_of_utf_string_len(m_data, other, other_len);
			CheckSize(newSize+1, 0);
			utf_string_from_utf_string_len(m_data, m_allocatedSize, other, other_len);
			m_data[newSize] = 0;
		}else{
			setEmpty();
		}
	}
	// Old name. TODO remove
	template<typename O>
	void StrnCpy(const O* other, size_t other_len) { strncpy(other, other_len); }

	/* strcpy */
	template<typename O>
	void strcpy(const O* other)
	{
		if ( other && *other ) {
			size_t newSize = utf_size_of_utf_string(m_data, other);
			CheckSize(newSize+1, 0);
			utf_string_from_utf_string(m_data, m_allocatedSize, other);
			m_data[newSize] = 0;
		}else{
			setEmpty();
		}
	}
	/* strncat */
	template<typename O>
	void strncat(const O* other, size_t other_len)
	{
		if ( other && *other && other_len > 0 ) {
			size_t currentSize = size_of_utf_string(m_data);
			size_t newSize = currentSize + utf_size_of_utf_string_len(m_data, other, other_len);
			CheckSize(newSize+1, 0);
			utf_string_from_utf_string_len(m_data+currentSize, m_allocatedSize, other, other_len);
			m_data[newSize] = 0;
		}else{
			// nothing to do
		}
	}
	/* strcat */
	template<typename O>
	void strcat(const O* other)
	{
		if ( other && *other ) {
			size_t currentSize = size_of_utf_string(m_data); // size is number of T, not in bytes
			size_t newSize = currentSize + utf_size_of_utf_string(m_data, other); // size is number of T, not in bytes
			CheckSize(newSize+1, 0);
			utf_string_from_utf_string(m_data+currentSize, m_allocatedSize-currentSize, other);
			m_data[newSize] = 0;
		}else{
			// nothing to do
		}
	}
	/* takeValueFrom */
	template<typename O, class OtherXStringClass>
	ThisXStringClass& takeValueFrom(const XStringAbstract<O, OtherXStringClass>& S) { strcpy(S.s()); return *((ThisXStringClass*)this); }
	template<typename O>
	ThisXStringClass& takeValueFrom(const O* S) { strcpy(S); return *((ThisXStringClass*)this); }
	template<typename O, class OtherXStringClass>
	ThisXStringClass& takeValueFrom(const XStringAbstract<O, OtherXStringClass>& S, size_t len) { strncpy(S.data(0), len); return *((ThisXStringClass*)this);	}
	template<typename O>
	ThisXStringClass& takeValueFrom(const O* S, size_t len) {	strncpy(S, len); return *((ThisXStringClass*)this); }
	
	/* copy ctor */
	XStringAbstract<T, ThisXStringClass>(const XStringAbstract<T, ThisXStringClass> &S)	{ Init(0); takeValueFrom(S); }
	/* ctor */
	template<typename O, class OtherXStringClass>
	explicit XStringAbstract<T, ThisXStringClass>(const XStringAbstract<O, OtherXStringClass>& S) { Init(0); takeValueFrom(S); }
//	template<typename O>
//	explicit XStringAbstract<T, ThisXStringClass>(const O* S) { Init(0); takeValueFrom(S); }

	/* Copy Assign */ // Only other XString, no litteral at the moment.
	XStringAbstract<T, ThisXStringClass>& operator =(const XStringAbstract<T, ThisXStringClass>& S) { strcpy(S.s()); return *this; }
	/* Assign */
	template<typename O, class OtherXStringClass>
	ThisXStringClass& operator =(const XStringAbstract<O, OtherXStringClass>& S)	{ strcpy(S.s()); return *((ThisXStringClass*)this); }
//	template<class O>
//	ThisXStringClass& operator =(const O* S)	{ strcpy(S); return *this; }

	/* += */
	template<typename O, class OtherXStringClass>
	ThisXStringClass& operator += (const XStringAbstract<O, OtherXStringClass>& S) { strcat(S.s()); return *((ThisXStringClass*)this); }
	template<typename O>
	ThisXStringClass& operator += (const O* S) { strcat(S); return *((ThisXStringClass*)this); }


	template<typename O, class OtherXStringClass>
	ThisXStringClass operator + (const XStringAbstract<O, OtherXStringClass>& p2) const { XStringAbstract s; s=*this; s+=p2; return s; }
	template<typename O>
	ThisXStringClass operator + (const O* p2) const { XStringAbstract s; s=*this; s+=p2; return s; }
	template<typename O>
	friend ThisXStringClass operator + (const O *p1,   const ThisXStringClass& p2) { XStringAbstract s; s.strcat(p1); s.strcat(p2.s()); return s; }


	//--------------------------------------------------------------------- indexOf, rindexOf
	
	/* indexOf */
	size_t indexOf(char32_t char32Searched, size_t Pos = 0) const
	{
		char32_t buf[2] = { char32Searched, 0};
		return XStringAbstract__indexOf(m_data, Pos, buf, false);
	}
	template<typename O>
	size_t indexOf(const O* S, size_t Pos = 0) const { return XStringAbstract__indexOf(m_data, Pos, S, false); }
	template<typename O, class OtherXStringClass>
	size_t indexOf(const XStringAbstract<O, OtherXStringClass>& S, size_t Pos = 0) const { return indexOf(S.s(), Pos); }
	/* IC */
	size_t indexOfIC(char32_t char32Searched, size_t Pos = 0) const
	{
		char32_t buf[2] = { char32Searched, 0};
		return XStringAbstract__indexOf(m_data, Pos, buf, true);
	}
	template<typename O>
	size_t indexOfIC(const O* S, size_t Pos = 0) const { return XStringAbstract__indexOf(m_data, Pos, S, true); }
	template<typename O, class OtherXStringClass>
	size_t indexOfIC(const XStringAbstract<O, OtherXStringClass>& S, size_t Pos = 0) const { return indexOfIC(S.s(), Pos); }


	/* rindexOf */
	size_t rindexOf(const char32_t char32Searched, size_t Pos = MAX_XSIZE-1) const
	{
		char32_t buf[2] = { char32Searched, 0};
		return XStringAbstract__rindexOf(m_data, Pos, buf, false);
	}
	template<typename O>
	size_t rindexOf(const O* S, size_t Pos = MAX_XSIZE-1) const { return XStringAbstract__rindexOf(m_data, Pos, S, false); }
	template<typename O, class OtherXStringClass>
	size_t rindexOf(const XStringAbstract<O, OtherXStringClass>& S, size_t Pos = MAX_XSIZE-1) const { return rindexOf(S.s(), Pos); }
	/* IC */
	size_t rindexOfIC(const char32_t char32Searched, size_t Pos = MAX_XSIZE-1) const
	{
		char32_t buf[2] = { char32Searched, 0};
		return XStringAbstract__rindexOf(m_data, Pos, buf, true);
	}
	template<typename O>
	size_t rindexOfIC(const O* S, size_t Pos = MAX_XSIZE-1) const { return XStringAbstract__rindexOf(m_data, Pos, S, true); }
	template<typename O, class OtherXStringClass>
	size_t rindexOfIC(const XStringAbstract<O, OtherXStringClass>& S, size_t Pos = MAX_XSIZE-1) const { return rindexOf(S.s(), Pos); }


	//---------------------------------------------------------------------

	void lowerAscii()
	{
		T* s = m_data;
		while ( *s ) {
			*s = asciiToLower(*s);
			s++;
		}
	}
	
	void trim()
	{
		T* start = 0;
		size_t count = 0;
		T* s = m_data;
		while ( *s && unsigned_type(T)(*s) <= 32 ) s++;
		start = s;
		while ( *s && unsigned_type(T)(*s) > 32 ) s++;
		count = uintptr_t(s - start);
		memmove(m_data, start, count*sizeof(T));
		m_data[count] = 0;
	}

//	void deleteCountCharsAt(size_t pos, size_t count=1);
//{
//	if ( pos < size() ) {
//		if ( count != MAX_XSIZE  &&  pos + count < size() ) {
//			memmove( _data(pos), data(pos+count), (size()-pos-count)*sizeof(T)); // memmove handles overlapping memory move
//			setLength(size()-count);/* data()[length()-count]=0 done in setLength */
//		}else{
//			setSize(pos);/* data()[pos]=0 done in setLength */
//		}
//	}
//}
//	void insert(const XStringAbstract<T, ThisXStringClass>& Str, size_t pos);
//{
//	if ( pos < size() ) {
//		CheckSize(size()+Str.size());
//		memmove(_data(pos + Str.size()),  data(pos),  (size()-pos)*sizeof(T));
//		memmove(_data(pos), Str.data(), Str.size()*sizeof(T));
//		setLength(size()+Str.size());
//	}else{
//		StrCat(Str);
//	}
//}

	ThisXStringClass subString(size_t pos, size_t count) const
	{
		if ( pos > length() ) return ThisXStringClass();
		if ( count > length()-pos ) count = length()-pos;
		
		ThisXStringClass ret;

		const T* src = m_data;
		char32_t char32 = 1;
		while ( char32  && pos > 0 ) {
			src = get_char32_from_string(src, &char32);
			pos -= 1;
		};
		ret.strncat(src, count);
		return ret;
	}

	// todo rename to contains
	template<typename O, class OtherXStringClass>
	bool contains(const XStringAbstract<O, OtherXStringClass>& S) const { return indexOf(S) != MAX_XSIZE; }
	template<typename O>
	bool contains(const O* S) const { return indexOf(S) != MAX_XSIZE; }
	template<typename O, class OtherXStringClass>
	size_t containsIC(const XStringAbstract<O, OtherXStringClass>& S) const { return indexOfIC(S) != MAX_XSIZE; }
	template<typename O>
	size_t containsIC(const O* S) const { return indexOfIC(S) != MAX_XSIZE; }


//	void ToLower(bool FirstCharIsCap = false);
//	bool IsLetters() const;
//	bool IsLettersNoAccent() const;
//	bool IsDigits() const;
//{
//  const T *p;
//
//	p = data();
//	if ( !*p ) return false;
//	for ( ; *p ; p+=1 ) {
//		if ( *p < '0' ) return false;
//		if ( *p > '9' ) return false;
//	}
//	return true;
//}
//	bool IsDigits(size_t pos, size_t count) const;
//{
//  const T *p;
//  const T *q;
//
//	if ( pos >= size() ) {
//		return false;
//	}
//	if ( pos+count > size() ) {
//		return false;
//	}
//	p = data() + pos;
//	q = p + count;
//	for ( ; p < q ; p+=1 ) {
//		if ( *p < '0' ) return false;
//		if ( *p > '9' ) return false;
//	}
//	return true;
//}

//	void Replace(T c1, T c2)
//	{
//		T* p;
//
//		p = s();
//		while ( *p ) {
//			if ( *p == c1 ) *p = c2;
//			p += 1;
//		}
//	}
//	XStringAbstract SubStringReplace(T c1, T c2);
//{
//  T* p;
//  XStringAbstract Result;
//
//	p = s();
//	while ( *p  ) {
//		if ( *p == c1 ) Result += c2;
//		else Result += *p;
//		p++;
//	}
//	return Result;
//}
//
//	SubType basename() const
//	{
//		size_t idx = RIdxOf(LPATH_SEPARATOR);
//		if ( idx == MAX_XSIZE ) return SubType();
//		return SubString(idx+1, size()-idx-1);
//	}
//	SubType dirname() const
//	{
//		size_t idx = RIdxOf(LPATH_SEPARATOR);
//		if ( idx == MAX_XSIZE ) return SubType();
//		return SubString(0, idx);
//	}
//	void RemoveLastEspCtrl();

	//---------------------------------------------------------------------

	template<typename O>
	int strcmp(const O* S) const { return XStringAbstract__compare(m_data, S, false); }
//	int Compare(const char* S) const { return ::Compare<T, char>(m_data, S); }
//	int Compare(const char16_t* S) const { return ::Compare<T, char16_t>(m_data, S); };
//	int Compare(const char32_t* S) const { return ::Compare<T, char32_t>(m_data, S); };
//	int Compare(const wchar_t* S) const { return ::Compare<T, wchar_t>(m_data, S); };
//
	template<typename O, class OtherXStringClass>
	bool equalIC(const XStringAbstract<O, OtherXStringClass>& S) const { return XStringAbstract__compare(m_data, S.s(), true) == 0; }
	template<typename O>
	bool equalIC(const O* S) const { return XStringAbstract__compare(m_data, S, true) == 0; }
//	bool startWith(const T* S) const { return (memcmp(data(), S, wcslen(S)) == 0); }
//	bool SubStringEqual(size_t Pos, const T* S) const { return (memcmp(data(Pos), S, wcslen(S)) == 0); }

public:
	// == operator
	template<typename O, class OtherXStringClass>
	bool operator == (const XStringAbstract<O, OtherXStringClass>& s2) const { return (*this).strcmp(s2.s()) == 0; }
//	template<typename O>
//	bool operator == (const O* s2) const { return (*this).strcmp(s2) == 0; }
//	template<typename O>
//	friend bool operator == (const O* s1, ThisXStringClass& s2) { return s2.strcmp(s1) == 0; }

	template<typename O, class OtherXStringClass>
	bool operator != (const XStringAbstract<O, OtherXStringClass>& s2) const { return !(*this == s2); }
//	template<typename O>
//	bool operator != (const O* s2) const { return !(*this == s2); }
//	template<typename O>
//	friend bool operator != (const O* s1, const ThisXStringClass& s2) { return s2.strcmp(s1) != 0; }

	template<typename O, class OtherXStringClass>
	bool operator <  (const XStringAbstract<O, OtherXStringClass>& s2) const { return (*this).strcmp(s2.s()) < 0; }
//	template<typename O>
//	bool operator <  (const O* s2) const { return (*this).strcmp(s2) < 0; }
//	template<typename O>
//	friend bool operator <  (const O* s1, const ThisXStringClass& s2) { return s2.strcmp(s1) > 0; }

	template<typename O, class OtherXStringClass>
	bool operator >  (const XStringAbstract<O, OtherXStringClass>& s2) const { return (*this).strcmp(s2.s()) > 0; }
//	template<typename O>
//	bool operator >  (const O* s2) const { return  (*this).strcmp(s2) > 0; }
//	template<typename O>
//	friend bool operator >  (const O* s1, const ThisXStringClass& s2) { return s2.strcmp(s1) < 0; }

	template<typename O, class OtherXStringClass>
	bool operator <= (const XStringAbstract<O, OtherXStringClass>& s2) const { return (*this).strcmp(s2.s()) <= 0; }
//	template<typename O>
//	bool operator <= (const O* s2) const { return  (*this).strcmp(s2) <= 0; }
//	template<typename O>
//	friend bool operator <= (const O* s1, const ThisXStringClass& s2) { return s2.strcmp(s1) >= 0; }

	template<typename O, class OtherXStringClass>
	bool operator >= (const XStringAbstract<O, OtherXStringClass>& s2) const { return (*this).strcmp(s2.s()) >= 0; }
//	template<typename O>
//	bool operator >= (const O* s2) const { return  (*this).strcmp(s2) >= 0; }
//	template<typename O>
//	friend bool operator >= (const O* s1, const ThisXStringClass& s2) { return s2.strcmp(s1) <= 0; }

};


template<class T, class ThisXStringClass>
T XStringAbstract<T, ThisXStringClass>::nullChar = 0;



//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


//
//template<class T, class SubType, size_t growBy>
//void XStringAbstract<T>::RemoveLastEspCtrl()
//{
//  T *p;
//
//	if ( size() > 0 ) {
//		p = s() + size() - 1;
//	#if __WCHAR_MIN__ < 0
//		if ( *p >= 0 && *p <= ' ' ) {
//	#else
//		if ( *p <= ' ' ) {
//	#endif
//			p -= 1;
//	#if __WCHAR_MIN__ < 0
//			while ( p>data() && *p >= 0 && *p <= ' ' ) p -= 1;
//	#else
//			while ( p>data() && *p <= ' ' ) p -= 1;
//	#endif
//			if ( p>data() ) {
//				setSize( (size_t)(p-data())+1);
//			}else{
//	#if __WCHAR_MIN__ < 0
//				if ( *p >= 0 && *p <= ' ' ) setSize(0);
//	#else
//				if ( *p <= ' ' ) setSize(0);
//	#endif
//				else setSize(1);
//			}
//		}
//	}
//}















#undef DBG_XSTRING
#undef asciiToLower
#undef unsigned_type
#undef is_integral
#undef is_char
#undef enable_if



#endif // __XSTRINGABSTRACT_H__

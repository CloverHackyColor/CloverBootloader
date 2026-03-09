/*
 *
 * Created by jief in 1997.
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#if !defined(__XString_H__)
#define __XString_H__

#include <XToolsConf.h>
#include "XStringAbstract.h"

#include "../../Include/Library/printf_lite.h"

#ifndef XString16GrowByDefault
#define XString16GrowByDefault 16
#endif

//------------------------------------------------------------------------------------------------------------------
class XString8;
class LString8 : public LString<char, XString8, LString8>
{
  public:
	constexpr LString8() = delete;
  #ifdef XSTRING_CACHING_OF_SIZE
    LString8(const char* s) : LString<char, XString8>(s, utf8_size_of_utf8_string(s)) {};
    constexpr LString8(const char* s, size_t size) : LString<char, XString8>(s, size) {};
  #else
    constexpr LString8(const char* s) : LString<char, XString8, LString8>(s) {};
    constexpr LString8(const char* s, size_t size) : LString<char, XString8, LString8>(s) {};
  #endif

	// no assignement, no destructor

	friend constexpr LString8 operator ""_XS8 ( const char* s, size_t size) { return LString8(s, size); }

  const char* c_str() const { return data(); }

};

class XString8 : public XStringAbstract<char, XString8, LString8>
{
  public:
	XString8() : XStringAbstract<char, XString8, LString8>() {};
	XString8(const XString8& S) : XStringAbstract<char, XString8, LString8>(S) {}
	XString8(const LString8& S) : XStringAbstract<char, XString8, LString8>(S) { }

	template<class OtherXStringClass, enable_if( is___String(OtherXStringClass) && !is___LString(OtherXStringClass))> // enable_if is to avoid constructing with a non-corresponding LString. To avoid memory allocation.
	XString8(const OtherXStringClass& S) : XStringAbstract<char, XString8, LString8>(S) {}

	XString8& operator=(const XString8 &S) { this->XStringAbstract<char, XString8, LString8>::operator=(S); return *this; }

	using XStringAbstract<char, XString8, LString8>::operator =;

  const char* c_str() const { return data(); }
//  char* copy_str() const { return (char*)AllocateCopyPool(length()+1, m_data); }

protected:
	static void transmitS8Printf(const char* buf, unsigned int nbchar, void* context)
	{
		((XString8*)(context))->strsicat(buf, nbchar);
	}
public:
	void vS8Printf(const char* format, XTOOLS_VA_LIST va)
	{
		setEmpty();
		vprintf_with_callback(format, va, transmitS8Printf, this);
	}
	void S8Printf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)))
	{
		XTOOLS_VA_LIST     va;

		XTOOLS_VA_START (va, format);
		vS8Printf(format, va);
		XTOOLS_VA_END(va);
	}
  void vS8Catf(const char* format, XTOOLS_VA_LIST va)
  {
    vprintf_with_callback(format, va, transmitS8Printf, this);
  }
  void S8Catf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)))
  {
    XTOOLS_VA_LIST     va;

    XTOOLS_VA_START (va, format);
    vS8Catf(format, va);
    XTOOLS_VA_END(va);
  }
};


//------------------------------------------------------------------------------------------------------------------
class XString16;
class LString16 : public LString<char16_t, XString16, LString16>
{
  #ifdef XSTRING_CACHING_OF_SIZE
    constexpr LString16(const char16_t* s, size_t size) : LString<char16_t, XString16, LString16>(s, size) {};
  #else
    constexpr LString16(const char16_t* s, size_t size) : LString<char16_t, XString16, LString16>(s) {};
  #endif
	
	friend constexpr LString16 operator ""_XS16 ( const char16_t* s, size_t size) { return LString16(s, size); }
};

class XString16 : public XStringAbstract<char16_t, XString16, LString16>
{
  public:
	XString16() : XStringAbstract<char16_t, XString16, LString16>() {};
  XString16(const XString16& S) : XStringAbstract<char16_t, XString16, LString16>(S) {}
  XString16(const LString16& S) : XStringAbstract<char16_t, XString16, LString16>(S) {}

	template<class OtherXStringClass, enable_if( is___String(OtherXStringClass) && !is___LString(OtherXStringClass))> // enable_if is to avoid constructing with a non-corresponding LString. To avoid memory allocation.
	XString16(const OtherXStringClass& S) : XStringAbstract<char16_t, XString16, LString16>(S) {}

	XString16& operator=(const XString16 &S) { this->XStringAbstract<char16_t, XString16, LString16>::operator=(S); return *this; }

	using XStringAbstract<char16_t, XString16, LString16>::operator =;

//	friend LString16 operator "" _XS16 ( const char16_t* s, size_t len);
};


//------------------------------------------------------------------------------------------------------------------
class XString32;
class LString32 : public LString<char32_t, XString32 ,LString32>
{
  #ifdef XSTRING_CACHING_OF_SIZE
    constexpr LString32(const char32_t* s, size_t size) : LString<char32_t, XString32 ,LString32>(s, size) {};
  #else
    constexpr LString32(const char32_t* s, size_t size) : LString<char32_t, XString32 ,LString32>(s) {};
  #endif
	
	friend constexpr LString32 operator ""_XS32 ( const char32_t* s, size_t size) { return LString32(s, size); }
};

class XString32 : public XStringAbstract<char32_t, XString32, LString32>
{
  public:
	XString32() : XStringAbstract<char32_t, XString32 ,LString32>() {};
  XString32(const XString32& S) : XStringAbstract<char32_t, XString32 ,LString32>(S) {}
  XString32(const LString32& S) : XStringAbstract<char32_t, XString32 ,LString32>(S) {}

	template<class OtherXStringClass, enable_if( is___String(OtherXStringClass) && !is___LString(OtherXStringClass))> // enable_if is to avoid constructing with a non-corresponding LString. To avoid memory allocation.
	XString32(const OtherXStringClass& S) : XStringAbstract<char32_t, XString32, LString32>(S) {}

	XString32& operator=(const XString32 &S) { this->XStringAbstract<char32_t, XString32 ,LString32>::operator=(S); return *this; }
	
	using XStringAbstract<char32_t, XString32 ,LString32>::operator =;

//	friend LString32 operator "" _XS32 ( const char32_t* s, size_t len);
};

//------------------------------------------------------------------------------------------------------------------
class XStringW;
class LStringW : public LString<wchar_t, XStringW, LStringW>
{
  public:
	constexpr LStringW() = delete;
  
  #ifdef XSTRING_CACHING_OF_SIZE
    LStringW(const wchar_t* s) : LString<wchar_t, XStringW>(s, wchar_size_of_wchar_string(s)) {};
    constexpr LStringW(const wchar_t* s, size_t size) : LString<wchar_t, XStringW>(s, size) {};
  #else
    constexpr LStringW(const wchar_t* s) : LString<wchar_t, XStringW, LStringW>(s) {};
    constexpr LStringW(const wchar_t* s, size_t size) : LString<wchar_t, XStringW, LStringW>(s) {};
  #endif

	friend constexpr LStringW operator ""_XSW ( const wchar_t* s, size_t size) { return LStringW(s, size); }

  const wchar_t* wc_str() const { return data(); }
};

class XStringW : public XStringAbstract<wchar_t, XStringW, LStringW>
{
public:
	XStringW() : XStringAbstract<wchar_t, XStringW, LStringW>() {};
	XStringW(const XStringW& S) : XStringAbstract<wchar_t, XStringW, LStringW>(S) {}
  XStringW(const LStringW& S) : XStringAbstract<wchar_t, XStringW, LStringW>(S) { }

	template<class OtherXStringClass, enable_if( is___String(OtherXStringClass) && !is___LString(OtherXStringClass))> // enable_if is to avoid constructing with a non-corresponding LString. To avoid memory allocation.
	XStringW(const OtherXStringClass& S) : XStringAbstract<wchar_t, XStringW, LStringW>(S) {}
	

	XStringW& operator=(const XStringW &S) { this->XStringAbstract<wchar_t, XStringW, LStringW>::operator=(S); return *this; }

	using XStringAbstract<wchar_t, XStringW, LStringW>::operator =;

  const wchar_t* wc_str() const { return data(); }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	const wchar_t* wc_str(IntegralType idx) const { return data(idx); }

protected:
	static void transmitSWPrintf(const wchar_t* buf, unsigned int nbchar, void* context)
	{
		((XStringW*)(context))->strsicat(buf, nbchar);
	}
public:
	void vSWPrintf(const char* format, XTOOLS_VA_LIST va)
	{
		setEmpty();
		vwprintf_with_callback(format, va, transmitSWPrintf, this);
	}
	void SWPrintf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)))
	{
		XTOOLS_VA_LIST     va;

		XTOOLS_VA_START (va, format);
		vSWPrintf(format, va);
		XTOOLS_VA_END(va);
	}
  void vSWCatf(const char* format, XTOOLS_VA_LIST va)
  {
    vwprintf_with_callback(format, va, transmitSWPrintf, this);
  }
  void SWCatf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)))
  {
    XTOOLS_VA_LIST     va;

    XTOOLS_VA_START (va, format);
    vSWCatf(format, va);
    XTOOLS_VA_END(va);
  }
};


constexpr LString8 operator ""_XS8 ( const char* s, size_t len);
constexpr LString16 operator ""_XS16 ( const char16_t* s, size_t len);
constexpr LString32 operator ""_XS32 ( const char32_t* s, size_t len);
constexpr LStringW operator ""_XSW ( const wchar_t* s, size_t len);


#ifdef _MSC_VER
// I don't know why it's needed with VS.

template<>
struct _xstringarray__char_type<XString8, void>
{
  static const typename XString8::char_t* getCharPtr(const XString8& t) { return t.s(); }
};

template<>
struct _xstringarray__char_type<XStringW, void>
{
  static const typename XStringW::char_t* getCharPtr(const XStringW& t) { return t.s(); }
};

#endif

extern const XString8 NullXString8;
extern const XStringW NullXStringW;

#ifdef _MSC_VER
#   define __attribute__(x)
#endif

XString8 S8Printf(const char* format, ...) __attribute__((__format__ (__printf__, 1, 2)));
XStringW SWPrintf(const char* format, ...) __attribute__((__format__ (__printf__, 1, 2)));


//
//XStringAbstract SubString(const T *S, size_t pos, size_t count);

#endif

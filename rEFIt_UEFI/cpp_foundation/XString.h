//*************************************************************************************************
//*************************************************************************************************
//
//                                      STRING
//
// Developed by jief666, from 1997.
//
//*************************************************************************************************
//*************************************************************************************************


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
class LString8 : public LString<char, XString8>
{
  public:
	constexpr LString8() = delete;
	constexpr LString8(const char* s) : LString<char, XString8>(s) {};

	// no assignement, no destructor

	friend constexpr LString8 operator "" _XS8 ( const char* s, size_t) { return LString8(s); }
};

class XString8 : public XStringAbstract<char, XString8>
{
  public:
	XString8() : XStringAbstract<char, XString8>() {};
	XString8(const XString8& S) : XStringAbstract<char, XString8>(S) {}
	XString8(const LString8& S) : XStringAbstract<char, XString8>(S) { }

	template<class OtherXStringClass, enable_if( is___String(OtherXStringClass) && !is___LString(OtherXStringClass))> // enable_if is to avoid constructing with a non-corresponding LString. To avoid memory allocation.
	XString8(const OtherXStringClass& S) : XStringAbstract<char, XString8>(S) {}

	XString8& operator=(const XString8 &S) { this->XStringAbstract<char, XString8>::operator=(S); return *this; }

	using XStringAbstract<char, XString8>::operator =;
	
protected:
	static void transmitS8Printf(const char* buf, unsigned int nbchar, void* context)
	{
		((XString8*)(context))->strncat(buf, nbchar);
	}
public:
	void vS8Printf(const char* format, va_list va)
	{
		setEmpty();
		vprintf_with_callback(format, va, transmitS8Printf, this);
	}
	void S8Printf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)))
	{
		va_list     va;

		va_start (va, format);
		vS8Printf(format, va);
		va_end(va);
	}
};


//------------------------------------------------------------------------------------------------------------------
class XString16;
class LString16 : public LString<char16_t, XString16>
{
	constexpr LString16(const char16_t* s) : LString<char16_t, XString16>(s) {};
	
	friend constexpr LString16 operator "" _XS16 ( const char16_t* s, size_t) { return LString16(s); }
};

class XString16 : public XStringAbstract<char16_t, XString16>
{
  public:
	XString16() : XStringAbstract<char16_t, XString16>() {};
  XString16(const XString16& S) : XStringAbstract<char16_t, XString16>(S) {}
  XString16(const LString16& S) : XStringAbstract<char16_t, XString16>(S) {}

	template<class OtherXStringClass, enable_if( is___String(OtherXStringClass) && !is___LString(OtherXStringClass))> // enable_if is to avoid constructing with a non-corresponding LString. To avoid memory allocation.
	XString16(const OtherXStringClass& S) : XStringAbstract<char16_t, XString16>(S) {}

	XString16& operator=(const XString16 &S) { this->XStringAbstract<char16_t, XString16>::operator=(S); return *this; }

	using XStringAbstract<char16_t, XString16>::operator =;

//	friend LString16 operator "" _XS16 ( const char16_t* s, size_t len);
};


//------------------------------------------------------------------------------------------------------------------
class XString32;
class LString32 : public LString<char32_t, XString32>
{
	constexpr LString32(const char32_t* s) : LString<char32_t, XString32>(s) {};
	
	friend constexpr LString32 operator "" _XS32 ( const char32_t* s, size_t) { return LString32(s); }
};

class XString32 : public XStringAbstract<char32_t, XString32>
{
  public:
	XString32() : XStringAbstract<char32_t, XString32>() {};
  XString32(const XString32& S) : XStringAbstract<char32_t, XString32>(S) {}
  XString32(const LString32& S) : XStringAbstract<char32_t, XString32>(S) {}

	template<class OtherXStringClass, enable_if( is___String(OtherXStringClass) && !is___LString(OtherXStringClass))> // enable_if is to avoid constructing with a non-corresponding LString. To avoid memory allocation.
	XString32(const OtherXStringClass& S) : XStringAbstract<char32_t, XString32>(S) {}

	XString32& operator=(const XString32 &S) { this->XStringAbstract<char32_t, XString32>::operator=(S); return *this; }
	
	using XStringAbstract<char32_t, XString32>::operator =;

//	friend LString32 operator "" _XS32 ( const char32_t* s, size_t len);
};

//------------------------------------------------------------------------------------------------------------------
class XStringW;
class LStringW : public LString<wchar_t, XStringW>
{
  public:
	constexpr LStringW() = delete;
	constexpr LStringW(const wchar_t* s) : LString<wchar_t, XStringW>(s) {};
	
	friend constexpr LStringW operator "" _XSW ( const wchar_t* s, size_t) { return LStringW(s); }
};

class XStringW : public XStringAbstract<wchar_t, XStringW>
{
public:
	XStringW() : XStringAbstract<wchar_t, XStringW>() {};
	XStringW(const XStringW& S) : XStringAbstract<wchar_t, XStringW>(S) {}
  XStringW(const LStringW& S) : XStringAbstract<wchar_t, XStringW>(S) { }

	template<class OtherXStringClass, enable_if( is___String(OtherXStringClass) && !is___LString(OtherXStringClass))> // enable_if is to avoid constructing with a non-corresponding LString. To avoid memory allocation.
	XStringW(const OtherXStringClass& S) : XStringAbstract<wchar_t, XStringW>(S) {}
	

	XStringW& operator=(const XStringW &S) { this->XStringAbstract<wchar_t, XStringW>::operator=(S); return *this; }

	using XStringAbstract<wchar_t, XStringW>::operator =;

protected:
	static void transmitSPrintf(const wchar_t* buf, unsigned int nbchar, void* context)
	{
		((XStringW*)(context))->strncat(buf, nbchar);
	}
public:
	void vSWPrintf(const char* format, va_list va)
	{
		setEmpty();
		vwprintf_with_callback(format, va, transmitSPrintf, this);
	}
	void SWPrintf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)))
	{
		va_list     va;

		va_start (va, format);
		vSWPrintf(format, va);
		va_end(va);
	}
};


constexpr LString8 operator "" _XS8 ( const char* s, size_t len);
constexpr LString16 operator "" _XS16 ( const char16_t* s, size_t len);
constexpr LString32 operator "" _XS32 ( const char32_t* s, size_t len);
constexpr LStringW operator "" _XSW ( const wchar_t* s, size_t len);

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

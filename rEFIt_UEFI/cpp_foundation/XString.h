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

//typedef  XStringAbstract<char> XString;

class XString : public XStringAbstract<char, XString>
{
  public:
	XString() : XStringAbstract<char, XString>() {};
	XString(const XString& S) : XStringAbstract<char, XString>(S) {}

	template<typename O, class OtherXStringClass>
	XString(const XStringAbstract<O, OtherXStringClass> &S) : XStringAbstract<char, XString>(S) {}

	XString& operator=(const XString &S) { this->XStringAbstract<char, XString>::operator=(S); return *this; }

	using XStringAbstract<char, XString>::operator =;



protected:
	static void transmitSPrintf(const char* buf, unsigned int nbchar, void* context)
	{
		((XString*)(context))->strncat(buf, nbchar);
	}
public:
	void vSPrintf(const char* format, va_list va)
	{
		setEmpty();
		vprintf_with_callback(format, va, transmitSPrintf, this);
	}
	void SPrintf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)))
	{
		va_list     va;

		va_start (va, format);
		vSPrintf(format, va);
		va_end(va);
	}

};

class XString16 : public XStringAbstract<char16_t, XString16>
{
  public:
	XString16() : XStringAbstract<char16_t, XString16>() {};
	XString16(const XString16& S) : XStringAbstract<char16_t, XString16>(S) {}

	template<typename O, class OtherXStringClass>
	XString16(const XStringAbstract<O, OtherXStringClass> &S) : XStringAbstract<char16_t, XString16>(S) {}

	XString16& operator=(const XString16 &S) { this->XStringAbstract<char16_t, XString16>::operator=(S); return *this; }

	using XStringAbstract<char16_t, XString16>::operator =;
};

class XString32 : public XStringAbstract<char32_t, XString32>
{
  public:
	XString32() : XStringAbstract<char32_t, XString32>() {};
	XString32(const XString32& S) : XStringAbstract<char32_t, XString32>(S) {}

	template<typename O, class OtherXStringClass>
	XString32(const XStringAbstract<O, OtherXStringClass> &S) : XStringAbstract<char32_t, XString32>(S) {}

	XString32& operator=(const XString32 &S) { this->XStringAbstract<char32_t, XString32>::operator=(S); return *this; }
	
	using XStringAbstract<char32_t, XString32>::operator =;
};

class XStringW : public XStringAbstract<wchar_t, XStringW>
{
  public:
	XStringW() : XStringAbstract<wchar_t, XStringW>() {};
	XStringW(const XStringW& S) : XStringAbstract<wchar_t, XStringW>(S) {}

	template<class OtherXStringClass>
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

XString operator"" _XS ( const char* s, size_t len);
XString16 operator"" _XS16 ( const char16_t* s, size_t len);
XString32 operator"" _XS32 ( const char32_t* s, size_t len);
XStringW operator"" _XSW ( const char* s, size_t len);
XStringW operator"" _XSW ( const wchar_t* s, size_t len);

extern const XString NullXString;
extern const XStringW NullXStringW;

#ifdef _MSC_VER
#   define __attribute__(x)
#endif

XString SPrintf(const char* format, ...) __attribute__((__format__ (__printf__, 1, 2)));
XStringW SWPrintf(const char* format, ...) __attribute__((__format__ (__printf__, 1, 2)));


//
//XStringAbstract SubString(const T *S, size_t pos, size_t count);
//XStringAbstract CleanCtrl(const XStringAbstract &S);

#endif

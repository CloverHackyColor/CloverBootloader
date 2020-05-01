//*************************************************************************************************
//*************************************************************************************************
//
//                                      STRING
//
// Developed by jief666, from 1997.
//
//*************************************************************************************************
//*************************************************************************************************


#if 0
#define DBG(...) printf__VA_ARGS__)
#else
#define DBG(...)
#endif

#include <Platform.h>
#include "XString.h"

#include "../../Include/Library/printf_lite.h"

//-----------------------------------------------------------------------------
//                                 Functions
//-----------------------------------------------------------------------------
//
//constexpr LString8 operator"" _XS8 ( const char* s, size_t len)
//{
////  LString8 returnValue;
////	returnValue.takeValueFromLiteral(s);
////	(void)len;
////    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
//    return LString8(s); // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
//}
//
//XString16 operator"" _XS16 ( const char16_t* s, size_t len)
//{
//  XString16 returnValue;
//	returnValue.takeValueFromLiteral(s);
//	(void)len;
//    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
//}
//
//XString32 operator"" _XS32 ( const char32_t* s, size_t len)
//{
//  XString32 returnValue;
//	returnValue.takeValueFromLiteral(s);
//	(void)len;
//    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
//}
//
//XStringW operator"" _XSW ( const wchar_t* s, size_t len)
//{
//  XStringW returnValue;
//	returnValue.takeValueFromLiteral(s);
//	(void)len;
//    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
//}

const XString8 NullXString;
const XString16 NullXString16;
const XString32 NullXString32;
const XStringW NullXStringW;


//template<class O/*, enable_if(is_char(O))*/>
//XStringW LStringW::operator + (const O* p2) { XStringW s; s.strcat(this->s()); s.strcat(p2); return s; }
//
//template<>
//XStringW LStringW::operator + (const wchar_t* p2) { XStringW s; s.strcat(this->s()); s.strcat(p2); return s; }


XString8 SPrintf(const char* format, ...)
{
  va_list     va;
  XString8 str;

  va_start (va, format);
  str.vSPrintf(format, va);
  va_end(va);

  return str;
}

XStringW SWPrintf(const char* format, ...)
{
  va_list     va;
  XStringW str;

  va_start (va, format);
  str.vSWPrintf(format, va);
  va_end(va);

  return str;
}

//XStringW SubString(const wchar_t *S, UINTN pos, UINTN count)
//{
//	if ( wcslen(S)-pos < count ) count = wcslen(S)-pos;
//	XStringW ret;
//	ret.StrnCpy(S+pos, count);
////	return ( XStringW(S+pos, count) );
//	return ret;
//}
//

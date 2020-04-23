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
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif

#include "XToolsCommon.h"
#include "XString.h"

#include "../../Include/Library/printf_lite.h"


//-----------------------------------------------------------------------------
//                                 Functions
//-----------------------------------------------------------------------------

XString operator"" _XS ( const char* s, size_t len)
{
  XString returnValue;
	returnValue.takeValueFrom(s, len);
    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
}

XString16 operator"" _XS16 ( const char16_t* s, size_t len)
{
  XString16 returnValue;
	returnValue.takeValueFrom(s, len);
    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
}

XString32 operator"" _XS32 ( const char32_t* s, size_t len)
{
  XString32 returnValue;
	returnValue.takeValueFrom(s, len);
    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
}

XStringW operator"" _XSW ( const wchar_t* s, size_t len)
{
  XStringW returnValue;
	returnValue.takeValueFrom(s, len);
    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
}

const XString NullXString;
const XStringW NullXStringW;


XString SPrintf(const char* format, ...)
{
  va_list     va;
  XString str;

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
//
//XStringW CleanCtrl(const XStringW &S)
//{
//  XStringW ReturnValue;
//  UINTN i;
//
//	for ( i=0 ; i<S.size() ; i+=1 ) {
//#if __WCHAR_MIN__ < 0
//		if ( S.wc_str()[i] >=0  &&  S.wc_str()[i] < ' ' ) ReturnValue += 'x'; /* wchar_t are signed */
//#else
//		if ( S.wc_str()[i] < ' ' ) ReturnValue += 'x'; /* wchar_t are unsigned */
//#endif
//		else ReturnValue += S.wc_str()[i];
//	}
//	return ReturnValue;
//}



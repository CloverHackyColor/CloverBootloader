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

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "XString.h"

#include "../../Include/Library/printf_lite.h"


const XString8 NullXString8;
const XString16 NullXString16;
const XString32 NullXString32;
const XStringW NullXStringW;


XString8 S8Printf(const char* format, ...)
{
  va_list     va;
  XString8 str;

  va_start (va, format);
  str.vS8Printf(format, va);
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

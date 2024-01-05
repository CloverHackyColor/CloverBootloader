/*
 *
 * Created by jief in 1997.
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */


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
  XTOOLS_VA_LIST     va;
  XString8 str;

  XTOOLS_VA_START (va, format);
  str.vS8Printf(format, va);
  XTOOLS_VA_END(va);

  return str;
}

XStringW SWPrintf(const char* format, ...)
{
  XTOOLS_VA_LIST     va;
  XStringW str;

  XTOOLS_VA_START (va, format);
  str.vSWPrintf(format, va);
  XTOOLS_VA_END(va);

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

//*************************************************************************************************
//*************************************************************************************************
//
//                                      STRING
//
// Developed by jief666, from 1997.
//
//*************************************************************************************************
//*************************************************************************************************


#if !defined(__XStringW_CPP__)
#define __XStringW_CPP__

#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif

#include "XToolsCommon.h"
#include "XStringWP.h"

#include "printf_lite.h"


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Constructor
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


XStringWP::XStringWP(const wchar_t *S)
{
	if ( !S ) {
		DebugLog(2, "XStringWP(const wchar_t *S) called with NULL. Use setEmpty()\n");
		panic();
	}
DBG("Constructor(const wchar_t *S) : %s, StrLen(S)=%d\n", S, StrLen(S));
	Init(StrLen(S));
	StrCpy(S);
}

//XStringW::XStringW(const wchar_t *S, UINTN count)
//{
//DBG("Constructor(const wchar_t *S, UINTN count) : %s, %d\n", S, count);
//	Init(count);
//	StrnCpy(S, count);
//}
//
//XStringW::XStringW(const wchar_t aChar)
//{
//DBG("Constructor(const wchar_t aChar)\n");
//	Init(1);
//	StrnCpy(&aChar, 1);
//}

XStringWP::XStringWP(const char* S)
{
DBG("Constructor(const char* S)\n");
	xsize newLen = StrLenInWChar(S, AsciiStrLen(S));
	Init(newLen);
	utf8ToWChar(m_data, m_allocatedSize+1, S, AsciiStrLen(S)); // m_size doesn't count the NULL terminator
	SetLength(newLen);
}


#endif

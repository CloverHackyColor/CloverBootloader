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

#include "../../Include/Library/printf_lite.h"


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Constructor
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


XStringWP::XStringWP(const wchar_t *S)
{
	if ( !S ) {
//		DebugLog(2, "XStringWP(const wchar_t *S) called with NULL. Use setEmpty()\n");
//		panic();
    Init(0);
  } else {
    DBG("Constructor(const wchar_t *S) : %ls, StrLen(S)=%d\n", S, StrLen(S));
    Init(wcslen(S));
    StrCpy(S);
  }
}

XStringWP::XStringWP(const char* S)
{
  DBG("Constructor(const char* S)\n");
	xsize newLen = StrLenInWChar(S);
	Init(newLen);
	utf8ToWChar(m_data, m_allocatedSize+1, S); // m_size doesn't count the NULL terminator
	SetLength(newLen);
}


#endif

////*************************************************************************************************
////*************************************************************************************************
////
////                                      STRING
////
//// Developed by jief666, from 1997.
////
////*************************************************************************************************
////*************************************************************************************************
//
//#if 0
//#define DBG(...) DebugLog(2, __VA_ARGS__)
//#else
//#define DBG(...)
//#endif
//
//#include "XToolsCommon.h"
//#include "XUINTN.h"
//
////xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//// Constructor
////xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//XStringW::XStringW()
//{
//DBG("Construteur\n");
//	Init();
//}
//
//XStringW::XStringW(const XStringW &aString)
//{
//DBG("Constructor(const XStringW &aString) : %s\n", aString.data());
//	Init(aString.length());
//	StrnCpy(aString.data(), aString.length());
//}
////
////XStringW::XStringW(const wchar_t *S)
////{
////	if ( !S ) {
////		DebugLog(2, "XStringW(const wchar_t *S) called with NULL. Use setEmpty()\n");
////		panic();
////	}
////DBG("Constructor(const wchar_t *S) : %s, StrLen(S)=%d\n", S, StrLen(S));
////	Init(StrLen(S));
////	StrCpy(S);
////}
////
////XStringW::XStringW(const wchar_t *S, UINTN count)
////{
////DBG("Constructor(const wchar_t *S, UINTN count) : %s, %d\n", S, count);
////	Init(count);
////	StrnCpy(S, count);
////}
////
////XStringW::XStringW(const wchar_t aChar)
////{
////DBG("Constructor(const wchar_t aChar)\n");
////	Init(1);
////	StrnCpy(&aChar, 1);
////}
////
////XStringW::XStringW(const char* S)
////{
////DBG("Constructor(const char* S)\n");
////	xsize newLen = StrLenInWChar(S, AsciiStrLen(S));
////	Init(newLen);
////	utf8ToWChar(m_data, m_allocatedSize+1, S, AsciiStrLen(S)); // m_size doesn't count the NULL terminator
////	SetLength(newLen);
////}
//
//const XStringW& XStringW::takeValueFrom(const wchar_t* S)
//{
//	if ( !S ) {
//		DebugLog(2, "takeValueFrom(const wchar_t* S) called with NULL. Use setEmpty()\n");
//		panic();
//	}
//	Init(StrLen(S));
//	StrCpy(S);
//	return *this;
//}
//
//const XStringW& XStringW::takeValueFrom(const char* S)
//{
//	UINTN asciiStrLen = AsciiStrLen(S);
//	xsize newLen = StrLenInWChar(S, asciiStrLen);
//	Init(newLen);
//	utf8ToWChar(m_data, m_allocatedSize+1, S, asciiStrLen); // m_size doesn't count the NULL terminator
//	SetLength(newLen);
//	return *this;
//}
//
////xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
////
////xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

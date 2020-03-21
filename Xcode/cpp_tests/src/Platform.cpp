//
//  Platform.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <wchar.h>
#include <locale.h>

#include <string>
#include <locale>
#include <codecvt>
#include <vector>

#include "../../../rEFIt_UEFI/cpp_foundation/utf8Conversion.h"

void CpuDeadLoop(void)
{
	exit(1);
}

static char *dull_replace(const char *in, const char *pattern, const char *by, char* res)
{
//    size_t outsize = strlen(in) + 1;
    // TODO maybe avoid reallocing by counting the non-overlapping occurences of pattern
//    char *res = malloc(outsize);
    // use this to iterate over the output
    size_t resoffset = 0;

    const char *needle;
    while ( (needle = strstr(in, pattern)) ) {
        // copy everything up to the pattern
        memcpy(res + resoffset, in, (size_t)(needle - in));
        resoffset += (size_t)(needle - in);

        // skip the pattern in the input-string
        in = needle + strlen(pattern);

        // adjust space for replacement
//        outsize = outsize - strlen(pattern) + strlen(by);
//        res = realloc(res, outsize);

        // copy the pattern
        memcpy(res + resoffset, by, strlen(by));
        resoffset += strlen(by);
    }

    // copy the remaining input
    strcpy(res + resoffset, in);

    return res;
}

void DebugLog(INTN DebugMode, const char *FormatString, ...)
{
	(void)DebugMode;

	char* NewFormat = (char*)alloca(strlen(FormatString)+1);
	dull_replace(FormatString, "%a", "%s", NewFormat);
	
	va_list va;
	va_start(va, FormatString);
	vprintf(NewFormat, va);
	va_end(va);
}

void PauseForKey(const wchar_t* msg)
{
	printf("%ls", msg);
	getchar();
}



UINTN StrLen(const wchar_t* String)
{
	return wchar_len(String);
}
UINTN StrLen(const char16_t* String)
{
	return char16_len(String);
}




void* AllocatePool(UINTN  AllocationSize)
{
	return (void*)malloc((size_t)AllocationSize);
}

void* AllocateZeroPool(UINTN  AllocationSize)
{
	void* p = (void*)malloc((size_t)AllocationSize);
	memset(p, 0, AllocationSize);
	return p;
}

void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer)
{
	(void)OldSize;
	if ( !OldBuffer ) return AllocatePool(NewSize);
	return (void*)realloc(OldBuffer, (size_t)NewSize);
}

void FreePool(const void* Buffer)
{
	free((void*)Buffer);
}

void ZeroMem(void *Destination, UINTN Length)
{
	memset(Destination, 0, Length);
}

void SetMem(void *Destination, UINTN Length, char c)
{
	memset(Destination, c, Length);
}

void CopyMem(void *Destination, void *Source, UINTN Length)
{
	memmove(Destination, Source, (size_t)Length);
}

//UINTN AsciiStrLen(const char* String)
//{
//	return (UINTN)strlen(String);
//}
//
//INTN AsciiStrCmp (const char *FirstString,const char *SecondString)
//{
//	return (INTN)strcmp(FirstString, SecondString);
//}
//int StrCmp(const wchar_t* FirstString, const wchar_t* SecondString)
//{
//#if __WCHAR_MAX__ > 0xFFFFu || _MSC_VER
//	int ret = wcscmp(FirstString, SecondString);
//	return ret;
//#else
//	// Looks like wcscmp doesn't work with Utf16, even if compiled with -fshort-wchar.
//	// So conversion to Utf32 needed first.
//
//	std::vector<char32_t> FirstStringUtf32;
//	std::vector<char32_t> SecondStringUtf32;
//
//	convert_utf16_to_utf32((const char16_t*)FirstString, StrLen(FirstString), &FirstStringUtf32);
//	convert_utf16_to_utf32((const char16_t*)SecondString, StrLen(SecondString), &SecondStringUtf32);
//
//	int ret = wcscmp((const wchar_t*)FirstStringUtf32.data(), (const wchar_t*)SecondStringUtf32.data());
//	return ret;
//#endif
//}
//
//int StrnCmp(const wchar_t* FirstString, const wchar_t* SecondString, UINTN Length)
//{
//#if __WCHAR_MAX__ > 0xFFFFu
//	return wcsncmp(FirstString, SecondString, Length);
//#else
//
//#ifdef _MSC_VER
//	return wcsncmp(FirstString, SecondString, (size_t)Length);
//#else
//	// Looks like wcscmp doesn't work with Utf16, even if compiled with -fshort-wchar.
//	// So conversion to Utf32 needed first.
//
//	std::vector<char32_t> FirstStringUtf32;
//	std::vector<char32_t> SecondStringUtf32;
//
//	convert_utf16_to_utf32((const char16_t*)FirstString, StrLen(FirstString), &FirstStringUtf32);
//	convert_utf16_to_utf32((const char16_t*)SecondString, StrLen(FirstString), &SecondStringUtf32);
//
//	int ret = wcsncmp((const wchar_t*)FirstStringUtf32.data(), (const wchar_t*)SecondStringUtf32.data(), Length);
////printf("wcsncmp=%d\n", ret);
//	return ret;
//#endif
//#endif
//}

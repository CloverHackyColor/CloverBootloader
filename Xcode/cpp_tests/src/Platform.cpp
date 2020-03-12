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
#include <wchar.h>
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

void DebugLog(int DebugMode, const char *FormatString, ...)
{
	(void)DebugMode;
	va_list va;
	va_start(va, FormatString);
	vprintf(FormatString, va);
	va_end(va);
}



void* AllocatePool(UINTN  AllocationSize)
{
	return (void*)malloc((size_t)AllocationSize);
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

void CopyMem(void *Destination, void *Source, UINTN Length)
{
	memmove(Destination, Source, (size_t)Length);
}

void PauseForKey(const wchar_t* msg)
{
	printf("%ls", msg);
	getchar();
}

UINTN AsciiStrLen(const char* String)
{
	return (UINTN)strlen(String);
}

INTN AsciiStrCmp (const char *FirstString,const char *SecondString)
{
	return (INTN)strcmp(FirstString, SecondString);
}

#if __WCHAR_MAX__ <= 0xFFFFu

#ifndef _MSC_VER

std::string utf16_to_utf8(const wchar_t* ws)
{
	std::u16string s((const char16_t*)ws);
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffff, std::codecvt_mode::little_endian>, char16_t> cnv;
    std::string utf8 = cnv.to_bytes(s);
//    if(cnv.converted() < s.size())
//        throw std::runtime_error("incomplete conversion");
    return utf8;
}

int is_surrogate(char16_t uc) { return (uc - 0xd800u) < 2048u; }
int is_high_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xd800; }
int is_low_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

char32_t surrogate_to_utf32(char16_t high, char16_t low) {
    return char32_t((high << 10) + low - 0x35fdc00); // Safe cast, it fits in 32 bits
}

void convert_utf16_to_utf32(const char16_t* input, size_t input_size, std::vector<char32_t>* output)
{
    const char16_t * const end = input + input_size;
    while (input < end) {
        const char16_t uc = *input++;
        if (!is_surrogate(uc)) {
            (*output).push_back(uc);
        } else {
            if (is_high_surrogate(uc) && input < end && is_low_surrogate(*input))
                (*output).push_back(surrogate_to_utf32(uc, *input++));
            else {
                // ERROR
			}
        }
    }
    (*output).push_back(0);
}
#endif

#endif


UINTN StrLen(const wchar_t* String)
{
	// wcslen seems not to work if sizeof(wchar_t) == 2
	const wchar_t* p;
	for ( p = String ; *p ; p++ );
	return (UINTN)(p-String);
}

int StrCmp(const wchar_t* FirstString, const wchar_t* SecondString)
{
#if __WCHAR_MAX__ > 0xFFFFu || _MSC_VER
	int ret = wcscmp(FirstString, SecondString);
	return ret;
#else
	// Looks like wcscmp doesn't work with Utf16, even if compiled with -fshort-wchar.
	// So conversion to Utf32 needed first.

	std::vector<char32_t> FirstStringUtf32;
	std::vector<char32_t> SecondStringUtf32;
	
	convert_utf16_to_utf32((const char16_t*)FirstString, StrLen(FirstString), &FirstStringUtf32);
	convert_utf16_to_utf32((const char16_t*)SecondString, StrLen(FirstString), &SecondStringUtf32);
	
	int ret = wcscmp((const wchar_t*)FirstStringUtf32.data(), (const wchar_t*)SecondStringUtf32.data());
	return ret;
#endif
}

int StrnCmp(const wchar_t* FirstString, const wchar_t* SecondString, UINTN Length)
{
#if __WCHAR_MAX__ > 0xFFFFu
	return wcsncmp(FirstString, SecondString, Length);
#else

#ifdef _MSC_VER
	return wcsncmp(FirstString, SecondString, (size_t)Length);
#else
	// Looks like wcscmp doesn't work with Utf16, even if compiled with -fshort-wchar.
	// So conversion to Utf32 needed first.

	std::vector<char32_t> FirstStringUtf32;
	std::vector<char32_t> SecondStringUtf32;
	
	convert_utf16_to_utf32((const char16_t*)FirstString, StrLen(FirstString), &FirstStringUtf32);
	convert_utf16_to_utf32((const char16_t*)SecondString, StrLen(FirstString), &SecondStringUtf32);
	
	int ret = wcsncmp((const wchar_t*)FirstStringUtf32.data(), (const wchar_t*)SecondStringUtf32.data(), Length);
//printf("wcsncmp=%d\n", ret);
	return ret;
#endif
#endif
}

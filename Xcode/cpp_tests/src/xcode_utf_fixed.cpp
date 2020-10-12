//
//  wfunction.hpp
//  cpp_tests
//
//  Created by jief on 15.03.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

/*
 * clang had poisoned identifier like wcslen.
 * It's not possible to define a function with that name, not even a macro... except on the command line.
 * So, for this to work, pass macro definition argument on command line : wcslen=utf16_wcslen wcsncmp=utf16_wcsncmp wcsstr=utf16_wcsstr
 */

#include "xcode_utf_fixed.h"
#include <wchar.h>
#include <vector>
#include <cwchar>

#if __WCHAR_MAX__ < 0x10000

static int is_surrogate(char16_t uc) { return (uc - 0xd800u) < 2048u; }
static int is_high_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xd800; }
static int is_low_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

static char32_t surrogate_to_utf32(char16_t high, char16_t low) {
    return char32_t((high << 10) + low - 0x35fdc00); // Safe cast, it fits in 32 bits
}

static void convert_utf16_to_utf32(const char16_t* input, size_t input_size, std::vector<char32_t>* output)
{
    const char16_t* const end = input + input_size;
    while (input < end &&  *input) {
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


size_t wcslen_fixed(const wchar_t *s)
{
#if __WCHAR_MAX__ >= 0x10000
	return wcslen(s);
#else
	// wcslen seems not to work if sizeof(wchar_t) == 2
	const wchar_t* p;
	for ( p = s ; *p ; p++ );
	return (size_t)(p-s);
#endif

}

int wcsncmp_fixed(const wchar_t *s1, const wchar_t * s2, size_t n)
{
#if __WCHAR_MAX__ >= 0x10000
	return wcsncmp(s1, s2, n);
#else
	// Looks like wcscmp doesn't work with Utf16, even if compiled with -fshort-wchar.
	// So conversion to Utf32 needed first.

	std::vector<char32_t> s1Utf32;
	std::vector<char32_t> s2Utf32;

	convert_utf16_to_utf32((const char16_t*)s1, n, &s1Utf32);
	convert_utf16_to_utf32((const char16_t*)s2, n, &s2Utf32);

	// we don't know the new value of n (x UTF16 chars is not x*2 UTF32 chars), so we can't call wcsncmp
	// but that's ok because we converted only n UTF16 chars in the call of convert_utf16_to_utf32
	int ret = wcscmp((const wchar_t*)s1Utf32.data(), (const wchar_t*)s2Utf32.data());
	return ret;
#endif
}


const wchar_t* wcsstr_fixed(const wchar_t* haystack, const wchar_t* needle)
{
#if __WCHAR_MAX__ >= 0x10000
	return wcsstr(haystack, needle);
#else
	// Looks like wcscmp doesn't work with Utf16, even if compiled with -fshort-wchar.
	// So conversion to Utf32 needed first.


    const wchar_t *a = haystack, *b = needle;
    for (;;)
        if      (!*b)          return a - wcslen_fixed(needle);
        else if (!*a)          return NULL;
        else if (*a++ != *b++) { a = ++haystack; b = needle;}

#endif
}



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
 * So, for this to work, pass macro definition argument on command line : wcslen=wcslen_fixed wcscmp=__wcscmp_is_disabled__ wcsncmp=wcsncmp_fixed wcsstr=wcsstr_fixed
 */

/*
 * 2021, Mojave.
 *   wcslen, wcsncmp seems fixed !
 *   wcsstr is NOT.
 *   printf with %ls is not.
 *   wprintf is not.
 */

#include <wchar.h>
#include <vector>
#include <cwchar>
#include <unistd.h>
#include <locale.h>
#include <locale>
#include <string>
#include <codecvt>
//#include "../../../Include/Library/printf_lite.h"
#include "../../../rEFIt_UEFI/Platform/BootLog.h"

/* few tests for debug purpose */
extern "C" void xcode_utf_fixed_tests()
{
  setlocale(LC_ALL, "en_US"); // to allow printf unicode char

  printf("sizeof(wchar_t)=%zu\n", sizeof(wchar_t));
  printf("sizeof(size_t)=%zu\n", sizeof(size_t));
  printf("sizeof(long)=%zu\n", sizeof(long));
  printf("sizeof(long long)=%zu\n", sizeof(long long));
  printf("sizeof(size_t)=%zu\n", sizeof(size_t));
  #ifndef _MSC_VER
  //printf("%zu\n", (size_t)MAX_UINT64);
  //printf("%zd\n", (size_t)MAX_UINT64);
  #endif
  printf("%lc\n", L'Ľ');

  #if PRINTF_LITE_REPLACE_STANDARD_FUNCTION == 1
    printf("%ls\n", L"Hello world1");
    char buf[50];
    snprintf(buf, 50, "%ls", L"Hello world2");
    printf("%s\n", buf);
    wprintf(L"%ls\n", L"Hello world൧楔");
  #endif
  
  uint64_t uint64 = 1;
  printf("Hello world൧楔 %llu \n", uint64);
  DebugLog(2, "Hello world൧楔 %llu \n", uint64);

  size_t len1 = wcslen(L"Hell൧楔o world൧楔");
  size_t len1f = wcslen_fixed(L"Hell൧楔o world൧楔");
  printf("len1 = %zu, len1f = %zd\n", len1, len1f);

  int cmp1 = wcsncmp(L"12楔34", L"12楔35", 4);
  int cmp1f = wcsncmp_fixed(L"12楔34", L"12楔35", 4);
  printf("cmp1 = %d, cmp1f = %d\n", cmp1, cmp1f);
  
  {
    const wchar_t* str = L"12ク34";
    const wchar_t* strstr1 = wcsstr(str, L"ク");
    const wchar_t* strstr1f = wcsstr_fixed(str, L"ク");
    printf("strstr1 = %ld, strstr1f = %ld\n", strstr1-str, strstr1f-str);
  }

  printf("\n\n\n");

//  char32_t c32 = (int)-1;
}


#include "../Include/xcode_utf_fixed.h"


#if __WCHAR_MAX__ < 0x10000

#undef wcslen
extern "C" size_t wcslen(const wchar_t *);
#undef wcscmp
extern "C" int wcscmp(const wchar_t *s1, const wchar_t * s2);
#undef wcsncmp
extern "C" int wcsncmp(const wchar_t *, const wchar_t *, size_t);
#undef wcsstr
extern "C" wchar_t *wcsstr(const wchar_t *, const wchar_t *);

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

std::string to_utf8(const char16_t* s)
{
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  return conv.to_bytes(s);
}

#else

std::string to_utf8(const char32_t* s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
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

#ifdef _LIBCPP_WCHAR_H
wchar_t* wcsstr_fixed(const wchar_t* haystack, const wchar_t* needle)
#else
const wchar_t* wcsstr_fixed(const wchar_t* haystack, const wchar_t* needle)
#endif
{
#if __WCHAR_MAX__ >= 0x10000
  return (wchar_t*)wcsstr(haystack, needle);
#else
  // Looks like wcscmp doesn't work with Utf16, even if compiled with -fshort-wchar.
  // So conversion to Utf32 needed first.


    const wchar_t *a = haystack, *b = needle;
    for (;;)
        if      (!*b)          return (wchar_t*)(a - wcslen_fixed(needle));
        else if (!*a)          return NULL;
        else if (*a++ != *b++) { a = ++haystack; b = needle;}

#endif
}

/*
 * macOS 10.15 define vsnprintf as a macro that calls __vsnprintf_chk
 */
extern "C" int __vsnprintf_chk (char* buf, size_t len, int check, size_t size, const char* format, va_list va)
{
  (void)check;
  (void)size;
  return PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnprint, PRINTF_CFUNCTION_SUFFIX)(buf, len, format, va);
}

#if PRINTF_LITE_REPLACE_STANDARD_FUNCTION == 1
extern "C" int printf(const char* format, ...)
{
  int ret;
  #if __WCHAR_MAX__ <= 0xFFFF
    va_list va;
    va_start(va, format);
    char buf[4095];
    ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnprint, PRINTF_CFUNCTION_SUFFIX)(buf, sizeof(buf)-1, format, va);
    write(1, buf, strlen(buf));
    va_end(va);
  #else
    va_list va;
    va_start(va, format);
    ret = vprintf(format, va);
    va_end(va);
  #endif
  return ret;
}
extern "C" int vprintf(const char* format, va_list va)
{
  int ret;
  #if __WCHAR_MAX__ <= 0xFFFF
    char buf[4095];
    ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnprint, PRINTF_CFUNCTION_SUFFIX)(buf, sizeof(buf)-1, format, va);
    write(1, buf, strlen(buf));
  #else
    ret = vprintf(format, va);
  #endif
  return ret;
}
#endif

//extern "C" int snprintf(char * __restrict buf, size_t len, const char * __restrict format, ...)
//{
//  int ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, snprint, PRINTF_CFUNCTION_SUFFIX)(buf, sizeof(buf)-1, format);
//  return ret;
//}

int wprintf(const wchar_t* wformat, ...)
{
  int ret;
  #if __WCHAR_MAX__ <= 0xFFFF
    std::string format = to_utf8((char16_t*)wformat);
    wchar_t wbuf[4095];
    
    va_list va;
    va_start(va, wformat);
    ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wbuf, sizeof(wbuf)-1, format.c_str(), va);
    va_end(va);
    std::string buf = to_utf8((char16_t*)wbuf);
    write(1, buf.c_str(), buf.length());
  #else
    std::string format = to_utf8((char32_t*)wformat);
    wchar_t wbuf[4095];
    va_list va;
    va_start(va, wformat);
    ret = PRINTF_FUNCTION_NAME(PRINTF_CFUNCTION_PREFIX, vsnwprint, PRINTF_CFUNCTION_SUFFIX)(wbuf, sizeof(wbuf)-1, format.c_str(), va);
    va_end(va);
    std::string buf = to_utf8((char32_t*)wbuf);
    write(1, buf.c_str(), buf.length());
  #endif
  return ret;
}

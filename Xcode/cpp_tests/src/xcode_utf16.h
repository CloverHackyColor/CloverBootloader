//
//  wfunction.hpp
//  cpp_tests
//
//  Created by jief on 15.03.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#ifndef __xcode_utf16_h__
#define __xcode_utf16_h__


#if defined(__APPLE__) && defined(__clang__) && __WCHAR_MAX__ <= 0xFFFFu

//#define wcslen utf16_wcsleny
//#define strlen utf16_wcslenx

#ifdef __cplusplus
extern "C" {
#endif

/*
 * all the functions w... seems to expect utf32 even when compiled with short-wchar
 */

size_t utf16_wcslen(const wchar_t *s);
int    utf16_wcsncmp(const wchar_t *s1, const wchar_t * s2, size_t n);

#ifdef __cplusplus
}
#endif



#endif // defined(__APPLE__) && defined(__clang__) && __WCHAR_MAX__ <= 0xFFFFu
#endif /* wfunction_hpp */

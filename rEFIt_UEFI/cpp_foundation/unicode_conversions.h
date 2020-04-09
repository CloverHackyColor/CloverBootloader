//
//  utf8Conversion.hpp
//
//  Created by jief the 24 Feb 2020.
//

#ifndef __unicode_conversions_h__
#define __unicode_conversions_h__

#include <stddef.h>
#include <wchar.h>

#ifndef __cplusplus
//typedef uint16_t wchar_t;
typedef uint32_t char32_t;
typedef uint16_t char16_t;
#endif


size_t char16_len(const char16_t* s);
size_t char32_len(const char32_t* s);
size_t wchar_len(const wchar_t* s);


char32_t get_char32_from_utf8(const char *s);
void get_char16_from_char32(char32_t char32, char16_t* char16_1, char16_t* char16_2);

size_t utf8_string_char16_count(const char *src);
size_t char16_string_from_utf8_string(char16_t* dst, size_t dst_max_len,  const char *s);
size_t utf8_string_from_char16_string(char* dst, size_t dst_max_len, const char16_t *s);

size_t utf8_string_char32_count(const char *s);
size_t char32_string_from_utf8_string(char32_t* dst, size_t dst_max_len, const char *s);
size_t utf8_string_from_char32_string(char* dst, size_t dst_max_len, const char32_t *s);


size_t utf8_string_wchar_count(const char *src);
size_t wchar_string_from_utf8_string(wchar_t* dst, size_t dst_max_len,  const char *s);
size_t utf8_string_from_wchar_string(char* dst, size_t dst_max_len, const wchar_t* s);


size_t char16_string_char32_count(const char16_t *s);
size_t utf32_string_to_char16_string(char32_t* dst, size_t dst_max_len, const char16_t *s);
size_t utf16_string_to_char32_string(char16_t* dst, size_t dst_max_len, const char32_t *s);




#endif /* utf816Conversion_hpp */

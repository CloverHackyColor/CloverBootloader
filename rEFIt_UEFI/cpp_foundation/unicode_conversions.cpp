//
//  utf8Conversion.hpp
//
//  Created by jief the 24 Feb 2020.
//

#include "unicode_conversions.h"

#include <string.h> // for memcpy

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif


#if __WCHAR_MAX__ <= 0xFFFFu
    #define wchar_cast char16_t
#else
    #define wchar_cast char32_t
#endif

#ifndef wchar_cast
#error wchar_cast
#endif


//
//size_t char32_len_from_wchar(const wchar_t* s)
//{
//#if __WCHAR_MAX__ <= 0xFFFFu
//	return char32_len((const char16_t*)s);
//#else
//	return char32_len((const char32_t*)s);
//#endif
//}
//
//size_t wchar_len(const wchar_t* s)
//{
//#if __WCHAR_MAX__ <= 0xFFFFu
//	return char16_len((const char16_t*)s);
//#else
//	return char32_len((const char32_t*)s);
//#endif
//}


static inline int is_surrogate(char16_t uc) { return (uc - 0xd800u) < 2048u; }
static inline int is_high_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xd800; }
static inline int is_low_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

static inline char32_t surrogate_to_utf32(char16_t high, char16_t low) {
    return char32_t((high << 10) + low - 0x35fdc00); // Safe cast, it fits in 32 bits
}


#define halfBase 0x0010000UL
#define halfMask 0x3FFUL
#define halfShift 10 /* used for shifting by 10 bits */
#define UNI_SUR_HIGH_START  0xD800u
#define UNI_SUR_LOW_START   0xDC00u


/*************************************************************   Utility   *********************************************************/

/*
 * Size of an UTF32 char when represented in UTF8
 * Return value : size
 */
size_t utf8_size_of_utf32_char(char32_t c) {
	if ( c == 0 ) return 0;
	else if ( c <= 0x7f ) return 1;
	else if ( c <= 0x7ff ) return 2;
	else if ( c <= 0xFFFF ) return 3;
	else return 4;
}

/*
 * Size in bytes of an utf32 string if it were converted to utf8
 * Return value : pointer to the end of string or at the error
 */
size_t utf8_size_of_utf32_string(const char32_t* s)
{
	if ( !s ) return 0;
	size_t size = 0;
	while ( *s ) s = utf8_size_of_utf32_char_ptr(s, &size);
	return size;
}

/*
 * Increment size and return a pointer to the next char
 * Return value : pointer to the end of string or at the error
 */
const char32_t* utf8_size_of_utf32_char_ptr(const char32_t *s, size_t* size)
{
	if ( *s == 0 ) return s;
	*size += utf8_size_of_utf32_char(*s++);
	return s;
}

/*
 * Store an utf32 char in dst, if there is enough room (dst_max_size is >= size of converted utf32 char)
 * If there is enough room, dst_max_size is decrement and dst is increment and returned
 * If there isn't enough room, dst_max_size is set to 0 and dst is returned
 */
char* get_utf8_from_char32(char* dst, size_t* dst_max_size, char32_t utf32_char)
{
#ifdef JIEF_DEBUG
	char* dst_debug = dst;
	(void)dst_debug;
#endif
	if ( *dst_max_size <= 0 ) return dst;
	/* assertion: utf32_char is a single UTF-4 value */

	int bits = 0; // just to silence the warning

	if (utf32_char < 0x80) {
		*dst++ = (char)utf32_char;
		*dst_max_size -= 1;
		bits = -6;
	}
	else if (utf32_char < 0x800) {
		if ( *dst_max_size < 2 ) {
			*dst_max_size = 0;
			return dst;
		}
		*dst++ = (char)(((utf32_char >> 6) & 0x1F) | 0xC0);
		*dst_max_size -= 1;
		bits = 0;
	}
	else if (utf32_char < 0x10000) {
		if ( *dst_max_size < 3 ) {
			*dst_max_size = 0;
			return dst;
		}
		*dst++ = (char)(((utf32_char >> 12) & 0x0F) | 0xE0);
		*dst_max_size -= 1;
		bits = 6;
	}
	else {
		if ( *dst_max_size < 4 ) {
			*dst_max_size = 0;
			return dst;
		}
		*dst++ = (char)(((utf32_char >> 18) & 0x07) | 0xF0);
		*dst_max_size -= 1;
		bits = 12;
	}
	for (  ; /* *dst_max_size > 0 && */  bits >= 0  ;  bits -= 6  ) { // no need to check dst_max_size, it's made before
		*dst++ = (char)(((utf32_char >> bits) & 0x3F) | 0x80);
		*dst_max_size -= 1;
	}
#ifdef JIEF_DEBUG
	if ( *dst_max_size > 0 ) *dst = 0;
#endif
	return dst;
}


/*
 * Store an utf32 char in dst, if there is enough room (dst_max_size is >= size of converted utf32 char)
 * If there is enough room, dst_max_size is decrement and dst is increment and returned
 * If there isn't enough room, dst_max_size is set to 0 and dst is returned
 */
char16_t* get_utf16_from_char32(char16_t* dst, size_t* dst_max_size, char32_t utf32_char)
{
	if ( *dst_max_size <= 0 ) return dst;
	
	char16_t char16_1, char16_2;
	get_char16_from_char32(utf32_char, &char16_1, &char16_2);
	if ( char16_2 != 0 ) {
		if ( *dst_max_size < 2 ) {
			*dst_max_size = 0;
		}else{
			*dst++ = char16_1;
			*dst++ = char16_2;
			*dst_max_size -= 2;
		}
	}else{
		if ( *dst_max_size < 1 ) {
			*dst_max_size = 0;
		}else{
			*dst++ = char16_1;
			*dst_max_size -= 1;
		}
	}
	return dst;
}

/*
 * Store an utf32 char in dst, if there is enough room (dst_max_size is >= size of converted utf32 char)
 * If there is enough room, dst_max_size is decrement and dst is increment and returned
 * If there isn't enough room, dst_max_size is set to 0 and dst is returned
 */
char32_t* get_utf32_from_char32(char32_t* dst, size_t* dst_max_size, char32_t utf32_char)
{
	if ( *dst_max_size <= 0 ) return dst;
	*dst = utf32_char;
	*dst_max_size -= 1;
	return dst + 1;
}

/*
Number		Bits for		First	Last			Byte 1		Byte 2		Byte 3		cByte 4
of bytes	code point
1			7				U+0000	U+007F			0xxxxxxx
2			11				U+0080	U+07FF			110xxxxx	10xxxxxx
3			16				U+0800	U+FFFF			1110xxxx	10xxxxxx	10xxxxxx
4			21				U+10000	U+10FFFF[12]	11110xxx	10xxxxxx	10xxxxxx	10xxxxxx
*/

/*
 * char32 will be set to 0 at the end of string or at error
 * Return value : pointer to the end of string or at the error
 */
const char* get_char32_from_utf8_string(const char* s, char32_t* char32)
{
	if ( !*s ) {
		*char32 = 0;
		return s;
	}
	char32_t c;
	if (*s & 0x80) {
		if ((*(s+1) & 0xc0) != 0x80) {  // 0xc0 = 0b11000000. Equivalent to if ( *(s+1) != 0x10xxxxxx )
			// Finished in the middle of an utf8 multibyte char
			*char32 = 0;
			return s;
		}
		if ((*s & 0xe0) == 0xe0) { // 0xe0 == 0b11100000. Equivalent to if ( *(s) == 0x111xxxxx )
		    // Here, it's a 3 or 4 bytes
			// Byte 3 has to be 0x10xxxxxx
			if ((*(s+2) & 0xc0) != 0x80) {  // 0xc0 = 0b11000000. Equivalent to if ( *(s+2) != 0x10xxxxxx )
				// Finished in the middle of an utf8 multibyte char
				*char32 = 0;
				return s;
			}
			if ((*s & 0xf0) == 0xf0) { // 0xf0 = 0b1111xxxx. Equivalent to if ( *(s) == 0x1111xxxx )
				// Here, it's a 4 bytes
				// Byte 4 has to be 0x10xxxxxx
				if ((*s & 0xf8) != 0xf0 || (*(s+3) & 0xc0) != 0x80) { // 0xf8 = 0b11111xxx. Equivalent to if ( *(s) != 0x11110xxx || *(s+3) != 0x10xxxxxx )
					// Finished in the middle of an utf8 multibyte char
					*char32 = 0;
					return s;
				}
				/* 4-byte code */
				c = char32_t((*s & 0x7) << 18); // & result type is int. We know it fits in 32 bits. Safe to cast to char32_t
				c |= char32_t((*(s+1) & 0x3f) << 12);
				c |= char32_t((*(s+2) & 0x3f) << 6);
				c |= *(s+3) & 0x3f;
				s += 4;
			} else {
				/* 3-byte code */
				c = char32_t((*s & 0xf) << 12);
				c |= char32_t((*(s+1) & 0x3f) << 6);
				c |= *(s+2) & 0x3f;
				s += 3;
			}
		} else {
			/* 2-byte code */
			c = char32_t((*s & 0x1f) << 6);
			c |= *(s+1) & 0x3f;
			s += 2;
		}
	} else {
		/* 1-byte code */
		c = (unsigned char)(*s); // in case we compiled with signed char
		s += 1;
	}
	*char32 = c;
	return s;
}

/*
 * get nth char32 of an utf8 string
 * Return value : pointer to the end of string or at the error
 */
char32_t get_char32_from_utf8_string_at_pos(const char* s, size_t pos)
{
	if ( !s ) return 0;
	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32 && pos > 0 ) {
		s = get_char32_from_utf8_string(s, &char32);
		pos--;
	}
	return char32;
}


/*************************************************************   utf8 - char32   *********************************************************/

//const char* utf8_move_forward(const char* s)
//{
//}

/*
 * Size in bytes of an utf32 string if it were converted to utf8
 * Return value : pointer to the end of string or at the error
 */
size_t utf32_size_of_utf8_string(const char* s)
{
	if ( !s ) return 0;
	size_t size = 0;
	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32 ) {
		size += 1;
		s = get_char32_from_utf8_string(s, &char32);
	}
	return size;
}

/*
 * Size in bytes of an utf32 string of len char if it were converted to utf8
 * Return value : pointer to the end of string or at the error
 */
size_t utf8_size_of_utf32_string_len(const char32_t* s, size_t len)
{
	if ( !s  ||  len <= 0 ) return 0; // <= in case size_t is signed
	size_t size = 0;
	while ( *s  &&  len > 0 ) {
		s = utf8_size_of_utf32_char_ptr(s, &size);
		len --;
	}
	return size;
}

size_t utf32_string_from_utf8_string(char32_t* dst, size_t dst_max_size, const char* s)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s ) {
		*dst = 0;
		return 0;
	}
	char32_t* p = dst;
	char32_t* p_max = dst + dst_max_size - 1;

	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32 != 0   &&  p < p_max ) {
		*p++ = char32;
		s = get_char32_from_utf8_string(s, &char32);
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf32_size_of_utf8_string_len(const char* s, size_t len)
{
	if ( !s  ||  len <= 0 ) return 0; // <= in case size_t is signed
	size_t size = 0;
	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32  &&  len > 0 ) {
		size += 1;
		s = get_char32_from_utf8_string(s, &char32);
		len --;
	}
	return size;
}

size_t utf32_string_from_utf8_string_len(char32_t* dst, size_t dst_max_size, const char* s, size_t len)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s  || len <= 0 ) {
		*dst = 0;
		return 0;
	}
	char32_t* p = dst;
	char32_t* p_max = dst + dst_max_size - 1;

	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32 != 0   &&  p < p_max  && len > 0 ) {
		*p++ = char32;
		s = get_char32_from_utf8_string(s, &char32);
		len--;
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf8_string_from_utf32_string(char* dst, size_t dst_max_size, const char32_t *s)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s ) {
		*dst = 0;
		return 0;
	}
	dst_max_size -= 1;
	char* p = dst;
	while ( *s  &&  dst_max_size > 0 ) {
		p = get_utf8_from_char32(p, &dst_max_size, *s++);
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf8_string_from_utf32_string_len(char* dst, size_t dst_max_size, const char32_t *s, size_t len)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s  || len <= 0 ) {
		*dst = 0;
		return 0;
	}
	dst_max_size -= 1;
	char* p = dst;
	while ( *s  &&  dst_max_size > 0 && len > 0 ) {
		p = get_utf8_from_char32(p, &dst_max_size, *s++);
		len--;
	}
	*p = 0;
	return (size_t)(p-dst);
}


/*************************************************************   utf8 - char16   *********************************************************/

//static size_t utf16_size_of_utf32_char(char32_t c);

/*
 * Increment size
 * Return value : pointer to the end of string or at the error
 */
const char16_t* utf8_size_of_utf16_char_ptr(const char16_t *s, size_t* size) {
	char32_t c;
	s = get_char32_from_utf16_string(s, &c);
	if ( c == 0 ) return s;
	*size += utf8_size_of_utf32_char(c);
	return s;
}

/*
 * Size in bytes of an utf16 string if it were converted to utf8
 * Return value : pointer to the end of string or at the error
 */
size_t utf8_size_of_utf16_string(const char16_t* s)
{
	if ( !s ) return 0;
	size_t size = 0;
	while ( *s ) s = utf8_size_of_utf16_char_ptr(s, &size);
	return size;
}

/*
 * Size in bytes of an utf16 string of len char if it were converted to utf8
 * Return value : pointer to the end of string or at the error
 */
size_t utf8_size_of_utf16_string_len(const char16_t* s, size_t len)
{
	if ( !s  ||  len <= 0 ) return 0; // <= in case size_t is signed
	size_t size = 0;
	while ( *s  &&  len > 0 ) {
		s = utf8_size_of_utf16_char_ptr(s, &size);
		len --;
	}
	return size;
}

size_t utf16_size_of_utf8_string(const char* s)
{
	if ( !s ) return 0;
	size_t size = 0;

	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32 ) {
		size += utf16_size_of_utf32_char(char32);
		s = get_char32_from_utf8_string(s, &char32);
	}
	return size;
}

size_t utf16_size_of_utf8_string_len(const char* s, size_t len)
{
	if ( !s  ||  len <= 0 ) return 0; // <= in case size_t is signed
	size_t size = 0;

	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32  &&  len > 0 ) {
		size += utf16_size_of_utf32_char(char32);
		len --;
		s = get_char32_from_utf8_string(s, &char32);
	}
	return size;
}



size_t utf8_string_from_utf16_string(char* dst, size_t dst_max_size, const char16_t *s)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s ) {
		*dst = 0;
		return 0;
	}
	char* p = dst;
	dst_max_size -= 1;
	while ( *s  &&  dst_max_size > 0 ) {
		char32_t utf32_char;
		s = get_char32_from_utf16_string(s, &utf32_char);
		p = get_utf8_from_char32(p, &dst_max_size, utf32_char);
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf8_string_from_utf16_string_len(char* dst, size_t dst_max_size, const char16_t *s, size_t len)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s  || len <= 0 ) {
		*dst = 0;
		return 0;
	}
	char* p = dst;
	dst_max_size -= 1;
	while ( *s  &&  dst_max_size > 0  && len > 0 ) {
		char32_t utf32_char;
		s = get_char32_from_utf16_string(s, &utf32_char);
		p = get_utf8_from_char32(p, &dst_max_size, utf32_char);
		len--;
	}
	*p = 0;
	return (size_t)(p-dst);
}


size_t utf16_string_from_utf8_string(char16_t* dst, size_t dst_max_size, const char* s)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s ) {
		*dst = 0;
		return 0;
	}
	dst_max_size -= 1;

//	size_t dst_len = 0;
	char16_t* p = dst;
//	char16_t* p_max = dst + dst_max_size;

	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32  &&  dst_max_size > 0 ) {
		p = get_utf16_from_char32(p, &dst_max_size, char32);
		s = get_char32_from_utf8_string(s, &char32);
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf16_string_from_utf8_string_len(char16_t* dst, size_t dst_max_size, const char* s, size_t len)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s  || len <= 0 ) {
		*dst = 0;
		return 0;
	}
	dst_max_size -= 1;

//	size_t dst_len = 0;
	char16_t* p = dst;
//	char16_t* p_max = dst + dst_max_size;

	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32  &&  dst_max_size > 0  && len > 0 ) {
		p = get_utf16_from_char32(p, &dst_max_size, char32);
		s = get_char32_from_utf8_string(s, &char32);
		len--;
	}
	*p = 0;
	return (size_t)(p-dst);
}


/*************************************************************   utf16 - utf32   *********************************************************/

size_t utf16_size_of_utf32_char(char32_t c)
{
	if ( c <= 0xFFFF) return 1;
	else return 2;
}

void get_char16_from_char32(char32_t char32, char16_t* char16_1, char16_t* char16_2)
{
	if ( char32 <= 0xFFFF) {
		*char16_1 = (char16_t)char32;
		*char16_2 = 0;
	} else {
		char32 -= halfBase;
		*char16_1 = (char16_t)((char32 >> halfShift) + UNI_SUR_HIGH_START);
		*char16_2 = (char16_t)((char32 & halfMask) + UNI_SUR_LOW_START);
	}
}

char32_t get_char32_from_char16(char16_t char16_1, char16_t char16_2)
{
	if (!is_surrogate(char16_1)) {
		return char16_1;
	} else {
		if (is_high_surrogate(char16_1) && is_low_surrogate(char16_2)) {
			return surrogate_to_utf32(char16_1, char16_2);
		} else {
			return 0;
		}
	}
}

/*
 * char32 will be set to 0 at the end of string or at error
 * Return value : pointer to the end of string or at the error
 */
const char16_t* get_char32_from_utf16_string(const char16_t* s, char32_t* char32)
{
	const char16_t char16_1 = *s;
	if ( char16_1 == 0 ) {
		*char32 = 0;
		return s;
	}
	s++;
	if (!is_surrogate(char16_1)) {
		*char32 = char16_1;
		return s;
	} else {
		if (is_high_surrogate(char16_1) && is_low_surrogate(*s)) {
			*char32 = surrogate_to_utf32(char16_1, *s++);
			return s;
		} else {
			*char32 = 0;
			if ( !is_high_surrogate(char16_1) ) return s-1;
			return s;
		}
	}
}




size_t utf16_size_of_utf32_string(const char32_t *s)
{
	if ( !s ) return 0;
	size_t size = 0;
	while ( *s ) size += utf16_size_of_utf32_char(*s++);
	return size;
}

size_t utf16_size_of_utf32_string_len(const char32_t *s, size_t len)
{
	if ( !s ) return 0;
	size_t size = 0;
	while ( *s  &&  len > 0 ) {
		size += utf16_size_of_utf32_char(*s++);
		len--;
	}
	return size;
}

size_t utf32_size_of_utf16_string(const char16_t *s)
{
	if ( !s ) return 0;
	size_t size = 0;
	char32_t char32;
	s = get_char32_from_utf16_string(s, &char32);
	while ( char32 ) {
		size += 1;
		s = get_char32_from_utf16_string(s, &char32);
	}
	return size;
}

size_t utf32_size_of_utf16_string_len(const char16_t *s, size_t len)
{
	if ( !s  ||  len <= 0 ) return 0; // <= in case size_t is signed
	size_t size = 0;
	char32_t char32;
	s = get_char32_from_utf16_string(s, &char32);
	while ( char32  &&  len > 0 ) {
		size += 1;
		s = get_char32_from_utf16_string(s, &char32);
		len --;
	}
	return size;
}


size_t utf16_string_from_utf32_string(char16_t* dst, size_t dst_max_size, const char32_t *s)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s ) {
		*dst = 0;
		return 0;
	}
	char16_t* p = dst;
//	char16_t* p_max = dst + dst_max_size - 1;

	while ( *s  &&  dst_max_size > 0 ) {
		p = get_utf16_from_char32(p, &dst_max_size, *s++);
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf16_string_from_utf32_string_len(char16_t* dst, size_t dst_max_size, const char32_t *s, size_t len)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s  || len <= 0 ) {
		*dst = 0;
		return 0;
	}
	char16_t* p = dst;
//	char16_t* p_max = dst + dst_max_size - 1;

	dst_max_size--;
	while ( *s  &&  dst_max_size > 0  &&  len > 0 ) {
		p = get_utf16_from_char32(p, &dst_max_size, *s++);
		len--;
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf32_string_from_utf16_string(char32_t* dst, size_t dst_max_size, const char16_t *s)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s ) {
		*dst = 0;
		return 0;
	}
	char32_t* p = dst;
	char32_t* p_max = dst + dst_max_size - 1;

	char32_t c;
	while ( *s  &&  p < p_max ) {
		s = get_char32_from_utf16_string(s, &c);
		if ( c == 0 ) return (size_t)(p-dst);
		*p++ = c;
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf32_string_from_utf16_string_len(char32_t* dst, size_t dst_max_size, const char16_t *s, size_t len)
{
	if ( dst_max_size <= 0 ) return 0;
	if ( !s  || len <= 0 ) {
		*dst = 0;
		return 0;
	}
	char32_t* p = dst;
	char32_t* p_max = dst + dst_max_size - 1;

	char32_t c;
	while ( *s  &&  p < p_max && len > 0 ) {
		s = get_char32_from_utf16_string(s, &c);
		if ( c == 0 ) return (size_t)(p-dst);
		*p++ = c;
		len--;
	}
	*p = 0;
	return (size_t)(p-dst);
}




/*************************************************************   utf8 - wchar_t   *********************************************************/

size_t utf8_size_of_wchar_string(const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf8_size_of_utf16_string((wchar_cast*)s);
#else
	return utf8_size_of_utf32_string((wchar_cast*)s);
#endif
}

size_t utf8_size_of_wchar_string_len(const wchar_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf8_size_of_utf16_string_len((wchar_cast*)s, len);
#else
	return utf8_size_of_utf32_string_len((wchar_cast*)s, len);
#endif
}

size_t wchar_size_of_utf8_string(const char* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf8_string(s);
#else
	return utf32_size_of_utf8_string(s);
#endif
}

size_t wchar_size_of_utf8_string_len(const char* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf8_string_len(s, len);
#else
	return utf32_size_of_utf8_string_len(s, len);
#endif
}

size_t utf8_string_from_wchar_string(char* dst, size_t dst_max_size, const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf8_string_from_utf16_string(dst, dst_max_size, (char16_t*)s);
#else
	return utf8_string_from_utf32_string(dst, dst_max_size, (char32_t*)s);
#endif
}

size_t utf8_string_from_wchar_string_len(char* dst, size_t dst_max_size, const wchar_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf8_string_from_utf16_string_len(dst, dst_max_size, (char16_t*)s, len);
#else
	return utf8_string_from_utf32_string_len(dst, dst_max_size, (char32_t*)s, len);
#endif
}

size_t wchar_string_from_utf8_string(wchar_t* dst, size_t dst_max_size, const char* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf8_string((char16_t*)dst, dst_max_size, s);
#else
	return utf32_string_from_utf8_string((char32_t*)dst, dst_max_size, s);
#endif
}

size_t wchar_string_from_utf8_string_len(wchar_t* dst, size_t dst_max_size, const char* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf8_string_len((char16_t*)dst, dst_max_size, s, len);
#else
	return utf32_string_from_utf8_string_len((char32_t*)dst, dst_max_size, s, len);
#endif
}


/*************************************************************   utf16 - wchar_t   *********************************************************/

size_t utf16_size_of_wchar_string(const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf16_string((wchar_cast*)s);
#else
	return utf16_size_of_utf32_string((wchar_cast*)s);
#endif
}

size_t utf16_size_of_wchar_string_len(const wchar_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf16_string_len((wchar_cast*)s, len);
#else
	return utf16_size_of_utf32_string_len((wchar_cast*)s, len);
#endif
}

size_t wchar_size_of_utf16_string(const char16_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf16_string(s);
#else
	return utf32_size_of_utf16_string(s);
#endif
}

size_t wchar_size_of_utf16_string_len(const char16_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf16_string_len(s, len);
#else
	return utf32_size_of_utf16_string_len(s, len);
#endif
}

size_t utf16_string_from_wchar_string(char16_t* dst, size_t dst_max_size, const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf16_string(dst, dst_max_size, (char16_t*)s);
#else
	return utf16_string_from_utf32_string(dst, dst_max_size, (char32_t*)s);
#endif
}

size_t utf16_string_from_wchar_string_len(char16_t* dst, size_t dst_max_size, const wchar_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf16_string_len(dst, dst_max_size, (char16_t*)s, len);
#else
	return utf16_string_from_utf32_string_len(dst, dst_max_size, (char32_t*)s, len);
#endif
}

size_t wchar_string_from_utf16_string(wchar_t* dst, size_t dst_max_size, const char16_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf16_string((char16_t*)dst, dst_max_size, s);
#else
	return utf32_string_from_utf16_string((char32_t*)dst, dst_max_size, s);
#endif
}

size_t wchar_string_from_utf16_string_len(wchar_t* dst, size_t dst_max_size, const char16_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf16_string_len((char16_t*)dst, dst_max_size, s, len);
#else
	return utf32_string_from_utf16_string_len((char32_t*)dst, dst_max_size, s, len);
#endif
}

/*************************************************************   utf32 - wchar_t   *********************************************************/


size_t utf32_size_of_wchar_string(const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf32_size_of_utf16_string((wchar_cast*)s);
#else
	return utf32_size_of_utf32_string((wchar_cast*)s);
#endif
}

size_t utf32_size_of_wchar_string_len(const wchar_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf32_size_of_utf16_string_len((wchar_cast*)s, len);
#else
	return utf32_size_of_utf32_string_len((wchar_cast*)s, len);
#endif
}


size_t wchar_size_of_utf32_string(const char32_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf32_string(s);
#else
	return utf32_size_of_utf32_string(s);
#endif
}

size_t wchar_size_of_utf32_string_len(const char32_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf32_string_len(s, len);
#else
	return utf32_size_of_utf32_string_len(s, len);
#endif
}

size_t utf32_string_from_wchar_string(char32_t* dst, size_t dst_max_size, const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf32_string_from_utf16_string(dst, dst_max_size, (char16_t*)s);
#else
	return utf32_string_from_utf32_string(dst, dst_max_size, (char32_t*)s);
#endif
}

size_t wchar_string_from_utf32_string(wchar_t* dst, size_t dst_max_size, const char32_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf32_string((char16_t*)dst, dst_max_size, s);
#else
	return utf32_string_from_utf32_string((char32_t*)dst, dst_max_size, s);
#endif
}






/*************************************************************   no conversion   *********************************************************/

// Not efficient. Could be map to the ones provided by operating system
size_t utf8_size_of_utf8_string(const char* s)
{
	if ( !s ) return 0;
	char32_t char32 = 1;
	const char* p = s; // = get_char32_from_utf8_string(s, &char32);
	while ( char32 ) {
		p = get_char32_from_utf8_string(p, &char32);
	}
	return (uintptr_t(p)-uintptr_t(s));
//
//	const char* p = s;
//	while ( *p++ );
//	return (size_t)(p-s-1);
}

size_t utf8_size_of_utf8_string_len(const char* s, size_t len)
{
	if ( !s  ||  len <= 0 ) return 0;
	char32_t char32 = 1;
	const char* p = s; // = get_char32_from_utf8_string(s, &char32);
	while ( char32  && len > 0 ) {
		p = get_char32_from_utf8_string(p, &char32);
		len -= 1;
	}
	return (size_t)(p-s);// p-s is in number of char32_t, not bytes. Careful, uintptr_t(p)-uintptr_t(s) would be in bytes
}

size_t utf16_size_of_utf16_string(const char16_t* s)
{
	if ( !s ) return 0;
	char32_t char32 = 1;
	const char16_t* p = s; // = get_char32_from_utf8_string(s, &char32);
	while ( char32 ) {
		p = get_char32_from_utf16_string(p, &char32);
	}
	return (size_t)(p-s);// p-s is in number of char32_t, not bytes. Careful, uintptr_t(p)-uintptr_t(s) would be in bytes
//	const char16_t* p = s;
//	while ( *p++ );
//	return (size_t)(p-s-1);
}

size_t utf16_size_of_utf16_string_len(const char16_t* s, size_t len)
{
	if ( !s ) return 0;
	size_t size = 0;
	char32_t char32;
	s = get_char32_from_utf16_string(s, &char32);
	while ( char32  && len > 0 ) {
		size += utf16_size_of_utf32_char(char32);
		s = get_char32_from_utf16_string(s, &char32);
		len -= 1;
	}
	return size;
}

size_t utf32_size_of_utf32_string(const char32_t* s)
{
	const char32_t* p = s;
	while ( *p++ );
	return ((size_t)(p-s-1)); // p-s is in number of char32_t, not bytes. // Let's hope that p-s-1 is not > MAX_SIZET
}

size_t utf32_size_of_utf32_string_len(const char32_t* s, size_t len)
{
	const char32_t* p = s;
	while ( *p++  &&  len > 0 ) len -= 1;
	return ((size_t)(p-s-1)); // p-s is in number of char32_t, not bytes. // Let's hope that p-s-1 is not > MAX_SIZET
}

size_t wchar_size_of_wchar_string(const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf16_string((char16_t*)s);
#else
	return utf32_size_of_utf32_string((char32_t*)s);
#endif
}

size_t wchar_size_of_wchar_string_len(const wchar_t* s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_size_of_utf16_string_len((char16_t*)s, len);
#else
	return utf32_size_of_utf32_string_len((char32_t*)s, len);
#endif
}



size_t utf8_string_from_utf8_string(char* dst, size_t dst_max_size,  const char *s)
{
	if ( !s  ||  dst_max_size <= 1 ) {
		if ( dst_max_size > 0 ) *dst = 0;
		return 0;
	}
	dst_max_size -= 1;
	char* p = dst;
	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32 && dst_max_size > 0 ) {
		p = get_utf8_from_char32(p, &dst_max_size, char32);
		s = get_char32_from_utf8_string(s, &char32);
	}
	*p = 0;
	return uintptr_t(p)-uintptr_t(dst)-1;
}

size_t utf8_string_from_utf8_string_len(char* dst, size_t dst_max_size,  const char *s, size_t len)
{
	if ( !s  || len <= 0 || dst_max_size <= 1 ) {
		if ( dst_max_size > 0 ) *dst = 0;
		return 0;
	}
	dst_max_size -= 1;
	char* p = dst;
	char32_t char32;
	s = get_char32_from_utf8_string(s, &char32);
	while ( char32 && dst_max_size > 0  && len > 0) {
		p = get_utf8_from_char32(p, &dst_max_size, char32);
		s = get_char32_from_utf8_string(s, &char32);
		len--;
	}
	*p = 0;
	return uintptr_t(p)-uintptr_t(dst)-1;
}

size_t utf16_string_from_utf16_string(char16_t* dst, size_t dst_max_size,  const char16_t *s)
{
	if ( !s  ||  dst_max_size <= 1 ) {
		if ( dst_max_size > 0 ) *dst = 0;
		return 0;
	}
	dst_max_size -= 1;
	char16_t* p = dst;
	char32_t char32;
	s = get_char32_from_utf16_string(s, &char32);
	while ( char32 && dst_max_size > 0 ) {
		p = get_utf16_from_char32(p, &dst_max_size, char32);
		s = get_char32_from_utf16_string(s, &char32);
	}
	*p = 0;
	return uintptr_t(p)-uintptr_t(dst)-1;
//	size_t s_len = utf16_size_of_utf16_string(s);
//	if ( dst_max_size >  s_len ) dst_max_size = s_len;
//	else dst_max_size -= 1;
//	memcpy((void*)dst, (void*)s, dst_max_size * sizeof(char16_t));
//	dst[dst_max_size] = 0;
//	return dst_max_size * sizeof(char16_t);
}

size_t utf16_string_from_utf16_string_len(char16_t* dst, size_t dst_max_size,  const char16_t *s, size_t len)
{
	if ( !s  || len <= 0 || dst_max_size <= 1 ) {
		if ( dst_max_size > 0 ) *dst = 0;
		return 0;
	}
	dst_max_size -= 1;
	char16_t* p = dst;
	char32_t char32;
	s = get_char32_from_utf16_string(s, &char32);
	while ( char32 && dst_max_size > 0  && len > 0) {
		p = get_utf16_from_char32(p, &dst_max_size, char32);
		s = get_char32_from_utf16_string(s, &char32);
		len--;
	}
	*p = 0;
	return uintptr_t(p)-uintptr_t(dst)-1;
}

size_t utf32_string_from_utf32_string(char32_t* dst, size_t dst_max_size,  const char32_t *s)
{
	if ( !s  ||  dst_max_size <= 1 ) {
		if ( dst_max_size > 0 ) *dst = 0;
		return 0;
	}
	size_t s_len = utf32_size_of_utf32_string(s);
	if ( dst_max_size >  s_len ) dst_max_size = s_len;
	else dst_max_size -= 1;
	memcpy((void*)dst, (void*)s, dst_max_size * sizeof(char32_t));
	dst[dst_max_size] = 0;
	return dst_max_size * sizeof(char32_t);
}

size_t utf32_string_from_utf32_string_len(char32_t* dst, size_t dst_max_size,  const char32_t *s, size_t len)
{
	if ( !s  || len <= 0 || dst_max_size <= 1 ) {
		if ( dst_max_size > 0 ) *dst = 0;
		return 0;
	}
	size_t s_len = utf32_size_of_utf32_string_len(s, len);
	if ( dst_max_size >  s_len ) dst_max_size = s_len;
	else dst_max_size -= 1;
	memcpy((void*)dst, (void*)s, dst_max_size * sizeof(char32_t));
	dst[dst_max_size] = 0;
	return dst_max_size * sizeof(char32_t);
}

size_t wchar_string_from_wchar_string(wchar_t* dst, size_t dst_max_size, const wchar_t *s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf16_string((char16_t*)dst, dst_max_size, (char16_t*)s);
#else
	return utf32_string_from_utf32_string((char32_t*)dst, dst_max_size, (char32_t*)s);
#endif
}

size_t wchar_string_from_wchar_string_len(wchar_t* dst, size_t dst_max_size, const wchar_t *s, size_t len)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf16_string_from_utf16_string_len((char16_t*)dst, dst_max_size, (char16_t*)s, len);
#else
	return utf32_string_from_utf32_string_len((char32_t*)dst, dst_max_size, (char32_t*)s, len);
#endif
}


/******   convenience   *****/

size_t length_of_wchar_string(const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return length_of_utf16_string((char16_t*)s);
#else
	return length_of_utf32_string((char32_t*)s);
#endif
}


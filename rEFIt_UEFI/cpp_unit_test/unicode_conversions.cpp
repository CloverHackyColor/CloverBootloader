//
//  utf8Conversion.hpp
//
//  Created by jief the 24 Feb 2020.
//

#include "unicode_conversions.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif



size_t char16_len(const char16_t* s)
{
	const char16_t* p = s;
	while ( *p++ );
	return (size_t)(p-s-1);
}

size_t char32_len(const char32_t* s)
{
	const char32_t* p = s;
	while ( *p++ );
	return (size_t)(p-s-1);
}

size_t wchar_len(const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return char16_len((const char16_t*)s);
#else
	return char32_len((const char32_t*)s);
#endif
}


static inline int is_surrogate(char16_t uc) { return (uc - 0xd800u) < 2048u; }
static inline int is_high_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xd800; }
static inline int is_low_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

static inline char32_t surrogate_to_utf32(char16_t high, char16_t low) {
    return char32_t((high << 10) + low - 0x35fdc00); // Safe cast, it fits in 32 bits
}


/*************************************************************   Char conversion   *********************************************************/

#define halfBase 0x0010000UL
#define halfMask 0x3FFUL
#define halfShift 10 /* used for shifting by 10 bits */
#define UNI_SUR_HIGH_START  0xD800u
#define UNI_SUR_LOW_START   0xDC00u



const char* get_char32_from_utf8_string(const char *s, char32_t* char32)
{
	char32_t c;
	if (*s & 0x80) {
		if (*(s+1) == 0) {
			// Finished in the middle of an utf8 multibyte char
			return 0;
		}
		if ((*(s+1) & 0xc0) != 0x80) {
			s += 1;
			return 0;
		}
		if ((*s & 0xe0) == 0xe0) {
			if (*(s+2) == 0) {
			// Finished in the middle of an utf8 multibyte char
			return 0;
		}
			if ((*(s+2) & 0xc0) != 0x80) {
				s += 2;
				return 0;
			}
			if ((*s & 0xf0) == 0xf0) {
				if (*(s+3) == 0) {
					// Finished in the middle of an utf8 multibyte char
					return 0;
				}
				if ((*s & 0xf8) != 0xf0 || (*(s+3) & 0xc0) != 0x80) {
					s += 3;
					return 0;
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

const char16_t* get_char32_from_char16_string(const char16_t* s, char32_t* char32)
{
	const char16_t char16_1 = *s++;
	if (!is_surrogate(char16_1)) {
		*char32 = char16_1;
		return s;
	} else {
		if (is_high_surrogate(char16_1) && is_low_surrogate(*s)) {
			*char32 = surrogate_to_utf32(char16_1, *s++);
			return s;
		} else {
			return 0;
		}
	}
}

/*
 * dst_max_len MUST be >= 1 when called
 */
char* get_utf8_from_char32(char* dst, size_t* dst_max_len, char32_t utf32_char)
{
#ifdef JIEF_DEBUG
	char* dst_debug = dst;
	(void)dst_debug;
#endif
	/* assertion: utf32_char is a single UTF-4 value */
	/* assertion: dst_max_len >= 1 */
	int bits = 0; // just to silence the warning
	
	if (utf32_char < 0x80) {
		*dst++ = (char)utf32_char;
		*dst_max_len -= 1;
		bits = -6;
	}
	else if (utf32_char < 0x800) {
		*dst++ = (char)(((utf32_char >> 6) & 0x1F) | 0xC0);
		*dst_max_len -= 1;
		bits = 0;
	}
	else if (utf32_char < 0x10000) {
		*dst++ = (char)(((utf32_char >> 12) & 0x0F) | 0xE0);
		*dst_max_len -= 1;
		bits = 6;
	}
	else {
		*dst++ = (char)(((utf32_char >> 18) & 0x07) | 0xF0);
		*dst_max_len -= 1;
		bits = 12;
	}
	for (  ;  *dst_max_len > 0  &&  bits >= 0  ;  bits -= 6  ) {
		*dst++ = (char)(((utf32_char >> bits) & 0x3F) | 0x80);
		*dst_max_len -= 1;
	}
#ifdef JIEF_DEBUG
	*dst = 0;
#endif
	return dst;
}


/*************************************************************   utf8 - char16   *********************************************************/


size_t utf8_string_char16_count(const char *s)
{
	if ( !s ) return 0;
	size_t len = 0;

	while ( *s ) {
		char32_t c;
		s = get_char32_from_utf8_string(s, &c);
		if ( c == 0 ) return len;
		if ( c <= 0xFFFF) {
			len += 1;
		}else{
			len += 2;
		}
	}
	return len;
}

size_t char16_string_from_utf8_string(char16_t* dst, size_t dst_max_len, const char *s)
{
	if ( dst_max_len == 0 ) return 0;
	dst_max_len -= 1;

//	size_t dst_len = 0;
	char16_t* p = dst;
	char16_t* p_max = dst + dst_max_len;

	while ( *s  &&  p < p_max ) {
		char32_t c;
		s = get_char32_from_utf8_string(s, &c);
		if ( c == 0 ) return (size_t)(p-dst);
		char16_t char16_1, char16_2;
		get_char16_from_char32(c, &char16_1, &char16_2);
		if ( char16_2 != 0 ) {
			if ( p < p_max-1 ) {
				*p++ = char16_1;
				*p++ = char16_2;
			}else{
				*p = 0;
				return (size_t)(p-dst);
			}
		}else{
			*p++ = char16_1;
		}
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf8_string_from_char16_string(char* dst, size_t dst_max_len, const char16_t *s)
{
	char* p = dst;
	while ( *s  &&  dst_max_len > 0 ) {
		char32_t utf32_char;
		s = get_char32_from_char16_string(s, &utf32_char);
		p = get_utf8_from_char32(p, &dst_max_len, utf32_char);
	}
	*p = 0;
	return (size_t)(p-dst);
}



/*************************************************************   utf8 - char32   *********************************************************/


size_t utf8_string_char32_count(const char *s)
{
	if ( !s ) return 0;
	size_t len = 0;

	while ( *s ) {
		char32_t c;
		s = get_char32_from_utf8_string(s, &c);
		if ( c == 0 ) return len;
		len += 1;
	}
	return len;
}


size_t char32_string_from_utf8_string(char32_t* dst, size_t dst_max_len, const char *s)
{
	if ( dst_max_len == 0 ) return 0;
	char32_t* p = dst;
	char32_t* p_max = dst + dst_max_len - 1;

	while ( *s  &&  p < p_max ) {
		char32_t c;
		s = get_char32_from_utf8_string(s, &c);
		if ( c == 0 ) return (size_t)(p-dst);
		*p++ = c;
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf8_string_from_char32_string(char* dst, size_t dst_max_len, const char32_t *s)
{
	char* p = dst;
	while ( *s  &&  dst_max_len > 0 ) {
		p = get_utf8_from_char32(p, &dst_max_len, *s++);
	}
	*p = 0;
	return (size_t)(p-dst);
}


/*************************************************************   utf8 - wchar   *********************************************************/

size_t utf8_string_wchar_count(const char *s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf8_string_char16_count(s);
#else
	return utf8_string_char32_count(s);
#endif
}

size_t wchar_string_from_utf8_string(wchar_t* dst, size_t dst_max_len, const char *s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return char16_string_from_utf8_string((char16_t*)dst, dst_max_len, s);
#else
	return char32_string_from_utf8_string((char32_t*)dst, dst_max_len, s);
#endif
}

size_t utf8_string_from_wchar_string(char* dst, size_t dst_max_len, const wchar_t* s)
{
#if __WCHAR_MAX__ <= 0xFFFFu
	return utf8_string_from_char16_string(dst, dst_max_len, (char16_t*)s);
#else
	return utf8_string_from_char32_string(dst, dst_max_len, (char32_t*)s);
#endif
}




/*************************************************************   char16 - char32   *********************************************************/


size_t char16_string_char32_count(const char16_t *s)
{
	if ( !s ) return 0;
	size_t len = 0;

	while ( *s ) {
		char32_t c;
		s = get_char32_from_char16_string(s, &c);
		if ( c == 0 ) return len;
		len += 1;
	}
	return len;
}


size_t utf32_string_to_char16_string(char32_t* dst, size_t dst_max_len, const char16_t *s)
{
	if ( dst_max_len == 0 ) return 0;
	char32_t* p = dst;
	char32_t* p_max = dst + dst_max_len - 1;

	while ( *s  &&  p < p_max ) {
		char32_t c;
		s = get_char32_from_char16_string(s, &c);
		if ( c == 0 ) return (size_t)(p-dst);
		*p++ = c;
	}
	*p = 0;
	return (size_t)(p-dst);
}

size_t utf16_string_to_char32_string(char16_t* dst, size_t dst_max_len, const char32_t *s)
{
	if ( dst_max_len == 0 ) return 0;
	char16_t* p = dst;
	char16_t* p_max = dst + dst_max_len - 1;

	while ( *s  &&  p < p_max ) {
		char16_t char16_1, char16_2;
		get_char16_from_char32(*s++, &char16_1, &char16_2);
		if ( char16_2 != 0 ) {
			if ( p < p_max-1 ) {
				*p++ = char16_1;
				*p++ = char16_2;
			}else{
				*p = 0;
				return (size_t)(p-dst);
			}
		}else{
			*p++ = char16_1;
		}
	}
	*p = 0;
	return (size_t)(p-dst);
}

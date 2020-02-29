//
//  utf8Conversion.hpp
//
//  Created by jief the 24 Feb 2020.
//

#include "utf8Conversion.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define uint16_t UINT16
#define uint32_t UINT32
#define size_t UINTN


size_t StrLenInWChar(const char *s, size_t src_len)
{
	size_t dst_len = 0;

	while ( *s ) {
		char32_t c;
		if (*s & 0x80) {
			if (*(s+1) == 0) {
				// Finished in the middle of an utf8 multibyte char
				return dst_len;
			}
			if ((*(s+1) & 0xc0) != 0x80) {
				s += 1;
				continue;
			}
			if ((*s & 0xe0) == 0xe0) {
				if (*(s+2) == 0) {
				// Finished in the middle of an utf8 multibyte char
				return dst_len;
			}
				if ((*(s+2) & 0xc0) != 0x80) {
					s += 2;
					continue;
				}
				if ((*s & 0xf0) == 0xf0) {
					if (*(s+3) == 0) {
						// Finished in the middle of an utf8 multibyte char
						return dst_len;
					}
					if ((*s & 0xf8) != 0xf0 || (*(s+3) & 0xc0) != 0x80) {
						s += 3;
						continue;
					}
					/* 4-byte code */
					c = (*s & 0x7) << 18;
					c |= (*(s+1) & 0x3f) << 12;
					c |= (*(s+2) & 0x3f) << 6;
					c |= *(s+3) & 0x3f;
					s += 4;
				} else {
					/* 3-byte code */
					c = (*s & 0xf) << 12;
					c |= (*(s+1) & 0x3f) << 6;
					c |= *(s+2) & 0x3f;
					s += 3;
				}
			} else {
				/* 2-byte code */
				c = (*s & 0x1f) << 6;
				c |= *(s+1) & 0x3f;
				s += 2;
			}
		} else {
			/* 1-byte code */
			c = *s;
			s += 1;
		}
#if __WCHAR_MAX__ > 0xFFFFu
		dst_len++;
#else
		if ( c <= 0xFFFF) {
				dst_len++;
		} else {
				dst_len++;
				dst_len++;
		}
#endif
	}
	return dst_len;
}






#define halfBase 0x0010000UL
#define halfMask 0x3FFUL
#define halfShift 10 /* used for shifting by 10 bits */
#define UNI_SUR_HIGH_START  0xD800u
#define UNI_SUR_LOW_START   0xDC00u



void utf8ToWChar(wchar_t* dst, size_t dst_max_len,  const char *s, size_t src_len)
{
	if ( dst_max_len == 0 ) return;
	dst_max_len -= 1;

	size_t dst_len = 0;

	while ( *s ) {
		char32_t c;
		if (*s & 0x80) {
			if (*(s+1) == 0) {
				// Finished in the middle of an utf8 multibyte char
				goto exit;
			}
			if ((*(s+1) & 0xc0) != 0x80) {
				s += 1;
				continue;
			}
			if ((*s & 0xe0) == 0xe0) {
				if (*(s+2) == 0) {
				// Finished in the middle of an utf8 multibyte char
				goto exit;
			}
				if ((*(s+2) & 0xc0) != 0x80) {
					s += 2;
					continue;
				}
				if ((*s & 0xf0) == 0xf0) {
					if (*(s+3) == 0) {
						// Finished in the middle of an utf8 multibyte char
						goto exit;
					}
					if ((*s & 0xf8) != 0xf0 || (*(s+3) & 0xc0) != 0x80) {
						s += 3;
						continue;
					}
					/* 4-byte code */
					c = (*s & 0x7) << 18;
					c |= (*(s+1) & 0x3f) << 12;
					c |= (*(s+2) & 0x3f) << 6;
					c |= *(s+3) & 0x3f;
					s += 4;
				} else {
					/* 3-byte code */
					c = (*s & 0xf) << 12;
					c |= (*(s+1) & 0x3f) << 6;
					c |= *(s+2) & 0x3f;
					s += 3;
				}
			} else {
				/* 2-byte code */
				c = (*s & 0x1f) << 6;
				c |= *(s+1) & 0x3f;
				s += 2;
			}
		} else {
			/* 1-byte code */
			c = *s;
			s += 1;
		}
#if __WCHAR_MAX__ > 0xFFFFu
		dst[dst_len++] = c;
		if ( dst_len == dst_max_len ) goto exit;
#else
		if ( c <= 0xFFFF) {
				dst[dst_len++] = (wchar_t)c;
		if ( dst_len == dst_max_len ) goto exit;
		} else {
				c -= halfBase;
				dst[dst_len++] = (wchar_t)((c >> halfShift) + UNI_SUR_HIGH_START);
				if ( dst_len == dst_max_len ) goto exit;
				dst[dst_len++] = (wchar_t)((c & halfMask) + UNI_SUR_LOW_START);
				if ( dst_len == dst_max_len ) goto exit;
		}
#endif
	}
exit:
	dst[dst_len] = 0;
}



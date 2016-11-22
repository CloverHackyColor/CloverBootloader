/* utf8.c - UTF-8 conversion routines */
/*
 *  Copyright © 2014 Pete Batard <pete@akeo.ie>
 *  Based on Netscape security libraries:
 *  Copyright © 1994-2000 Netscape Communications Corporation.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "driver.h"

/*
 * Define this if you want to support UTF-16 in UCS-2
 */
#define UTF16

/*
 * From RFC 2044:
 *
 * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
 * 0000 0000-0000 007F   0xxxxxxx
 * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
 * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
 * 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx
 */  

/*
 * From http://www.imc.org/draft-hoffman-utf16
 *
 * For U on [0x00010000,0x0010FFFF]:  Let U' = U - 0x00010000
 *
 * U' = yyyyyyyyyyxxxxxxxxxx
 * W1 = 110110yyyyyyyyyy
 * W2 = 110111xxxxxxxxxx
 */

#ifdef IS_BIG_ENDIAN
#define L_0 0
#define L_1 1
#define L_2 2
#define L_3 3
#define H_0 0
#define H_1 1
#else
#define L_0 3
#define L_1 2
#define L_2 1
#define L_3 0
#define H_0 1
#define H_1 0
#endif

#ifdef WITH_UCS4
/**
 * Convert an UTF-8 string to and from UCS-4
 *
 * @v toUnicode         If TRUE, convert from UTF-8 to UCS-4, opposite otherwise
 * @v inBuf             The input string
 * @v inBufLen          The input string length, in BYTES
 * @v outBuf            The output string
 * @v maxOutBufLen      The size of the output buffer, in BYTES
 * @v outBufLen         A pointer that returns the length of the output string, in BYTES
 * @ret Boolean         TRUE if the conversion was successful. If the output buffer is
 *                      too small, the function fails but return the required length in
 *                      outBufLen.
 */
static BOOLEAN
ConvertUcs4Utf8(BOOLEAN toUnicode, UINT8 *inBuf, UINTN inBufLen,
		UINT8 *outBuf, UINTN maxOutBufLen, UINTN *outBufLen)
{
	ASSERT((UINTN *)NULL != outBufLen);

	if( toUnicode ) {
		UINTN i, len = 0;

		for( i = 0; i < inBufLen; ) {
			if( (inBuf[i] & 0x80) == 0x00 ) i += 1;
			else if( (inBuf[i] & 0xE0) == 0xC0 ) i += 2;
			else if( (inBuf[i] & 0xF0) == 0xE0 ) i += 3;
			else if( (inBuf[i] & 0xF8) == 0xF0 ) i += 4;
			else if( (inBuf[i] & 0xFC) == 0xF8 ) i += 5;
			else if( (inBuf[i] & 0xFE) == 0xFC ) i += 6;
			else return FALSE;

			len += 4;
		}

		if( len > maxOutBufLen ) {
			*outBufLen = len;
			return FALSE;
		}

		len = 0;

		for( i = 0; i < inBufLen; ) {
			if( (inBuf[i] & 0x80) == 0x00 ) {
				/* 0000 0000-0000 007F <- 0xxxxxx */
				/* 0abcdefg -> 
				   00000000 00000000 00000000 0abcdefg */

				outBuf[len+L_0] = 0x00;
				outBuf[len+L_1] = 0x00;
				outBuf[len+L_2] = 0x00;
				outBuf[len+L_3] = inBuf[i+0] & 0x7F;

				i += 1;
			} else if( (inBuf[i] & 0xE0) == 0xC0 ) {

				if( (inBuf[i+1] & 0xC0) != 0x80 ) return FALSE;

				/* 0000 0080-0000 07FF <- 110xxxxx 10xxxxxx */
				/* 110abcde 10fghijk ->
				   00000000 00000000 00000abc defghijk */

				outBuf[len+L_0] = 0x00;
				outBuf[len+L_1] = 0x00;
				outBuf[len+L_2] = ((inBuf[i+0] & 0x1C) >> 2);
				outBuf[len+L_3] = ((inBuf[i+0] & 0x03) << 6) | ((inBuf[i+1] & 0x3F) >> 0);

				i += 2;
			} else if( (inBuf[i] & 0xF0) == 0xE0 ) {

				if( (inBuf[i+1] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+2] & 0xC0) != 0x80 ) return FALSE;

				/* 0000 0800-0000 FFFF <- 1110xxxx 10xxxxxx 10xxxxxx */
				/* 1110abcd 10efghij 10klmnop ->
				   00000000 00000000 abcdefgh ijklmnop */

				outBuf[len+L_0] = 0x00;
				outBuf[len+L_1] = 0x00;
				outBuf[len+L_2] = ((inBuf[i+0] & 0x0F) << 4) | ((inBuf[i+1] & 0x3C) >> 2);
				outBuf[len+L_3] = ((inBuf[i+1] & 0x03) << 6) | ((inBuf[i+2] & 0x3F) >> 0);

				i += 3;
			} else if( (inBuf[i] & 0xF8) == 0xF0 ) {

				if( (inBuf[i+1] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+2] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+3] & 0xC0) != 0x80 ) return FALSE;

				/* 0001 0000-001F FFFF <- 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
				/* 11110abc 10defghi 10jklmno 10pqrstu -> 
				   00000000 000abcde fghijklm nopqrstu */

				outBuf[len+L_0] = 0x00;
				outBuf[len+L_1] = ((inBuf[i+0] & 0x07) << 2) | ((inBuf[i+1] & 0x30) >> 4);
				outBuf[len+L_2] = ((inBuf[i+1] & 0x0F) << 4) | ((inBuf[i+2] & 0x3C) >> 2);
				outBuf[len+L_3] = ((inBuf[i+2] & 0x03) << 6) | ((inBuf[i+3] & 0x3F) >> 0);

				i += 4;
			} else if( (inBuf[i] & 0xFC) == 0xF8 ) {

				if( (inBuf[i+1] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+2] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+3] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+4] & 0xC0) != 0x80 ) return FALSE;

				/* 0020 0000-03FF FFFF <- 111110xx 10xxxxxx ... 10xxxxxx */
				/* 111110ab 10cdefgh 10ijklmn 10opqrst 10uvwxyz -> 
				   000000ab cdefghij klmnopqr stuvwxyz */

				outBuf[len+L_0] = inBuf[i+0] & 0x03;
				outBuf[len+L_1] = ((inBuf[i+1] & 0x3F) << 2) | ((inBuf[i+2] & 0x30) >> 4);
				outBuf[len+L_2] = ((inBuf[i+2] & 0x0F) << 4) | ((inBuf[i+3] & 0x3C) >> 2);
				outBuf[len+L_3] = ((inBuf[i+3] & 0x03) << 6) | ((inBuf[i+4] & 0x3F) >> 0);

				i += 5;
			} else /* if( (inBuf[i] & 0xFE) == 0xFC ) */ {

				if( (inBuf[i+1] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+2] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+3] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+4] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+5] & 0xC0) != 0x80 ) return FALSE;

				/* 0400 0000-7FFF FFFF <- 1111110x 10xxxxxx ... 10xxxxxx */
				/* 1111110a 10bcdefg 10hijklm 10nopqrs 10tuvwxy 10zABCDE -> 
				   0abcdefg hijklmno pqrstuvw xyzABCDE */

				outBuf[len+L_0] = ((inBuf[i+0] & 0x01) << 6) | ((inBuf[i+1] & 0x3F) >> 0);
				outBuf[len+L_1] = ((inBuf[i+2] & 0x3F) << 2) | ((inBuf[i+3] & 0x30) >> 4);
				outBuf[len+L_2] = ((inBuf[i+3] & 0x0F) << 4) | ((inBuf[i+4] & 0x3C) >> 2);
				outBuf[len+L_3] = ((inBuf[i+4] & 0x03) << 6) | ((inBuf[i+5] & 0x3F) >> 0);

				i += 6;
			}

			len += 4;
		}

		*outBufLen = len;
		return TRUE;
	} else {
		UINTN i, len = 0;
		ASSERT((inBufLen % 4) == 0);
		if ((inBufLen % 4) != 0) {
			*outBufLen = 0;
			return FALSE;
		}

		for( i = 0; i < inBufLen; i += 4 ) {
			if( inBuf[i+L_0] >= 0x04 ) len += 6;
			else if( (inBuf[i+L_0] > 0x00) || (inBuf[i+L_1] >= 0x20) ) len += 5;
			else if( inBuf[i+L_1] >= 0x01 ) len += 4;
			else if( inBuf[i+L_2] >= 0x08 ) len += 3;
			else if( (inBuf[i+L_2] > 0x00) || (inBuf[i+L_3] >= 0x80) ) len += 2;
			else len += 1;
		}

		if( len > maxOutBufLen ) {
			*outBufLen = len;
			return FALSE;
		}

		len = 0;

		for( i = 0; i < inBufLen; i += 4 ) {
			if( inBuf[i+L_0] >= 0x04 ) {
				/* 0400 0000-7FFF FFFF -> 1111110x 10xxxxxx ... 10xxxxxx */
				/* 0abcdefg hijklmno pqrstuvw xyzABCDE ->
				   1111110a 10bcdefg 10hijklm 10nopqrs 10tuvwxy 10zABCDE */

				outBuf[len+0] = 0xFC | ((inBuf[i+L_0] & 0x40) >> 6);
				outBuf[len+1] = 0x80 | ((inBuf[i+L_0] & 0x3F) >> 0);
				outBuf[len+2] = 0x80 | ((inBuf[i+L_1] & 0xFC) >> 2);
				outBuf[len+3] = 0x80 | ((inBuf[i+L_1] & 0x03) << 4)
					| ((inBuf[i+L_2] & 0xF0) >> 4);
				outBuf[len+4] = 0x80 | ((inBuf[i+L_2] & 0x0F) << 2)
					| ((inBuf[i+L_3] & 0xC0) >> 6);
				outBuf[len+5] = 0x80 | ((inBuf[i+L_3] & 0x3F) >> 0);

				len += 6;
			} else if( (inBuf[i+L_0] > 0x00) || (inBuf[i+L_1] >= 0x20) ) {
				/* 0020 0000-03FF FFFF -> 111110xx 10xxxxxx ... 10xxxxxx */
				/* 000000ab cdefghij klmnopqr stuvwxyz ->
				   111110ab 10cdefgh 10ijklmn 10opqrst 10uvwxyz */

				outBuf[len+0] = 0xF8 | ((inBuf[i+L_0] & 0x03) >> 0);
				outBuf[len+1] = 0x80 | ((inBuf[i+L_1] & 0xFC) >> 2);
				outBuf[len+2] = 0x80 | ((inBuf[i+L_1] & 0x03) << 4)
					| ((inBuf[i+L_2] & 0xF0) >> 4);
				outBuf[len+3] = 0x80 | ((inBuf[i+L_2] & 0x0F) << 2)
					| ((inBuf[i+L_3] & 0xC0) >> 6);
				outBuf[len+4] = 0x80 | ((inBuf[i+L_3] & 0x3F) >> 0);

				len += 5;
			} else if( inBuf[i+L_1] >= 0x01 ) {
				/* 0001 0000-001F FFFF -> 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
				/* 00000000 000abcde fghijklm nopqrstu ->
				   11110abc 10defghi 10jklmno 10pqrstu */

				outBuf[len+0] = 0xF0 | ((inBuf[i+L_1] & 0x1C) >> 2);
				outBuf[len+1] = 0x80 | ((inBuf[i+L_1] & 0x03) << 4)
					| ((inBuf[i+L_2] & 0xF0) >> 4);
				outBuf[len+2] = 0x80 | ((inBuf[i+L_2] & 0x0F) << 2)
					| ((inBuf[i+L_3] & 0xC0) >> 6);
				outBuf[len+3] = 0x80 | ((inBuf[i+L_3] & 0x3F) >> 0);

				len += 4;
			} else if( inBuf[i+L_2] >= 0x08 ) {
				/* 0000 0800-0000 FFFF -> 1110xxxx 10xxxxxx 10xxxxxx */
				/* 00000000 00000000 abcdefgh ijklmnop ->
				   1110abcd 10efghij 10klmnop */

				outBuf[len+0] = 0xE0 | ((inBuf[i+L_2] & 0xF0) >> 4);
				outBuf[len+1] = 0x80 | ((inBuf[i+L_2] & 0x0F) << 2)
					| ((inBuf[i+L_3] & 0xC0) >> 6);
				outBuf[len+2] = 0x80 | ((inBuf[i+L_3] & 0x3F) >> 0);

				len += 3;
			} else if( (inBuf[i+L_2] > 0x00) || (inBuf[i+L_3] >= 0x80) ) {
				/* 0000 0080-0000 07FF -> 110xxxxx 10xxxxxx */
				/* 00000000 00000000 00000abc defghijk ->
				   110abcde 10fghijk */

				outBuf[len+0] = 0xC0 | ((inBuf[i+L_2] & 0x07) << 2)
					| ((inBuf[i+L_3] & 0xC0) >> 6);
				outBuf[len+1] = 0x80 | ((inBuf[i+L_3] & 0x3F) >> 0);

				len += 2;
			} else {
				/* 0000 0000-0000 007F -> 0xxxxxx */
				/* 00000000 00000000 00000000 0abcdefg ->
				   0abcdefg */

				outBuf[len+0] = (inBuf[i+L_3] & 0x7F);

				len += 1;
			}
		}

		*outBufLen = len;
		return TRUE;
	}
}
#endif

/**
 * Convert an UTF-8 string to and from UCS-2/UTF-16
 *
 * @v toUnicode         If TRUE, convert from UTF-8 to UCS-2/UTF-16, opposite otherwise
 * @v inBuf             The input string
 * @v inBufLen          The input string length, in BYTES
 * @v outBuf            The output string
 * @v maxOutBufLen      The size of the output buffer, in BYTES
 * @v outBufLen         A pointer that returns the length of the output string, in BYTES
 * @ret Boolean         TRUE if the conversion was successful. If the output buffer is
 *                      too small, the function fails but return the required length in
 *                      outBufLen.
 */
static BOOLEAN
ConvertUcs2Utf8(BOOLEAN toUnicode, UINT8 *inBuf, UINTN inBufLen,
		UINT8 *outBuf, UINTN maxOutBufLen, UINTN *outBufLen)
{
	ASSERT((UINTN *)NULL != outBufLen);

	if( toUnicode ) {
		UINTN i, len = 0;

		for( i = 0; i < inBufLen; ) {
			if( (inBuf[i] & 0x80) == 0x00 ) {
				i += 1;
				len += 2;
			} else if( (inBuf[i] & 0xE0) == 0xC0 ) {
				i += 2;
				len += 2;
			} else if( (inBuf[i] & 0xF0) == 0xE0 ) {
				i += 3;
				len += 2;
#ifdef UTF16
			} else if( (inBuf[i] & 0xF8) == 0xF0 ) { 
				i += 4;
				len += 4;

				if( (inBuf[i] & 0x04) && 
					((inBuf[i] & 0x03) || (inBuf[i+1] & 0x30)) ) {
						/* Not representable as UTF16 */
						return FALSE;
				}

#endif /* UTF16 */
			} else return FALSE;
		}

		if( len > maxOutBufLen ) {
			*outBufLen = len;
			return FALSE;
		}

		len = 0;

		for( i = 0; i < inBufLen; ) {
			if( (inBuf[i] & 0x80) == 0x00 ) {
				/* 0000-007F <- 0xxxxxx */
				/* 0abcdefg -> 00000000 0abcdefg */

				outBuf[len+H_0] = 0x00;
				outBuf[len+H_1] = inBuf[i+0] & 0x7F;

				i += 1;
				len += 2;
			} else if( (inBuf[i] & 0xE0) == 0xC0 ) {

				if( (inBuf[i+1] & 0xC0) != 0x80 ) return FALSE;

				/* 0080-07FF <- 110xxxxx 10xxxxxx */
				/* 110abcde 10fghijk -> 00000abc defghijk */

				outBuf[len+H_0] = ((inBuf[i+0] & 0x1C) >> 2);
				outBuf[len+H_1] = ((inBuf[i+0] & 0x03) << 6) | ((inBuf[i+1] & 0x3F) >> 0);

				i += 2;
				len += 2;
			} else if( (inBuf[i] & 0xF0) == 0xE0 ) {

				if( (inBuf[i+1] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+2] & 0xC0) != 0x80 ) return FALSE;

				/* 0800-FFFF <- 1110xxxx 10xxxxxx 10xxxxxx */
				/* 1110abcd 10efghij 10klmnop -> abcdefgh ijklmnop */

				outBuf[len+H_0] = ((inBuf[i+0] & 0x0F) << 4) | ((inBuf[i+1] & 0x3C) >> 2);
				outBuf[len+H_1] = ((inBuf[i+1] & 0x03) << 6) | ((inBuf[i+2] & 0x3F) >> 0);

				i += 3;
				len += 2;
#ifdef UTF16
			} else if( (inBuf[i] & 0xF8) == 0xF0 ) { 
				int abcde, BCDE;

				if( (inBuf[i+1] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+2] & 0xC0) != 0x80 ) return FALSE;
				if( (inBuf[i+3] & 0xC0) != 0x80 ) return FALSE;

				/* 0001 0000-001F FFFF <- 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
				/* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx -> [D800-DBFF] [DC00-DFFF] */

				/* 11110abc 10defghi 10jklmno 10pqrstu -> 
				   { Let 0BCDE = abcde - 1 }
				   110110BC DEfghijk 110111lm nopqrstu */

				abcde = ((inBuf[i+0] & 0x07) << 2) | ((inBuf[i+1] & 0x30) >> 4);
				BCDE = abcde - 1;

				ASSERT(BCDE < 0x10); /* should have been caught above */

				outBuf[len+0+H_0] = 0xD8 | ((BCDE & 0x0C) >> 2);
				outBuf[len+0+H_1] = ((BCDE & 0x03) << 6) 
					| ((inBuf[i+1] & 0x0F) << 2)
					| ((inBuf[i+2] & 0x30) >> 4);
				outBuf[len+2+H_0] = 0xDC | ((inBuf[i+2] & 0x0C) >> 2);
				outBuf[len+2+H_1] = ((inBuf[i+2] & 0x03) << 6) | ((inBuf[i+3] & 0x3F) >> 0);

				i += 4;
				len += 4;
#endif /* UTF16 */
			} else return FALSE;
		}

		*outBufLen = len;
		return TRUE;
	} else {
		UINTN i, len = 0;
		ASSERT((inBufLen % 2) == 0);
		if ((inBufLen % 2) != 0) {
			*outBufLen = 0;
			return FALSE;
		}

		for( i = 0; i < inBufLen; i += 2 ) {
			if( (inBuf[i+H_0] == 0x00) && ((inBuf[i+H_0] & 0x80) == 0x00) ) len += 1;
			else if( inBuf[i+H_0] < 0x08 ) len += 2;
#ifdef UTF16
			else if( ((inBuf[i+0+H_0] & 0xDC) == 0xD8) ) {
				if( ((inBuf[i+2+H_0] & 0xDC) == 0xDC) && ((inBufLen - i) > 2) ) {
					i += 2;
					len += 4;
				} else {
					return FALSE;
				}
			}
#endif /* UTF16 */
			else len += 3;
		}

		if( len > maxOutBufLen ) {
			*outBufLen = len;
			return FALSE;
		}

		len = 0;

		for( i = 0; i < inBufLen; i += 2 ) {
			if( (inBuf[i+H_0] == 0x00) && ((inBuf[i+H_1] & 0x80) == 0x00) ) {
				/* 0000-007F -> 0xxxxxx */
				/* 00000000 0abcdefg -> 0abcdefg */

				outBuf[len] = inBuf[i+H_1] & 0x7F;

				len += 1;
			} else if( inBuf[i+H_0] < 0x08 ) {
				/* 0080-07FF -> 110xxxxx 10xxxxxx */
				/* 00000abc defghijk -> 110abcde 10fghijk */

				outBuf[len+0] = 0xC0 | ((inBuf[i+H_0] & 0x07) << 2) 
					| ((inBuf[i+H_1] & 0xC0) >> 6);
				outBuf[len+1] = 0x80 | ((inBuf[i+H_1] & 0x3F) >> 0);

				len += 2;
#ifdef UTF16
			} else if( (inBuf[i+H_0] & 0xDC) == 0xD8 ) {
				int abcde, BCDE;

				ASSERT(((inBuf[i+2+H_0] & 0xDC) == 0xDC) && ((inBufLen - i) > 2));

				/* D800-DBFF DC00-DFFF -> 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
				/* 110110BC DEfghijk 110111lm nopqrstu ->
				   { Let abcde = BCDE + 1 }
				   11110abc 10defghi 10jklmno 10pqrstu */

				BCDE = ((inBuf[i+H_0] & 0x03) << 2) | ((inBuf[i+H_1] & 0xC0) >> 6);
				abcde = BCDE + 1;

				outBuf[len+0] = 0xF0 | ((abcde & 0x1C) >> 2);
				outBuf[len+1] = 0x80 | ((abcde & 0x03) << 4) 
					| ((inBuf[i+0+H_1] & 0x3C) >> 2);
				outBuf[len+2] = 0x80 | ((inBuf[i+0+H_1] & 0x03) << 4)
					| ((inBuf[i+2+H_0] & 0x03) << 2)
					| ((inBuf[i+2+H_1] & 0xC0) >> 6);
				outBuf[len+3] = 0x80 | ((inBuf[i+2+H_1] & 0x3F) >> 0);

				i += 2;
				len += 4;
#endif /* UTF16 */
			} else {
				/* 0800-FFFF -> 1110xxxx 10xxxxxx 10xxxxxx */
				/* abcdefgh ijklmnop -> 1110abcd 10efghij 10klmnop */

				outBuf[len+0] = 0xE0 | ((inBuf[i+H_0] & 0xF0) >> 4);
				outBuf[len+1] = 0x80 | ((inBuf[i+H_0] & 0x0F) << 2) 
					| ((inBuf[i+H_1] & 0xC0) >> 6);
				outBuf[len+2] = 0x80 | ((inBuf[i+H_1] & 0x3F) >> 0);

				len += 3;
			}
		}

		*outBufLen = len;
		return TRUE;
	}
}

#ifdef WITH_ISO_8859_1
/**
 * Convert an ISO-8859-1 string to UTF-8
 *
 * @v inBuf             The input string
 * @v inBufLen          The input string length, in BYTES
 * @v outBuf            The output string
 * @v maxOutBufLen      The size of the output buffer, in BYTES
 * @v outBufLen         A pointer that returns the length of the output string, in BYTES
 * @ret Boolean         TRUE if the conversion was successful. If the output buffer is
 *                      too small, the function fails but return the required length in
 *                      outBufLen.
 */
static BOOLEAN
ConvertIso88591ToUtf8(const UINT8 *inBuf, UINTN inBufLen,
		UINT8 *outBuf, UINTN maxOutBufLen, UINTN *outBufLen)
{
	UINTN i, len = 0;

	ASSERT((UINTN *)NULL != outBufLen);

	for( i = 0; i < inBufLen; i++) {
		if( (inBuf[i] & 0x80) == 0x00 ) len += 1;
		else len += 2;
	}

	if( len > maxOutBufLen ) {
		*outBufLen = len;
		return FALSE;
	}

	len = 0;

	for( i = 0; i < inBufLen; i++) {
		if( (inBuf[i] & 0x80) == 0x00 ) {
			/* 00-7F -> 0xxxxxxx */
			/* 0abcdefg -> 0abcdefg */

			outBuf[len] = inBuf[i];
			len += 1;
		} else {
			/* 80-FF <- 110xxxxx 10xxxxxx */
			/* 00000000 abcdefgh -> 110000ab 10cdefgh */

			outBuf[len+0] = 0xC0 | ((inBuf[i] & 0xC0) >> 6);
			outBuf[len+1] = 0x80 | ((inBuf[i] & 0x3F) >> 0);

			len += 2;
		}
	}

	*outBufLen = len;
	return TRUE;
}
#endif

/**
 * Convert an UTF-8 string to an allocated UTF-16 string
 *
 * @v src               A NUL terminated UTF-8 input string
 * @ret String          A NUL terminated UTF-16 string, or NULL on error.
 *                      The caller needs to free the returned string.
 */
CHAR16
*Utf8ToUtf16Alloc(CHAR8 *src)
{
	UINTN DstLen = 0, srcLen = strlena(src);
	static CHAR16 *Dst = NULL;

	if (srcLen++ == 0)	/* +1 for NUL terminator */
		return L"";

	/* Failure is expected on this call, since we only want the length */
	ConvertUcs2Utf8(TRUE, (UINT8 *) src, srcLen, NULL, 0, &DstLen);
	if (DstLen == 0)
		goto error;

	Dst = (CHAR16 *) AllocatePool(DstLen);
	if (Dst == NULL)
		goto error;
	
	if (!ConvertUcs2Utf8(TRUE, (UINT8 *) src, srcLen, (UINT8 *) Dst, DstLen, &DstLen))
		goto error;

	return Dst;

error:
    if (Dst != NULL)
    {
        FreePool(Dst);
        Dst = NULL;
    }

	return NULL;
}

/**
 * Convert an UTF-8 string to UTF-16, using an user supplied buffer
 *
 * @v src               A NUL terminated UTF-8 input string
 * @v Dst               A pointer an UTF-16 string buffer
 * @v Len               The length of the target UTF-16 string buffer, in BYTES
 * @ret Status          EFI_SUCCESS if the conversion was successful, and EFI error code on error
 */
EFI_STATUS
Utf8ToUtf16NoAlloc(CHAR8 *src, CHAR16 *Dst, UINTN Len)
{
	UINTN RetLen, srcLen = strlena(src);

	if ((Dst == NULL) || (Len < 1))
		return EFI_INVALID_PARAMETER;

	if (srcLen++ == 0) {	/* +1 for NUL terminator */
		*Dst = 0;
		return EFI_SUCCESS;
	}

	if (!ConvertUcs2Utf8(TRUE, (UINT8 *) src, srcLen, (UINT8 *) Dst, Len, &RetLen))
		return EFI_NO_MAPPING;

	if (RetLen > Len)
		return EFI_BUFFER_TOO_SMALL;

	return EFI_SUCCESS;
}

/**
 * Convert an UTF-16 string to an allocated UTF-8 string
 *
 * @v Src               A NUL terminated UTF-16 input string
 * @ret string          A NUL terminated UTF-8 string, or NULL on error.
 *                      The caller needs to free the returned string.
 */
CHAR8
*Utf16ToUtf8Alloc(CHAR16 *Src)
{
	UINTN dstLen = 0, SrcLen = StrLen(Src);
	static CHAR8 *dst = NULL;

	if (SrcLen++ == 0)	/* +1 for NUL terminator */
		return "";

	/* Failure is expected on this call, since we want the length */
	ConvertUcs2Utf8(FALSE, (UINT8 *) Src, SrcLen * sizeof(CHAR16), NULL, 0, &dstLen);
	if (dstLen == 0)
		goto error;

	dst = (CHAR8 *) AllocatePool(dstLen);
	if (dst == NULL)
		goto error;
	
	if (!ConvertUcs2Utf8(FALSE, (UINT8 *) Src, SrcLen * sizeof(CHAR16), (UINT8 *) dst, dstLen, &dstLen))
		goto error;

	return dst;

error:
    if (dst != NULL)
    {
        FreePool(dst);
        dst = NULL;
    }

	return NULL;
}

/**
 * Convert an UTF-16 string to UTF-8, using an user supplied buffer
 *
 * @v Src               A NUL terminated UTF-16 input string
 * @v dst               A pointer an UTF-8 string buffer
 * @v len               The length of the target UTF-8 string buffer, in BYTES
 * @ret Status          EFI_SUCCESS if the conversion was successful, and EFI error code on error
 */
EFI_STATUS
Utf16ToUtf8NoAlloc(CHAR16 *Src, CHAR8 *dst, UINTN len)
{
	UINTN retlen, SrcLen = StrLen(Src);

	if ((dst == NULL) || (len < 1))
		return EFI_INVALID_PARAMETER;

	if (SrcLen++ == 0) {	/* +1 for NUL terminator */
		*dst = 0;
		return EFI_SUCCESS;
	}

	if (!ConvertUcs2Utf8(FALSE, (UINT8 *) Src, SrcLen * sizeof(CHAR16), (UINT8 *) dst, len, &retlen))
		return EFI_NO_MAPPING;

	if (retlen > len)
		return EFI_BUFFER_TOO_SMALL;

	return EFI_SUCCESS;
}

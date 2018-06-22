#ifndef _UTILS_H
#define _UTILS_H

/*
 * Define the 8-bit and 16-bit numeric types.
 *
typedef signed char			INT8;
typedef unsigned char		UINT8;
typedef UINT8 				  BOOLEAN;
typedef char				    CHAR8;
typedef short				    INT16;
typedef unsigned short	UINT16;
typedef int					    INT32;
typedef unsigned int		UINT32;
typedef long long			  INT64;
typedef unsigned long long	UINT64;
typedef	float				    FLOAT;
typedef	double			  	DOUBLE;
*/
/*
 * alpha    = lowalpha | upalpha
 */
#define IS_ALPHA(x) (IS_LOWALPHA(x) || IS_UPALPHA(x))

/*
 * lowalpha = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" |
 *            "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" |
 *            "u" | "v" | "w" | "x" | "y" | "z"
 */
#define IS_LOWALPHA(x) (((unsigned char)(x) >= 'a') && ((unsigned char)(x) <= 'z'))

/*
 * upalpha = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" |
 *           "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" |
 *           "U" | "V" | "W" | "X" | "Y" | "Z"
 */
#define IS_UPALPHA(x) (((unsigned char)(x) >= 'A') && ((unsigned char)(x) <= 'Z'))

#ifdef IS_DIGIT
#undef IS_DIGIT
#endif
/*
 * digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
 */
#define IS_DIGIT(x) (((unsigned char)(x) >= '0') && ((unsigned char)(x) <= '9'))

/*
 * alphanum = alpha | digit
 */
#define IS_ALPHANUM(x) (IS_ALPHA(x) || IS_DIGIT(x))

/*
 * hex = digit | "A" | "B" | "C" | "D" | "E" | "F" |
 *               "a" | "b" | "c" | "d" | "e" | "f"
 */
#ifndef IS_HEX
#define IS_HEX(x) ((IS_DIGIT(x)) || (((unsigned char)(x) >= 'a') && ((unsigned char)(x) <= 'f')) || \
	    (((unsigned char)(x) >= 'A') && ((unsigned char)(x) <= 'F')))
#endif
/*
 * mark = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")" | ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | "," |
 * 	      "[" | "]"
 */

#define IS_MARK(x) (((unsigned char)(x) == '-') || ((unsigned char)(x) == '_') || ((unsigned char)(x) == '.') || ((unsigned char)(x) == '!') || ((unsigned char)(x) == '~') || ((unsigned char)(x) == '*') || ((unsigned char)(x) == '\'') || \
    ((unsigned char)(x) == '(') || ((unsigned char)(x) == ')') || ((unsigned char)(x) == ';') || ((unsigned char)(x) == '/') || ((unsigned char)(x) == '?') ||	((unsigned char)(x) == ':') || ((unsigned char)(x) == '@') || ((unsigned char)(x) == '&') || ((unsigned char)(x) == '=') ||	\
	((unsigned char)(x) == '+') || ((unsigned char)(x) == '$') || ((unsigned char)(x) == ',') || ((unsigned char)(x) == '[') || ((unsigned char)(x) == ']'))

#define IS_SPACE(x) (((unsigned char)(x) == ' '))

/*
 * alphanummark = alphanum | mark
 */
#define IS_ALPHANUMMARK(x) (IS_ALPHANUM(x) || IS_MARK(x) || IS_SPACE(x))

/*
 * Read little-endian values of various sizes from memory buffers.
 */
#define	_READ_BYTE(buf,offset)	((UINT32)(UINT8)(((UINT8 *)(buf))[(offset)]))
#define	_READ_BYTE_SHIFT(buf,offset,shift)	(((UINT32)(UINT8)(((UINT8 *)(buf))[(offset)])) << (shift))

#define	READ_INT8(buf)		((INT8)(_READ_BYTE((buf), 0)))
#define	READ_UINT8(buf)		((UINT8)(_READ_BYTE((buf), 0)))
#define	READ_INT16(buf)		((INT16)(_READ_BYTE((buf), 0) | _READ_BYTE_SHIFT((buf), 1, 8)))
#define	READ_UINT16(buf)	((UINT16)(_READ_BYTE((buf), 0) | _READ_BYTE_SHIFT((buf), 1, 8)))
#define	READ_INT32(buf)		((INT32)(_READ_BYTE((buf), 0) | _READ_BYTE_SHIFT((buf), 1, 8) | _READ_BYTE_SHIFT((buf), 2, 16) | _READ_BYTE_SHIFT((buf), 3, 24)))
#define	READ_UINT32(buf)	((UINT32)(_READ_BYTE((buf), 0) | _READ_BYTE_SHIFT((buf), 1, 8) | _READ_BYTE_SHIFT((buf), 2, 16) | _READ_BYTE_SHIFT((buf), 3, 24)))
#define	READ_INT64(buf)		(((INT64)(READ_UINT32((buf)))) | (((INT64)(READ_INT32(((UINT8 *)(buf)) + 4))) << 32))
#define	READ_UINT64(buf)	(((UINT64)(READ_UINT32((buf)))) | (((UINT64)(READ_UINT32(((UINT8 *)(buf)) + 4))) << 32))

/*
 * Write little-endian values of various sizes to memory buffers.
 */
#define	WRITE_UINT8(buf, value)	\
			do { \
				(buf)[0] = (unsigned char)(value & 0xFF); \
			} while (0) 
#define	WRITE_UINT16(buf, value)	\
			do { \
				(buf)[0] = (unsigned char)(value & 0xFF); \
				(buf)[1] = (unsigned char)(((value) >> 8) & 0xFF); \
			} while (0)
#define	WRITE_INT16(buf, value)	WRITE_UINT16((buf), (UINT16)(value))
#define	WRITE_UINT32(buf, value)	\
			do { \
				(buf)[0] = (unsigned char)(value & 0xFF); \
				(buf)[1] = (unsigned char)(((value) >> 8) & 0xFF); \
				(buf)[2] = (unsigned char)(((value) >> 16) & 0xFF); \
				(buf)[3] = (unsigned char)(((value) >> 24) & 0xFF); \
			} while (0)
#define	WRITE_INT32(buf, value)	WRITE_UINT32((buf), (UINT32)(value))
#define	WRITE_UINT64(buf, value)	\
			do { \
				(buf)[0] = (unsigned char)(value & 0xFF); \
				(buf)[1] = (unsigned char)(((value) >> 8) & 0xFF); \
				(buf)[2] = (unsigned char)(((value) >> 16) & 0xFF); \
				(buf)[3] = (unsigned char)(((value) >> 24) & 0xFF); \
				(buf)[4] = (unsigned char)(((value) >> 32) & 0xFF); \
				(buf)[5] = (unsigned char)(((value) >> 40) & 0xFF); \
				(buf)[6] = (unsigned char)(((value) >> 48) & 0xFF); \
				(buf)[7] = (unsigned char)(((value) >> 56) & 0xFF); \
			} while (0)
#define	WRITE_INT64(buf, value)	WRITE_UINT64((buf), (UINT64)(value))

void assertion(int condition, char * message);

/*
 * Read a UTF-8 character from a string position.
 */
unsigned long UTF8ReadChar(const void *str, int len, int *posn);

/*
 * Write a UTF-8 character to a buffer.  Returns the
 * number of bytes used.  If the buffer is NULL, then
 * return the number of bytes needed.
 */
int UTF8WriteChar(char *str, unsigned long ch);

/*
 * Read a UTF-16 character from a 16-bit string position.
 * "len" and "posn" are indexes into a 16-bit array.
 */
unsigned long UTF16ReadChar(const unsigned short *str, int len, int *posn);

/*
 * Read a UTF-16 character from a string as little-endian values.
 * "len" and "posn" are indexes into a byte array.
 */
unsigned long UTF16ReadCharAsBytes(const void *str, int len, int *posn);

/*
 * Convert a 32-bit Unicode character into UTF-16.  Returns the
 * number of 16-bit characters required (1 or 2), or zero if
 * the character cannot be represented using UTF-16.  If "buf"
 * is NULL, then return the number of characters required.
 */
int UTF16WriteChar(unsigned short *buf, unsigned long ch);

/*
 * Convert a 32-bit Unicode character into UTF-16, and store it
 * using little-endian bytes at "buf".  Returns the number of
 * bytes (2 or 4), or zero if the character cannot be represented.
 * If "buf" is NULL, then return the number of bytes required.
 */
int UTF16WriteCharAsBytes(void *buf, unsigned long ch);

// Skip the leading white space and '0x' or '0X' of a integer string
char * TrimHexStr (char *Str, int *IsHex);

// Convert hex string to uint
unsigned int Xtoi (char *Str, unsigned int *Bytes);

// Convert hex string to 64 bit data.
void Xtoi64 (char *Str, unsigned long *Data, unsigned int *Bytes);

// Convert decimal string to uint
unsigned int Dtoi (char *str);

// Convert decimal string to uint
void Dtoi64 (char *str,unsigned long *Data);

// Convert integer string to uint.
unsigned int Strtoi (char *Str, unsigned int *Bytes);

//  Convert integer string to 64 bit data.
void Strtoi64 (char *Str, unsigned long *Data, unsigned int *Bytes);

int StrToBuf (unsigned char *Buf, unsigned int BufferLength, char *Str);

/* Converts Unicode string to binary buffer.
 * The conversion may be partial.
 * The first character in the string that is not hex digit stops the conversion.
 * At a minimum, any blob of data could be represented as a hex string.
 */
int HexStringToBuf (unsigned char *Buf, unsigned int *Len, char *Str, unsigned int *ConvertedStrLen);

// Determines if a Unicode character is a hexadecimal digit.
int IsCharHexDigit (unsigned char *Digit, char Char);

void CatPrintf(char *target, const char *format, ...);


void *MallocCopy(unsigned int size, void *buf);
#endif

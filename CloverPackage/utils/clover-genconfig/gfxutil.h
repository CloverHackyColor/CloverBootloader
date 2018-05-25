/*
 *  Created by mcmatrix on 08.01.08.
 *  Copyright 2008 mcmatrix All rights reserved.
 *
 */
 
#include <CoreFoundation/CoreFoundation.h>            // (CFDictionary, ...)
#include "efidevp.h"

// Constants
#define MAX_FILENAME 255

/*
 * Define the 8-bit and 16-bit numeric types.
 *
typedef signed char			INT8;
typedef unsigned char		UINT8;
typedef INT8 				    BOOLEAN;
typedef char				    CHAR8;
typedef short				    INT16;
typedef unsigned short	UINT16;
typedef int					    INT32;
typedef unsigned int		UINT32;
typedef long				    INT64;
typedef unsigned long		UINT64;
typedef	float				    FLOAT;
typedef	double				  DOUBLE;
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
#define IS_HEX(x) ((IS_DIGIT(x)) || (((unsigned char)(x) >= 'a') && ((unsigned char)(x) <= 'f')) || \
(((unsigned char)(x) >= 'A') && ((unsigned char)(x) <= 'F')))

/*
 * mark = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")" | ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | "," |
 *         "[" | "]"
 */

#define IS_MARK(x) (((unsigned char)(x) == '-') || ((unsigned char)(x) == '_') || ((unsigned char)(x) == '.') || ((unsigned char)(x) == '!') || ((unsigned char)(x) == '~') || ((unsigned char)(x) == '*') || ((unsigned char)(x) == '\'') || \
((unsigned char)(x) == '(') || ((unsigned char)(x) == ')') || ((unsigned char)(x) == ';') || ((unsigned char)(x) == '/') || ((unsigned char)(x) == '?') ||  ((unsigned char)(x) == ':') || ((unsigned char)(x) == '@') || ((unsigned char)(x) == '&') || ((unsigned char)(x) == '=') ||  \
((unsigned char)(x) == '+') || ((unsigned char)(x) == '$') || ((unsigned char)(x) == ',') || ((unsigned char)(x) == '[') || ((unsigned char)(x) == ']'))

#define IS_SPACE(x) (((unsigned char)(x) == ' '))

/*
 * alphanummark = alphanum | mark
 */
#define IS_ALPHANUMMARK(x) (IS_ALPHANUM(x) || IS_MARK(x) || IS_SPACE(x))


/*
 * Read little-endian values of various sizes from memory buffers.
 */
#define  _READ_BYTE(buf,offset)  ((UINT32)(UINT8)(((UINT8 *)(buf))[(offset)]))
#define  _READ_BYTE_SHIFT(buf,offset,shift)  (((UINT32)(UINT8)(((UINT8 *)(buf))[(offset)])) << (shift))

#define  READ_INT8(buf)    ((INT8)(_READ_BYTE((buf), 0)))
#define  READ_UINT8(buf)    ((UINT8)(_READ_BYTE((buf), 0)))
#define  READ_INT16(buf)    ((INT16)(_READ_BYTE((buf), 0) | _READ_BYTE_SHIFT((buf), 1, 8)))
#define  READ_UINT16(buf)  ((UINT16)(_READ_BYTE((buf), 0) | _READ_BYTE_SHIFT((buf), 1, 8)))
#define  READ_INT32(buf)    ((INT32)(_READ_BYTE((buf), 0) | _READ_BYTE_SHIFT((buf), 1, 8) | _READ_BYTE_SHIFT((buf), 2, 16) | _READ_BYTE_SHIFT((buf), 3, 24)))
#define  READ_UINT32(buf)  ((UINT32)(_READ_BYTE((buf), 0) | _READ_BYTE_SHIFT((buf), 1, 8) | _READ_BYTE_SHIFT((buf), 2, 16) | _READ_BYTE_SHIFT((buf), 3, 24)))
#define  READ_INT64(buf)    (((INT64)(READ_UINT32((buf)))) | (((INT64)(READ_INT32(((UINT8 *)(buf)) + 4))) << 32))
#define  READ_UINT64(buf)  (((UINT64)(READ_UINT32((buf)))) | (((UINT64)(READ_UINT32(((UINT8 *)(buf)) + 4))) << 32))

typedef enum DATA_TYPES
{
	DATA_INT8	= 1,
	DATA_INT16	= 2,
	DATA_INT32	= 3,
	DATA_BINARY	= 4,
	DATA_STRING	= 5

} DATA_TYPES;

typedef enum FILE_TYPES
{
	FILE_BIN = 1,
	FILE_HEX = 2,
	FILE_XML = 3
	
} FILE_TYPES;

typedef struct SETTINGS
{
	char ifile[MAX_FILENAME];	// input filename
	FILE_TYPES ifile_type;		// input file type
	char ofile[MAX_FILENAME];	// output filename
	FILE_TYPES ofile_type;		// output file type
	int verbose;				// verbose mode
	int detect_strings;			// detect strings from binary data
	int detect_numbers;			// detect numberic data from binary
} SETTINGS;

// gfx main header
typedef struct GFX_HEADER
{
	unsigned int filesize;				// filesize
	unsigned int var1;					// unknown
	unsigned int countofblocks;			// count of datablocks
	struct GFX_BLOCKHEADER * blocks;	// pointer to datablock		
} GFX_HEADER;

// gfx block header
typedef struct GFX_BLOCKHEADER
{
	unsigned int blocksize;			// datablock size
	unsigned int records;			// records count
	EFI_DEVICE_PATH *devpath;		// device address binary
	unsigned int devpath_len;		// device address binary len	
	struct GFX_ENTRY * entries;		// pointer to block entries
	struct GFX_BLOCKHEADER * next;	// pointer to next datablock	 
} GFX_BLOCKHEADER;

// gfx data entries
typedef struct GFX_ENTRY
{
	unsigned char *bkey;	// unicode key binary value
	unsigned int bkey_len;	// binary unicode key length
	char *key;				// ascii key value
	unsigned int key_len;	// ascii key length
	unsigned char *val;		// data binary value
	unsigned int val_len;	// binary data length
	DATA_TYPES val_type;	// binary data type
	struct GFX_ENTRY * next;	
} GFX_ENTRY;

GFX_HEADER *parse_binary(CFTypeRef dp, SETTINGS settings);
CFDictionaryRef CreateGFXDictionary(GFX_HEADER * gfx);

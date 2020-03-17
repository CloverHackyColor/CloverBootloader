#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "utils.h"

void assertion(int condition, char * message)
{
    if (condition == 0)
    {
        fprintf(stderr, "error: %s.\n", message);
        exit(1);
    }
}
   
unsigned long UTF16ReadChar(const unsigned short *str, int len, int *posn)
{
	unsigned long ch = (unsigned long)(str[*posn]);
	unsigned long low;
	if(ch < (unsigned long)0xD800 || ch > (unsigned long)0xDBFF)
	{
		/* Regular 16-bit character, or a low surrogate in the
		   wrong position for UTF-16 */
		++(*posn);
		return ch;
	}
	else if((*posn + 2) <= len &&
		    (low = (unsigned long)(str[*posn + 1])) >= (unsigned long)0xDC00 &&
			low <= (unsigned long)0xDFFF)
	{
		/* Surrogate character */
		*posn += 2;
		return ((ch - (unsigned long)0xD800) << 10) + (low & 0x03FF) +
			   (unsigned long)0x10000;
	}
	else
	{
		/* High surrogate without a low surrogate following it */
		++(*posn);
		return ch;
	}
}
typedef unsigned short  UINT16;
typedef unsigned char  UINT8;

unsigned long UTF16ReadCharAsBytes(const void *_str, int len, int *posn)
{
	const unsigned char *str = (const unsigned char *)_str;
	unsigned long ch;
	unsigned long low;
	if((*posn + 2) > len)
	{
		/* We have a character left over, which is an error.
		   But we have to do something, so return it as-is */
		ch = (unsigned long)(str[*posn]);
		++(*posn);
		return ch;
	}
//	ch = (unsigned long)(READ_UINT16(str + *posn));
  ch = (unsigned long)str[*posn] + (unsigned long)str[*posn+1] * 256;
	*posn += 2;
	if(ch < (unsigned long)0xD800 || ch > (unsigned long)0xDBFF)
	{
		/* Regular 16-bit character, or a low surrogate in the
		   wrong position for UTF-16 */
		return ch;
	}
	if((*posn + 2) > len)
	{
		/* Not enough bytes: return the high surrogate as-is */
		return ch;
	}
//	low = (unsigned long)(READ_UINT16(str + *posn));
  low = (unsigned long)str[*posn] + (unsigned long)str[*posn+1] * 256;
	if(low < (unsigned long)0xDC00 || low > (unsigned long)0xDFFF)
	{
		/* High surrogate without a low surrogate following it */
		return ch;
	}
	*posn += 2;
	return ((ch - (unsigned long)0xD800) << 10) + (low & 0x03FF) +
		   (unsigned long)0x10000;
}

int UTF16WriteChar(unsigned short *buf, unsigned long ch)
{
	if(buf)
	{
		if(ch < (unsigned long)0x10000)
		{
			*buf = (unsigned short)ch;
			return 1;
		}
		else if(ch < (unsigned long)0x110000)
		{
			ch -= 0x10000;
			buf[0] = (unsigned short)((ch >> 10) + 0xD800);
			buf[1] = (unsigned short)((ch & 0x03FF) + 0xDC00);
			return 2;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if(ch < (unsigned long)0x10000)
		{
			return 1;
		}
		else if(ch < (unsigned long)0x110000)
		{
			return 2;
		}
		else
		{
			return 0;
		}
	}
}

int UTF16WriteCharAsBytes(void *_buf, unsigned long ch)
{
	if(_buf)
	{
		unsigned char *buf = (unsigned char *)_buf;
		if(ch < (unsigned long)0x10000)
		{
			buf[0] = (unsigned char)ch;
			buf[1] = (unsigned char)(ch >> 8);
			return 2;
		}
		else if(ch < (unsigned long)0x110000)
		{
			unsigned tempch;
			ch -= 0x10000;
			tempch = (unsigned)((ch >> 10) + 0xD800);
			buf[0] = (unsigned char)tempch;
			buf[1] = (unsigned char)(tempch >> 8);
			tempch = (unsigned)((ch & 0x03FF) + 0xDC00);
			buf[2] = (unsigned char)tempch;
			buf[3] = (unsigned char)(tempch >> 8);
			return 4;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if(ch < (unsigned long)0x10000)
		{
			return 2;
		}
		else if(ch < (unsigned long)0x110000)
		{
			return 4;
		}
		else
		{
			return 0;
		}
	}
}

unsigned long UTF8ReadChar(const void *_str, int len, int *posn)
{
	const char *str = (const char *)_str;
	char ch = str[*posn];
	unsigned long result;
	if((ch & 0x80) == 0)
	{
		/* Single-byte UTF-8 encoding */
		++(*posn);
		return (unsigned long)ch;
	}
	else if((ch & (char)0xE0) == (char)0xC0 && (*posn + 2) <= len)
	{
		/* Two-byte UTF-8 encoding */
		result = ((((unsigned long)(ch & 0x1F)) << 6) |
		           ((unsigned long)(str[(*posn) + 1] & 0x3F)));
		(*posn) += 2;
		return result;
	}
	else if((ch & (char)0xF0) == (char)0xE0 && (*posn + 3) <= len)
	{
		/* Three-byte UTF-8 encoding */
		result = ((((unsigned long)(ch & 0x0F)) << 12) |
		          (((unsigned long)(str[(*posn) + 1] & 0x3F)) << 6) |
		           ((unsigned long)(str[(*posn) + 2] & 0x3F)));
		(*posn) += 3;
		return result;
	}
	else if((ch & (char)0xF8) == (char)0xF0 && (*posn + 4) <= len)
	{
		/* Four-byte UTF-8 encoding */
		result = ((((unsigned long)(ch & 0x07)) << 18) |
		          (((unsigned long)(str[(*posn) + 1] & 0x3F)) << 12) |
		          (((unsigned long)(str[(*posn) + 2] & 0x3F)) << 6) |
		           ((unsigned long)(str[(*posn) + 3] & 0x3F)));
		(*posn) += 4;
		return result;
	}
	else if((ch & (char)0xFC) == (char)0xF8 && (*posn + 5) <= len)
	{
		/* Five-byte UTF-8 encoding */
		result = ((((unsigned long)(ch & 0x03)) << 24) |
		          (((unsigned long)(str[(*posn) + 1] & 0x3F)) << 18) |
		          (((unsigned long)(str[(*posn) + 2] & 0x3F)) << 12) |
		          (((unsigned long)(str[(*posn) + 3] & 0x3F)) << 6) |
		           ((unsigned long)(str[(*posn) + 4] & 0x3F)));
		(*posn) += 5;
		return result;
	}
	else if((ch & (char)0xFC) == (char)0xFC && (*posn + 6) <= len)
	{
		/* Six-byte UTF-8 encoding */
		result = ((((unsigned long)(ch & 0x03)) << 30) |
		          (((unsigned long)(str[(*posn) + 1] & 0x3F)) << 24) |
		          (((unsigned long)(str[(*posn) + 2] & 0x3F)) << 18) |
		          (((unsigned long)(str[(*posn) + 3] & 0x3F)) << 12) |
		          (((unsigned long)(str[(*posn) + 4] & 0x3F)) << 6) |
		           ((unsigned long)(str[(*posn) + 5] & 0x3F)));
		(*posn) += 6;
		return result;
	}
	else
	{
		/* Invalid UTF-8 encoding: treat as an 8-bit Latin-1 character */
		++(*posn);
		return (((unsigned long)ch) & 0xFF);
	}
}

int UTF8WriteChar(char *str, unsigned long ch)
{
	if(str)
	{
		/* Write the character to the buffer */
		if(!ch)
		{
			/* Encode embedded NUL's as 0xC0 0x80 so that code
			   that uses C-style strings doesn't get confused */
			str[0] = (char)0xC0;
			str[1] = (char)0x80;
			return 2;
		}
		else if(ch < (unsigned long)0x80)
		{
			str[0] = (char)ch;
			return 1;
		}
		else if(ch < (((unsigned long)1) << 11))
		{
			str[0] = (char)(0xC0 | (ch >> 6));
			str[1] = (char)(0x80 | (ch & 0x3F));
			return 2;
		}
		else if(ch < (((unsigned long)1) << 16))
		{
			str[0] = (char)(0xE0 | (ch >> 12));
			str[1] = (char)(0x80 | ((ch >> 6) & 0x3F));
			str[2] = (char)(0x80 | (ch & 0x3F));
			return 3;
		}
		else if(ch < (((unsigned long)1) << 21))
		{
			str[0] = (char)(0xF0 | (ch >> 18));
			str[1] = (char)(0x80 | ((ch >> 12) & 0x3F));
			str[2] = (char)(0x80 | ((ch >> 6) & 0x3F));
			str[3] = (char)(0x80 | (ch & 0x3F));
			return 4;
		}
		else if(ch < (((unsigned long)1) << 26))
		{
			str[0] = (char)(0xF8 | (ch >> 24));
			str[1] = (char)(0x80 | ((ch >> 18) & 0x3F));
			str[2] = (char)(0x80 | ((ch >> 12) & 0x3F));
			str[3] = (char)(0x80 | ((ch >> 6) & 0x3F));
			str[4] = (char)(0x80 | (ch & 0x3F));
			return 5;
		}
		else
		{
			str[0] = (char)(0xFC | (ch >> 30));
			str[1] = (char)(0x80 | ((ch >> 24) & 0x3F));
			str[2] = (char)(0x80 | ((ch >> 18) & 0x3F));
			str[3] = (char)(0x80 | ((ch >> 12) & 0x3F));
			str[4] = (char)(0x80 | ((ch >> 6) & 0x3F));
			str[5] = (char)(0x80 | (ch & 0x3F));
			return 6;
		}
	}
	else
	{
		/* Determine the length of the character */
		if(!ch)
		{
			return 2;
		}
		else if(ch < (unsigned long)0x80)
		{
			return 1;
		}
		else if(ch < (((unsigned long)1) << 11))
		{
			return 2;
		}
		else if(ch < (((unsigned long)1) << 16))
		{
			return 3;
		}
		else if(ch < (((unsigned long)1) << 21))
		{
			return 4;
		}
		else if(ch < (((unsigned long)1) << 26))
		{
			return 5;
		}
		else
		{
			return 6;
		}
	}
}

// Skip the leading white space and '0x' or '0X' of a integer string
char * TrimHexStr (char *Str, int *IsHex)
{
  *IsHex = 0;

  // skip preceeding white space
  while (*Str && *Str == ' ') 
  {
    Str += 1;
  }

  // skip preceeding zeros
  while (*Str && *Str == '0') 
  {
    Str += 1;
  }
  // skip preceeding character 'x'
  if (*Str && (*Str == 'x' || *Str == 'X')) 
  {
    Str += 1;
    *IsHex = 1;
  }

  return Str;
}

// Convert hex string to uint
unsigned int Xtoi (char *Str, unsigned int *Bytes)
{
  unsigned int   u;
  unsigned int   Length;
  
  assert(Str != NULL);

  // convert hex digits
  u = 0;
  Length = sizeof (unsigned int);
  HexStringToBuf ((unsigned char *) &u, &Length, Str, Bytes);

  return u;
}

// Convert hex string to 64 bit data.
void Xtoi64 (char *Str, unsigned long *Data, unsigned int *Bytes)
{
  unsigned int Length;

  *Data = 0;
  Length = sizeof (unsigned long);
  HexStringToBuf ((unsigned char *) Data, &Length, Str, Bytes);
}

// Convert decimal string to uint
unsigned int Dtoi (char *str)
{
  unsigned int   u;
  char  c;
  unsigned int   m;
  unsigned int   n;

  assert(str != NULL);

  m = (unsigned int) -1 / 10;
  n = (unsigned int) -1 % 10;

  // skip preceeding white space
  while (*str && *str == ' ') 
  {
    str += 1;
  }

  // convert digits
  u = 0;
  c = *(str++);
  while (c) 
  {
    if (c >= '0' && c <= '9') 
	  {
      if ((u > m) || ((u == m) && ((c - '0') > (int)n)))
	    {
        return (unsigned int) -1;
      }
      u = (u * 10) + c - '0';
    } else {
      break;
    }
    c = *(str++);
  }

  return u;
}

// Convert decimal string to uint
void Dtoi64 (char *str,unsigned long *Data)
{
  unsigned long   u;
  char   c;
  unsigned long   m;
  unsigned long   n;

  assert(str != NULL);
  assert(Data != NULL);

  // skip preceeding white space
  while (*str && *str == ' ') 
  {
    str += 1;
  }

  // convert digits
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= '0' && c <= '9') 
	{
      m = u << 3;
      n = u << 1;
      u = m + n + c - '0';
    } else {
      break;
    }

    c = *(str++);
  }

  *Data = u;
}

// Convert integer string to uint.
unsigned int Strtoi (char *Str, unsigned int *Bytes)
{
  int IsHex;

  Str = TrimHexStr (Str, &IsHex);

  if (IsHex) 
  {
    return Xtoi (Str, Bytes);
  } else {
    return Dtoi (Str);
  }
}

//  Convert integer string to 64 bit data.
void Strtoi64 (char *Str, unsigned long *Data, unsigned int *Bytes)
{
  int IsHex;

  Str = TrimHexStr (Str, &IsHex);

  if (IsHex) {
    Xtoi64 (Str, Data, Bytes);
  } else {
    Dtoi64 (Str, Data);
  }
}

int StrToBuf (unsigned char *Buf, unsigned int BufferLength, char *Str)
{
  unsigned int  Index;
  unsigned int  StrLength;
  unsigned char Digit = 0;
  unsigned char Byte;

  // Two hex char make up one byte
  StrLength = BufferLength * sizeof (char);

  for(Index = 0; Index < StrLength; Index++, Str++) 
  {

    IsCharHexDigit (&Digit, *Str);

    // For odd charaters, write the upper nibble for each buffer byte,
    // and for even characters, the lower nibble.
	
    if ((Index & 1) == 0) 
	{
      Byte = Digit << 4;
    } else {
      Byte = Buf[Index / 2];
      Byte &= 0xF0;
      Byte |= Digit;
    }

    Buf[Index / 2] = Byte;
  }

  return 1;
}

/* Converts Unicode string to binary buffer.
 * The conversion may be partial.
 * The first character in the string that is not hex digit stops the conversion.
 * At a minimum, any blob of data could be represented as a hex string.
 */
int HexStringToBuf (unsigned char *Buf, unsigned int *Len, char *Str, unsigned int *ConvertedStrLen)
{
  unsigned int HexCnt;
  unsigned int Idx;
  unsigned int BufferLength;
  unsigned char Digit;
  unsigned char Byte;

  // Find out how many hex characters the string has.
  for (Idx = 0, HexCnt = 0; IsCharHexDigit (&Digit, Str[Idx]); Idx++, HexCnt++);

  if (HexCnt == 0) 
  {
    *Len = 0;
    return 1;
  }

  // Two Unicode characters make up 1 buffer byte. Round up.
  BufferLength = (HexCnt + 1) / 2; 

  // Test if  buffer is passed enough.
  if (BufferLength > (*Len)) 
  {
    *Len = BufferLength;
    return 0;
  }

  *Len = BufferLength;

  for (Idx = 0; Idx < HexCnt; Idx++) 
  {
    IsCharHexDigit (&Digit, Str[HexCnt - 1 - Idx]);

    // For odd charaters, write the lower nibble for each buffer byte,
    // and for even characters, the upper nibble.
    if ((Idx & 1) == 0) {
      Byte = Digit;
    } else {
      Byte = Buf[Idx / 2];
      Byte &= 0x0F;
      Byte |= Digit << 4;
    }

    Buf[Idx / 2] = Byte;
  }

  if (ConvertedStrLen != NULL) {
    *ConvertedStrLen = HexCnt;
  }

  return 1;
}

// Determines if a Unicode character is a hexadecimal digit.
int IsCharHexDigit (unsigned char *Digit, char Char)
{
  if ((Char >= '0') && (Char <= '9')) 
  {
    *Digit = (unsigned char) (Char - '0');
    return 1;
  }

  if ((Char >= 'A') && (Char <= 'F')) 
  {
    *Digit = (unsigned char) (Char - 'A' + 0xA);
    return 1;
  }

  if ((Char >= 'a') && (Char <= 'f')) 
  {
    *Digit = (unsigned char) (Char - 'a' + 0xA);
    return 1;
  }

  return 0;
}

void CatPrintf(char *target, const char *format, ...)
{
	va_list varargs;
	
	while(*target) target++;
	
	va_start(varargs, format);
	vsprintf(target, format, varargs);
	va_end(varargs);
}

void *MallocCopy(unsigned int size, void *buf)
{
	void *new = NULL;
	
	if( (new = (void *)malloc(size * sizeof(void) ) ) != NULL)
	{
		memcpy(new, buf, size);
	}
	
	return new;
}


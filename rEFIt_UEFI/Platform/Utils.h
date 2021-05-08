//
//All rights reserved. This program and the accompanying materials
//are licensed and made available under the terms and conditions of the BSD License
//which accompanies this distribution. The full text of the license may be found at
//http://opensource.org/licenses/bsd-license.php
//
//THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
//WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//
//Module Name:
//
//  Utils
//
//

#ifndef _UTILS_H_
#define _UTILS_H_

#include "BootLog.h"

//Unicode
#define IS_COMMA(a)                ((a) == L',')
#define IS_HYPHEN(a)               ((a) == L'-')
#define IS_DOT(a)                  ((a) == L'.')
#define IS_LEFT_PARENTH(a)         ((a) == L'(')
#define IS_RIGHT_PARENTH(a)        ((a) == L')')
#define IS_SLASH(a)                ((a) == L'/')
#define IS_NULL(a)                 ((a) == L'\0')
//Ascii
#define IS_DIGIT(a)            (((a) >= '0') && ((a) <= '9'))
#define IS_HEX(a)            ((((a) >= 'a') && ((a) <= 'f')) || (((a) >= 'A') && ((a) <= 'F')))
#define IS_UPPER(a)          (((a) >= 'A') && ((a) <= 'Z'))
#define IS_ALFA(x) (((x >= 'a') && (x <='z')) || ((x >= 'A') && (x <='Z')))
#define IS_ASCII(x) ((x>=0x20) && (x<=0x7F))
#define IS_PUNCT(x) ((x == '.') || (x == '-'))
#define IS_BLANK(x) ((x == ' ') || (x == '\t'))

inline bool isPathSeparator(char32_t c) { return c == '/' || c == '\\'; }


////void        LowCase (IN OUT CHAR8 *Str);
//UINT32      hex2bin(IN const CHAR8 *hex, OUT UINT8 *bin, UINT32 len);
BOOLEAN     IsHexDigit (CHAR8 c);
UINT8       hexstrtouint8 (CONST CHAR8* buf); //one or two hex letters to one byte


#ifdef __cplusplus

#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XBuffer.h"

template <typename T, enable_if( is_char_ptr(T)  ||  is___String(T) )>
size_t hex2bin(const T hex, size_t hexlen, uint8_t *out, size_t outlen)
{
  size_t outidx = 0;
  char  buf[3] = {0,0,0};

  if ( hex == NULL || hexlen <= 0 ) {
    //DBG("[ERROR] bin2hex input error\n"); //this is not error, this is empty value
    return 0;
  }
  if ( out == NULL && outlen > 0 ) {
    DebugLog(1, "[WARN] bin2hex outlen is ignored when out==NULL\n");
  }
  if ( out != NULL && outlen == 0 ) {
    DebugLog(1, "[WARN] bin2hex out is ignored when outlen==0\n");
  }

  for (size_t hexidx = 0; hexidx < hexlen ; )
  {
    while ( hex[hexidx] == 0x20  ||  hex[hexidx] == ','  ||  hex[hexidx] == '\n'  ||  hex[hexidx] == '\r' ) {
      if ( hexidx == hexlen ) {
        return outidx;
      }
      hexidx++; //skip spaces and commas
    }
    if ( hexidx == hexlen-1 ) {
      DebugLog(1, "[ERROR] bin2hex '%.*s' uneven char nuber\n", (int)hexlen, XString8().takeValueFrom(hex).c_str());
      return 0;
    }
    if (!IsHexDigit(hex[hexidx]) || !IsHexDigit(hex[hexidx+1])) {
      if ( hexlen > 200 ) hexlen = 200; // Do not print more than 200 chars.
      DebugLog(1, "[ERROR] bin2hex '%.*s' syntax error\n", (int)hexlen, XString8().takeValueFrom(hex).c_str());
      return 0;
    }
    buf[0] = (char)hex[hexidx++]; // safe cast, it's an hex digit
    buf[1] = (char)hex[hexidx++]; // safe cast, it's an hex digit
    if ( out != NULL ) {
      if ( outidx == outlen ) {
        DebugLog(1, "[ERROR] bin2hex '%.*s' outbuffer not big enough\n", (int)hexlen, XString8().takeValueFrom(hex).c_str());
        return 0;
      }
      out[outidx] = hexstrtouint8(buf);
    }
    ++outidx;
  }
  //bin[outlen] = 0;
  return outidx;
}

size_t hex2bin(const XString8& s, uint8_t *out, size_t outlen);
size_t hex2bin(const XBuffer<char>& buffer, uint8_t *out, size_t outlen);
size_t hex2bin(const XStringW& s, uint8_t *out, size_t outlen);


XString8    Bytes2HexStr(UINT8 *data, UINTN len);

inline UINT64 EFIAPI AsciiStrHexToUint64(const XString8& String)
{
  return AsciiStrHexToUint64(String.c_str());
}

inline UINTN EFIAPI AsciiStrHexToUintn(const XString8& String)
{
  return AsciiStrHexToUintn(String.c_str());
}

inline UINTN EFIAPI AsciiStrDecimalToUintn(const XString8& String)
{
  return AsciiStrDecimalToUintn(String.c_str());
}


UINT32 GetCrc32(UINT8 *Buffer, UINTN Size);




extern BOOLEAN haveError;


#endif // __cplusplus

#endif // _UTILS_H_

/*
 * guid.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_GUID_H_
#define PLATFORM_GUID_H_

#include "../cpp_foundation/XStringArray.h"
#include "../cpp_foundation/unicode_conversions.h"


extern "C" EFI_GUID  gEfiMiscSubClassGuid;

/** Returns TRUE is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
template <typename T, enable_if( is___String(T) )>
BOOLEAN IsValidGuidAsciiString(const T& Str)
{
  UINTN   Index;

  if ( Str.length() != 36 ) {
    return FALSE;
  }

  for (Index = 0; Index < 36; Index++) {
    if (Index == 8 || Index == 13 || Index == 18 || Index == 23) {
      if (Str[Index] != '-') {
        return FALSE;
      }
    } else {
      if (!(
            (Str[Index] >= '0' && Str[Index] <= '9')
            || (Str[Index] >= 'a' && Str[Index] <= 'f')
            || (Str[Index] >= 'A' && Str[Index] <= 'F')
            )
          )
      {
        return FALSE;
      }
    }
  }

  return TRUE;
}


template <typename T, enable_if( is_char_ptr(T)  ||  is___String(T) )>
EFI_STATUS
StrHToBuf (
          OUT UINT8    *Buf,
          IN  UINTN    BufferLength,
          const T& t
          )
{
  UINTN       Index;
  UINTN       StrLength;
  UINT8       Digit;
  UINT8       Byte;

  Digit = 0;
  auto Str = _xstringarray__char_type<T>::getCharPtr(t);

  //
  // Two hex char make up one byte
  //
  StrLength = BufferLength * sizeof (CHAR16);

  for(Index = 0; Index < StrLength; Index++) {

    if ((Str[Index] >= (__typeof__(*Str))'a') && (Str[Index] <= (__typeof__(*Str))'f')) {
      Digit = (UINT8) (Str[Index] - (__typeof__(*Str))'a' + 0x0A);
    } else if ((Str[Index] >= (__typeof__(*Str))'A') && (Str[Index] <= (__typeof__(*Str))'F')) {
      Digit = (UINT8) (Str[Index] - L'A' + 0x0A);
    } else if ((Str[Index] >= (__typeof__(*Str))'0') && (Str[Index] <= (__typeof__(*Str))'9')) {
      Digit = (UINT8) (Str[Index] - (__typeof__(*Str))'0');
    } else {
      return EFI_INVALID_PARAMETER;
    }

    //
    // For odd characters, write the upper nibble for each buffer byte,
    // and for even characters, the lower nibble.
    //
    if ((Index & 1) == 0) {
      Byte = (UINT8) (Digit << 4);
    } else {
      Byte = Buf[Index / 2];
      Byte &= 0xF0;
      Byte = (UINT8) (Byte | Digit);
    }

    Buf[Index / 2] = Byte;
  }

  return EFI_SUCCESS;
}

/*
 * Convert a string to a GUID.
 * First parameter can by of one of these type : char*, char16_t*, char32_t*, wchar_t*, XString8, XString16, XString32, XStringW
 */
template <typename T, enable_if( is_char_ptr(T)  ||  is___String(T) )>
EFI_STATUS
StrToGuidLE (
           const T& t,
           OUT EFI_GUID *Guid
           )
{
  EFI_STATUS Status;
  UINT8 GuidLE[16];
  
  auto Str = _xstringarray__char_type<T>::getCharPtr(t);

  Status = StrHToBuf (&GuidLE[0], 4, Str);
  if ( Status != EFI_SUCCESS ) return Status;

  while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
    Str ++;
  }

  if (IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }

  Status = StrHToBuf (&GuidLE[4], 2, Str);
  if ( Status != EFI_SUCCESS ) return Status;
  while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
    Str ++;
  }

  if (IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }

  Status = StrHToBuf (&GuidLE[6], 2, Str);
  if ( Status != EFI_SUCCESS ) return Status;
  while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
    Str ++;
  }

  if (IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }

  Status = StrHToBuf (&GuidLE[8], 2, Str);
  if ( Status != EFI_SUCCESS ) return Status;
  while (!IS_HYPHEN (*Str) && !IS_NULL (*Str)) {
    Str ++;
  }

  if (IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }

  Status = StrHToBuf (&GuidLE[10], 6, Str);
  if ( Status != EFI_SUCCESS ) return Status;

  if (Guid) {
    CopyMem((UINT8*)Guid, &GuidLE[0], sizeof(EFI_GUID));
  }

  return EFI_SUCCESS;
}


XStringW GuidBeToStr(EFI_GUID *Guid);
XString8 GuidLEToXString8(EFI_GUID *Guid);
XStringW GuidLEToXStringW(EFI_GUID *Guid);



#endif /* PLATFORM_GUID_H_ */

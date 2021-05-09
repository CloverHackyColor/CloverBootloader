/*
 * guid.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_GUID_H_
#define PLATFORM_GUID_H_

#include "Utils.h"
#include "../cpp_foundation/XStringArray.h"
#include "../cpp_foundation/unicode_conversions.h"

extern "C" {
#include <Uefi/UefiBaseType.h> // for EFI_GUID
}

extern "C" EFI_GUID  gEfiMiscSubClassGuid;

/*
 * Wrapper class to bring some syntaxic sugar : initialisation at construction, assignment, == operator, etc.
 */
class EFI_GUIDClass : public EFI_GUID
{
public:
  EFI_GUIDClass() { Data1 = 0; Data2 = 0; Data3 = 0; memset(Data4, 0, sizeof(Data4)); }

  EFI_GUIDClass(const EFI_GUID& other) { Data1 = other.Data1; Data2 = other.Data2; Data3 = other.Data3; memcpy(Data4, other.Data4, sizeof(Data4)); }
  
  bool operator == (const EFI_GUID& other) const {
    if ( !(Data1 == other.Data1) ) return false;
    if ( !(Data2 == other.Data2) ) return false;
    if ( !(Data3 == other.Data3) ) return false;
    if ( !(memcmp(Data4, other.Data4, sizeof(Data4)) == 0) ) return false;
    return true;
  }
};

extern const XString8 nullGuidAsString;
extern EFI_GUID nullGuid;

/** Returns TRUE is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
template <typename T, typename IntegralType, enable_if( is_char(T) && is_integral(IntegralType) ) >
BOOLEAN IsValidGuidString(const T* Str, IntegralType n)
{
  UINTN   Index4IsValidGuidAsciiString; // stupid name to avoid warning : Declaration shadows a variable in the global namespace

  if ( n != 36 ) {
    return FALSE;
  }

  for (Index4IsValidGuidAsciiString = 0; Index4IsValidGuidAsciiString < 36; Index4IsValidGuidAsciiString++) {
    if (Index4IsValidGuidAsciiString == 8 || Index4IsValidGuidAsciiString == 13 || Index4IsValidGuidAsciiString == 18 || Index4IsValidGuidAsciiString == 23) {
      if (Str[Index4IsValidGuidAsciiString] != '-') {
        return FALSE;
      }
    } else {
      if (!(
            (Str[Index4IsValidGuidAsciiString] >= '0' && Str[Index4IsValidGuidAsciiString] <= '9')
            || (Str[Index4IsValidGuidAsciiString] >= 'a' && Str[Index4IsValidGuidAsciiString] <= 'f')
            || (Str[Index4IsValidGuidAsciiString] >= 'A' && Str[Index4IsValidGuidAsciiString] <= 'F')
            )
          )
      {
        return FALSE;
      }
    }
  }

  return TRUE;
}

/** Returns TRUE is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
template <typename T, enable_if( is___String(T) )>
BOOLEAN IsValidGuidString(const T& Str)
{
  if ( Str.isEmpty() ) return false;
  return IsValidGuidString(Str.data(), Str.length());
}

///** Returns TRUE is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
//template <typename T, enable_if( is___String(T) )>
//BOOLEAN IsValidGuidAsciiString(const T& Str)
//{
//  UINTN   Index4IsValidGuidAsciiString; // stupid name to avoid warning : Declaration shadows a variable in the global namespace
//
//  if ( Str.length() != 36 ) {
//    return FALSE;
//  }
//
//  for (Index4IsValidGuidAsciiString = 0; Index4IsValidGuidAsciiString < 36; Index4IsValidGuidAsciiString++) {
//    if (Index4IsValidGuidAsciiString == 8 || Index4IsValidGuidAsciiString == 13 || Index4IsValidGuidAsciiString == 18 || Index4IsValidGuidAsciiString == 23) {
//      if (Str[Index4IsValidGuidAsciiString] != '-') {
//        return FALSE;
//      }
//    } else {
//      if (!(
//            (Str[Index4IsValidGuidAsciiString] >= '0' && Str[Index4IsValidGuidAsciiString] <= '9')
//            || (Str[Index4IsValidGuidAsciiString] >= 'a' && Str[Index4IsValidGuidAsciiString] <= 'f')
//            || (Str[Index4IsValidGuidAsciiString] >= 'A' && Str[Index4IsValidGuidAsciiString] <= 'F')
//            )
//          )
//      {
//        return FALSE;
//      }
//    }
//  }
//
//  return TRUE;
//}


template <typename T, enable_if( is_char_ptr(T)  ||  is___String(T) )>
EFI_STATUS
StrHToBuf (
          OUT UINT8    *Buf,
          IN  UINTN    BufferLength,
          const T& t
          )
{
  UINTN       Index4IsValidGuidAsciiString; // stupid name to avoid warning : Declaration shadows a variable in the global namespace
  UINTN       StrLength;
  UINT8       Digit;
  UINT8       Byte;

  Digit = 0;
  auto Str = _xstringarray__char_type<T>::getCharPtr(t);

  //
  // Two hex char make up one byte
  //
  StrLength = BufferLength * sizeof (CHAR16);

  for(Index4IsValidGuidAsciiString = 0; Index4IsValidGuidAsciiString < StrLength; Index4IsValidGuidAsciiString++) {

    if ((Str[Index4IsValidGuidAsciiString] >= (decltype(*Str))'a') && (Str[Index4IsValidGuidAsciiString] <= (decltype(*Str))'f')) {
      Digit = (UINT8) (Str[Index4IsValidGuidAsciiString] - (decltype(*Str))'a' + 0x0A);
    } else if ((Str[Index4IsValidGuidAsciiString] >= (decltype(*Str))'A') && (Str[Index4IsValidGuidAsciiString] <= (decltype(*Str))'F')) {
      Digit = (UINT8) (Str[Index4IsValidGuidAsciiString] - L'A' + 0x0A);
    } else if ((Str[Index4IsValidGuidAsciiString] >= (decltype(*Str))'0') && (Str[Index4IsValidGuidAsciiString] <= (decltype(*Str))'9')) {
      Digit = (UINT8) (Str[Index4IsValidGuidAsciiString] - (decltype(*Str))'0');
    } else {
      return EFI_INVALID_PARAMETER;
    }

    //
    // For odd characters, write the upper nibble for each buffer byte,
    // and for even characters, the lower nibble.
    //
    if ((Index4IsValidGuidAsciiString & 1) == 0) {
      Byte = (UINT8) (Digit << 4);
    } else {
      Byte = Buf[Index4IsValidGuidAsciiString / 2];
      Byte &= 0xF0;
      Byte = (UINT8) (Byte | Digit);
    }

    Buf[Index4IsValidGuidAsciiString / 2] = Byte;
  }

  return EFI_SUCCESS;
}

/*
 * Convert a string to a GUID.
 * First parameter can by of one of these type : char*, char16_t*, char32_t*, wchar_t*, XString8, XString16, XString32, XStringW
 */
template <typename T, enable_if( is_char_ptr(T)  ||  is___String(T) )>
EFI_STATUS
StrToGuidBE (
           const T& t,
           OUT EFI_GUID *Guid
           )
{
  EFI_STATUS Status;
  UINT8 GuidLE[16];
  
  if ( Guid == NULL ) {
    log_technical_bug("%s : call with Guid==NULL", __PRETTY_FUNCTION__);
    return EFI_UNSUPPORTED;
  }

  auto Str = _xstringarray__char_type<T>::getCharPtr(t);

  if ( Str == NULL ) {
    memset(Guid, 0, sizeof(EFI_GUID));
    return EFI_SUCCESS;
  }


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
    memcpy((UINT8*)Guid, &GuidLE[0], sizeof(EFI_GUID));
  }

  return EFI_SUCCESS;
}


XString8 GuidBeToXString8(const EFI_GUID& Guid);
XStringW GuidBeToXStringW(const EFI_GUID& Guid);
XString8 GuidLEToXString8(const EFI_GUID& Guid);
XStringW GuidLEToXStringW(const EFI_GUID& Guid);



#endif /* PLATFORM_GUID_H_ */

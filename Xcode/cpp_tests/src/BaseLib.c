//
//  BaseLib.c
//  cpp_tests UTF16 signed char
//
//  Created by Jief on 12/10/2020.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#include <ProcessorBind.h>
#include <Base.h>
#include <Library/BaseLib.h>

#include "../../../rEFIt_UEFI/PlatformPOSIX/posix/abort.h"

#if defined(__APPLE__) && defined(__clang__) && __WCHAR_MAX__ <= 0xFFFFu
// 2020-03 : w... function are broken under macOs and clang with short-wchar.
//           Currently with clang version Apple LLVM version 10.0.0 (clang-1000.11.45.5) with High Sierra
//           If it's fixed one day, a version number could added to this #ifdef

#   include "../../../PosixCompilation/xcode_utf_fixed.h"
#else
#   include <wchar.h>
#endif

#include "../../../rEFIt_UEFI/cpp_foundation/unicode_conversions.h"


CHAR16* StrStr (IN CONST CHAR16 *String, IN CONST CHAR16 *SearchString)
{
  #if __WCHAR_MAX__ <= 0xFFFFu
    return (CHAR16*)wcsstr_fixed(String, SearchString);
  #else
    return wcsstr(String, SearchString);
  #endif
}


UINTN StrLen(const wchar_t* String)
{
  #if __WCHAR_MAX__ <= 0xFFFFu
    return wchar_size_of_utf16_string(String);
  #else
    return wchar_size_of_utf32_string((char32_t*)String);
  #endif
}
//
//#if __WCHAR_MAX__ > 0xFFFFu
//UINTN StrLen(const char16_t* String)
//{
//  return wchar_size_of_utf16_string(String);
//}
//#endif



UINTN
EFIAPI
AsciiStrDecimalToUintn (
  IN      CONST CHAR8               *String
  )
{
  if ( !String ) panic("AsciiStrDecimalToUintn : !String");
  UINTN value;
  int ret = sscanf(String, "%llu", &value);
  if ( ret == 0 ) return 0;
  return value;
}


RETURN_STATUS
EFIAPI
AsciiStrDecimalToUintnS (
  IN  CONST CHAR8              *String,
  OUT       CHAR8              **EndPointer,  OPTIONAL
  OUT       UINTN              *Data
  )
{
  *Data = 0;
  if ( !String ) return RETURN_INVALID_PARAMETER;
  int ret = sscanf(String, "%llu", Data);
  if ( EndPointer ) *EndPointer += ret;
  if ( ret == 0 ) return RETURN_INVALID_PARAMETER;
  return RETURN_SUCCESS;
}

UINTN EFIAPI AsciiStrHexToUintn(IN CONST CHAR8 *String)
{
  if ( !String ) return RETURN_INVALID_PARAMETER;
  UINTN value = 0;
  int ret = sscanf(String, "%llx", &value);
  if ( ret == 0 ) return 0;
  return value;
}

///*
// * Not sure it works exactly like EDK. Espscially in case of error.
// */
//RETURN_STATUS
//EFIAPI
//AsciiStrHexToUint64S (
//  IN  CONST CHAR8              *String,
//  OUT       CHAR8              **EndPointer,  OPTIONAL
//  OUT       UINT64             *Data
//  )
//{
//(void)EndPointer;
//  if ( !String ) return RETURN_INVALID_PARAMETER;
//  int ret = sscanf(String, "%llx", Data);
//  if ( ret == 0 ) return 0;
//  return EFI_SUCCESS;
//}

UINT64
EFIAPI
AsciiStrHexToUint64 (
  IN      CONST CHAR8                *String
  )
{
  if ( !String ) return RETURN_INVALID_PARAMETER;
  UINTN value = 0;
  int ret = sscanf(String, "%llx", &value);
  if ( ret == 0 ) return 0;
  return value;
}

UINTN AsciiStrLen(const char* String)
{
  return strlen(String);
}



void CpuDeadLoop(void)
{
  exit(1);
}

//
//  BaseLib.h
//  cpp_tests UTF16 signed char
//
//  Created by Jief on 12/10/2020.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#ifndef BaseLib_h
#define BaseLib_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <Uefi.h>

//UINTN StrLen(const char16_t* String);
UINTN StrLen(const wchar_t* String);
//int StrCmp(const wchar_t* FirstString, const wchar_t* SecondString);
//int StrnCmp(const wchar_t* FirstString, const wchar_t* SecondString, UINTN Length);
//UINTN StrLen(const wchar_t* String);
UINTN AsciiStrLen(const char* String);
//INTN AsciiStrCmp (const char *FirstString,const char *SecondString);

CHAR16* StrStr (IN CONST CHAR16 *String, IN CONST CHAR16 *SearchString);

UINTN
EFIAPI
AsciiStrDecimalToUintn (
  IN      CONST CHAR8               *String
  );

RETURN_STATUS
EFIAPI
AsciiStrDecimalToUintnS (
  IN  CONST CHAR8              *String,
  OUT       CHAR8              **EndPointer,  OPTIONAL
  OUT       UINTN              *Data
  );

RETURN_STATUS
EFIAPI
AsciiStrHexToUintnS (
  IN  CONST CHAR8              *String,
  OUT       CHAR8              **EndPointer,  OPTIONAL
  OUT       UINTN              *Data
  );
UINTN
EFIAPI
AsciiStrHexToUintn (
  IN      CONST CHAR8               *String
  );

//RETURN_STATUS
//EFIAPI
//AsciiStrHexToUint64S (
//  IN  CONST CHAR8              *String,
//  OUT       CHAR8              **EndPointer,  OPTIONAL
//  OUT       UINT64             *Data
//  );

UINT64
EFIAPI
AsciiStrHexToUint64 (
  IN      CONST CHAR8                *String
  );


#ifdef __cplusplus
}
#endif

#endif /* BaseLib_h */

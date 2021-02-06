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

CHAR8 *
EFIAPI
AsciiStrStr (
  IN      CONST CHAR8               *String,
  IN      CONST CHAR8               *SearchString
  );


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

//
///**
//  Returns a 64-bit Machine Specific Register(MSR).
//
//  Reads and returns the 64-bit MSR specified by Index. No parameter checking is
//  performed on Index, and some Index values may cause CPU exceptions. The
//  caller must either guarantee that Index is valid, or the caller must set up
//  exception handlers to catch the exceptions. This function is only available
//  on IA-32 and x64.
//
//  @param  Index The 32-bit MSR index to read.
//
//  @return The value of the MSR identified by Index.
//
//**/
//UINT64
//EFIAPI
//AsmReadMsr64 (
//  IN      UINT32                    Index
//  );
//
///**
//  Writes a 64-bit value to a Machine Specific Register(MSR), and returns the
//  value.
//
//  Writes the 64-bit value specified by Value to the MSR specified by Index. The
//  64-bit value written to the MSR is returned. No parameter checking is
//  performed on Index or Value, and some of these may cause CPU exceptions. The
//  caller must either guarantee that Index and Value are valid, or the caller
//  must establish proper exception handlers. This function is only available on
//  IA-32 and x64.
//
//  @param  Index The 32-bit MSR index to write.
//  @param  Value The 64-bit value to write to the MSR.
//
//  @return Value
//
//**/
//UINT64
//EFIAPI
//AsmWriteMsr64 (
//  IN      UINT32                    Index,
//  IN      UINT64                    Value
//  );
//

//
// Math Services
//

/**
  Shifts a 64-bit integer left between 0 and 63 bits. The low bits are filled
  with zeros. The shifted value is returned.

  This function shifts the 64-bit value Operand to the left by Count bits. The
  low Count bits are set to zero. The shifted value is returned.

  If Count is greater than 63, then ASSERT().

  @param  Operand The 64-bit operand to shift left.
  @param  Count   The number of bits to shift left.

  @return Operand << Count.

**/
UINT64
EFIAPI
LShiftU64 (
  IN      UINT64                    Operand,
  IN      UINTN                     Count
  );

/**
  Multiples a 64-bit unsigned integer by a 64-bit unsigned integer and
  generates a 64-bit unsigned result.

  This function multiples the 64-bit unsigned value Multiplicand by the 64-bit
  unsigned value Multiplier and generates a 64-bit unsigned result. This 64-
  bit unsigned result is returned.

  @param  Multiplicand  A 64-bit unsigned value.
  @param  Multiplier    A 64-bit unsigned value.

  @return Multiplicand * Multiplier.

**/
UINT64
EFIAPI
MultU64x64 (
  IN      UINT64                    Multiplicand,
  IN      UINT64                    Multiplier
  );



#ifdef __cplusplus
}
#endif

#endif /* BaseLib_h */

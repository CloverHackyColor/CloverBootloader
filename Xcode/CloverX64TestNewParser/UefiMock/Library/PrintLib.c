//
//  PrintLib.c
//  cpp_tests
//
//  Created by Jief on 30/01/2021.
//  Copyright © 2021 JF Knudsen. All rights reserved.
//

#include "PrintLib.h"

UINTN
EFIAPI
AsciiSPrint (
  OUT CHAR8        *StartOfBuffer,
  IN  UINTN        BufferSize,
  IN  CONST CHAR8  *FormatString,
  ...
  )
{
  va_list va;
  va_start(va, FormatString);
  int ret = vsnprintf(StartOfBuffer, BufferSize, FormatString, va);
  va_end(va);
  return (UINTN)ret; // vsnprintf seems to always return >= 0. So cast should be safe.
}

UINTN
EFIAPI
AsciiBSPrint (
  OUT CHAR8         *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR8   *FormatString,
  IN  BASE_LIST     Marker
  )
{
  panic("not yet");
}

UINTN
EFIAPI
AsciiVSPrint (
  OUT CHAR8         *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR8   *FormatString,
  IN  VA_LIST       Marker
  )
{
  panic("not yet");
}

UINTN
EFIAPI
SPrintLength (
  IN  CONST CHAR16   *FormatString,
  IN  VA_LIST       Marker
  )
{
  panic("not yet");
}

UINTN
EFIAPI
SPrintLengthAsciiFormat (
  IN  CONST CHAR8   *FormatString,
  IN  VA_LIST       Marker
  )
{
  panic("not yet");
}
  
UINTN
EFIAPI
UnicodeSPrint (
  OUT CHAR16        *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR16  *FormatString,
  ...
  )
{
  panic("not yet");
}

UINTN
EFIAPI
UnicodeVSPrint (
  OUT CHAR16        *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR16  *FormatString,
  IN  VA_LIST       Marker
  )
{
  panic("not yet");
}

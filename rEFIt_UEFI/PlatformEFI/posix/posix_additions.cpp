/*
 * posix_additions.cpp
 *
 *  Created on: Feb 5, 2021
 *      Author: jief
 */

#include "posix_additions.h"
#include "stdio.h"
#include "stddef.h"
/*
 * We need to use AsciiSPrint to be able to use %r and %g
 */
extern "C" {
//#   include <Library/UefiLib.h>
#   include <Library/PrintLib.h>

//      UINTN
//      EFIAPI
//      AsciiSPrint (
//         OUT CHAR8        *StartOfBuffer,
//         IN  UINTN        BufferSize,
//         IN  CONST CHAR8  *FormatString,
//         ...
//         );

}

#include "../../cpp_foundation/XString.h"

static XString8 stdio_static_buf = XString8().takeValueFrom("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX  "); // prealloc stdio_static_buf. It has to be at least 2 chars because of 'while ( n > size - 2 )' in strguid and efiStrError

const char* efiStrError(EFI_STATUS Status)
{
  size_t size = stdio_static_buf.allocatedSize();
  UINTN n = 0;
  n = AsciiSPrint(stdio_static_buf.dataSized(size), size, "%r", Status);
  while ( n > size - 2 )
  {
    size += 10;
    n = AsciiSPrint(stdio_static_buf.dataSized(size), size, "%r", Status);
  }
  return stdio_static_buf.s();
}

//this function print guid in LittleEndian format while we need BigEndian as Apple do
const char* strguid(EFI_GUID* guid)
{
  size_t size = stdio_static_buf.allocatedSize();
  UINTN n = 0;
  n = AsciiSPrint(stdio_static_buf.dataSized(size), size, "%g", guid);
  while ( n > size - 2 )
  {
    size += 10;
    n = AsciiSPrint(stdio_static_buf.dataSized(size), size, "%g", guid);
  }
  return stdio_static_buf.s();
}

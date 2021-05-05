/*
 * posix_additions.cpp
 *
 *  Created on: Feb 5, 2021
 *      Author: jief
 */

#include "posix_additions.h"
#include "stdio.h"

/*
 * We need to use AsciiSPrint to be able to use %r and %g
 */
extern "C" {
//#   include <Library/UefiLib.h>
//#   include <Library/PrintLib.h>

//      UINTN
//      EFIAPI
//      AsciiSPrint (
//         OUT CHAR8        *StartOfBuffer,
//         IN  UINTN        BufferSize,
//         IN  CONST CHAR8  *FormatString,
//         ...
//         );

}

//#include <Platform.h>
#include "../../rEFIt_UEFI/cpp_foundation/XString.h"

XString8 stdio_static_buf = XString8().takeValueFrom("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX  "); // prealloc stdio_static_buf. It has to be at least 2 chars because of 'while ( n > size - 2 )' in strguid and efiStrError


static char efiStrError_buf[90];
const char* efiStrError(EFI_STATUS Status)
{
  if ( !EFI_ERROR(Status) ) {
    snprintf(efiStrError_buf, sizeof(efiStrError_buf), "efi success (%llu,0x%llx)", Status, Status);
  }else{
    snprintf(efiStrError_buf, sizeof(efiStrError_buf), "efi error (%llu,0x%llx)", Status, Status);
  }
  return efiStrError_buf;
}

//this function print guid in LittleEndian format while we need BigEndian as Apple do
const char* strguid(EFI_GUID* guid)
{
  stdio_static_buf.S8Printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
  return stdio_static_buf.s();
}

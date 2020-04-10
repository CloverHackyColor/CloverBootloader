

#include "stdio.h"
#include "stdarg.h"
#include <Library/printf_lite.h>

extern "C" {
#   include <Library/UefiLib.h>
#   include <Library/PrintLib.h>

//	    UINTN
//	    EFIAPI
//	    AsciiSPrint (
//				 OUT CHAR8        *StartOfBuffer,
//				 IN  UINTN        BufferSize,
//				 IN  CONST CHAR8  *FormatString,
//				 ...
//				 );

}

#include "../../cpp_foundation/XString.h"
#include "../../cpp_foundation/XStringW.h"

static XString stdio_static_buf;
static XStringW stdio_static_wbuf;

int printf(const char* format, ...)
{
  va_list     va;

  // AsciiPrint seems no to work with utf8 chars. We have to use Print instead
	va_start (va, format);
	stdio_static_wbuf.vSWPrintf(format, va);
	int ret = (int)Print(L"%s", stdio_static_wbuf.wc_str());
	va_end(va);
	return ret;
}


const char* strerror(EFI_STATUS Status)
{
	UINTN n = 0;
	do {
		stdio_static_buf.CheckSize(stdio_static_buf.length()+10);
		n = AsciiSPrint(stdio_static_buf.dataSized(0, stdio_static_buf.allocatedSize()), stdio_static_buf.allocatedSize(), "%r", Status);
	} while ( n > stdio_static_buf.allocatedSize() - 2 );
	
	return stdio_static_buf.data();
}

//this function print guid in LittleEndian format while we need BigEndian as Apple do
const char* strguid(EFI_GUID* guid)
{
	UINTN n = 0;
	do {
		stdio_static_buf.CheckSize(stdio_static_buf.length()+10);
		n = AsciiSPrint(stdio_static_buf.dataSized(0, stdio_static_buf.allocatedSize()), stdio_static_buf.allocatedSize(), "%g", guid);
	} while ( n > stdio_static_buf.allocatedSize() - 2 );

	return stdio_static_buf.data();
}




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

static XString8 stdio_static_buf = XString8().takeValueFrom("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX  "); // prealloc stdio_static_buf. It has to be at least 2 chars because of 'while ( n > size - 2 )' in strguid and efiStrError
                                                                                                       // = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX  "_XS8 won't work because allocatedSize() will stay 0
static XStringW stdio_static_wbuf;

int vprintf(const char* format, VA_LIST va)
{
  // AsciiPrint seems no to work with utf8 chars. We have to use Print instead
	stdio_static_wbuf.vSWPrintf(format, va);
	stdio_static_wbuf.replaceAll("\n"_XS8, "\r\n"_XS8);
	int ret = (int)Print(L"%s", stdio_static_wbuf.wc_str());
	return ret;
}

int printf(const char* format, ...)
{
  va_list     va;
	va_start (va, format);
	int ret = vprintf(format, va);
	va_end(va);
	return ret;
}


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


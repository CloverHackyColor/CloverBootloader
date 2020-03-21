

#include "stdio.h"
#include "stdarg.h"
#include <Library/printf_lite.h>

extern "C" {
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
}

#include "../../cpp_foundation/XString.h"

int printf(const char* format, ...)
{
  va_list     va;
	char buf[1024]; // that's quick and dirty !!!

	va_start (va, format);
	vsnprintf(buf, sizeof(buf), format, va);
	buf[sizeof(buf)-1] = 0; // just in case
	int ret = (int)AsciiPrint("%a", buf);
	va_end(va);
	return ret;
}


static XString stdio_static_buf;

char* strerror(EFI_STATUS Status)
{
	UINTN n = 0;
	do {
		stdio_static_buf.CheckSize(stdio_static_buf.length()+10);
		n = AsciiSPrint(stdio_static_buf.data(), stdio_static_buf.m_allocatedSize, "%r", Status);
	} while ( n > stdio_static_buf.m_allocatedSize - 2 );
	
	return stdio_static_buf.data();
}

char* strguid(EFI_GUID* guid)
{
	UINTN n = 0;
	do {
		stdio_static_buf.CheckSize(stdio_static_buf.length()+10);
		n = AsciiSPrint(stdio_static_buf.data(), stdio_static_buf.m_allocatedSize, "%g", guid);
	} while ( n > stdio_static_buf.m_allocatedSize - 2 );

	return stdio_static_buf.data();
}


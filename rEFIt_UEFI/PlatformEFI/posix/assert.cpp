
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "assert.h"
#include "abort.h"



void _assert(bool b, const char* format, ...)
{
  if ( !b ) {
    VA_LIST va;
    VA_START(va, format);
    log_technical_bug(format, va); // panic doesn't return
  }
}

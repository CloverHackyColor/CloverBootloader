

#include "../Platform/Platform.h"

extern "C" {
/*
 * memset and memcpy has to be provided for clang
 */


void* memset(void* dst, int ch, UINT64 count) __attribute__ ((used));
void* memcpy(void* dst, const void* src, UINT64 count) __attribute__ ((used));

void* memset(void* dst, int ch, UINT64 count)
{
  SetMem(dst, count, (UINT8)(ch));
  return dst;
}

void* memcpy(void* dst, const void* src, UINT64 count)
{
  CopyMem(dst, src, count);
  return dst;
}

} // extern "C"


CONST CHAR16 *
EFIAPI
ConstStrStr (
  IN      CONST CHAR16              *String,
  IN      CONST CHAR16              *SearchString
  )
{
  return StrStr((CHAR16*)String, SearchString);
}

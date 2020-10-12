//
//  Platform.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <wchar.h>
#include <locale.h>

#include <string>
#include <locale>
#include <codecvt>
#include <vector>

#include "../../../rEFIt_UEFI/cpp_foundation/unicode_conversions.h"

void CpuDeadLoop(void)
{
	exit(1);
}
//
//static char *dull_replace(const char *in, const char *pattern, const char *by, char* res)
//{
////    size_t outsize = strlen(in) + 1;
//    // TODO maybe avoid reallocing by counting the non-overlapping occurences of pattern
////    char *res = malloc(outsize);
//    // use this to iterate over the output
//    size_t resoffset = 0;
//
//    const char *needle;
//    while ( (needle = strstr(in, pattern)) ) {
//        // copy everything up to the pattern
//        memcpy(res + resoffset, in, (size_t)(needle - in));
//        resoffset += (size_t)(needle - in);
//
//        // skip the pattern in the input-string
//        in = needle + strlen(pattern);
//
//        // adjust space for replacement
////        outsize = outsize - strlen(pattern) + strlen(by);
////        res = realloc(res, outsize);
//
//        // copy the pattern
//        memcpy(res + resoffset, by, strlen(by));
//        resoffset += strlen(by);
//    }
//
//    // copy the remaining input
//    strcpy(res + resoffset, in);
//
//    return res;
//}

void DebugLog(INTN DebugMode, const char *FormatString, ...)
{
	(void)DebugMode;

	va_list va;
	va_start(va, FormatString);
	vprintf(FormatString, va);
	va_end(va);
}

void PauseForKey(const wchar_t* msg)
{
	printf("%ls", msg);
	getchar();
}

static char efiStrError_buf[40];
const char* efiStrError(EFI_STATUS Status)
{
  snprintf(efiStrError_buf, sizeof(efiStrError_buf), "efi error %llu(0x%llx)", Status, Status);
  return efiStrError_buf;
}


RETURN_STATUS
EFIAPI
AsciiStrDecimalToUintnS (
  IN  CONST CHAR8              *String,
  OUT       CHAR8              **EndPointer,  OPTIONAL
  OUT       UINTN              *Data
  )
{
  *Data = 0;
  if ( !String ) return RETURN_INVALID_PARAMETER;
  int ret = sscanf(String, "%llu", Data);
  if ( EndPointer ) *EndPointer += ret;
  if ( ret == 0 ) return RETURN_INVALID_PARAMETER;
  return RETURN_SUCCESS;
}

UINTN EFIAPI AsciiStrHexToUintn(IN CONST CHAR8 *String)
{
  if ( !String ) return RETURN_INVALID_PARAMETER;
  UINTN value = 0;
  int ret = sscanf(String, "%llx", &value);
  if ( ret == 0 ) return 0;
  return value;
}

UINTN
EFIAPI
AsciiStrDecimalToUintn (
  IN      CONST CHAR8               *String
  )
{
  if ( !String ) panic("AsciiStrDecimalToUintn : !String");
  UINTN value;
  int ret = sscanf(String, "%llu", &value);
  if ( ret == 0 ) return 0;
  return value;
}






UINTN StrLen(const wchar_t* String)
{
	return size_of_utf_string(String);
}
UINTN StrLen(const char16_t* String)
{
	return size_of_utf_string(String);
}




void* AllocatePool(UINTN  AllocationSize)
{
	return (void*)malloc((size_t)AllocationSize);
}

void* AllocateZeroPool(UINTN  AllocationSize)
{
	void* p = (void*)malloc((size_t)AllocationSize);
	memset(p, 0, (size_t)AllocationSize);
	return p;
}

void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer)
{
	(void)OldSize;
	if ( !OldBuffer ) return AllocatePool(NewSize);
	return (void*)realloc(OldBuffer, (size_t)NewSize);
}

void FreePool(const void* Buffer)
{
	free((void*)Buffer);
}

void ZeroMem(void *Destination, UINTN Length)
{
	memset(Destination, 0, (size_t)Length);
}

void SetMem(void *Destination, UINTN Length, char c)
{
	memset(Destination, c, (size_t)Length);
}

void CopyMem(void *Destination, const void *Source, UINTN Length)
{
	memmove(Destination, Source, (size_t)Length);
}

INTN CompareMem(const void* DestinationBuffer, const void* SourceBuffer, UINTN Length)
{
  return memcmp(SourceBuffer, DestinationBuffer, Length);
}

CHAR16* EfiStrDuplicate (IN CONST CHAR16 *Src)
{
	CHAR16* newS = (CHAR16*)malloc((wcslen_fixed(Src)+1)*sizeof(wchar_t));
	memcpy(newS, Src, (wcslen_fixed(Src)+1)*sizeof(wchar_t));
	return newS;
}

CHAR16* StrStr (IN CONST CHAR16 *String, IN CONST CHAR16 *SearchString)
{
	return (CHAR16*)wcsstr_fixed(String, SearchString);
}

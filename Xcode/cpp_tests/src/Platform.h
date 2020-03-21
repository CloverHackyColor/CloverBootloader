//
//  Platform.h.h
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#ifndef Platform_h_h
#define Platform_h_h

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <wchar.h>
#include "../../../rEFIt_UEFI/cpp_unit_test/unicode_conversions.h"
#include "posix.h"
#include "xcode_utf16.h"


#ifndef __cplusplus
//typedef uint16_t wchar_t;
typedef uint32_t char32_t;
typedef uint16_t char16_t;
#endif


#define IN
#define OUT

#define TRUE true
#define FALSE false

#define VA_LIST va_list
#define VA_START va_start
#define VA_END va_end
#define VA_ARG va_arg
#define VA_COPY va_copy

#define VOID void
#define EFIAPI
#define CONST const
#define EFI_STATUS INT64

typedef UINTN RETURN_STATUS;
#define MAX_BIT     0x8000000000000000ULL
#define ENCODE_ERROR(StatusCode)     ((RETURN_STATUS)(MAX_BIT | (StatusCode)))

#define RETURN_OUT_OF_RESOURCES      ENCODE_ERROR (9)

#define EFI_SUCCESS 0
#define EFI_OUT_OF_RESOURCES      RETURN_OUT_OF_RESOURCES

#define OPTIONAL
#define ASSERT(x)

void CpuDeadLoop(void);
void DebugLog(INTN DebugMode, const char *FormatString, ...);

void PauseForKey(const wchar_t* msg);


void* AllocatePool(UINTN  AllocationSize);
void* AllocateZeroPool(UINTN  AllocationSize);
void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer);
void FreePool(const void* Buffer);

void ZeroMem(void *Destination, UINTN Length);
void SetMem(void *Destination, UINTN Length, char c);
void CopyMem(void *Destination, void *Source, UINTN Length);


//UINTN StrLen(const char16_t* String);
UINTN StrLen(const wchar_t* String);
//int StrCmp(const wchar_t* FirstString, const wchar_t* SecondString);
//int StrnCmp(const wchar_t* FirstString, const wchar_t* SecondString, UINTN Length);
//UINTN StrLen(const wchar_t* String);
//UINTN AsciiStrLen(const char* String);
//INTN AsciiStrCmp (const char *FirstString,const char *SecondString);



#endif /* Platform_h_h */

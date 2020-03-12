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

#define UINTN unsigned long long
#define MAX_UINTN ULONG_MAX
#define BOOLEAN bool
#define INTN int32_t
#define CHAR8 unsigned char
#define CHAR16 char16_t

#define UINT8  uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define INT8  int8_t
#define INT16 int16_t
#define INT32 int32_t
#define INT64 int64_t

#define MAX_INT8    ((INT8)0x7F)
#define MAX_UINT8   ((UINT8)0xFF)
#define MAX_INT16   ((INT16)0x7FFF)
#define MAX_UINT16  ((UINT16)0xFFFF)
#define MAX_INT32   ((INT32)0x7FFFFFFF)
#define MAX_UINT32  ((UINT32)0xFFFFFFFF)
#define MAX_INT64   ((INT64)0x7FFFFFFFFFFFFFFFULL)
#define MAX_UINT64  ((UINT64)0xFFFFFFFFFFFFFFFFULL)


#define IN

#define VA_LIST va_list
#define VA_START va_start
#define VA_END va_end
#define VA_ARG va_arg
#define VOID void
#define EFIAPI
#define CONST const

void CpuDeadLoop(void);
void DebugLog(int DebugMode, const char *FormatString, ...);

void* AllocatePool(UINTN  AllocationSize);
void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer);
void FreePool(const void* Buffer);
void CopyMem(void *Destination, void *Source, UINTN Length);
void PauseForKey(const wchar_t* msg);
int StrCmp(const wchar_t* FirstString, const wchar_t* SecondString);
int StrnCmp(const wchar_t* FirstString, const wchar_t* SecondString, UINTN Length);
UINTN StrLen(const wchar_t* String);
UINTN AsciiStrLen(const char* String);
INTN AsciiStrCmp (const char *FirstString,const char *SecondString);



#endif /* Platform_h_h */

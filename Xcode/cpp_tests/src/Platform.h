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

#define UINTN unsigned long
#define MAX_UINTN ULONG_MAX
#define BOOLEAN bool
#define INTN int
#define CHAR16 char16_t

#define IN

#define VA_LIST va_list
#define VA_START va_start
#define VA_END va_end
#define VA_ARG va_arg


void CpuDeadLoop(void);
void DebugLog(int DebugMode, const char *FormatString, ...);

void* AllocatePool(UINTN  AllocationSize);
void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer);
void FreePool(const void* Buffer);
void CopyMem(void *Destination, void *Source, UINTN Length);
void PauseForKey(const wchar_t* msg);
int StrCmp(const wchar_t* FirstString, const wchar_t* SecondString);
int StrnCmp(const wchar_t* FirstString, const wchar_t* SecondString, UINTN Length);
unsigned int StrLen(const wchar_t* String);
int AsciiStrLen(const char* String);




#endif /* Platform_h_h */

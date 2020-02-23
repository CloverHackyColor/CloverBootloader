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
#include <wchar.h>

void CpuDeadLoop(void)
{

}

void DebugLog(int DebugMode, const char *FormatString, ...)
{
	va_list va;
	va_start(va, FormatString);
	vprintf(FormatString, va);
	va_end(va);
}



void* AllocatePool(UINTN  AllocationSize)
{
	return malloc(AllocationSize);
}
void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer)
{
	if ( !OldBuffer ) return AllocatePool(NewSize);
	return realloc(OldBuffer, NewSize);
}

void FreePool(const void* Buffer)
{
	free((void*)Buffer);
}

void CopyMem(void *Destination, void *Source, UINTN Length)
{
	memmove(Destination, Source, Length);
}

void PauseForKey(const wchar_t* msg)
{
	printf("%ls", msg);
	getchar();
}

int StrCmp(const wchar_t* FirstString, const wchar_t* SecondString)
{
	return wcscmp(FirstString, SecondString);
}

int StrnCmp(const wchar_t* FirstString, const wchar_t* SecondString, UINTN Length)
{
	return wcsncmp(FirstString, SecondString, Length);
}

int StrLen(const wchar_t* String)
{
	return (int)wcslen(String);
}

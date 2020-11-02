//
//  Platform.h.h
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#ifndef Platform_h_h
#define Platform_h_h

#ifdef _MSC_VER
#include <Windows.h>
#endif

#include "Uefi.h"
#include "../Include/Library/Base.h"
#include "../Include/Library/BaseLib.h"
#include "../Include/Library/BaseMemoryLib.h"
#include "../../../rEFIt_UEFI/Platform/Utils.h"
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <wchar.h>
#include "posix.h"

#ifndef __cplusplus
//typedef uint16_t wchar_t;
typedef uint32_t char32_t;
typedef uint16_t char16_t;
#endif



#include "../../../rEFIt_UEFI/Platform/Posix/abort.h"
#include "../../../rEFIt_UEFI/cpp_foundation/unicode_conversions.h"
#include "../../../rEFIt_UEFI/cpp_foundation/XString.h"
#include "../../../rEFIt_UEFI/cpp_foundation/XObjArray.h"

#include "xcode_utf_fixed.h"


void CpuDeadLoop(void);
void DebugLog(INTN DebugMode, const char *FormatString, ...);
#define MsgLog ::printf

void PauseForKey(const wchar_t* msg);

const char* efiStrError(EFI_STATUS Status);





void* AllocatePool(UINTN  AllocationSize);
void* AllocateZeroPool(UINTN  AllocationSize);
void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer);
void FreePool(const void* Buffer);

//void ZeroMem(void *Destination, UINTN Length);
//void SetMem(void *Destination, UINTN Length, char c);
//void CopyMem(void *Destination, const void *Source, UINTN Length);
//INTN CompareMem(const void* DestinationBuffer, const void* SourceBuffer, UINTN Length);

CHAR16* EfiStrDuplicate (IN CONST CHAR16 *Src);


#endif /* Platform_h_h */

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


#ifdef __cplusplus
extern "C" {
#endif

#include "Uefi.h"
#include "../Include/Library/Base.h"
#include "../Include/Library/BaseLib.h"
#include "../Include/Library/BaseMemoryLib.h"
#include "../Include/Library/MemoryAllocationLib.h"
#include <BootLog.h>

#ifdef __cplusplus
}
#endif


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

#ifdef __cplusplus
#include "../../../rEFIt_UEFI/cpp_foundation/XString.h"
#include "../../../rEFIt_UEFI/cpp_foundation/XObjArray.h"
#endif
#include "../../../rEFIt_UEFI/Platform/Utils.h"

#include "xcode_utf_fixed.h"


void CpuDeadLoop(void);

void PauseForKey(const wchar_t* msg);

const char* efiStrError(EFI_STATUS Status);

CHAR16* EfiStrDuplicate (IN CONST CHAR16 *Src);

#define DEBUG_VERBOSE 0
#define DEBUG( expression )

#endif /* Platform_h_h */
